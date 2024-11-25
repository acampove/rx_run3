'''
Script used to create YAML file with dictionary between Renato's nicknames and DecFiles based nicknames 
'''
import re
import yaml

import dmu.physics.utilities as phut
# ----------------------------------
class Data:
    regex = r'\s*\(\s*(\".*\")\s*,\s*\"(.*)\"\s*,\s*"2024.*\)'

# ----------------------------------
def _is_good_line(line : str) -> bool:
    if 'Comment' in line:
        return False

    if 'ddb' in line:
        return True

    return False
# ----------------------------------
def _get_lines() -> list[str]:
    with open('samples_list.txt', encoding='utf-8') as ifile:
        l_line = ifile.read().splitlines()

    l_line = [ line.replace('\'', '\"') for line in l_line ]
    l_line = [ line for line in l_line if _is_good_line(line)]

    return l_line
# ----------------------------------
def _info_from_line(line : str) -> tuple[str, str]:
    mtch = re.match(Data.regex, line)
    if not mtch:
        raise ValueError(f'Cannot match {line} with {Data.regex}')

    name = mtch.group(1)
    evtt = mtch.group(2)

    name = name.replace('"', '')

    return name, evtt
# ----------------------------------
def _get_dictionary(l_line : list[str]) -> dict[str,str]:
    d_data = {}
    for line in l_line:
        name, evt_type = _info_from_line(line)
        nick_name      = phut.read_decay_name(event_type=evt_type, style= 'safe_1')

        d_data[name] = nick_name

    return d_data
# ----------------------------------
def main():
    '''
    Script starts here
    '''
    l_line = _get_lines()

    d_nick = _get_dictionary(l_line)

    with open('nickname.yaml', 'w', encoding='utf-8') as ofile:
        yaml.safe_dump(d_nick, ofile)
# ----------------------------------
if __name__ == '__main__':
    main()
