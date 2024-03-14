from base import Base
from pathlib import Path
import json


class RetroArch(Base):
    DATA_NAME = 'playlists'
    VALID_IMAGE_TYPES = ['Boxarts', 'Snaps', 'Titles']

    def __load_lpl(self, name):
        items = {}
        lpl_name = Path(name).stem
        lpl = json.load(open(name, encoding='utf-8'))
        for item in lpl['items']:
            item['plp_name'] = lpl_name
            items[item['path']] = item
        return items

    def load(self, data_path):
        self.path = data_path
        for name in (Path(data_path) / self.DATA_NAME).glob('*.lpl'):
            self.update(self.__load_lpl(name))

    def get_rom_path(self, key):
        return key

    def get_image_path(self, key, image_type=None):
        image_type = self._check_image_type(image_type)
        item = self[key]
        return f'{self.path}/thumbnails/{item["plp_name"]}/Named_{image_type}/{item["label"]}.png'

    def get_rom_name(self, key):
        return self[key]['label']
