#include "TupleProcess.hpp"
#include "ParserSvc.hpp"
#include "IOSvc.hpp"
#include "HelperProcessing.hpp"
#include "PQWeights.hpp"
/**
 * TupleProcess
 * tupleProcess.out --yaml yaml/tuples/config-TupleProcess-###.yaml
*/
int main(int argc, char ** argv) {

    auto tStart = chrono::high_resolution_clock::now();

    ParserSvc parser("");
    parser.Init(argc, argv);
    if (parser.Run(argc, argv) != 0) return 1;

    SettingDef::Tuple::addTuple = true;

    EventType et = EventType();
    
    TupleProcess tp = TupleProcess(et, "");
    
    tp.Process();
    
    if (et.GetSample() == "LPTSS" and SettingDef::Tuple::option == "gng") {
        //================================================================
        // Run also the SS Hadron TupleProcess and merge the two samples.
        // A branch "SSLepton" is added to show origin of candidate
        //================================================================
        MessageSvc::Line();
        MessageSvc::Info(Color::Cyan, "tupleProcess", (TString)"Skimmming SS Hadron Data..");
        SettingDef::Config::sample = "LPTSSHH";
        EventType etH = EventType();
        TupleProcess tpH = TupleProcess(etH, "");
        tpH.Process();
        MessageSvc::Line();
        //================================================================
        // Merges the SS HH and SS LL tuples
        //================================================================
        HelperProcessing::MergeSSData();
    }
    if(  (et.GetSample().Contains("Lb2pKJPs") || et.GetSample().Contains("Lb2pKPsi"))){        
        //================================================================
        // Run for Lb2pKJps and Lb2pKPsi samples the attaching of PQ analysis Lb->pKJ/Psi 
        // A branch "wdp, mkp, mjpsip" is added
        // If addToMCDT is true, the weights are computed on MCDT and ported to DT via RunNb,EvtNb
        // Valid from v10 on-wards, for RPhi, need to "redo" tuples with MCDT in
        //================================================================
        bool addToMCDT = false;
        if( et.GetProject() == Prj::RKst) addToMCDT = true;
        if( et.GetProject() == Prj::RPhi) addToMCDT = false;
        if( et.GetProject() != Prj::RK  ){
            //NO PQ attaching on RK tuples... must re-do MCDT if used, not used tuples anyway...
            ROOT::RDataFrame df("DecayTuple", "TupleProcess.root");
            if( ! df.HasColumn("wdp")){
                MessageSvc::Warning("Adding PQ weights to local TupleProcess.root file");
                PQWeights::AppendPQWeights( et, "TupleProcess.root", addToMCDT);
            }
        }
    }
    //run it regardless
    //TODO : return a RNode for the function and handle the RDataFrame here directly. Will boost execution time by large factors.
    //TODO : if centralized in EventType, we can even load and add Definitions of columns on the fly saving time on running TupleProcess itself.
    //TODO : scheme would be Filter()->AllRNodeDefines->UseLastNode we get
    //Add to DT the PID Weights following updates in HelperProcessing.cpp     
    if(et.HasWeightOption("PID")) HelperProcessing::AddPIDWeights(et);    
    //Add to DT the Kst_M for part-recoed RKst samples in RK 
    if(et.HasWeightOption("PORTBRANCH")) HelperProcessing::PortTrueBMass( et);
    if(et.HasWeightOption("PORTBRANCH")) HelperProcessing::PortTrueKstMass( et);
    if(et.HasWeightOption("PORTBRANCH")) HelperProcessing::PortTrueQ2( et );

    //Add to DT the L0 weights
    if(et.HasWeightOption("L0"))  HelperProcessing::AddL0Weights( et);    
    //Add to DT the HLT weights
    if(et.HasWeightOption("HLT")) HelperProcessing::AddHLTWeights( et);
    //Add to DT the TRK weights
    if(et.HasWeightOption("TRK")) HelperProcessing::AddTRKWeights( et);

    et.Close();    
    if( et.GetYear() == Year::Y2012 && et.IsMC() && et.GetProject()!=Prj::RPhi ){
        //Add to DT the HLT1_TOS_alignment
        MessageSvc::Line();
        MessageSvc::Warning((TString) "Performing 2012 TCK alignment for AllTrackL0Decision_TOS");        
        HelperProcessing::AlignHLT1TrackAllL0DecisionTOS12MC(et.GetAna(), et.GetProject());
        MessageSvc::Warning((TString) "Performing 2012 TCK alignment for AllTrackL0Decision_TOS DONE");
    }
    // if( et.GetYear() == Year::Y2016 && et.IsMC() && et.GetProject()!= Prj::RPhi){ //TO UPDATE FOR RPHI, no need given HelperSvc doing the logic in the Cut-string
    //     MessageSvc::Line();
    //     MessageSvc::Warning((TString) "Performing 2016 TCK alignment for TrackMVA_TOS HLT1");
    //     HelperProcessing::AlignHLT1TrackMVATOS16MC(et.GetAna(), et.GetProject());
    //     MessageSvc::Warning((TString) "Performing 2016 TCK alignment for TrackMVA_TOS DONE");
    // }        
    auto tEnd = chrono::high_resolution_clock::now();
    MessageSvc::Line();
    MessageSvc::Warning((TString) SettingDef::IO::exe, "Took", to_string(chrono::duration_cast< chrono::seconds >(tEnd - tStart).count()), "seconds");
    MessageSvc::Line();
    return 0;
}

