/******************************************************************************
 *
 * Project:  SXF Translator
 * Purpose:  Definition of classes for OGR SXF Layers.
 * Author:   Ben Ahmed Daho Ali, bidandou(at)yahoo(dot)fr
 *           Dmitry Baryshnikov, polimax@mail.ru
 *           Alexandr Lisovenko, alexander.lisovenko@gmail.com
 *
 ******************************************************************************
 * Copyright (c) 2011, Ben Ahmed Daho Ali
 * Copyright (c) 2013-2020, NextGIS <info@nextgis.com>
 * Copyright (c) 2014, Even Rouault <even dot rouault at spatialys.com>
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
#include "cpl_conv.h"
#include "cpl_string.h"
#include "ogr_p.h"

CPL_CVSID("$Id$")

constexpr const char *CLCODE = "CLCODE";
constexpr const char *CLNAME = "CLNAME";
constexpr const char *OT = "OT";
constexpr const char *EXT = "EXT";
constexpr const char *GROUP_NUMBER = "group_number";
constexpr const char *NUMBER_IN_GROUP = "number_in_group";
constexpr const char *OBJECTNUMB = "OBJECTNUMB";
constexpr const char *ANGLE = "ANGLE";
constexpr const char *TEXT = "TEXT";

static enum SXFGeometryType OGRTypeToSXFType(OGRwkbGeometryType eType)
{
    switch (wkbFlatten(eType))
    {
    case wkbPoint:
    case wkbMultiPoint:
        return SXF_GT_Point;
    case wkbMultiLineString:
    case wkbLineString:
        return SXF_GT_Line;
    case wkbPolygon:
    case wkbMultiPolygon:
    case wkbPolyhedralSurface:
    case wkbTIN:
        return SXF_GT_Polygon;

    default:
        return SXF_GT_Unknown;
    }
}

static bool IsTypesCompatible(OGRwkbGeometryType eOGRType, 
    enum SXFGeometryType eSXFType)
{
    switch (eSXFType)
    {
    case SXF_GT_Line:
        return wkbFlatten(eOGRType) == wkbLineString || 
            wkbFlatten(eOGRType) == wkbMultiLineString;
    case SXF_GT_Polygon:
        return wkbFlatten(eOGRType) == wkbPolygon || 
            wkbFlatten(eOGRType) == wkbMultiPolygon || 
            wkbFlatten(eOGRType) == wkbPolyhedralSurface || 
            wkbFlatten(eOGRType) == wkbTIN;
    case SXF_GT_Point:
        return wkbFlatten(eOGRType) == wkbPoint ||
            wkbFlatten(eOGRType) == wkbMultiPoint;
    case SXF_GT_Text:
        return wkbFlatten(eOGRType) == wkbLineString ||
            wkbFlatten(eOGRType) == wkbMultiLineString;
    case SXF_GT_Vector:
        return wkbFlatten(eOGRType) == wkbLineString ||
            wkbFlatten(eOGRType) == wkbMultiLineString;
    case SXF_GT_TextTemplate:
        return wkbFlatten(eOGRType) == wkbPoint ||
            wkbFlatten(eOGRType) == wkbMultiPoint;

    default:
        return false;
    }
}

static bool IsFieldList(OGRFeatureDefn *poDef, int nIndex)
{
    auto poFieldDefn = poDef->GetFieldDefn(nIndex);
    if (poFieldDefn)
    {
        return poFieldDefn->GetType() == OFTIntegerList ||
            poFieldDefn->GetType() == OFTRealList ||
            poFieldDefn->GetType() == OFTStringList;
    }
    return false;
}

static bool HasField(OGRFeatureDefn *poDef, const std::string &osFieldName)
{
    return poDef->GetFieldIndex(osFieldName.c_str()) >= 0;
}

static size_t WritePointCount(GUInt32 nPointCount, GByte *pBuff)
{
    if (nPointCount > 65535)
    {
        memcpy(pBuff, &nPointCount, sizeof(GUInt32));
        return sizeof(GUInt32);
    }
    else
    {
        memset(pBuff, 0, sizeof(GUInt16));
        GUInt16 nPointCountSmall = static_cast<GUInt16>(nPointCount);
        memcpy(pBuff + sizeof(GUInt16), &nPointCountSmall, sizeof(GUInt16));
        return sizeof(GUInt16) * 2;
    }
}

static size_t WritePoint(const OGRPoint *poPt, GByte *pBuff)
{
    size_t nOffset = 0;
    double dfX = poPt->getX();
    double dfY = poPt->getY();
    size_t nSize = sizeof(double);
    memcpy(pBuff + nOffset, &dfY, nSize);
    nOffset += nSize;
    memcpy(pBuff + nOffset, &dfX, nSize);
    nOffset += nSize;
    if (poPt->Is3D())
    {
        double dfZ = poPt->getZ();
        memcpy(pBuff + nOffset, &dfZ, nSize);
        nOffset += nSize;
    }
    return nOffset;
}

static size_t WriteLine(const OGRLineString *poLn, GByte *pBuff)
{
    size_t nOffset = 0;
    for (const auto &poPt : poLn)
    {
        nOffset += WritePoint(&poPt, pBuff + nOffset);
    }
    return nOffset;
}

static GByte* WriteRings(const std::vector<OGRLinearRing*> &apoRings, 
    size_t &nSize, size_t nPointSize)
{
    GUInt32 nPointCount = apoRings[0]->getNumPoints();
    nSize = nPointSize * nPointCount;
    auto pBuff = static_cast<GByte*>(CPLMalloc(nSize));
    size_t nOffset = WriteLine(apoRings[0], pBuff);
    
    for (size_t i = 1; i < apoRings.size(); i++)
    {
        nPointCount = apoRings[i]->getNumPoints();
        nSize += nPointSize * nPointCount + 4;
        pBuff = static_cast<GByte*>(CPLRealloc(pBuff, nSize));

        nOffset += WritePointCount(nPointCount, pBuff + nOffset);
        nOffset += WriteLine(apoRings[i], pBuff + nOffset);
    }
    return pBuff;
}

static GByte *WriteLineToBuffer(size_t nPointSize, GByte *pBuff, size_t &nOffset,
    OGRLineString *poLn, SXFGeometryType eGeomType,
    const char *pszText, size_t &nSize, const std::string &osEncoding)
{
    GByte *pLocalBuff = nullptr;
    if (eGeomType == SXF_GT_Line)
    {
        GUInt32 nPointCount = poLn->getNumPoints();
        nSize += nPointSize * nPointCount;
        if (pBuff == nullptr)
        {
            pLocalBuff = static_cast<GByte*>(CPLMalloc(nSize));
        }
        else
        {
            nSize += 4;
            pLocalBuff = static_cast<GByte*>(CPLRealloc(pBuff, nSize));
            nOffset += WritePointCount(nPointCount, pLocalBuff + nOffset);
        }
        nOffset += WriteLine(poLn, pLocalBuff + nOffset);
    }
    else if (eGeomType == SXF_GT_Vector)
    {
        nSize += nPointSize * 2;
        if (pBuff == nullptr)
        {
            pLocalBuff = static_cast<GByte*>(CPLMalloc(nSize));
        }
        else
        {
            nSize += 4;
            pLocalBuff = static_cast<GByte*>(CPLRealloc(pBuff, nSize));
            nOffset += WritePointCount(2, pLocalBuff + nOffset);
        }
        OGRPoint pt;
        poLn->StartPoint(&pt);
        nOffset += WritePoint(&pt, pLocalBuff + nOffset);
        poLn->EndPoint(&pt);
        nOffset += WritePoint(&pt, pLocalBuff + nOffset);
    }
    else if (eGeomType == SXF_GT_Text)
    {
        size_t nTextLenght = CPLStrnlen(pszText, 254);
        size_t nPointCount = poLn->getNumPoints();
        nSize += nPointSize * nPointCount + nTextLenght + 2;
        if (pBuff == nullptr)
        {
            pLocalBuff = static_cast<GByte*>(CPLMalloc(nSize));
        }
        else
        {
            nSize += 4;
            pLocalBuff = static_cast<GByte*>(CPLRealloc(pBuff, nSize));
            nOffset += WritePointCount(nPointCount, pLocalBuff + nOffset);
        }
        nOffset += WriteLine(poLn, pLocalBuff + nOffset);
        GByte nTextL = static_cast<GByte>(nTextLenght);
        memcpy(pLocalBuff + nOffset, &nTextL, 1);
        nOffset++;
        // FIXME: Possible broken last character if text length greater 254
        SXF::WriteEncString(pszText, pLocalBuff + nOffset, nTextLenght + 1,
            osEncoding.c_str());
        nOffset += nTextLenght + 1;
    }
    return pLocalBuff;
}

static GByte *WriteGeometryToBuffer(OGRGeometry *poGeom, SXFGeometryType eGeomType,
    const char *pszText, size_t &nSize, const std::string &osEncoding)
{
    if (poGeom->IsEmpty())
    {
        return nullptr;
    }

    auto nPointSize = sizeof(double) * (poGeom->Is3D() ? 3 : 2);

    if (wkbFlatten(poGeom->getGeometryType()) == wkbPoint)
    {
        nSize = nPointSize;
        GByte *pBuff = static_cast<GByte*>(CPLMalloc(nSize));
        WritePoint(static_cast<OGRPoint*>(poGeom), pBuff);
        return pBuff;
    }
    else if (wkbFlatten(poGeom->getGeometryType()) == wkbMultiPoint)
    {
        auto poMPt = static_cast<OGRMultiPoint*>(poGeom);
        nSize = nPointSize * poMPt->getNumGeometries() + 
            4 * (poMPt->getNumGeometries() - 1);
        GByte *pBuff = static_cast<GByte*>(CPLMalloc(nSize));
        size_t nOffset = 0;
        for (int i = 0; i < poMPt->getNumGeometries(); i++)
        {
            auto poPt = static_cast<OGRPoint*>(poMPt->getGeometryRef(i));
            nOffset += WritePoint(poPt, pBuff + nOffset);
            if (i < poMPt->getNumGeometries() - 1)
            {
                memset(pBuff + nOffset, 0, sizeof(GUInt16));
                nOffset += sizeof(GUInt16);
                memset(pBuff + nOffset, 1, sizeof(GUInt16));
                nOffset += sizeof(GUInt16);
            }
        }
        return pBuff;
    }
    else if (wkbFlatten(poGeom->getGeometryType()) == wkbLineString)
    {
        auto poLn = static_cast<OGRLineString*>(poGeom);
        size_t nOffset = 0;
        return WriteLineToBuffer(nPointSize, nullptr, nOffset, poLn,
            eGeomType, pszText, nSize, osEncoding);
    }
    else if (wkbFlatten(poGeom->getGeometryType()) == wkbMultiLineString)
    {
        auto poMLn = static_cast<OGRMultiLineString*>(poGeom);
        // Get first main line
        auto poLn = static_cast<OGRLineString*>(poMLn->getGeometryRef(0));
        size_t nOffset = 0;
        GByte *pBuff = WriteLineToBuffer(nPointSize, nullptr, nOffset, poLn,
            eGeomType, pszText, nSize, osEncoding);

        for (int i = 1; i < poMLn->getNumGeometries(); i++)
        {
            poLn = static_cast<OGRLineString*>(poMLn->getGeometryRef(i));
            pBuff = WriteLineToBuffer(nPointSize, pBuff, nOffset, poLn,
                eGeomType, pszText, nSize, osEncoding);
        }
        return pBuff;
    }
    else if (wkbFlatten(poGeom->getGeometryType()) == wkbPolygon)
    {
        auto poPlg = static_cast<OGRPolygon*>(poGeom);
        std::vector<OGRLinearRing*> apoRings;
        apoRings.push_back(poPlg->getExteriorRing());
        for (int i = 0; i < poPlg->getNumInteriorRings(); i++)
        {
            apoRings.push_back(poPlg->getInteriorRing(i));
        }
        return WriteRings(apoRings, nSize, nPointSize);
    }
    else if (wkbFlatten(poGeom->getGeometryType()) == wkbMultiPolygon)
    {
        auto poMPlg = static_cast<OGRMultiPolygon*>(poGeom);
        std::vector<OGRLinearRing*> apoRings;
        for (int i = 0; i < poMPlg->getNumGeometries(); i++)
        {
            auto poPlg = static_cast<OGRPolygon*>(poMPlg->getGeometryRef(i));
            apoRings.push_back(poPlg->getExteriorRing());
            for (int j = 0; j < poPlg->getNumInteriorRings(); j++)
            {
                apoRings.push_back(poPlg->getInteriorRing(j));
            }
        }
        return WriteRings(apoRings, nSize, nPointSize);
    }
    else if (wkbFlatten(poGeom->getGeometryType()) == wkbPolyhedralSurface ||
        wkbFlatten(poGeom->getGeometryType()) == wkbTIN)
    {
        auto poGeomToDelete = std::unique_ptr<OGRGeometry>(
            OGRGeometryFactory::forceTo(poGeom->clone(),
                wkbMultiPolygon, nullptr));
        auto poMPlg = static_cast<OGRMultiPolygon*>(poGeomToDelete.get());
        std::vector<OGRLinearRing*> apoRings;
        for (int i = 0; i < poMPlg->getNumGeometries(); i++)
        {
            auto poPlg = static_cast<OGRPolygon*>(poMPlg->getGeometryRef(i));
            apoRings.push_back(poPlg->getExteriorRing());
            for (int j = 0; j < poPlg->getNumInteriorRings(); j++)
            {
                apoRings.push_back(poPlg->getInteriorRing(j));
            }
        }
        return WriteRings(apoRings, nSize, nPointSize);
    }

    return nullptr;
}

static GByte *WriteAttributesToBuffer(OGRFeature *poFeature, size_t &nSize,
    const std::map<std::string, int> &mnClassCodes, const std::string &osEncoding)
{
    GByte *pBuff = nullptr;
    nSize = 0;
    auto poDefn = poFeature->GetDefnRef();
    for (int i = 0; i < poDefn->GetFieldCount(); i++)
    {
        auto poField = poDefn->GetFieldDefn(i);
        if (EQUAL(poField->GetNameRef(), CLCODE) ||
            EQUAL(poField->GetNameRef(), CLNAME) ||
            EQUAL(poField->GetNameRef(), OT) ||
            EQUAL(poField->GetNameRef(), EXT) ||
            EQUAL(poField->GetNameRef(), GROUP_NUMBER) ||
            EQUAL(poField->GetNameRef(), NUMBER_IN_GROUP) ||
            EQUAL(poField->GetNameRef(), OBJECTNUMB) ||
            EQUAL(poField->GetNameRef(), ANGLE) ||
            EQUAL(poField->GetNameRef(), TEXT))
        {
            // Don't write system fields into the attributes
            continue;
        }

        int nCode = OGRSXFLayer::GetFieldNameCode(poField->GetNameRef());
        if (nCode == -1)
        {
            auto key = OGRSXFLayer::CreateFieldKey(poField);
            auto it = mnClassCodes.find(key);
            if (it != mnClassCodes.end())
            {
                nCode = it->second;
            }
        }

        GByte iType;
        switch (poField->GetType())
        {
        case OFTInteger:
        case OFTIntegerList:
        case OFTInteger64:
        case OFTInteger64List:
            iType = 4;
            break;
        case OFTReal:
        case OFTRealList:
            iType = 8;
            break;
        case OFTString:
        case OFTStringList:
        case OFTDate:
        case OFTTime:
        case OFTDateTime:
            iType = 126; // Default ANSI
            break;
        default:
            return nullptr;
        }

        SXFRecordAttributeInfo stRecordHeader = {
            static_cast<GUInt16>(nCode),
            iType,
            0
        };

        switch (iType)
        {
        case 4: // Integer
        {
            if (poField->GetType() == OFTInteger ||
                poField->GetType() == OFTInteger64)
            { // Possible lose data for integer64 as SXF can only hold 4 or 8 bit. 
                size_t nCurrentSize = nSize;
                nSize += sizeof(SXFRecordAttributeInfo) + sizeof(GInt32);
                pBuff = static_cast<GByte*>(CPLRealloc(pBuff, nSize));
                memcpy(pBuff + nCurrentSize, &stRecordHeader, 
                    sizeof(SXFRecordAttributeInfo));
                nCurrentSize += sizeof(SXFRecordAttributeInfo);
                GInt32 val = poFeature->GetFieldAsInteger(i);
                memcpy(pBuff + nCurrentSize, &val, sizeof(GInt32));
            }
            else // List
            {
                int nCount = 0;
                auto anList = poFeature->GetFieldAsIntegerList(i, &nCount);
                for (int j = 0; j < nCount; j++)
                {
                    size_t nCurrentSize = nSize;
                    nSize += sizeof(SXFRecordAttributeInfo) + sizeof(GInt32);
                    pBuff = static_cast<GByte*>(CPLRealloc(pBuff, nSize));
                    memcpy(pBuff + nCurrentSize, &stRecordHeader, 
                        sizeof(SXFRecordAttributeInfo));
                    nCurrentSize += sizeof(SXFRecordAttributeInfo);
                    GInt32 val = anList[j];
                    memcpy(pBuff + nCurrentSize, &val, sizeof(GInt32));
                }
            }
            break;
        }
        case 8:
        {
            if (poField->GetType() == OFTReal)
            {
                size_t nCurrentSize = nSize;
                nSize += sizeof(SXFRecordAttributeInfo) + sizeof(double);
                pBuff = static_cast<GByte*>(CPLRealloc(pBuff, nSize));
                memcpy(pBuff + nCurrentSize, &stRecordHeader, 
                    sizeof(SXFRecordAttributeInfo));
                nCurrentSize += sizeof(SXFRecordAttributeInfo);
                double val = poFeature->GetFieldAsDouble(i);
                memcpy(pBuff + nCurrentSize, &val, sizeof(double));
            }
            else
            {
                int nCount = 0;
                auto anList = poFeature->GetFieldAsDoubleList(i, &nCount);
                for (int j = 0; j < nCount; j++)
                {
                    size_t nCurrentSize = nSize;
                    nSize += sizeof(SXFRecordAttributeInfo) + sizeof(double);
                    pBuff = static_cast<GByte*>(CPLRealloc(pBuff, nSize));
                    memcpy(pBuff + nCurrentSize, &stRecordHeader, 
                        sizeof(SXFRecordAttributeInfo));
                    nCurrentSize += sizeof(SXFRecordAttributeInfo);
                    double val = anList[j];
                    memcpy(pBuff + nCurrentSize, &val, sizeof(double));
                }
            }
            break;
        }
        case 126:
        {
            if (poField->GetType() == OFTStringList)
            {
                CPLStringList aosList(poFeature->GetFieldAsStringList(i));
                for (int j = 0; j < aosList.size(); j++)
                {
                    auto pszText = aosList[j];
                    auto pszRecodedText = CPLRecode(pszText, CPL_ENC_UTF8, osEncoding.c_str());
                    GByte nTextSize = static_cast<GByte>(CPLStrnlen(pszRecodedText, 254));
                    GUInt32 nNewTextSize = nTextSize + 1;
                    stRecordHeader.nScale = nTextSize;
                    size_t nCurrentSize = nSize;
                    nSize += sizeof(SXFRecordAttributeInfo) + nNewTextSize;
                    pBuff = static_cast<GByte*>(CPLRealloc(pBuff, nSize));
                    memcpy(pBuff + nCurrentSize, &stRecordHeader, 
                        sizeof(SXFRecordAttributeInfo));
                    nCurrentSize += sizeof(SXFRecordAttributeInfo);
                    memcpy(pBuff + nCurrentSize, pszRecodedText, nTextSize);
                    nCurrentSize += nTextSize;
                    memset(pBuff + nCurrentSize, 0, 1);
                    CPLFree(pszRecodedText);
                }
            }
            else
            {
                auto pszText = poFeature->GetFieldAsString(i);
                auto pszRecodedText = CPLRecode(pszText, CPL_ENC_UTF8, osEncoding.c_str());
                GByte nTextSize = static_cast<GByte>(CPLStrnlen(pszRecodedText, 254));
                GUInt32 nNewTextSize = nTextSize + 1;
                stRecordHeader.nScale = nTextSize;
                size_t nCurrentSize = nSize;
                nSize += sizeof(SXFRecordAttributeInfo) + nNewTextSize;
                pBuff = static_cast<GByte*>(CPLRealloc(pBuff, nSize));
                memcpy(pBuff + nCurrentSize, &stRecordHeader, 
                    sizeof(SXFRecordAttributeInfo));
                nCurrentSize += sizeof(SXFRecordAttributeInfo);
                memcpy(pBuff + nCurrentSize, pszRecodedText, nTextSize);
                nCurrentSize += nTextSize;
                memset(pBuff + nCurrentSize, 0, 1);
                CPLFree(pszRecodedText);
            }
            break;
        }
        default:
            break;
        }

    }
    return pBuff;
}

static int GetExtention(const std::string &strCode, OGRFeature &feature, const SXFLayerDefn &sxfDefn)
{
    SXFLimits oLimits = sxfDefn.GetLimits(strCode);
    auto oLimitCodes = oLimits.GetLimitCodes();
    if (oLimitCodes.first >= 0)
    {
        std::string oSCName = CPLSPrintf("SC_%d", oLimitCodes.first);
        double dfVal1 = feature.GetFieldAsDouble(oSCName.c_str());
        double dfVal2 = 0.0;
        oSCName = CPLSPrintf("SC_%d", oLimitCodes.second);
        if (feature.GetFieldIndex(oSCName.c_str()) >= 0)
        {
            dfVal2 = feature.GetFieldAsDouble(oSCName.c_str());
        }
        return oLimits.GetExtention(dfVal1, dfVal2);
    }
    return 0;
}

static void AddCode(OGRFeature *poFeature, SXFLayerDefn &oSXFDefn)
{
    auto nClcodeIndex = poFeature->GetFieldIndex(CLCODE);
    if (nClcodeIndex != -1 && poFeature->GetGeometryRef())
    {
        std::string osName;
        auto nClnameIndex = poFeature->GetFieldIndex(CLNAME);
        if (nClnameIndex != -1)
        {
            osName = poFeature->GetFieldAsString(nClnameIndex);
        }
        else
        {
            osName = poFeature->GetFieldAsString(nClcodeIndex);
        }

        std::string osCode, osDefaultClass;
        auto nOtIndex = poFeature->GetFieldIndex(OT);
        if (nOtIndex != -1)
        {
            osCode = poFeature->GetFieldAsString(nOtIndex);
        }
        else
        {
            auto eType = 
                OGRTypeToSXFType(poFeature->GetGeometryRef()->getGeometryType());
            osCode = SXFFile::SXFTypeToString(eType);

            osDefaultClass = std::to_string(oSXFDefn.GenerateCode(eType));
        }

        std::string osClass = poFeature->GetFieldAsString(nClcodeIndex);
        if (osClass.empty())
        {
            osClass = osDefaultClass;
            if (osName.empty())
            {
                osName = osCode + osClass;
            }
        }

        auto osStrCode = osCode + osClass;
        int nExt = GetExtention(osStrCode, *poFeature, oSXFDefn);

        oSXFDefn.AddCode({ osStrCode, osName, nExt });
    }
}

////////////////////////////////////////////////////////////////////////////
/// OGRSXFLayer
////////////////////////////////////////////////////////////////////////////

OGRSXFLayer::OGRSXFLayer(OGRSXFDataSource *poDSIn,
    const SXFLayerDefn &oSXFDefn, bool bIsNewBehavior) :
    OGRMemLayer(oSXFDefn.GetName().c_str(),
        const_cast<OGRSpatialReference*>(poDSIn->GetSpatialRef()), wkbUnknown),
    poDS(poDSIn),
    bIsNewBehavior(bIsNewBehavior),
    oSXFLayerDefn(oSXFDefn)
{

    auto poFeatureDefn = GetLayerDefn();
    OGRFieldDefn oClCodeField( CLCODE, OFTInteger );
    poFeatureDefn->AddFieldDefn( &oClCodeField );

    OGRFieldDefn oClNameField( CLNAME, OFTString );
    oClNameField.SetWidth(32);
    poFeatureDefn->AddFieldDefn( &oClNameField );


    if( bIsNewBehavior )
    {
        OGRFieldDefn ocOTField( OT, OFTString );
        ocOTField.SetWidth(1);
        poFeatureDefn->AddFieldDefn( &ocOTField );

        OGRFieldDefn ocEXTField(EXT, OFTInteger);
        poFeatureDefn->AddFieldDefn(&ocEXTField);

        OGRFieldDefn ocGNField( GROUP_NUMBER, OFTInteger );
        poFeatureDefn->AddFieldDefn( &ocGNField );
        OGRFieldDefn ocNGField( NUMBER_IN_GROUP, OFTInteger );
        poFeatureDefn->AddFieldDefn( &ocNGField );

        for( const auto &field : oSXFDefn.GetFields() )
        {
            OGRFieldDefn fieldDefn(field.osName.c_str(), field.eFieldType);
            fieldDefn.SetAlternativeName(field.osAlias.c_str());
            if( field.nWidth > 0 && field.eFieldType == OFTString)
            {
                fieldDefn.SetWidth(field.nWidth);
            }
            poFeatureDefn->AddFieldDefn( &fieldDefn );
        }
    }
    else
    {
        OGRFieldDefn oNumField( OBJECTNUMB, OFTInteger );
        oNumField.SetWidth(10);
        poFeatureDefn->AddFieldDefn( &oNumField );

        OGRFieldDefn oAngField( ANGLE, OFTReal );
        poFeatureDefn->AddFieldDefn( &oAngField );

    }

    // Field TEXT always present fo labels
    OGRFieldDefn  oTextField( TEXT, OFTString );
    oTextField.SetWidth(255);
    poFeatureDefn->AddFieldDefn( &oTextField );

    if (!poDS->Encoding().empty())
    {
        SetAdvertizeUTF8(CPLCanRecode("test", poDS->Encoding().c_str(), 
            CPL_ENC_UTF8));
    }
    
    SetUpdatable(poDS->GetAccess() == GA_Update);
}

bool OGRSXFLayer::AddRecord(GIntBig nFID, const std::string &osClassCode, 
    const SXFFile &oSXF, vsi_l_offset nOffset, int nGroupID, int nSubObjectID)
{
    if( oSXFLayerDefn.HasCode(osClassCode) ||
        EQUAL(GetName(), "Not_Classified") )
    {
        VSIFSeekL(oSXF.File(), nOffset, SEEK_SET);
        auto poFeature = GetRawFeature(oSXF, nGroupID, nSubObjectID);
        if (poFeature)
        {
            poFeature->SetFID(nFID);
            // FIXME: Do we need this? poFeature->SetField(osFIDColumn.c_str(), nFID);
            SetUpdatable(true);  // Temporary toggle on updatable flag.
            CPL_IGNORE_RET_VAL(OGRMemLayer::SetFeature(poFeature));
            SetUpdatable(poDS->GetAccess() == GA_Update);
            OGRFeature::DestroyFeature(poFeature);
        }
        return true;
    }
    return false;
}


OGRErr OGRSXFLayer::GetExtent(OGREnvelope *psExtent, int bForce)
{
    if (bForce)
    {
        return OGRLayer::GetExtent(psExtent, bForce);
    }
    else
    {
        return poDS->GetExtent(psExtent);
    }
}

OGRErr OGRSXFLayer::GetExtent(int iGeomField, OGREnvelope *psExtent, int bForce)
{ 
    return OGRLayer::GetExtent(iGeomField, psExtent, bForce); 
}

int OGRSXFLayer::TestCapability( const char * pszCap )
{


    if (EQUAL(pszCap, OLCCurveGeometries) || EQUAL(pszCap, OLCMeasuredGeometries))
        return FALSE;
    return OGRMemLayer::TestCapability(pszCap);
}

GUInt32 OGRSXFLayer::TranslateXYH(const SXFFile &oSXF, const SXFRecordHeader &header, 
    GByte *psBuff, GUInt32 nBufLen, double *dfX, double *dfY, double *dfH )
{
/**
 * TODO : Take into account information given in the passport
 * like unit of measurement, type and dimensions (integer, float, double) of
 * coordinate, the vector format, etc.
 */
    // Xp, Yp(м) = Xo, Yo(м) + (Xd, Yd / R * S), (1)

    int offset = 0;
    switch (header.eCoordinateValueType)
    {
    case SXF_VT_SHORT:
    {
        if( nBufLen < 4 )
        {
            return 0;
        }
        GInt16 x, y;
        memcpy(&y, psBuff, 2);
        CPL_LSBPTR16(&y);
        memcpy(&x, psBuff + 2, 2);
        CPL_LSBPTR16(&x);

        oSXF.TranslateXY(x, y, dfX, dfY);

        offset += 4;

        if (dfH != nullptr)
        {
            if( nBufLen < 4 + 4 )
            {
                return 0;
            }
            float h;
            memcpy(&h, psBuff + 4, 4); // H always in float
            CPL_LSBPTR32(&h);
            *dfH = static_cast<double>(h);

            offset += 4;
        }
    }
    break;
    case SXF_VT_FLOAT:
    {
        if( nBufLen < 8 )
        {    
            return 0;
        }
        float x, y;
        memcpy(&y, psBuff, 4);
        CPL_LSBPTR32(&y);
        memcpy(&x, psBuff + 4, 4);
        CPL_LSBPTR32(&x);

        oSXF.TranslateXY(x, y, dfX, dfY);

        offset += 8;

        if (dfH != nullptr)
        {
            if( nBufLen < 8 + 4 )
            {
                return 0;
            }
            float h;
            memcpy(&h, psBuff + 8, 4); // H always in float
            CPL_LSBPTR32(&h);
            *dfH = static_cast<double>(h);

            offset += 4;
        }
    }
    break;
    case SXF_VT_INT:
    {
        if( nBufLen < 8 )
        {
            return 0;
        }
        GInt32 x, y;
        memcpy(&y, psBuff, 4);
        CPL_LSBPTR32(&y);
        memcpy(&x, psBuff + 4, 4);
        CPL_LSBPTR32(&x);

        oSXF.TranslateXY(x, y, dfX, dfY);

        offset += 8;

        if (dfH != nullptr)
        {
            if( nBufLen < 8 + 4 )
            {
                return 0;
            }
            float h;
            memcpy(&h, psBuff + 8, 4); // H always in float
            CPL_LSBPTR32(&h);
            *dfH = static_cast<double>(h);

            offset += 4;
        }
    }
    break;
    case SXF_VT_DOUBLE:
    {
        if( nBufLen < 16 )
        {
            return 0;
        }
        double x, y;
        memcpy(&y, psBuff, 8);
        CPL_LSBPTR64(&y);
        memcpy(&x, psBuff + 8, 8);
        CPL_LSBPTR64(&x);

        oSXF.TranslateXY(x, y, dfX, dfY);

        offset += 16;

        if (dfH != nullptr)
        {
            if( nBufLen < 16 + 8 )
            {    
                return 0;
            }
            double h;
            memcpy(&h, psBuff + 16, 8); // H in double
            CPL_LSBPTR64(&h);
            *dfH = static_cast<double>(h);

            offset += 8;
        }
    }
    break;
    };

    return offset;
}

static SXFValueType GetCoordinateValueType(GByte nType, GByte nSize)
{
    if(nType == 0)
    {
        if( nSize == 0)
        {
            return SXF_VT_SHORT;
        }
        else
        {
            return SXF_VT_INT;
        }        
    }
    else
    {
        if( nSize == 0)
        {
            return SXF_VT_FLOAT;
        }
        else
        {
            return SXF_VT_DOUBLE;
        }
    }    
}

// Limits to 100Mb in case of damaged data
constexpr GInt32 MAX_GEOMETRY_DATA_SIZE = 100 * 1024 * 1024;
// Limits to 1Mb in case of damaged data
constexpr GInt32 MAX_ATTRIBUTES_DATA_SIZE = 1024 * 1024;

static bool ReadRawFeautre(SXFRecordHeader &recordHeader, VSILFILE *pFile, 
    GUInt32 nVersion, const std::string &defaultEncoding)
{
    if (nVersion == 3)
    {
        SXFRecordHeaderV3 stRecordHeader;
        auto nObjectRead = VSIFReadL(&stRecordHeader, sizeof(SXFRecordHeaderV3), 
            1, pFile);
        if (nObjectRead != 1)
        {
            return false;
        }

        CPL_LSBPTR32(&(stRecordHeader.nSign));
        if (stRecordHeader.nSign != IDSXFOBJ)
        {
            return false;
        }
        CPL_LSBPTR32(&stRecordHeader.nGeometryLength);
        if (stRecordHeader.nGeometryLength > MAX_GEOMETRY_DATA_SIZE) 
        {
            return false;
        }      
        recordHeader.eGeometryType = 
            SXFFile::CodeToGeometryType(stRecordHeader.nLocalizaton);
        recordHeader.bHasZ = stRecordHeader.nDimension == 1;
        recordHeader.eCoordinateValueType = 
            GetCoordinateValueType(stRecordHeader.nElementType, 
                stRecordHeader.nCoordinateValueSize);
        recordHeader.nGeometryLength = stRecordHeader.nGeometryLength;
        CPL_LSBPTR16(&stRecordHeader.nSubObjectCount);
        recordHeader.nSubObjectCount = stRecordHeader.nSubObjectCount;
        CPL_LSBPTR32(&stRecordHeader.nClassifyCode);
        recordHeader.nClassifyCode = stRecordHeader.nClassifyCode;
        CPL_LSBPTR16(&stRecordHeader.nPointCount);
        recordHeader.nPointCount = stRecordHeader.nPointCount;

        CPL_LSBPTR16(&stRecordHeader.anGroup[0]);
        CPL_LSBPTR16(&stRecordHeader.anGroup[1]);
        recordHeader.nNumberInGroup = stRecordHeader.anGroup[0];
        recordHeader.nGroupNumber = stRecordHeader.anGroup[1];

        if (stRecordHeader.nHasSemantics == 1)
        {
            CPL_LSBPTR32(&stRecordHeader.nFullLength);
            GInt32 nAttributesLength = 
                stRecordHeader.nFullLength - 32 - stRecordHeader.nGeometryLength;
            if (nAttributesLength < 1 || nAttributesLength > MAX_ATTRIBUTES_DATA_SIZE)
            {
                return false;
            }
            recordHeader.nAttributesLength = nAttributesLength;
        }
        else
        {
            recordHeader.nAttributesLength = 0;
        }

        if (recordHeader.eGeometryType == SXF_GT_Text || 
            recordHeader.eGeometryType == SXF_GT_TextTemplate)
        {
            recordHeader.bHasTextSign = stRecordHeader.nIsText == 1;
            recordHeader.osEncoding = defaultEncoding;
        }
        else
        {
            recordHeader.bHasTextSign = false;
        }

        return true;
    } 
    else if (nVersion == 4)
    {
        SXFRecordHeaderV4 stRecordHeader;
        auto nObjectRead = VSIFReadL(&stRecordHeader, sizeof(SXFRecordHeaderV4),
            1, pFile);
        if (nObjectRead != 1)
        {
            return false;
        }
        CPL_LSBPTR32(&(stRecordHeader.nSign));
        if (stRecordHeader.nSign != IDSXFOBJ)
        {
            return false;
        }
        CPL_LSBPTR32(&stRecordHeader.nGeometryLength);
        if (stRecordHeader.nGeometryLength > MAX_GEOMETRY_DATA_SIZE) 
        {
            return false;
        }
        recordHeader.eGeometryType = 
            SXFFile::CodeToGeometryType(stRecordHeader.nLocalizaton);

        recordHeader.bHasZ = stRecordHeader.nDimension == 1;
        recordHeader.eCoordinateValueType = 
            GetCoordinateValueType(stRecordHeader.nElementType, 
                stRecordHeader.nCoordinateValueSize);
        recordHeader.nGeometryLength = stRecordHeader.nGeometryLength;
        CPL_LSBPTR16(&stRecordHeader.nSubObjectCount);
        recordHeader.nSubObjectCount = stRecordHeader.nSubObjectCount;
        CPL_LSBPTR32(&stRecordHeader.nClassifyCode);
        recordHeader.nClassifyCode = stRecordHeader.nClassifyCode;
        CPL_LSBPTR16(&stRecordHeader.nPointCountSmall);
        if (stRecordHeader.nPointCountSmall == 65535)
        {
            CPL_LSBPTR32(&stRecordHeader.nPointCount);
            recordHeader.nPointCount = stRecordHeader.nPointCount;
        }
        else
        {
            recordHeader.nPointCount = stRecordHeader.nPointCountSmall;
        }

        CPL_LSBPTR16(&stRecordHeader.anGroup[0]);
        CPL_LSBPTR16(&stRecordHeader.anGroup[1]);
        recordHeader.nNumberInGroup = stRecordHeader.anGroup[0];
        recordHeader.nGroupNumber = stRecordHeader.anGroup[1];

        if (stRecordHeader.nHasSemantics == 1)
        {
            CPL_LSBPTR32(&stRecordHeader.nFullLength);
            GInt32 nAttributesLength = 
                stRecordHeader.nFullLength - 32 - stRecordHeader.nGeometryLength;
            if (nAttributesLength < 1 || nAttributesLength > MAX_ATTRIBUTES_DATA_SIZE)
            {
                return false;
            }
            recordHeader.nAttributesLength = nAttributesLength;
        }
        else
        {
            recordHeader.nAttributesLength = 0;
        }

        if (recordHeader.eGeometryType == SXF_GT_Text || 
            recordHeader.eGeometryType == SXF_GT_TextTemplate)
        {
            recordHeader.bHasTextSign = stRecordHeader.nIsText == 1;
            if (stRecordHeader.nIsUTF16TextEnc == 1)
            {
                recordHeader.osEncoding = CPL_ENC_UTF16;
            }
            else
            {
                recordHeader.osEncoding = defaultEncoding;
            }
        }
        else
        {
            recordHeader.bHasTextSign = false;
        }
        return true;
    }

    return false;
}

void OGRSXFLayer::AddValue(OGRFeature *poFeature, const std::string &osFieldName, 
    const std::string &value)
{
    int nIndex = GetLayerDefn()->GetFieldIndex(osFieldName.c_str());
    if (IsFieldList(GetLayerDefn(), nIndex))
    {
        CPLStringList aosList(poFeature->GetFieldAsStringList(nIndex), FALSE);
        aosList.AddString(value.c_str());
        // FIXME: Is this needed? 
        // poFeature->SetField(nIndex, aosList);
        return;
    }
    poFeature->SetField(nIndex, value.c_str());
}

void OGRSXFLayer::AddValue(OGRFeature *poFeature, const std::string &osFieldName, 
    int value)
{
    int nIndex = GetLayerDefn()->GetFieldIndex(osFieldName.c_str());
    if (IsFieldList(GetLayerDefn(), nIndex))
    {
        int count = 0;
        auto list = poFeature->GetFieldAsIntegerList(nIndex, &count);
        auto size = count * sizeof(int);
        auto newList = static_cast<int*>(CPLMalloc(size + sizeof(int)));
        memcpy(newList, list, size);
        newList[count] = value;
        poFeature->SetField(nIndex, count + 1, newList);
        return;
    }
    poFeature->SetField(nIndex, value);
}

void OGRSXFLayer::AddValue(OGRFeature *poFeature, const std::string &osFieldName, 
    double value)
{
    int nIndex = GetLayerDefn()->GetFieldIndex(osFieldName.c_str());
    if (IsFieldList(GetLayerDefn(), nIndex))
    {
        int count = 0;
        auto list = poFeature->GetFieldAsDoubleList(nIndex, &count);
        auto size = count * sizeof(double);
        auto newList = static_cast<double*>(CPLMalloc(size + sizeof(double)));
        memcpy(newList, list, size);
        newList[count] = value;
        poFeature->SetField(nIndex, count + 1, newList);
        return;
    }
    poFeature->SetField(nIndex, value);
}

OGRFeature *OGRSXFLayer::GetRawFeature(const SXFFile &oSXF,  
    int nGroupID, int nSubObjectID)
{
    SXFRecordHeader stRecordHeader;
    if( !ReadRawFeautre(stRecordHeader, oSXF.File(), oSXF.Version(),
        oSXF.Encoding()) )
    {
        CPLError(CE_Failure, CPLE_FileIO, "SXF. Read record failed.");
        return nullptr;
    }

    std::shared_ptr<GByte> geometryBuff(
        static_cast<GByte*>(VSI_MALLOC_VERBOSE(stRecordHeader.nGeometryLength)), 
        CPLFree);
    if( geometryBuff == nullptr )
    {
        return nullptr;
    }

    if( VSIFReadL(geometryBuff.get(), stRecordHeader.nGeometryLength, 1, 
        oSXF.File()) != 1)
    {
        CPLError(CE_Failure, CPLE_FileIO, "SXF. Read geometry failed.");
        return nullptr;
    }

    struct VAR {
        OGRFieldType eType;
        std::string str;
        int iVal;
        double dfVal;

        VAR(OGRFieldType eTypeIn, const std::string &strIn, int iValIn, 
            double dfValIn)
            : eType(eTypeIn), str(strIn), iVal(iValIn), dfVal(dfValIn) {}
        int ToInt() {
            switch (eType)
            {
            case OFTInteger:
            case OFTIntegerList:
            case OFTInteger64:
            case OFTInteger64List:
                return iVal;
            case OFTReal:
            case OFTRealList:
                return int(dfVal);
            case OFTString:
            case OFTStringList:
                return atoi(str.c_str());
            default:
                return -1;
            }
        }
    };

    std::map<std::string, std::vector<VAR>> mFieldValues;

    if (stRecordHeader.nAttributesLength > 0)
    {
        std::shared_ptr<GByte> attributesBuff(
            static_cast<GByte*>(VSI_MALLOC_VERBOSE(stRecordHeader.nAttributesLength)),
            CPLFree);
        if (attributesBuff == nullptr)
        {
            return nullptr;
        }

        if (VSIFReadL(attributesBuff.get(), stRecordHeader.nAttributesLength, 1,
            oSXF.File()) != 1)
        {
            return nullptr;
        }

        size_t nOffset = 0;
        size_t nSemanticsSize = stRecordHeader.nAttributesLength;

        auto bIsUpdatable = IsUpdatable();
        SetUpdatable(true);

        while (nOffset + sizeof(SXFRecordAttributeInfo) < nSemanticsSize)
        {
            SXFRecordAttributeInfo stAttInfo;
            memcpy(&stAttInfo, attributesBuff.get() + nOffset,
                sizeof(SXFRecordAttributeInfo));
            CPL_LSBPTR16(&stAttInfo.nCode);
            nOffset += sizeof(SXFRecordAttributeInfo);

            std::string oFieldName = "SC_" + std::to_string(stAttInfo.nCode);

            switch (stAttInfo.nType)
            {
            case 0: // SXF_RAT_ASCIIZ_DOS
            {
                size_t nLen = size_t(stAttInfo.nScale) + 1;
                if (nOffset + nLen > nSemanticsSize)
                {
                    nSemanticsSize = 0;
                    break;
                }
                
                // Expected here that input encoding set by parameters or from 
                // SXF header is ASCIIZ
                auto val = SXF::ReadEncString(attributesBuff.get() +
                    nOffset, nLen, oSXF.Encoding().c_str()); 
                if (!HasField(GetLayerDefn(), oFieldName))
                {
                    OGRFieldDefn oField(oFieldName.c_str(), OFTString);
                    oField.SetWidth(255);
                    CreateField(&oField);
                }
                mFieldValues[oFieldName].emplace_back( OFTString , val, 0, 0.0 );
                nOffset += stAttInfo.nScale + 1;
                break;
            }
            case 1: // SXF_RAT_ONEBYTE
            {
                if (nOffset + 1 > nSemanticsSize)
                {
                    nSemanticsSize = 0;
                    break;
                }
                GByte nTmpVal;
                memcpy(&nTmpVal, attributesBuff.get() + nOffset, 1);

                char nScale = static_cast<char>(stAttInfo.nScale);
                auto val = double(nTmpVal) * pow(10.0,
                    static_cast<double>(nScale));
                if (!HasField(GetLayerDefn(), oFieldName))
                {
                    OGRFieldDefn oField(oFieldName.c_str(), OFTReal);
                    CreateField(&oField);
                }
                mFieldValues[oFieldName].emplace_back( OFTReal , "", 0, val );
                nOffset += 1;
                break;
            }
            case 2: // SXF_RAT_TWOBYTE
            {
                if (nOffset + 2 > nSemanticsSize)
                {
                    nSemanticsSize = 0;
                    break;
                }
                GInt16 nTmpVal;
                memcpy(&nTmpVal, attributesBuff.get() + nOffset, 2);
                CPL_LSBPTR16(&nTmpVal);

                char nScale = static_cast<char>(stAttInfo.nScale);
                auto val = double(CPL_LSBWORD16(nTmpVal)) * pow(10.0,
                    static_cast<double>(nScale));
                if (!HasField(GetLayerDefn(), oFieldName))
                {
                    OGRFieldDefn oField(oFieldName.c_str(), OFTReal);
                    CreateField(&oField);
                }
                mFieldValues[oFieldName].emplace_back( OFTReal , "", 0, val );
                nOffset += 2;
                break;
            }
            case 4: // SXF_RAT_FOURBYTE
            {
                if (nOffset + 4 > nSemanticsSize)
                {
                    nSemanticsSize = 0;
                    break;
                }
                GInt32 nTmpVal;
                memcpy(&nTmpVal, attributesBuff.get() + nOffset, 4);
                CPL_LSBPTR32(&nTmpVal);

                char nScale = static_cast<char>(stAttInfo.nScale);
                auto val = double(CPL_LSBWORD32(nTmpVal)) * pow(10.0,
                    static_cast<double>(nScale));
                if (!HasField(GetLayerDefn(), oFieldName))
                {
                    OGRFieldDefn oField(oFieldName.c_str(), OFTReal);
                    CreateField(&oField);
                }
                mFieldValues[oFieldName].emplace_back( OFTReal , "", 0, val );
                nOffset += 4;
                break;
            }
            case 8: // SXF_RAT_EIGHTBYTE
            {
                if (nOffset + 8 > nSemanticsSize)
                {
                    nSemanticsSize = 0;
                    break;
                }
                double dfTmpVal;
                memcpy(&dfTmpVal, attributesBuff.get() + nOffset, 8);
                CPL_LSBPTR64(&dfTmpVal);

                char nScale = static_cast<char>(stAttInfo.nScale);
                auto val = dfTmpVal * pow(10.0,
                    static_cast<double>(nScale));
                if (!HasField(GetLayerDefn(), oFieldName))
                {
                    OGRFieldDefn oField(oFieldName.c_str(), OFTReal);
                    CreateField(&oField);
                }
                mFieldValues[oFieldName].emplace_back( OFTReal , "", 0, val );
                nOffset += 8;
                break;
            }
            case 126: // SXF_RAT_ANSI_WIN
            {
                size_t nLen = size_t(stAttInfo.nScale) + 1;
                if (nOffset + nLen > nSemanticsSize)
                {
                    nSemanticsSize = 0;
                    break;
                }

                auto val =
                    SXF::ReadEncString(attributesBuff.get() + nOffset,
                        nLen, oSXF.Encoding().c_str());
                if (!HasField(GetLayerDefn(), oFieldName))
                {
                    OGRFieldDefn oField(oFieldName.c_str(), OFTString);
                    oField.SetWidth(255);
                    CreateField(&oField);
                }
                mFieldValues[oFieldName].emplace_back( OFTString , val, 0, 0.0 );
                nOffset += nLen;
                break;
            }
            case 127: // SXF_RAT_UNICODE
            {
                size_t nLen = (size_t(stAttInfo.nScale) + 1) * 2;
                if (nLen < 2 || nLen + nOffset > nSemanticsSize)
                {
                    nSemanticsSize = 0;
                    break;
                }
                auto src = static_cast<char*>(CPLMalloc(nLen));
                memcpy(src, attributesBuff.get() + nOffset, nLen - 2);
                src[nLen - 1] = 0;
                src[nLen - 2] = 0;
                auto dst = static_cast<char*>(CPLMalloc(nLen));
                int nCount = 0;
                for (unsigned i = 0; i < nLen; i += 2)
                {
                    unsigned char ucs = src[i];

                    if (ucs < 0x80U)
                    {
                        dst[nCount++] = ucs;
                    }
                    else
                    {
                        dst[nCount++] = 0xc0 | (ucs >> 6);
                        dst[nCount++] = 0x80 | (ucs & 0x3F);
                    }
                }
                if (!HasField(GetLayerDefn(), oFieldName))
                {
                    OGRFieldDefn oField(oFieldName.c_str(), OFTString);
                    oField.SetWidth(255);
                    CreateField(&oField);
                }
                mFieldValues[oFieldName].emplace_back( OFTString , dst, 0, 0.0 );
                CPLFree(dst);
                CPLFree(src);

                nOffset += nLen;
                break;
            }
            case 128: // SXF_RAT_BIGTEXT
            {
                // FIXME: Need example of UTF16 encoded data
                if (nOffset + 4 > nSemanticsSize)
                {
                    nSemanticsSize = 0;
                    break;
                }
                GUInt32 nTextLen;
                memcpy(&nTextLen, attributesBuff.get() + nOffset, 4);
                CPL_LSBPTR32(&nTextLen);
                nOffset += 4;

                if (nOffset + nTextLen > nSemanticsSize)
                {
                    nSemanticsSize = 0;
                    break;
                }

                std::string val;
                if (nTextLen > 0)
                {
                    char *pBuff = static_cast<char*>(CPLMalloc(nTextLen));
                    memcpy(pBuff, attributesBuff.get() + nOffset, nTextLen);
                    auto pszRecodedText = 
                        CPLRecodeFromWChar(reinterpret_cast<wchar_t*>(pBuff),
                            CPL_ENC_UTF16, CPL_ENC_UTF8);
                    val = pszRecodedText;
                    CPLFree(pszRecodedText);
                    CPLFree(pBuff);
                }
                
                if (!HasField(GetLayerDefn(), oFieldName))
                {
                    OGRFieldDefn oField(oFieldName.c_str(), OFTString);
                    oField.SetWidth(255);
                    CreateField(&oField);
                }
                mFieldValues[oFieldName].emplace_back( OFTString , val, 0, 0.0 );
                nOffset += nTextLen;
                break;
            }
            default:
                SetUpdatable(bIsUpdatable);
                return nullptr;
            }
        }
        SetUpdatable(bIsUpdatable);
    }

    OGRFeature *poFeature = nullptr;
    if (stRecordHeader.eGeometryType == SXF_GT_Point)
    {
        poFeature = TranslatePoint(oSXF, stRecordHeader, geometryBuff.get());
    }
    else if (stRecordHeader.eGeometryType == SXF_GT_Line)
    {
        poFeature = TranslateLine(oSXF, stRecordHeader, geometryBuff.get());
    }
    else if (stRecordHeader.eGeometryType == SXF_GT_Polygon)
    {
        poFeature = TranslatePolygon(oSXF, stRecordHeader, geometryBuff.get());
    }
    else if (stRecordHeader.eGeometryType == SXF_GT_Text ||
        stRecordHeader.eGeometryType == SXF_GT_TextTemplate)
    {
        poFeature = TranslateText(oSXF, stRecordHeader, geometryBuff.get(), 
            nSubObjectID);
    }
    else if (stRecordHeader.eGeometryType == SXF_GT_Vector)
    {
        poFeature = TranslateVetorAngle(oSXF, stRecordHeader, geometryBuff.get());
    }
    else
    {
        CPLError(CE_Failure, CPLE_NotSupported, "SXF. Unsupported geometry type.");
        return nullptr;
    }

    if (poFeature == nullptr)
    {
        CPLError(CE_Failure, CPLE_AppDefined, "SXF. Failed fill feature.");
        return nullptr;
    }

    poFeature->SetField(CLCODE, stRecordHeader.nClassifyCode);

    auto osStrCode = SXFFile::ToStringCode(stRecordHeader.eGeometryType, 
        stRecordHeader.nClassifyCode);
   
    for (auto fieldVal : mFieldValues)
    {
        for (auto val : fieldVal.second)
        {
            switch (val.eType)
            {
            case OFTString:
                AddValue(poFeature, fieldVal.first, val.str);
                break;
            case OFTReal:
                AddValue(poFeature, fieldVal.first, val.dfVal);
                break;
            case OFTInteger:
                AddValue(poFeature, fieldVal.first, val.iVal);
                break;
            default:
                break;
            }
        }
    }

    int nExt = GetExtention(osStrCode, *poFeature, oSXFLayerDefn);
    auto osName = oSXFLayerDefn.GetCodeName(osStrCode, nExt);
    poFeature->SetField(CLNAME, osName.c_str());

    if (bIsNewBehavior)
    {
        poFeature->SetField(OT,
            SXFFile::SXFTypeToString(stRecordHeader.eGeometryType).c_str());
        // Add extra 10000 as group id and id in group may present
        poFeature->SetField(GROUP_NUMBER,
            stRecordHeader.nGroupNumber + 10000 * nGroupID);
        poFeature->SetField(NUMBER_IN_GROUP,
            stRecordHeader.nNumberInGroup + 10000 * nSubObjectID);
        // Add EXT
        poFeature->SetField(EXT, nExt);
    }
    else
    {
        poFeature->SetField(OBJECTNUMB, stRecordHeader.nSubObjectCount);
    }

    return poFeature;
}


GUInt32 OGRSXFLayer::TranslatePoint(const SXFFile &oSXF, OGRPoint *poPT, 
    const SXFRecordHeader &header, GByte *pBuff, GUInt32 nBuffSize)
{
    double dfX = 0.0;
    double dfY = 0.0;
    double dfZ = 0.0;
    GUInt32 nDelta;

    if (header.bHasZ)
    {
        nDelta = TranslateXYH( oSXF, header, pBuff, nBuffSize, &dfX, &dfY, &dfZ );
        poPT->setZ(dfZ);
    }
    else
    {
        nDelta = TranslateXYH( oSXF, header, pBuff, nBuffSize, &dfX, &dfY );
    }

    poPT->setX(dfX);
    poPT->setY(dfY);

    return nDelta;
}

OGRFeature *OGRSXFLayer::TranslatePoint(const SXFFile &oSXF, 
    const SXFRecordHeader &header, GByte *psRecordBuf)
{
    auto poFeatureDefn = GetLayerDefn();
    OGRFeature *poFeature = new OGRFeature(poFeatureDefn);

    auto poSpaRef = poFeatureDefn->GetGeomFieldDefn(0)->GetSpatialRef();

    OGRPoint *poPT = new OGRPoint();
    GUInt32 nOffset = TranslatePoint(oSXF, poPT, header, psRecordBuf, 
        header.nGeometryLength);
    if (header.nSubObjectCount == 0)
    {
        poPT->assignSpatialReference( poSpaRef );
        poFeature->SetGeometryDirectly( poPT );
        return poFeature;
    }

    OGRMultiPoint *poMPt = new OGRMultiPoint();
    poMPt->addGeometryDirectly( poPT );

/*---------------------- Reading SubObjects --------------------------------*/

    for (int count = 0; count < header.nSubObjectCount; count++)
    {
        if (nOffset + 4 > header.nGeometryLength)
        {
            break;
        }

        GUInt16 nSubObj,nCoords;
        memcpy(&nSubObj, psRecordBuf + nOffset, 2);
        CPL_LSBPTR16(&nSubObj);
        memcpy(&nCoords, psRecordBuf + nOffset + 2, 2);
        CPL_LSBPTR16(&nCoords);

        if (header.nPointCount > 65535)
        {
            nCoords += nSubObj << 16;
        }

        nOffset +=4;

        for (int i = 0; i < nCoords ; i++)
        {
            poPT = new OGRPoint();
            nOffset += TranslatePoint(oSXF, poPT, header, psRecordBuf + nOffset, 
                header.nGeometryLength - nOffset);

            poMPt->addGeometryDirectly( poPT );
        }
    }

    poMPt->assignSpatialReference( poSpaRef );
    poFeature->SetGeometryDirectly( poMPt );

    return poFeature;
}

GUInt32 OGRSXFLayer::TranslateLine(const SXFFile &oSXF, OGRLineString* poLS,
    const SXFRecordHeader &header, GByte *pBuff, GUInt32 nBuffSize, 
    GUInt32 nPointCount)
{
    double dfX, dfY;
    double dfZ = 0.0;
    GUInt32 nOffset = 0; 
    for (GUInt32 count = 0; count < nPointCount; count++)
    {
        auto psCoords = pBuff + nOffset;

        GUInt32 nDelta;
        if (header.bHasZ)
        {
            nDelta = TranslateXYH( oSXF, header, psCoords, nBuffSize - nOffset, 
                &dfX, &dfY, &dfZ );
        }
        else
        {
            dfZ = 0.0;
            nDelta = TranslateXYH( oSXF, header, psCoords, nBuffSize - nOffset, 
                &dfX, &dfY );
        }

        if (nDelta == 0)
        {
            break;
        }
        poLS->addPoint( dfX, dfY, dfZ );
        nOffset += nDelta;
    }
    return nOffset;
}

OGRFeature *OGRSXFLayer::TranslateLine(const SXFFile &oSXF, 
    const SXFRecordHeader &header, GByte *psRecordBuf)
{
    auto poFeatureDefn = GetLayerDefn();
    OGRFeature *poFeature = new OGRFeature(poFeatureDefn);
    OGRMultiLineString *poMLS = new OGRMultiLineString();

/*---------------------- Reading Primary Line ----------------------------*/

    OGRLineString *poLS = new OGRLineString();
    GUInt32 nOffset = TranslateLine(oSXF, poLS, header, psRecordBuf, 
        header.nGeometryLength, header.nPointCount);
    poMLS->addGeometryDirectly( poLS );

/*---------------------- Reading Sub Lines --------------------------------*/

    for (int count = 0; count < header.nSubObjectCount; count++)
    {
        if (nOffset + 4 > header.nGeometryLength)
        {    
            break;
        }
        GUInt16 nSubObj, nCoords;
        memcpy(&nSubObj, psRecordBuf + nOffset, 2);
        CPL_LSBPTR16(&nSubObj);
        memcpy(&nCoords, psRecordBuf + nOffset + 2, 2);
        CPL_LSBPTR16(&nCoords);

        if (header.nPointCount > 65535)
        {
            nCoords += nSubObj << 16;
        }

        nOffset +=4;

        poLS = new OGRLineString();
        nOffset += TranslateLine(oSXF, poLS, header, psRecordBuf + nOffset, 
            header.nGeometryLength - nOffset, nCoords);

        poMLS->addGeometryDirectly( poLS );
    }

    auto poSpaRef = poFeatureDefn->GetGeomFieldDefn(0)->GetSpatialRef();
    poMLS->assignSpatialReference( poSpaRef );
    poFeature->SetGeometryDirectly( poMLS );

    return poFeature;
}

OGRFeature *OGRSXFLayer::TranslateVetorAngle(const SXFFile &oSXF, 
    const SXFRecordHeader &header, GByte *psRecordBuf)
{
    if (header.nPointCount != 2)
    {
        CPLError(CE_Failure, CPLE_NotSupported,
            "SXF. The vector object should have 2 points.");
        return nullptr;
    }

    auto poFeatureDefn = GetLayerDefn();
    OGRFeature *poFeature = new OGRFeature(poFeatureDefn);
    auto poSpaRef = poFeatureDefn->GetGeomFieldDefn(0)->GetSpatialRef();

    /*---------------------- Reading Primary Line --------------------------*/

    OGRLineString* poLS = new OGRLineString();
    TranslateLine(oSXF, poLS, header, psRecordBuf, header.nGeometryLength, 
        header.nPointCount);

    if (bIsNewBehavior)
    {
        poLS->assignSpatialReference( poSpaRef );
        poFeature->SetGeometryDirectly( poLS );
    }
    else
    {
        OGRPoint *poPT = new OGRPoint();
        poLS->StartPoint(poPT);

        OGRPoint *poAngPT = new OGRPoint();
        poLS->EndPoint(poAngPT);

        const double xDiff = poPT->getX() - poAngPT->getX();
        const double yDiff = poPT->getY() - poAngPT->getY();
        double dfAngle = atan2(xDiff, yDiff) * TO_DEGREES - 90;
        if (dfAngle < 0)
        {
            dfAngle += 360;
        }
        poPT->assignSpatialReference( poSpaRef );
        poFeature->SetGeometryDirectly( poPT );
        poFeature->SetField("ANGLE", dfAngle);

        delete poAngPT;
        delete poLS;
    }

    return poFeature;
}

OGRFeature *OGRSXFLayer::TranslatePolygon(const SXFFile &oSXF, 
    const SXFRecordHeader &header, GByte *psRecordBuf)
{
    auto poFeatureDefn = GetLayerDefn();
    OGRFeature *poFeature = new OGRFeature(poFeatureDefn);
    auto poSpaRef = poFeatureDefn->GetGeomFieldDefn(0)->GetSpatialRef();

    std::vector<OGRPolygon*> apoPolygons;
    OGRPolygon *poPoly = new OGRPolygon();

/*---------------------- Reading Primary Polygon --------------------------*/
    OGRLineString* poLS = new OGRLineString();
    GUInt32 nOffset = TranslateLine(oSXF, poLS, header, psRecordBuf, 
        header.nGeometryLength, header.nPointCount);
    OGRLinearRing *poLR = new OGRLinearRing();
    poLR->addSubLineString( poLS, 0 );
    poLR->closeRings();
    poPoly->addRingDirectly( poLR );

    apoPolygons.emplace_back(poPoly);

/*---------------------- Reading Sub Lines --------------------------------*/

    for (int count = 0; count < header.nSubObjectCount; count++)
    {

        if (nOffset + 4 > header.nGeometryLength)
        {
            break;
        }
        GUInt16 nSubObj, nCoords;
        memcpy(&nSubObj, psRecordBuf + nOffset, 2);
        CPL_LSBPTR16(&nSubObj);
        memcpy(&nCoords, psRecordBuf + nOffset + 2, 2);
        CPL_LSBPTR16(&nCoords);
        
        if (header.nPointCount > 65535)
        {
            nCoords += nSubObj << 16;
        }

        nOffset +=4;

        poLS->empty();
        nOffset += TranslateLine(oSXF, poLS, header, psRecordBuf + nOffset, 
            header.nGeometryLength - nOffset, nCoords);

        bool bIsExternalRing = true;
        for (auto poPolygon : apoPolygons)
        {
            if (poLS->Within(poPolygon))
            {
                poLR = new OGRLinearRing();
                poLR->addSubLineString( poLS, 0 );
                poLR->closeRings();

                poPolygon->addRingDirectly( poLR );
                bIsExternalRing = false;
                break;
            }
        }

        if (bIsExternalRing)
        {
            OGRPolygon *poPoly2 = new OGRPolygon();
            poLR = new OGRLinearRing();
            poLR->addSubLineString(poLS, 0);
            poLR->closeRings();
            poPoly2->addRingDirectly(poLR);

            apoPolygons.emplace_back(poPoly2);
        }
    }

    if (apoPolygons.size() > 1)
    {
        OGRMultiPolygon *poMultiPoly = new OGRMultiPolygon();
        for (auto poPolygon : apoPolygons)
        {
            poMultiPoly->addGeometryDirectly(poPolygon);
        }
        poMultiPoly->assignSpatialReference( poSpaRef );
        poFeature->SetGeometryDirectly( poMultiPoly );
    }
    else
    {
        poPoly->assignSpatialReference( poSpaRef );
        poFeature->SetGeometryDirectly( poPoly );
    }
    delete poLS;

    return poFeature;
}

OGRFeature *OGRSXFLayer::TranslateText(const SXFFile &oSXF, 
    const SXFRecordHeader &header, GByte *psRecordBuf, int nSubObject)
{
    auto poFeatureDefn = GetLayerDefn();
    OGRFeature *poFeature = new OGRFeature(poFeatureDefn);
    auto poSpaRef = poFeatureDefn->GetGeomFieldDefn(0)->GetSpatialRef();

    CPLString soText;
    if (header.nPointCount > 1)
    {
        auto poLS = new  OGRLineString();

  /*---------------------- Reading Primary Line ----------------------------*/

        GUInt32 nOffset = TranslateLine(oSXF, poLS, header, psRecordBuf, 
            header.nGeometryLength, header.nPointCount);

  /*------------------     READING TEXT VALUE   ----------------------------*/

        if (header.bHasTextSign)
        {
            if (nOffset + 1 > header.nGeometryLength)
            {
                return poFeature;
            }

            GByte nTextL;
            memcpy(&nTextL, psRecordBuf + nOffset, 1);
            nOffset += 1;
            if (nOffset + nTextL > header.nGeometryLength)
            {
                delete poLS;
                return poFeature;
            }

            nTextL += 1;

            soText = SXF::ReadEncString(psRecordBuf + nOffset, nTextL,
                header.osEncoding.c_str());
            nOffset += nTextL;
        }

        if (nSubObject < 2)
        {
            poLS->assignSpatialReference( poSpaRef );
            poFeature->SetGeometryDirectly( poLS );
        }
        else
        {

    /*---------------------- Reading Sub Lines --------------------------------*/

            for (int count = 0; count < header.nSubObjectCount; count++)
            {
                if (nOffset + 4 > header.nGeometryLength)
                {
                    break;
                }

                GUInt16 nSubObj, nCoords;
                memcpy(&nSubObj, psRecordBuf + nOffset, 2);
                CPL_LSBPTR16(&nSubObj);
                memcpy(&nCoords, psRecordBuf + nOffset + 2, 2);
                CPL_LSBPTR16(&nCoords);

                if (header.nPointCount > 65535)
                {
                    nCoords += nSubObj << 16;
                }

                nOffset +=4;

                poLS->empty();

                nOffset += TranslateLine(oSXF, poLS, header, psRecordBuf + nOffset,
                    header.nGeometryLength - nOffset, nCoords);

                if( nOffset + 1 > header.nGeometryLength )
                {
                    delete poLS;
                    return poFeature;
                }

                GByte nTextL;
                memcpy(&nTextL, psRecordBuf + nOffset, 1);
                nOffset += 1;
                if (nOffset + nTextL > header.nGeometryLength)
                {
                    delete poLS;
                    return poFeature;
                }

                nTextL += 1;

                soText = SXF::ReadEncString(psRecordBuf + nOffset,
                    nTextL, header.osEncoding.c_str());

                if (count == nSubObject - 2)
                {
                    poLS->assignSpatialReference( poSpaRef );
                    poFeature->SetGeometryDirectly( poLS );
                    break;
                }

                nOffset += nTextL;
            }
        }
    }
    else if (header.nPointCount == 1)
    {
        // FIXME: Need real example of text template
        OGRMultiPoint *poMPT = new OGRMultiPoint();

    /*---------------------- Reading Primary Line --------------------------------*/

        OGRPoint *poPT = new OGRPoint();
        GUInt32 nOffset = TranslatePoint(oSXF, poPT, header, psRecordBuf, 
            header.nGeometryLength);
        poMPT->addGeometryDirectly( poPT );

    /*------------------     READING TEXT VALUE   --------------------------------*/

        if (header.bHasTextSign)
        {
            if (nOffset + 1 > header.nGeometryLength)
            {
                return poFeature;
            }

            GByte nTextL;
            memcpy(&nTextL, psRecordBuf + nOffset, 1);
            nOffset += 1;
            if (nOffset + nTextL > header.nGeometryLength)
            {
                return poFeature;
            }

            nTextL += 1;

            soText = SXF::ReadEncString(psRecordBuf + nOffset, nTextL,
                header.osEncoding.c_str());
            nOffset += nTextL;
        }

    /*---------------------- Reading Sub Lines --------------------------------*/

        for (int count = 0; count < header.nSubObjectCount; count++)
        {
            if (nOffset + 4 > header.nGeometryLength)
            {
                poMPT->assignSpatialReference( poSpaRef );
                poFeature->SetGeometryDirectly( poMPT );
                break;
            }

            GUInt16 nSubObj, nCoords;
            memcpy(&nSubObj, psRecordBuf + nOffset, 2);
            CPL_LSBPTR16(&nSubObj);
            memcpy(&nCoords, psRecordBuf + nOffset + 2, 2);
            CPL_LSBPTR16(&nCoords);

            if (header.nPointCount > 65535)
            {
                nCoords += nSubObj << 16;
            }

            nOffset += 4;

            for (int j = 0; j < nCoords; j++)
            {
                poPT = new OGRPoint();

                nOffset += TranslatePoint(oSXF, poPT, header, psRecordBuf + nOffset,
                    header.nGeometryLength - nOffset);
                // FIXME: Skip addtional points in text template
                // poMPT->addGeometryDirectly(poPT);

                if (nOffset + 1 > header.nGeometryLength)
                {
                    return poFeature;
                }
            }

            GByte nTextL;
            memcpy(&nTextL, psRecordBuf + nOffset, 1);
            nOffset += 1;
            if( nOffset + nTextL > header.nGeometryLength )
            {
                return poFeature;
            }

            nTextL += 1;

            soText += " " + SXF::ReadEncString(psRecordBuf + nOffset,
                nTextL, header.osEncoding.c_str());

            nOffset += nTextL;
        }

        poMPT->assignSpatialReference( poSpaRef );
        poFeature->SetGeometryDirectly( poMPT );
    }

    poFeature->SetField("TEXT", soText);
    return poFeature;
}

OGRErr OGRSXFLayer::SyncToDisk()
{
    poDS->FlushCache();
    return OGRERR_NONE;
}

int OGRSXFLayer::SetRawFeature(OGRFeature *poFeature, OGRGeometry *poGeom, 
    const SXFFile &oSXF, const std::map<std::string, int> &mnClassCodes)
{
    if (!poGeom)
    {
        CPLError(CE_Warning, CPLE_AppDefined,
            "SXF. Geometry is mandatory. Unexpected entry.");
        return 0;
    }

    if (wkbFlatten(poGeom->getGeometryType()) == wkbGeometryCollection)
    {
        int nWrites = 0;
        OGRGeometryCollection *poGC = 
            static_cast<OGRGeometryCollection*>(poGeom);
        for (int iGeom = 0; iGeom < poGC->getNumGeometries(); iGeom++)
        {
            auto poSubGeom = poGC->getGeometryRef(iGeom);
            if (!poSubGeom)
            {
                continue;
            }
            nWrites += SetRawFeature(poFeature, poSubGeom, oSXF, mnClassCodes);
        }
        return nWrites;
    }

    int nOTFieldIndex = poFeature->GetDefnRef()->GetFieldIndex(OT);
    SXFGeometryType eGeomType = SXF_GT_Unknown;
    // Set SXF geometry type from field
    if (nOTFieldIndex != -1)
    {
        eGeomType = SXFFile::StringToSXFType(
            poFeature->GetFieldAsString(nOTFieldIndex));
    }
    
    // Set SXF geometry type from OGR geometry type
    if(eGeomType == SXF_GT_Unknown)
    {
        eGeomType = OGRTypeToSXFType(poGeom->getGeometryType());
    }

    if (eGeomType == SXF_GT_Unknown || 
        !IsTypesCompatible(poGeom->getGeometryType(), eGeomType)) // Check if OGR geometry type compatible with SXF geometry type
    {
        CPLError(CE_Warning, CPLE_AppDefined,
            "SXF. Unsupported geometry type %s.", 
            poGeom->getGeometryName());
        return 0;
    }
    else if (eGeomType == SXF_GT_TextTemplate)
    {
        // Write templates not supported yet
        eGeomType = SXF_GT_Point;
    }
    
    GUInt32 nPointCount = 0;
    GUInt16 nPartsCount = 0;
    using OGRGeometryPtr = std::unique_ptr<OGRGeometry>;
    OGRGeometryPtr poWriteGeom;
    if (wkbFlatten(poGeom->getGeometryType()) == wkbPoint || 
        wkbFlatten(poGeom->getGeometryType()) == wkbMultiPoint)
    {
        if (eGeomType != SXF_GT_Point && eGeomType != SXF_GT_TextTemplate)
        {
            CPLError(CE_Warning, CPLE_AppDefined,
                "SXF. Inconsistent geometry type. Expected point or text template, got %s", 
                SXFFile::SXFTypeToString(eGeomType).c_str());
            return 0;
        }
        poWriteGeom = OGRGeometryPtr(poGeom->clone());
        nPointCount = 1;
        if (wkbFlatten(poGeom->getGeometryType()) == wkbMultiPoint)
        {
            auto poMulti = static_cast<OGRMultiPoint*>(poGeom);
            nPartsCount = static_cast<GUInt16>(poMulti->getNumGeometries()) - 1;
        }
    }
    else if (wkbFlatten(poGeom->getGeometryType()) == wkbLineString || 
        wkbFlatten(poGeom->getGeometryType()) == wkbMultiLineString)
    {
        if (eGeomType != SXF_GT_Line && eGeomType != SXF_GT_Vector && eGeomType != SXF_GT_Text)
        {
            CPLError(CE_Warning, CPLE_AppDefined,
                "SXF. Inconsistent geometry type. Expected line, vector or text, got %s",
                SXFFile::SXFTypeToString(eGeomType).c_str());
            return 0;
        }
        poWriteGeom = OGRGeometryPtr(poGeom->clone());

        OGRLineString *poLine;
        if (wkbFlatten(poGeom->getGeometryType()) == wkbMultiLineString)
        {
            auto poMulti = static_cast<OGRMultiLineString*>(poGeom);
            nPartsCount = static_cast<GUInt16>(poMulti->getNumGeometries()) - 1;
            poLine = static_cast<OGRLineString*>(poMulti->getGeometryRef(0));
        }
        else
        {
            poLine = static_cast<OGRLineString*>(poGeom);
        }

        if (poLine)
        {
            nPointCount = poLine->getNumPoints();
        }
    }
    else if (wkbFlatten(poGeom->getGeometryType()) == wkbPolygon ||
        wkbFlatten(poGeom->getGeometryType()) == wkbMultiPolygon)
    {
        if (eGeomType != SXF_GT_Polygon)
        {
            CPLError(CE_Warning, CPLE_AppDefined,
                "SXF. Inconsistent geometry type. Expected polygon, got %s",
                SXFFile::SXFTypeToString(eGeomType).c_str());
            return 0;
        }
        poWriteGeom = OGRGeometryPtr(poGeom->clone());

        OGRPolygon *poPoly = nullptr;
        if (wkbFlatten(poGeom->getGeometryType()) == wkbMultiPolygon)
        {
            auto poMulti = static_cast<OGRMultiPolygon*>(poGeom);
            for (int i = 0; i < poMulti->getNumGeometries(); i++)
            {
                auto tmpPoly = static_cast<OGRPolygon*>(poMulti->getGeometryRef(i));
                nPartsCount += tmpPoly->getNumInteriorRings() + 1;
                if (poPoly == nullptr)
                {
                    poPoly = tmpPoly;
                }
            }
            nPartsCount--;
        }
        else
        {
            poPoly = static_cast<OGRPolygon*>(poGeom);
            nPartsCount = poPoly->getNumInteriorRings();
        }

        if (poPoly)
        {
            auto poRing = poPoly->getExteriorRing();
            if (poRing)
            {
                nPointCount = poRing->getNumPoints();
            }
        }
    }
    // For PolyhedralSurface and TIN
    else if (wkbFlatten(poGeom->getGeometryType()) == wkbPolyhedralSurface || 
        wkbFlatten(poGeom->getGeometryType()) == wkbTIN)
    {
        if (eGeomType != SXF_GT_Polygon)
        {
            CPLError(CE_Warning, CPLE_AppDefined,
                "SXF. Inconsistent geometry type. Expected polygon, got %s",
                SXFFile::SXFTypeToString(eGeomType).c_str());
            return 0;
        }
        poWriteGeom = OGRGeometryPtr(OGRGeometryFactory::forceTo(
            poGeom->clone(), wkbMultiPolygon, nullptr));
        OGRPolygon *poPoly = nullptr;
        if (wkbFlatten(poGeom->getGeometryType()) == wkbMultiPolygon)
        {
            auto poMulti = static_cast<OGRMultiPolygon*>(poGeom);
            for (int i = 0; i < poMulti->getNumGeometries(); i++)
            {
                auto tmpPoly = static_cast<OGRPolygon*>(poMulti->getGeometryRef(i));
                nPartsCount += tmpPoly->getNumInteriorRings() + 1;
                if (poPoly == nullptr)
                {
                    poPoly = tmpPoly;
                }
            }
            nPartsCount--;
        }
        else
        {
            poPoly = static_cast<OGRPolygon*>(poGeom);
            nPartsCount = poPoly->getNumInteriorRings();
        }

        if (poPoly)
        {
            auto poRing = poPoly->getExteriorRing();
            if (poRing)
            {
                nPointCount = poRing->getNumPoints();
            }
        }
    }
    else
    {
        CPLError(CE_Warning, CPLE_AppDefined,
            "SXF. Unsupported geometry type %s.",
            poGeom->getGeometryName());
        return 0;
    }
    
    // SXF support only:
    // - point, multipoint
    // - linestring, multilinestring
    // - polygon, multipolygon
    // - special types VECTOR, TEXT and TEXTTEMPLATE

    SXFRecordHeaderV4 stRecordHeader = { 0 };
    stRecordHeader.nSign = IDSXFOBJ;

    int nFieldIndex = 0;
    bool bSetClcode = false;
    if ((nFieldIndex = poFeature->GetFieldIndex(CLCODE)) != -1)
    {
        if (poFeature->IsFieldSet(nFieldIndex))
        {
            stRecordHeader.nClassifyCode = poFeature->GetFieldAsInteger(nFieldIndex);
            bSetClcode = true;
        }
    }

    if(!bSetClcode)
    {
        stRecordHeader.nClassifyCode = oSXFLayerDefn.GenerateCode(eGeomType);
    }
    
    if ((nFieldIndex = poFeature->GetFieldIndex(GROUP_NUMBER)) != -1)
    {
        stRecordHeader.anGroup[0] = 
            static_cast<GUInt16>(poFeature->GetFieldAsInteger(nFieldIndex));
    }
    else
    {
        stRecordHeader.anGroup[0] = 0;
    }
    if ((nFieldIndex = poFeature->GetFieldIndex(NUMBER_IN_GROUP)) != -1)
    {
        stRecordHeader.anGroup[1] =
            static_cast<GUInt16>(poFeature->GetFieldAsInteger(nFieldIndex));
    }
    else
    {
        stRecordHeader.anGroup[1] = static_cast<GUInt16>(poFeature->GetFID());
    }

    stRecordHeader.nMultiPolygonPartsOut = 
        (poWriteGeom->getGeometryType() == wkbMultiPolygon && nPartsCount > 0) ? 1 : 0;
    stRecordHeader.nLocalizaton = static_cast<GByte>(eGeomType);
    stRecordHeader.nCoordinateValueSize = 1; // 1 - double precision
    stRecordHeader.nDimension = poWriteGeom->Is3D() ? 1 : 0;
    stRecordHeader.nElementType = 1; // 1 - double
    if (eGeomType == SXF_GT_Text || eGeomType == SXF_GT_TextTemplate)
    {
        stRecordHeader.nIsText = 1;
    }
    stRecordHeader.nLowViewScale = 15; //// 0xFF;
    stRecordHeader.nHighViewScale = 15; //// 0xFF;
    if (nPointCount > 65535)
    {
        stRecordHeader.nPointCount = nPointCount;
        stRecordHeader.nPointCountSmall = 65535;
    }
    else
    {
        stRecordHeader.nPointCount = nPointCount;
        stRecordHeader.nPointCountSmall = static_cast<GUInt16>(nPointCount);
    }
    stRecordHeader.nSubObjectCount = nPartsCount;

    // Write geometry
    size_t nGeometryBufferSize = 0;
    auto pszText = poFeature->GetFieldAsString(TEXT);
    auto pGeomBuffer = WriteGeometryToBuffer(poWriteGeom.get(),
        eGeomType, pszText, nGeometryBufferSize, poDS->Encoding());

    // Write attributes 
    size_t nAttributesBufferSize = 0;
    auto pAttributesBuffer = WriteAttributesToBuffer(poFeature, nAttributesBufferSize, 
        mnClassCodes, oSXF.Encoding());
    // stRecordHeader.nIsUTF16TextEnc = 0; // Default ANSI

    stRecordHeader.nHasSemantics = (nAttributesBufferSize > 0) ? 1 : 0;

    stRecordHeader.nFullLength = static_cast<GUInt32>(sizeof(SXFRecordHeaderV4) + 
        nGeometryBufferSize + nAttributesBufferSize);
    stRecordHeader.nGeometryLength = static_cast<GUInt32>(nGeometryBufferSize);

    // Write record data 
    VSIFWriteL(&stRecordHeader, sizeof(SXFRecordHeaderV4), 1, oSXF.File());
    VSIFWriteL(pGeomBuffer, nGeometryBufferSize, 1, oSXF.File());
    VSIFWriteL(pAttributesBuffer, nAttributesBufferSize, 1, oSXF.File());

    CPLFree(pGeomBuffer);
    CPLFree(pAttributesBuffer);

    return 1;
}

int OGRSXFLayer::Write(const SXFFile &oSXF, 
    const std::map<std::string, int> &mnClassCodes)
{
    int nWrites = 0;
    ResetReading();
    using OGRFeaturePtr = std::unique_ptr<OGRFeature> ;
    OGRFeaturePtr poFeature;
    while ((poFeature = OGRFeaturePtr(GetNextFeature())) != nullptr)
    {
        nWrites += SetRawFeature(poFeature.get(), 
            poFeature->GetGeometryRef(), oSXF, mnClassCodes);
    }
    return nWrites;
}

OGRErr OGRSXFLayer::ISetFeature(OGRFeature *poFeature)
{
    auto eErr = OGRMemLayer::ISetFeature(poFeature);
    if (eErr != OGRERR_NONE)
    {
        return eErr;
    }

    auto geom = poFeature->GetGeometryRef();
    if (!geom)
    {
        CPLError(CE_Failure, CPLE_NotSupported,
            "SXF. Geometry is mandatory.");
        return OGRERR_UNSUPPORTED_GEOMETRY_TYPE;
    }
    OGREnvelope env;
    geom->getEnvelope(&env);
    poDS->UpdateExtent(env);

    AddCode(poFeature, oSXFLayerDefn);
    
    return eErr;
}

OGRErr OGRSXFLayer::ICreateFeature(OGRFeature *poFeature)
{
    auto eErr = OGRMemLayer::ICreateFeature(poFeature);
    if (eErr != OGRERR_NONE)
    {
        return eErr;
    }

    auto geom = poFeature->GetGeometryRef();
    if (!geom)
    {
        CPLError(CE_Failure, CPLE_NotSupported,
            "SXF. Geometry is mandatory.");
        return OGRERR_UNSUPPORTED_GEOMETRY_TYPE;
    }
    
    OGREnvelope env;
    geom->getEnvelope(&env);
    poDS->UpdateExtent(env);
    
    AddCode(poFeature, oSXFLayerDefn);

    return eErr;
}

OGRErr OGRSXFLayer::DeleteFeature(GIntBig nFID)
{
    auto eErr = OGRMemLayer::DeleteFeature(nFID);
    if (eErr != OGRERR_NONE)
    {
        return eErr;
    }
    poDS->SetHasChanges();
    return eErr;
}

OGRErr OGRSXFLayer::CreateField(OGRFieldDefn *poField, int bApproxOK)
{
    if (poField == nullptr || poField->GetType() > 5)
    {
        CPLError(CE_Failure, CPLE_NotSupported, 
            "SXF. Unsupported field type %d or field is null.", poField->GetType());
        return OGRERR_UNSUPPORTED_OPERATION;
    }
    return OGRMemLayer::CreateField(poField, bApproxOK);
}

std::string OGRSXFLayer::OGRFieldTypeToString(const OGRFieldType type)
{
    switch (type)
    {
    case OFTInteger:
    case OFTIntegerList:
        return "I";
    case OFTReal:
    case OFTRealList:
        return "R";
    case OFTString:
    case OFTStringList:
        return "S";
    default:
        return "";
    }
}

std::string OGRSXFLayer::CreateFieldKey(OGRFieldDefn *poFld)
{
    return std::string(poFld->GetNameRef()) + ":" + 
        OGRSXFLayer::OGRFieldTypeToString(poFld->GetType());
}


int OGRSXFLayer::GetFieldNameCode(const char * pszFieldName)
{
    int nCode = -1;
    if (pszFieldName == nullptr)
    {
        return nCode;
    }
    int n = 0; // For case of SC_8_txt - we only expected SC_<int>
    int cnt = sscanf(pszFieldName, "SC_%d%n", &nCode, &n);
    if (cnt == 1 && n > 0 && pszFieldName[n] == '\0')
    {
        return nCode;
    }
    return -1;
}

bool OGRSXFLayer::IsFieldNameHasCode(const char *pszFieldName)
{
    return OGRSXFLayer::GetFieldNameCode(pszFieldName) != -1 ||
        EQUAL(pszFieldName, CLCODE) ||
        EQUAL(pszFieldName, CLNAME) || 
        EQUAL(pszFieldName, OT) ||
        EQUAL(pszFieldName, EXT) ||
        EQUAL(pszFieldName, GROUP_NUMBER) || 
        EQUAL(pszFieldName, NUMBER_IN_GROUP) ||
        EQUAL(pszFieldName, OBJECTNUMB) ||
        EQUAL(pszFieldName, ANGLE) ||
        EQUAL(pszFieldName, TEXT);
}

std::vector<SXFClassCode> OGRSXFLayer::GetClassifyCodes()
{
    return oSXFLayerDefn.GetCodes(true);
}
