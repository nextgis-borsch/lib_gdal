/******************************************************************************
 * $Id$
 *
 * Project:  WMS Client Driver
 * Purpose:  Implementation of Dataset and RasterBand classes for WMS
 *           and other similar services.
 * Author:   Adam Nowacki, nowak@xpam.de
 *
 ******************************************************************************
 * Copyright (c) 2007, Adam Nowacki
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ****************************************************************************/

#include "wmsdriver.h"

GDALWMSCache::GDALWMSCache() {
    m_cache_path = "./gdalwmscache";
    m_max_size = -1; // no limits
}

GDALWMSCache::~GDALWMSCache() {
}

CPLErr GDALWMSCache::Initialize(CPLXMLNode *config, CPLXMLNode *service_config) {
    const char *xmlcache_path = CPLGetXMLValue(config, "Path", NULL);
    const char *usercache_path = CPLGetConfigOption("GDAL_DEFAULT_WMS_CACHE_PATH", NULL);
    if(xmlcache_path)
    {
        m_cache_path = xmlcache_path;
    }
    else if(usercache_path)
    {
        m_cache_path = usercache_path;
    }

    for(CPLXMLNode* psChildNode = service_config->psChild; NULL != psChildNode;
        psChildNode = psChildNode->psNext)
    {
        if( psChildNode->eType == CXT_Element
            && EQUAL(psChildNode->pszValue,"ServerUrl")
            && psChildNode->psChild != NULL )
        {
            if(m_key_name.empty ())
                m_key_name = psChildNode->psChild->pszValue;
            else
            {
                m_key_name += "+";
                m_key_name += psChildNode->psChild->pszValue;
            }
        }
    }

    m_max_size = atoll (CPLGetXMLValue(config, "SizeLimit", "-1"));
    // life time in seconds. Default 7 days
    m_ttl = atof(CPLGetXMLValue(config, "LifeTime", "604800"));

    return CE_None;
}

