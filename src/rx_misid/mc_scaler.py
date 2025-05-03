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
    def _get_ratio(self, nsig : float, nctr : float) -> float:
        if nsig == 0:
            log.warning(f'Zero yield in {self._sample}/{self._q2bin} => scale is zero')
            return 0

        rat  = nctr / nsig

        return rat
    # ----------------------------------
    def _get_nsignal(self) -> float:
        fit_dir = os.environ['FITDIR']
        trigger = self._trigger.replace('_ext', '')

        fit_dir = f'{fit_dir}/DATAp/{trigger}/v1/{self._q2bin}/default'
        log.debug(f'Reading signal yield from: {fit_dir}')

        obj  = FitStats(fit_dir=fit_dir)
        nsig = obj.get_value(name='nsig', kind='value')

        return nsig
    # ----------------------------------
    def get_scale(self) -> tuple[int,int,float]:
        '''
        Returns tuple with three elements, nsig, nctr and rat,
        where the former two are the signal and control yields and rat:
        Signal yield x MC control / MC signal
        '''

        rdf        = self._get_rdf()
        nsig, nctr = self._get_stats(rdf)
        rat        = self._get_ratio(nsig=nsig, nctr=nctr)
        nsig       = self._get_nsignal()
        scale      = rat * nsig

        log.info(f'Predicted {self._sample} yield in MisID region: {scale:.0f}')

        return nsig, nctr, scale
# ----------------------------------
