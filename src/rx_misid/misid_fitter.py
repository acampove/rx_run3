'''
Module with MisIDFitter class
'''
from dmu.stats.zfit          import zfit
from dmu.logging.log_store   import LogStore
from dmu.stats.model_factory import ModelFactory
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

        self._allowed_component = {
                'signal' : 'Bu_Kee_eq_btosllball05_DPC',
                'leakage': 'Bu_JpsiK_ee_eq_DPC',
                'kkk'    : 'Bu_KplKplKmn_eq_sqDalitz_DPC',
                'kpipi'  : 'Bu_piplpimnKpl_eq_sqDalitz_DPC'}
    # --------------------------------------------------
    def _get_combinatorial(self) -> zpdf:
        obj  = ModelFactory(
            preffix = 'cmb',
            obs     = self._obs,
            l_pdf   = ['exp'],
            l_float = [],
            l_shared= [])

        pdf  = obj.get_pdf()
        ncmb = zfit.Parameter('ncmb', 10, 0, 10_000)
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

        nev = zfit.param.Parameter(f'n{kind}', 10, 0, 1000_000)
        pdf.set_yield(nev)

        return pdf
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
        obj = Fitter(model, self._data)
        res = obj.fit()
        d_nevt = self._yield_from_result(res)

        l_pdf = []
        for sample, nevt in d_nevt.items():
            rdf     = self._get_rdf(sample)
            arr_wgt = self._get_weights(rdf, nevt)
            pdf_sam = self._pdf_from_rdf(rdf=rdf, wgt=arr_wgt)
            l_pdf.append(pdf_sam)

        pdf = zfit.pdf.SumPDF(l_pdf)

        return pdf
# --------------------------------------------------
