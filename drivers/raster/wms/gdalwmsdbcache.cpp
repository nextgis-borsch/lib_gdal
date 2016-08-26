/******************************************************************************
 * $Id$
 *
 * Project:  WMS Client Driver
 * Purpose:  Implementation of Dataset and RasterBand classes for WMS
 *           and other similar services.
 * Author:   Adam Nowacki, nowak@xpam.de
 *
 ******************************************************************************
 * Copyright (c) 2007, Adam Nowacki
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

#include "wmsdriver.h"

#ifdef SQLITE_ENABLED

#define CACHE_DB "cache.db"

GDALWMSDbCache::GDALWMSDbCache() : GDALWMSCache () {
//    m_hDB = NULL;
}

GDALWMSDbCache::~GDALWMSDbCache() {
//    sqlite3_close(m_hDB);
//    m_hDB = NULL;
}

CPLErr GDALWMSDbCache::Initialize(CPLXMLNode *config, CPLXMLNode *service_config) {

    CPLErr error = GDALWMSCache::Initialize (config, service_config);
    if(CE_None != error)
        return error;

    m_cache_path = CPLFormFilename (m_cache_path, CACHE_DB, NULL);

    return CE_None;
}

CPLErr GDALWMSCache::Write(const char *key, const CPLString &file_name) {

    return CE_None;
}

CPLErr GDALWMSCache::Read(const char *key, CPLString *file_name) {

    return CE_None;
}

/*
{
SQLCommand(m_hTempDB, "PRAGMA synchronous = OFF");
        SQLCommand(m_hTempDB, (CPLString("PRAGMA journal_mode = ") + CPLGetConfigOption("PARTIAL_TILES_JOURNAL_MODE", "OFF")).c_str());
        SQLCommand(m_hTempDB, "CREATE TABLE partial_tiles("
                                    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                    "zoom_level INTEGER NOT NULL,"
                                    "tile_column INTEGER NOT NULL,"
                                    "tile_row INTEGER NOT NULL,"
                                    "tile_data_band_1 BLOB,"
                                    "tile_data_band_2 BLOB,"
                                    "tile_data_band_3 BLOB,"
                                    "tile_data_band_4 BLOB,"
                                    "partial_flag INTEGER NOT NULL,"
                                    "age INTEGER NOT NULL,"
                                    "UNIQUE (zoom_level, tile_column, tile_row))" );
        SQLCommand(m_hTempDB, "CREATE INDEX partial_tiles_partial_flag_idx "
                                "ON partial_tiles(partial_flag)");
        SQLCommand(m_hTempDB, "CREATE INDEX partial_tiles_age_idx "
                                "ON partial_tiles(age)");
}
*/

#endif //SQLITE_ENABLED
