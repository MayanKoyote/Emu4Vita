from pathlib import Path
import sqlite3
import os
import re
import zipfile
import shutil


def try_make_dirs(path):
    try:
        os.makedirs(Path(path).parent)
    except:
        pass


def legal_file_name(name):
    return re.sub(r'[<>:"/\\|?*]', '', name)


def is_compressed_file(path):
    return Path(path).suffix.lower() in ('.zip', '.7z', '.rar', '.chd', '.iso')


class Base(dict):
    VALID_IMAGE_TYPES = []

    def __init__(self, data_path):
        self.path = Path(data_path)
        self.load(data_path)

    def load(self, data_path):
        raise NotImplementedError

    def get_rom_path(self, key):
        raise NotImplementedError

    def get_image_path(self, key, image_type):
        raise NotImplementedError

    def get_rom_name(self, key):
        raise NotImplementedError

    def convert(self, output_path, image_type, compress=False):
        for key in self.keys():
            rom_path = self.get_rom_path(key)
            image_path = self.get_image_path(key, image_type)
            rom_name = self.get_rom_name(key)

            new_path = Path(output_path) / Path(rom_path).parent
            if image_path:
                new_image_path = str(new_path / 'images' / rom_name) + Path(image_path).suffix
                print(f'{image_path} ==> {new_image_path}')
                try_make_dirs(new_image_path)
                shutil.copy(image_path, new_image_path)

            if compress and not is_compressed_file(rom_path):
                zip_path = str(new_path / rom_name) + '.zip'
                print(f'{rom_path} ==> {zip_path}')
                try_make_dirs(zip_path)
                zipfile.ZipFile(zip_path, 'w', compression=zipfile.ZIP_DEFLATED).writestr(
                    Path(rom_path).name, open(rom_path, 'rb').read()
                )
            else:
                new_rom_path = str(new_path / rom_name) + Path(rom_path).suffix
                print(f'{rom_path} ==> {new_rom_path}')
                try_make_dirs(new_rom_path)
                shutil.copy(rom_path, new_rom_path)

    def __check_image_type(self, image_type):
        if image_type is None and len(self.VALID_IMAGE_TYPES) > 0:
            return self.VALID_IMAGE_TYPES[0]
        elif image_type not in self.VALID_IMAGE_TYPES:
            raise ValueError(f'image_type "{image_type}" is not in {self.VALID_IMAGE_TYPES}')
        return image_type


class SimUI(Base):
    DATA_NAME = 'data.dll'
    VALID_IMAGE_TYPES = ['cassette', 'icon', 'packing', 'poster', 'thumbs', 'title']

    def load(self, data_path):
        con = sqlite3.connect(data_path / self.DATA_NAME)
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

    def get_image_path(self, key, image_type=None):
        image_type = self.__check_image_type(image_type)

        row = self[key]
        if 'pname' in row or len(row['pname']) > 0:
            return self.get_image_path(self[row['pname']]['file_md5'])

        for key in ('base_name_en', 'name', 'rom_path'):
            for ext in ('.png', '.jpg'):
                pure_name = Path(row[key]).stem
                image_path = f'{self.path}/{self.platform[image_type+"_path"]}/{pure_name}{ext}'
                if os.path.exists(image_path):
                    return image_path

    def get_rom_name(self, key):
        return legal_file_name(self[key]['name'])


class Pegasus(Base):
    DATA_NAME = 'metadata.pegasus.txt'
    VALID_IMAGE_TYPES = ['box_front', 'logo']
    KEYS = [
        'game',
        'file',
        'files',
        'launch',
        'command',
        'workdir',
        'cwd',
        'developer',
        'developers',
        'publisher',
        'publishers',
        'genre',
        'genres',
        'tag',
        'tags',
        'players',
        'summary',
        'description',
        'release',
        'rating',
        'sorttitle',
        'sortname',
        'sort_title',
        'sort_name',
        'sort-title',
        'sort-name',
        'sortby',
        'sort_by',
        'sort-by',
        'asserts',
    ]
    KEYS_RE = r'^(' + '|'.join(KEYS) + ')'

    def load(self, data_path):
        data = {}
        k = None
        for line in open(Path(data_path) / self.DATA_NAME, encoding='utf-8'):
            line = line.strip()
            if len(line) == 0 or line.startswith('#'):
                continue

            if ':' in line and re.search(self.KEYS_RE, line):
                k, v = line.split(':', 1)
                k = k.strip()
                v = v.strip()
                if k == 'game' and 'game' in data:
                    self[data['game']] = data
                    data = {}
                data[k] = v
            else:
                data[k] += '\n' + line

        if 'game' in data:
            self[data['game']] = data

    def get_rom_path(self, key):
        return self.path / self[key]['file']

    def get_image_path(self, key, image_type=None):
        image_type = self.__check_image_type(image_type)
        row = self[key]

        image_key = f'assets.{image_type}'
        if image_key in row:
            image_path = f'{self.path}/{row[image_key]}'
            if os.path.exists(image_path):
                return image_path
        else:
            if image_type == 'box_front':
                image_type = 'BoxFront'
            for ext in ('.jpg', '.png'):
                image_path = f'media/{key}/{image_type}{ext}'
                if os.path.exists(image_path):
                    return image_path

        return None

    def get_rom_name(self, key):
        return key


class RetroArch(Base):
    def load(self, data_path):
        pass


if __name__ == '__main__':
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument('input', action='store', nargs=1, default='./', help='input path')
    parser.add_argument('output', action='store', nargs=1, help='output path')
    parser.add_argument('--compress', action='store_true', default=False)
    parser.add_argument('--image_type', action='store')
    args = parser.parse_args()

    input_path = Path(args.input[0])
    roms = None
    for klass in (SimUI, Pegasus):
        if (input_path / klass.DATA_NAME).exists():
            roms = klass(input_path)
            break

    if roms:
        roms.convert(args.output[0], args.image_type, args.compress)
    else:
        parser.print_help()
