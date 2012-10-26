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

namespace rho
{

CRhoParams::CRhoParams(rho_param *p): m_pParams(p){}

const rho_param * CRhoParams::findHashParam(const char* name) const
{
    if (m_pParams->type == RHO_PARAM_HASH)
    {
        for (int i = 0; i < m_pParams->v.hash->size; ++i)
        {
            if (strcasecmp(name, m_pParams->v.hash->name[i]) == 0)
                return m_pParams->v.hash->value[i];
        }
    }
    return null;
}

boolean CRhoParams::has(const char* name) const
{
    if (m_pParams->type == RHO_PARAM_HASH)
    {
        for (int i = 0; i < m_pParams->v.hash->size; ++i)
        {
            if (strcasecmp(name, m_pParams->v.hash->name[i]) == 0)
                return true;
        }
    }
    return false;
}

String CRhoParams::getString(const char* szName) const
{
    return getString(szName, "");
}

String CRhoParams::getString(const char* szName, const char* szDefValue) const
{
    const rho_param * value = findHashParam(szName);
    String strRes = value && value->v.string ? value->v.string : "";
    if (strRes.length() == 0 && szDefValue && *szDefValue)
        strRes = szDefValue;

    return strRes;
}

void CRhoParams::getHash(const char* name, rho::Hashtable<rho::String,rho::String>& mapHeaders) const
{
    const rho_param * hash = findHashParam(name);
    if (!hash || hash->type != RHO_PARAM_HASH)
        return;

    for (int i = 0; i < hash->v.hash->size; ++i)
    {
        rho_param * value = hash->v.hash->value[i];
        mapHeaders.put( hash->v.hash->name[i], value->v.string );
    }
}

boolean CRhoParams::getBool(const char* name) const
{
	String strValue = getString(name);
	if ( strValue.length() == 0 )
		return false;

    return strValue.compare("1") == 0 || strValue.compare("true") == 0;
}

void CRhoParams::free_params()
{
    if ( m_pParams != null )
        rho_param_free(m_pParams);
}

CRhoParamArray::CRhoParamArray(CRhoParams& oParams, const char* name) : CRhoParams(oParams)
{
    m_array = null;
    const rho_param * ar = findHashParam(name);
    if (ar != null && ar->type == RHO_PARAM_ARRAY)
        m_array = ar->v.array;
}

int CRhoParamArray::size() const
{
    if ( m_array == null )
        return 0;

    return m_array->size;
}

CRhoParams CRhoParamArray::getItem(int nIndex) const
{
    return m_array->value[nIndex];
}

}

#ifdef __cplusplus
extern "C" {
#endif

rho_param *rho_param_str(char *s)
{
    rho_param *ret = (rho_param *)malloc(sizeof(rho_param));
    ret->type = RHO_PARAM_STRING;
    ret->v.string = strdup(s);
    return ret;
}

rho_param *rho_param_array(int size)
{
    int i;
    rho_param *ret = (rho_param *)malloc(sizeof(rho_param));
    ret->type = RHO_PARAM_ARRAY;
    ret->v.array = (rho_array *)malloc(sizeof(rho_array));
    ret->v.array->size = size;
    ret->v.array->value = (rho_param **)malloc(size * sizeof(rho_param *));
    for (i = 0; i < size; ++i)
        ret->v.array->value[i] = NULL;
    return ret;
}

rho_param *rho_param_hash(int size)
{
    int i;
    rho_param *ret = (rho_param *)malloc(sizeof(rho_param));
    ret->type = RHO_PARAM_HASH;
    ret->v.hash = (rho_hash *)malloc(sizeof(rho_hash));
    ret->v.hash->size = size;
    ret->v.hash->name = (char **)malloc(size * sizeof(char *));
    ret->v.hash->value = (rho_param **)malloc(size * sizeof(rho_param *));
    for (i = 0; i < size; ++i) {
        ret->v.hash->name[i] = NULL;
        ret->v.hash->value[i] = NULL;
    }
    return ret;
}

rho_param *rho_param_hash_get(rho_param *p, const char* name)
{
    if ( p->type != RHO_PARAM_HASH )
        return 0;

    for (int i = 0; i < p->v.hash->size; ++i) 
    {
        if (strcasecmp(name, p->v.hash->name[i]) == 0)
            return p->v.hash->value[i];
    }

    return 0;
}

rho_param *rho_param_dup(rho_param *p)
{
    int i, lim;
    rho_param *ret = NULL;

    if (!p) return NULL;

    switch (p->type) {
    case RHO_PARAM_STRING:
        ret = rho_param_str(p->v.string);
        break;
    case RHO_PARAM_ARRAY:
        ret = rho_param_array(p->v.array->size);
        for (i = 0, lim = p->v.array->size; i < lim; ++i)
            ret->v.array->value[i] = rho_param_dup(p->v.array->value[i]);
        break;
    case RHO_PARAM_HASH:
        ret = rho_param_hash(p->v.hash->size);
        for (i = 0, lim = p->v.hash->size; i < lim; ++i) {
            ret->v.hash->name[i] = strdup(p->v.hash->name[i]);
            ret->v.hash->value[i] = rho_param_dup(p->v.hash->value[i]);
        }
        break;
    }
    return ret;
}

void rho_param_free(rho_param *p)
{
    int i, lim;

    if (!p)
        return;

    switch (p->type) {
    case RHO_PARAM_STRING:
        free(p->v.string);
        break;
    case RHO_PARAM_ARRAY:
        for (i = 0, lim = p->v.array->size; i < lim; ++i)
            rho_param_free(p->v.array->value[i]);
        free(p->v.array->value);
        free(p->v.array);
        break;
    case RHO_PARAM_HASH:
        for (i = 0, lim = p->v.hash->size; i < lim; ++i) {
            free(p->v.hash->name[i]);
            rho_param_free(p->v.hash->value[i]);
        }
        free(p->v.hash->name);
        free(p->v.hash->value);
        free(p->v.hash);
        break;
    }
    free(p);
}

#ifdef __cplusplus
}
#endif

