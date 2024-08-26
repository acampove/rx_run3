import os
import toml
import tqdm
import ROOT
import pprint
import utils_noroot          as utnr
import data_checks.utilities as utdc

from data_checks.selector  import selector
from log_store             import log_store

log=log_store.add_logger('data_checks:FilterFile')
#--------------------------------------
class FilterFile:
    '''
    Class used to pick a ROOT file path and produce a smaller version
    '''
    #--------------------------------------
    def __init__(self, file_path=None, cfg_nam=None):
        self._file_path   = file_path 
        self._cfg_nam     = cfg_nam

        self._nevts       = None 
        self._cfg_dat     = None 
        self._l_line_name = None
        self._store_branch= None

        self._initialized = False
    #--------------------------------------
    def _initialize(self):
        if self._initialized:
            return

        self._cfg_dat = utdc.load_config(self._cfg_nam)

        self._set_tree_names()
        self._set_save_pars()

        self._initialized = True 
    #--------------------------------------
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
    #--------------------------------------
    def _get_names_from_config(self):
        '''
        Will return all the HLT line names from config
        '''
        d_l_name = self._cfg_dat['branches']
        l_name   = list()
        for val in d_l_name.values():
            l_name += val

        nline = len(l_name)
        log.debug(f'Found {nline} lines in config')

        return l_name
    #--------------------------------------
    def _set_tree_names(self):
        '''
        Will set the list of line names `self._l_line_name`
        '''
        ifile = ROOT.TFile.Open(self._file_path)
        l_key = ifile.GetListOfKeys()
        l_nam = [ key.GetName() for key in l_key]
        ifile.Close()

        l_hlt = [ hlt           for hlt in l_nam if hlt.startswith('Hlt2RD_') ]
        nline = len(l_hlt)
        log.debug(f'Found {nline} lines in file')

        l_tree_name = self._get_names_from_config()
        l_flt = [ flt           for flt in l_hlt if flt in l_tree_name  ]

        nline = len(l_flt)
        log.info(f'Found {nline} lines in file that match config')
        self._l_line_name = l_flt
    #--------------------------------------
    def _keep_branch(self, name):
        has_ccbar_const = ('DTF_PV_Jpsi_' in name) or ('DTF_PV_Psi2S' in name)
        if ('_DTF_PV_' in name) and not has_ccbar_const:
            return False

        if '_cone_indx_' in name:
            return False

        if '_DTF_PV_B_mass_const_' in name:
            return False

        if '_DTF_noPV_' in name:
            return False

        if 'ChargedIso' in name:
            return False

        if 'NeutralIso' in name:
            return False

        if 'SMOG2'      in name:
            return False

        if '_K2P_'  in name:
            return False

        if '_K2Pi_' in name:
            return False

        if '_KP2PK_' in name:
            return False

        if '_KMu2MuK_' in name:
            return False

        if '_P2K_' in name:
            return False

        if '_MuMu2KK_' in name:
            return False

        if '_MuMu2PiPi_' in name:
            return False

        if '_MuMuK2KKPi_' in name:
            return False

        if '_MuMuK2PiPiPi_' in name:
            return False

        if '_POS_COV_MATRIX_' in name:
            return False

        if '_MOM_POS_COV_MATRIX_' in name:
            return False

        if '_MOM_COV_MATRIX_' in name:
            return False

        if '_POSITION_STATEAT_' in name:
            return False

        if 'CutBasedIncl' in name:
            return False

        if '_VTXISO_' in name:
            return False

        if '_InclDet' in name:
            return False

        if '_RICH_THRESHOLD_' in name:
            return False

        if name.startswith('B_Hlt1'):
            return False

        if name.startswith('Lz_Hlt1'):
            return False

        if name.startswith('Lb_Hlt1'):
            return False

        if name.startswith('Jpsi_Hlt1'):
            return False

        if name.startswith('B_K2Pi_DTF_'):
            return False

        if name.startswith('Hlt2') and not name.startswith('Hlt2RD'):
            return False

        if name.startswith('Hlt1Di'):
            return False
        
        if name.startswith('Hlt1') and ('Track' not in name):
            return False

        if name.startswith('Hlt1BGI'):
            return False

        return True 
    #--------------------------------------
    def _range_rdf(self, rdf):
        '''
        Will use part of the tree only if max_events specified in saving section of config
        and if number is bigger than zero
        '''
        if self._nevts is None or self._nevts < 0:
            return rdf

        rdf = rdf.Range(self._nevts)

        return rdf
    #--------------------------------------
    def _get_rdf(self, line_name):
        '''
        Will build a dataframe from a given HLT line and return the dataframe
        _get_branches decides what branches are kept
        '''
        rdf      = ROOT.RDataFrame(f'{line_name}/DecayTree', self._file_path)
        rdf      = self._range_rdf(rdf)

        rdf      = self._attach_branches(rdf, line_name) 
        l_branch = rdf.l_branch
        ninit    = rdf.ninit
        nfnal    = rdf.nfnal

        norg     = rdf.Count().GetValue()
        obj      = selector(rdf=rdf, cfg_nam=self._cfg_nam) 
        rdf      = obj.run()
        nfnl     = rdf.Count().GetValue()

        log.debug(f'{line_name:<50}{ninit:<10}{"->":5}{nfnal:<10}{norg:<10}{"->":5}{nfnl:<10}')

        rdf.l_branch = l_branch
        rdf.name     = line_name

        return rdf
    #--------------------------------------
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
    #--------------------------------------
    def _save_file(self, l_rdf):
        '''
        Will save all ROOT dataframes to a file
        '''
        file_name = os.path.basename(self._file_path)
        opts      = ROOT.RDF.RSnapshotOptions()
        opts.fMode= 'update'
        opts.fCompressionLevel=self._cfg_dat['saving']['compression']

        for rdf in tqdm.tqdm(l_rdf, ascii=' -'):
            tree_name = rdf.name
            l_branch  = rdf.l_branch

            rdf.Snapshot(tree_name, file_name, l_branch, opts)
    #--------------------------------------
    @utnr.timeit
    def run(self):
        '''
        Will run filtering of files
        '''
        self._initialize()

        log.debug(100 * '-')
        log.debug(f'{"Line":<50}{"BOrg":<10}{"":5}{"BFnl":<10}{"#Org":<10}{"":5}{"#Fnl":<10}')
        log.debug(100 * '-')
        l_rdf = [ self._get_rdf(tree_name) for tree_name in self._l_line_name ]

        self._save_file(l_rdf)
#--------------------------------------























