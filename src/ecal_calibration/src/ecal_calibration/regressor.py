'''
Module containing the Regressor class
'''
# pylint: disable=too-many-instance-attributes

import os

import pandas            as pnd
import matplotlib.pyplot as plt
import mplhep
import torch
import numpy

from torch             import nn
from torch             import optim
from torch             import Tensor
from scipy.interpolate import griddata

from dask.dataframe           import DataFrame as DDF
from dmu.logging.log_store    import LogStore
from ecal_calibration.network import Network, ConstantModel
from ecal_calibration         import calo_translator as ctran

log=LogStore.add_logger('ecal_calibration:regressor')
# ---------------------------------------------
class Regressor:
    '''
    Class used to train a regressor to _learn_ energy
    corrections
    '''
    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    # ---------------------------------------------
    def __init__(self, ddf_tr : DDF, ddf_ts : DDF, cfg : dict):
        '''
        Parameters
        -------------------
        ddf_tr : Dask dataframe storing training dataset
        ddf_ts : Dask dataframe storing testing dataset
        cfg    : Dictionary holding configuration
        '''
        plt.style.use(mplhep.style.LHCb2)

        self._ddf_tr = ddf_tr
        self._ddf_ts = ddf_ts
        self._cfg    = cfg
        self._out_dir= Regressor.get_out_dir(cfg=cfg, create=True)

        self._d_area = {0 : 'Outer', 1 : 'Middle', 2 : 'Inner'}
        self._d_color= {0 : 'blue' , 1 : 'green' , 2 : 'red'}
        self._d_var  = {'Real' : 'mu', 'Predicted' : 'mu_pred'}

        self._net : Network
    # ---------------------------------------------
    def _save_regressor(self, regressor : Network) -> None:
        out_path = f'{self._out_dir}/model.pth'
        log.info(f'Saving model to: {out_path}')
        torch.save(regressor, out_path)
    # ---------------------------------------------
    def train(self, constant_target : float = None) -> None:
        '''
        Will train the regressor

        Parameters
        -------------
        constant_target (float) : By default None. If passed, will create network that outputs always this value. Used for debugging
        '''

        features, targets   = Regressor.get_tensors(cfg = self._cfg, ddf = self._ddf_tr)
        nsamples, nfeatures = features.shape

        log.info(f'Training with {nsamples} samples')

        if constant_target is None:
            net = Network(nfeatures=nfeatures, model=self._cfg['model'])
        else:
            net = ConstantModel(target=constant_target)

        net      = Regressor.move_to_gpu(net)
        features = Regressor.move_to_gpu(features)
        targets  = Regressor.move_to_gpu(targets)

        criterion = nn.MSELoss()

        cfg_trn   = self._cfg['train']

        learning_rate = cfg_trn['lr']
        log.debug(f'Using learning_rate: {learning_rate}')
        optimizer = optim.Adam(net.parameters(), lr=learning_rate)
        for epoch in range(cfg_trn['epochs']):
            net.train()
            optimizer.zero_grad()
            outputs = net(features)
            loss    = criterion(outputs, targets)
            loss.backward()
            optimizer.step()

            if epoch % 2000 == 0:
                log.info(f'Epoch {epoch}, Loss: {loss.item():.4f}')

        self._save_regressor(regressor=net)

        net.eval()
        self._net = net
    # ---------------------------------------------
    def _add_xy(self, row : pnd.Series, var : str) -> pnd.Series:
        r = row['row']
        c = row['col']
        a = row['are']

        x, y = ctran.from_id_to_xy(row=int(r), col=int(c), area=int(a))

        return x if var == 'x' else y
    # ---------------------------------------------
    def test(self) -> None:
        '''
        Runs comparison of predicted
        '''
        self._net = Regressor.load(model_dir = self._out_dir)
        if self._net is None:
            log.info('Model not found, training it')
            self.train()

        self._net     = Regressor.move_to_gpu(self._net)
        l_fea         = self._cfg['features']
        ddf           = self._ddf_ts
        ddf           = ddf[ddf['row'] > 1]
        ddf           = ddf[ddf['col'] > 1]

        df            = ddf.compute()
        arr_fea       = df[l_fea].values
        features      = torch.tensor(arr_fea, dtype=torch.float32)

        df['mu_pred'] = self.predict(features=features) / 1000.
        df['mu']      = df['mu'] / 1000.
        df['x']       = df.apply(self._add_xy, args=('x',), axis=1)
        df['y']       = df.apply(self._add_xy, args=('y',), axis=1)

        self._plot_corrections(df=df)
        self._plot_by_energy(df=df)
        self._plot_by_area(df=df)

        self._plot_by_npvs(df=df, kind=     'Real')
        self._plot_by_npvs(df=df, kind='Predicted')

        self._plot_by_block(df=df, kind=     'Real')
        self._plot_by_block(df=df, kind='Predicted')

        self._plot_in_xy(df=df, kind=     'Real')
        self._plot_in_xy(df=df, kind='Predicted')
    # ---------------------------------------------
    def _plot_in_xy(self, df : pnd.DataFrame, kind : str) -> None:
        var = self._d_var[kind]
        grid_x, grid_y = numpy.mgrid[-3000:+3000:100j, -3000:+3000:100j]

        levels  = numpy.linspace(0.0, 1.5, 50)
        grid_z  = griddata((df['x'], df['y']), df[var], (grid_x, grid_y), method='cubic')
        contour = plt.contourf(grid_x, grid_y, grid_z, levels=levels, cmap='viridis')

        plt.colorbar(contour, label=kind)
        plt.xlabel('x')
        plt.ylabel('y')
        plt.title(r'$\mu$ vs $x$ and $y$')
        plt.tight_layout()
        plt.savefig(f'{self._out_dir}/xy_correction_{kind}.png')
        plt.close()
    # ---------------------------------------------
    def _plot_corrections(self, df : pnd.DataFrame) -> None:
        nentries = len(df)

        ax = None
        for area, df_area in df.groupby('are'):
            area = int(area)
            name = self._d_area[area]
            color= self._d_color[area]

            ax   = df_area.plot.scatter(x='mu', y='mu_pred', s=1, label=name, ax=ax, color=color)

        plt.xlabel('Real')
        plt.ylabel('Predicted')
        plt.title(f'Entries: {nentries}')
        plt.grid()
        plt.savefig(f'{self._out_dir}/prediction_vs_real.png')
        plt.close()
    # ---------------------------------------------
    def _plot_by_area(self, df : pnd.DataFrame) -> None:
        for area, df_area in df.groupby('are'):
            area = int(area)
            name = self._d_area[area]

            ax = None
            ax = df_area['mu'     ].plot.hist(bins=20, range=[0.0, 1.5], label='Real'     , alpha   =   0.3, ax=ax, color='blue')
            ax = df_area['mu_pred'].plot.hist(bins=20, range=[0.0, 1.5], label='Predicted', histtype='step', ax=ax, color='red' )

            plt.title(f'Region: {name}')
            plt.xlabel('Correction')
            plt.ylabel('Entries')
            plt.legend()
            plt.savefig(f'{self._out_dir}/area_{name}.png')
            plt.close()
    # ---------------------------------------------
    def _plot_by_energy(self, df : pnd.DataFrame) -> None:
        ax = None
        ax = df.plot.scatter('eng', 'mu'     , label='Real'     , color='blue', s=1, ax=ax)
        ax = df.plot.scatter('eng', 'mu_pred', label='Predicted', color='red' , s=1, ax=ax)

        plt.legend()
        plt.xlabel(r'$E(\gamma)$ MeV')
        plt.ylabel(r'$\mu$')
        plt.savefig(f'{self._out_dir}/corr_vs_energy.png')
        plt.close()
    # ---------------------------------------------
    def _plot_by_npvs(self, df : pnd.DataFrame, kind : str) -> None:
        ax  = None
        var = self._d_var[kind]
        for npv, df_npv in df.groupby('npv'):
            npv = int(npv)

            if npv % 2 == 0:
                continue

            if npv > 10:
                break

            ax = df_npv[var].plot.hist(label=f'nPV={npv}', bins=10, range=[0.0, 1.5], histtype='step', density=True, ax=ax)

        plt.legend()
        plt.title(kind)
        plt.xlabel(r'Correction')
        plt.ylabel(r'$\mu$')

        plt.savefig(f'{self._out_dir}/corr_vs_npv_{kind}.png')
        plt.close()
    # ---------------------------------------------
    def _plot_by_block(self, df : pnd.DataFrame, kind : str) -> None:
        ax  = None
        var = self._d_var[kind]
        for blk, df_blk in df.groupby('blk'):
            blk= int(blk)
            ax = df_blk[var].plot.hist(label=f'Block={blk}', bins=10, range=[0.0, 1.5], histtype='step', density=True, ax=ax)

        plt.legend()
        plt.title(kind)
        plt.xlabel(r'Correction')
        plt.ylabel(r'$\mu$')

        plt.savefig(f'{self._out_dir}/corr_vs_block_{kind}.png')
        plt.close()
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
        self._net = Regressor.load(model_dir = self._out_dir)
        if self._net is None:
            log.info('Model not found, training it')
            self.train()

        self._net = Regressor.move_to_gpu(self._net)
        features  = Regressor.move_to_gpu(features)
        targets   = self._net(features)
        targets   = targets.cpu()

        return targets.detach().numpy()
    # ---------------------------------------------
    @staticmethod
    def move_to_gpu(x):
        '''
        Will move tensor to GPU in order to do training, prediction, etc
        '''
        if not torch.cuda.is_available():
            log.warning('Cannot move object to GPU, GPU not available?')
            return x

        log.debug('Moving object to GPU')

        x = x.to(Regressor.device)

        return x
    # ---------------------------------------------
    @staticmethod
    def get_out_dir(cfg : dict, create : bool = False) -> str:
        '''
        This function assumes that the path will **start** in ANADIR

        Parameters
        -----------------
        cfg   : Config with 'saving:out_dir' entry specifying path
        create: If true, it will try to create the directory
        '''
        ana_dir = os.environ['ANADIR']
        out_dir = cfg['saving']['out_dir']
        out_dir = f'{ana_dir}/{out_dir}'

        log.info(f'Using output directory: {out_dir}')
        if create:
            os.makedirs(out_dir, exist_ok=True)

        return out_dir
    # ---------------------------------------------
    @staticmethod
    def get_tensors(cfg : dict, ddf : DDF) -> tuple[Tensor,Tensor]:
        '''
        Provides features and target tensors in a tuple

        Parameters
        -----------------
        cfg : Dictionary storing configuration
        ddf : Dask dataframe with data
        '''
        target     = cfg['target']
        l_feat     = cfg['features']

        log.debug(f'Using features: {l_feat}')
        log.debug(f'Using target  : {target}')

        df         = ddf.compute()
        arr_target = df[target].to_numpy()
        arr_feat   = df[l_feat].values

        features   = torch.tensor(arr_feat, dtype=torch.float32)
        targets    = torch.tensor(arr_target, dtype=torch.float32)
        targets    = targets.unsqueeze(1)

        log.debug(f'Features shape: {features.shape}')
        log.debug(f'Targets shape: {targets.shape}')

        return features, targets
    # ---------------------------------------------
    @staticmethod
    def load(model_dir : str) -> Network|None:
        '''
        Parameters
        ----------------
        model_dir: Directory where model.pth is found

        Returns
        ----------------
        Network with model
        '''
        model_path = f'{model_dir}/model.pth'
        log.debug(f'Picking model from: {model_path}')

        if not os.path.isfile(model_path):
            log.info(f'Model not found in: {model_path}')
            return None

        net = torch.load(model_path, map_location='cpu', weights_only=False)
        net.eval()

        return net
# ---------------------------------------------
