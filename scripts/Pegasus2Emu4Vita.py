import os
from pathlib import Path
import argparse
import zipfile
import shutil


class Metadata(dict):
    def __init__(self, metadata_path='metadata.pegasus.txt'):
        self.path = Path(metadata_path).parent
        data = {}
        for line in open(metadata_path, encoding='utf-8'):
            if ':' in line:
                k, v = line.split(':', 1)
                k = k.strip()
                v = v.strip()
                if k == 'game' and 'game' in data:
                    self[data['game']] = data
                    data = {}
                data[k] = v

        if len(data) > 0:
            self[data['game']] = data

    def get_rom_path(self, name):
        return self.path / self[name]['file']

    def get_image_path(self, name):
        path = self.path / 'media' / name
        if os.path.exists(path / 'boxFront.jpg'):
            return path / 'boxFront.jpg'
        elif os.path.exists(path / 'boxFront.png'):
            return path / 'boxFront.jpg'


if __name__ == '__main__':
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument('input', action="store", nargs=1, default='./')
    parser.add_argument('output', action="store", nargs=1)
    parser.add_argument('--compress', action="store_true", default=False)
    args = parser.parse_args()

    metadata = Metadata(Path(args.input[0]) / 'metadata.pegasus.txt')
    for k, v in metadata.items():
        print(metadata.get_rom_path(k))
        print(metadata.get_image_path(k))
