'''
Module with CmbConstraints class
'''
import zfit

from typing       import Final
from dmu          import LogStore
from dmu.stats    import ConstraintND, Fitter
from omegaconf    import DictConfig
from zfit         import Space               as zobs
from zfit         import Data                as zdat
from zfit.pdf     import BasePDF             as zpdf
from zfit.result  import FitResult           as zres
from zfit.loss    import ExtendedUnbinnedNLL as zlos
from rx_common    import Qsq, Sample, Trigger
from rx_data      import RDFGetter
from rx_selection import selection as sel

# Name of combinatorial component
_COMBINATORIAL_NAME : Final[str] = 'combinatorial'

log=LogStore.add_logger('fitter:cmb_constraints')
# ------------------------------------
class CmbConstraints:
    '''
    Class intended to provide constraints for shape of combinatorial model
    '''
    # ----------------------
    def __init__(
        self, 
        nll  : zlos,
        cfg  : DictConfig,
        q2bin: Qsq) -> None:
        '''
        Parameters
        -------------
        obs  : Zfit observable
        cfg  : fit configuration
        q2bin: E.g. central
        '''
        self._q2bin  = q2bin
        self._cfg    = cfg
        self._cmb_cfg= cfg.model.components[_COMBINATORIAL_NAME]

        self._obs, self._model = self._model_from_nll(nll = nll)
    # ----------------------
    def _model_from_nll(self, nll : zlos) -> tuple[zobs, zpdf]:
        '''
        Parameters
        -------------
        nll: Likelihood associated to current signal region

        Returns
        -------------
        Tuple with observable and combinatorial PDF
        '''
        models : list[zpdf] = []
        for pdf in nll.model:
            if _COMBINATORIAL_NAME in pdf.name:
                log.info(f'Picking: {pdf.name}')
                models.append(pdf)
            else:
                log.debug(f'Skipping: {pdf.name}')

        nmodels = len(models)
        if nmodels != 1:
            raise ValueError(f'Expected one combinatorial model, got: {nmodels}')

        pdf = models[0]
        obs = pdf.space
        if not isinstance(obs, zobs):
            raise ValueError('Observable not of Space type')

        return obs, pdf
    # ---------------------
    def _get_data(self) -> zdat:
        '''
        Returns
        -------------
        1D numpy array with masses to fit
        '''
        cons   = self._cmb_cfg[self._q2bin]['constraints']
        sample = Sample(cons.sample)
        trigger= Trigger(cons.trigger)

        gtr = RDFGetter(sample = sample, trigger = trigger)
        rdf = gtr.get_rdf(per_file = False)

        rdf = sel.apply_full_selection(
            rdf     = rdf,
            trigger = trigger,
            process = sample,
            q2bin   = self._q2bin,
            uid     = gtr.get_uid(),
        )

        if rdf.Count().GetValue() == 0:
            rep = rdf.Report()
            rep.Print()
            raise ValueError('Found no entries in dataset')

        if self._obs.obs is None:
            raise ValueError('Cannot extract observable names')

        name  = self._obs.obs[0]
        array = rdf.AsNumpy([name])[name]
        data  = zfit.Data.from_numpy(obs = self._obs, array = array)

        if not isinstance(data, zdat):
            raise ValueError('Data is not a zfit.Data instance')

        log.info(f'Found data with shape: {data.shape}')

        return data
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
        ftr = Fitter(pdf = self._model, data = data)
        res = ftr.fit()

        log.info(res)

        return res
    # ----------------------
    def _pick_parameter(self, name : str) -> bool:
        '''
        Parameters
        -------------
        name: Parameter name from SS fit

        Returns
        -------------
        True if this parameter is meant to be constrained
        '''
        par_names = self._cmb_cfg[self._q2bin].constraints.parameters

        return any( name.startswith(par_name) for par_name in par_names )
    # ----------------------
    def get_constraint(self) -> ConstraintND:
        '''
        Returns
        ----------------------
        N dimensional Gaussian constraint
        '''
        data = self._get_data()
        res  = self._fit(data=data)

        values = [ float(par.value().numpy()) for par in res.params if self._pick_parameter(name = par.name) ]
        names  = [ par.name                   for par in res.params if self._pick_parameter(name = par.name) ]
        params = [ par                        for par in res.params if self._pick_parameter(name = par.name) ]
        cov    = res.covariance(params = params)

        cns = ConstraintND(
            kind       = 'GaussianConstraint',
            parameters = names, 
            values     = values, 
            cov        = cov.tolist(),
        )

        return cns
# ------------------------------------
