#ifndef CUTDEFRKS_HPP
#define CUTDEFRKS_HPP

#include "ConstDef.hpp"

#include "TCut.h"

/**
 * \namespace CutDefRKS
 * \brief RK cut definitions
 **/
namespace CutDefRKS {

    namespace Trigger {

        const TCut L0M1 = "l1_L0MuonDecision_TOS";
        const TCut L0M2 = "l2_L0MuonDecision_TOS";
        const TCut L0M  = L0M1 || L0M2;
        const TCut L0I  = "B0_L0Global_TIS";

        const TCut Hlt1TIS = "B0_Hlt1Phys_TIS";
        const TCut Hlt2TIS = "B0_Hlt2Phys_TIS";
        const TCut HltTIS  = Hlt1TIS && Hlt2TIS;

        namespace Run1 {

            const TCut nSPD = "nSPDHits < 600";

            const TCut Hlt1MM = "l1_Hlt1TrackAllL0Decision_TOS || l1_Hlt1TrackMuonDecision_TOS || l2_Hlt1TrackAllL0Decision_TOS || l2_Hlt1TrackMuonDecision_TOS";

            const TCut Hlt2MM = "B0_Hlt2Topo2BodyBBDTDecision_TOS || B0_Hlt2Topo3BodyBBDTDecision_TOS";

        }   // namespace Run1

        namespace Run2 {

            const TCut nSPD = "nSPDHits < 450";

            const TCut Hlt1MM = "l1_Hlt1TrackMVADecision_TOS || l1_Hlt1TrackMuonDecision_TOS || l2_Hlt1TrackMVADecision_TOS || l2_Hlt1TrackMuonDecision_TOS";

            const TCut Hlt2MM = "B0_Hlt2Topo2BodyDecision_TOS || B0_Hlt2Topo3BodyDecision_TOS";

        }   // namespace Run2
    }       // namespace Trigger

    /**
     * \namespace CutDefRKS::Track
     * \brief RKS Track cut definitions
     **/
    namespace Track {

        const TCut LL = "p_TRACK_Type == 3 && pi_TRACK_Type == 3";
        const TCut DD = "p_TRACK_Type == 5 && pi_TRACK_Type == 5";

    }   // namespace Track

    /**
     * \namespace CutDefRKS::Mass
     * \brief RKS Mass cut definitions
     **/
    namespace Mass {

        const TCut KS = (TCut)((TString) Form("TMath::Abs(KS_M - %f) < 15", PDG::Mass::KShort));

        const TCut JPsMM = (TCut)((TString) Form("TMath::Abs(JPs_M - %f) < 50", PDG::Mass::JPs));

    }   // namespace Mass

    /**
     * \namespace CutDefRKS::Quality
     * \brief RKS Quality cut definitions
     **/
    namespace Quality {

        const TCut hasDetMM = "l1_hasMuon && l2_hasMuon";
        const TCut hasDetPi = "p_hasRich && pi_hasRich";

        const TCut qualityMM = hasDetMM && hasDetPi;

    }   // namespace Quality

    /**
     * \namespace CutDefRKS::Background
     * \brief RKS Background cut definitions
     **/
    namespace Background {}   // namespace Background

    /**
     * \namespace CutDefRKS::PID
     * \brief RKS PID cut definitions
     **/
    namespace PID {

        const TCut pidMM = "1";

    }   // namespace PID

    namespace MVA {

        const TCut mvaMM = "1";

    }   // namespace MVA

}   // namespace CutDefRKS

#endif
