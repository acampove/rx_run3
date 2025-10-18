'''
Script needed to calculate smearing factors for q2 distribution
'''

import os
import argparse
from functools import cache

import hist
import numpy
import mplhep
import pandas              as pnd
import matplotlib.pyplot   as plt

from ROOT                   import RDF # type: ignore
from dmu.stats.zfit         import zfit
from zfit.pdf               import BasePDF   as zpdf
from zfit.data              import Data      as zdata
from zfit.interface         import ZfitSpace as zobs
from zfit.result            import FitResult as zres

from dmu.generic             import utilities        as gut
from dmu.stats               import utilities        as sut
from dmu.stats.fitter        import Fitter
from dmu.stats.parameters    import ParameterLibrary as PL
from dmu.stats.model_factory import ModelFactory
from dmu.stats.zfit_plotter  import ZFitPlotter
from dmu.logging.log_store   import LogStore

from rx_selection            import selection as sel
from rx_data.rdf_getter      import RDFGetter
from rx_q2.config            import Config

Parameters=dict[str,tuple[float,float]]

log=LogStore.add_logger('rx_q2:get_q2_tables')
#-------------------
@cache
def _load_config() -> Config:
    parser = argparse.ArgumentParser(description='Used to produce q2 smearing factors systematic tables')
    parser.add_argument('-v', '--vers' , type =str, help='Version, used for naming of output directory', required=True)
    parser.add_argument('-t', '--trig' , type =str, help='Trigger'                                     , required=True)
    parser.add_argument('-y', '--year' , type =str, help='Year'                                        , required=True)
    parser.add_argument('-b', '--brem' , type =str, help='Brem category'                               , required=True)
    parser.add_argument('-k', '--kind' , type =str, help='Kind of sample'                              , required=True)
    parser.add_argument('-B', '--block', type =int, help='Block, by default -1, all'                   , default=-1) 
    parser.add_argument('-x', '--syst' , type =str, help='Systematic variabion'                        , default='nom')
    parser.add_argument('-l', '--logl' , type =int, help='Logging level'                               , default=20   )
    parser.add_argument('-e', '--nent' , type =int, help='Number of entries to run over, for tests'    , default=-1)
    parser.add_argument('--skip_fit'   , help='Will not fit, just plot the model'                      , action ='store_true')
    args = parser.parse_args()

    input            = {}
    input['nent']    = args.nent
    input['year']    = args.year
    input['trigger'] = args.trig
    input['brem']    = args.brem
    input['block']   = 'all' if args.block == -1 else str(args.block)
    input['kind']    = args.kind
    input['nentries']= args.nent
    input['skip_fit']= args.skip_fit

    data = gut.load_data(package='rx_q2_data', fpath=f'config/{args.vers}.yaml')
    data['ana_dir'] = os.environ['ANADIR']
    data['vers'   ] = args.vers
    data['syst'   ] = args.syst
    data['logl'   ] = args.logl
    data['input'  ].update(input)

    zfit.settings.changed_warnings.hesse_name = False

    return Config(**data)
#-------------------
def _set_vars():
    cfg         = _load_config()
    Data.l_syst = cfg['syst']
    d_input     = cfg['input'  ]
    d_fitting   = cfg['fitting']

    Data.l_pdf  = d_fitting['model']['pdfs']
    Data.l_year = [ str(year) for year in d_input['year'] ]
    Data.l_brem = [ str(brem) for brem in d_input['brem'] ]
    Data.l_trig = d_input['trigger']
    Data.l_cali = d_input['cali']
    Data.d_samp = d_input['samples']
    Data.l_kind = list(Data.d_samp)
    Data.d_sel  = d_input['selection']

    Data.nbins       = d_fitting['binning']['nbins']
    Data.cfg_sim_fit = d_fitting['simulation']
    Data.j_mass      = d_fitting['mass']
    Data.weights     = d_fitting['weights']

    Data.d_obs_range = { str(brem) : val for brem, val in d_fitting['ranges'].items() }
#-------------------
def _initialize():
    plt.style.use(mplhep.style.LHCb2)
    d_cut={'nbrem' : f'nbrem == {Data.brem}'}

    if   Data.block == 'all':
        d_cut['block'] =  '(1)'
    elif Data.block == '12':
        d_cut['block'] =  '(block == 1) || (block == 2)'
    elif Data.block == '78':
        d_cut['block'] =  '(block == 7) || (block == 8)'
    else:
        d_cut['block'] = f'block == {Data.block}'

    d_cut.update(Data.d_sel)
    sel.set_custom_selection(d_cut = d_cut)

    Data.obs_range= Data.d_obs_range[Data.brem]
    Data.obs      = zfit.Space(Data.j_mass, limits=Data.obs_range)

    LogStore.set_level('dmu:statistics:fitter', Data.logl)
    LogStore.set_level('dmu:stats:utilities'  , Data.logl)
    LogStore.set_level('rx_q2:get_q2_tables'  , Data.logl)
    LogStore.set_level('rx_data:rdf_getter'   , Data.logl)

    Data.out_dir = f'{Data.ana_dir}/q2/fits/{Data.out_vers}/{Data.kind}/{Data.trig}_{Data.year}_{Data.brem}_{Data.block}_{Data.syst}'
    os.makedirs(Data.out_dir, exist_ok=True)
#-------------------
def _set_pdf_pars(pdf : zpdf, d_val : Parameters) -> None:
    '''
    This function takes pad and dictionary of parameters
    sets PDF parameter values according to dictionary
    '''
    l_par    = list(pdf.get_params(floating=True)) + list(pdf.get_params(floating=False))
    log.info('Setting initial values:')
    for par in l_par:
        name = par.name
        if name not in d_val:
            continue

        val  = d_val[name]
        par.set_value(val)
        log.info(f'{name:<20}{"->":<10}{val:<10.3}')
#-------------------
def _get_sig_pdf() -> zpdf:
    for pdf_name in Data.l_pdf:
        PL.set_values(kind=pdf_name, parameter='mu', val=3000, low=2800, high=3300)
        PL.set_values(kind=pdf_name, parameter='sg', val= 100, low=  10, high= 200)

        if pdf_name == 'cbr':
            PL.set_values(kind=pdf_name, parameter='nc', val= 3, low= 0.1, high= 10)

        if pdf_name == 'dscb':
            PL.set_values(kind=pdf_name, parameter='nr', val= 3, low= 0.1, high=128)

    mod     = ModelFactory(
    preffix = 'q2_smearing',
    obs     = Data.obs,
    l_pdf   = Data.l_pdf,
    l_shared= ['mu', 'sg'],
    l_float = ['mu', 'sg'])
    pdf     = mod.get_pdf()

    return pdf
#-------------------
def _get_bkg_pdf() -> zpdf:
    if hasattr(Data, 'bkg_pdf'):
        return Data.bkg_pdf

    lam = zfit.Parameter('lam', 0.0, -0.01, +0.01)
    bkg = zfit.pdf.Exponential(lam=lam, obs=Data.obs, name='')

    nbk = zfit.Parameter('nbk', 10, 1, 10_000)
    bkg = bkg.create_extended(nbk, name='Combinatorial')

    Data.bkg_pdf = bkg

    return bkg
#-------------------
def _get_full_pdf():
    sig = _get_sig_pdf()
    nsg = zfit.Parameter('nsg', 10_000, 100, 2_000_000)
    sig = sig.create_extended(nsg, name='Signal')

    return sig

    bkg = _get_bkg_pdf()
    pdf = zfit.pdf.SumPDF([sig, bkg], name='Model')

    log.debug(f'Signal    : {sig}')
    log.debug(f'Background: {bkg}')
    log.debug(f'Model     : {pdf}')

    return pdf
#-------------------
def _fix_pdf(pdf : zpdf, d_fix : Parameters) -> zpdf:
    '''
    This will take a PDF and a dictionary of paramters
    It will fix the PDF parameters according to dictionary
    '''
    if d_fix is None:
        return pdf

    l_par = list(pdf.get_params(floating=True))

    log.info('-----------------')
    log.info('Fixing parameters')
    log.info('-----------------')
    for par in l_par:
        if par.name not in d_fix:
            log.info(f'{par.name:<20}{"->":<10}{"floating":>20}')
            continue

        fix_val, _ = d_fix[par.name]
        par.set_value(fix_val)
        par.floating=False

        log.info(f'{par.name:<20}{"->":<10}{fix_val:>20.3e}')

    return pdf
#-------------------
def _get_pdf(kind : str) -> zpdf:
    if kind == 'sim':
        return _get_sig_pdf()

    if kind == 'dat':
        return _get_full_pdf()

    raise NotImplementedError(f'Cannot get PDF for: {kind}')
#-------------------
def _par_val_from_dict(d_par : dict) -> tuple[float,float]:
    value = d_par['value']
    if 'hesse' not in d_par:
        return value, None

    error = d_par['hesse']['error']

    return value, error
#-------------------
def _get_pars(res : zres) -> Parameters:
    try:
        d_par = { name : _par_val_from_dict(d_val) for name, d_val in res.params.items() }
    except Exception as exc:
        log.info(res)
        log.info(res.params)
        raise ValueError('Cannot extract parameters') from exc

    return d_par
#-------------------
def _fit(d_fix : Parameters = None)-> Parameters:
    kind= 'sim' if d_fix is None else 'dat'

    pdf = _get_pdf(kind)
    dat = _get_data(pdf, kind)
    pdf = _fix_pdf(pdf, d_fix)
    obj = Fitter(pdf, dat)

    if Data.skip_fit:
        log.warning('Skipping fit')
        return None

    res=obj.fit(cfg=Data.cfg_sim_fit)
    if res is None:
        _plot_fit(dat, pdf, res)

        log.info(res)
        raise ValueError('Fit failed')

    if res.status != 0:
        _plot_fit(dat, pdf, res)

        log.info(res)
        raise ValueError(f'Finished with status/validity: {res.status}/{res.valid}')

    log.info(30 * '-')
    log.info('Found parameters:')
    log.info(30 * '-')
    log.info(res)
    log.info(30 * '-')
    _plot_fit(dat, pdf, res)

    d_par = _get_pars(res)

    sut.save_fit(data=dat, model=pdf, res=res, fit_dir=Data.out_dir)

    return d_par
#-------------------
def _get_data(pdf : zpdf, kind : str) -> zdata:
    data_path = f'{Data.out_dir}/data.json'
    if os.path.isfile(data_path):
        log.warning(f'Data found, loading from: {data_path}')
        df  = pnd.read_json(data_path)
        # TODO: Here the weights need to be put back, once the bug in zfit be fixed
        dat = zfit.data.Data.from_pandas(df, obs=Data.obs)

        return dat

    rdf     = _get_rdf(kind=kind)
    d_data  = rdf.AsNumpy([Data.j_mass, Data.weights])
    df      = pnd.DataFrame(d_data)
    df.to_json(data_path) # Caching now will avoid redoing this if fit fails

    obs     = pdf.space
    # TODO: Here the weights need to be put back, once the bug in zfit be fixed
    dat     = zfit.Data.from_pandas(obs=obs, df=df)

    _plot_data(df, obs, kind)

    return dat
#-------------------
def _plot_data(
        df         : pnd.DataFrame,
        obs        : zobs,
        kind       : str) -> None:

    arr_mas = df[Data.j_mass ].to_numpy()
    arr_wgt = df[Data.weights].to_numpy()

    [[lower]], [[upper]] = obs.limits
    _, ax     = plt.subplots(figsize=(15, 10))
    data_hist = hist.Hist.new.Regular(Data.nbins, lower, upper, name='', underflow=False, overflow=False)
    data_hist = data_hist.Weight()
    data_hist.fill(arr_mas, weight=arr_wgt)

    mplhep.histplot(
        data_hist,
        yerr    = True,
        color   = 'black',
        histtype= 'errorbar',
        ax      = ax
    )

    title=f'Entries={arr_wgt.size:.0f}; Sum={numpy.sum(arr_wgt):.0f}'

    plt_path = f'{Data.out_dir}/{kind}.png'

    log.info(f'Saving to: {plt_path}')
    plt.title(title)
    plt.savefig(plt_path)
    plt.close('all')
#-------------------
def _add_q2_region_lines(obj : ZFitPlotter) -> None:
    axis = obj.axs[0]
    axis.axvline(x=2450, c='red', ls=':')
    axis.axvline(x=3600, c='red', ls=':')
#-------------------
def _get_text(data : zdata) -> tuple[str,str]:
    nentries= data.nevents.numpy()
    text    = f'Candidates={nentries}'
    l_part  = []
    for name, value in Data.d_sel.items():
        l_part.append(f'{name}: {value}')

    title = '; '.join(l_part)

    return text, title
#-------------------
def _get_naming() -> dict[str,str]:
    if Data.kind == 'dat':
        return {
                'Data'   : 'Data',
                'Signal' : 'PDF',
                }

    if Data.kind == 'sim':
        return {
                'Data'   : 'Simulation',
                'dscb_1' : 'PDF',
                }

    raise NotImplementedError(f'Invalid sample: {Data.kind}')
#-------------------
def _plot_fit(
        dat        : zdata,
        pdf        : zpdf,
        res        : zres) -> None:
    obj=ZFitPlotter(data=dat, model=pdf, result=res)
    for yscale in ['log', 'linear']:
        for add_pars in ['pars', 'no_pars']:
            pars  = None if add_pars == 'no_pars' else 'all'
            text, title = _get_text(data = dat)

            obj.plot(
                    nbins     = Data.nbins,
                    d_leg     = _get_naming(),
                    plot_range= Data.obs_range,
                    yscale    = yscale,
                    ext_text  = text,
                    add_pars  = pars)

            _add_q2_region_lines(obj)

            obj.axs[0].set_title(title)
            obj.axs[0].axhline(y=0, c='gray')

            plot_path = f'{Data.out_dir}/{add_pars}_{yscale}.png'
            log.info(f'Saving to: {plot_path}')
            plt.savefig(plot_path)
            plt.close()
#-------------------
def _get_fix_pars() -> Parameters:
    d_par = Data.d_sim_par
    d_fix = { key : val for key, val in d_par.items() if ('mu' not in key) and ('sg' not in key) }

    return d_fix
#-------------------
def _get_rdf(kind : str) -> RDataFrame:
    log.info(f'Getting data for: {kind}')
    sample = Data.d_samp[kind]
    gtr    = RDFGetter(sample=sample, trigger=Data.trig)
    rdf    = gtr.get_rdf()

    d_sel = sel.selection(trigger=Data.trig, q2bin='jpsi', process=sample)
    for cut_name, cut_value in d_sel.items():
        log.debug(f'{cut_name:<20}{cut_value}')
        rdf = rdf.Filter(cut_value, cut_name)

    if log.getEffectiveLevel() == 10:
        rep = rdf.Report()
        rep.Print()

    return rdf
#-------------------
def _make_table():
    if Data.kind == 'sim':
        log.info('Running simulation fit')
        _fit(d_fix=None)
        return

    log.info('Running data fit')
    dat_par_path   = f'{Data.out_dir}/parameters.json'
    sim_par_path   = dat_par_path.replace('/dat/', '/sim/')
    Data.d_sim_par = gut.load_json(sim_par_path)
    d_fix_par      = _get_fix_pars()

    _fit(d_fix=d_fix_par)
#-------------------
#-------------------
@gut.timeit
def main():
    '''
    Start here
    '''
    _set_vars()
    _get_args()
    _initialize()
    _make_table()
#-------------------
if __name__ == '__main__':
    main()
