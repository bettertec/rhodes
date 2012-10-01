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

#include "stdafx.h"

#include "MainWindow.h"
#include "rubyext/NativeToolbarExt.h"

extern CMainWindow& getAppWindow();

extern "C"
{
int rho_wmsys_has_touchscreen();
void remove_native_toolbar();
void create_native_toolbar(int bar_type, rho_param *p) 
{
    if ( bar_type == NOBAR_TYPE )
        remove_native_toolbar();
    else if ( bar_type == TOOLBAR_TYPE )
    {
        getAppWindow().performOnUiThread(new CNativeToolbar::CCreateTask(p) );
    }else
    {
    	RAWLOGC_ERROR("NativeBar", "Only Toolbar control is supported.");
    }
}

void create_nativebar(int bar_type, rho_param *p) 
{
	RAWLOGC_INFO("NativeBar", "NativeBar.create() is DEPRECATED. Use Rho::NativeToolbar.create() or Rho::NativeTabbar.create().");
    create_native_toolbar(bar_type, p);
}

void remove_native_toolbar() 
{
    getAppWindow().performOnUiThread(new CNativeToolbar::CRemoveTask() );
}

void remove_nativebar() 
{
	RAWLOGC_INFO("NativeBar", "NativeBar.remove() is DEPRECATED API ! Please use Rho::NativeToolbar.remove() or Rho::NativeTabbar.remove().");
	remove_native_toolbar();
}

VALUE nativebar_started() 
{
    bool bStarted = CNativeToolbar::getInstance().isStarted();
    return rho_ruby_create_boolean(bStarted?1:0);
}

//Tabbar
void remove_native_tabbar()
{
#if defined(OS_WINDOWS_DESKTOP)
    getAppWindow().performOnUiThread(new CNativeTabbar::CRemoveTask() );
#endif
}

void create_native_tabbar(int bar_type, rho_param *p)
{
#if defined(OS_WINDOWS_DESKTOP)
    // check for iPad SplitTabBar type -> redirect to TabBar
    if (bar_type == VTABBAR_TYPE) {
        bar_type = TABBAR_TYPE;
    }

	if ( bar_type == NOBAR_TYPE )
        remove_native_tabbar();
    else if ( bar_type == TABBAR_TYPE )
    {
        getAppWindow().performOnUiThread(new CNativeTabbar::CCreateTask(bar_type, p) );
    } else
    {
    	RAWLOGC_ERROR("NativeTabbar", "Only Tabbar control is supported.");
    }
#endif
}

void native_tabbar_switch_tab(int index)
{
#if defined(OS_WINDOWS_DESKTOP)
    getAppWindow().performOnUiThread(new CNativeTabbar::CSwitchTask(index) );
#endif
}

void native_tabbar_set_tab_badge(int index,char *val)
{
#if defined(OS_WINDOWS_DESKTOP)
    getAppWindow().performOnUiThread(new CNativeTabbar::CBadgeTask(index, val) );
#endif
}

void nativebar_set_tab_badge(int index,char* val)
{
	RAWLOGC_INFO("NativeBar", "NativeBar.set_tab_badge() is DEPRECATED. Use Rho::NativeTabbar.set_tab_badge().");
    native_tabbar_set_tab_badge(index, val);
}

int native_tabbar_get_current_tab() 
{
#if defined(OS_WINDOWS_DESKTOP)
	return getAppWindow().tabbarGetCurrent();
#else
	return 0;
#endif
}

void nativebar_switch_tab(int index)
{
	RAWLOGC_INFO("NativeBar", "NativeBar.switch_tab() is DEPRECATED. Use Rho::NativeTabbar.switch_tab().");
	native_tabbar_switch_tab(index);
}

//NavBar - iphone only
void create_navbar(rho_param *p)
{
}

void remove_navbar()
{
}

VALUE navbar_started()
{
    return rho_ruby_create_boolean(0);
}

}