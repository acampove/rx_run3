'''
Module storing ZModel class
'''
# pylint: disable=too-many-lines, import-error

from typing import Callable, Union

import zfit
from zfit.core.interfaces   import ZfitSpace as zobs
from zfit.core.basepdf      import BasePDF   as zpdf
from zfit.core.parameter    import Parameter as zpar
from dmu.logging.log_store  import LogStore

log=LogStore.add_logger('rx_calibration:zmodel')


#-----------------------------------------
class MethodRegistry:
    # Registry dictionary to hold methods
    _d_method = {}

    @classmethod
    def register(cls, nickname : str):
        '''
        Decorator in charge of registering method for given nickname
        '''
        def decorator(method):
            cls._d_method[nickname] = method
            return method

        return decorator

    @classmethod
    def get_method(cls, nickname : str) -> Union[Callable,None]:
        '''
        Will return method in charge of building PDF, for an input nickname
        '''
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
    def _get_exponential(self, suffix : str = '') -> zpdf:
        c   = zfit.param.Parameter(f'c_cmb{suffix}', -0.005, -0.05, 0.05)
        pdf = zfit.pdf.Exponential(c, self._obs)

        return pdf
    #-----------------------------------------
    @MethodRegistry.register('POL1')
    def _get_pol1(self, suffix : str = '') -> zpdf:
        a   = zfit.param.Parameter(f'a_cmb{suffix}', -0.005, -0.95, 0.00)
        pdf = zfit.pdf.Chebyshev(obs=self._obs, coeffs=[a])

        return pdf
    #-----------------------------------------
    @MethodRegistry.register('POL2')
    def _get_pol2(self, suffix : str = '') -> zpdf:
        a   = zfit.param.Parameter(f'a_cmb{suffix}', -0.005, -0.95, 0.00)
        b   = zfit.param.Parameter(f'b_cmb{suffix}',  0.000, -0.95, 0.95)
        pdf = zfit.pdf.Chebyshev(obs=self._obs, coeffs=[a, b])

        return pdf
    #-----------------------------------------
    @MethodRegistry.register('CBR')
    def _get_cb(self, suffix : str = '') -> zpdf:
        mu  = zfit.param.Parameter(f'mu{suffix}', 5300, 5250, 5350)
        sg  = zfit.param.Parameter(f'sg{suffix}', 10, 2, 300)

        ar  = zfit.param.Parameter(f'ar{suffix}', -2, -4., -1.)
        nr  = zfit.param.Parameter(f'nr{suffix}' , 1, 0.5, 5)

        pdf = zfit.pdf.CrystalBall(mu, sg, ar, nr, self._obs)

        return pdf
    #-----------------------------------------
    @MethodRegistry.register('DSCB')
    def _get_dscb(self, suffix : str = '') -> zpdf:
        mu  = zfit.param.Parameter(f'mu{suffix}' , 5300, 5250, 5400)
        sg  = zfit.param.Parameter(f'sg{suffix}' ,   10,    2,   30)
        ar  = zfit.param.Parameter(f'ar{suffix}' ,    1,    0,    5)
        al  = zfit.param.Parameter(f'al{suffix}' ,    1,    0,    5)
        nr  = zfit.param.Parameter(f'nr{suffix}' ,    2,    1,    5)
        nl  = zfit.param.Parameter(f'nl{suffix}' ,    2,    0,    5)

        pdf = zfit.pdf.DoubleCB(mu, sg, al, nl, ar, nr, self._obs)

        return pdf
    #-----------------------------------------
    @MethodRegistry.register('CBL')
    def _get_cb(self, suffix : str = '') -> zpdf:
        mu  = zfit.param.Parameter(f'mu{suffix}', 5300, 5250, 5350)
        sg  = zfit.param.Parameter(f'sg{suffix}', 10, 2, 300)

        al  = zfit.param.Parameter(f'al{suffix}',  2,  1.,  4.)
        nl  = zfit.param.Parameter(f'nl{suffix}' , 2, 0.5, 5)

        pdf = zfit.pdf.CrystalBall(mu, sg, al, nl, self._obs)

        return pdf
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
