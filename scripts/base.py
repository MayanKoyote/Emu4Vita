import os
import re
from pathlib import Path
import shutil
import zipfile


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

    def _check_image_type(self, image_type):
        if image_type not in self.VALID_IMAGE_TYPES:
            raise ValueError(f'image_type "{image_type}" is not in {self.VALID_IMAGE_TYPES}')
