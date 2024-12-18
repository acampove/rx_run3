#ifndef CUTDEFRKST_HPP
#define CUTDEFRKST_HPP

#include "ConstDef.hpp"

#include "CutDefRX.hpp"

#include "TCut.h"

/**
 * \namespace CutDefRKst
 * \brief RKst cut definitions
 **/
namespace CutDefRKst {

    /**
     * \namespace CutDefRKst::Trigger
     * \brief RKst Trigger cut definitions
     **/
    namespace Trigger {

        const TCut L0H = "(K_L0HadronDecision_TOS || Pi_L0HadronDecision_TOS) && (K_L0Calo_HCAL_region >= 0 && Pi_L0Calo_HCAL_region >= 0 && Kst_PT > 3000)";

    }   // namespace Trigger

    /**
     * \namespace CutDefRKst::Mass
     * \brief RKst Mass cut definitions
     **/
    namespace Mass {

        const TCut Kst      = (TCut)((TString) Form("TMath::Abs(Kst_M - %f) < 100", PDG::Mass::Kst));
        const TCut Kst_Port = (TCut)((TString) Form("TMath::Abs(Kst_TRUE_M_FromMCDT - %f) < 100", PDG::Mass::Kst));

        const TCut KstLoose = (TCut)((TString) Form("TMath::Abs(Kst_M - %f) < 200", PDG::Mass::Kst));

        const TCut Q2Low     = "TMath::Sq(JPs_M/1000) > 0.1 && TMath::Sq(JPs_M/1000) < 1.1";

        const TCut Q2LowTight = "TMath::Sq(JPs_M/1000) > 0.1 && TMath::Sq(JPs_M/1000) < 0.98"; //Used for Muon Branching Fraction Measurement
        
        const TCut Q2Central = "TMath::Sq(JPs_M/1000) > 1.1 && TMath::Sq(JPs_M/1000) < 6";
        const TCut Q2HighMM  = "TMath::Sq(JPs_M/1000) > 15 && TMath::Sq(JPs_M/1000) < 19";
        const TCut Q2HighEE  = "TMath::Sq(JPs_TRACK_M/1000) > 14";   // && TMath::Sq(JPs_TRACK_M/1000) < 19

        const TCut Q2Gamma = "TMath::Sq(JPs_M/1000) > 0 && TMath::Sq(JPs_M/1000) < TMath::Sq(4/1000)";

        // const TCut CombBVetoJPs = (TCut)((TString) Form("TMath::Abs({HEAD}_DTF_JPs_M - %f) > 100", PDG::Mass::Bd));
        // const TCut CombBVetoPsi = (TCut)((TString) Form("TMath::Abs({HEAD}_DTF_Psi_M - %f) > 100", PDG::Mass::Bd));
        const TCut CombBVetoJPs = (TCut)((TString) Form("TMath::Abs({HEAD}_DTF_JPs_M - %f) > 50", PDG::Mass::Bd));
        const TCut CombBVetoPsi = (TCut)((TString) Form("TMath::Abs({HEAD}_DTF_Psi_M - %f) > 50", PDG::Mass::Bd));


    }   // namespace Mass

    /**
     * \namespace CutDefRKst::Quality
     * \brief RKst Quality cut definitions
     **/
    namespace Quality {

        const TCut trgAccL0H = "TMath::Min(K_L0Calo_HCAL_region,Pi_L0Calo_HCAL_region) >= 0";

        // const TCut etaAccH = "TMath::Min(K_ETA,Pi_ETA) > 2.0 && TMath::Max(K_ETA,Pi_ETA) < 4.0";

        const TCut inAccH = "K_InAccHcal==1 && Pi_InAccHcal==1";

        const TCut pidAccH = "K_hasRich==1 && Pi_hasRich==1 && K_InAccMuon==1 && Pi_InAccMuon==1";

        const TCut pidCalibAccH = "K_PT > 250 && Pi_PT > 250 && Pi_P > 2000 && K_P > 2000";

        const TCut stripAccH = "Kst_PT > 500 && TMath::Min(K_IPCHI2_OWNPV,Pi_IPCHI2_OWNPV) > 9 ";

        const TCut trackAccH = "TMath::Max(K_TRACK_CHI2NDOF,Pi_TRACK_CHI2NDOF) < 3 && TMath::Max(K_TRACK_GhostProb,Pi_TRACK_GhostProb) < 0.4";

        const TCut openingAngle = "L1L2_OA > 0.0005 && H1H2_OA > 0.0005 && L1H1_OA > 0.0005 && L1H2_OA > 0.0005 && L2H1_OA > 0.0005 && L2H2_OA > 0.0005";

        const TCut qualityMM = CutDefRX::Quality::inAccM && pidAccH && CutDefRX::Quality::pidAccM && pidCalibAccH && CutDefRX::Quality::pidCalibAccM && stripAccH && CutDefRX::Quality::stripAccM && trackAccH && CutDefRX::Quality::trackAccM && openingAngle;

        const TCut qualityMM_noIsMuon = CutDefRX::Quality::inAccM && pidAccH && CutDefRX::Quality::pidAccM_noIsMuon && pidCalibAccH && CutDefRX::Quality::pidCalibAccM && stripAccH && CutDefRX::Quality::stripAccM && trackAccH && CutDefRX::Quality::trackAccM && openingAngle;

        const TCut qualityEE = CutDefRX::Quality::inAccE && CutDefRX::Quality::trgAccL0E_ECALregion && CutDefRX::Quality::inCaloHoleE && pidAccH && CutDefRX::Quality::pidAccE && pidCalibAccH && CutDefRX::Quality::pidCalibAccE && stripAccH && CutDefRX::Quality::stripAccE && trackAccH && CutDefRX::Quality::trackAccE && openingAngle;
        const TCut qualityME = CutDefRX::Quality::inAccME && CutDefRX::Quality::trgAccL0E_ECALregion_E2 && !CutDefRX::Quality::inCaloHoleE2 && pidAccH && CutDefRX::Quality::pidAccME && pidCalibAccH && CutDefRX::Quality::pidCalibAccME && stripAccH && CutDefRX::Quality::stripAccME && trackAccH && CutDefRX::Quality::trackAccME && openingAngle;

    }   // namespace Quality

    /**
     * \namespace CutDefRKst::Background
     * \brief RKst Background cut definitions
     **/
    namespace Background {

        // B0 ==> ^(J/psi(1S) ==> ^l+ ^l-) ^(K*(892)0 ==> ^K+ ^pi-)
        // DecayTreeTuple          L1  L2                  K   Pi
        // TupleToolSubMass        1   0                   2   3

        // Bu2KLL
        const TCut Bu2K_SS = "B0_M012 < 5100";
        const TCut Bu2K    = "TMath::Max(B0_M012,B0_M013_Subst3_pi2K) < 5100";
        
        // Bs2PhiLL
        const TCut Bs2Phi     = "B0_M23_Subst3_pi2K > 1040";
        const TCut Bs2Phi_PID = "!(B0_M23_Subst3_pi2K < 1040 && Pi_{TUNE}_ProbNNpi < 0.8)";

        // Bd2KstG
        const TCut Bd2KstG = "JPs_ENDVERTEX_ZERR < 30";

        // Lb2pKLL
        // const TCut Lb2pK = "1";

        // Bd2DX
        const TCut Bd2DX         = "TMath::Abs(TMath::Cos(B0_ThetaL_custom)) < 0.8";
        const TCut Bd2DXMass     = "B0_M023 > 1780."; //m(K+ e- pi-)
        const TCut Bs2DXMass     = "B0_M123 > 1880."; //m(K+ e+ pi-)

        // Emulate SL cuts as in RK 
        /* Variant of SL cuts in Central q2 for electrons 
            Option 1  : -SLCTL
            Option 2  : -SLCTLMKE
        */
        const TCut SLCTL    = "TMath::Cos(B0_ThetaL_custom) < 0.6";
        const TCut SLCTLMKE = "B0_M02 > 1885";
        
        const TCut Bd2DPiMNu     = (TCut)((TString) Form("TMath::Abs(B0_M02_Subst0_mu2pi - %f) > 30.", PDG::Mass::D0));
        const TCut Bd2DPiMNu_PID = (TCut)((TString) Form("!(TMath::Abs(B0_M02_Subst0_mu2pi - %f) < 30 && M2_{TUNE}_ProbNNmu < 0.8)", PDG::Mass::D0));
        const TCut Bd2DMNu       = (TCut)((TString) Form("TMath::Abs(B0_M023_Subst0_mu2pi - %f) > 30.", PDG::Mass::Dplus));
        const TCut Bd2DMNu_PID   = (TCut)((TString) Form("!(TMath::Abs(B0_M023_Subst0_mu2pi - %f) < 30 && M2_{TUNE}_ProbNNmu < 0.8)", PDG::Mass::Dplus));
        const TCut Bd2DPiENu     = (TCut)((TString) Form("TMath::Abs(B0_TRACK_M12_Subst1_e2pi - %f) > 30.", PDG::Mass::D0));
        const TCut Bd2DPiENu_PID = (TCut)((TString) Form("!(TMath::Abs(B0_TRACK_M12_Subst1_e2pi - %f) < 30 && E2_{TUNE}_ProbNNe < 0.8)", PDG::Mass::D0));
        const TCut Bd2DENu       = (TCut)((TString) Form("TMath::Abs(TMath::Sqrt(TMath::Sq(K_PE+Pi_PE+TMath::Sqrt(TMath::Sq(%f)+TMath::Sq(E2_TRACK_PX)+TMath::Sq(E2_TRACK_PY)+TMath::Sq(E2_TRACK_PZ))) - TMath::Sq(K_PX+Pi_PX+E2_TRACK_PX) - TMath::Sq(K_PY+Pi_PY+E2_TRACK_PY) - TMath::Sq(K_PZ+Pi_PZ+E2_TRACK_PZ)) - %f) > 30.", PDG::Mass::Pi, PDG::Mass::Dplus));
        const TCut Bd2DENu_PID   = (TCut)((TString) Form("!(TMath::Abs(TMath::Sqrt(TMath::Sq(K_PE+Pi_PE+TMath::Sqrt(TMath::Sq(%f)+TMath::Sq(E2_TRACK_PX)+TMath::Sq(E2_TRACK_PY)+TMath::Sq(E2_TRACK_PZ))) - TMath::Sq(K_PX+Pi_PX+E2_TRACK_PX) - TMath::Sq(K_PY+Pi_PY+E2_TRACK_PY) - TMath::Sq(K_PZ+Pi_PZ+E2_TRACK_PZ)) - %f) < 30 && E2_{TUNE}_ProbNNe < 0.8)", PDG::Mass::Pi, PDG::Mass::Dplus));

        // MisID
        const TCut misIDJPsMM     = (TCut)((TString) Form("TMath::Abs(B0_M02_Subst2_K2mu - %f) > 60 && TMath::Abs(B0_M13_Subst3_pi2mu - %f) > 60", PDG::Mass::JPs, PDG::Mass::JPs));
        const TCut misIDJPsMM_PID = (TCut)((TString) Form("!((TMath::Abs(B0_M02_Subst2_K2mu - %f) < 60 && M1_{TUNE}_ProbNNmu < 0.8) || (TMath::Abs(B0_M13_Subst3_pi2mu - %f) < 60 && M2_{TUNE}_ProbNNmu < 0.8))", PDG::Mass::JPs, PDG::Mass::JPs));
        const TCut misIDPsiMM     = (TCut)((TString) Form("TMath::Abs(B0_M02_Subst2_K2mu - %f) > 60 && TMath::Abs(B0_M13_Subst3_pi2mu - %f) > 60", PDG::Mass::Psi, PDG::Mass::Psi));
        const TCut misIDPsiMM_PID = (TCut)((TString) Form("!((TMath::Abs(B0_M02_Subst2_K2mu - %f) < 60 && M1_{TUNE}_ProbNNmu < 0.8) || (TMath::Abs(B0_M13_Subst3_pi2mu - %f) < 60 && M2_{TUNE}_ProbNNmu < 0.8))", PDG::Mass::Psi, PDG::Mass::Psi));
        const TCut misIDJPsEE     = (TCut)((TString) Form("TMath::Abs(B0_TRACK_M_Subst_Kl2lK_DTF_JPs - %f) > 60 && TMath::Abs(B0_TRACK_M_Subst_lpi2pil_DTF_JPs - %f) > 60", PDG::Mass::Bd, PDG::Mass::Bd));
        const TCut misIDJPsEE_PID = (TCut)((TString) Form("!((TMath::Abs(B0_TRACK_M_Subst_Kl2lK_DTF_JPs - %f) < 60 && E1_{TUNE}_ProbNNe < 0.8) || (TMath::Abs(B0_TRACK_M_Subst_lpi2pil_DTF_JPs - %f) < 60 && E2_{TUNE}_ProbNNe < 0.8))", PDG::Mass::Bd, PDG::Mass::Bd));
        const TCut misIDPsiEE     = (TCut)((TString) Form("TMath::Abs(B0_TRACK_M_Subst_Kl2lK_DTF_Psi - %f) > 60 && TMath::Abs(B0_TRACK_M_Subst_lpi2pil_DTF_Psi - %f) > 60", PDG::Mass::Bd, PDG::Mass::Bd));
        const TCut misIDPsiEE_PID = (TCut)((TString) Form("!((TMath::Abs(B0_TRACK_M_Subst_Kl2lK_DTF_Psi - %f) < 60 && E1_{TUNE}_ProbNNe < 0.8) || (TMath::Abs(B0_TRACK_M_Subst_lpi2pil_DTF_Psi - %f) < 60 && E2_{TUNE}_ProbNNe < 0.8))", PDG::Mass::Bd, PDG::Mass::Bd));
        const TCut misIDKst       = (TCut)((TString) Form("TMath::Abs(B0_TRACK_M03_Subst0_e2K - %f) > 80. && TMath::Abs(B0_TRACK_M12_Subst1_e2pi - %f) > 80.", PDG::Mass::Kst, PDG::Mass::Kst));
        const TCut misIDKst_PID   = (TCut)((TString) Form("!((TMath::Abs(B0_TRACK_M03_Subst0_e2K - %f) < 80 && E1_{TUNE}_ProbNNe < 0.8) || (TMath::Abs(B0_TRACK_M12_Subst1_e2pi - %f) < 80 && E2_{TUNE}_ProbNNe < 0.8))", PDG::Mass::Kst, PDG::Mass::Kst));
        const TCut misIDHad       = (TCut)((TString) Form("TMath::Abs(B0_M23_Subst23_Kpi2piK - %f) > 50", PDG::Mass::Kst));

        // Peaking
        const TCut peakingBkgMM     = Bu2K && Bs2Phi && misIDJPsMM && misIDPsiMM;
        const TCut peakingBkgMM_PID = Bu2K && Bs2Phi_PID && misIDJPsMM_PID && misIDPsiMM_PID;
        const TCut peakingBkgEE     = Bu2K && Bs2Phi && misIDJPsEE && misIDPsiEE;
        const TCut peakingBkgEE_PID = Bu2K && Bs2Phi_PID && misIDJPsEE_PID && misIDPsiEE_PID;
     
        const TCut peakingBkgMM_noOverReco     = Bs2Phi && misIDJPsMM && misIDPsiMM;
        const TCut peakingBkgMM_PID_noOverReco = Bs2Phi_PID && misIDJPsMM_PID && misIDPsiMM_PID;
        const TCut peakingBkgEE_noOverReco     = Bs2Phi && misIDJPsEE && misIDPsiEE;
        const TCut peakingBkgEE_PID_noOverReco = Bs2Phi_PID && misIDJPsEE_PID && misIDPsiEE_PID;

        //Peaking sub-set for SS data 
        const TCut peakingBkgMM_SSData     = Bu2K_SS;
        const TCut peakingBkgEE_SSData     = Bu2K_SS;


        // Semileptonic
        const TCut semilepBkgMM     = Bd2DMNu && Bd2DPiMNu;
        const TCut semilepBkgMM_PID = Bd2DMNu_PID && Bd2DPiMNu_PID;
        const TCut semilepBkgEE     = Bd2DENu && Bd2DPiENu;
        const TCut semilepBkgEE_PID = Bd2DENu_PID && Bd2DPiENu_PID;
        // Semileptonic sub-set for SS data 
        const TCut semilepBkgEE_SSData         = Bd2DENu;
        const TCut semilepBkgEE_SSData_PID     = Bd2DENu_PID;

    }   // namespace Background

    /**
     * \namespace CutDefRKst::PID
     * \brief RKst PID cut definitions
     **/
    namespace PID {

        const TCut pidStripH = "K_PIDK > -5";

        const TCut dllK  = "K_PIDK > 0";
        const TCut dllE  = "TMath::Min(E1_PIDe, E2_PIDe) > 2";
        const TCut dllE2 = "E2_PIDe > 2";

        const TCut probnnK  = "K_{TUNE}_ProbNNk * (1 - K_{TUNE}_ProbNNp) > 0.05";
        const TCut probnnPi = "Pi_{TUNE}_ProbNNpi * (1 - Pi_{TUNE}_ProbNNk) * (1 - Pi_{TUNE}_ProbNNp) > 0.1";
        const TCut probnnM  = "TMath::Min(M1_{TUNE}_ProbNNmu,M2_{TUNE}_ProbNNmu) > 0.2";
        const TCut probnnE  = "TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.2";
        const TCut probnnME = "TMath::Min(M1_{TUNE}_ProbNNmu,E2_{TUNE}_ProbNNe) > 0.2";

        const TCut pidE1E2 = CutDefRX::PID::pidStripE && probnnE && dllE;
        const TCut pidMM   = (pidStripH && probnnK && dllK) && probnnPi && probnnM;
        const TCut pidEE   = (pidStripH && probnnK && dllK) && probnnPi && pidE1E2;
        const TCut pidME   = (pidStripH && probnnK && dllK) && probnnPi && (CutDefRX::PID::pidStripME && probnnME && dllE2);

        //Variation on electron mode for PIDe>3 ONLY
        const TCut dllE_PID3    = "TMath::Min(E1_PIDe, E2_PIDe) > 3";
        const TCut pidE1E2_PID3 = CutDefRX::PID::pidStripE && dllE_PID3;
        const TCut dllE2_PID3   = "E2_PIDe > 3";
        const TCut pidEE_PID3   = (pidStripH && probnnK && dllK) && probnnPi && pidE1E2_PID3;
        //Variation on electron mode for PIDe>3 & ProbNNe>0.4
        const TCut probnnE4  = "TMath::Min(E1_{TUNE}_ProbNNe,E2_{TUNE}_ProbNNe) > 0.4";
        const TCut pidE1E2_PID3_Prob4 = CutDefRX::PID::pidStripE && dllE_PID3 && probnnE4;;
        const TCut pidEE_PID3_Prob4   = (pidStripH && probnnK && dllK) && probnnPi && pidE1E2_PID3_Prob4;

        //Variation on electron mode for PIDe>2 ONLY
        const TCut dllE_PID2    = "TMath::Min(E1_PIDe, E2_PIDe) > 2";
        const TCut pidE1E2_PID2 = CutDefRX::PID::pidStripE && dllE_PID2;
        const TCut dllE2_PID2   = "E2_PIDe > 2";
        const TCut pidEE_PID2   = (pidStripH && probnnK && dllK) && probnnPi && pidE1E2_PID2;

        //Variation on electron mode for PIDe>5 ONLY
        const TCut dllE_PID5    = "TMath::Min(E1_PIDe, E2_PIDe) > 5";
        const TCut pidE1E2_PID5 = CutDefRX::PID::pidStripE && dllE_PID5;
        const TCut pidEE_PID5   = (pidStripH && probnnK && dllK) && probnnPi && pidE1E2_PID5;
        
        //Variation on electron mode for PIDe>7 ONLY
        const TCut dllE_PID7    = "TMath::Min(E1_PIDe, E2_PIDe) > 7";
        const TCut pidE1E2_PID7 = CutDefRX::PID::pidStripE && dllE_PID7;
        const TCut pidEE_PID7   = (pidStripH && probnnK && dllK) && probnnPi && pidE1E2_PID7;
    }   // namespace PID

    /**
     * \namespace CutDefRKst::MVA
     * \brief RKst MVA cut definitions
     **/
    namespace MVA {

        const map< tuple< Analysis, Year, Q2Bin >, TString > CAT = {   //
            {make_tuple(Analysis::MM, Year::Run1, Q2Bin::JPsi), "cat_wMVA_lowcen > 0.05"},
            {make_tuple(Analysis::MM, Year::Run2p1, Q2Bin::JPsi), "cat_wMVA_lowcen > 0.05"},
            {make_tuple(Analysis::MM, Year::Run2p2, Q2Bin::JPsi), "cat_wMVA_lowcen > 0.05"},

            {make_tuple(Analysis::MM, Year::Run1, Q2Bin::Psi), "cat_wMVA_lowcen > 0.05"},
            {make_tuple(Analysis::MM, Year::Run2p1, Q2Bin::Psi), "cat_wMVA_lowcen > 0.05"},
            {make_tuple(Analysis::MM, Year::Run2p2, Q2Bin::Psi), "cat_wMVA_lowcen > 0.05"},

            {make_tuple(Analysis::MM, Year::Run1, Q2Bin::Low), "cat_wMVA_lowcen > 0.29"},
            {make_tuple(Analysis::MM, Year::Run2p1, Q2Bin::Low), "cat_wMVA_lowcen > 0.54"},
            {make_tuple(Analysis::MM, Year::Run2p2, Q2Bin::Low), "cat_wMVA_lowcen > 0.55"},

            {make_tuple(Analysis::MM, Year::Run1, Q2Bin::Central), "cat_wMVA_lowcen > 0.63"},
            {make_tuple(Analysis::MM, Year::Run2p1, Q2Bin::Central), "cat_wMVA_lowcen > 0.77"},
            {make_tuple(Analysis::MM, Year::Run2p2, Q2Bin::Central), "cat_wMVA_lowcen > 0.64"},

            {make_tuple(Analysis::MM, Year::Run1, Q2Bin::High), "cat_wMVA_high > 0."},
            {make_tuple(Analysis::MM, Year::Run2p1, Q2Bin::High), "cat_wMVA_high > 0."},
            {make_tuple(Analysis::MM, Year::Run2p2, Q2Bin::High), "cat_wMVA_high > 0."},

            {make_tuple(Analysis::EE, Year::Run1, Q2Bin::JPsi), "cat_wMVA_lowcen > 0.2 && cat_wMVA_PR_lowcen > 0.05"},
            {make_tuple(Analysis::EE, Year::Run2p1, Q2Bin::JPsi), "cat_wMVA_lowcen > 0.2 && cat_wMVA_PR_lowcen > 0.05"},
            {make_tuple(Analysis::EE, Year::Run2p2, Q2Bin::JPsi), "cat_wMVA_lowcen > 0.2 && cat_wMVA_PR_lowcen > 0.05"},

            {make_tuple(Analysis::EE, Year::Run1, Q2Bin::Psi), "cat_wMVA_lowcen > 0.30"},
            {make_tuple(Analysis::EE, Year::Run2p1, Q2Bin::Psi), "cat_wMVA_lowcen > 0.55"},
            {make_tuple(Analysis::EE, Year::Run2p2, Q2Bin::Psi), "cat_wMVA_lowcen > 0.67"},

            {make_tuple(Analysis::EE, Year::Run1, Q2Bin::Low), "cat_wMVA_lowcen > 0.50 && cat_wMVA_PR_lowcen > 0.50"},
            {make_tuple(Analysis::EE, Year::Run2p1, Q2Bin::Low), "cat_wMVA_lowcen > 0.50 && cat_wMVA_PR_lowcen > 0.50"},
            {make_tuple(Analysis::EE, Year::Run2p2, Q2Bin::Low), "cat_wMVA_lowcen > 0.50 && cat_wMVA_PR_lowcen > 0.50"},

            {make_tuple(Analysis::EE, Year::Run1, Q2Bin::Central), "cat_wMVA_lowcen > 0.90 && cat_wMVA_PR_lowcen > 0.40"},
            {make_tuple(Analysis::EE, Year::Run2p1, Q2Bin::Central), "cat_wMVA_lowcen > 0.90 && cat_wMVA_PR_lowcen > 0.40"},
            {make_tuple(Analysis::EE, Year::Run2p2, Q2Bin::Central), "cat_wMVA_lowcen > 0.90 && cat_wMVA_PR_lowcen > 0.40"},

            {make_tuple(Analysis::EE, Year::Run1, Q2Bin::High), "cat_wMVA_high > 0."},
            {make_tuple(Analysis::EE, Year::Run2p1, Q2Bin::High), "cat_wMVA_high > 0."},
            {make_tuple(Analysis::EE, Year::Run2p2, Q2Bin::High), "cat_wMVA_high > 0."}};
        /*
        const map< tuple< Analysis, Year, Q2Bin >, TString > CAT = {   //
            {make_tuple(Analysis::MM, Year::Run1, Q2Bin::Low), "cat_wMVA_lowcen > 0.48"},
            {make_tuple(Analysis::MM, Year::Run1, Q2Bin::Central), "cat_wMVA_lowcen > 0.36"},
            {make_tuple(Analysis::MM, Year::Run1, Q2Bin::High), "cat_wMVA_high > 0."},
            {make_tuple(Analysis::MM, Year::Run1, Q2Bin::JPsi), "cat_wMVA_lowcen > 0.05"},
            {make_tuple(Analysis::MM, Year::Run1, Q2Bin::Psi), "cat_wMVA_lowcen > 0.05"},

            {make_tuple(Analysis::MM, Year::Run2p1, Q2Bin::Low), "cat_wMVA_lowcen > 0.70"},
            {make_tuple(Analysis::MM, Year::Run2p1, Q2Bin::Central), "cat_wMVA_lowcen > 0.58"},
            {make_tuple(Analysis::MM, Year::Run2p1, Q2Bin::High), "cat_wMVA_high > 0."},
            {make_tuple(Analysis::MM, Year::Run2p1, Q2Bin::JPsi), "cat_wMVA_lowcen > 0.05"},
            {make_tuple(Analysis::MM, Year::Run2p1, Q2Bin::Psi), "cat_wMVA_lowcen > 0.05"},

            {make_tuple(Analysis::MM, Year::Run2p2, Q2Bin::Low), "cat_wMVA_lowcen > 0."},
            {make_tuple(Analysis::MM, Year::Run2p2, Q2Bin::Central), "cat_wMVA_lowcen > 0."},
            {make_tuple(Analysis::MM, Year::Run2p2, Q2Bin::High), "cat_wMVA_high > 0."},
            {make_tuple(Analysis::MM, Year::Run2p2, Q2Bin::JPsi), "cat_wMVA_lowcen > 0.05"},
            {make_tuple(Analysis::MM, Year::Run2p2, Q2Bin::Psi), "cat_wMVA_lowcen > 0."},

            {make_tuple(Analysis::EE, Year::Run1, Q2Bin::Low), "cat_wMVA_lowcen > 0.70 && cat_wMVA_PR_lowcen > 0.50"},
            {make_tuple(Analysis::EE, Year::Run1, Q2Bin::Central), "cat_wMVA_lowcen > 0.95 && cat_wMVA_PR_lowcen > 0.40"},
            {make_tuple(Analysis::EE, Year::Run1, Q2Bin::High), "cat_wMVA_high > 0."},
            {make_tuple(Analysis::EE, Year::Run1, Q2Bin::JPsi), "cat_wMVA_lowcen > 0.05"},
            {make_tuple(Analysis::EE, Year::Run1, Q2Bin::Psi), "cat_wMVA_lowcen > 0.05"},

            {make_tuple(Analysis::EE, Year::Run2p1, Q2Bin::Low), "cat_wMVA_lowcen > 0.90 && cat_wMVA_PR_lowcen > 0.25"},
            {make_tuple(Analysis::EE, Year::Run2p1, Q2Bin::Central), "cat_wMVA_lowcen > 0.95 && cat_wMVA_PR_lowcen > 0.55"},
            {make_tuple(Analysis::EE, Year::Run2p1, Q2Bin::High), "cat_wMVA_high > 0."},
            {make_tuple(Analysis::EE, Year::Run2p1, Q2Bin::JPsi), "cat_wMVA_lowcen > 0.05"},
            {make_tuple(Analysis::EE, Year::Run2p1, Q2Bin::Psi), "cat_wMVA_lowcen > 0.05"},

            {make_tuple(Analysis::EE, Year::Run2p2, Q2Bin::Low), "cat_wMVA_lowcen > 0. && cat_wMVA_PR_lowcen > 0."},
            {make_tuple(Analysis::EE, Year::Run2p2, Q2Bin::Central), "cat_wMVA_lowcen > 0. && cat_wMVA_PR_lowcen > 0."},
            {make_tuple(Analysis::EE, Year::Run2p2, Q2Bin::High), "cat_wMVA_high > 0."},
            {make_tuple(Analysis::EE, Year::Run2p2, Q2Bin::JPsi), "cat_wMVA_lowcen > 0.05"},
            {make_tuple(Analysis::EE, Year::Run2p2, Q2Bin::Psi), "cat_wMVA_lowcen > 0."}};

        const map< tuple< Analysis, Year, Q2Bin >, TString > CAT_LOOSE = {   //
            //{make_tuple(Analysis::EE, Year::Run1, Q2Bin::Low), "cat_wMVA_lowcen > 0.60 && cat_wMVA_PR_lowcen > 0.45"},   // -2.1%
            {make_tuple(Analysis::EE, Year::Run1, Q2Bin::Low), "cat_wMVA_lowcen > 0.50 && cat_wMVA_PR_lowcen > 0.40"},   // -3.2%
            //{make_tuple(Analysis::EE, Year::Run1, Q2Bin::Central), "cat_wMVA_lowcen > 0.925 && cat_wMVA_PR_lowcen > 0.35"},   // -2.3%
            {make_tuple(Analysis::EE, Year::Run1, Q2Bin::Central), "cat_wMVA_lowcen > 0.900 && cat_wMVA_PR_lowcen > 0.30"},   // -4.1%

            //{make_tuple(Analysis::EE, Year::Run2p1, Q2Bin::Low), "cat_wMVA_lowcen > 0.80 && cat_wMVA_PR_lowcen > 0.20"},   // -2.1%
            {make_tuple(Analysis::EE, Year::Run2p1, Q2Bin::Low), "cat_wMVA_lowcen > 0.70 && cat_wMVA_PR_lowcen > 0.15"},   // -3.3%
            //{make_tuple(Analysis::EE, Year::Run2p1, Q2Bin::Central), "cat_wMVA_lowcen > 0.925 && cat_wMVA_PR_lowcen > 0.50"}};   // -2.0%
            {make_tuple(Analysis::EE, Year::Run2p1, Q2Bin::Central), "cat_wMVA_lowcen > 0.900 && cat_wMVA_PR_lowcen > 0.45"}};   // -2.7%

        const map< tuple< Analysis, Year, Q2Bin >, TString > CAT_OLD = {   //
            {make_tuple(Analysis::MM, Year::Run1, Q2Bin::Low), "cat_wMVA_lowcen > 0.45"},
            {make_tuple(Analysis::MM, Year::Run1, Q2Bin::Central), "cat_wMVA_lowcen > 0.35"},
            {make_tuple(Analysis::MM, Year::Run1, Q2Bin::JPsi), "cat_wMVA_lowcen > 0.05"},
            {make_tuple(Analysis::MM, Year::Run1, Q2Bin::Psi), "cat_wMVA_lowcen > 0.05"},

            {make_tuple(Analysis::MM, Year::Run2p1, Q2Bin::Low), "cat_wMVA_lowcen > 0.65"},
            {make_tuple(Analysis::MM, Year::Run2p1, Q2Bin::Central), "cat_wMVA_lowcen > 0.60"},
            {make_tuple(Analysis::MM, Year::Run2p1, Q2Bin::JPsi), "cat_wMVA_lowcen > 0.05"},
            {make_tuple(Analysis::MM, Year::Run2p1, Q2Bin::Psi), "cat_wMVA_lowcen > 0.05"},

            {make_tuple(Analysis::MM, Year::Run2p2, Q2Bin::JPsi), "cat_wMVA_lowcen > 0.05"},

            {make_tuple(Analysis::EE, Year::Run1, Q2Bin::Low), "cat_wMVA_lowcen > 0.8 && cat_wMVA_PR_lowcen > 0.4"},
            {make_tuple(Analysis::EE, Year::Run1, Q2Bin::Central), "cat_wMVA_lowcen > 0.8 && cat_wMVA_PR_lowcen > 0.6"},
            {make_tuple(Analysis::EE, Year::Run1, Q2Bin::JPsi), "cat_wMVA_lowcen > 0.05"},
            {make_tuple(Analysis::EE, Year::Run1, Q2Bin::Psi), "cat_wMVA_lowcen > 0.05"},

            {make_tuple(Analysis::EE, Year::Run2p1, Q2Bin::Low), "cat_wMVA_lowcen > 0.8 && cat_wMVA_PR_lowcen > 0.4"},
            {make_tuple(Analysis::EE, Year::Run2p1, Q2Bin::Central), "cat_wMVA_lowcen > 0.8 && cat_wMVA_PR_lowcen > 0.6"},
            {make_tuple(Analysis::EE, Year::Run2p1, Q2Bin::JPsi), "cat_wMVA_lowcen > 0.05"},
            {make_tuple(Analysis::EE, Year::Run2p1, Q2Bin::Psi), "cat_wMVA_lowcen > 0.05"},

            {make_tuple(Analysis::EE, Year::Run2p2, Q2Bin::JPsi), "cat_wMVA_lowcen > 0.05"}};
        */

    }   // namespace MVA

}   // namespace CutDefRKst

#endif
