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
    os.makedirs(to_folder)
    files = glob.glob(src + "/*.*")
    for f in files:
        if not os.path.isdir(f):
            file_extension = os.path.splitext(f)[1].replace('.','')
            if file_extension in exts:
                shutil.copy(f, dest)


start = '/media/sim/Windows7_OS/work/lib_gdal/'
finish = '/media/sim/Windows7_OS/work/lib_gdal_new/'

mappings = read_mappings('/home/sim/work/gdal_folders.csv')

for row in mappings:

    from_folder = row['old']
    to_folder = row['new']
    action = row['action']
    exts = row['ext2keep']
    
    if action == 'skip':
        continue
    else:
        from_folder = start + from_folder
        to_folder = finish + to_folder
        copy_dir(from_folder, to_folder, exts)
    print(from_folder + ' ... processed' )