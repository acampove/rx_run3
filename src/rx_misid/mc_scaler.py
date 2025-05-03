'''
Module containing MCScaler class
'''
import os

from ROOT                   import RDataFrame
from dmu.logging.log_store  import LogStore
from dmu.stats.fit_stats    import FitStats
from rx_selection           import selection as sel
from rx_data.rdf_getter     import RDFGetter

log=LogStore.add_logger('rx_misid:ms_scaler')
# ----------------------------------
class MCScaler:
    '''
    Class meant to provide scale factor to extrapolate leakage and signal yields
    to misID control regions
    '''
    # ----------------------------------
    def __init__(self, q2bin : str, sample : str, sig_reg : str):
        '''
        q2bin  : q2 bin
        sample : Name of MC sample, e.g. Bu_12345_Kee_btosll...
        sig_reg: Cut defining the signal region, it will be inverted to build the control region
        '''
        self._q2bin   = q2bin
        self._sample  = sample
        self._sig_reg = sig_reg
        self._trigger = 'Hlt2RD_BuToKpEE_MVA_ext'
        self._project = 'RK'
    # ----------------------------------
    def _get_rdf(self) -> RDataFrame:
        log.debug('Retrieving dataframe')

        gtr = RDFGetter(sample=self._sample, trigger=self._trigger)
        rdf = gtr.get_rdf()

        d_sel = sel.selection(
                project=self._project,
                trigger=self._trigger,
                q2bin  =self._q2bin,
                process=self._sample)

        d_sel['pid_l'] = '(1)'

        for cut_name, cut_expr in d_sel.items():
            log.debug(f'{cut_name:<20}{cut_expr}')
            rdf = rdf.Filter(cut_expr, cut_name)

        if log.getEffectiveLevel() == 10:
            rep = rdf.Report()
            rep.Print()

        return rdf
    # ----------------------------------
    def _get_stats(self, rdf : RDataFrame) -> tuple[float,float]:
        log.debug(f'Getting ratio of MC yields with signal region: {self._sig_reg}')

        sig_reg = self._sig_reg
        ctr_reg = f'({self._sig_reg}) == 0'

        rdf_sig = rdf.Filter(sig_reg, 'Signal' )
        rdf_ctr = rdf.Filter(ctr_reg, 'Control')

        if log.getEffectiveLevel() == 10:
            rep_sig = rdf_sig.Report()
            rep_ctr = rdf_ctr.Report()

            log.debug('Signal:')
            rep_sig.Print()
            log.debug('Control:')
            rep_ctr.Print()

        nctr = rdf_ctr.Count().GetValue()
        nsig = rdf_sig.Count().GetValue()

        return nsig, nctr
    # ----------------------------------
    def _get_ratio(self, num : float, den : float) -> float:
        '''
        num: Is meant to be a data yield, from fits
        den: Is meant to be a MC yield
        '''
        if num == 0:
            log.warning(f'Zero yield in data for {self._sample}/{self._q2bin} => scale is zero')
            return 0

        if den == 0:
            raise ValueError(f'Zero yield in MC for {self._sample}/{self._q2bin} but not in data')

        return num / den
    # ----------------------------------
    def _get_nsignal(self) -> float:
        fit_dir = os.environ['FITDIR']
        trigger = self._trigger.replace('_ext', '')

        fit_dir = f'{fit_dir}/DATAp/{trigger}/v1/{self._q2bin}/default'
        log.debug(f'Reading signal yield from: {fit_dir}')

        obj  = FitStats(fit_dir=fit_dir)

        if   self._sample == 'Bu_JpsiK_ee_eq_DPC':
            if self._q2bin in ['low', 'high']:
                log.info('Assuming no leakage from Jpsi sample in low and high q2bin')
                return 0

            poi = 'nBu_JpsiK_ee_eq_DPC'
        elif self._sample == 'Bu_Kee_eq_btosllball05_DPC':
            poi = 'nsig'
        else:
            raise ValueError(f'Invalid sample: {self._sample}')

        nsig = obj.get_value(name=poi, kind='value')

        return nsig
    # ----------------------------------
    def get_scale(self) -> tuple[int,int,float]:
        '''
        Returns tuple with three elements, nsig, nctr and scale,
        where the former two are the signal and control yields and rat:

        Data_{x}^{Signal region} / MC_{x}^{signal region}

        i.e. the ratio of yields of the component "x" in the signal region in data and in MC.
        '''

        rdf              = self._get_rdf()
        nsig_mc, nctr_mc = self._get_stats(rdf)
        nsig_dt          = self._get_nsignal()
        scale            = self._get_ratio(num=nsig_dt, den=nsig_mc)

        log.info(f'Scale for {self._sample}: {scale:.3f}')

        return nsig_mc, nctr_mc, scale
# ----------------------------------
