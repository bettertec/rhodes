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

#include "common/rhoparams.h"
#include "common/RhoFilePath.h"
#include "rubyext/WebView.h"
#include "MainWindowImpl.h"
#include "rubyext/NativeToolbarExt.h"
#include "rubyext/WebView.h"
#include "NativeTabbarImpl.h"
#undef null
#include <QString>
#include <QApplication>
#include <QtGui/QAction>

IMPLEMENT_LOGCLASS(CNativeTabbar,"NativeTabbar");

using namespace rho;
using namespace rho::common;

CNativeTabbar& CNativeTabbar::getInstance()
{
    return CMainWindow::getInstance()->getTabbar();
}

void CNativeTabbar::createTabbar(int bar_type, rho_param *p)
{
    CMainWindow::getInstance()->createTabbar(bar_type, p);
}

void CNativeTabbar::removeTabbar()
{
    CMainWindow::getInstance()->removeTabbar();
}

int CNativeTabbar::getHeight()
{
    return CMainWindow::getInstance()->getTabbarHeight();
}

bool CNativeTabbar::isStarted()
{
    return CMainWindow::getInstance()->isStarted();
}

void CNativeTabbar::tabbarSwitch(int index)
{
    CMainWindow::getInstance()->tabbarSwitch(index);
}

void CNativeTabbar::tabbarBadge(int index, char* badge)
{
    CMainWindow::getInstance()->tabbarBadge(index, badge);
}

extern "C" {

void remove_native_tabbar()
{
    CMainWindow::getInstance()->executeRunnable(new CNativeTabbar::CRemoveTask() );
}

void create_native_tabbar(int bar_type, rho_param *p)
{
    // check for iPad SplitTabBar type -> redirect to TabBar
    if (bar_type == VTABBAR_TYPE) {
        bar_type = TABBAR_TYPE;
    }

    if ( bar_type == NOBAR_TYPE )
        remove_native_tabbar();
    else if ( bar_type == TABBAR_TYPE )
    {
        CMainWindow::getInstance()->executeRunnable(new CNativeTabbar::CCreateTask(bar_type, p) );
    } else
    {
        RAWLOGC_ERROR("NativeTabbar", "Only Tabbar control is supported.");
    }
}

void native_tabbar_switch_tab(int index)
{
    CMainWindow::getInstance()->executeRunnable(new CNativeTabbar::CSwitchTask(index) );
}

void native_tabbar_set_tab_badge(int index, char *val)
{
    CMainWindow::getInstance()->executeRunnable(new CNativeTabbar::CBadgeTask(index, val) );
}

void nativebar_set_tab_badge(int index, char* val)
{
    RAWLOGC_INFO("NativeBar", "NativeBar.set_tab_badge() is DEPRECATED. Use Rho::NativeTabbar.set_tab_badge().");
    native_tabbar_set_tab_badge(index, val);
}

int native_tabbar_get_current_tab()
{
    return CMainWindow::getInstance()->tabbarGetCurrent();
}

void nativebar_switch_tab(int index)
{
    RAWLOGC_INFO("NativeBar", "NativeBar.switch_tab() is DEPRECATED. Use Rho::NativeTabbar.switch_tab().");
    native_tabbar_switch_tab(index);
}

} //extern "C"
