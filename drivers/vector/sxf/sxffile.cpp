/******************************************************************************
 *
 * Project:  SXF Driver
 * Purpose:  SXF file, SXF passport.
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


#define MD_SHEET_CREATE_DATE_KEY "SHEET_CREATE_DATE"
#define MD_SHEET_KEY "SHEET"
#define MD_SHEET_NAME_KEY "SHEET_NAME"
#define MD_SXF_VERSION_KEY "SXF_VERSION"
#define MD_SCALE_KEY "SCALE"

#define MD_SW_X_CORNER_KEY "SW_X"
#define MD_SW_Y_CORNER_KEY "SW_Y"
#define MD_NW_X_CORNER_KEY "NW_X"
#define MD_NW_Y_CORNER_KEY "NW_Y"
#define MD_NE_X_CORNER_KEY "NE_X"
#define MD_NE_Y_CORNER_KEY "NE_Y"
#define MD_SE_X_CORNER_KEY "SE_X"
#define MD_SE_Y_CORNER_KEY "SE_Y"

#define MD_MATH_BASE_SHEET_ELLIPSOID_KEY "MATH_BASE_SHEET.Ellipsoid"
#define MD_MATH_BASE_SHEET_HEIGHT_SYSTEM_KEY "MATH_BASE_SHEET.Height_system"
#define MD_MATH_BASE_SHEET_PROJECTION_KEY "MATH_BASE_SHEET.Projection"
#define MD_MATH_BASE_SHEET_SRS_KEY "MATH_BASE_SHEET.SRS"
#define MD_MATH_BASE_SHEET_MEASURE_PLANE_KEY "MATH_BASE_SHEET.Measure_unit_plane"
#define MD_MATH_BASE_SHEET_MEASURE_HEIGHT_KEY "MATH_BASE_SHEET.Measure_unit_height"
#define MD_MATH_BASE_SHEET_FRAMTE_TYPE_KEY "MATH_BASE_SHEET.Frame type"
#define MD_MATH_BASE_SHEET_MAP_TYPE_KEY "MATH_BASE_SHEET.Map type"

#define MD_SOURCE_INFO_SURVEY_DATE_KEY "SOURCE_INFO.Survey date"
#define MD_SOURCE_INFO_SOURCE_TYPE_KEY "SOURCE_INFO.Source type"
#define MD_SOURCE_INFO_SOURCE_SUBTYPE_KEY "SOURCE_INFO.Source subtype"
#define MD_SOURCE_INFO_MSK_ZONE_ID_KEY "SOURCE_INFO.MSK Zone ID"
#define MD_SOURCE_INFO_MAP_BORDER_LIMIT_KEY "SOURCE_INFO.Map limits by border"
#define MD_SOURCE_INFO_MAGNETIC_DECLINATION_KEY "SOURCE_INFO.Magnetic declination"
#define MD_SOURCE_INFO_AVG_APPROACH_OF_MERIDIANS_KEY "SOURCE_INFO.Average approach of meridians"
#define MD_SOURCE_INFO_ANNUAL_MAGNETIC_DECLINATION_CHANGE_KEY "SOURCE_INFO.Annual magnetic declination change"
#define MD_SOURCE_INFO_MAGNETIC_DECLINATION_CHECK_DATE_KEY "SOURCE_INFO.Magnetic declination check date"
#define MD_SOURCE_INFO_MSK_ZONE_KEY "SOURCE_INFO.MSK Zone"
#define MD_SOURCE_TERRAIN_STEP_KEY "SOURCE_INFO.Terrain height step"
#define MD_AXIS_ROTATION_KEY "AXIS_ROTATION"
#define MD_SCAN_RESOLUTION_KEY "SCAN_RESOLUTION"

#define MD_MAP_PROJ_INFO_LAT_1SP_KEY "PROJECTION_INFO.Latitude of the first standard parallel"
#define MD_MAP_PROJ_INFO_LAT_2SP_KEY "PROJECTION_INFO.Latitude of the second standard parallel"
#define MD_MAP_PROJ_INFO_LAT_CENTER_KEY "PROJECTION_INFO.Latitude of center of projection"
#define MD_MAP_PROJ_INFO_LONG_CENTER_KEY "PROJECTION_INFO.Longitude of center of projection"
#define MD_MAP_PROJ_INFO_FALSE_EASTING_KEY "PROJECTION_INFO.False Easting"
#define MD_MAP_PROJ_INFO_FALSE_NORTHING_KEY "PROJECTION_INFO.False Northing"

#define MD_DESC_CLASSIFY_CODE_KEY "DESCRIPTION.Classify"
#define MD_DESC_AUTO_GUID_KEY "DESCRIPTION.Automatic generate GUID"
#define MD_DESC_AUTO_TIMESTAMPS_KEY "DESCRIPTION.Automatic generate timestamps"
#define MD_DESC_USE_ALT_FONTS_KEY "DESCRIPTION.Use alternative fonts"

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

/*
static enum SXFMapType ValueToMapType(GByte mapType)
{
    if (mapType > 0 && mapType < 22)
    {
        return static_cast<SXFMapType>(mapType);
    }
    return SXF_MT_UNDEFINED;
}
*/

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

/******************************************************************************/
/* SXFDate                                                                    */
/******************************************************************************/

class SXFDate
{
    public:
        SXFDate(GUInt16 nYear = 1970, GUInt16 nMonth = 1, GUInt16 nDay = 1) :
            m_nYear(nYear), m_nMonth(nMonth), m_nDay(nDay) {}

        OGRErr Read(int version, VSILFILE *poSXFFile)
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
            return OGRERR_NONE;
        }

        OGRErr Write(VSILFILE *poSXFFile) // Only support v4 format
        {
            return OGRERR_UNSUPPORTED_OPERATION;
        }

        void ToMetadata(OGRSXFDataSource *poDS, const char *metadataItemKey) const
        {
            poDS->SetMetadataItem(metadataItemKey, 
                CPLSPrintf("%.4u-%.2u-%.2u", m_nYear, m_nMonth, m_nDay));
        }

        OGRErr FromMetadata(OGRSXFDataSource *poDS, const char *metadataItemKey)
        {
            const char *val = poDS->GetMetadataItem(metadataItemKey);
            if (CPLsscanf(val, "%hu-%hu-%hu", &m_nYear, &m_nMonth, &m_nDay) != 3)
            {
                return OGRERR_CORRUPT_DATA;
            }
            return OGRERR_NONE;
        }

    private:
        GUInt16 m_nYear, m_nMonth, m_nDay;
};

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

bool SXFFile::Open(const std::string &osPath)
{
    fpSXF = VSIFOpenL(osPath.c_str(), "rb");
    if( fpSXF == nullptr )
    {
        CPLError(CE_Warning, CPLE_OpenFailed, "SXF open file %s failed", 
            osPath.c_str());
        return false;
    }
    return true;
}

void SXFFile::Close()
{
    if( hIOMutex != nullptr )
    {
        CPLDestroyMutex(hIOMutex);
        hIOMutex = nullptr;
    }

    if (nullptr != pSpatRef) 
    {
        pSpatRef->Release();
    }

    if( fpSXF != nullptr )
    {
        VSIFCloseL( fpSXF );
        fpSXF = nullptr;
    }
}

VSILFILE *SXFFile::File() const
{
    return fpSXF;
}

CPLMutex **SXFFile::Mutex()
{
    return &hIOMutex;
}

GUInt32 SXFFile::Version() const
{
    return nVersion;
}

OGRSpatialReference *SXFFile::SpatialRef() const
{
    return pSpatRef;
}

OGRErr SXFFile::FillExtent(OGREnvelope *env) const
{
    env->MinX = oEnvelope.MinX;
    env->MaxX = oEnvelope.MaxX;
    env->MinY = oEnvelope.MinY;
    env->MaxY = oEnvelope.MaxY;
    return OGRERR_NONE;
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


std::string SXFFile::ReadSXFString(const void *pBuffer, size_t nLen, 
    const char *pszSrcEncoding)
{
    if(nLen == 0)
    {
        return "";
    }
    char *value = static_cast<char*>(CPLMalloc(nLen + 1));
    memcpy(value, pBuffer, nLen);
    value[nLen] = 0;
    char *pszRecoded = CPLRecode(value, pszSrcEncoding, CPL_ENC_UTF8);
    std::string out(pszRecoded);
    CPLFree(pszRecoded);
    CPLFree(value);
    return out;
}

SXFGeometryType SXFFile::CodeToGeometryType(GByte nType)
{
    if( nType >= 0 && nType < 6 )
    {
        return static_cast<SXFGeometryType>(nType);
    }

    return SXF_GT_Unknown;
}

OGRErr SXFFile::Read(OGRSXFDataSource *poDS, CSLConstList papszOpenOpts)
{
    if( fpSXF == nullptr )
    {
        return OGRERR_FAILURE;
    }

    // Read header
    Header stSXFFileHeader;
    const size_t nObjectsRead =
        VSIFReadL(&stSXFFileHeader, sizeof(Header), 1, fpSXF);

    if (nObjectsRead != 1)
    {
        CPLError(CE_Failure, CPLE_None, "SXF head read failed");
        return OGRERR_CORRUPT_DATA;
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

    if ( nVersion < 3 || nVersion > 4 )
    {
        CPLError(CE_Failure, CPLE_NotSupported , "SXF File version not supported");
        return OGRERR_FAILURE;
    }

    GUInt32 nCheckSum;
    VSIFReadL(&nCheckSum, 4, 1, fpSXF);
    CPL_LSBPTR32(&nCheckSum);
    CPLDebug("SXF", "Checksum is %d", nCheckSum);

    // Read passport //////////////////////////////////////////////////////////
    
    // Read create date
    SXFDate createDate;
    if (createDate.Read(nVersion, fpSXF) != OGRERR_NONE)
    {
        CPLError(CE_Failure, CPLE_NotSupported , "Failed to read SXF file create date");
        return OGRERR_FAILURE;
    }

    // Read sheet nomenclature
    std::string sheetNomk;
    if (nVersion == 3)
    {
        GByte buff[24];
        VSIFReadL(buff, 24, 1, fpSXF);
        sheetNomk = ReadSXFString(buff, 24, "CP1251");
    }
    else if (nVersion == 4)
    {
        GByte buff[32];
        VSIFReadL(buff, 32, 1, fpSXF);
        sheetNomk = ReadSXFString(buff, 32, "CP1251");
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
        sheetName = ReadSXFString(buff, 26, "CP866");
    }
    else if (nVersion == 4)
    {
        GByte buff[32];
        VSIFReadL(buff, 32, 1, fpSXF);
        sheetName = ReadSXFString(buff, 32, "CP1251");
    }

    // Read information flags
    GUInt32 epsgCode = 0;
    if( nVersion == 3 )
    {
        SXFInformationFlagsV3 val;
        VSIFReadL(&val, sizeof(val), 1, fpSXF);
        bHasRealCoordinates = val.hasRealCoords;

        GUInt32 classCode;
        VSIFReadL(&classCode, 4, 1, fpSXF);

        GUInt64 reserve;
        VSIFReadL(&reserve, 8, 1, fpSXF);
    }
    else if( nVersion == 4 )
    {
        SXFInformationFlagsV4 val;
        VSIFReadL(&val, sizeof(val), 1, fpSXF);

        bHasRealCoordinates = val.hasRealCoords || val.accuracy > 0;
        
        VSIFReadL(&epsgCode, 4, 1, fpSXF);

        CPL_LSBPTR32(&epsgCode);

        if( epsgCode >= MIN_EPSG && epsgCode <= MAX_EPSG ) // Check epsg valid range
        {
            pSpatRef = new OGRSpatialReference();
            if (pSpatRef->importFromEPSG(epsgCode) != OGRERR_NONE)
            {
                delete pSpatRef;
                pSpatRef = nullptr;
            }
        }

        if( val.textEncoding == 0 )
        {
            osEncoding = "CP866";
        }
        else if( val.textEncoding == 2 )
        {
            osEncoding = "KOI8-R";
        }
    }

    // Read sheet corners
    oEnvelope.MaxX = -VAL_100M;
    oEnvelope.MinX = VAL_100M;
    oEnvelope.MaxY = -VAL_100M;
    oEnvelope.MinY = VAL_100M;
    bool bIsX = true;
    double projCoords[8];
    double geogCoords[8];
    if (nVersion == 3)
    {
        GInt32 nCorners[8];

        // Get projected corner coords
        VSIFReadL(&nCorners, 32, 1, fpSXF);

        for( int i = 0; i < 8; i++ )
        {
            CPL_LSBPTR32(&nCorners[i]);
            projCoords[i] = double(nCorners[i]) / 10.0;
            if( bIsX ) //X
            {
                if (oEnvelope.MaxY < projCoords[i])
                    oEnvelope.MaxY = projCoords[i];
                if (oEnvelope.MinY > projCoords[i])
                    oEnvelope.MinY = projCoords[i];
            }
            else
            {
                if (oEnvelope.MaxX < projCoords[i])
                    oEnvelope.MaxX = projCoords[i];
                if (oEnvelope.MinX > projCoords[i])
                    oEnvelope.MinX = projCoords[i];
            }
            bIsX = !bIsX;
        }

        // Get geographic corner coords
        VSIFReadL(&nCorners, 32, 1, fpSXF);

        for( int i = 0; i < 8; i++ )
        {
            CPL_LSBPTR32(&nCorners[i]);
            geogCoords[i] = double(nCorners[i]) * TO_DEGREE_100M;
        }
    }
    else if( nVersion == 4 )
    {
        double dfCorners[8];
        VSIFReadL(&dfCorners, 64, 1, fpSXF);

        for( int i = 0; i < 8; i++ )
        {
            CPL_LSBPTR64(&dfCorners[i]);
            projCoords[i] = dfCorners[i];
            if( bIsX ) //X
            {
                if( oEnvelope.MaxY < projCoords[i] )
                    oEnvelope.MaxY = projCoords[i];
                if( oEnvelope.MinY > projCoords[i] )
                    oEnvelope.MinY = projCoords[i];
            }
            else
            {
                if( oEnvelope.MaxX < projCoords[i] )
                    oEnvelope.MaxX = projCoords[i];
                if( oEnvelope.MinX > projCoords[i] )
                    oEnvelope.MinX = projCoords[i];
            }
            bIsX = !bIsX;
        }

        // Get geographic corner coords
       VSIFReadL(&dfCorners, 64, 1, fpSXF);

        for( int i = 0; i < 8; i++ )
        {
            CPL_LSBPTR64(&dfCorners[i]);
            geogCoords[i] = dfCorners[i] * TO_DEGREES; // to degree
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
    // SXFMapType eMapType = ValueToMapType(stProjInfo.nMapType);
    SXFFrameType eFrameType = ValueToFrameType(stProjInfo.nFrameType);

    // Read reference data of map sources 
    SXFDate surveyDate;
    if( surveyDate.Read(nVersion, fpSXF) != OGRERR_NONE )
    {
        CPLError(CE_Failure, CPLE_NotSupported , "Failed to read map source survey date");
        return OGRERR_FAILURE;
    }

    GByte srcType;
    VSIFReadL(&srcType, 1, 1, fpSXF);
    SXFMapSourceType eMapSourceType = SXF_MS_UNDEFINED;
    if( srcType > 0 && srcType < 4 )
    {
        eMapSourceType = static_cast<SXFMapSourceType>(srcType);
    }

    GByte srcSubType;
    VSIFReadL(&srcSubType, 1, 1, fpSXF);
    
    GByte MSK63ZoneID = 0;
    GByte mapLimitByFrame = 1;
    if( nVersion == 4 )
    {
        VSIFReadL(&MSK63ZoneID, 1, 1, fpSXF);
        VSIFReadL(&mapLimitByFrame, 1, 1, fpSXF);
    }

    double magneticDeclination;
    double avgApproachMeridians;
    double annualMagneticDeclinationChange;
    double heightStep;
    if( nVersion == 3 )
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
    else if( nVersion == 4 )
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
    if( nVersion == 3 )
    {
        // Reserve
        VSIFSeekL(fpSXF, 10, SEEK_CUR);
    }
    else if( nVersion == 4 )
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
    GUInt32 frameCoords[8];

    if( nVersion == 3 )
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
    else if( nVersion == 4 )
    {
        GUInt32 buff[10];
        VSIFReadL(&buff, 40, 1, fpSXF);
        for( int i = 0; i < 10; i++ )
        {
            CPL_LSBPTR32(&buff[i]);
        }

        nResolution = buff[0]; //resolution
        for( int i = 0; i < 8; i++ )
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
    if( nVersion == 3 )
    {
        GInt32 anParams[5];
        VSIFReadL(&anParams, 20, 1, fpSXF);
        for( int i = 0; i < 5; i++ )
        {
            CPL_LSBPTR32(&anParams[i]);
        }

        adfPrjParams[0] = double(anParams[0]) / VAL_100M;
        adfPrjParams[1] = double(anParams[1]) / VAL_100M;
        adfPrjParams[2] = double(anParams[2]) / VAL_100M;
        adfPrjParams[3] = double(anParams[3]) / VAL_100M;

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
    else if( nVersion == 4 )
    {
        double adfParams[6];
        VSIFReadL(&adfParams, 48, 1, fpSXF);
        for( int i = 0; i < 6; i++ )
        {
            CPL_LSBPTR64(&adfParams[i]);
        }

        if( adfParams[1] != -1 )
        {
            dfProjScale = adfParams[1];
            adfPrjParams[4] = dfProjScale;
        }
        else
        {
           adfPrjParams[0] = adfParams[0];
           adfPrjParams[1] = adfParams[1];
        }

        adfPrjParams[2] = adfParams[2];
        adfPrjParams[3] = adfParams[3];
        adfPrjParams[5] = adfParams[4];
        adfPrjParams[6] = adfParams[5];
        
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
    for( int i = 0; i < 8; i++ )
    {
        frameCoordsDf[i] = frameCoords[i];
    }
    CornersToMetadata(poDS, "FRAME.", frameCoordsDf);

    /* This metadata not needed becouse we can get necessary parameteres from SRS
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
        std::to_string(eFrameType).c_str());

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
        std::to_string(adfPrjParams[0]).c_str());
    poDS->SetMetadataItem(MD_MAP_PROJ_INFO_LAT_2SP_KEY,
        std::to_string(adfPrjParams[1]).c_str());
    poDS->SetMetadataItem(MD_MAP_PROJ_INFO_LAT_CENTER_KEY,
        std::to_string(adfPrjParams[2]).c_str());
    poDS->SetMetadataItem(MD_MAP_PROJ_INFO_LONG_CENTER_KEY,
        std::to_string(adfPrjParams[3]).c_str());
    poDS->SetMetadataItem(MD_MAP_PROJ_INFO_FALSE_EASTING_KEY,
        std::to_string(adfPrjParams[5]).c_str());
    poDS->SetMetadataItem(MD_MAP_PROJ_INFO_FALSE_NORTHING_KEY,
        std::to_string(adfPrjParams[6]).c_str());

    // Init values
    if( !bHasRealCoordinates )
    {
        bHasRealCoordinates = nResolution < 0;
    }

    if( !bHasRealCoordinates )
    {
        dfScaleRatio = double(nScale) / nResolution;
        if( frameCoords[0] == 0 && frameCoords[1] == 0 && frameCoords[2] == 0 && 
            frameCoords[3] == 0 && frameCoords[4] == 0 && frameCoords[5] == 0 && 
            frameCoords[6] == 0 && frameCoords[7] == 0 )
        {
            bHasRealCoordinates = true;
        }
        else
        {
            dfXOr = projCoords[1] - frameCoords[1] * dfScaleRatio;
            dfYOr = projCoords[0] - frameCoords[0] * dfScaleRatio;
        }
    }

    OGRErr eErr = SetSRS(stProjInfo.nEllipsoidType, stProjInfo.nProjectionType, 
        stProjInfo.nHeightSrsType, eUnitInPlan, geogCoords, adfPrjParams, 
        papszOpenOpts);
    if( eErr != OGRERR_NONE )
    {
        return eErr;
    }
    
    // Read description ///////////////////////////////////////////////////////

    if( nVersion == 3 )
    {
        VSIFSeekL(fpSXF, 256, SEEK_SET);
        SXFDataDescriptorV3 stDataDescriptor;
        VSIFReadL(&stDataDescriptor, sizeof(SXFDataDescriptorV3), 1, fpSXF);
        CPL_LSBPTR32(&stDataDescriptor.nFeatureCount);
        nFeatureCount = stDataDescriptor.nFeatureCount;
    }
    else if( nVersion == 4 )
    {
        VSIFSeekL(fpSXF, 400, SEEK_SET);
        SXFDataDescriptorV4 stDataDescriptor;
        VSIFReadL(&stDataDescriptor, sizeof(SXFDataDescriptorV4), 1, fpSXF);
        CPL_LSBPTR32(&stDataDescriptor.nFeatureCount);
        nFeatureCount = stDataDescriptor.nFeatureCount;

        poDS->SetMetadataItem(MD_DESC_CLASSIFY_CODE_KEY, CPLSPrintf("%d", stDataDescriptor.nClassify));
        poDS->SetMetadataItem(MD_DESC_AUTO_GUID_KEY, CPLSPrintf("%d", stDataDescriptor.nAutoGenGUID));
        poDS->SetMetadataItem(MD_DESC_AUTO_TIMESTAMPS_KEY, CPLSPrintf("%d", stDataDescriptor.nAutoTimestamp));
        poDS->SetMetadataItem(MD_DESC_USE_ALT_FONTS_KEY, CPLSPrintf("%d", stDataDescriptor.nAltFonts));
    }
    /* else nOffset and nObjectsRead will be 0 */

    CPLDebug("SXF", "Total feature count %d", nFeatureCount);

    return OGRERR_NONE;
}
        
OGRErr SXFFile::Write(OGRSXFDataSource *poDS)
{
    return OGRERR_UNSUPPORTED_OPERATION;
}

OGRErr SXFFile::SetVertCS(const long iVCS, CSLConstList papszOpenOpts)
{
    const char *pszSetVertCS =
        CSLFetchNameValueDef(papszOpenOpts,
                             "SXF_SET_VERTCS",
                              CPLGetConfigOption("SXF_SET_VERTCS", "NO"));
    if( !CPLTestBool(pszSetVertCS) )
    {
        return OGRERR_NONE;
    }
    return pSpatRef->importVertCSFromPanorama(static_cast<int>(iVCS));
}


OGRErr SXFFile::SetSRS(const long iEllips, const long iProjSys, 
    const long iVCS, enum SXFCoordinateMeasureUnit eUnitInPlan, 
    double *padfGeoCoords, double *padfPrjParams, CSLConstList papszOpenOpts)
{
    if( nullptr != pSpatRef )
    {
        return SetVertCS(iVCS, papszOpenOpts);
    }

    // Normalize some coordintates systems
    if( (iEllips == 1 || iEllips == 0 ) && iProjSys == 1 ) // Pulkovo 1942 / Gauss-Kruger
    {
        double dfCenterLongEnv = GetCenter(padfGeoCoords[1], padfGeoCoords[5]);
        int nZoneEnv = GetZoneNumber(dfCenterLongEnv);
        if (nZoneEnv > 1 && nZoneEnv < 33)
        {
            int nEPSG = 28400 + nZoneEnv;
            pSpatRef = new OGRSpatialReference();
            pSpatRef->SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
            OGRErr eErr = pSpatRef->importFromEPSG(nEPSG);
            if (eErr != OGRERR_NONE)
            {
                return eErr;
            }
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
    else if( iEllips == 9 && iProjSys == 17 ) // WGS84 / UTM
    {
        double dfCenterLongEnv = GetCenter(padfGeoCoords[1], padfGeoCoords[5]);
        int nZoneEnv = 30 + GetZoneNumber(dfCenterLongEnv);
        bool bNorth = padfGeoCoords[6] + (padfGeoCoords[2] - padfGeoCoords[6]) / 2 < 0;
        int nEPSG = 0;
        if( bNorth )
        {
            nEPSG = 32600 + nZoneEnv;
        }
        else
        {
            nEPSG = 32700 + nZoneEnv;
        }
        pSpatRef = new OGRSpatialReference();
        pSpatRef->SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
        OGRErr eErr = pSpatRef->importFromEPSG(nEPSG);
        if (eErr != OGRERR_NONE)
        {
            return eErr;
        }
        return SetVertCS(iVCS, papszOpenOpts);
    }
    else if( iEllips == 45 && iProjSys == 35 ) //Mercator 3857 on sphere wgs84
    {
        pSpatRef = new OGRSpatialReference();
        pSpatRef->SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
        OGRErr eErr = pSpatRef->importFromEPSG(3857);
        if (eErr != OGRERR_NONE)
        {
            return eErr;
        }
        return SetVertCS(iVCS, papszOpenOpts);
    }
    else if( iEllips == 9 && iProjSys == 35 ) //Mercator 3395 on ellips wgs84
    {
        pSpatRef = new OGRSpatialReference();
        pSpatRef->SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
        OGRErr eErr = pSpatRef->importFromEPSG(3395);
        if (eErr != OGRERR_NONE)
        {
            return eErr;
        }
        return SetVertCS(iVCS, papszOpenOpts);
    }
    else if( iEllips == 9 && iProjSys == 34 ) //Miller 54003 on sphere wgs84
    {
        pSpatRef = new OGRSpatialReference("PROJCS[\"World_Miller_Cylindrical\",GEOGCS[\"GCS_GLOBE\", DATUM[\"GLOBE\", SPHEROID[\"GLOBE\", 6367444.6571, 0.0]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",0.017453292519943295]],PROJECTION[\"Miller_Cylindrical\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",0],UNIT[\"Meter\",1],AUTHORITY[\"ESRI\",\"54003\"]]");
        pSpatRef->SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
        return SetVertCS(iVCS, papszOpenOpts);
    }
    else if( iEllips == 9 && iProjSys == 33 && eUnitInPlan == SXF_COORD_MU_DEGREE )
    {
        pSpatRef = new OGRSpatialReference(SRS_WKT_WGS84_LAT_LONG);
        pSpatRef->SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
        return SetVertCS(iVCS, papszOpenOpts);
    }

    pSpatRef = new OGRSpatialReference();
    pSpatRef->SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
    OGRErr eErr = 
        pSpatRef->importFromPanorama(iProjSys, 0L, iEllips, padfPrjParams);
    if( eErr != OGRERR_NONE )
    {
        return eErr;
    }
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
