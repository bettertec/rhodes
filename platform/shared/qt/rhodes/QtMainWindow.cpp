/*------------------------------------------------------------------------
* (The MIT License)
* 
* Copyright (c) 2008-2011 Rhomobile, Inc.
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
* 
* http://rhomobile.com
*------------------------------------------------------------------------*/

#ifdef _MSC_VER
#pragma warning(disable:4018)
#pragma warning(disable:4996)
#endif
#include "QtMainWindow.h"
#include "ui_QtMainWindow.h"
#include "ExternalWebView.h"
#include "RhoSimulator.h"
#include <QResizeEvent>
#include <QWebFrame>
#include <QWebSettings>
#include <QWebSecurityOrigin>
#include <QWebHistory>
#include <QLabel>
#include <QtNetwork/QNetworkCookie>
#include <QFileDialog>
#include <QDesktopServices>
#include "ext/rho/rhoruby.h"
#include "common/RhoStd.h"
#include "common/RhodesApp.h"
#include "rubyext/WebView.h"
#include "rubyext/NativeToolbarExt.h"
#undef null
#include "DateTimeDialog.h"

#if defined(OS_MACOSX) || defined(OS_LINUX)
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif

#ifdef OS_SYMBIAN
#include "qwebviewselectionsuppressor.h"
#include "qwebviewkineticscroller.h"
#endif

IMPLEMENT_LOGCLASS(QtMainWindow,"MainWindow");

extern "C" {
    extern VALUE rb_thread_main(void);
    extern VALUE rb_thread_wakeup(VALUE thread);
}

QtMainWindow::QtMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::QtMainWindow),
    webInspectorWindow(new QtWebInspector()),
    cb(NULL),
    cur_tbrp(0),
    m_alertDialog(0),
    m_LogicalDpiX(0),
    m_LogicalDpiY(0)
    //TODO: m_SyncStatusDlg
{
#ifdef OS_WINDOWS_DESKTOP
    QPixmap icon(":/images/rho.png");
    QApplication::setWindowIcon(icon);
#endif

    ui->setupUi(this);

    QWebSettings* qs = QWebSettings::globalSettings(); //this->ui->webView->settings();
    qs->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
    qs->setAttribute(QWebSettings::LocalStorageEnabled, true);
    qs->setAttribute(QWebSettings::OfflineStorageDatabaseEnabled, true);
    qs->setAttribute(QWebSettings::OfflineWebApplicationCacheEnabled, true);
    qs->setOfflineStorageDefaultQuota(1024*1024*1024);

    rho::String rs_dir = RHODESAPP().getRhoRootPath()+RHO_EMULATOR_DIR;
    qs->setOfflineWebApplicationCachePath(rs_dir.c_str());
    qs->setLocalStoragePath(rs_dir.c_str());
    qs->setOfflineStoragePath(rs_dir.c_str());

    this->ui->webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    this->ui->webView->page()->mainFrame()->securityOrigin().setDatabaseQuota(1024*1024*1024);
    this->main_webView = this->ui->webView;
    this->main_webInspector = webInspectorWindow->webInspector();
    this->cur_webInspector = this->main_webInspector;

    this->move(0,0);
    this->ui->toolBar->hide();
    this->ui->toolBarRight->hide();

    // connecting WebInspector
    main_webInspector->setPage(ui->webView->page());

#ifdef OS_SYMBIAN
    QWebViewKineticScroller *newScroller = new QWebViewKineticScroller();
    newScroller->setWidget(this->ui->webView);
    QWebViewSelectionSuppressor* suppressor = new QWebViewSelectionSuppressor(this->ui->webView);
#endif

    webInspectorWindow->show();
}

QtMainWindow::~QtMainWindow()
{
    tabbarRemoveAllTabs(false);
    if (m_alertDialog) delete m_alertDialog;
    //TODO: m_SyncStatusDlg
    delete webInspectorWindow;
    delete ui;
}

void QtMainWindow::setCallback(IMainWindowCallback* callback)
{
    cb = callback;
}

void QtMainWindow::hideEvent(QHideEvent *)
{
    if (cb) cb->onActivate(0);
}

void QtMainWindow::showEvent(QShowEvent *)
{
    if (cb) cb->onActivate(1);
}

void QtMainWindow::closeEvent(QCloseEvent *ce)
{
    rb_thread_wakeup(rb_thread_main());
    if (cb) cb->onWindowClose();
    tabbarRemoveAllTabs(false);
    webInspectorWindow->close();
    QMainWindow::closeEvent(ce);
}

void QtMainWindow::resizeEvent(QResizeEvent *event)
{
    m_LogicalDpiX = this->logicalDpiY();
    m_LogicalDpiY = this->logicalDpiY();
    if (cb)
        cb->updateSizeProperties(event->size().width(), event->size().height());
}

bool QtMainWindow::isStarted(void)
{
    return (tabViews.size() > 0) ||
      ( (ui->toolBar->isVisible() || ui->toolBarRight->isVisible()) &&
        ((ui->toolBar->actions().size() > 0) || (ui->toolBarRight->actions().size() > 0))
      );
}

void QtMainWindow::on_actionBack_triggered()
{
    if (ui->webView)
        ui->webView->back();
}

bool QtMainWindow::internalUrlProcessing(const QUrl& url)
{
    int ipos;
    QString sUrl = url.toString();
    if (sUrl.startsWith("mailto:")) {
        QDesktopServices::openUrl(url);
        return true;
    }
    if (sUrl.startsWith("tel:")) {
        sUrl.remove(0, 4);
        if ((ipos = sUrl.indexOf('?')) >= 0) sUrl = sUrl.left(ipos);
        QMessageBox::information(0, "Phone call", "Call to " + sUrl);
        return true;
    }
    if (sUrl.startsWith("sms:")) {
        sUrl.remove(0, 4);
        if ((ipos = sUrl.indexOf('?')) >= 0) sUrl = sUrl.left(ipos);
        QMessageBox::information(0, "SMS", "Send SMS to " + sUrl);
        return true;
    }
    return false;
}

void QtMainWindow::on_webView_linkClicked(const QUrl& url)
{
    QString sUrl = url.toString();
    if (sUrl.contains("rho_open_target=_blank")) {
        if (cb) cb->logEvent("WebView: open external browser");
        ExternalWebView* externalWebView = new ExternalWebView();
        externalWebView->navigate(QUrl(sUrl.remove("rho_open_target=_blank")));
        externalWebView->show();
        externalWebView->activateWindow();
    } else if (ui->webView) {
        if (!internalUrlProcessing(url)) {
            sUrl.remove(QRegExp("#+$"));
            if (sUrl.compare(ui->webView->url().toString())!=0) {
#ifdef OS_MACOSX
                if (cb && !sUrl.startsWith("javascript:", Qt::CaseInsensitive))
                    cb->onWebViewUrlChanged(sUrl.toStdString());
#endif
                ui->webView->load(QUrl(sUrl));
            }
        }
    }
}

void QtMainWindow::on_webView_loadStarted()
{
    if (cb) cb->logEvent("WebView: loading...");
}

void QtMainWindow::on_webView_loadFinished(bool ok)
{
    if (cb) {
        cb->logEvent((ok?"WebView: loaded ":"WebView: failed "));
#ifdef OS_MACOSX
        if (ok) cb->onWebViewUrlChanged(ui->webView->url().toString().toStdString());
#endif
    }
}

void QtMainWindow::on_webView_urlChanged(QUrl url)
{
    if (cb) {
#ifdef OS_MACOSX
        ::std::string sUrl = url.toString().toStdString();
        cb->logEvent("WebView: URL changed to " + sUrl);
        cb->onWebViewUrlChanged(sUrl);
#else
        cb->logEvent("WebView: URL changed");
#endif
    }
}

void QtMainWindow::on_menuMain_aboutToShow()
{
    if (cb) cb->createCustomMenu();
}

void QtMainWindow::navigate(QString url, int index)
{
    QWebView* wv = (index < tabViews.size()) && (index >= 0) ? tabViews[index] : ui->webView;
    if (wv) {
        if (url.startsWith("javascript:", Qt::CaseInsensitive)) {
            url.remove(0,11);
            wv->stop();
            wv->page()->mainFrame()->evaluateJavaScript(url);
        } else if (!internalUrlProcessing(url)) {
#ifdef OS_MACOSX
            if (cb) cb->onWebViewUrlChanged(url.toStdString());
#endif
            wv->load(QUrl(url));
        }
    }
}

void QtMainWindow::GoBack(void)
{
    if (ui->webView)
        ui->webView->back();
}

void QtMainWindow::GoForward(void)
{
    if (ui->webView)
        ui->webView->forward();
}

void QtMainWindow::Refresh(int index)
{
    QWebView* wv = (index < tabViews.size()) && (index >= 0) ? tabViews[index] : ui->webView;
    if (wv) {
#ifdef OS_MACOSX
        if (cb) cb->onWebViewUrlChanged(wv->url().toString().toStdString());
#endif
        wv->reload();
    }
}

int QtMainWindow::getLogicalDpiX()
{
    return m_LogicalDpiX;
}

int QtMainWindow::getLogicalDpiY()
{
    return m_LogicalDpiY;
}


// Tabbar:

void QtMainWindow::tabbarRemoveAllTabs(bool restore)
{
    // removing WebViews
    for (int i=0; i<tabViews.size(); ++i) {
        tabbarDisconnectWebView(tabViews[i], tabInspect[i]);
        if (tabViews[i] != main_webView) {
            ui->verticalLayout->removeWidget(tabViews[i]);
            tabViews[i]->setParent(0);
            if (ui->webView == tabViews[i])
                ui->webView = 0;
            delete tabViews[i];
        }
        if (tabInspect[i] != main_webInspector) {
            webInspectorWindow->removeWebInspector(tabInspect[i]);
            if (cur_webInspector==tabInspect[i])
                cur_webInspector = 0;
            delete tabInspect[i];
        }
    }
    tabViews.clear();
    tabInspect.clear();

    // removing Tabs
    for (int i=ui->tabBar->count()-1; i >= 0; --i)
        ui->tabBar->removeTab(i);

    // restoring main WebView
    tabbarWebViewRestore(restore);
}

void QtMainWindow::tabbarInitialize()
{
    if (ui->webView)
        ui->webView->stop();
    tabbarRemoveAllTabs(false);
    ui->tabBar->clearStyleSheet();
}

int QtMainWindow::tabbarAddTab(const QString& label, const char* icon, bool disabled, const QColor* web_bkg_color, QTabBarRuntimeParams& tbrp)
{
    QWebView* wv = main_webView;
    QWebInspector* wI = main_webInspector;
    if (!tbrp["use_current_view_for_tab"].toBool()) {
        // creating web view
        wv = new QWebView();
        wv->setMaximumSize(0,0);
        wv->setParent(ui->centralWidget);
        ui->verticalLayout->addWidget(wv);
        wv->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
        wv->page()->mainFrame()->securityOrigin().setDatabaseQuota(1024*1024*1024);
        if (web_bkg_color && (web_bkg_color->name().length()>0))
            wv->setHtml( QString("<html><body style=\"background:") + web_bkg_color->name() + QString("\"></body></html>") );
        // creating and attaching web inspector
        wI = new QWebInspector();
        wI->setWindowTitle("Web Inspector");
        wI->setPage(wv->page());
    }

    tabViews.push_back(wv);
    tabInspect.push_back(wI);
    webInspectorWindow->addWebInspector(wI);

    cur_tbrp = &tbrp;
    if (icon && (strlen(icon) > 0))
        ui->tabBar->addTab(QIcon(QString(icon)), label);
    else
        ui->tabBar->addTab(label);
    ui->tabBar->setTabToolTip(ui->tabBar->count()-1, label);
    if (disabled)
        ui->tabBar->setTabEnabled(ui->tabBar->count()-1, false);
    cur_tbrp = 0;
	ui->tabBar->setTabData(ui->tabBar->count()-1, tbrp);

    return ui->tabBar->count() - 1;
}

void QtMainWindow::tabbarShow()
{
    ui->tabBar->show();
}

void QtMainWindow::tabbarConnectWebView(QWebView* webView, QWebInspector* webInspector)
{
    if (webView) {
        webView->setMaximumSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX); //->show();
        if (webView != main_webView) {
            QObject::connect(webView, SIGNAL(linkClicked(const QUrl&)), this, SLOT(on_webView_linkClicked(const QUrl&)));
            QObject::connect(webView, SIGNAL(loadStarted()), this, SLOT(on_webView_loadStarted()));
            QObject::connect(webView, SIGNAL(loadFinished(bool)), this, SLOT(on_webView_loadFinished(bool)));
            QObject::connect(webView, SIGNAL(urlChanged(QUrl)), this, SLOT(on_webView_urlChanged(QUrl)));
        }
        ui->webView = webView;
    }
    if (webInspectorWindow) {
        webInspectorWindow->showWebInspector(webInspector);
        cur_webInspector = webInspector;
    }
}

void QtMainWindow::tabbarDisconnectWebView(QWebView* webView, QWebInspector* webInspector)
{
    if (webView) {
        webView->setMaximumSize(0,0); //->hide();
        if (webView != main_webView) {
            QObject::disconnect(webView, SIGNAL(linkClicked(const QUrl&)), this, SLOT(on_webView_linkClicked(const QUrl&)));
            QObject::disconnect(webView, SIGNAL(loadStarted()), this, SLOT(on_webView_loadStarted()));
            QObject::disconnect(webView, SIGNAL(loadFinished(bool)), this, SLOT(on_webView_loadFinished(bool)));
            QObject::disconnect(webView, SIGNAL(urlChanged(QUrl)), this, SLOT(on_webView_urlChanged(QUrl)));
        }
    }
    if (webInspector)
        webInspectorWindow->hideWebInspector(webInspector);
}

void QtMainWindow::tabbarWebViewRestore(bool reload)
{
    if ((ui->webView == 0) || (ui->webView != main_webView)) {
        tabbarDisconnectWebView(ui->webView, cur_webInspector);
        tabbarConnectWebView(main_webView, main_webInspector);
    } else if (reload) {
        main_webView->setMaximumSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX); //->show();
        main_webInspector->setMaximumSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX); //->show();
    }
}

void QtMainWindow::tabbarHide()
{
    tabbarRemoveAllTabs(true);
}

int QtMainWindow::tabbarGetHeight()
{
    return ui->tabBar->height();
}

void QtMainWindow::tabbarSwitch(int index)
{
    ui->tabBar->setCurrentIndex(index);
}

int QtMainWindow::tabbarGetCurrent()
{
    return tabViews.size() > 0 ? ui->tabBar->currentIndex() : 0;
}

void QtMainWindow::tabbarSetBadge(int index, char* badge)
{
    ui->tabBar->setTabButton(index, QTabBar::RightSide, (badge && (*badge != '\0') ? new QLabel(badge) : 0));
}

void QtMainWindow::on_tabBar_currentChanged(int index)
{
    if ((index < tabViews.size()) && (index >= 0)) {
        QTabBarRuntimeParams tbrp = cur_tbrp != 0 ? *cur_tbrp : ui->tabBar->tabData(index).toHash();
        bool use_current_view_for_tab = tbrp["use_current_view_for_tab"].toBool();

        webInspectorWindow->setWindowTitle(QString("Web Inspector - Tab ") + QVariant(index+1).toString() +
            QString(" - ") + tbrp["label"].toString());

        if (use_current_view_for_tab) {
            tabbarConnectWebView(main_webView, main_webInspector);
        } else {
            tabbarDisconnectWebView(main_webView, main_webInspector);
        }

        for (unsigned int i=0; i<tabViews.size(); ++i) {
            if (tabViews[i] != main_webView) {
                if (use_current_view_for_tab || (i != index)) {
                    tabbarDisconnectWebView(tabViews[i], tabInspect[i]);
                } else {
                    tabbarConnectWebView(tabViews[i], tabInspect[i]);
                }
            }
        }

        if (tbrp["on_change_tab_callback"].toString().length() > 0) {
            QString body = QString("&rho_callback=1&tab_index=") + QVariant(index).toString();
            rho::String* cbStr = new rho::String(tbrp["on_change_tab_callback"].toString().toStdString());
            rho::String* bStr = new rho::String(body.toStdString());
            const char* b = bStr->c_str();
            rho_net_request_with_data(RHODESAPP().canonicalizeRhoUrl(*cbStr).c_str(), b);
        }

        if (tbrp["reload"].toBool() || (ui->webView && (ui->webView->history()->count()==0))) {
            rho::String* strAction = new rho::String(tbrp["action"].toString().toStdString());
            RHODESAPP().loadUrl(*strAction);
        }
    }
}


// Toolbar:

void QtMainWindow::toolbarRemoveAllButtons()
{
    ui->toolBar->clear();
    ui->toolBarRight->clear();
}

void QtMainWindow::toolbarShow()
{
    ui->toolBar->show();
    ui->toolBarRight->show();
}

void QtMainWindow::toolbarHide()
{
    ui->toolBar->hide();
    ui->toolBarRight->hide();
}

int QtMainWindow::toolbarGetHeight()
{
    return ui->toolBar->height();
}

void QtMainWindow::toolbarAddAction(const QString & text)
{
    ui->toolBar->addAction(text);
}

void QtMainWindow::toolbarActionEvent(bool checked)
{
    QObject* sender = QObject::sender();
    QAction* action;
    if (sender && (action = dynamic_cast<QAction*>(sender))) {
        rho::String* strAction = new rho::String(action->data().toString().toStdString());
        if ( strcasecmp(strAction->c_str(), "forward") == 0 )
            rho_webview_navigate_forward();
        else
            RHODESAPP().loadUrl(*strAction);
    }
}

void QtMainWindow::toolbarAddAction(const QIcon & icon, const QString & text, const char* action, bool rightAlign)
{
    QAction* qAction = new QAction(icon, text, ui->toolBar);
    qAction->setData(QVariant(action));
    QObject::connect(qAction, SIGNAL(triggered(bool)), this, SLOT(toolbarActionEvent(bool)) );
    if (rightAlign)
        ui->toolBarRight->insertAction( (ui->toolBarRight->actions().size() > 0 ? ui->toolBarRight->actions().last() : 0), qAction);
    else
        ui->toolBar->addAction(qAction);
}

void QtMainWindow::toolbarAddSeparator()
{
    ui->toolBar->addSeparator();
}

void QtMainWindow::setToolbarStyle(bool border, QString background)
{
    QString style = "";
    if (!border) style += "border:0px";
    if (background.length()>0) {
        if (style.length()>0) style += ";";
        style += "background:"+background;
    }
    if (style.length()>0) {
        style = "QToolBar{"+style+"}";
        ui->toolBar->setStyleSheet(style);
        ui->toolBarRight->setStyleSheet(style);
    }
}


// Menu:

void QtMainWindow::menuAddAction(const QString & text, int item)
{
    QAction* qAction = new QAction(text, ui->toolBar);
    qAction->setData(QVariant(item));
    QObject::connect(qAction, SIGNAL(triggered(bool)), this, SLOT(menuActionEvent(bool)) );
    ui->menuMain->addAction(qAction);
}

void QtMainWindow::menuClear(void)
{
    ui->menuMain->clear();
}

void QtMainWindow::menuAddSeparator()
{
    ui->menuMain->addSeparator();
}

void QtMainWindow::menuActionEvent(bool checked)
{
    QObject* sender = QObject::sender();
    QAction* action;
    if (sender && (action = dynamic_cast<QAction*>(sender)) && cb)
        cb->onCustomMenuItemCommand(action->data().toInt());
}

void QtMainWindow::on_actionAbout_triggered()
{
    #ifndef RHO_SYMBIAN
    QMessageBox::about(this, RHOSIMULATOR_NAME, RHOSIMULATOR_NAME " v" RHOSIMULATOR_VERSION);
    #endif
}

// slots:

void QtMainWindow::exitCommand()
{
    this->close();
}

void QtMainWindow::navigateBackCommand()
{
    this->GoBack();
}

void QtMainWindow::navigateForwardCommand()
{
    this->GoForward();
}

void QtMainWindow::logCommand()
{
    //TODO: logCommand
    //if ( !m_logView.IsWindow() ) {
    //    LoadLibrary(_T("riched20.dll"));
    //    m_logView.Create(NULL);
    //}
    //m_logView.ShowWindow(SW_SHOWNORMAL);
}

void QtMainWindow::refreshCommand(int tab_index)
{
    this->Refresh(tab_index);
}

void QtMainWindow::navigateCommand(TNavigateData* nd)
{
    if (nd) {
        if (nd->url) {
            this->navigate(QString::fromWCharArray(nd->url), nd->index);
            free(nd->url);
        }
        free(nd);
    }
}

void QtMainWindow::takePicture(char* callbackUrl)
{
    selectPicture(callbackUrl);
}

void QtMainWindow::selectPicture(char* callbackUrl)
{
    bool wasError = false;
    rho::StringW strBlobRoot = rho::common::convertToStringW( RHODESAPP().getBlobsDirPath() );
    QString fileName = QFileDialog::getOpenFileName(this, "Open Image", QString::fromStdWString(strBlobRoot), "Image Files (*.png *.jpg *.gif *.bmp)");
    char image_uri[4096];
    image_uri[0] = '\0';

    if (!fileName.isNull()) {

        int ixExt = fileName.lastIndexOf('.');
        QString szExt = (ixExt >= 0) && (fileName.lastIndexOf('/') < ixExt) ? fileName.right(fileName.length()-ixExt) : "";
        QDateTime now = QDateTime::currentDateTimeUtc();
        int tz = (int)(now.secsTo(QDateTime::currentDateTime())/3600);

        char file_name[4096];
        ::sprintf(file_name, "Image_%02i-%02i-%0004i_%02i.%02i.%02i_%c%03i%s",
            now.date().month(), now.date().day(), now.date().year(),
            now.time().hour(), now.time().minute(), now.time().second(),
            tz>0?'_':'-',abs(tz),szExt.toStdString().c_str());

        QString full_name = QString::fromStdWString(strBlobRoot);
        full_name.append("/");
        full_name.append(file_name);

        if (QFile::copy(fileName,full_name))
            strcpy( image_uri, file_name );
        else
            wasError = true;
    }

    RHODESAPP().callCameraCallback( (const char*)callbackUrl, rho::common::convertToStringA(image_uri),
        (wasError ? "Error" : ""), fileName.isNull());

    free(callbackUrl);
}

void QtMainWindow::alertShowPopup(CAlertParams * params)
{
    rho::StringW strAppName = RHODESAPP().getAppNameW();

    if (params->m_dlgType == CAlertParams::DLG_STATUS) {
    //    if (m_SyncStatusDlg == NULL) 
    //        m_SyncStatusDlg = new CSyncStatusDlg();
    //    m_SyncStatusDlg->setStatusText(convertToStringW(params->m_message).c_str());
    //    m_SyncStatusDlg->setTitle( convertToStringW(params->m_title).c_str() );
    //    if ( !m_SyncStatusDlg->m_hWnd )
    //        m_SyncStatusDlg->Create(m_hWnd, 0);
    //    else
    //    {
    //        m_SyncStatusDlg->ShowWindow(SW_SHOW);
    //        m_SyncStatusDlg->BringWindowToTop();
    //    }
    } else if (params->m_dlgType == CAlertParams::DLG_DEFAULT) {
        QMessageBox::warning(0, QString::fromWCharArray(strAppName.c_str()),
            QString::fromWCharArray(rho::common::convertToStringW(params->m_message).c_str()));
    } else if (params->m_dlgType == CAlertParams::DLG_CUSTOM) {
        if ( params->m_buttons.size() == 1 && strcasecmp(params->m_buttons[0].m_strCaption.c_str(), "ok") == 0)
            QMessageBox::warning(0, QString::fromWCharArray(rho::common::convertToStringW(params->m_title).c_str()),
                QString::fromWCharArray(rho::common::convertToStringW(params->m_message).c_str()));
        else if (params->m_buttons.size() == 2 && strcasecmp(params->m_buttons[0].m_strCaption.c_str(), "ok") == 0 &&
            strcasecmp(params->m_buttons[1].m_strCaption.c_str(), "cancel") == 0)
        {
            QMessageBox::StandardButton response = QMessageBox::warning(0,
                QString::fromWCharArray(rho::common::convertToStringW(params->m_title).c_str()),
                QString::fromWCharArray(rho::common::convertToStringW(params->m_message).c_str()),
                QMessageBox::Ok | QMessageBox::Cancel);
            int nBtn = response == QMessageBox::Cancel ? 1 : 0;
            RHODESAPP().callPopupCallback(params->m_callback, params->m_buttons[nBtn].m_strID, params->m_buttons[nBtn].m_strCaption);
        } else if (m_alertDialog == NULL) {
            QMessageBox::Icon icon = QMessageBox::NoIcon;
            if (stricmp(params->m_icon.c_str(),"alert")==0) {
                icon = QMessageBox::Warning;
            } else if (stricmp(params->m_icon.c_str(),"question")==0) {
                icon = QMessageBox::Question;
            } else if (stricmp(params->m_icon.c_str(),"info")==0) {
                icon = QMessageBox::Information;
            }
            m_alertDialog = new QMessageBox(icon, //new DateTimeDialog(params, 0);
                QString::fromWCharArray(rho::common::convertToStringW(params->m_title).c_str()),
                QString::fromWCharArray(rho::common::convertToStringW(params->m_message).c_str()));
            m_alertDialog->setStandardButtons(0);
            for (int i = 0; i < (int)params->m_buttons.size(); i++) {
#ifdef OS_SYMBIAN
                if(i == 0)
#endif
                    m_alertDialog->addButton(QString::fromWCharArray(rho::common::convertToStringW(params->m_buttons[i].m_strCaption).c_str()), QMessageBox::ActionRole);
#ifdef OS_SYMBIAN
                else if( i == 1)
                    m_alertDialog->addButton(QString::fromWCharArray(rho::common::convertToStringW(params->m_buttons[i].m_strCaption).c_str()), QMessageBox::RejectRole);
                else if(i == 2)
                    break;
#endif
            }
            m_alertDialog->exec();
            if (m_alertDialog) {
                const QAbstractButton* btn = m_alertDialog->clickedButton();
                if (btn) {
                    for (int i = 0; i < m_alertDialog->buttons().count(); ++i) {
                        if (btn == m_alertDialog->buttons().at(i)) {
#ifdef OS_SYMBIAN
                            RHODESAPP().callPopupCallback(params->m_callback, params->m_buttons[m_alertDialog->buttons().count() - i - 1].m_strID, params->m_buttons[m_alertDialog->buttons().count() - i - 1].m_strCaption);
#else
                            RHODESAPP().callPopupCallback(params->m_callback, params->m_buttons[i].m_strID, params->m_buttons[i].m_strCaption);
#endif
                            break;
                        }
                    }
                }
                delete m_alertDialog;
                m_alertDialog = 0;
            }
        } else {
            LOG(WARNING) + "Trying to show alert dialog while it exists.";
        }
    }

    if (params)
        delete params;
}

void QtMainWindow::alertHidePopup()
{
    if (m_alertDialog) {
        m_alertDialog->done(QMessageBox::Accepted);
        delete m_alertDialog;
        m_alertDialog = 0;
    }
}

void QtMainWindow::dateTimePicker(CDateTimeMessage* msg)
{
    if (msg) {
        int retCode    = -1;

        DateTimeDialog timeDialog(msg, this);
        retCode = timeDialog.exec();

        rho_rhodesapp_callDateTimeCallback( msg->m_callback,
            retCode == QDialog::Accepted ? (long) timeDialog.getUnixTime() : 0,
            msg->m_data,
            retCode == QDialog::Accepted ? 0 : 1);

        delete msg;
    }
}

void QtMainWindow::executeCommand(RhoNativeViewRunnable* runnable)
{
    if (runnable) {
        runnable->run();
        delete runnable;
    }
}

void QtMainWindow::executeRunnable(rho::common::IRhoRunnable* pTask)
{
    if (pTask) {
        pTask->runObject();
        delete pTask;
    }
}

void QtMainWindow::takeSignature(void*) //TODO: Signature::Params*
{
    //TODO: takeSignature
}

void QtMainWindow::fullscreenCommand(int enable)
{
    //TODO: fullscreenCommand
    LOG(INFO) + (enable ? "Switched to Fullscreen mode" : "Switched to Normal mode" );
}

void QtMainWindow::setCookie(const char* url, const char* cookie)
{
    if (url && cookie) {
        QNetworkCookieJar* cj = ui->webView->page()->networkAccessManager()->cookieJar();
        cj->setCookiesFromUrl(QNetworkCookie::parseCookies(QByteArray(cookie)), QUrl(QString::fromUtf8(url)));
    }
}

void QtMainWindow::bringToFront()
{
    this->show();
    this->raise();
    this->activateWindow();
}
