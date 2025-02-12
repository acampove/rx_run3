'''
Module with class used to swap mass hypotheses
'''


import os
import re
import random

import numpy
import vector
import pandas as pnd

from tqdm                  import tqdm
from particle              import Particle  as part
from pandarallel           import pandarallel
from dmu.logging.log_store import LogStore

log = LogStore.add_logger('rx_data:swap_calculator')
#---------------------------------
class SWPCalculator:
    '''
    Class used to calculate di-track masses, after mass hypothesis swaps
    '''
    def __init__(self, df, d_lep=None, d_had=None):
        self._df     = df
        self._d_lep  = d_lep
        self._d_had  = d_had
        self._plt_dir= None

        self._initialized=False
    #---------------------------------
    def _initialize(self):
        if self._initialized:
            return

        self._check_particle(self._d_lep)
        self._check_particle(self._d_had)

        if self._plt_dir is not None:
            os.makedirs(self._plt_dir, exist_ok=True)

        pandarallel.initialize(progress_bar=True)
        tqdm.pandas(ascii=' -')

        self._initialized=True
    #---------------------------------
    @property
    def plt_dir(self):
        '''
        Used to set plot directory where validation plots are stored
        If not pased, won't make plots
        '''
        return self._plt_dir

    @plt_dir.setter
    def plt_dir(self, value):
        self._plt_dir = value
    #---------------------------------
    def _check_particle(self, d_part):
        if not isinstance(d_part, dict):
            log.error(f'Dictionary expected, found: {d_part}')
            raise

        for par_name, pdg_id in d_part.items():
            try:
                part.from_pdgid(pdg_id)
            except:
                log.error(f'Cannot create particle for PDGID: {pdg_id}')
                raise
    #---------------------------------
    def _build_mass(self, row, d_part):
        l_vec = []
        for name, new_id in d_part.items():
            par    = part.from_pdgid(new_id)
            ms     = par.mass
            old_id = row[f'{name}_ID']

            px = row[f'{name}_PX']
            py = row[f'{name}_PY']
            pz = row[f'{name}_PZ']
            pe = row[f'{name}_PE']
            vec_1 = vector.obj(px=px, py=py, pz=pz, t=pe)
            log.debug(f'{name}: {vec_1.mass:0f}({old_id}) -> {ms:.0f}({new_id})')
            vec_2 = vector.obj(pt=vec_1.pt, phi=vec_1.phi, theta=vec_1.theta, mass=ms)
            l_vec.append(vec_2)

        [vec_1, vec_2] = l_vec

        vec = vec_1 + vec_2

        return vec.mass
    #---------------------------------
    def _combine(self, row, had_name, kind, new_had_id, multiple_candidates):
        old_had_id = row[f'{had_name}_ID']
        had        = part.from_pdgid(old_had_id)
        l_mass     = []
        for lep_name, new_lep_id in self._d_lep.items():
            old_lep_id = row[f'{lep_name}_ID']
            lep        = part.from_pdgid(old_lep_id)

            if lep.charge == had.charge:
                continue

            lep_id = new_lep_id if kind == 'swp' else old_lep_id
            had_id = new_had_id if kind == 'swp' else old_had_id

            mass   = self._build_mass(row, {had_name : had_id, lep_name : lep_id})

            l_mass.append(mass)

        if not multiple_candidates:
            l_mass = self._pick_mass(l_mass)

        return l_mass
    #---------------------------------
    def _pick_mass(self, l_mass):
        l_mass_filt = [ mass for mass in l_mass if not numpy.isnan(mass) ]

        if len(l_mass_filt) == 0:
            return [ numpy.nan ]

        mass = random.choice(l_mass_filt)

        return [mass]
    #---------------------------------
    def _get_charges(self, row, name):
        pdg_id = row[f'{name}_ID']
        partic = part.from_pdgid(pdg_id)

        return partic.charge
    #---------------------------------
    def _remove_nan(self, sr_val : pnd.Series, nan_val : float) -> pnd.Series:
        init   = sr_val.size
        sr_val = sr_val.explode()

        if nan_val is None:
            sr_val = sr_val.dropna(ignore_index=True)
        else:
            sr_val = sr_val.fillna(nan_val)

        fnal   = sr_val.size

        if init != fnal:
            log.warning(f'Dropped NA values, changed size: {init} -> {fnal}')

        return sr_val
    #---------------------------------
    def get_df(self, nan_val : float = 0, multiple_candidates : bool =True):
        '''
        Parameters:
        ------------------
        nan_val (float|int) When a NaN is found, if not set, will remove the entry. Otherwise will replace it with nan_val
        multiple_candidates (bool): If true (default), will store all found combinations, otherwise will pick one randomly out of
        the set of masses that are not NaNs.

        Returns:
        ------------------
        Pandas dataframe with orignal and swapped masses, i.e. masses after the mass hypothesis swap
        '''
        self._initialize()

        d_comb = {}
        for had_name, new_had_id in self._d_had.items():
            for kind in ['org', 'swp']:
                log.debug(f'Adding column for {had_name}/{new_had_id}/{kind}')
                sr_mass = self._df.parallel_apply(self._combine, args=(had_name, kind, new_had_id, multiple_candidates), axis=1)
                sr_mass = self._remove_nan(sr_mass, nan_val)

                d_comb[f'{had_name}_{kind}'] = sr_mass

        df = pnd.DataFrame(d_comb)

        return df
#---------------------------------
