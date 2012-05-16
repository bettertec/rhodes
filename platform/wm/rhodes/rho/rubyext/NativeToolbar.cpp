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

#include "NativeToolbar.h"
#include "common/rhoparams.h"
#include "MainWindow.h"
#include "common/RhoFilePath.h"
#include "rubyext/WebView.h"
#include "rubyext/NativeToolbarExt.h"

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

extern CMainWindow& getAppWindow();

IMPLEMENT_LOGCLASS(CNativeToolbar,"NativeToolbar");
extern "C" int rho_wmsys_has_touchscreen();

CNativeToolbar::CNativeToolbar(void)
{
}

CNativeToolbar::~CNativeToolbar(void)
{
}

void CNativeToolbar::OnFinalMessage(HWND /*hWnd*/)
{
    removeAllButtons();
}

/*static*/ CNativeToolbar& CNativeToolbar::getInstance()
{
    return getAppWindow().getToolbar();
}

void CNativeToolbar::removeAllButtons()
{
    if ( m_hWnd )
    {
        int nCount = GetButtonCount();
        for( int i = 0; i < nCount; i++)
            DeleteButton(0);

        SetImageList(NULL);
    }

    m_listImages.Destroy();
    m_arButtons.removeAllElements();
}

static int getColorFromString(const char* szColor)
{
    if ( !szColor || !*szColor )
        return RGB(0, 0, 0);

	int c = atoi(szColor);

	int cR = (c & 0xFF0000) >> 16;
	int cG = (c & 0xFF00) >> 8;
	int cB = (c & 0xFF);

    return RGB(cR, cG, cB);
}

void CNativeToolbar::createToolbar(rho_param *p)
{
//#if defined( OS_PLATFORM_MOTCE )
//    return;
//#endif

    if (!rho_rhodesapp_check_mode() || !rho_wmsys_has_touchscreen() )
        return;

    int bar_type = TOOLBAR_TYPE;
    m_rgbBackColor = RGB(220,220,220);
    m_rgbMaskColor = RGB(255,255,255);
    m_nHeight = MIN_TOOLBAR_HEIGHT;

	rho_param *params = NULL;
    switch (p->type) 
    {
        case RHO_PARAM_ARRAY:
            params = p;
            break;
        case RHO_PARAM_HASH: 
            {
                for (int i = 0, lim = p->v.hash->size; i < lim; ++i) 
                {
                    const char *name = p->v.hash->name[i];
                    rho_param *value = p->v.hash->value[i];
                    
                    if (strcasecmp(name, "background_color") == 0) 
					    m_rgbBackColor = getColorFromString(value->v.string);
                    else if (strcasecmp(name, "mask_color") == 0) 
					    m_rgbMaskColor = getColorFromString(value->v.string);
                    else if (strcasecmp(name, "view_height") == 0) 
					    m_nHeight = atoi(value->v.string);
                    else if (strcasecmp(name, "buttons") == 0 || strcasecmp(name, "tabs") == 0) 
                        params = value;
                }
            }
            break;
        default: {
            LOG(ERROR) + "Unexpected parameter type for create_nativebar, should be Array or Hash";
            return;
        }
    }
    
    if (!params) {
        LOG(ERROR) + "Wrong parameters for create_nativebar";
        return;
    }

    int size = params->v.array->size;
    if ( size == 0 )
    {
        removeToolbar();
        return;
    }

    if ( m_hWnd )
    {
        removeAllButtons();
    }else
    {
        RECT rcToolbar;
        rcToolbar.left = 0;
        rcToolbar.right = 0;
        rcToolbar.top = 0;
        rcToolbar.bottom = m_nHeight;
        Create(getAppWindow().m_hWnd, rcToolbar, NULL, WS_CHILD|CCS_NOPARENTALIGN|CCS_NORESIZE|CCS_NOMOVEY|CCS_BOTTOM|CCS_NODIVIDER |
            TBSTYLE_FLAT |TBSTYLE_LIST|TBSTYLE_TRANSPARENT ); //TBSTYLE_AUTOSIZE

        SetButtonStructSize();
    }

    for (int i = 0; i < size; ++i) 
    {
        rho_param *hash = params->v.array->value[i];
        if (hash->type != RHO_PARAM_HASH) {
            LOG(ERROR) + "Unexpected type of array item for create_nativebar, should be Hash";
            return;
        }
        
        const char *label = NULL;
        const char *action = NULL;
        const char *icon = NULL;
        const char *colored_icon = NULL;
		int  nItemWidth = 0;

        for (int j = 0, lim = hash->v.hash->size; j < lim; ++j) 
        {
            const char *name = hash->v.hash->name[j];
            rho_param *value = hash->v.hash->value[j];
            if (value->type != RHO_PARAM_STRING) {
                LOG(ERROR) + "Unexpected '" + name + "' type, should be String";
                return;
            }
            
            if (strcasecmp(name, "label") == 0)
                label = value->v.string;
            else if (strcasecmp(name, "action") == 0)
                action = value->v.string;
            else if (strcasecmp(name, "icon") == 0)
                icon = value->v.string;
            else if (strcasecmp(name, "colored_icon") == 0)
                colored_icon = value->v.string;
            else if (strcasecmp(name, "width") == 0)
                nItemWidth = atoi(value->v.string);
        }
        
        if (label == NULL && bar_type == TOOLBAR_TYPE)
            label = "";
        
        if ( label == NULL || action == NULL) {
            LOG(ERROR) + "Illegal argument for create_nativebar";
            return;
        }
        if ( strcasecmp(action, "forward") == 0 && rho_conf_getBool("jqtouch_mode") )
            continue;

        m_arButtons.addElement( new CToolbarBtn(label, action, icon, nItemWidth) );
	}

    CSize sizeMax = getMaxImageSize();
    m_nHeight = max(m_nHeight, sizeMax.cy+MIN_TOOLBAR_IDENT);
    int nBtnSize = m_nHeight-MIN_TOOLBAR_IDENT;
    SetButtonSize(max(nBtnSize,sizeMax.cx), max(nBtnSize,sizeMax.cy));
    SetBitmapSize(sizeMax.cx, sizeMax.cy);
    m_listImages.Create(sizeMax.cx, sizeMax.cy, ILC_MASK|ILC_COLOR32, m_arButtons.size(), 0);
    SetImageList(m_listImages);

    for ( int i = 0; i < (int)m_arButtons.size(); i++ )
        addToolbarButton( *m_arButtons.elementAt(i), i );

    AutoSize();

    alignSeparatorWidth();

    ShowWindow(SW_SHOW);

#if defined (OS_WINDOWS_DESKTOP)
    RECT rcWnd;
    getAppWindow().GetWindowRect(&rcWnd);
    getAppWindow().SetWindowPos( 0, 0,0,rcWnd.right-rcWnd.left-1,rcWnd.bottom-rcWnd.top, SWP_NOMOVE|SWP_NOZORDER|SWP_FRAMECHANGED);
    getAppWindow().SetWindowPos( 0, 0,0,rcWnd.right-rcWnd.left,rcWnd.bottom-rcWnd.top, SWP_NOMOVE|SWP_NOZORDER|SWP_FRAMECHANGED);
#else
    getAppWindow().SetWindowPos( 0, 0,0,0,0, SWP_NOMOVE|SWP_NOZORDER|SWP_NOSIZE|SWP_FRAMECHANGED);
#endif
}

void CNativeToolbar::alignSeparatorWidth()
{
    int nSepPos = -1;
    for ( int i = 0; i < (int)m_arButtons.size()-1; i++ )
    {
        if ( m_arButtons.elementAt(i)->isSeparator() )
        {
            if ( nSepPos == -1 )
                nSepPos = i;
            else
                return; //if more than one separator, do anything
        }
    }

    if ( nSepPos == -1 )
        return;

    //right align all buttons after single separator
    CRect rcFirstBtn, rcLastBtn, rcToolbar, rcSep;
    GetItemRect(0,&rcFirstBtn);
    GetItemRect(m_arButtons.size()-1,&rcLastBtn);
    GetItemRect(nSepPos,&rcSep);
    getAppWindow().GetClientRect(&rcToolbar);
    int nAdd = rcToolbar.Width() - 2*rcFirstBtn.left - rcLastBtn.right;
    int nSepWidth = rcSep.Width();
    nSepWidth += nAdd;
    if ( nSepWidth < 0 )
        nSepWidth = 0;
    
    m_arButtons.elementAt(nSepPos)->m_nItemWidth = nSepWidth;

    DeleteButton(nSepPos);
    TBBUTTON btn = {0};
/*
    if ( nSepWidth <= 114 ) //maximum size for empty button
    {
        btn.fsStyle = TBSTYLE_BUTTON;
        btn.iBitmap = I_IMAGENONE;
            
        InsertButton(nSepPos, &btn);

        TBBUTTONINFO oBtnInfo = {0};
        oBtnInfo.cbSize = sizeof(TBBUTTONINFO);
        oBtnInfo.dwMask = TBIF_BYINDEX|TBIF_SIZE;

        oBtnInfo.cx = nSepWidth;

        SetButtonInfo(nSepPos, &oBtnInfo);

    }else   */
    {
        btn.fsStyle = TBSTYLE_SEP;
        btn.iBitmap = nSepWidth;

        InsertButton(nSepPos, &btn);
    }

    AutoSize();
}

void CNativeToolbar::addToolbarButton(CToolbarBtn& oButton, int nPos)
{
    LOG(INFO) + "addToolbarButton: " + oButton.toString();

    TBBUTTON btn = {0};
    btn.iBitmap = I_IMAGENONE;

    if ( oButton.isSeparator() )
    {
        btn.fsStyle = TBSTYLE_SEP;
        if ( oButton.m_nItemWidth )
            btn.iBitmap = oButton.m_nItemWidth;
        else
        {
            CSize size;
            m_listImages.GetIconSize(size);
            btn.iBitmap = size.cx;
            oButton.m_nItemWidth = btn.iBitmap;
        }
    }else
    {
        btn.iString = (INT_PTR)oButton.m_strLabelW.c_str();

        btn.fsStyle = TBSTYLE_BUTTON|TBSTYLE_AUTOSIZE;
        btn.fsState = TBSTATE_ENABLED;
        btn.idCommand = ID_CUSTOM_TOOLBAR_ITEM_FIRST + nPos;

        if ( oButton.m_hImage )
        {
            btn.iBitmap = m_listImages.Add(oButton.m_hImage, m_rgbMaskColor);
            //btn.iBitmap = AddBitmap(1,oButton.m_hImage);
        }
    }

    BOOL bRes = AddButton(&btn);

    if ( !bRes )
        LOG(ERROR) + "Error";
}

LRESULT CNativeToolbar::OnEraseBkgnd(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
    CRect rect;
    GetClientRect(rect);

    CDCHandle hdc = (HDC)wParam;
    hdc.FillSolidRect(rect, m_rgbBackColor );

    bHandled = TRUE;
    return 1;
}

CSize CNativeToolbar::getMaxImageSize()
{
    CSize sizeMax(m_nHeight-MIN_TOOLBAR_IDENT, m_nHeight-MIN_TOOLBAR_IDENT);

    for ( int i = 0; i < (int)m_arButtons.size(); i++ )
    {
        //sizeMax.cx = m_arButtons.elementAt(i)->m_sizeImage.cx > sizeMax.cx ? m_arButtons.elementAt(i)->m_sizeImage.cx : sizeMax.cx;
        //sizeMax.cy = m_arButtons.elementAt(i)->m_sizeImage.cy > sizeMax.cy ? m_arButtons.elementAt(i)->m_sizeImage.cy : sizeMax.cy;
        if ( m_arButtons.elementAt(i)->m_hImage )
        {
            sizeMax = m_arButtons.elementAt(i)->m_sizeImage;
            break;
        }
    }

    return sizeMax;
}

bool CNativeToolbar::CToolbarBtn::isSeparator()
{
    return strcasecmp(m_strAction.c_str(), "separator")==0;
}

String CNativeToolbar::CToolbarBtn::getDefaultImagePath(const String& strAction)
{
    String strImagePath;
    if ( strcasecmp(strAction.c_str(), "options")==0 )
        strImagePath = "lib/res/options_btn.png";
    else if ( strcasecmp(strAction.c_str(), "home")==0 )
        strImagePath = "lib/res/home_btn.png";
    else if ( strcasecmp(strAction.c_str(), "refresh")==0 )
        strImagePath = "lib/res/refresh_btn.png";
    else if ( strcasecmp(strAction.c_str(), "back")==0 )
        strImagePath = "lib/res/back_btn.png";
    else if ( strcasecmp(strAction.c_str(), "forward")==0 )
        strImagePath = "lib/res/forward_btn.png";

    return strImagePath.length() > 0 ? CFilePath::join( RHODESAPP().getRhoRuntimePath(), strImagePath) : String();
}

CNativeToolbar::CToolbarBtn::CToolbarBtn( const char *label, const char *action, const char *icon, int nItemWidth  )
{
    m_hImage = 0;
    m_sizeImage = CSize(0,0);
    m_nItemWidth = nItemWidth;

    if ( label )
        convertToStringW( label, m_strLabelW );

    m_strAction = action ? action : "";
    String strImagePath;
    if ( icon && *icon )
        strImagePath = rho::common::CFilePath::join( RHODESAPP().getAppRootPath(), icon );
    else
        strImagePath = getDefaultImagePath(m_strAction);

    if ( strImagePath.length() > 0  )
    {
    	m_hImage = SHLoadImageFile( convertToStringW(strImagePath).c_str() );
        if ( m_hImage )
        {
	        BITMAP bmp;
            ::GetObject( m_hImage, sizeof(bmp), &bmp);
            m_sizeImage.cx = bmp.bmWidth;
            m_sizeImage.cy = bmp.bmHeight;
        }
    }
}

CNativeToolbar::CToolbarBtn::~CToolbarBtn()
{
    if ( m_hImage )
        DeleteObject(m_hImage);
}

String CNativeToolbar::CToolbarBtn::toString()
{
    return String("Label: '") + convertToStringA(m_strLabelW) + "';Action: '" + m_strAction + "'" ;
}

void CNativeToolbar::processCommand(int nItemPos)
{
    if ( nItemPos >= 0 && nItemPos < (int)m_arButtons.size() )
    {
        String strAction = m_arButtons.elementAt(nItemPos)->m_strAction;
        if ( strcasecmp(strAction.c_str(), "forward") == 0 )
                rho_webview_navigate_forward();
        else
            RHODESAPP().loadUrl(strAction);
    }
}

void CNativeToolbar::removeToolbar()
{
    if ( m_hWnd )
    {
        ShowWindow(SW_HIDE);
        m_nHeight = 0;

#if defined (OS_WINDOWS_DESKTOP)
        RECT rcWnd;
        getAppWindow().GetWindowRect(&rcWnd);
        getAppWindow().SetWindowPos( 0, 0,0,rcWnd.right-rcWnd.left-1,rcWnd.bottom-rcWnd.top, SWP_NOMOVE|SWP_NOZORDER|SWP_FRAMECHANGED);
        getAppWindow().SetWindowPos( 0, 0,0,rcWnd.right-rcWnd.left,rcWnd.bottom-rcWnd.top, SWP_NOMOVE|SWP_NOZORDER|SWP_FRAMECHANGED);
#else
        getAppWindow().SetWindowPos( 0, 0,0,0,0, SWP_NOMOVE|SWP_NOZORDER|SWP_NOSIZE|SWP_FRAMECHANGED);
#endif
    }
}

bool CNativeToolbar::isStarted()
{
    return m_hWnd && IsWindowVisible();
}
