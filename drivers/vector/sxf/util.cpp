/******************************************************************************
 *
 * Project:  SXF Driver
 * Purpose:  Utility functions.
 * Author:   Dmitry Baryshnikov, polimax@mail.ru
 *
 ******************************************************************************
 * Copyright (c) 2020, NextGIS <info@nextgis.com>
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

#include "ogr_sxf.h"

namespace SXF {
    size_t WriteEncString(const char *pszSrcText, GByte *pDst,
        int nSize, const char *pszEncoding)
    {
        memset(pDst, 0, nSize);
        if (pszSrcText == nullptr)
        {
            return 0;
        }
        char *pszRecoded = CPLRecode(pszSrcText, CPL_ENC_UTF8, pszEncoding);
        size_t maxSize = CPLStrnlen(pszRecoded, nSize);
        if (pszRecoded != nullptr)
        {
            memcpy(pDst, pszRecoded, maxSize);
            pDst[maxSize + 1] = 0;
        }
        CPLFree(pszRecoded);

        return maxSize + 1;
    }

    void WriteEncString(const char *pszText, int nSize,
        const char *pszEncoding, VSILFILE *fpSXF)
    {
        GByte *pVal = new GByte[nSize];
        WriteEncString(pszText, pVal, nSize, pszEncoding);
        VSIFWriteL(pVal, nSize, 1, fpSXF);
        delete [] pVal;
    }

    std::string ReadEncString(const void *pBuffer, size_t nLen,
        const char *pszSrcEncoding)
    {
        if (nLen == 0)
        {
            return "";
        }
        char *pszRecoded;
        if (EQUAL(pszSrcEncoding, CPL_ENC_UTF16) || EQUAL(pszSrcEncoding, CPL_ENC_UCS2))
        {
            nLen = (nLen - 2) / 2;
            wchar_t *pwszSource = new wchar_t[nLen + 1];
            int j = 0;
            const char *pabySrc = static_cast<const char *>(pBuffer);
            for (int i = 0; i < nLen; i++, j++)
            {
                pwszSource[j] = (pabySrc[2 * i + 1] << 8) + pabySrc[2 * i];
#ifndef _WIN32
                /* Is there a surrogate pair ? See http://en.wikipedia.org/wiki/UTF-16
                 */
                /* On Windows, CPLRecodeFromWChar does this for us, because wchar_t is
                 * only */
                /* 2 bytes wide, whereas on Unix it is 32bits */
                if (pwszSource[j] >= 0xD800 && pwszSource[j] <= 0xDBFF && i + 1 < nLen)
                {
                    /* should be in the range 0xDC00... 0xDFFF */
                    wchar_t nTrailSurrogate =
                            (pabySrc[2 * (i + 1) + 1] << 8) + pabySrc[2 * (i + 1)];
                    if (nTrailSurrogate >= 0xDC00 && nTrailSurrogate <= 0xDFFF)
                    {
                        pwszSource[j] = ((pwszSource[j] - 0xD800) << 10) +
                                        (nTrailSurrogate - 0xDC00) + 0x10000;
                        i++;
                    }
                }
#endif
            }
            pwszSource[j] = 0;

            pszRecoded = CPLRecodeFromWChar(pwszSource, pszSrcEncoding, CPL_ENC_UTF8);
            CPLFree(pwszSource);
        }
        else
        {
            auto value = static_cast<char*>(CPLMalloc(nLen + 1));
            memset(value, 0, nLen + 1);
            memcpy(value, pBuffer, nLen);
            pszRecoded = CPLRecode(value, pszSrcEncoding, CPL_ENC_UTF8);
            CPLFree(value);
        }
        std::string out(pszRecoded);
        CPLFree(pszRecoded);
        return out;
    }
} // namespace SXF
