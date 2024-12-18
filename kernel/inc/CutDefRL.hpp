#ifndef CUTDEFRL_HPP
#define CUTDEFRL_HPP

#include "ConstDef.hpp"

#include "TCut.h"

/**
 * \namespace CutDefRL
 * \brief RK cut definitions
 **/
namespace CutDefRL {

    /**
     * \namespace CutDefRL::Trigger
     * \brief RL Trigger cut definitions
     **/
    namespace Trigger {
        /**
         * Three analyses to define selections for
         * BF (Lb -> Jpsi L) : Mu + MuMu triggers, only for MM
         * Lb->Lemu :          Single Mu triggers:   separate L0, Hlt1, Hlt2 for mumu, emu modes
         * RL:                 As equal as possible: separate L0  and   Hlt2 for mumu, ee  modes
         **/

        const TCut L0M1   = "l1_L0MuonDecision_TOS";                                //
        const TCut L0M2   = "l2_L0MuonDecision_TOS";                                // L0 Lemu for Lemu
        const TCut L0M    = L0M1 || L0M2;                                           // L0 Lmm  for Lemu,Lee
        const TCut L0BRMM = "JPs_L0MuonDecision_TOS || JPs_L0DiMuonDecision_TOS";   // L0 LJpsimm for LJpsi
        const TCut L0E1   = "l1_L0ElectronDecision_TOS";                            //
        const TCut L0E2   = "l2_L0ElectronDecision_TOS";                            //
        const TCut L0E    = L0E1 || L0E2;                                           // L0 Lee for Lee
        const TCut L0I    = "Lb_L0Global_TIS";                                      //

        const TCut Hlt1TIS = "Lb_Hlt1Phys_TIS";
        const TCut Hlt2TIS = "Lb_Hlt2Phys_TIS";
        const TCut HltTIS  = Hlt1TIS && Hlt2TIS;

        namespace Run1 {

            const TCut nSPD = "nSPDHits < 600";

            // RL
            const TCut Hlt1MM = "JPs_Hlt1TrackAllL0Decision_TOS";
            const TCut Hlt1EE = "JPs_Hlt1TrackAllL0Decision_TOS";
            // LME
            const TCut Hlt1ME    = "l2_Hlt1TrackAllL0Decision_TOS || l2_Hlt1TrackMuonDecision_TOS";
            const TCut Hlt1JPsME = "JPs_Hlt1TrackAllL0Decision_TOS || JPs_Hlt1TrackMuonDecision_TOS";
            // BR LJPs
            const TCut Hlt1BRMM = "JPs_Hlt1TrackAllL0Decision_TOS || JPs_Hlt1TrackMuonDecision_TOS || JPs_Hlt1DiMuonHighMassDecision_TOS";

            const TCut Hlt2Topo = "Lb_Hlt2Topo2BodyBBDTDecision_TOS || Lb_Hlt2Topo3BodyBBDTDecision_TOS";
            // RL
            const TCut Hlt2MM = Hlt2Topo || "Lb_Hlt2TopoMu2BodyBBDTDecision_TOS || Lb_Hlt2TopoMu3BodyBBDTDecision_TOS";
            const TCut Hlt2EE = Hlt2Topo || "Lb_Hlt2TopoE2BodyBBDTDecision_TOS || Lb_Hlt2TopoE3BodyBBDTDecision_TOS";
            // LME
            const TCut Hlt2ME    = Hlt2Topo || "Lb_Hlt2TopoMu2BodyBBDTDecision_TOS || Lb_Hlt2TopoMu3BodyBBDTDecision_TOS||Lb_Hlt2TopoE2BodyBBDTDecision_TOS||Lb_Hlt2TopoE3BodyBBDTDecision_TOS";
            const TCut Hlt2JPsME = Hlt2Topo || "Lb_Hlt2TopoMu2BodyBBDTDecision_TOS || Lb_Hlt2TopoMu3BodyBBDTDecision_TOS";
            // BR LJPs
            const TCut Hlt2BRMM = Hlt2Topo || "Lb_Hlt2TopoMu2BodyBBDTDecision_TOS || Lb_Hlt2TopoMu3BodyBBDTDecision_TOS || JPs_Hlt2DiMuonDetachedJpsiDecision_TOS";

        }   // namespace Run1

        namespace Run2 {

            const TCut nSPD = "nSPDHits < 450";

            // RL
            const TCut Hlt1MM = "JPs_Hlt1TrackMVADecision_TOS";
            const TCut Hlt1EE = "JPs_Hlt1TrackMVADecision_TOS";
            // LME
            const TCut Hlt1ME    = "l2_Hlt1TrackMuonMVADecision_TOS  || l2_Hlt1TrackMVADecision_TOS  || l2_Hlt1TrackMuonDecision_TOS";
            const TCut Hlt1JPsME = "JPs_Hlt1TrackMuonMVADecision_TOS || JPs_Hlt1TrackMVADecision_TOS || JPs_Hlt1TrackMuonDecision_TOS";
            // BR LJPs
            const TCut Hlt1BRMM = "JPs_Hlt1TrackMuonMVADecision_TOS || JPs_Hlt1TrackMVADecision_TOS || JPs_Hlt1TrackMuonDecision_TOS || JPs_Hlt1DiMuonHighMassDecision_TOS";

            const TCut Hlt2Topo = "Lb_Hlt2Topo2BodyDecision_TOS || Lb_Hlt2Topo3BodyDecision_TOS";
            // RL
            const TCut Hlt2MM = Hlt2Topo || "Lb_Hlt2TopoMu2BodyDecision_TOS || Lb_Hlt2TopoMu3BodyDecision_TOS || Lb_Hlt2TopoMuMu2BodyDecision_TOS || Lb_Hlt2TopoMuMu3BodyDecision_TOS";
            const TCut Hlt2EE = Hlt2Topo || "Lb_Hlt2TopoE2BodyDecision_TOS || Lb_Hlt2TopoE3BodyDecision_TOS || Lb_Hlt2TopoEE2BodyDecision_TOS || Lb_Hlt2TopoEE3BodyDecision_TOS";
            // LME
            const TCut Hlt2ME    = Hlt2Topo || "Lb_Hlt2TopoE2BodyDecision_TOS || Lb_Hlt2TopoE3BodyDecision_TOS || Lb_Hlt2TopoMu2BodyDecision_TOS || Lb_Hlt2TopoMu3BodyDecision_TOS || Lb_Hlt2TopoMuE2BodyDecision_TOS || Lb_Hlt2TopoMuE3BodyDecision_TOS";
            const TCut Hlt2JPsME = Hlt2Topo || "Lb_Hlt2TopoMu2BodyDecision_TOS || Lb_Hlt2TopoMu3BodyDecision_TOS || Lb_Hlt2TopoMuMu2BodyDecision_TOS || Lb_Hlt2TopoMuMu3BodyDecision_TOS";
            // BR LJPs
            const TCut Hlt2BRMM = Hlt2Topo || "Lb_Hlt2TopoMu2BodyDecision_TOS || Lb_Hlt2TopoMu3BodyDecision_TOS || Lb_Hlt2TopoMuMu2BodyDecision_TOS || Lb_Hlt2TopoMuMu3BodyDecision_TOS || JPs_Hlt2DiMuonDetachedJpsiDecision_TOS";

            namespace Y2015 {

                // RL
                const TCut Hlt1MM = "JPs_Hlt1TrackMVADecision_TOS";
                const TCut Hlt1EE = "JPs_Hlt1TrackMVADecision_TOS";
                // LME
                const TCut Hlt1ME    = "l2_Hlt1TrackMVADecision_TOS  || l2_Hlt1TrackMuonDecision_TOS";
                const TCut Hlt1JPsME = "JPs_Hlt1TrackMVADecision_TOS || JPs_Hlt1TrackMuonDecision_TOS";
                // BR LJPs
                const TCut Hlt1BRMM = "JPs_Hlt1TrackMVADecision_TOS || JPs_Hlt1TrackMuonDecision_TOS || JPs_Hlt1DiMuonHighMassDecision_TOS";

                const TCut Hlt2Topo = "Lb_Hlt2Topo2BodyDecision_TOS || Lb_Hlt2Topo3BodyDecision_TOS";
                // RL
                const TCut Hlt2MM = Hlt2Topo;
                const TCut Hlt2EE = Hlt2Topo;
                // LME
                const TCut Hlt2ME    = Hlt2Topo;
                const TCut Hlt2JPsME = Hlt2Topo;
                // BR LJPs
                const TCut Hlt2BRMM = Hlt2Topo || "JPs_Hlt2DiMuonDetachedJpsiDecision_TOS";

            }   // namespace Y2015

        }   // namespace Run2
    }       // namespace Trigger

    /**
     * \namespace CutDefRL::Brem
     * \brief RL Brem cut definitions
     **/
    namespace Brem {

        const TCut G0 = "(l1_BremMultiplicity + l2_BremMultiplicity) == 0";
        const TCut G1 = "(l1_BremMultiplicity + l2_BremMultiplicity) == 1";
        const TCut G2 = "(l1_BremMultiplicity + l2_BremMultiplicity) >= 2";

        const TCut E0 = "l1_BremMultiplicity == 0";
        const TCut E1 = "l1_BremMultiplicity >= 1";

        /* In Lb2Lemu ana set up like this instead
        const TCut G0 = "(l1_HasBremAdded + l2_HasBremAdded) == 0";
        const TCut G1 = "(l1_HasBremAdded + l2_HasBremAdded) == 1";
        const TCut G2 = "(l1_HasBremAdded + l2_HasBremAdded) == 2";
        const TCut G3 = "(l1_HasBremAdded + l2_HasBremAdded) > 0";
        */
    }   // namespace Brem

    /**
     * \namespace CutDefRL::Track
     * \brief RL Track cut definitions
     **/
    namespace Track {

        const TCut LL = "p_TRACK_Type == 3 && pi_TRACK_Type == 3";
        const TCut DD = "p_TRACK_Type == 5 && pi_TRACK_Type == 5";

    }   // namespace Track

    /**
     * \namespace CutDefRL::Mass
     * \brief RL Mass cut definitions
     **/
    namespace Mass {

        const TCut L0      = (TCut)((TString) Form("TMath::Abs(L0_M - %f) < 15", PDG::Mass::L0));
        const TCut L0Loose = (TCut)((TString) Form("TMath::Abs(L0_M - %f) < 50", PDG::Mass::L0));

        const TCut Q2Low = "TMath::Sq(JPs_M/1000) > 0.1 && TMath::Sq(JPs_M/1000) < 6.0";
        // const TCut Q2Central = "TMath::Sq(JPs_M/1000) > 1.1 && TMath::Sq(JPs_M/1000) < 6.0";
        const TCut Q2HighMM = "TMath::Sq(JPs_M/1000) > 15 && TMath::Sq(JPs_M/1000) < 20";
        const TCut Q2HighEE = "TMath::Sq(JPs_TRACK_M/1000) > 14";
        const TCut Q2ME     = "TMath::Sq(JPs_M/1000) > 0.1 && TMath::Sq(JPs_M/1000) < 20.0";

        const TCut JPsMM = (TCut)((TString) Form("TMath::Abs(JPs_M - %f) < 50", PDG::Mass::JPs));
        const TCut JPsEE = "TMath::Sq(JPs_M/1000) > 6 && TMath::Sq(JPs_M/1000) < 11";

        const TCut PsiMM = (TCut)((TString) Form("TMath::Abs(JPs_M - %f) < 50", PDG::Mass::Psi));
        const TCut PsiEE = "TMath::Sq(JPs_M/1000) > 11 && TMath::Sq(JPs_M/1000) < 15";

        const TCut HOPEE = "Lb_hop_LoKi_mass_bv > (2080 + 248 * log(Lb_FDCHI2_OWNPV))";
        const TCut HOPME = "Lb_hop_LoKi_mass_bv > (2900 + 170 * log(Lb_FDCHI2_OWNPV))";

    }   // namespace Mass

    /**
     * \namespace CutDefRL::Quality
     * \brief RL Quality cut definitions
     **/
    namespace Quality {

        const TCut accMM = "l1_InAccMuon==1 && l2_InAccMuon==1";
        const TCut accEE = "l1_InAccEcal==1 && l2_InAccEcal==1";
        const TCut accME = "l1_InAccEcal==1 && l2_InAccMuon==1";

        const TCut hasDetHH = "p_hasRich && pi_hasRich";
        const TCut hasDetMM = "l1_hasMuon && l2_hasMuon";
        const TCut hasDetEE = "l1_hasCalo && l2_hasCalo";
        const TCut hasDetME = "l1_hasCalo && l2_hasMuon";

        // const TCut pidHH = "TMath::Min(p_PT,pi_PT) > 250 && TMath::Min(p_P,pi_P) > 2000";
        const TCut pidHH = "p_PT > 250 && p_P > 2000";   // currently not included because of LL VeryLooseV0 in restripping, possibly will only include for proton

        const TCut pidMM = "TMath::Min(l1_PT,l2_PT) > 800 && TMath::Min(l1_P,l2_P) > 3000";
        const TCut pidEE = "TMath::Min(l1_PT,l2_PT) > 500 && TMath::Min(l1_P,l2_P) > 3000";
        const TCut pidME = "l1_PT > 500 && l2_PT > 800 && TMath::Min(l1_P,l2_P) > 3000";

        const TCut stripHH = "L0_PT > 500 && TMath::Min(p_IPCHI2_OWNPV,pi_IPCHI2_OWNPV) > 9 ";
        const TCut stripMM = "TMath::Min(l1_PT,l2_PT) > 300 && TMath::Min(l1_IPCHI2_OWNPV,l2_IPCHI2_OWNPV) > 9";
        const TCut stripEE = "TMath::Min(l1_PT,l2_PT) > 300 && TMath::Min(l1_IPCHI2_OWNPV,l2_IPCHI2_OWNPV) > 9 && TMath::Min(TMath::Sqrt(TMath::Sq(l1_TRACK_PX)+TMath::Sq(l1_TRACK_PY)),TMath::Sqrt(TMath::Sq(l2_TRACK_PX)+TMath::Sq(l2_TRACK_PY))) > 200 && JPs_PT > 500";
        const TCut stripME = "TMath::Min(l1_PT,l2_PT) > 300 && TMath::Min(l1_IPCHI2_OWNPV,l2_IPCHI2_OWNPV) > 9 && TMath::Min(TMath::Sqrt(TMath::Sq(l1_TRACK_PX)+TMath::Sq(l1_TRACK_PY)),TMath::Sqrt(TMath::Sq(l2_PX)+TMath::Sq(l2_PY))) > 200 && JPs_PT > 500";

        const TCut trackHH = "TMath::Max(p_TRACK_CHI2NDOF,pi_TRACK_CHI2NDOF) < 4 && TMath::Max(p_TRACK_GhostProb,pi_TRACK_GhostProb) < 0.4";
        const TCut trackLL = "TMath::Max(l1_TRACK_CHI2NDOF,l2_TRACK_CHI2NDOF) < 4 && TMath::Max(l1_TRACK_GhostProb,l2_TRACK_GhostProb) < 0.4";   // These cuts are at 3,0.3 in Run 1; could implement separately

        const TCut fiducialL0 = "L0_FDCHI2_OWNPV > 0 && L0_ENDVERTEX_Z > 0 && L0_ENDVERTEX_Z < 2250 && L0_TAU*1000 > 0.5 && L0_TAU*1000 < 2000 && L0_DIRA_OWNPV > 0";

        const TCut fiducialLb = "Lb_PT > 4000 && Lb_PT < 25000 && TMath::ATanH(Lb_PZ/Lb_P) > 2.0 && TMath::ATanH(Lb_PZ/Lb_P) < 5.0";   // http://lhcbproject.web.cern.ch/lhcbproject/Publications/LHCbProjectPublic/LHCb-PAPER-2018-050.html

        const TCut qualityMM = hasDetMM && pidMM && stripHH && stripMM && trackHH && trackLL && fiducialL0;
        const TCut qualityEE = hasDetMM && pidEE && stripHH && stripMM && trackHH && trackLL && fiducialL0;
        const TCut qualityME = hasDetME && pidME && stripHH && stripMM && trackHH && trackLL && fiducialL0;

    }   // namespace Quality

    /**
     * \namespace CutDefRL::Background
     * \brief RL Background cut definitions
     **/
    namespace Background {

        const TCut Lc  = "Lb_M013 > 2300";
        const TCut JPs = "!(TMath::Sq(JPs_M/1000) > 9.0 && TMath::Sq(JPs_M/1000) < 10.1";
        const TCut vtx = "Lb_ENDVERTEX_ZERR < 30";   // Only needs to be included for low q2; to reject converted photons

    }   // namespace Background

    /**
     * \namespace CutDefRL::PID
     * \brief RL PID cut definitions
     **/
    namespace PID {

        const TCut pidM  = "l1_isMuon==1 && l2_isMuon==1";
        const TCut pidE  = "TMath::Min(l1_PIDe,l2_PIDe) > 0";
        const TCut pidP  = "p_PIDp > -5";
        const TCut pidPi = "1";   // Placeholder

        // const TCut calibM  = "MC12TuneV2_ProbNNmu * (1 - MC12TuneV2_ProbNNK) * (1 - MC12TuneV2_ProbNNp) > 0.4";
        // const TCut calibE  = "MC12TuneV3_ProbNNe > 0.5 && MC12TuneV3_ProbNNK < 0.95";
        // const TCut calibP  = "MC12TuneV2_ProbNNp > 0.5";
        // const TCut calibK  = "MC12TuneV2_ProbNNK * (1 - MC12TuneV2_ProbNNpi) * (1 - MC12TuneV2_ProbNNp) > 0.5";
        // const TCut calibPi = "MC12TuneV2_ProbNNpi * (1 - MC12TuneV2_ProbNNK) * (1 - MC12TuneV2_ProbNNp) > 0.5";

        const TCut pidMM = pidM;
        const TCut pidEE = pidE;
        const TCut pidME = "l1_PIDe > 0 && l2_isMuon==1";

    }   // namespace PID

    namespace MVA {

        const TCut mvaMM = "xgboost8_fixhop > 0.6";
        const TCut mvaEE = "Lb_Lee_DataAll_Allq2_MVA_AllTracks_AllTrig_trained > 0.0";
        const TCut mvaME = "1";

    }   // namespace MVA

}   // namespace CutDefRL

#endif
