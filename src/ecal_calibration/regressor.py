'''
Module containing the Regressor class
'''
import os

import torch
from torch import nn
from torch import optim
from torch import Tensor

from dask.dataframe.core      import DataFrame as DDF
from dmu.logging.log_store    import LogStore
from ecal_calibration.network import Network

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
    # ---------------------------------------------
    def _get_training_data(self) -> tuple[Tensor,Tensor]:
        df         = self._ddf.compute()
        arr_target = df[self._cfg['target']]
        df         = df.drop(self._cfg['target'], axis=1)

        features = torch.tensor(df.values , dtype=torch.float32)
        targets  = torch.tensor(arr_target, dtype=torch.float32)

        return features, targets
    # ---------------------------------------------
    def train(self) -> None:
        '''
        Will train the regressor
        '''
        cfg_trn   = self._cfg['train']
        net       = Network()
        criterion = nn.MSELoss()
        optimizer = optim.Adam(net.parameters(), lr=cfg_trn['lr'])

        features, targets = self._get_training_data()
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
    # ---------------------------------------------
    def _save_regressor(self, regressor : Network) -> None:
        ana_dir = os.environ['ANADIR']
        out_dir = self._cfg['saving']['out_dir']
        out_dir = f'{ana_dir}/{out_dir}'
        os.makedirs(out_dir, exist_ok=True)

        out_path = f'{out_dir}/model.pth'
        log.info(f'Saving model to: {out_path}')
        torch.save(regressor, out_path)
# ---------------------------------------------
