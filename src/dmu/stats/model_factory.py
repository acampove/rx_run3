'''
Module storing ZModel class
'''
# pylint: disable=too-many-lines, import-error, too-many-positional-arguments, too-many-arguments

from typing import Callable, Union

import zfit
from zfit.core.interfaces   import ZfitSpace as zobs
from zfit.core.basepdf      import BasePDF   as zpdf
from zfit.core.parameter    import Parameter as zpar
from dmu.logging.log_store  import LogStore

log=LogStore.add_logger('dmu:stats:model_factory')
#-----------------------------------------
class MethodRegistry:
    '''
    Class intended to store protected methods belonging to ModelFactory class
    which is defined in this same module
    '''
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
        method = cls._d_method.get(nickname, None)

        if method is not None:
            return method

        log.warning('Available PDFs:')
        for value in cls._d_method:
            log.info(f'    {value}')

        return method
#-----------------------------------------
class ModelFactory:
    '''
    Class used to create Zfit PDFs by passing only the nicknames, e.g.:

    ```python
    from dmu.stats.model_factory import ModelFactory

    l_pdf = ['dscb', 'gauss']
    l_shr = ['mu']
    mod   = ModelFactory(preffix = 'signal', obs = obs, l_pdf = l_pdf, l_shared=l_shr)
    pdf   = mod.get_pdf()
    ```

    where one can specify which parameters can be shared among the PDFs
    '''
    #-----------------------------------------
    def __init__(self,
                 preffix  : str,
                 obs      : zobs,
                 l_pdf    : list[str],
                 l_shared : list[str],
                 l_float  : list[str]):
        '''
        preffix:  used to identify PDF, will be used to name every parameter
        obs:      zfit obserbable
        l_pdf:    List of PDF nicknames which are registered below
        l_shared: List of parameter names that are shared
        l_float:  List of parameter names to allow to float
        '''

        self._preffix         = preffix
        self._l_pdf           = l_pdf
        self._l_shr           = l_shared
        self._l_flt           = l_float
        self._obs             = obs

        self._d_par : dict[str,zpar] = {}
    #-----------------------------------------
    def _split_name(self, name : str) -> tuple[str,str]:
        l_part = name.split('_')
        pname  = l_part[0]
        xname  = '_'.join(l_part[1:])

        return pname, xname
    #-----------------------------------------
    def _get_parameter_name(self, name : str, suffix : str) -> str:
        pname, xname = self._split_name(name)

        log.debug(f'Using physical name: {pname}')

        if pname in self._l_shr:
            name = f'{pname}_{self._preffix}'
        else:
            name = f'{pname}_{xname}_{self._preffix}{suffix}'

        if pname in self._l_flt:
            return f'{name}_flt'

        return name
    #-----------------------------------------
    def _get_parameter(self,
                       name   : str,
                       suffix : str,
                       val    : float,
                       low    : float,
                       high   : float) -> zpar:

        name = self._get_parameter_name(name, suffix)
        log.debug(f'Assigning name: {name}')

        if name in self._d_par:
            return self._d_par[name]

        par  = zfit.param.Parameter(name, val, low, high)

        self._d_par[name] = par

        return par
    #-----------------------------------------
    @MethodRegistry.register('exp')
    def _get_exponential(self, suffix : str = '') -> zpdf:
        c   = self._get_parameter('c_exp', suffix, -0.005, -0.20, 0.00)
        pdf = zfit.pdf.Exponential(c, self._obs, name=f'exp{suffix}')

        return pdf
    #-----------------------------------------
    @MethodRegistry.register('pol1')
    def _get_pol1(self, suffix : str = '') -> zpdf:
        a   = self._get_parameter('a_pol1', suffix, -0.005, -0.95, 0.00)
        pdf = zfit.pdf.Chebyshev(obs=self._obs, coeffs=[a], name=f'pol1{suffix}')

        return pdf
    #-----------------------------------------
    @MethodRegistry.register('pol2')
    def _get_pol2(self, suffix : str = '') -> zpdf:
        a   = self._get_parameter('a_pol2', suffix, -0.005, -0.95, 0.00)
        b   = self._get_parameter('b_pol2', suffix,  0.000, -0.95, 0.95)
        pdf = zfit.pdf.Chebyshev(obs=self._obs, coeffs=[a, b], name=f'pol2{suffix}')

        return pdf
    #-----------------------------------------
    @MethodRegistry.register('cbr')
    def _get_cbr(self, suffix : str = '') -> zpdf:
        mu  = self._get_parameter('mu_cbr', suffix, 5300, 5100, 5350)
        sg  = self._get_parameter('sg_cbr', suffix,   10,    2,  300)
        ar  = self._get_parameter('ac_cbr', suffix,   -2, -14., -0.1)
        nr  = self._get_parameter('nc_cbr', suffix,    1,  0.5,  150)

        pdf = zfit.pdf.CrystalBall(mu, sg, ar, nr, self._obs, name=f'cbr{suffix}')

        return pdf
    #-----------------------------------------
    @MethodRegistry.register('suj')
    def _get_suj(self, suffix : str = '') -> zpdf:
        mu  = self._get_parameter('mu_suj', suffix, 5300, 4000, 6000)
        sg  = self._get_parameter('sg_suj', suffix,   10,    2, 5000)
        gm  = self._get_parameter('gm_suj', suffix,    1,  -10,   10)
        dl  = self._get_parameter('dl_suj', suffix,    1,  0.1,   10)

        pdf = zfit.pdf.JohnsonSU(mu, sg, gm, dl, self._obs, name=f'suj{suffix}')

        return pdf
    #-----------------------------------------
    @MethodRegistry.register('cbl')
    def _get_cbl(self, suffix : str = '') -> zpdf:
        mu  = self._get_parameter('mu_cbl', suffix, 5300, 5100, 5350)
        sg  = self._get_parameter('sg_cbl', suffix,   10,    2,  300)
        al  = self._get_parameter('ac_cbl', suffix,    2,  0.1,  14.)
        nl  = self._get_parameter('nc_cbl', suffix,    1,  0.5,  150)

        pdf = zfit.pdf.CrystalBall(mu, sg, al, nl, self._obs, name=f'cbl{suffix}')

        return pdf
    #-----------------------------------------
    @MethodRegistry.register('gauss')
    def _get_gauss(self, suffix : str = '') -> zpdf:
        mu  = self._get_parameter('mu_gauss', suffix, 5300, 5100, 5350)
        sg  = self._get_parameter('sg_gauss', suffix,   10,    2,  300)

        pdf = zfit.pdf.Gauss(mu, sg, self._obs, name=f'gauss{suffix}')

        return pdf
    #-----------------------------------------
    @MethodRegistry.register('dscb')
    def _get_dscb(self, suffix : str = '') -> zpdf:
        mu  = self._get_parameter('mu_dscb', suffix, 4000, 4000, 5400)
        sg  = self._get_parameter('sg_dscb', suffix,   10,    2,  500)
        ar  = self._get_parameter('ar_dscb', suffix,    1,    0,    5)
        al  = self._get_parameter('al_dscb', suffix,    1,    0,    5)
        nr  = self._get_parameter('nr_dscb', suffix,    2,    1,  150)
        nl  = self._get_parameter('nl_dscb', suffix,    2,    0,  150)

        pdf = zfit.pdf.DoubleCB(mu, sg, al, nl, ar, nr, self._obs, name=f'dscb{suffix}')

        return pdf
    #-----------------------------------------
    def _get_pdf_types(self) -> list[tuple[str,str]]:
        d_name_freq = {}

        l_type = []
        for name in self._l_pdf:
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
    def _add_pdf(self, l_pdf : list[zpdf]) -> zpdf:
        nfrc = len(l_pdf)
        if nfrc == 1:
            log.debug('Requested only one PDF, skipping sum')
            return l_pdf[0]

        l_frc= [ zfit.param.Parameter(f'frc_{ifrc + 1}', 0.5, 0, 1) for ifrc in range(nfrc - 1) ]

        pdf = zfit.pdf.SumPDF(l_pdf, name=self._preffix, fracs=l_frc)

        return pdf
    #-----------------------------------------
    def get_pdf(self) -> zpdf:
        '''
        Given a list of strings representing PDFs returns the a zfit PDF which is
        the sum of them
        '''
        l_type=   self._get_pdf_types()
        l_pdf = [ self._get_pdf(kind, preffix) for kind, preffix in l_type ]
        pdf   =   self._add_pdf(l_pdf)

        return pdf
#-----------------------------------------
