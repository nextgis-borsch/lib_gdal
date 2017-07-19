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

#ifndef CPL_JSON_H_INCLUDED
#define CPL_JSON_H_INCLUDED

#include "cpl_string.h"

/**
 * \file cpl_json.h
 *
 * Interface for read and write JSON documents
 */
typedef void *JSONObjectH;

CPL_C_START

class CPLJSONArray;
/**
 * @brief The CPLJSONArray class holds JSON object from CPLJSONDocument
 */
class CPL_DLL CPLJSONObject
{
    friend class CPLJSONArray;
    friend class CPLJSONDocument;
public:
    enum Type {
        Null,
        Object,
        Array,
        Boolean,
        String,
        Integer,
        Long,
        Double
    };

public:
    CPLJSONObject();
    explicit CPLJSONObject(const char *pszName, const CPLJSONObject& oParent);

private:
    CPLJSONObject(const CPLString& soName, JSONObjectH poJsonObject);

public:
    // setters
    void Add(const char *pszName, const char *pszValue);
    void Add(const char *pszName, double dfValue);
    void Add(const char *pszName, int nValue);
    void Add(const char *pszName, long nValue);
    void Add(const char *pszName, const CPLJSONArray& oValue);
    void Add(const char *pszName, const CPLJSONObject& oValue);
    void Add(const char *pszName, bool bValue);

    void Set(const char *pszName, const char *pszValue);
    void Set(const char *pszName, double dfValue);
    void Set(const char *pszName, int nValue);
    void Set(const char *pszName, long nValue);
    void Set(const char *pszName, bool bValue);

    // getters
    const char* GetString(const char *pszDefault, const char *pszName) const;
    double GetDouble(const char *pszName, double dfDefault) const;
    int GetInteger(const char *pszName, int nDefault) const;
    long GetLong(const char *pszName, long nDdefault) const;
    bool GetBool(const char *pszName, bool bDefault) const;
    const char* GetString(const char *pszDefault) const;
    double GetDouble(double dfDefault) const;
    int GetInteger(int nDefault) const;
    long GetLong(long nDefault) const;
    bool GetBool(bool bDefault) const;

    //
    void Delete(const char* pszName);
    CPLJSONArray GetArray(const char *pszName) const;
    CPLJSONObject GetObject(const char *pszName) const;
    enum Type GetType() const;
    const char* GetName() const { return m_soKey; }
    CPLJSONObject** GetChildren() const;
    bool IsValid() const;

protected:
    CPLJSONObject GetObjectByPath(const char *pszPath, char *pszName) const;

private:
    JSONObjectH m_poJsonObject;
    CPLString m_soKey;
};

/**
 * @brief The JSONArray class JSON array from JSONDocument
 */
class CPL_DLL CPLJSONArray : public CPLJSONObject
{
    friend class CPLJSONObject;
public:
    CPLJSONArray(const CPLString& soName);
private:
    explicit CPLJSONArray(const CPLString& soName, JSONObjectH poJsonObject);
public:
    int Size() const;
    void Add(const CPLJSONObject& oValue);
    CPLJSONObject operator[](int nKey);
    const CPLJSONObject operator[](int nKey) const;
};

/**
 * @brief The CPLJSONDocument class Wrapper class around json-c library
 */
class CPL_DLL CPLJSONDocument
{
public:
    CPLJSONDocument();
    ~CPLJSONDocument();
    bool Save(const char *pszPath);
    CPLJSONObject GetRoot();
    bool Load(const char *pszPath);
    bool Load(GByte *pabyData, int nLength = -1);

    // TODO: add JSONObject reader(stream);
    // JsonReader reader = new JsonReader(new InputStreamReader(in, "UTF-8"));
    // reader.beginObject()
    // while (reader.hasNext())
    // String name = reader.nextName()
    // reader.beginArray();
private:
    JSONObjectH m_poRootJsonObject;
};

CPL_C_END

#endif // CPL_JSON_H_INCLUDED
