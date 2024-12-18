#ifndef CUTDEFRK_HPP
#define CUTDEFRK_HPP

#include "ConstDef.hpp"

#include "CutDefRX.hpp"

#include "TCut.h"

/**
 * \namespace CutDefRK
 * \brief RK cut definitions
 **/
namespace CutDefRK {

    /**
     * \namespace CutDefRK::Trigger
     * \brief RK Trigger cut definitions
     **/
    namespace Trigger {

        const TCut L0H = "(K_L0HadronDecision_TOS && K_L0Calo_HCAL_region >= 0 && K_L0Calo_HCAL_realET > 4000)";

    }   // namespace Trigger

    /**
     * \namespace CutDefRK::Mass
     * \brief RK Mass cut definitions
     **/
    namespace Mass {

        const TCut Q2Low      = "TMath::Sq(JPs_M/1000) > 0.1 && TMath::Sq(JPs_M/1000) < 1.1";

        const TCut Q2LowTight = "TMath::Sq(JPs_M/1000) > 0.1 && TMath::Sq(JPs_M/1000) < 0.98"; //Used for Muon Branching Fraction Measurement
        
        const TCut Q2Central  = "TMath::Sq(JPs_M/1000) > 1.1 && TMath::Sq(JPs_M/1000) < 6";
        const TCut Q2HighMM   = "TMath::Sq(JPs_M/1000) > 15 && TMath::Sq(JPs_M/1000) < 22";
        const TCut Q2HighEE   = "TMath::Sq(JPs_TRACK_M/1000) > 14";   // && TMath::Sq(JPs_TRACK_M/1000) < 22

        // const TCut CombBVetoJPs = (TCut)((TString) Form("TMath::Abs({HEAD}_DTF_JPs_M - %f) > 100", PDG::Mass::Bu));
        // const TCut CombBVetoPsi = (TCut)((TString) Form("TMath::Abs({HEAD}_DTF_Psi_M - %f) > 100", PDG::Mass::Bu));
        const TCut CombBVetoJPs = (TCut)((TString) Form("TMath::Abs({HEAD}_DTF_JPs_M - %f) > 50", PDG::Mass::Bu));
        const TCut CombBVetoPsi = (TCut)((TString) Form("TMath::Abs({HEAD}_DTF_Psi_M - %f) > 50", PDG::Mass::Bu));    
    }// namespace Mass

    /**
     * \namespace CutDefRK::Quality
     * \brief RK Quality cut definitions
    **/
    namespace Quality {

        const TCut trgAccL0H = "K_L0Calo_HCAL_region >= 0";

        // const TCut etaAccH = "K_ETA > 2.0 && K_ETA < 4.0";

        const TCut inAccH = "K_InAccHcal==1";

        const TCut pidAccH = "K_hasRich==1 && K_InAccMuon==1";

        const TCut pidCalibAccH = "K_PT > 250 && K_P > 2000";

        const TCut stripAccH = "K_PT > 400 && K_IPCHI2_OWNPV > 9";

        const TCut trackAccH = "K_TRACK_CHI2NDOF < 3 && K_TRACK_GhostProb < 0.4";

        const TCut openingAngle = "L1L2_OA > 0.0005 && L1H1_OA > 0.0005 && L2H1_OA > 0.0005";

        const TCut qualityMM = CutDefRX::Quality::inAccM && pidAccH && CutDefRX::Quality::pidAccM && pidCalibAccH && CutDefRX::Quality::pidCalibAccM && stripAccH && CutDefRX::Quality::stripAccM && trackAccH && CutDefRX::Quality::trackAccM && openingAngle;
        const TCut qualityMM_noIsMuon = CutDefRX::Quality::inAccM && pidAccH && CutDefRX::Quality::pidAccM_noIsMuon && pidCalibAccH && CutDefRX::Quality::pidCalibAccM && stripAccH && CutDefRX::Quality::stripAccM && trackAccH && CutDefRX::Quality::trackAccM && openingAngle;

        const TCut qualityEE = CutDefRX::Quality::inAccE && CutDefRX::Quality::trgAccL0E_ECALregion && CutDefRX::Quality::inCaloHoleE && pidAccH && CutDefRX::Quality::pidAccE && pidCalibAccH && CutDefRX::Quality::pidCalibAccE && stripAccH && CutDefRX::Quality::stripAccE && trackAccH && CutDefRX::Quality::trackAccE && openingAngle;

    }   // namespace Quality

    /**
     * \namespace CutDefRK::Background
     * \brief RK Background cut definitions
     **/
    namespace Background {

        // B+ ==> ^(J/psi(1S) ==> ^l+ ^l-) ^K+
        // DecayTreeTuple          L1  L2   K
        // TupleToolSubMass        1   0    2

        // Bu2DX
        const TCut Bu2DX       = "TMath::Abs(TMath::Cos(Bp_ThetaL_custom)) < 0.8";
        const TCut D2Klnu      = "Bp_M02 > 1885";
        const TCut Bu2DMNu     = (TCut)((TString) Form("TMath::Abs(Bp_M02_Subst0_mu2pi - %f) > 40", PDG::Mass::D0));
        const TCut Bu2DMNu_PID = (TCut)((TString) Form("!(TMath::Abs(Bp_M02_Subst0_mu2pi - %f) < 40 && M2_{TUNE}_ProbNNmu < 0.8)", PDG::Mass::D0));
        const TCut Bu2DENu     = (TCut)((TString) Form("TMath::Abs(Bp_TRACK_M12_Subst1_e2pi - %f) > 40", PDG::Mass::D0));   // WRONG NAME, SHOULD INSTEAD BE Bp_TRACK_M02_Subst0_e2pi
        const TCut Bu2DENu_PID = (TCut)((TString) Form("!(TMath::Abs(Bp_TRACK_M12_Subst1_e2pi - %f) < 40 && E2_{TUNE}_ProbNNe < 0.8)", PDG::Mass::D0));

        /*
            SS data version of the Veto cuts following same charge logics in OR condition  
            K_ID * E2_ID > 0 :   [l+ l+ K-]
            K_ID * E2_ID < 0 :   [l- l- K-]
        */            
        const TCut KL_SameSign       = "K_ID * E2_ID < 0";
        const TCut KL_OppositeSign   = "K_ID * E2_ID > 0";
        const TCut Bu2DX_OTHER       = "TMath::Abs(TMath::Cos(Bp_ThetaL_custom)) < 0.8";
        const TCut D2Klnu_OTHER      = "Bp_M12 > 1885";
        const TCut Bu2DMNu_OTHER     = (TCut)((TString) Form("TMath::Abs(Bp_M12_Subst1_mu2pi - %f) > 40", PDG::Mass::D0));
        const TCut Bu2DMNu_PID_OTHER = (TCut)((TString) Form("!(TMath::Abs(Bp_M12_Subst1_mu2pi - %f) < 40 && M1_{TUNE}_ProbNNmu < 0.8)", PDG::Mass::D0));
        const TCut Bu2DENu_OTHER     = (TCut)((TString) Form("TMath::Abs(Bp_TRACK_M02_Subst0_e2pi - %f) > 40", PDG::Mass::D0));   // WRONG NAME, SHOULD INSTEAD BE Bp_TRACK_M02_Subst0_e2pi
        const TCut Bu2DENu_PID_OTHER = (TCut)((TString) Form("!(TMath::Abs(Bp_TRACK_M02_Subst0_e2pi - %f) < 40 && E2_{TUNE}_ProbNNe < 0.8)", PDG::Mass::D0));        

        // MisID
        const TCut misIDJPsMM     = (TCut)((TString) Form("TMath::Abs(Bp_M02_Subst2_K2mu - %f) > 60", PDG::Mass::JPs));
        const TCut misIDJPsMM_PID = (TCut)((TString) Form("!(TMath::Abs(Bp_M02_Subst2_K2mu - %f) < 60 && M1_{TUNE}_ProbNNmu < 0.8)", PDG::Mass::JPs));
        const TCut misIDPsiMM     = (TCut)((TString) Form("TMath::Abs(Bp_M02_Subst2_K2mu - %f) > 60", PDG::Mass::Psi));
        const TCut misIDPsiMM_PID = (TCut)((TString) Form("!(TMath::Abs(Bp_M02_Subst2_K2mu - %f) < 60 && M1_{TUNE}_ProbNNmu < 0.8)", PDG::Mass::Psi));
        const TCut misIDJPsEE     = (TCut)((TString) Form("TMath::Abs(Bp_TRACK_M_Subst_Kl2lK_DTF_JPs - %f) > 60", PDG::Mass::Bu));
        const TCut misIDJPsEE_PID = (TCut)((TString) Form("!(TMath::Abs(Bp_TRACK_M_Subst_Kl2lK_DTF_JPs - %f) < 60 && K_{TUNE}_ProbNNk < 0.8)", PDG::Mass::Bu));
        const TCut misIDPsiEE     = (TCut)((TString) Form("TMath::Abs(Bp_TRACK_M_Subst_Kl2lK_DTF_Psi - %f) > 60", PDG::Mass::Bu));
        const TCut misIDPsiEE_PID = (TCut)((TString) Form("!(TMath::Abs(Bp_TRACK_M_Subst_Kl2lK_DTF_Psi - %f) < 60 && K_{TUNE}_ProbNNk < 0.8)", PDG::Mass::Bu));

        // Peaking
        const TCut peakingBkgMM     = misIDJPsMM && misIDPsiMM;
        const TCut peakingBkgMM_PID = misIDJPsMM_PID && misIDPsiMM_PID;
        const TCut peakingBkgEE     = misIDJPsEE && misIDPsiEE;
        const TCut peakingBkgEE_PID = misIDJPsEE_PID && misIDPsiEE_PID;

        // Semileptonic
	    /* TODO : FIX In v11
            const TCut semilepBkgMM     = Bu2DMNu;
            const TCut semilepBkgMM_PID = Bu2DMNu_PID;
            const TCut semilepBkgEE     = Bu2DENu;
            const TCut semilepBkgEE_PID = Bu2DENu_PID;
        */

        const TCut semilepBkgMM     = D2Klnu && Bu2DMNu;
        const TCut semilepBkgMM_PID = D2Klnu && Bu2DMNu_PID;
        const TCut semilepBkgEE     = D2Klnu && Bu2DENu;
        const TCut semilepBkgEE_PID = D2Klnu && Bu2DENu_PID;
      
    }   // namespace Background

    /**
     * \namespace CutDefRK::PID
     * \brief RK PID cut definitions
     **/
    namespace PID {

        const TCut pidStripH = "K_PIDK > -5";

        const TCut dllK = "K_PIDK > 0";
        const TCut dllE = "TMath::Min(E1_PIDe, E2_PIDe) > 2";

        const TCut probnnK = "K_{TUNE}_ProbNNk * (1 - K_{TUNE}_ProbNNp) > 0.05";
        const TCut probnnM = "TMath::Min(M1_{TUNE}_ProbNNmu,M2_{TUNE}_ProbNNmu) > 0.2";
        const TCut probnnE = "TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.2";

        const TCut pidE1E2 = CutDefRX::PID::pidStripE && probnnE && dllE;
        const TCut pidMM   = (pidStripH && probnnK && dllK) && probnnM;
        const TCut pidEE   = (pidStripH && probnnK && dllK) && pidE1E2;


        //Variation on electron mode for PIDe>3 ONLY
        const TCut dllE_PID3    = "TMath::Min(E1_PIDe, E2_PIDe) > 3";
        const TCut pidE1E2_PID3 = CutDefRX::PID::pidStripE && dllE_PID3;
        const TCut pidEE_PID3   = (pidStripH && probnnK && dllK) && pidE1E2_PID3;

        //Variation on electron mode for PIDe>3 & ProbNNe >0.4
        const TCut probnnE4 = "TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.4";
        const TCut pidE1E2_PID3_Prob4 = CutDefRX::PID::pidStripE && dllE_PID3 && probnnE4;
        const TCut pidEE_PID3_Prob4   = (pidStripH && probnnK && dllK) && pidE1E2_PID3_Prob4;

        //Variation on electron mode for PIDe>2 ONLY
        const TCut dllE_PID2    = "TMath::Min(E1_PIDe, E2_PIDe) > 2";
        const TCut pidE1E2_PID2 = CutDefRX::PID::pidStripE && dllE_PID2;
        const TCut pidEE_PID2   = (pidStripH && probnnK && dllK) && pidE1E2_PID2;
        
        //Variation on electron mode for PIDe>5 ONLY
        const TCut dllE_PID5    = "TMath::Min(E1_PIDe, E2_PIDe) > 5";
        const TCut pidE1E2_PID5 = CutDefRX::PID::pidStripE && dllE_PID5;
        const TCut pidEE_PID5   = (pidStripH && probnnK && dllK) && pidE1E2_PID5;

        //Variation on electron mode for PIDe>7 ONLY
        const TCut dllE_PID7    = "TMath::Min(E1_PIDe, E2_PIDe) > 7";
        const TCut pidE1E2_PID7 = CutDefRX::PID::pidStripE && dllE_PID7;
        const TCut pidEE_PID7   = (pidStripH && probnnK && dllK) && pidE1E2_PID7;

    }   // namespace PID

    /**
     * \namespace CutDefRK::MVA
     * \brief RK MVA cut definitions
     **/
    namespace MVA {
        const map< tuple< Analysis, Year, Q2Bin >, TString > CAT = {   //
            {make_tuple(Analysis::MM, Year::Run1, Q2Bin::JPsi),   "cat_wMVA_lowcen > 0.05"},
            {make_tuple(Analysis::MM, Year::Run2p1, Q2Bin::JPsi), "cat_wMVA_lowcen > 0.05"},
            {make_tuple(Analysis::MM, Year::Run2p2, Q2Bin::JPsi), "cat_wMVA_lowcen > 0.05"},

            {make_tuple(Analysis::MM, Year::Run1, Q2Bin::Psi),   "cat_wMVA_lowcen > 0.05"},
            {make_tuple(Analysis::MM, Year::Run2p1, Q2Bin::Psi), "cat_wMVA_lowcen > 0.05"},
            {make_tuple(Analysis::MM, Year::Run2p2, Q2Bin::Psi), "cat_wMVA_lowcen > 0.05"},

            {make_tuple(Analysis::MM, Year::Run1, Q2Bin::Low),   "cat_wMVA_lowcen > 0.70"},
            {make_tuple(Analysis::MM, Year::Run2p1, Q2Bin::Low), "cat_wMVA_lowcen > 0.85"},
            {make_tuple(Analysis::MM, Year::Run2p2, Q2Bin::Low), "cat_wMVA_lowcen > 0.85"},

            {make_tuple(Analysis::MM, Year::Run1, Q2Bin::Central),   "cat_wMVA_lowcen > 0.70"},
            {make_tuple(Analysis::MM, Year::Run2p1, Q2Bin::Central), "cat_wMVA_lowcen > 0.80"},
            {make_tuple(Analysis::MM, Year::Run2p2, Q2Bin::Central), "cat_wMVA_lowcen > 0.80"},

            {make_tuple(Analysis::MM, Year::Run1, Q2Bin::High),   "cat_wMVA_high > 0."},
            {make_tuple(Analysis::MM, Year::Run2p1, Q2Bin::High), "cat_wMVA_high > 0."},
            {make_tuple(Analysis::MM, Year::Run2p2, Q2Bin::High), "cat_wMVA_high > 0."},

            {make_tuple(Analysis::EE, Year::Run1,   Q2Bin::JPsi), "cat_wMVA_lowcen > 0.10 && cat_wMVA_PR_lowcen > 0.05"},
            {make_tuple(Analysis::EE, Year::Run2p1, Q2Bin::JPsi), "cat_wMVA_lowcen > 0.10 && cat_wMVA_PR_lowcen > 0.05"},
            {make_tuple(Analysis::EE, Year::Run2p2, Q2Bin::JPsi), "cat_wMVA_lowcen > 0.10 && cat_wMVA_PR_lowcen > 0.05"},

            {make_tuple(Analysis::EE, Year::Run1,   Q2Bin::Psi), "cat_wMVA_lowcen > 0.47"},
            {make_tuple(Analysis::EE, Year::Run2p1, Q2Bin::Psi), "cat_wMVA_lowcen > 0.36"},
            {make_tuple(Analysis::EE, Year::Run2p2, Q2Bin::Psi), "cat_wMVA_lowcen > 0.59"},

            {make_tuple(Analysis::EE, Year::Run1,   Q2Bin::Low), "cat_wMVA_lowcen > 0.90 && cat_wMVA_PR_lowcen > 0.40"},
            {make_tuple(Analysis::EE, Year::Run2p1, Q2Bin::Low), "cat_wMVA_lowcen > 0.90 && cat_wMVA_PR_lowcen > 0.40"},
            {make_tuple(Analysis::EE, Year::Run2p2, Q2Bin::Low), "cat_wMVA_lowcen > 0.90 && cat_wMVA_PR_lowcen > 0.40"},

            {make_tuple(Analysis::EE, Year::Run1,   Q2Bin::Central), "cat_wMVA_lowcen > 0.90 && cat_wMVA_PR_lowcen > 0.40"},
            {make_tuple(Analysis::EE, Year::Run2p1, Q2Bin::Central), "cat_wMVA_lowcen > 0.90 && cat_wMVA_PR_lowcen > 0.40"},
            {make_tuple(Analysis::EE, Year::Run2p2, Q2Bin::Central), "cat_wMVA_lowcen > 0.90 && cat_wMVA_PR_lowcen > 0.40"},

            {make_tuple(Analysis::EE, Year::Run1,   Q2Bin::High), "cat_wMVA_high > 0."},
            {make_tuple(Analysis::EE, Year::Run2p1, Q2Bin::High), "cat_wMVA_high > 0."},
            {make_tuple(Analysis::EE, Year::Run2p2, Q2Bin::High), "cat_wMVA_high > 0."}};
        /*
        const map< tuple< Analysis, Year, Q2Bin >, TString > CAT = {   //
            {make_tuple(Analysis::MM, Year::Run1, Q2Bin::Low), "cat_wMVA_lowcen > 0.84"},
            {make_tuple(Analysis::MM, Year::Run1, Q2Bin::Central), "cat_wMVA_lowcen > 0.76"},
            {make_tuple(Analysis::MM, Year::Run1, Q2Bin::High), "cat_wMVA_high > 0."},
            {make_tuple(Analysis::MM, Year::Run1, Q2Bin::JPsi), "cat_wMVA_lowcen > 0.05"},
            {make_tuple(Analysis::MM, Year::Run1, Q2Bin::Psi), "cat_wMVA_lowcen > 0.02"},

            {make_tuple(Analysis::MM, Year::Run2p1, Q2Bin::Low), "cat_wMVA_lowcen > 0.92"},
            {make_tuple(Analysis::MM, Year::Run2p1, Q2Bin::Central), "cat_wMVA_lowcen > 0.92"},
            {make_tuple(Analysis::MM, Year::Run2p1, Q2Bin::High), "cat_wMVA_high > 0."},
            {make_tuple(Analysis::MM, Year::Run2p1, Q2Bin::JPsi), "cat_wMVA_lowcen > 0.05"},
            {make_tuple(Analysis::MM, Year::Run2p1, Q2Bin::Psi), "cat_wMVA_lowcen > 0.02"},

            {make_tuple(Analysis::MM, Year::Run2p2, Q2Bin::Low), "cat_wMVA_lowcen > 0."},
            {make_tuple(Analysis::MM, Year::Run2p2, Q2Bin::Central), "cat_wMVA_lowcen > 0."},
            {make_tuple(Analysis::MM, Year::Run2p2, Q2Bin::High), "cat_wMVA_high > 0."},
            {make_tuple(Analysis::MM, Year::Run2p2, Q2Bin::JPsi), "cat_wMVA_lowcen > 0.05"},
            {make_tuple(Analysis::MM, Year::Run2p2, Q2Bin::Psi), "cat_wMVA_lowcen > 0."},

            {make_tuple(Analysis::EE, Year::Run1, Q2Bin::Low), "cat_wMVA_lowcen > 0.925 && cat_wMVA_PR_lowcen > 0.35"},
            {make_tuple(Analysis::EE, Year::Run1, Q2Bin::Central), "cat_wMVA_lowcen > 0.950 && cat_wMVA_PR_lowcen > 0.35"},
            {make_tuple(Analysis::EE, Year::Run1, Q2Bin::High), "cat_wMVA_high > 0."},
            {make_tuple(Analysis::EE, Year::Run1, Q2Bin::JPsi), "cat_wMVA_lowcen > 0.05"},
            {make_tuple(Analysis::EE, Year::Run1, Q2Bin::Psi), "cat_wMVA_lowcen > 0.30"},

            {make_tuple(Analysis::EE, Year::Run2p1, Q2Bin::Low), "cat_wMVA_lowcen > 0.950 && cat_wMVA_PR_lowcen > 0.40"},
            {make_tuple(Analysis::EE, Year::Run2p1, Q2Bin::Central), "cat_wMVA_lowcen > 0.975 && cat_wMVA_PR_lowcen > 0.15"},
            {make_tuple(Analysis::EE, Year::Run2p1, Q2Bin::High), "cat_wMVA_high > 0."},
            {make_tuple(Analysis::EE, Year::Run2p1, Q2Bin::JPsi), "cat_wMVA_lowcen > 0.05"},
            {make_tuple(Analysis::EE, Year::Run2p1, Q2Bin::Psi), "cat_wMVA_lowcen > 0.30"},

            {make_tuple(Analysis::EE, Year::Run2p2, Q2Bin::Low), "cat_wMVA_lowcen > 0. && cat_wMVA_PR_lowcen > 0."},
            {make_tuple(Analysis::EE, Year::Run2p2, Q2Bin::Central), "cat_wMVA_lowcen > 0. && cat_wMVA_PR_lowcen > 0."},
            {make_tuple(Analysis::EE, Year::Run2p2, Q2Bin::High), "cat_wMVA_high > 0."},
            {make_tuple(Analysis::EE, Year::Run2p2, Q2Bin::JPsi), "cat_wMVA_lowcen > 0.05"},
            {make_tuple(Analysis::EE, Year::Run2p2, Q2Bin::Psi), "cat_wMVA_lowcen > 0."}};

        const map< tuple< Analysis, Year, Q2Bin >, TString > CAT_LOOSE = {   //
            //{make_tuple(Analysis::EE, Year::Run1, Q2Bin::Low), "cat_wMVA_lowcen > 0.900 && cat_wMVA_PR_lowcen > 0.30"},   // -1.0%
            {make_tuple(Analysis::EE, Year::Run1, Q2Bin::Low), "cat_wMVA_lowcen > 0.875 && cat_wMVA_PR_lowcen > 0.25"},   // -2.7%
            //{make_tuple(Analysis::EE, Year::Run1, Q2Bin::Central), "cat_wMVA_lowcen > 0.925 && cat_wMVA_PR_lowcen > 0.30"},   // -2.1%
            {make_tuple(Analysis::EE, Year::Run1, Q2Bin::Central), "cat_wMVA_lowcen > 0.900 && cat_wMVA_PR_lowcen > 0.25"},   // -2.3%

            //{make_tuple(Analysis::EE, Year::Run2p1, Q2Bin::Low), "cat_wMVA_lowcen > 0.925 && cat_wMVA_PR_lowcen > 0.35"},   // -0.7%
            {make_tuple(Analysis::EE, Year::Run2p1, Q2Bin::Low), "cat_wMVA_lowcen > 0.900 && cat_wMVA_PR_lowcen > 0.30"},   // -1.2%
            //{make_tuple(Analysis::EE, Year::Run2p1, Q2Bin::Central), "cat_wMVA_lowcen > 0.950 && cat_wMVA_PR_lowcen > 0.10"}};   // -1.0%
            {make_tuple(Analysis::EE, Year::Run2p1, Q2Bin::Central), "cat_wMVA_lowcen > 0.925 && cat_wMVA_PR_lowcen > 0.05"}};   // -3.6%

        const map< tuple< Analysis, Year, Q2Bin >, TString > CAT_OLD = {   //
            {make_tuple(Analysis::MM, Year::Run1, Q2Bin::Low), "cat_wMVA_lowcen > 0.80"},
            {make_tuple(Analysis::MM, Year::Run1, Q2Bin::Central), "cat_wMVA_lowcen > 0.75"},
            {make_tuple(Analysis::MM, Year::Run1, Q2Bin::JPsi), "cat_wMVA_lowcen > 0.05"},
            {make_tuple(Analysis::MM, Year::Run1, Q2Bin::Psi), "cat_wMVA_lowcen > 0.02"},

            {make_tuple(Analysis::MM, Year::Run2p1, Q2Bin::Low), "cat_wMVA_lowcen > 0.95"},
            {make_tuple(Analysis::MM, Year::Run2p1, Q2Bin::Central), "cat_wMVA_lowcen > 0.95"},
            {make_tuple(Analysis::MM, Year::Run2p1, Q2Bin::JPsi), "cat_wMVA_lowcen > 0.05"},
            {make_tuple(Analysis::MM, Year::Run2p1, Q2Bin::Psi), "cat_wMVA_lowcen > 0.02"},

            {make_tuple(Analysis::MM, Year::Run2p2, Q2Bin::JPsi), "cat_wMVA_lowcen > 0.05"},

            {make_tuple(Analysis::EE, Year::Run1, Q2Bin::Low), "cat_wMVA_lowcen > 0.9 && cat_wMVA_PR_lowcen > 0.4"},
            {make_tuple(Analysis::EE, Year::Run1, Q2Bin::Central), "cat_wMVA_lowcen > 0.9 && cat_wMVA_PR_lowcen > 0.5"},
            {make_tuple(Analysis::EE, Year::Run1, Q2Bin::JPsi), "cat_wMVA_lowcen > 0.05"},
            {make_tuple(Analysis::EE, Year::Run1, Q2Bin::Psi), "cat_wMVA_lowcen > 0.30"},

            {make_tuple(Analysis::EE, Year::Run2p1, Q2Bin::Low), "cat_wMVA_lowcen > 0.9 && cat_wMVA_PR_lowcen > 0.4"},
            {make_tuple(Analysis::EE, Year::Run2p1, Q2Bin::Central), "cat_wMVA_lowcen > 0.9 && cat_wMVA_PR_lowcen > 0.5"},
            {make_tuple(Analysis::EE, Year::Run2p1, Q2Bin::JPsi), "cat_wMVA_lowcen > 0.05"},
            {make_tuple(Analysis::EE, Year::Run2p1, Q2Bin::Psi), "cat_wMVA_lowcen > 0.30"},

            {make_tuple(Analysis::EE, Year::Run2p2, Q2Bin::JPsi), "cat_wMVA_lowcen > 0.05"}};
        */
    }   // namespace MVA

}   // namespace CutDefRK

#endif
