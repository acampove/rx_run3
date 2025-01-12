'''
Module storing ZModel class 
'''
# pylint: disable=too-many-lines, import-error

from typing import Callable, Union

import zfit
from zfit.core.interfaces   import ZfitSpace
from zfit.core.basepdf      import BasePDF
from dmu.logging.log_store  import LogStore

zspace = ZfitSpace
zpdf   = BasePDF

log=LogStore.add_logger('rx_calibration:zmodel')


#-----------------------------------------
class MethodRegistry:
    # Registry dictionary to hold methods
    _d_method = {}

    @classmethod
    def register(cls, nickname):
        def decorator(method):
            cls._d_method[nickname] = method
            return method

        return decorator

    @classmethod
    def get_method(cls, nickname):
        return cls._d_method.get(nickname, None)
#-----------------------------------------
class ModelFactory:
    '''
    Class used to create Zfit PDFs
    '''
    #-----------------------------------------
    def __init__(self, obs : zspace):
        self._obs = obs
    #-----------------------------------------
    @MethodRegistry.register('EXP')
    def _get_exponential(self, suffix : str = ''):
        c   = zfit.param.Parameter(f'c_cmb{suffix}', -0.005, -0.05, 0.05)
        pdf = zfit.pdf.Exponential(c, self._obs)

        return pdf
    #-----------------------------------------
    @MethodRegistry.register('POL')
    def _get_pol(self, order : int, suffix : str = ''):
        if order == 1:
            a   = zfit.param.Parameter(f'a_cmb{suffix}', -0.005, -0.95, 0.00)
            pdf = zfit.pdf.Chebyshev(obs=self._obs, coeffs=[a])

            return pdf

        if order == 2:
            a   = zfit.param.Parameter(f'a_cmb{suffix}', -0.005, -0.95, 0.00)
            b   = zfit.param.Parameter(f'b_cmb{suffix}',  0.000, -0.95, 0.95)
            pdf = zfit.pdf.Chebyshev(obs=self._obs, coeffs=[a, b])

            return pdf

        raise ValueError(f'Invalid polynomial of order: {order}')
    #-----------------------------------------
    def _get_pdf_types(self, l_name) -> list[tuple[str,str]]:
        d_name_freq = {}

        l_type = []
        for name in l_name:
            if name not in d_name_freq:
                d_name_freq[name] = 1
            else:
                d_name_freq[name]+= 1

            frq = d_name_freq[name]
            frq = f'_{frq}'

            l_type.append((name, frq))

        return l_type
    #-----------------------------------------
    def _get_pdf(self, kind : str, preffix : str) -> zpdf:
        fun = MethodRegistry.get_method(kind)
        if fun is None:
            raise NotImplementedError(f'PDF of type {kind} is not implemented')

        return fun(self, preffix)
    #-----------------------------------------
    def get_pdf(self, l_name : list[str]) -> zpdf:
        '''
        Given a list of strings representing PDFs returns the a zfit PDF which is
        the sum of them
        '''
        l_type= self._get_pdf_types(l_name)
        l_pdf = [ self._get_pdf(kind, preffix) for kind, preffix in l_type ]

        return l_pdf
#-----------------------------------------
