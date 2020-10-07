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
 * Copyright (c) 2013-2020, NextGIS
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

#define TO_DEGREES 57.2957795130823208766
#define TO_RADIANS 0.017453292519943295769

class OGRSXFDataSource;

/************************************************************************/
/*                           SXFFile                                    */
/************************************************************************/
class SXFFile
{
    public:
        SXFFile();
        ~SXFFile();
        bool Open(const std::string &osPath);
        void Close();
        VSILFILE *File() const;
        CPLMutex **Mutex();
        GUInt32 FeatureCount() const;
        OGRErr Read(OGRSXFDataSource *poDS, CSLConstList papszOpenOpts);
        OGRErr Write(OGRSXFDataSource *poDS);
        GUInt32 Version() const;
        OGRSpatialReference *SpatialRef() const;
        OGRErr FillExtent(OGREnvelope *env) const;
        void TranslateXY(double x, double y, double *dfX, double *dfY) const;
        std::string Encoding() const;

    // Static
    public:
        static std::string ReadSXFString(const void *pBuffer, size_t nLen, 
            const char *pszSrcEncoding);
        static SXFGeometryType CodeToGeometryType(GByte nType);

    private:
        OGRErr SetSRS(const long iEllips, const long iProjSys, const long iVCS, 
            enum SXFCoordinateMeasureUnit eUnitInPlan, double *padfGeoCoords,
            double *padfPrjParams, CSLConstList papszOpenOpts);
        OGRErr SetVertCS(const long iVCS, CSLConstList papszOpenOpts);

    private:
        CPL_DISALLOW_COPY_ASSIGN(SXFFile)

    private:
        VSILFILE *fpSXF = nullptr;
        CPLMutex *hIOMutex = nullptr;

    private:        
        GUInt32 nVersion = 0;
        double dfScaleRatio = 1.0;
        double dfXOr = 0.0;
        double dfYOr = 0.0;
        bool bHasRealCoordinates = true; 
        OGRSpatialReference *pSpatRef = nullptr;
        OGREnvelope oEnvelope;
        std::string osEncoding = "CP1251";
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
        OGRErr Read(const std::string &osPath, CSLConstList papszOpenOpts);

    public:
        std::map<GByte, SXFLayerDefn> mstLayers;

    // static
    public:
        static std::map<GByte, SXFLayerDefn> GetDefaultLayers();

    private:
        CPL_DISALLOW_COPY_ASSIGN(RSCFile)
};

/************************************************************************/
/*                         OGRSXFLayer                                */
/************************************************************************/
class OGRSXFLayer final: public OGRLayer
{
protected:
    typedef struct{
        vsi_l_offset offset;
        int group;
        int subObject;
        OGRFeature *poFeature;
    } SXFFeature;
    OGRFeatureDefn *poFeatureDefn;
    SXFFile *fpSXF;
    GUInt16 nLayerID;
    std::map<GUInt32, std::string> mnClassificators;
    std::map<GIntBig, SXFFeature> mnRecordDesc;
    std::map<GIntBig, SXFFeature>::const_iterator oNextIt;
    std::set<GUInt32> snAttributeCodes;
    std::set<GIntBig> anCacheIds;
    std::string osFIDColumn;
    bool bIsNewBehavior;

    virtual OGRFeature *GetNextRawFeature(GIntBig nFID, int nGroupId, int nSubObject);

    GUInt32 TranslateXYH(const SXFRecordHeader &header, GByte *psBuff, 
        GUInt32 nBufLen, double *dfX, double *dfY, double *dfH = nullptr);
    GUInt32 TranslateLine(OGRLineString* poLS, const SXFRecordHeader &header, 
        GByte *pBuff, GUInt32 nBuffSize, GUInt32 nPointCount);
    GUInt32 TranslatePoint(OGRPoint *poPT, const SXFRecordHeader &header, 
        GByte *pBuff, GUInt32 nBuffSize);

    OGRFeature *TranslatePoint(const SXFRecordHeader &stHeader, GByte *psRecordBuf);
    OGRFeature *TranslateText(const SXFRecordHeader &header, GByte *psRecordBuf, int nSubObject);
    OGRFeature *TranslatePolygon(const SXFRecordHeader &header, GByte *psRecordBuf);
    OGRFeature *TranslateLine(const SXFRecordHeader &stHeader, GByte *psRecordBuf);
    OGRFeature *TranslateVetorAngle(const SXFRecordHeader &header, GByte *psRecordBuf);
    void AddToCache(GIntBig nFID, OGRFeature *poFeature);
    void DeleteCachedFeature(GIntBig nFID);
    void AddValue(OGRFeature *poFeature, const std::string &osFieldName, const std::string &value);
    void AddValue(OGRFeature *poFeature, const std::string &osFieldName, int value);
    void AddValue(OGRFeature *poFeature, const std::string &osFieldName, double value);
    bool IsFieldList(int nIndex) const;
public:
    OGRSXFLayer(SXFFile *fp, GUInt16 nID, const char *pszLayerName, 
        const std::vector<SXFField> &astFields, bool bIsNewBehavior);
    virtual ~OGRSXFLayer();

    virtual void ResetReading() override;
    virtual OGRFeature *GetNextFeature() override;
    virtual OGRErr SetNextByIndex(GIntBig nIndex) override;
    virtual OGRFeature *GetFeature(GIntBig nFID) override;
    virtual OGRFeatureDefn *GetLayerDefn() override;

    virtual int TestCapability( const char * ) override;

    virtual GIntBig GetFeatureCount(int bForce = TRUE) override;
    virtual OGRErr GetExtent(OGREnvelope *psExtent, int bForce = TRUE) override;
    virtual OGRErr GetExtent(int iGeomField, OGREnvelope *psExtent, 
        int bForce) override;
    virtual OGRSpatialReference *GetSpatialRef() override;
    virtual const char *GetFIDColumn() override;

    GByte GetId() const;
    void AddClassifyCode(GUInt32 nClassCode, const std::string &soName);
    bool AddRecord( GIntBig nFID, unsigned nClassCode, vsi_l_offset nOffset,
        bool bHasSemantic, size_t nSemanticsSize, int nGroup = 0,
        int nSubObjectId = 0 );
};

/************************************************************************/
/*                            OGRSXFDataSource                          */
/************************************************************************/

class OGRSXFDataSource final: public OGRDataSource
{
    SXFFile oSXFFile;

    CPLString pszName;

    std::vector<OGRLayer*> poLayers;

    void FillLayers(bool bIsNewBehavior);
    void CreateLayers(const std::map<GByte, SXFLayerDefn>& mstLayers, 
        bool bIsNewBehavior);
    OGRSXFLayer *GetLayerById(GByte nId);

public:
    OGRSXFDataSource();
    virtual ~OGRSXFDataSource();

    int Open(const char *pszFilename, bool bUpdate,
             CSLConstList papszOpenOpts = nullptr );

    virtual const char *GetName() override;
    virtual int GetLayerCount() override;
    virtual OGRLayer *GetLayer( int ) override;
    virtual int TestCapability( const char * ) override;
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
    static CPLErr DeleteDataSource(const char *pszName);
};

#endif
