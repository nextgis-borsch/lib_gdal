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

#include "cpl_time.h"

#include <memory>

/************************************************************************/
/*                            RSCInfo                                   */
/************************************************************************/

constexpr int DEFAULT_RGB = 0x00B536AD; // rgb(181, 54, 173);
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
    GByte nEncoding[4];
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
    RSCSection Objects;            // OBJ
    RSCSection Semantic;        // SEM
    RSCSection ClassifySemantic;// CLS
    RSCSection DefaultsSemantic;// DEF
    RSCSection PossibleSemantic;// POS
    RSCSection Layers;            // SEG
    RSCSection Domains;            // LIM
    RSCSection Parameters;        // PAR
    RSCSection Print;            // PRN
    RSCSection Palettes;        // PAL
    RSCSection Fonts;            // TXT
    RSCSection Libs;            // IML
    RSCSection ImageParams;
    RSCSection Tables;            // TAB
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
    GByte nDrawOrder; // 0 - 255. Less number will draw earlier
    GUInt16 nSemanticCount;
    GUInt32 reserve;
} RSCLayer;

typedef struct {
    GUInt32 nCode;
    GUInt16 nType;
    GByte bAllowMultiple;
    GByte bAllowAnythere;
    GByte szName[32];
    GByte szShortName[16];
    GByte szMeasurementValue[8];
    GUInt16 nFieldSize; // 0 - 255
    GByte nPrecision; // Number of digits after decimal point
    GByte bIsComplex;
    GUInt32 nClassifyOffset;
    GUInt32 nClassifyCount;
    GUInt32 nClassifyDefaultsOffset;
    GUInt32 nClassifyDefaultsCount;

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

enum RSCGraphicsType {
    RSC_GT_LINE = 128,
    RSC_GT_DASHED_LINE = 129,
    RSC_GT_DOTED_LINE = 148,
    RSC_GT_SQUARE = 135,
    RSC_GT_HATCH_SQUARE = 153,
    RSC_GT_POINT = 143,
    RSC_GT_POINTED_SQUARE = 144,
    RSC_GT_ROUND = 140,
    RSC_GT_FILL = 154,
    RSC_GT_VECTOR = 149,
    RSC_GT_VECTOR_SQUARE = 155,
    RSC_GT_DECORATED_LINE = 157,
    RSC_GT_TEXT = 142,
    RSC_GT_USER_FONT = 152,
    RSC_GT_TEMLATE = 150,
    RSC_GT_TTF_SYM = 151,
    RSC_GT_GRAPTH_GROUP = 147,
    RSC_GT_DASHED_LINE_2 = 158,
    RSC_GT_IMG = 165,
    RSC_GT_USER_OBJ = 250
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
    GByte nScalable;
    GByte nLowViewLevel;
    GByte nHeighViewLevel;
    GByte nExtLocalization;
    GByte nDigitizeDirection;
    GByte nUseSemantics;
    GUInt16 nExtNo;
    GByte nLabelsCount;
    GByte nSqueeze;
    GByte nMaxZoom;
    GByte nMinZoom;
    GByte nUseBorders;
    GByte reseve;
    GUInt32 nLinkedText;
    GUInt32 nSemCode;
    GByte szPref[7];
    GByte nZeroes;
} RSCObject;

typedef struct {
    GUInt32 nLength;
    GUInt32 nObjectCode;
    GByte nLocalization;
    GByte reserve[3];
    GUInt16 nMandatorySemCount;
    GUInt16 nPossibleSemCount;
} RSCObjectSemantics;

typedef struct {
    GUInt32 nLength;
    GUInt32 nObjectCode;
    GByte nLocalization;
    GByte reserve[7];
    GUInt32 nSC1;
    GUInt16 nSC1LimCount;
    GUInt16 nSC1LimDefIndex;
    GUInt32 nSC2;
    GUInt16 nSC2LimCount;
    GUInt16 nSC2LimDefIndex;
} RSCLimitsRecord;

typedef struct {
    GUInt32 nColorsTablesOffset;
    GUInt32 nColorsTablesLength;
    GUInt32 nRecordCount;
    GByte reserve[60];
} RSCTables;

constexpr GByte CMYK[] = {
    0, 0, 0, 255, 
    170, 170, 0, 85, 
    170, 0, 170, 85,
    85, 0, 0, 170, 
    0, 170, 170, 85, 
    0, 85, 170, 0,
    0, 85, 170, 85, 
    0, 0, 0, 85, 
    0, 0, 0, 170,
    255, 85, 0, 0, 
    170, 0, 170, 0, 
    170, 170, 0, 0,
    0, 255, 170, 0, 
    0, 170, 0, 0, 
    0, 0, 170, 0,
    0, 0, 0, 0
};

constexpr GByte CROSS[] = {
    130, 0, 0, 0,
    68, 0, 0, 0,
    40, 0, 0, 0,
    16, 0, 0, 0,
    40, 0, 0, 0,
    68, 0, 0, 0,
    130, 0, 0, 0
};

typedef struct {
    GByte anPal[1024];
    GByte pszName[32];
} RSCPalette;

typedef struct {
    GByte pszName[32];
    GByte pszCode[32];
    GUInt32 nCode;
    GByte nTestSym;
    GByte nCodePage;
    GByte reserve[2];
} RSCFont;

typedef struct {
    GUInt32 nLength;
    GUInt16 nCode;
    GUInt16 nType;
} RSCParameter;

typedef struct 
{
    int nCode;
    int nType;
    std::string osName;
    int nFieldSize;
    int nPrecision;
    bool bAllowAnythere;
    bool bAllowMultiple;
} RSCSem;

typedef struct {
    int nCode;
    int nLoc;
    std::string osName;
    int nLayer;
    std::vector<RSCSem> aoSem;
} RSCObj;

static GUInt32 FileLength(VSILFILE *pofRSC)
{
    VSIFSeekL(pofRSC, 0, SEEK_END);
    return static_cast<GUInt32>(VSIFTellL(pofRSC));
}

static bool WriteLength(VSILFILE *pofRSC)
{
    GUInt32 nSize = FileLength(pofRSC);
    VSIFSeekL(pofRSC, 4, SEEK_SET);

    CPLDebug("SXF", "RSC Length is %d", nSize);
    return VSIFWriteL(&nSize, 4, 1, pofRSC) == 1;
}

static bool WriteNextID(GUInt32 nNextID, VSILFILE *pofRSC)
{
    VSIFSeekL(pofRSC, 28, SEEK_SET);

    return VSIFWriteL(&nNextID, 4, 1, pofRSC) == 1;
}

static void WriteRSCSection(vsi_l_offset pos, GUInt32 size, GUInt32 count,
    vsi_l_offset nOffset, VSILFILE *poFile)
{
    RSCSection stSect = RSCSection();
    stSect.nOffset = static_cast<GUInt32>(pos);
    stSect.nLength = size;
    stSect.nRecordCount = count;

    auto currentPos = VSIFTellL(poFile);
    VSIFSeekL(poFile, nOffset, SEEK_SET);
    VSIFWriteL(&stSect, sizeof(RSCSection), 1, poFile);
    VSIFSeekL(poFile, currentPos, SEEK_SET);
}

static GUInt32 WritePolygonDefaultParam(GUInt16 nCode, VSILFILE *poFile)
{
    RSCParameter stParam = RSCParameter();
    stParam.nLength = sizeof(RSCParameter) + 4;
    stParam.nType = 135;
    stParam.nCode = nCode;
    VSIFWriteL(&stParam, sizeof(RSCParameter), 1, poFile);
    GUInt32 nColor = static_cast<GUInt32>(DEFAULT_RGB);
    VSIFWriteL(&nColor, 4, 1, poFile);

    return stParam.nLength;
}

static GUInt32 WriteHatchPolygonDefaultParam(GUInt16 nCode, VSILFILE *poFile)
{    
    struct HatchPolygonParam {
        GUInt32 nLength;
        GUInt32 nAngle;
        GUInt32 nHatchStep;
        GUInt32 nType;
        GUInt32 nColor;
        GUInt32 nWidth;
    };

    RSCParameter stParam = RSCParameter();
    stParam.nLength = sizeof(RSCParameter) + sizeof(struct HatchPolygonParam);
    stParam.nType = 153;
    stParam.nCode = nCode;
    VSIFWriteL(&stParam, sizeof(RSCParameter), 1, poFile);

    struct HatchPolygonParam stHatchParam = {};
    stHatchParam.nLength = sizeof(struct HatchPolygonParam);
    stHatchParam.nAngle = 45;
    stHatchParam.nHatchStep = 5 * 250;
    stHatchParam.nType = 128;
    stHatchParam.nWidth = 1 * 250;
    stHatchParam.nColor = static_cast<GUInt32>(DEFAULT_RGB);

    VSIFWriteL(&stHatchParam, sizeof(struct HatchPolygonParam), 1, poFile);

    return stParam.nLength;
}

static GUInt32 WritePointDefaultParam(GUInt16 nCode, VSILFILE *poFile)
{    
    struct PointParam {
        GUInt32 nLength;
        GUInt32 nColorCount;
        GUInt32 nSize;
        GUInt32 nAnchorX;
        GUInt32 nAnchorY;
    };

    struct ColorMask {
        GUInt32 nColor;
        GByte anMask[128];
    };

    RSCParameter stParam = RSCParameter();
    stParam.nLength = sizeof(RSCParameter) + sizeof(struct PointParam) + 
        sizeof(struct ColorMask);
    stParam.nType = 143;
    stParam.nCode = nCode;
    VSIFWriteL(&stParam, sizeof(RSCParameter), 1, poFile);

    struct PointParam stTP = {
        sizeof(struct PointParam) + sizeof(struct ColorMask),
        1, 8000, 750, 750
    };
    VSIFWriteL(&stTP, sizeof(struct PointParam), 1, poFile);

    struct ColorMask stTM = {};
    stTM.nColor = DEFAULT_RGB;
    memcpy(stTM.anMask, CROSS, sizeof(CROSS));

    VSIFWriteL(&stTM, sizeof(struct ColorMask), 1, poFile);

    return stParam.nLength;
}

static GUInt32 WriteTextDefaultParam(GUInt16 nCode, VSILFILE *poFile)
{    
    struct TextParam {
        GUInt32 nColor;
        GUInt32 nBkColor; 
        GUInt32 nHeight;
        GUInt32 nThicknes; 
        GUInt16 nCenter;
        GUInt16 nReserve;
        GByte nSymWidth;
        GByte nOrientation;
        GByte nItalic;
        GByte nUnderline;
        GByte nStrike;
        GByte nFontIndex;
        GByte nCodePage;
        GByte nScaled;
    };

    RSCParameter stParam = RSCParameter();
    stParam.nLength = sizeof(RSCParameter) + sizeof(struct TextParam);
    stParam.nType = 142;
    stParam.nCode = nCode;
    VSIFWriteL(&stParam, sizeof(RSCParameter), 1, poFile);

    struct TextParam stPar = {};
    stPar.nColor = static_cast<GUInt32>(DEFAULT_RGB);
    stPar.nBkColor = 0x0FFFFFFFF;
    stPar.nHeight = 14;
    stPar.nThicknes = 400;
    stPar.nOrientation = 1;
    stPar.nCodePage = 1; // DEFAULT_CHARSET

    VSIFWriteL(&stPar, sizeof(struct TextParam), 1, poFile);

    return stParam.nLength;
}

static GUInt32 WriteVectorDefaultParam(GUInt16 nCode, VSILFILE *poFile)
{
    struct VectorParam {
        GUInt32 nLength;
        GUInt32 nAnchorX;
        GUInt32 nAnchorY;
        GUInt32 nSize;
        GUInt32 nBeginX;
        GUInt32 nEndX;
        GUInt32 nSizeX;
        GUInt32 nBeginY;
        GUInt32 nEndY;
        GUInt32 nSizeY;
        GByte nOrientation;
        GByte nMirror;
        GByte nScaleX;
        GByte nScaleY;
        GByte nCenter;
        GByte reserve[3];
        GUInt32 nMaxSize;
        GUInt32 nFragmentCount;
    };

    struct VectorFragment {
        GByte nType;
        GByte nParamType;
        GUInt16 nParamLength;
        GUInt32 nLineColor;
        GUInt32 nLineWidth;
        GUInt32 nPointsCount;
    };

    struct Point {
        GUInt32 X, Y;
    };

    struct VectorParam stV = {};
    stV.nLength = sizeof(struct VectorParam) + sizeof(struct VectorFragment) + 
        sizeof(struct Point) * 2;
    stV.nAnchorX = 500;
    stV.nAnchorY = 500;
    stV.nSize = 4500;
    stV.nBeginX = 0;
    stV.nEndX = 2500;
    stV.nSizeX = 2500;
    stV.nBeginY = 0;
    stV.nEndY = 2500;
    stV.nSizeY = 2500;
    stV.nFragmentCount = 1;

    RSCParameter stParam = RSCParameter();
    stParam.nLength = sizeof(RSCParameter) + stV.nLength;
    stParam.nType = 149;
    stParam.nCode = nCode;
    VSIFWriteL(&stParam, sizeof(RSCParameter), 1, poFile);

    VSIFWriteL(&stV, sizeof(struct VectorParam), 1, poFile);

    struct VectorFragment stVF = {};
    stVF.nType = 1;
    stVF.nParamType = 128;
    stVF.nParamLength = sizeof(struct Point) * 2;
    stVF.nLineColor = DEFAULT_RGB;
    stVF.nLineWidth = 500;
    stVF.nPointsCount = 2;

    VSIFWriteL(&stVF, sizeof(struct VectorFragment), 1, poFile);

    struct Point stBegin = { 0, 0 };
    VSIFWriteL(&stBegin, sizeof(struct Point), 1, poFile);
    struct Point stEnd = { 4500, 0 };
    VSIFWriteL(&stEnd, sizeof(struct Point), 1, poFile);

    return stParam.nLength;
}

static GUInt32 WriteTemplateDefaultParam(GUInt16 nCode, VSILFILE *poFile)
{
    struct TemplateParam {
        GUInt32 nLength;
        GUInt32 nTemplateHeaderLength;
        GUInt32 nCellType[12];
        GUInt32 nAnchorCell;
        GUInt32 nOrientation;
        GUInt32 nFiguresCount;
    };

    struct TemplateParamItem {
        GUInt16 nLength;
        GInt16 nIndex;
    };

    struct TemplateParam stTP = {};
    stTP.nLength = sizeof(struct TemplateParam) + sizeof(struct TemplateParamItem) + 8;
    stTP.nTemplateHeaderLength = 60;
    stTP.nCellType[1] = -1;
    stTP.nAnchorCell = 1;
    stTP.nOrientation = 1;
    stTP.nFiguresCount = 1;

    RSCParameter stParam = RSCParameter();
    stParam.nLength = sizeof(RSCParameter) + stTP.nLength;
    stParam.nType = 150;
    stParam.nCode = nCode;
    VSIFWriteL(&stParam, sizeof(RSCParameter), 1, poFile);

    VSIFWriteL(&stTP, sizeof(struct TemplateParam), 1, poFile);
        
    struct TemplateParamItem stTM = { sizeof(struct TemplateParamItem) + 8, 128 };
    VSIFWriteL(&stTM, sizeof(struct TemplateParamItem), 1, poFile);

    GUInt32 nColor = static_cast<GUInt32>(DEFAULT_RGB);
    VSIFWriteL(&nColor, 4, 1, poFile);
    GUInt32 nSize = 250;
    VSIFWriteL(&nSize, 4, 1, poFile);

    return stParam.nLength;
}

static GUInt32 WriteLineDefaultParam(GUInt16 nCode, VSILFILE *poFile)
{
    RSCParameter stParam = RSCParameter();
    stParam.nLength = sizeof(RSCParameter) + 8;
    stParam.nType = 128;
    stParam.nCode = nCode;
    VSIFWriteL(&stParam, sizeof(RSCParameter), 1, poFile);
    GUInt32 nColor = static_cast<GUInt32>(DEFAULT_RGB);
    VSIFWriteL(&nColor, 4, 1, poFile);
    GUInt32 nSize = 250;
    VSIFWriteL(&nSize, 4, 1, poFile);

    return stParam.nLength;
}

/************************************************************************/
/*                             RSCFile                                  */
/************************************************************************/

RSCFile::RSCFile()
{

}

RSCFile::~RSCFile()
{

}

static std::string GetName(const char *pszName, const std::string &osEncoding)
{
    if( pszName[0] == 0 )
    {    
        return "Unnamed";
    }

    char *pszRecoded = CPLRecode(pszName, osEncoding.c_str(), CPL_ENC_UTF8);
    std::string out(pszRecoded);
    CPLFree(pszRecoded);
    return out;
}

bool RSCFile::Read(const std::string &osPath, CSLConstList papszOpenOpts)
{
    mstLayers.clear();

    auto fpRSC = 
        std::shared_ptr<VSILFILE>(VSIFOpenL(osPath.c_str(), "rb"), VSIFCloseL);
    if (fpRSC == nullptr)
    {
        CPLError(CE_Warning, CPLE_OpenFailed, "RSC file %s open failed",
                    osPath.c_str());
        return false;
    }
        
    CPLDebug( "SXF", "RSC Filename: %s", osPath.c_str() );

    std::string osEncoding =
        CSLFetchNameValueDef(papszOpenOpts, "SXF_ENCODING",
            CPLGetConfigOption("SXF_ENCODING", ""));

    auto nFileLength = FileLength(fpRSC.get());
    VSIFSeekL(fpRSC.get(), 0, SEEK_SET);

    // Read header
    Header stRSCFileHeader;
    size_t nObjectsRead =
        VSIFReadL(&stRSCFileHeader, sizeof(Header), 1, fpRSC.get());

    if (nObjectsRead != 1)
    {
        CPLError(CE_Failure, CPLE_None, "RSC header read failed");
        return false;
    }

    CPL_LSBPTR32(&stRSCFileHeader.nLength);
    if (stRSCFileHeader.nLength != nFileLength)
    {
        CPLError(CE_Warning, CPLE_None, 
            "RSC file length is wrong. Expected %d, got %d", 
            stRSCFileHeader.nLength, nFileLength);
    }

    // Check version
    GByte ver[4];
    VSIFReadL(&ver, 4, 1, fpRSC.get());
    int nVersion = ver[1];

    if ( nVersion != 7 )
    {
        CPLError(CE_Failure, CPLE_NotSupported , 
            "RSC File version %d not supported", nVersion);
        return false;
    }
    
    RSCHeader stRSCFileHeaderEx;
    nObjectsRead = 
        VSIFReadL(&stRSCFileHeaderEx, sizeof(RSCHeader), 1, fpRSC.get());

    if (nObjectsRead != 1)
    {
        CPLError(CE_Warning, CPLE_None, "RSC head read failed");
        return false;
    }

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

    if (osEncoding.empty()) // Input encoding overrides the one set in header
    {
        if (stRSCFileHeaderEx.nFontEnc == 125)
        {
            osEncoding = DEFAULT_ENC_KOI8;
        }
        else if (stRSCFileHeaderEx.nFontEnc == 126)
        {
            osEncoding = DEFAULT_ENC_ANSI;
        }
    }
    
    std::map<GUInt32, SXFField> mstSemantics;
    std::map<std::string, SXFLimits> moLimits;
    if( bIsNewBehavior )
    {
        // Read all semantics
        CPLDebug("SXF", "Read %d attributes from RSC", 
            stRSCFileHeaderEx.Semantic.nRecordCount);
        vsi_l_offset nOffset = stRSCFileHeaderEx.Semantic.nOffset;
        VSIFSeekL(fpRSC.get(), nOffset, SEEK_SET);
        for( GUInt32 i = 0; i < stRSCFileHeaderEx.Semantic.nRecordCount; i++ )
        {
            RSCSemantics stSemantics;
            VSIFReadL(&stSemantics, sizeof(RSCSemantics), 1, fpRSC.get());
            CPL_LSBPTR32(&stSemantics.nCode);
            CPL_LSBPTR16(&stSemantics.nType);
            CPL_LSBPTR32(&stSemantics.nClassifyOffset);

            std::string osAlias =
                GetName(reinterpret_cast<const char*>(stSemantics.szName), 
                    osEncoding);
            RSCSemanticsType eType = RSC_SC_TEXT;
            if( stSemantics.nType == 1 || (stSemantics.nType > 8 && 
                stSemantics.nType < 16) )
            {
                eType = static_cast<RSCSemanticsType>(stSemantics.nType);
            }
            if( stSemantics.nClassifyOffset != 0 )
            {
                eType = RSC_SC_LINK;
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
                { stSemantics.nCode, name, osAlias, eFieldType,
                  stSemantics.nFieldSize };

            nOffset += 84L;
            VSIFSeekL(fpRSC.get(), nOffset, SEEK_SET);   
        }

        // Read limits
        CPLDebug("SXF", "Read %d limits from RSC",
            stRSCFileHeaderEx.Domains.nRecordCount);
        nOffset = stRSCFileHeaderEx.Domains.nOffset;
        VSIFSeekL(fpRSC.get(), nOffset, SEEK_SET);
        for (GUInt32 i = 0; i < stRSCFileHeaderEx.Domains.nRecordCount; i++)
        {
            RSCLimitsRecord stLim;
            VSIFReadL(&stLim, sizeof(RSCLimitsRecord), 1, fpRSC.get());
            CPL_LSBPTR32(&stLim.nLength);
            CPL_LSBPTR32(&stLim.nObjectCode);
            CPL_LSBPTR32(&stLim.nSC1);
            CPL_LSBPTR16(&stLim.nSC1LimCount);
            CPL_LSBPTR16(&stLim.nSC1LimDefIndex);
            CPL_LSBPTR32(&stLim.nSC2);
            CPL_LSBPTR16(&stLim.nSC2LimCount);
            CPL_LSBPTR16(&stLim.nSC2LimDefIndex);

            SXFLimits oLim(stLim.nSC1, stLim.nSC2);
            std::vector<double> aSC1Def, aSC2Def;
            std::vector<int> aMatrix;
            for (int j = 0; j < stLim.nSC1LimCount; j++)
            {
                double dfSCLimDef;
                VSIFReadL(&dfSCLimDef, 8, 1, fpRSC.get());
                CPL_LSBPTR64(&dfSCLimDef);
                aSC1Def.emplace_back(dfSCLimDef);
            }

            for (int j = 0; j < stLim.nSC2LimCount; j++)
            {
                double dfSCLimDef;
                VSIFReadL(&dfSCLimDef, 8, 1, fpRSC.get());
                CPL_LSBPTR64(&dfSCLimDef);
                aSC2Def.emplace_back(dfSCLimDef);
            }

            if (stLim.nSC2LimCount == 0)
            {
                stLim.nSC2LimCount = 1;
            }

            for (int j = 0; j < stLim.nSC1LimCount * stLim.nSC2LimCount; j++)
            {
                GByte nExt;
                VSIFReadL(&nExt, 1, 1, fpRSC.get());
                aMatrix.emplace_back(nExt);
            }

            if (aSC2Def.size() > 0)
            {
                for (size_t j = 0; j < aSC2Def.size(); j++)
                {                
                    for (size_t k = 0; k < aSC1Def.size(); k++)
                    {
                        size_t nMatrixIndex = j * aSC1Def.size() + k;
                        oLim.AddRange(aSC1Def[k], aSC2Def[j], aMatrix[nMatrixIndex]);
                    }
                }
            }
            else
            {
                for (size_t j = 0; j < aSC1Def.size(); j++)
                {
                    oLim.AddRange(aSC1Def[j], 0.0, aMatrix[j]);
                }
            }

            int nDefaultExt;
            if (aSC2Def.size() > 0)
            {
                size_t nMatrixIndex = (stLim.nSC2LimDefIndex - 1) * aSC1Def.size() + (stLim.nSC1LimDefIndex - 1);
                nDefaultExt = aMatrix[nMatrixIndex];
            }
            else
            {
                nDefaultExt = aMatrix[stLim.nSC1LimDefIndex - 1];
            }
            oLim.SetDefaultExt(nDefaultExt);

            auto eGeomType = SXFFile::CodeToGeometryType(stLim.nLocalization);
            auto osFullCode = SXFFile::ToStringCode(eGeomType, stLim.nObjectCode);
            moLimits[osFullCode] = oLim;

            nOffset += stLim.nLength;
            VSIFSeekL(fpRSC.get(), nOffset, SEEK_SET);
        }
    }

    // Read classify code -> semantics[]
    CPLDebug("SXF", "Read %d classify code -> attributes[] from RSC", 
        stRSCFileHeaderEx.PossibleSemantic.nRecordCount);
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
        for( GUInt32 j = 0; j < count; j++ )
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
    CPLDebug("SXF", "Read %d layers from RSC", 
        stRSCFileHeaderEx.Layers.nRecordCount);
    nOffset = stRSCFileHeaderEx.Layers.nOffset;
    VSIFSeekL(fpRSC.get(), nOffset, SEEK_SET);

    for( GUInt32 i = 0; i < stRSCFileHeaderEx.Layers.nRecordCount; i++ )
    {
        RSCLayer stLayer;
        VSIFReadL(&stLayer, sizeof(RSCLayer), 1, fpRSC.get());

        CPL_LSBPTR32(&stLayer.nLength);
        
        std::string osLayerName;
        if (bLayerFullName)
        {
            osLayerName = GetName(reinterpret_cast<const char*>(stLayer.szName),
                osEncoding);
        }
        else
        {
            osLayerName = GetName(reinterpret_cast<const char*>(stLayer.szShortName),
                osEncoding);
        }

        mstLayers[stLayer.nNo] = SXFLayerDefn(stLayer.nNo * EXTRA_ID, osLayerName);

        nOffset += stLayer.nLength;
        VSIFSeekL(fpRSC.get(), nOffset, SEEK_SET);
    }

    // Read Objects
    CPLDebug("SXF", "Read %d objects from RSC", stRSCFileHeaderEx.Objects.nRecordCount);
    nOffset = stRSCFileHeaderEx.Objects.nOffset;
    VSIFSeekL(fpRSC.get(), nOffset, SEEK_SET);
    for( GUInt32 i = 0; i < stRSCFileHeaderEx.Objects.nRecordCount; i++ )
    {
        RSCObject stRSCObject;
        VSIFReadL(&stRSCObject, sizeof(RSCObject), 1, fpRSC.get());
        CPL_LSBPTR32(&stRSCObject.nLength);
        CPL_LSBPTR32(&stRSCObject.nClassifyCode);
        CPL_LSBPTR16(&stRSCObject.nExtNo);

        auto eGeomType = SXFFile::CodeToGeometryType(stRSCObject.nGeometryType);
        auto osFullCode = SXFFile::ToStringCode(eGeomType, stRSCObject.nClassifyCode);

        auto name = GetName(reinterpret_cast<const char*>(stRSCObject.szName),
            osEncoding);

        auto layer = mstLayers.find(stRSCObject.nLayerId);
        if ( layer != mstLayers.end() ) {
            SXFClassCode cc = { osFullCode, name, stRSCObject.nExtNo };
            layer->second.AddCode(cc);

            if( bIsNewBehavior )
            {
                // Add limits for specific code and geometry type
                auto mLimits = moLimits.find(osFullCode);
                if (mLimits != moLimits.end())
                {
                    layer->second.AddLimits(osFullCode, mLimits->second);
                }

                // Get semantics for classify code
                auto semantics = codeSemMap[stRSCObject.nClassifyCode];

                for( auto semantic : semantics )
                {
                    if( !layer->second.HasField(semantic) )
                    {
                        auto semanticsIt = mstSemantics.find(semantic); 
                        if( semanticsIt != mstSemantics.end() )
                        {
                            auto ss = mstSemantics[semantic];
                            layer->second.AddField(ss);
                        }
                    }
                }
            }
        }

        nOffset += stRSCObject.nLength;
        VSIFSeekL(fpRSC.get(), nOffset, SEEK_SET);
    }

    return true;
}

static std::map<GByte, SXFLayerDefn> GetDefaultLayers()
{
    std::map<GByte, SXFLayerDefn> mstDefaultLayers;
    SXFLayerDefn defn(0, "SYSTEM");

    // Some initial codes
    defn.AddCode({ "L1000000001", "Selection line", 0});
    defn.AddCode({ "S1000000002", "Selection square", 0 });
    defn.AddCode({ "P1000000003", "Selection point", 0 });
    defn.AddCode({ "T1000000004", "Selection text", 0 });
    defn.AddCode({ "V1000000005", "Selection vector", 0 });
    defn.AddCode({ "C1000000006", "Selection template", 0 });
    defn.AddCode({ "L1000000007", "System object", 0 });
    defn.AddCode({ "L1000000008", "System object", 0 });
    defn.AddCode({ "L1000000009", "System object", 0 });
    defn.AddCode({ "L1000000010", "System object", 0 });
    defn.AddCode({ "L1000000011", "System object", 0 });
    defn.AddCode({ "L1000000012", "System object", 0 });
    defn.AddCode({ "L1000000013", "System object", 0 });
    defn.AddCode({ "L1000000014", "System object", 0 });
    mstDefaultLayers[0] = defn;
    return mstDefaultLayers;
}

std::map<GByte, SXFLayerDefn> RSCFile::GetLayers() const
{
    if (mstLayers.empty())
    {
        return GetDefaultLayers();
    }
    return mstLayers;
}

static vsi_l_offset WriteCMY(VSILFILE *fpRSC)
{
    GByte acId[4] = { 'C', 'M', 'Y', 0 };
    VSIFWriteL(acId, 4, 1, fpRSC);

    GByte anCMYK[1024] = { 0 };
    memcpy(anCMYK, CMYK, sizeof(CMYK));
    auto pos = VSIFTellL(fpRSC);
    VSIFWriteL(anCMYK, 1024, 1, fpRSC);
    return pos;
}

static vsi_l_offset WriteTAB(VSILFILE *fpRSC, vsi_l_offset nCMYOffset)
{
    GByte acId[4] = { 'T', 'A', 'B', 0 };
    VSIFWriteL(acId, 4, 1, fpRSC);

    RSCTables stRSCTables = RSCTables();
    stRSCTables.nColorsTablesLength = 1024;
    stRSCTables.nColorsTablesOffset = static_cast<GUInt32>(nCMYOffset);
    stRSCTables.nRecordCount = 1;
    auto pos = VSIFTellL(fpRSC);
    VSIFWriteL(&stRSCTables, sizeof(RSCTables), 1, fpRSC);
    GByte nop[8] = { 0 };
    VSIFWriteL(nop, 8, 1, fpRSC);

    WriteRSCSection(pos, sizeof(RSCTables) + 8, 1, 276, fpRSC);

    return pos;
}

static void WriteOBJ(VSILFILE *fpRSC, const std::vector<RSCObj> &astObj, 
    const char *pszEncoding)
{
    GByte objId[4] = { 'O', 'B', 'J', 0 };
    VSIFWriteL(objId, 4, 1, fpRSC);

    auto pos = VSIFTellL(fpRSC);
    GUInt32 nRecordCount = static_cast<GUInt32>(astObj.size());
    GUInt32 nLength = nRecordCount * sizeof(RSCObject);
    GUInt32 nInternalCode = 1;
    for (const auto &stObj : astObj)
    {
        RSCObject stObject = RSCObject();
        stObject.nLength = sizeof(RSCObject);
        stObject.nClassifyCode = stObj.nCode;
        stObject.nInternalCode = nInternalCode;
        stObject.nIdCode = nInternalCode++;
        SXF::WriteEncString(stObj.osName.c_str(), stObject.szShortName, 32, 
            pszEncoding);
        SXF::WriteEncString(stObj.osName.c_str(), stObject.szName, 32, 
            pszEncoding);
        stObject.nGeometryType = stObj.nLoc; // Same as enum SXFGeometryType
        stObject.nLayerId = stObj.nLayer;

        VSIFWriteL(&stObject, sizeof(RSCObject), 1, fpRSC);
    }
    GByte nop[12] = { 0 };
    VSIFWriteL(nop, 12, 1, fpRSC);
    nLength += 12;

    WriteRSCSection(pos, nLength, nRecordCount, 120, fpRSC);
}

static void WriteEmptyBlock(VSILFILE *fpRSC, const char *panCode, 
    vsi_l_offset nOffset)
{
    GByte anCode[4] = { 0 };
    anCode[0] = panCode[0];
    anCode[1] = panCode[1];
    anCode[2] = panCode[2];
    VSIFWriteL(anCode, 4, 1, fpRSC);

    auto pos = VSIFTellL(fpRSC);
    GByte nop[12] = { 0 };
    VSIFWriteL(nop, sizeof(nop), 1, fpRSC);

    WriteRSCSection(pos, 0, 0, nOffset, fpRSC);
}

static void WriteSEM(VSILFILE *fpRSC, const std::vector<RSCSem> &astSem, 
    const char *pszEncoding)
{
    GByte semId[4] = { 'S', 'E', 'M', 0 };
    VSIFWriteL(semId, 4, 1, fpRSC);

    GUInt32 nRecordCount = static_cast<GUInt32>(astSem.size());
    GUInt32 nLength = nRecordCount * sizeof(RSCSemantics);


    auto pos = VSIFTellL(fpRSC);
    for (const auto &stSem : astSem)
    {
        RSCSemantics stSemVal = RSCSemantics();
        stSemVal.nCode = stSem.nCode;
        stSemVal.nType = stSem.nType;
        SXF::WriteEncString(stSem.osName.c_str(), stSemVal.szName, 32, pszEncoding);
        SXF::WriteEncString(stSem.osName.c_str(), stSemVal.szShortName, 16, pszEncoding);
        stSemVal.nFieldSize = stSem.nFieldSize;
        stSemVal.nPrecision = stSem.nPrecision;
        stSemVal.bAllowAnythere = stSem.bAllowAnythere ? 1 : 0;
        stSemVal.bAllowMultiple = stSem.bAllowMultiple ? 1 : 0;
        VSIFWriteL(&stSemVal, sizeof(RSCSemantics), 1, fpRSC);
    }
    GByte nop[12] = { 0 };
    VSIFWriteL(nop, 12, 1, fpRSC);
    nLength += 12;

    WriteRSCSection(pos, nLength, nRecordCount, 132, fpRSC);
}

static void WritePOS(VSILFILE *fpRSC, const std::vector<RSCObj> &astObj, 
    const char *pszEncoding)
{
    GByte posId[4] = { 'P', 'O', 'S', 0 };
    VSIFWriteL(posId, 4, 1, fpRSC);

    GUInt32 nRecordCount = static_cast<GUInt32>(astObj.size());
    GUInt32 nLength = 0;

    auto pos = VSIFTellL(fpRSC);
    for (const auto &stObj : astObj)
    {
        RSCObjectSemantics stPos = RSCObjectSemantics();
        stPos.nPossibleSemCount = static_cast<GUInt16>(stObj.aoSem.size());
        stPos.nLength = sizeof(RSCObjectSemantics) + stPos.nPossibleSemCount * 4;
        stPos.nObjectCode = stObj.nCode;
        stPos.nLocalization = stObj.nLoc;

        VSIFWriteL(&stPos, sizeof(RSCObjectSemantics), 1, fpRSC);

        for (const auto &oSem : stObj.aoSem)
        {
            VSIFWriteL(&oSem.nCode, 4, 1, fpRSC);
        }
        nLength += stPos.nLength;
    }
    GByte nop[12] = { 0 };
    VSIFWriteL(nop, 12, 1, fpRSC);
    nLength += 12;

    WriteRSCSection(pos, nLength, nRecordCount, 168, fpRSC);
}

static void WriteSEG(VSILFILE *fpRSC, const std::vector<std::string> &aosLyr, 
    const char *pszEncoding)
{
    GByte lyrId[4] = { 'S', 'E', 'G', 0 };
    VSIFWriteL(lyrId, 4, 1, fpRSC);

    auto pos = VSIFTellL(fpRSC);
    GUInt32 nRecordCount = static_cast<GUInt32>(aosLyr.size());
    GUInt32 nLength = nRecordCount * sizeof(RSCLayer);

    for (GByte i = 0; i < aosLyr.size(); i++)
    {
        RSCLayer stRSCLayer = RSCLayer();
        stRSCLayer.nLength = sizeof(RSCLayer);
        stRSCLayer.nNo = i;
        if (aosLyr[i] == "SYSTEM")
        {
            stRSCLayer.nDrawOrder = 255;
        }
        SXF::WriteEncString(aosLyr[i].c_str(), stRSCLayer.szName, 32, 
            pszEncoding);
        SXF::WriteEncString(aosLyr[i].c_str(), stRSCLayer.szShortName, 16, 
            pszEncoding);
        VSIFWriteL(&stRSCLayer, sizeof(RSCLayer), 1, fpRSC);
    }
    GByte nop[12] = { 0 };
    VSIFWriteL(nop, 12, 1, fpRSC);
    nLength += 12;

    WriteRSCSection(pos, nLength, nRecordCount, 180, fpRSC);
}

static void WritePAR(VSILFILE *fpRSC, const std::vector<RSCObj> &astObj)
{
    GByte parId[4] = { 'P', 'A', 'R', 0 };
    VSIFWriteL(parId, 4, 1, fpRSC);

    auto pos = VSIFTellL(fpRSC);
    GUInt32 nRecordCount = static_cast<GUInt32>(astObj.size());
    GUInt32 nLength = 0;
    GUInt16 nCounter = 1;
    for (auto stObj : astObj)
    {
        switch (stObj.nLoc)
        {
        case SXF_GT_Polygon:
            nLength += WriteHatchPolygonDefaultParam(nCounter++, fpRSC);
            break;
        case SXF_GT_Point:
            nLength += WritePointDefaultParam(nCounter++, fpRSC);
            break;
        case SXF_GT_Text:
            nLength += WriteTextDefaultParam(nCounter++, fpRSC);
            break;
        case SXF_GT_Vector:
            nLength += WriteVectorDefaultParam(nCounter++, fpRSC);
            break;
        case SXF_GT_TextTemplate:
            nLength += WriteTemplateDefaultParam(nCounter++, fpRSC);
            break;
        case SXF_GT_Line:
        default:
            nLength += WriteLineDefaultParam(nCounter++, fpRSC);
            break;
        }
    }
    GByte nop[12] = { 0 };
    VSIFWriteL(nop, 12, 1, fpRSC);
    nLength += 12;

    WriteRSCSection(pos, nLength, nRecordCount, 204, fpRSC);
}

bool RSCFile::Write(const std::string &osPath, OGRSXFDataSource *poDS, 
    const std::string &osEncoding, const std::map<std::string, int> &mnClassMap)
{
    auto fpRSC =
        std::shared_ptr<VSILFILE>(VSIFOpenL(osPath.c_str(), "wb"), VSIFCloseL);
    if (fpRSC == nullptr)
    {
        CPLError(CE_Warning, CPLE_OpenFailed, "RSC file %s open failed",
            osPath.c_str());
        return false;
    }

    GByte rscId[4] = { 'R', 'S', 'C', 0 };
    size_t nObjectsWrite = VSIFWriteL(rscId, 4, 1, fpRSC.get());
    if (nObjectsWrite != 1)
    {
        CPLError(CE_Failure, CPLE_None, "SXF head write failed");
        return false;
    }

    GUInt32 nLength = 0; // We don't know file size at this moment
    VSIFWriteL(&nLength, 4, 1, fpRSC.get());

    GByte ver[4] = { 2, 7, 0, 0 };
    VSIFWriteL(ver, 4, 1, fpRSC.get());

    RSCHeader stHeader = RSCHeader();
    stHeader.nEncoding[0] = 'N';//126; // ANSI encoding
    stHeader.nEncoding[1] = 'A';
    stHeader.nFileState = 0x0f;
    stHeader.nFileModState = 0x0c;
    stHeader.nLang = 1;       // 1 - en, 2 - ru
    
    // Create date
    struct tm tm;
    CPLUnixTimeToYMDHMS(time(nullptr), &tm);
    memcpy(stHeader.date, std::to_string(tm.tm_year + 1900).c_str(), 4);
    memcpy(stHeader.date + 4, std::to_string(tm.tm_mon + 1).c_str(), 2);
    memcpy(stHeader.date + 6, std::to_string(tm.tm_mday).c_str(), 2);

    auto pszFileName = CPLGetBasename(osPath.c_str());
    auto nFileNameLen = CPLStrnlen(pszFileName, 31);
    memcpy(stHeader.szClassifyName, pszFileName, nFileNameLen);

    auto pszScale = poDS->GetMetadataItem(MD_SCALE_KEY);
    GUInt32 nScale = 200000;
    if (pszScale != nullptr && CPLStrnlen(pszScale, 255) > 4)
    {
        nScale = atoi(pszScale + 4);
    }
    stHeader.nScale = nScale;
    stHeader.nFontEnc = 126; // ANSI encoding
    stHeader.nColorsInPalette = 16;
    
    // Write header
    nObjectsWrite = VSIFWriteL(&stHeader, sizeof(RSCHeader), 1, fpRSC.get());
    if (nObjectsWrite != 1)
    {
        CPLError(CE_Failure, CPLE_None, "RSC head write failed");
        return false;
    }

    /////////////////////////////////////////////////////////

    std::vector<std::string> aosLyr;
    std::vector<RSCObj> astObjs;
    std::vector<RSCSem> astSem;

    /// Mandatory default values order and position at the beginning

    /// Add system layer
    aosLyr.push_back("SYSTEM");

    /// Add default objects
    RSCObj stRSCObject1 = RSCObj();
    stRSCObject1.nCode = 1000000001;
    stRSCObject1.nLayer = 0;
    stRSCObject1.nLoc = SXF_GT_Line;
    stRSCObject1.osName = "L1000000001";
    astObjs.emplace_back(stRSCObject1);

    RSCObj stRSCObject2 = RSCObj();
    stRSCObject2.nCode = 1000000002;
    stRSCObject2.nLayer = 0;
    stRSCObject2.nLoc = SXF_GT_Polygon;
    stRSCObject2.osName = "S1000000002";
    astObjs.emplace_back(stRSCObject2);

    RSCObj stRSCObject3 = RSCObj();
    stRSCObject3.nCode = 1000000003;
    stRSCObject3.nLayer = 0;
    stRSCObject3.nLoc = SXF_GT_Point;
    stRSCObject3.osName = "P1000000003";
    astObjs.emplace_back(stRSCObject3);

    RSCObj stRSCObject4 = RSCObj();
    stRSCObject4.nCode = 1000000004;
    stRSCObject4.nLayer = 0;
    stRSCObject4.nLoc = SXF_GT_Text;
    stRSCObject4.osName = "T1000000004";
    astObjs.emplace_back(stRSCObject4);

    RSCObj stRSCObject5 = RSCObj();
    stRSCObject5.nCode = 1000000005;
    stRSCObject5.nLayer = 0;
    stRSCObject5.nLoc = SXF_GT_Vector;
    stRSCObject5.osName = "V1000000005";
    astObjs.emplace_back(stRSCObject5);

    RSCObj stRSCObject6 = RSCObj();
    stRSCObject6.nCode = 1000000006;
    stRSCObject6.nLayer = 0;
    stRSCObject6.nLoc = SXF_GT_TextTemplate;
    stRSCObject6.osName = "C1000000006";
    astObjs.emplace_back(stRSCObject6);

    for (int i = 7; i < 15; i++)
    {
        auto nCode = 1000000000 + i;
        auto stName = "L" + std::to_string(nCode);

        RSCObj stRSCObjectX = RSCObj();
        stRSCObjectX.nCode = nCode;
        stRSCObjectX.nLayer = 0;
        stRSCObjectX.nLoc = SXF_GT_Line;
        stRSCObjectX.osName = stName;
        astObjs.emplace_back(stRSCObjectX);
    }

    for (int i = 0; i < poDS->GetLayerCount() && i < 255; i++)
    {
        auto poLayer = static_cast<OGRSXFLayer*>(poDS->GetLayer(i));
        if (EQUAL(poLayer->GetName(), "SYSTEM") || 
            (EQUAL(poLayer->GetName(), "Not_CLassified") && 
                poLayer->GetFeatureCount(FALSE) == 0) )
        {
            continue;
        }

        aosLyr.push_back(poLayer->GetName());

        auto poLayerDefn = poLayer->GetLayerDefn();

        std::vector<RSCSem> astLayerSem;
        for (int j = 0; j < poLayerDefn->GetFieldCount(); j++)
        {
            auto poFld = poLayerDefn->GetFieldDefn(j);
            auto nCode = OGRSXFLayer::GetFieldNameCode(poFld->GetNameRef());
            if (nCode == -1)
            {
                auto osCode = OGRSXFLayer::CreateFieldKey(poFld);
                auto it = mnClassMap.find(osCode);
                if (it != mnClassMap.end())
                {
                    nCode = it->second;
                }
            }

            if (nCode == -1)
            {
                continue;
            }

            RSCSem stSem = RSCSem();
            stSem.nCode = nCode;
            
            switch (poFld->GetType())
            {
            case OFTInteger:
            case OFTReal:
            case OFTInteger64:
                stSem.nType = 1;
                break;
            case OFTIntegerList:
            case OFTRealList:
            case OFTInteger64List:
                stSem.nType = 1;
                stSem.bAllowMultiple = true;
                break;
            case OFTStringList:
                stSem.bAllowMultiple = true;
                break;
            default:
                break;
            }

            stSem.nFieldSize = poFld->GetWidth();
            if (stSem.nFieldSize > 255 || stSem.nFieldSize <= 0)
            {
                stSem.nFieldSize = 255;
            }
            
            stSem.nPrecision = poFld->GetPrecision();
            if (stSem.nPrecision > 255 || stSem.nPrecision <= 0)
            {
                stSem.nPrecision = 0;
            }

            stSem.osName = poFld->GetAlternativeNameRef();
            if (stSem.osName.empty())
            {
                stSem.osName = poFld->GetNameRef();
            }

            // Add to global array
            bool bHasItem = false;
            for (const auto& stSemItem : astSem)
            {
                if (stSemItem.nCode == stSem.nCode)
                {
                    bHasItem = true;
                    break;
                }
            }
            if (!bHasItem)
            {
                astSem.emplace_back(stSem);
            }
            astLayerSem.emplace_back(stSem);
        }

        for (const auto &stCode : poLayer->GetClassifyCodes())
        {
            RSCObj stObj = RSCObj();

            if (stCode.osCode.size() < 2)
            {
                continue;
            }

            auto osCode = stCode.osCode.substr(1);
            auto eType = SXFFile::StringToSXFType(stCode.osCode.substr(0, 1));
            int nCode = atoi(osCode.c_str());
            
            stObj.nCode = nCode;
            stObj.nLayer = i + 1; // 0 index is for code of SYSTEM layer
            stObj.osName = stCode.osName;
            stObj.nLoc = static_cast<int>(eType);
            stObj.aoSem = astLayerSem;
            astObjs.emplace_back(stObj);
        }
    }

    /////////////////////////////////////////////////////////

    // Write color table
    auto nCMYOffset = WriteCMY(fpRSC.get());

    // Write tables
    WriteTAB(fpRSC.get(), nCMYOffset);

    // Write objects
    WriteOBJ(fpRSC.get(), astObjs, osEncoding.c_str());

    // Write classify
    WriteEmptyBlock(fpRSC.get(), "CLS", 144);

    // Write defaults
    WriteEmptyBlock(fpRSC.get(), "DEF", 156);

    // Write semantics
    WriteSEM(fpRSC.get(), astSem, osEncoding.c_str());

    // Write possible semantics
    WritePOS(fpRSC.get(), astObjs, osEncoding.c_str());

    // Write layers
    WriteSEG(fpRSC.get(), aosLyr, osEncoding.c_str());

    // Write display parameters
    WritePAR(fpRSC.get(), astObjs);

    // Write limits
    WriteEmptyBlock(fpRSC.get(), "LIM", 192);

    // Write print params
    WriteEmptyBlock(fpRSC.get(), "PRN", 216);

    // Write palletes
    GByte palId[4] = { 'P', 'A', 'L', 0 };
    VSIFWriteL(palId, 4, 1, fpRSC.get());

    RSCPalette stRSCPalette = RSCPalette();
    memcpy(stRSCPalette.anPal, CMYK, sizeof(CMYK));
    SXF::WriteEncString("Standard", stRSCPalette.pszName, 32, osEncoding.c_str());
    auto pos = VSIFTellL(fpRSC.get());
    VSIFWriteL(&stRSCPalette, sizeof(RSCPalette), 1, fpRSC.get());

    WriteRSCSection(pos, sizeof(RSCPalette), 1, 228, fpRSC.get());

    // Write libs
    WriteEmptyBlock(fpRSC.get(), "IML", 252);

    // Write fonts
    GByte txtId[4] = { 'T', 'X', 'T', 0 };
    VSIFWriteL(txtId, 4, 1, fpRSC.get());

    RSCFont stRSCFont = RSCFont();
    SXF::WriteEncString("Arial", stRSCFont.pszName, 32, osEncoding.c_str());
    SXF::WriteEncString("Arial", stRSCFont.pszCode, 32, osEncoding.c_str());
    stRSCFont.nCode = 1;
    stRSCFont.nCodePage = 204;
    stRSCFont.nTestSym = 48;

    pos = VSIFTellL(fpRSC.get());
    VSIFWriteL(&stRSCFont, sizeof(RSCFont), 1, fpRSC.get());

    WriteRSCSection(pos, sizeof(RSCFont), 1, 240, fpRSC.get());

    // Write image params
    WriteEmptyBlock(fpRSC.get(), "GRS", 264);

    // Update values
    WriteNextID(static_cast<GUInt32>(astObjs.size()), fpRSC.get());

    return WriteLength(fpRSC.get());
}
