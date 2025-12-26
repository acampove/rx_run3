'''
Module with CmbConstraints class
'''
from dmu.stats import ConstraintND
from omegaconf import DictConfig
from rx_common import Qsq
from zfit      import Space               as zobs

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
        self._obs   = obs
        self._cfg   = cfg
        self._q2bin = q2bin
    # ----------------------
    def get_constraint(self) -> ConstraintND:
        '''
        Returns
        ----------------------
        N dimensional Gaussian constraint
        '''
        cns = ConstraintND(
            kind       = 'GaussianConstraint',
            parameters = ['a', 'b', 'c'],
            values     = [1.0, 2.0, 3.0],
            cov        = [
                [1, 2, 3],
                [4, 5, 6],
                [7, 8, 9]]
        )

        return cns
# ------------------------------------
