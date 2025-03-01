'''
Module containing EfficiencyScanner class
'''

from ROOT                  import RDataFrame
from dmu.logging.log_store import LogStore
from rx_data.rdf_getter    import RDFGetter
from rx_selection          import selection as sel

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
        self._cfg = cfg

        RDFGetter.samples = cfg['input']['paths']
    # --------------------------------
    def _get_selection(self) -> dict[str,str]:
        sample = self._cfg['input']['sample']

        d_sel = sel.selection(project='RK', analysis='EE', q2bin='jpsi', process=sample)

        return d_sel
    # --------------------------------
    def _skip_cut(self, expr : str) -> bool:
        d_var = self._cfg['variables']

        for var_name in d_var:
            if var_name in expr:
                return True

        return False
    # --------------------------------
    def _get_rdf(self) -> RDataFrame:
        d_cut = self._get_selection()

        sample  = self._cfg['input']['sample']
        trigger = self._cfg['input']['trigger']

        gtr = RDFGetter(sample=sample, trigger=trigger)
        rdf = gtr.get_rdf()

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
    def run(self):
        '''
        return dataframe with efficiency and values of variables in scan
        '''
        rdf   = self._get_rdf()

        return
# --------------------------------
