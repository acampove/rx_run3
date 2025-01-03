'''
Module holding efficiency class and helper functions
'''
# pylint: disable=import-error, line-too-long

import math

import ROOT
import numpy
import jacobi   as jac

from dmu.logging.log_store  import LogStore

#-----------------------------------------
class InconsistentEff(Exception):
    '''
    When multiplying efficiencies, numerator of first one has to be equal to denominator of second one
    '''
    log=LogStore.add_logger('rx_selection:InconsistentEff')
    def __init__(self, eff_1, eff_2):
        self.message = 'Inconsistent efficiencies'
        super().__init__(self.message)
        self.log.error(f'{eff_1._npas} != {eff_2._npas} + {eff_2._nfal}')
        print(eff_1)
        print(eff_2)
#-----------------------------------------
class DifferentLabel(Exception):
    '''
    When adding two efficiencies, labels (symbolizing sample and selection) need to be the same
    '''
    log=LogStore.add_logger('rx_selection:DifferentLabel')
    def __init__(self, eff_1, eff_2):
        self.message = 'Different labels'
        super().__init__(self.message)
        self.log.error(f'{eff_1.label} != {eff_2.label}')
        self.log.error(eff_1)
        self.log.error(eff_2)
#-----------------------------------------
class ZeroYields(Exception):
    '''
    Numerator and denominator cannot be zero in an efficiency
    '''
    def __init__(self, message):
        self.message = message
        super().__init__(self.message)
#-----------------------------------------
class efficiency:
    log=LogStore.add_logger('rx_selection:efficiency')
    """
    Class meant to represent efficiencies
    """
    def __init__(self,
                 arg_pas,
                 arg_fal=None,
                 arg_tot=None,
                 err_pas : float = 0.,
                 err_fal : float = 0.,
                 cut     : str   ='no-cut',
                 lab     : str   ='none'):
        '''
        arg_pas: int, tuple, ndarray
        arg_fal: int, tuple, ndarray
        arg_tot: int, tuple, ndarray

        tuple = (yield, frequency)
        '''
        pas, fal, npas, nfal = self._get_yields(arg_pas, arg_fal=arg_fal, arg_tot=arg_tot)

        self.log.debug(f'pas: {pas}')
        self.log.debug(f'fal: {fal}')

        if pas + fal == 0:
            raise ZeroYields('Both passed and failed yields are zero')

        if pas == 0:
            self.log.debug('Passed yield is zero')

        self._pas  = pas
        self._fal  = fal
        self._npas = npas
        self._nfal = nfal

        self._cut = cut
        self._lab = lab
        self._lvl = 0.682689

        self._epsilon = 1e-7
        self._err_pas = err_pas
        self._err_fal = err_fal

        self._tp_hash = (self._pas, self._fal, self._npas, self._nfal, self._cut, self._lab)
    #------------------------------------
    def _get_yields(self, arg_pas, arg_fal=None, arg_tot=None):
        if   isinstance(arg_pas,           int) and isinstance(arg_fal,           int):
            self.log.debug('pas/fal args')

            pas  = arg_pas
            fal  = arg_fal

            npas = pas
            nfal = fal
        elif isinstance(arg_pas,           int) and isinstance(arg_tot,           int):
            self.log.debug('pas/tot args')
            pas  = arg_pas
            fal  = arg_tot - arg_pas

            npas = pas
            nfal = fal
        elif isinstance(arg_pas,         tuple) and isinstance(arg_fal,         tuple):
            self.log.debug('tup_pas/tup_fal args')
            pas, npas = arg_pas
            fal, nfal = arg_fal
        elif isinstance(arg_pas,         tuple) and isinstance(arg_tot,         tuple):
            self.log.debug('tup_pas/tup_tot args')
            pas, npas = arg_pas
            tot, ntot = arg_tot

            fal       = tot  - pas
            nfal      = ntot - npas
        elif isinstance(arg_pas, numpy.ndarray) and isinstance(arg_fal, numpy.ndarray):
            self.log.debug('arr_pas/arr_fal args')
            pas  = numpy.sum(arg_pas)
            fal  = numpy.sum(arg_fal)

            npas = len(arg_pas)
            nfal = self._get_nfal(arg_fal)
        elif isinstance(arg_pas, numpy.ndarray) and isinstance(arg_tot, numpy.ndarray):
            self.log.debug('arr_pas/arr_tot args')
            pas  = numpy.sum(arg_pas)
            fal  = numpy.sum(arg_tot) - pas

            npas = len(arg_pas)
            nfal = len(arg_tot) - npas
        else:
            self.log.error(f'Passed: {arg_pas}')
            self.log.error(f'Failed: {arg_fal}')
            self.log.error(f'Total:  {arg_tot}')
            raise ValueError('Pass, fail and or total arguments are neither integers nor arrays')

        return pas, fal, npas, nfal
    #------------------------------------
    def _get_nfal(self, arr_fal):
        nfal = len(arr_fal)
        if nfal == 0:
            return nfal

        #Needed if a cut has an above 1 efficiency, i.e.:
        #negative failed yield == negative number of failed events
        if numpy.all(arr_fal == arr_fal[0]) and numpy.all(arr_fal < 0):
            nfal = -nfal

        return nfal
    #------------------------------------
    def __lt__(self, other):
        this_tot  = self._pas  + self._fal
        other_tot = other._pas + other._fal

        if this_tot == other_tot:
            return self._pas < other._pas

        return this_tot < other_tot
    #------------------------------------
    @property
    def npas(self):
        return self._npas
    #------------------------------------
    @property
    def nfal(self):
        return self._nfal
    #------------------------------------
    @property
    def pas(self):
        return self._pas
    #------------------------------------
    @property
    def fal(self):
        return self._fal
    #------------------------------------
    @property
    def cut(self):
        return self._cut
    #------------------------------------
    @property
    def label(self):
        return self._lab
    #------------------------------------
    @label.setter
    def label(self, label):
        self._lab = label
    #------------------------------------
    @property
    def val(self):
        '''
        Returns tuple (efficiency, error_up, error_down)
        '''

        if self._err_pas == 0 and self._err_fal == 0:
            val = self._get_val_noerror()
        else:
            eff, var = jac.propagate(lambda x : x[0] / ( x[0] + x[1] ), [self._pas, self._fal], [[self._err_pas ** 2, 0], [0, self._err_fal ** 2]])
            err      = math.sqrt(var)
            val      = eff, err, err

        return val
    #------------------------------------
    def copy(self, label :str = '', cut : str = ''):
        '''
        Function that optionally (to override) takes a label and cut and makes a copy of current efficiency
        '''
        label = self._lab if label == '' else label
        cut   = self._cut if cut   == '' else cut

        return efficiency((self._pas, self._npas), arg_fal = (self._fal, self._nfal), cut=cut, lab=label)
    #------------------------------------
    def _check_precision(self, eff, err_dn, err_up):
        prec_dn = 100 * err_dn/eff if eff > 0 else -1
        prec_up = 100 * err_up/eff if eff > 0 else -1

        if prec_dn > 10 or prec_dn < 0:
            self.log.debug(f'Precision for error down is {prec_dn:.3e}% of efficiency {eff:.3e}')
            self.log.debug(f'Eff central:{eff:.3e}   ')
            self.log.debug(f'Eff down:   {err_dn:.3e}')

        if prec_up > 10 or prec_dn < 0:
            self.log.debug(f'Precision for error down is {prec_up:.3e}% of efficiency {eff:.3e}')
            self.log.debug(f'Eff central:{eff:.3e}   ')
            self.log.debug(f'Eff up:     {err_up:.3e}')
    #------------------------------------
    def _get_val_noerror(self):
        total  = self._pas + self._fal
        eff_cn = self._pas / total

        if eff_cn > 1:
            return (eff_cn, 0, 0)

        eff_up = ROOT.TEfficiency.ClopperPearson(total, self._pas, self._lvl,  True)
        eff_dn = ROOT.TEfficiency.ClopperPearson(total, self._pas, self._lvl, False)

        err_up = eff_up - eff_cn
        err_dn = eff_cn - eff_dn

        self._check_precision(eff_cn, err_dn, err_up)

        return eff_cn, err_dn, err_up
    #------------------------------------
    def show(self):
        '''
        Prints current efficiency object, yields and efficiencies with errors
        '''
        eff_cn, err_dn, err_up = self.val

        tot = self._pas + self._fal

        self.log.info( '------------------------------------------------')
        self.log.info(f'{"Cut"   :<20}{self.cut :<20}')
        self.log.info(f'{"Pased" :<20}{self._pas:<20}')
        self.log.info(f'{"Failed":<20}{self._fal:<20}')
        self.log.info(f'{"Total" :<20}{tot      :<20}')
        self.log.info('------------------------------------------------')
        self.log.info(f'{"Cut":<20}{self.cut:<20}   ')
        self.log.info(f'{"Eff":<20}{eff_cn  :<20.3e}')
        self.log.info(f'{"Eup":<20}{err_up  :<20.3e}')
        self.log.info(f'{"Edn":<20}{err_dn  :<20.3e}')
        self.log.info('------------------------------------------------')
    #------------------------------------
    def __add__(self, other):
        """
        Takes efficiency object and returns efficiency object
        resulting from combining corresponding sample, with current
        object's sample
        """
        if not isinstance(other, efficiency):
            return NotImplemented

        #----------------------------------------------------------
        if   self.label != 'padded' and other.label == 'padded':
            cut = self.cut
            lab = self.label
        elif self.label == 'padded' and other.label != 'padded':
            cut = other.cut
            lab = other.label
        elif self.label == 'padded' and other.label == 'padded':
            raise ValueError('Cannot add two padded efficiencies')
        #----------------------------------------------------------
        elif (other.cut != self.cut) or (other.label != self.label):
            self.log.error(f'This :  {self.cut}/{self.label}')
            self.log.error(f'Other: {other.cut}/{other.label}')
            raise ValueError('Cannot add cuts/labels, cut strings differ:')
        else:
            cut = self.cut
            lab = self.label

        pas  = other._pas  + self._pas
        fal  = other._fal  + self._fal
        tot  =        pas  +       fal

        npas = other._npas + self._npas
        nfal = other._nfal + self._nfal
        ntot =        npas +       nfal

        err_pas   = math.sqrt(other._err_pas ** 2 + self._err_pas ** 2)
        err_fal   = math.sqrt(other._err_fal ** 2 + self._err_fal ** 2)

        tp_pas    = (pas, npas)
        tp_tot    = (tot, ntot)
        obj       = efficiency(tp_pas, arg_tot=tp_tot, err_pas=err_pas, err_fal=err_fal, cut=cut, lab=lab)

        return obj
    #------------------------------------
    @staticmethod
    def _check_eff_dic(d_eff):
        for eff, scl in d_eff.items():
            if not isinstance(eff, efficiency):
                efficiency.log.error(f'Key of dictionary is not an efficiency but: {type(eff)}')
                raise ValueError

            if not isinstance(scl, (float, int)):
                efficiency.log.error(f'Value of dictionary is not numeric but: {type(scl)}')
                raise ValueError
    #------------------------------------
    @staticmethod
    def _normalize_scales(d_eff):
        l_scale = list(d_eff.values())
        nscale  = len(l_scale)
        total   = sum(l_scale)
        l_wgt   = [ scale * nscale / total for scale in l_scale ]

        return l_wgt
    #------------------------------------
    def _scale_sample(self, other):
        '''
        Will modify the passed and failed yields for a sample that is `other` times
        larger. Central value of efficiency should stay the same, errors are scaled too.
        '''
        if not isinstance(other, float):
            raise NotImplementedError

        pas       = self._pas * other
        fal       = self._fal * other
        tot       = pas  +       fal

        npas      = self._npas
        nfal      = self._nfal
        ntot      = npas +       nfal

        err_pas   = self._err_pas
        err_fal   = self._err_fal

        tp_pas    = (pas, npas)
        tp_tot    = (tot, ntot)
        obj       = efficiency(tp_pas, arg_tot=tp_tot, err_pas=err_pas, err_fal=err_fal, cut=self._cut, lab=self._lab)

        return obj
    #------------------------------------
    def __hash__(self):
        if not hasattr(self, '_tp_hash'):
            return hash((self._pas, self._fal, self._npas, self._nfal, self._cut))

        return hash(self._tp_hash)
    #------------------------------------
    def __mul__(self, other):
        """
        Returns product of efficiency objects, current object and argument object
        """

        if not isinstance(other, efficiency):
            return NotImplemented

        is_consistent = math.isclose(self._npas, other._npas + other._nfal, rel_tol=10e-6)
        same_label    = self.label == other.label

        if is_consistent is False:
            raise InconsistentEff(self, other)

        if same_label    is False:
            raise DifferentLabel(self, other)

        err_fal   = math.sqrt(other._err_fal ** 2 + self._err_fal ** 2)
        cut       = f'({self.cut}) && ({other.cut})'
        lab       = self.label

        tp_pas    = (           other._pas,             other._npas)
        tp_tot    = (self._pas + self._fal, self._npas + self._nfal)
        obj       = efficiency(tp_pas, arg_tot = tp_tot, err_pas = other._err_pas, err_fal = err_fal, cut = cut, lab=lab)

        return obj
    #------------------------------------
    def __truediv__(self, other):
        '''
        Takes numerical value of efficiency, returns ratio of this efficiency by other efficiency
        '''
        if not isinstance(other, float):
            return NotImplemented

        eff_this , _, _ =  self.val
        eff_other, _, _ = other.val

        if eff_other == 0:
            raise ValueError('Denominator efficiency is zero')

        return eff_this / eff_other
    #------------------------------------
    def __eq__(self, other):
        if not isinstance(other, efficiency):
            return NotImplemented

        equal_yields = math.isclose(self._pas, other._pas, rel_tol=self._epsilon) and math.isclose(self._fal, other._fal, rel_tol=self._epsilon)
        equal_stats  = (self._npas == other._npas) and (self._nfal == other._nfal)
        equal_cuts   =  self.cut == other.cut

        return equal_yields and equal_cuts and equal_stats
    #------------------------------------
    def __str__(self):
        obj  = self
        tot  = obj._pas  + obj._fal
        ntot = obj._npas + obj._nfal

        eff, edn, eup  = self.val
        line = f'{obj._pas:10.3e} [{obj._npas:10.3e}] / {tot:10.3e} [{ntot:10.3e}] = {eff:<.3e}+{eup:<.3e}-{edn:<.3e} | {obj._cut:<40}{obj._lab:<20}'

        return line
#-----------------------------------------
