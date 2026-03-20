'''
Script holding ConstraintReader class
'''

import yaml

from pathlib             import Path
from typing              import Final
from dmu                 import LogStore
from dmu.stats           import Constraint, Constraint1D, ConstraintType
from rx_common           import Component
from zfit.loss           import ExtendedUnbinnedNLL

from .configs            import CombinatorialConf, RXFitConfig
from .prec_scales        import PrecScales
from .misid_constraints  import MisIDConstraints 
from .cmb_constraints    import CmbConstraints
from .signal_constraints import SignalConstraints

_MISID_COMPONENTS   : Final[set[Component]] = {Component.bpkkk, Component.bpkpipi}

log=LogStore.add_logger('fitter:constraint_reader')
# -------------------------------------------------------------
class ConstraintReader:
    '''
    Class meant to provide constraints for fitting model
    '''
    # ----------------------
    def __init__(
        self, 
        nll   : ExtendedUnbinnedNLL, 
        cfg   : RXFitConfig,
        signal: Component = Component.bpkpee):
        '''
        Parameters
        -------------------
        nll   : Likelihoood 
        cfg   : Object storing fit configuration
        signal: Process considered as the signal when calculating denominator of rare Partially
                reconstructed scale constraints
        '''
        s_par         = nll.get_params(floating=True) 
        if not s_par:
            raise ValueError('No floating parameters found in parameters holder')

        self._nll     = nll
        self._l_par   = [ par.name for par in s_par ] 
        self._cfg     = cfg
        self._signal  = signal 

        self._constraints : list[Constraint] = []
    # ----------------------
    def _add_prec_constraints(self) -> None:
        '''
        Appends constraints to _constraints list for parameters in parameter holder
        whose names start with `pscale`
        '''

        scales : dict[str,Component] = { comp.scale : comp for comp in self._cfg.mod_cfg.constraints.pre_rare}
        for par in self._l_par:
            if par not in scales:
                continue

            log.info(f'Adding part reco constraint for: {par}')

            obj      = PrecScales(
                output_directory = self._cfg.output_directory,
                comp             = scales[par], 
                q2bin            = self._cfg.q2bin)
            val, err = obj.get_scale(signal=self._signal)

            cns = Constraint1D(
                name = par,
                kind = ConstraintType.gauss,
                mu   = val,
                sg   = err)

            self._constraints.append(cns)
    # ----------------------
    def _add_misid_constraints(self) -> None:
        '''
        Adds constraints for fully hadronic MisID components
        '''
        components= self._cfg.mod_cfg.components
        all_found = all(component in components for component in _MISID_COMPONENTS)
        any_found = any(component in components for component in _MISID_COMPONENTS)
        if not all_found and     any_found:
            raise ValueError('At least one misID component was found, but not all')

        if not all_found and not any_found:
            log.info('Skipping misID constraints')
            return

        log.info('Adding MisID constraints')
        
        cfg = self._cfg.mod_cfg.constraints.misid
        if cfg is None:
            log.debug('No MisID constraints config found, not adding constraints')
            return

        mrd       = MisIDConstraints(
            cfg   = cfg,
            q2bin = self._cfg.q2bin)

        self._constraints += mrd.get_constraints()
    # ----------------------
    def _add_combinatorial_constraints(self) -> None:
        '''
        Adds combinatorial constraints
        '''
        components= self._cfg.mod_cfg.components
        if Component.comb not in components:
            log.warning(f'Combinatorial {Component.comb} component not found, not calculating constraints')
            return

        cmb_cfg = self._cfg.mod_cfg.components[Component.comb]
        if not isinstance(cmb_cfg, CombinatorialConf):
            raise ValueError(f'Expected combinatorial config, got: {cmb_cfg}')

        if self._cfg.q2bin not in cmb_cfg.constraints:
            log.info(f'Not constraining the combinatorial shape for: {self._cfg.q2bin}/{self._cfg.mod_cfg.trigger}')
            return

        calc      = CmbConstraints(
            name  = Component.comb,
            nll   = self._nll,
            cfg   = cmb_cfg,
            q2bin = self._cfg.q2bin)

        self._constraints.append( calc.get_constraint() )
    # ----------------------
    def _add_signal_constraints(self) -> None:
        '''
        Update _constraints with constraints to the mass scale, resolution
        '''
        calc = SignalConstraints(
            comp= self._signal,
            nll = self._nll)

        cons = calc.get_constraints()

        log.info(f'Found {len(cons)} signal constraints')

        self._constraints += cons
    # ----------------------
    def _save_constraints(self, path : Path) -> None:
        '''
        Parameters
        -------------
        path: Path to YAML file to save constraints to
        '''
        log.info(f'Saving constraints to: {path}')

        if not self._constraints:
            return

        path.parent.mkdir(parents = True, exist_ok = True)

        data = { cons.name : cons.model_dump() for cons in self._constraints }

        yaml_string = yaml.dump(data)
        path.write_text(yaml_string)
    # ----------------------
    def get_constraints(self, save_to : Path | None = None) -> list[Constraint]:
        '''
        Parameters
        ---------------
        save_to: Path to yaml file where constraints are save, default None

        Returns dictionary with constraints, i.e.

        Key  : Name of fitting parameter
        Value: Tuple with mu and error
        '''
        self._add_misid_constraints()
        self._add_prec_constraints()
        self._add_combinatorial_constraints()
        self._add_signal_constraints()

        if save_to is None:
            return self._constraints

        self._save_constraints(path = save_to)

        return self._constraints
# -------------------------------------------------------------
