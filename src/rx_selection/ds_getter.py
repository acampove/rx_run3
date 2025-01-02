'''
Module containing class that provides ROOT dataframe after a given selection
'''
# pylint: disable=import-error

import os
import re
import glob
import logging
from typing              import Optional
from importlib.resources import files

import pprint
import yaml

from ROOT import RDataFrame


from dmu.rdataframe.atr_mgr import AtrMgr
from dmu.logging.log_store  import LogStore


from rk.dbase_paths        import dbase_paths as dbpath
from rk.mva_man            import mva_man

from rk.cutflow            import cutflow
from rk.efficiency         import efficiency
from rk.efficiency         import ZeroYields

import rk_selection.selection      as sel
from rx_selection import utilities as ut

log=LogStore.add_logger('rx_selection:ds_getter')
# -----------------------------------------
class ds_getter:
    '''
    Class used to provide dataframe after a given selection
    '''
    # pylint: disable = too-many-instance-attributes
    # pylint: disable = invalid-name
    # ------------------------------------
    def __init__(self,
                 q2bin : str,
                 trig  : str,
                 year  : str,
                 version   : str,
                 partition : tuple[int, int],
                 kind      : str,
                 sel       : str):
        # pylint: disable = too-many-arguments, too-many-positional-arguments
        self._q2bin      = q2bin
        self._vers       = version
        self._trig       = trig
        self._sel        = sel
        self._kind       = kind
        self._year       = year
        self._part       = partition
        self._cfg        = {}
        self._sample     = None
        self._is_sim     : bool
        self._d_ext_bdt  : dict[str,str]
        self._remove_cuts= None
        self._is_signal  : bool
        self._decay      : str

        self._debug_mode = False
        self._h_ipchi2   = '(1)' if year in ['2024'] else 'H_IPCHI2_OWNPV > 4'

        self._bdt_dir_cmb : str
        self._bdt_dir_prc : str

        #Fake trigger needed to get nspd hits cut, tool needs a trigger
        #but cut does not depend on trigger
        self._dummy_trigger = 'ETOS'

        self._initialized   = False
    # ------------------------------------
    def _initialize(self):
        if self._initialized:
            return

        self._cfg       = self._get_config()
        self._decay     = self._get_decay()
        self._is_sim    = self._kind in self._cfg['simulation']
        self._is_signal = self._kind in self._cfg['signal']

        self._set_bdt_paths()
        self._set_logs()

        self._sample = self._get_sample()

        ut.check_included(self._year, self._cfg['datasets']['all'])
        ut.check_included(self._kind, self._cfg['processes'])

        try:
            if self._part is not None:
                _, _ = self._part
        except ValueError as exc:
            raise ValueError(f'Could not extract partitioning scheme from: {self._part}') from exc

        self._initialized = True
    # ------------------------------------
    def _get_decay(self) -> str:
        '''
        For a given kind of sample (used for naming directories where trees are)
        Return the decay, used to pick up YAML files with selection
        '''

        # Run1/2
        if self._kind in ['cmb', 'data', 'ctrl', 'sign']:
            decay = 'bukCHAN'
        # Run 3
        else:
            decay = self._cfg['decays'][self._kind]

        chan = 'mm' if self._trig == 'MTOS' else 'ee'
        decay= decay.replace('CHAN', chan)

        return decay
    # ------------------------------------
    def _get_config(self):
        '''
        Load YAML config and returns dictionary
        '''
        cfg_path = files('tools_data').joinpath('selection/samples.yaml')
        cfg_path = str(cfg_path)
        if not os.path.isfile(cfg_path):
            raise FileNotFoundError(f'File not found: {cfg_path}')

        with open(cfg_path, encoding='utf-8') as ifile:
            cfg = yaml.safe_load(ifile)

        return cfg
    # ------------------------------------
    def _set_logs(self):
        '''
        Silence log messages of tools
        '''

        AtrMgr.log.setLevel(logging.WARNING)
        cutflow.log.setLevel(logging.WARNING)
        efficiency.log.setLevel(logging.WARNING)
    # ------------------------------------
    def _set_bdt_paths(self):
        db                = dbpath(year=self._year)
        self._bdt_dir_cmb = db('bdt_cmb')
        self._bdt_dir_prc = db('bdt_prc')
    # ------------------------------------
    @property
    def debug_mode(self) -> bool:
        '''
        Flag indicating if this tool will be ran
        for debugging purposes
        '''
        return self._debug_mode

    @debug_mode.setter
    def debug_mode(self, value) -> None:
        if not isinstance(value, bool):
            raise ValueError(f'Argument was  not a bool: {value}')

        self._debug_mode = value
    # ------------------------------------
    @property
    def extra_bdts(self):
        '''
        Dictionary holding information on extra BDTs
        '''
        return self._d_ext_bdt

    @extra_bdts.setter
    def extra_bdts(self, value):
        self._d_ext_bdt= value
    # ------------------------------------
    def _get_sample(self):
        '''
        Will return proces_chan for given kind of sample, e.g. ctrl_ee
        '''
        key = 'processes'
        if key not in self._cfg:
            log.error(f'Key {key} not found in:')
            pprint.pprint(self._cfg)
            raise KeyError

        d_proc = self._cfg[key]

        proc   = d_proc[self._kind]
        chan   = 'mm' if self._trig == 'MTOS' else 'ee'

        return f'{proc}_{chan}'
    # ------------------------------------
    def _add_reco_cuts(self, d_cut):
        d_cut_extra = {}
        for key, cut in d_cut.items():
            if key != 'truth':
                d_cut_extra[key] = cut
                continue

            d_cut_extra[key]        = cut
            d_cut_extra['K_IPChi2'] = self._h_ipchi2

        return d_cut_extra
    # ------------------------------------
    def _update_bdt_cut(self, cut : str, skip_cmb : bool, skip_prec : bool) -> str:
        '''
        Will pick BDT cut, cmb and prec. Will return only one of them, depending on which one is skipped
        If none is skipped, will return original cut
        '''
        if not skip_cmb and not skip_prec:
            log.debug('No bdt cut is skipped, will not redefine')
            return cut

        if cut == '(1)':
            log.debug('No cut was passed, will not redefine')
            return cut

        regex=r'(BDT_cmb\s>\s[0-9\.]+)\s&&\s(BDT_prc\s>\s[0-9\.]+)'
        mtch =re.match(regex, cut)
        if not mtch:
            raise ValueError(f'Cannot match {cut} with {regex}')

        [bdt_cmb, bdt_prc] = mtch.groups()

        return bdt_cmb if skip_prec else bdt_prc
    # ------------------------------------
    def _filter_bdt(self, rdf, cut, skip_prec : bool, skip_cmb : bool):
        '''
        Will add BDT score column and apply a cut on it
        '''
        if skip_prec and skip_cmb:
            log.warning('Skipping both BDTs')
            return rdf, '(1)'

        if not skip_cmb:
            log.info(f'Picking up combinatorial BDT from: {self._bdt_dir_cmb}')
            man_cmb=mva_man(rdf, self._bdt_dir_cmb, self._trig)
            rdf    =man_cmb.add_scores('BDT_cmb')

        if not skip_prec:
            log.info(f'Picking up PRec BDT from: {self._bdt_dir_prc}')
            man_prc=mva_man(rdf, self._bdt_dir_prc, self._trig)
            rdf    =man_prc.add_scores('BDT_prc')

        cut = self._update_bdt_cut(cut, skip_cmb, skip_prec)
        log.info(f'Using bdt cut: \"{cut}\"')
        rdf = rdf.Filter(cut, 'bdt')

        return rdf, cut
    # ------------------------------------
    def _add_extra_bdts(self, rdf):
        if not hasattr(self, '_d_ext_bdt'):
            log.info('No extra BDT added')
            return rdf

        log.info('Adding extra BDTs')
        for var, location in self._d_ext_bdt.items():
            log.debug(f'---> {var}')
            man =mva_man(rdf, location, self._trig)
            rdf =man.add_scores(var)

        return rdf
    # ------------------------------------
    def _skim_df(self, df):
        if self._part is None:
            return df

        islice, nslice = self._part

        df = ut.get_df_range(df, islice, nslice)

        return df
    # ------------------------------------
    def _get_tree_path(self) -> str:
        if   self._kind == 'cmb'            and self._trig in ['ETOS', 'GTIS']:
            tree_path = 'KSS_ee'
        elif self._kind == 'cmb'            and self._trig == 'MTOS':
            tree_path = 'KSS_mm'
        elif self._trig in ['ETOS', 'GTIS'] and self._kind.endswith('_cal'):
            tree_path = 'KCL_ee'
        elif self._trig in ['ETOS', 'GTIS'] and self._kind.endswith('_misid'):
            tree_path = 'KMI_ee'
        elif self._kind == 'cmb_em':
            tree_path = 'KEM'
        elif self._trig == 'MTOS':
            tree_path = 'KMM'
        elif self._trig in ['ETOS', 'GTIS']:
            tree_path = 'KEE'
        else:
            raise ValueError(f'Cannot pick tree path, invalid kind/trigger: {self._kind}/{self._trig}')

        return tree_path
    # ------------------------------------
    def _get_file_path(self) -> str:
        dat_dir   = os.environ['DATDIR']
        if not self._debug_mode:
            file_path = f'{dat_dir}/{self._sample}/{self._vers}/{self._year}.root'
            ut.check_file(file_path)
            return file_path

        log.warning('Running in debugging mode, using single file')
        file_wc   = f'{dat_dir}/{self._sample}/{self._vers}/{self._year}/*.root'
        l_path    = glob.glob(file_wc)
        file_path = l_path[0]
        ut.check_file(file_path)

        return file_path
    # ------------------------------------
    def _get_df_raw(self) -> RDataFrame:
        tree_path = self._get_tree_path()
        file_path = self._get_file_path()

        log.info('------------------------------------')
        log.info( 'Retrieving dataframe for:')
        log.info(f'{"File path  ":<20}{file_path:<100}')
        log.info(f'{"Tree path  ":<20}{tree_path:<100}')
        log.info('------------------------------------')

        df = RDataFrame(tree_path, file_path)
        df = df.Define('sample', f'std::string("{self._decay}")')
        df = self._skim_df(df)

        df.filepath = file_path
        df.treename = tree_path
        df.year     = self._year

        return df
    # ------------------------------------
    def _get_gen_nev(self):
        dat_dir   = os.environ['DATDIR']
        file_path = f'{dat_dir}/{self._sample}/{self._vers}/{self._year}.root'

        ut.check_file(file_path)
        log.debug('Retrieving gen statistics:')
        df  = RDataFrame('gen', file_path)
        df  = self._skim_df(df)
        nev = df.Count().GetValue()

        return nev
    # ------------------------------------
    def _redefine(self, d_cut : dict[str,str], d_redefine : dict[str,str]) -> dict[str,str]:
        '''
        Takes dictionary with selection and overrides with with entries in d_redefine
        Returns redefined dictionary
        '''
        for key, new_cut in d_redefine.items():
            if key not in d_cut:
                log.error(f'Cannot redefine {key}, not a valid cut, choose from: {d_cut.keys()}')
                pprint.pprint(d_cut)
                raise ValueError

            old_cut    = d_cut[key]
            d_cut[key] = new_cut

            old_cut    = re.sub(' +', ' ', old_cut)
            new_cut    = re.sub(' +', ' ', new_cut)
            log.info(f'{key:<15}{old_cut:<70}{"--->":10}{new_cut:<40}')

        return d_cut
    # ------------------------------------
    def _add_reco(self, cf : cutflow, nrec : int, truth_string : str) -> cutflow:
        '''
        Takes cutflow and nreco to calculate the reco efficiency and add it to the cutflow
        Returns updated cutflow
        '''
        if not self._is_sim:
            return cf

        if self._year in self._cfg['datasets']['run3']:
            log.warning(f'Using a reconstruction efficiency of 1 for {self._year}')
            ngen = nrec
        else:
            ngen = self._get_gen_nev()

        cf['reco'] = efficiency(nrec, ngen - nrec, cut=truth_string)

        return cf
    # ------------------------------------
    def _redefine_mass(self, rdf):
        '''
        Takes ROOT dataframe, for Run3 the const_mass branches are floats. This needs to be harmonized to make them RVec as in Run2.
        Returns dataframe with redefined mass branches
        '''

        if self._year in ['2011', '2012', '2015', '2016', '2017', '2018']:
            log.debug(f'Not redefining any mass column as RVecF for {self._year}')
            return rdf

        v_col = rdf.GetColumnNames()
        l_col = [col.c_str() for col in v_col]
        l_mas = [col         for col in l_col if '_const_mass_' in col]

        for name in l_mas:
            rdf = rdf.Redefine(name, f'float val = {name}; return ROOT::RVecF({{val}});')
            log.debug(f'Redefining {name} as RVecF for {self._year}')

        return rdf
    # ----------------------------------------
    def get_df(self,
               remove_cuts: Optional[list[str]]     = None,
               d_redefine : Optional[dict[str,str]] = None,
               skip_cmb   : bool = False,
               skip_prec  : bool = False):
        '''
        Returns ROOT dataframe after selection

        Parameters:
        ----------------------
        remove_cuts (list) : list of strings associated to cuts to skip
        d_redefine (dict)  : Dictionary {name : 'new cut'} used to redefine "name"
        skip_prec  (bool)  : If true, will not calculate PRec BDT score
        skip_cmb   (bool)  : If true, will not calculate cmb BDT score
        '''

        remove_cuts = [] if remove_cuts is None else remove_cuts
        self._initialize()

        self._remove_cuts = remove_cuts

        df    = self._get_df_raw()
        dfmgr = AtrMgr(df)

        cf    = cutflow(d_meta = {'file' : df.filepath, 'tree' : df.treename})
        tot   = df.Count().GetValue()
        d_cut = sel.selection(self._sel,
                               self._trig,
                               self._year,
                               self._sample,
                               q2bin=self._q2bin,
                               decay=self._decay)

        if not self._is_sim:
            d_cut = dict( [('truth', '(1)')] + list(d_cut.items()) )

        d_cut = self._add_reco_cuts(d_cut)

        if d_redefine is not None:
            d_cut = self._redefine(d_cut, d_redefine)

        log.info(f'Applying selection: {self._sel}')


        for key, cut in d_cut.items():
            if key in self._remove_cuts:
                log.info(f'{"skip":<10}{key:>20}')
                continue

            log.info(f'{"":<10}{key:>20}')

            if key == 'bdt':
                df      = self._add_extra_bdts(df)
                df, cut = self._filter_bdt(df, cut, skip_prec=skip_prec, skip_cmb=skip_cmb)
            else:
                df = df.Filter(cut, key)

            pas=df.Count().GetValue()

            try:
                eff = efficiency(pas, tot - pas, cut=cut)
            except ZeroYields:
                log.error(f'Last cut ({cut}) passed zero events:')
                print(cf)
                raise

            if key == 'truth' and self._is_signal:
                cf = self._add_reco(cf, pas, cut)
            else:
                cf[key]  = eff

            tot=pas

        df          = self._redefine_mass(df)
        df          = dfmgr.add_atr(df)
        df.treename = self._trig
        df.cf       = cf

        return df
# -----------------------------------------
