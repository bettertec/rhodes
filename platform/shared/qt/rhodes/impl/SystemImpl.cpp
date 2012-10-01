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

#include "common/RhoPort.h"
#include "ruby/ext/rho/rhoruby.h"
#include "common/RhoDefs.h"
#include "common/RhoFilePath.h"
#include "logging/RhoLog.h"
#undef null
#include <qglobal.h>
#include <QWebPage>
#include <QLocale>
#include <QDesktopServices>
#include <QMessageBox>
#include "MainWindowImpl.h"

using namespace rho;
using namespace rho::common;

extern "C" {

VALUE phone_number()
{
    return rho_ruby_get_NIL();
}

static int has_camera()
{
    return 1;
}

static double get_screen_ppi_x()
{
    return CMainWindow::getInstance()->getLogicalDpiX();
}

static double get_screen_ppi_y()
{
    return CMainWindow::getInstance()->getLogicalDpiY();
}

VALUE rho_sys_get_locale()
{
    return rho_ruby_create_string(QLocale::system().name().left(2).toStdString().c_str());
}

int rho_sys_get_screen_width()
{
    return CMainWindow::getScreenWidth();
}

int rho_sys_get_screen_height()
{
    return CMainWindow::getScreenHeight();
}

int rho_sysimpl_get_property(char* szPropName, VALUE* resValue)
{
    if (strcasecmp("webview_framework",szPropName) == 0) {
        *resValue = rho_ruby_create_string("WEBKIT/" QTWEBKIT_VERSION_STR);
        return 1;
    }

    if (strcasecmp("main_window_closed",szPropName) == 0) {
        *resValue = rho_ruby_create_boolean(CMainWindow::mainWindowClosed);
        return 1;
    }

    if (strcasecmp("has_camera",szPropName) == 0) {
        *resValue = rho_ruby_create_boolean(has_camera());
        return 1;
    }

    if (strcasecmp("phone_number",szPropName) == 0) {
        *resValue = phone_number();
        return 1;
    }

    if (strcasecmp("ppi_x",szPropName) == 0) {
        *resValue = rho_ruby_create_double(get_screen_ppi_x());
        return 1;
    }

    if (strcasecmp("ppi_y",szPropName) == 0) {
        *resValue = rho_ruby_create_double(get_screen_ppi_y());
        return 1;
    }

    if (strcasecmp("locale",szPropName) == 0) {
        *resValue = rho_sys_get_locale();
        return 1;
    }

    if (strcasecmp("country",szPropName) == 0) {
        *resValue = rho_ruby_create_string(QLocale::system().name().right(2).toStdString().c_str());
        return 1;
    }

    if (strcasecmp("device_name",szPropName) == 0) {
        *resValue = rho_ruby_create_string("Qt");
        return 1;
    }

#if defined(__SYMBIAN32__)
    if (strcasecmp("platform",szPropName) == 0) {
        *resValue = rho_ruby_create_string("Symbian");
        return 1;
    }

     if (strcasecmp("os_version",szPropName) == 0) {
        *resValue = rho_ruby_create_string("");
        return 1;
    }
#elif defined(RHO_SYMBIAN)
    if (strcasecmp("platform",szPropName) == 0) {
        *resValue = rho_ruby_create_string("Qt Emulator");
        return 1;
    }

     if (strcasecmp("os_version",szPropName) == 0) {
        *resValue = rho_ruby_create_string("");
        return 1;
    }
#endif

    if (strcasecmp("is_emulator",szPropName) == 0) {
        *resValue = rho_ruby_create_boolean(1);
        return 1;
    }

    if (strcasecmp("has_calendar",szPropName) == 0) {
        *resValue = rho_ruby_create_boolean(1);
        return 1;
    }

    if (strcasecmp("has_touchscreen",szPropName) == 0) {
        *resValue = rho_ruby_create_boolean(1);
        return 1;
    }

    if (strcasecmp("screen_orientation",szPropName) == 0) {
        if (rho_sys_get_screen_width() <= rho_sys_get_screen_height()) 
        {
            *resValue = rho_ruby_create_string("portrait");
        }
        else {
            *resValue = rho_ruby_create_string("landscape");
        }                                                          
        return 1;
    }

    return 0;
}

VALUE rho_sys_makephonecall(const char* callname, int nparams, char** param_names, char** param_values) 
{
    return rho_ruby_get_NIL();
}

static int g_rho_has_network = 1;

void rho_sysimpl_sethas_network(int nValue)
{
    g_rho_has_network = nValue;
}

VALUE rho_sys_has_network()
{
    return rho_ruby_create_boolean(g_rho_has_network!=0);
}

void rho_sys_app_exit()
{
    CMainWindow::getInstance()->exitCommand();
}

void rho_sys_open_url(const char* url)
{
    QString sUrl = QString::fromUtf8(url);
    if (sUrl.startsWith("/")) sUrl.prepend("file://");
    QDesktopServices::openUrl(QUrl(sUrl));
}

void rho_sys_run_app(const char *appname, VALUE params)
{
    //TODO: rho_sys_run_app
    RAWLOGC_INFO("SystemImpl", "rho_sys_run_app() has no implementation in RhoSimulator.");
}

void rho_sys_bring_to_front()
{
    // TODO: rho_sys_bring_to_front
    return CMainWindow::getInstance()->bringToFront();
}

void rho_sys_report_app_started()
{
    // TODO: rho_sys_report_app_started
    RAWLOGC_INFO("SystemImpl", "rho_sys_report_app_started() has no implementation in RhoSimulator.");
}

int rho_sys_is_app_installed(const char *appname)
{
    int nRet = 0;
    //TODO: rho_sys_is_app_installed
    RAWLOGC_INFO("SystemImpl", "rho_sys_is_app_installed() has no implementation in RhoSimulator.");
    return nRet;
}

void rho_sys_app_install(const char *url)
{
    rho_sys_open_url(url);
}

void rho_sys_app_uninstall(const char *appname)
{
    //TODO: rho_sys_app_uninstall
    RAWLOGC_INFO("SystemImpl", "rho_sys_app_uninstall() has no implementation in RhoSimulator.");
}

#if !defined(OS_WINDOWS_DESKTOP) && !defined(OS_SYMBIAN)
int rho_sys_set_sleeping(int sleeping)
{
    //TODO: rho_sys_set_sleeping
    return 1;
}
#endif

void rho_sys_set_application_icon_badge(int badge_number)
{
    //TODO: rho_sys_set_application_icon_badge
    RAWLOGC_INFO1("SystemImpl", "rho_sys_set_application_icon_badge(%d) was called", badge_number);
}

void rho_sys_impl_exit_with_errormessage(const char* szTitle, const char* szMsg)
{
    QMessageBox::critical(0, QString(szTitle), QString(szMsg));
}

RHO_GLOBAL void rho_platform_restart_application()
{
    //TODO: rho_platform_restart_application
    RAWLOGC_INFO("SystemImpl", "rho_platform_restart_application() has no implementation in RhoSimulator.");
}

const char* rho_native_reruntimepath()
{
    return rho_native_rhopath();
}

int MotorolaLicence_check(const char* company, const char* licence)
{
    return 1;
}

void rho_sys_set_window_frame(int x0, int y0, int width, int height)
{
    CMainWindow::getInstance()->setFrame(x0, y0, width, height);
}

void rho_sys_set_window_position(int x0, int y0)
{
    CMainWindow::getInstance()->setPosition(x0, y0);
}

void rho_sys_set_window_size(int width, int height)
{
    CMainWindow::getInstance()->setSize(width, height);
}

void rho_sys_lock_window_size(int locked)
{
    CMainWindow::getInstance()->lockSize(locked);
}

} //extern "C"
