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

#pragma once

#include <string>
#include "logging/RhoLog.h"
#include "common/RhoDefs.h"
#include "common/RhoConf.h"
#include "common/RhodesApp.h"
#include "common/rhoparams.h"
#include "common/RhoNativeViewManager.h"
#include "../MainWindowCallback.h"
#include "NativeToolbarImpl.h"
#include "NativeTabbarImpl.h"
#include "AlertImpl.h"
#include "DateTimePickerImpl.h"
#undef null
#include <QObject>

#ifdef OS_MACOSX
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif

class CMainWindow : public QObject, IMainWindowCallback
{
    Q_OBJECT
    DEFINE_LOGCLASS
private:
    explicit CMainWindow();
public:
    ~CMainWindow();
    static CMainWindow* getInstance(void);
    // IMainWindowCallback
    virtual void updateSizeProperties(int width, int height);
    virtual void onActivate(int active);
    virtual void logEvent(const ::std::string& message);
    virtual void createCustomMenu(void);
    virtual void onCustomMenuItemCommand(int nItemPos);
    virtual void onWindowClose(void);
    virtual void onWebViewUrlChanged(const ::std::string& url);
    // public methods:
    bool Initialize(const wchar_t* title);
    void DestroyUi(void);
    CNativeToolbar& getToolbar(){ return m_toolbar; }
    CNativeTabbar& getTabbar(){ return m_tabbar; }
    // for 'main_window_closed' System property
    static bool mainWindowClosed;

    // proxy methods:
    bool init(IMainWindowCallback* callback, const wchar_t* title);
    void setCallback(IMainWindowCallback* callback);
    void messageLoop(void);
    void navigate(const wchar_t* url, int index);
    void GoBack(void);
    void GoForward(void);
    void Refresh(int index);
    int getLogicalDpiX();
    int getLogicalDpiY();
    // toolbar/tabbar
    bool isStarted();
    // toolbar proxy
    int getToolbarHeight();
    void createToolbar(rho_param *p);
    void removeToolbar();
    void removeAllButtons();
    // menu proxy
    void menuClear();
    void menuAddSeparator();
    void menuAddAction(const char* label, int item);
    // tabbar
    int getTabbarHeight();
    void removeAllTabs(bool restore);
    void createTabbar(int bar_type, rho_param *p);
    void removeTabbar();
    void tabbarSwitch(int index);
    void tabbarBadge(int index, char* badge);
    int tabbarGetCurrent();

private:
    //TODO: CLogView m_logView;
    CNativeToolbar m_toolbar;
    CNativeTabbar m_tabbar;
    bool m_started;
    void* qtMainWindow;
    void* qtApplication;

private:
    static int m_screenWidth;
    static int m_screenHeight;
    
public:
    static int getScreenWidth() {return m_screenWidth;}
    static int getScreenHeight() {return m_screenHeight;}

private:
    rho::Vector<rho::common::CAppMenuItem> m_arAppMenuItems;

public:
    void exitCommand(void);
    void navigateBackCommand(void);
    void navigateForwardCommand(void);
    void logCommand(void);
    void refreshCommand(int);
    void navigateCommand(TNavigateData*);
    void takePicture(char*);
    void selectPicture(char*);
    void alertShowPopup(CAlertParams *);
    void alertHidePopup(void);
    void dateTimePicker(CDateTimeMessage *);
    void executeCommand(RhoNativeViewRunnable*);
    void executeRunnable(rho::common::IRhoRunnable*);
    void takeSignature(void*); //TODO: Signature::Params*
    void fullscreenCommand(int);
    void setCookie(const char*, const char*);
    void bringToFront();
    void setFrame(int x, int y, int width, int height);
    void setPosition(int x, int y);
    void setSize(int width, int height);
    void lockSize(int locked);

signals:
    void doExitCommand(void);
    void doNavigateBackCommand(void);
    void doNavigateForwardCommand(void);
    void doLogCommand(void);
    void doRefreshCommand(int);
    void doNavigateCommand(TNavigateData*);
    void doTakePicture(char*);
    void doSelectPicture(char*);
    void doAlertShowPopup(CAlertParams *);
    void doAlertHidePopup(void);
    void doDateTimePicker(CDateTimeMessage *);
    void doExecuteCommand(RhoNativeViewRunnable*);
    void doExecuteRunnable(rho::common::IRhoRunnable*);
    void doTakeSignature(void*); //TODO: Signature::Params*
    void doFullscreenCommand(int);
    void doSetCookie(const char*, const char*);
    void doBringToFront(void);
    void doSetFrame(int x, int y, int width, int height);
    void doSetPosition(int x, int y);
    void doSetSize(int width, int height);
    void doLockSize(int locked);
};
