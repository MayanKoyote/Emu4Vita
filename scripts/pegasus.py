from base import *


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
        for line in open(data_path, encoding='utf-8'):
            line = line.strip()
            if len(line) == 0 or line.startswith('#'):
                continue

            if ':' in line and re.search(self.KEYS_RE, line):
                k, v = line.split(':', 1)
                k = k.strip()
                v = v.strip()
                if k == 'game' and 'game' in data:
                    if 'files' in data:
                        for f in data['files'].strip().split('\n'):
                            f = f.strip()
                            data['file'] = f
                            self[f"{data['game']} {Path(f).stem}"] = data
                    else:
                        self[data['game']] = data
                    data = {}
                data[k] = v
            elif k is not None:
                data[k] += '\n' + line

        if 'game' in data:
            self[data['game']] = data

    def get_rom_path(self, key):
        return self.path / self[key]['file']

    def get_image_path(self, key, image_type):
        self._check_image_type(image_type)
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
                image_path = f'{self.path}/media/{key}/{image_type}{ext}'
                if os.path.exists(image_path):
                    return image_path

        return None

    def get_rom_name(self, key):
        return legal_file_name(key)
