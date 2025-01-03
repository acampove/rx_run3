'''
Module holding cutflow and cutflow manager classes
'''
# pylint: disable=import-error, line-too-long

from collections import UserDict

import pandas       as pnd

from rx_selection.differential_efficiency import defficiency
from rx_selection.ndict                   import ndict
from dmu.logging.log_store      import LogStore

from rx_selection.efficiency    import efficiency
from rx_selection               import utilities as ut

#-----------------------------------------
class cutflow(UserDict):
    log=LogStore.add_logger('rx_selection:cutflow')
    #-------------------------------
    def __init__(self, d_meta=None):
        self._d_meta      = d_meta
        self._tot_eff     = 1.
        self._ful_eff     : efficiency

        self._df_stat     : pnd.DataFrame
        self._df_cuts     : pnd.DataFrame
        self._hash        = None

        self._initialized = False

        super().__init__()
    #-------------------------------
    def __setitem__(self, cut, obj):
        if not isinstance(obj, (efficiency, defficiency)):
            raise ValueError(f'For cut {cut}, value has to be efficiency or differential efficiency, found: {type(obj)}')

        self.data[cut] = obj

        if not hasattr(self, '_ful_eff'):
            self._ful_eff  = obj.copy()
        else:
            self._ful_eff  = self._ful_eff * obj
    #-------------------------------
    def _initialize(self):
        if self._initialized:
            return

        d_cuts = {}
        d_stat = {}
        ful_eff= None
        for label, obj in self.data.items():
            if isinstance(obj, defficiency):
                eff = obj.efficiency()
            else:
                eff = obj

            ful_eff       = eff if ful_eff is None else ful_eff * eff
            eff_val       = eff.val[0]
            self._tot_eff = ful_eff.val[0]
            d_cuts[label] = eff.cut

            ut.add_to_dic_lst(d_stat, 'Total'     , eff.fal + eff.pas)
            ut.add_to_dic_lst(d_stat, 'Pased'     ,           eff.pas)
            ut.add_to_dic_lst(d_stat, 'Efficiency',           eff_val)
            ut.add_to_dic_lst(d_stat, 'Cumulative',     self._tot_eff)
            ut.add_to_dic_lst(d_stat, 'Cut'       ,             label)

        self._df_stat=pnd.DataFrame(d_stat, columns=['Cut', 'Total', 'Pased', 'Efficiency', 'Cumulative'])
        self._df_stat=self._df_stat.set_index('Cut')

        self._df_cuts=pnd.DataFrame(d_cuts, index=['Cut'])
        self._df_cuts=self._df_cuts.T

        self._hash = self._get_hash()

        self._initialized = True
    #-------------------------------
    def _get_hash(self):
        hash_val = 0

        for lab, eff in self.data.items():
            hash_val += hash(lab) + hash(eff)

        return hash_val
    #-------------------------------
    def __eq__(self, other):
        if not isinstance(other, cutflow):
            return NotImplemented

        return self.data == other.data
    #-------------------------------
    def __hash__(self):
        self._initialize()
        if not hasattr(self, '_hash'):
            return self._get_hash()

        if not isinstance(self._hash, int):
            self.log.error(f'Hash attribute is not an int: {self._hash}')
            raise ValueError

        return self._hash
    #-------------------------------
    @staticmethod
    def _check_cfl_dict(d_cfl):
        l_cut_last = None
        for cfl, scale in d_cfl.items():
            if not isinstance(cfl, cutflow):
                cutflow.log.error(f'Key is not a cutflow but: {type(cfl)}')
                raise ValueError

            if not isinstance(scale, (int, float)):
                cutflow.log.error(f'Value is not numeric but: {type(scale)}')
                raise ValueError

            l_cut_this = list(cfl.keys())
            if l_cut_last is not None:
                if l_cut_last != l_cut_this:
                    cutflow.log.error(l_cut_last)
                    cutflow.log.error(l_cut_this)
                    raise ValueError('Cutflows contain different cuts:')

            l_cut_last = l_cut_this
    #-------------------------------
    @staticmethod
    def _get_eff_type(d_eff):
        l_type = [ key.__class__.__name__ for key in d_eff ]
        s_type = set(l_type)
        l_type = list(s_type)
        if len(l_type) != 1:
            raise ValueError(f'Not one and only one type found: {l_type}')

        return l_type[0]
    #-------------------------------
    @staticmethod
    def average(d_cfl):
        cutflow._check_cfl_dict(d_cfl)
        l_cfl = list(d_cfl.keys())
        l_scl = list(d_cfl.values())
        l_cut = list(l_cfl[0].keys())

        cfl_avg = cutflow()
        for cut in l_cut:
            d_eff        = { cfl[cut] : scl for cfl, scl in zip(l_cfl, l_scl) }
            type_eff     = cutflow._get_eff_type(d_eff)
            if   type_eff == 'efficiency':
                eff          =  efficiency.average(d_eff)
            elif type_eff == 'defficiency':
                eff          = defficiency.average(d_eff)
            else:
                cutflow.log.error(f'Invalid kind of efficiency: {type_eff}')
                raise TypeError

            cfl_avg[cut] = eff

        return cfl_avg
    #-------------------------------
    @property
    def df_eff(self):
        self._initialize()

        return self._df_stat
    #-------------------------------
    @property
    def df_cut(self):
        self._initialize()

        return self._df_cuts
    #-------------------------------
    @property
    def tot_eff(self):
        '''
        Returns numerical value of total efficiency
        '''
        self._initialize()

        return self._tot_eff
    #-------------------------------
    @property
    def efficiency(self):
        '''
        Returns efficiency object, product of all efficiencies
        '''
        self._initialize()

        return self._ful_eff
    #-------------------------------
    def __str__(self):
        self._initialize()

        msg = 40 * '-' + '\n'
        msg+= f'{"Kind":<20}{"Passed":>10} [{"Entries":>10}] / {"Total":>10} [{"Entries":>10}] = {"Eff":<9} | {"Cut":<40}{"Label":>20}\n \n'
        for kind, obj in self.items():
            if isinstance(obj, defficiency):
                eff = obj.efficiency()
            else:
                eff = obj

            eff_str = eff.__str__()

            msg += f'{kind:<20}{eff_str:<50}\n'


        if isinstance(self._ful_eff, defficiency):
            eff = self._ful_eff.efficiency()
        else:
            eff = self._ful_eff

        msg += '-----\n'
        msg += f'{"Total":<20}{eff.__str__():<50}\n'
        msg += 40 * '-' + '\n'

        return msg
    #-------------------------------
    def to_json(self, path : str) -> None:
        '''
        Will save cutflow to JSON
        '''
        d_eff = self.df_eff.to_dict()
        d_cut = self.df_cut.to_dict()

        d_eff.update(d_cut)

        if self._d_meta is not None:
            d_eff.update(self._d_meta)

        ut.dump_json(d_eff, path)
    #-------------------------------
    def __add__(self, other):
        self._initialize()

        if self.keys() != other.keys():
            print(self.df_eff)
            print(other.df_eff)
            raise ValueError('Cannot add cutflows with different cuts:')

        res_cfl = cutflow()

        for key in other:
            other_eff = other[key]
            this_eff  =  self[key]

            eff = other_eff + this_eff

            res_cfl[key] = eff

        return res_cfl
#-----------------------------------------
class cutflow_manager():
    '''
    Class used to build cutflow objects. It takes care of switching between efficiencies, depending on the systematics
    '''
    log=LogStore.add_logger('rx_selection:cutflow:cutflow_manager')
    #----------------------------------
    def __init__(self):
        self._d_d_eff   = {}
        self._s_sys     = set()
        self._l_cut     = []
        self._has_dif   = False
        self._s_dif_var : set
    #----------------------------------
    def _check_nominal(self, d_eff, kind):
        '''
        Check if dictionary contains nominal efficiency
        '''
        if   isinstance(d_eff,  dict)                 and 'nom' not in d_eff:
            print(d_eff.keys())
            raise ValueError(f'Nominal efficiency not found for: {kind}')

        if isinstance(d_eff, ndict) and not d_eff.has_val('nom', axis='x'):
            print(d_eff)
            raise ValueError(f'Nominal efficiency not found for: {kind}')
    #----------------------------------
    def __setitem__(self, cut, d_eff):
        self._check_nominal(d_eff, cut)
        self._check_sys_lab(d_eff, cut)

        if cut in self._l_cut:
            raise ValueError(f'Kind {cut} already added')

        self._l_cut.append(cut)

        if   isinstance(d_eff, ndict) and not self._has_dif:
            self._has_dif   = True
            self._s_dif_var = d_eff.y_axis
            self._s_sys     = d_eff.x_axis.union(self._s_sys)
        elif isinstance(d_eff,  dict):
            self._s_sys= set(d_eff.keys()).union(self._s_sys)
        elif isinstance(d_eff, ndict) and     self._has_dif:
            raise ValueError('Cannot pass multiple differential efficiencies')
        else:
            raise ValueError(f'Argument is neither dict nor ndict, but: {type(d_eff)}')

        self._d_d_eff[cut] = d_eff
    #----------------------------------
    def _pad_eff_int(self, d_eff):
        '''
        Takes {sys:eff}, pads with nominal missing sistematics
        '''
        eff_nom = d_eff['nom']

        for sys in self._s_sys:
            if sys in d_eff:
                continue

            d_eff[sys] = eff_nom.copy(label=sys)

        return d_eff
    #----------------------------------
    def _pad_eff_dif(self, d_eff):
        for var in d_eff.y_axis:
            nom_eff = d_eff['nom', var]
            for sys in self._s_sys:
                if (sys, var) not in d_eff:
                    d_eff[sys, var] = nom_eff.copy(label=sys, varname=var)

        return d_eff
    #----------------------------------
    def _pad_all(self):
        '''
        Will pad with nominal (cut, syst) locations for systematics that do not make sense for given cut.
        '''
        d_d_eff = {}
        for cut, d_eff in self._d_d_eff.items():
            if   isinstance(d_eff,  dict):
                d_d_eff[cut] = self._pad_eff_int(d_eff)
            elif isinstance(d_eff, ndict):
                d_d_eff[cut] = self._pad_eff_dif(d_eff)
            else:
                raise ValueError(f'Object is not a dict or ndict, but: {type(d_eff)}')

        return d_d_eff
    #----------------------------------
    def _check_sys_lab(self, d_eff, cut):
        for key, eff in d_eff.items():
            try:
                sys, _ = key
            except ValueError:
                sys    = key

            if sys != eff.label:
                print(eff)
                raise ValueError(f'For cut {cut} systematic and efficiency label dissagree: {sys}/{eff.label}')
    #----------------------------------
    def _get_cf_int(self, sys, d_d_eff_pad):
        '''
        Takes sys string and {cut : {sys : eff...}...} and for given systematic returns cutflow object
        '''
        cf = cutflow()
        for cut in self._l_cut:
            d_eff   = d_d_eff_pad[cut]
            eff     = d_eff[sys]
            cf[cut] = eff

        return cf
    #----------------------------------
    def _get_cf_dif(self, sys, var, d_d_eff_pad):
        '''
        Takes sys, var strings and {cut : {sys[,var] : [d]eff...}...},
        i.e. inner dict (with sys -> eff) or ndict (with sys, var -> deff)

        Returns cutflow for given sys, var combination.
        '''
        cf = cutflow()
        for cut in self._l_cut:
            d_eff   = d_d_eff_pad[cut]
            if   isinstance(d_eff , dict):
                eff = d_eff[sys]
            elif isinstance(d_eff, ndict):
                eff = d_eff[sys, var]
            else:
                raise ValueError('Dictionary off efficiencies is neither dict nor ndict')

            cf[cut] = eff

        return cf
    #----------------------------------
    def get_cf(self):
        '''
        Returns either {sys : cutflow} dict or {sys, var : cutflow} ndict

        Latter is returned if one of the efficiencies is differential
        '''
        d_d_eff_pad = self._pad_all()

        d_cf = ndict() if self._has_dif else {}

        self.log.info('Creating cutflows:')
        for sys in self._s_sys:
            self.log.info(sys)
            if not self._has_dif:
                d_cf[sys]          = self._get_cf_int(sys,      d_d_eff_pad)
            else:
                for var in self._s_dif_var:
                    d_cf[sys, var] = self._get_cf_dif(sys, var, d_d_eff_pad)

        return d_cf
#----------------------------------
