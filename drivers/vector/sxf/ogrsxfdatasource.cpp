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

#include "cpl_conv.h"
#include "ogr_sxf.h"
#include "cpl_string.h"
#include "cpl_multiproc.h"

#include <math.h>
#include <map>
#include <string>

CPL_CVSID("$Id$")

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
    for( size_t i = 0; i < poLayers.size(); i++ )
        delete poLayers[i];
    poLayers.clear();
}

/************************************************************************/
/*                           TestCapability()                           */
/************************************************************************/

int OGRSXFDataSource::TestCapability( CPL_UNUSED const char * pszCap )
{
    return FALSE;
}

/************************************************************************/
/*                              GetLayer()                              */
/************************************************************************/

OGRLayer *OGRSXFDataSource::GetLayer( int iLayer )

{
    if ( iLayer < 0 || iLayer >= GetLayerCount() )
    {
        return nullptr;
    }
    return poLayers[iLayer];    
}

////////////////////////////////////////////////////////////////////////////
// GetName()
const char *OGRSXFDataSource::GetName()
{ 
    return pszName; 
}

////////////////////////////////////////////////////////////////////////////
// GetLayerCount()
int OGRSXFDataSource::GetLayerCount()
{ 
    return static_cast<int>(poLayers.size()); 
}

/************************************************************************/
/*                                Open()                                */
/************************************************************************/

int OGRSXFDataSource::Open(const char * pszFilename, bool bUpdateIn,
                           CSLConstList papszOpenOpts)
{
    // TODO: Add write support
    if( bUpdateIn )
    {
        return FALSE;
    }

    pszName = pszFilename;

    if( !oSXFFile.Open(pszName) )
    {
        return FALSE;
    }

    if( oSXFFile.Read(this, papszOpenOpts) != OGRERR_NONE )
    {
        oSXFFile.Close();
        return FALSE;
    }

    /*---------------- TRY READ THE RSC FILE HEADER  -----------------------*/
    CPLString osRSCFileName;
    const char* pszRSCFileName =
        CSLFetchNameValueDef(papszOpenOpts, "SXF_RSC_FILENAME",
                             CPLGetConfigOption("SXF_RSC_FILENAME", ""));
    if (pszRSCFileName != nullptr && CPLCheckForFile((char *)pszRSCFileName, nullptr) == TRUE)
    {
        osRSCFileName = pszRSCFileName;
    }

    if(osRSCFileName.empty())
    {
        pszRSCFileName = CPLResetExtension(pszFilename, "rsc");
        if (CPLCheckForFile((char *)pszRSCFileName, nullptr) == TRUE)
        {
            osRSCFileName = pszRSCFileName;
        }
    }

    if(osRSCFileName.empty())
    {
        pszRSCFileName = CPLResetExtension(pszFilename, "RSC");
        if (CPLCheckForFile((char *)pszRSCFileName, nullptr) == TRUE)
        {
            osRSCFileName = pszRSCFileName;
        }
    }

    // 1. Create layers from RSC file or create default set of layers from
    // gdal_data/default.rsc.

    if(osRSCFileName.empty())
    {
        pszRSCFileName = CPLFindFile( "gdal", "default.rsc" );
        if (nullptr != pszRSCFileName)
        {
            osRSCFileName = pszRSCFileName;
        }
        else
        {
            CPLDebug( "OGRSXFDataSource", "Default RSC file not found" );
        }
    }

    const char *pszIsNewBehavior =
        CSLFetchNameValueDef(papszOpenOpts,
                             "SXF_NEW_BEHAVIOR",
                             CPLGetConfigOption("SXF_NEW_BEHAVIOR", "NO"));
    bool bNewBehavior = CPLTestBool(pszIsNewBehavior);                        
    if (osRSCFileName.empty())
    {
        CPLError(CE_Warning, CPLE_None, "RSC file for %s not exist", pszFilename);
    }
    else
    {
        RSCFile oRSCFile;
        if( oRSCFile.Read(osRSCFileName, papszOpenOpts) == OGRERR_NONE )
        {
            // Fill layers
            auto mstLayers = oRSCFile.mstLayers;
            if( mstLayers.empty() )
            {
                // Create default set of layers
                CreateLayers(oRSCFile.GetDefaultLayers(), bNewBehavior);
            }
            else
            {
                CreateLayers(mstLayers, bNewBehavior);
            }            
        }
    }

    FillLayers(bNewBehavior);

    return TRUE;
}

void OGRSXFDataSource::FillLayers(bool bIsNewBehavior)
{
    GUInt32 nOffset = 0;
    if( oSXFFile.Version() == 3 )
    {
        nOffset = 256 + 44; // Passport + descriptor size
    }
    else if( oSXFFile.Version() == 4 )
    {
        nOffset = 400 + 52;
    }
    //2. Read all records (only classify code and offset) and add this to correspondence layer
    VSIFSeekL(oSXFFile.File(), nOffset, SEEK_SET);

    GIntBig nFID = 0;
    int nGroupId = 1;
    for( GUInt32 i = 0; i < oSXFFile.FeatureCount(); i++ )
    {
        GUInt32 nCurrentOffset = nOffset;
        bool bHasAttributes = false;
        int nAttributesSize = 0;
        SXFGeometryType eGeomType = SXF_GT_Unknown;
        GUInt32 nCode = 0;
        int nSubObjectCount = 0;
        if( oSXFFile.Version() == 3 )
        {
            SXFRecordHeaderV3 record;
            VSIFReadL(&record, sizeof(SXFRecordHeaderV3), 1, oSXFFile.File());

            CPL_LSBPTR32(&record.nClassifyCode);
            nCode = record.nClassifyCode;

            bHasAttributes = record.nHasSemantics;
            if( bHasAttributes )
            {
                CPL_LSBPTR32(&record.nFullLength);
                CPL_LSBPTR32(&record.nGeometryLength);
                nAttributesSize = record.nFullLength - 32 - record.nGeometryLength;
                if( nAttributesSize < 0 )
                {
                    bHasAttributes = false;
                    nAttributesSize = 0;
                }
                else 
                {
                    VSIFSeekL(oSXFFile.File(), record.nGeometryLength, SEEK_CUR);
                }
            }

            CPL_LSBPTR32(&record.nFullLength);
            nOffset += record.nFullLength;

            eGeomType =  SXFFile::CodeToGeometryType(record.nLocalizaton);
            CPL_LSBPTR16(&record.nSubObjectCount);
            nSubObjectCount = record.nSubObjectCount;
        }
        else if( oSXFFile.Version() == 4 )
        {
            SXFRecordHeaderV4 record;
            VSIFReadL(&record, sizeof(SXFRecordHeaderV4), 1, oSXFFile.File());

            CPL_LSBPTR32(&record.nClassifyCode);
            nCode = record.nClassifyCode;

            bHasAttributes = record.nHasSemantics;
            if( bHasAttributes )
            {
                CPL_LSBPTR32(&record.nFullLength);
                CPL_LSBPTR32(&record.nGeometryLength);
                nAttributesSize = record.nFullLength - 32 - record.nGeometryLength;
                if( nAttributesSize < 0 )
                {
                    bHasAttributes = false;
                    nAttributesSize = 0;
                }
                else 
                {
                    VSIFSeekL(oSXFFile.File(), record.nGeometryLength, SEEK_CUR);
                }
            }

            CPL_LSBPTR32(&record.nFullLength);
            nOffset += record.nFullLength;

            eGeomType =  SXFFile::CodeToGeometryType(record.nLocalizaton);
            CPL_LSBPTR16(&record.nSubObjectCount);
            nSubObjectCount = record.nSubObjectCount;
        }

        for( auto poLayer : poLayers )
        {
            auto pOGRSXFLayer = static_cast<OGRSXFLayer*>(poLayer);
            if( pOGRSXFLayer && pOGRSXFLayer->AddRecord(nFID++, nCode,
                nCurrentOffset, bHasAttributes, nAttributesSize,
                nSubObjectCount == 0 ? 0 : nGroupId++,
                nSubObjectCount == 0 ? 0 : 1 ) )
            {
                if( eGeomType == SXF_GT_Text )
                {
                    for(int i = 0; i < nSubObjectCount; i++)
                    {
                        pOGRSXFLayer->AddRecord(nFID++, nCode,
                            nCurrentOffset, bHasAttributes, nAttributesSize, nGroupId, i + 2);
                    }
                }
                break;
            }
        }
        
        VSIFSeekL(oSXFFile.File(), nOffset, SEEK_SET);

        // Prevent reading out of file size in case of broken SXF file
        if( VSIFEofL(oSXFFile.File()) == TRUE )
        {
            break;
        }
    }

    if( !bIsNewBehavior )
    {
        //3. Delete empty layers
        for( size_t i = 0; i < poLayers.size(); i++ )
        {
            OGRSXFLayer* pOGRSXFLayer = static_cast<OGRSXFLayer*>(poLayers[i]);
            if( pOGRSXFLayer ) 
            {
                if( pOGRSXFLayer->GetFeatureCount() == 0 )
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

OGRSXFLayer *OGRSXFDataSource::GetLayerById(GByte nID)
{
    for( size_t i = 0; i < poLayers.size(); i++ )
    {
        OGRSXFLayer *pOGRSXFLayer = static_cast<OGRSXFLayer*>(poLayers[i]);
        if( pOGRSXFLayer && pOGRSXFLayer->GetId() == nID )
        {
            return pOGRSXFLayer;
        }
    }
    return nullptr;
}

void OGRSXFDataSource::CreateLayers(const std::map<GByte, SXFLayerDefn>& mstLayers, 
    bool bIsNewBehavior)
{
    for( const auto &layer : mstLayers )
    {
        OGRSXFLayer *poLayer = new OGRSXFLayer(&oSXFFile, layer.first,
            layer.second.osName.c_str(), layer.second.astFields, bIsNewBehavior);
        for( const auto &code : layer.second.astCodes )
        {
            poLayer->AddClassifyCode(code.nCode, code.osName);
        }    
        poLayers.emplace_back(poLayer);
    }

    poLayers.emplace_back(new OGRSXFLayer(&oSXFFile, 255, 
        "Not_Classified", std::vector<SXFField>(), bIsNewBehavior));
}
