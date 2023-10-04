import os
import ROOT
import utils
import numpy
import logging

import version_management as vmg
import utils_noroot       as utnr
import read_selection     as rs

from rk.wgt_mgr   import wgt_mgr
#----------------------------------------------
class data_set:
    log = utnr.getLogger(__name__)
    #----------------------------------------------
    def __init__(self, is_mc=None, trigger=None, dset=None):
        self._dset        = dset 
        self._is_mc       = is_mc
        self._trigger     = trigger 

        self._cas_dir     = None
        self._plt_dir     = None
        self._nentries    = -1 
        self._cal_sys     = 'nom'
        self._dat_vers    = 'v10.21p2'

        self._initialized = False
    #----------------------------------------------
    @property
    def nentries(self):
        return self._nentries

    @nentries.setter
    def nentries(self, value):
        if not isinstance(value, int):
            self.log.error(f'Value {value} cannot be used as number of entries')
            raise ValueError

        self._nentries = value
    #----------------------------------------------
    @property
    def plt_dir(self):
        return self._plt_dir

    @plt_dir.setter
    def plt_dir(self, value):
        try:
            os.makedirs(value, exist_ok=True)
        except:
            self.log.error(f'Cannot create directory: {value}')
            raise

        self._plt_dir = value
    #----------------------------------------------
    def _initialize(self):
        self._cas_dir = os.environ['CASDIR']

        if self._initialized:
            return

        if self._plt_dir is None:
            self.log.error('Plotting directory not specified, use plt_dir attribute')
            raise

        if self._is_mc   not in [True, False]:
            self.log.error(f'Invalid value for is_mc: {self._is_mc}')
            raise ValueError

        if self._trigger not in ['ETOS', 'GTIS']:
            self.log.error(f'Invalid value for trigger: {self._trigger}')
            raise ValueError

        if self._dset    not in ['r1', 'r2p1', '2017', '2018']:
            self.log.error(f'Invalid value for dset: {self._dset}')
            raise ValueError

        self._initialized = True
    #-------------------
    def _get_years(self):
        if   self._dset == 'r1':
            l_year = ['2011', '2012'] 
        elif self._dset == 'r2p1':
            l_year = ['2015', '2016'] 
        else:
            l_year = [self._dset] 
    
        return l_year
    #-------------------
    def _get_range_rdf(self, rdf):
        if self._nentries > 0:
            ntotal = rdf.Count().GetValue()
            self.log.visible(f'Using {self._nentries}/{ntotal} entries')
            rdf = rdf.Range(self._nentries)
    
        return rdf
    #-------------------
    def _get_cached_paths(self, proc):
        l_year    = self._get_years()
        cache_dir = f'{self._cas_dir}/tools/apply_selection/q2_smear'
        l_path    = []
        for year in l_year:
            wc      = f'{cache_dir}/{proc}/{self._dat_vers}/{year}_{self._trigger}/*.root'
            self.log.info(f'Using files in: {wc}')
            l_path += utnr.glob_wc(wc, allow_empty=False)
    
        all_found = True 
        for path in l_path:
            if not os.path.isfile(path):
                all_found=False
                break

        l_path.sort()
        l_year.sort()
    
        return l_path, l_year, all_found
    #-------------------
    def _check_nopid(self, rdf, year):
        '''Checks that PID cut has not been added
        '''
        self.log.info('Checking for PID cut')
        pid_cut = rs.get('pid', self._trigger, q2bin='none', year=year)
    
        rdf_org = rdf.Range(1000)
        rdf_cut = rdf.Filter(pid_cut, 'PID')
    
        norg = rdf_org.Count().GetValue()
        ncut = rdf_cut.Count().GetValue()
    
        if norg == ncut:
            self.log.error(f'PID cut already applied')
            rep = rdf_cut.Report()
            rep.Print()
            raise
    #-------------------
    def _add_brem(self, rdf):
        self.log.info('Adding Bremsstrahlung branch')
        v_col_name = rdf.GetColumnNames()
        l_col_name = [col_name.c_str() for col_name in v_col_name]

        if 'nbrem' in l_col_name:
            return rdf

        rdf = rdf.Define('nbrem', 'int ngamma = L1_BremMultiplicity + L2_BremMultiplicity; int nbrem = ngamma >= 2 ? 2 : ngamma; return nbrem')

        return rdf
    #-------------------
    def _add_weights(self, rdf):
        if self._dset not in ['r1', 'r2p1']:
            rdf = self._add_wgt_year(rdf, self._dset)
        else:
            rdf = self._add_wgt_dset(rdf)
    
        rdf = rdf.Redefine('weight', 'float(weight)')
        wgt = rdf.Sum('weight').GetValue()
        self.log.visible(f'Sum of weights: {wgt}')
    
        return rdf
    #-------------------
    def _add_wgt_year(self, rdf, year):
        self.log.info(f'Adding weights for {year}')
        if self._is_mc:
            rdf = self._add_sim_wgt_year(rdf, year)
        else:
            rdf = self._add_dat_wgt_year(rdf, year)

        return rdf
    #-------------------
    def _add_dat_wgt_year(self, rdf, year):
        pid_cut = rs.get('pid', self._trigger, q2bin='none', year=year)
        rdf     = rdf.Define('weight', pid_cut)

        return rdf
    #-------------------
    def _add_sim_wgt_year(self, rdf, year):
        rdf.filepath = 'no-path'
        rdf.treename = self._trigger
        rdf.trigger  = self._trigger 
        rdf.year     = year
    
        d_set            = {}
        d_set['val_dir'] = f'{self._plt_dir}/{year}'
        d_set['replica'] = 0
        d_set['bts_ver'] ='10'
        d_set['bts_sys'] ='nom'
        d_set['pid_sys'] = self._cal_sys 
        d_set['trk_sys'] = self._cal_sys 
        d_set['gen_sys'] = self._cal_sys 
        d_set['lzr_sys'] = self._cal_sys 
        d_set['hlt_sys'] = self._cal_sys 
        d_set['rec_sys'] = self._cal_sys 
    
        obj         = wgt_mgr(d_set)
        obj.log_lvl = logging.WARNING
        rsl         = obj.get_reader('sel', rdf)
    
        gen_syst, lzr_syst, hlt_syst, rec_syst = self._get_wgt_syst()
    
        arr_pid     = rsl('pid', self._cal_sys)
        arr_trk     = rsl('trk', self._cal_sys)
        arr_gen     = rsl('gen', gen_syst)
        arr_lzr     = rsl('lzr', lzr_syst)
        arr_hlt     = rsl('hlt', hlt_syst)
        arr_rec     = rsl('rec', rec_syst)
    
        arr_cal = arr_pid * arr_trk * arr_gen * arr_lzr * arr_hlt * arr_rec

        regex = '(tmva_.*|B_Hlt.*|B_DTFM.*)'
        rdf   = utils.add_df_column(rdf, arr_cal,  'weight', d_opt={'exclude_re' : regex})
    
        return rdf
    #-------------------
    def _add_wgt_dset(self, rdf):
        if  self._dset == 'r1':
            yr_1 = '2011'
            yr_2 = '2012'
        elif self._dset == 'r2p1':
            yr_1 = '2015'
            yr_2 = '2016'
        else:
            self.log.error(f'Invalid dataset: {self._dset}')
            raise
    
        rdf_1 = rdf.Filter(f'yearLabbel == {yr_1}')
        rdf_2 = rdf.Filter(f'yearLabbel == {yr_2}')
    
        rdf_1 = self._add_wgt_year(rdf_1, yr_1)
        rdf_2 = self._add_wgt_year(rdf_2, yr_2)
    
        self.log.info('Merging weights')
        arr_wgt_1 = rdf_1.AsNumpy(['weight'])['weight']
        arr_wgt_2 = rdf_2.AsNumpy(['weight'])['weight']
        arr_wgt   = numpy.concatenate([arr_wgt_1, arr_wgt_2], axis=0)
    
        self.log.info('Adding weights column')

        regex = '(tmva_.*|B_Hlt.*|B_DTFM.*)'
        rdf   = utils.add_df_column(rdf, arr_wgt,  'weight', d_opt={'exclude_re' : regex})
    
        return rdf
    #-------------------
    def _get_wgt_syst(self):
        '''Will return systematic for L0 and GEN weights corresponding to _cal_sys [nom, 000]
        '''
        if   self._cal_sys == 'nom':
            gen_syst = 'MTOS'
        elif self._cal_sys == '000':
            gen_syst = '000'
        else:
            self.log.error(f'Invalid systematic: {self._cal_sys}')
            raise
    
        if   self._cal_sys == '000':
            lzr_syst = '000'
        elif self._trigger == 'MTOS':
            lzr_syst = 'L0MuonTIS'
        elif self._trigger == 'ETOS':
            lzr_syst = 'L0ElectronTIS'
        elif self._trigger == 'GTIS':
            lzr_syst = 'L0TIS_EMMH.L0HadronElEL.L0ElectronTIS'
        else:
            log.error(f'Invalid HLT tag: {self._trigger}')
            raise
    
        if   self._cal_sys == 'nom':
            hlt_syst = self._trigger 
        elif self._cal_sys == '000':
            hlt_syst = '000'
        else:
            log.error(f'Invalid systematic: {self._cal_sys}')
            raise
    
        return gen_syst, lzr_syst, hlt_syst, hlt_syst
    #-------------------
    def _check_nonempty(self, rdf):
        nentries = rdf.Count().GetValue()

        if nentries == 0:
            self.log.error('Found empty dataset')
            raise
        else:
            self.log.info(f'Dataset has {nentries} entries')
    #-------------------
    def _apply_selection(self, rdf):
        l_year    = self._get_years()
        l_cut_str = []
        for year in l_year:
            bdt_cut = rs.get('bdt', self._trigger, q2bin='none', year=year)
            cut_str = f'({bdt_cut} == 1  && yearLabbel == {year})'
            l_cut_str.append(cut_str)

        cut_str = '||'.join(l_cut_str)

        self.log.debug(f'Using BDT cut: {cut_str}')

        rdf = rdf.Filter(cut_str)

        return rdf
    #-------------------
    def get_rdf(self):
        self._initialize()

        proc = 'ctrl' if self._is_mc else 'data'
        l_cache_path, l_year, all_exist = self._get_cached_paths(proc)
        if not all_exist:
            self.log.error('Not found all files')
            raise
    
        self.log.info(f'Found cached {len(l_cache_path)} files for trigger {self._trigger}')
        for cache_path in l_cache_path:
            self.log.debug(f'{cache_path}')

        rdf = ROOT.RDataFrame(self._trigger, l_cache_path)
        rdf = self._apply_selection(rdf)
        rdf = self._get_range_rdf(rdf)

        for year in l_year:
            self._check_nopid(rdf, year)
    
        rdf = self._add_brem(rdf) 
        rdf = self._add_weights(rdf)

        #rdf.Describe().Print()
    
        return rdf
#----------------------------------------------

