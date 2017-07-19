/******************************************************************************
 * $Id$
 *
 * Project:  Common Portability Library
 * Purpose:  Function wrapper for libcurl HTTP access.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 * Author:   Dmitry Baryshnikov, dmitry.baryshnikov@nextgis.com
 *
 ******************************************************************************
 * Copyright (c) 2006, Frank Warmerdam
 * Copyright (c) 2009, Even Rouault <even dot rouault at mines-paris dot org>
 * Copyright (c) 2017 NextGIS, <info@nextgis.com>
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

#ifndef CPL_HTTP_H_INCLUDED
#define CPL_HTTP_H_INCLUDED

#include "cpl_conv.h"
#include "cpl_string.h"
#include "cpl_vsi.h"

#include <vector>

/**
 * \file cpl_http.h
 *
 * Interface for downloading HTTP, FTP documents
 */

CPL_C_START

/*! Describe a part of a multipart message */
typedef struct {
    /*! NULL terminated array of headers */ char **papszHeaders;

    /*! Buffer with data of the part     */ GByte *pabyData;
    /*! Buffer length                    */ int    nDataLen;
} CPLMimePart;

/*! Describe the result of a CPLHTTPFetch() call */
typedef struct {
    /*! cURL error code : 0=success, non-zero if request failed */
    int     nStatus;

    /*! Content-Type of the response */
    char    *pszContentType;

    /*! Error message from curl, or NULL */
    char    *pszErrBuf;

    /*! Length of the pabyData buffer */
    int     nDataLen;
    /*! Allocated size of the pabyData buffer */
    int     nDataAlloc;

    /*! Buffer with downloaded data */
    GByte   *pabyData;

    /*! Headers returned */
    char    **papszHeaders;

    /*! Number of parts in a multipart message */
    int     nMimePartCount;

    /*! Array of parts (resolved by CPLHTTPParseMultipartMime()) */
    CPLMimePart *pasMimePart;

    /*! HTTP response code */
    long nHTTPResponseCode;

} CPLHTTPResult;

int CPL_DLL   CPLHTTPEnabled( void );
CPLHTTPResult CPL_DLL *CPLHTTPFetch( const char *pszURL, char **papszOptions);
void CPL_DLL  CPLHTTPCleanup( void );
void CPL_DLL  CPLHTTPDestroyResult( CPLHTTPResult *psResult );
int  CPL_DLL  CPLHTTPParseMultipartMime( CPLHTTPResult *psResult );

/* -------------------------------------------------------------------- */
/*      The following is related to OAuth2 authorization around         */
/*      google services like fusion tables, and potentially others      */
/*      in the future.  Code in cpl_google_oauth2.cpp.                  */
/*                                                                      */
/*      These services are built on CPL HTTP services.                  */
/* -------------------------------------------------------------------- */

char CPL_DLL *GOA2GetAuthorizationURL( const char *pszScope );
char CPL_DLL *GOA2GetRefreshToken( const char *pszAuthToken,
                                   const char *pszScope );
char CPL_DLL *GOA2GetAccessToken( const char *pszRefreshToken,
                                  const char *pszScope );
/* HTTP Auth event triggers */
typedef enum  {
    HTTPAUTH_UPDATE = 1, /*< Auth properties updated */
    HTTPAUTH_EXPIRED     /*< Auth properties is expired and cannot be used any more. Need user interaction */
} CPLHTTPAuthChangeCode;

/**
 * @brief Prototype of function, which executed when changes accured.
 * @param pszUrl The URL of server which token changed
 * @param eOperation Operation which trigger notification.
 */
typedef void (*HTTPAuthNotifyFunc)(const char* pszUrl, CPLHTTPAuthChangeCode eOperation);

int CPL_DLL CPLHTTPAuthAdd(const char* pszUrl, char** papszOptions,
                            HTTPAuthNotifyFunc func);
void CPL_DLL CPLHTTPAuthDelete(const char* pszUrl);
char** CPL_DLL CPLHTTPAuthProperties(const char* pszUrl);

CPL_C_END

#ifdef __cplusplus
/*! @cond Doxygen_Suppress */
// Not sure if this belong here, used in cpl_http.cpp, cpl_vsil_curl.cpp and frmts/wms/gdalhttp.cpp
void* CPLHTTPSetOptions(void *pcurl, const char * const* papszOptions);
char** CPLHTTPGetOptionsFromEnv();

/**
 * @brief The IHTTPAuth class is base class for HTTP Authorization headers
 */
class IHTTPAuth {
public:
    virtual ~IHTTPAuth() {}
    virtual const char* GetUrl() const = 0;
    virtual const char* GetHeader() = 0;
    virtual char** GetProperties() const = 0;
};

/**
 * @brief The CPLHTTPAuthBearer class The oAuth2 bearer update token class
 */
class CPLHTTPAuthBearer : public IHTTPAuth {

public:
    explicit CPLHTTPAuthBearer(const CPLString& soUrl, const CPLString& soClientId,
                               const CPLString& soTokenServer,
                               const CPLString& soAccessToken,
                               const CPLString& soUpdateToken, int nExpiresIn,
                               HTTPAuthNotifyFunc function,
                               const CPLString& soConnTimeout,
                               const CPLString& soTimeout,
                               const CPLString& soMaxRetry,
                               const CPLString& soRetryDelay);
    virtual ~CPLHTTPAuthBearer() {}
    virtual const char* GetUrl() const CPL_OVERRIDE { return m_soUrl; }
    virtual const char* GetHeader() CPL_OVERRIDE;
    virtual char** GetProperties() const CPL_OVERRIDE;

private:
    CPLString m_soUrl;
    CPLString m_soClientId;
    CPLString m_soAccessToken;
    CPLString m_soUpdateToken;
    CPLString m_soTokenServer;
    int m_nExpiresIn;
    HTTPAuthNotifyFunc m_NotifyFunction;
    CPLString m_soConnTimeout;
    CPLString m_soTimeout;
    CPLString m_soMaxRetry;
    CPLString m_soRetryDelay;
    time_t m_nLastCheck;
};

/**
 * @brief The CPLHTTPAuthStore class Storage for authorisation options
 */
class CPLHTTPAuthStore
{
public:
    static bool Add(const char *pszUrl, char **papszOptions,
                    HTTPAuthNotifyFunc func = NULL);
    static CPLHTTPAuthStore& instance();

public:
    void Add(IHTTPAuth* poAuth);
    void Delete(const char* pszUrl);
    const char* GetAuthHeader(const char* pszUrl);
    char** GetProperties(const char* pszUrl) const;

private:
    CPLHTTPAuthStore() {}
    ~CPLHTTPAuthStore() {}
    CPLHTTPAuthStore(CPLHTTPAuthStore const&) {}
    CPLHTTPAuthStore& operator= (CPLHTTPAuthStore const&) { return *this; }

private:
    std::vector<IHTTPAuth*> m_poAuths;
};



/*! @endcond */
#endif // __cplusplus

#endif /* ndef CPL_HTTP_H_INCLUDED */
