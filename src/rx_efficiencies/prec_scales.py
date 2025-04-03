'''
Module containing class ModelScales and helper functions
'''

import os
import json
import math
from importlib.resources import files

import numpy
import pandas            as pnd
import jacobi            as jac
from dmu.logging.log_store          import LogStore
from dmu.generic.version_management import get_last_version


log=LogStore.add_logger('rx_efficiencies:model_scales')
#------------------------------------------
class ModelScales:
    '''
    Class used to calculate scale factor between yields of partially reconstructed component and signal
    '''
    #------------------------------------------
    def __init__(self, dset : str, trig : str, kind : str):
        self._dset = dset
        self._trig = trig
        self._kind = kind

        self._l_kind = ['bpks', 'bdks', 'bsph', 'bpk1', 'bpk2']
        self._l_trig = ['ETOS', 'GTIS']
        self._l_year = ['2011', '2012', '2015', '2016', '2017', '2018']
        self._l_dset = self._l_year + ['r1', 'r2p1', 'all']

        self._d_frbf = None
        self._df_eff = None

        self._initialized = False
    #------------------------------------------
    def _check_arg(self, l_val, val, name):
        if val not in l_val:
            raise ValueError(f'{name} {val} not allowed')

        log.debug(f'{name:<20}{"->":20}{val:<20}')
    #------------------------------------------
    def _initialize(self):
        if self._initialized:
            return

        log.debug('Initializing')
        self._load_fractions()
        self._load_efficiencies()

        self._check_arg(self._l_kind, self._kind, 'Background')
        self._check_arg(self._l_dset, self._dset, 'Dataset')
        self._check_arg(self._l_trig, self._trig, 'Trigger')

        self._initialized = True
    #------------------------------------------
    def _load_fractions(self):
        log.debug('Getting hadronization fractions and branching ratios')

        frbf_dir  = files('rx_efficiencies_data').joinpath('prec_sf')
        frbf_path = get_last_version(dir_path=frbf_dir, version_only=False)
        frbf_path = f'{frbf_path}/fr_bf.json'

        log.debug(f'Picking up branching fractions from: {frbf_path}')
        with open(frbf_path, encoding='utf-8') as ifile:
            self._d_frbf = json.load(ifile)
    #------------------------------------------
    def _load_efficiencies(self):
        log.debug('Getting efficiencies')

        eff_dir  = files('rx_efficiencies_data').joinpath('prec_sf')
        eff_path = get_last_version(dir_path=eff_dir, version_only=False)
        eff_path = f'{eff_path}/{self._dset}_{self._trig}_{self._kind}.json'

        df      = pnd.read_json(eff_path)
        df.Year = df.Year.astype(str)

        self._df_eff = df
    #------------------------------------------
    def _get_fr(self, kind):
        key= 'bpkp' if kind == 'sig' else self._kind
        fx = {'bpkp' : 'fu', 'bpks' : 'fu', 'bdks' : 'fd', 'bsph' : 'fs', 'bpk1' : 'fu', 'bpk2' : 'fu'}[key]
        fx = self._d_frbf['fr'][fx]

        return fx
    #------------------------------------------
    def _mult_brs(self, br_1, br_2):
        l_br_val = [br_1[0], br_2[0]]
        br_cov   = [[br_1[1] ** 2, 0], [0, br_2[1] ** 2]]
        val, var = jac.propagate(lambda x : x[0] * x[1], l_br_val, br_cov)

        return val, math.sqrt(var)
    #------------------------------------------
    def _get_br(self, kind):
        if   kind == 'sig':
            br = self._d_frbf['bf']['bpkp']
            return br

        key = self._kind

        if   key in 'bpk1':
            b1 = self._d_frbf['bf'][   key]
            b2 = self._d_frbf['bf']['k13h']
        elif key in 'bpk2':
            b1 = self._d_frbf['bf'][   key]
            b2 = self._d_frbf['bf']['k23h']
        elif key == 'bpks':
            b1 = self._d_frbf['bf'][   key]
            b2 = self._d_frbf['bf']['k+kp']
        elif key == 'bdks':
            b1 = self._d_frbf['bf'][   key]
            b2 = self._d_frbf['bf']['kokp']
        elif key == 'bsph':
            b1 = self._d_frbf['bf'][   key]
            b2 = self._d_frbf['bf']['phkk']
        else:
            log.error(f'Invalid key: {key}')
            raise

        br = self._mult_brs(b1, b2)

        return br
    #------------------------------------------
    def _filter_by_dataset(self, val):
        if   self._dset == 'all':
            return True
        elif self._dset == 'r1':
            return (val == '2011') or (val == '2012')
        elif self._dset == 'r2p1':
            return val == '2015' or val == '2016'
        elif self._dset in self._l_year:
            return val == self._dset
        else:
            log.error(f'Invalid dataset: {self._dset}')
            raise ValueError
    #------------------------------------------
    def _get_ef(self, kind):
        proc= 'bpkp' if kind == 'sig' else self._kind
        df  = self._df_eff

        df  = df[df.Trigger == self._trig]
        df  = df[df.Process ==       proc]
        df  = df[df.Year.apply(self._filter_by_dataset)]

        passed = df.Passed.sum()
        total  = df.Total.sum()

        eff, eup, edn = utils.get_eff_err(passed, total)
        err = 0.5 * (eup + edn)

        return eff, err
    #------------------------------------------
    def _print_vars(self, l_tup):
        log.debug('-' * 20)
        log.debug(f'{"Var":<20}{"Value":<20}{"Error":<20}')
        log.debug('-' * 20)
        for (val, err), name in zip(l_tup, ['fr sig', 'br sig', 'eff sig', 'fr bkg', 'br bkg', 'eff bkg']):
            log.debug(f'{name:<20}{val:<20.3e}{err:<20.3e}')
        log.debug('-' * 20)
    #------------------------------------------
    def get_scale(self):
        self._initialize()

        fr_sig = self._get_fr('sig')
        br_sig = self._get_br('sig')
        ef_sig = self._get_ef('sig')

        fr_bkg = self._get_fr('bkg')
        br_bkg = self._get_br('bkg')
        ef_bkg = self._get_ef('bkg')

        l_tup = [fr_sig, br_sig, ef_sig, fr_bkg, br_bkg, ef_bkg]
        l_val = [ tup[0] for tup in l_tup]
        l_err = [ tup[1] for tup in l_tup]
        cov   = numpy.diag(l_err) ** 2

        self._print_vars(l_tup)

        val, var = jac.propagate(lambda x : (x[3] * x[4] * x[5]) / (x[0] * x[1] * x[2]), l_val, cov)
        val = float(val)
        err = math.sqrt(var)

        return val, err
#------------------------------------------
