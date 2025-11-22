'''
Script used to calculate trigger efficiencies
'''

from ROOT                                    import RDataFrame
from rx_calibration.hltcalibration.parameter import Parameter
from rx_calibration.hltcalibration.component import FitComponent

# --------------------------------
class Data:
    cfg : dict

    l_component = ['sig', 'csp', 'cmb']
# --------------------------------
def _get_component(kind : str) -> FitComponent:
    ...
# --------------------------------
def _get_pars(data : RDataFrame, l_mod : list[FitComponent]) -> Parameter:
    ...
# --------------------------------
def main():
    rdf_dat = get_rdf(kind='data')
    d_mod   = [_get_component(kind=component) for component in Data.l_component]
    par     = _get_pars(data=rdf_dat, d_mod=d_mod)
# --------------------------------
if __name__ == '__main__':
    main()
