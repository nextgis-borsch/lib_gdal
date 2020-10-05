/******************************************************************************
 *
 * Project:  SXF Driver
 * Purpose:  RSC file.
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

#include <memory>

/************************************************************************/
/*                            RSCInfo                                   */
/************************************************************************/

/*
    RSC File record
*/
typedef struct {
    GUInt32 nOffset;      // RSC Section offset in bytes from the beginning of the RSC file
    GUInt32 nLength;      // RSC Section record length
    GUInt32 nRecordCount; // Count of records in the section
} RSCSection;

/*
    RSC File header
*/
typedef struct {
    GUInt32 nEncoding;
    GUInt32 nFileState;
    GUInt32 nFileModState;
    GUInt32 nLang;              // 1 - en, 2 - ru
    GUInt32 nNextID;
    GByte date[8];
    GByte szMapType[32];
    GByte szClassifyName[32];
    GByte szClassifyCode[8];
    GUInt32 nScale;
    GUInt32 nScalesRange;       // 1 for scales 1:1 to 1:10000, for others - 0
    RSCSection Objects;
    RSCSection Semantic;
    RSCSection ClassifySemantic;
    RSCSection DefaultsSemantic;
    RSCSection PossibleSemantic;
    RSCSection Layers;
    RSCSection Domains;
    RSCSection Parameters;
    RSCSection Print;
    RSCSection Palettes;
    RSCSection Fonts;
    RSCSection Libs;
    RSCSection ImageParams;
    RSCSection Tables;
    GByte nFlagKeysAsCodes;
    GByte nFlagPaletteMods;
    GByte Reserved[30];
    GUInt32 nFontEnc;
    GUInt32 nColorsInPalette;
} RSCHeader;

typedef struct {
    GUInt32 nLength;
    GByte szName[32];
    GByte szShortName[16];
    GByte nNo;
    // Unused
    // GByte nDrawOrder; // 0 -255. Less number will draw earlier
    // GUInt16 nSemanticCount;
} RSCLayer;

typedef struct {
    GUInt32 nCode;
    GUInt16 nType;
    GByte bAllowMultiple;
    GByte bAllowAnythere;
    GByte szName[32];
} RSCSemantics;

enum RSCSemanticsType {
    RSC_SC_TEXT = 0,
    RSC_SC_REAL = 1,
    RSC_SC_FILE_NAME = 9,
    RSC_SC_BMP_NAME = 10,
    RSC_SC_OPE_NAME = 11,
    RSC_SC_LINK = 12,  // Map object identifier
    RSC_SC_PASSPORT_FILE = 13,
    RSC_SC_TXT_FILE = 14,
    RSC_SC_PCX_FILE = 15
};

typedef struct {
    GUInt32 nLength;
    GUInt32 nClassifyCode;
    GUInt32 nInternalCode;
    GUInt32 nIdCode;
    GByte szShortName[32];
    GByte szName[32];
    GByte nGeometryType; // Same as enum SXFGeometryType
    GByte nLayerId;
} RSCObject;

typedef struct {
    GUInt32 nLength;
    GUInt32 nObjectCode;
    GByte nLocalization;
    GByte reserve[3];
    GUInt16 nMandatorySemCount;
    GUInt16 nPossibleSemCount;
} RSCObjectSemantics;

/************************************************************************/
/*                             RSCFile                                  */
/************************************************************************/

RSCFile::RSCFile()
{

}

RSCFile::~RSCFile()
{

}

static std::string GetName(const char *pszName, int nFontEncoding)
{
    if( pszName[0] == 0 )
    {    
        return "Unnamed";
    }
    else if( nFontEncoding == 125 )
    {
        char *pszRecoded = CPLRecode(pszName, "KOI8-R", CPL_ENC_UTF8);
        std::string out(pszRecoded);
        CPLFree(pszRecoded);
        return out;
    }
    else if( nFontEncoding == 126 )
    {
        char *pszRecoded = CPLRecode(pszName, "CP1251", CPL_ENC_UTF8);
        std::string out(pszRecoded);
        CPLFree(pszRecoded);
        return out;
    }
    return std::string(pszName);
}

OGRErr RSCFile::Read(const std::string &osPath, CSLConstList papszOpenOpts)
{
    mstLayers.clear();

    auto fpRSC = 
        std::shared_ptr<VSILFILE>(VSIFOpenL(osPath.c_str(), "rb"), VSIFCloseL);
    if (fpRSC == nullptr)
    {
        CPLError(CE_Warning, CPLE_OpenFailed, "RSC file %s open failed",
                    osPath.c_str());
        return OGRERR_FAILURE;
    }
    
    CPLDebug( "OGRSXFDataSource", "RSC Filename: %s", osPath.c_str() );

        // Read header
    Header stRSCFileHeader;
    size_t nObjectsRead =
        VSIFReadL(&stRSCFileHeader, sizeof(Header), 1, fpRSC.get());

    if (nObjectsRead != 1)
    {
        CPLError(CE_Failure, CPLE_None, "RSC header read failed");
        return OGRERR_FAILURE;
    }

    // Unused CPL_LSBPTR32(&stRSCFileHeader.nLength);

    // Check version
    GByte ver[4];
    VSIFReadL(&ver, 4, 1, fpRSC.get());
    int nVersion = ver[1];

    if ( nVersion != 7 )
    {
        CPLError(CE_Failure, CPLE_NotSupported , "RSC File version %d not supported", nVersion);
        return OGRERR_FAILURE;
    }

    RSCHeader stRSCFileHeaderEx;
    nObjectsRead = 
        VSIFReadL(&stRSCFileHeaderEx, sizeof(RSCHeader), 1, fpRSC.get());

    if (nObjectsRead != 1)
    {
        CPLError(CE_Warning, CPLE_None, "RSC head read failed");
        return OGRERR_FAILURE;
    }

    CPL_LSBPTR32(&stRSCFileHeaderEx.nEncoding);
    CPL_LSBPTR32(&stRSCFileHeaderEx.nFileState);
    CPL_LSBPTR32(&stRSCFileHeaderEx.nFileModState);
    CPL_LSBPTR32(&stRSCFileHeaderEx.nLang);
    CPL_LSBPTR32(&stRSCFileHeaderEx.nNextID);
    CPL_LSBPTR32(&stRSCFileHeaderEx.nScale);

#define SWAP_SECTION(x) \
    CPL_LSBPTR32(&(x.nOffset)); \
    CPL_LSBPTR32(&(x.nLength)); \
    CPL_LSBPTR32(&(x.nRecordCount));

    SWAP_SECTION(stRSCFileHeaderEx.Objects);
    SWAP_SECTION(stRSCFileHeaderEx.Semantic);
    SWAP_SECTION(stRSCFileHeaderEx.ClassifySemantic);
    SWAP_SECTION(stRSCFileHeaderEx.DefaultsSemantic);
    SWAP_SECTION(stRSCFileHeaderEx.PossibleSemantic);
    SWAP_SECTION(stRSCFileHeaderEx.Layers);
    SWAP_SECTION(stRSCFileHeaderEx.Domains);
    SWAP_SECTION(stRSCFileHeaderEx.Parameters);
    SWAP_SECTION(stRSCFileHeaderEx.Print);
    SWAP_SECTION(stRSCFileHeaderEx.Palettes);
    SWAP_SECTION(stRSCFileHeaderEx.Fonts);
    SWAP_SECTION(stRSCFileHeaderEx.Libs);
    SWAP_SECTION(stRSCFileHeaderEx.ImageParams);
    SWAP_SECTION(stRSCFileHeaderEx.Tables);

    CPL_LSBPTR32(&stRSCFileHeaderEx.nFontEnc);
    CPL_LSBPTR32(&stRSCFileHeaderEx.nColorsInPalette);

    bool bIsNewBehavior = CPLTestBool(
        CSLFetchNameValueDef(papszOpenOpts,
                             "SXF_NEW_BEHAVIOR",
                             CPLGetConfigOption("SXF_NEW_BEHAVIOR", "NO")));

    bool bLayerFullName = CPLTestBool(
        CSLFetchNameValueDef(papszOpenOpts, "SXF_LAYER_FULLNAME",
            CPLGetConfigOption("SXF_LAYER_FULLNAME", "NO")));
    
    std::map<GUInt32, SXFField> mstSemantics;
    if( bIsNewBehavior )
    {
        // Read all semantics
        CPLDebug("SXF", "Read %d semantics from RSC", stRSCFileHeaderEx.Semantic.nRecordCount);
        vsi_l_offset nOffset = stRSCFileHeaderEx.Semantic.nOffset;
        VSIFSeekL(fpRSC.get(), nOffset, SEEK_SET);
        for( GUInt32 i = 0; i < stRSCFileHeaderEx.Semantic.nRecordCount; i++ )
        {
            RSCSemantics stSemantics;
            VSIFReadL(&stSemantics, sizeof(RSCSemantics), 1, fpRSC.get());
            CPL_LSBPTR32(&stSemantics.nCode);
            CPL_LSBPTR16(&stSemantics.nType);

            auto alias = 
                GetName(reinterpret_cast<const char*>(stSemantics.szName), 
                    stRSCFileHeaderEx.nFontEnc);
            RSCSemanticsType eType = RSC_SC_TEXT;
            if( stSemantics.nType == 1 || (stSemantics.nType > 8 && 
                stSemantics.nType < 16) )
            {
                eType = static_cast<RSCSemanticsType>(stSemantics.nType);
            }

            bool bAllowMultiple = stSemantics.bAllowMultiple == 1;
            OGRFieldType eFieldType;
            switch (eType)
            {
            case RSC_SC_LINK:
                if( bAllowMultiple )
                {
                    eFieldType = OFTIntegerList;
                }
                else
                {
                    eFieldType = OFTInteger;
                }
                break;
            case RSC_SC_REAL: 
                if( bAllowMultiple )
                {
                    eFieldType = OFTRealList;
                }
                else
                {
                    eFieldType = OFTReal;
                }
                break;            
            default:
                if( bAllowMultiple )
                {
                    eFieldType = OFTStringList;
                }
                else
                {
                    eFieldType = OFTString;
                }
                break;
            }

            std::string name = CPLSPrintf("SC_%d", stSemantics.nCode);

            mstSemantics[stSemantics.nCode] = 
                {stSemantics.nCode, name, alias, eFieldType};

            nOffset += 84L;
            VSIFSeekL(fpRSC.get(), nOffset, SEEK_SET);   
        }
    }

    // Read classify code -> semantics[]
    CPLDebug("SXF", "Read %d classify code -> semantics[] from RSC", stRSCFileHeaderEx.PossibleSemantic.nRecordCount);
    vsi_l_offset nOffset = stRSCFileHeaderEx.PossibleSemantic.nOffset;
    VSIFSeekL(fpRSC.get(), nOffset, SEEK_SET);

    std::map<GUInt32, std::vector<GUInt32>> codeSemMap;

    for( GUInt32 i = 0; i < stRSCFileHeaderEx.PossibleSemantic.nRecordCount; i++ )
    {
        RSCObjectSemantics stRSCOS;
        VSIFReadL(&stRSCOS, sizeof(RSCObjectSemantics), 1, fpRSC.get());
        CPL_LSBPTR32(&stRSCOS.nLength);
        CPL_LSBPTR32(&stRSCOS.nObjectCode);
        CPL_LSBPTR16(&stRSCOS.nMandatorySemCount);
        CPL_LSBPTR16(&stRSCOS.nPossibleSemCount);

        GUInt32 count = stRSCOS.nMandatorySemCount + stRSCOS.nPossibleSemCount;
        for( GUInt32 i = 0; i < count; i++ )
        {
            GUInt32 sc;
            VSIFReadL(&sc, 4, 1, fpRSC.get());
            CPL_LSBPTR32(&sc);
            codeSemMap[stRSCOS.nObjectCode].push_back(sc);
        }

        nOffset += stRSCOS.nLength;
        VSIFSeekL(fpRSC.get(), nOffset, SEEK_SET);
    }

    // Read layers
    CPLDebug("SXF", "Read %d layers from RSC", stRSCFileHeaderEx.Layers.nRecordCount);
    nOffset = stRSCFileHeaderEx.Layers.nOffset;
    VSIFSeekL(fpRSC.get(), nOffset, SEEK_SET);

    for( GUInt32 i = 0; i < stRSCFileHeaderEx.Layers.nRecordCount; i++ )
    {
        RSCLayer stLayer;
        VSIFReadL(&stLayer, sizeof(RSCLayer), 1, fpRSC.get());

        CPL_LSBPTR32(&stLayer.nLength);
        
        std::string name;
        if (bLayerFullName)
        {
            name = GetName(reinterpret_cast<const char*>(stLayer.szName), 
                stRSCFileHeaderEx.nFontEnc);
        }
        else
        {
            name = GetName(reinterpret_cast<const char*>(stLayer.szShortName), 
                stRSCFileHeaderEx.nFontEnc);
        }

        SXFLayerDefn defn;
        defn.osName = name;
        mstLayers[stLayer.nNo] = defn;

        nOffset += stLayer.nLength;
        VSIFSeekL(fpRSC.get(), nOffset, SEEK_SET);
    }

    // Read Objects
    CPLDebug("SXF", "Read %d objects from RSC", stRSCFileHeaderEx.Objects.nRecordCount);
    nOffset = stRSCFileHeaderEx.Objects.nOffset;
    VSIFSeekL(fpRSC.get(), nOffset, SEEK_SET);
    std::set<GUInt32> sUsedCodes;
    for( GUInt32 i = 0; i < stRSCFileHeaderEx.Objects.nRecordCount; i++ )
    {
        RSCObject stRSCObject;
        VSIFReadL(&stRSCObject, sizeof(RSCObject), 1, fpRSC.get());
        CPL_LSBPTR32(&stRSCObject.nLength);
        CPL_LSBPTR32(&stRSCObject.nClassifyCode);

        if( sUsedCodes.find(stRSCObject.nClassifyCode) == sUsedCodes.end() )
        {
            sUsedCodes.insert(stRSCObject.nClassifyCode);

            auto name = 
                GetName(reinterpret_cast<const char*>(stRSCObject.szName),
                    stRSCFileHeaderEx.nFontEnc);

            auto layer = mstLayers.find(stRSCObject.nLayerId);
            if ( layer != mstLayers.end() ) {
                SXFClassCode cc = {stRSCObject.nClassifyCode, name};
                layer->second.astCodes.emplace_back(cc);
                if( bIsNewBehavior )
                {
                    // Get semantics for classify code
                    auto semantics = codeSemMap[stRSCObject.nClassifyCode];

                    for( auto semantic : semantics )
                    {
                        bool bHasField = false;
                        for( const auto &field : layer->second.astFields ) {
                            if( field.nCode == semantic )
                            {
                                bHasField = true;
                                break;
                            }
                        }

                        if( !bHasField )
                        {
                            auto semanticsIt = mstSemantics.find(semantic); 
                            if( semanticsIt != mstSemantics.end() )
                            {
                                auto ss = mstSemantics[semantic];
                                layer->second.astFields.emplace_back(ss);
                            }
                        }
                    }
                }
            }
        }

        nOffset += stRSCObject.nLength;
        VSIFSeekL(fpRSC.get(), nOffset, SEEK_SET);
    }

    return OGRERR_NONE;
}

std::map<GByte, SXFLayerDefn> RSCFile::GetDefaultLayers()
{
    std::map<GByte, SXFLayerDefn> mstDefaultLayers;
    SXFLayerDefn defn;
    defn.osName = "SYSTEM";

    //default codes
    for( unsigned int i = 1000000001; i < 1000000015; i++ )
    {
        defn.astCodes.push_back({i, ""});
    }
    defn.astCodes.push_back({91000000, "SHEET FRAME"});
    mstDefaultLayers[0] = defn;
    return mstDefaultLayers;
}
