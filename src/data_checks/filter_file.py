import os
import tqdm
import ROOT
import utils_noroot          as utnr
import data_checks.utilities as utdc

from data_checks.selector  import selector
from log_store             import log_store

log = log_store.add_logger('data_checks:FilterFile')
# --------------------------------------
class FilterFile:
    '''
    Class used to pick a ROOT file path and produce a smaller version
    '''
    # --------------------------------------
    def __init__(self, kind=None, file_path=None, cfg_nam=None):
        self._kind        = kind
        self._file_path   = file_path
        self._cfg_nam     = cfg_nam

        self._nevts       = None
        self._is_mc       = None
        self._cfg_dat     = None
        self._l_line_name = None
        self._store_branch= None
        self._has_lumitree= None

        self._initialized = False
    # --------------------------------------
    def _initialize(self):
        if self._initialized:
            return

        if self._file_path is None:
            log.error('File path not set')
            raise FileNotFoundError

        self._cfg_dat = utdc.load_config(self._cfg_nam)

        self._check_mcdt()
        self._set_tree_names()
        self._set_save_pars()

        self._initialized = True
    # --------------------------------------
    def _check_mcdt(self):
        '''
        Will set self._is_mc flag based on config name
        '''
        if   self._cfg_nam.startswith('dt_'):
            self._is_mc = False
        elif self._cfg_nam.startswith('mc_'):
            self._is_mc = True
        else:
            log.error(f'Cannot determine Data/MC from config name: {self.cfg_nam}')
            raise
    # --------------------------------------
    def _set_save_pars(self):
        try:
            self._nevts = self._cfg_dat['saving']['max_events']
            log.info(f'Filtering dataframe with {self._nevts} entries')
        except KeyError:
            log.debug('Not filtering, max_events not specified')

        try:
            self._store_branch = self._cfg_dat['saving']['store_branch']
        except KeyError:
            log.debug('Not storing branches')
    # --------------------------------------
    def _get_names_from_config(self):
        '''
        Will return all the HLT line names from config
        '''
        d_l_name = self._cfg_dat['hlt_lines']
        l_name   = list()
        for val in d_l_name.values():
            l_name += val

        nline = len(l_name)
        log.debug(f'Found {nline} lines in config')

        return l_name
    # --------------------------------------
    def _set_tree_names(self):
        '''
        Will set the list of line names `self._l_line_name`
        '''
        ifile = ROOT.TFile.Open(self._file_path)
        l_key = ifile.GetListOfKeys()
        l_nam = [ key.GetName() for key in l_key]
        ifile.Close()

        self._has_lumitree = 'lumiTree' in l_nam

        l_hlt = [ hlt           for hlt in l_nam if hlt.startswith('Hlt2RD_') ]
        nline = len(l_hlt)
        log.debug(f'Found {nline} lines in file')

        l_tree_name = self._get_names_from_config()
        l_flt = [ flt           for flt in l_hlt if flt in l_tree_name  ]

        nline = len(l_flt)
        log.info(f'Found {nline} lines in file that match config')
        self._l_line_name = l_flt
    # --------------------------------------
    def _keep_branch(self, name):
        '''
        Will take the name of a branch and return True (keep) or False (drop)
        '''
        has_ccbar_const = ('DTF_PV_Jpsi_' in name) or ('DTF_PV_Psi2S' in name)
        if ('_DTF_PV_' in name) and not has_ccbar_const:
            return False

        if name.startswith('Hlt2') and not name.startswith('Hlt2RD'):
            return False

        if name.startswith('Hlt1') and ('Track' not in name):
            return False

        l_svar = self._cfg_dat['drop_branches']['starts_with']
        for svar in l_svar:
            if name.startswith(svar):
                return False

        l_ivar = self._cfg_dat['drop_branches']['includes'   ]
        for ivar in l_ivar:
            if ivar in name:
                return False

        return True
    # --------------------------------------
    def _rename_kaon_branches(self, rdf):
        '''
        Will define K_ = H_ for kaon branches. K_ branches will be dropped later
        '''

        v_name = rdf.GetColumnNames()
        l_name = [ name.c_str() for name in v_name ]
        l_kaon = [ name         for name in l_name if name.startswith('K_') ]

        for old in l_kaon:
            new = 'H_' + old[2:]
            rdf = rdf.Define(new, old)

        return rdf
    # --------------------------------------
    def _rename_mapped_branches(self, rdf):
        '''
        Will define branches from mapping in config. Original branches will be dropped later
        '''
        v_name = rdf.GetColumnNames()
        l_name = [ name.c_str() for name in v_name ]

        d_name = self._cfg_dat['rename']
        for org, new in d_name.items():
            if org not in l_name:
                continue

            rdf = rdf.Define(new, org)

        return rdf
    # --------------------------------------
    def _rename_branches(self, rdf):
        rdf = self._rename_kaon_branches(rdf)
        rdf = self._rename_mapped_branches(rdf)

        return rdf
    # --------------------------------------
    def _get_rdf(self, line_name):
        '''
        Will build a dataframe from a given HLT line and return the dataframe
        _get_branches decides what branches are kept
        '''
        rdf      = ROOT.RDataFrame(f'{line_name}/DecayTree', self._file_path)
        rdf      = self._rename_branches(rdf)
        rdf.lumi = False
        rdf      = self._attach_branches(rdf, line_name)
        l_branch = rdf.l_branch
        ninit    = rdf.ninit
        nfnal    = rdf.nfnal

        norg     = rdf.Count().GetValue()
        if not rdf.lumi:
            obj  = selector(rdf=rdf, cfg_nam=self._cfg_nam, is_mc=self._is_mc)
            rdf  = obj.run()
        nfnl     = rdf.Count().GetValue()

        log.debug(f'{line_name:<50}{ninit:<10}{"->":5}{nfnal:<10}{norg:<10}{"->":5}{nfnl:<10}')

        return rdf
    # --------------------------------------
    def _attach_branches(self, rdf, line_name):
        '''
        Will check branches in rdf
        Branches are dropped by only keeping branches in _keep_branch function
        line_name used to name file where branches will be saved.
        '''
        v_col = rdf.GetColumnNames()
        l_col = [ col.c_str() for col in v_col ]

        ninit = len(l_col)
        l_flt = [ flt         for flt in l_col if self._keep_branch(flt) ]
        nfnal = len(l_flt)

        rdf.ninit    = ninit
        rdf.nfnal    = nfnal
        rdf.l_branch = l_flt
        rdf.name     = line_name

        if self._store_branch:
            utnr.dump_json(l_flt, f'./{line_name}.json')

        return rdf
    # --------------------------------------
    def _save_file(self, l_rdf):
        '''
        Will save all ROOT dataframes to a file
        '''
        opts                   = ROOT.RDF.RSnapshotOptions()
        opts.fMode             = 'update'
        opts.fCompressionLevel = self._cfg_dat['saving']['compression']

        file_name = os.path.basename(self._file_path)
        for rdf in tqdm.tqdm(l_rdf, ascii=' -'):
            tree_name = rdf.name
            l_branch  = rdf.l_branch

            rdf.Snapshot(tree_name, f'{self._kind}_{file_name}', l_branch, opts)
    # --------------------------------------
    def _add_lumi_rdf(self, l_rdf):
        if not self._has_lumitree:
            return l_rdf

        rdf          = ROOT.RDataFrame('lumiTree', self._file_path)
        rdf.lumi     = True
        rdf.name     = 'lumiTree'
        rdf.l_branch = rdf.GetColumnNames()
        l_rdf.append(rdf)

        return l_rdf
    # --------------------------------------
    @utnr.timeit
    def run(self):
        '''
        Will run filtering of files
        '''
        self._initialize()

        log.info(f'Filtering: {self._file_path}')
        log.debug(100 * '-')
        log.debug(f'{"Line":<50}{"BOrg":<10}{"":5}{"BFnl":<10}{"#Org":<10}{"":5}{"#Fnl":<10}')
        log.debug(100 * '-')
        l_rdf = [ self._get_rdf(tree_name) for tree_name in self._l_line_name ]
        l_rdf = self._add_lumi_rdf(l_rdf)

        self._save_file(l_rdf)
# --------------------------------------
