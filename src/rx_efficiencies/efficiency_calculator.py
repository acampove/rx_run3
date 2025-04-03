'''
Module containing EfficiencyCalculator class
'''
import os
import re
import glob
from datetime            import datetime
import ROOT
import mplhep
import matplotlib.pyplot as plt

from dmu.logging.log_store             import LogStore
from rx_efficiencies.acceptance_reader import AcceptanceReader

log=LogStore.add_logger('rx_efficiencies:efficiency_calculator')
#------------------------------------------
class EfficiencyCalculator:
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
        obj = AcceptanceReader(year=year, proc=proc)
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
