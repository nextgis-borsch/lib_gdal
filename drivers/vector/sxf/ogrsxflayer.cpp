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
 * Copyright (c) 2013-2020, NextGIS
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
#include "ogr_srs_api.h"
#include "cpl_multiproc.h"

CPL_CVSID("$Id$")

static std::string SXFTypeToString(enum SXFGeometryType type)
{
    switch (type)
    {
    case SXF_GT_Line:
        return "L";
    case SXF_GT_Polygon:
        return "S";
    case SXF_GT_Point:
        return "P";
    case SXF_GT_Text:
        return "T";
    case SXF_GT_Vector:
        return "V";
    case SXF_GT_TextTemplate:
        return "E";

    default:
        return "";
    }
}

constexpr int MAX_CACHE_SIZE = 1024;

////////////////////////////////////////////////////////////////////////////
/// OGRSXFLayer
///////////////////////////////////////////////////////?////////////////////

OGRSXFLayer::OGRSXFLayer(SXFFile *fp, GUInt16 nID, const char *pszLayerName,
    const std::vector<SXFField> &astFields, bool bIsNewBehavior) :
    OGRLayer(),
    poFeatureDefn(new OGRFeatureDefn(pszLayerName)),
    fpSXF(fp),
    nLayerID(nID),
    osFIDColumn("ogc_fid"),
    bIsNewBehavior(bIsNewBehavior)
{
    oNextIt = mnRecordDesc.begin();
    SetDescription( poFeatureDefn->GetName() );
    poFeatureDefn->Reference();

    poFeatureDefn->SetGeomType(wkbUnknown);
    if (poFeatureDefn->GetGeomFieldCount() != 0)
    {
        poFeatureDefn->GetGeomFieldDefn(0)->
            SetSpatialRef(fp->SpatialRef());
    }

    OGRFieldDefn oFIDField(osFIDColumn.c_str(), OFTInteger64);
    poFeatureDefn->AddFieldDefn(&oFIDField);

    OGRFieldDefn oClCodeField( "CLCODE", OFTInteger );
    oClCodeField.SetWidth(10);
    poFeatureDefn->AddFieldDefn( &oClCodeField );

    OGRFieldDefn oClNameField( "CLNAME", OFTString );
    oClNameField.SetWidth(32);
    poFeatureDefn->AddFieldDefn( &oClNameField );


    if( bIsNewBehavior )
    {
        OGRFieldDefn ocOTField( "OT", OFTString );
        ocOTField.SetWidth(1);
        poFeatureDefn->AddFieldDefn( &ocOTField );

        OGRFieldDefn ocGNField( "group_number", OFTInteger );
        poFeatureDefn->AddFieldDefn( &ocGNField );
        OGRFieldDefn ocNGField( "number_in_group", OFTInteger );
        poFeatureDefn->AddFieldDefn( &ocNGField );

        for( const auto &field : astFields )
        {
            OGRFieldDefn fieldDefn(field.osName.c_str(), field.eFieldType);
            fieldDefn.SetAlternativeName(field.osAlias.c_str());
            poFeatureDefn->AddFieldDefn( &fieldDefn );

            snAttributeCodes.insert(field.nCode);
        }
    }
    else
    {
        OGRFieldDefn oNumField( "OBJECTNUMB", OFTInteger );
        oNumField.SetWidth(10);
        poFeatureDefn->AddFieldDefn( &oNumField );

        OGRFieldDefn oAngField( "ANGLE", OFTReal );
        poFeatureDefn->AddFieldDefn( &oAngField );

    }

    // Field TEXT always present fo labels
    OGRFieldDefn  oTextField( "TEXT", OFTString );
    oTextField.SetWidth(255);
    poFeatureDefn->AddFieldDefn( &oTextField );
}

OGRSXFLayer::~OGRSXFLayer()
{
    for( auto nFID : anCacheIds)
    {
        DeleteCachedFeature(nFID);
    }
    poFeatureDefn->Release();
}

/************************************************************************/
/*                          AddClassifyCode                             */
/* Add layer supported classify codes. Only records with this code can  */
/* be in layer                                                          */
/************************************************************************/

void OGRSXFLayer::AddClassifyCode(GUInt32 nClassCode, const std::string &soName)
{
    if (soName.empty())
    {
        mnClassificators[nClassCode] = CPLSPrintf("%d", nClassCode);
    }
    else
    {
        mnClassificators[nClassCode] = soName;
    }
}

bool OGRSXFLayer::AddRecord( GIntBig nFID, GUInt32 nClassCode, vsi_l_offset nOffset,
        bool bHasSemantic, size_t nSemanticsSize, int nGroup, int nSubObjectId )
{
    if( mnClassificators.find(nClassCode) != mnClassificators.end() ||
        EQUAL(GetName(), "Not_Classified") )
    {
        mnRecordDesc[nFID] = {nOffset, nGroup, nSubObjectId, nullptr};
        // Add additional semantics (attribute fields).
        if (bHasSemantic)
        {
            size_t offset = 0;

            while( offset < nSemanticsSize )
            {
                SXFRecordAttributeInfo stAttrInfo;
                size_t nCurrOff = 0;
                size_t nReadObj = VSIFReadL(&stAttrInfo, 
                    sizeof(SXFRecordAttributeInfo), 1, fpSXF->File());
                if( nReadObj == 1 )
                {
                    CPL_LSBPTR16(&stAttrInfo.nCode);
                    CPLString oFieldName;
                    bool bAddField = false;
                    if( snAttributeCodes.find(stAttrInfo.nCode) == 
                        snAttributeCodes.end() )
                    {
                        bAddField = true;
                        snAttributeCodes.insert(stAttrInfo.nCode);
                        oFieldName.Printf("SC_%d", stAttrInfo.nCode);
                    }

                    SXFRecordAttributeType eType = 
                        static_cast<SXFRecordAttributeType>(stAttrInfo.nType);

                    offset += 4;

                    switch (eType)
                    {
                    case SXF_RAT_ASCIIZ_DOS:
                    {
                        if (bAddField)
                        {
                            OGRFieldDefn oField(oFieldName, OFTString);
                            oField.SetWidth(255);
                            poFeatureDefn->AddFieldDefn(&oField);
                        }
                        offset += stAttrInfo.nScale + 1;
                        nCurrOff = stAttrInfo.nScale + 1;
                        break;
                    }
                    case SXF_RAT_ONEBYTE:
                    {
                        if (bAddField)
                        {
                            OGRFieldDefn oField(oFieldName, OFTReal);
                            poFeatureDefn->AddFieldDefn(&oField);
                        }
                        offset += 1;
                        nCurrOff = 1;
                        break;
                    }
                    case SXF_RAT_TWOBYTE:
                    {
                        if (bAddField)
                        {
                            OGRFieldDefn oField(oFieldName, OFTReal);
                            poFeatureDefn->AddFieldDefn(&oField);
                        }
                        offset += 2;
                        nCurrOff = 2;
                        break;
                    }
                    case SXF_RAT_FOURBYTE:
                    {
                        if (bAddField)
                        {
                            OGRFieldDefn oField(oFieldName, OFTReal);
                            poFeatureDefn->AddFieldDefn(&oField);
                        }
                        offset += 4;
                        nCurrOff = 4;
                        break;
                    }
                    case SXF_RAT_EIGHTBYTE:
                    {
                        if (bAddField)
                        {
                            OGRFieldDefn oField(oFieldName, OFTReal);
                            poFeatureDefn->AddFieldDefn(&oField);
                        }
                        offset += 8;
                        nCurrOff = 8;
                        break;
                    }
                    case SXF_RAT_ANSI_WIN:
                    {
                        if (bAddField)
                        {
                            OGRFieldDefn oField(oFieldName, OFTString);
                            oField.SetWidth(255);
                            poFeatureDefn->AddFieldDefn(&oField);
                        }
                        unsigned nLen = unsigned(stAttrInfo.nScale) + 1;
                        offset += nLen;
                        nCurrOff = nLen;
                        break;
                    }
                    case SXF_RAT_UNICODE:
                    {
                        if (bAddField)
                        {
                            OGRFieldDefn oField(oFieldName, OFTString);
                            oField.SetWidth(255);
                            poFeatureDefn->AddFieldDefn(&oField);
                        }
                        unsigned nLen = (unsigned(stAttrInfo.nScale) + 1) * 2;
                        offset += nLen;
                        nCurrOff = nLen;
                        break;
                    }
                    case SXF_RAT_BIGTEXT:
                    {
                        if (bAddField)
                        {
                            OGRFieldDefn oField(oFieldName, OFTString);
                            oField.SetWidth(1024);
                            poFeatureDefn->AddFieldDefn(&oField);
                        }
                        GUInt32 scale2 = 0;
                        VSIFReadL(&scale2, sizeof(GUInt32), 1, fpSXF->File());
                        CPL_LSBPTR32(&scale2);

                        offset += scale2;
                        nCurrOff = scale2;
                        break;
                    }
                    default:
                        break;
                    }
                }
                if( nCurrOff == 0 )
                {
                    break;
                }
                VSIFSeekL(fpSXF->File(), nCurrOff, SEEK_CUR);
            }
        }
        return true;
    }

    return false;
}

OGRErr OGRSXFLayer::SetNextByIndex(GIntBig nIndex)
{
    if (nIndex < 0 || nIndex > static_cast<GIntBig>(mnRecordDesc.size()))
        return OGRERR_FAILURE;

    oNextIt = mnRecordDesc.begin();
    std::advance(oNextIt, static_cast<size_t>(nIndex));

    return OGRERR_NONE;
}

OGRFeature *OGRSXFLayer::GetFeature(GIntBig nFID)
{
    CPLMutexHolderD(fpSXF->Mutex());
    const auto IT = mnRecordDesc.find(nFID);
    if (IT != mnRecordDesc.end())
    {
        auto feature = IT->second;
        if(feature.poFeature != nullptr)
        {
            return feature.poFeature->Clone();
        }
        VSIFSeekL(fpSXF->File(), feature.offset, SEEK_SET);
        auto poFeature = GetNextRawFeature(IT->first, feature.group,
            feature.subObject);
        if( poFeature != nullptr && poFeature->GetGeometryRef() != nullptr && 
            GetSpatialRef() != nullptr )
        {
            poFeature->GetGeometryRef()->assignSpatialReference(GetSpatialRef());
        }
        return poFeature;
    }

    return nullptr;
}

OGRFeatureDefn *OGRSXFLayer::GetLayerDefn()
{ 
    return poFeatureDefn;
}


OGRSpatialReference *OGRSXFLayer::GetSpatialRef()
{
    return fpSXF->SpatialRef();
}

OGRErr OGRSXFLayer::GetExtent(OGREnvelope *psExtent, int bForce)
{
    if (bForce)
    {
        return OGRLayer::GetExtent(psExtent, bForce);
    }
    else
    {
        return fpSXF->FillExtent(psExtent);
    }
}

OGRErr OGRSXFLayer::GetExtent(int iGeomField, OGREnvelope *psExtent, int bForce)
{ 
    return OGRLayer::GetExtent(iGeomField, psExtent, bForce); 
}

GIntBig OGRSXFLayer::GetFeatureCount(int bForce)
{
    if (m_poFilterGeom == nullptr && m_poAttrQuery == nullptr)
        return static_cast<int>(mnRecordDesc.size());
    else
        return OGRLayer::GetFeatureCount(bForce);
}

void OGRSXFLayer::ResetReading()

{
    oNextIt = mnRecordDesc.begin();
}

OGRFeature *OGRSXFLayer::GetNextFeature()
{
    CPLMutexHolderD(fpSXF->Mutex());
    while( oNextIt != mnRecordDesc.end() )
    {
        auto feature = oNextIt->second;
        if(feature.poFeature != nullptr)
        {
            return feature.poFeature->Clone();
        }

        VSIFSeekL(fpSXF->File(), feature.offset, SEEK_SET);
        auto poFeature = GetNextRawFeature(oNextIt->first, feature.group,
            feature.subObject);

        ++oNextIt;

        if( poFeature == nullptr )
        {
            continue;
        }

        if( (m_poFilterGeom == nullptr
            || FilterGeometry(poFeature->GetGeometryRef()))
            && (m_poAttrQuery == nullptr
            || m_poAttrQuery->Evaluate(poFeature)) )
        {
            if( poFeature->GetGeometryRef() != nullptr && GetSpatialRef() != nullptr )
            {
                poFeature->GetGeometryRef()->assignSpatialReference(GetSpatialRef());
            }
            return poFeature;
        }
        delete poFeature;
    }
    return nullptr;
}

int OGRSXFLayer::TestCapability( const char * pszCap )

{
    if (EQUAL(pszCap, OLCStringsAsUTF8) &&
        CPLCanRecode("test", "CP1251", CPL_ENC_UTF8) &&
        CPLCanRecode("test", "KOI8-R", CPL_ENC_UTF8))
        return TRUE;
    else if (EQUAL(pszCap, OLCRandomRead))
        return TRUE;
    else if (EQUAL(pszCap, OLCFastFeatureCount))
        return TRUE;
    else if (EQUAL(pszCap, OLCFastGetExtent))
        return TRUE;
    else if (EQUAL(pszCap, OLCFastSetNextByIndex))
        return TRUE;

    return FALSE;
}

GUInt32 OGRSXFLayer::TranslateXYH( const SXFRecordHeader &header, GByte *psBuff, 
    GUInt32 nBufLen, double *dfX, double *dfY, double *dfH )
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

        fpSXF->TranslateXY(x, y, dfX, dfY);

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

        fpSXF->TranslateXY(x, y, dfX, dfY);

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

        fpSXF->TranslateXY(x, y, dfX, dfY);

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

        fpSXF->TranslateXY(x, y, dfX, dfY);

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
    if( nVersion == 3 )
    {
        SXFRecordHeaderV3 stRecordHeader;
        auto nObjectRead = VSIFReadL(&stRecordHeader, sizeof(SXFRecordHeaderV3), 
            1, pFile);
        if( nObjectRead != 1 )
        {
            return false;
        }

        CPL_LSBPTR32(&(stRecordHeader.nSign));
        if( stRecordHeader.nSign != IDSXFOBJ )
        {
            return false;
        }
        CPL_LSBPTR32(&stRecordHeader.nGeometryLength);
        if( stRecordHeader.nGeometryLength > MAX_GEOMETRY_DATA_SIZE) 
        {
            return false;
        }      
        recordHeader.eGeometryType = SXFFile::CodeToGeometryType(stRecordHeader.nLocalizaton);
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

        if(stRecordHeader.nHasSemantics == 1)
        {
            CPL_LSBPTR32(&stRecordHeader.nFullLength);
            GInt32 nAttributesLength = stRecordHeader.nFullLength - 32 - stRecordHeader.nGeometryLength;
            if( nAttributesLength < 1 || nAttributesLength > MAX_ATTRIBUTES_DATA_SIZE )
            {
                return false;
            }
            recordHeader.nAttributesLength = nAttributesLength;
        }
        else
        {
            recordHeader.nAttributesLength = 0;
        }

        if( recordHeader.eGeometryType == SXF_GT_Text || 
            recordHeader.eGeometryType == SXF_GT_TextTemplate )
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
    else if( nVersion == 4 )
    {
        SXFRecordHeaderV4 stRecordHeader;
        auto nObjectRead = VSIFReadL(&stRecordHeader, sizeof(SXFRecordHeaderV4),
            1, pFile);
        if( nObjectRead != 1 )
        {
            return false;
        }
        CPL_LSBPTR32(&(stRecordHeader.nSign));
        if( stRecordHeader.nSign != IDSXFOBJ )
        {
            return false;
        }
        CPL_LSBPTR32(&stRecordHeader.nGeometryLength);
        if( stRecordHeader.nGeometryLength > MAX_GEOMETRY_DATA_SIZE) 
        {
            return false;
        }
        recordHeader.eGeometryType = SXFFile::CodeToGeometryType(stRecordHeader.nLocalizaton);

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
        if( stRecordHeader.nPointCountSmall == 65535 )
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

        if( stRecordHeader.nHasSemantics == 1 )
        {
            CPL_LSBPTR32(&stRecordHeader.nFullLength);
            GInt32 nAttributesLength = stRecordHeader.nFullLength - 32 - stRecordHeader.nGeometryLength;
            if( nAttributesLength < 1 || nAttributesLength > MAX_ATTRIBUTES_DATA_SIZE )
            {
                return false;
            }
            recordHeader.nAttributesLength = nAttributesLength;
        }
        else
        {
            recordHeader.nAttributesLength = 0;
        }

        if( recordHeader.eGeometryType == SXF_GT_Text || 
            recordHeader.eGeometryType == SXF_GT_TextTemplate )
        {
            recordHeader.bHasTextSign = stRecordHeader.nIsText == 1;
            if( stRecordHeader.nIsUTF16TextEnc == 1 )
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

OGRFeature *OGRSXFLayer::GetNextRawFeature(GIntBig nFID, int nGroupId, int nSubObject)
{
    SXFRecordHeader stRecordHeader;
    if( !ReadRawFeautre(stRecordHeader, fpSXF->File(), fpSXF->Version(),
        fpSXF->Encoding()) )
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
        fpSXF->File()) != 1)
    {
        CPLError(CE_Failure, CPLE_FileIO, "SXF. Read geometry failed.");
        return nullptr;
    }

    OGRFeature *poFeature = nullptr;
    if( stRecordHeader.eGeometryType == SXF_GT_Point )
    {
        poFeature = TranslatePoint(stRecordHeader, geometryBuff.get());
    }
    else if( stRecordHeader.eGeometryType == SXF_GT_Line )
    {
        poFeature = TranslateLine(stRecordHeader, geometryBuff.get());
    }
    else if( stRecordHeader.eGeometryType == SXF_GT_Polygon )
    {
        poFeature = TranslatePolygon(stRecordHeader, geometryBuff.get());
    }
    else if( stRecordHeader.eGeometryType == SXF_GT_Text ||
        stRecordHeader.eGeometryType == SXF_GT_TextTemplate )
    {
        poFeature = TranslateText(stRecordHeader, geometryBuff.get(), nSubObject);
    }
    else if( stRecordHeader.eGeometryType == SXF_GT_Vector )
    {
        poFeature = TranslateVetorAngle(stRecordHeader, geometryBuff.get());
    }
    else
    {
        CPLError(CE_Failure, CPLE_NotSupported, "SXF. Unsupported geometry type.");
        return nullptr;
    }

    if( poFeature == nullptr )
    {
        CPLError(CE_Failure, CPLE_AppDefined, "SXF. Failed fill feature.");
        return nullptr;
    }

    poFeature->SetField(osFIDColumn.c_str(), static_cast<GIntBig>(nFID));
    poFeature->SetField("CLCODE", stRecordHeader.nClassifyCode);

    CPLString szName = mnClassificators[stRecordHeader.nClassifyCode];
    if (szName.empty())
    {
        szName.Printf("%d", stRecordHeader.nClassifyCode);
    }
    poFeature->SetField("CLNAME", szName);

    if( bIsNewBehavior )
    {
        poFeature->SetField("OT", SXFTypeToString(stRecordHeader.eGeometryType).c_str());
        // Add extra 10000 as group id and id in group may present
        poFeature->SetField("group_number", stRecordHeader.nGroupNumber + 10000 * nGroupId);
        poFeature->SetField("number_in_group", stRecordHeader.nNumberInGroup + 10000 * nSubObject);
    }
    else
    {
        poFeature->SetField("OBJECTNUMB", stRecordHeader.nSubObjectCount);
    }
        
    if( stRecordHeader.nAttributesLength > 0 )
    {
        std::shared_ptr<GByte> attributesBuff(
            static_cast<GByte*>(VSI_MALLOC_VERBOSE(stRecordHeader.nAttributesLength)), 
            CPLFree);
        if( attributesBuff == nullptr )
        {
            delete poFeature;
            return nullptr;
        }

        if( VSIFReadL(attributesBuff.get(), stRecordHeader.nAttributesLength, 1, 
            fpSXF->File()) != 1)
        {
            delete poFeature;
            return nullptr;
        }
        
        size_t nOffset = 0;
        size_t nSemanticsSize = stRecordHeader.nAttributesLength;

        while( nOffset + sizeof(SXFRecordAttributeInfo) < nSemanticsSize )
        {
            SXFRecordAttributeInfo stAttInfo;
            memcpy(&stAttInfo, attributesBuff.get() + nOffset, 
                sizeof(SXFRecordAttributeInfo));
            CPL_LSBPTR16(&stAttInfo.nCode);
            nOffset += sizeof(SXFRecordAttributeInfo);

            CPLString oFieldName;
            oFieldName.Printf("SC_%d", stAttInfo.nCode);

            CPLString oFieldValue;
            switch (stAttInfo.nType)
            {
                case 0: // SXF_RAT_ASCIIZ_DOS
                {
                    size_t nLen = size_t(stAttInfo.nScale) + 1;
                    if( nOffset + nLen > nSemanticsSize )
                    {
                        nSemanticsSize = 0;
                        break;
                    }
                    auto val = SXFFile::ReadSXFString(attributesBuff.get() + 
                        nOffset, nLen, "CP866");                    
                    poFeature->SetField(oFieldName, val.c_str());
                    nOffset += stAttInfo.nScale + 1;
                    break;
                }
                case 1: // SXF_RAT_ONEBYTE
                {
                    if( nOffset + 1 > nSemanticsSize )
                    {
                        nSemanticsSize = 0;
                        break;
                    }
                    GByte nTmpVal;
                    memcpy(&nTmpVal, attributesBuff.get() + nOffset, 1);
                    auto val = double(nTmpVal) * pow(10.0, 
                        static_cast<double>(stAttInfo.nScale));
                    poFeature->SetField(oFieldName, val);
                    nOffset += 1;
                    break;
                }
                case 2: // SXF_RAT_TWOBYTE
                {
                    if( nOffset + 2 > nSemanticsSize )
                    {
                        nSemanticsSize = 0;
                        break;
                    }
                    GInt16 nTmpVal;
                    memcpy(&nTmpVal, attributesBuff.get() + nOffset, 2);
                    CPL_LSBPTR16(&nTmpVal);
                    auto val = double(CPL_LSBWORD16(nTmpVal)) * pow(10.0, 
                        static_cast<double>(stAttInfo.nScale));

                    poFeature->SetField(oFieldName, val);
                    nOffset += 2;
                    break;
                }
                case 4: // SXF_RAT_FOURBYTE
                {
                    if( nOffset + 4 > nSemanticsSize )
                    {
                        nSemanticsSize = 0;
                        break;
                    }
                    GInt32 nTmpVal;
                    memcpy(&nTmpVal, attributesBuff.get() + nOffset, 4);
                    CPL_LSBPTR32(&nTmpVal);
                    auto val = double(CPL_LSBWORD32(nTmpVal)) * pow(10.0,
                        static_cast<double>(stAttInfo.nScale));

                    poFeature->SetField(oFieldName, val);
                    nOffset += 4;
                    break;
                }
                case 8: // SXF_RAT_EIGHTBYTE
                {
                    if( nOffset + 8 > nSemanticsSize )
                    {
                        nSemanticsSize = 0;
                        break;
                    }
                    double dfTmpVal;
                    memcpy(&dfTmpVal, attributesBuff.get() + nOffset, 8);
                    CPL_LSBPTR64(&dfTmpVal);
                    auto val = dfTmpVal * pow(10.0,
                        static_cast<double>(stAttInfo.nScale));

                    poFeature->SetField(oFieldName, val);
                    nOffset += 8;
                    break;
                }
                case 126: // SXF_RAT_ANSI_WIN
                {
                    size_t nLen = size_t(stAttInfo.nScale) + 1;
                    if( nOffset + nLen > nSemanticsSize )
                    {
                        nSemanticsSize = 0;
                        break;
                    }
                    
                    auto val = 
                        SXFFile::ReadSXFString(attributesBuff.get() + nOffset, 
                            nLen, "CP1251");                    
                    poFeature->SetField(oFieldName, val.c_str());

                    nOffset += nLen;
                    break;
                }
                case 127: // SXF_RAT_UNICODE
                {
                    size_t nLen = (size_t(stAttInfo.nScale) + 1) * 2;
                    if( nLen < 2 || nLen + nOffset > nSemanticsSize )
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
                    for(unsigned i = 0; i < nLen; i += 2)
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

                    poFeature->SetField(oFieldName, dst);
                    CPLFree(dst);
                    CPLFree(src);

                    nOffset += nLen;
                    break;
                }
                case 128: // SXF_RAT_BIGTEXT
                {
                    if( nOffset + 4 > nSemanticsSize )
                    {
                        nSemanticsSize = 0;
                        break;
                    }
                    GUInt32 scale2;
                    memcpy(&scale2, attributesBuff.get() + nOffset, 4);
                    CPL_LSBPTR32(&scale2);
                    nOffset += 4;

                    if( nOffset + scale2 > nSemanticsSize )
                    {
                        nSemanticsSize = 0;
                        break;
                    }

                    auto val = 
                        SXFFile::ReadSXFString(attributesBuff.get() + nOffset, 
                            scale2, CPL_ENC_UTF16);                    
                    poFeature->SetField(oFieldName, val.c_str());
                    
                    nOffset += scale2;
                    break;
                }
                default:
                    delete poFeature;
                    return nullptr;
            }
            
        }
     }

    poFeature->SetFID(nFID);

    return poFeature;
}


GUInt32 OGRSXFLayer::TranslatePoint(OGRPoint *poPT, const SXFRecordHeader &header, 
        GByte *pBuff, GUInt32 nBuffSize)
{
    double dfX = 0.0;
    double dfY = 0.0;
    double dfZ = 0.0;
    GUInt32 nDelta;

    if (header.bHasZ)
    {
        nDelta = TranslateXYH( header, pBuff, nBuffSize, &dfX, &dfY, &dfZ );
    }
    else
    {
        nDelta = TranslateXYH( header, pBuff, nBuffSize, &dfX, &dfY );
    }

    poPT->setX(dfX);
    poPT->setY(dfY);
    poPT->setZ(dfZ);

    return nDelta;
}

OGRFeature *OGRSXFLayer::TranslatePoint(const SXFRecordHeader &header,
    GByte *psRecordBuf)
{
    OGRFeature *poFeature = new OGRFeature(poFeatureDefn);
    OGRMultiPoint *poMPt = new OGRMultiPoint();

    OGRPoint *poPT = new OGRPoint();
    GUInt32 nOffset = TranslatePoint(poPT, header, psRecordBuf, 
        header.nGeometryLength);
    poMPt->addGeometryDirectly( poPT );

/*---------------------- Reading SubObjects --------------------------------*/

    for( int count = 0; count < header.nSubObjectCount; count++ )
    {
        if( nOffset + 4 > header.nGeometryLength )
        {
            break;
        }

        GUInt16 nSubObj,nCoords;
        memcpy(&nSubObj, psRecordBuf + nOffset, 2);
        CPL_LSBPTR16(&nSubObj);
        memcpy(&nCoords, psRecordBuf + nOffset + 2, 2);
        CPL_LSBPTR16(&nCoords);

        nOffset +=4;

        for( int i = 0; i < nCoords ; i++ )
        {
            poPT = new OGRPoint();
            nOffset += TranslatePoint(poPT, header, psRecordBuf + nOffset, 
                header.nGeometryLength - nOffset);

            poMPt->addGeometryDirectly( poPT );
        }
    }

    poFeature->SetGeometryDirectly( poMPt );

    return poFeature;
}

GUInt32 OGRSXFLayer::TranslateLine(OGRLineString* poLS, 
    const SXFRecordHeader &header, GByte *pBuff, GUInt32 nBuffSize, 
    GUInt32 nPointCount)
{
    double dfX, dfY;
    double dfZ = 0.0;
    GUInt32 nOffset = 0; 
    for( GUInt32 count = 0; count < nPointCount; count++ )
    {
        auto psCoords = pBuff + nOffset;

        GUInt32 nDelta;
        if( header.bHasZ )
        {
            nDelta = TranslateXYH( header, psCoords, nBuffSize - nOffset, 
                &dfX, &dfY, &dfZ );
        }
        else
        {
            dfZ = 0.0;
            nDelta = TranslateXYH( header, psCoords, nBuffSize - nOffset, 
                &dfX, &dfY );
        }

        if( nDelta == 0 )
        {
            break;
        }
        poLS->addPoint( dfX, dfY, dfZ );
        nOffset += nDelta;
    }
    return nOffset;
}

OGRFeature *OGRSXFLayer::TranslateLine(const SXFRecordHeader &header, 
    GByte *psRecordBuf)
{
    OGRFeature *poFeature = new OGRFeature(poFeatureDefn);
    OGRMultiLineString *poMLS = new OGRMultiLineString();

/*---------------------- Reading Primary Line ----------------------------*/

    OGRLineString *poLS = new OGRLineString();
    GUInt32 nOffset = TranslateLine(poLS, header, psRecordBuf, 
        header.nGeometryLength, header.nPointCount);
    poMLS->addGeometryDirectly( poLS );

/*---------------------- Reading Sub Lines --------------------------------*/

    for( int count = 0; count < header.nSubObjectCount; count++ )
    {
        if( nOffset + 4 > header.nGeometryLength )
        {    
            break;
        }
        GUInt16 nSubObj, nCoords;
        memcpy(&nSubObj, psRecordBuf + nOffset, 2);
        CPL_LSBPTR16(&nSubObj);
        memcpy(&nCoords, psRecordBuf + nOffset + 2, 2);
        CPL_LSBPTR16(&nCoords);

        nOffset +=4;

        poLS = new OGRLineString();
        nOffset += TranslateLine(poLS, header, psRecordBuf + nOffset, 
            header.nGeometryLength - nOffset, nCoords);

        poMLS->addGeometryDirectly( poLS );
    }

    poFeature->SetGeometryDirectly( poMLS );

    return poFeature;
}

OGRFeature *OGRSXFLayer::TranslateVetorAngle(const SXFRecordHeader &header, 
    GByte *psRecordBuf)
{
    if( header.nPointCount != 2 )
    {
        CPLError(CE_Failure, CPLE_NotSupported,
            "SXF. The vector object should have 2 points.");
        return nullptr;
    }

    OGRFeature *poFeature = new OGRFeature(poFeatureDefn);

    /*---------------------- Reading Primary Line --------------------------*/

    OGRLineString* poLS = new OGRLineString();
    TranslateLine(poLS, header, psRecordBuf, header.nGeometryLength, header.nPointCount);

    if( bIsNewBehavior )
    {
        poFeature->SetGeometryDirectly(poLS);
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
        poFeature->SetGeometryDirectly(poPT);
        poFeature->SetField("ANGLE", dfAngle);

        delete poAngPT;
        delete poLS;
    }

    return poFeature;
}

void OGRSXFLayer::DeleteCachedFeature(GIntBig nFID)
{
    auto record = mnRecordDesc.find(nFID);
    if(record != mnRecordDesc.end())
    {
        delete record->second.poFeature;
        record->second.poFeature = nullptr;
    }
}

void OGRSXFLayer::AddToCache(GIntBig nFID, OGRFeature *poFeature)
{
    auto record = mnRecordDesc.find(nFID);
    if(record != mnRecordDesc.end())
    {
        record->second.poFeature = poFeature;
        anCacheIds.insert(nFID);
    }

    if(anCacheIds.size() > MAX_CACHE_SIZE)
    {
        auto it = anCacheIds.begin();
        DeleteCachedFeature(*it);
    }
}

OGRFeature *OGRSXFLayer::TranslatePolygon(const SXFRecordHeader &header, 
    GByte *psRecordBuf)
{
    OGRFeature *poFeature = new OGRFeature(poFeatureDefn);
    OGRPolygon *poPoly = new OGRPolygon();

/*---------------------- Reading Primary Polygon --------------------------*/
    OGRLineString* poLS = new OGRLineString();
    GUInt32 nOffset = TranslateLine(poLS, header, psRecordBuf, 
        header.nGeometryLength, header.nPointCount);
    OGRLinearRing *poLR = new OGRLinearRing();
    poLR->addSubLineString( poLS, 0 );
    poPoly->addRingDirectly( poLR );

/*---------------------- Reading Sub Lines --------------------------------*/

    for( int count = 0; count < header.nSubObjectCount; count++ )
    {

        if( nOffset + 4 > header.nGeometryLength )
        {
            break;
        }
        GUInt16 nSubObj, nCoords;
        memcpy(&nSubObj, psRecordBuf + nOffset, 2);
        CPL_LSBPTR16(&nSubObj);
        memcpy(&nCoords, psRecordBuf + nOffset + 2, 2);
        CPL_LSBPTR16(&nCoords);

        nOffset +=4;

        poLS->empty();
        nOffset += TranslateLine(poLS, header, psRecordBuf + nOffset, 
            header.nGeometryLength - nOffset, nCoords);

        poLR = new OGRLinearRing();
        poLR->addSubLineString( poLS, 0 );

        poPoly->addRingDirectly( poLR );
    }

    poFeature->SetGeometryDirectly( poPoly );
    delete poLS;

    return poFeature;
}

OGRFeature *OGRSXFLayer::TranslateText(const SXFRecordHeader &header, 
    GByte *psRecordBuf, int nSubObject)
{
    OGRFeature *poFeature = new OGRFeature(poFeatureDefn);
    CPLString soText;
    if( header.nPointCount > 1 )
    {
        auto poLS = new  OGRLineString();

  /*---------------------- Reading Primary Line ----------------------------*/

        GUInt32 nOffset = TranslateLine(poLS, header, psRecordBuf, 
            header.nGeometryLength, header.nPointCount);

  /*------------------     READING TEXT VALUE   ----------------------------*/

        if ( header.bHasTextSign )
        {
            if( nOffset + 1 > header.nGeometryLength )
            {
                return poFeature;
            }

            GByte nTextL;
            memcpy(&nTextL, psRecordBuf + nOffset, 1);
            nOffset += 1;
            if( nOffset + nTextL > header.nGeometryLength )
            {
                delete poLS;
                return poFeature;
            }

            nTextL += 1;

            soText = SXFFile::ReadSXFString(psRecordBuf + nOffset, nTextL,
                header.osEncoding.c_str());
            nOffset += nTextL;
        }

        if(nSubObject < 2)
        {
            poFeature->SetGeometryDirectly( poLS );
        }
        else
        {

    /*---------------------- Reading Sub Lines --------------------------------*/

            for( int count = 0; count < header.nSubObjectCount; count++ )
            {
                if( nOffset + 4 > header.nGeometryLength )
                {
                    break;
                }

                GUInt16 nSubObj, nCoords;
                memcpy(&nSubObj, psRecordBuf + nOffset, 2);
                CPL_LSBPTR16(&nSubObj);
                memcpy(&nCoords, psRecordBuf + nOffset + 2, 2);
                CPL_LSBPTR16(&nCoords);

                nOffset +=4;

                poLS->empty();

                nOffset += TranslateLine(poLS, header, psRecordBuf + nOffset,
                    header.nGeometryLength - nOffset, nCoords);

                if( nOffset + 1 > header.nGeometryLength )
                {
                    delete poLS;
                    return poFeature;
                }

                GByte nTextL;
                memcpy(&nTextL, psRecordBuf + nOffset, 1);
                nOffset += 1;
                if( nOffset + nTextL > header.nGeometryLength )
                {
                    delete poLS;
                    return poFeature;
                }

                nTextL += 1;

                soText = SXFFile::ReadSXFString(psRecordBuf + nOffset,
                    nTextL, header.osEncoding.c_str());

                if( count == nSubObject - 1 )
                {
                    poFeature->SetGeometryDirectly( poLS );
                    break;
                }

                nOffset += nTextL;
            }
        }
    }
    else if( header.nPointCount == 1 )
    {
        // FIXME: Need real example of text template
        OGRMultiPoint *poMPT = new OGRMultiPoint();

    /*---------------------- Reading Primary Line --------------------------------*/

        OGRPoint *poPT = new OGRPoint();
        GUInt32 nOffset = TranslatePoint(poPT, header, psRecordBuf, 
            header.nGeometryLength);
        poMPT->addGeometryDirectly( poPT );

    /*------------------     READING TEXT VALUE   --------------------------------*/

        if ( header.bHasTextSign )
        {
            if( nOffset + 1 > header.nGeometryLength )
            {
                return poFeature;
            }

            GByte nTextL;
            memcpy(&nTextL, psRecordBuf + nOffset, 1);
            if( nOffset + 1 + nTextL > header.nGeometryLength )
            {
                return poFeature;
            }

            nTextL += 1;

            soText = SXFFile::ReadSXFString(psRecordBuf + nOffset + 1, nTextL,
                header.osEncoding.c_str());
            nOffset += nTextL;
        }

    /*---------------------- Reading Sub Lines --------------------------------*/

        for( int count = 0; count < header.nSubObjectCount; count++ )
        {
            if( nOffset + 4 > header.nGeometryLength )
            {    
                break;
            }
            GUInt16 nSubObj, nCoords;
            memcpy(&nSubObj, psRecordBuf + nOffset, 2);
            CPL_LSBPTR16(&nSubObj);
            memcpy(&nCoords, psRecordBuf + nOffset + 2, 2);
            CPL_LSBPTR16(&nCoords);

            nOffset +=4;

            poPT = new OGRPoint();

            nOffset += TranslatePoint(poPT, header, psRecordBuf + nOffset, 
                header.nGeometryLength - nOffset);
            poMPT->addGeometryDirectly( poPT );

            if( nOffset + 1 > header.nGeometryLength )
            {
                return poFeature;
            }

            GByte nTextL;
            memcpy(&nTextL, psRecordBuf + nOffset, 1);
            nOffset += 1;
            if( nOffset + nTextL > header.nGeometryLength )
            {
                return poFeature;
            }

            nTextL += 1;

            soText += " " + SXFFile::ReadSXFString(psRecordBuf + nOffset, 
                nTextL, header.osEncoding.c_str());

            nOffset += nTextL;
        }

        poFeature->SetGeometryDirectly( poMPT );
    }

    poFeature->SetField("TEXT", soText);
    return poFeature;
}

const char *OGRSXFLayer::GetFIDColumn()
{
    return osFIDColumn.c_str();
}

GByte OGRSXFLayer::GetId() const 
{ 
    return nLayerID; 
}
