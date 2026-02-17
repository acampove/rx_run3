'''
Module with tests for fitting_utilities module
'''
import numpy
import pytest
import matplotlib.pyplot as plt

from pathlib       import Path
from rpk_tools     import RpKPDF
from rpk_tools     import RpKSumPDF
from rx_generic    import rxran
from rx_generic    import utilities         as gut
from rpk_tools     import fitting_utilities as fut
from rpk_tools     import FitConfig
from rpk_tools     import RareEESystematics 
from rpk_tools     import Holder
from rpk_tools     import Component
from rx_logging    import LogStore

log = LogStore.add_logger('test_fitting_utilities')
# ----------------------
@pytest.fixture(scope='module', autouse=True)
def initialize():
    '''
    This will run before any test
    '''
    LogStore.set_level('rpk:zmodel_v2'               , 10)
    LogStore.set_level('rpk_tools::fitting_utilities', 10)
    LogStore.set_level('rpk_tools::fit_config'       , 10)

    with rxran.seed(value = 42):
        yield
# ------------------------------------
def _plot_pdf(pdf : RpKPDF, name : str, label : str) -> None:
    xs = numpy.linspace(5000, 6500, num = 200)
    ys = pdf.pdf(xs) 

    if pdf._weights is None: # type: ignore
        wgts = 'unweighted'
    else:
        wgt   = pdf._weights.numpy() # type: ignore
        wgts  = str(wgt[:10])

    nevt  = pdf.get_yield()
    if nevt is None:
        raise ValueError(f'Cannot get yield for : {pdf}')

    label = f'{label} {wgts} {nevt.value()}'

    plt.figure(num = name)
    plt.scatter(xs, ys * nevt.value(), label = label, s=5)
    plt.legend()
# ------------------------------------
def _component_from_model(full_model : RpKSumPDF, name : str) -> RpKPDF:
    for pdf in full_model.pdfs:
        if not pdf.label.endswith(name):
            continue

        if not isinstance(pdf, RpKPDF):
            raise ValueError('Component PDF not an RpKPDF')

        if not pdf.is_extended:
            raise ValueError(f'Component is not an extended PDF: {pdf}')

        return pdf 

    for pdf in full_model.pdfs:
        log.error(pdf.label)

    raise ValueError(f'Cannot find {name} pdf in {full_model}')
# ------------------------------------
def _get_systematics() -> RareEESystematics:
    data = gut.load_data(
        package = 'rpk_configs', 
        fpath   = 'toys/sys_rare_ee.yaml')

    data['jpsi_ee']['bootstrap'] = 3 
    data['pKstee' ]['bootstrap'] = 3 

    syst = RareEESystematics(**data)

    return syst
# ------------------------------------
def _get_data(cfg : FitConfig) -> Holder[numpy.ndarray]:
    datasets                 = Holder[numpy.ndarray]()
    datasets[Component.pKee] = numpy.random.normal(
        loc  = 5620, 
        scale= 40, 
        size = 300)
    datasets[Component.comb] = numpy.random.exponential(
        scale= 500, 
        size = 500) + cfg.fit_range[0]

    datasets[Component.jpsi_ee] = numpy.random.uniform(low=5000, high=6500, size=100)
    datasets[Component.pKst_ee] = numpy.random.uniform(low=5000, high=6500, size=100)
    datasets[Component.pKKK   ] = numpy.random.uniform(low=5000, high=6500, size=100)
    datasets[Component.pKKpi  ] = numpy.random.uniform(low=5000, high=6500, size=100)
    datasets[Component.pKpipi ] = numpy.random.uniform(low=5000, high=6500, size=100)

    return datasets
# ----------------------
def _build_models(
    path      : Path,
    kind      : str,
    cfg       : FitConfig[RareEESystematics],
    data      : Holder[numpy.ndarray],
    skip_plot : bool = False) -> tuple[Holder[RpKPDF],Holder[numpy.ndarray]]:
    '''
    Parameters
    -------------
    tmp_path : Needed to save plots
    cfg      : Used to build models
    data     : Used to fit models
    kind     : Type of model, needed to name saved plot

    Returns
    -------------
    Tuple with models and data associated
    '''
    models, new_data = fut.build_models(
    cfg    = cfg,
    params = dict(),
    dat    = data)

    assert isinstance(models  , Holder) 
    assert isinstance(new_data, Holder) 

    if skip_plot:
        return models, new_data

    with gut.environment(mapping = {'ANADIR' : str(path)}):
        fut.save_outputs(
            only_mc = True,
            dat     = new_data,
            pdf     = models,
            kind    = kind,
            cfg     = cfg)

    return models, new_data
# ------------------------------------
def test_rare_mm_builder():
    '''
    Test building Holder of PDFs for rare mode
    '''
    cfg  = FitConfig(
        systematics  = _get_systematics(),
        poi          = 'mm_dt_sig_N',
        channel      = 'mm',
        data_version = 'na',
        sign_version = 'na',
        year         = 'na',
        trig         = 'L0M',
        fit_version  = 'na',
        rseed        = 1,
        sel          = '')

    datasets = Holder[numpy.ndarray]()
    datasets[Component.pKmm] = numpy.random.normal(
        loc  = 5620, 
        scale= 40, 
        size = 2000)
    datasets[Component.comb] = numpy.random.exponential(
        scale= 200, 
        size = 2000) + cfg.fit_range[0]
    models, datasets = fut.build_models(
        cfg    = cfg,
        params = dict(),
        dat    = datasets)

    assert isinstance(models  , Holder) 
    assert isinstance(datasets, Holder) 
# ------------------------------------
def test_rare_ee_builder(tmp_path : Path):
    '''
    Test building Holder of PDFs for rare mode
    '''
    cfg  = FitConfig(
        systematics  = _get_systematics(),
        poi          = 'ee_dt_sig_N',
        channel      = 'ee',
        data_version = 'na',
        sign_version = 'na',
        year         = 'na',
        trig         = 'L0E',
        fit_version  = 'na',
        rseed        = 1,
        sel          = '')

    data = _get_data(cfg = cfg)

    models, new_data = fut.build_models(
        cfg    = cfg,
        params = dict(),
        dat    = data)

    with gut.environment(mapping = {'ANADIR' : str(tmp_path)}):
        fut.save_outputs(
            only_mc = True,
            dat     = new_data,
            pdf     = models,
            kind    = 'nominal',
            cfg     = cfg)

    assert isinstance(models  , Holder) 
    assert isinstance(new_data, Holder) 
# ------------------------------------
def test_rare_ee_builder_model_syst(tmp_path : Path):
    '''
    Test building Holder of PDFs for rare mode
    with systematics
    '''

    cfg_nom = FitConfig[RareEESystematics](
        systematics  = _get_systematics(),
        poi          = 'ee_dt_sig_N',
        channel      = 'ee',
        data_version = 'na',
        sign_version = 'na',
        year         = 'na',
        trig         = 'L0E',
        fit_version  = 'na',
        rseed        = 1,
        sel          = '')

    data = _get_data(cfg = cfg_nom)

    models_norm, _ = _build_models(
        path = tmp_path,
        data = data, 
        kind = 'nominal',
        cfg  = cfg_nom)

    cfg_sig, _ = cfg_nom.vary_model(kind = 'sig')

    models_sigm, _ = _build_models(
        path = tmp_path, 
        data = data, 
        kind = 'sigmod',
        cfg  = cfg_sig)

    cfg_cmb, _ = cfg_nom.vary_model(kind = 'cmb')
    models_cmbm, _ = _build_models(
        path = tmp_path, 
        data = data, 
        kind = 'cmbmod',
        cfg  = cfg_cmb)

    assert models_norm[Component.pKee].kind != models_sigm[Component.pKee].kind
    assert models_norm[Component.comb].kind != models_cmbm[Component.comb].kind
# ------------------------------------
def test_rare_ee_builder_npv_syst(tmp_path : Path):
    '''
    Test building Holder of PDFs for rare mode
    with systematics
    '''
    syst = _get_systematics()

    cfg = FitConfig(
        systematics  = syst,
        poi          = 'ee_dt_sig_N',
        channel      = 'ee',
        data_version = 'na',
        sign_version = 'na',
        year         = 'na',
        trig         = 'L0E',
        fit_version  = 'na',
        rseed        = 1,
        sel          = '')

    data = _get_data(cfg = cfg)

    sys = cfg.systematics
    sys.mutable_cfg['run_npv'] = True
    plt.close('all')
    while sys.npvs:
        sys_name = sys.this_npv_name
        pdf_name = sys.this_npv_pdf_name
        models, _     = _build_models(
            skip_plot = True,
            path      = tmp_path,
            data      = data, 
            kind      = sys_name,
            cfg       = cfg)

        full_model = models[Component.all]
        if not isinstance(full_model, RpKSumPDF):
            raise ValueError('Full model is not a sum')

        pdf      = _component_from_model(full_model = full_model, name = pdf_name)
        _plot_pdf(pdf = pdf, name = pdf_name, label = sys_name)

    for name in plt.get_figlabels():
        fig = plt.figure(name)
        fig.savefig(tmp_path / f'{name}.png')
# ------------------------------------
