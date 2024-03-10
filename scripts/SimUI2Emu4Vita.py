import sqlite3
import os
from pathlib import Path
import argparse
import zipfile
import shutil
import re

# https://www.simui.net/


class DB(dict):
    def __init__(self, db_name='data.dll'):
        con = sqlite3.connect(db_name)
        con.row_factory = sqlite3.Row
        cur = con.cursor()

        cur.execute('select * from platform')
        self.platform = cur.fetchone()

        cur.execute('select * from rom')
        result = cur.fetchall()
        for row in result:
            self[row['file_md5']] = row

    def get_image_path(self, file_md5, image_type='cassette'):
        row = self[file_md5]
        if 'pname' in row or len(row['pname']) > 0:
            return self.get_image_path(self[row['pname']]['file_md5'])

        for key in ('base_name_en', 'name', 'rom_path'):
            for ext in ('.png', '.jpg'):
                pure_name = Path(row[key]).stem
                image_path = f'{self.platform[image_type+"_path"]}/{pure_name}{ext}'
                if os.path.exists(image_path):
                    return image_path

    def get_rom_path(self, file_md5):
        return f'{self.platform["rom_path"]}/{self[file_md5]["rom_path"]}'


def try_make_dirs(p):
    p = Path(p).parent
    try:
        os.makedirs(p)
    except:
        pass


def legal_file_name(n):
    return re.sub(r'[<>:"/\\|?*]', '', n)


def convert(db, output, compress=False, image_type='cassette'):
    for md5, row in db.items():
        print(row['rom_path'])
        new_path = Path(output) / Path(row['rom_path']).parent
        image_path = db.get_image_path(md5, image_type)
        if image_path:
            new_image_path = str(new_path / 'images' / row['name']) + Path(image_path).suffix
            try_make_dirs(new_image_path)
            shutil.copy(image_path, new_image_path)

        rom_path = db.get_rom_path(md5)
        if compress and Path(row['rom_path']).suffix.lower() not in ('.zip', '.7z', '.rar', '.chd', '.iso'):
            zip_path = str(new_path / row['name']) + '.zip'
            zipfile.ZipFile(zip_path, 'w', compression=zipfile.ZIP_DEFLATED).writestr(
                Path(row['rom_path']).stem, open(rom_path, 'rb').read()
            )
        else:
            new_rom_path = str(new_path / row['name']) + Path(row['rom_path']).suffix
            try_make_dirs(new_rom_path)
            shutil.copy(rom_path, new_rom_path)


if __name__ == '__main__':
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument('output', action="store", nargs=1)
    parser.add_argument('--compress', action="store_true", default=False)
    parser.add_argument(
        '--image_type',
        action="store",
        default='cassette',
        choices=['cassette', 'icon', 'packing', 'poster', 'thumbs', 'title'],
    )
    args = parser.parse_args()

    db = DB()
    convert(db, args.output[0], args.compress, args.image_type)
