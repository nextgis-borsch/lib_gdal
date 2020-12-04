/******************************************************************************
 * $Id$
 *
 * Project:  SXF Translator
 * Purpose:  Include file defining Records Structures for file reading and
 *           basic constants.
 * Author:   Ben Ahmed Daho Ali, bidandou(at)yahoo(dot)fr
 *           Dmitry Baryshnikov, polimax@mail.ru
 *           Alexandr Lisovenko, alexander.lisovenko@gmail.com
 *
 ******************************************************************************
 * Copyright (c) 2011, Ben Ahmed Daho Ali
 * Copyright (c) 2013-2020, NextGIS <info@nextgis.com>
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
 *
 ******************************************************************************
 * Structure of the SXF file :
 * ----------------------
 *    - Header
 *    - Passport
 *    - Descriptor of data
 *    - Records
 *         - Title of the record
 *         - The certificate of the object (the geometry)
 *             - sub-objects
 *             - The graphic description of object
 *             - The description of the vector of the tying of the 3d- model of object
 *         - Semantics of object
 *
 ****************************************************************************/

#ifndef SXF_DEFS_H
#define SXF_DEFS_H

#include "cpl_port.h"

// All notes from sxf4bin.pdf
constexpr GUInt32 IDSXFOBJ = 0X7FFF7FFF;
constexpr const char * DEFAULT_ENC_ASCIIZ = "CP866";
constexpr const char * DEFAULT_ENC_ANSI = "CP1251";
constexpr const char * DEFAULT_ENC_KOI8 = "KOI8-R";

constexpr const char * MD_SCALE_KEY = "SCALE";

constexpr int START_CODE = 7654321;
constexpr int DEFAULT_CLCODE_L = START_CODE;
constexpr int DEFAULT_CLCODE_S = START_CODE + 1;
constexpr int DEFAULT_CLCODE_P = START_CODE + 2;
constexpr int DEFAULT_CLCODE_T = START_CODE + 3;
constexpr int DEFAULT_CLCODE_V = START_CODE + 4;
constexpr int DEFAULT_CLCODE_C = START_CODE + 5;

enum SXFDataState /* Flag of the state of the data (Note 1) */
{
    SXF_DS_UNKNOWN = 0,
    SXF_DS_EXCHANGE = 8
};

enum SXFCodingType /* Flag of the semantics coding type (Note 3) */
{
    SXF_SEM_DEC = 0,
    SXF_SEM_HEX = 1,
    SXF_SEM_TXT = 2
};

enum SXFGeneralizationType /* Flag of the source for generalization data (Note 5) */
{
    SXF_GT_SMALL_SCALE = 0,
    SXF_GT_LARGE_SCALE = 1
};

enum SXFTextEncoding /* Flag of text encoding (Note 6) */
{
    SXF_ENC_DOS = 0,
    SXF_ENC_WIN = 1,
    SXF_ENC_KOI_8 = 2
};

enum SXFCoordinatesAccuracy /* Flag of coordinate storing accuracy (Note 7) */
{
    SXF_COORD_ACC_UNDEFINED = 0,
    SXF_COORD_ACC_HIGH = 1, //meters, radians or degree
    SXF_COORD_ACC_CM = 2,   //cantimeters
    SXF_COORD_ACC_MM = 3,   //millimeters
    SXF_COORD_ACC_DM = 4    //decimeters
};

enum SXFCoordinateMeasureUnit
{
    SXF_COORD_MU_METRE = 1,
    SXF_COORD_MU_DECIMETRE,
    SXF_COORD_MU_CENTIMETRE,
    SXF_COORD_MU_MILLIMETRE,
    SXF_COORD_MU_DEGREE,
    SXF_COORD_MU_RADIAN,
    SXF_COORD_MU_FOOT
} ;

enum SXFCoordinateType
{
    SXF_CT_RECTANGULAR = 0,
    SXF_CT_GEODETIC
};

enum SXFMapSourceType
{
    SXF_MS_UNDEFINED = 0,
    SXF_MS_MAP = 1,
    SXF_MS_PHOTOPLAN = 2,
    SXF_MS_IMAGERY = 3
};

enum SXFMapSourceSubtype
{
    SXF_MSS_UNDEFINED = 0,
    SXF_MSS_PRINT = 1,
    SXF_MSS_ORIGINAL = 2,
    SXF_MSS_PRODUCED_ORIGINAL = 3,
    SXF_MSS_MEASURE_ORIGINAL = 4
};

enum SXFImagerySourceSubtype
{
    SXF_ISS_UNDEFIND = 0,
    SXF_ISS_SAT = 1,
    SXF_ISS_AERIAL = 2,
    SXF_ISS_SURVEY = 3
};

enum SXFMapType
{
    SXF_MT_UNDEFINED = 255,
    SXF_MT_SK_42 = 1,
    SXF_MT_GLOBAL_GEO = 2,
    SXF_MT_GLOBE = 3,
    SXF_MT_CITY_PLAN = 4,
    SXF_MT_PLAN = 5,
    SXF_MT_AERIAL_NAV = 6,
    SXF_MT_NAVI_NAV = 7,
    SXF_MT_AERIAL = 8,
    SXF_MT_BLANKING = 9, 
    SXF_MT_UTM_NAD_27 = 10,
    SXF_MT_UTM_WGS_84 = 11,
    SXF_MT_UTM_CUSTOM_ELLIPS = 12,
    SXF_MT_SK_63 = 13,
    SXF_MT_SK_95 = 14,
    SXF_MT_TOPO = 15,
    SXF_MT_SPHERE = 16,
    SXF_MT_MILLER_Cylindrical = 17,
    SXF_MT_MSK_ON_SK_63 = 18,
    SXF_MT_WEB_MERCATOR = 19,
    SXF_MT_Mercator_2SP = 20,
    SXF_MT_GSK_2011 = 21
};

enum SXFFrameType
{
    SXF_FT_UNDEFIEND = 255,
    SXF_FT_NO_LIMIT = 0,
    SXF_FT_TRAPEZOIDAL_NO_ADD_POINTS = 1,
    SXF_FT_TRAPEZOIDAL_ADD_POINTS = 2,
    SXF_FT_RECTANGULAR = 3,
    SXF_FT_ROUND = 4,
    SXF_FT_ARBITRARY = 5
};

/*
 * List of SXF file format geometry types.
 */
enum SXFGeometryType
{
    SXF_GT_Unknown = -1,
    SXF_GT_Line    = 0,         // MultiLineString geometric object
    SXF_GT_Polygon = 1,         // Polygon geometric object
    SXF_GT_Point = 2,           // MultiPoint geometric object
    SXF_GT_Text = 3,            // LineString geometric object with associated labe
    SXF_GT_Vector = 4,          // Vector geometric object with associated label
    SXF_GT_TextTemplate = 5     // Text template
};

enum SXFValueType
{
    SXF_VT_SHORT = 0,   /* 2 byte integer */
    SXF_VT_FLOAT = 1,   /* 2 byte float */
    SXF_VT_INT = 2,     /* 4 byte integer*/
    SXF_VT_DOUBLE = 3   /* 8 byte float */
};

typedef struct
{
    GUInt16 nCode;
    GByte   nType;
    GByte   nScale;
} SXFRecordAttributeInfo;

enum SXFRecordAttributeType
{
    SXF_RAT_UNKNOWN = 255,
    SXF_RAT_ASCIIZ_DOS = 0, //text in DOS encoding
    SXF_RAT_ONEBYTE = 1,    //number 1 byte
    SXF_RAT_TWOBYTE = 2,    //number 2 byte
    SXF_RAT_FOURBYTE = 4,   //number 4 byte
    SXF_RAT_EIGHTBYTE = 8,  //float point number 8 byte
    SXF_RAT_ANSI_WIN = 126, //text in Win encoding
    SXF_RAT_UNICODE = 127,  //text in unicode
    SXF_RAT_BIGTEXT = 128   //text more than 255 chars
};

typedef struct{
    GUInt32 nSign;              /* Identifier of the beginning of record (0x7FFF7FFF) */
    GUInt32 nFullLength;        /* The overall length of record (with the title) */
    GUInt32 nGeometryLength;    /* Length of coordinates (in bytes) */
    GUInt32 nClassifyCode;      /* Classification code */
    GUInt16 anGroup[2];         /* 0 - no group, 1 - in group */
    GByte nLocalizaton : 4;     /* The nature of localization */
    GByte nMultiPolygonPartsOut : 1; /* 1 - Mutlipolygon parts maybe outside */ 
    GByte nMainMetricsDuplicate : 1;
    GByte : 2; /* reserve */
    GByte nSymbolTransform : 1;
    GByte nHasSemantics : 1;
    GByte nCoordinateValueSize : 1;
    GByte : 1;
    GByte nIsUTF16TextEnc : 1;
    GByte nObjectAlignment : 1;
    GByte nObjectOnTop : 1;
    GByte nObjectOnBottom : 1;
    GByte nMetricsDuplicates : 1;
    GByte nDimension : 1;
    GByte nElementType : 1;
    GByte nIsText : 1;
    GByte nHasSymbol : 1;
    GByte nScaledSymbols : 1;
    GByte nSpline : 2;
    GByte nLowViewScale : 4;
    GByte nHighViewScale : 4;
    GUInt32 nPointCount;        /* Point count */
    GUInt16 nSubObjectCount;    /* The sub object count */
    GUInt16 nPointCountSmall;   /* Point count in small geometries */
} SXFRecordHeaderV4;


typedef struct{
    GUInt32 nSign;              /* Identifier of the beginning of record (0x7FFF7FFF) */
    GUInt32 nFullLength;        /* The overall length of record (with the title) */
    GUInt32 nGeometryLength;    /* Length of coordinates (in bytes) */
    GUInt32 nClassifyCode;      /* Classification code */
    GUInt16 anGroup[2];         /* 0 - no group, 1 - in group */
    GByte nLocalizaton : 4;     /* The nature of localization */
    GByte nTouchesFrame : 4;
    GByte nClosed : 1;
    GByte nHasSemantics: 1;
    GByte nCoordinateValueSize : 1;
    GByte nIsGroupObject : 1;
    GByte : 4;
    GByte nFormat : 1;
    GByte nDimension : 1;
    GByte nElementType : 1;
    GByte nIsText : 1;
    GByte : 4;
    GByte nLowViewScale : 4;
    GByte nHighViewScale : 4;
    GUInt32 nGroupId;
    GUInt16 nSubObjectCount;    /* The sub object count */
    GUInt16 nPointCount;   /* Point count in small geometries */
} SXFRecordHeaderV3;

typedef struct {
    int nClassifyCode;
    SXFGeometryType eGeometryType;
    SXFValueType eCoordinateValueType;
    bool bHasZ;
    GUInt32 nGeometryLength;
    int nSubObjectCount;
    GUInt32 nPointCount;
    GUInt32 nAttributesLength;
    bool bHasTextSign;
    std::string osEncoding;
    int nGroupNumber;
    int nNumberInGroup;
} SXFRecordHeader;

typedef struct
{
    GByte szID[4]; //the file ID should be "SXF" or "RSC"
    GUInt32 nLength; //the Header length
} Header;

typedef struct {
    GUInt32 nCode;
    std::string osName;
    std::string osAlias;
    OGRFieldType eFieldType;
    int nWidth;
} SXFField;

typedef struct _SXFClassCode {
    std::string osCode;
    std::string osName;
    int nExt;

    bool operator==(const struct _SXFClassCode &rhs) const
    {
        return osCode == rhs.osCode && 
            osName == rhs.osName && nExt == rhs.nExt;
    }
} SXFClassCode;

#endif  /* SXF_DEFS_H */
