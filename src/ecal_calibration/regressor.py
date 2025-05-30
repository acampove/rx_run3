'''
Module containing the Regressor class
'''
import os

import torch
import numpy

from torch import nn
from torch import optim
from torch import Tensor

from dask.dataframe           import DataFrame as DDF
from dmu.logging.log_store    import LogStore
from ecal_calibration.network import Network, ConstantModel

log=LogStore.add_logger('ecal_calibration:regressor')
# ---------------------------------------------
class Regressor:
    '''
    Class used to train a regressor to _learn_ energy
    corrections
    '''
    # ---------------------------------------------
    def __init__(self, ddf : DDF, cfg : dict):
        '''
        Parameters
        -------------------
        ddf : Dask dataframe storing the target and the features
        cfg : Dictionary holding configuration
        '''

        self._ddf = ddf
        self._cfg = cfg

        self._device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')

        self._net : Network
    # ---------------------------------------------
    def _get_training_data(self) -> tuple[Tensor,Tensor]:
        target     = self._cfg['target']
        df         = self._ddf.compute()
        arr_target = df[target]
        df         = df.drop(target, axis=1)

        features   = torch.tensor(df.values , dtype=torch.float32)
        targets    = torch.tensor(arr_target, dtype=torch.float32)

        return features, targets
    # ---------------------------------------------
    def _get_model_path(self) -> str:
        ana_dir = os.environ['ANADIR']
        out_dir = self._cfg['saving']['out_dir']
        out_dir = f'{ana_dir}/{out_dir}'
        os.makedirs(out_dir, exist_ok=True)

        out_path = f'{out_dir}/model.pth'

        return out_path
    # ---------------------------------------------
    def _save_regressor(self, regressor : Network) -> None:
        out_path = self._get_model_path()

        log.info(f'Saving model to: {out_path}')
        torch.save(regressor, out_path)
    # ---------------------------------------------
    def _move_to_gpu(self, x) -> None:
        if not torch.cuda.is_available():
            log.warning('Cannot move object to GPU, GPU not available?')
            return

        log.debug('Moving object to GPU')

        x.to(self._device)
    # ---------------------------------------------
    def train(self, constant_target : float = None) -> None:
        '''
        Will train the regressor

        Parameters
        -------------
        constant_target (float) : By default None. If passed, will create network that outputs always this value. Used for debugging
        '''

        features, targets   = self._get_training_data()
        nsamples, nfeatures = features.shape

        log.info(f'Training with {nsamples} samples')

        if constant_target is None:
            net = Network(nfeatures=nfeatures)
        else:
            net = ConstantModel(target=constant_target)

        self._move_to_gpu(net)
        self._move_to_gpu(features)
        self._move_to_gpu(targets)

        criterion = nn.MSELoss()
        optimizer = optim.Adam(net.parameters(), lr=cfg_trn['lr'])

        cfg_trn   = self._cfg['train']
        for epoch in range(cfg_trn['epochs']):
            net.train()
            optimizer.zero_grad()
            outputs = net(features)
            loss    = criterion(outputs, targets)
            loss.backward()
            optimizer.step()

            if epoch % 50 == 0:
                log.info(f'Epoch {epoch}, Loss: {loss.item():.4f}')

        self._save_regressor(regressor=net)

        net.eval()
        self._net = net
    # ---------------------------------------------
    def load(self) -> bool:
        '''
        Will load model. This would do exactly what `train` does, but without training.
        The model has to exist as a `mode.pth` file.

        If model is not found, return False
        '''
        model_path = self._get_model_path()
        log.debug(f'Picking model from: {model_path}')

        if not os.path.isfile(model_path):
            log.info(f'Model not found in: {model_path}')
            return False

        net        = torch.load(model_path, weights_only=False)
        net.eval()
        self._net  = net

        return True
    # ---------------------------------------------
    def predict(self, features : Tensor) -> numpy.ndarray:
        '''
        Runs prediction of targets and returns them as a numpy array

        If model does not exist it will train it with the data passed in the initializer

        Parameters
        ---------------
        features: Tensor with features to predict from

        Returns
        ---------------
        Numpy array with values of predicted targets
        '''
        if not self.load():
            log.info('Model not found, training it')
            self.train()

        targets = self._net(features)

        return targets.detach().numpy()
# ---------------------------------------------
