'''
Module containing DataFitter class
'''
from typing import cast

from omegaconf          import DictConfig, OmegaConf
from dmu.workflow.cache import Cache
from fitter.base_fitter import BaseFitter
from fitter.data_model  import DataModel

# ------------------------
class DataFitter(BaseFitter, Cache):
    '''
    Fitter for data
    '''
    # ------------------------
    def __init__(self, cfg : DictConfig):
        '''
        cfg : configuration for the fit as a DictConfig object
        '''
        self._cfg = cfg

        BaseFitter.__init__(self)
        Cache.__init__(
                self,
                out_path = cfg.output_directory,
                config   = cfg)
    # ------------------------
    def run(self) -> DictConfig:
        '''
        Runs fit

        Returns
        ------------
        DictConfig object with fitting results
        '''
        result_path = f'{self._cfg.output_directory}/parameters.yaml'
        if self._copy_from_cache():
            res = OmegaConf.load(result_path)
            res = cast(DictConfig, res)

            return res

        data = self._get_data()
        model= self._get_model()
        res  = self._fit(data=data, model=model)

        OmegaConf.save(res, result_path)
        self._cache()

        return res
# ------------------------
