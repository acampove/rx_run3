'''
Script used to create YAML file with dictionary between Renato's nicknames and DecFiles based nicknames
'''
import re
import ap_utilities.physics.utilities as aput

# ----------------------------------
class Nicknames:
    '''
    Class used to create a dictionary between old naming and DecayFiles based naming
    '''
    def __init__(self, path : str):
        self._path  = path
        self._regex = r'#?\s*\(\s*(\".*\")\s*,\s*\"(.*)\"\s*,\s*"2024.*\)'
    # ----------------------------------
    def _is_good_line(self, line : str) -> bool:
        if 'ddb-' in line and 'sim' in line:
            return True

        return False
    # ----------------------------------
    def _get_lines(self) -> list[str]:
        with open(self._path, encoding='utf-8') as ifile:
            l_line = ifile.read().splitlines()

        l_line = [ line.replace('\'', '\"') for line in l_line ]
        l_line = [ line for line in l_line if self._is_good_line(line)]

        return l_line
    # ----------------------------------
    def _info_from_line(self, line : str) -> tuple[str, str]:
        mtch = re.match(self._regex, line)
        if not mtch:
            raise ValueError(f'Cannot match {line} with {self._regex}')

        name = mtch.group(1)
        evtt = mtch.group(2)

        name = name.replace('"', '')

        return name, evtt
    # ----------------------------------
    def _get_dictionary(self, l_line : list[str]) -> dict[str,str]:
        d_data = {}
        for line in l_line:
            name, evt_type = self._info_from_line(line)
            nick_name      = aput.read_decay_name(event_type=evt_type, style= 'safe_1')

            d_data[name] = nick_name

        return d_data
    # ----------------------------------
    def get_nicknames(self) -> dict[str,str]:
        '''
        Returns dictionary between old and new naming
        '''
        l_line = self._get_lines()
        d_nick = self._get_dictionary(l_line)

        return d_nick
# ----------------------------------
