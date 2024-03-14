from simui import SimUI
from pegasus import Pegasus
from retroarch import RetroArch
from pathlib import Path
import argparse

KLASS = (SimUI, Pegasus, RetroArch)


def image_type_help():
    t = ['image type']
    for klass in KLASS:
        t.append(f'  {klass.__name__+":":12s}' + ','.join(klass.VALID_IMAGE_TYPES))
    return '\n'.join(t)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument('input', action='store', nargs=1, default='./', help='input path')
    parser.add_argument('output', action='store', nargs=1, help='output path')
    parser.add_argument('--compress', action='store_true', default=False, help='compress roms to zip')
    parser.add_argument('--image_type', action='store', help=image_type_help(), metavar='')
    args = parser.parse_args()

    input_path = Path(args.input[0])
    roms = None
    for klass in KLASS:
        if (input_path / klass.DATA_NAME).exists():
            roms = klass(input_path)
            break

    if roms:
        roms.convert(args.output[0], args.image_type, args.compress)
    else:
        parser.print_help()
