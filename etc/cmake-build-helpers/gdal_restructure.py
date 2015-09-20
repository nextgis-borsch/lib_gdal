#!/usr/bin/env python
#******************************************************************************
# 
#  Project:  GDAL
#  Purpose:  Script to one folder structure of GDAL (old) to another (new).
#  Author:   Maxim Dubinin, sim@gis-lab.info
#  Example:  python gdal_restructure.py /media/sim/Windows7_OS/work/gdal_cmake/ /media/sim/Windows7_OS/work/lib_gdal_new/ /home/sim/work/lib_gdal/etc/cmake-build-helpers/gdal_folders.csv
#******************************************************************************
#  Copyright (c) 2015, Maxim Dubinin
# 
#  Permission is hereby granted, free of charge, to any person obtaining a
#  copy of this software and associated documentation files (the "Software"),
#  to deal in the Software without restriction, including without limitation
#  the rights to use, copy, modify, merge, publish, distribute, sublicense,
#  and/or sell copies of the Software, and to permit persons to whom the
#  Software is furnished to do so, subject to the following conditions:
# 
#  The above copyright notice and this permission notice shall be included
#  in all copies or substantial portions of the Software.
# 
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
#  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#  DEALINGS IN THE SOFTWARE.
#******************************************************************************

import sys
import os
import shutil
import glob
import csv

def read_mappings(csv_path):
    fieldnames_data = ('old','new','action','ext2keep')
    f_csv = open(csv_path)
    csvreader = csv.DictReader(f_csv, fieldnames=fieldnames_data)

    return csvreader

def copyonly(dirpath, contents):
     ignore_list = set(contents) - set(shutil.ignore_patterns(*exts_to_keep)(dirpath, contents),)
     ignore_list2 = list(ignore_list)
     for item in ignore_list:
        if os.path.isdir(dirpath + '/' + item): ignore_list2.remove(item)

     return ignore_list2
 
def copy_dir(src, dest, exts):
    if not os.path.exists(to_folder):
        os.makedirs(to_folder)

    files = glob.glob(src + "/*.*")
    for f in files:
        if not os.path.isdir(f):
            file_extension = os.path.splitext(f)[1].replace('.','')
            if file_extension in exts:
                shutil.copy(f, dest)


start_folder = sys.argv[1]         #where to copy from
finish_folder = sys.argv[2]        #where to copy to
if not os.path.exists(finish_folder): os.makedirs(finish_folder)
mappings_csv = sys.argv[3]         #csv with folder mappings

mappings = read_mappings(mappings_csv)

for row in mappings:

    from_folder = row['old']
    to_folder = row['new']
    action = row['action']
    exts = row['ext2keep']
    from_folder = start_folder + from_folder
    to_folder = finish_folder + to_folder
    
    if os.path.exists(from_folder):
        if action == 'skip':
            continue
        else:
            copy_dir(from_folder, to_folder, exts)
        print(from_folder + ' ... processed' )