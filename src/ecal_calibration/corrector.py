'''
Module containing the Corrector class
'''
import torch
import pandas as pnd

from vector                     import MomentumObject4D as v4d
from dmu.logging.log_store      import LogStore
from ecal_calibration.network   import Network
from ecal_calibration.regressor import Regressor

log=LogStore.add_logger('ecal_calibration:corrector')
# ---------------------------------------
class Corrector:
    '''
    Class intended to calibrate electrons
    '''
    # ---------------------------------------
    def __init__(self, cfg : dict):
        '''
        Parameters:

        cfg: Dictionary with configuration
        '''
        self._cfg = cfg
        self._net = self._get_network()

        self._electron_mass = 0.511
    # ---------------------------------------
    def _get_network(self) -> Network:
        model_dir = Regressor.get_out_dir(cfg=self._cfg)
        net       = Regressor.load(model_dir=model_dir)
        if net is None:
            raise FileNotFoundError(f'Model could not be found in: {model_dir}')

        return net
    # ---------------------------------------
    def _get_correction_from_nn(self, row : pnd.Series) -> float:
        features = row.to_numpy()
        features = torch.tensor(features, dtype=torch.float32)

        targets  = self._net(features)
        val      = targets.detach().numpy()

        return val
    # ---------------------------------------
    def run(
            self,
            electron : v4d,
            row      : pnd.Series,
            from_nn  : bool = True) -> v4d:
        '''
        Calibrates electron

        Parameters
        -----------------
        electron: Lorentz vector before calibration
        row     : Pandas series with the features needed for predicting correction
        from_nn : If true, it will evaluate an NN, otherwise it will read the correction from input series

        Returns
        -----------------
        Lorentz vector representing calibrated electron
        '''
        if from_nn:
            val = self._get_correction_from_nn(row=row)
        else:
            val = row['mu']

        # The target was scaled by 1000 for training
        # Scale back the correction
        val         = float(val) / 1000.

        log.debug(f'Original : {electron}')
        res = v4d(
                pt  = val * electron.pt,
                eta =       electron.eta,
                phi =       electron.phi,
                mass= self._electron_mass)

        log.debug(f'Corrected: {res}')

        return res
# ---------------------------------------
