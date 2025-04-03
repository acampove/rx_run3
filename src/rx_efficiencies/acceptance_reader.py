'''
Module holding AcceptanceReader class
'''

import os
from importlib.resources import files

import pandas as pnd

from dmu.generic.version_management import get_last_version
from dmu.logging.log_store          import LogStore

log=LogStore.add_logger('rx_efficiencies:acceptance_reader')
#----------------------------------
class AcceptanceReader:
    '''
    Class meant to read Geometric acceptances calculated from rapidsim ntuples
    '''
    #----------------------------------
    def __init__(self, year : str, proc : str):
        self._year = year
        self._proc = proc
    #----------------------------------
    def _get_energy(self) -> dict[str,str]:
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
    def _get_process(self) -> dict[str,str]:
        d_proc = {
                r'$B_d\to K^{*0}(\to K^+\pi^-)e^+e^-$'                 : 'bdkskpiee',
                r'$B^+\to K^+e^+e^-$'                                  : 'bpkpee',
                r'$B_s\to \phi(1020)e^+e^-$'                           : 'bsphiee',
                r'$B^+\to K_2(1430)^+(\to X \to K^+\pi^+\pi^-)e^+e^-$' : 'bpk2kpipiee',
                r'$B^+\to K_1(1270)^+(\to K^+\pi^+\pi^-)e^+e^-$'       : 'bpk1kpipiee',
                r'$B^+\to K^{*+}(\to K^+\pi^0)e^+e^-$'                 : 'bpkskpiee',
                }

        return d_proc
    #----------------------------------
    def read(self) -> float:
        '''
        Reads JSON files, returns acceptance value
        '''
        prc_dir = files('rx_efficiencies_data').joinpath('acceptances')
        vers    = get_last_version(dir_path=prc_dir, version_only=True)
        energy  = self._get_energy()
        prc_path= f'{prc_dir}/{vers}/acceptances_{energy}.json'

        if not os.path.isfile(prc_path):
            log.error(f'File not found: {prc_path}')
            raise FileNotFoundError

        d_proc    = self._get_process()
        df        = pnd.read_json(prc_path)
        l_proc_in = df.Process.tolist()
        l_proc_ot = [ d_proc[proc_in] for proc_in in l_proc_in ]
        df['Process'] = l_proc_ot

        df  = df[ df.Process == self._proc ]
        if len(df) == 0:
            raise ValueError(f'Process {self._proc} not found in {prc_path}')

        try:
            [val] = df.Physical.tolist()
        except ValueError as exc:
            raise ValueError(f'More than one acceptance for process: {self._proc}') from exc

        return val
#----------------------------------
