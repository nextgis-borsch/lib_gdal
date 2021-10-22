/******************************************************************************
 * $Id$
 *
 * Project:  SXF Translator
 * Purpose:  Include file defining classes for OGR SXF driver, datasource and layers.
 * Author:   Ben Ahmed Daho Ali, bidandou(at)yahoo(dot)fr
 *           Dmitry Baryshnikov, polimax@mail.ru
 *           Alexandr Lisovenko, alexander.lisovenko@gmail.com
 *
 ******************************************************************************
 * Copyright (c) 2011, Ben Ahmed Daho Ali
 * Copyright (c) 2013-2021, NextGIS
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

#ifndef OGR_SXF_H_INCLUDED
#define OGR_SXF_H_INCLUDED

#include <set>
#include <vector>
#include <map>

#include "ogrsf_frmts.h"
#include "org_sxf_defs.h"
#include "../mem/ogr_mem.h"

constexpr double TO_DEGREES = 57.2957795130823208766;
constexpr double TO_RADIANS = 0.017453292519943295769;
constexpr int EXTRA_ID = 1000000;

class OGRSXFDataSource;

namespace SXF {
    size_t WriteEncString(const char *pszSrcText, GByte *pDst,
        int nSize, const char *pszEncoding);
    void WriteEncString(const char *pszText, int nSize,
        const char *pszEncoding, VSILFILE *fpSXF);
    std::string ReadEncString(const void *pBuffer, size_t nLen,
        const char *pszSrcEncoding);
} // namespace SXF

/************************************************************************/
/*                            SXFLimits                                 */
/************************************************************************/
class SXFLimits
{
    public:
        typedef struct {
            double dfStart;
            double dfEnd;
            int nExt;
        } Range;

    public:
        explicit SXFLimits(int nSC1In = -1, int nSC2In = -1);
        void AddRange(double dfStart, double dfEnd, int nExt);
        void SetDefaultExt(int nExt);
        int GetExtention(double dfnSC1Val, double dfSC2Val) const;
        std::pair<int, int> GetLimitCodes() const;
    private:
		int nSC1, nSC2;
		std::vector<Range> aRanges;
        int nDefaultExt = 0;
};

/************************************************************************/
/*                          SXFLayerDefn                                */
/************************************************************************/
class SXFLayerDefn 
{
	public:
		explicit SXFLayerDefn(int nLayerID = 0, const std::string &osLayerName = "");
		//int GetExtentionNo(const std::string &osCode, const std::pair<int, int> &nSCValues) const;
		bool HasCode(const std::string &osCode) const;
		void AddCode(SXFClassCode stCode);
		bool HasField(GUInt32 nSemCode) const;
		void AddField(SXFField stField);
        void AddLimits(const std::string &osCode, const SXFLimits &oLimIn);
		std::string GetName() const;
		std::vector<SXFClassCode> GetCodes(bool bForce = false);
		std::vector<SXFField> GetFields() const;
        SXFLimits GetLimits(const std::string &osCode) const;
        std::string GetCodeName(const std::string &osCode, int nExt) const;
        GUInt32 GenerateCode(SXFGeometryType eGeometryType) const;
	
	private:
		int nID;
		std::string osName;
		std::vector<SXFClassCode> astCodes;
		std::vector<SXFField> astFields;
		std::map<std::string, SXFLimits> mLim;
};

/************************************************************************/
/*                           SXFFile                                    */
/************************************************************************/
class SXFFile
{
    public:
        SXFFile();
        ~SXFFile();
        bool Open(const std::string &osPath, bool bReadOnly, 
            const std::string &osCodePage);
        void Close();
        VSILFILE *File() const;
        GUInt32 FeatureCount() const;
        bool Read(OGRSXFDataSource *poDS, CSLConstList papszOpenOpts);
        bool Write(OGRSXFDataSource *poDS);
        GUInt32 Version() const;
        OGRSpatialReference *SpatialRef() const;
        OGREnvelope Extent() const;
        void TranslateXY(double x, double y, double *dfX, double *dfY) const;
        std::string Encoding() const;
        bool WriteCheckSum();
        bool WriteTotalFeatureCount(GUInt32 nTotalFeatureCount);
        bool CheckSum() const;

    // Static
    public:
        static SXFGeometryType CodeToGeometryType(GByte nType);
        static std::string SXFTypeToString(enum SXFGeometryType eType);
        static enum SXFGeometryType StringToSXFType(const std::string &type);
        static std::string ToStringCode(enum SXFGeometryType eType, 
            GUInt32 nClassifyCode);
        static GUInt32 CodeForGeometryType(enum SXFGeometryType eGeomType);

    private:
        OGRErr SetSRS(const long iEllips, const long iProjSys, const long iCS, 
            const long iVCS, enum SXFCoordinateMeasureUnit eUnitInPlan, 
            double *padfGeoCoords, double *padfPrjParams, 
            CSLConstList papszOpenOpts);
        OGRErr SetVertCS(const long iVCS, CSLConstList papszOpenOpts);

    private:
        CPL_DISALLOW_COPY_ASSIGN(SXFFile)

    private:
        VSILFILE *fpSXF = nullptr;

    private:        
        GUInt32 nVersion = 0;
        double dfScaleRatio = 1.0;
        double dfXOr = 0.0;
        double dfYOr = 0.0;
        bool bHasRealCoordinates = true; 
        OGRSpatialReference *pSpatRef = nullptr;
        OGREnvelope oEnvelope;
        std::string osEncoding = "";
        GUInt32 nFeatureCount = 0;
};

/************************************************************************/
/*                           RSCFile                                    */
/************************************************************************/
class RSCFile
{
    public:
        RSCFile();
        ~RSCFile();
        bool Read(const std::string &osPath, CSLConstList papszOpenOpts);
        bool Write(const std::string &osPath, OGRSXFDataSource *poDS, 
            const std::string &osEncoding, 
            const std::map<std::string, int> &mnClassMap);
		std::map<GByte, SXFLayerDefn> GetLayers() const;

    private:
        std::map<GByte, SXFLayerDefn> mstLayers;

    private:
        CPL_DISALLOW_COPY_ASSIGN(RSCFile)
};

/************************************************************************/
/*                         OGRSXFLayer                                */
/************************************************************************/
class OGRSXFLayer final: public OGRMemLayer
{
public:
    explicit OGRSXFLayer(OGRSXFDataSource *poDSIn, 
        const SXFLayerDefn &oSXFDefn, bool bIsNewBehavior);
    virtual ~OGRSXFLayer() override = default;

    virtual int TestCapability( const char * ) override;
    virtual OGRErr SyncToDisk() override;

    virtual OGRErr GetExtent(OGREnvelope *psExtent, int bForce = TRUE) override;
    virtual OGRErr GetExtent(int iGeomField, OGREnvelope *psExtent, 
        int bForce) override;
    OGRErr ISetFeature(OGRFeature *poFeature) override;
    OGRErr ICreateFeature(OGRFeature *poFeature) override;
    virtual OGRErr DeleteFeature(GIntBig nFID) override;
    OGRErr CreateField(OGRFieldDefn *poField, int bApproxOK = FALSE) override;

	std::vector<SXFClassCode> GetClassifyCodes();
    bool AddRecord(GIntBig nFID, const std::string &osClassCode, const SXFFile &oSXF,
        vsi_l_offset nOffset, int nGroupID = 0, int nSubObjectID = 0);
    int Write(const SXFFile &oSXF, const std::map<std::string, int> &mnClassCodes);

public: // static
    static std::string OGRFieldTypeToString(const OGRFieldType type);
    static std::string CreateFieldKey(OGRFieldDefn *poFld);
    static int GetFieldNameCode(const char * pszFieldName);
    static bool IsFieldNameHasCode(const char *pszFieldName);

private:
    OGRFeature *GetRawFeature(const SXFFile &oSXF, int nGroupID, int nSubObjectID);
    int SetRawFeature(OGRFeature *poFeature, OGRGeometry *poGeom,
        const SXFFile &oSXF, const std::map<std::string, int> &mnClassCodes);
    GUInt32 TranslateXYH(const SXFFile &oSXF, const SXFRecordHeader &header, 
        GByte *psBuff, GUInt32 nBufLen, double *dfX, double *dfY, double *dfH = nullptr);
    GUInt32 TranslateLine(const SXFFile &oSXF, OGRLineString* poLS, 
        const SXFRecordHeader &header, GByte *pBuff, GUInt32 nBuffSize, 
        GUInt32 nPointCount);
    GUInt32 TranslatePoint(const SXFFile &oSXF, OGRPoint *poPT, 
        const SXFRecordHeader &header, GByte *pBuff, GUInt32 nBuffSize);

    OGRFeature *TranslatePoint(const SXFFile &oSXF,  const SXFRecordHeader &stHeader,
        GByte *psRecordBuf);
    OGRFeature *TranslateText(const SXFFile &oSXF, const SXFRecordHeader &header, 
        GByte *psRecordBuf, int nSubObject);
    OGRFeature *TranslatePolygon(const SXFFile &oSXF, const SXFRecordHeader &header,
        GByte *psRecordBuf);
    OGRFeature *TranslateLine(const SXFFile &oSXF, const SXFRecordHeader &stHeader,
        GByte *psRecordBuf);
    OGRFeature *TranslateVetorAngle(const SXFFile &oSXF, const SXFRecordHeader &header, 
        GByte *psRecordBuf);
    void AddValue(OGRFeature *poFeature, const std::string &osFieldName, 
        const std::string &value);
    void AddValue(OGRFeature *poFeature, const std::string &osFieldName, 
        int value);
    void AddValue(OGRFeature *poFeature, const std::string &osFieldName, 
        double value);

private:
    OGRSXFDataSource *poDS = nullptr;
    bool bIsNewBehavior;
	SXFLayerDefn oSXFLayerDefn;
};

/************************************************************************/
/*                            OGRSXFDataSource                          */
/************************************************************************/

class OGRSXFDataSource final: public OGRDataSource
{
public:
    OGRSXFDataSource();
    virtual ~OGRSXFDataSource();

    int Open(const char *pszFilename, bool bUpdate,
        CSLConstList papszOpenOpts = nullptr );
    int Create(const char *pszFilename, 
        CSLConstList papszOpenOpts = nullptr);
    OGRErr GetExtent(OGREnvelope *psExtent) const;
    void UpdateExtent(const OGREnvelope &env);
    void SetHasChanges();
    std::string Encoding() const;

    virtual const char *GetName() override;
    virtual int GetLayerCount() override;
    virtual OGRLayer *GetLayer( int ) override;
    virtual int TestCapability( const char * ) override;
    virtual void FlushCache(void) override;
    virtual const OGRSpatialReference *GetSpatialRef() const override;
    virtual CPLErr SetSpatialRef(const OGRSpatialReference *poSRS) override;
    virtual char **GetFileList(void) override; 
    virtual OGRErr DeleteLayer(int iLayer) override;

protected:
    virtual OGRLayer *ICreateLayer(const char *pszName,
        OGRSpatialReference *poSpatialRef = nullptr,
        OGRwkbGeometryType eGType = wkbUnknown,
        char ** papszOptions = nullptr) override;

private:
    void FillLayers(const SXFFile &oSXFFile, bool bIsNewBehavior);
    void CreateLayers(const OGREnvelope &oEnv, 
        const std::map<GByte, SXFLayerDefn> &mstLayers, bool bIsNewBehavior);
    std::map<std::string, int> GenerateSXFClassMap() const;

private:
    OGREnvelope oExtent;
    std::vector<OGRLayer*> poLayers;
    OGRSpatialReference *poSpatialRef = nullptr;
    CPLStringList aosFileList;
    bool bHasChanges = false;
    std::string osEncoding = "";
    bool bWriteRSC = true;
};

/************************************************************************/
/*                         OGRSXFDriver                          */
/************************************************************************/

class OGRSXFDriver final: public GDALDriver
{
  public:
    ~OGRSXFDriver();

    static GDALDataset *Open( GDALOpenInfo * );
    static int Identify( GDALOpenInfo * );
    static CPLErr DeleteDataSource( const char * );
    static GDALDataset *Create( const char *, int, int, int, GDALDataType, char ** );
};

#endif
