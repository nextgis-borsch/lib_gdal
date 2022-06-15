/******************************************************************************
 *
 * Project:  SXF Translator
 * Purpose:  Definition of classes for OGR SXF driver.
 * Author:   Ben Ahmed Daho Ali, bidandou(at)yahoo(dot)fr
 *           Dmitry Baryshnikov, polimax@mail.ru
 *
 ******************************************************************************
 * Copyright (c) 2011, Ben Ahmed Daho Ali
 * Copyright (c) 2013-2020, NextGIS <info@nextgis.com>
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

#include "cpl_conv.h"
#include "ogr_sxf.h"

CPL_CVSID("$Id$")

extern "C" void RegisterOGRSXF();

/************************************************************************/
/*                       ~OGRSXFDriver()                         */
/************************************************************************/

OGRSXFDriver::~OGRSXFDriver()
{
}

/************************************************************************/
/*                                Open()                                */
/************************************************************************/

GDALDataset *OGRSXFDriver::Open(GDALOpenInfo *poOpenInfo)

{
/* -------------------------------------------------------------------- */
/*      Determine what sort of object this is.                          */
/* -------------------------------------------------------------------- */

    VSIStatBufL sStatBuf;
    if (!EQUAL(CPLGetExtension(poOpenInfo->pszFilename), "sxf") ||
        VSIStatL(poOpenInfo->pszFilename, &sStatBuf) != 0 ||
        !VSI_ISREG(sStatBuf.st_mode))
        return nullptr;

    OGRSXFDataSource *poDS = new OGRSXFDataSource();

    if( !poDS->Open( poOpenInfo->pszFilename,
                     poOpenInfo->eAccess == GA_Update,
                     poOpenInfo->papszOpenOptions ) )
    {
        delete poDS;
        poDS = nullptr;
    }

    return poDS;
}

/************************************************************************/
/*                              Identify()                              */
/************************************************************************/

int OGRSXFDriver::Identify(GDALOpenInfo *poOpenInfo)
{
    if (!EQUAL(CPLGetExtension(poOpenInfo->pszFilename), "sxf") ||
        !poOpenInfo->bStatOK || poOpenInfo->bIsDirectory)
    {
        return GDAL_IDENTIFY_FALSE;
    }
        
    if(poOpenInfo->nHeaderBytes < 4)
    {
        return GDAL_IDENTIFY_UNKNOWN;
    }
    
    if(0 != memcmp(poOpenInfo->pabyHeader, "SXF", 3))
    {
        return GDAL_IDENTIFY_FALSE;
    }

    return GDAL_IDENTIFY_TRUE;
}

/************************************************************************/
/*                           DeleteDataSource()                         */
/************************************************************************/

CPLErr OGRSXFDriver::DeleteDataSource(const char *pszName)
{
    //TODO: add more extensions if applicable
    static const char * const apszExtensions[] = { 
        "szf", "rsc", "SZF", "RSC", nullptr 
    };

    VSIStatBufL sStatBuf;
    if (VSIStatL(pszName, &sStatBuf) != 0)
    {
        CPLError(CE_Failure, CPLE_AppDefined,
            "%s does not appear to be a valid sxf file.",
            pszName);

        return CE_Failure;
    }

    for( int iExt = 0; apszExtensions[iExt] != nullptr; iExt++ )
    {
        const char *pszFile = CPLResetExtension(pszName,
            apszExtensions[iExt]);
        if (VSIStatL(pszFile, &sStatBuf) == 0)
            VSIUnlink(pszFile);
    }

    return CE_None;
}

/************************************************************************/
/*                               Create()                               */
/************************************************************************/

GDALDataset *OGRSXFDriver::Create(const char *pszName,
    int /* nBands */,
    int /* nXSize */,
    int /* nYSize */,
    GDALDataType /* eDT */,
    char **papszOptions)
{
    OGRSXFDataSource *poDS = new OGRSXFDataSource();

    if (!poDS->Create(pszName, papszOptions))
    {
        delete poDS;
        poDS = nullptr;
    }

    return poDS;
}


/************************************************************************/
/*                        RegisterOGRSXF()                       */
/************************************************************************/
void RegisterOGRSXF()
{
    if( GDALGetDriverByName( "SXF" ) != nullptr )
        return;

    OGRSXFDriver* poDriver = new OGRSXFDriver;

    poDriver->SetDescription( "SXF" );
    poDriver->SetMetadataItem( GDAL_DCAP_VECTOR, "YES" );
    poDriver->SetMetadataItem( GDAL_DMD_LONGNAME,
                               "Storage and eXchange Format" );
    poDriver->SetMetadataItem( GDAL_DMD_HELPTOPIC, "drv_sxf.html" );
    poDriver->SetMetadataItem( GDAL_DMD_EXTENSION, "sxf" );
    poDriver->SetMetadataItem( GDAL_DCAP_VIRTUALIO, "YES" );
    poDriver->SetMetadataItem( GDAL_DMD_OPENOPTIONLIST,
        "<OpenOptionList>"
        "  <Option name='SXF_LAYER_FULLNAME' type='boolean' description='Use long layer names' default='NO'/>"
        "  <Option name='SXF_RSC_FILENAME' type='string' description='RSC file name' default=''/>"
        "  <Option name='SXF_SET_VERTCS' type='boolean' description='Layers spatial reference will include vertical coordinate system description if exist' default='NO'/>"
        "  <Option name='SXF_NEW_BEHAVIOR' type='boolean' description='New behavior - vector object to lines, empty layers are presence' default='NO'/>"
        "  <Option name='SXF_ENCODING' type='string' description='Character Encodings (ASCIIZ for format v3 and ANSI code page for format v4)' default=''/>"
        "  <Option name='SXF_WRITE_RSC' type='boolean' description='Write RSC file. Always write file with same name as SXF but with RSC extension' default='YES'/>"
        "</OpenOptionList>");
    poDriver->SetMetadataItem(GDAL_DMD_CREATIONFIELDDATATYPES,
        "Integer Real String IntegerList RealList StringList");
    poDriver->SetMetadataItem(GDAL_DMD_CREATIONOPTIONLIST,
        "<CreationOptionList>"
        "  <Option name='SXF_ENCODING' type='string' description='Character Encodings (Only format v4 and ANSI code page supported)' default='CP1251'/>"
        "  <Option name='SXF_WRITE_RSC' type='boolean' description='Write RSC file' default='YES'/>"
        "  <Option name='SXF_MAP_NAME' type='string' description='Override metadata item SHEET_NAME' default=''/>"
        "  <Option name='SXF_SHEET_KEY' type='string' description='Override metadata item SHEET' default=''/>"
        "  <Option name='SXF_MAP_SCALE' type='int' description='Override metadata item SCALE' default='1000000'/>"
        "</CreationOptionList>");
    poDriver->SetMetadataItem(GDAL_DS_LAYER_CREATIONOPTIONLIST,
        "<LayerCreationOptionList>"
        "  <Option name='SXF_NEW_BEHAVIOR' type='boolean' description='New behavior - vector object to lines, empty layers are presence' default='NO'/>"
        "</LayerCreationOptionList>");

    poDriver->pfnOpen = OGRSXFDriver::Open;
    poDriver->pfnDelete = OGRSXFDriver::DeleteDataSource;
    poDriver->pfnIdentify = OGRSXFDriver::Identify;
    poDriver->pfnCreate = OGRSXFDriver::Create;

    GetGDALDriverManager()->RegisterDriver( poDriver );
}
