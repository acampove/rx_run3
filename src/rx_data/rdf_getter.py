'''
Module holding RDFGetter class
'''
import os
import glob
import json
import hashlib
import fnmatch
from importlib.resources import files

import yaml
from ROOT                  import RDF, RDataFrame
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('rx_data:rdf_getter')
# ---------------------------------------------------
class RDFGetter:
    '''
    Class meant to load dataframes with friend trees
    '''
    max_entries = -1
    # ---------------------------------------------------
    def __init__(self, sample : str, trigger : str, tree : str = 'DecayTree'):
        '''
        Sample: Sample's nickname, e.g. DATA_24_MagDown_24c2
        Trigger: HLT2 trigger, e.g. Hlt2RD_BuToKpEE_MVA
        Tree: E.g. DecayTree or MCDecayTree, default DecayTree
        '''
        self._sample      = sample
        self._trigger     = trigger
        self._samples     : dict[str,str]


        self._tree_name       = tree
        self._tmp_path        : str
        self._l_columns       : list[str]
        self._cfg             = self._load_config()
        self._main_tree       = self._cfg['trees']['main']
        self._l_electron_only = self._cfg['trees']['electron_only']

        self._ext_weight      = '(L1_PID_E > 1 && L2_PID_E > 1) ? 1 : 10'

        self._initialize()
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
        - Find samples, assuming they are in $DATADIR/samples/*.yaml
        - Add them to the samples member of RDFGetter

        If no samples found, will raise FileNotFoundError
        '''
        data_dir     = os.environ['DATADIR']
        cfg_wildcard = f'{data_dir}/samples/*.yaml'
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
        self._tmp_path  = self._get_tmp_path()
    # ---------------------------------------------------
    def _get_tmp_path(self) -> str:
        samples_str = json.dumps(self._samples, sort_keys=True)
        samples_bin = samples_str.encode()
        hsh         = hashlib.sha256(samples_bin)
        hsh         = hsh.hexdigest()
        tmp_path    = f'/tmp/config_{self._sample}_{self._trigger}_{hsh}.json'

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
    def _get_json_conf(self):
        d_data = {'samples' : {}, 'friends' : {}}

        log.info('Adding samples')
        for sample, yaml_path in self._samples.items():
            log.debug(f'    {sample}|{yaml_path}')

            d_section = self._get_section(yaml_path)
            if sample == 'main':
                d_data['samples'][sample] = d_section
            else:
                d_data['friends'][sample] = d_section

        with open(self._tmp_path, 'w', encoding='utf-8') as ofile:
            json.dump(d_data, ofile, indent=4, sort_keys=True)
    # ---------------------------------------------------
    def _add_column(self, rdf: RDataFrame, name : str, definition : str) -> RDataFrame:
        if not hasattr(self, '_l_columns'):
            self._l_columns = [ name.c_str() for name in rdf.GetColumnNames() ]

        if name in self._l_columns:
            log.debug(f'Not adding column {name}, already found')
            return rdf

        log.debug(f'Adding column {name}, not found')

        rdf = rdf.Define(name, definition)

        self._l_columns.append(name)

        return rdf
    # ---------------------------------------------------
    def _add_columns(self, rdf : RDataFrame) -> RDataFrame:
        if self._tree_name != 'DecayTree':
            return rdf

        for name, definition in self._cfg['definitions'].items():
            rdf = self._add_column(rdf, name, definition)

        log.warning('Sending pre-UT candidates to block 4')
        rdf = rdf.Redefine('block', 'block < 0 ? 4 : block')

        # TODO: The weight (taking into account prescale) should be removed
        # for 2025 data
        if self._trigger.endswith('_ext'):
            log.info('Adding weight of 10 to MisID sample')
            rdf = rdf.Define('weight', self._ext_weight)

        return rdf
    # ---------------------------------------------------
    def get_rdf(self) -> RDataFrame:
        '''
        Returns ROOT dataframe
        '''
        self._get_json_conf()

        log.debug(f'Building datarame from {self._tmp_path}')
        rdf = RDF.Experimental.FromSpec(self._tmp_path)
        if RDFGetter.max_entries > 0:
            log.warning(f'Returning dataframe with at most {RDFGetter.max_entries} entries')
            rdf = rdf.Range(RDFGetter.max_entries)

        rdf = self._add_columns(rdf)

        return rdf
# ---------------------------------------------------
