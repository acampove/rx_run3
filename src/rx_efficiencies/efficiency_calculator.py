'''
Module containing EfficiencyCalculator class
'''
import os
import math
import mplhep
import pandas            as pnd
import matplotlib.pyplot as plt

from ROOT                              import RDataFrame
from dmu.logging.log_store             import LogStore
from rx_data.rdf_getter                import RDFGetter
from rx_selection                      import selection as sel
from rx_efficiencies.acceptance_reader import AcceptanceReader
from rx_efficiencies.decay_names       import DecayNames

log=LogStore.add_logger('rx_efficiencies:efficiency_calculator')
#------------------------------------------
class EfficiencyCalculator:
    '''
    Class used to calculate efficiencies for partially reconstructed samples
    '''
    #------------------------------------------
    def __init__(self, q2bin : str, d_cut : dict[str,str]=None):
        '''
        Proc: Nickname of process, if not passed, will do all processes.

        q2bin : Either low, central or high
        d_cut : Dictionary meant to be used to override default selection
        '''
        self._q2bin      = q2bin
        self._d_cut      = d_cut

        self._year       = '2024'
        self._d_sel      = {'Process' : [], 'Value' : [], 'Error' : []}
        self._l_proc     = DecayNames.get_decays()
        self._out_dir    = None
        self._trigger    = 'Hlt2RD_BuToKpEE_MVA'

        plt.style.use(mplhep.style.LHCb2)

        self._initialized=False
    #------------------------------------------
    @property
    def out_dir(self) -> str:
        '''
        Returns path to directory where validation plots should go
        '''
        return self._out_dir

    @out_dir.setter
    def out_dir(self, value : str):
        try:
            os.makedirs(value, exist_ok=True)
        except:
            log.error(f'Cannot make: {value}')
            raise

        self._out_dir = value
    #------------------------------------------
    def _get_title(self) -> str:
        if 'mva' in self._d_cut:
            wp    = self._d_cut['mva']
            title = f'$q^2$: {self._q2bin}; {wp}'
        else:
            title = f'$q^2$: {self._q2bin}'

        return title
    #------------------------------------------
    def _plot_sel_eff(self):
        if self._out_dir is None:
            return

        log.debug('Plotting selection efficiencies')

        os.makedirs(self._out_dir, exist_ok=True)

        df          = pnd.DataFrame(self._d_sel)
        df['Decay'] = df.Process.map(DecayNames.tex)
        df['Value'] = df.Value * 100
        df['Error'] = df.Error * 100
        df          = df.sort_values(by='Value')
        df.plot(x='Decay', y='Value', yerr='Error', figsize=(20, 8), kind='barh', legend=False)

        plt_path = f'{self._out_dir}/rec_sel_eff_{self._q2bin}.png'
        log.debug(f'Saving to: {plt_path}')

        title = self._get_title()

        plt.grid()
        plt.xlim(0, 1.8)
        plt.title(title)
        plt.ylabel('')
        plt.xlabel(r'$\varepsilon_{sel}$[%]')
        plt.tight_layout()
        plt.savefig(plt_path)
        plt.close('all')
    #------------------------------------------
    def _get_geo_eff(self, proc):
        obj = AcceptanceReader(year=self._year, proc=proc)
        acc = obj.read()

        return acc
    #------------------------------------------
    def _add_sel_eff(self, passed : int, total : int, proc : str) -> None:
        eff = passed / total
        err = math.sqrt(eff * (1 - eff) / total)

        self._d_sel['Process'].append(proc)
        self._d_sel['Value'  ].append(eff)
        self._d_sel['Error'  ].append(err)
    #------------------------------------------
    def _get_yields(self, proc=None) -> tuple[int,int]:
        sel_yld = self._get_sel_yld(proc)
        gen_yld = self._get_gen_yld(proc)
        geo_acc = self._get_geo_eff(proc)
        tot_yld = gen_yld / geo_acc

        self._add_sel_eff(passed=sel_yld, total=gen_yld, proc=proc)

        return sel_yld, tot_yld
    #------------------------------------------
    def _get_sel_yld(self, proc : str) -> int:
        sample = DecayNames.sample_from_decay(proc)
        rdf    = self._get_rdf(proc=proc, tree_name='DecayTree')

        d_sel  = sel.selection(project='RK', trigger=self._trigger, q2bin=self._q2bin, process=sample)
        if self._d_cut is not None:
            log.warning('Overriding default selection')
            d_sel.update(self._d_cut)

        for cut_name, cut_expr in d_sel.items():
            log.debug(f'{cut_name:<20}{cut_expr}')
            rdf = rdf.Filter(cut_expr, cut_name)

        return rdf.Count().GetValue()
    #------------------------------------------
    def _get_rdf(self, proc : str, tree_name : str) -> RDataFrame:
        sample = DecayNames.sample_from_decay(proc)

        gtr = RDFGetter(sample=sample, trigger=self._trigger, tree=tree_name)
        rdf = gtr.get_rdf()

        return rdf
    #------------------------------------------
    def _get_gen_yld(self, proc : str) -> int:
        rdf      = self._get_rdf(proc=proc, tree_name='MCDecayTree')
        nentries = rdf.Count().GetValue()

        return nentries
    #------------------------------------------
    def get_stats(self) -> pnd.DataFrame:
        '''
        Returns pandas dataframe with passed and total yields for a given process, year and trigger
        '''
        d_data = {'Process' : [], 'Passed' : [], 'Total' : []}
        for proc in self._l_proc:
            pas, tot = self._get_yields(proc=proc)

            d_data['Process'].append(proc)
            d_data['Passed' ].append(pas)
            d_data['Total'  ].append(tot)

        self._plot_sel_eff()

        df = pnd.DataFrame(d_data)

        return df
#------------------------------------------
