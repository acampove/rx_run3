'''
Module containing HOPVarCalculator class
'''

import numpy
from ROOT import RDataFrame, RDF

# -------------------------------
class HOPVarCalculator:
    '''
    Class meant to calculate HOP variables from a ROOT dataframe. For info on HOP see:

    https://cds.cern.ch/record/2102345/files/LHCb-INT-2015-037.pdf
    '''
    # -------------------------------
    def __init__(self, rdf : RDataFrame):
        self._rdf = rdf
    # -------------------------------
    def _get_alpha(self) -> numpy.ndarray:
        lv_l1 = _get_lvector('L1')
        lv_l2 = _get_lvector('L2')
        lv_kp = _get_lvector('H' )
        tv_pv = _get_tvector('')
    # -------------------------------
    def get_rdf(self) -> RDataFrame:
        '''
        Returns ROOT dataframe with HOP variables
        '''

        arr_alpha = self._get_alpha()
        arr_mass  = self._get_mass(arr_alpha)

        rdf = RDF.FromNumpy({'alpha_hop' : arr_alpha, 'mass_hop' : arr_mass})

        return rdf
# -------------------------------

# double HOP::Alpha(const TVector3 &B_OWN_PV, const TVector3 &B_END_VTX, const TLorentzVector &Hadron, const TLorentzVector &DiLepton, TLorentzVector &Lepton1, TLorentzVector &Lepton2)
# {
#     TVector3 Hadron_Vect   = Hadron.Vect();
#     TVector3 DiLepton_Vect = DiLepton.Vect();
#     TVector3 Lepton1_Vect  = Lepton1.Vect();
#     TVector3 Lepton2_Vect  = Lepton2.Vect();
#
#     TVector3 B_Vect     = B_END_VTX - B_OWN_PV;
#     TVector3 B_Vect_Dir = B_Vect.Unit();
#
#     // ctk = B scalar Hadron
#     double _CosTheta_Hadron = B_Vect.Dot(Hadron_Vect) / (Hadron_Vect.Mag() * B_Vect.Mag());
#     double _Hadron_PT       = Hadron_Vect.Mag() * TMath::Sqrt(1.0 - _CosTheta_Hadron * _CosTheta_Hadron);
#     double _CosThetaY       = B_Vect.Dot(DiLepton_Vect) / (DiLepton_Vect.Mag() * B_Vect.Mag());
#     double _DiLepton_PT     = DiLepton_Vect.Mag() * sqrt(1.0 - _CosThetaY * _CosThetaY);
#     double _Alpha_HOP       = _DiLepton_PT > 0. ? _Hadron_PT / _DiLepton_PT : 1.0;
#     return _Alpha_HOP;
# };
#
# double HOP::Mass_DIELECTRON(const TVector3 &B_OWN_PV, const TVector3 &B_END_VTX, const TLorentzVector &Hadron, const TLorentzVector &DiLepton, TLorentzVector &Lepton1, TLorentzVector &Lepton2)
# {
#     double _Alpha_HOP = Alpha(B_OWN_PV, B_END_VTX, Hadron, DiLepton, Lepton1, Lepton2);
#     // Corrected 4D momentum for leptons
#     // If we have e-mu mass ?
#     TLorentzVector Lepton1_LV_Corr;
#     Lepton1_LV_Corr.SetXYZM(_Alpha_HOP * Lepton1.Px(), _Alpha_HOP * Lepton1.Py(), _Alpha_HOP * Lepton1.Pz(), PDG::Mass::E);

#     TLorentzVector Lepton2_LV_Corr;
#     Lepton2_LV_Corr.SetXYZM(_Alpha_HOP * Lepton2.Px(), _Alpha_HOP * Lepton2.Py(), _Alpha_HOP * Lepton2.Pz(), PDG::Mass::E);
#
#     TLorentzVector DiLepton_Corr = Lepton1_LV_Corr + Lepton2_LV_Corr;
#
#     TLorentzVector B_Corr = DiLepton_Corr + Hadron;
#     double _HOP_M = B_Corr.Mag();
#     return _HOP_M;
# };

