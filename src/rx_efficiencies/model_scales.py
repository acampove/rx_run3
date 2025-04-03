from importlib.resources import files
from datetime            import datetime
from log_store           import log_store
from version_management  import get_last_version 
from acceptance_reader   import reader            as acreader

import os
import re
import math
import glob
import ROOT
import numpy
import utils
import mplhep
import pprint
import pandas            as pnd
import jacobi            as jac
import utils_noroot      as utnr
import matplotlib.pyplot as plt

log=log_store.add_logger(name='rk_extractor:scales')
#------------------------------------------
def get_proc_labels():
    d_proc_lab         = dict() 
    d_proc_lab['bpkp'] = r'$B^+\to K^+ e^+e^-$'
    d_proc_lab['bpks'] = r'$B^+\to K^{*+}(\to K\pi^0) e^+e^-$'
    d_proc_lab['bdks'] = r'$B_d\to K^{*0}(\to K\pi) e^+e^-$'
    d_proc_lab['bsph'] = r'$B_s\to \phi(\to K K) e^+e^-$'
    d_proc_lab['bpk1'] = r'$B^+\to K_1(\to K \pi\pi) e^+e^-$'
    d_proc_lab['bpk2'] = r'$B^+\to K_2(\to K \pi\pi) e^+e^-$'

    return d_proc_lab
#------------------------------------------
class eff_calc:
    def __init__(self, proc=None, year=None, trig=None):
        self._l_proc = ['bpkp', 'bpks', 'bdks', 'bsph']                  if proc is None else proc
        self._l_trig = ['ETOS']                                          if trig is None else trig
        self._l_year = ['2011', '2012', '2015', '2016', '2017', '2018']  if year is None else year

        self._d_proc_lab = get_proc_labels()
        self._d_geo_eff  = {'Process' : [], 'Year' : [], 'Value' : [], 'Error' : []}
        self._d_sel_tis  = {'Process' : [], 'Year' : [], 'Value' : [], 'Error' : []}
        self._d_sel_tos  = {'Process' : [], 'Year' : [], 'Value' : [], 'Error' : []}
        self._dvers      = 'v10.21p2'
        self._min_date   = '01.09.23'
        self._out_dir    = None

        plt.style.use(mplhep.style.LHCb2)

        self._initialized=False
    #------------------------------------------
    @property
    def out_dir(self):
        return self._out_dir

    @out_dir.setter
    def out_dir(self, value):
        try:
            os.makedirs(value, exist_ok=True)
        except:
            log.error(f'Cannot make: {value}')
            raise

        self._out_dir = value
    #------------------------------------------
    def get_stats(self):
        '''
        Parameters
        ---------------------
        '''
        d_data = {'Process' : [], 'Trigger' : [], 'Year' : [], 'Passed' : [], 'Total' : []}
        for proc in self._l_proc: 
            for trig in self._l_trig: 
                for year in self._l_year: 
                    pas, tot = self._get_yields(proc=proc, trig=trig, year=year)

                    d_data['Process'].append(proc)
                    d_data['Trigger'].append(trig)
                    d_data['Year'   ].append(year)
                    d_data['Passed' ].append(pas)
                    d_data['Total'  ].append(tot)

        self._plot_geo_eff()
        self._plot_sel_eff()

        df = pnd.DataFrame(d_data)

        return df 
    #------------------------------------------
    def _plot_geo_eff(self):
        if self._out_dir is None:
            return

        df      = pnd.DataFrame(self._d_geo_eff)
        plt_dir = f'{self._out_dir}/plots'
        os.makedirs(plt_dir, exist_ok=True)

        ax=None
        for proc, df_p in df.groupby('Process'):
            ax=df_p.plot(x='Year', y='Value', yerr='Error', ax=ax, label=self._d_proc_lab[proc], marker='o')

        plt_path = f'{plt_dir}/geo_eff.png'
        log.info(f'Saving: {plt_path}')
        plt.grid()
        plt.ylim(0.00, 0.20)
        plt.savefig(plt_path)
        plt.close('all')
    #------------------------------------------
    def _plot_sel_eff(self):
        if self._out_dir is None:
            return

        plt_dir = f'{self._out_dir}/plots'
        os.makedirs(plt_dir, exist_ok=True)
        for trig, d_data in [('ETOS' , self._d_sel_tos)]:
            if trig not in self._l_trig:
                continue
            df = pnd.DataFrame(d_data) 

            ax=None
            for proc, df_p in df.groupby('Process'):
                ax=df_p.plot(x='Year', y='Value', yerr='Error', ax=ax, label=self._d_proc_lab[proc], marker='o')

            plt_path = f'{plt_dir}/sel_eff_{trig}.png'
            plt.grid()
            plt.savefig(plt_path)
            plt.close('all')
    #------------------------------------------
    def _switch_sample(self, proc, year):
        '''
        Switch samples for unavailable sim08 generator efficiencies
        '''
        if  proc in ['bpk1_ee', 'bpk2_ee'] and year in ['2011', '2012', '2015', '2016']:
            new_year = '2017'
            log.warning(f'Gen efficiency using {year} -> {new_year} for {proc}')
            year = new_year 

        return proc, year
    #------------------------------------------
    def _get_geo_eff(self, proc, year):
        obj = acreader(year=year, proc=proc)
        acc = obj.read()

        self._d_geo_eff['Process'].append(proc) 
        self._d_geo_eff['Year'   ].append(year) 
        self._d_geo_eff['Value'  ].append( acc) 
        self._d_geo_eff['Error'  ].append(   0) 

        return acc
    #------------------------------------------
    def _get_yields(self, proc=None, trig=None, year=None):
        sel      = self._get_sel_yld(proc, trig, year)
        gen      = self._get_gen_yld(proc,       year)
        geo_acc  = self._get_geo_eff(proc,       year)
        tot      = gen / geo_acc

        return sel, tot 
    #------------------------------------------
    def _get_sel_yld(self, proc, trig, year):
        proc     = 'sign' if proc == 'bpkp' else proc
        cas_dir  = os.environ['CASDIR']
        root_wc  = f'{cas_dir}/tools/apply_selection/rare_backgrounds/{proc}/{self._dvers}/{year}_{trig}/*.root'

        l_root   = glob.glob(root_wc)
        if len(l_root) == 0:
            log.error(f'No file found in: {root_wc}')
            raise

        rdf = ROOT.RDataFrame(trig, l_root)

        return rdf.Count().GetValue()
    #------------------------------------------
    def _get_date_flag(self, date):
        l_date = re.findall('\d{2}\.\d{2}\.\d{2}', date)
        if len(l_date) == 0:
            return False

        dstr = datetime.strptime

        return any( dstr(self._min_date, '%d.%m.%y') < dstr(date, '%d.%m.%y') for date in l_date)
    #------------------------------------------
    def _switch_yields(self, proc, year):
        if   proc in ['bpk1', 'bpk2'] and year in ['2015', '2016']:
            new_year = '2017'
            log.warning(f'Reco yields using {year} -> {new_year} for {proc}')
            year = new_year
        elif proc == 'bpk2' and year == '2011':
            new_year = '2012'
            log.warning(f'Reco yields using {year} -> {new_year} for {proc}')
            year = new_year

        return proc, year
    #------------------------------------------
    def _get_year_entries(self, df, proc, year):
        proc, year = self._switch_yields(proc, year)

        df_f = df
        df_f = df_f[(df_f.Year     == str(year)) | (df_f.Year     == int(year))]
        df_f = df_f[(df_f.Polarity == 'MagUp'  ) | (df_f.Polarity == 'MagDown')]

        if len(df_f) not in [1, 2]:
            log.error(f'Found more than two or fewer than one entries (polarities) after fully filtering')
            print(df)
            print(df.dtypes)
            log.info('--->')
            print(df_f)
            raise

        l_pol=df_f.Polarity.tolist()
        s_pol=set(l_pol)
        if   len(df_f) == 2 and s_pol != {'MagUp', 'MagDown'}:
            log.error(f'Wrong polarities: {s_pol}')
            raise ValueError
        elif len(df_f) == 1:
            log.warning(f'Found only polarity: {s_pol}')

        return df_f.Events.sum()
    #------------------------------------------
    def _get_gen_yld(self, proc, year):
        ganga_dir = os.environ['GANDIR']
        json_path = f'{ganga_dir}/job_stats/{proc}.json'

        df = pnd.read_json(json_path)
        df = df.loc[[ self._get_date_flag(date) for date in df.Dates ]]

        yld= self._get_year_entries(df, proc, year)

        return yld 
#------------------------------------------
class scales:
    def __init__(self, dset=None, trig=None, kind=None):
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
            log.error(f'{name} {val} not allowed')
            raise ValueError
        else:
            log.debug(f'{name:<20}{"->":20}{val:<20}')
    #------------------------------------------
    def _initialize(self):
        if self._initialized:
            return

        log.debug('Initializing')
        self._vers = self._get_fractions_version()

        self._load_fractions()
        self._load_efficiencies() 

        self._check_arg(self._l_kind, self._kind, 'Background')
        self._check_arg(self._l_dset, self._dset, 'Dataset')
        self._check_arg(self._l_trig, self._trig, 'Trigger')

        self._initialized = True
    #------------------------------------------
    def _get_fractions_version(self):
        frc_dir = files('extractor_data').joinpath(f'rare_sf')
        vers    = get_last_version(dir_path=frc_dir, version_only=True)

        log.debug(f'Using fractions version: {vers}')

        return vers
    #------------------------------------------
    def _load_fractions(self):
        log.debug(f'Using rare-signal scales version: {self._vers}')
        frbf_path    = files('extractor_data').joinpath(f'rare_sf/{self._vers}/fr_bf.json')
        self._d_frbf = utnr.load_json(frbf_path)
    #------------------------------------------
    def _load_efficiencies(self):
        log.debug('Getting efficiencies')

        eff_path = files('extractor_data').joinpath(f'rare_sf/{self._vers}/eff_real.json'  )
        if not os.path.isfile(eff_path):
            log.error(f'Cannot find: {eff_path}')
            raise FileNotFoundError

        df = pnd.read_json(eff_path)
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

