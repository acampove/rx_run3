'''
Module containing EfficiencyScanner class
'''

import numpy
import tqdm
import pandas as pnd

from ROOT                  import RDataFrame
from dmu.logging.log_store import LogStore
from dmu.generic           import hashing
from dmu.generic           import utilities as gut
from rx_data.rdf_getter    import RDFGetter
from rx_selection          import selection as sel

from rx_efficiencies.efficiency_calculator import EfficiencyCalculator
from rx_efficiencies.decay_names           import DecayNames

log = LogStore.add_logger('rx_efficiencies:efficiency_scanner')
# --------------------------------
class EfficiencyScanner:
    '''
    Class meant to scan efficiencies in MC samples

    It will

    - Apply full selection, except for cuts involving variables been scanned
    - Scan those variables and provide dataframe with efficiencies
    '''
    # --------------------------------
    def __init__(self, cfg : dict):
        self._cfg    = cfg
        [xvar, yvar] = list(cfg['variables'].keys())

        self._xvar = xvar
        self._yvar = yvar
        self._zvar = 'yield'

        self._yld_default : int = -1
    # --------------------------------
    def _get_selection(self) -> dict[str,str]:
        log.debug('Getting selection')

        sample  = self._cfg['input']['sample']
        trigger = self._cfg['input']['trigger']
        q2bin   = self._cfg['input']['q2bin']
        d_sel   = sel.selection(trigger=trigger, q2bin=q2bin, process=sample)

        return d_sel
    # --------------------------------
    def _check_rdf(self, rdf : RDataFrame) -> None:
        l_variable = [ var.c_str()        for var in rdf.GetColumnNames() ]
        # Drop prefix for variables in friend trees
        l_variable = [ var.split('.')[-1] for var in rdf.GetColumnNames() ]
        l_variable = sorted(l_variable)

        fail = False
        for variable in self._cfg['variables']:
            if variable not in l_variable:
                log.error(f'Missing variable: {variable}')
                fail = True

        if fail:
            log.info(l_variable)
            raise ValueError('At least one variable was not found')
    # --------------------------------
    def _skip_cut(self, expr : str) -> bool:
        d_var = self._cfg['variables']

        for var_name in d_var:
            if var_name in expr:
                return True

        return False
    # --------------------------------
    def _get_rdf(self) -> tuple[RDataFrame,str]:
        '''
        Load dataframe and lazily apply selection, except for the scanning part

        Return dataframe and hash with unique identifier for:
            - Dataset itself
            - Selection used on it
        '''
        log.debug('Getting dataframe')
        d_cut = self._get_selection()

        sample  = self._cfg['input']['sample']
        trigger = self._cfg['input']['trigger']

        gtr = RDFGetter(sample=sample, trigger=trigger)
        rdf = gtr.get_rdf()
        uid = gtr.get_uid()

        self._check_rdf(rdf=rdf)

        log.debug('Applying selection')

        d_cut_used = {}
        for name, expr in d_cut.items():
            log.debug(f'{name:<20}{expr}')
            if self._skip_cut(expr):
                log.info(f'Skipping cut: {name}')
                expr = '(1)'

            d_cut_used[name] = expr
            rdf = rdf.Filter(expr, name)

        hsh = hashing.hash_object([uid, d_cut_used])

        return rdf, hsh
    # --------------------------------
    def _scan_rdf(self, rdf : RDataFrame) -> dict[str,RDataFrame]:
        d_var        = self._cfg['variables']
        [xvar, yvar] = list(d_var.keys())
        arr_xval     = d_var[xvar]
        arr_yval     = d_var[yvar]
        X, Y         = numpy.meshgrid(arr_xval, arr_yval, indexing='xy')
        arr_bound    = numpy.stack([X.ravel(), Y.ravel()], axis=1)

        d_rdf = {}
        for [xval, yval] in arr_bound:
            expr                = f'({xvar} > {xval}) && ({yvar} > {yval})'
            d_rdf[(xval, yval)] = rdf.Filter(expr)

        return d_rdf
    # --------------------------------
    def _eff_from_yield(self, df_tgt : pnd.DataFrame) -> pnd.DataFrame:
        '''
        Takes dataframe with yields and adds an efficiency column.
        The denominator is taken from an `EfficiencyCalculator` instance
        at the correct process.

        Parameters
        ------------------
        df_tgt: Dataframe with yields for different working points

        Returns
        ------------------
        Same dataframe with eff column added
        '''
        if self._yld_default < 0:
            raise ValueError(f'Default WP yield (unitizialized?) found to be negative: {self._yld_default}')

        if self._yld_default == 0:
            raise ValueError('Default WP yield is zero')

        sample = self._cfg['input']['sample']
        obj    = EfficiencyCalculator(q2bin='central', sample=sample)
        eff, _ = obj.get_efficiency(sample = sample)

        # eff = self._yld_default / yld_total
        # Therefore this should provide efficiency at given WP
        df_tgt['eff'] = eff * df_tgt['yield'] / self._yld_default

        return df_tgt
    # --------------------------------
    def _get_yields(self, rdf : RDataFrame) -> pnd.DataFrame:
        '''
        Parameters 
        ------------------
        rdf: ROOT dataframe with actual data/mc

        Returns 
        ------------------
        pandas dataframe with:

        X/Y: Coordinates at which cuts are evaluated
        Z  : Yield
        '''
        d_rdf = self._scan_rdf(rdf=rdf)

        log.info('Evaluating yields')

        d_data= {self._xvar : [], self._yvar : [], self._zvar : []}
        for (xval, yval), rdf_sel in tqdm.tqdm(d_rdf.items()):
            zval = rdf_sel.Count().GetValue()

            d_data[self._xvar].append(xval)
            d_data[self._yvar].append(yval)
            d_data[self._zvar].append(zval)

        return pnd.DataFrame(d_data)
    # ----------------------
    def _set_default_wp_yield(self, rdf : RDataFrame) -> None:
        '''
        This method sets _yld_default value. I.e. yield of MC sample
        for default selection.

        Parameters
        -------------
        rdf: ROOT dataframe before selection
        '''
        sample  = self._cfg['input']['sample']
        trigger = self._cfg['input']['trigger']
        q2bin   = self._cfg['input']['q2bin']

        rdf_sel = sel.apply_full_selection(rdf=rdf, q2bin=q2bin, trigger=trigger, process=sample)

        self._yld_default = rdf_sel.Count().GetValue()

        log.info(f'Setting default WP yield to: {self._yld_default}')
    # --------------------------------
    def run(self) -> pnd.DataFrame:
        '''
        return dataframe with efficiency and values of variables in scan

        Caching and Hashing
        -----------
        This method caches the efficiencies and uses hashing
        The hashing uses:

        - The identity of the input data
        - The configuration, which specifies the variables to scan and where
        '''

        rdf, hsh = self._get_rdf()

        data = gut.load_cached(hash_obj=[hsh, self._cfg], on_fail=-999)
        if data != -999:
            log.info('Efficiencies cached, reloading them')
            df = pnd.DataFrame(data)
            return df

        log.info('Efficiencies not cached, recalculating them')
        self._set_default_wp_yield(rdf=rdf)

        df = self._get_yields(rdf=rdf)
        df = self._eff_from_yield(df_tgt=df)

        data = df.to_dict(orient='records')
        gut.cache_data(data, hash_obj=[hsh, self._cfg])

        return df
# --------------------------------
