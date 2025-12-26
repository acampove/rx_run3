'''
Module with CmbConstraints class
'''
import zfit

from dmu.stats    import ModelFactory
from dmu.stats    import ConstraintND, Fitter
from omegaconf    import DictConfig
from zfit         import Space     as zobs
from zfit         import Data      as zdat
from zfit.pdf     import BasePDF   as zpdf
from zfit.result  import FitResult as zres
from rx_common    import Qsq, Sample, Trigger
from rx_data      import RDFGetter
from rx_selection import selection as sel

# ------------------------------------
class CmbConstraints:
    '''
    Class intended to provide constraints for shape of combinatorial model
    '''
    # ----------------------
    def __init__(
        self, 
        obs  : zobs,
        cfg  : DictConfig,
        q2bin: Qsq) -> None:
        '''
        Parameters
        -------------
        obs  : Zfit observable
        cfg  : configuration for combinatorial component
        q2bin: E.g. central
        '''
        self._obs    = obs
        self._cfg    = cfg
        self._cmb_cfg= cfg.model.combinatorial
        self._q2bin  = q2bin
    # ----------------------
    def _get_data(self) -> zdat:
        '''
        Returns
        -------------
        1D numpy array with masses to fit
        '''
        cmb_cfg= self._cfg.model.combinatorial

        cons   = cmb_cfg[self._q2bin]['constraints']
        sample = Sample(cons.sample)
        trigger= Trigger(cons.trigger)

        gtr = RDFGetter(sample = sample, trigger = trigger)
        rdf = gtr.get_rdf(per_file = False)

        rdf = sel.apply_full_selection(
            rdf     = rdf,
            trigger = trigger,
            process = sample,
            q2bin   = self._q2bin,
        )

        array = rdf.AsNumpy([self._obs.name])[self._obs.name]
        data  = zfit.Data.from_numpy(obs = self._obs, array = array)

        if not isinstance(data, zdat):
            raise ValueError('Data is not a zfit.Data instance')

        return data
    # ----------------------
    def _get_model(self) -> zpdf:
        '''
        Returns
        -------------
        PDF used to fit combinatorial
        '''

        mod         = ModelFactory(
            preffix = 'ss' ,
            obs     = self._obs,
            l_pdf   = self._cmb_cfg['categories'].main.models[self._q2bin],
            l_shared= self._cmb_cfg[self._q2bin ].shared,
            l_float = self._cmb_cfg[self._q2bin ].float,
        )

        pdf = mod.get_pdf()

        return pdf
    # ----------------------
    def _fit(self, data : zdat) -> zres:
        '''
        Parameters
        -------------
        data: 1D array with masses

        Returns
        -------------
        Result of fit
        '''
        pdf = self._get_model()

        ftr = Fitter(pdf = pdf, data = data)
        res = ftr.fit()

        return res
    # ----------------------
    def get_constraint(self) -> ConstraintND:
        '''
        Returns
        ----------------------
        N dimensional Gaussian constraint
        '''
        data = self._get_data()
        res  = self._fit(data=data)

        all_pars   = self._cmb_cfg[self._q2bin].constraints.parameters
        values     = [ float(par.value().numpy()) for par in res.params if par.name in all_pars ]
        parameters = [ par.name                   for par in res.params if par.name in all_pars ]
        cov        = res.covariance(params=parameters)

        cns = ConstraintND(
            kind       = 'GaussianConstraint',
            parameters = parameters, 
            values     = values, 
            cov        = cov,
        )

        return cns
# ------------------------------------
