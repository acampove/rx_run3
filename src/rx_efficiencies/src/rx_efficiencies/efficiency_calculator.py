'''
Module containing EfficiencyCalculator class
'''
import os
import math
from typing import cast

import mplhep
import pandas            as pnd
import matplotlib.pyplot as plt

from ROOT                              import RDF # type: ignore
from dmu.logging.log_store             import LogStore
from dmu.workflow.cache                import Cache
from dmu.generic                       import hashing

from rx_common.types                   import Trigger
from rx_data.rdf_getter                import RDFGetter
from rx_selection                      import selection as sel
from rx_efficiencies.acceptance_reader import AcceptanceReader
from rx_efficiencies.decay_names       import DecayNames

log=LogStore.add_logger('rx_efficiencies:efficiency_calculator')
#------------------------------------------
class EfficiencyCalculator(Cache):
    '''
    Class used to calculate efficiencies for:

    - Full nominal selection
    - Full selection defined as the product of the acceptance, selection and reconstruction efficiencies
    '''
    #------------------------------------------
    def __init__(
        self, 
        q2bin   : str, 
        trigger : Trigger  =  Trigger.rk_ee_os,
        sample  : str|None = None):
        '''
        Parameters
        -----------------
        q2bin   : Either low, central or high
        trigger : By default Hlt2RD_BuToKpEE_MVA
        sample  : MC sample for which the efficiency is calculated, e.g. Bu_JpsiK_ee_eq_DPC. 
                 If None, will calculate it for all samples found through DecayNames.get_decays() 
        '''
        self._q2bin      = q2bin
        self._year       = '2024'
        self._trigger    = trigger 
        self._d_sel      = {'Process' : [], 'Value' : [], 'Error' : []}
        self._l_proc     = DecayNames.get_decays() if sample is None else [DecayNames.nic_from_sample(sample=sample)]

        plt.style.use(mplhep.style.LHCb2)

        super().__init__(
            out_path = f'efficiencies/{q2bin}_{self._year}',
            d_sel    = self._get_selection_hash(),
            q2bin    = self._q2bin,
            trigger  = self._trigger,
            sample   = sample)

        self._initialized=False
    #------------------------------------------
    def _get_selection_hash(self) -> str:
        '''
        Returns
        ------------
        Hash associated to selection for all the processes used, trigger and q2bin
        '''
        hsh = ''
        for decay in self._l_proc:
            sample = DecayNames.sample_from_decay(decay=decay)
            d_sel  = sel.selection(q2bin=self._q2bin, trigger=self._trigger, process=sample)
            hsh   += hashing.hash_object(d_sel)

        return hsh
    #------------------------------------------
    @property
    def out_dir(self) -> str|None:
        '''
        Returns path to directory where validation plots should go
        '''
        if not hasattr(self, '_out_dir'):
            return None

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
    def _plot_sel_eff(self) -> None:
        '''
        Will plot selection efficiency for each process if _out_dir was specified
        '''
        if not hasattr(self, '_out_dir'):
            log.info('No plotting directory passed through out_dir property, skipping plots')
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

        title = f'$q^2$: {self._q2bin}'

        plt.grid()
        plt.xlim(0, 1.3)
        plt.title(title)
        plt.ylabel('')
        plt.xlabel(r'$\varepsilon_{sel}$[%]')
        plt.tight_layout()
        plt.savefig(plt_path)
        plt.close('all')
    #------------------------------------------
    def _get_geo_eff(self, proc : str) -> float:
        obj = AcceptanceReader(year=self._year, proc=proc)
        acc = obj.read()

        return acc
    #------------------------------------------
    def _add_sel_eff(
            self,
            passed : int,
            total  : int,
            proc   : str) -> None:
        eff = passed / total
        err = math.sqrt(eff * (1 - eff) / total)

        self._d_sel['Process'].append(proc)
        self._d_sel['Value'  ].append(eff)
        self._d_sel['Error'  ].append(err)
    #------------------------------------------
    def _get_yields(self, proc : str) -> tuple[int,int]:
        sel_yld = self._get_sel_yld(proc)
        gen_yld = self._get_gen_yld(proc)
        geo_acc = self._get_geo_eff(proc)
        tot_yld = int(gen_yld / geo_acc)

        self._add_sel_eff(passed=sel_yld, total=gen_yld, proc=proc)

        return sel_yld, tot_yld
    #------------------------------------------
    def _get_sel_yld(self, proc : str) -> int:
        sample   = DecayNames.sample_from_decay(proc)
        rdf      = self._get_rdf(proc=proc, tree_name='DecayTree')
        d_sel    = sel.selection(trigger=self._trigger, q2bin=self._q2bin, process=sample)

        log.info('Calculating selected yield')
        for cut_name, cut_expr in d_sel.items():
            log.debug(f'{cut_name:<20}{cut_expr}')
            rdf = rdf.Filter(cut_expr, cut_name)

        if log.getEffectiveLevel() < 20:
            rep = rdf.Report()
            rep.Print()

        nsel = rdf.Count().GetValue()

        return nsel
    #------------------------------------------
    def _get_rdf(self, proc : str, tree_name : str) -> RDF.RNode:
        sample = DecayNames.sample_from_decay(proc)

        gtr = RDFGetter(
            sample  = sample, 
            trigger = self._trigger, 
            tree    = tree_name)
        rdf = gtr.get_rdf(per_file=False)

        return rdf
    #------------------------------------------
    def _get_gen_yld(self, proc : str) -> int:
        rdf      = self._get_rdf(proc=proc, tree_name='MCDecayTree')
        nentries = rdf.Count().GetValue()

        return nentries
    #------------------------------------------
    def _get_stats(self) -> pnd.DataFrame:
        '''
        Returns pandas dataframe with `Passed` and `Total` yields for a given `Process`
        '''
        d_data = {'Process' : [], 'Passed' : [], 'Total' : []}
        for proc in self._l_proc:
            log.info(f'Calculating yields for process: {proc}')
            pas, tot = self._get_yields(proc=proc)

            d_data['Process'].append(proc)
            d_data['Passed' ].append(pas)
            d_data['Total'  ].append(tot)

        self._plot_sel_eff()

        df = pnd.DataFrame(d_data)

        return df
    #------------------------------------------
    def _efficiency_from_sample(
            self,
            sample : str,
            df     : pnd.DataFrame) -> tuple[float,float]:
        '''
        Parameters
        -----------------
        df     : Dataframe with yields, passed and failed for each sample
        sample : Nickname to MC signal sample

        Returns
        -----------------
        Tuple with:
           Efficiency value
           Error in efficiency
        '''
        nickname = DecayNames.nic_from_sample(sample)

        df = df[ df['Process'] == nickname ]
        df = cast(pnd.DataFrame, df)

        if len(df) != 1:
            log.error(df)
            raise ValueError(f'After specifying process {nickname} dataframe does not have one and only one column')

        pas = df['Passed'].iloc[0]
        tot = df['Total' ].iloc[0]

        eff = pas / tot
        err = math.sqrt(eff * (1 - eff) / tot)

        return eff, err
    #------------------------------------------
    def get_efficiency(self, sample : str) -> tuple[float,float]:
        '''
        Parameters
        -------------
        sample: Sample name, e.g. Bu_JpsiK_ee_eq_DPC
        kind  : Either `value` (default) or `dataframe`. 
                It controls what object should be returned

        Returns
        -------------
        Tuple with effiency and error associated
        '''
        data_path = f'{self._out_path}/yields.parquet'
        if self._copy_from_cache():
            log.info(f'Found yields cached, loading: {data_path}')
            df = pnd.read_parquet(data_path)
            return self._efficiency_from_sample(df=df, sample=sample)

        log.warning('Recalculating dataframe with yields')
        df = self._get_stats()
        df.to_parquet(data_path)

        self._cache()
        return self._efficiency_from_sample(df=df, sample=sample)
#------------------------------------------
