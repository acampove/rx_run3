'''
Module holding AcceptanceReader class
'''

import os
import pandas as pnd

from pathlib                        import Path
from dmu.generic.version_management import get_last_version
from dmu                            import LogStore

from rx_efficiencies.decay_names    import DecayNames
from rx_common                      import Sample

log=LogStore.add_logger('rx_efficiencies:acceptance_reader')
#----------------------------------
class AcceptanceReader:
    '''
    Class meant to read Geometric acceptances calculated from rapidsim ntuples
    '''
    #----------------------------------
    def __init__(self, year : str, sample : Sample):
        self._year    = year
        self._sample  = sample
        self._ana_dir = Path(os.environ['ANADIR'])
    #----------------------------------
    def _get_energy(self) -> str:
        '''
        Returns
        -----------------
        Center of mass energy string associated to _year
        '''
        d_energy  = {
                '2011' : '7TeV',
                '2012' : '8TeV',
                '2015' : '13TeV',
                '2016' : '13TeV',
                '2017' : '13TeV',
                '2018' : '13TeV',
                '2024' : '14TeV', # TODO: This needs to be updated, when 13.6 TeV files be available
                                  # Will average 2018 and 2024 for now.
                }

        if self._year not in d_energy:
            raise ValueError(f'Invalid year: {self._year}')

        return d_energy[self._year]
    #----------------------------------
    def read(self) -> float:
        '''
        Reads JSON files, returns acceptance value
        '''
        prc_dir = self._ana_dir / 'efficiencies/acceptances'
        vers    = get_last_version(dir_path=prc_dir, version_only=True)
        energy  = self._get_energy()
        prc_path= prc_dir / f'{vers}/acceptances_{energy}.json'

        if not prc_path.exists():
            log.error(f'File not found: {prc_path}')
            raise FileNotFoundError

        df            = pnd.read_json(prc_path)
        df['Process'] = df['Process'].replace(DecayNames.tex_nic)

        df  = df[ df.Process == self._proc ]
        if len(df) == 0:
            raise ValueError(f'Process {self._proc} not found in {prc_path}')

        try:
            [val] = df.Physical.tolist()
        except ValueError as exc:
            raise ValueError(f'More than one acceptance for process: {self._proc}') from exc

        return val
#----------------------------------
