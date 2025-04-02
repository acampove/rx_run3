from importlib.resources import files
from version_management  import get_last_version 

import os
import pandas as pnd

from log_store import log_store

log=log_store.add_logger('rx_prec_acceptances:acceptance_reader')
#----------------------------------
class reader:
    def __init__(self, year=None, proc=None):
        self._year = year
        self._proc = proc
    #----------------------------------
    def _get_energy(self):
        d_energy  = {
                '2011' : '7TeV', 
                '2012' : '8TeV', 
                '2015' : '13TeV', 
                '2016' : '13TeV', 
                '2017' : '13TeV', 
                '2018' : '13TeV', 
                }

        if self._year not in d_energy:
            log.error(f'Invalid year: {self._year}')
            raise

        return d_energy[self._year]
    #----------------------------------
    def _get_process(self):
        d_proc = {
                r'$B_d\to K^{*0}(\to K^+\pi^-)e^+e^-$'                 : 'bdks',
                r'$B^+\to K^+e^+e^-$'                                  : 'bpkp',
                r'$B_s\to \phi(1020)e^+e^-$'                           : 'bsph',
                r'$B^+\to K_2(1430)^+(\to X \to K^+\pi^+\pi^-)e^+e^-$' : 'bpk2',
                r'$B^+\to K_1(1270)^+(\to K^+\pi^+\pi^-)e^+e^-$'       : 'bpk1',
                r'$B^+\to K^{*+}(\to K^+\pi^0)e^+e^-$'                 : 'bpks',
                }

        return d_proc
    #----------------------------------
    def read(self):
        prc_dir = files('prec_acceptances_data')
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
            log.error(f'Invalid process: {self._proc}')
            raise

        try:
            [val] = df.Physical.tolist()
        except:
            log.error(f'More than one acceptance for process: {self._proc}')
            raise

        return val
#----------------------------------
