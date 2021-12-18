import json
from argparse import ArgumentParser


def append_license(outfile, license_filename='LICENSE.txt'):
    with open(license_filename, mode='r') as f:
        for line in f:
            outfile.write('" {}'.format(line))

        outfile.write('\n')


def append_from_map_json(outfile, json_filename: str):
    tagtable = {
        "cdef": "command",
        "endef": "ennoremap",
        "evdef": "evnoremap",
        "gndef": "gnnoremap",
        "gvdef": "gvnoremap",
        "idef": "inoremap",
        "rdef": "rnoremap",
    }
    with open(json_filename, mode='r') as fin:
        objects = json.load(fin)

        for obj in objects:
            outfile.write('" {}\n'.format(obj['en']))

            for key in obj.keys():
                if not (key in tagtable):
                    continue

                commands = obj[key]
                if len(commands) <= 0:
                    continue

                first_syscode = commands[0][1:-1]
                if first_syscode in tagtable:
                    commands = obj[first_syscode]

                for cmd in commands:
                    outfile.write('{} {} {}\n'.format(
                        tagtable[key],
                        cmd.replace(' ', '<space>'),
                        obj['name']))

            outfile.write('\n')


def append_from_set_json(outfile, json_filename: str):
    with open(json_filename, mode='r') as fin:
        objects = json.load(fin)

        for name, obj in objects.items():
            outfile.write('" {}\n'.format(obj['en']))
            outfile.write('set {}={}\n'.format(name, obj['value']))
            outfile.write('\n')

        outfile.write('\n')


def generate_vindrc(
        out_filename: str,
        map_jsons: list = [],
        set_jsons: list = []):

    with open(out_filename, mode='w') as fout:
        append_license(fout)

        if len(set_jsons) > 0:
            fout.write('" => set')
            for f in set_jsons:
                append_from_set_json(fout, f)

        if len(map_jsons) > 0:
            fout.write('" => mappings')
            for f in map_jsons:
                append_from_map_json(fout, f)


if __name__ == '__main__':
    parser = ArgumentParser()
    parser.add_argument('--mapjson', nargs='+', type=str, required=True)
    parser.add_argument('--setjson', nargs='+', type=str, required=True)
    parser.add_argument('--output', type=str, required=True)

    cfg = parser.parse_args()

    generate_vindrc(
            cfg.output,
            map_jsons=cfg.mapjson,
            set_jsons=cfg.setjson)