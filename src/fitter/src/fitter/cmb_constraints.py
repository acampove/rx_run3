'''
Module with CmbConstraints class
'''
import zfit

from pathlib      import Path
from omegaconf    import OmegaConf
from ROOT         import RDF # type: ignore 
from typing       import Final
from dmu          import LogStore
from dmu.workflow import Cache
from dmu.stats    import ConstraintND
from omegaconf    import DictConfig
from zfit         import Space               as zobs
from zfit         import Data                as zdat
from zfit.pdf     import BasePDF             as zpdf
from zfit.loss    import ExtendedUnbinnedNLL as zlos
from rx_common    import Qsq, Sample, Trigger
from rx_data      import RDFGetter
from rx_selection import selection as sel
from .base_fitter import BaseFitter

# Name of combinatorial component
_COMBINATORIAL_NAME : Final[str] = 'combinatorial'

log=LogStore.add_logger('fitter:cmb_constraints')
# ------------------------------------
class CmbConstraints(BaseFitter, Cache):
    '''
    Class intended to provide constraints for shape of combinatorial model
    '''
    # ----------------------
    def __init__(
        self, 
        name : str,
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
        BaseFitter.__init__(self)

        self._q2bin  = q2bin
        self._cfg    = cfg
        self._cmb_cfg= cfg.model.components[_COMBINATORIAL_NAME]

        cons         = self._cmb_cfg[self._q2bin]['constraints']
        self._sample = Sample(cons.sample)
        self._trigger= Trigger(cons.trigger)

        self._obs, self._model = self._model_from_nll(nll = nll)

        self._base_path = Path(f'{cfg.output_directory}/{name}/{self._cfg.trigger}_{self._q2bin}')
        self._rdf, uid, self._cuts = self._get_rdf()


        Cache.__init__(
            self,
            rdf_uid  = uid,
            out_path = self._base_path,
            config   = OmegaConf.to_container(cfg, resolve=True))
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
    def _get_rdf(self) -> tuple[RDF.RNode, str, DictConfig]:
        '''
        Returns
        -------------
        Tuple with:

        - DataFrame after full selection 
        - Unique identifier for input ntuples and selection
        - Dictionary with selection used
        '''
        gtr = RDFGetter(sample = self._sample, trigger = self._trigger)
        rdf = gtr.get_rdf(per_file = False)

        rdf = sel.apply_full_selection(
            rdf     = rdf,
            trigger = self._trigger,
            process = self._sample,
            q2bin   = self._q2bin,
            uid     = gtr.get_uid(),
        )

        uid = getattr(rdf, 'uid')
        cuts= getattr(rdf, 'sel')

        with sel.custom_selection(d_sel = {}, force_override = True):
            default_cuts = sel.selection(
                q2bin    = self._q2bin, 
                process  = self._sample,
                trigger  = self._trigger,
            )

        cuts= {'default' : default_cuts, 'fit' : cuts}

        cuts= OmegaConf.create(obj=cuts)
        if not isinstance(cuts, DictConfig):
            raise ValueError('Cuts cannot be transformed to DictConfig')

        return rdf, uid, cuts
    # ---------------------
    def _get_data(self, rdf : RDF.RNode) -> zdat:
        '''
        Parameters
        -------------
        rdf: ROOT dataframe with e.g. SS data after selection

        Returns
        -------------
        Zfit dataset instance
        '''

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
        data = self._get_data(rdf = self._rdf)
        res  = self._fit(data=data, model=self._model, cfg = self._cfg)

        self._save_fit(
            data    = data, 
            res     = res, 
            out_path= self._out_path,
            plt_cfg = self._cmb_cfg[self._q2bin].constraints.plots,
            cut_cfg = self._cuts,
            model   = self._model)

        params = [ par for par in self._model.get_params() if self._pick_parameter(name = par.name) ] 
        values = [ float(par.value().numpy()) for par in params ]
        names  = [ par.name                   for par in params ]
        cov    = res.covariance(params = names)

        cns = ConstraintND(
            kind       = 'GaussianConstraint',
            parameters = names, 
            values     = values, 
            cov        = cov.tolist(),
        )

        constraints_path = self._out_path / 'constraints.json'
        cns.to_json(constraints_path)

        return cns
# ------------------------------------
