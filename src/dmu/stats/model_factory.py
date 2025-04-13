'''
Module storing ZModel class
'''
# pylint: disable=too-many-lines, import-error, too-many-positional-arguments, too-many-arguments

from typing import Callable, Union

import zfit

from zfit.core.interfaces   import ZfitSpace as zobs
from zfit.core.basepdf      import BasePDF   as zpdf
from zfit.core.parameter    import Parameter as zpar
from dmu.stats.zfit_models  import HypExp
from dmu.stats.zfit_models  import ModExp
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

    @classmethod
    def get_pdf_names(cls) -> list[str]:
        '''
        Returns list of PDFs that are registered/supported
        '''
        return list(cls._d_method)
#-----------------------------------------
class ModelFactory:
    '''
    Class used to create Zfit PDFs by passing only the nicknames, e.g.:

    ```python
    from dmu.stats.model_factory import ModelFactory

    l_pdf = ['dscb', 'gauss']
    l_shr = ['mu']
    l_flt = ['mu', 'sg']
    d_rep = {'mu' : 'scale', 'sg' : 'reso'}
    mod   = ModelFactory(preffix = 'signal', obs = obs, l_pdf = l_pdf, l_shared = l_shr, d_rep = d_rep)
    pdf   = mod.get_pdf()
    ```

    where one can specify which parameters

    - Can be shared among the PDFs
    - Are meant to float if this fit is done to MC, in order to fix parameters in data.
    - Are scales or resolutions that need reparametrizations
    '''
    #-----------------------------------------
    def __init__(self,
                 preffix  : str,
                 obs      : zobs,
                 l_pdf    : list[str],
                 l_shared : list[str],
                 l_float  : list[str],
                 d_fix    : dict[str:float] = None,
                 d_rep    : dict[str:str]   = None):
        '''
        preffix:  used to identify PDF, will be used to name every parameter
        obs:      zfit obserbable
        l_pdf:    List of PDF nicknames which are registered below
        l_shared: List of parameter names that are shared
        l_float:  List of parameter names to allow to float
        d_fix:    Dictionary with keys as the beginning of the name of a parameter and value as the number
                  to which it has to be fixed. If not one and only one parameter is found, ValueError is raised
        d_rep:    Dictionary with keys as variables that will be reparametrized
        '''

        self._preffix         = preffix
        self._l_pdf           = l_pdf
        self._l_shr           = l_shared
        self._l_flt           = l_float
        self._d_fix           = d_fix
        self._d_rep           = d_rep
        self._obs             = obs

        self._d_par : dict[str,zpar] = {}

        self._check_reparametrization()
    #-----------------------------------------
    def _check_reparametrization(self) -> None:
        if self._d_rep is None:
            return

        s_par_1 = set(self._d_rep)
        s_par_2 = set(self._l_flt)

        if not s_par_1.isdisjoint(s_par_2):
            raise ValueError('Non empty intersection between floating and reparametrization parameters')

        s_kind  = set(self._d_rep.values())
        if not s_kind.issubset({'scale', 'reso'}):
            raise ValueError(f'Only scales and resolution reparametrizations allowed, found: {s_kind}')
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
    def _get_parameter(
            self,
            name   : str,
            suffix : str,
            val    : float,
            low    : float,
            high   : float) -> zpar:

        par_name = self._get_parameter_name(name, suffix)
        log.debug(f'Assigning name: {par_name}')

        if par_name in self._d_par:
            return self._d_par[par_name]

        is_reparametrized = self._is_reparametrized(name)

        if is_reparametrized:
            init_name, _ = self._split_name(par_name)
            par  = self._get_reparametrization(par_name, init_name, val, low, high)
        else:
            par  = zfit.param.Parameter(par_name, val, low, high)

        self._d_par[par_name] = par

        return par
    #-----------------------------------------
    def _is_reparametrized(self, name : str) -> bool:
        if self._d_rep is None:
            return False

        root_name, _ = self._split_name(name)

        is_rep = root_name in self._d_rep

        log.debug(f'Reparametrizing {name}: {is_rep}')

        return is_rep
    #-----------------------------------------
    def _get_reparametrization(self, par_name : str, init_name : str, value : float, low : float, high : float) -> zpar:
        log.debug(f'Reparametrizing {par_name}')
        par_const = zfit.Parameter(par_name, value, low, high)
        par_const.floating = False

        kind = self._d_rep[init_name]
        if   kind == 'reso':
            par_reso  = zfit.Parameter(f'{par_name}_reso_flt' , 1.0, 0.20, 5.0)
            par       = zfit.ComposedParameter(f'{par_name}_cmp', lambda d_par : d_par['par_const'] * d_par['reso' ], params={'par_const' : par_const, 'reso'  : par_reso } )
        elif kind == 'scale':
            par_scale = zfit.Parameter(f'{par_name}_scale_flt', 0.0, -100, 100)
            par       = zfit.ComposedParameter(f'{par_name}_cmp', lambda d_par : d_par['par_const'] + d_par['scale'], params={'par_const' : par_const, 'scale' : par_scale} )
        else:
            raise ValueError(f'Invalid kind: {kind}')

        return par
    #-----------------------------------------
    @MethodRegistry.register('exp')
    def _get_exponential(self, suffix : str = '') -> zpdf:
        c   = self._get_parameter('c_exp', suffix, -0.010, -0.020, -0.0001)
        pdf = zfit.pdf.Exponential(c, self._obs, name=f'exp{suffix}')

        return pdf
    # ---------------------------------------------
    @MethodRegistry.register('hypexp')
    def _get_hypexp(self, suffix : str = '') -> zpdf:
        mu = zfit.Parameter('mu_hypexp',  5000,   4000,  6000)
        ap = zfit.Parameter('ap_hypexp', 0.020,      0,  0.10)
        bt = zfit.Parameter('bt_hypexp', 0.002, 0.0001, 0.003)

        pdf= HypExp(obs=self._obs, mu=mu, alpha=ap, beta=bt, name=f'hypexp{suffix}')

        return pdf
    # ---------------------------------------------
    @MethodRegistry.register('modexp')
    def _get_modexp(self, suffix : str = '') -> zpdf:
        mu = zfit.Parameter('mu_modexp',  4250,  4250,  4500)
        ap = zfit.Parameter('ap_modexp', 0.002, 0.002, 0.026)
        bt = zfit.Parameter('bt_modexp', 0.002, 0.002, 0.020)

        pdf= ModExp(obs=self._obs, mu=mu, alpha=ap, beta=bt, name=f'modexp{suffix}')

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
        pdf = zfit.pdf.Chebyshev(obs=self._obs, coeffs=[a, b   ], name=f'pol2{suffix}')

        return pdf
    # ---------------------------------------------
    @MethodRegistry.register('pol3')
    def _get_pol3(self, suffix : str = '') -> zpdf:
        a   = zfit.Parameter('a_pol3', -0.005, -0.95, 0.00)
        b   = zfit.Parameter('b_pol3',  0.000, -0.95, 0.95)
        c   = zfit.Parameter('c_pol3',  0.000, -0.95, 0.95)
        pdf = zfit.pdf.Chebyshev(obs=self._obs, coeffs=[a, b, c], name=f'pol3{suffix}')

        return pdf
    #-----------------------------------------
    @MethodRegistry.register('cbr')
    def _get_cbr(self, suffix : str = '') -> zpdf:
        mu  = self._get_parameter('mu_cbr', suffix, 5300, 5100, 5500)
        sg  = self._get_parameter('sg_cbr', suffix,   10,    2,  300)
        ar  = self._get_parameter('ac_cbr', suffix,   -2, -14., -0.1)
        nr  = self._get_parameter('nc_cbr', suffix,    1,  0.5,  150)

        pdf = zfit.pdf.CrystalBall(mu, sg, ar, nr, self._obs, name=f'cbr{suffix}')

        return pdf
    #-----------------------------------------
    @MethodRegistry.register('suj')
    def _get_suj(self, suffix : str = '') -> zpdf:
        mu  = self._get_parameter('mu_suj', suffix, 5300, 5000, 6000)
        sg  = self._get_parameter('sg_suj', suffix,   10,    2, 5000)
        gm  = self._get_parameter('gm_suj', suffix,    1,  -10,   10)
        dl  = self._get_parameter('dl_suj', suffix,    1,  0.1,   40)

        pdf = zfit.pdf.JohnsonSU(mu, sg, gm, dl, self._obs, name=f'suj{suffix}')

        return pdf
    #-----------------------------------------
    @MethodRegistry.register('cbl')
    def _get_cbl(self, suffix : str = '') -> zpdf:
        mu  = self._get_parameter('mu_cbl', suffix, 5300, 5100, 5500)
        sg  = self._get_parameter('sg_cbl', suffix,   10,    2,  300)
        al  = self._get_parameter('ac_cbl', suffix,    2,  0.0,  14.)
        nl  = self._get_parameter('nc_cbl', suffix,    1,  0.5,  150)

        pdf = zfit.pdf.CrystalBall(mu, sg, al, nl, self._obs, name=f'cbl{suffix}')

        return pdf
    #-----------------------------------------
    @MethodRegistry.register('gauss')
    def _get_gauss(self, suffix : str = '') -> zpdf:
        mu  = self._get_parameter('mu_gauss', suffix, 5300, 5100, 5500)
        sg  = self._get_parameter('sg_gauss', suffix,   10,    2,  300)

        pdf = zfit.pdf.Gauss(mu, sg, self._obs, name=f'gauss{suffix}')

        return pdf
    #-----------------------------------------
    @MethodRegistry.register('dscb')
    def _get_dscb(self, suffix : str = '') -> zpdf:
        mu  = self._get_parameter('mu_dscb', suffix, 5300, 5000, 5400)
        sg  = self._get_parameter('sg_dscb', suffix,   10,    2,  500)
        ar  = self._get_parameter('ar_dscb', suffix,    1,    0,    5)
        al  = self._get_parameter('al_dscb', suffix,    1,    0,    5)
        nr  = self._get_parameter('nr_dscb', suffix,    2,    1,  150)
        nl  = self._get_parameter('nl_dscb', suffix,    2,    0,  150)

        pdf = zfit.pdf.DoubleCB(mu, sg, al, nl, ar, nr, self._obs, name=f'dscb{suffix}')

        return pdf
    #-----------------------------------------
    @MethodRegistry.register('voigt')
    def _get_voigt(self, suffix : str = '') -> zpdf:
        mu  = zfit.Parameter('mu_voigt', 5280,  5040, 5500)
        sg  = zfit.Parameter('sg_voigt',   20,    10,  400)
        gm  = zfit.Parameter('gm_voigt',    4,   0.1,  100)

        pdf = zfit.pdf.Voigt(m=mu, sigma=sg, gamma=gm, obs=self._obs, name=f'voigt{suffix}')

        return pdf
    #-----------------------------------------
    @MethodRegistry.register('qgauss')
    def _get_qgauss(self, suffix : str = '') -> zpdf:
        mu  = zfit.Parameter('mu_qgauss', 5280,  5040, 5500)
        sg  = zfit.Parameter('sg_qgauss',   20,    10,  400)
        q   = zfit.Parameter( 'q_qgauss',    1,     1,    3)

        pdf = zfit.pdf.QGauss(q=q, mu=mu, sigma=sg, obs=self._obs, name =f'qgauss{suffix}')

        return pdf
    #-----------------------------------------
    @MethodRegistry.register('cauchy')
    def _get_cauchy(self, suffix : str = '') -> zpdf:
        mu  = zfit.Parameter('mu', 5280,  5040, 5500)
        gm  = zfit.Parameter('gm',  150,    50,  500)

        pdf = zfit.pdf.Cauchy(obs=self._obs, m=mu, gamma=gm, name=f'cauchy{suffix}')

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

        l_frc= [ zfit.param.Parameter(f'frc_{self._preffix}_{ifrc + 1}', 0.5, 0, 1) for ifrc in range(nfrc - 1) ]

        pdf = zfit.pdf.SumPDF(l_pdf, name=self._preffix, fracs=l_frc)

        return pdf
    #-----------------------------------------
    def _find_par(self, s_par : set[zpar], name_start : str) -> zpar:
        l_par_match = [ par for par in s_par if par.name.startswith(name_start) ]

        if len(l_par_match) != 1:
            for par in s_par:
                log.info(par.name)

            raise ValueError(f'Not found one and only one parameter starting with: {name_start}')

        return l_par_match[0]
    #-----------------------------------------
    def _fix_parameters(self, pdf : zpdf) -> zpdf:
        if self._d_fix is None:
            log.debug('Not fixing any parameter')
            return pdf

        s_par = pdf.get_params()

        log.info('-' * 30)
        log.info('Fixing parameters')
        log.info('-' * 30)
        for name_start, value in self._d_fix.items():
            par = self._find_par(s_par, name_start)
            par.set_value(value)

            log.info(f'{name_start:<20}{value:<20.3f}')
            par.floating = False

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
        pdf   =   self._fix_parameters(pdf)

        return pdf
#-----------------------------------------
