'''
Module holding RDFGetter class
'''
import os
import glob
import json
import pprint
import hashlib
import fnmatch
from importlib.resources import files

import dmu.generic.utilities as gut
from dmu.logging.log_store import LogStore

import yaml
from ROOT                  import RDF, RDataFrame, GetThreadPoolSize

log=LogStore.add_logger('rx_data:rdf_getter')
# ---------------------------------------------------
class AlreadySetColumns(Exception):
    '''
    Class used to raise exception when columns have already been defined.
    This is supposed to be done once per session
    '''
    def __init__(self, message : str):
        '''
        Takes message to show in exception
        '''
        super().__init__(message)
# ---------------------------------------------------
class RDFGetter:
    '''
    Class meant to load dataframes with friend trees

    This class has the following attributes:

    max_entries : Limits the number of entries that will be provided
    friends     : List of names of samples, to be treated as friend trees. By default this is None and everything will be processed
    main_tree   : Name of tree treated as the main tree when building dataframes with friend trees through `FromSpec`
    skip_adding_columns : By default false. If true, it will skip defining new columns.
    '''
    max_entries         = -1
    skip_adding_columns = False

    friends             : list[str]
    main_tree           : str

    JPSI_PDG_MASS    = 3096.90 # https://pdg.lbl.gov/2018/listings/rpp2018-list-J-psi-1S.pdf
    BPLS_PDG_MASS    = 5279.34 # https://pdg.lbl.gov/2022/tables/rpp2022-tab-mesons-bottom.pdf
    d_custom_columns : dict[str,str]
    # ---------------------------------------------------
    def __init__(self, sample : str, trigger : str, tree : str = 'DecayTree'):
        '''
        Sample: Sample's nickname, e.g. DATA_24_MagDown_24c2
        Trigger: HLT2 trigger, e.g. Hlt2RD_BuToKpEE_MVA
        Tree: E.g. DecayTree or MCDecayTree, default DecayTree
        '''
        self._sample          = sample
        self._trigger         = trigger
        self._samples         : dict[str,str]
        self._jpsi_pdg_mass   = RDFGetter.JPSI_PDG_MASS

        self._tree_name       = tree
        self._tmp_path        : str
        self._l_columns       : list[str]
        self._cfg             = self._load_config()
        self._main_tree       = self._get_main_tree()
        self._l_electron_only = self._cfg['trees']['electron_only']
        self._ext_weight      = '(L1_PID_E > 1 && L2_PID_E > 1) ? 1 : 10'

        self._l_ee_trigger    = ['Hlt2RD_BuToKpEE_MVA',
                                 'Hlt2RD_BuToKpEE_MVA_cal',
                                 'Hlt2RD_BuToKpEE_MVA_misid',
                                 'Hlt2RD_BuToKpEE_MVA_ext',
                                 'Hlt2RD_BuToKpEE_SameSign_MVA']

        self._l_mm_trigger    = ['Hlt2RD_BuToKpMuMu_MVA',
                                 'Hlt2RD_BuToKpMuMu_SameSign_MVA']

        self._analysis        = self._analysis_from_trigger()
        self._initialize()
    # ---------------------------------------------------
    def _get_main_tree(self):
        if not hasattr(RDFGetter, 'main_tree'):
            return self._cfg['trees']['main']

        log.warning(f'Overriding main tree with: {RDFGetter.main_tree}')

        return RDFGetter.main_tree
    # ---------------------------------------------------
    def _analysis_from_trigger(self) -> str:
        if self._trigger in self._l_mm_trigger:
            return 'MM'

        if self._trigger in self._l_ee_trigger:
            return 'EE'

        raise NotImplementedError(f'Cannot deduce analysis from trigger: {self._trigger}')
    # ---------------------------------------------------
    def _load_config(self) -> dict:
        config_path = files('rx_data_data').joinpath('rdf_getter/config.yaml')
        with open(config_path, encoding='utf-8') as ifile:
            cfg = yaml.safe_load(ifile)

        return cfg
    # ---------------------------------------------------
    def _skip_path(self, file_name : str) -> bool:
        if file_name in self._l_electron_only and 'MuMu' in self._trigger:
            return True

        return False
    # ---------------------------------------------------
    def _initialize(self) -> None:
        '''
        Function will:
        - Find samples, assuming they are in $ANADIR/Data/samples/*.yaml
        - Add them to the samples member of RDFGetter

        If no samples found, will raise FileNotFoundError
        '''
        self._check_multithreading()

        data_dir     = os.environ['ANADIR']
        cfg_wildcard = f'{data_dir}/Data/samples/*.yaml'
        l_config     = glob.glob(cfg_wildcard)
        npath        = len(l_config)
        if npath == 0:
            raise FileNotFoundError(f'No files found in: {cfg_wildcard}')

        d_sample = {}
        log.info('Adding samples, found:')
        for path in l_config:
            file_name   = os.path.basename(path)
            if self._skip_path(file_name):
                continue

            sample_name = file_name.replace('.yaml', '')
            d_sample[sample_name] = path
            log.info(f'    {sample_name}')

        d_sample        = self._filter_samples(d_sample)
        self._samples   = d_sample
    # ---------------------------------------------------
    def _check_multithreading(self) -> None:
        nthreads = GetThreadPoolSize()
        if nthreads > 1:
            raise ValueError(f'Cannot run with mulithreading, using {nthreads} threads')
    # ---------------------------------------------------
    def _get_tmp_path(self, identifier : str) -> str:
        '''
        This method creates paths to temporary config files in /tmp.
        Needed to configure creation of dataframes

        Parameters
        ----------------
        identifier : String identifying sample/file whose configuration will be stored

        Returns
        ----------------
        Path to JSON file that will be used to dump configuration
        '''
        samples_str = json.dumps(self._samples, sort_keys=True)
        identifier  = f'{samples_str}.{identifier}'

        bidentifier = identifier.encode()
        hsh         = hashlib.sha256(bidentifier)
        hsh         = hsh.hexdigest()

        tmp_dir     =  '/tmp/rx_data/rdf_getter'
        os.makedirs(tmp_dir, exist_ok=True)

        tmp_path    = f'{tmp_dir}/config_{self._sample}_{self._trigger}_{hsh}.json'

        log.debug(f'Using config JSON: {tmp_path}')

        return tmp_path
    # ---------------------------------------------------
    def _filter_samples(self, d_sample : dict[str,str]) -> dict[str,str]:
        if self._tree_name == 'DecayTree':
            return d_sample

        # MCDecayTree has no friends
        if self._tree_name == 'MCDecayTree':
            path = d_sample[self._main_tree]
            return {self._main_tree : path}

        raise ValueError(f'Invalid tree name: {self._tree_name}')
    # ---------------------------------------------------
    def _get_trigger_paths(self, d_trigger : dict[str,list[str]]) -> list[str]:
        if self._trigger in d_trigger:
            return d_trigger[self._trigger]

        if not self._trigger.endswith('_ext'):
            raise ValueError(f'Invalid trigger name {self._trigger}')

        # TODO: When misid trigger be processed also for MC, this has to be updated
        if not self._sample.startswith('DATA_24_'):
            trigger = self._trigger.replace('_ext', '')
            log.warning(f'For sample {self._sample} will use {trigger} instead of {self._trigger}')
            return d_trigger[trigger]

        log.debug(f'Found extended trigger: {self._trigger}')
        trig_misid    = self._trigger.replace('_ext', '_misid')
        trig_analysis = self._trigger.replace('_ext',       '')

        l_path = []
        l_path+= d_trigger[trig_analysis]
        l_path+= d_trigger[trig_misid   ]

        return l_path
    # ---------------------------------------------------
    def _get_section(self, yaml_path : str) -> dict:
        d_section = {'trees' : [self._tree_name]}

        with open(yaml_path, encoding='utf-8') as ifile:
            d_data = yaml.safe_load(ifile)

        l_path = []
        nopath = False
        nosamp = True
        for sample in d_data:
            if not fnmatch.fnmatch(sample, self._sample):
                continue

            nosamp = False
            try:
                d_trigger     = d_data[sample]
                l_path_sample = self._get_trigger_paths(d_trigger=d_trigger)
            except KeyError as exc:
                raise KeyError(f'Cannot access {yaml_path}:{sample}/{self._trigger}') from exc

            nsamp = len(l_path_sample)
            if nsamp == 0:
                log.error(f'No paths found for {sample} in {yaml_path}')
                nopath = True
            else:
                log.debug(f'Found {nsamp} paths for {sample} in {yaml_path}')

            l_path += l_path_sample

        if nopath:
            raise ValueError('Samples with paths missing')

        if nosamp:
            raise ValueError(f'Could not find any sample matching {self._sample} in {yaml_path}')

        d_section['files'] = l_path

        return d_section
    # ---------------------------------------------------
    def _skip_sample(self, sample : str) -> bool:
        if sample == self._main_tree:
            return False

        if not hasattr(RDFGetter, 'friends'):
            return False

        if not isinstance(RDFGetter.friends, list):
            raise ValueError(f'List of friend trees is not a list: {RDFGetter.friends}')

        return sample not in RDFGetter.friends
    # ---------------------------------------------------
    def _get_json_conf(self):
        d_data = {'samples' : {}, 'friends' : {}}

        log.info('Adding samples')
        for sample, yaml_path in self._samples.items():
            if self._skip_sample(sample=sample):
                log.warning(f'Skipping sample: {sample}')
                continue

            log.debug(f'{"":<4}{sample:<15}{yaml_path}')

            d_section = self._get_section(yaml_path)
            if sample == self._main_tree:
                d_data['samples'][sample] = d_section
            else:
                d_data['friends'][sample] = d_section

        with open(self._tmp_path, 'w', encoding='utf-8') as ofile:
            json.dump(d_data, ofile, indent=4, sort_keys=True)

        self._check_samples(samples=d_data)
    # ---------------------------------------------------
    def _check_samples(self, samples : dict) -> None:
        gut.dump_json(samples, '/tmp/debugging/rx_data/samples.yaml')

        l_path_main = samples['samples'][self._main_tree]['files']
        l_fname_main= [ os.path.basename(path) for path in l_path_main]
        nmain       = len(l_fname_main)

        fail = False
        for sample_name, sample in samples['friends'].items():
            l_path_frnd = sample['files']
            l_fname_frnd= [ os.path.basename(path) for path in l_path_frnd]

            nfrnd = len(l_path_frnd)
            if l_fname_main == l_fname_frnd:
                continue

            fail = True
            if nfrnd != nmain:
                log.error(f'{sample_name:<20}{nfrnd}')
                log.error(f'{"Main"     :<20}{nmain}')
            else:
                data = {'main' : l_fname_main, sample_name : l_fname_frnd}
                pprint.pprint(data)

                log.error('File matching did not work')

        if fail:
            raise ValueError('Samples check failed')
    # ---------------------------------------------------
    def _add_column(self, rdf: RDataFrame, name : str, definition : str) -> RDataFrame:
        if not hasattr(self, '_l_columns'):
            self._l_columns = [ name.c_str() for name in rdf.GetColumnNames() ]

        if name in self._l_columns:
            raise ValueError(f'Cannot add {name}={definition}, column already found')

        log.debug(f'Defining: {name}={definition}')

        try:
            rdf = rdf.Define(name, definition)
        except TypeError as exc:
            raise TypeError(f'Cannot define {name}={definition}') from exc

        self._l_columns.append(name)

        return rdf
    # ---------------------------------------------------
    def _define_common_columns(self, rdf : RDataFrame) -> RDataFrame:
        log.info('Adding common columns')

        d_def = self._cfg['definitions'][self._analysis]
        if hasattr(RDFGetter, 'd_custom_columns'):
            log.warning('Adding custom column definitions')
            d_def.update(RDFGetter.d_custom_columns)

        for name, definition in d_def.items():
            rdf = self._add_column(rdf, name, definition)

        # TODO: The weight (taking into account prescale) should be removed
        # for 2025 data
        if self._trigger.endswith('_ext'):
            log.info('Adding weight of 10 to MisID sample')
            rdf = rdf.Define('weight', self._ext_weight)
        else:
            rdf = rdf.Define('weight',              '1')

        return rdf
    # ---------------------------------------------------
    def _define_mc_columns(self, rdf : RDataFrame) -> RDataFrame:
        log.info('Adding MC only columns')
        if self._sample.startswith('DATA'):
            return rdf

        d_def = self._cfg['definitions']['MC']
        for var, expr in d_def.items():
            rdf = self._add_column(rdf=rdf, name=var, definition=expr)

        try:
            rdf = RDFGetter.add_truem(rdf=rdf)
        except TypeError as exc:
            raise TypeError(f'Cannot add TRUEM branches to {self._sample}/{self._trigger}') from exc

        return rdf
    # ---------------------------------------------------
    def _define_data_columns(self, rdf : RDataFrame) -> RDataFrame:
        log.info('Adding data only columns')

        if not self._sample.startswith('DATA'):
            return rdf

        d_def = self._cfg['definitions']['DATA']
        for name, definition in d_def.items():
            rdf = self._add_column(rdf, name, definition)

        return rdf
    # ---------------------------------------------------
    def _redefine_columns(self, rdf : RDataFrame) -> RDataFrame:
        log.info('Redefining columns')

        d_def = self._cfg['redefinitions']
        for name, definition in d_def.items():
            if name == 'block':
                log.warning('Sending pre-UT candidates to block 0')
            else:
                log.debug(f'Redefining: {name}={definition}')

            rdf = rdf.Redefine(name, definition)

        return rdf
    # ---------------------------------------------------
    def _add_columns(self, rdf : RDataFrame) -> RDataFrame:
        if RDFGetter.skip_adding_columns:
            log.warning('Not adding new columns')
            return rdf

        if self._tree_name != 'DecayTree':
            return rdf

        rdf = self._define_mc_columns(rdf=rdf)
        rdf = self._define_data_columns(rdf=rdf)

        # Common definitions need to happen after sample specific ones
        # e.g. TRACK_PT needs to be put in place before q2_track
        rdf = self._define_common_columns(rdf=rdf)

        # Redefinitions need to come after definitions
        # Because they might be in function of defined columns
        # E.g. q2 -> Jpsi_Mass
        rdf = self._redefine_columns(rdf=rdf)

        return rdf
    # ---------------------------------------------------
    def get_rdf(self) -> RDataFrame:
        '''
        Takes ROOT dataframe associated to MC sample:

        - Adds TRUEM branches missing

        Returns dataframe
        '''
        log.info('Adding TRUEM branches')

        tv_tp   = 'ROOT::Math::XYZVector'
        fv_tp   = 'ROOT::Math::PtEtaPhiM4D<double>'

        par_3d  =f'{tv_tp} PAR_3D(PAR_TRUEPX, PAR_TRUEPY, PAR_TRUEPZ); auto PAR_truept=PAR_3D.Rho(); auto PAR_trueeta=PAR_3D.Eta(); auto PAR_truephi=PAR_3D.Phi()'
        l1_3d   = par_3d.replace('PAR', 'L1')
        l2_3d   = par_3d.replace('PAR', 'L2')
        kp_3d   = par_3d.replace('PAR',  'H')

        lep_4d  =f'{fv_tp} PAR_4D(PAR_truept, PAR_trueeta, PAR_truephi, 0.511)'
        kpl_4d  =f'{fv_tp} PAR_4D(PAR_truept, PAR_trueeta, PAR_truephi, 493.7)'
        l1_4d   = lep_4d.replace('PAR', 'L1')
        l2_4d   = lep_4d.replace('PAR', 'L2')
        kp_4d   = kpl_4d.replace('PAR',  'H')

        lv      =f'ROOT::Math::LorentzVector<{fv_tp}>(PAR_4D)'
        lv1     = lv.replace('PAR', 'L1')
        lv2     = lv.replace('PAR', 'L2')
        lv3     = lv.replace('PAR',  'H')

        jps_4d  =f'auto jpsi_4d = {lv1} + {lv2};'
        bpl_4d  =f'auto bpls_4d = {lv1} + {lv2} + {lv3};'

        expr_jp =f'{l1_3d}; {l2_3d}         ; {l1_4d}; {l2_4d}         ; {jps_4d}; auto val = jpsi_4d.M(); return val!=val ? {RDFGetter.JPSI_PDG_MASS} : val'
        expr_bp =f'{l1_3d}; {l2_3d}; {kp_3d}; {l1_4d}; {l2_4d}; {kp_4d}; {bpl_4d}; auto val = bpls_4d.M(); return val!=val ? {RDFGetter.BPLS_PDG_MASS} : val'

        log.debug('Jpsi_TRUEM')
        log.debug('-->')
        log.debug(expr_jp)

        log.debug('B_TRUEM')
        log.debug('-->')
        log.debug(expr_bp)

        rdf = rdf.Define('Jpsi_TRUEM', expr_jp)
        rdf = rdf.Define(   'B_TRUEM', expr_bp)

        return rdf
    # ---------------------------------------------------
    @staticmethod
    def set_custom_columns(d_def : dict[str,str]) -> None:
        '''
        Defines custom columns that the getter class will use to
        provide dataframes
        '''
        if hasattr(RDFGetter, 'd_custom_columns'):
            raise AlreadySetColumns('Custom columns have already been set')

        log.warning('Defining custom columns')
        for column, definition in d_def.items():
            log.info(f'{column:<30}{definition}')

        RDFGetter.d_custom_columns = d_def
# ---------------------------------------------------
