'''
Module containing class that provides ROOT dataframe after a given selection
'''
# pylint: disable = import-error
# pylint: disable = too-many-instance-attributes
# pylint: disable = line-too-long
# pylint: disable = invalid-name
# pylint: disable = too-many-arguments, too-many-positional-arguments

import os
import re
import glob

from importlib.resources import files
from typing              import Union

import pprint
import yaml

from ROOT import RDataFrame

from dmu.rdataframe.atr_mgr import AtrMgr
from dmu.logging.log_store  import LogStore


from rx_selection            import cutflow     as cfl
from rx_selection.efficiency import efficiency
from rx_selection.efficiency import ZeroYields

from rx_selection import selection as sel
from rx_selection import utilities as ut

log=LogStore.add_logger('rx_selection:ds_getter')
# -----------------------------------------
class ds_getter:
    '''
    Class used to provide dataframe after a given selection
    '''
    # ------------------------------------
    def __init__(self, cfg : dict):
        ipart                 = cfg['ipart'   ]
        npart                 = cfg['npart'   ]
        self._part            = [ipart, npart ]
        self._q2bin           = cfg['q2bin'   ]
        self._sample          = cfg['sample'  ]
        self._project         = cfg['project' ]
        self._d_redefine_cuts = cfg['redefine']
        self._hlt2            = cfg['hlt2'    ]
        self._cutver          = cfg['cutver'  ]
        self._ipath           = cfg['ipath'   ]

        self._skip_cmb      = True
        self._skip_prc      = True
        self._is_sim        : bool

        self._initialized   = False
    # ------------------------------------
    def _initialize(self):
        if self._initialized:
            return

        self._is_sim    = not self._sample.startswith('data')

        self._set_logs()

        self._initialized = True
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

        LogStore.set_level('dmu:rdataframe:atr_mgr' , 30)
        LogStore.set_level('rx_selection:cutflow'   , 30)
        LogStore.set_level('rx_selection:efficiency', 30)
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
    def _filter_bdt(self, rdf : RDataFrame, cut : str) -> tuple[RDataFrame, str]:
        '''
        Will add BDT score column and apply a cut on it
        '''
        if self._skip_prc and self._skip_cmb:
            log.warning('Skipping both BDTs')
            return rdf, '(1)'

        raise NotImplementedError(f'BDT filtering has not been implemented for cut: {cut}')
    # ------------------------------------
    def _skim_df(self, rdf : RDataFrame) -> RDataFrame:
        if self._part is None:
            return rdf

        islice, nslice = self._part

        rdf = ut.get_rdf_range(rdf, islice, nslice)

        return rdf
    # ------------------------------------
    def _get_files_path(self) -> list[str]:
        files_wc = f'{self._ipath}/{self._sample}/{self._hlt2}/*.root'
        l_path   = glob.glob(files_wc)
        npath    = len(l_path)
        if npath == 0:
            raise FileNotFoundError('No files found in: {files_wc}')

        log.info(f'Found {npath} files')

        return l_path
    # ------------------------------------
    def _get_df_raw(self) -> RDataFrame:
        l_file_path = self._get_files_path()

        rdf = RDataFrame('DecayTree', l_file_path)
        rdf = self._skim_df(rdf)
        rdf.filepath = str(l_file_path)
        rdf.treename = 'DecayTree'

        return rdf
    # ------------------------------------
    def _get_gen_nev(self) -> Union[int,None]:
        if not self._is_sim:
            return None

        log.warning('Reading number of entries from MCDecayTree not implemented')

        return 1
    # ------------------------------------
    def _redefine_cuts(self, d_cut : dict[str,str]) -> dict[str,str]:
        '''
        Takes dictionary with selection and overrides with with entries in self._d_redefine_cuts
        Returns redefined dictionary
        '''
        for cut_name, new_cut in self._d_redefine_cuts.items():
            if cut_name not in d_cut:
                pprint.pprint(d_cut)
                raise ValueError(f'Cannot redefine {cut_name}, not a valid cut, choose from: {d_cut.keys()}')

            old_cut         = d_cut[cut_name]
            d_cut[cut_name] = new_cut

            old_cut    = re.sub(' +', ' ', old_cut)
            new_cut    = re.sub(' +', ' ', new_cut)

            log.info(f'{cut_name:<15}{old_cut:<70}{"--->":10}{new_cut:<40}')

        return d_cut
    # ------------------------------------
    def _add_reco_efficiency(self, cf : cfl.cutflow, nrec : int, truth_string : str) -> cfl.cutflow:
        '''
        Takes cutflow and nreco to calculate the reco efficiency and add it to the cutflow
        Returns updated cutflow
        '''
        if not self._is_sim:
            return cf

        ngen = self._get_gen_nev()

        cf['reco'] = efficiency(nrec, ngen - nrec, cut=truth_string)

        return cf
    # ------------------------------------
    def _get_analysis(self):
        hlt2_nomva = self._hlt2.replace('_MVA', '')

        if hlt2_nomva.endswith('EE'):
            return 'EE'

        if hlt2_nomva.endswith('MuMu'):
            return 'MM'

        raise ValueError(f'Usupported HLT2 trigger: {hlt2_nomva}')
    # ----------------------------------------
    def _redefine_cut(self, cut_name : str, cut_value : str) -> str:
        '''
        Takes cut, checks if it is meant to be redefined, returns updated value
        '''
        if cut_name not in self._d_redefine_cuts:
            return cut_value

        new_cut = self._d_redefine_cuts[cut_name]
        cut     = new_cut

        log.warning(f'{cut_name:<20}{"->":<20}{cut:<100}')

        return cut
    # ----------------------------------------
    def get_rdf(self) -> RDataFrame:
        '''
        Returns ROOT dataframe after selection
        '''

        self._initialize()

        rdf   = self._get_df_raw()
        dfmgr = AtrMgr(rdf)

        cf    = cfl.cutflow(d_meta = {'file' : rdf.filepath, 'tree' : rdf.treename})
        tot   = rdf.Count().GetValue()
        d_cut = sel.selection(
                analysis = self._get_analysis(),
                project  = self._project,
                q2bin    = self._q2bin,
                process  = self._sample)

        d_cut = self._redefine_cuts(d_cut)

        log.info(f'Applying selection version: {self._cutver}')

        for cut_name, cut in d_cut.items():
            cut = self._redefine_cut(cut_name, cut)

            log.info(f'{"":<10}{cut_name:>20}')

            if cut_name == 'bdt':
                rdf, cut = self._filter_bdt(rdf, cut)
            else:
                rdf = rdf.Filter(cut, cut_name)

            pas=rdf.Count().GetValue()

            if cut_name == 'truth':
                cf = self._add_reco_efficiency(cf, pas, cut)
                tot= pas
                continue

            try:
                eff = efficiency(pas, tot - pas, cut=cut)
            except ZeroYields:
                log.error(f'Last cut ({cut}) passed zero events:')
                print(cf)
                raise

            cf[cut_name] = eff

            tot=pas

        rdf          = dfmgr.add_atr(rdf)
        rdf.treename = 'DecayTree'
        rdf.cf       = cf

        return rdf
# -----------------------------------------
