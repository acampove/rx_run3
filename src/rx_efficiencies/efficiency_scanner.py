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
    def _get_rdf(self) -> RDataFrame:
        log.debug('Getting dataframe')
        d_cut = self._get_selection()

        sample  = self._cfg['input']['sample']
        trigger = self._cfg['input']['trigger']

        gtr = RDFGetter(sample=sample, trigger=trigger)
        rdf = gtr.get_rdf()

        self._check_rdf(rdf=rdf)

        log.debug('Applying selection')
        for name, expr in d_cut.items():
            log.debug(f'{name:<20}{expr}')
            if self._skip_cut(expr):
                log.info(f'Skipping cut: {name}')
                expr = '(1)'

            rdf = rdf.Filter(expr, name)

        rep = rdf.Report()
        rep.Print()

        return rdf
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
    def run(self):
        '''
        return dataframe with efficiency and values of variables in scan
        '''
        rdf   = self._get_rdf()
        d_rdf = self._scan_rdf(rdf=rdf)

        log.info('Evaluating yields')

        d_data= {self._xvar : [], self._yvar : [], self._zvar : []}
        for (xval, yval), rdf in tqdm.tqdm(d_rdf.items()):
            zval = rdf.Count().GetValue()

            d_data[self._xvar].append(xval)
            d_data[self._yvar].append(yval)
            d_data[self._zvar].append(zval)

        return pnd.DataFrame(d_data)
# --------------------------------
