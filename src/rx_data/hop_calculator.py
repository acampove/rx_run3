'''
Module containing HOPVarCalculator class
'''
import math
from typing import Union

import numpy
from ROOT                   import RDataFrame, RDF  # type: ignore
from ROOT.Math              import LorentzVector, XYZVector # type: ignore
from dmu.logging.log_store  import LogStore
from rx_common              import info

log = LogStore.add_logger('rx_data:hop_calculator')

Vector=Union[LorentzVector, XYZVector]
# -------------------------------
class HOPCalculator:
    '''
    Class meant to calculate HOP variables from a ROOT dataframe. For info on HOP see:

    https://cds.cern.ch/record/2102345/files/LHCb-INT-2015-037.pdf
    '''
    # -------------------------------
    def __init__(
        self, 
        rdf    : RDataFrame,
        trigger: str):
        '''
        Parameters
        -----------------
        rdf: ROOT dataframe with input needed to calculate HOP varibles
        trigger: HLT2 trigger name, needed to decide how to add vectors
        '''

        self._rdf           = rdf
        self._trigger       = trigger
        self._extra_branches= ['EVENTNUMBER', 'RUNNUMBER']
    # -------------------------------
    def _val_to_vector(self, arr_val : numpy.ndarray, ndim : int) -> Vector:
        if   ndim == 4:
            [px, py, pz, pe] = arr_val.tolist()
            vector = LorentzVector('ROOT::Math::PxPyPzE4D<double>')(px, py, pz, pe)
        elif ndim == 3:
            [px, py, pz    ] = arr_val.tolist()
            vector = XYZVector(px, py, pz)
        else:
            raise NotImplementedError(f'Invalid dimentionality: {ndim}')

        return vector
    # -------------------------------
    def _get_xvector(self, name : str, ndim : int) -> list[Vector]:
        '''
        Parameters
        -----------------
        name: Identifier of the object, i.e L1_P, Hadrons, etc.
        ndim: Number of dimensions for associated vector

        Returns
        -----------------
        List of vectors in dataframe, associated to object
        '''
        project  = info.project_from_trigger(trigger=self._trigger, lower_case=True)
        if   name == 'Hadrons' and project == 'rkst':
            l_h1 = self._get_xvector(name='H1_P', ndim=ndim)
            l_h2 = self._get_xvector(name='H2_P', ndim=ndim)
            l_hh = [ h1 + h2 for h1, h2 in zip(l_h1, l_h2) ]

            return l_hh
        elif name == 'Hadrons' and project == 'rk':
            name = 'H_P'
        elif name in ['L1_P', 'L2_P', 'H1_P', 'H2_P', 'B_BPV', 'B_END_V']:
            pass
        else:
            raise ValueError(f'Invalid project/name: {project}/{name}')

        if   ndim == 4:
            l_branch = [f'{name}X', f'{name}Y', f'{name}Z', f'{name}E']
        elif ndim == 3:
            l_branch = [f'{name}X', f'{name}Y', f'{name}Z']
        else:
            raise NotImplementedError(f'Invalid ndim={ndim}')

        d_data   = self._rdf.AsNumpy(l_branch)
        l_array  = [ d_data[branch] for branch in l_branch ]
        arr_dat  = numpy.array(l_array).T
        l_vec    = [ self._val_to_vector(arr_val, ndim) for arr_val in arr_dat ]

        return l_vec
    # -------------------------------
    def _get_alpha(self, pv : XYZVector, sv : XYZVector, l1 : LorentzVector, l2 : LorentzVector, kp : LorentzVector) -> float:
        l1_3v     = l1.Vect()
        l2_3v     = l2.Vect()
        ll_3v     = l1_3v + l2_3v
        kp_3v     = kp.Vect()
        bp_dr     = sv - pv

        cos_thhad = bp_dr.Dot(kp_3v) / (kp_3v.R() * bp_dr.R())
        sin_thhad = math.sqrt(1.0 - cos_thhad ** 2)
        had_pt    = kp_3v.R() * sin_thhad

        cos_thll  = bp_dr.Dot(ll_3v) / (ll_3v.R() * bp_dr.R())
        sin_thll  = math.sqrt(1.0 - cos_thll ** 2 )
        ll_pt     = ll_3v.R() * sin_thll

        alpha     = had_pt / ll_pt if ll_pt > 0. else 1.0

        return alpha
    # -------------------------------
    def _correct_kinematics(self, alpha : float, particle : LorentzVector) -> LorentzVector:
        px = alpha * particle.px()
        py = alpha * particle.py()
        pz = alpha * particle.pz()
        ms =         particle.M()

        return LorentzVector('ROOT::Math::PxPyPzM4D<double>')(px, py, pz, ms)
    # -------------------------------
    def _get_values(self) -> tuple[list[float], list[float]]:
        l_l1 = self._get_xvector(ndim=4, name='L1_P'   )
        l_l2 = self._get_xvector(ndim=4, name='L2_P'   )
        l_kp = self._get_xvector(ndim=4, name='H_P'    )
        l_pv = self._get_xvector(ndim=3, name='B_BPV'  )
        l_sv = self._get_xvector(ndim=3, name='B_END_V')

        l_alpha = []
        l_mass  = []
        for pv, sv, l1, l2, kp in zip(l_pv, l_sv, l_l1, l_l2, l_kp):
            alpha   = self._get_alpha(pv, sv, l1, l2, kp)
            l1_corr = self._correct_kinematics(alpha, l1)
            l2_corr = self._correct_kinematics(alpha, l2)
            mass    = (l1_corr + l2_corr + kp).M()

            l_alpha.append(alpha)
            l_mass.append(mass)

        return l_alpha, l_mass
    # -------------------------------
    def _attach_extra_branches(self, d_data : dict) -> RDataFrame:
        log.debug(f'Attaching extra branches: {self._extra_branches}')

        d_ext = self._rdf.AsNumpy(self._extra_branches)
        d_data.update(d_ext)

        return d_data
    # -------------------------------
    def get_rdf(self, preffix : str) -> RDataFrame:
        '''
        Returns ROOT dataframe with HOP variables
        '''

        l_alpha, l_mass = self._get_values()
        arr_alpha       = numpy.array(l_alpha)
        arr_mass        = numpy.array(l_mass )
        d_data          = {f'{preffix}_alpha' : arr_alpha, f'{preffix}_mass' : arr_mass}
        d_data          = self._attach_extra_branches(d_data)

        rdf = RDF.FromNumpy(d_data)

        return rdf
# -------------------------------
