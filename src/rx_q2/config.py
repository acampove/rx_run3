'''
Module containing Conf class
'''

from pydantic import BaseModel

#-------------------
class Config(BaseModel):
    '''
    Data class
    '''
    zfit.settings.changed_warnings.hesse_name = False
    ana_dir        = Path(os.environ['ANADIR'])
    cfg_vers : str = 'v2'
    gut.TIMER_ON   = True

    out_dir      : str
    logl         : int
    kind         : str
    syst         : str

    l_year       : list[str]
    l_trig       : list[str]
    l_brem       : list[str]
    l_syst       : list[str]
    l_kind       : list[str]
    l_cali       : list[str]
    d_sel        : dict[str,str]
    d_samp       : dict[str,str]
    obs_range    : list[float]
    d_obs_range  : dict[str,list[float]]

    trig         : str
    year         : str
    brem         : str
    block        : str
    nentries     : int
    skip_fit     : bool
    nevs_data    : int
    cal_sys      : str
    out_vers     : str

    j_mass       : str
    weights      : str
    nbins        : int
    obs          : zobs
    sig_pdf_splt : zpdf
    sig_pdf_merg : zpdf
    bkg_pdf      : zpdf
    l_pdf        : list[str]
    d_sim_par    : dict[str,tuple[float,float]]
    cfg_sim_fit  : dict

