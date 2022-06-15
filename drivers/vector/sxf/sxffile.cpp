/******************************************************************************
 *
 * Project:  SXF Driver
 * Purpose:  SXF file, SXF passport.
 * Author:   Dmitry Baryshnikov, polimax@mail.ru
 *
 ******************************************************************************
 * Copyright (c) 2020-2021, NextGIS <info@nextgis.com>
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

constexpr const char * MD_SHEET_CREATE_DATE_KEY = "CREATE_DATE";
constexpr const char * MD_SHEET_KEY = "SHEET";
constexpr const char * MD_SHEET_NAME_KEY = "NAME";
constexpr const char * MD_SXF_VERSION_KEY = "SXF_VERSION";

constexpr const char * MD_SW_X_CORNER_KEY = "SW_X";
constexpr const char * MD_SW_Y_CORNER_KEY = "SW_Y";
constexpr const char * MD_NW_X_CORNER_KEY = "NW_X";
constexpr const char * MD_NW_Y_CORNER_KEY = "NW_Y";
constexpr const char * MD_NE_X_CORNER_KEY = "NE_X";
constexpr const char * MD_NE_Y_CORNER_KEY = "NE_Y";
constexpr const char * MD_SE_X_CORNER_KEY = "SE_X";
constexpr const char * MD_SE_Y_CORNER_KEY = "SE_Y";

constexpr const char * MD_MATH_BASE_SHEET_ELLIPSOID_KEY = "MATH_BASE.Ellipsoid";
constexpr const char * MD_MATH_BASE_SHEET_HEIGHT_SYSTEM_KEY = "MATH_BASE.Height_system";
constexpr const char * MD_MATH_BASE_SHEET_PROJECTION_KEY = "MATH_BASE.Projection";
constexpr const char * MD_MATH_BASE_SHEET_SRS_KEY = "MATH_BASE.SRS";
constexpr const char * MD_MATH_BASE_SHEET_MEASURE_PLANE_KEY = "MATH_BASE.Measure_unit_plane";
constexpr const char * MD_MATH_BASE_SHEET_MEASURE_HEIGHT_KEY = "MATH_BASE.Measure_unit_height";
constexpr const char * MD_MATH_BASE_SHEET_FRAMTE_TYPE_KEY = "MATH_BASE.Frame type";
constexpr const char * MD_MATH_BASE_SHEET_MAP_TYPE_KEY = "MATH_BASE.Map type";

constexpr const char * MD_SOURCE_INFO_SURVEY_DATE_KEY = "SOURCE_INFO.Survey date";
constexpr const char * MD_SOURCE_INFO_SOURCE_TYPE_KEY = "SOURCE_INFO.Source type";
constexpr const char * MD_SOURCE_INFO_SOURCE_SUBTYPE_KEY = "SOURCE_INFO.Source subtype";
constexpr const char * MD_SOURCE_INFO_MSK_ZONE_ID_KEY = "SOURCE_INFO.MSK Zone ID";
constexpr const char * MD_SOURCE_INFO_MAP_BORDER_LIMIT_KEY = "SOURCE_INFO.Map limits by border";
constexpr const char * MD_SOURCE_INFO_MAGNETIC_DECLINATION_KEY = "SOURCE_INFO.Magnetic declination";
constexpr const char * MD_SOURCE_INFO_AVG_APPROACH_OF_MERIDIANS_KEY = "SOURCE_INFO.Average approach of meridians";
constexpr const char * MD_SOURCE_INFO_ANNUAL_MAGNETIC_DECLINATION_CHANGE_KEY = "SOURCE_INFO.Annual magnetic declination change";
constexpr const char * MD_SOURCE_INFO_MAGNETIC_DECLINATION_CHECK_DATE_KEY = "SOURCE_INFO.Magnetic declination check date";
constexpr const char * MD_SOURCE_INFO_MSK_ZONE_KEY = "SOURCE_INFO.MSK Zone";
constexpr const char * MD_SOURCE_TERRAIN_STEP_KEY = "SOURCE_INFO.Terrain height step";
constexpr const char * MD_AXIS_ROTATION_KEY = "AXIS_ROTATION";
constexpr const char * MD_SCAN_RESOLUTION_KEY = "SCAN_RESOLUTION";

constexpr const char * MD_MAP_PROJ_INFO_LAT_1SP_KEY = "PROJECTION_INFO.Latitude of the first standard parallel";
constexpr const char * MD_MAP_PROJ_INFO_LAT_2SP_KEY = "PROJECTION_INFO.Latitude of the second standard parallel";
constexpr const char * MD_MAP_PROJ_INFO_LAT_CENTER_KEY = "PROJECTION_INFO.Latitude of center of projection";
constexpr const char * MD_MAP_PROJ_INFO_LONG_CENTER_KEY = "PROJECTION_INFO.Longitude of center of projection";
constexpr const char * MD_MAP_PROJ_INFO_FALSE_EASTING_KEY = "PROJECTION_INFO.False Easting";
constexpr const char * MD_MAP_PROJ_INFO_FALSE_NORTHING_KEY = "PROJECTION_INFO.False Northing";

constexpr const char * MD_DESC_CLASSIFY_CODE_KEY = "DESCRIPTION.Classify";
constexpr const char * MD_DESC_AUTO_GUID_KEY = "DESCRIPTION.Automatic generate GUID";
constexpr const char * MD_DESC_AUTO_TIMESTAMPS_KEY = "DESCRIPTION.Automatic generate timestamps";
constexpr const char * MD_DESC_USE_ALT_FONTS_KEY = "DESCRIPTION.Use alternative fonts";

typedef struct
{
    GByte dataState : 2;
    GByte isProjected : 1;
    GByte hasRealCoords : 2;
    GByte codingType : 2;
    GByte generalizationType : 1;
    GByte reserve[3];
} SXFInformationFlagsV3;

typedef struct
{ 
    GByte dataState : 2;            /* Flag of the state of the data (Note 1) */
    GByte :1;
    GByte hasRealCoords : 2;        /* Flag of real coordinates (Note 2) */
    GByte codingType : 2;           /* Flag of coding type (Note 3) */
    GByte generalizationType : 1;   /* Flag of generalization type (Note 4) */
    GByte textEncoding;             /* Flag of text encoding (Note 5) */
    GByte accuracy;                 /* Flag of coordinate storing accuracy (Note 6) */
    GByte specialSort : 1;          /* Flag of special sort (Note 7) */
    GByte :7;
} SXFInformationFlagsV4;

typedef struct
{
    GByte nEllipsoidType;
    GByte nHeightSrsType;
    GByte nProjectionType;
    GByte nSrsType;
    GByte nPlaneMeasureUnit;
    GByte nHeightMeasureUnit;
    GByte nFrameType;
    GByte nMapType;
} SXFProjectionInfo;


typedef struct
{
    GByte nSectionID[4];    // 0х00544144 (DAT)
    GUInt32 nLength;
    GByte szScheet[24];
    GUInt32 nFeatureCount;
    GByte nDataState : 2;
    GByte nProjCorrespondence : 1;
    GByte nRealCoordinates : 2;
    GByte nEncoding : 2;
    GByte nGenTable : 1;
    GByte : 3;
    GUInt16 nObjClass;
    GUInt16 nSemClass; 
} SXFDataDescriptorV3;

typedef struct
{
    GByte nSectionID[4];    // 0х00544144 (DAT)
    GUInt32 nLength;        // = 52
    GByte szScheet[32];
    GUInt32 nFeatureCount;
    GByte nDataState : 2;
    GByte nProjCorrespondence : 1;
    GByte nRealCoordinates : 2;
    GByte nEncoding : 2;
    GByte nGenTable : 1;
    GByte nLablesEncode;
    GByte nClassify;
    GByte nAutoGenGUID : 1;
    GByte nAutoTimestamp : 1;
    GByte nAltFonts : 1;
    GByte : 5;
    GUInt32 reserve;        // =0
} SXFDataDescriptorV4;

// EPSG code range http://gis.stackexchange.com/a/18676/9904
constexpr int MIN_EPSG = 1000;
constexpr int MAX_EPSG = 32768;

constexpr double VAL_100M = 100000000.0;
constexpr double TO_DEGREE_100M = TO_DEGREES / VAL_100M; // From radians to degree * 100 000 000

constexpr double DELTA = 0.00000001;

static GByte ToGByte(const char *pszVal)
{
    if (pszVal)
    {
        return static_cast<GByte>(atoi(pszVal));
    }
    return 0;
}

static double ToDouble(const char *pszVal)
{
    if (pszVal)
    {
        return atof(pszVal);
    }
    return 0.0;
}

static void CornersToMetadata(OGRSXFDataSource *ds, const char *prefix, double *data)
{
    ds->SetMetadataItem(CPLSPrintf("%s%s", prefix, MD_SW_X_CORNER_KEY), 
        std::to_string(data[0]).c_str());
    ds->SetMetadataItem(CPLSPrintf("%s%s", prefix, MD_SW_Y_CORNER_KEY), 
        std::to_string(data[1]).c_str());
    ds->SetMetadataItem(CPLSPrintf("%s%s", prefix, MD_NW_X_CORNER_KEY), 
        std::to_string(data[2]).c_str());
    ds->SetMetadataItem(CPLSPrintf("%s%s", prefix, MD_NW_Y_CORNER_KEY), 
        std::to_string(data[3]).c_str());
    ds->SetMetadataItem(CPLSPrintf("%s%s", prefix, MD_NE_X_CORNER_KEY), 
        std::to_string(data[4]).c_str());
    ds->SetMetadataItem(CPLSPrintf("%s%s", prefix, MD_NE_Y_CORNER_KEY), 
        std::to_string(data[5]).c_str());
    ds->SetMetadataItem(CPLSPrintf("%s%s", prefix, MD_SE_X_CORNER_KEY), 
        std::to_string(data[6]).c_str());
    ds->SetMetadataItem(CPLSPrintf("%s%s", prefix, MD_SE_Y_CORNER_KEY), 
        std::to_string(data[7]).c_str());
}

static void CornersFromMetadata(OGRSXFDataSource *ds, const char *prefix, double *data)
{
    auto val = ds->GetMetadataItem(CPLSPrintf("%s%s", prefix, MD_SW_X_CORNER_KEY));
    data[0] = ToDouble(val);
    val = ds->GetMetadataItem(CPLSPrintf("%s%s", prefix, MD_SW_Y_CORNER_KEY));
    data[1] = ToDouble(val);
    val = ds->GetMetadataItem(CPLSPrintf("%s%s", prefix, MD_NW_X_CORNER_KEY));
    data[2] = ToDouble(val);
    val = ds->GetMetadataItem(CPLSPrintf("%s%s", prefix, MD_NW_Y_CORNER_KEY));
    data[3] = ToDouble(val);
    val = ds->GetMetadataItem(CPLSPrintf("%s%s", prefix, MD_NE_X_CORNER_KEY));
    data[4] = ToDouble(val);
    val = ds->GetMetadataItem(CPLSPrintf("%s%s", prefix, MD_NE_Y_CORNER_KEY));
    data[5] = ToDouble(val);
    val = ds->GetMetadataItem(CPLSPrintf("%s%s", prefix, MD_SE_X_CORNER_KEY));
    data[6] = ToDouble(val);
    val = ds->GetMetadataItem(CPLSPrintf("%s%s", prefix, MD_SE_Y_CORNER_KEY));
    data[7] = ToDouble(val);
}

static SXFCoordinateMeasureUnit ValueToMeasureUnit(int version, int val)
{
    if (version == 3)
    {
        switch (val)
        {
        case 1:
            return SXF_COORD_MU_DECIMETRE;
        case 2:
            return SXF_COORD_MU_CENTIMETRE;
        case 3:
            return SXF_COORD_MU_MILLIMETRE;
        case 16:
            return SXF_COORD_MU_FOOT;
        case 64:
        case 130:
            return SXF_COORD_MU_RADIAN;
        case 65:
        case 129:
            return SXF_COORD_MU_DEGREE;
        default:
            return SXF_COORD_MU_METRE;
        }
    }
    else if (version == 4)
    {
        switch (val)
        {
        case 64:
            return SXF_COORD_MU_RADIAN;
        case 65:
            return SXF_COORD_MU_DEGREE;
        default:
            return SXF_COORD_MU_METRE;
        }
    }
    return SXF_COORD_MU_METRE;
}

static enum SXFMapType ValueToMapType(int mapType)
{
    if (mapType > 0 && mapType < 22)
    {
        return static_cast<SXFMapType>(mapType);
    }
    return SXF_MT_UNDEFINED;
}

static enum SXFFrameType ValueToFrameType(int frameType)
{
    if (frameType > -1 && frameType < 6)
    {
        return static_cast<SXFFrameType>(frameType);
    }
    return SXF_FT_UNDEFIEND;
}

static int GetZoneNumber(double dfCenterLong)
{
    return static_cast<int>( (dfCenterLong + 3.0) / 6.0 + 0.5 );
}

static double GetCenter(double dfMinX, double dfMaxX)
{
    return dfMinX + fabs(dfMaxX - dfMinX) / 2;
}

static void WriteSXFExtents(const OGREnvelope &stEnv, VSILFILE *fpSXF, 
    bool bSwapCoordinates)
{
    double corners[8];
    if (bSwapCoordinates)
    {
        corners[0] = stEnv.MinY;
        corners[1] = stEnv.MinX;
        corners[2] = stEnv.MaxY;
        corners[3] = stEnv.MinX;
        corners[4] = stEnv.MaxY;
        corners[5] = stEnv.MaxX;
        corners[6] = stEnv.MinY;
        corners[7] = stEnv.MaxX;
    }
    else
    {
        corners[0] = stEnv.MinX;
        corners[1] = stEnv.MinY;
        corners[2] = stEnv.MinX;
        corners[3] = stEnv.MaxY;
        corners[4] = stEnv.MaxX;
        corners[5] = stEnv.MaxY;
        corners[6] = stEnv.MaxX;
        corners[7] = stEnv.MinY;
    }

    VSIFWriteL(corners, 64, 1, fpSXF);
}

/******************************************************************************/
/* SXFDate                                                                    */
/******************************************************************************/

class SXFDate
{
    public:
        SXFDate(GUInt16 nYear = 1970, GUInt16 nMonth = 1, GUInt16 nDay = 1) :
            m_nYear(nYear), m_nMonth(nMonth), m_nDay(nDay) {}

        bool Read(int version, VSILFILE *poSXFFile)
        {
            if (version == 3)
            {
                GByte buff[10];
                VSIFReadL(buff, 10, 1, poSXFFile);

                char date[3] = { 0 };

                // Read year
                memcpy(date, buff, 2);
                m_nYear = static_cast<GUInt16>(atoi(date));
                if (m_nYear < 50)
                    m_nYear += 2000;
                else
                    m_nYear += 1900;

                // Read month
                memcpy(date, buff + 2, 2);

                m_nMonth = static_cast<GUInt16>(atoi(date));

                // Read day
                memcpy(date, buff + 4, 2);

                m_nDay = static_cast<GUInt16>(atoi(date));
            }
            else if (version == 4)
            {
                GByte buff[12];
                VSIFReadL(buff, 12, 1, poSXFFile);

                char date[5] = { 0 };

                // Read year
                memcpy(date, buff, 4);
                m_nYear = static_cast<GUInt16>(atoi(date));

                // Read month
                memcpy(date, buff + 4, 2);
                memset(date+2, 0, 3);

                m_nMonth = static_cast<GUInt16>(atoi(date));

                // Read day
                memcpy(date, buff + 6, 2);

                m_nDay = static_cast<GUInt16>(atoi(date));
            }
            return true;
        }

        bool Write(VSILFILE *poSXFFile)
        {
            // NOTE: Only support SXF v4 format
            GByte buff[12] = { 0 };
            memcpy(buff, std::to_string(m_nYear).c_str(), 4);
            memcpy(buff + 4, std::to_string(m_nMonth).c_str(), 2);
            memcpy(buff + 6, std::to_string(m_nDay).c_str(), 2);
            return VSIFWriteL(&buff, 12, 1, poSXFFile) == 1;
        }

        void ToMetadata(OGRSXFDataSource *poDS, const char *metadataItemKey) const
        {
            poDS->SetMetadataItem(metadataItemKey, 
                CPLSPrintf("%.4u-%.2u-%.2u", m_nYear, m_nMonth, m_nDay));
        }

        bool FromMetadata(OGRSXFDataSource *poDS, const char *metadataItemKey)
        {
            const char *val = poDS->GetMetadataItem(metadataItemKey);
            if (val && 
                sscanf(val, "%hu-%hu-%hu", &m_nYear, &m_nMonth, &m_nDay) != 3)
            {
                return false;
            }
            return true;
        }

    private:
        GUInt16 m_nYear, m_nMonth, m_nDay;
};

/******************************************************************************/
/* SXFLimits                                                                  */
/******************************************************************************/
SXFLimits::SXFLimits(int nSC1In, int nSC2In) : nSC1(nSC1In), nSC2(nSC2In)
{

}

void SXFLimits::AddRange(double dfStart, double dfEnd, int nExt)
{
    aRanges.push_back({ dfStart, dfEnd, nExt });
}

void SXFLimits::SetDefaultExt(int nExt)
{
    nDefaultExt = nExt;
}

static bool IsDoubleEqual(double v1, double v2)
{
    if (std::abs(v1 - v2) < 0.0000001)
    {
        return true;
    }
    return false;
}

int SXFLimits::GetExtention(double dfSC1Val, double dfSC2Val) const
{
    for (const auto &stRange : aRanges)
    {
        if (IsDoubleEqual(dfSC1Val, stRange.dfStart))
        {
            if (IsDoubleEqual(dfSC2Val, 0.0) || 
                IsDoubleEqual(dfSC2Val, stRange.dfEnd))
            {
                {
                    return stRange.nExt;
                }
            }
        }
    }
    return nDefaultExt;
}

std::pair<int, int> SXFLimits::GetLimitCodes() const
{
    return std::make_pair(nSC1, nSC2);
}

/******************************************************************************/
/* SXFLayerDefn                                                               */
/******************************************************************************/

SXFLayerDefn::SXFLayerDefn(int nLayerID, const std::string &osLayerName) :
    nID(nLayerID), osName(osLayerName)
{

}

void SXFLayerDefn::AddCode(SXFClassCode stCodeIn)
{
    // Check duplicates
    for (const auto &stCode : astCodes)
    {
        if (stCode == stCodeIn)
        {
            return;
        }
    }
    astCodes.emplace_back(stCodeIn);
}

bool SXFLayerDefn::HasField(GUInt32 nSemCode) const
{
    for (const auto &field : astFields) {
        if (field.nCode == nSemCode)
        {
            return true;
        }
    }
    return false;
}

void SXFLayerDefn::AddField(SXFField stField)
{
    astFields.emplace_back(stField);
}

std::string SXFLayerDefn::GetName() const
{
    return osName;
}

std::vector<SXFClassCode> SXFLayerDefn::GetCodes(bool bForce)
{
    if (bForce && astCodes.empty())
    {
        auto osCode = "L" + std::to_string(nID + DEFAULT_CLCODE_L);
        astCodes.push_back({ osCode, osCode });
        osCode = "S" + std::to_string(nID + DEFAULT_CLCODE_S);
        astCodes.push_back({ osCode, osCode });
        osCode = "P" + std::to_string(nID + DEFAULT_CLCODE_P);
        astCodes.push_back({ osCode, osCode });
        osCode = "T" + std::to_string(nID + DEFAULT_CLCODE_T);
        astCodes.push_back({ osCode, osCode });
        osCode = "V" + std::to_string(nID + DEFAULT_CLCODE_V);
        astCodes.push_back({ osCode, osCode });
        osCode = "C" + std::to_string(nID + DEFAULT_CLCODE_C);
        astCodes.push_back({ osCode, osCode });
    }
    return astCodes;
}

std::vector<SXFField> SXFLayerDefn::GetFields() const
{
    return astFields;
}

bool SXFLayerDefn::HasCode(const std::string &osCode) const
{
    for (const auto &stCode : astCodes)
    {
        if (stCode.osCode == osCode)
        {
            return true;
        }
    }
    return false;
}

GUInt32 SXFLayerDefn::GenerateCode(SXFGeometryType eGeomType) const
{
    return nID + SXFFile::CodeForGeometryType(eGeomType);
}


void SXFLayerDefn::AddLimits(const std::string &osCode, const SXFLimits &oLimIn)
{
    if (mLim.find(osCode) == mLim.end())
    {
        mLim[osCode] = oLimIn;
    }
}


SXFLimits SXFLayerDefn::GetLimits(const std::string &osCode) const
{
    auto it = mLim.find(osCode);
    if (it != mLim.end())
    {
        return it->second;
    }
    return SXFLimits();
}

std::string SXFLayerDefn::GetCodeName(const std::string &osCode, int nExt) const
{
    for (const auto &stCode : astCodes)
    {
        if (stCode.osCode == osCode && stCode.nExt == nExt)
        {
            return stCode.osName;
        }
    }
    return "";
}

/******************************************************************************/
/* SXFFile                                                                    */
/******************************************************************************/

SXFFile::SXFFile()
{
}

SXFFile::~SXFFile()
{
    Close();
}

bool SXFFile::Open(const std::string &osPath, bool bReadOnly, 
    const std::string &osCodePage)
{
    osEncoding = osCodePage;
    fpSXF = VSIFOpenL(osPath.c_str(), bReadOnly ? "rb" : "wb");
    if (fpSXF == nullptr)
    {
        CPLError(CE_Warning, CPLE_OpenFailed, "SXF open file %s failed", 
            osPath.c_str());
        return false;
    }
    return true;
}

void SXFFile::Close()
{
    if (pSpatRef != nullptr) 
    {
        pSpatRef->Release();
        pSpatRef = nullptr;
    }

    if (fpSXF != nullptr)
    {
        VSIFCloseL( fpSXF );
        fpSXF = nullptr;
    }
}

VSILFILE *SXFFile::File() const
{
    return fpSXF;
}

GUInt32 SXFFile::Version() const
{
    return nVersion;
}

OGRSpatialReference *SXFFile::SpatialRef() const
{
    return pSpatRef;
}

OGREnvelope SXFFile::Extent() const
{
    return oEnvelope;
}

void SXFFile::TranslateXY(double x, double y, double *dfX, double *dfY) const
{
    if (bHasRealCoordinates)
    {
        *dfX = (double)x;
        *dfY = (double)y;
    }
    else
    {
        if (nVersion == 3)
        {
            *dfX = dfXOr + (double)x * dfScaleRatio;
            *dfY = dfYOr + (double)y * dfScaleRatio;
        }
        else if (nVersion == 4)
        {
            *dfX = dfXOr + (double)x * dfScaleRatio;
            *dfY = dfYOr + (double)y * dfScaleRatio;
        }
    }
}

SXFGeometryType SXFFile::CodeToGeometryType(GByte nType)
{
    if (nType >= 0 && nType < 6)
    {
        return static_cast<SXFGeometryType>(nType);
    }

    return SXF_GT_Unknown;
}


std::string SXFFile::SXFTypeToString(enum SXFGeometryType eType)
{
    switch (eType)
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
        return "C";

    default:
        return "";
    }
}

enum SXFGeometryType SXFFile::StringToSXFType(const std::string &type)
{
    if (type == "L")
    {
        return SXF_GT_Line;
    }
    else if (type == "S")
    {
        return SXF_GT_Polygon;
    }
    else if (type == "P")
    {
        return SXF_GT_Point;
    }
    else if (type == "T")
    {
        return SXF_GT_Text;
    }
    else if (type == "V")
    {
        return SXF_GT_Vector;
    }
    else if (type == "C")
    {
        return SXF_GT_TextTemplate;
    }
    return SXF_GT_Unknown;
}

std::string SXFFile::ToStringCode(enum SXFGeometryType eType, 
    GUInt32 nClassifyCode)
{
    return SXFTypeToString(eType) + std::to_string(nClassifyCode);
}

bool SXFFile::Read(OGRSXFDataSource *poDS, CSLConstList papszOpenOpts)
{
    if (fpSXF == nullptr)
    {
        return false;
    }

    // Read header
    Header stSXFFileHeader;
    const size_t nObjectsRead =
        VSIFReadL(&stSXFFileHeader, sizeof(Header), 1, fpSXF);

    if (nObjectsRead != 1)
    {
        CPLError(CE_Failure, CPLE_None, "SXF head read failed");
        return false;
    }

    CPL_LSBPTR32(&stSXFFileHeader.nLength);

    // Check version
    if (stSXFFileHeader.nLength > 256) //if size == 400 then version >= 4
    {
        GByte ver[4];
        VSIFReadL(&ver, 4, 1, fpSXF);
        nVersion = ver[2];
    }
    else
    {
        GByte ver[2];
        VSIFReadL(&ver, 2, 1, fpSXF);
        nVersion = ver[1];
    }

    if (nVersion < 3 || nVersion > 4)
    {
        CPLError(CE_Failure, CPLE_NotSupported , "SXF File version not supported");
        return false;
    }

    GUInt32 nCheckSum;
    VSIFReadL(&nCheckSum, 4, 1, fpSXF);
    CPL_LSBPTR32(&nCheckSum);
    CPLDebug("SXF", "Checksum is %d", nCheckSum);

    // Read passport //////////////////////////////////////////////////////////
    
    // Read create date
    SXFDate createDate;
    if (!createDate.Read(nVersion, fpSXF))
    {
        CPLError(CE_Failure, CPLE_NotSupported , 
            "Failed to read SXF file create date");
        return false;
    }

    // Read sheet nomenclature
    std::string sheetNomk;
    if (nVersion == 3)
    {
        if (osEncoding.empty())
        {
            osEncoding = DEFAULT_ENC_ASCIIZ;
        }
        GByte buff[24];
        VSIFReadL(buff, 24, 1, fpSXF);
        sheetNomk = SXF::ReadEncString(buff, 24, osEncoding.c_str());
    }
    else if (nVersion == 4)
    {
        if (osEncoding.empty())
        {
            osEncoding = DEFAULT_ENC_ANSI;
        }
        GByte buff[32];
        VSIFReadL(buff, 32, 1, fpSXF);
        sheetNomk = SXF::ReadEncString(buff, 32, osEncoding.c_str());
    }

    // Read sheet scale
    GUInt32 nScale;
    VSIFReadL(&nScale, 4, 1, fpSXF);
    CPL_LSBPTR32(&nScale);

    // Read sheet name
    std::string sheetName;
    if (nVersion == 3)
    {
        GByte buff[26];
        VSIFReadL(buff, 26, 1, fpSXF);
        sheetName = SXF::ReadEncString(buff, 26, osEncoding.c_str());
    }
    else if (nVersion == 4)
    {
        GByte buff[32];
        VSIFReadL(buff, 32, 1, fpSXF);
        sheetName = SXF::ReadEncString(buff, 32, osEncoding.c_str());
    }

    // Read information flags
    GUInt32 epsgCode = 0;
    if (nVersion == 3)
    {
        SXFInformationFlagsV3 val;
        VSIFReadL(&val, sizeof(val), 1, fpSXF);
        bHasRealCoordinates = val.hasRealCoords;

        GUInt32 classCode;
        VSIFReadL(&classCode, 4, 1, fpSXF);

        GUInt64 reserve;
        VSIFReadL(&reserve, 8, 1, fpSXF);
    }
    else if (nVersion == 4)
    {
        SXFInformationFlagsV4 val;
        VSIFReadL(&val, sizeof(val), 1, fpSXF);

        bHasRealCoordinates = val.hasRealCoords > 0 || val.accuracy > 0;
        
        VSIFReadL(&epsgCode, 4, 1, fpSXF);

        CPL_LSBPTR32(&epsgCode);

        /* Override file set encoding in favour of defaults or input options
            if (val.textEncoding == 0)
            {
                osEncoding = DEFAULT_ASCIIZ;
            }
            else if (val.textEncoding == 1)
            {
                osEncoding = DEFAULT_ANSI;
            }
            else if (val.textEncoding == 2)
            {
                osEncoding = "KOI8-R";
            }
        */
    }

    // Read sheet corners
    oEnvelope.MaxX = -VAL_100M;
    oEnvelope.MinX = VAL_100M;
    oEnvelope.MaxY = -VAL_100M;
    oEnvelope.MinY = VAL_100M;
    bool bIsX = false;
    double projCoords[8];
    double geogCoords[8];
    if (nVersion == 3)
    {
        GInt32 nCorners[8];

        // Get projected corner coords
        VSIFReadL(&nCorners, 32, 1, fpSXF);

        for (int i = 0; i < 8; i++)
        {
            CPL_LSBPTR32(&nCorners[i]);
            projCoords[i] = double(nCorners[i]) / 10.0;
            if (bIsX)
            {
                if (oEnvelope.MaxX < projCoords[i])
                    oEnvelope.MaxX = projCoords[i];
                if (oEnvelope.MinX > projCoords[i])
                    oEnvelope.MinX = projCoords[i];
            }
            else
            {
                if (oEnvelope.MaxY < projCoords[i])
                    oEnvelope.MaxY = projCoords[i];
                if (oEnvelope.MinY > projCoords[i])
                    oEnvelope.MinY = projCoords[i];
            }
            bIsX = !bIsX;
        }

        // Get geographic corner coords
        VSIFReadL(&nCorners, 32, 1, fpSXF);

        for (int i = 0; i < 8; i++)
        {
            CPL_LSBPTR32(&nCorners[i]);
            geogCoords[i] = double(nCorners[i]) * TO_DEGREE_100M;
        }
    }
    else if (nVersion == 4)
    {
        double dfCorners[8];
        VSIFReadL(&dfCorners, 64, 1, fpSXF);

        for (int i = 0; i < 8; i++)
        {
            CPL_LSBPTR64(&dfCorners[i]);
            projCoords[i] = dfCorners[i];
            if (bIsX)
            {
                if( oEnvelope.MaxX < projCoords[i] )
                    oEnvelope.MaxX = projCoords[i];
                if( oEnvelope.MinX > projCoords[i] )
                    oEnvelope.MinX = projCoords[i];
            }
            else
            {
                if( oEnvelope.MaxY < projCoords[i] )
                    oEnvelope.MaxY = projCoords[i];
                if( oEnvelope.MinY > projCoords[i] )
                    oEnvelope.MinY = projCoords[i];
            }
            bIsX = !bIsX;
        }

        // Get geographic corner coords
       VSIFReadL(&dfCorners, 64, 1, fpSXF);

        for (int i = 0; i < 8; i+=2)
        {
            CPL_LSBPTR64(&dfCorners[i+1]);
            geogCoords[i] = dfCorners[i+1] * TO_DEGREES; // to degree
            CPL_LSBPTR64(&dfCorners[i]);
            geogCoords[i+1] = dfCorners[i] * TO_DEGREES; // to degree
        }
    }

    // Read sheet projection details
    SXFProjectionInfo stProjInfo;
    VSIFReadL(&stProjInfo, sizeof(SXFProjectionInfo), 1, fpSXF);

    SXFCoordinateMeasureUnit eUnitInPlan = 
        ValueToMeasureUnit(nVersion, stProjInfo.nPlaneMeasureUnit);
    // Unused
    // SXFCoordinateMeasureUnit eUnitHeight = 
    //     ValueToMeasureUnit(nVersion, stProjInfo.nHeightMeasureUnit);
    SXFMapType eMapType = ValueToMapType(stProjInfo.nMapType);
    SXFFrameType eFrameType = ValueToFrameType(stProjInfo.nFrameType);

    // Read reference data of map sources 
    SXFDate surveyDate;
    if (!surveyDate.Read(nVersion, fpSXF))
    {
        CPLError(CE_Failure, CPLE_NotSupported, 
            "Failed to read map source survey date");
        return false;
    }

    GByte srcType;
    VSIFReadL(&srcType, 1, 1, fpSXF);
    SXFMapSourceType eMapSourceType = SXF_MS_UNDEFINED;
    if (srcType > 0 && srcType < 4)
    {
        eMapSourceType = static_cast<SXFMapSourceType>(srcType);
    }

    GByte srcSubType;
    VSIFReadL(&srcSubType, 1, 1, fpSXF);
    
    GByte MSK63ZoneID = 0;
    GByte mapLimitByFrame = 1;
    if (nVersion == 4)
    {
        VSIFReadL(&MSK63ZoneID, 1, 1, fpSXF);
        VSIFReadL(&mapLimitByFrame, 1, 1, fpSXF);
    }

    double magneticDeclination = 0.0;
    double avgApproachMeridians = 0.0;
    double annualMagneticDeclinationChange = 0.0;
    double heightStep = 1.0;
    if (nVersion == 3)
    {
        GInt32 md;
        VSIFReadL(&md, 4, 1, fpSXF);
        CPL_LSBPTR32(&md);
        magneticDeclination = double(md) * TO_DEGREE_100M;

        VSIFReadL(&md, 4, 1, fpSXF);
        CPL_LSBPTR32(&md);
        avgApproachMeridians = double(md) * TO_DEGREE_100M;

        GUInt16 height;
        VSIFReadL(&height, 2, 1, fpSXF);
        CPL_LSBPTR16(&height);
        heightStep = double(height) / 10; // To meters

        VSIFReadL(&md, 4, 1, fpSXF);
        CPL_LSBPTR32(&md);
        annualMagneticDeclinationChange = double(md) * TO_DEGREE_100M;
    } 
    else if (nVersion == 4)
    {
        VSIFReadL(&magneticDeclination, 8, 1, fpSXF);
        CPL_LSBPTR64(&magneticDeclination);
        magneticDeclination *= TO_DEGREES;

        VSIFReadL(&avgApproachMeridians, 8, 1, fpSXF);
        CPL_LSBPTR64(&avgApproachMeridians);
        avgApproachMeridians *= TO_DEGREES;

        VSIFReadL(&annualMagneticDeclinationChange, 8, 1, fpSXF);
        CPL_LSBPTR64(&annualMagneticDeclinationChange);
        annualMagneticDeclinationChange *= TO_DEGREES;
    }
    
    SXFDate inclinationMeasureDate;
    inclinationMeasureDate.Read(nVersion, fpSXF);

    GUInt32 MSK63Zone = 0;
    double axisAngle = 0.0;
    if (nVersion == 3)
    {
       // Reserve
        VSIFSeekL(fpSXF, 10, SEEK_CUR);
    }
    else if (nVersion == 4)
    {
        VSIFReadL(&MSK63Zone, 4, 1, fpSXF);
        CPL_LSBPTR32(&MSK63Zone);

        VSIFReadL(&heightStep, 8, 1, fpSXF);
        CPL_LSBPTR64(&heightStep);

        VSIFReadL(&axisAngle, 8, 1, fpSXF);
        CPL_LSBPTR64(&axisAngle);
        axisAngle *= TO_DEGREES;
    }

    GInt32 nResolution = 1;
    GUInt32 frameCoords[8] = { 0 };

    if (nVersion == 3)
    {
        struct _buff{
            GInt32 nRes;
            GInt16 anFrame[8];
        } buff;
        VSIFReadL(&buff, 20, 1, fpSXF);
        CPL_LSBPTR32(&buff.nRes);
        nResolution = buff.nRes; //resolution

        for( int i = 0; i < 8; i++ )
        {
            CPL_LSBPTR16(&(buff.anFrame[i]));
            frameCoords[i] = buff.anFrame[i];
        }
    }
    else if (nVersion == 4)
    {
        GInt32 buff[9];
        VSIFReadL(&buff, 36, 1, fpSXF);
        for (int i = 0; i < 9; i++)
        {
            CPL_LSBPTR32(&buff[i]);
        }

        nResolution = buff[0]; //resolution
        for (int i = 0; i < 8; i++)
        {
            frameCoords[i] = buff[1 + i];
        }
    }

    GUInt32 frameCode;
    VSIFReadL(&frameCode, 4, 1, fpSXF);
    CPL_LSBPTR32(&frameCode);

    // Read additional projection information
    double dfProjScale;
    double adfPrjParams[8] = { 0 };
    if (nVersion == 3)
    {
        GInt32 anParams[5];
        VSIFReadL(&anParams, 20, 1, fpSXF);
        for (int i = 0; i < 5; i++)
        {
            CPL_LSBPTR32(&anParams[i]);
        }

        adfPrjParams[0] = double(anParams[0]) / VAL_100M;
        adfPrjParams[1] = double(anParams[1]) / VAL_100M;
        adfPrjParams[2] = double(anParams[3]) / VAL_100M;
        adfPrjParams[3] = double(anParams[2]) / VAL_100M;

        if( anParams[0] != -1 )
        {
            dfProjScale = double(anParams[0]) / VAL_100M;
        }

        if( anParams[2] != -1 )
        {
            dfXOr = double(anParams[2]) * TO_DEGREE_100M;
        }

        if( anParams[3] != -1 )
        {
            dfYOr = double(anParams[2]) * TO_DEGREE_100M;
        }
    }
    else if (nVersion == 4)
    {
        double adfParams[6];
        VSIFReadL(&adfParams, 48, 1, fpSXF);
        for( int i = 0; i < 6; i++ )
        {
            CPL_LSBPTR64(&adfParams[i]);
        }

        if (eMapType == SXF_MT_TOPO)
        {
            dfProjScale = adfParams[1];
            adfPrjParams[4] = dfProjScale;
        }
        else
        {
           adfPrjParams[0] = adfParams[0];
           adfPrjParams[1] = adfParams[1];
        }

        adfPrjParams[2] = adfParams[3];
        adfPrjParams[3] = adfParams[2];
        adfPrjParams[5] = adfParams[5];
        adfPrjParams[6] = adfParams[4];
        
        dfXOr = adfParams[2] * TO_DEGREES;
        dfYOr = adfParams[3] * TO_DEGREES;
    }

    // Fill metadata

    poDS->SetMetadataItem(MD_SXF_VERSION_KEY, CPLSPrintf("%u", nVersion));
    createDate.ToMetadata(poDS, MD_SHEET_CREATE_DATE_KEY);
    poDS->SetMetadataItem(MD_SHEET_KEY, sheetNomk.c_str());
    poDS->SetMetadataItem(MD_SCALE_KEY, CPLSPrintf("1 : %u", nScale));
    poDS->SetMetadataItem(MD_SHEET_NAME_KEY, sheetName.c_str());
    CornersToMetadata(poDS, "PROJECTION.", projCoords);
    CornersToMetadata(poDS, "GEOGRAPHIC.", geogCoords);
    double frameCoordsDf[8];
    for (int i = 0; i < 8; i++)
    {
        frameCoordsDf[i] = frameCoords[i];
    }
    CornersToMetadata(poDS, "FRAME.", frameCoordsDf);

    /* This metadata not needed because we can get necessary parameteres from SRS
    poDS->SetMetadataItem(MD_MATH_BASE_SHEET_ELLIPSOID_KEY,
        std::to_string(stProjInfo.nEllipsoidType).c_str());
    poDS->SetMetadataItem(MD_MATH_BASE_SHEET_HEIGHT_SYSTEM_KEY,
        std::to_string(stProjInfo.nHeightSrsType).c_str());
    poDS->SetMetadataItem(MD_MATH_BASE_SHEET_PROJECTION_KEY,
        std::to_string(stProjInfo.nProjectionType).c_str());
    poDS->SetMetadataItem(MD_MATH_BASE_SHEET_SRS_KEY,
        std::to_string(stProjInfo.nSrsType).c_str());
    poDS->SetMetadataItem(MD_MATH_BASE_SHEET_MEASURE_PLANE_KEY,
        std::to_string(eUnitInPlan).c_str());
    poDS->SetMetadataItem(MD_MATH_BASE_SHEET_MEASURE_HEIGHT_KEY,
        std::to_string(eUnitHeight).c_str());
    */
    poDS->SetMetadataItem(MD_MATH_BASE_SHEET_FRAMTE_TYPE_KEY,
        std::to_string(eFrameType).c_str());
    poDS->SetMetadataItem(MD_MATH_BASE_SHEET_MAP_TYPE_KEY,
        std::to_string(eMapType).c_str());

    surveyDate.ToMetadata(poDS, MD_SOURCE_INFO_SURVEY_DATE_KEY);
    poDS->SetMetadataItem(MD_SOURCE_INFO_SOURCE_TYPE_KEY, 
        std::to_string(eMapSourceType).c_str());
    poDS->SetMetadataItem(MD_SOURCE_INFO_SOURCE_SUBTYPE_KEY, 
        std::to_string(srcSubType).c_str());
    poDS->SetMetadataItem(MD_SOURCE_INFO_MSK_ZONE_ID_KEY,
        std::to_string(MSK63ZoneID).c_str());
    poDS->SetMetadataItem(MD_SOURCE_INFO_MAP_BORDER_LIMIT_KEY,
        std::to_string(mapLimitByFrame).c_str());
    poDS->SetMetadataItem(MD_SOURCE_INFO_MAGNETIC_DECLINATION_KEY,
        std::to_string(magneticDeclination).c_str());
    poDS->SetMetadataItem(MD_SOURCE_INFO_AVG_APPROACH_OF_MERIDIANS_KEY,
        std::to_string(avgApproachMeridians).c_str());
    poDS->SetMetadataItem(MD_SOURCE_INFO_ANNUAL_MAGNETIC_DECLINATION_CHANGE_KEY,
        std::to_string(annualMagneticDeclinationChange).c_str());
    poDS->SetMetadataItem(MD_SOURCE_INFO_MSK_ZONE_KEY,
        std::to_string(MSK63Zone).c_str());
    poDS->SetMetadataItem(MD_AXIS_ROTATION_KEY,
        std::to_string(axisAngle).c_str());
    poDS->SetMetadataItem(MD_SCAN_RESOLUTION_KEY,
        std::to_string(nResolution).c_str());

    poDS->SetMetadataItem(MD_MAP_PROJ_INFO_LAT_1SP_KEY,
        std::to_string(adfPrjParams[0] * TO_DEGREES).c_str());
    poDS->SetMetadataItem(MD_MAP_PROJ_INFO_LAT_2SP_KEY,
        std::to_string(adfPrjParams[1] * TO_DEGREES).c_str());
    poDS->SetMetadataItem(MD_MAP_PROJ_INFO_LAT_CENTER_KEY,
        std::to_string(adfPrjParams[2] * TO_DEGREES).c_str());
    poDS->SetMetadataItem(MD_MAP_PROJ_INFO_LONG_CENTER_KEY,
        std::to_string(adfPrjParams[3] * TO_DEGREES).c_str());
    poDS->SetMetadataItem(MD_MAP_PROJ_INFO_FALSE_EASTING_KEY,
        std::to_string(adfPrjParams[5]).c_str());
    poDS->SetMetadataItem(MD_MAP_PROJ_INFO_FALSE_NORTHING_KEY,
        std::to_string(adfPrjParams[6]).c_str());

    // Init values
    if (!bHasRealCoordinates)
    {
        bHasRealCoordinates = nResolution < 0;
    }

    if (!bHasRealCoordinates)
    {
        dfScaleRatio = double(nScale) / nResolution;
        if (frameCoords[0] == 0 && frameCoords[1] == 0 && frameCoords[2] == 0 && 
            frameCoords[3] == 0 && frameCoords[4] == 0 && frameCoords[5] == 0 && 
            frameCoords[6] == 0 && frameCoords[7] == 0)
        {
            bHasRealCoordinates = true;
        }
        else
        {
            dfXOr = projCoords[1] - frameCoords[1] * dfScaleRatio;
            dfYOr = projCoords[0] - frameCoords[0] * dfScaleRatio;
        }
    }

    if (SetSRS(stProjInfo.nEllipsoidType, stProjInfo.nProjectionType, 
        stProjInfo.nSrsType, stProjInfo.nHeightSrsType, eUnitInPlan, geogCoords, 
        adfPrjParams, papszOpenOpts))
    {
        // Try set from EPSG code
        if (epsgCode >= MIN_EPSG && epsgCode <= MAX_EPSG) // Check epsg valid range
        {
            pSpatRef = new OGRSpatialReference();
            pSpatRef->SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
            if (pSpatRef->importFromEPSG(epsgCode) != OGRERR_NONE)
            {
                delete pSpatRef;
                pSpatRef = nullptr;
                return false;
            }
        }
        else
        {
            // Not SetSRS and not EPSG are suitable
            return false;
        }
    }

    if (oEnvelope.MaxX - oEnvelope.MinX < DELTA || 
        oEnvelope.MaxY - oEnvelope.MinY < DELTA)
    {
        oEnvelope.MinX = MIN(MIN(geogCoords[0], geogCoords[2]), 
            MIN(geogCoords[4], geogCoords[6]));
        oEnvelope.MaxX = MAX(MAX(geogCoords[0], geogCoords[2]), 
            MAX(geogCoords[4], geogCoords[6]));
        oEnvelope.MinY = MIN(MIN(geogCoords[1], geogCoords[3]), 
            MIN(geogCoords[5], geogCoords[7]));
        oEnvelope.MaxY = MAX(MAX(geogCoords[1], geogCoords[3]), 
            MAX(geogCoords[5], geogCoords[7]));
    }

    // Read description ///////////////////////////////////////////////////////

    if (nVersion == 3)
    {
        VSIFSeekL(fpSXF, 256, SEEK_SET);
        SXFDataDescriptorV3 stDataDescriptor;
        VSIFReadL(&stDataDescriptor, sizeof(SXFDataDescriptorV3), 1, fpSXF);
        CPL_LSBPTR32(&stDataDescriptor.nFeatureCount);
        nFeatureCount = stDataDescriptor.nFeatureCount;
    }
    else if (nVersion == 4)
    {
        VSIFSeekL(fpSXF, 400, SEEK_SET);
        SXFDataDescriptorV4 stDataDescriptor;
        VSIFReadL(&stDataDescriptor, sizeof(SXFDataDescriptorV4), 1, fpSXF);
        CPL_LSBPTR32(&stDataDescriptor.nFeatureCount);
        nFeatureCount = stDataDescriptor.nFeatureCount;

        poDS->SetMetadataItem(MD_DESC_CLASSIFY_CODE_KEY, 
            CPLSPrintf("%d", stDataDescriptor.nClassify));
        poDS->SetMetadataItem(MD_DESC_AUTO_GUID_KEY, 
            CPLSPrintf("%d", stDataDescriptor.nAutoGenGUID));
        poDS->SetMetadataItem(MD_DESC_AUTO_TIMESTAMPS_KEY, 
            CPLSPrintf("%d", stDataDescriptor.nAutoTimestamp));
        poDS->SetMetadataItem(MD_DESC_USE_ALT_FONTS_KEY, 
            CPLSPrintf("%d", stDataDescriptor.nAltFonts));
    }
    /* else nOffset and nObjectsRead will be 0 */

    CPLDebug("SXF", "Total feature count %d", nFeatureCount);

    return true;
}
        
bool SXFFile::Write(OGRSXFDataSource *poDS)
{
    // NOTE: Support only SXF v4

    // Write header
    if (fpSXF == nullptr)
    {
        CPLError(CE_Failure, CPLE_None, "SXF File not openned for write");
        return false;
    }

    // Read header
    Header stSXFFileHeader = { {'S', 'X', 'F', 0}, 400 };

    size_t nObjectsWrite =
        VSIFWriteL(&stSXFFileHeader, sizeof(Header), 1, fpSXF);

    if (nObjectsWrite != 1)
    {
        CPLError(CE_Failure, CPLE_None, "SXF head write failed");
        return false;
    }
    
    GByte ver[4] = { 0,0,4,0 };
    VSIFWriteL(&ver, 4, 1, fpSXF);
    
    // Write temp checksum value
    GUInt32 nCheckSum = 0;
    VSIFWriteL(&nCheckSum, 4, 1, fpSXF);

    // Write passport //////////////////////////////////////////////////////////

    // Write create date
    struct tm tm;
    CPLUnixTimeToYMDHMS(time(nullptr), &tm);
    SXFDate createDate(static_cast<GUInt16>(tm.tm_year + 1900), 
        static_cast<GUInt16>(tm.tm_mon + 1), static_cast<GUInt16>(tm.tm_mday));
    if (!createDate.Write(fpSXF))
    {
        CPLError(CE_Failure, CPLE_NotSupported, 
            "Failed to write SXF file create date");
        return false;
    }

    // Write sheet 
    auto pszSheet = poDS->GetMetadataItem(MD_SHEET_KEY);
    SXF::WriteEncString(pszSheet, 32, osEncoding.c_str(), fpSXF);

    // Write scale
    GUInt32 nScale = 1000000;
    auto pSRS = poDS->GetSpatialRef();
    if (pSRS && (pSRS->IsGeographic() || pSRS->IsGeocentric()))
    {
        // Force scale for Geographic
        nScale = 100000;
    }
    else
    {
        auto pszScale = poDS->GetMetadataItem(MD_SCALE_KEY);        
        if (pszScale != nullptr && CPLStrnlen(pszScale, 255) > 4)
        {
            nScale = atoi(pszScale + 4);
        }
    }
    VSIFWriteL(&nScale, 4, 1, fpSXF);

    // Write sheet name
    auto pszSheetName = poDS->GetMetadataItem(MD_SHEET_NAME_KEY);
    SXF::WriteEncString(pszSheetName, 32, osEncoding.c_str(), fpSXF);

    // Write information flags
    SXFInformationFlagsV4 stFlags = SXFInformationFlagsV4();
    stFlags.dataState = 3;
    stFlags.hasRealCoords = 1;
    stFlags.textEncoding = 1; // ANSI
    stFlags.accuracy = 1;

    VSIFWriteL(&stFlags, sizeof(SXFInformationFlagsV4), 1, fpSXF);
    
    GUInt32 nEPSG = 0;
    if (pSRS)
    {
        if (pSRS->GetAuthorityName(nullptr) != nullptr &&
            pSRS->GetAuthorityCode(nullptr) != nullptr &&
            EQUAL(pSRS->GetAuthorityName(nullptr), "EPSG"))
        {
            nEPSG = atoi(pSRS->GetAuthorityCode(nullptr));
        }
    }
    VSIFWriteL(&nEPSG, 4, 1, fpSXF);
    
    OGREnvelope stEnv;
    poDS->GetExtent(&stEnv);

    if (pSRS && pSRS->IsProjected())
    {
        WriteSXFExtents(stEnv, fpSXF, true);
        auto pGeogCS = pSRS->CloneGeogCS();
        auto ct = OGRCreateCoordinateTransformation(pSRS, pGeogCS);
        if (ct)
        {
            double x[4];
            double y[4];
            x[0] = stEnv.MinX;
            y[0] = stEnv.MinY;
            x[1] = stEnv.MinX;
            y[1] = stEnv.MaxY;
            x[2] = stEnv.MaxX;
            y[2] = stEnv.MaxY;
            x[3] = stEnv.MaxX;
            y[3] = stEnv.MinY;
            ct->Transform(4, x, y);
            stEnv.MinX = MIN(MIN(x[0], x[1]), MIN(x[2], x[3]));
            stEnv.MaxX = MAX(MAX(x[0], x[1]), MAX(x[2], x[3]));
            stEnv.MinY = MIN(MIN(y[0], y[1]), MIN(y[2], y[3]));
            stEnv.MaxY = MAX(MAX(y[0], y[1]), MAX(y[2], y[3]));
            OGRCoordinateTransformation::DestroyCT(ct);
        }
    }
    else
    {
        /*OGRSpatialReference oSRS_EPSG4087;
        oSRS_EPSG4087.importFromEPSG(4087);
        oSRS_EPSG4087.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
        auto ct = OGRCreateCoordinateTransformation(pSRS, &oSRS_EPSG4087);
        if (ct)
        {
            double x[4];
            double y[4];
            x[0] = stEnv.MinX;
            y[0] = stEnv.MinY;
            x[1] = stEnv.MinX;
            y[1] = stEnv.MaxY;
            x[2] = stEnv.MaxX;
            y[2] = stEnv.MaxY;
            x[3] = stEnv.MaxX;
            y[3] = stEnv.MinY;
            ct->Transform(4, x, y);
            OGREnvelope stTmpEnv;
            stTmpEnv.MinX = MIN(MIN(x[0], x[1]), MIN(x[2], x[3]));
            stTmpEnv.MaxX = MAX(MAX(x[0], x[1]), MAX(x[2], x[3]));
            stTmpEnv.MinY = MIN(MIN(y[0], y[1]), MIN(y[2], y[3]));
            stTmpEnv.MaxY = MAX(MAX(y[0], y[1]), MAX(y[2], y[3]));
            OGRCoordinateTransformation::DestroyCT(ct);
            WriteSXFExtents(stTmpEnv, fpSXF, true);
        }*/
        // For GeogCS write 0.0 to projected extents
        OGREnvelope stEmptyEnv;
        stEmptyEnv.MinX = 0.0;
        stEmptyEnv.MaxX = 0.0;
        stEmptyEnv.MinY = 0.0;
        stEmptyEnv.MaxY = 0.0;
        WriteSXFExtents(stEmptyEnv, fpSXF, true);
    }

    stEnv.MinX *= TO_RADIANS;
    stEnv.MinY *= TO_RADIANS;
    stEnv.MaxX *= TO_RADIANS;
    stEnv.MaxY *= TO_RADIANS;

    WriteSXFExtents(stEnv, fpSXF, true);
        
    long iProjSys(0), iDatum(0), iEllips(0), iZone(0);
    double adfPrjParams[7] = { 0 };
    if (pSRS)
    {
        pSRS->exportToPanorama(&iProjSys, &iDatum, &iEllips, &iZone, adfPrjParams);
    }
    GByte val = static_cast<GByte>(iEllips);
    VSIFWriteL(&val, 1, 1, fpSXF);

    // Vertical SRS
    OGRErr eErr = OGRERR_NONE;
    if (pSRS)
    {
        int nVertNo = 0;
        eErr = pSRS->exportVertCSToPanorama(&nVertNo);
        if (eErr == OGRERR_NONE)
        {
            val = nVertNo;
        }
    }
    // Try get from metadata    
    if (eErr != OGRERR_NONE || pSRS == nullptr)
    {
        auto psVertSRS = poDS->GetMetadataItem(MD_MATH_BASE_SHEET_HEIGHT_SYSTEM_KEY);
        if (pSRS && (pSRS->IsGeographic() || pSRS->IsGeocentric()))
        {
            val = 25; // Baltic 1977 height (EPSG : 5705)
        }
        else
        {
            val = ToGByte(psVertSRS);
        }
    }
    VSIFWriteL(&val, 1, 1, fpSXF);

    val = static_cast<GByte>(iProjSys);
    VSIFWriteL(&val, 1, 1, fpSXF);

    val = static_cast<GByte>(iDatum);
    VSIFWriteL(&val, 1, 1, fpSXF);

    val = 0; // Default meters
    if (pSRS && (pSRS->IsGeographic() || pSRS->IsGeocentric()))
    {
        val = 65; // Degrees
    }
    VSIFWriteL(&val, 1, 1, fpSXF);

    auto pszHeightMeasureUnit = 
        poDS->GetMetadataItem(MD_MATH_BASE_SHEET_MEASURE_HEIGHT_KEY);
    val = ToGByte(pszHeightMeasureUnit);
    VSIFWriteL(&val, 1, 1, fpSXF);

    val = SXF_FT_NO_LIMIT;
    if (pSRS && (pSRS->IsGeographic() || pSRS->IsGeocentric()))
    {
        val = SXF_FT_RECTANGULAR;
    }
    auto pszFrameType = poDS->GetMetadataItem(MD_MATH_BASE_SHEET_FRAMTE_TYPE_KEY);
    if (pszFrameType)
    {
        val = static_cast<GByte>(ValueToFrameType(atoi(pszFrameType)));
    }
    VSIFWriteL(&val, 1, 1, fpSXF);
    
    val = SXF_MT_UNDEFINED;
    if (pSRS && (pSRS->IsGeographic() || pSRS->IsGeocentric()))
    {
        val = SXF_MT_SPHERE;
    }
    auto pszMapType = poDS->GetMetadataItem(MD_MATH_BASE_SHEET_MAP_TYPE_KEY);
    if (pszMapType)
    {
        val = static_cast<GByte>(ValueToMapType(atoi(pszMapType)));
    }
    VSIFWriteL(&val, 1, 1, fpSXF);

    SXFDate surveyDate;
    surveyDate.FromMetadata(poDS, MD_SOURCE_INFO_SURVEY_DATE_KEY);
    if (!surveyDate.Write(fpSXF))
    {
        CPLError(CE_Failure, CPLE_NotSupported, 
            "Failed to write SXF file survey date");
        return false;
    }

    auto pszSourceType = 
        poDS->GetMetadataItem(MD_SOURCE_INFO_SOURCE_TYPE_KEY);
    val = ToGByte(pszSourceType);
    VSIFWriteL(&val, 1, 1, fpSXF);

    auto pszSourceSubType = 
        poDS->GetMetadataItem(MD_SOURCE_INFO_SOURCE_SUBTYPE_KEY);
    val = ToGByte(pszSourceSubType);
    VSIFWriteL(&val, 1, 1, fpSXF);

    auto pszMSK63Letter = 
        poDS->GetMetadataItem(MD_SOURCE_INFO_MSK_ZONE_ID_KEY);
    val = ToGByte(pszMSK63Letter);
    VSIFWriteL(&val, 1, 1, fpSXF);

    auto pszMapLimited = 
        poDS->GetMetadataItem(MD_SOURCE_INFO_MAP_BORDER_LIMIT_KEY);
    val = ToGByte(pszMapLimited);
    VSIFWriteL(&val, 1, 1, fpSXF);

    auto pszMagneticDeclination = 
        poDS->GetMetadataItem(MD_SOURCE_INFO_MAGNETIC_DECLINATION_KEY);
    double fval = ToDouble(pszMagneticDeclination) * TO_RADIANS;
    VSIFWriteL(&fval, 8, 1, fpSXF);

    auto pszAverageAapproachMeridians = 
        poDS->GetMetadataItem(MD_SOURCE_INFO_AVG_APPROACH_OF_MERIDIANS_KEY);
    fval = ToDouble(pszAverageAapproachMeridians) * TO_RADIANS;
    VSIFWriteL(&fval, 8, 1, fpSXF);

    auto pszAnnualMagneticDecl = 
        poDS->GetMetadataItem(MD_SOURCE_INFO_ANNUAL_MAGNETIC_DECLINATION_CHANGE_KEY);
    fval = ToDouble(pszAnnualMagneticDecl) * TO_RADIANS;
    VSIFWriteL(&fval, 8, 1, fpSXF);

    SXFDate checkDate;
    checkDate.FromMetadata(poDS, MD_SOURCE_INFO_MAGNETIC_DECLINATION_CHECK_DATE_KEY);
    if (!checkDate.Write(fpSXF))
    {
        CPLError(CE_Failure, CPLE_NotSupported, "Failed to write SXF file survey date");
        return false;
    }

    auto pszMSKZone = poDS->GetMetadataItem(MD_SOURCE_INFO_MSK_ZONE_KEY);
    GUInt32 iVal = 0;
    if (pszMSKZone)
    {
        iVal = atoi(pszMSKZone);
    }
    VSIFWriteL(&iVal, 4, 1, fpSXF);

    auto pszHeightStep = poDS->GetMetadataItem(MD_SOURCE_TERRAIN_STEP_KEY);
    fval = ToDouble(pszHeightStep);
    VSIFWriteL(&fval, 8, 1, fpSXF);
    
    auto pszAxisRotation = poDS->GetMetadataItem(MD_AXIS_ROTATION_KEY);
    fval = ToDouble(pszAxisRotation) * TO_RADIANS;
    VSIFWriteL(&fval, 8, 1, fpSXF);

    auto pszResolution = poDS->GetMetadataItem(MD_SCAN_RESOLUTION_KEY);
    GInt32 nVal = -1;
    if (pszResolution)
    {
        nVal = atoi(pszResolution);
    }
    VSIFWriteL(&nVal, 4, 1, fpSXF);

    double adfFrameCornes[8] = { 0 };
    CornersFromMetadata(poDS, "FRAME.", adfFrameCornes);
    GInt32 adfFrameCornersShort[8];
    for (int i = 0; i < 8; i++)
    {
        adfFrameCornersShort[i] = static_cast<GInt32>(adfFrameCornes[i]);
    }
    VSIFWriteL(adfFrameCornersShort, 32, 1, fpSXF);

    iVal = 91000000;
    VSIFWriteL(&iVal, 4, 1, fpSXF);

    double adfProjDetails[6];
    adfProjDetails[0] = adfPrjParams[0];
    adfProjDetails[1] = adfPrjParams[1];
    adfProjDetails[2] = adfPrjParams[3];
    adfProjDetails[3] = adfPrjParams[2];
    adfProjDetails[4] = adfPrjParams[5];
    adfProjDetails[5] = adfPrjParams[6];

    VSIFWriteL(adfProjDetails, 48, 1, fpSXF);
    
    // Write data description ///////////////////////////////////////////////////
    
    SXFDataDescriptorV4 stDataDesc = SXFDataDescriptorV4();
    stDataDesc.nSectionID[0] = 'D';
    stDataDesc.nSectionID[1] = 'A';
    stDataDesc.nSectionID[2] = 'T';
    stDataDesc.nLength = sizeof(SXFDataDescriptorV4);
    SXF::WriteEncString(pszSheet, stDataDesc.szScheet, 32, osEncoding.c_str());
    stDataDesc.nFeatureCount = 0;
    stDataDesc.nDataState = 3;
    stDataDesc.nProjCorrespondence = 1;
    stDataDesc.nRealCoordinates = 3; // 1?
    stDataDesc.nLablesEncode = 1; // ANSI

    auto pszClass = poDS->GetMetadataItem(MD_DESC_CLASSIFY_CODE_KEY);
    stDataDesc.nClassify = ToGByte(pszClass);

    auto pszAutoGenGUID = poDS->GetMetadataItem(MD_DESC_AUTO_GUID_KEY);
    stDataDesc.nAutoGenGUID = ToGByte(pszAutoGenGUID);
    auto pszAutoTimestamps = poDS->GetMetadataItem(MD_DESC_AUTO_TIMESTAMPS_KEY);
    stDataDesc.nAutoTimestamp = ToGByte(pszAutoTimestamps);
    auto pszAltFonts = poDS->GetMetadataItem(MD_DESC_USE_ALT_FONTS_KEY);
    stDataDesc.nAltFonts = ToGByte(pszAltFonts);

    VSIFWriteL(&stDataDesc, sizeof(SXFDataDescriptorV4), 1, fpSXF);

    return true;
}

OGRErr SXFFile::SetVertCS(const long iVCS, CSLConstList papszOpenOpts)
{
    const char *pszSetVertCS =
        CSLFetchNameValueDef(papszOpenOpts,
                             "SXF_SET_VERTCS",
                             CPLGetConfigOption("SXF_SET_VERTCS", "NO"));
    if (!CPLTestBool(pszSetVertCS))
    {
        return OGRERR_NONE;
    }
    return pSpatRef->importVertCSFromPanorama(static_cast<int>(iVCS));
}


OGRErr SXFFile::SetSRS(const long iEllips, const long iProjSys, const long iCS,
    const long iVCS, enum SXFCoordinateMeasureUnit eUnitInPlan, 
    double *padfGeoCoords, double *padfPrjParams, CSLConstList papszOpenOpts)
{
    if (nullptr != pSpatRef)
    {
        return SetVertCS(iVCS, papszOpenOpts);
    }

    // Normalize some coordintates systems
    if (iEllips == 45 && iProjSys == 35) //Mercator 3857 on sphere wgs84
    {
        pSpatRef = new OGRSpatialReference();
        OGRErr eErr = pSpatRef->importFromEPSG(3857);
        if (eErr != OGRERR_NONE)
        {
            return eErr;
        }
        pSpatRef->SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
        return SetVertCS(iVCS, papszOpenOpts);
    }
    else if (iEllips == 9 && iProjSys == 35) //Mercator 3395 on ellips wgs84
    {
        pSpatRef = new OGRSpatialReference();
        OGRErr eErr = pSpatRef->importFromEPSG(3395);
        if (eErr != OGRERR_NONE)
        {
            return eErr;
        }
        pSpatRef->SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
        return SetVertCS(iVCS, papszOpenOpts);
    }
    else if (iEllips == 9 && iProjSys == 34) //Miller 54003 on sphere wgs84
    {
        pSpatRef = new OGRSpatialReference("PROJCS[\"World_Miller_Cylindrical\",GEOGCS[\"GCS_GLOBE\", DATUM[\"GLOBE\", SPHEROID[\"GLOBE\", 6367444.6571, 0.0]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",0.017453292519943295]],PROJECTION[\"Miller_Cylindrical\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",0],UNIT[\"Meter\",1],AUTHORITY[\"ESRI\",\"54003\"]]");
        pSpatRef->SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
        return SetVertCS(iVCS, papszOpenOpts);
    }
    else if (iEllips == 9 && iProjSys == 33 && eUnitInPlan == SXF_COORD_MU_DEGREE)
    {
        pSpatRef = new OGRSpatialReference(SRS_WKT_WGS84_LAT_LONG);
        pSpatRef->SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
        return SetVertCS(iVCS, papszOpenOpts);
    }
    else if (iEllips == 1 && iProjSys == 20 && iCS == 9)
    {
        CPLString osRefStr = CPLSPrintf("PROJCS[\"Equidistant_Conic\",GEOGCS[\"Pulkovo 1995\",DATUM[\"Pulkovo_1995\",SPHEROID[\"Krassowsky 1940\", 6378245, 298.3,AUTHORITY[\"EPSG\", \"7024\"]],AUTHORITY[\"EPSG\", \"6200\"]],PRIMEM[\"Greenwich\", 0,AUTHORITY[\"EPSG\", \"8901\"]],UNIT[\"degree\", 0.0174532925199433,AUTHORITY[\"EPSG\", \"9122\"]],AUTHORITY[\"EPSG\", \"4200\"]],PROJECTION[\"Equidistant_Conic\"],PARAMETER[\"False_Easting\", %f],PARAMETER[\"False_Northing\", %f],PARAMETER[\"Longitude_Of_Center\", %f],PARAMETER[\"Standard_Parallel_1\", %f],PARAMETER[\"Standard_Parallel_2\", %f],PARAMETER[\"Latitude_Of_Center\", %f],UNIT[\"Meter\", 1]]", TO_DEGREES * padfPrjParams[5], TO_DEGREES * padfPrjParams[6], TO_DEGREES * padfPrjParams[3], TO_DEGREES * padfPrjParams[0], TO_DEGREES * padfPrjParams[1], TO_DEGREES * padfPrjParams[2]);
        pSpatRef = new OGRSpatialReference(osRefStr);
        pSpatRef->SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
        return SetVertCS(iVCS, papszOpenOpts);
    }
    else if ((iEllips == 1 || iEllips == 0 ) && iCS == 1 && iProjSys == 1) // Pulkovo 1942 / Gauss-Kruger
    {
        // First try to get center meridian from metadata
        double dfCenterLongEnv = padfPrjParams[3] * TO_DEGREES;
        if (dfCenterLongEnv < 9 || dfCenterLongEnv > 189) 
        {
            // Next try to get center meridian from sheet bounds. May be errors for double/triple/quad sheets.
            dfCenterLongEnv = GetCenter(padfGeoCoords[1], padfGeoCoords[5]);
        }
        int nZoneEnv = GetZoneNumber(dfCenterLongEnv);
        if (nZoneEnv > 1 && nZoneEnv < 33)
        {
            int nEPSG = 28400 + nZoneEnv;
            pSpatRef = new OGRSpatialReference();
            OGRErr eErr = pSpatRef->importFromEPSG(nEPSG);
            if (eErr != OGRERR_NONE)
            {
                return eErr;
            }
            pSpatRef->SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
            return SetVertCS(iVCS, papszOpenOpts);
        }
        else
        {
            padfPrjParams[7] = nZoneEnv;

            if (padfPrjParams[5] == 0) // False Easting
            {
                if (oEnvelope.MaxX < 500000)
                {
                    padfPrjParams[5] = 500000;
                }
                else
                {
                    padfPrjParams[5] = nZoneEnv * 1000000 + 500000;
                }
            }
        }
    }
    else if ((iEllips == 1 || iEllips == 0) && iCS == 9 && iProjSys == 1) // Pulkovo 1995 / Gauss-Kruger
    {
        // First try to get center meridian from metadata
        double dfCenterLongEnv = padfPrjParams[3] * TO_DEGREES;
        if (dfCenterLongEnv < 9 || dfCenterLongEnv > 189)
        {
            // Next try to get center meridian from sheet bounds. May be errors for double/triple/quad sheets.
            dfCenterLongEnv = GetCenter(padfGeoCoords[1], padfGeoCoords[5]);
        }
        int nZoneEnv = GetZoneNumber(dfCenterLongEnv);
        if (nZoneEnv > 3 && nZoneEnv < 33)
        {
            int nEPSG = 20000 + nZoneEnv;
            pSpatRef = new OGRSpatialReference();
            OGRErr eErr = pSpatRef->importFromEPSG(nEPSG);
            if (eErr != OGRERR_NONE)
            {
                return eErr;
            }
            pSpatRef->SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
            return SetVertCS(iVCS, papszOpenOpts);
        }
        else
        {
            padfPrjParams[7] = nZoneEnv;

            if (padfPrjParams[5] == 0) // False Easting
            {
                if (oEnvelope.MaxX < 500000)
                {
                    padfPrjParams[5] = 500000;
                }
                else
                {
                    padfPrjParams[5] = nZoneEnv * 1000000 + 500000;
                }
            }
        }
    }
    else if (iEllips == 9 && iCS == 2) // WGS84 / UTM
    {
        // First try to get center meridian from metadata
        double dfCenterLongEnv = padfPrjParams[3] * TO_DEGREES;
        if (dfCenterLongEnv < 9 || dfCenterLongEnv > 189) 
        {
            // Next try to get center meridian from sheet bounds. May be errors for double/triple/quad sheets.
            dfCenterLongEnv = GetCenter(padfGeoCoords[1], padfGeoCoords[5]);
        }
        int nZoneEnv = 30 + GetZoneNumber(dfCenterLongEnv);
        bool bNorth = padfGeoCoords[6] + (padfGeoCoords[2] - padfGeoCoords[6]) / 2 < 0;
        int nEPSG = 0;
        if (bNorth)
        {
            nEPSG = 32600 + nZoneEnv;
        }
        else
        {
            nEPSG = 32700 + nZoneEnv;
        }
        pSpatRef = new OGRSpatialReference();
        OGRErr eErr = pSpatRef->importFromEPSG(nEPSG);
        if (eErr != OGRERR_NONE)
        {
            return eErr;
        }
        pSpatRef->SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
        return SetVertCS(iVCS, papszOpenOpts);
    }

    pSpatRef = new OGRSpatialReference();
    OGRErr eErr = 
        pSpatRef->importFromPanorama(iProjSys, 0L, iEllips, padfPrjParams);
    if (eErr != OGRERR_NONE)
    {
        return eErr;
    }
    pSpatRef->SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
    return SetVertCS(iVCS, papszOpenOpts);
}

std::string SXFFile::Encoding() const
{
    return osEncoding;
}

GUInt32 SXFFile::FeatureCount() const
{
    return nFeatureCount;
}

bool SXFFile::WriteTotalFeatureCount(GUInt32 nTotalFeatureCount)
{
    if (fpSXF == nullptr)
    {
        CPLError(CE_Failure, CPLE_None, "SXF File not openned for write");
        return false;
    }
    VSIFSeekL(fpSXF, 400 + 40, SEEK_SET);
    return VSIFWriteL(&nTotalFeatureCount, 4, 1, fpSXF) == 1;
}

static GInt32 GetChecksum(VSILFILE *fpSXF)
{
    VSIFSeekL(fpSXF, 0, SEEK_SET);
    GInt32 nCheckSum = 0;
    int nCounter = 0;
    size_t nRead = 0;
    char nByte;
    while ((nRead = VSIFReadL(&nByte, 1, 1, fpSXF)) > 0)
    {
        nCounter += nRead;
        if (nCounter <= 12 || nCounter > 16) // Skip checksum field
        {
            nCheckSum += nByte;
        }
    }
    return nCheckSum;
}

bool SXFFile::WriteCheckSum()
{
    if (fpSXF == nullptr)
    {
        CPLError(CE_Failure, CPLE_None, "SXF File not openned for write");
        return false;
    }

    GInt32 nCheckSum = GetChecksum(fpSXF);

    CPLDebug("SXF", "Checksum is %d", nCheckSum);
    VSIFSeekL(fpSXF, 12, SEEK_SET);

    return VSIFWriteL(&nCheckSum, 4, 1, fpSXF) == 1;
}

bool SXFFile::CheckSum() const
{
    if (fpSXF == nullptr)
    {
        CPLError(CE_Failure, CPLE_None, "SXF File not openned");
        return false;
    }

    GUInt32 nCheckSum = GetChecksum(fpSXF);

    VSIFSeekL(fpSXF, 12, SEEK_SET);
    GInt32 nCheckSumRecorded = 0;
    VSIFReadL(&nCheckSumRecorded, 4, 1, fpSXF);

    return nCheckSumRecorded - nCheckSum == 0;
}

 GUInt32 SXFFile::CodeForGeometryType(enum SXFGeometryType eGeomType)
{
    switch (eGeomType)
    {
    case SXF_GT_Unknown:
        return 0;
    case SXF_GT_Line:
        return DEFAULT_CLCODE_L;
    case SXF_GT_Polygon:
        return DEFAULT_CLCODE_S;
    case SXF_GT_Point:
        return DEFAULT_CLCODE_P;
    case SXF_GT_Text:
        return DEFAULT_CLCODE_T;
    case SXF_GT_Vector:
        return DEFAULT_CLCODE_V;
    case SXF_GT_TextTemplate:
        return DEFAULT_CLCODE_C;
    default:
        return 0;
    }
}