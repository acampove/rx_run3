'''
Module with MisIDFitter class
'''
import os

from dmu.stats.zfit          import zfit
from dmu.stats.zfit_plotter  import ZFitPlotter
from dmu.logging.log_store   import LogStore
from dmu.stats.model_factory import ModelFactory
from dmu.stats               import utilities  as sut
from dmu.stats.fitter        import Fitter

from zfit.core.interfaces    import ZfitData   as zdata
from zfit.core.interfaces    import ZfitPDF    as zpdf
from rx_misid.pdf_maker      import PDFMaker

log=LogStore.add_logger('rx_misid:misid_fitter')
# --------------------------------------------------
class MisIDFitter:
    '''
    Class intended to:

    - Pick up data in the PID inverted region
    - Return the PDF representing sum of $B^+\to K^+hh$ decays in the signal region
      with the final normalization
    '''
    # --------------------------------------------------
    def __init__(
            self,
            data  : zdata,
            q2bin : str):
        '''
        Parameters
        -----------------
        data: Zfit dataset representing data in the control (by PID) region
        q2bin: q2 bin, e.g. central
        '''
        self._obs     = data.space
        self._data    = data
        self._q2bin   = q2bin
        self._trigger = 'Hlt2RD_BuToKpEE_MVA_noPID'
        self._val_dir = None

        self._allowed_component = {
            'signal' : 'Bu_Kee_eq_btosllball05_DPC',
            'leakage': 'Bu_JpsiK_ee_eq_DPC',
            'kkk'    : 'Bu_KplKplKmn_eq_sqDalitz_DPC',
            'kpipi'  : 'Bu_piplpimnKpl_eq_sqDalitz_DPC'}
    # --------------------------------------------------
    @property
    def validation_directory(self) -> str|None:
        '''
        This is where validation outputs will go, e.g. fit plots.
        '''
        return self._val_dir
    # --------------------------------------------------
    @validation_directory.setter
    def validation_directory(self, value : str) -> None:
        '''
        This is where validation outputs will go, e.g. fit plots.
        '''
        os.makedirs(value, exist_ok=True)

        self._val_dir = value
    # --------------------------------------------------
    def _get_combinatorial(self) -> zpdf:
        model = {'low' : 'hypexp', 'central' : 'exp', 'high' : 'suj'}[self._q2bin]
        d_fix = {}
        if self._q2bin == 'high':
            d_fix = {'dl_suj' : 2.5,
                    'gm_suj' : -10}

        obj  = ModelFactory(
            preffix = 'cmb',
            obs     = self._obs,
            l_pdf   = [model],
            d_fix   = d_fix,
            l_float = [],
            l_shared= [])

        pdf  = obj.get_pdf()
        ncmb = zfit.Parameter('ncmb', 1000, 0, 10_000)
        pdf  = pdf.create_extended(ncmb, name='Combinatorial')

        return pdf
    # --------------------------------------------------
    def _get_mc_component(self, kind : str) -> zpdf|None:
        '''
        Parameters
        ---------------
        kind : Describes mc component, e.g. KKK Kpipi, signal
        '''
        if kind == 'kkk':
            log.warning(f'Skipping {kind} component, due to bugged MC')
            return None

        if kind not in self._allowed_component:
            for val in self._allowed_component:
                log.info(val)
            raise ValueError(f'Invalid component {kind}')

        sample = self._allowed_component[kind]

        mkr = PDFMaker(
            sample =sample,
            q2bin  =self._q2bin,
            trigger=self._trigger)
        pdf = mkr.get_pdf(obs=self._obs, is_sig=False)
        if pdf is None: # Early return for missing PDFs, due to low statistics dataset
            return None

        nev = zfit.param.Parameter(f'n{kind}', 1000, 0, 1000_000)
        pdf.set_yield(nev)

        return pdf
    # --------------------------------------------------
    def _get_component_names(self) -> dict[str,str]:
        d_name = {
            'Bu_Kee_eq_btosllball05_DPC'    : r'$B^+\to K^+ e^+e^-$',
            'Bu_piplpimnKpl_eq_sqDalitz_DPC': r'$B^+\to K^+ \pi^+\pi^-$',
            'Bu_JpsiK_ee_eq_DPC'            : r'$B^+\to K^+ J/\psi(\to e^+e^-)$'}

        return d_name
    # --------------------------------------------------
    def _get_model(self) -> zpdf:
        '''
        Returns model needed to fit mass distribution in control region
        '''
        pdf_cmb = self._get_combinatorial()
        pdf_mi1 = self._get_mc_component(kind=  'kpipi')
        pdf_mi2 = self._get_mc_component(kind=    'kkk')
        pdf_sig = self._get_mc_component(kind= 'signal')
        pdf_lek = self._get_mc_component(kind='leakage')

        l_pdf   = [ pdf_cmb, pdf_mi1, pdf_mi2, pdf_sig, pdf_lek ]
        l_pdf   = [ pdf for pdf in l_pdf if pdf is not None ]

        pdf     = zfit.pdf.SumPDF(l_pdf)

        return pdf
    # --------------------------------------------------
    def get_pdf(self) -> zpdf:
        '''
        Returns
        ------------------
        PDF defining hadronic misID background in the signal region
        This should be the sum of all the backgrounds, i.e. KKK, Kpipi, etc
        '''
        model = self._get_model()
        obj   = Fitter(model, self._data)
        res   = obj.fit()

        if self._val_dir is not None:
            d_leg = self._get_component_names()

            obj= ZFitPlotter(data=self._data, model=model)
            obj.plot(stacked=True, nbins=32, d_leg=d_leg)
            ax = obj.axs[0]

            ax.set_title(f'$q^2$: {self._q2bin}')

            sut.save_fit(
                data   = self._data,
                model  = model,
                res    = res,
                fit_dir= self._val_dir)
        else:
            log.warning('No validation directory found, not saving fit results')

        return model
# --------------------------------------------------
