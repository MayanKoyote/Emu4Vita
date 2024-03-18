from base import *
import sqlite3
from pathlib import Path
import os


class SimUI(Base):
    DATA_PATTERN = 'data.dll'
    VALID_IMAGE_TYPES = ['cassette', 'icon', 'packing', 'poster', 'thumbs', 'title']

    def load(self, data_path):
        con = sqlite3.connect(data_path)
        con.row_factory = sqlite3.Row
        cur = con.cursor()

        cur.execute('select * from platform')
        self.platform = cur.fetchone()

        cur.execute('select * from rom')
        result = cur.fetchall()
        for row in result:
            self[row['file_md5']] = row

    def get_rom_path(self, key):
        return f'{self.path}/{self.platform["rom_path"]}/{self[key]["rom_path"]}'

    def get_image_path(self, key, image_type):
        self._check_image_type(image_type)

        row = self[key]
        if 'pname' in row or len(row['pname']) > 0:
            return self.get_image_path(self[row['pname']]['file_md5'], image_type)

        for key in ('base_name_en', 'name', 'rom_path'):
            for ext in ('.png', '.jpg'):
                pure_name = Path(row[key]).stem
                image_path = f'{self.path}/{self.platform[image_type+"_path"]}/{pure_name}{ext}'
                if os.path.exists(image_path):
                    return image_path

    def get_rom_name(self, key):
        return legal_file_name(self[key]['name'])
