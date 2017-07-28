/******************************************************************************
 * Project:  Common Portability Library
 * Purpose:  Function wrapper for libjson-c access.
 * Author:   Dmitry Baryshnikov, dmitry.baryshnikov@nextgis.com
 *
 ******************************************************************************
 * Copyright (c) 2016-2017 NextGIS, <info@nextgis.com>
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

#include "cpl_json.h"


#include "cpl_error.h"
#include "cpl_vsi.h"

#include "json.h"

#ifdef HAVE_CURL
#include "cpl_http.h"
#include "cpl_multiproc.h"
#  include <curl/curl.h>
// CURLINFO_RESPONSE_CODE was known as CURLINFO_HTTP_CODE in libcurl 7.10.7 and
// earlier.
#if LIBCURL_VERSION_NUM < 0x070a07
#define CURLINFO_RESPONSE_CODE CURLINFO_HTTP_CODE
#endif

#endif

#define TO_JSONOBJ(x) static_cast<json_object*>(x)

static const char *JSON_PATH_DELIMITER = "/";
static const unsigned short JSON_NAME_MAX_SIZE = 255;

//------------------------------------------------------------------------------
// JSONDocument
//------------------------------------------------------------------------------

CPLJSONDocument::CPLJSONDocument() : m_poRootJsonObject(NULL)
{

}

CPLJSONDocument::~CPLJSONDocument()
{
    if(m_poRootJsonObject)
        json_object_put( TO_JSONOBJ(m_poRootJsonObject) );
}

bool CPLJSONDocument::Save(const char *pszPath)
{
    VSILFILE *fp = VSIFOpenL( pszPath, "wt" );
    if( NULL == fp )
    {
        CPLError( CE_Failure, CPLE_NoWriteAccess, "Open file %s to write failed",
                 pszPath );
        return false;
    }

    const char *pabyData = json_object_to_json_string_ext(
                TO_JSONOBJ(m_poRootJsonObject), JSON_C_TO_STRING_PRETTY );
    VSIFWriteL(pabyData, 1, strlen(pabyData), fp);

    VSIFCloseL(fp);

    return true;
}

CPLJSONObject CPLJSONDocument::GetRoot()
{
    if( NULL == m_poRootJsonObject )
        m_poRootJsonObject = json_object_new_object();
    return CPLJSONObject( "", m_poRootJsonObject );
}

bool CPLJSONDocument::Load(const char *pszPath)
{
    GByte *pabyOut = NULL;
    vsi_l_offset nSize = 0;
    if( !VSIIngestFile( NULL, pszPath, &pabyOut, &nSize, 4 * 1024 * 1024) ) // Maximum 4 Mb allowed
    {
        CPLError( CE_Failure, CPLE_FileIO, "Load json file %s failed", pszPath );
        return false;
    }

    bool bResult = Load(pabyOut, static_cast<int>(nSize));
    VSIFree(pabyOut);
    return bResult;
}

bool CPLJSONDocument::Load(GByte *pabyData, int nLength)
{
    if(NULL == pabyData)
    {
        return false;
    }
    json_tokener *jstok = json_tokener_new();
    m_poRootJsonObject = json_tokener_parse_ex( jstok,
                                                reinterpret_cast<const char*>(pabyData),
                                                nLength );
    bool bParsed = jstok->err == json_tokener_success;
    if(!bParsed)
    {
        CPLError( CE_Failure, CPLE_AppDefined, "JSON parsing error: %s (at offset %d)",
                 json_tokener_error_desc( jstok->err ), jstok->char_offset );

        return false;
    }
    json_tokener_free( jstok );
    return bParsed;
}

bool CPLJSONDocument::LoadChunks(const char *pszPath, size_t nChunkSize,
                                 GDALProgressFunc pfnProgress,
                                 void *pProgressArg)
{
    VSIStatBufL sStatBuf;
    if(VSIStatL(pszPath, &sStatBuf) != 0)
    {
        CPLError(CE_Failure, CPLE_FileIO, "Cannot open %s", pszPath);
        return false;
    }

    VSILFILE *fp = VSIFOpenL(pszPath, "rb");
    if( fp == NULL )
    {
        CPLError(CE_Failure, CPLE_FileIO, "Cannot open %s", pszPath);
        return false;
    }


    void *pBuffer = CPLMalloc( nChunkSize );
    json_tokener *tok = json_tokener_new();
    bool bSuccess = true;
    GUInt32 nFileSize = static_cast<GUInt32>(sStatBuf.st_size);
    double dfTotalRead = 0.0;

    while( true )
    {
        size_t nRead = VSIFReadL( pBuffer, 1, nChunkSize, fp );
        dfTotalRead += nRead;

        m_poRootJsonObject = json_tokener_parse_ex(tok,
                                                   static_cast<const char*>(pBuffer),
                                                   static_cast<int>(nRead));

        enum json_tokener_error jerr = json_tokener_get_error(tok);
        if(jerr != json_tokener_continue && jerr != json_tokener_success)
        {
            CPLError( CE_Failure, CPLE_AppDefined, "JSON error: %s",
                     json_tokener_error_desc(jerr) );
            bSuccess = false;
            break;
        }

        if( nRead < nChunkSize )
        {
            break;
        }

        if( NULL != pfnProgress )
        {
            pfnProgress(dfTotalRead / nFileSize, "Loading ...", pProgressArg);
        }
    }

    json_tokener_free(tok);
    CPLFree(pBuffer);
    VSIFCloseL(fp);

    if( NULL != pfnProgress )
    {
        pfnProgress(1.0, "Loading ...", pProgressArg);
    }

    return bSuccess;
}

typedef struct {
    json_object *pObject;
    json_tokener *pTokener;
    int nDataLen;
} JsonContext, *JsonContextL;

static size_t WriteFunction(char *pBuffer, size_t nSize, size_t nMemb,
                                           void *pUserData)
{
    size_t nLength = nSize * nMemb;
    JsonContextL ctx = static_cast<JsonContextL>(pUserData);
    ctx->pObject = json_tokener_parse_ex(ctx->pTokener, pBuffer,
                                         static_cast<int>(nLength));
    ctx->nDataLen = static_cast<int>(nLength);
    switch (json_tokener_get_error(ctx->pTokener)) {
    case json_tokener_continue:
    case json_tokener_success:
        return nLength;
    default:
        return 0; /* error: interrupt the transfer */
    }
}

#ifdef HAVE_CURL

typedef struct {
    GDALProgressFunc pfnProgress;
    void *pProgressArg;
} CurlProcessData, *CurlProcessDataL;

static int NewProcessFunction(void *p,
                              curl_off_t dltotal, curl_off_t dlnow,
                              curl_off_t /*ultotal*/, curl_off_t /*ulnow*/)
{
    CurlProcessDataL pData = static_cast<CurlProcessDataL>(p);
    if(pData->pfnProgress) {
        double dfDone = double(dlnow) / dltotal;
        return pData->pfnProgress(dfDone, "Downloading ...",
                                  pData->pProgressArg) == TRUE ? 0 : 1;
    }

    return 0;
}

static int ProcessFunction(void *p, double dltotal, double dlnow,
                                     double ultotal, double ulnow)
{
    return NewProcessFunction(p, static_cast<curl_off_t>(dltotal),
                              static_cast<curl_off_t>(dlnow),
                              static_cast<curl_off_t>(ultotal),
                              static_cast<curl_off_t>(ulnow));
}

static size_t HeaderWriteFunction( void *buffer, size_t size, size_t nmemb,
                                   void *reqInfo )
{
    size_t nLength = size * nmemb;
    char **papszHeaders = static_cast<char**>(reqInfo);
    char* pszHdr = static_cast<char *>(CPLCalloc(nmemb + 1, size));
    CPLPrintString(pszHdr, static_cast<char *>(buffer),
                   static_cast<int>(nLength));
    char *pszKey = NULL;
    const char *pszValue = CPLParseNameValue(pszHdr, &pszKey );
    papszHeaders = CSLSetNameValue(papszHeaders, pszKey, pszValue);
    CPLFree(pszHdr);
    CPLFree(pszKey);
    return nLength;
}
#endif // HAVE_CURL

bool CPLJSONDocument::LoadUrl(const char* pszUrl, char **papszOptions,
                              GDALProgressFunc pfnProgress,
                              void *pProgressArg)
{
#ifdef HAVE_CURL
    const char* pszAuthHeader = CPLHTTPAuthStore::instance().GetAuthHeader(pszUrl);
    if( EQUAL( pszAuthHeader, "expired" ) )
    {
        return false;
    }

    CURL *http_handle = curl_easy_init();
    CURLcode res;
    char szCurlErrBuf[CURL_ERROR_SIZE+1] = {};

    const char *pszArobase = strchr(pszUrl, '@');
    const char *pszSlash = strchr(pszUrl, '/');
    const char *pszColon = (pszSlash) ? strchr(pszSlash, ':') : NULL;
    if( pszArobase != NULL && pszColon != NULL && pszArobase - pszColon > 0 )
    {
        /* http://user:password@www.example.com */
        char *pszSanitizedURL = CPLStrdup(pszUrl);
        pszSanitizedURL[pszColon-pszUrl] = 0;
        CPLDebug( "JSON HTTP", "Fetch(%s:#password#%s)", pszSanitizedURL, pszArobase );
        CPLFree(pszSanitizedURL);
    }
    else
    {
        CPLDebug( "JSON HTTP", "Fetch(%s)", pszUrl );
    }

    CurlProcessData stProcessData = { pfnProgress, pProgressArg };
    curl_easy_setopt(http_handle, CURLOPT_URL, pszUrl );
    curl_easy_setopt(http_handle, CURLOPT_PROGRESSFUNCTION, ProcessFunction);
    curl_easy_setopt(http_handle, CURLOPT_PROGRESSDATA, &stProcessData);

#if LIBCURL_VERSION_NUM >= 0x072000
    curl_easy_setopt(http_handle, CURLOPT_XFERINFOFUNCTION, NewProcessFunction);
    curl_easy_setopt(http_handle, CURLOPT_XFERINFODATA, &stProcessData);
#endif
    curl_easy_setopt(http_handle, CURLOPT_NOPROGRESS, 0L);

    struct curl_slist *headers= reinterpret_cast<struct curl_slist*>(
                            CPLHTTPSetOptions(http_handle, papszOptions));

    if( !EQUAL(pszAuthHeader, "") )
    {
        headers = curl_slist_append(headers, pszAuthHeader);
    }

    const char *pszHeaders = CSLFetchNameValue( papszOptions, "HEADERS" );
    if( pszHeaders != NULL )
    {
        CPLDebug ("JSON HTTP", "These HTTP headers were set: %s", pszHeaders);
        char** papszTokensHeaders = CSLTokenizeString2(pszHeaders, "\r\n", 0);
        for( int i=0; papszTokensHeaders[i] != NULL; ++i )
            headers = curl_slist_append(headers, papszTokensHeaders[i]);
        CSLDestroy(papszTokensHeaders);
    }

    if( headers != NULL )
    {
        curl_easy_setopt(http_handle, CURLOPT_HTTPHEADER, headers);
    }

    int nDepth = atoi( CSLFetchNameValueDef( papszOptions, "JSON_DEPTH", "10") );
    JsonContext ctx = { NULL, json_tokener_new_ex(nDepth), 0 };
    curl_easy_setopt(http_handle, CURLOPT_WRITEDATA, &ctx );
    curl_easy_setopt(http_handle, CURLOPT_WRITEFUNCTION, WriteFunction );

    // Capture response headers.
    char **papszHeaders = NULL;
    curl_easy_setopt(http_handle, CURLOPT_HEADERDATA, papszHeaders);
    curl_easy_setopt(http_handle, CURLOPT_HEADERFUNCTION, HeaderWriteFunction);

    szCurlErrBuf[0] = '\0';
    curl_easy_setopt(http_handle, CURLOPT_ERRORBUFFER, szCurlErrBuf );

    static bool bHasCheckVersion = false;
    static bool bSupportGZip = false;
    if( !bHasCheckVersion )
    {
        bSupportGZip = strstr(curl_version(), "zlib/") != NULL;
        bHasCheckVersion = true;
    }
    bool bGZipRequested = false;
    if( bSupportGZip &&
        CPLTestBool(CPLGetConfigOption("CPL_CURL_GZIP", "YES")) )
    {
        bGZipRequested = true;
        curl_easy_setopt(http_handle, CURLOPT_ENCODING, "gzip");
    }

    const char *pszRetryDelay =
        CSLFetchNameValue( papszOptions, "RETRY_DELAY" );
    if( pszRetryDelay == NULL )
        pszRetryDelay = CPLGetConfigOption( "GDAL_HTTP_RETRY_DELAY", "30" );
    const char *pszMaxRetries = CSLFetchNameValue( papszOptions, "MAX_RETRY" );
    if( pszMaxRetries == NULL )
        pszMaxRetries = CPLGetConfigOption( "GDAL_HTTP_MAX_RETRY", "0" );
    int nRetryDelaySecs = atoi(pszRetryDelay);
    int nMaxRetries = atoi(pszMaxRetries);
    int nRetryCount = 0;
    bool bRequestRetry;

    do
    {
        bRequestRetry = false;
        res = curl_easy_perform(http_handle);

        if( strlen(szCurlErrBuf) > 0 )
        {
            bool bSkipError = false;
            if( bGZipRequested &&
                strstr(szCurlErrBuf, "transfer closed with") &&
                strstr(szCurlErrBuf, "bytes remaining to read") )
            {
                const char* pszContentLength =
                    CSLFetchNameValue(papszHeaders, "Content-Length");
                if( pszContentLength && ctx.nDataLen != 0 &&
                    atoi(pszContentLength) == ctx.nDataLen )
                {
                    const char* pszCurlGZIPOption =
                        CPLGetConfigOption("CPL_CURL_GZIP", NULL);
                    if( pszCurlGZIPOption == NULL )
                    {
                        CPLSetConfigOption("CPL_CURL_GZIP", "NO");
                        CPLDebug("JSON HTTP",
                                 "Disabling CPL_CURL_GZIP, "
                                 "because %s doesn't support it properly",
                                 pszUrl);
                    }
                    bSkipError = true;
                }
            }
            if( !bSkipError )
            {
                CPLError( CE_Failure, CPLE_AppDefined, "%s", szCurlErrBuf );
            }
        }
        else
        {
            long nHTTPResponseCode = 0;
            curl_easy_getinfo(http_handle, CURLINFO_RESPONSE_CODE,
                              &nHTTPResponseCode);
            if( nHTTPResponseCode >= 400 && nHTTPResponseCode < 600 )
            {
                if( (nHTTPResponseCode >= 502 && nHTTPResponseCode <= 504) &&
                    nRetryCount < nMaxRetries )
                {
                    CPLError(CE_Warning, CPLE_AppDefined, "JSON HTTP error code: %d - %s. "
                             "Retrying again in %d secs",
                             static_cast<int>(nHTTPResponseCode), pszUrl,
                             nRetryDelaySecs);
                    CPLSleep(nRetryDelaySecs);
                    nRetryCount++;

                    bRequestRetry = true;
                }
                else
                {
                    CPLError(CE_Failure, CPLE_AppDefined, "%s", szCurlErrBuf);
                }
            }
        }
    }
    while( bRequestRetry );

    bool bResult = true;
    if (res != CURLE_OK) {
        CPLError(CE_Warning, CPLE_AppDefined, "JSON HTTP error: %s",
               curl_easy_strerror(res));
        bResult = false;
    }
    curl_easy_cleanup( http_handle );
    curl_slist_free_all(headers);

    enum json_tokener_error jerr;
    if ((jerr = json_tokener_get_error(ctx.pTokener)) != json_tokener_success) {
        CPLError(CE_Warning, CPLE_AppDefined, "JSON error: %s\n",
               json_tokener_error_desc(jerr));
        bResult = false;
    }
    else {
        m_poRootJsonObject = ctx.pObject;
    }
    json_tokener_free(ctx.pTokener);

    return bResult;
#else
    return false;
#endif
}

//------------------------------------------------------------------------------
// JSONObject
//------------------------------------------------------------------------------

CPLJSONObject::CPLJSONObject()
{
    m_poJsonObject = json_object_new_object();
}

CPLJSONObject::CPLJSONObject(const char *pszName, const CPLJSONObject &oParent) :
    m_soKey(pszName)
{
    m_poJsonObject = json_object_new_object();
    json_object_object_add( TO_JSONOBJ(oParent.m_poJsonObject), pszName,
                            TO_JSONOBJ(m_poJsonObject) );
}

CPLJSONObject::CPLJSONObject(const CPLString &soName, JSONObjectH poJsonObject) :
    m_poJsonObject(poJsonObject),
    m_soKey(soName)
{

}

void CPLJSONObject::Add(const char *pszName, const CPLString soValue)
{
    char objectName[JSON_NAME_MAX_SIZE];
    CPLJSONObject object = GetObjectByPath( pszName, &objectName[0] );
    if( object.IsValid() )
    {
        json_object *poVal = json_object_new_string( soValue.c_str() );
        json_object_object_add( TO_JSONOBJ(object.m_poJsonObject), objectName,
                                poVal );
    }
}

void CPLJSONObject::Add(const char *pszName, const char *pszValue)
{
    if( NULL == pszName )
        return;
    char objectName[JSON_NAME_MAX_SIZE];
    CPLJSONObject object = GetObjectByPath( pszName, &objectName[0] );
    if( object.IsValid() )
    {
        json_object *poVal = json_object_new_string( pszValue );
        json_object_object_add( TO_JSONOBJ(object.m_poJsonObject), objectName,
                                poVal );
    }
}

void CPLJSONObject::Add(const char *pszName, double dfValue)
{
    if( NULL == pszName )
        return;
    char objectName[JSON_NAME_MAX_SIZE];
    CPLJSONObject object = GetObjectByPath( pszName, &objectName[0] );
    if(object.IsValid())
    {
        json_object *poVal = json_object_new_double( dfValue );
        json_object_object_add( TO_JSONOBJ(object.m_poJsonObject), objectName,
                                poVal );
    }
}

void CPLJSONObject::Add(const char *pszName, int nValue)
{
    if( NULL == pszName )
        return;
    char objectName[JSON_NAME_MAX_SIZE];
    CPLJSONObject object = GetObjectByPath( pszName, &objectName[0] );
    if( object.IsValid() )
    {
        json_object *poVal = json_object_new_int( nValue );
        json_object_object_add( TO_JSONOBJ(object.m_poJsonObject), objectName,
                                poVal );
    }
}

void CPLJSONObject::Add(const char *pszName, long nValue)
{
    if( NULL == pszName )
        return;
    char objectName[JSON_NAME_MAX_SIZE];
    CPLJSONObject object = GetObjectByPath( pszName, &objectName[0] );
    if( object.IsValid() )
    {
        json_object *poVal = json_object_new_int64( nValue );
        json_object_object_add( TO_JSONOBJ(object.m_poJsonObject), objectName,
                                poVal );
    }
}

void CPLJSONObject::Add(const char *pszName, const CPLJSONArray &oValue)
{
    if( NULL == pszName )
        return;
    char objectName[JSON_NAME_MAX_SIZE];
    CPLJSONObject object = GetObjectByPath(pszName, &objectName[0]);
    if( object.IsValid() )
    {
        json_object_object_add( TO_JSONOBJ(object.m_poJsonObject), objectName,
                                TO_JSONOBJ(oValue.m_poJsonObject) );
    }
}

void CPLJSONObject::Add(const char *pszName, const CPLJSONObject &oValue)
{
    if( NULL == pszName )
        return;
    char objectName[JSON_NAME_MAX_SIZE];
    CPLJSONObject object = GetObjectByPath( pszName, &objectName[0] );
    if( object.IsValid() )
    {
        json_object_object_add( TO_JSONOBJ(object.m_poJsonObject), objectName,
                                TO_JSONOBJ(oValue.m_poJsonObject) );
    }
}

void CPLJSONObject::Add(const char *pszName, bool bValue)
{
    if( NULL == pszName )
        return;
    char objectName[JSON_NAME_MAX_SIZE];
    CPLJSONObject object = GetObjectByPath( pszName, &objectName[0] );
    if(object.IsValid())
    {
        json_object *poVal = json_object_new_boolean( bValue );
        json_object_object_add( TO_JSONOBJ(object.m_poJsonObject), objectName,
                                poVal );
    }
}

void CPLJSONObject::Set(const char *pszName, const char *pszValue)
{
    Delete( pszName );
    Add( pszName, pszValue );
}

void CPLJSONObject::Set(const char *pszName, double dfValue)
{
    Delete( pszName );
    Add( pszName, dfValue );
}

void CPLJSONObject::Set(const char *pszName, int nValue)
{
    Delete( pszName );
    Add( pszName, nValue );
}

void CPLJSONObject::Set(const char *pszName, long nValue)
{
    Delete( pszName );
    Add( pszName, nValue );
}

void CPLJSONObject::Set(const char *pszName, bool bValue)
{
    Delete( pszName );
    Add( pszName, bValue );
}

CPLJSONArray CPLJSONObject::GetArray(const char *pszName) const
{
    if( NULL == pszName )
        return CPLJSONArray( NULL );
    char objectName[JSON_NAME_MAX_SIZE];
    CPLJSONObject object = GetObjectByPath( pszName, &objectName[0] );
    if( object.IsValid() )
    {
        json_object *poVal = NULL;
        if( json_object_object_get_ex( TO_JSONOBJ(object.m_poJsonObject),
                                       objectName, &poVal ) )
        {
            if( poVal && json_object_get_type( poVal ) == json_type_array )
            {
                return CPLJSONArray( objectName, poVal );
            }
        }
    }
    return CPLJSONArray( NULL );
}

CPLJSONObject CPLJSONObject::GetObject(const char *pszName) const
{
    if( NULL == pszName )
        return CPLJSONArray( NULL );
    char objectName[JSON_NAME_MAX_SIZE];
    CPLJSONObject object = GetObjectByPath( pszName, &objectName[0] );
    if(object.IsValid())
    {
        json_object* poVal = NULL;
        if(json_object_object_get_ex( TO_JSONOBJ(object.m_poJsonObject),
                                      objectName, &poVal ) )
        {
            return CPLJSONObject( objectName, poVal );
        }
    }
    return CPLJSONObject( "", NULL );
}

void CPLJSONObject::Delete(const char *pszName)
{
    if(NULL == pszName)
        return;
    char objectName[JSON_NAME_MAX_SIZE];
    CPLJSONObject object = GetObjectByPath( pszName, &objectName[0] );
    if(object.IsValid())
    {
        json_object_object_del( TO_JSONOBJ(object.m_poJsonObject), objectName );
    }
}

const char *CPLJSONObject::GetString(const char *pszName, const char* pszDefault) const
{
    if( NULL == pszName )
        return pszDefault;
    CPLJSONObject object = GetObject( pszName );
    return object.GetString( pszDefault );
}

const char* CPLJSONObject::GetString(const char* pszDefault) const
{
    if( m_poJsonObject && json_object_get_type( TO_JSONOBJ(m_poJsonObject) ) ==
            json_type_string )
        return json_object_get_string( TO_JSONOBJ(m_poJsonObject) );
    return pszDefault;
}

double CPLJSONObject::GetDouble(const char *pszName, double dfDefault) const
{
    if( NULL == pszName )
        return dfDefault;
    CPLJSONObject object = GetObject( pszName );
    return object.GetDouble( dfDefault );
}

double CPLJSONObject::GetDouble(double dfDefault) const
{
    if( m_poJsonObject && json_object_get_type( TO_JSONOBJ(m_poJsonObject) ) ==
            json_type_double )
        return json_object_get_double( TO_JSONOBJ(m_poJsonObject) );
    return dfDefault;
}

int CPLJSONObject::GetInteger(const char *pszName, int nDefault) const
{
    if( NULL == pszName )
        return nDefault;
    CPLJSONObject object = GetObject( pszName );
    return object.GetInteger( nDefault );
}

int CPLJSONObject::GetInteger(int nDefault) const
{
    if( m_poJsonObject && json_object_get_type( TO_JSONOBJ(m_poJsonObject) ) ==
            json_type_int )
        return json_object_get_int( TO_JSONOBJ(m_poJsonObject) );
    return nDefault;
}

long CPLJSONObject::GetLong(const char *pszName, long nDdefault) const
{
    if( NULL == pszName )
        return nDdefault;
    CPLJSONObject object = GetObject( pszName );
    return object.GetLong( nDdefault );
}

long CPLJSONObject::GetLong(long nDefault) const
{
    if( m_poJsonObject && json_object_get_type( TO_JSONOBJ(m_poJsonObject) ) ==
            json_type_int ) // FIXME: How to differ long and int if json_type_int64 or json_type_long not exist?
        return json_object_get_int64( TO_JSONOBJ(m_poJsonObject) );
    return nDefault;
}

bool CPLJSONObject::GetBool(const char *pszName, bool bDefault) const
{
    if( NULL == pszName )
        return bDefault;
    CPLJSONObject object = GetObject( pszName );
    return object.GetBool( bDefault );
}

CPLJSONObject **CPLJSONObject::GetChildren() const
{
    CPLJSONObject **papoChildren = NULL;
    size_t nChildrenCount = 0;
    json_object_object_foreach( TO_JSONOBJ(m_poJsonObject), key, val ) {
        CPLJSONObject *child = new CPLJSONObject(key, val);
        papoChildren = reinterpret_cast<CPLJSONObject **>(
            CPLRealloc( papoChildren,  sizeof(CPLJSONObject *) *
                        (nChildrenCount + 1) ) );
        papoChildren[nChildrenCount++] = child;
    }

    papoChildren = reinterpret_cast<CPLJSONObject **>(
        CPLRealloc( papoChildren,  sizeof(CPLJSONObject *) *
                    (nChildrenCount + 1) ) );
    papoChildren[nChildrenCount] = NULL;

    return papoChildren;
}

bool CPLJSONObject::GetBool(bool bDefault) const
{
    if( m_poJsonObject && json_object_get_type( TO_JSONOBJ(m_poJsonObject) ) ==
            json_type_boolean )
        return json_object_get_boolean( TO_JSONOBJ(m_poJsonObject) );
    return bDefault;
}

CPLJSONObject CPLJSONObject::GetObjectByPath(const char *pszPath, char *pszName) const
{
    json_object *poVal = NULL;
    CPLStringList pathPortions( CSLTokenizeString2( pszPath, JSON_PATH_DELIMITER,
                                                    0 ) );
    int portionsCount = pathPortions.size();
    if( 0 == portionsCount )
        return CPLJSONObject( CPLString(""), NULL );
    CPLJSONObject object = *this;
    for( int i = 0; i < portionsCount - 1; ++i ) {
        // TODO: check array index in path - i.e. settings/catalog/root/id:1/name
        // if EQUALN(pathPortions[i+1], "id:", 3) -> getArray
        if( json_object_object_get_ex( TO_JSONOBJ(object.m_poJsonObject),
                                       pathPortions[i], &poVal ) )
        {
            object = CPLJSONObject( pathPortions[i], poVal );
        }
        else
        {
            object = CPLJSONObject( pathPortions[i], object );
        }
    }

//    // Check if such object already  exists
//    if(json_object_object_get_ex(object.m_jsonObject,
//                                 pathPortions[portionsCount - 1], &poVal))
//        return JSONObject(NULL);

    CPLStrlcpy( pszName, pathPortions[portionsCount - 1], JSON_NAME_MAX_SIZE );
    return object;
}

CPLJSONObject::Type CPLJSONObject::GetType() const
{
    if(NULL == m_poJsonObject)
        return CPLJSONObject::Null;
    switch ( json_object_get_type( TO_JSONOBJ(m_poJsonObject) ) ) {
    case  json_type_null:
        return CPLJSONObject::Null;
    case json_type_boolean:
        return CPLJSONObject::Boolean;
    case json_type_double:
        return CPLJSONObject::Double;
    case json_type_int:
        return CPLJSONObject::Integer;
    case json_type_object:
        return CPLJSONObject::Object;
    case json_type_array:
        return CPLJSONObject::Array;
    case json_type_string:
        return CPLJSONObject::String;
    }
    return CPLJSONObject::Null;
}

bool CPLJSONObject::IsValid() const
{
    return NULL != m_poJsonObject;
}

void CPLJSONObject::DestroyJSONObjectList(CPLJSONObject **papsoList)
{
    if( !papsoList )
        return;

    for( CPLJSONObject **papsoPtr = papsoList; *papsoPtr != NULL; ++papsoPtr )
    {
        CPLFree(*papsoPtr);
    }

    CPLFree(papsoList);
}

//------------------------------------------------------------------------------
// JSONArray
//------------------------------------------------------------------------------

CPLJSONArray::CPLJSONArray(const CPLString& soName) :
    CPLJSONObject( soName, json_object_new_array() )
{

}

CPLJSONArray::CPLJSONArray(const CPLString &soName, JSONObjectH poJsonObject) :
    CPLJSONObject(soName, poJsonObject)
{

}

int CPLJSONArray::Size() const
{
    if( NULL == m_poJsonObject )
        return 0;
    return json_object_array_length( TO_JSONOBJ(m_poJsonObject) );
}

void CPLJSONArray::Add(const CPLJSONObject &oValue)
{
    if( oValue.m_poJsonObject )
        json_object_array_add( TO_JSONOBJ(m_poJsonObject),
                               TO_JSONOBJ(oValue.m_poJsonObject) );
}

CPLJSONObject CPLJSONArray::operator[](int nKey)
{
    return CPLJSONObject( CPLSPrintf("id:%d", nKey),
                          json_object_array_get_idx( TO_JSONOBJ(m_poJsonObject),
                                                     nKey ) );
}

const CPLJSONObject CPLJSONArray::operator[](int nKey) const
{
    return CPLJSONObject( CPLSPrintf("id:%d", nKey),
                          json_object_array_get_idx( TO_JSONOBJ(m_poJsonObject),
                                                     nKey ) );
}
