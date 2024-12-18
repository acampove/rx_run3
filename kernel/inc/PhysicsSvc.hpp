#ifndef PHYSICSSVC_HPP
#define PHYSICSSVC_HPP

#include "ConstDef.hpp"

#include "MessageSvc.hpp"

#include "TupleReader.hpp"
#include "VeloMaterial.hpp"

#include <algorithm>
#include <map>
#include <vector>

#include "TLorentzVector.h"
#include "TMath.h"
#include "TString.h"
#include "TVector3.h"

struct InfoEvent {
    TTreeReaderValue< uint > *               RN;
    TTreeReaderValue< unsigned long long > * EN;
    TTreeReaderValue< int > *                NSPD;
    TTreeReaderValue< int > *                NVELO;
    TTreeReaderValue< int > *                NTRACKS;
    TTreeReaderValue< double > *             NSPD_D;
    TTreeReaderValue< double > *             NVELO_D;
    TTreeReaderValue< double > *             NTRACKS_D;
    InfoEvent() {
        RN        = nullptr;
        EN        = nullptr;
        NSPD      = nullptr;
        NVELO     = nullptr;
        NTRACKS   = nullptr;
        NSPD_D    = nullptr;
        NVELO_D   = nullptr;
        NTRACKS_D = nullptr;
    }
    InfoEvent(TupleReader & _tupleReader, TString _option = "") {
        MessageSvc::Info("InfoEvent", _option);
        RN = _tupleReader.GetValuePtr< uint >("runNumber");
        EN = _tupleReader.GetValuePtr< unsigned long long >("eventNumber");
        if (_option == "TRUE") {
            NSPD_D    = _tupleReader.GetValuePtr< double >("nSPDHits");
            NVELO_D   = _tupleReader.GetValuePtr< double >("nVeloTracks");
            NTRACKS_D = _tupleReader.GetValuePtr< double >("nTracks");
        } else {
            NSPD    = _tupleReader.GetValuePtr< int >("nSPDHits");
            NVELO   = _tupleReader.GetValuePtr< int >("nVeloTracks");
            NTRACKS = _tupleReader.GetValuePtr< int >("nTracks");
        }
    }
    int nTRACKS() { return **NTRACKS; };
};

struct InfoMomentum {
    TTreeReaderValue< double > * PX;
    TTreeReaderValue< double > * PY;
    TTreeReaderValue< double > * PZ;
    TTreeReaderValue< double > * PE;
    InfoMomentum() {
        PX = nullptr;
        PY = nullptr;
        PZ = nullptr;
        PE = nullptr;
    }
    InfoMomentum(TupleReader & _tupleReader, TString _particle, TString _option = "") {
        MessageSvc::Info("InfoMomentum", _particle, _option);
        if (_option == "TRACK") {
            PX = _tupleReader.GetValuePtr< double >(_particle + "_TRACK_PX");
            PY = _tupleReader.GetValuePtr< double >(_particle + "_TRACK_PY");
            PZ = _tupleReader.GetValuePtr< double >(_particle + "_TRACK_PZ");
            PE = _tupleReader.GetValuePtr< double >(_particle + "_PE");
        } else if (_option == "TRUE") {
            PX = _tupleReader.GetValuePtr< double >(_particle + "_TRUEP_X");
            PY = _tupleReader.GetValuePtr< double >(_particle + "_TRUEP_Y");
            PZ = _tupleReader.GetValuePtr< double >(_particle + "_TRUEP_Z");
            PE = _tupleReader.GetValuePtr< double >(_particle + "_TRUEP_E");
        } else {
            PX = _tupleReader.GetValuePtr< double >(_particle + "_PX");
            PY = _tupleReader.GetValuePtr< double >(_particle + "_PY");
            PZ = _tupleReader.GetValuePtr< double >(_particle + "_PZ");
            PE = _tupleReader.GetValuePtr< double >(_particle + "_PE");
        }
    }

    TLorentzVector LV(TString _mass = "") {
        TLorentzVector _p;
        if (_mass == "") {
            if ((PX != nullptr) && (PY != nullptr) && (PZ != nullptr) && (PE != nullptr))
                _p.SetPxPyPzE(**PX, **PY, **PZ, **PE);
            else
                MessageSvc::Warning("InfoMomentum::LV", (TString) "PX PY PZ PE are nullptr");
        } else {
            if ((PX != nullptr) && (PY != nullptr) && (PZ != nullptr)) {
                double M = 0;
                if (_mass == "K") M = PDG::Mass::K;
                if (_mass == "Pi") M = PDG::Mass::Pi;
                if (_mass == "E") M = PDG::Mass::E;
                if (_mass == "M") M = PDG::Mass::M;
                _p.SetPxPyPzE(**PX, **PY, **PZ, TMath::Sqrt(TMath::Sq(**PX) + TMath::Sq(**PY) + TMath::Sq(**PZ) + TMath::Sq(M)));
            } else {
                MessageSvc::Warning("InfoMomentum::LV", (TString) "PX PY PZ are nullptr");
            }
        }
        return _p;
    };

    double PTOT(TString _particle = "") { return LV(_particle).P(); };
    double PT(TString _particle = "") { return LV(_particle).Perp(); };
    double ETA(TString _particle = "") { return LV(_particle).PseudoRapidity(); };
    double PHI(TString _particle = "") { return LV(_particle).Phi(); };
    double M(TString _particle = "") { return LV(_particle).M(); };
};

struct InfoVertex {
    TTreeReaderValue< double > * X;
    TTreeReaderValue< double > * Y;
    TTreeReaderValue< double > * Z;
    TTreeReaderValue< double > * CHI2;
    TTreeReaderValue< int > *    NDOF;
    InfoVertex() {
        X    = nullptr;
        Y    = nullptr;
        Z    = nullptr;
        CHI2 = nullptr;
        NDOF = nullptr;
    }
    InfoVertex(TupleReader & _tupleReader, TString _particle, TString _option = "") {
        MessageSvc::Info("InfoVertex", _particle, _option);
        InfoVertex();
        if (_option == "OWNPV") {
            X = _tupleReader.GetValuePtr< double >(_particle + "_OWNPV_X");
            Y = _tupleReader.GetValuePtr< double >(_particle + "_OWNPV_Y");
            Z = _tupleReader.GetValuePtr< double >(_particle + "_OWNPV_Z");
        } else if (_option == "TRUE") {
            X = _tupleReader.GetValuePtr< double >(_particle + "_TRUEENDVERTEX_X");
            Y = _tupleReader.GetValuePtr< double >(_particle + "_TRUEENDVERTEX_Y");
            Z = _tupleReader.GetValuePtr< double >(_particle + "_TRUEENDVERTEX_Z");
        } else if (_option == "ERR") {
            X = _tupleReader.GetValuePtr< double >(_particle + "_ENDVERTEX_XERR");
            Y = _tupleReader.GetValuePtr< double >(_particle + "_ENDVERTEX_YERR");
            Z = _tupleReader.GetValuePtr< double >(_particle + "_ENDVERTEX_ZERR");
        } else {
            X    = _tupleReader.GetValuePtr< double >(_particle + "_ENDVERTEX_X");
            Y    = _tupleReader.GetValuePtr< double >(_particle + "_ENDVERTEX_Y");
            Z    = _tupleReader.GetValuePtr< double >(_particle + "_ENDVERTEX_Z");
            CHI2 = _tupleReader.GetValuePtr< double >(_particle + "_ENDVERTEX_CHI2");
            NDOF = _tupleReader.GetValuePtr< int >(_particle + "_ENDVERTEX_NDOF");
        }
    }
};

struct InfoParticle {
    InfoMomentum                 P;
    TTreeReaderValue< int > *    ID;
    TTreeReaderValue< int > *    TID;
    TTreeReaderValue< int > *    CAT;
    InfoVertex                   VTX;
    TTreeReaderValue< double > * M;
    TTreeReaderValue< double > * IPC;
    TTreeReaderValue< double > * FD;
    TTreeReaderValue< double > * DIRA;
    TTreeReaderValue< bool > *   HASBREM;
    TTreeReaderValue< double > * REALET;
    TTreeReaderValue< int > *    CALOREG;
    TTreeReaderValue< bool > *   TOS;

    InfoParticle() {
        P       = InfoMomentum();
        ID      = nullptr;
        TID     = nullptr;
        CAT     = nullptr;
        VTX     = InfoVertex();
        M       = nullptr;
        IPC     = nullptr;
        FD      = nullptr;
        DIRA    = nullptr;
        HASBREM = nullptr;
        REALET  = nullptr;
        CALOREG = nullptr;
        TOS     = nullptr;
    }
    InfoParticle(TupleReader & _tupleReader, TString _particle, TString _option = "") {
        MessageSvc::Info("InfoParticle", _particle, _option);
        InfoParticle();

        P  = InfoMomentum(_tupleReader, _particle, _option);
        ID = _tupleReader.GetValuePtr< int >(_particle + "_ID");

        if (!((TString) _tupleReader.Tuple()->GetName()).Contains("MCDecay")) {
            TID = _tupleReader.GetValuePtr< int >(_particle + "_TRUEID");
            CAT = _tupleReader.GetValuePtr< int >(_particle + "_BKGCAT");

            VTX = InfoVertex(_tupleReader, _particle, _option);

            M = _tupleReader.GetValuePtr< double >(_particle + "_M");

            IPC  = _tupleReader.GetValuePtr< double >(_particle + "_IPCHI2_OWNPV");
            FD   = _tupleReader.GetValuePtr< double >(_particle + "_FD_OWNPV");
            DIRA = _tupleReader.GetValuePtr< double >(_particle + "_DIRA_OWNPV");

            HASBREM = _tupleReader.GetValuePtr< bool >(_particle + "_HasBremAdded");
            REALET  = _tupleReader.GetValuePtr< double >(_particle + "_L0Calo_ECAL_realET");
            CALOREG = _tupleReader.GetValuePtr< int >(_particle + "_L0Calo_ECAL_region");
            if (_particle.Contains("E"))
                TOS = _tupleReader.GetValuePtr< bool >(_particle + "_L0ElectronDecision_TOS");
            else if (_particle.Contains("M"))
                TOS = _tupleReader.GetValuePtr< bool >(_particle + "_L0MuonDecision_TOS");
        }
    }

    double PTOT(TString _particle = "") { return P.PTOT(_particle); };
    double PT(TString _particle = "") { return P.PT(_particle); };
    double ETA(TString _particle = "") { return P.ETA(_particle); };
    double PHI(TString _particle = "") { return P.PHI(_particle); };
    bool   RFFOIL(TString _particle = "") {
        if ((TMath::Abs(P.PHI(_particle) - TMath::PiOver2()) < TMath::Pi() / 8.) || (TMath::Abs(P.PHI(_particle) + TMath::PiOver2()) < TMath::Pi() / 8.)) return true;
        return false;
    };
};

struct InfoHOP {
    InfoVertex                   HEAD_ENDVERTEX;
    InfoVertex                   HEAD_OWNPV;
    TTreeReaderValue< double > * HEAD_FD_OWNPV;
    InfoMomentum                 HADS;
    InfoMomentum                 LEPS;
    InfoMomentum                 L1;
    InfoMomentum                 L2;
    InfoHOP() {
        HEAD_ENDVERTEX = InfoVertex();
        HEAD_OWNPV     = InfoVertex();
        HEAD_FD_OWNPV  = nullptr;
        HADS           = InfoMomentum();
        LEPS           = InfoMomentum();
        L1             = InfoMomentum();
        L2             = InfoMomentum();
    }
    InfoHOP(TupleReader & _tupleReader, TString _head, TString _diHad, TString _diLep, TString _lep1, TString _lep2) {
        MessageSvc::Info("InfoHOP", _head, _diHad, _diLep, _lep1, _lep2);
        HEAD_ENDVERTEX = InfoVertex(_tupleReader, _head);
        HEAD_OWNPV     = InfoVertex(_tupleReader, _head, "OWNPV");
        HEAD_FD_OWNPV  = _tupleReader.GetValuePtr< double >(_head + "_FD_OWNPV");
        HADS           = InfoMomentum(_tupleReader, _diHad);
        LEPS           = InfoMomentum(_tupleReader, _diLep);
        L1             = InfoMomentum(_tupleReader, _lep1);
        L2             = InfoMomentum(_tupleReader, _lep2);
    }
};

struct InfoVELO {
    InfoVertex   VERTEX;
    InfoMomentum MOMENTUM;
    InfoVELO() {
        VERTEX   = InfoVertex();
        MOMENTUM = InfoMomentum();
    }
    InfoVELO(TupleReader & _tupleReader, TString _particles, TString _particle, TString _option = "") {
        MessageSvc::Info("InfoVELO", _particles, _particle, _option);
        VERTEX   = InfoVertex(_tupleReader, _particles, _option);
        MOMENTUM = InfoMomentum(_tupleReader, _particle, _option);
    }
};

struct InfoPCov {
    TTreeReaderValue< double > * PCOV00;
    TTreeReaderValue< double > * PCOV01;
    TTreeReaderValue< double > * PCOV02;
    TTreeReaderValue< double > * PCOV03;
    TTreeReaderValue< double > * PCOV10;
    TTreeReaderValue< double > * PCOV11;
    TTreeReaderValue< double > * PCOV12;
    TTreeReaderValue< double > * PCOV13;
    TTreeReaderValue< double > * PCOV20;
    TTreeReaderValue< double > * PCOV21;
    TTreeReaderValue< double > * PCOV22;
    TTreeReaderValue< double > * PCOV23;
    TTreeReaderValue< double > * PCOV30;
    TTreeReaderValue< double > * PCOV31;
    TTreeReaderValue< double > * PCOV32;
    TTreeReaderValue< double > * PCOV33;
    InfoPCov() {
        PCOV00 = nullptr;
        PCOV01 = nullptr;
        PCOV02 = nullptr;
        PCOV03 = nullptr;
        PCOV10 = nullptr;
        PCOV11 = nullptr;
        PCOV12 = nullptr;
        PCOV13 = nullptr;
        PCOV20 = nullptr;
        PCOV21 = nullptr;
        PCOV22 = nullptr;
        PCOV23 = nullptr;
        PCOV30 = nullptr;
        PCOV31 = nullptr;
        PCOV32 = nullptr;
        PCOV33 = nullptr;
    }
    InfoPCov(TupleReader & _tupleReader, TString _particle) {
        MessageSvc::Info("InfoPCov", _particle);
        PCOV00 = _tupleReader.GetValuePtr< double >(_particle + "_PCOV00");
        PCOV01 = _tupleReader.GetValuePtr< double >(_particle + "_PCOV01");
        PCOV02 = _tupleReader.GetValuePtr< double >(_particle + "_PCOV02");
        PCOV03 = _tupleReader.GetValuePtr< double >(_particle + "_PCOV03");
        PCOV10 = _tupleReader.GetValuePtr< double >(_particle + "_PCOV10");
        PCOV11 = _tupleReader.GetValuePtr< double >(_particle + "_PCOV11");
        PCOV12 = _tupleReader.GetValuePtr< double >(_particle + "_PCOV12");
        PCOV13 = _tupleReader.GetValuePtr< double >(_particle + "_PCOV13");
        PCOV20 = _tupleReader.GetValuePtr< double >(_particle + "_PCOV20");
        PCOV21 = _tupleReader.GetValuePtr< double >(_particle + "_PCOV21");
        PCOV22 = _tupleReader.GetValuePtr< double >(_particle + "_PCOV22");
        PCOV23 = _tupleReader.GetValuePtr< double >(_particle + "_PCOV23");
        PCOV30 = _tupleReader.GetValuePtr< double >(_particle + "_PCOV30");
        PCOV31 = _tupleReader.GetValuePtr< double >(_particle + "_PCOV31");
        PCOV32 = _tupleReader.GetValuePtr< double >(_particle + "_PCOV32");
        PCOV33 = _tupleReader.GetValuePtr< double >(_particle + "_PCOV33");
    }
};

struct InfoTCov {
    TTreeReaderValue< double > * TCOV00;
    TTreeReaderValue< double > * TCOV01;
    TTreeReaderValue< double > * TCOV02;
    TTreeReaderValue< double > * TCOV03;
    TTreeReaderValue< double > * TCOV04;
    TTreeReaderValue< double > * TCOV10;
    TTreeReaderValue< double > * TCOV11;
    TTreeReaderValue< double > * TCOV12;
    TTreeReaderValue< double > * TCOV13;
    TTreeReaderValue< double > * TCOV14;
    TTreeReaderValue< double > * TCOV20;
    TTreeReaderValue< double > * TCOV21;
    TTreeReaderValue< double > * TCOV22;
    TTreeReaderValue< double > * TCOV23;
    TTreeReaderValue< double > * TCOV24;
    TTreeReaderValue< double > * TCOV30;
    TTreeReaderValue< double > * TCOV31;
    TTreeReaderValue< double > * TCOV32;
    TTreeReaderValue< double > * TCOV33;
    TTreeReaderValue< double > * TCOV34;
    TTreeReaderValue< double > * TCOV40;
    TTreeReaderValue< double > * TCOV41;
    TTreeReaderValue< double > * TCOV42;
    TTreeReaderValue< double > * TCOV43;
    TTreeReaderValue< double > * TCOV44;
    InfoTCov() {
        TCOV00 = nullptr;
        TCOV01 = nullptr;
        TCOV02 = nullptr;
        TCOV03 = nullptr;
        TCOV04 = nullptr;
        TCOV10 = nullptr;
        TCOV11 = nullptr;
        TCOV12 = nullptr;
        TCOV13 = nullptr;
        TCOV14 = nullptr;
        TCOV20 = nullptr;
        TCOV21 = nullptr;
        TCOV22 = nullptr;
        TCOV23 = nullptr;
        TCOV24 = nullptr;
        TCOV30 = nullptr;
        TCOV31 = nullptr;
        TCOV32 = nullptr;
        TCOV33 = nullptr;
        TCOV34 = nullptr;
        TCOV40 = nullptr;
        TCOV41 = nullptr;
        TCOV42 = nullptr;
        TCOV43 = nullptr;
        TCOV44 = nullptr;
    }
    InfoTCov(TupleReader & _tupleReader, TString _particle) {
        MessageSvc::Info("InfoTCov", _particle);
        TCOV00 = _tupleReader.GetValuePtr< double >(_particle + "_TrCOV00");
        TCOV01 = _tupleReader.GetValuePtr< double >(_particle + "_TrCOV01");
        TCOV02 = _tupleReader.GetValuePtr< double >(_particle + "_TrCOV02");
        TCOV03 = _tupleReader.GetValuePtr< double >(_particle + "_TrCOV03");
        TCOV04 = _tupleReader.GetValuePtr< double >(_particle + "_TrCOV04");
        TCOV10 = _tupleReader.GetValuePtr< double >(_particle + "_TrCOV10");
        TCOV11 = _tupleReader.GetValuePtr< double >(_particle + "_TrCOV11");
        TCOV12 = _tupleReader.GetValuePtr< double >(_particle + "_TrCOV12");
        TCOV13 = _tupleReader.GetValuePtr< double >(_particle + "_TrCOV13");
        TCOV14 = _tupleReader.GetValuePtr< double >(_particle + "_TrCOV14");
        TCOV20 = _tupleReader.GetValuePtr< double >(_particle + "_TrCOV20");
        TCOV21 = _tupleReader.GetValuePtr< double >(_particle + "_TrCOV21");
        TCOV22 = _tupleReader.GetValuePtr< double >(_particle + "_TrCOV22");
        TCOV23 = _tupleReader.GetValuePtr< double >(_particle + "_TrCOV23");
        TCOV24 = _tupleReader.GetValuePtr< double >(_particle + "_TrCOV24");
        TCOV30 = _tupleReader.GetValuePtr< double >(_particle + "_TrCOV30");
        TCOV31 = _tupleReader.GetValuePtr< double >(_particle + "_TrCOV31");
        TCOV32 = _tupleReader.GetValuePtr< double >(_particle + "_TrCOV32");
        TCOV33 = _tupleReader.GetValuePtr< double >(_particle + "_TrCOV33");
        TCOV34 = _tupleReader.GetValuePtr< double >(_particle + "_TrCOV34");
        TCOV40 = _tupleReader.GetValuePtr< double >(_particle + "_TrCOV40");
        TCOV41 = _tupleReader.GetValuePtr< double >(_particle + "_TrCOV41");
        TCOV42 = _tupleReader.GetValuePtr< double >(_particle + "_TrCOV42");
        TCOV43 = _tupleReader.GetValuePtr< double >(_particle + "_TrCOV43");
        TCOV44 = _tupleReader.GetValuePtr< double >(_particle + "_TrCOV44");
    }
};

struct InfoFitCOV {
    TTreeReaderValue< int > * HEAD_ID;
    InfoMomentum              L1_TRACK;
    InfoMomentum              L2_TRACK;
    InfoMomentum              L1;
    InfoMomentum              L2;
    InfoMomentum              H1;
    InfoMomentum              H2;
    InfoPCov                  L1_COV;
    InfoPCov                  L2_COV;
    InfoTCov                  H1_COV;
    InfoTCov                  H2_COV;
    InfoFitCOV() {
        HEAD_ID  = nullptr;
        L1_TRACK = InfoMomentum();
        L2_TRACK = InfoMomentum();
        L1       = InfoMomentum();
        L2       = InfoMomentum();
        H1       = InfoMomentum();
        H2       = InfoMomentum();
        L1_COV   = InfoPCov();
        L2_COV   = InfoPCov();
        H1_COV   = InfoTCov();
        H2_COV   = InfoTCov();
    }
    InfoFitCOV(TupleReader & _tupleReader, TString _head, TString _had1, TString _had2, TString _lep1, TString _lep2) {
        MessageSvc::Info("InfoFitCOV", _head, _had1, _had2, _lep1, _lep2);
        HEAD_ID  = _tupleReader.GetValuePtr< int >(_head + "_ID");
        L1_TRACK = InfoMomentum(_tupleReader, _lep1, "TRACK");
        L2_TRACK = InfoMomentum(_tupleReader, _lep2, "TRACK");
        L1       = InfoMomentum(_tupleReader, _lep1);
        L2       = InfoMomentum(_tupleReader, _lep2);
        H1       = InfoMomentum(_tupleReader, _had1);
        H2       = InfoMomentum(_tupleReader, _had2);
        L1_COV   = InfoPCov(_tupleReader, _lep1);
        L2_COV   = InfoPCov(_tupleReader, _lep2);
        H1_COV   = InfoTCov(_tupleReader, _had1);
        H2_COV   = InfoTCov(_tupleReader, _had2);
    }
};

TVectorD Get4Momentum(TVectorD & _momentumVec, TString _particle);

TMatrixD GetMomentumCovariance(TMatrixD _trcov, double _p, double _qoverp, double _tx, double _ty);

TMatrixD GetFullCovariance(TMatrixD _momcov, TVectorD _mom);

void GetHOP(InfoHOP & _infoHOP, double & _HOP, double & _HEAD_HOP_M, double & _LEPS_HOP_M);

double GetFEH(InfoVELO & _infoVELO, VeloMaterial & _material);

double GetMCORR(double _m, double _p, double _dira);

void ConstraintMassFit(InfoFitCOV & _infoCFit, double & _jpsMassLKswapBConstr, double & _bMassLKswapJPsConstr, double & _bMassLKswapPsiConstr, double & _jpsMassLPiswapBConstr, double & _bMassLPiswapJPsConstr, double & _bMassLPiswapPsiConstr, TString _prj);

/**
 * \brief      Assign value to _thetaL in B->Kll [ angle between leptons ].
 * @param[in]  _bCharge  The b charge
 * @param      _lPlus    The l plus
 * @param      _lMinus   The l minus
 * @param      _kPlus    The k plus
 * @param      _thetaL   The theta_l angle to update....
 */
void HelicityAngles3Bodies(bool _bCharge, InfoMomentum & _lPlus, InfoMomentum & _lMinus, InfoMomentum & _kPlus, double & _thetaL);

void HelicityAngles4Bodies(bool _bCharge, InfoMomentum & _lPlus, InfoMomentum & _lMinus, InfoMomentum & _kPlus, InfoMomentum & _piMinus, double & _thetaL, double & _thetaK, double & _phi);

double GetBR(const TString & _sample, bool _debug = false);

#endif
