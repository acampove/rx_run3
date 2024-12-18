#ifndef CUTDEFRPHI_HPP
#define CUTDEFRPHI_HPP

#include "ConstDef.hpp"

#include "CutDefRX.hpp"

#include "TCut.h"

/**
 * \namespace CutDefRPhi
 * \brief RPhi cut definitions
 **/
namespace CutDefRPhi {
    /**
     * \namespace CutDefRPhi::Trigger
     * \brief RPhi Trigger cut definitions
     **/
    namespace Trigger {
        const TCut L0H = "K1_L0HadronDecision_TOS || K2_L0HadronDecision_TOS";
    }   // namespace Trigger

    /**
     * \namespace CutDefRPhi::PID
     * \brief RPhi PID cut definitions
     **/
    namespace PID {
        const TCut wPID = "wPID";

        const TCut pidStripK = "TMath::Min(K1_PIDK, K2_PIDK) > -5";

        const TCut probnnK = "TMath::Min(K1_{TUNE}_ProbNNk, K2_{TUNE}_ProbNNk) > 0.1 && TMath::Max(K1_{TUNE}_ProbNNp * (1 - K1_{TUNE}_ProbNNk), K2_{TUNE}_ProbNNp * (1 - K1_{TUNE}_ProbNNk)) < 0.2";
        const TCut probnnM = "TMath::Min(M1_{TUNE}_ProbNNmu, M2_{TUNE}_ProbNNmu) > 0.1";
        const TCut probnnE = "TMath::Min(E1_{TUNE}_ProbNNe, E2_{TUNE}_ProbNNe) > 0.1";

        const TCut pidMM = (pidStripK && probnnK) && (CutDefRX::PID::pidStripM && probnnM);
        const TCut pidEE = (pidStripK && probnnK) && (CutDefRX::PID::pidStripE && probnnE);

        // loose PID cuts for MVA training
        const TCut probnnKL = "TMath::Min(K1_{TUNE}_ProbNNk, K2_{TUNE}_ProbNNk) > 0.05";
        const TCut probnnML = "TMath::Min(M1_{TUNE}_ProbNNmu, M2_{TUNE}_ProbNNmu) > 0.05";
        const TCut probnnEL = "TMath::Min(E1_{TUNE}_ProbNNe, E2_{TUNE}_ProbNNe) > 0.05";

        const TCut pidMML = (pidStripK && probnnKL) && (CutDefRX::PID::pidStripM && probnnML);
        const TCut pidEEL = (pidStripK && probnnKL) && (CutDefRX::PID::pidStripE && probnnEL);
    }   // namespace PID

    /**
     * \namespace CutDefRPhi::Mass
     * \brief RPhi Mass cut definitions
     **/
    namespace Mass {
        const TCut Phi = "TMath::Abs(Phi_M - 1019.461) < 12";
        // loose Phi cut for MVA training
        const TCut PhiL = "TMath::Abs(Phi_M - 1019.461) < 50";

        const TCut Q2Low     = "TMath::Sq(JPs_M/1000) > 0.1 && TMath::Sq(JPs_M/1000) < 1.1";
        const TCut Q2Central = "TMath::Sq(JPs_M/1000) > 1.1 && TMath::Sq(JPs_M/1000) < 6";
        const TCut Q2High    = "TMath::Sq(JPs_M/1000) > 15 && TMath::Sq(JPs_M/1000) < 19";
        const TCut Q2        = "TMath::Sq(JPs_M/1000) > 1 && TMath::Sq(JPs_M/1000) < 6";
        const TCut Q2Gamma   = "TMath::Sq(JPs_M/1000) > TMath::Sq(0/1000) && TMath::Sq(JPs_M/1000) < TMath::Sq(4/1000)";
    }   // namespace Mass

    /**
     * \namespace CutDefRPhi::Quality
     * \brief RPhi Quality cut definitions
     **/
    namespace Quality {
        const TCut GhostK = "TMath::Max(K1_TRACK_GhostProb, K2_TRACK_GhostProb) < 0.4";
        // const TCut GhostM     = "TMath::Max(M1_TRACK_GhostProb, M2_TRACK_GhostProb) < 0.3";
        // const TCut GhostE     = "TMath::Max(E1_TRACK_GhostProb, E2_TRACK_GhostProb) < 0.3";
        const TCut Ghost = GhostK;   // && GhostE && GhostM;

        // cuts due to di-electron bug
        const TCut TrackPTE = "TMath::Min(TMath::Sqrt(TMath::Power(E1_TRACK_PX, 2) + TMath::Power(E1_TRACK_PY, 2)), TMath::Sqrt(TMath::Power(E2_TRACK_PX, 2) + TMath::Power(E2_TRACK_PY, 2))) > 200";
        const TCut PTJPs    = "JPs_PT > 500";

        const TCut ECAL_ProjE = "(TMath::Abs(E1_L0Calo_ECAL_xProjection) > 363.6 || TMath::Abs(E1_L0Calo_ECAL_yProjection) > 282.6) && (TMath::Abs(E2_L0Calo_ECAL_xProjection) > 363.6 || TMath::Abs(E2_L0Calo_ECAL_yProjection) > 282.6)";
        // const TCut CAL_regK   = "TMath::Min(K1_L0Calo_HCAL_region, K2_L0Calo_HCAL_region) >= 0";
        // const TCut CAL_regE   = "TMath::Min(E1_L0Calo_ECAL_region, E2_L0Calo_ECAL_region) >= 0";

        const TCut qualMM = "1";                                                   // GhostM;
        const TCut qualEE = TrackPTE && PTJPs && CutDefRX::Quality::inCaloHoleE;   // ECAL_ProjE;   // && CAL_regE && GhostE;
    }                                                                              // namespace Quality

    /**
     * \namespace CutDefRPhi::Fiducial
     * \brief RPhi Fiducial cut definitions
     **/
    namespace Fiducial {
        const TCut K_Rich = "K1_hasRich==1 && K2_hasRich==1";
        const TCut E_Rich = "E1_hasRich==1 && E2_hasRich==1";

        const TCut E_Calo      = "E1_hasCalo==1 && E2_hasCalo==1";
        const TCut K_InAccMuon = "K1_InAccMuon==1 && K2_InAccMuon==1";

        const TCut E_PT = "TMath::Min(E1_PT, E2_PT) > 500";
        const TCut E_P  = "TMath::Min(E1_P, E2_P) > 3000";

        const TCut K_PT = "TMath::Min(K1_PT, K2_PT) > 250";
        const TCut K_P  = "TMath::Min(K1_P, K2_P) > 2000";
        const TCut M_PT = "TMath::Min(M1_PT, M2_PT) > 800";
        const TCut M_P  = "TMath::Min(M1_P, M2_P) > 3000";

        const TCut openingAngle = "L1L2_OA > 0.0005 && H1H2_OA > 0.0005 && L1H1_OA > 0.0005 && L1H2_OA > 0.0005 && L2H1_OA > 0.0005 && L2H2_OA > 0.0005";

        namespace Run1 {
            const TCut fidK  = K_Rich && K_InAccMuon && K_PT && K_P;
            const TCut fidMM = fidK && M_PT && M_P;
            const TCut fidEE = fidK && E_Rich && E_Calo && E_PT && E_P;
        }   // namespace Run1

        namespace Run2p1 {
            const TCut fidK  = K_Rich && K_InAccMuon && K_PT && K_P;
            const TCut fidMM = fidK && M_PT && M_P;
            const TCut fidEE = fidK && E_Rich && E_Calo && E_PT && E_P;
        }   // namespace Run2p1

        namespace Run2p2 {
            const TCut fidK  = K_Rich && K_InAccMuon && K_PT && K_P;
            const TCut fidMM = fidK && M_PT && M_P;
            const TCut fidEE = fidK && E_Rich && E_Calo && E_PT && E_P;
        }   // namespace Run2p2
    }       // namespace Fiducial

    /**
     * \namespace CutDefRPhi::Background
     * \brief RPhi Background cut definitions
     **/
    namespace Background {
        // Sideband
        const TCut upperSBMM = "Bs_DTF_M > 5400";
        const TCut upperSBEE = "Bs_DTF_M > 5600";
        const TCut lowerSB   = "Bs_DTF_JPs_M < 5150";

        // BKG vetos
        const TCut misIDJPsMM     = (TCut)((TString) Form("TMath::Abs(Bs_M02_Subst2_K2mu - %f) > 60 && TMath::Abs(Bs_M13_Subst3_K2mu - %f) > 60", PDG::Mass::JPs, PDG::Mass::JPs));
        const TCut misIDJPsMM_PID = (TCut)((TString) Form("!((TMath::Abs(Bs_M02_Subst2_K2mu - %f) < 60 && M1_{TUNE}_ProbNNmu < 0.8) || (TMath::Abs(Bs_M13_Subst3_K2mu - %f) < 60 && M2_{TUNE}_ProbNNmu))", PDG::Mass::JPs, PDG::Mass::JPs));
        const TCut misIDPsiMM     = (TCut)((TString) Form("TMath::Abs(Bs_M02_Subst2_K2mu - %f) > 60 && TMath::Abs(Bs_M13_Subst3_K2mu - %f) > 60", PDG::Mass::Psi, PDG::Mass::Psi));
        const TCut misIDPsiMM_PID = (TCut)((TString) Form("!((TMath::Abs(Bs_M02_Subst2_K2mu - %f) < 60 && M1_{TUNE}_ProbNNmu < 0.8) || (TMath::Abs(Bs_M13_Subst3_K2mu - %f) < 60 && M2_{TUNE}_ProbNNmu))", PDG::Mass::Psi, PDG::Mass::Psi));
        const TCut misIDJPsEE     = (TCut)((TString) Form("TMath::Abs(Bs_TRACK_M_Subst_Kl2lK_DTF_JPs - %f) > 60 && TMath::Abs(Bs_TRACK_M_Subst_lK2Kl_DTF_JPs - %f) > 60", PDG::Mass::Bd, PDG::Mass::Bd));
        const TCut misIDJPsEE_PID = (TCut)((TString) Form("!((TMath::Abs(Bs_TRACK_M_Subst_Kl2lK_DTF_JPs - %f) < 60 && E1_{TUNE}_ProbNNe < 0.8) || (TMath::Abs(Bs_TRACK_M_Subst_lK2Kl_DTF_JPs - %f) < 60 && E2_{TUNE}_ProbNNe < 0.8))", PDG::Mass::Bs, PDG::Mass::Bs));
        const TCut misIDPsiEE     = (TCut)((TString) Form("TMath::Abs(Bs_TRACK_M_Subst_Kl2lK_DTF_Psi - %f) > 60 && TMath::Abs(Bs_TRACK_M_Subst_lK2Kl_DTF_Psi - %f) > 60", PDG::Mass::Bd, PDG::Mass::Bd));
        const TCut misIDPsiEE_PID = (TCut)((TString) Form("!((TMath::Abs(Bs_TRACK_M_Subst_Kl2lK_DTF_Psi - %f) < 60 && E1_{TUNE}_ProbNNe < 0.8) || (TMath::Abs(Bs_TRACK_M_Subst_lK2Kl_DTF_Psi - %f) < 60 && E2_{TUNE}_ProbNNe < 0.8))", PDG::Mass::Bs, PDG::Mass::Bs));
        // const TCut misIDp_PID     = "((((K1_{TUNE}_ProbNNp * (1 - K1_{TUNE}_ProbNNk)) > 0.35) && (TMath::Abs(Bs_M0123_Subst3_K2p - 5619.60) > 50.)) || !((K1_{TUNE}_ProbNNp * (1 - K1_{TUNE}_ProbNNk)) > 0.35)) && ((((K2_{TUNE}_ProbNNp * (1 - K2_{TUNE}_ProbNNk)) > 0.35) && (TMath::Abs(Bs_M0123_Subst2_K2p - 5619.60) > 50.)) || !((K2_{TUNE}_ProbNNp * (1 - K2_{TUNE}_ProbNNk)) > 0.35))";
        const TCut misIDp_PID = "TMath::Max(K1_{TUNE}_ProbNNp * (1 - K1_{TUNE}_ProbNNk), K2_{TUNE}_ProbNNp * (1 - K2_{TUNE}_ProbNNk)) < 0.20";
        const TCut misIDp     = "TMath::Min(TMath::Abs(Bs_M23_Subst3_K2p - 1519.50), TMath::Abs(Bs_M23_Subst2_K2p - 1519.50)) > 20.";
        // const TCut misIDp_PID     = "TMath::Max(K1_{TUNE}_ProbNNp, K2_{TUNE}_ProbNNp) < 0.35";
        const TCut misIDpiMM_PID = "(((1) && (TMath::Abs(Bs_M03_Subst0_mu2pi - 895.55) > 40.)) || !(1)) && (((1) && (TMath::Abs(Bs_M12_Subst1_mu2pi - 895.55) > 40.)) || !(1))";
        const TCut misIDpiEE_PID = "(((1) && (TMath::Abs(Bs_M03_Subst0_e2pi  - 895.55) > 40.)) || !(1)) && (((1) && (TMath::Abs(Bs_M12_Subst1_e2pi  - 895.55) > 40.)) || !(1))";
    }   // namespace Background

    /**
     * \namespace CutDefRPhi::MVA
     * \brief RPhi MVA cut definitions
     **/
    namespace MVA {
        const map< tuple< Analysis, Year, Q2Bin >, TString > CAT = {{make_tuple(Analysis::MM, Year::Run1, Q2Bin::Low), "cat_wMVA_lowcen > 0.49"},   {make_tuple(Analysis::MM, Year::Run1, Q2Bin::Central), "cat_wMVA_lowcen > 0.49"},   {make_tuple(Analysis::MM, Year::Run1, Q2Bin::High), "cat_wMVA_lowcen > 0.49"},   {make_tuple(Analysis::MM, Year::Run1, Q2Bin::JPsi), "cat_wMVA_lowcen > 0.49"},   {make_tuple(Analysis::MM, Year::Run1, Q2Bin::Psi), "cat_wMVA_lowcen > 0.49"},

                                                                    {make_tuple(Analysis::MM, Year::Run2p1, Q2Bin::Low), "cat_wMVA_lowcen > 0.56"}, {make_tuple(Analysis::MM, Year::Run2p1, Q2Bin::Central), "cat_wMVA_lowcen > 0.56"}, {make_tuple(Analysis::MM, Year::Run2p1, Q2Bin::High), "cat_wMVA_lowcen > 0.56"}, {make_tuple(Analysis::MM, Year::Run2p1, Q2Bin::JPsi), "cat_wMVA_lowcen > 0.56"}, {make_tuple(Analysis::MM, Year::Run2p1, Q2Bin::Psi), "cat_wMVA_lowcen > 0.56"},

                                                                    {make_tuple(Analysis::MM, Year::Run2p2, Q2Bin::Low), "cat_wMVA_lowcen > 0.00"}, {make_tuple(Analysis::MM, Year::Run2p2, Q2Bin::Central), "cat_wMVA_lowcen > 0.00"}, {make_tuple(Analysis::MM, Year::Run2p2, Q2Bin::High), "cat_wMVA_lowcen > 0.00"}, {make_tuple(Analysis::MM, Year::Run2p2, Q2Bin::JPsi), "cat_wMVA_lowcen > 0.00"}, {make_tuple(Analysis::MM, Year::Run2p2, Q2Bin::Psi), "cat_wMVA_lowcen > 0.00"},

                                                                    {make_tuple(Analysis::EE, Year::Run1, Q2Bin::Low), "cat_wMVA_lowcen > 0.79"},   {make_tuple(Analysis::EE, Year::Run1, Q2Bin::Central), "cat_wMVA_lowcen > 0.79"},   {make_tuple(Analysis::EE, Year::Run1, Q2Bin::High), "cat_wMVA_lowcen > 0.79"},   {make_tuple(Analysis::EE, Year::Run1, Q2Bin::JPsi), "cat_wMVA_lowcen > 0.79"},   {make_tuple(Analysis::EE, Year::Run1, Q2Bin::Psi), "cat_wMVA_lowcen > 0.79"},

                                                                    {make_tuple(Analysis::EE, Year::Run2p1, Q2Bin::Low), "cat_wMVA_lowcen > 0.91"}, {make_tuple(Analysis::EE, Year::Run2p1, Q2Bin::Central), "cat_wMVA_lowcen > 0.91"}, {make_tuple(Analysis::EE, Year::Run2p1, Q2Bin::High), "cat_wMVA_lowcen > 0.91"}, {make_tuple(Analysis::EE, Year::Run2p1, Q2Bin::JPsi), "cat_wMVA_lowcen > 0.91"}, {make_tuple(Analysis::EE, Year::Run2p1, Q2Bin::Psi), "cat_wMVA_lowcen > 0.91"},

                                                                    {make_tuple(Analysis::EE, Year::Run2p2, Q2Bin::Low), "cat_wMVA_lowcen > 0.00"}, {make_tuple(Analysis::EE, Year::Run2p2, Q2Bin::Central), "cat_wMVA_lowcen > 0.00"}, {make_tuple(Analysis::EE, Year::Run2p2, Q2Bin::High), "cat_wMVA_lowcen > 0.00"}, {make_tuple(Analysis::EE, Year::Run2p2, Q2Bin::JPsi), "cat_wMVA_lowcen > 0.00"}, {make_tuple(Analysis::EE, Year::Run2p2, Q2Bin::Psi), "cat_wMVA_lowcen > 0.00"}};
    }   // namespace MVA

}   // namespace CutDefRPhi

#endif
