'''
Module holding RDFGetter class
'''
from contextlib import contextmanager
import os
import glob
import math
import json
import copy
import pprint
import hashlib
import fnmatch
from importlib.resources import files

import yaml
import dmu.generic.utilities as gut

from ROOT                  import RDF, RDataFrame, GetThreadPoolSize, TFile
from dmu.generic           import version_management as vmn
from dmu.generic           import hashing
from dmu.logging.log_store import LogStore
from rx_data.path_splitter import PathSplitter

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
    _max_entries         = -1
    _skip_adding_columns = False

    _custom_versions     : dict[str,str] = {}
    _main_tree           : str

    _cache_dir        = '/tmp/rx_data/cache/rdf_getter' # Here is where all the temporary output will go
    _excluded_friends = []
    _JPSI_PDG_MASS    = 3096.90 # https://pdg.lbl.gov/2018/listings/rpp2018-list-J-psi-1S.pdf
    _BPLS_PDG_MASS    = 5279.34 # https://pdg.lbl.gov/2022/tables/rpp2022-tab-mesons-bottom.pdf
    _d_custom_columns : dict[str,str]
    # ---------------------------------------------------
    def __init__(
            self,
            sample  : str,
            trigger : str,
            analysis: str = 'rx',
            tree    : str = 'DecayTree'):
        '''
        Sample: Sample's nickname, e.g. DATA_24_MagDown_24c2
        Trigger: HLT2 trigger, e.g. Hlt2RD_BuToKpEE_MVA
        Tree: E.g. DecayTree or MCDecayTree, default DecayTree
        '''
        self._sample          = sample
        self._trigger         = trigger
        self._analysis        = analysis
        self._samples         : dict[str,str]
        self._l_columns       : list[str]

        self._tree_name       = tree
        self._cfg             = self._load_config()
        self._main_tree       = self._get_main_tree()
        self._l_electron_only = self._cfg['trees']['electron_only']
        self._ext_weight      = '(L1_PID_E > 1 && L2_PID_E > 1) ? 1 : 10'

        self._l_ee_trigger    = [
                'Hlt2RD_BuToKpEE_MVA',
                'Hlt2RD_BuToKpEE_MVA_cal',
                'Hlt2RD_BuToKpEE_MVA_misid',
                'Hlt2RD_BuToKpEE_MVA_ext',
                'Hlt2RD_BuToKpEE_SameSign_MVA']

        self._l_mm_trigger    = ['Hlt2RD_BuToKpMuMu_MVA',
                                 'Hlt2RD_BuToKpMuMu_SameSign_MVA']

        self._rdf    : RDataFrame     # This is where the dataframe will be stored, prevents recalculation
        self._l_path : list[str] = [] # list of paths to all the ROOT files
        self._channel            = self._channel_from_trigger()
        self._initialize()
    # ---------------------------------------------------
    def _get_main_tree(self) -> str:
        if not hasattr(RDFGetter, '_main_tree'):
            return self._cfg['trees']['main']

        log.warning(f'Overriding main tree with: {RDFGetter._main_tree}')

        return RDFGetter._main_tree
    # ---------------------------------------------------
    def _channel_from_trigger(self) -> str:
        '''
        Returns EE or MM given the HLT2 trigger
        '''
        # noPID files should be assigned same channel as PID files
        trigger = self._trigger.removesuffix('_noPID')

        if trigger in self._l_mm_trigger:
            return 'MM'

        if trigger in self._l_ee_trigger:
            return 'EE'

        raise NotImplementedError(f'Cannot deduce channel from trigger: {self._trigger}')
    # ---------------------------------------------------
    def _load_config(self) -> dict:
        config_path = files('rx_data_data').joinpath('rdf_getter/config.yaml')
        config_path = str(config_path)
        with open(config_path, encoding='utf-8') as ifile:
            cfg = yaml.safe_load(ifile)

        return cfg
    # ---------------------------------------------------
    def _initialize(self) -> None:
        '''
        Function will:
        - Find samples, assuming they are in $ANADIR/Data/self._analysis as friend tree directories
        - Add them to the samples attribute of RDFGetter

        If no samples found, will raise FileNotFoundError
        '''
        os.makedirs(RDFGetter._cache_dir, exist_ok=True)
        self._check_multithreading()

        self._samples = self._get_yaml_paths()
    # ---------------------------------------------------
    def _get_yaml_paths(self) -> dict[str,str]:
        '''
        This function will return a dictionary with:

        key  : Name of sample, e.g. main, mva
        value: Path to YAML file with the directory structure needed to make an RDataFrame
        '''
        data_dir     = os.environ['ANADIR']
        ftree_wc     = f'{data_dir}/Data/{self._analysis}/*'
        l_ftree_dir  = glob.glob(ftree_wc)
        if len(l_ftree_dir) == 0:
            raise ValueError(f'No directories with samples found in: {ftree_wc}')

        d_ftree_dir  = { os.path.basename(ftree_dir) : ftree_dir for ftree_dir in l_ftree_dir }
        d_ftree_dir  = self._filter_samples(d_ftree_dir=d_ftree_dir)

        log.info(40 * '-')
        log.info(f'{"Friend":<20}{"Version":<20}')
        log.info(40 * '-')
        d_vers_dir   = { ftree_name : self._versioned_from_ftrees(ftree_dir)        for ftree_name, ftree_dir in d_ftree_dir.items() }
        d_yaml_path  = { ftree_name : self._yaml_path_from_ftree(dir_path=vers_dir) for ftree_name,   vers_dir in d_vers_dir.items()   }

        return d_yaml_path
    # ---------------------------------------------------
    def _versioned_from_ftrees(self, ftree_dir :  str) -> str:
        '''
        Takes path to directory corresponding to a friend tree.
        Finds latest/custom version and returns this path
        '''
        ftree = os.path.basename(ftree_dir)
        if ftree in RDFGetter._custom_versions:
            version     = RDFGetter._custom_versions[ftree]
            version_dir = f'{ftree_dir}/{version}'

            log.warning(f'{ftree:<20}{version:<20}')

            return version_dir

        version = vmn.get_last_version(dir_path=ftree_dir, version_only=True)
        log.info(f'{ftree:<20}{version:<20}')

        return f'{ftree_dir}/{version}'
    # ---------------------------------------------------
    def _yaml_path_from_ftree(self, dir_path : str) -> str:
        '''
        Takes path to directory with ROOT files associated to friend tree
        returns path to YAML file with correctly structured files
        '''
        l_root_path = glob.glob(f'{dir_path}/*.root')
        nroot_path  = len(l_root_path)
        if nroot_path == 0:
            raise ValueError(f'No ROOT files found in {dir_path}')

        spl  = PathSplitter(paths=l_root_path)
        data = spl.split(nested=True)
        val  = hashing.hash_object(data)
        val  = val[:10] # Ten characters are long enough for a hash

        out_path = f'{RDFGetter._cache_dir}/{val}.yaml'
        log.debug(f'Saving friend tree structure to {out_path}')

        # In a cluster, two jobs might interfere each other
        # If the YAML file was made by one job, do not make it in another
        if not os.path.isfile(out_path):
            gut.dump_json(data, out_path)

        return out_path
    # ---------------------------------------------------
    def _check_multithreading(self) -> None:
        nthreads = GetThreadPoolSize()
        if nthreads > 1:
            raise ValueError(f'Cannot run with mulithreading, using {nthreads} threads')
    # ---------------------------------------------------
    def _filter_samples(self, d_ftree_dir : dict[str,str]) -> dict[str,str]:
        '''
        Parameters
        --------------
        d_ftree_dir: Dictionary where:
            key : Is the friend tree name
            val : Is the path to the directory with the friend trees

        Returns
        --------------
        Same as input, but after filtering for not needed samples
        '''
        d_ftree_dir_flt = { ftree : ftree_dir for ftree, ftree_dir in d_ftree_dir.items() if not self._skip_ftree(ftree=ftree) }

        if self._tree_name == 'DecayTree':
            return d_ftree_dir_flt

        # MCDecayTree has no friends
        if self._tree_name == 'MCDecayTree':
            path = d_ftree_dir_flt[self._main_tree]
            return {self._main_tree : path}

        raise ValueError(f'Invalid tree name: {self._tree_name}')
    # ---------------------------------------------------
    def _get_trigger_paths(
            self,
            sample    : str,
            ftree     : str,
            d_trigger : dict[str,list[str]]) -> list[str]:
        '''
        Parameters
        ----------------
        d_trigger : Dictionary mapping HLT2 trigger names to lists of ROOT files
        sample    : Name of sample, e.g Bu_Kp...
        ftree     : Name of friend tree, e.g. mva

        Returns
        ----------------
        Gets list of paths to ROOT files for a given HLT2 trigger
        '''
        if self._trigger in d_trigger:
            return d_trigger[self._trigger]

        if not self._trigger.endswith('_ext'):
            raise ValueError(f'Invalid trigger name {self._trigger} for sample {sample} and friend tree {ftree}')

        # TODO: When misid trigger be processed also for MC, this has to be updated
        if not self._sample.startswith('DATA_24_'):
            trigger = self._trigger.replace('_ext', '')
            log.warning(f'For sample {self._sample} will use {trigger} instead of {self._trigger}')
            return d_trigger[trigger]

        log.debug(f'Found extended trigger: {self._trigger}')
        trig_misid   = self._trigger.replace('_ext', '_misid')
        trig_channel = self._trigger.replace('_ext',       '')

        l_path = []
        l_path+= d_trigger[trig_channel]
        l_path+= d_trigger[trig_misid  ]

        return l_path
    # ---------------------------------------------------
    def _get_section(
            self,
            yaml_path : str,
            ftree     : str) -> dict:
        '''
        This method should return the different sections (friend/main tree)
        needed to make the JSON file taken by FromSpec

        Parameters:
        --------------------
        yaml_path : Path to YAML file specifying samples:trigger:files
        ftree     : Friend tree name, e.g mva, main
        '''
        d_section = {'trees' : [self._tree_name]}

        log.debug(f'Building section from: {yaml_path}')
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
                l_path_sample = self._get_trigger_paths(
                        d_trigger= d_trigger,
                        ftree    = ftree,
                        sample   = sample)
            except KeyError as exc:
                raise KeyError(f'For friend tree {ftree}, cannot access {yaml_path}:{sample}/{self._trigger}') from exc

            nsamp = len(l_path_sample)
            if nsamp == 0:
                log.error(f'No paths found for {sample} in {yaml_path} and friend tree {ftree}')
                nopath = True
            else:
                log.debug(f'Found {nsamp} paths for {sample} in {yaml_path}')

            l_path += l_path_sample

        if nopath:
            raise ValueError('Samples with paths missing')

        if nosamp:
            raise ValueError(f'Could not find any sample matching {self._sample} with friend tree {ftree} in {yaml_path}')

        self._l_path      += l_path
        d_section['files'] = l_path

        return d_section
    # ---------------------------------------------------
    def _skip_ftree(self, ftree : str) -> bool:
        '''
        Will decide if a friend tree should be skipped

        Parameters
        ----------------
        ftree: Name of friend tree, e.g. mva
        '''
        if ftree == 'not_used': # This is the directory where old samples will go
            return True

        if ftree == self._main_tree:
            return False

        if ftree in RDFGetter._excluded_friends:
            log.warning(f'Excluding friend tree: {ftree}')
            return True

        if ftree in self._l_electron_only and 'MuMu' in self._trigger:
            log.info(f'Excluding friend tree {ftree} for muon trigger {self._trigger}')
            return True

        return False
    # ---------------------------------------------------
    def _get_paths_to_conf(self, per_file : bool) -> dict[str,str]:
        '''
        Parameters
        ----------------------
        per_file : If true will process configs per file, otherwise it will do the full sample

        Returns
        ----------------------
        Dictionary with:

        key  : Path to the ROOT file, '' if per_file is False
        value: Path to JSON config file, needed to build dataframe though FromSpec
        '''
        d_data = self._get_samples()

        if not per_file:
            log.debug('Not splitting per file')
            cfg_path = RDFGetter.get_tmp_path(identifier='full_sample', data=d_data)
            with open(cfg_path, 'w', encoding='utf-8') as ofile:
                json.dump(d_data, ofile, indent=4, sort_keys=True)

            return {'' : cfg_path}

        log.debug('Splitting per file')
        return RDFGetter.split_per_file(data=d_data, main=self._main_tree)
    # ---------------------------------------------------
    def _get_samples(self) -> dict:
        '''
        Returns a dictionary with information on the main samples and the friend trees, needed to build dataframes
        '''
        d_data = {'samples' : {}, 'friends' : {}}

        log.info('Adding samples')
        for ftree, yaml_path in self._samples.items():
            log.debug(f'{"":<4}{ftree:<15}{yaml_path}')

            d_section = self._get_section(yaml_path=yaml_path, ftree=ftree)
            if ftree == self._main_tree:
                d_data['samples'][ftree] = d_section
            else:
                d_data['friends'][ftree] = d_section

        return d_data
    # ---------------------------------------------------
    def _skip_brem_track_2_definition(self, name: str, definition : str) -> bool:
        '''
        Parameters
        -------------------
        name      : Name of variable to be defined
        definition: Definition...

        Returns
        -------------------
        True: This definition is not possible, due to absence of brem_track_2
        False: Definition possible
        '''

        if 'brem_track_2' not in RDFGetter._excluded_friends:
            return False

        # Variables containing these in their definitions, cannot be defined
        # without brem_track_2
        l_substr = ['brem_track_2', '_smr ', 'Jpsi_Mass', 'B_Mass']

        for substr in l_substr:
            if substr in definition:
                log.debug(f'Skipping definition {name}={definition}')
                return True

        log.debug(f'Not skipping definition {name}={definition}')

        return False
    # ---------------------------------------------------
    def _add_column(self, rdf: RDataFrame, name : str, definition : str) -> RDataFrame:
        '''
        Wrapper function to Define
        '''
        if self._skip_brem_track_2_definition(name=name, definition=definition):
            return rdf

        if name in self._l_columns:
            raise ValueError(f'Cannot add {name}={definition}, column already found')

        log.debug(f'Defining: {name}={definition}')
        rdf = rdf.Define(name, definition)
        self._l_columns.append(name)

        return rdf
    # ---------------------------------------------------
    def _define_common_columns(self, rdf : RDataFrame) -> RDataFrame:
        log.info('Adding common columns')

        d_def = self._cfg['definitions'][self._channel]
        if hasattr(RDFGetter, '_d_custom_columns'):
            log.warning('Adding custom column definitions')
            d_def.update(RDFGetter._d_custom_columns)

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
        if self._sample.startswith('DATA'):
            log.debug(f'Not adding MC only columns for: {self._sample}')
            return rdf

        log.info('Adding MC only columns')
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
        if not self._sample.startswith('DATA'):
            log.info(f'Not adding data columns for: {self._sample}')
            return rdf

        log.info('Adding data only columns')
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
                log.debug('Sending pre-UT candidates to block 0')
            else:
                log.debug(f'Redefining: {name}={definition}')

            if self._skip_brem_track_2_definition(name=name, definition=definition):
                continue

            rdf = rdf.Redefine(name, definition)

        return rdf
    # ---------------------------------------------------
    def _add_columns(self, rdf : RDataFrame) -> RDataFrame:
        if RDFGetter._skip_adding_columns:
            log.warning('Not adding new columns')
            return rdf

        if self._tree_name != 'DecayTree':
            log.debug(f'Not adding columns to {self._tree_name}')
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
    def _rdf_from_conf(self, conf_path : str) -> RDataFrame:
        '''
        Parameters
        ------------------
        conf_path: Path to JSON file with configuration needed to build dataframe

        Returns
        ------------------
        Dataframe after some basic preprocessing
        '''
        log.debug(f'Building dataframe from {conf_path}')
        rdf = RDF.Experimental.FromSpec(conf_path)

        self._l_columns = [name.c_str() for name in rdf.GetColumnNames() ]
        log.debug(f'Dataframe at: {id(rdf)}')

        rdf = self._filter_dataframe(rdf=rdf)
        rdf = self._add_columns(rdf=rdf)

        return rdf
    # ---------------------------------------------------
    def _filter_dataframe(self, rdf : RDataFrame) -> RDataFrame:
        '''
        Parameters
        ------------
        rdf :  DataFame built from JSON spec file

        Returns
        ------------
        Dataframe after optional filter
        '''
        nentries = rdf.Count().GetValue()

        if RDFGetter._max_entries < 0 or RDFGetter._max_entries > nentries:
            return rdf

        frac = RDFGetter._max_entries / nentries
        part = math.ceil(1.0 / frac)

        log.warning(f'Returning dataframe with around {RDFGetter._max_entries} entries')
        log.debug(f'Filter 1 / {part} entries => {frac:.3f} fraction')

        rdf  = rdf.Filter(f'rdfentry_ % {part} == 0', f'random_{part:02}_part')
        nentries = rdf.Count().GetValue()
        log.warning(f'New dataset size: {nentries}')

        return rdf
    # ---------------------------------------------------
    def get_rdf(
            self,
            per_file : bool = False) -> RDataFrame|dict[str,RDataFrame]:
        '''
        Returns sample in the form of dataframes

        Parameters
        -----------------
        per_file : Flag controlling returned object

        Returns
        -----------------
        Based on `per_file` flag it will return:

        - A dictionary with the key as the path to the ROOT file and the value as the dataframe
        - The dataframe for the full sample
        '''
        if hasattr(self, '_rdf'):
            log.debug('Returned already calculated dataframe')
            return self._rdf

        # This is a dictionary with:
        #
        # key  : Path to ROOT file from the main sample, if per_file==True. Otherwise empty string
        # Value: Path to config used to build DataFrame
        d_sample = self._get_paths_to_conf(per_file=per_file)
        if per_file:
            log.info('Building one dataframe per file')
            d_rdf = { fpath : self._rdf_from_conf(conf_path) for fpath, conf_path in d_sample.items() }

            return d_rdf

        nconf = len(d_sample)
        if nconf != 1:
            raise ValueError(f'Sample-wise config dictionary expects only one entry, found {nconf}')

        _, conf_path = next(iter(d_sample.items()))
        log.debug(f'Building datarame from file {conf_path}')

        self._rdf = self._rdf_from_conf(conf_path)

        return self._rdf
    # ---------------------------------------------------
    def get_uid(self) -> str:
        '''
        Retrieves unique identifier for this sample
        Build on top of the UUID from each file
        '''
        self.get_rdf() # Full RDF calculation needs to kick in before calculating GUID

        if len(self._l_path) == 0:
            raise ValueError('No path to ROOT files was found')

        log.debug('Calculating GUUIDs')
        all_guuid = ''
        for path in self._l_path:
            ifile = TFile(path)
            all_guuid += ifile.GetUUID().AsString()
            ifile.Close()

        val = hashing.hash_object(all_guuid)
        val = val[:10]

        return val
    # ---------------------------------------------------
    @staticmethod
    def add_truem(rdf : RDataFrame) -> RDataFrame:
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

        expr_jp =f'{l1_3d}; {l2_3d}         ; {l1_4d}; {l2_4d}         ; {jps_4d}; auto val = jpsi_4d.M(); return val!=val ? {RDFGetter._JPSI_PDG_MASS} : val'
        expr_bp =f'{l1_3d}; {l2_3d}; {kp_3d}; {l1_4d}; {l2_4d}; {kp_4d}; {bpl_4d}; auto val = bpls_4d.M(); return val!=val ? {RDFGetter._BPLS_PDG_MASS} : val'

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
        if hasattr(RDFGetter, '_d_custom_columns'):
            raise AlreadySetColumns('Custom columns have already been set')

        log.warning('Defining custom columns')
        for column, definition in d_def.items():
            log.info(f'{column:<30}{definition}')

        RDFGetter._d_custom_columns = d_def
    # ---------------------------------------------------
    @staticmethod
    def split_per_file(data : dict, main : str) -> dict[str,str]:
        '''
        Parameters
        --------------------
        data: Dictionary representing _spec_ needed to build ROOT dataframe with friend trees
        main: Name of the main category, e.g. not the friend trees.

        Returns
        --------------------
        Dictionary with the:

        key  : As the ROOT file path in the main category
        Value: The path to the JSON config file
        '''
        try:
            l_file = data['samples'][main]['files']
        except KeyError as exc:
            pprint.pprint(data)
            raise KeyError('Cannot access list of files from JSON config needed by FromSpec') from exc

        nfiles = len(l_file)

        d_config = {}
        for ifile in range(nfiles):
            data_copy, fpath = RDFGetter._remove_all_but(data, ifile, main)
            cpath            = RDFGetter.get_tmp_path(identifier=str(ifile), data=data_copy)
            gut.dump_json(data_copy, cpath)
            d_config[fpath]  = cpath

        return d_config
    # ---------------------------------------------------
    @staticmethod
    def _remove_all_but(data : dict, ifile : int, main : str) -> tuple[dict,str]:
        '''
        Will:

        - Take the file specification structure `data`
        - Make a local copy
        - Remove all the paths except the ifile th entry
        - Return the copy after removal alongside the path not removed AND beloging to the main sample
        '''

        datac = copy.deepcopy(data)
        fpath = data['samples'][main]['files'][ifile]

        datac['samples'][main]['files'] = [fpath]

        data_frnd = data['friends']
        for kind, data_kind in data_frnd.items():
            fpath = data_kind['files'][ifile]
            datac['friends'][kind]['files'] = [fpath]

        return datac, fpath
    # ---------------------------------------------------
    @staticmethod
    def get_tmp_path(identifier : str, data : dict) -> str:
        '''
        This method creates paths to temporary config files in /tmp.
        Needed to configure creation of dataframes

        Parameters
        ----------------
        identifier : String identifying sample/file whose configuration will be stored
        data       : Dictionary with structure as needed by ROOT to make dataframe with friend trees

        Returns
        ----------------
        Path to JSON file that will be used to dump configuration
        '''
        samples_str = json.dumps(data, sort_keys=True)
        identifier  = f'{samples_str}.{identifier}'

        bidentifier = identifier.encode()
        hsh         = hashlib.sha256(bidentifier)
        hsh         = hsh.hexdigest()
        hsh         = hsh[:10]
        tmp_path    = f'{RDFGetter._cache_dir}/config_{hsh}.json'

        log.debug(f'Using config JSON: {tmp_path}')

        return tmp_path
    # ---------------------------------------------------
    @contextmanager
    @staticmethod
    def skip_adding_columns(value : bool):
        '''
        Contextmanager to control if column (re)definitions from config are used or not

        value: If true it will not define any column in dataframe, i.e. this is what is in the ROOT files, False by default
        '''
        old_val = RDFGetter._skip_adding_columns
        try:
            RDFGetter._skip_adding_columns = value
            yield
        finally:
            RDFGetter._skip_adding_columns = old_val
    # ---------------------------------------------------
    @contextmanager
    @staticmethod
    def exclude_friends(names : list[str]):
        '''
        It will build the dataframe, excluding the friend trees
        in the `names` list
        '''
        old_val = RDFGetter._excluded_friends
        try:
            RDFGetter._excluded_friends = names
            yield
        finally:
            RDFGetter._excluded_friends = old_val
    # ---------------------------------------------------
    @contextmanager
    @staticmethod
    def custom_friends(versions : dict[str,str]):
        '''
        It will pick a dictionary between:

        key: Friend tree names, e.g. mva
        val: Versions, e.g. v5

        and override the version used for this friend tree
        '''
        old_val = RDFGetter._custom_versions
        try:
            RDFGetter._custom_versions = versions
            yield
        finally:
            RDFGetter._custom_versions = old_val
# ---------------------------------------------------
