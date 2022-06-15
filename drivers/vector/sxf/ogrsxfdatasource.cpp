/******************************************************************************
 *
 * Project:  SXF Translator
 * Purpose:  Definition of classes for OGR SXF Datasource.
 * Author:   Ben Ahmed Daho Ali, bidandou(at)yahoo(dot)fr
 *           Dmitry Baryshnikov, polimax@mail.ru
 *           Alexandr Lisovenko, alexander.lisovenko@gmail.com
 *
 ******************************************************************************
 * Copyright (c) 2011, Ben Ahmed Daho Ali
 * Copyright (c) 2013, NextGIS
 * Copyright (c) 2014, Even Rouault <even dot rouault at spatialys.com>
 * Copyright (c) 2019-2020, NextGIS, <info@nextgis.com>
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

#include <math.h>

CPL_CVSID("$Id$")

static bool CheckFileExists(const char *pszPath)
{
    VSIStatBufL sStat;
    if (VSIStatExL(pszPath, &sStat,
        VSI_STAT_EXISTS_FLAG | VSI_STAT_NATURE_FLAG) == 0 &&
        VSI_ISREG(sStat.st_mode))
    {
        return true;
    }
    return false;
}

/************************************************************************/
/*                         OGRSXFDataSource()                           */
/************************************************************************/
OGRSXFDataSource::OGRSXFDataSource()
{
}

/************************************************************************/
/*                          ~OGRSXFDataSource()                         */
/************************************************************************/
OGRSXFDataSource::~OGRSXFDataSource()
{
    FlushCache();

    for (auto poLayer : poLayers)
    {
        delete poLayer;
    }
    poLayers.clear();

    if (poSpatialRef)
    {
        poSpatialRef->Release();
    }
}

/************************************************************************/
/*                           TestCapability()                           */
/************************************************************************/
int OGRSXFDataSource::TestCapability( const char *pszCap )
{
    if (EQUAL(pszCap, ODsCCreateLayer))
    {
        return GetAccess() == GA_Update ? TRUE : FALSE;
    }
    else if (EQUAL(pszCap, ODsCDeleteLayer))
    {
        return GetAccess() == GA_Update ? TRUE : FALSE;
    }
    else if (EQUAL(pszCap, ODsCCreateGeomFieldAfterCreateLayer))
    {
        return GetAccess() == GA_Update ? TRUE : FALSE;
    }
    else if (EQUAL(pszCap, ODsCRandomLayerWrite))
    {
        return GetAccess() == GA_Update ? TRUE : FALSE;
    }
    else if (EQUAL(pszCap, ODsCRandomLayerRead))
    {
        return TRUE;
    }
    return FALSE;
}

/************************************************************************/
/*                              GetLayer()                              */
/************************************************************************/
OGRLayer *OGRSXFDataSource::GetLayer( int iLayer )
{
    if (iLayer < 0 || iLayer >= GetLayerCount())
    {
        return nullptr;
    }
    return poLayers[iLayer];    
}

////////////////////////////////////////////////////////////////////////////
// GetName
const char *OGRSXFDataSource::GetName()
{
    if (aosFileList.empty())
    {
        return "";
    }
    return aosFileList[0];
}

////////////////////////////////////////////////////////////////////////////
// GetLayerCount
int OGRSXFDataSource::GetLayerCount()
{ 
    return static_cast<int>(poLayers.size()); 
}

////////////////////////////////////////////////////////////////////////////
// Create
int OGRSXFDataSource::Create(const char *pszFilename, 
    CSLConstList papszOpenOpts)
{
    eAccess = GA_Update;
    aosFileList.Clear();
    aosFileList.AddString(pszFilename);

    osEncoding =
        CSLFetchNameValueDef(papszOpenOpts, "SXF_ENCODING",
            CPLGetConfigOption("SXF_ENCODING", "CP1251"));

    const char *pszWriteRSC =
        CSLFetchNameValueDef(papszOpenOpts,
            "SXF_WRITE_RSC",
            CPLGetConfigOption("SXF_WRITE_RSC", "YES"));
    bWriteRSC = CPLTestBool(pszWriteRSC);

    auto mapName = CSLFetchNameValueDef(papszOpenOpts, "SXF_MAP_NAME",
        CPLGetConfigOption("SXF_MAP_NAME", ""));
    SetMetadataItem("SHEET_NAME", mapName);

    auto sheet = CSLFetchNameValueDef(papszOpenOpts, "SXF_SHEET_KEY",
        CPLGetConfigOption("SXF_SHEET_KEY", ""));
    SetMetadataItem("SHEET", sheet);

    auto scale = CSLFetchNameValueDef(papszOpenOpts, "SXF_MAP_SCALE",
        CPLGetConfigOption("SXF_MAP_SCALE", "1000000"));
    SetMetadataItem("SCALE", CPLSPrintf("1 : %s", scale));

    return TRUE;
}

/************************************************************************/
/*                                Open()                                */
/************************************************************************/
int OGRSXFDataSource::Open(const char *pszFilename, bool bUpdateIn,
    CSLConstList papszOpenOpts)
{
    eAccess = bUpdateIn ? GA_Update : GA_ReadOnly;
    aosFileList.Clear();
    aosFileList.AddString(pszFilename);

    osEncoding =
        CSLFetchNameValueDef(papszOpenOpts, "SXF_ENCODING",
            CPLGetConfigOption("SXF_ENCODING", ""));

    SXFFile oSXFFile;
    if (!oSXFFile.Open(aosFileList[0], true, osEncoding))
    {
        return FALSE;
    }

    if (!oSXFFile.Read(this, papszOpenOpts))
    {
        oSXFFile.Close();
        return FALSE;
    }

    if (!oSXFFile.CheckSum())
    {
        CPLError(CE_Warning, CPLE_None, "Checksum failed");
    }

    const char *pszWriteRSC =
        CSLFetchNameValueDef(papszOpenOpts,
            "SXF_WRITE_RSC",
            CPLGetConfigOption("SXF_WRITE_RSC", "YES"));
    bWriteRSC = CPLTestBool(pszWriteRSC);

    oExtent = oSXFFile.Extent();
    osEncoding = oSXFFile.Encoding();
    // Set Spatial reference
    SetSpatialRef(oSXFFile.SpatialRef());

    /*---------------- TRY READ THE RSC FILE HEADER  -----------------------*/
    CPLString osRSCFileName;
    const char* pszRSCFileName =
        CSLFetchNameValueDef(papszOpenOpts, "SXF_RSC_FILENAME",
                             CPLGetConfigOption("SXF_RSC_FILENAME", ""));
    if (pszRSCFileName != nullptr && 
        CPLCheckForFile((char *)pszRSCFileName, nullptr) == TRUE)
    {
        osRSCFileName = pszRSCFileName;
    }

    if (osRSCFileName.empty())
    {
        pszRSCFileName = CPLResetExtension(pszFilename, "rsc");
        if (CheckFileExists(pszRSCFileName))
        {
            osRSCFileName = pszRSCFileName;
        }
    }

    if (osRSCFileName.empty())
    {
        pszRSCFileName = CPLResetExtension(pszFilename, "RSC");
        if (CheckFileExists(pszRSCFileName))
        {
            osRSCFileName = pszRSCFileName;
        }
    }

    // 1. Create layers from RSC file or create default set of layers from
    // gdal_data/default.rsc.

    if (osRSCFileName.empty())
    {
        pszRSCFileName = CPLFindFile( "gdal", "default.rsc" );
        if (nullptr != pszRSCFileName)
        {
            osRSCFileName = pszRSCFileName;
        }
        else
        {
            CPLDebug( "SXF", "Default RSC file not found" );
        }
    }
    else
    {
        aosFileList.AddString(osRSCFileName);
    }

    const char *pszIsNewBehavior =
        CSLFetchNameValueDef(papszOpenOpts,
                             "SXF_NEW_BEHAVIOR",
                             CPLGetConfigOption("SXF_NEW_BEHAVIOR", "NO"));
    bool bNewBehavior = CPLTestBool(pszIsNewBehavior);  
    RSCFile oRSCFile;
    if (osRSCFileName.empty())
    {
        CPLError(CE_Warning, CPLE_None, "RSC file for %s not exist", pszFilename);
    }
    else
    {
        oRSCFile.Read(osRSCFileName, papszOpenOpts); // If read failed we get default layers and codes
    }
    auto mstLayers = oRSCFile.GetLayers();
    CreateLayers(oSXFFile.Extent(), mstLayers, bNewBehavior);
    FillLayers(oSXFFile, bNewBehavior);

    return TRUE;
}

void OGRSXFDataSource::FillLayers(const SXFFile &oSXF, bool bIsNewBehavior)
{
    GUInt32 nOffset = 0;
    if (oSXF.Version() == 3)
    {
        nOffset = 256 + 44; // Passport + descriptor size
    }
    else if (oSXF.Version() == 4)
    {
        nOffset = 400 + 52;
    }
    // 2. Read all records (only classify code and offset) and add this to 
    // correspondence layer
    VSIFSeekL(oSXF.File(), nOffset, SEEK_SET);

    GIntBig nFID = 0;
    int nGroupId = 0;
    for (GUInt32 i = 0; i < oSXF.FeatureCount(); i++)
    {
        GUInt32 nCurrentOffset = nOffset;
        SXFGeometryType eGeomType = SXF_GT_Unknown;
        GUInt32 nCode = 0;
        int nSubObjectCount = 0;
        if (oSXF.Version() == 3)
        {
            SXFRecordHeaderV3 record;
            VSIFReadL(&record, sizeof(SXFRecordHeaderV3), 1, oSXF.File());

            CPL_LSBPTR32(&record.nClassifyCode);
            nCode = record.nClassifyCode;
            eGeomType =  SXFFile::CodeToGeometryType(record.nLocalizaton);
            CPL_LSBPTR16(&record.nSubObjectCount);
            nSubObjectCount = record.nSubObjectCount;

            CPL_LSBPTR32(&record.nFullLength);
            nOffset += record.nFullLength;
        }
        else if (oSXF.Version() == 4)
        {
            SXFRecordHeaderV4 record;
            VSIFReadL(&record, sizeof(SXFRecordHeaderV4), 1, oSXF.File());

            CPL_LSBPTR32(&record.nClassifyCode);
            nCode = record.nClassifyCode;
            eGeomType =  SXFFile::CodeToGeometryType(record.nLocalizaton);
            CPL_LSBPTR16(&record.nSubObjectCount);
            nSubObjectCount = record.nSubObjectCount;

            CPL_LSBPTR32(&record.nFullLength);
            nOffset += record.nFullLength;
        }
        
        auto osStrCode = SXFFile::ToStringCode(eGeomType, nCode);
        for (auto poLayer : poLayers)
        {
            auto pOGRSXFLayer = static_cast<OGRSXFLayer*>(poLayer);
            if (pOGRSXFLayer && pOGRSXFLayer->AddRecord(nFID++, osStrCode,
                oSXF, nCurrentOffset, nSubObjectCount == 0 ? 0 : ++nGroupId,
                nSubObjectCount == 0 ? 0 : 1 ))
            {
                if (eGeomType == SXF_GT_Text)
                {
                    for (int j = 0; j < nSubObjectCount; j++)
                    {
                        pOGRSXFLayer->AddRecord(nFID++, osStrCode,
                            oSXF, nCurrentOffset, nGroupId, j + 2);
                    }
                }
                break;
            }
        }
        
        // Clear changes flag as we just loaded features from file without any modifications
        bHasChanges = false;
        VSIFSeekL(oSXF.File(), nOffset, SEEK_SET);

        // Prevent reading out of file size in case of broken SXF file
        if (VSIFEofL(oSXF.File()) == TRUE)
        {
            break;
        }
    }

    if (!bIsNewBehavior)
    {
        //3. Delete empty layers
        for (size_t i = 0; i < poLayers.size(); i++)
        {
            OGRSXFLayer *pOGRSXFLayer = static_cast<OGRSXFLayer*>(poLayers[i]);
            if (pOGRSXFLayer) 
            {
                if (pOGRSXFLayer->GetFeatureCount(FALSE) == 0)
                {
                    delete pOGRSXFLayer;
                    poLayers.erase(poLayers.begin() + i);
                }
                else
                {
                    pOGRSXFLayer->ResetReading();
                }
            }
            else 
            {
                poLayers.erase(poLayers.begin() + i);
            }
        }
    }
}

void OGRSXFDataSource::CreateLayers(const OGREnvelope &oEnv, 
    const std::map<GByte, SXFLayerDefn>& mstLayers, bool bIsNewBehavior)
{
    for (const auto &layer : mstLayers)
    {
        OGRSXFLayer *poLayer = new OGRSXFLayer(this,
            layer.second, bIsNewBehavior); 
        poLayers.emplace_back(poLayer);
    }

    SXFLayerDefn oNotClassify(EXTRA_ID * poLayers.size() + 1, "Not_Classified");
    poLayers.emplace_back(new OGRSXFLayer(this, oNotClassify, bIsNewBehavior));
}

void OGRSXFDataSource::FlushCache(bool bAtClosing)
{
    if (!bHasChanges)
    {
        return;
    }

    auto mnClassMap = GenerateSXFClassMap();
    
    SXFFile oSXFFile;
    if (!oSXFFile.Open(aosFileList[0], false, osEncoding))
    {
        return;
    }
    
    // Write header 
    if (!oSXFFile.Write(this))
    {
        oSXFFile.Close();
        return;
    }


    // Write features
    int nTotalFeatureCount = 0;
    
    for (auto poLayer : poLayers)
    {
        auto layer = static_cast<OGRSXFLayer*>(poLayer);
        nTotalFeatureCount += layer->Write(oSXFFile, mnClassMap);
    }

    oSXFFile.WriteTotalFeatureCount(nTotalFeatureCount);    
    oSXFFile.WriteCheckSum();

    if (bWriteRSC)
    {
        auto pszRSCFileName = CPLResetExtension(aosFileList[0], "rsc");

        RSCFile oRSCFile;
        if (!oRSCFile.Write(pszRSCFileName, this, osEncoding, mnClassMap))
        {
            return;
        }

        bool bHasName = false;
        for (int i = 0; i < aosFileList.size(); i++)
        {
            if (EQUAL(aosFileList[i], pszRSCFileName))
            {
                bHasName = true;
                break;
            }
        }
        if (!bHasName)
        {
            aosFileList.AddString(pszRSCFileName);
        }
    }
}

const OGRSpatialReference *OGRSXFDataSource::GetSpatialRef() const
{
    return poSpatialRef;
}

CPLErr OGRSXFDataSource::SetSpatialRef(const OGRSpatialReference *poSRS)
{
    if (poSpatialRef)
    {
        poSpatialRef->Release();
    }
    poSpatialRef = poSRS->Clone();
    return CE_None;
}

char **OGRSXFDataSource::GetFileList(void)
{
    return aosFileList.StealList();
}

OGRLayer *OGRSXFDataSource::ICreateLayer(const char *pszName,
    OGRSpatialReference *poSRS, CPL_UNUSED OGRwkbGeometryType eGType,
    char **papszOptions)
{
    // All layers must have same Spatial Reference.
    if (poSRS != nullptr && poSpatialRef == nullptr)
    {
        SetSpatialRef(poSRS);
    }
    const char *pszIsNewBehavior =
        CSLFetchNameValueDef(papszOptions, "SXF_NEW_BEHAVIOR",
            CPLGetConfigOption("SXF_NEW_BEHAVIOR", "NO"));
    bool bNewBehavior = CPLTestBool(pszIsNewBehavior);    
    SXFLayerDefn oSXFLayerDefn(EXTRA_ID * poLayers.size() + 1, pszName);
    auto poNewLayer = new OGRSXFLayer(this, oSXFLayerDefn, bNewBehavior);
    poLayers.emplace_back(poNewLayer);
    SetHasChanges();
    return poNewLayer;
}

OGRErr OGRSXFDataSource::DeleteLayer(int iLayer)
{
    if (iLayer < 0 || iLayer >= GetLayerCount())
    {
        return OGRERR_FAILURE;
    }
    delete poLayers[iLayer];
    poLayers.erase(poLayers.begin() + iLayer);
    SetHasChanges();
    return OGRERR_NONE;
}

OGRErr OGRSXFDataSource::GetExtent(OGREnvelope *psExtent) const
{
    psExtent->MinX = oExtent.MinX;
    psExtent->MinY = oExtent.MinY;
    psExtent->MaxX = oExtent.MaxX;
    psExtent->MaxY = oExtent.MaxY;
    return OGRERR_NONE;
}

void OGRSXFDataSource::UpdateExtent(const OGREnvelope &env)
{
    oExtent.Merge(env);
    SetHasChanges();
}

std::map<std::string, int> OGRSXFDataSource::GenerateSXFClassMap() const
{
    // For fields with names differs from standard TEXT, OP, etc. and not SC_<int> 
    // we need to create unique numeric identifiers. Fields in different layers with
    // same names but different types must have different identifiers.
    // Form name with following template: field name:field type

    int nCounter = 5000;
    std::map<std::string, int> mnSXFClassMap;
    for (auto poLayer : poLayers)
    {
        auto poDef = poLayer->GetLayerDefn();
        for (int i = 0; i < poDef->GetFieldCount(); i++)
        {
            auto poFld = poDef->GetFieldDefn(i);
            if (!OGRSXFLayer::IsFieldNameHasCode(poFld->GetNameRef()))
            {
                std::string key = OGRSXFLayer::CreateFieldKey(poFld);
                if (mnSXFClassMap.find(key) == mnSXFClassMap.end())
                {
                    mnSXFClassMap[key] = nCounter++;
                }
            }
        }
    }
    return mnSXFClassMap;
}

void OGRSXFDataSource::SetHasChanges()
{
    if (GetAccess() == GA_Update)
    {
        bHasChanges = true;
    }
}

std::string OGRSXFDataSource::Encoding() const
{
    return osEncoding;
}