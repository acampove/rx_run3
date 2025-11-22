'''
Module holding AcceptanceCalculator class
'''

import os
import pprint
import matplotlib.pyplot as plt

from dmu import LogStore

log = LogStore.add_logger('prec_acceptances::calculator')
#-----------------------------
class AcceptanceCalculator:
    '''
    Class meant to calculate geometric acceptances from a Tree created by RapidSim in the 4pi
    acceptance.
    '''
    #-----------------------------
    def __init__(self, rdf):
        self._rdf         = rdf
        self._plot_dir    : str
        self._l_all_trk   : list[str]

        self._initialized = False
    #-----------------------------
    def _initialize(self):
        if self._initialized:
            return

        self._l_all_trk = self._get_all_tracks()
        for track in self._l_all_trk:
            self._rdf = self._rdf.Define(f'{track}_et', f'{track}_eta_TRUE')
            self._rdf = self._rdf.Define(f'{track}_th', f'2 * TMath::ATan( TMath::Exp(-{track}_et) )')
            self._rdf = self._rdf.Define(f'{track}_in', f'({track}_th > 0.010) && ({track}_th < 0.400)')

            self._plot_split(self._rdf, f'{track}_th', f'{track}_in')
            self._plot(self._rdf, f'{track}_et')

        self._initialized = True
    #-----------------------------
    @property
    def plot_dir(self) -> str:
        '''
        Returns path to directory where validation plots go
        '''
        return self._plot_dir

    @plot_dir.setter
    def plot_dir(self, value) -> None:
        try:
            os.makedirs(value, exist_ok=True)
        except:
            log.error(f'Cannot make: {value}')
            raise

        self._plot_dir = value
    #-----------------------------
    def _plot_split(self, rdf, theta, flag) -> None:
        rdf_in = rdf.Filter(f'{flag} == 1')
        rdf_ot = rdf.Filter(f'{flag} == 0')

        arr_in = rdf_in.AsNumpy([theta])[theta]
        arr_ot = rdf_ot.AsNumpy([theta])[theta]

        plt.hist(arr_in, bins=100, range=(0, 3.1415), alpha=0.7, color='r', label='In')
        plt.hist(arr_ot, bins=100, range=(0, 3.1415), alpha=0.7, color='b', label='Out')
        plt.legend()
        plt.ylabel('Entries')
        plt.xlabel(r'$\theta$[rad]')

        plot_path=f'{self._plot_dir}/{theta}.png'
        log.info(f'Saving to: {plot_path}')
        plt.savefig(plot_path)
        plt.close('all')
    #-----------------------------
    def _plot(self, rdf, var) -> None:
        arr_var = rdf.AsNumpy([var])[var]

        plt.hist(arr_var, bins=100, range=(-6, +6), alpha=0.7, color='b', label='Out')
        plt.ylabel('Entries')
        plt.xlabel(r'$\eta$')

        plt.savefig(f'{self._plot_dir}/{var}.png')
        plt.close('all')
    #-----------------------------
    def _get_all_tracks(self) -> list[str]:
        v_col = self._rdf.GetColumnNames()
        l_col = [ col.c_str() for col in v_col ]
        l_trk = [ trk         for trk in l_col if trk.endswith('_eta_TRUE')]
        l_trk = [ trk.replace('_eta_TRUE', '') for trk in l_trk ]

        log.info('Found following tracks:')
        pprint.pprint(l_trk)

        return l_trk
    #-----------------------------
    def _get_numerators(self) -> tuple[int,int]:
        rdf          = self._rdf
        l_all_trk_in = [ f'{trk}_in' for trk in self._l_all_trk ]
        all_in       = '&&'.join(l_all_trk_in)

        rdf = rdf.Define('is_lhc', all_in)
        if 'Km_0' in self._l_all_trk:
            rdf = rdf.Define('is_phy', 'em_0_in && ep_0_in && (Kp_0_in || Km_0_in)')
        else:
            rdf = rdf.Define('is_phy', 'em_0_in && ep_0_in && Kp_0_in')

        nlhc = rdf.Sum('is_lhc').GetValue()
        nphy = rdf.Sum('is_phy').GetValue()

        return nphy, nlhc
    #-----------------------------
    def get_acceptances(self) -> tuple[float,float]:
        '''
        Will return tuple with acceptances:

        physical: That contributes to the PRec of RK
        LHCb: Where all tracks are inside the LHCb acceptance
        '''
        self._initialize()
        ntot       = self._rdf.Count().GetValue()
        ntot       = float(ntot)
        nphy, nlhc = self._get_numerators()

        eff_phy, eff_lhc = (nphy/ntot), (nlhc/ntot)

        log.info(f'Physical: {eff_phy:.3f}')
        log.info(f'LHCb:     {eff_lhc:.3f}')

        return eff_phy, eff_lhc
#-----------------------------
