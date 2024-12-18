#ifndef PHYSICSSVC_CPP
#define PHYSICSSVC_CPP

#include "PhysicsSvc.hpp"

#include "SettingDef.hpp"

TVectorD Get4Momentum(TVectorD & _momentumVec, TString _particle) {
    TVectorD _p(4);
    _p = _momentumVec;
    if (_particle != "") {
        double _mass = 0;
        if (_particle == "E") _mass = PDG::Mass::E;
        if (_particle == "Pi") _mass = PDG::Mass::Pi;
        if (_particle == "K") _mass = PDG::Mass::K;
        _p[3] = TMath::Sqrt(TMath::Sq(_momentumVec[0]) + TMath::Sq(_momentumVec[1]) + TMath::Sq(_momentumVec[2]) + TMath::Sq(_mass));
    }
    return _p;
}

TMatrixD GetMomentumCovariance(TMatrixD _trcov, double _p, double _qoverp, double _tx, double _ty) {
    assert(_trcov.GetNrows() == 5);

    const double _qp2      = _qoverp * _qoverp;
    const double _tx2      = _tx * _tx;
    const double _ty2      = _ty * _ty;
    const double _invNorm  = 1. / sqrt(1. + _tx2 + _ty2);
    const double _invNorm3 = _invNorm * _invNorm * _invNorm;
    const int    _q        = (_qoverp > 0 ? 1 : -1);
    // ROOT::Math::SMatrix<double, 6, 5> jmat;
    // 6 rows, 5 columns
    // adressing row, column
    TMatrixD _jmat(6, 5);
    for (unsigned int i = 0; i < 6; i++) {
        for (unsigned int j = 0; j < 5; j++) _jmat(i, j) = 0.0;
    }

    _jmat(0, 0) = _jmat(1, 1) = 1.;
    _jmat(3, 2)               = _p * (1. + _ty2) * _invNorm3;
    _jmat(3, 3) = _jmat(4, 2) = -_p * _tx * _ty * _invNorm3;
    _jmat(3, 4)               = -_q * _tx * _invNorm / (_qp2);
    _jmat(4, 3)               = _p * (1. + _tx2) * _invNorm3;
    _jmat(4, 4)               = -_q * _ty * _invNorm / (_qp2);
    _jmat(5, 2)               = -_p * _tx * _invNorm3;
    _jmat(5, 3)               = -_p * _ty * _invNorm3;
    _jmat(5, 4)               = -_q * _invNorm * _p * _p;   // -q == -1/q

    TMatrixD _jmatT(_jmat);
    _jmatT.Transpose(_jmatT);
    return (_jmat * _trcov) * _jmatT;
}

TMatrixD GetFullCovariance(TMatrixD _momcov, TVectorD _mom) {
    assert(_momcov.GetNrows() == 6 && _momcov.GetNcols() == 6);
    assert(_mom.GetNrows() == 4);
    // px
    // py
    // pz
    // E
    TMatrixD _result(4, 4);
    _result(0, 0) = _momcov(3, 3);
    _result(0, 1) = _momcov(3, 4);
    _result(0, 2) = _momcov(3, 5);
    _result(0, 3) = (_mom[0] * _momcov(3, 3) + _mom[1] * _momcov(3, 4) + _mom[2] * _momcov(3, 5)) / _mom[3];   // cov(E,px)
    _result(1, 0) = _momcov(4, 3);
    _result(1, 1) = _momcov(4, 4);
    _result(1, 2) = _momcov(4, 5);
    _result(1, 3) = (_mom[0] * _momcov(4, 3) + _mom[1] * _momcov(4, 4) + _mom[2] * _momcov(4, 5)) / _mom[3];   // cov(E,py)
    _result(2, 0) = _momcov(5, 3);
    _result(2, 1) = _momcov(5, 4);
    _result(2, 2) = _momcov(5, 5);
    _result(2, 3) = (_mom[0] * _momcov(5, 3) + _mom[1] * _momcov(5, 4) + _mom[2] * _momcov(5, 5)) / _mom[3];                                                                                                                                                                           // cov(E,pz)
    _result(3, 0) = (_mom[0] * _momcov(3, 3) + _mom[1] * _momcov(3, 4) + _mom[2] * _momcov(3, 5)) / _mom[3];                                                                                                                                                                           // cov(E,px)
    _result(3, 1) = (_mom[0] * _momcov(4, 3) + _mom[1] * _momcov(4, 4) + _mom[2] * _momcov(4, 5)) / _mom[3];                                                                                                                                                                           // cov(E,py)
    _result(3, 2) = (_mom[0] * _momcov(5, 3) + _mom[1] * _momcov(5, 4) + _mom[2] * _momcov(5, 5)) / _mom[3];                                                                                                                                                                           // cov(E,pz)
    _result(3, 3) = (_mom[0] * _mom[0] * _momcov(3, 3) + _mom[1] * _mom[1] * _momcov(4, 4) + _mom[2] * _mom[2] * _momcov(5, 5) + 2.0 * _mom[0] * _mom[1] * _momcov(3, 4) + 2.0 * _mom[0] * _mom[2] * _momcov(3, 5) + 2.0 * _mom[1] * _mom[2] * _momcov(4, 5)) / (_mom[3] * _mom[3]);   // cov(E,E)
    return _result;
}

void GetHOP(InfoHOP & _infoHOP, double & _HOP, double & _HEAD_HOP_M, double & _LEPS_HOP_M) {
    double _pKst        = TMath::Sqrt(TMath::Sq(**_infoHOP.HADS.PX) + TMath::Sq(**_infoHOP.HADS.PY) + TMath::Sq(**_infoHOP.HADS.PZ));
    double _cosThetaKst = ((**_infoHOP.HEAD_ENDVERTEX.X - **_infoHOP.HEAD_OWNPV.X) * **_infoHOP.HADS.PX + (**_infoHOP.HEAD_ENDVERTEX.Y - **_infoHOP.HEAD_OWNPV.Y) * **_infoHOP.HADS.PY + (**_infoHOP.HEAD_ENDVERTEX.Z - **_infoHOP.HEAD_OWNPV.Z) * **_infoHOP.HADS.PZ) / (_pKst * **_infoHOP.HEAD_FD_OWNPV);
    double _pTKst       = _pKst * TMath::Sqrt(1. - TMath::Sq(_cosThetaKst));

    double _pY        = TMath::Sqrt(TMath::Sq(**_infoHOP.LEPS.PX) + TMath::Sq(**_infoHOP.LEPS.PY) + TMath::Sq(**_infoHOP.LEPS.PZ));
    double _cosThetaY = ((**_infoHOP.HEAD_ENDVERTEX.X - **_infoHOP.HEAD_OWNPV.X) * **_infoHOP.LEPS.PX + (**_infoHOP.HEAD_ENDVERTEX.Y - **_infoHOP.HEAD_OWNPV.Y) * **_infoHOP.LEPS.PY + (**_infoHOP.HEAD_ENDVERTEX.Z - **_infoHOP.HEAD_OWNPV.Z) * **_infoHOP.LEPS.PZ) / (_pY * **_infoHOP.HEAD_FD_OWNPV);
    double _pTY       = _pY * TMath::Sqrt(1. - TMath::Sq(_cosThetaY));

    _HOP = _pTKst / _pTY;
    double _massL;
    if (SettingDef::Config::ana == to_string(Analysis::MM)) _massL = PDG::Mass::M;
    if (SettingDef::Config::ana == to_string(Analysis::EE)) _massL = PDG::Mass::E;

    TLorentzVector _ScaledL1Mom;
    _ScaledL1Mom.SetXYZM(_HOP * **_infoHOP.L1.PX, _HOP * **_infoHOP.L1.PY, _HOP * **_infoHOP.L1.PZ, _massL);
    TLorentzVector _ScaledL2Mom;
    _ScaledL2Mom.SetXYZM(_HOP * **_infoHOP.L2.PX, _HOP * **_infoHOP.L2.PY, _HOP * **_infoHOP.L2.PZ, _massL);

    TLorentzVector _KstMom;
    _KstMom.SetPxPyPzE(**_infoHOP.HADS.PX, **_infoHOP.HADS.PY, **_infoHOP.HADS.PZ, **_infoHOP.HADS.PE);

    TLorentzVector ScaledBMom;
    ScaledBMom  = _ScaledL1Mom + _ScaledL2Mom + _KstMom;
    _HEAD_HOP_M = ScaledBMom.M();

    TLorentzVector _ScaledJPsMom;
    _ScaledJPsMom = _ScaledL1Mom + _ScaledL2Mom;
    _LEPS_HOP_M   = _ScaledJPsMom.M();

    return;
}

double GetFEH(InfoVELO & _infoVELO, VeloMaterial & _material) {
    TVector3 _vector    = TVector3(**_infoVELO.MOMENTUM.PX, **_infoVELO.MOMENTUM.PY, **_infoVELO.MOMENTUM.PZ);
    TVector3 _direction = _vector.Unit();
    return _material.expFirstHit(**_infoVELO.VERTEX.X, **_infoVELO.VERTEX.Y, **_infoVELO.VERTEX.Z, _direction.X(), _direction.Y(), _direction.Z());
}

double GetMCORR(double _mass, double _p, double _dira) {
    double _sin = TMath::Sqrt(1 - TMath::Sq(_dira));
    return TMath::Sqrt(TMath::Sq(_mass) + TMath::Sq(_p * _sin)) + _p * _sin;
}

void ConstraintMassFit(InfoFitCOV & _infoCFit, double & _jpsMassLKswapBConstr, double & _bMassLKswapJPsConstr, double & _bMassLKswapPsiConstr, double & _jpsMassLPiswapBConstr, double & _bMassLPiswapJPsConstr, double & _bMassLPiswapPsiConstr, TString _prj) {
    _jpsMassLKswapBConstr  = -1.;
    _bMassLKswapJPsConstr  = -1.;
    _bMassLKswapPsiConstr  = -1.;
    _jpsMassLPiswapBConstr = -1.;
    _bMassLPiswapJPsConstr = -1.;
    _bMassLPiswapPsiConstr = -1.;
    TVectorD _l1P(4);
    _l1P[0] = **_infoCFit.L1.PX;
    _l1P[1] = **_infoCFit.L1.PY;
    _l1P[2] = **_infoCFit.L1.PZ;
    _l1P[3] = **_infoCFit.L1.PE;

    TVectorD _l2P(4);
    _l2P[0] = **_infoCFit.L2.PX;
    _l2P[1] = **_infoCFit.L2.PY;
    _l2P[2] = **_infoCFit.L2.PZ;
    _l2P[3] = **_infoCFit.L2.PE;

    TVectorD _h1P(4);
    _h1P[0] = **_infoCFit.H1.PX;
    _h1P[1] = **_infoCFit.H1.PY;
    _h1P[2] = **_infoCFit.H1.PZ;
    _h1P[3] = **_infoCFit.H1.PE;

    TVectorD _h2P(4);
    if (_prj != "RK") {
        _h2P[0] = **_infoCFit.H2.PX;
        _h2P[1] = **_infoCFit.H2.PY;
        _h2P[2] = **_infoCFit.H2.PZ;
        _h2P[3] = **_infoCFit.H2.PE;
    }

    // mis ID vectors
    TVectorD _l1P_L2K(_l1P);
    _l1P_L2K = Get4Momentum(_l1P_L2K, "K");

    TVectorD _l2P_L2Pi(_l2P);
    if (_prj == "RKst") _l2P_L2Pi = Get4Momentum(_l2P_L2Pi, "Pi");
    if (_prj == "RPhi") _l2P_L2Pi = Get4Momentum(_l2P_L2Pi, "K");

    TVectorD _h1P_H2L(_h1P);
    _h1P_H2L = Get4Momentum(_h1P_H2L, "E");

    TVectorD _h2P_H2L(_h2P);
    if (_prj != "RK") _h2P_H2L = Get4Momentum(_h2P_H2L, "E");

    TMatrixD _l1COV(4, 4);
    _l1COV(0, 0) = **_infoCFit.L1_COV.PCOV00;
    _l1COV(0, 1) = **_infoCFit.L1_COV.PCOV01;
    _l1COV(0, 2) = **_infoCFit.L1_COV.PCOV02;
    _l1COV(0, 3) = **_infoCFit.L1_COV.PCOV03;
    _l1COV(1, 0) = **_infoCFit.L1_COV.PCOV10;
    _l1COV(1, 1) = **_infoCFit.L1_COV.PCOV11;
    _l1COV(1, 2) = **_infoCFit.L1_COV.PCOV12;
    _l1COV(1, 3) = **_infoCFit.L1_COV.PCOV13;
    _l1COV(2, 0) = **_infoCFit.L1_COV.PCOV20;
    _l1COV(2, 1) = **_infoCFit.L1_COV.PCOV21;
    _l1COV(2, 2) = **_infoCFit.L1_COV.PCOV22;
    _l1COV(2, 3) = **_infoCFit.L1_COV.PCOV23;
    _l1COV(3, 0) = **_infoCFit.L1_COV.PCOV30;
    _l1COV(3, 1) = **_infoCFit.L1_COV.PCOV31;
    _l1COV(3, 2) = **_infoCFit.L1_COV.PCOV32;
    _l1COV(3, 3) = **_infoCFit.L1_COV.PCOV33;
    TMatrixD _l2COV(4, 4);
    _l2COV(0, 0) = **_infoCFit.L2_COV.PCOV00;
    _l2COV(0, 1) = **_infoCFit.L2_COV.PCOV01;
    _l2COV(0, 2) = **_infoCFit.L2_COV.PCOV02;
    _l2COV(0, 3) = **_infoCFit.L2_COV.PCOV03;
    _l2COV(1, 0) = **_infoCFit.L2_COV.PCOV10;
    _l2COV(1, 1) = **_infoCFit.L2_COV.PCOV11;
    _l2COV(1, 2) = **_infoCFit.L2_COV.PCOV12;
    _l2COV(1, 3) = **_infoCFit.L2_COV.PCOV13;
    _l2COV(2, 0) = **_infoCFit.L2_COV.PCOV20;
    _l2COV(2, 1) = **_infoCFit.L2_COV.PCOV21;
    _l2COV(2, 2) = **_infoCFit.L2_COV.PCOV22;
    _l2COV(2, 3) = **_infoCFit.L2_COV.PCOV23;
    _l2COV(3, 0) = **_infoCFit.L2_COV.PCOV30;
    _l2COV(3, 1) = **_infoCFit.L2_COV.PCOV31;
    _l2COV(3, 2) = **_infoCFit.L2_COV.PCOV32;
    _l2COV(3, 3) = **_infoCFit.L2_COV.PCOV33;
    TMatrixD _h1TrCOV(5, 5);
    _h1TrCOV(0, 0) = **_infoCFit.H1_COV.TCOV00;
    _h1TrCOV(0, 1) = **_infoCFit.H1_COV.TCOV01;
    _h1TrCOV(0, 2) = **_infoCFit.H1_COV.TCOV02;
    _h1TrCOV(0, 3) = **_infoCFit.H1_COV.TCOV03;
    _h1TrCOV(0, 4) = **_infoCFit.H1_COV.TCOV04;
    _h1TrCOV(1, 0) = **_infoCFit.H1_COV.TCOV10;
    _h1TrCOV(1, 1) = **_infoCFit.H1_COV.TCOV11;
    _h1TrCOV(1, 2) = **_infoCFit.H1_COV.TCOV12;
    _h1TrCOV(1, 3) = **_infoCFit.H1_COV.TCOV13;
    _h1TrCOV(1, 4) = **_infoCFit.H1_COV.TCOV14;
    _h1TrCOV(2, 0) = **_infoCFit.H1_COV.TCOV20;
    _h1TrCOV(2, 1) = **_infoCFit.H1_COV.TCOV21;
    _h1TrCOV(2, 2) = **_infoCFit.H1_COV.TCOV22;
    _h1TrCOV(2, 3) = **_infoCFit.H1_COV.TCOV23;
    _h1TrCOV(2, 4) = **_infoCFit.H1_COV.TCOV24;
    _h1TrCOV(3, 0) = **_infoCFit.H1_COV.TCOV30;
    _h1TrCOV(3, 1) = **_infoCFit.H1_COV.TCOV31;
    _h1TrCOV(3, 2) = **_infoCFit.H1_COV.TCOV32;
    _h1TrCOV(3, 3) = **_infoCFit.H1_COV.TCOV33;
    _h1TrCOV(3, 4) = **_infoCFit.H1_COV.TCOV34;
    _h1TrCOV(4, 0) = **_infoCFit.H1_COV.TCOV40;
    _h1TrCOV(4, 1) = **_infoCFit.H1_COV.TCOV41;
    _h1TrCOV(4, 2) = **_infoCFit.H1_COV.TCOV42;
    _h1TrCOV(4, 3) = **_infoCFit.H1_COV.TCOV43;
    _h1TrCOV(4, 4) = **_infoCFit.H1_COV.TCOV44;

    double   _h1_trp     = sqrt(pow(_h1P[0], 2) + pow(_h1P[1], 2) + pow(_h1P[2], 2));
    double   _h1_trq     = **_infoCFit.HEAD_ID > 0 ? 1. : -1.;
    TMatrixD _h1TrMomCov = GetMomentumCovariance(_h1TrCOV, _h1_trp, _h1_trq / _h1_trp, _h1P[0] / _h1_trp, _h1P[1] / _h1_trp);
    TMatrixD _h1FullCov  = GetFullCovariance(_h1TrMomCov, _h1P);

    TMatrixD _h1FullCovLarge(_h1FullCov);
    double   de = pow(0.1 * (_h1P_H2L[3]), 2);
    _h1FullCovLarge(0, 0) += de * (_h1P[0] / _h1_trp) * (_h1P[0] / _h1_trp);
    _h1FullCovLarge(0, 1) += de * (_h1P[0] / _h1_trp) * (_h1P[1] / _h1_trp);
    _h1FullCovLarge(0, 2) += de * (_h1P[0] / _h1_trp) * (_h1P[2] / _h1_trp);
    _h1FullCovLarge(0, 3) += de * (_h1P[0] / _h1_trp);
    _h1FullCovLarge(1, 0) += de * (_h1P[1] / _h1_trp) * (_h1P[0] / _h1_trp);
    _h1FullCovLarge(1, 1) += de * (_h1P[1] / _h1_trp) * (_h1P[1] / _h1_trp);
    _h1FullCovLarge(1, 2) += de * (_h1P[1] / _h1_trp) * (_h1P[2] / _h1_trp);
    _h1FullCovLarge(1, 3) += de * (_h1P[1] / _h1_trp);
    _h1FullCovLarge(2, 0) += de * (_h1P[2] / _h1_trp) * (_h1P[0] / _h1_trp);
    _h1FullCovLarge(2, 1) += de * (_h1P[2] / _h1_trp) * (_h1P[1] / _h1_trp);
    _h1FullCovLarge(2, 2) += de * (_h1P[2] / _h1_trp) * (_h1P[2] / _h1_trp);
    _h1FullCovLarge(2, 3) += de * (_h1P[2] / _h1_trp);
    _h1FullCovLarge(3, 0) += de * (_h1P[0] / _h1_trp);
    _h1FullCovLarge(3, 1) += de * (_h1P[1] / _h1_trp);
    _h1FullCovLarge(3, 2) += de * (_h1P[2] / _h1_trp);
    _h1FullCovLarge(3, 3) += de;

    double _mBSq   = pow(PDG::Mass::Bd, 2);
    double _mJPsSq = pow(PDG::Mass::JPs, 2);
    double _mPsiSq = pow(PDG::Mass::Psi, 2);
    // double misid kaon-electron jpsi constrained b mass, only track quantities enlarging kaon covariances
    TVectorD _diLepP = _l2P + _h1P_H2L;
    double   _m0sq   = _diLepP[3] * _diLepP[3] - _diLepP[0] * _diLepP[0] - _diLepP[1] * _diLepP[1] - _diLepP[2] * _diLepP[2];
    TVectorD D(4);
    D[0]                   = -2.0 * _diLepP[0];
    D[1]                   = -2.0 * _diLepP[1];
    D[2]                   = -2.0 * _diLepP[2];
    D[3]                   = +2.0 * _diLepP[3];
    TMatrixD _sumcov       = _l2COV + _h1FullCovLarge;
    TVectorD _sumcov_D     = _sumcov * D;
    double   _factor       = _sumcov_D * D;
    double   _lambda       = (_m0sq - _mJPsSq) / _factor;
    TVectorD _new_diLepP   = _diLepP - (_lambda * _sumcov_D);
    TVectorD _constrainedB = _new_diLepP + _l1P_L2K + _h2P;
    _bMassLKswapJPsConstr  = sqrt(_constrainedB[3] * _constrainedB[3] - _constrainedB[0] * _constrainedB[0] - _constrainedB[1] * _constrainedB[1] - _constrainedB[2] * _constrainedB[2]);

    // double misid kaon-electron psi2s constrained b mass, only track quantities enlarging kaon covariances
    _diLepP               = _l2P + _h1P_H2L;
    _m0sq                 = _diLepP[3] * _diLepP[3] - _diLepP[0] * _diLepP[0] - _diLepP[1] * _diLepP[1] - _diLepP[2] * _diLepP[2];
    D[0]                  = -2.0 * _diLepP[0];
    D[1]                  = -2.0 * _diLepP[1];
    D[2]                  = -2.0 * _diLepP[2];
    D[3]                  = +2.0 * _diLepP[3];
    _sumcov               = _l2COV + _h1FullCovLarge;
    _sumcov_D             = _sumcov * D;
    _factor               = _sumcov_D * D;
    _lambda               = (_m0sq - _mPsiSq) / _factor;
    _new_diLepP           = _diLepP - (_lambda * _sumcov_D);
    _constrainedB         = _new_diLepP + _l1P_L2K + _h2P;
    _bMassLKswapPsiConstr = sqrt(_constrainedB[3] * _constrainedB[3] - _constrainedB[0] * _constrainedB[0] - _constrainedB[1] * _constrainedB[1] - _constrainedB[2] * _constrainedB[2]);

    // kaon misid B mass constrained jpsi mass, track quantities only, enlarged kaon covariance
    _diLepP               = _l2P + _h1P_H2L;
    _m0sq                 = (_diLepP[3] + _l1P_L2K[3] + _h2P[3]) * (_diLepP[3] + _l1P_L2K[3] + _h2P[3]) - (_diLepP[0] + _l1P_L2K[0] + _h2P[0]) * (_diLepP[0] + _l1P_L2K[0] + _h2P[0]) - (_diLepP[1] + _l1P_L2K[1] + _h2P[1]) * (_diLepP[1] + _l1P_L2K[1] + _h2P[1]) - (_diLepP[2] + _l1P_L2K[2] + _h2P[2]) * (_diLepP[2] + _l1P_L2K[2] + _h2P[2]);
    D[0]                  = -2.0 * (_diLepP[0] + _h2P[0] + _l1P_L2K[0]);
    D[1]                  = -2.0 * (_diLepP[1] + _h2P[1] + _l1P_L2K[1]);
    D[2]                  = -2.0 * (_diLepP[2] + _h2P[2] + _l1P_L2K[2]);
    D[3]                  = +2.0 * (_diLepP[3] + _h2P[3] + _l1P_L2K[3]);
    _sumcov               = _l2COV + _h1FullCovLarge;
    _sumcov_D             = _sumcov * D;
    _factor               = _sumcov_D * D;
    _lambda               = (_m0sq - _mBSq) / _factor;
    _new_diLepP           = _diLepP - (_lambda * _sumcov_D);
    _jpsMassLKswapBConstr = sqrt(_new_diLepP[3] * _new_diLepP[3] - _new_diLepP[0] * _new_diLepP[0] - _new_diLepP[1] * _new_diLepP[1] - _new_diLepP[2] * _new_diLepP[2]);

    if (_prj == "RK") return;

    TMatrixD _h2TrCOV(5, 5);
    _h2TrCOV(0, 0) = **_infoCFit.H2_COV.TCOV00;
    _h2TrCOV(0, 1) = **_infoCFit.H2_COV.TCOV01;
    _h2TrCOV(0, 2) = **_infoCFit.H2_COV.TCOV02;
    _h2TrCOV(0, 3) = **_infoCFit.H2_COV.TCOV03;
    _h2TrCOV(0, 4) = **_infoCFit.H2_COV.TCOV04;
    _h2TrCOV(1, 0) = **_infoCFit.H2_COV.TCOV10;
    _h2TrCOV(1, 1) = **_infoCFit.H2_COV.TCOV11;
    _h2TrCOV(1, 2) = **_infoCFit.H2_COV.TCOV12;
    _h2TrCOV(1, 3) = **_infoCFit.H2_COV.TCOV13;
    _h2TrCOV(1, 4) = **_infoCFit.H2_COV.TCOV14;
    _h2TrCOV(2, 0) = **_infoCFit.H2_COV.TCOV20;
    _h2TrCOV(2, 1) = **_infoCFit.H2_COV.TCOV21;
    _h2TrCOV(2, 2) = **_infoCFit.H2_COV.TCOV22;
    _h2TrCOV(2, 3) = **_infoCFit.H2_COV.TCOV23;
    _h2TrCOV(2, 4) = **_infoCFit.H2_COV.TCOV24;
    _h2TrCOV(3, 0) = **_infoCFit.H2_COV.TCOV30;
    _h2TrCOV(3, 1) = **_infoCFit.H2_COV.TCOV31;
    _h2TrCOV(3, 2) = **_infoCFit.H2_COV.TCOV32;
    _h2TrCOV(3, 3) = **_infoCFit.H2_COV.TCOV33;
    _h2TrCOV(3, 4) = **_infoCFit.H2_COV.TCOV34;
    _h2TrCOV(4, 0) = **_infoCFit.H2_COV.TCOV40;
    _h2TrCOV(4, 1) = **_infoCFit.H2_COV.TCOV41;
    _h2TrCOV(4, 2) = **_infoCFit.H2_COV.TCOV42;
    _h2TrCOV(4, 3) = **_infoCFit.H2_COV.TCOV43;
    _h2TrCOV(4, 4) = **_infoCFit.H2_COV.TCOV44;

    double   _h2_trp     = sqrt(pow(_h2P[0], 2) + pow(_h2P[1], 2) + pow(_h2P[2], 2));
    double   _h2_trq     = **_infoCFit.HEAD_ID > 0 ? -1. : 1.;
    TMatrixD _h2TrMomCov = GetMomentumCovariance(_h2TrCOV, _h2_trp, _h2_trq / _h2_trp, _h2P[0] / _h2_trp, _h2P[1] / _h2_trp);
    TMatrixD _h2FullCov  = GetFullCovariance(_h2TrMomCov, _h2P);

    TMatrixD _h2FullCovLarge(_h2FullCov);
    de = pow(0.1 * (_h2P_H2L[3]), 2);
    _h2FullCovLarge(0, 0) += de * (_h2P[0] / _h2_trp) * (_h2P[0] / _h2_trp);
    _h2FullCovLarge(0, 1) += de * (_h2P[0] / _h2_trp) * (_h2P[1] / _h2_trp);
    _h2FullCovLarge(0, 2) += de * (_h2P[0] / _h2_trp) * (_h2P[2] / _h2_trp);
    _h2FullCovLarge(0, 3) += de * (_h2P[0] / _h2_trp);
    _h2FullCovLarge(1, 0) += de * (_h2P[1] / _h2_trp) * (_h2P[0] / _h2_trp);
    _h2FullCovLarge(1, 1) += de * (_h2P[1] / _h2_trp) * (_h2P[1] / _h2_trp);
    _h2FullCovLarge(1, 2) += de * (_h2P[1] / _h2_trp) * (_h2P[2] / _h2_trp);
    _h2FullCovLarge(1, 3) += de * (_h2P[1] / _h2_trp);
    _h2FullCovLarge(2, 0) += de * (_h2P[2] / _h2_trp) * (_h2P[0] / _h2_trp);
    _h2FullCovLarge(2, 1) += de * (_h2P[2] / _h2_trp) * (_h2P[1] / _h2_trp);
    _h2FullCovLarge(2, 2) += de * (_h2P[2] / _h2_trp) * (_h2P[2] / _h2_trp);
    _h2FullCovLarge(2, 3) += de * (_h2P[2] / _h2_trp);
    _h2FullCovLarge(3, 0) += de * (_h2P[0] / _h2_trp);
    _h2FullCovLarge(3, 1) += de * (_h2P[1] / _h2_trp);
    _h2FullCovLarge(3, 2) += de * (_h2P[2] / _h2_trp);
    _h2FullCovLarge(3, 3) += de;

    // double misid pion-electron jpsi constrained b mass, only track quantities enlarging pion covariances
    _diLepP                = _l1P + _h2P_H2L;
    _m0sq                  = _diLepP[3] * _diLepP[3] - _diLepP[0] * _diLepP[0] - _diLepP[1] * _diLepP[1] - _diLepP[2] * _diLepP[2];
    D[0]                   = -2.0 * _diLepP[0];
    D[1]                   = -2.0 * _diLepP[1];
    D[2]                   = -2.0 * _diLepP[2];
    D[3]                   = +2.0 * _diLepP[3];
    _sumcov                = _l1COV + _h2FullCovLarge;
    _sumcov_D              = _sumcov * D;
    _factor                = _sumcov_D * D;
    _lambda                = (_m0sq - _mJPsSq) / _factor;
    _new_diLepP            = _diLepP - (_lambda * _sumcov_D);
    _constrainedB          = _new_diLepP + _h1P + _l2P_L2Pi;
    _bMassLPiswapJPsConstr = sqrt(_constrainedB[3] * _constrainedB[3] - _constrainedB[0] * _constrainedB[0] - _constrainedB[1] * _constrainedB[1] - _constrainedB[2] * _constrainedB[2]);

    // double misid pion-electron jpsi constrained b mass, only track quantities enlarging pion covariances
    _diLepP                = _l1P + _h2P_H2L;
    _m0sq                  = _diLepP[3] * _diLepP[3] - _diLepP[0] * _diLepP[0] - _diLepP[1] * _diLepP[1] - _diLepP[2] * _diLepP[2];
    D[0]                   = -2.0 * _diLepP[0];
    D[1]                   = -2.0 * _diLepP[1];
    D[2]                   = -2.0 * _diLepP[2];
    D[3]                   = +2.0 * _diLepP[3];
    _sumcov                = _l1COV + _h2FullCovLarge;
    _sumcov_D              = _sumcov * D;
    _factor                = _sumcov_D * D;
    _lambda                = (_m0sq - _mPsiSq) / _factor;
    _new_diLepP            = _diLepP - (_lambda * _sumcov_D);
    _constrainedB          = _new_diLepP + _h1P + _l2P_L2Pi;
    _bMassLPiswapPsiConstr = sqrt(_constrainedB[3] * _constrainedB[3] - _constrainedB[0] * _constrainedB[0] - _constrainedB[1] * _constrainedB[1] - _constrainedB[2] * _constrainedB[2]);

    // pion misid B mass constrained jpsi mass, track quantities only, enlarged pion covariance
    _diLepP                = _l1P + _h2P_H2L;
    _m0sq                  = (_diLepP[3] + _h1P[3] + _l2P_L2Pi[3]) * (_diLepP[3] + _h1P[3] + _l2P_L2Pi[3]) - (_diLepP[0] + _h1P[0] + _l2P_L2Pi[0]) * (_diLepP[0] + _h1P[0] + _l2P_L2Pi[0]) - (_diLepP[1] + _h1P[1] + _l2P_L2Pi[1]) * (_diLepP[1] + _h1P[1] + _l2P_L2Pi[1]) - (_diLepP[2] + _h1P[2] + _l2P_L2Pi[2]) * (_diLepP[2] + _h1P[2] + _l2P_L2Pi[2]);
    D[0]                   = -2.0 * (_diLepP[0] + _h1P[0] + _l2P_L2Pi[0]);
    D[1]                   = -2.0 * (_diLepP[1] + _h1P[1] + _l2P_L2Pi[1]);
    D[2]                   = -2.0 * (_diLepP[2] + _h1P[2] + _l2P_L2Pi[2]);
    D[3]                   = +2.0 * (_diLepP[3] + _h1P[3] + _l2P_L2Pi[3]);
    _sumcov                = _l1COV + _h2FullCovLarge;
    _sumcov_D              = _sumcov * D;
    _factor                = _sumcov_D * D;
    _lambda                = (_m0sq - _mBSq) / _factor;
    _new_diLepP            = _diLepP - (_lambda * _sumcov_D);
    _jpsMassLPiswapBConstr = sqrt(_new_diLepP[3] * _new_diLepP[3] - _new_diLepP[0] * _new_diLepP[0] - _new_diLepP[1] * _new_diLepP[1] - _new_diLepP[2] * _new_diLepP[2]);

    return;
}

void HelicityAngles3Bodies(bool _KCharge, InfoMomentum & _lPlus, InfoMomentum & _lMinus, InfoMomentum & _kPlus, double & _thetaL) {
    TLorentzVector _lP   = _lPlus.LV();
    TLorentzVector _lM   = _lMinus.LV();
    TLorentzVector _k    = _kPlus.LV();
    TLorentzVector _b    = _lP + _lM + _k;
    TLorentzVector _mumu = _lP + _lM;

    // determine _thetaL
    TVector3 _mumuBoost(-_mumu.BoostVector());
    TVector3 _bBoost(-_b.BoostVector());

    TLorentzVector _muMinusD(_lM);
    _muMinusD.Boost(_mumuBoost);
    TLorentzVector _muPlusD(_lP);
    _muPlusD.Boost(_mumuBoost);
    TLorentzVector _bD(_b);
    _bD.Boost(_mumuBoost);

    if (_KCharge)
        _thetaL = _muPlusD.Vect().Angle(-_bD.Vect());
    else
        _thetaL = _muMinusD.Vect().Angle(-_bD.Vect());
    return;
}

// calculate decay angles according to lhcb convention http://arxiv.org/abs/1304.6325, _KCharge is true for B0 and false for B0bar
void HelicityAngles4Bodies(bool _KCharge, InfoMomentum & _lPlus, InfoMomentum & _lMinus, InfoMomentum & _kPlus, InfoMomentum & _piMinus, double & _thetaL, double & _thetaK, double & _phi) {
    TLorentzVector _lP = _lPlus.LV();
    TLorentzVector _lM = _lMinus.LV();
    TLorentzVector _k  = _kPlus.LV();
    TLorentzVector _pi = _piMinus.LV();
    // set up boost vectors
    TLorentzVector _b    = _lP + _lM + _k + _pi;
    TLorentzVector _mumu = _lP + _lM;
    TLorentzVector _kpi  = _k + _pi;
    TVector3       _mumuBoost(-_mumu.BoostVector());
    TVector3       _kpiBoost(-_kpi.BoostVector());
    TVector3       _bBoost(-_b.BoostVector());

    // determine _thetaL
    TLorentzVector _muMinusD(_lM);
    _muMinusD.Boost(_mumuBoost);
    TLorentzVector _muPlusD(_lP);
    _muPlusD.Boost(_mumuBoost);
    TLorentzVector _bD(_b);
    _bD.Boost(_mumuBoost);

    if (_KCharge)
        _thetaL = _muPlusD.Vect().Angle(-_bD.Vect());
    else
        _thetaL = _muMinusD.Vect().Angle(-_bD.Vect());

    // determine _thetaK
    TLorentzVector _kDD(_k);
    _kDD.Boost(_kpiBoost);
    TLorentzVector _bDD(_b);
    _bDD.Boost(_kpiBoost);

    _thetaK = _kDD.Vect().Angle(-_bDD.Vect());

    // determine phi
    TLorentzVector _kDDD(_k);
    _kDDD.Boost(_bBoost);
    TLorentzVector _piDDD(_pi);
    _piDDD.Boost(_bBoost);
    TLorentzVector _muMinusDDD(_lM);
    _muMinusDDD.Boost(_bBoost);
    TLorentzVector _muPlusDDD(_lP);
    _muPlusDDD.Boost(_bBoost);

    TVector3       _normalKPi  = _kDDD.Vect().Cross(_piDDD.Vect());
    TVector3       _normalMuMu = _muPlusDDD.Vect().Cross(_muMinusDDD.Vect());
    TLorentzVector _kpiDDD(_kpi);
    _kpiDDD.Boost(_bBoost);

    if (_KCharge) {
        _phi = _normalKPi.Angle(_normalMuMu);
        if ((_normalMuMu.Cross(_normalKPi)).Dot(_kpiDDD.Vect()) < 0.0) _phi = -_phi;
    } else {
        _phi = -_normalKPi.Angle(-_normalMuMu);
        if ((_normalMuMu.Cross(_normalKPi)).Dot(_kpiDDD.Vect()) < 0.0) _phi = -_phi;
    }
    return;
}

double GetBR(const TString & _sample, bool _debug) {
    double _br = 1;
    // RKst
    if (_sample.Contains("Bd2Kst")) _br = PDG::BF::Bd2KstMM * PDG::BF::Kst2KPi;
    if (_sample.Contains("Bd2KstJPs")) _br = PDG::BF::Bd2KstJPs * PDG::BF::Kst2KPi * PDG::BF::JPs2MM;
    if (_sample.Contains("Bd2KstPsi")) _br = PDG::BF::Bd2KstPsi * PDG::BF::Kst2KPi * PDG::BF::Psi2MM;
    // RK
    if (_sample.Contains("Bu2K")) _br = PDG::BF::Bu2KMM;
    if (_sample.Contains("Bu2KJPs")) _br = PDG::BF::Bu2KJPs * PDG::BF::JPs2MM;
    if (_sample.Contains("Bu2KPsi")) _br = PDG::BF::Bu2KPsi * PDG::BF::Psi2MM;
    // RPhi
    if (_sample.Contains("MM")) {
        if (_sample.Contains("Bs2Phi")) _br = PDG::BF::Bs2PhiMM;
        if (_sample.Contains("Bs2PhiJPs")) _br = PDG::BF::Bs2PhiJPs * PDG::BF::JPs2MM;
        if (_sample.Contains("Bs2PhiPsi")) _br = PDG::BF::Bs2PhiPsi * PDG::BF::Psi2MM;
    } else if (_sample.Contains("EE")) {
        if (_sample.Contains("Bs2Phi")) _br = PDG::BF::Bs2PhiEE;
        if (_sample.Contains("Bs2PhiJPs")) _br = PDG::BF::Bs2PhiJPs * PDG::BF::JPs2EE;
        if (_sample.Contains("Bs2PhiPsi")) _br = PDG::BF::Bs2PhiPsi * PDG::BF::Psi2EE;
    }
    return _br;
}

#endif
