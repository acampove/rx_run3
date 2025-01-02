'''
Module containing differential efficiency class
'''
# pylint: disable=import-error, line-too-long

import math
from collections           import UserDict
from dmu.logging.log_store import LogStore

import matplotlib.pyplot as plt

from rx_selection.efficiency import efficiency

#-----------------------------------
class defficiency(UserDict):
    log = LogStore.add_logger('rx_selection:defficiency') 
    #-----------------------------------
    def __init__(self, lab='deff_lab', varname=None):
        self._varname     = varname
        self._label       = lab

        self._epsilon     = 1e-8
        self._total       = None
        self._min_eff     = 1
        self._max_eff     = 0
        self._ntotal      = None
        self._initialized = False
        self._hash        = None

        super().__init__()
    #-----------------------------------
    def _initialize(self):
        if self._initialized:
            return

        if self._varname is None:
            self.log.error(f'Variable name was not specified')
            raise

        self.data = dict(sorted(self.data.items()))

        self._hash = self._get_hash()

        self._initialized = True
    #-----------------------------------
    @staticmethod
    def _check_deff_dict(d_deff):
        if len(d_deff) < 2:
            defficiency.log.error(f'Merging fewer than 2 differential efficiencies')
            raise

        l_var  = [ deff.varname for deff in d_deff ]
        s_var  = set(l_var)
        if len(s_var) != 1:
            defficiency.log.error(f'Differential efficiencies from multiple variables cannot be averaged: {s_var}')
            raise

        l_lab  = [ deff.label for deff in d_deff ]
        s_lab  = set(l_lab)
        if len(s_lab) != 1:
            defficiency.log.error(f'Differential efficiencies with multiple labels cannot be averaged: {s_lab}')
            raise

        l_scl  = list(d_deff.values())
        for scl in l_scl:
            if not isinstance(scl, (int, float)):
                defficiency.log.error(f'At least one of the scales is not numeric: {l_scl}')
                raise
    #-----------------------------------
    @staticmethod
    def average(d_deff):
        defficiency._check_deff_dict(d_deff)
        l_deff_scl = list(d_deff.items())
        l_deff     = list(d_deff.keys())

        deff_z, _  = l_deff_scl[0]
        varname    = deff_z.varname
        label      = deff_z.label
        def_avg    = defficiency(lab=label, varname=varname)

        l_scale    = list(d_deff.values())

        l_s_bound  = [ set(deff.keys()) for deff in d_deff ]
        s_bound_all= set().union(*l_s_bound)
        l_bound_all= list(s_bound_all)

        last_d_eff_scl = None
        for bound in sorted(l_bound_all):
            d_eff_scl = {}
            for deff, scale in zip(l_deff, l_scale):
                if bound not in deff:
                    eff = efficiency((0, 0), arg_fal=(deff.total, deff.ntotal), cut='padded', lab='padded') 
                    defficiency.log.debug(f'No efficiency found at {bound}, padding with:')
                    defficiency.log.debug(eff)
                else:
                    eff = deff[bound]

                d_eff_scl[eff] = scale 

            aeff = efficiency.average(d_eff_scl)
            try:
                def_avg[bound] = aeff
            except:
                defficiency.log.error('Cannot insert efficiency:')
                defficiency.log.error(aeff)
                defficiency.log.error('Into:')
                defficiency.log.error(def_avg)
                defficiency.log.error('Now averaging:')
                for obj in d_eff_scl:
                    defficiency.log.error(obj)
                defficiency.log.error('Before averaging:')
                for obj in last_d_eff_scl:
                    defficiency.log.error(obj)
                raise
            last_d_eff_scl = d_eff_scl

        return def_avg
    #-----------------------------------
    def _get_hash(self):
        hash_val = 0
        for key, val in self.data.items():
            hash_val += hash(key) + hash(val)

        return hash_val
    #-----------------------------------
    def __hash__(self):
        self._initialize()
        if not hasattr(self, '_hash'):
            return self._get_hash() 

        return self._hash 
    #-----------------------------------
    @property
    def varname(self):
        self._initialize()

        return self._varname
    #-----------------------------------
    @property
    def total(self):
        self._initialize()

        return self._total
    #-----------------------------------
    @property
    def ntotal(self):
        self._initialize()

        return self._ntotal
    #-----------------------------------
    @property
    def label(self):
        self._initialize()

        return self._label
    #-----------------------------------
    @property
    def min(self):
        self._initialize()

        return self._min_eff
    #-----------------------------------
    @property
    def max(self):
        self._initialize()

        return self._max_eff
    #-----------------------------------
    def copy(self, label=None):
        self._initialize()

        label   = self._lab     if label   is None else label

        obj = defficiency(lab=label, varname=self._varname)
        for key, eff in self.data.items():
            obj[key] = eff

        return obj
    #-----------------------------------
    def scale_sample(self, scale):
        '''
        Will scale dataset from which efficiencies were calculated 
        '''
        obj = defficiency(lab=self._label, varname=self._varname)

        for key, eff in self.data.items():
            obj[key] = eff.scale_sample(scale)

        return obj
    #-----------------------------------
    def __setitem__(self, bounds, eff):
        '''
        Takes bounds = (minx, maxx) as key and efficiency within those bounds as value
        '''
        tot  = eff.pas  + eff.fal
        ntot = eff.npas + eff.nfal

        if self._total is None:
            self._total  = tot
            self._ntotal = ntot
            super().__setitem__(bounds, eff)
        elif not math.isclose(self._total, tot, rel_tol=self._epsilon):
            self.log.error(f'Total yield for efficiency in {bounds} disagrees with past yields, now/past: {tot:.3f}/{self._total:.3f}')
            raise
        else:
            super().__setitem__(bounds, eff)

        eff_val, _, _ = eff.val

        if eff_val > self._max_eff:
            self._max_eff = eff_val

        if eff_val < self._min_eff:
            self._min_eff = eff_val
    #-----------------------------------
    def __str__(self):
        self._initialize()
        msg = '\n--------------\n'
        msg = f'{self._label}, {self._varname}\n'
        msg+= f'{"Low":<10}{"High":<10}{"Efficiency":<100}\n'
        for (low, hig), val in self.data.items():
            eff_str = val.__str__()
            msg += f'{low:<10.3f}{hig:<10.3f}{eff_str:<100}\n'

        return msg
    #-----------------------------------
    def __rmul__(self, eff_l):
        self._initialize()
        prod = defficiency(lab=self._label, varname=self._varname)
        for bounds, eff_r in self.data.items():
            prod[bounds]   = eff_l * eff_r

        return prod
    #-----------------------------------
    def __eq__(self, other):
        if not isinstance(other, defficiency):
            return NotImplemented

        eq_var = self.varname == other.varname
        eq_dat = self.data    == other.data

        return eq_var and eq_dat
    #-----------------------------------
    def _get_keys(self, deff_1, deff_2):
        s_k_1 = set(deff_1.keys())
        s_k_2 = set(deff_2.keys())
        s_k   = s_k_1.union(s_k_2)

        return s_k
    #-----------------------------------
    def __add__(self, other):
        self._initialize()
        s_key = self._get_keys(self, other)

        res = defficiency(lab=self._label, varname=self._varname)
        for key in s_key:
            #TODO: 
            #If over/under flow is allowed, one of the efficiencies might be zero => pas = 0 total = float => exception
            if   key not in self:
                eff_2    = other[key]
                eff_1    = efficiency((0,0),  (self._total,  self._ntotal), cut=eff_2._cut, lab=eff_2._lab)
            elif key not in other:
                eff_1    = self[key]
                eff_2    = efficiency((0,0), (other._total, other._ntotal), cut=eff_1._cut, lab=eff_1._lab)
            else:
                eff_1    =  self[key]
                eff_2    = other[key]

            res[key] = eff_1 + eff_2

        return res
    #-----------------------------------
    def _remove_over_under_flow(self, l_tbound):
        '''
        Remove (-inf, x) and (y, inf) elements
        '''
        l_res = []
        l_ind = []
        for ind, (vmin, vmax) in enumerate(l_tbound):
            if vmin == -math.inf or vmax == math.inf:
                continue

            l_res.append((vmin, vmax))
            l_ind.append(ind)

        return (l_res, l_ind)
    #-----------------------------------
    def _get_xaxis(self, l_tbound):
        '''
        Takes list of bounds [(min, max)...] 
        returns list of bin centers and assymetric errors
        Removes under and over flows and keeps track of what was kept in l_index
        '''
        l_tbound, l_index = self._remove_over_under_flow(l_tbound)
        l_midle = [ (tbound[0] + tbound[1]) / 2. for tbound in l_tbound]
        l_errup = [ abs(midle - hig) for midle, (  _, hig) in zip(l_midle, l_tbound)]
        l_errdn = [ abs(midle - low) for midle, (low,   _) in zip(l_midle, l_tbound)]

        ncenter = len(l_midle)
        nbound  = len(l_tbound)

        if nbound != ncenter:
            self.log.error(f'Number of boundaries and centers differ:')
            l_tpbnd_str = [ f'({low:<.3e}, {hig:<.3})' for low, hig in l_tbound]
            l_bound_str = [ f'{bound:<.3e}'            for bound    in l_bound ]
            l_midle_str = [ f'{midle:<.3e}'            for midle    in l_midle ]
            l_errup_str = [ f'{errup:<.3e}'            for errup    in l_errup ]
            l_errdn_str = [ f'{errdn:<.3e}'            for errdn    in l_errdn ]

            self.log.error(l_tpbnd_str)
            self.log.error(l_bound_str)
            self.log.error(l_midle_str)
            self.log.error(l_errup_str)
            self.log.error(l_errdn_str)
            raise
    
        return (l_midle, l_errdn, l_errup, l_index)
    #-----------------------------------
    def plot(self, linestyle='-'):
        self._initialize()

        l_tbnd_eff = self.data.items()
        l_tbnd     = [ tbnd        for tbnd, _   in l_tbnd_eff ]
        l_eff      = [ eff.val[0]  for    _, eff in l_tbnd_eff ]

        l_xax, l_errl, l_errh, l_index = self._get_xaxis(l_tbnd)
        l_eff_trim = [ l_eff[index] for index in l_index]

        _, _, bar  = plt.errorbar(l_xax, l_eff_trim, xerr=[l_errl, l_errh], label=self._label, linestyle='None')
        bar[0].set_linestyle([linestyle])

        plt.xlabel(self._varname)
        plt.ylabel('$\\varepsilon$')
    #-----------------------------------
    def to_dict(self):
        self._initialize()
        d_res = {}
        for (low, hig), eff in self.data.items():
            val, eup, edn = eff.val
            err = (eup + edn) / 2.

            key = f'(({low}, {hig}),)'
            val = [[val, err], 0]

            d_res[key] = val

        return d_res
    #-----------------------------------
    def efficiency(self):
        self._initialize()
        pas = 0
        npas= 0
        for _, eff in self.data.items():
            pas  += eff.pas
            npas += eff.npas

        t_pas = (pas        ,         npas)
        t_tot = (self._total, self._ntotal)

        eff = efficiency(t_pas, arg_tot=t_tot, lab=self._label)

        return eff
#-----------------------------------
