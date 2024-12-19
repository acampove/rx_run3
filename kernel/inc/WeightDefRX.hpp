#ifndef WEIGHTDEFRX_HPP
#define WEIGHTDEFRX_HPP

#include "EnumeratorSvc.hpp"

#include "TCut.h"
#include "TString.h"
#include <tuple>
#include <functional>

using namespace std;

/**
 * \namespace WeightDefRX
 * \brief RX weight definitions
 **/
namespace WeightDefRX {
    /**
     * Namespace useful to navigate inside map of weights built for a given
     * WeigthHolder. Each build weight is mapped to a specific m_weights[TString]
     * which you can ask after... Easy to do GetWeigth( WeightDefRX::ID::PID ) and never think
     * about matching strings...
     */
    namespace ID {

        const TString PID = "PID";
        const TString ISMUON = "ISMUON";

        const TString TRK = "TRK";

        const TString L0  = "L0";
        const TString HLT = "HLT";

        const TString BKIN = "BKIN";
        const TString MULT = "MULT";
        const TString RECO = "RECO";
        const TString BDT  = "BDT";    // weights from MM L0L incl (BKIN + MULT + RECO)       
        const TString BDT2 = "BDT2";   // RECO weights from EE L0L incl
        const TString RW1D = "RW1D";

        const TString TRGALL = "TRGALL";

        const TString LB    = "LB";
        const TString LB_KIN = "LB_KIN";

        const TString PTRECO = "PTRECO";

        const TString BS = "BS";

        const TString SP = "SP";

        const TString FULL = "FULL";

    }   // namespace ID

    const TCut PID = "wPIDCalib";
    // const TCut PID_BS = "wPIDCalib_BS[{IDX}]";

    const TCut ISMUON = "wPIDCalibM1_IsMuon * wPIDCalibM2_IsMuon";
    // const TCut ISMUON_BS = "wPIDCalibM1_IsMuon_BS * wPIDCalibM2_IsMuon_BS";

    const TCut TRK    = "wTRKCalib";
    // const TCut TRK_BS = "wTRKCalib_BS";

    // OLD
    // const TCut L0M_inc = "(M1_wL0L_incl_{B} * M1_L0MuonDecision_TOS + M2_wL0L_incl_{B} * M2_L0MuonDecision_TOS - M1_wL0L_incl_{B} * M1_L0MuonDecision_TOS * M2_wL0L_incl_{B} * M2_L0MuonDecision_TOS)";
    // const TCut L0E_inc = "(E1_wL0L_incl_{B} * E1_L0ElectronDecision_TOS + E2_wL0L_incl_{B} * E2_L0ElectronDecision_TOS - E1_wL0L_incl_{B} * E1_L0ElectronDecision_TOS * E2_wL0L_incl_{B} * E2_L0ElectronDecision_TOS)";
    // NEW
    const TCut L0M_inc    = Form("((1. - (1. - %s * %s) * (1. - %s * %s)) / (1. - (1. - %s * %s) * (1. - %s * %s)))", //
                              "M1_wL0L_incl_{B}_effCL", "M1_L0MuonDecision_TOS", "M2_wL0L_incl_{B}_effCL", "M2_L0MuonDecision_TOS",   //
                              "M1_wL0L_incl_{B}_effMC", "M1_L0MuonDecision_TOS", "M2_wL0L_incl_{B}_effMC", "M2_L0MuonDecision_TOS"    //
                              // "M1_L0MuonDecision_TOS", "M2_L0MuonDecision_TOS"                                                     // added to avoid dividing by 0 (dropped)
    );
    const TCut L0M_inc_BS = Form("((1. - (1. - %s * %s) * (1. - %s * %s)) / (1. - (1. - %s * %s) * (1. - %s * %s)))", //
                              "M1_wL0L_incl_{B}_effCL_BS", "M1_L0MuonDecision_TOS", "M2_wL0L_incl_{B}_effCL_BS", "M2_L0MuonDecision_TOS",   //
                              "M1_wL0L_incl_{B}_effMC_BS", "M1_L0MuonDecision_TOS", "M2_wL0L_incl_{B}_effMC_BS", "M2_L0MuonDecision_TOS"    //
                              // "M1_L0MuonDecision_TOS", "M2_L0MuonDecision_TOS"                                                     // added to avoid dividing by 0 (dropped)
    );    
    // BECAREFUL , the particle status is defined in the formula , but it gets removed by WeightHolderRX.cpp
    // Do we need a protection for events  being !TOS on both ? Will we ever use the weight on events being not TOS on both electrons?
    // const TCut L0E_inc = Form("((1 - (1 - %s * %s) * (1 - %s * %s)) / (1 - (1 - %s * %s) * (1 - %s * %s) + (!%s && !%s)))",                   //
    //                           "E1_wL0L_incl_{B}_effCL", "E1_L0ElectronDecision_TOS", "E2_wL0L_incl_{B}_effCL", "E2_L0ElectronDecision_TOS",   //
    //                           "E1_wL0L_incl_{B}_effMC", "E1_L0ElectronDecision_TOS", "E2_wL0L_incl_{B}_effMC", "E2_L0ElectronDecision_TOS",   //
    //                           "E1_L0ElectronDecision_TOS", "E2_L0ElectronDecision_TOS"                                                        // added to avoid dividing by 0
    // );
    const TCut L0E_inc = Form("((1. - (1. - %s * %s) * (1. - %s * %s)) / (1. - (1. - %s * %s) * (1. - %s * %s) ))" ,//
                              "E1_wL0L_incl_{B}_effCL", "E1_L0ElectronDecision_TOS", "E2_wL0L_incl_{B}_effCL", "E2_L0ElectronDecision_TOS",   //
                              "E1_wL0L_incl_{B}_effMC", "E1_L0ElectronDecision_TOS", "E2_wL0L_incl_{B}_effMC", "E2_L0ElectronDecision_TOS"    //
                              // "E1_L0ElectronDecision_TOS", "E2_L0ElectronDecision_TOS"                                                     // added to avoid dividing by 0
    );
    const TCut L0E_inc_BS = Form("((1. - (1. - %s * %s) * (1. - %s * %s)) / (1. - (1. - %s * %s) * (1. - %s * %s) ))" ,//
                              "E1_wL0L_incl_{B}_effCL_BS", "E1_L0ElectronDecision_TOS", "E2_wL0L_incl_{B}_effCL_BS", "E2_L0ElectronDecision_TOS",   //
                              "E1_wL0L_incl_{B}_effMC_BS", "E1_L0ElectronDecision_TOS", "E2_wL0L_incl_{B}_effMC_BS", "E2_L0ElectronDecision_TOS"    //
                              // "E1_L0ElectronDecision_TOS", "E2_L0ElectronDecision_TOS"                                                     // added to avoid dividing by 0
    );

    const TCut L0I_inc = Form("(%s / %s)",                                                 //
                              "{HEAD}_wL0I_incl_{B}_effCL", "{HEAD}_wL0I_incl_{B}_effMC"   //
    );
    const TCut L0I_inc_BS = Form("(%s / %s)",                                                 //
                              "{HEAD}_wL0I_incl_{B}_effCL_BS", "{HEAD}_wL0I_incl_{B}_effMC_BS"   //
    );
    // OLD
    // const TCut L0M_exc = "(M1_wL0L_excl_{B} * M1_L0MuonDecision_TOS + M2_wL0L_excl_{B} * M2_L0MuonDecision_TOS - M1_wL0L_excl_{B} * M1_L0MuonDecision_TOS * M2_wL0L_excl_{B} * M2_L0MuonDecision_TOS)";
    // const TCut L0E_exc = "(E1_wL0L_excl_{B} * E1_L0ElectronDecision_TOS + E2_wL0L_excl_{B} * E2_L0ElectronDecision_TOS - E1_wL0L_excl_{B} * E1_L0ElectronDecision_TOS * E2_wL0L_excl_{B} * E2_L0ElectronDecision_TOS)";
    // NEW
    const TCut L0M_not = Form("(((1. - %s * %s) * (1. - %s * %s)) / ((1. - %s * %s) * (1. - %s * %s)))",                                  //
                              "M1_wL0L_incl_{B}_effCL", "M1_L0MuonDecision_TOS", "M2_wL0L_incl_{B}_effCL", "M2_L0MuonDecision_TOS",   //
                              "M1_wL0L_incl_{B}_effMC", "M1_L0MuonDecision_TOS", "M2_wL0L_incl_{B}_effMC", "M2_L0MuonDecision_TOS"    //
    );
    const TCut L0E_not  = Form("(((1. - %s * %s) * (1. - %s * %s)) / ((1. - %s * %s) * (1. - %s * %s)))",                                          //
                              "E1_wL0L_incl_{B}_effCL", "E1_L0ElectronDecision_TOS", "E2_wL0L_incl_{B}_effCL", "E2_L0ElectronDecision_TOS",   //
                              "E1_wL0L_incl_{B}_effMC", "E1_L0ElectronDecision_TOS", "E2_wL0L_incl_{B}_effMC", "E2_L0ElectronDecision_TOS"    //
    );
    const TCut L0I_not = Form("((1. - %s) / (1. - %s))",                                   //
                              "{HEAD}_wL0I_incl_{B}_effCL", "{HEAD}_wL0I_incl_{B}_effMC"   //
    );
    const TCut L0H_inc = Form("(%s / %s)",                                                 //
                              "{HEAD}_wL0H_incl_{B}_effCL", "{HEAD}_wL0H_incl_{B}_effMC"   //
    );

    const TCut L0M_not_BS = Form("(((1. - %s * %s) * (1. - %s * %s)) / ((1. - %s * %s) * (1. - %s * %s)))",                                  //
                              "M1_wL0L_incl_{B}_effCL_BS", "M1_L0MuonDecision_TOS", "M2_wL0L_incl_{B}_effCL_BS", "M2_L0MuonDecision_TOS",   //
                              "M1_wL0L_incl_{B}_effMC_BS", "M1_L0MuonDecision_TOS", "M2_wL0L_incl_{B}_effMC_BS", "M2_L0MuonDecision_TOS"    //
    );
    const TCut L0E_not_BS = Form("(((1. - %s * %s) * (1. - %s * %s)) / ((1. - %s * %s) * (1. - %s * %s)))",                                          //
                              "E1_wL0L_incl_{B}_effCL_BS", "E1_L0ElectronDecision_TOS", "E2_wL0L_incl_{B}_effCL_BS", "E2_L0ElectronDecision_TOS",   //
                              "E1_wL0L_incl_{B}_effMC_BS", "E1_L0ElectronDecision_TOS", "E2_wL0L_incl_{B}_effMC_BS", "E2_L0ElectronDecision_TOS"    //
    );
    const TCut L0I_not_BS = Form("((1. - %s) / (1. - %s))",                                     //
                              "{HEAD}_wL0I_incl_{B}_effCL_BS", "{HEAD}_wL0I_incl_{B}_effMC_BS"   //
    );

    const map< tuple< TriggerConf, Trigger, Analysis >, TString > wL0 = {
        {make_tuple(TriggerConf::Inclusive, Trigger::L0I, Analysis::MM), TString(WeightDefRX::L0I_inc.GetTitle())},   //
        {make_tuple(TriggerConf::Inclusive, Trigger::L0I, Analysis::EE), TString(WeightDefRX::L0I_inc.GetTitle())},   //
        {make_tuple(TriggerConf::Inclusive, Trigger::L0L, Analysis::MM), TString(WeightDefRX::L0M_inc.GetTitle())},   //
        {make_tuple(TriggerConf::Inclusive, Trigger::L0L, Analysis::EE), TString(WeightDefRX::L0E_inc.GetTitle())},   //
        {make_tuple(TriggerConf::Inclusive, Trigger::L0H, Analysis::MM), TString(WeightDefRX::L0H_inc.GetTitle())},   //
        {make_tuple(TriggerConf::Inclusive, Trigger::L0H, Analysis::EE), TString(WeightDefRX::L0H_inc.GetTitle())},   //

        {make_tuple(TriggerConf::Exclusive, Trigger::L0I, Analysis::MM), TString(WeightDefRX::L0I_inc.GetTitle())},   //
        {make_tuple(TriggerConf::Exclusive, Trigger::L0I, Analysis::EE), TString(WeightDefRX::L0I_inc.GetTitle())},   //
        {make_tuple(TriggerConf::Exclusive, Trigger::L0L, Analysis::MM), TString(Form("(%s) * (%s)", WeightDefRX::L0M_inc.GetTitle(), WeightDefRX::L0I_not.GetTitle()))},
        {make_tuple(TriggerConf::Exclusive, Trigger::L0L, Analysis::EE), TString(Form("(%s) * (%s)", WeightDefRX::L0E_inc.GetTitle(), WeightDefRX::L0I_not.GetTitle()))},
        {make_tuple(TriggerConf::Exclusive, Trigger::L0H, Analysis::MM), TString(Form("(%s) * (%s) * (%s)", WeightDefRX::L0H_inc.GetTitle(), WeightDefRX::L0I_not.GetTitle(), WeightDefRX::L0M_not.GetTitle()))},
        {make_tuple(TriggerConf::Exclusive, Trigger::L0H, Analysis::EE), TString(Form("(%s) * (%s) * (%s)", WeightDefRX::L0H_inc.GetTitle(), WeightDefRX::L0I_not.GetTitle(), WeightDefRX::L0E_not.GetTitle()))},

        {make_tuple(TriggerConf::Exclusive2, Trigger::L0I, Analysis::MM), TString(Form("(%s) * (%s)", WeightDefRX::L0I_inc.GetTitle(), WeightDefRX::L0M_not.GetTitle()))},
        {make_tuple(TriggerConf::Exclusive2, Trigger::L0I, Analysis::EE), TString(Form("(%s) * (%s)", WeightDefRX::L0I_inc.GetTitle(), WeightDefRX::L0E_not.GetTitle()))},
        {make_tuple(TriggerConf::Exclusive2, Trigger::L0L, Analysis::MM), TString(WeightDefRX::L0M_inc.GetTitle())},   //
        {make_tuple(TriggerConf::Exclusive2, Trigger::L0L, Analysis::EE), TString(WeightDefRX::L0E_inc.GetTitle())},   //
    };

    // Special hadnling of L0L corrections ( done using DiLepton system directly for L0L category on both muon and electrons)
    const TCut L0M_inc_comb     = "(TMath::Max(M1_wL0L_comb_{B}_effCL * M1_L0MuonDecision_TOS,M2_wL0L_comb_{B}_effCL * M1_L0MuonDecision_TOS))/(TMath::Max(M1_wL0L_comb_{B}_effMC * M1_L0MuonDecision_TOS,M2_wL0L_comb_{B}_effMC * M2_L0MuonDecision_TOS))";
    const TCut L0E_inc_comb     = "(TMath::Max(E1_wL0L_comb_{B}_effCL * E1_L0ElectronDecision_TOS,E2_wL0L_comb_{B}_effCL * E2_L0ElectronDecision_TOS))/(TMath::Max(E1_wL0L_comb_{B}_effMC * E1_L0ElectronDecision_TOS,E2_wL0L_comb_{B}_effMC * E2_L0ElectronDecision_TOS))";
    const TCut L0M_inc_comb_not = "(1. - TMath::Max(M1_wL0L_comb_{B}_effCL * M1_L0MuonDecision_TOS,M2_wL0L_comb_{B}_effCL * M1_L0MuonDecision_TOS))/(1. - TMath::Max(M1_wL0L_comb_{B}_effMC * M1_L0MuonDecision_TOS,M2_wL0L_comb_{B}_effMC * M2_L0MuonDecision_TOS))";
    const TCut L0E_inc_comb_not = "(1. - TMath::Max(E1_wL0L_comb_{B}_effCL * E1_L0ElectronDecision_TOS,E2_wL0L_comb_{B}_effCL * E2_L0ElectronDecision_TOS))/(1. - TMath::Max(E1_wL0L_comb_{B}_effMC * E1_L0ElectronDecision_TOS,E2_wL0L_comb_{B}_effMC * E2_L0ElectronDecision_TOS))";
    
    // const TCut L0M_inc_comb     = "(M_wL0L_comb_{B}_effCL/M_wL0L_comb_{B}_effMC)";
    // const TCut L0E_inc_comb     = "(E_wL0L_comb_{B}_effCL/E_wL0L_comb_{B}_effMC)";
    // const TCut L0M_inc_comb_not = "(1. - M_wL0L_comb_{B}_effCL)/(1. - M_wL0L_comb_{B}_effMC)";
    // const TCut L0E_inc_comb_not = "(1. - E_wL0L_comb_{B}_effCL)/(1. - E_wL0L_comb_{B}_effMC)";

    const TCut L0M_inc_comb_BS     = "(M_wL0L_comb_{B}_effCL_BS[{IDX}]/M_wL0L_comb_{B}_effMC_BS[{IDX}])";
    const TCut L0E_inc_comb_BS     = "(E_wL0L_comb_{B}_effCL_BS[{IDX}]/E_wL0L_comb_{B}_effMC_BS[{IDX}])";
    const TCut L0M_inc_comb_not_BS = "(1. - M_wL0L_comb_{B}_effCL_BS[{IDX}])/(1. - M_wL0L_comb_{B}_effMC_BS[{IDX}])";
    const TCut L0E_inc_comb_not_BS = "(1. - E_wL0L_comb_{B}_effCL_BS[{IDX}])/(1. - E_wL0L_comb_{B}_effMC_BS[{IDX}])";    
    const map< tuple< TriggerConf , Trigger, Analysis > ,TString > wL0comb = { 
        {make_tuple(TriggerConf::Inclusive, Trigger::L0I, Analysis::MM), TString(WeightDefRX::L0I_inc.GetTitle() )},   //
        {make_tuple(TriggerConf::Inclusive, Trigger::L0I, Analysis::EE), TString(WeightDefRX::L0I_inc.GetTitle() )},   //
        {make_tuple(TriggerConf::Inclusive, Trigger::L0L, Analysis::MM), TString(WeightDefRX::L0M_inc_comb.GetTitle() )},   //
        {make_tuple(TriggerConf::Inclusive, Trigger::L0L, Analysis::EE), TString(WeightDefRX::L0E_inc_comb.GetTitle() )},   //
        {make_tuple(TriggerConf::Exclusive, Trigger::L0I, Analysis::MM), TString(WeightDefRX::L0I_inc.GetTitle() )},   //
        {make_tuple(TriggerConf::Exclusive, Trigger::L0I, Analysis::EE), TString(WeightDefRX::L0I_inc.GetTitle() )},   //
        {make_tuple(TriggerConf::Exclusive, Trigger::L0L, Analysis::MM), TString(Form("(%s) * (%s)", WeightDefRX::L0M_inc_comb.GetTitle(), WeightDefRX::L0I_not.GetTitle()))},
        {make_tuple(TriggerConf::Exclusive, Trigger::L0L, Analysis::EE), TString(Form("(%s) * (%s)", WeightDefRX::L0E_inc_comb.GetTitle(), WeightDefRX::L0I_not.GetTitle()))},
        {make_tuple(TriggerConf::Exclusive2, Trigger::L0I, Analysis::MM), TString(Form("(%s) * (%s)", WeightDefRX::L0I_inc.GetTitle(), WeightDefRX::L0M_inc_comb_not.GetTitle()))},
        {make_tuple(TriggerConf::Exclusive2, Trigger::L0I, Analysis::EE), TString(Form("(%s) * (%s)", WeightDefRX::L0I_inc.GetTitle(), WeightDefRX::L0E_inc_comb_not.GetTitle()))},
        {make_tuple(TriggerConf::Exclusive2, Trigger::L0L, Analysis::MM), TString(WeightDefRX::L0M_inc_comb.GetTitle() )},   //
        {make_tuple(TriggerConf::Exclusive2, Trigger::L0L, Analysis::EE), TString(WeightDefRX::L0E_inc_comb.GetTitle() )},   //        
    };
    const map< tuple< TriggerConf , Trigger, Analysis > ,TString > wL0comb_BS = { 
        {make_tuple(TriggerConf::Inclusive, Trigger::L0I, Analysis::MM), TString(WeightDefRX::L0I_inc_BS.GetTitle() )},   //
        {make_tuple(TriggerConf::Inclusive, Trigger::L0I, Analysis::EE), TString(WeightDefRX::L0I_inc_BS.GetTitle() )},   //
        {make_tuple(TriggerConf::Inclusive, Trigger::L0L, Analysis::MM), TString(WeightDefRX::L0M_inc_comb_BS.GetTitle() )},   //
        {make_tuple(TriggerConf::Inclusive, Trigger::L0L, Analysis::EE), TString(WeightDefRX::L0E_inc_comb_BS.GetTitle() )},   //
        {make_tuple(TriggerConf::Exclusive, Trigger::L0I, Analysis::MM), TString(WeightDefRX::L0I_inc_BS.GetTitle() )},   //
        {make_tuple(TriggerConf::Exclusive, Trigger::L0I, Analysis::EE), TString(WeightDefRX::L0I_inc_BS.GetTitle() )},   //
        {make_tuple(TriggerConf::Exclusive, Trigger::L0L, Analysis::MM), TString(Form("(%s) * (%s)", WeightDefRX::L0M_inc_comb_BS.GetTitle(), WeightDefRX::L0I_not_BS.GetTitle()))},
        {make_tuple(TriggerConf::Exclusive, Trigger::L0L, Analysis::EE), TString(Form("(%s) * (%s)", WeightDefRX::L0E_inc_comb_BS.GetTitle(), WeightDefRX::L0I_not_BS.GetTitle()))},
        {make_tuple(TriggerConf::Exclusive2, Trigger::L0I, Analysis::MM), TString(Form("(%s) * (%s)", WeightDefRX::L0I_inc_BS.GetTitle(), WeightDefRX::L0M_inc_comb_not_BS.GetTitle()))},
        {make_tuple(TriggerConf::Exclusive2, Trigger::L0I, Analysis::EE), TString(Form("(%s) * (%s)", WeightDefRX::L0I_inc_BS.GetTitle(), WeightDefRX::L0E_inc_comb_not_BS.GetTitle()))},
        {make_tuple(TriggerConf::Exclusive2, Trigger::L0L, Analysis::MM), TString(WeightDefRX::L0M_inc_comb_BS.GetTitle())},   //
        {make_tuple(TriggerConf::Exclusive2, Trigger::L0L, Analysis::EE), TString(WeightDefRX::L0E_inc_comb_BS.GetTitle())},   //        
    };    
    //Same as before but without multiplying by status ( OLD approach, totally dropped ( See the ReplaceAll in WeightHolderRX.cpp ))
    // const TCut L0M_inc_noStatus = Form("((1 - (1 - %s ) * (1 - %s )) / (1 - (1 - %s ) * (1 - %s )))",           //
    //                                     "M1_wL0L_incl_{B}_effCL", "M2_wL0L_incl_{B}_effCL",   //
    //                                     "M1_wL0L_incl_{B}_effMC", "M2_wL0L_incl_{B}_effMC"   //
    // );
    // const TCut L0E_inc_noStatus = Form("((1 - (1 - %s ) * (1 - %s )) / (1 - (1 - %s ) * (1 - %s )))",                   //
    //                                     "E1_wL0L_incl_{B}_effCL", "E2_wL0L_incl_{B}_effCL",   //
    //                                     "E1_wL0L_incl_{B}_effMC", "E2_wL0L_incl_{B}_effMC"   //
    // );

    // //NEW no status version
    // const TCut L0M_not_noStatus = Form("(((1 - %s ) * (1 - %s )) / ((1 - %s ) * (1 - %s)))", //
    //                           "M1_wL0L_incl_{B}_effCL", "M2_wL0L_incl_{B}_effCL",    //
    //                           "M1_wL0L_incl_{B}_effMC", "M2_wL0L_incl_{B}_effMC"    //
    // );
    // const TCut L0E_not_noStatus = Form("(((1 - %s ) * (1 - %s )) / ((1 - %s ) * (1 - %s )))",                                          //
    //                           "E1_wL0L_incl_{B}_effCL",  "E2_wL0L_incl_{B}_effCL",    //
    //                           "E1_wL0L_incl_{B}_effMC",  "E2_wL0L_incl_{B}_effMC"    //
    // );
    // const TCut L0I_not_noStatus = Form("((1 - %s) / (1 - %s))",                                     //
    //                           "{HEAD}_wL0I_incl_{B}_effCL", "{HEAD}_wL0I_incl_{B}_effMC"   //
    // );
    // const map< tuple< TriggerConf, Trigger, Analysis >, TString > wL0_noStatus = {
    //     {make_tuple(TriggerConf::Inclusive, Trigger::L0I, Analysis::MM), TString(WeightDefRX::L0I_inc)},   //
    //     {make_tuple(TriggerConf::Inclusive, Trigger::L0I, Analysis::EE), TString(WeightDefRX::L0I_inc)},   //
    //     {make_tuple(TriggerConf::Inclusive, Trigger::L0L, Analysis::MM), TString(WeightDefRX::L0M_inc_noStatus)},   //
    //     {make_tuple(TriggerConf::Inclusive, Trigger::L0L, Analysis::EE), TString(WeightDefRX::L0E_inc_noStatus)},   //

    //     {make_tuple(TriggerConf::Exclusive, Trigger::L0I, Analysis::MM), TString(WeightDefRX::L0I_inc)},   //
    //     {make_tuple(TriggerConf::Exclusive, Trigger::L0I, Analysis::EE), TString(WeightDefRX::L0I_inc)},   //
    //     {make_tuple(TriggerConf::Exclusive, Trigger::L0L, Analysis::MM), TString(Form("(%s) * (%s)", WeightDefRX::L0M_inc_noStatus.GetTitle(), WeightDefRX::L0I_not_noStatus.GetTitle()))},
    //     {make_tuple(TriggerConf::Exclusive, Trigger::L0L, Analysis::EE), TString(Form("(%s) * (%s)", WeightDefRX::L0E_inc_noStatus.GetTitle(), WeightDefRX::L0I_not_noStatus.GetTitle()))},

    //     {make_tuple(TriggerConf::Exclusive2, Trigger::L0I, Analysis::MM), TString(Form("(%s) * (%s)", WeightDefRX::L0I_inc.GetTitle(), WeightDefRX::L0M_not_noStatus.GetTitle()))},
    //     {make_tuple(TriggerConf::Exclusive2, Trigger::L0I, Analysis::EE), TString(Form("(%s) * (%s)", WeightDefRX::L0I_inc.GetTitle(), WeightDefRX::L0E_not_noStatus.GetTitle()))},
    //     {make_tuple(TriggerConf::Exclusive2, Trigger::L0L, Analysis::MM), TString(WeightDefRX::L0M_inc_noStatus)},   //
    //     {make_tuple(TriggerConf::Exclusive2, Trigger::L0L, Analysis::EE), TString(WeightDefRX::L0E_inc_noStatus)},   //
    // };

    const TCut HLT_inc = "( {HEAD}_wHLT_{L0}_incl_{B}_effCL / {HEAD}_wHLT_{L0}_incl_{B}_effMC )";
    const TCut HLT_exc = "( {HEAD}_wHLT_{L0}_excl_{B}_effCL / {HEAD}_wHLT_{L0}_excl_{B}_effMC )";

    const TCut HLT_inc_BS = "( {HEAD}_wHLT_{L0}_incl_{B}_effCL_BS / {HEAD}_wHLT_{L0}_incl_{B}_effMC_BS )";
    const TCut HLT_exc_BS = "( {HEAD}_wHLT_{L0}_excl_{B}_effCL_BS / {HEAD}_wHLT_{L0}_excl_{B}_effMC_BS )";

    const map< pair< TriggerConf, Trigger >, TString > wHLT = {
        {{TriggerConf::Inclusive, Trigger::L0I}, TString(WeightDefRX::HLT_inc.GetTitle())},   //
        {{TriggerConf::Inclusive, Trigger::L0L}, TString(WeightDefRX::HLT_inc.GetTitle())},   //
        {{TriggerConf::Inclusive, Trigger::L0H}, TString(WeightDefRX::HLT_inc.GetTitle())},   //

        {{TriggerConf::Exclusive, Trigger::L0I}, TString(WeightDefRX::HLT_inc.GetTitle())},   //
        {{TriggerConf::Exclusive, Trigger::L0L}, TString(WeightDefRX::HLT_inc.GetTitle())},   //
        {{TriggerConf::Exclusive, Trigger::L0H}, TString(WeightDefRX::HLT_inc.GetTitle())},   //

        {{TriggerConf::Exclusive2, Trigger::L0I}, TString(WeightDefRX::HLT_inc.GetTitle())},   //
        {{TriggerConf::Exclusive2, Trigger::L0L}, TString(WeightDefRX::HLT_inc.GetTitle())},   //
        {{TriggerConf::Exclusive2, Trigger::L0H}, TString(WeightDefRX::HLT_inc.GetTitle())},   //
    };
    const map< pair< TriggerConf, Trigger >, TString > wHLT_BS = {
        {{TriggerConf::Inclusive, Trigger::L0I}, TString(WeightDefRX::HLT_inc_BS.GetTitle())},   //
        {{TriggerConf::Inclusive, Trigger::L0L}, TString(WeightDefRX::HLT_inc_BS.GetTitle())},   //
        {{TriggerConf::Inclusive, Trigger::L0H}, TString(WeightDefRX::HLT_inc_BS.GetTitle())},   //

        {{TriggerConf::Exclusive, Trigger::L0I}, TString(WeightDefRX::HLT_inc_BS.GetTitle())},   //
        {{TriggerConf::Exclusive, Trigger::L0L}, TString(WeightDefRX::HLT_inc_BS.GetTitle())},   //
        {{TriggerConf::Exclusive, Trigger::L0H}, TString(WeightDefRX::HLT_inc_BS.GetTitle())},   //

        {{TriggerConf::Exclusive2, Trigger::L0I}, TString(WeightDefRX::HLT_inc_BS.GetTitle())},   //
        {{TriggerConf::Exclusive2, Trigger::L0L}, TString(WeightDefRX::HLT_inc_BS.GetTitle())},   //
        {{TriggerConf::Exclusive2, Trigger::L0H}, TString(WeightDefRX::HLT_inc_BS.GetTitle())},   //
    };    

    const TCut RW1D = "wRW1D_{B}_MM_L0L";
    const TCut BKIN = "wBKIN_{B}_{LL}_{L0}";
    const TCut MULT = "wMULT_{B}_{LL}_{L0}";
    const TCut RECO = "wRECO_{B}_{LL}_{L0}";
    const TCut BDT  = "wBDT_{OPT}_{B}_MM_L0L";
    const TCut BDT_L0I = "wBDT_{OPT}_{B}_MM_L0I";
    const TCut BDT2 = "wBDT2_{OPT}_{LL}_RECO_{B}_{LL}_{L0}";

    const TCut LB     = "wdp";
    const TCut LB_KIN = "wkin_RpK.wkin"; //Kin weights ( see macros/Lb_RPk_Weights, mismatch name fix, this allows to use it )

    const TCut PTRECO = "wPTRECO";

    const TCut BS = "RndPoisson2";

    //Max number of RndPoisson we have.
    const int nBS = 100;

    //Number of nTracks slices for 3D PID maps
    const int N_ntracks = 5;

    //Number of nTracks slices for 3D PID maps for fit and count for electrons
    const int N_ntracks_fac = 4;

}   // namespace WeightDefRX

#endif
