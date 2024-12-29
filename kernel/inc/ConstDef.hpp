#pragma once

#include "EnumeratorSvc.hpp"

#include "fmt_ostream.h"
#include <map>
#include <utility>

#include "TMath.h"
#include "TString.h"

using namespace std;

/**
 * \namespace PDG
 * \brief PDG definitions http://pdg.lbl.gov/index.html
 **/
namespace PDG {

    /**
     * \namespace PDG::Mass
     * \brief PDG Masses
     */
    namespace Mass {

        const double Bd     = 5279.58;
        const double Bu     = 5279.26;
        const double Bs     = 5366.77;
        const double Lb     = 5619.60;
        const double KShort = 497.61;
        const double Kst    = 895.81;
        const double L0     = 1115.68;
        const double K      = 493.677;
        const double Pi     = 139.57018;
        const double Phi    = 1019.46;
        const double JPs    = 3096.916;
        const double Psi    = 3686.109;
        const double M      = 105.6583715;
        const double E      = 0.510998928;
        const double P      = 938.27208816;
        const double Dplus  = 1869.65;
        const double D0     = 1864.83;

    }   // namespace Mass

    /**
     * \namespace PDG::ID
     * \brief PDG MC IDs http://pdg.lbl.gov/2019/reviews/rpp2018-rev-monte-carlo-numbering.pdf
     */
    namespace ID {

        const int Bd = 511;
        const int Bu = 521;
        const int Bs = 531;
        const int Bc = 541;
        const int Lb = 5122;
        const int L0 = 3122;

        const int Kst        = 313;
        const int Kst_c      = 323;
        const int K_1_1270_z = 10313;   // K1(1270)0 (K1)
        const int K_1_1270_c = 10323;   // K1(1270)+-
        const int K_2_1430_c = 325;     // K*2(1430)  (K2)        
        const int K_2_1430_z = 315;     // K*2(1430)

        const int K_1_1400_z = 20313;  //K1'(1400)0  (K'1) 0 
        const int K_1_1400_c = 20323;  //K1'(1400)+- (K'1) +


        const int K      = 321;
        const int K0     = 311;
        const int KShort = 310;
        const int KLong  = 130;
        const int Pi     = 211;
        const int Pi0    = 111;

        const int Phi = 333;
        const int JPs = 443;
        const int Psi = 100443;

        const int M   = 13;
        const int E   = 11;
        const int Tau = 15;

        const int P = 2212;
        const int N = 2112;

        const int Eta       = 221;
        const int Eta_prime = 331;
        const int Rho0      = 113;
        const int Rho_c     = 213;
        const int Omega     = 223;
        const int D0        = 421;
        const int Dp        = 411;
        const int Ds        = 431;
        const int Bst0      = 513;
        const int Bst_plus  = 523;

        const int Photon    = 22;
        const map< int, TString > IDTONAME = {
            {511, "Bd"}, {521, "Bu"}, {531, "Bs"}, {541, "Bc"}, {5122, "Lb"}, {313, "Kst0"}, {323, "Kst+"}, {333, "Phi"}, {443, "JPs"}, {100443, "Psi"}, {321, "K"}, {211, "Pi"}, {13, "Mu"}, {11, "E"}, {2212, "P"}, {2112, "N"}, {111, "Pi0"}, {15, "Tau"}, {221, "Eta"}, {113, "Rho0"}, {213, "Rho_c"}, {223, "Omega"}, {331, "Eta_prime"}, {311, "K0"}, {310, "KShort"}, {130, "KLong"}, {10313, "K_1_1270_z"}, {10323, "K_1_1270_c"}, {325, "K_2_1430_c"}, {315, "K_2_1430_z"}, {421, "D0"}, {411, "Dp"}, {431, "Ds"}, {513, "Bst0"}, {523, "Bst_plus"},
        };

        static inline TString NameFromID(int IDParticle) {
            if (IDTONAME.find(abs(IDParticle)) != IDTONAME.end()) { return IDTONAME.at(abs(IDParticle)); }
            return TString(fmt::format("{0}", abs(IDParticle)));
        }

    }   // namespace ID

    /**
     * \namespace PDG::BF
     * \brief PDG Branching Fractions
     */
    namespace BF {

        // http://pdglive.lbl.gov/Particle.action?node=S042
        const double Bd2KstJPs       = 1.27E-3;
        const double Bd2KstJPs_err   = 0.05E-3;
        const double Bd2KstPsi       = 5.9E-4;
        const double Bd2KstPsi_err   = 0.4E-4;
        const double Bd2KstMM        = 9.4E-7;
        const double Bd2KstEE        = 1.03E-6;
        const double Bd2KSJPs        = 8.73E-4;
        const double Bd2KSJPs_err    = 0.32E-4;
        const double Bd2KSPsi        = 5.8E-4;
        const double Bd2KSPsi_err    = 0.5E-4;
        const double Bd2KSMM         = 3.39E-7;
        const double Bd2KSEE         = 1.6E-7;

        const double Bd2DNuKstNu     = 1.2E-3;
        const double Bd2DNuKstNu_err = 5.9E-5;

        // http://pdglive.lbl.gov/Particle.action?node=S041
        const double Bu2KJPs      = 1.010E-3;
        const double Bu2KJPs_err  = 0.028E-3;

        const double Bu2KstJPs      = 1.43E-3;
        const double Bu2KstJPs_err  = 0.08E-3;
        
        const double Bu2KPsi       = 6.21E-4;
        const double Bu2KPsi_err   = 0.22E-4;
        const double Bu2KMM        = 4.41E-7;
        const double Bu2KEE        = 5.5E-7;
        const double Bu2PiJPs      = 3.88E-5;
        const double Bu2PiJPs_err  = 0.12E-5;
        const double Bu2PiPsi      = 2.44E-5;
        const double Bu2PiPsi_err  = 0.30E-5;
        const double Bu2DKNuNu     = 8.3E-4;
        const double Bu2DKNuNu_err = 3.3E-5;

        // http://pdglive.lbl.gov/Particle.action?node=S086
        const double Bs2PhiJPs     = 1.08E-3;   // +/- 0.08E-3
        const double Bs2PhiJPs_err = 0.08E-3;
        const double Bs2PhiPsi     = 5.04E-4;   // +/- 0.60E-4
        const double Bs2PhiPsi_err = 0.60E-4;
        const double Bs2PhiMM      = 8.20E-7;   // +/- 1.20E-7
        const double Bs2PhiEE      = 8.20E-7;   // +/- 1.20E-7 // NOTE: Not measured. Using the value of Bs2PhiMM.
        const double Bs2KstJPs     = 4.1E-5;
        const double Bs2KstJPs_err = 0.4E-5;
        const double Bs2KstPsi     = 3.3E-5;
        const double Bs2KstPsi_err = 0.5E-5;
        const double Bs2KSJPs      = 1.88E-5;
        const double Bs2KSJPs_err  = 0.15E-5;

        // http://pdglive.lbl.gov/Particle.action?node=S040
        const double Lb2pKJPs     = 3.17E-4;
        /* 
            PDG value  (Lb->pKJPs) B.R. 
            3.17 ±0.04 (+0.57,−0.45)        
            However , https://iopscience.iop.org/article/10.1088/1674-1137/40/1/011001
            3.17 ± 0.04 ± 0.07 ± 0.34 (+0.45 , - 0.28) where first error is statistical, second systematic, third uncertainty on B->Kst*J/Psi, LAST is fLb/fd 
            Our fits uses g-consts on fLb/fd , therefore 
            Err = 3.17*TMath::Sqrt( TMath::Sq(0.04/3.17) + TMath::Sq( 0.07/3.17) + TMath::Sq( 0.34/3.17))=0.35
            Thus the fLb/fd component is estimated to be 
            
        */ 
        //const double Lb2pKJPs_err = 0.6E-4; //this is summing up all syst , also due to fLb/fd ( which is present later on as well, avoid double counting )
        const double Lb2pKJPs_err = 0.35E-4; 
        /*
        
        The absolute branching fractions Λ0 b→ ψ(2S)pK− and Λ0 b→ J/ψπ+π−pK− are derived using the branching fraction B(Λ0b→ J/ψpK−) = (3.04 ± 0.04 ± 0.06 ± 0.33 +0.43−0.27) × 10−4, measured in ref. [9], 
        where the third uncertainty is due to the uncertainty on the branching fraction of the decay B0 → J/ψK∗(892)0 and the fourth is due to the knowledge of the ratio of fragmentation fractions fΛ0b /fd. They are found to be
        B(Λ0b→ ψ(2S)pK−) = (6.29 ± 0.23 ± 0.14   +(1.14−0.90)) × 10−5, However, according to https://cds.cern.ch/record/2002164/files/LHCb-ANA-2015-015.pdf 
        The last systematic accounts already for the systematic in Lb0->J/PsipK in fLb/fd , which is overall a 30% of the total error budget , thus the overall error  we assign is reduced by 30% here , since fLb/fd is also gauss constrained in the fits
        Therefore, 
        Err(Lb2pKPsi) = 6.6 * TMath::Sqrt( TMath::Sq(1.2/6.6)[Total rel-err] - TMath::Sq( 0.45/3.17)) = 0.75
        Subtracted the Syst from fLb/fd from Lb2pKJps error. 
        */
        const double Lb2pKPsi     = 6.6E-5;
        // const double Lb2pKPsi_err = 1.2E-5;
        const double Lb2pKPsi_err = 0.75E-5;

        const double Lb2LJPs      = 5.8E-5;   // × B(b -> Lb)
        const double Lb2LJPs_err  = 0.8E-5;
        const double Lb2LPsi      = 0.501 * Lb2LJPs;
        const double Lb2LPsi_err  = TMath::Sqrt(TMath::Sq(Lb2LJPs * 0.038) + TMath::Sq(0.501 * Lb2LJPs_err));

        // http://pdglive.lbl.gov/ParticleGroup.action?init=0&node=MXXX025
        const double JPs2MM       = 0.05961;
        const double JPs2MM_err   = 0.00033;
        const double JPs2EE       = 0.05971;
        const double JPs2EE_err   = 0.00032;
        const double Psi2MM       = 8.0E-3;
        const double Psi2MM_err   = 0.6E-3;
        const double Psi2EE       = 7.93E-3;
        const double Psi2EE_err   = 0.17E-3;
        const double Psi2JPsX     = 0.614;
        const double Psi2JPsX_err = 0.006;
        const double Psi2JPsPiPi     = 0.324;
        const double Psi2JPsPiPi_err = 0.026;
        // http://pdglive.lbl.gov/ParticleGroup.action?init=0&node=MXXX020
        const double Kst2KPi     = 2.0 / 3.0;
        const double Kst2KPi_err = 0.;

        // http://pdglive.lbl.gov/ParticleGroup.action?init=0&node=MXXX005
        const double Phi2KK     = 0.492;
        const double Phi2KK_err = 0.005;

        //https://pdglive.lbl.gov/BranchingRatio.action?desig=248&parCode=S041&home=MXXX045
        //70.4±2.5 x10-6
        const double Bu2KEtaPrime         =  70.4E-6  ; 
        const double Bu2KEtaPrime_err     =  2.5E-6  ; 
        //https://pdglive.lbl.gov/Particle.action?init=0&node=M002&home=MXXX005        
        const double EtaPrime_EEGamma     =  4.91E-4; 
        const double EtaPrime_EEGamma_err =  0.27E-4;
        //https://arxiv.org/abs/1504.06016
        // fraction of Eta'->EE gamma decaying into low q2 bin
        const double EtaPrime_EEGamma_InLowQ2     =  0.154;
        const double EtaPrime_EEGamma_InLowQ2_err =  0.018;
                   
    }// namespace BF

    /**
     * \namespace PDG::DEC
     * \brief DEC files http://lhcbdoc.web.cern.ch/lhcbdoc/decfiles/
     */
    namespace DEC {

        const double fracBd2KstJPs = (0.1920 + 0.1850) / 2.;
        const double fracBu2KJPs   = (0.1596 + 0.1595) / 2.;
        const double fracBu2KstJPs = (0.1436 + 0.1436) / 2.; //In Bu2XJPs decay
        const double fracBs2PhiJPs = (0.1077 + 0.1077) / 2.;

        /*
        Bd_JPsX,mm=JPsiInAcc.dec
        Decay B0sig
        0.1920  MyJ/psi    MyK*0                  SVV_HELAMP PKHplus PKphHplus PKHzero PKphHzero PKHminus PKphHminus ;
        0.0753  Mypsi(2S)  MyK*0                  SVV_HELAMP PKHplus PKphHplus PKHzero PKphHzero PKHminus PKphHminus ;
        0.0548  Mypsi(2S)  K+         pi-         PHSP ;
        0.0357  MyJ/psi    K+         pi-         PHSP ;
        Decay anti-B0sig
        0.1850  MyJ/psi    Myanti-K*0             SVV_HELAMP PKHminus PKphHminus PKHzero PKphHzero PKHplus PKphHplus ;
        0.0761  Mypsi(2S)  Myanti-K*0             SVV_HELAMP PKHminus PKphHminus PKHzero PKphHzero PKHplus PKphHplus ;
        0.0554  Mypsi(2S)  K-         pi+         PHSP ;
        0.0361  MyJ/psi    K-         pi+         PHSP ;
        Decay MyJ/psi
         1.0000  mu+        mu-                    PHOTOS VLL ;
         1.0000  e+        e-                    PHOTOS VLL ;
        Decay Mypsi(2S)
         0.4763  MyJ/psi    pi+        pi-         VVPIPI ;
         0.1741  mu+        mu-                    PHOTOS VLL ;
         0.1741  e+        e-                    PHOTOS VLL ;         
        Decay MyK*0
         0.7993  K+         pi-                    VSS ;
        */
        const map< TString, double> fracBd2XJPs{ 
            { "JPsKst0" , (0.1920 + 0.1850)/2.},
            { "PsiKst0" , (0.0753 + 0.0761)/2.}, 
            { "JPsKPi"  , (0.0361 + 0.0357)/2.}, 
            { "PsiKPi"  , (0.0554 + 0.0548)/2.}, 
            { "JPs2LL"  , 1.}, 
            { "Psi2LL"  , 0.1741},
            { "Kst2KPi" , 0.7993}
        };

        /*
        Bu_JPsX,mm=JPsiInAcc.dec
        Decay MyJ/psi
         1.0000  mu+        mu-                    PHOTOS VLL ;
         1.0000  e+        e-                    PHOTOS VLL ;
        Decay Mypsi(2S)
         0.4762  MyJ/psi    pi+        pi-         VVPIPI ;
         0.1741  mu+        mu-                    PHOTOS VLL ;
         0.1741  e+        e-                    PHOTOS VLL ;
        Decay B+sig
         0.1596  MyJ/psi    K+                   SVS ;
         0.0729  Mypsi(2S)  K+                     SVS ;
        Decay B-sig
         0.1595  MyJ/psi    K-                     SVS ;
         0.0729  Mypsi(2S)  K-                     SVS ;
        */
        const map< TString, double>  fracBu2XJPs{ 
            { "JPsK" , (0.1596 + 0.1595)/2.},
            { "PsiK" , (0.0729 + 0.0761)/2.}, 
            { "JPs2LL"  , 1.}, 
            { "Psi2LL"  , ( 0.1741 + 0.0548)/2.}
        };

        /*
        Bs_JPsX,ee=JPsiInAcc.dec
        Decay B_s0sig
            0.1077  MyJ/psi     Myphi        PVV_CPLH 0.02 1 Hp pHp Hz pHz Hm pHm;
            0.0854  MyJ/psi    K-         K+          PHSP ;
            0.0748  Mypsi(2S)    Myphi        PVV_CPLH 0.02 1 Hp pHp Hz pHz Hm pHm;
            0.0286  Mypsi(2S)  K-         K+         pi0         PHSP ;
        Decay anti-B_s0sig
            0.1077  MyJ/psi     Myphi        PVV_CPLH 0.02 1 Hp pHp Hz pHz Hm pHm;
            0.0854  MyJ/psi    K-         K+          PHSP ;
            0.0748  Mypsi(2S)  Myphi         PVV_CPLH 0.02 1 Hp pHp Hz pHz Hm pHm;
            0.0286  Mypsi(2S)  K-         K+         pi0         PHSP ;            
        Decay Myphi
            0.7503  K+         K-                     VSS ;
        Decay Mypsi(2S)
            0.1738  mu+        mu-                    PHOTOS VLL ;
            0.1738   e+         e-                    PHOTOS VLL ;
            0.4755  MyJ/psi    pi+        pi-         VVPIPI ;
        */

        const map< TString, double>  fracBs2XJPs{ 
            { "JPsPhi" , (0.1077 + 0.1077)/2.},
            { "JPsKK"  , (0.0854 + 0.0854)/2.}, 
            { "Phi2KK"  , 0.7503 }, 
            { "PsiPhi"  , 0.0748},
            { "PsiKK"  ,  0.0286},
            { "Psi2LL"  , 0.1738},
            { "JPs2LL" ,  1. }
        };
    }   // namespace DEC

    /**
     * \namespace PDG::Const
     * \brief PDG Constants
     */
    namespace Const {

        // https://arxiv.org/pdf/1111.2357.pdf (assuming isospin symmetry)
        const double fsOverfd7     = 0.256;
        const double fsOverfd7_err = 0.020;

        // https://arxiv.org/pdf/1902.06794.pdf (assuming isospin symmetry)
        const double fsOverfd13     = 0.122 * 2;
        const double fsOverfd13_err = 0.006 * 2;

        /*
        from https://arxiv.org/pdf/1902.06794.pdf (assuming isospin symmetry)
        fΛb
        ------- (pT) =  A[p1 + exp (p2 + p3 × pT)] ,A = 1±0.061, p1 = (7.93±1.41)·10−2, p2 = −1.022±0.047, and p3 = −0.107 ± 0.002 GeV−1  //---- we may need this in the flatness.
        fu + fd

        fΛb
        ------- = 0.259 ± 0.018 ( average ) [13TeV analysis says it is consistent with 7TeV measurement]
        fu + fd
        */
        const double fLbOverfd     = 0.259 * 2;
        const double fLbOverfd_err = 0.018 * 2;

        const map< Year, double > SQS = {{Year::Y2011, 7.}, {Year::Y2012, 8.}, {Year::Y2015, 13.}, {Year::Y2016, 13.}, {Year::Y2017, 13.}, {Year::Y2018, 13.}};
    }   // namespace Const
}   // namespace PDG

