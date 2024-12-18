#ifndef HELPERPROCESSING_CPP
#define HELPERPROCESSING_CPP
#include "HelperProcessing.hpp"
#include "IOSvc.hpp"
#include "TRandom3.h"
#include "HistoAdders.hpp"
#include "itertools.hpp"
#include "EventType.hpp"
#include <fmt_ostream.h>
#include "TLorentzVector.h"
#include "Functors.hpp"
#include "THn.h"
#include "CutDefRX.hpp"

void HelperProcessing::MergeSSData(){
    ROOT::DisableImplicitMT();
    MessageSvc::Info(Color::Cyan, "MergeSSData", (TString)"Merging SS Hadrons and SS Leptons..");

    //=======================================================================
    // Load SS tuples
    //=======================================================================
    ROOT::RDataFrame dfLL( "DecayTuple", "tmpTupleProcessLL.root");
    ROOT::RDataFrame dfHH( "DecayTuple", "tmpTupleProcessHH.root");

    //=======================================================================
    // Define origin Branch SSID == true -> Lepton
    //=======================================================================
    auto _dfLL = dfLL.Define("SSID", "true");
    auto _dfHH = dfHH.Define("SSID", "false");

    //=======================================================================
    // Check that branches are the same for both DFs (except for the ones
    // where we know there is a difference!)
    // Drop DTF Branches that are not used and are not aligned to each other
    //=======================================================================
    vector<string> _wcToDrop  = {"f_2_1525", "Kst_0_1430_0"};
    vector<string> _wcToAlign = {"phi_1020", "Kst_892"};
    
    auto _checkColumnsLL      = DropColumnsWildcard(_dfLL.GetColumnNames(), _wcToAlign);
    auto _checkColumnsHH      = DropColumnsWildcard(_dfHH.GetColumnNames(), _wcToDrop);
    std::sort(_checkColumnsLL.begin(), _checkColumnsLL.end());
    std::sort(_checkColumnsHH.begin(), _checkColumnsHH.end());
    bool _areEqual = std::equal(_checkColumnsLL.begin(), _checkColumnsLL.end(),
				_checkColumnsHH.begin(), _checkColumnsHH.end());
    if (!_areEqual) MessageSvc::Error("MergeSSData", "Mismatch in Branches present in LL, HH tuples!", "MAKE SURE OUTPUT IS FINE");

    auto _columnsHH           = DropColumnsWildcard(_dfHH.GetColumnNames(), _wcToDrop);
    _dfLL.Snapshot("DecayTuple", "tmpTupleProcessLL2.root", _columnsHH);
    _dfHH.Snapshot("DecayTuple", "tmpTupleProcessHH2.root", _columnsHH);

    //=======================================================================
    // Merge and save to disk
    //=======================================================================
    ROOT::RDataFrame df( "DecayTuple", {"tmpTupleProcessLL2.root", "tmpTupleProcessHH2.root"});    
    df.Snapshot("DecayTuple", "TupleProcess.root");
    MessageSvc::Info(Color::Cyan, "MergeSSData", (TString)"..merging complete!");
}

void HelperProcessing::moveTupleFromTo( TString _treeName, TString _oldFileName, TString _newFileName){
    TFile oldfile(_oldFileName);
    TTree *oldtree;
    oldfile.GetObject(_treeName, oldtree);
    // Create a new file + a clone of old tree in new file
    if( oldtree == nullptr){
        MessageSvc::Info("moveTupleFromTo", _treeName+" : " + _oldFileName+" --> "+_newFileName +"   ( missing ttree, do nothing )");
        return;
    }
    TString _mode = "UPDATE";
    TFile testFile(_newFileName,"READ");
    //will be 0 if no MCDecayTuple in it.
    if( testFile.GetNkeys()==0) _mode = "RECREATE";
    testFile.Close();
    TFile newfile(_newFileName, _mode);
    auto newtree = oldtree->CloneTree();    
    newtree->Print();
    newfile.Write(0, TObject::kOverwrite);
    oldfile.Close();
    newfile.Close();
    MessageSvc::Info("moveTupleFromTo, successful");
    return;
};

std::vector<string> HelperProcessing::DropColumns( ROOT::Detail::RDF::ColumnNames_t && current_columns, ROOT::Detail::RDF::ColumnNames_t && current_defined_columns, const vector< std::string> & blacklist) {
    std::vector<string> Columns_to_Keep;
    if(blacklist.size() == 0){
        for( auto & colName : current_columns){
            Columns_to_Keep.push_back( colName );
        }
        for( auto & colName : current_defined_columns){
            Columns_to_Keep.push_back( colName );
        }
        return Columns_to_Keep;
    }
    for( auto & colName : current_columns){
        bool keepIT = true;
        for( auto & blacklisted : blacklist){
            if(colName == blacklisted){
                MessageSvc::Info("DropColumns", colName);
                keepIT = false;
            }
        }
        if( keepIT){
	  if(!CheckVectorContains(Columns_to_Keep,colName)){
            Columns_to_Keep.push_back(colName);
	  }
        }
    }
    for( auto & colName : current_defined_columns){
        bool keepIT = true;
        for( auto & blacklisted : blacklist){
            if(colName == blacklisted){
                MessageSvc::Info("DropColumns", colName);
                keepIT = false;
            }
        }
        if( keepIT){
	  if(!CheckVectorContains(Columns_to_Keep,colName)){
            Columns_to_Keep.push_back(colName);
	  }
        }
    }
    MessageSvc::Info("Columns ALL", to_string(current_columns.size()));
    MessageSvc::Info("Columns Defined", to_string(current_defined_columns.size()));
    MessageSvc::Info("Columns BlackListed", to_string(blacklist.size()));
    MessageSvc::Info("Columns Kept" , to_string(Columns_to_Keep.size()));
    std::set<std::string> UniqueColumns(Columns_to_Keep.begin(), Columns_to_Keep.end()); // both sorted & unique already
    MessageSvc::Info("Columns UniqueSet" , to_string(UniqueColumns.size()));
    
    if(Columns_to_Keep.size()!= UniqueColumns.size() ){
        MessageSvc::Warning("Somehow Return columns contains duplicate, clearing (Maybe Show,Print) will not show things in order");
        Columns_to_Keep.clear(); 
        Columns_to_Keep.assign(UniqueColumns.begin(), UniqueColumns.end());
        MessageSvc::Warning("(NEW) Columns Kept" , to_string(Columns_to_Keep.size()));
    }
    return Columns_to_Keep;
};

std::vector<string> HelperProcessing::DropColumnsWildcard( ROOT::Detail::RDF::ColumnNames_t && current_columns, const vector<std::string> & wildcard) {
    //======================================================
    // removes all columns that contain <wildcard>
    //======================================================
    std::vector<string> Columns_to_Keep;
    int _colsRemoved = 0;

    for( auto & colName : current_columns ){
        bool keepIT = true;
	TString _colName = colName;
	for ( auto _wildcard: wildcard ) {
	    if( _colName.Contains( _wildcard ) ){
		MessageSvc::Info("DropColumnsWildcard", colName);
		keepIT = false;
	    }
	}
	if( keepIT ) {
	    Columns_to_Keep.push_back( colName );
	} else {
	    _colsRemoved++;
	}
    }

    MessageSvc::Info("Columns ALL", to_string(current_columns.size()));
    MessageSvc::Info("Columns Kept" , to_string(Columns_to_Keep.size()));
    MessageSvc::Info("Columns Dropped" , to_string(_colsRemoved));
    return Columns_to_Keep;
};

int HelperProcessing::AlignHLT1TrackMVATOS16MC( const Analysis & ana, const Prj & prj ){
	ROOT::DisableImplicitMT();
    std::string head_check = prj == Prj::RKst ? "B0_Hlt1TrackMVADecision_TOS_update" : "Bp_Hlt1TrackMVADecision_TOS_update";
    std::string final_state_check = ana == Analysis::EE ? "E1_Hlt1TrackMVADecision_TOS" : "M1_Hlt1TrackMVADecision_TOS" ; //protection to not run this if tuples doesn't contain that branch
    bool Check_Pass = false;
    ROOT::RDataFrame df( "DecayTuple", "TupleProcess.root");
    for( const auto & colName : df.GetColumnNames()){ 
        if( head_check == colName){ MessageSvc::Info( "HLT1 Hlt1TrackMVADecision_TOS TOS alignment already performed, skipping"); return 0; }
        if( final_state_check == colName ){ Check_Pass = true; break;}
    }
    if( Check_Pass == false){
        MessageSvc::Info( "HLT1 AllTrackL0Decision TOS alignment cannot be performed, branches needed not in DV tuple, skipping"); return 0; 
    }    
    TString _newTreeName  = "DecayTuple"; 
    TString _newTFileName = "TupleProcess_tmp.root";        
    //TODO : RPhi    
    auto makeTrackInfo = []( const double & trackPT, const double & IPCHI2, const bool & isTOS){ return TrackMVAHLT1Info( trackPT, IPCHI2, isTOS);};
    auto evalTOSUpdate = []( const TrackMVAHLT1Info & finalStateParticle ,  const Int_t  & TCKCat ){
        //body function writing in place for tracks the updated Status for the HLT1TOS decision.
        double b_parameter = 1.1;
        if( TCKCat == 1) b_parameter = 1.6;
        if( TCKCat == 2) b_parameter = 2.3;
        bool _BidimCut     = finalStateParticle.PASS2DCUT( b_parameter);
        bool _InRange2DCut = (finalStateParticle.PTGEV() <= 25.0) && (finalStateParticle.PTGEV() >= 1.0); //PT > 1 GeV || PT < 25 GeV
        bool _HighPTIPCHI2 = (finalStateParticle.PTGEV()  > 25.0) &&  (finalStateParticle.IPCHI2() >7.4); //PT > 25 GeV && IPCHI2 > 7.4 
        bool _isTOSUPDATED = finalStateParticle.TOS()  && ( _HighPTIPCHI2 || ( _InRange2DCut && _BidimCut));
        return _isTOSUPDATED;
    };
    vector< std::string > blacklist = { 
        "E1_trPT", "E2_trPT" , 
        "E2_TrackMVA_obj", "E1_TrackMVA_obj", "K_TrackMVA_obj" , "Pi_TrackMVA_obj",
        "M1_TrackMVA_obj", "M2_TrackMVA_obj", "K1_TrackMVA_obj" , "K2_TrackMVA_obj"
    };
    if( ana == Analysis::EE && prj == Prj::RK){      
        auto dd = df.Define("E1_trPT", "TMath::Sqrt(E1_TRACK_PX*E1_TRACK_PX + E1_TRACK_PY*E1_TRACK_PY)")
                    .Define("E1_TrackMVA_obj", makeTrackInfo, {"E1_trPT", "E1_IPCHI2_OWNPV", "E1_Hlt1TrackMVADecision_TOS"} )
                    .Define("E2_trPT", "TMath::Sqrt(E2_TRACK_PX*E2_TRACK_PX + E2_TRACK_PY*E2_TRACK_PY)")
                    .Define("E2_TrackMVA_obj", makeTrackInfo, {"E2_trPT", "E2_IPCHI2_OWNPV", "E2_Hlt1TrackMVADecision_TOS"} )                    
                    .Define("K_TrackMVA_obj", makeTrackInfo, {"K_PT", "K_IPCHI2_OWNPV", "K_Hlt1TrackMVADecision_TOS"})
                    .Define("E1_Hlt1TrackMVADecision_TOS_update", evalTOSUpdate, {"E1_TrackMVA_obj", "TCKCat"})
                    .Define("E2_Hlt1TrackMVADecision_TOS_update", evalTOSUpdate, {"E2_TrackMVA_obj", "TCKCat"})
                    .Define("K_Hlt1TrackMVADecision_TOS_update", evalTOSUpdate, {"K_TrackMVA_obj", "TCKCat"} )
                    .Define("Bp_Hlt1TrackMVADecision_TOS_update", "(E1_Hlt1TrackMVADecision_TOS_update || E2_Hlt1TrackMVADecision_TOS_update || K_Hlt1TrackMVADecision_TOS_update)"); 
        dd.Snapshot(_newTreeName.Data(), _newTFileName.Data() , DropColumns(dd.GetColumnNames(),dd.GetDefinedColumnNames(), blacklist ) );
    }
    if( ana == Analysis::MM && prj == Prj::RK){
        auto dd = df.Define("M1_TrackMVA_obj", makeTrackInfo, {"M1_PT", "M1_IPCHI2_OWNPV", "M1_Hlt1TrackMVADecision_TOS"} )
                    .Define("M2_TrackMVA_obj", makeTrackInfo, {"M2_PT", "M2_IPCHI2_OWNPV", "M2_Hlt1TrackMVADecision_TOS"} )                    
                    .Define("K_TrackMVA_obj", makeTrackInfo, {"K_PT", "K_IPCHI2_OWNPV", "K_Hlt1TrackMVADecision_TOS"} )
                    .Define("M1_Hlt1TrackMVADecision_TOS_update", evalTOSUpdate, { "M1_TrackMVA_obj", "TCKCat"} )
                    .Define("M2_Hlt1TrackMVADecision_TOS_update", evalTOSUpdate, { "M2_TrackMVA_obj", "TCKCat"} )
                    .Define("K_Hlt1TrackMVADecision_TOS_update", evalTOSUpdate, { "K_TrackMVA_obj", "TCKCat"} )
                    .Define("Bp_Hlt1TrackMVADecision_TOS_update", "(M1_Hlt1TrackMVADecision_TOS_update || M2_Hlt1TrackMVADecision_TOS_update || K_Hlt1TrackMVADecision_TOS_update)");         
        dd.Snapshot(_newTreeName.Data(), _newTFileName.Data() , DropColumns(dd.GetColumnNames(),dd.GetDefinedColumnNames(), blacklist ) );        
    }
    if( ana == Analysis::EE && prj == Prj::RKst){      
        auto dd = df.Define("E1_trPT", "TMath::Sqrt(E1_TRACK_PX*E1_TRACK_PX + E1_TRACK_PY*E1_TRACK_PY)")
                    .Define("E1_TrackMVA_obj", makeTrackInfo, {"E1_trPT", "E1_IPCHI2_OWNPV", "E1_Hlt1TrackMVADecision_TOS"} )
                    .Define("E2_trPT", "TMath::Sqrt(E2_TRACK_PX*E2_TRACK_PX + E2_TRACK_PY*E2_TRACK_PY)")
                    .Define("E2_TrackMVA_obj", makeTrackInfo, {"E2_trPT", "E2_IPCHI2_OWNPV", "E2_Hlt1TrackMVADecision_TOS"} )                    
                    .Define("K_TrackMVA_obj", makeTrackInfo, {"K_PT", "K_IPCHI2_OWNPV", "K_Hlt1TrackMVADecision_TOS"})
                    .Define("Pi_TrackMVA_obj", makeTrackInfo, {"Pi_PT", "Pi_IPCHI2_OWNPV", "Pi_Hlt1TrackMVADecision_TOS"})
                    .Define("E1_Hlt1TrackMVADecision_TOS_update", evalTOSUpdate , { "E1_TrackMVA_obj", "TCKCat"} )
                    .Define("E2_Hlt1TrackMVADecision_TOS_update", evalTOSUpdate , { "E2_TrackMVA_obj", "TCKCat"} )
                    .Define("K_Hlt1TrackMVADecision_TOS_update", evalTOSUpdate , { "K_TrackMVA_obj", "TCKCat"} )
                    .Define("Pi_Hlt1TrackMVADecision_TOS_update", evalTOSUpdate , { "Pi_TrackMVA_obj", "TCKCat"} )
                    .Define("B0_Hlt1TrackMVADecision_TOS_update", "(E1_Hlt1TrackMVADecision_TOS_update || E2_Hlt1TrackMVADecision_TOS_update || K_Hlt1TrackMVADecision_TOS_update || Pi_Hlt1TrackMVADecision_TOS_update)"); 
        dd.Snapshot(_newTreeName.Data(), _newTFileName.Data() , DropColumns(dd.GetColumnNames(),dd.GetDefinedColumnNames(), blacklist ) );
    }
    if( ana == Analysis::MM && prj == Prj::RKst){
        auto dd = df.Define("M1_TrackMVA_obj", makeTrackInfo, {"M1_PT", "M1_IPCHI2_OWNPV", "M1_Hlt1TrackMVADecision_TOS"} )
                    .Define("M2_TrackMVA_obj", makeTrackInfo, {"M2_PT", "M2_IPCHI2_OWNPV", "M2_Hlt1TrackMVADecision_TOS"} )                    
                    .Define("K_TrackMVA_obj", makeTrackInfo, {"K_PT", "K_IPCHI2_OWNPV", "K_Hlt1TrackMVADecision_TOS"})
                    .Define("Pi_TrackMVA_obj", makeTrackInfo, {"Pi_PT", "Pi_IPCHI2_OWNPV", "Pi_Hlt1TrackMVADecision_TOS"})
                    .Define("M1_Hlt1TrackMVADecision_TOS_update", evalTOSUpdate , {  "M1_TrackMVA_obj", "TCKCat"} )
                    .Define("M2_Hlt1TrackMVADecision_TOS_update", evalTOSUpdate , {  "M2_TrackMVA_obj", "TCKCat"} )
                    .Define("K_Hlt1TrackMVADecision_TOS_update", evalTOSUpdate , {  "K_TrackMVA_obj"  , "TCKCat"} )
                    .Define("Pi_Hlt1TrackMVADecision_TOS_update", evalTOSUpdate , {  "Pi_TrackMVA_obj", "TCKCat"} )
                    .Define("B0_Hlt1TrackMVADecision_TOS_update", "(M1_Hlt1TrackMVADecision_TOS_update || M2_Hlt1TrackMVADecision_TOS_update || K_Hlt1TrackMVADecision_TOS_update || Pi_Hlt1TrackMVADecision_TOS_update)");
        dd.Snapshot(_newTreeName.Data(), _newTFileName.Data() , DropColumns(dd.GetColumnNames(),dd.GetDefinedColumnNames(), blacklist ) );        
    }
    if( ana == Analysis::EE && prj == Prj::RPhi){      
        auto dd = df.Define("E1_trPT", "TMath::Sqrt(E1_TRACK_PX*E1_TRACK_PX + E1_TRACK_PY*E1_TRACK_PY)")
                    .Define("E1_TrackMVA_obj", makeTrackInfo, {"E1_trPT", "E1_IPCHI2_OWNPV", "E1_Hlt1TrackMVADecision_TOS"} )
                    .Define("E2_trPT", "TMath::Sqrt(E2_TRACK_PX*E2_TRACK_PX + E2_TRACK_PY*E2_TRACK_PY)")
                    .Define("E2_TrackMVA_obj", makeTrackInfo, {"E2_trPT", "E2_IPCHI2_OWNPV", "E2_Hlt1TrackMVADecision_TOS"} )                    
                    .Define("K1_TrackMVA_obj", makeTrackInfo, {"K1_PT", "K1_IPCHI2_OWNPV", "K1_Hlt1TrackMVADecision_TOS"})
                    .Define("K2_TrackMVA_obj", makeTrackInfo, {"K2_PT", "K2_IPCHI2_OWNPV", "K2_Hlt1TrackMVADecision_TOS"})
                    .Define("E1_Hlt1TrackMVADecision_TOS_update", evalTOSUpdate , { "E1_TrackMVA_obj", "TCKCat"} )
                    .Define("E2_Hlt1TrackMVADecision_TOS_update", evalTOSUpdate , { "E2_TrackMVA_obj", "TCKCat"} )
                    .Define("K1_Hlt1TrackMVADecision_TOS_update", evalTOSUpdate , { "K1_TrackMVA_obj", "TCKCat"} )
                    .Define("K2_Hlt1TrackMVADecision_TOS_update", evalTOSUpdate , { "K2_TrackMVA_obj", "TCKCat"} )
                    .Define("Bs_Hlt1TrackMVADecision_TOS_update", "(E1_Hlt1TrackMVADecision_TOS_update || E2_Hlt1TrackMVADecision_TOS_update || K1_Hlt1TrackMVADecision_TOS_update || K2_Hlt1TrackMVADecision_TOS_update)"); 
        dd.Snapshot(_newTreeName.Data(), _newTFileName.Data() , DropColumns(dd.GetColumnNames(),dd.GetDefinedColumnNames(), blacklist ) );
    }
    if( ana == Analysis::MM && prj == Prj::RPhi){
        auto dd = df.Define("M1_TrackMVA_obj", makeTrackInfo, {"M1_PT", "M1_IPCHI2_OWNPV", "M1_Hlt1TrackMVADecision_TOS"} )
                    .Define("M2_TrackMVA_obj", makeTrackInfo, {"M2_PT", "M2_IPCHI2_OWNPV", "M2_Hlt1TrackMVADecision_TOS"} )                    
                    .Define("K1_TrackMVA_obj", makeTrackInfo, {"K1_PT", "K1_IPCHI2_OWNPV", "K1_Hlt1TrackMVADecision_TOS"})
                    .Define("K2_TrackMVA_obj", makeTrackInfo, {"K2_PT", "K2_IPCHI2_OWNPV", "K2_Hlt1TrackMVADecision_TOS"})
                    .Define("M1_Hlt1TrackMVADecision_TOS_update", evalTOSUpdate , {  "M1_TrackMVA_obj", "TCKCat"} )
                    .Define("M2_Hlt1TrackMVADecision_TOS_update", evalTOSUpdate , {  "M2_TrackMVA_obj", "TCKCat"} )
                    .Define("K1_Hlt1TrackMVADecision_TOS_update", evalTOSUpdate , {  "K1_TrackMVA_obj", "TCKCat"} )
                    .Define("K2_Hlt1TrackMVADecision_TOS_update", evalTOSUpdate , {  "K2_TrackMVA_obj", "TCKCat"} )
                    .Define("Bs_Hlt1TrackMVADecision_TOS_update", "(M1_Hlt1TrackMVADecision_TOS_update || M2_Hlt1TrackMVADecision_TOS_update || K1_Hlt1TrackMVADecision_TOS_update || K2_Hlt1TrackMVADecision_TOS_update)");
        dd.Snapshot(_newTreeName.Data(), _newTFileName.Data() , DropColumns(dd.GetColumnNames(),dd.GetDefinedColumnNames(), blacklist ) );        
    }
    TFile f("TupleProcess.root","READ");
    auto  original =  (TTree*)f.Get("DecayTuple");
    TFile f2("TupleProcess_tmp.root", "READ");
    auto updated = (TTree*)f2.Get("DecayTuple");
    auto entries = original->GetEntries();
    if( original->GetEntries() != updated->GetEntries()){
        MessageSvc::Error("Severe error on Snapshots", "","EXIT_FAILURE");
    }else{
        f.Close();
        f2.Close();
    }
    //We have now to "delete DecayTuple from TupleProcess.root" (rootrm), and save back (rootmv) the new snapshotted one into the original file, delete back stuff.
    //1) Remove TTree from TupleProcess.root ( DecayTuple )
    MessageSvc::Info("rootrm TupleProcess.root:DecayTuple");
    IOSvc::runCommand(TString("rootrm TupleProcess.root:DecayTuple"));    
    //2) Move   TTree from TupleProcess_tmp.root to TupleProcess.root 
    MessageSvc::Info("rootmv TupleProcess_tmp.root:DecayTuple TupleProcess.root");
    moveTupleFromTo( "DecayTuple", "TupleProcess_tmp.root", "TupleProcess.root");
    //3) Remove TupleProcess_tmp.root file
    // MessageSvc::Info("rm TupleProcess_tmp.root");
    IOSvc::runCommand(TString("rm TupleProcess_tmp.root"));
    MessageSvc::Info("Checking entires of final stored tuple is preserved");
    TFile ff("TupleProcess.root","READ");
    auto finalTree = (TTree*)ff.Get("DecayTuple");
    if( finalTree == nullptr){
        MessageSvc::Error("Failure TCK alignment 16 tuples shuffling","","EXIT_FAILURE");
    }
    if( finalTree->GetEntries() != entries){
        MessageSvc::Error("Something went wrong", "","EXIT_FAILURE");
    }
    MessageSvc::Info("Closing file");
    ff.Close();
    return 0;    
};

int HelperProcessing::AlignHLT1TrackAllL0DecisionTOS12MC( const Analysis & ana, const Prj & prj){
	ROOT::DisableImplicitMT();
    //Create DataFrame, check if final branch is already in TTree (to skip re-processing and re-adding of Alignment in multiple Tuple Process runs)
	ROOT::RDataFrame df("DecayTuple", "TupleProcess.root");
    std::string head_check = prj == Prj::RKst ? "B0_Hlt1TrackAllL0Decision_TOS_update" : "Bp_Hlt1TrackAllL0Decision_TOS_update";
    std::string final_state_check = ana == Analysis::EE ? "E1_Hlt1TrackAllL0Decision_TOS" : "M1_Hlt1TrackAllL0Decision_TOS" ; //protection to not run this if tuples doesn't contain that branch
    bool Check_Pass = false;
    for( const auto & colName : df.GetColumnNames()){ 
        if( head_check == colName){ MessageSvc::Info( "HLT1 AllTrackL0Decision TOS alignment already performed, skipping"); return 0; }
        if( final_state_check == colName ){ Check_Pass = true; break;}
    }
    if( Check_Pass == false){
        MessageSvc::Info( "HLT1 AllTrackL0Decision TOS alignment cannot be performed, branches needed not in DV tuple, skipping"); return 0; 
    }
    TString _newTreeName = "DecayTuple"; 
    TString _newTFileName = "TupleProcess_tmp.root";    
    TRandom3 rnd; //random number generator to align 30% of events passing on MC the HLT1_TOS requirement (AllTrackL0Decision_TOS)

    //Set of helper Lambda functions to eval Alignment of HLT1AllTrackL0Decision
    auto makeTrackInfo = [](double p, double pt, bool isTOS){ return TrackHLT1Info( p, pt, isTOS); };
    auto makeRandom    = [&rnd]( const uint & runNumber, const unsigned long long & evtNumber ){
        rnd.SetSeed(runNumber*evtNumber);
        double rndNumber = rnd.Uniform(0,1);
        if(rndNumber < 0 || rndNumber > 1) MessageSvc::Error("Issue with rnd.Uniform","","EXIT_FAILURE");
        return rndNumber;
    };
    auto evalTOSUpdate = []( const TrackHLT1Info & finalStateParticle ,  const bool & HEAD_Hlt1TrackAllL0Decision_TOS , const double rndValue ){
        if( HEAD_Hlt1TrackAllL0Decision_TOS == false){ return false; } //If the global OR is already false, it keeps to be false ( we only tight up the cut, not loosen it)
        if( finalStateParticle.hlt1_tos == false){  return false; } //If the particle TOS is already false, it keeps to be false ( we only tight up the cut, not loosen it)
        bool updatedStatus = finalStateParticle.hlt1_tos;
        if ( rndValue < 0.3){
            //For 30% of events tight up the P, PT cut [ assuming IPCHI2 is already aligned ! [ there are other cuts done by the HLT1AllTrackL0 [ IP are the same for all TCKs ]]]
            //Track Chi2 and number of hits cut is dropped for alignment since the info available in MC is "post-HLT2" reconstruction and NOT at HLT1 level
            updatedStatus = finalStateParticle.p > 10000.f && finalStateParticle.pt > 1700.f && finalStateParticle.hlt1_tos;
        }
        return updatedStatus;
    };
    auto evalTOSTrackMuonUpdate = []( const TrackHLT1Info & finalStateParticle ,  const bool & Bp_Hlt1TrackMuonDecision_TOS , const double rndValue ){
        if( Bp_Hlt1TrackMuonDecision_TOS == false){ return false; } //If the global OR is already false, it keeps to be false ( we only tight up the cut, not loosen it)
        if( finalStateParticle.hlt1_tos == false){  return false; } //If the particle TOS is already false, it keeps to be false ( we only tight up the cut, not loosen it)
        bool updatedStatus = finalStateParticle.hlt1_tos;
        if ( rndValue < 0.3){
            //For 30% of events tight up the P, PT cut [ assuming IPCHI2 is already aligned ! [ there are other cuts done by the HLT1AllTrackL0 [ IP are the same for all TCKs ]]]
            //Track Chi2 and number of hits cut is dropped for alignment since the info available in MC is "post-HLT2" reconstruction and NOT at HLT1 level
            updatedStatus = finalStateParticle.p > 8000.f && finalStateParticle.hlt1_tos;
        }
        return updatedStatus;
    };

    vector< std::string > blacklist = { 
        "rndUniform",
        "E1_HLTTRACK_PT", "E1_HLTTRACK_P", "E1_TrackHLT1Info" , 
        "E2_HLTTRACK_PT", "E2_HLTTRACK_P", "E2_TrackHLT1Info" , 
        "M1_TrackHLT1Info", "M2_TrackHLT1Info", "K_TrackHLT1Info", "Pi_TrackHLT1Info" ,
        "M1_TrackMuonHLT1Info", "M2_TrackMuonHLT1Info", "K_TrackMuonHLT1Info", "Pi_TrackMuonHLT1Info" 
    };
    if( ana == Analysis::EE && prj == Prj::RK){      
        auto dd = df.Define("rndUniform", makeRandom,{"runNumber","eventNumber"})
                    .Define("E1_HLTTRACK_PT", "TMath::Sqrt(E1_TRACK_PX*E1_TRACK_PX + E1_TRACK_PY*E1_TRACK_PY)")
	                .Define("E2_HLTTRACK_PT", "TMath::Sqrt(E2_TRACK_PX*E2_TRACK_PX + E2_TRACK_PY*E2_TRACK_PY)")
                    .Define("E1_HLTTRACK_P",  "TMath::Sqrt(E1_TRACK_PX*E1_TRACK_PX + E1_TRACK_PY*E1_TRACK_PY + E1_TRACK_PZ*E1_TRACK_PZ)")
                    .Define("E2_HLTTRACK_P",  "TMath::Sqrt(E2_TRACK_PX*E2_TRACK_PX + E2_TRACK_PY*E2_TRACK_PY + E2_TRACK_PZ*E2_TRACK_PZ)")		
                    .Define("E1_TrackHLT1Info", makeTrackInfo, {"E1_HLTTRACK_P", "E1_HLTTRACK_PT", "E1_Hlt1TrackAllL0Decision_TOS"})
                    .Define("E2_TrackHLT1Info", makeTrackInfo, {"E2_HLTTRACK_P", "E2_HLTTRACK_PT", "E2_Hlt1TrackAllL0Decision_TOS"})
                    .Define("K_TrackHLT1Info",  makeTrackInfo, {"K_P", "K_PT", "K_Hlt1TrackAllL0Decision_TOS"})
                    .Define("E1_Hlt1TrackAllL0Decision_TOS_update", evalTOSUpdate , { "E1_TrackHLT1Info", "Bp_Hlt1TrackAllL0Decision_TOS" , "rndUniform"})
                    .Define("E2_Hlt1TrackAllL0Decision_TOS_update", evalTOSUpdate , { "E2_TrackHLT1Info", "Bp_Hlt1TrackAllL0Decision_TOS" , "rndUniform"})
                    .Define("K_Hlt1TrackAllL0Decision_TOS_update", evalTOSUpdate ,  { "K_TrackHLT1Info" , "Bp_Hlt1TrackAllL0Decision_TOS" , "rndUniform"})
                    .Define("Bp_Hlt1TrackAllL0Decision_TOS_update", "E1_Hlt1TrackAllL0Decision_TOS_update || E2_Hlt1TrackAllL0Decision_TOS_update || K_Hlt1TrackAllL0Decision_TOS_update");
        dd.Snapshot(_newTreeName.Data(), _newTFileName.Data() , DropColumns(dd.GetColumnNames(),dd.GetDefinedColumnNames(), blacklist ) );
    }
    if( ana == Analysis::EE && prj == Prj::RKst){// || prj == Project::RPhi)){ RPhi project has not single track branches...
        auto dd = df.Define("rndUniform", makeRandom,{"runNumber","eventNumber"})
                    .Define("E1_HLTTRACK_PT", "TMath::Sqrt(E1_TRACK_PX*E1_TRACK_PX + E1_TRACK_PY*E1_TRACK_PY)")
	                .Define("E2_HLTTRACK_PT", "TMath::Sqrt(E2_TRACK_PX*E2_TRACK_PX + E2_TRACK_PY*E2_TRACK_PY)")
                    .Define("E1_HLTTRACK_P",  "TMath::Sqrt(E1_TRACK_PX*E1_TRACK_PX + E1_TRACK_PY*E1_TRACK_PY + E1_TRACK_PZ*E1_TRACK_PZ)")
                    .Define("E2_HLTTRACK_P",  "TMath::Sqrt(E2_TRACK_PX*E2_TRACK_PX + E2_TRACK_PY*E2_TRACK_PY + E2_TRACK_PZ*E2_TRACK_PZ)")		
                    .Define("E1_TrackHLT1Info", makeTrackInfo, {"E1_HLTTRACK_P", "E1_HLTTRACK_PT", "E1_Hlt1TrackAllL0Decision_TOS"})
                    .Define("E2_TrackHLT1Info", makeTrackInfo, {"E2_HLTTRACK_P", "E2_HLTTRACK_PT", "E2_Hlt1TrackAllL0Decision_TOS"})
                    .Define("K_TrackHLT1Info",  makeTrackInfo, {"K_P", "K_PT", "K_Hlt1TrackAllL0Decision_TOS"})
                    .Define("Pi_TrackHLT1Info",  makeTrackInfo, {"Pi_P", "Pi_PT", "Pi_Hlt1TrackAllL0Decision_TOS"})
                    .Define("E1_Hlt1TrackAllL0Decision_TOS_update", evalTOSUpdate , { "E1_TrackHLT1Info", "B0_Hlt1TrackAllL0Decision_TOS" , "rndUniform"})
                    .Define("E2_Hlt1TrackAllL0Decision_TOS_update", evalTOSUpdate , { "E2_TrackHLT1Info", "B0_Hlt1TrackAllL0Decision_TOS" , "rndUniform"})
                    .Define("K_Hlt1TrackAllL0Decision_TOS_update",  evalTOSUpdate , { "K_TrackHLT1Info",  "B0_Hlt1TrackAllL0Decision_TOS" , "rndUniform"})
                    .Define("Pi_Hlt1TrackAllL0Decision_TOS_update", evalTOSUpdate , { "Pi_TrackHLT1Info", "B0_Hlt1TrackAllL0Decision_TOS" , "rndUniform"})
                    .Define("B0_Hlt1TrackAllL0Decision_TOS_update", "E1_Hlt1TrackAllL0Decision_TOS_update || E2_Hlt1TrackAllL0Decision_TOS_update || K_Hlt1TrackAllL0Decision_TOS_update || Pi_Hlt1TrackAllL0Decision_TOS_update");
        dd.Snapshot(_newTreeName.Data(), _newTFileName.Data() , DropColumns(dd.GetColumnNames(),dd.GetDefinedColumnNames(), blacklist ) );
    }
    if( ana == Analysis::EE && prj == Prj::RPhi){
        auto dd = df.Define("rndUniform", makeRandom,{"runNumber","eventNumber"})
                    .Define("E1_HLTTRACK_PT", "TMath::Sqrt(E1_TRACK_PX*E1_TRACK_PX + E1_TRACK_PY*E1_TRACK_PY)")
	                .Define("E2_HLTTRACK_PT", "TMath::Sqrt(E2_TRACK_PX*E2_TRACK_PX + E2_TRACK_PY*E2_TRACK_PY)")
                    .Define("E1_HLTTRACK_P",  "TMath::Sqrt(E1_TRACK_PX*E1_TRACK_PX + E1_TRACK_PY*E1_TRACK_PY + E1_TRACK_PZ*E1_TRACK_PZ)")
                    .Define("E2_HLTTRACK_P",  "TMath::Sqrt(E2_TRACK_PX*E2_TRACK_PX + E2_TRACK_PY*E2_TRACK_PY + E2_TRACK_PZ*E2_TRACK_PZ)")		
                    .Define("E1_TrackHLT1Info", makeTrackInfo, {"E1_HLTTRACK_P", "E1_HLTTRACK_PT", "E1_Hlt1TrackAllL0Decision_TOS"})
                    .Define("E2_TrackHLT1Info", makeTrackInfo, {"E2_HLTTRACK_P", "E2_HLTTRACK_PT", "E2_Hlt1TrackAllL0Decision_TOS"})
                    .Define("K1_TrackHLT1Info", makeTrackInfo, {"K1_P", "K1_PT", "K1_Hlt1TrackAllL0Decision_TOS"})
                    .Define("K2_TrackHLT1Info", makeTrackInfo, {"K2_P", "K2_PT", "K2_Hlt1TrackAllL0Decision_TOS"})
                    .Define("E1_Hlt1TrackAllL0Decision_TOS_update", evalTOSUpdate , { "E1_TrackHLT1Info", "Bs_Hlt1TrackAllL0Decision_TOS" , "rndUniform"})
                    .Define("E2_Hlt1TrackAllL0Decision_TOS_update", evalTOSUpdate , { "E2_TrackHLT1Info", "Bs_Hlt1TrackAllL0Decision_TOS" , "rndUniform"})
                    .Define("K1_Hlt1TrackAllL0Decision_TOS_update", evalTOSUpdate , { "K1_TrackHLT1Info", "Bs_Hlt1TrackAllL0Decision_TOS" , "rndUniform"})
                    .Define("K2_Hlt1TrackAllL0Decision_TOS_update", evalTOSUpdate , { "K2_TrackHLT1Info", "Bs_Hlt1TrackAllL0Decision_TOS" , "rndUniform"})
                    .Define("Bs_Hlt1TrackAllL0Decision_TOS_update", "E1_Hlt1TrackAllL0Decision_TOS_update || E2_Hlt1TrackAllL0Decision_TOS_update || K1_Hlt1TrackAllL0Decision_TOS_update || K2_Hlt1TrackAllL0Decision_TOS_update");
        dd.Snapshot(_newTreeName.Data(), _newTFileName.Data() , DropColumns(dd.GetColumnNames(),dd.GetDefinedColumnNames(), blacklist ) );
    }
    if( ana == Analysis::MM && prj == Prj::RK){
        auto dd = df.Define("rndUniform", makeRandom,{"runNumber","eventNumber"})
                    .Define("M1_TrackHLT1Info", makeTrackInfo, {"M1_P", "M1_PT", "M1_Hlt1TrackAllL0Decision_TOS"})
                    .Define("M2_TrackHLT1Info", makeTrackInfo, {"M2_P", "M2_PT", "M2_Hlt1TrackAllL0Decision_TOS"})
                    .Define("M1_TrackMuonHLT1Info", makeTrackInfo, {"M1_P", "M1_PT", "M1_Hlt1TrackMuonDecision_TOS"})
                    .Define("M2_TrackMuonHLT1Info", makeTrackInfo, {"M2_P", "M2_PT", "M2_Hlt1TrackMuonDecision_TOS"})
                    .Define("K_TrackHLT1Info",  makeTrackInfo, {"K_P", "K_PT", "K_Hlt1TrackAllL0Decision_TOS"})
                    .Define("K_TrackMuonHLT1Info", makeTrackInfo, {"K_P", "K_PT", "K_Hlt1TrackMuonDecision_TOS"})
                    .Define("M1_Hlt1TrackAllL0Decision_TOS_update", evalTOSUpdate , { "M1_TrackHLT1Info", "Bp_Hlt1TrackAllL0Decision_TOS", "rndUniform"})
                    .Define("M2_Hlt1TrackAllL0Decision_TOS_update", evalTOSUpdate , { "M2_TrackHLT1Info", "Bp_Hlt1TrackAllL0Decision_TOS", "rndUniform"})
                    .Define("K_Hlt1TrackAllL0Decision_TOS_update", evalTOSUpdate ,  { "K_TrackHLT1Info", "Bp_Hlt1TrackAllL0Decision_TOS", "rndUniform"})
                    .Define("Bp_Hlt1TrackAllL0Decision_TOS_update", "M1_Hlt1TrackAllL0Decision_TOS_update || M2_Hlt1TrackAllL0Decision_TOS_update || K_Hlt1TrackAllL0Decision_TOS_update")
                    .Define("M1_Hlt1TrackMuonDecision_TOS_update", evalTOSTrackMuonUpdate , { "M1_TrackMuonHLT1Info", "Bp_Hlt1TrackMuonDecision_TOS", "rndUniform"})
                    .Define("M2_Hlt1TrackMuonDecision_TOS_update", evalTOSTrackMuonUpdate , { "M2_TrackMuonHLT1Info", "Bp_Hlt1TrackMuonDecision_TOS", "rndUniform"})
                    .Define("K_Hlt1TrackMuonecision_TOS_update",   evalTOSTrackMuonUpdate ,   { "K_TrackMuonHLT1Info",  "Bp_Hlt1TrackMuonDecision_TOS", "rndUniform"})
                    .Define("Bp_Hlt1TrackMuonDecision_TOS_update",  "M1_Hlt1TrackMuonDecision_TOS_update || M2_Hlt1TrackMuonDecision_TOS_update || K_Hlt1TrackMuonecision_TOS_update")
                    .Define("JPs_Hlt1TrackMuonDecision_TOS_update", "M1_Hlt1TrackMuonDecision_TOS_update || M2_Hlt1TrackMuonDecision_TOS_update");                                        
        dd.Snapshot(_newTreeName.Data(), _newTFileName.Data() , DropColumns(dd.GetColumnNames(),dd.GetDefinedColumnNames(), blacklist ) );
    }
    if( ana == Analysis::MM && prj == Prj::RKst){
        auto dd = df.Define("rndUniform", makeRandom,{"runNumber","eventNumber"})
                    .Define("M1_TrackHLT1Info", makeTrackInfo, {"M1_P", "M1_PT", "M1_Hlt1TrackAllL0Decision_TOS"})
                    .Define("M2_TrackHLT1Info", makeTrackInfo, {"M2_P", "M2_PT", "M2_Hlt1TrackAllL0Decision_TOS"})
                    .Define("K_TrackHLT1Info",  makeTrackInfo, {"K_P", "K_PT", "K_Hlt1TrackAllL0Decision_TOS"})
                    .Define("Pi_TrackHLT1Info",  makeTrackInfo, {"Pi_P", "Pi_PT", "Pi_Hlt1TrackAllL0Decision_TOS"})
                    .Define("M1_TrackMuonHLT1Info", makeTrackInfo, {"M1_P", "M1_PT", "M1_Hlt1TrackMuonDecision_TOS"})
                    .Define("M2_TrackMuonHLT1Info", makeTrackInfo, {"M2_P", "M2_PT", "M2_Hlt1TrackMuonDecision_TOS"})
                    .Define("Pi_TrackMuonHLT1Info", makeTrackInfo, {"Pi_P", "Pi_PT", "Pi_Hlt1TrackMuonDecision_TOS"})
                    .Define("K_TrackMuonHLT1Info",  makeTrackInfo, {"K_P", "K_PT", "K_Hlt1TrackMuonDecision_TOS"})
                    .Define("M1_Hlt1TrackAllL0Decision_TOS_update", evalTOSUpdate , { "M1_TrackHLT1Info", "B0_Hlt1TrackAllL0Decision_TOS", "rndUniform"})
                    .Define("M2_Hlt1TrackAllL0Decision_TOS_update", evalTOSUpdate , { "M2_TrackHLT1Info", "B0_Hlt1TrackAllL0Decision_TOS", "rndUniform"})
                    .Define("K_Hlt1TrackAllL0Decision_TOS_update", evalTOSUpdate ,  { "K_TrackHLT1Info" , "B0_Hlt1TrackAllL0Decision_TOS", "rndUniform"})
                    .Define("Pi_Hlt1TrackAllL0Decision_TOS_update", evalTOSUpdate , { "Pi_TrackHLT1Info", "B0_Hlt1TrackAllL0Decision_TOS", "rndUniform"})
                    .Define("B0_Hlt1TrackAllL0Decision_TOS_update", "M1_Hlt1TrackAllL0Decision_TOS_update || M2_Hlt1TrackAllL0Decision_TOS_update || K_Hlt1TrackAllL0Decision_TOS_update || Pi_Hlt1TrackAllL0Decision_TOS_update")
                    .Define("M1_Hlt1TrackMuonDecision_TOS_update", evalTOSTrackMuonUpdate , { "M1_TrackMuonHLT1Info", "B0_Hlt1TrackMuonDecision_TOS", "rndUniform"})
                    .Define("M2_Hlt1TrackMuonDecision_TOS_update", evalTOSTrackMuonUpdate , { "M2_TrackMuonHLT1Info", "B0_Hlt1TrackMuonDecision_TOS", "rndUniform"})
                    .Define("K_Hlt1TrackMuonecision_TOS_update", evalTOSTrackMuonUpdate , { "K_TrackMuonHLT1Info", "B0_Hlt1TrackMuonDecision_TOS", "rndUniform"})
                    .Define("Pi_Hlt1TrackMuonecision_TOS_update", evalTOSTrackMuonUpdate , { "Pi_TrackMuonHLT1Info", "B0_Hlt1TrackMuonDecision_TOS", "rndUniform"})
                    .Define("B0_Hlt1TrackMuonDecision_TOS_update",  "M1_Hlt1TrackMuonDecision_TOS_update || M2_Hlt1TrackMuonDecision_TOS_update || K_Hlt1TrackMuonecision_TOS_update || Pi_Hlt1TrackMuonecision_TOS_update")
                    .Define("JPs_Hlt1TrackMuonDecision_TOS_update", "M1_Hlt1TrackMuonDecision_TOS_update || M2_Hlt1TrackMuonDecision_TOS_update")                                          
                    .Define("Kst_Hlt1TrackMuonDecision_TOS_update", "K_Hlt1TrackMuonecision_TOS_update || Pi_Hlt1TrackMuonecision_TOS_update");                                                 
        dd.Snapshot(_newTreeName.Data(), _newTFileName.Data() , DropColumns(dd.GetColumnNames(),dd.GetDefinedColumnNames(), blacklist ) );
    }
    if( ana == Analysis::MM && prj == Prj::RPhi){
        auto dd = df.Define("rndUniform", makeRandom,{"runNumber","eventNumber"})
                    .Define("M1_TrackHLT1Info", makeTrackInfo, {"M1_P", "M1_PT", "M1_Hlt1TrackAllL0Decision_TOS"})
                    .Define("M2_TrackHLT1Info", makeTrackInfo, {"M2_P", "M2_PT", "M2_Hlt1TrackAllL0Decision_TOS"})
                    .Define("K1_TrackHLT1Info", makeTrackInfo, {"K1_P", "K1_PT", "K1_Hlt1TrackAllL0Decision_TOS"})
                    .Define("K2_TrackHLT1Info", makeTrackInfo, {"K2_P", "K2_PT", "K2_Hlt1TrackAllL0Decision_TOS"})
                    .Define("M1_TrackMuonHLT1Info", makeTrackInfo, {"M1_P", "M1_PT", "M1_Hlt1TrackMuonDecision_TOS"})
                    .Define("M2_TrackMuonHLT1Info", makeTrackInfo, {"M2_P", "M2_PT", "M2_Hlt1TrackMuonDecision_TOS"})
                    .Define("K1_TrackMuonHLT1Info", makeTrackInfo, {"K1_P", "K1_PT", "K1_Hlt1TrackMuonDecision_TOS"})
                    .Define("K2_TrackMuonHLT1Info", makeTrackInfo, {"K2_P", "K2_PT", "K2_Hlt1TrackMuonDecision_TOS"})
                    .Define("M1_Hlt1TrackAllL0Decision_TOS_update", evalTOSUpdate , { "M1_TrackHLT1Info", "Bs_Hlt1TrackAllL0Decision_TOS", "rndUniform"})
                    .Define("M2_Hlt1TrackAllL0Decision_TOS_update", evalTOSUpdate , { "M2_TrackHLT1Info", "Bs_Hlt1TrackAllL0Decision_TOS", "rndUniform"})
                    .Define("K1_Hlt1TrackAllL0Decision_TOS_update", evalTOSUpdate , { "K1_TrackHLT1Info", "Bs_Hlt1TrackAllL0Decision_TOS", "rndUniform"})
                    .Define("K2_Hlt1TrackAllL0Decision_TOS_update", evalTOSUpdate , { "K2_TrackHLT1Info", "Bs_Hlt1TrackAllL0Decision_TOS", "rndUniform"})
                    .Define("Bs_Hlt1TrackAllL0Decision_TOS_update", "M1_Hlt1TrackAllL0Decision_TOS_update || M2_Hlt1TrackAllL0Decision_TOS_update || K1_Hlt1TrackAllL0Decision_TOS_update || K2_Hlt1TrackAllL0Decision_TOS_update")
                    .Define("M1_Hlt1TrackMuonDecision_TOS_update", evalTOSTrackMuonUpdate , { "M1_TrackMuonHLT1Info", "Bs_Hlt1TrackMuonDecision_TOS", "rndUniform"})
                    .Define("M2_Hlt1TrackMuonDecision_TOS_update", evalTOSTrackMuonUpdate , { "M2_TrackMuonHLT1Info", "Bs_Hlt1TrackMuonDecision_TOS", "rndUniform"})
                    .Define("K1_Hlt1TrackMuonDecision_TOS_update", evalTOSTrackMuonUpdate , { "K1_TrackMuonHLT1Info", "Bs_Hlt1TrackMuonDecision_TOS", "rndUniform"})
                    .Define("K2_Hlt1TrackMuonDecision_TOS_update", evalTOSTrackMuonUpdate , { "K2_TrackMuonHLT1Info", "Bs_Hlt1TrackMuonDecision_TOS", "rndUniform"})
                    .Define("Bs_Hlt1TrackMuonDecision_TOS_update",  "M1_Hlt1TrackMuonDecision_TOS_update || M2_Hlt1TrackMuonDecision_TOS_update || K1_Hlt1TrackMuonecision_TOS_update || K2_Hlt1TrackMuonecision_TOS_update")
                    .Define("JPs_Hlt1TrackMuonDecision_TOS_update", "M1_Hlt1TrackMuonDecision_TOS_update || M2_Hlt1TrackMuonDecision_TOS_update")                                          
                    .Define("Phi_Hlt1TrackMuonDecision_TOS_update", "K1_Hlt1TrackMuonDecision_TOS_update || K2_Hlt1TrackMuonDecision_TOS_update");                                                 
        dd.Snapshot(_newTreeName.Data(), _newTFileName.Data() , DropColumns(dd.GetColumnNames(),dd.GetDefinedColumnNames(), blacklist ) );
    }
    TFile f("TupleProcess.root","READ");
    auto  original =  (TTree*)f.Get("DecayTuple");
    TFile f2("TupleProcess_tmp.root", "READ");
    auto updated = (TTree*)f2.Get("DecayTuple");
    auto entries = original->GetEntries();
    if( original->GetEntries() != updated->GetEntries()){
        MessageSvc::Error("Severe error on Snapshots", "","EXIT_FAILURE");
    }else{
        f.Close();
        f2.Close();
    }
    //We have now to "delete DecayTuple from TupleProcess.root" (rootrm), and save back (rootmv) the new snapshotted one into the original file, delete back stuff.
    //1) Remove TTree from TupleProcess.root ( DecayTuple )
    MessageSvc::Info("rootrm TupleProcess.root:DecayTuple");
    IOSvc::runCommand(TString("rootrm TupleProcess.root:DecayTuple"));    
    //2) Move   TTree from TupleProcess_tmp.root to TupleProcess.root 
    MessageSvc::Info("rootmv TupleProcess_tmp.root:DecayTuple TupleProcess.root");
    moveTupleFromTo( "DecayTuple", "TupleProcess_tmp.root", "TupleProcess.root");
    //3) Remove TupleProcess_tmp.root file
    // MessageSvc::Info("rm TupleProcess_tmp.root");
    IOSvc::runCommand(TString("rm TupleProcess_tmp.root"));
    MessageSvc::Info("Checking entires of final stored tuple is preserved");
    TFile ff("TupleProcess.root","READ");
    auto finalTree = (TTree*)ff.Get("DecayTuple");
    if( finalTree == nullptr){
        MessageSvc::Error("Failure TCK alignment 12 tuples shuffling","","EXIT_FAILURE");
    }
    if( finalTree->GetEntries() != entries){
        MessageSvc::Error("Something went wrong", "","EXIT_FAILURE");
    }
    MessageSvc::Info("Closing file");
    ff.Close();
    return 0;
};

RNode HelperProcessing::ApplyWeights1D(RNode df, const vector<TH1DHistoAdder> &hisotgrams , TString inVar1D ,unsigned int i){
    if (i == hisotgrams.size()) return df;
    return HelperProcessing::ApplyWeights1D(df.Define(    hisotgrams.at(i).branchName(), hisotgrams.at(i), {inVar1D.Data()} ), hisotgrams, inVar1D.Data(), i + 1);
};
RNode HelperProcessing::ApplyBSWeights1D(RNode df, const vector<BSTH1DHistoAdder> &hisotgrams , TString inVar1D , unsigned int i){
    if (i == hisotgrams.size()) return df; //will do nothing if no bootstrapping enable
    return HelperProcessing::ApplyBSWeights1D(df.Define(    hisotgrams.at(i).branchName(), hisotgrams.at(i), {inVar1D.Data() } ), hisotgrams, inVar1D.Data(), i + 1);
};

RNode HelperProcessing::ApplyWeights2D(RNode df, const vector<TH2DHistAdder> &hisotgrams , TString inVar_X , TString inVar_Y, unsigned int i){
    if (i == hisotgrams.size()) return df;
    return HelperProcessing::ApplyWeights2D(df.Define(    hisotgrams.at(i).branchName(), hisotgrams.at(i), {inVar_X.Data(),inVar_Y.Data() } ), hisotgrams, inVar_X.Data(), inVar_Y.Data(), i + 1);
};

RNode HelperProcessing::ApplyWeights2D_3Args(RNode df, const vector<TH2DHistAdderL0EBremSplit> &hisotgrams , TString inVar_X , TString inVar_Y, TString catVar , unsigned int i){
    if (i == hisotgrams.size()) return df;
    return HelperProcessing::ApplyWeights2D_3Args(df.Define(    hisotgrams.at(i).branchName(), hisotgrams.at(i), {inVar_X.Data(),inVar_Y.Data(), catVar.Data() } ), hisotgrams, inVar_X.Data(), inVar_Y.Data(), catVar.Data(),  i + 1);
};

RNode HelperProcessing::ApplyBSWeights2D(RNode df, const vector<BSTH2DHistAdder> &hisotgrams , TString inVar_X , TString inVar_Y, unsigned int i){
    if (i == hisotgrams.size()) return df; //will do nothing if no bootstrapping enable
    return HelperProcessing::ApplyBSWeights2D(df.Define(    hisotgrams.at(i).branchName(), hisotgrams.at(i), {inVar_X.Data(),inVar_Y.Data() } ), hisotgrams, inVar_X.Data(), inVar_Y.Data(), i + 1);
};

void HelperProcessing::AddTRKWeights( EventType & _eventType, bool _useET){
    ROOT::DisableImplicitMT();
    /*
        TODO : 
            1. Enable this for MM as well ( trk/ bookkeping to DO and also maps themselves to check )
            2. Split wTRK_effCL and wTRK_effMC and bootstrapp properly with [0,1] bounds on efficiencies. 
                Currently bootstrapping is done firstly identifying 100% correlate bins, 
                then gaussian smearing around the value according to the error. Gaussian smearing for very low eff-ratio can become negative, 
                thus iterative gaussian smear is done until generated value is positive. 
        INFO : 
        Files in /eos/lhcb/wg/RD/RKstar/weights/v10/trk/vXXX/
        electron_longreco_effratio_strip2011_rx_nopid.root  electron_longreco_effratio_strip2015_rx_nopid.root  electron_longreco_effratio_strip2017_rx_nopid.root
        electron_longreco_effratio_strip2012_rx_nopid.root  electron_longreco_effratio_strip2016_rx_nopid.root  electron_longreco_effratio_strip2018_rx_nopid.root
        We have currently 3 trk maps available ```vDefault (used), vDefault_FixRun1 (to use in v11), vPaper (for Systematics to do)        
    */
    auto _weightHolder =  _eventType.GetWeightHolder();
    auto &_configHolder =  _eventType;  

    bool _addTRK = _weightHolder.IsOption("TRK") && _eventType.GetAna() == Analysis::EE;
    if(!_addTRK){
        MessageSvc::Warning("AddTRKWeights, Nothing to do");
        return;
    }
    ROOT::RDataFrame df( "DecayTuple","TupleProcess.root");
    RNode lastNode = df;
    lastNode = AppendTRKColumns( lastNode, _configHolder);
    lastNode.Snapshot("DecayTuple", "TupleProcess_tmp.root");
    //Checking entries consistency 
    TFile f1("TupleProcess.root","READ");
    auto  original =  (TTree*)f1.Get("DecayTuple");
    auto entries = original->GetEntries();
    TFile f2("TupleProcess_tmp.root", "READ");
    auto updated = (TTree*)f2.Get("DecayTuple");
    if( original->GetEntries() != updated->GetEntries()) MessageSvc::Error("Severe error on Snapshots", "","EXIT_FAILURE");
    else{ f1.Close(); f2.Close(); }
    MessageSvc::Info("rootrm TupleProcess.root:DecayTuple");
    IOSvc::runCommand(TString("rootrm TupleProcess.root:DecayTuple"));    
    //2) Move   TTree from TupleProcess_tmp.root to TupleProcess.root 
    MessageSvc::Info("rootmv TupleProcess_tmp.root:DecayTuple TupleProcess.root");
    moveTupleFromTo( "DecayTuple", "TupleProcess_tmp.root", "TupleProcess.root");
    IOSvc::runCommand(TString("rm TupleProcess_tmp.root"));
    TFile finalFile("TupleProcess.root","READ");
    auto finalTree = (TTree*)finalFile.Get("DecayTuple");
    if( finalTree == nullptr) MessageSvc::Error("Failure TRK weights adder","","EXIT_FAILURE");
    if( finalTree->GetEntries() != entries) MessageSvc::Error("Failure TRK weights adder", "","EXIT_FAILURE");    
    MessageSvc::Info("Closing file");
    finalFile.Close();
    return;    
}
RNode HelperProcessing::AppendTRKColumns( RNode df, ConfigHolder & _configHolder){
    auto _year = _configHolder.GetYear();
    //Baseline 
    TString _gngVer     = SettingDef::Tuple::gngVer;
    /* 
        Use Default_January2021_BpB0 as baseline ( most recent bug free existing ones )
        TODO : 
        Add wTRKCalib_Bp/B0 + _MC,_CL + Bootstrapping properly x interp/nointerp
        Current baseline is to use wTRKCalib non interpolated attachment from RK samples 
        Do the others for systematics @renato , @alex 
    */
    TString _trkversion  = "Default_January2021_BpB0";
    TString _modeDecay = SettingDef::Weight::TrkFromRKst ? "jpsikst" : "jpsik";
    TString _path    = fmt::format( "/eos/lhcb/wg/RD/RKstar/weights/v{0}/trk/v{1}/electron_longreco_effratio_strip20{2}_{3}_rx_nopid.root", _gngVer.Data(), _trkversion.Data() , to_string(_year).Data(), _modeDecay.Data() ) ;
    _path = IOSvc::XRootDFileName(_path);
    if( SettingDef::Weight::TrkFromRKst){
        MessageSvc::Info("AppendTRK columns [TrkFromRKst]", (TString)"using the wB0 version of the maps, path",  _path);
    }else{ 
        MessageSvc::Info("AppendTRK columns [TrkFromRK]  ", (TString)"using the wBp version of the maps, path",  _path);
    }
    /* 
        Paper version 
        TODO : Paper maps ( the ones without PID cuts applied on top , used for the tracking map paper) for systematics 
        TString _trkversion  = "vPaper";
        TString _path    = fmt::format( "/eos/lhcb/wg/RD/RKstar/weights/v{0}/trk/v{1}/electron_longreco_effratio_strip20{2}.root", _gngVer.Data(), _trkversion.Data() , to_string(_year).Data());
        TString _mapkey  = "heffratio"; 
        auto _infoMap = _path+":"+_mapkey;          
    */
    auto _GetGaussianizedMaps = []( TString _version , const TH3F & InputMap ){
        vector< TH3F> _histos; 
        TRandom3 _rnd;         
        //Identify the bin indexes which contains a duplicate <value,error>
        map< pair<float, float> , vector<int> > _VALUEREMAPPER;
        for (int i = 0; i <= InputMap.GetNcells() + 1; ++i) {
            auto pVal = std::make_pair<float, float>( InputMap.GetBinContent(i) , InputMap.GetBinError(i));
            if( _VALUEREMAPPER.find( pVal) == _VALUEREMAPPER.end()){
                _VALUEREMAPPER[pVal];
                _VALUEREMAPPER[pVal].push_back(i);
            }else{
                _VALUEREMAPPER[pVal].push_back(i);
            }
        }
        for( int i = 0; i < WeightDefRX::nBS ; ++i){
            int _seed =  _version.Hash() + i + 1;
            //Set Seed for this Version of wTRKCalib and BS iteration.
            _rnd.SetSeed( _seed);
            //Cast all bins to map<int, RndGauss value to assign to it accounting for the fact correlated bins gets same value
            map< int, float > values_to_assign; 
            //Fill up the values_to_assign map with a unique float value after gaussian smearing for this BS index!
            int nCases_LargeErrors = 0;
            int nTot_Cases =0;
            for( const auto & el : _VALUEREMAPPER){                
                float Value; 
                //If relative uncertainty is > 50 % , gaussian smearing is around Bayesian prior on 1 with 10% relative uncertainty
                if( el.first.second ==0 && el.first.first ==0){
                    //this is the case on over/underflow bins.[ we don't do anything there ]
                    Value = 0.; 
                }else{
                    nTot_Cases++;
                    if( el.first.second/el.first.first >0.5 ){
                        nCases_LargeErrors++;
                    }
                    Value = _rnd.Gaus( el.first.first, el.first.second); //gaus( value, error in that bin) 
                    //Keep generating until value is negative, must be >0 since map is eff(data)/eff(mc)
                    while( Value<0.){
                        Value = _rnd.Gaus( el.first.first, el.first.second); //gaus( value, error in that bin)                
                    }
                }
                for( auto & bIndex : el.second){
                    values_to_assign[bIndex] = Value;
                }
            }
            MessageSvc::Warning( TString::Format("BS loop %i nBins huge relErrors ( err/Val > 50%) : %i/%i", i, nCases_LargeErrors, nTot_Cases) );
            TH3F * hNew =  (TH3F*)InputMap.Clone( TString(InputMap.GetName())+"["+ to_string(i)+"]");
            hNew->Reset("ICES");      
            for( const auto & bin_trk : values_to_assign){
                hNew->SetBinContent( bin_trk.first, bin_trk.second);
            }
            hNew->SetDirectory(0);
            _histos.push_back( *hNew );            
        }
        return _histos;
    };
	TString _mapkey_mc  	= "heffmc";
	TString _mapkey_data  	= "heffdata";
    TString _mapkey_ratio  	= "heffratio";
    auto _infoMap_mc 	= _path+":"+_mapkey_mc;
    auto _infoMap_data 	= _path+":"+_mapkey_data;
    auto _infoMap_ratio = _path+":"+_mapkey_ratio;
    TFile * _inFile  = IOSvc::OpenFile(_path , OpenMode::READ);
	TH3F * _histTRK_data  = (TH3F*)_inFile->Get(_mapkey_data);
    if( _histTRK_data == nullptr){
        _inFile->ls();
        MessageSvc::Error("GetWeightMapTRK", (TString) "Map", _mapkey_data, "does no exist");
	}
	TH3F * _histTRK_mc  = (TH3F*)_inFile->Get(_mapkey_mc);
    if( _histTRK_mc == nullptr){
        _inFile->ls();
        MessageSvc::Error("GetWeightMapTRK", (TString) "Map", _mapkey_mc, "does no exist");
    }
    TH3F * _histTRK_ratio  = (TH3F*)_inFile->Get(_mapkey_ratio);
    if( _histTRK_ratio == nullptr){
        _inFile->ls();
        MessageSvc::Error("GetWeightMapTRK", (TString) "Map", _mapkey_ratio, "does no exist");
    }
    //=============================================================
    // Actual HistoAdders (TH3F and vector<TH3F>)
    //=============================================================    
    TH3FHistoAdder _wE1TRK_mc 	( * _histTRK_mc, 	"wTRKCalibE1_MC", 	 false, _infoMap_mc 	);
    TH3FHistoAdder _wE1TRK_data ( * _histTRK_data, 	"wTRKCalibE1_CL", 	 false, _infoMap_data   );
    TH3FHistoAdder _wE1TRK_ratio( * _histTRK_ratio, "wTRKCalibE1_ratio", false, _infoMap_ratio  );

    TH3FHistoAdder _wE2TRK_mc   ( * _histTRK_mc, 	"wTRKCalibE2_MC", 	 false, _infoMap_mc 	);
    TH3FHistoAdder _wE2TRK_data ( * _histTRK_data, 	"wTRKCalibE2_CL",    false, _infoMap_data   );
    TH3FHistoAdder _wE2TRK_ratio( * _histTRK_ratio, "wTRKCalibE2_ratio", false, _infoMap_ratio  );
    //Interpolated version
    // TH3FHistoAdder _wiE1TRK_mc   ( * _histTRK_mc, 	"wiTRKCalibE1_MC", 	true, _infoMap_mc 		);
    // TH3FHistoAdder _wiE2TRK_mc   ( * _histTRK_mc, 	"wiTRKCalibE2_MC", 	true, _infoMap_mc 		);
    // TH3FHistoAdder _wiE1TRK_data ( * _histTRK_data, 	"wiTRKCalibE1_CL", 	true, _infoMap_data 	);
    // TH3FHistoAdder _wiE2TRK_data ( * _histTRK_data, 	"wiTRKCalibE2_CL", 	true, _infoMap_data 	);
    // TH3FHistoAdder _wiE1TRK_ratio( * _histTRK_ratio, "wiTRKCalibE1_ratio", 	true, _infoMap_ratio 	);
    // TH3FHistoAdder _wiE2TRK_ratio( * _histTRK_ratio, "wiTRKCalibE2_ratio", 	true, _infoMap_ratio 	);

    //Load maps for bootstrapping
    vector<TH3F> _histos_mc ; 
    vector<TH3F> _histos_data ; 
    vector<TH3F> _histos_ratio ; 
    if( SettingDef::Weight::useBS){
        _histos_mc = 	_GetGaussianizedMaps( _trkversion, *_histTRK_mc 	);    
        _histos_data = 	_GetGaussianizedMaps( _trkversion, *_histTRK_data 	);    
        _histos_ratio = _GetGaussianizedMaps( _trkversion, *_histTRK_ratio 	);
    }
    //The BS (noi)  maps for E1/E2
    BSTH3FHistoAdder _wE1TRK_BS_mc		( _histos_mc, 		"wTRKCalibE1_BS_MC", 	false, _infoMap_mc 		);
    BSTH3FHistoAdder _wE1TRK_BS_data	( _histos_data, 	"wTRKCalibE1_BS_CL", 	false, _infoMap_data 	);
    BSTH3FHistoAdder _wE1TRK_BS_ratio	( _histos_ratio, 	"wTRKCalibE1_BS_ratio", false, _infoMap_ratio 	);
    BSTH3FHistoAdder _wE2TRK_BS_mc		( _histos_mc, 		"wTRKCalibE2_BS_MC", 	false, _infoMap_mc 		);
    BSTH3FHistoAdder _wE2TRK_BS_data	( _histos_data, 	"wTRKCalibE2_BS_CL", 	false, _infoMap_data 	);
    BSTH3FHistoAdder _wE2TRK_BS_ratio	( _histos_ratio, 	"wTRKCalibE2_BS_ratio", false, _infoMap_ratio 	);

    //The BS (i)  maps for E1/E2
    // BSTH3FHistoAdder _wiE1TRK_BS_mc		( _histos_mc, 		"wiTRKCalibE1_BS_MC", 	 true, _infoMap_mc 		);
    // BSTH3FHistoAdder _wiE1TRK_BS_data	( _histos_data, 	"wiTRKCalibE1_BS_CL", 	 true, _infoMap_data 	);
    // BSTH3FHistoAdder _wiE1TRK_BS_ratio	( _histos_ratio, 	"wiTRKCalibE1_BS_ratio", true, _infoMap_ratio 	);
    // BSTH3FHistoAdder _wiE2TRK_BS_mc		( _histos_mc, 		"wiTRKCalibE2_BS_MC", 	 true, _infoMap_mc 		);
    // BSTH3FHistoAdder _wiE2TRK_BS_data	( _histos_data, 	"wiTRKCalibE2_BS_CL", 	 true, _infoMap_data 	);
    // BSTH3FHistoAdder _wiE2TRK_BS_ratio	( _histos_ratio, 	"wiTRKCalibE2_BS_ratio", true, _infoMap_ratio 	);


    auto _pt_to_use = []( int true_id_particle, double _true_pt, double _recoed_pt){
        //ghost electron track, no TRUEPT information
        if( true_id_particle == 0){ return _recoed_pt; }else{ return _true_pt; }
        return _true_pt;
    };    
    TString _combined_mc 	= Form( "%s * %s", _wE1TRK_mc.branchName(),    _wE2TRK_mc.branchName()    );
    TString _combined_data 	= Form( "%s * %s", _wE1TRK_data.branchName(),  _wE2TRK_data.branchName()  );
    TString _combined_ratio = Form( "%s * %s", _wE1TRK_ratio.branchName(), _wE2TRK_ratio.branchName() );
    
    // TString _combined_i_mc 	    = Form( "%s * %s", _wiE1TRK_mc.branchName(),    _wiE2TRK_mc.branchName()    );
    // TString _combined_i_data 	= Form( "%s * %s", _wiE1TRK_data.branchName(),  _wiE2TRK_data.branchName()  );
    // TString _combined_i_ratio   = Form( "%s * %s", _wiE1TRK_ratio.branchName(), _wiE2TRK_ratio.branchName() );

    auto lastNode = df.Define( "E1_wTRK_PT", _pt_to_use, {"E1_TRUEID", "E1_TRUEPT", "E1_PT"} ) //switch between the PT to use (E1)
                      .Define( "E1_wTRK_PHI", "TMath::ATan2(E1_PY,E1_PX)") //declare the PHI (not available in tuples E1)
                      .Define( "E2_wTRK_PT", _pt_to_use, {"E2_TRUEID", "E2_TRUEPT", "E2_PT"} ) //switch between the PT to use (E2)
                      .Define( "E2_wTRK_PHI", "TMath::ATan2(E2_PY,E2_PX)") //declare the PHI (not available in tuples E2)
                      .Define( _wE1TRK_mc.branchName(), 	_wE1TRK_mc , 	{"E1_wTRK_PHI", "E1_ETA", "E1_wTRK_PT"}) //Use Functor TH3F to attach weight 3D [x,y,z] inputs [E1]  
                      .Define( _wE1TRK_data.branchName(), 	_wE1TRK_data , 	{"E1_wTRK_PHI", "E1_ETA", "E1_wTRK_PT"})  
                      .Define( _wE1TRK_ratio.branchName(), 	_wE1TRK_ratio , {"E1_wTRK_PHI", "E1_ETA", "E1_wTRK_PT"}) 
                      .Define( _wE2TRK_mc.branchName(), 	_wE2TRK_mc , 	{"E2_wTRK_PHI", "E2_ETA", "E2_wTRK_PT"}) //Use Functor TH3F to attach weight 3D [x,y,z] inputs [E2]
                      .Define( _wE2TRK_data.branchName(), 	_wE2TRK_data , 	{"E2_wTRK_PHI", "E2_ETA", "E2_wTRK_PT"}) 
                      .Define( _wE2TRK_ratio.branchName(), 	_wE2TRK_ratio , {"E2_wTRK_PHI", "E2_ETA", "E2_wTRK_PT"});
                      //TODO : Interpolated branches 
                      //.Define( _wiE1TRK_mc.branchName(), 	_wiE1TRK_mc , 	{"E1_wTRK_PHI", "E1_ETA", "E1_wTRK_PT"})
                      //.Define( _wiE1TRK_data.branchName(), 	_wiE1TRK_data ,	{"E1_wTRK_PHI", "E1_ETA", "E1_wTRK_PT"})
                      //.Define( _wiE1TRK_ratio.branchName(), _wiE1TRK_ratio ,{"E1_wTRK_PHI", "E1_ETA", "E1_wTRK_PT"})
                      //.Define( _wiE2TRK_mc.branchName(), 	_wiE2TRK_mc , 	{"E2_wTRK_PHI", "E2_ETA", "E2_wTRK_PT"})
                      //.Define( _wiE2TRK_data.branchName(), 	_wiE2TRK_data ,	{"E2_wTRK_PHI", "E2_ETA", "E2_wTRK_PT"})
                      //.Define( _wiE2TRK_ratio.branchName(), _wiE2TRK_ratio ,{"E2_wTRK_PHI", "E2_ETA", "E2_wTRK_PT"});
    lastNode = lastNode.Define( "wTRKCalib_MC", _combined_mc.Data() 	)
    	               .Define( "wTRKCalib_CL", _combined_data.Data() 	)
                       .Define( "wTRKCalib", 	_combined_ratio.Data() 	);
                       //TODO : interpolated branches
                       //.Define( "wiTRKCalib_MC",_combined_i_mc.Data())
                       //.Define( "wiTRKCalib_CL",_combined_i_data.Data())
                       //.Define( "wiTRKCalib",   _combined_i_ratio.Data());

    if( SettingDef::Weight::useBS && _histos_ratio.size() != 0 ){
        lastNode = lastNode.Define(_wE1TRK_BS_ratio.branchName(), _wE1TRK_BS_ratio, {"E1_wTRK_PHI", "E1_ETA", "E1_wTRK_PT"} )
                           .Define(_wE2TRK_BS_ratio.branchName(), _wE2TRK_BS_ratio, {"E2_wTRK_PHI", "E2_ETA", "E2_wTRK_PT"} );
                           //TODO : Interpolated branches
                           //.Define(_wiE1TRK_BS_ratio.branchName(), _wiE1TRK_BS_ratio, {"E1_wTRK_PHI", "E1_ETA", "E1_wTRK_PT"} )
                           //.Define(_wiE2TRK_BS_ratio.branchName(), _wiE2TRK_BS_ratio, {"E2_wTRK_PHI", "E2_ETA", "E2_wTRK_PT"} );
        auto  Multiply = []( const ROOT::VecOps::RVec< double> & v1, const ROOT::VecOps::RVec< double> & v2){
            ROOT::VecOps::RVec< double> v3;
            if( v1.size() != v2.size()){
                MessageSvc::Error("Invalid product vector columns", "","EXIT_FAILURE");
            }
            for( int i = 0; i < v1.size(); ++i){
                v3.push_back( v1[i] * v2[i]);
            }
            return v3;
        };
        lastNode = lastNode.Define( "wTRKCalib_BS",     Multiply, { _wE1TRK_BS_ratio.branchName() ,  _wE2TRK_BS_ratio.branchName() }   );
        			 //    .Define( "wiTRKCalib_BS",    Multiply, { _wiE1TRK_BS_ratio.branchName() , _wiE2TRK_BS_ratio.branchName()}   );
    }

    MessageSvc::Info("AppendTRK columns ", (TString)"done");
    return lastNode; 
} 


void HelperProcessing::AddPIDWeights( EventType & _eventType, bool _useET){
    /*
        RK,RKst : wPIDCalib, wiPIDCalib 
        RPhi    : wPIDCalib, wiPIDCalib , wPIDCalibKDE
        TODO : Bootstrapping mode for RK,RKst nor supported yet in KDE mode [ requires redoing maps with BS-fits to e-mode]
    */
    
    //=============================================================
    // booleans for PID map types
    //=============================================================
    auto _weightHolder =  _eventType.GetWeightHolder();
    auto &_configHolder =  _eventType;  
    
    vector<string>   _wildCards       = {};
    vector< string > _colsToBeAdded = {};
    bool _addPID      = _weightHolder.IsOption("PID") && !(_configHolder.GetSample().Data() == "LPT"); 
    if( !_addPID){
        MessageSvc::Warning("HelperProcessing::AddPIDWeights", (TString)"nothing to do");
    }
    auto print_setup = []( const map< TString, bool> _mymap ){
        MessageSvc::Debug("Configuration of PID attachment");
        for( const auto & el : _mymap ){
            MessageSvc::Warning("AddPID", TString::Format("%s : %s", el.first.Data(), el.second? "True": "False"));
        }
    };

    // ROOT::DisableImplicitMT();
    ROOT::EnableImplicitMT(4);
    ROOT::RDataFrame df( "DecayTuple","TupleProcess.root");

    //The core of attaching weights , see details in the method
    RNode lastNode = AppendPIDColumns( df,  _configHolder ,  _wildCards );

    MessageSvc::Info(Color::Cyan, "AddPIDWeights", "Snapshotting..");
    // lastNode.Snapshot("DecayTuple", "TupleProcess_tmp.root", DropColumnsWildcard(lastNode.GetColumnNames(), lastNode.GetDefinedColumnNames(), _wildCards));
    lastNode.Snapshot("DecayTuple", "TupleProcess_tmp.root", DropColumnsWildcard(lastNode.GetColumnNames(), _wildCards));

    //=============================================================
    //Checking entries consistency 
    //=============================================================
    TFile f1("TupleProcess.root","READ");
    auto original = (TTree*)f1.Get("DecayTuple");
    auto entries  = original->GetEntries();
    TFile f2("TupleProcess_tmp.root", "READ");
    auto updated  = (TTree*)f2.Get("DecayTuple");
    if( original->GetEntries() != updated->GetEntries() ) {
	    MessageSvc::Error("AddPIDWeights", "Severe error on Snapshots, original and updates entries are different!","EXIT_FAILURE");
    }
    else { 
        delete original;
        delete updated;
	    f1.Close(); 
	    f2.Close(); 
    }
    
    //=============================================================
    // Move TTree from TupleProcess_tmp.root to TupleProcess.root 
    //=============================================================
    MessageSvc::Info(Color::Cyan, "AddPIDWeights", "rootrm TupleProcess.root:DecayTuple");
    IOSvc::runCommand(TString("rootrm TupleProcess.root:DecayTuple"));    

    MessageSvc::Info(Color::Cyan, "AddPIDWeights", "rootmv TupleProcess_tmp.root:DecayTuple TupleProcess.root");
    moveTupleFromTo( "DecayTuple", "TupleProcess_tmp.root", "TupleProcess.root");
    IOSvc::runCommand(TString("rm TupleProcess_tmp.root"));

    //=============================================================
    // check final Tree
    //=============================================================
    TFile  finalFile("TupleProcess.root","READ");
    TTree* finalTree = (TTree*)finalFile.Get("DecayTuple");
    if( finalTree == nullptr) {
	    MessageSvc::Error("Failure PID weight adder","","EXIT_FAILURE");
    }
    if( finalTree->GetEntries() != entries) {
	    MessageSvc::Error("Failure PID weight adder", "","EXIT_FAILURE");
    }
    MessageSvc::Info(Color::Green, "AddPIDWeights", "Closing file, PID Attachment complete!");
    delete finalTree;
    finalFile.Close();

    return;
}

ROOT::VecOps::RVec<int> addRndPoisson( const UInt_t & runNumber, const ULong64_t & eNumber ){
    TRandom3 rnd;   
    rnd.SetSeed(runNumber * eNumber);
    ROOT::VecOps::RVec<int> rndPoisson;
    rndPoisson.reserve(100);
    for( int i =0; i < 100; ++i){
        rndPoisson.push_back((int)rnd.PoissonD(1));
    }
    return rndPoisson;
};

RNode HelperProcessing::AppendBSColumns( RNode df ){
    if(df.HasColumn("RndPoisson2")) { MessageSvc::Info("AppendBSColumns", TString("Nothing to do") ); return df; };
    if (ROOT::IsImplicitMTEnabled() ) { //BE SURE THIS GO SINGLE THREADED , TRandom3 is not MT friendly.
        MessageSvc::Error("Failure AppendBSColumns needs ImplicitMT disabled", "","EXIT_FAILURE");
    }
    auto lastNode = RNode(df);
    lastNode = lastNode.Define("RndPoisson2", addRndPoisson, {"runNumber", "eventNumber"}); 

    return lastNode;
}


RNode HelperProcessing::AppendPIDColumns( RNode df, ConfigHolder & _configHolder , vector<string> & _wildCards ){
    //checker for function to work properly
    vector< string > _colsToBeAdded = {};
    auto print_setup = []( const map< TString, bool> _mymap ){
        MessageSvc::Debug("Configuration of PID attachment");
        for( const auto & el : _mymap ){
            MessageSvc::Warning("AddPID", TString::Format("%s : %s", el.first.Data(), el.second? "True": "False"));
        }
    };
    WeightHolder _weightHolder(_configHolder, "PID");
    /*
        usePID    = wPID no interpolated with F&C electrons
        useInterp = make wiPID with F&C electrons
        useKDE    = make wPIDKDE [sPlot pas/tot] with F&C electrons
        useWeight = use data/sim ratio weights for F&C electrons
        useAK     = Alternative Kernel  for KDE setup [RPhi specific]
        useAB     = Alternative Binning for KDE setup [RPhi specific]
        useAN     = Alternative nTracks binning [KDE setup] [RPhi specific]
        useBS     = Inferred from useBS flag on executable [append the wPIDCalib_BS]  branches as well
        PIDVerbose= Attach more stuff on tuples for debugging  
    */
    std::map< Prj, std::map< TString, bool > >  _what_to_attach{ 
        /* RPhi uses as baseline the KDE PIDCalib maps + F&C electrons */
        {Prj::RPhi,{ {"usePID"    , false },
                     {"useInterp" , false },
                     {"useKDE"    , true },
                     {"useWeight" , SettingDef::Weight::useMCRatioPID },
                     {"useAK"     , false },
                     {"useAB"     , false },
                     {"useAN"     , false },
                     {"useBS"     , SettingDef::Weight::useBS },
                     {"PIDVerbose", true } } } ,
        /* RK, RKst uses as baseline the PID binned maps + F&C electrons */
        {Prj::RK,   {{"usePID"    , true },
                    {"useInterp" , true },
                    {"useKDE"    , SettingDef::Weight::config.Contains("kde")},//TODO: redo KDE with BS configuration from Sebastian
                    {"useWeight" , SettingDef::Weight::useMCRatioPID },
                    {"useAK"     , false},
                    {"useAB"     , false},
                    {"useAN"     , false},
                    {"useBS"     , SettingDef::Weight::useBS},
                    {"PIDVerbose", false}}},
        {Prj::RKst, {{"usePID"    , true},
                     {"useInterp" , true},
                     {"useKDE"    , SettingDef::Weight::config.Contains("kde")},//TODO: redo KDE with BS configuration from Sebastian
                     {"useWeight" , SettingDef::Weight::useMCRatioPID },
                     {"useAK"     , false},
                     {"useAN"     , false},
                     {"useAB"     , false},
                     {"useBS"     , SettingDef::Weight::useBS},
                     {"PIDVerbose", false}}}
    };
    auto _pidattachingConf = _what_to_attach.at( _configHolder.GetProject() ); 
    print_setup(_pidattachingConf );
    bool _usePID     = _pidattachingConf.at("usePID");
    bool _useInterp  = _pidattachingConf.at("useInterp");
    bool _useKDE     = _pidattachingConf.at("useKDE");
    bool _useWeight  = (_pidattachingConf.at("useWeight") && _configHolder.GetAna() == Analysis::EE);
    bool _useAK      = _pidattachingConf.at("useAK");
    bool _useAN      = _pidattachingConf.at("useAN");
    bool _useAB      = _pidattachingConf.at("useAB");
    bool _useBS      = _pidattachingConf.at("useBS");
    bool _PIDVerbose = _pidattachingConf.at("PIDVerbose");
    if (_usePID)               _colsToBeAdded.push_back("wPIDCalib");
    if (_usePID && _useInterp) _colsToBeAdded.push_back("wiPIDCalib");
    if (_usePID && _useBS)     _colsToBeAdded.push_back("wPIDCalib_BS");

    if (_usePID && _useWeight)               _colsToBeAdded.push_back("wPIDCalib_Weight");
    if (_usePID && _useInterp && _useWeight) _colsToBeAdded.push_back("wiPIDCalib_Weight");
    if (_usePID && _useBS && _useWeight)     _colsToBeAdded.push_back("wPIDCalib_Weight_BS");
    if (_usePID && _useBS && _useWeight && _useInterp) _colsToBeAdded.push_back("wiPIDCalib_Weight_BS");


    if (_useKDE)                             _colsToBeAdded.push_back("wPIDCalibKDE");
    if (_useKDE && _useAK)                   _colsToBeAdded.push_back("wPIDCalibKDE_AK");
    if (_useKDE && _useWeight)               _colsToBeAdded.push_back("wPIDCalibKDE_Weight");
    if (_useKDE && _useAN)                   _colsToBeAdded.push_back("wPIDCalibKDE_AN");
    if (_useKDE && _useAB)                   _colsToBeAdded.push_back("wPIDCalibKDE_AB");
    if (_useKDE && _useBS)                   _colsToBeAdded.push_back("wPIDCalibKDE_BS");
    if (_useKDE && _useBS && _useWeight)     _colsToBeAdded.push_back("wPIDCalibKDE_WeightBS");

    auto _proj = _configHolder.GetProject();
    auto _ana  = _configHolder.GetAna();
    auto _BranchNames = _configHolder.GetParticleNames() ; 
    TString _had1Name  = _BranchNames.at("H1");
    TString _had2Name  = _BranchNames.at("H2");
    TString _lepName1   = _BranchNames.at("L1");
    TString _lepName2   = _BranchNames.at("L2");
    auto _tidToUse = []( const int true_id_particle, const int _idParticle){    	
        auto _idReturn = (true_id_particle == 0 ) ? TMath::Abs(_idParticle) : TMath::Abs(true_id_particle);
        return _idReturn;
    };
    /*
        Make the Abs value of it
    */
    auto _idToUse = []( const int _idParticle){    
        return TMath::Abs(_idParticle);
    };
    /*
        If ID == E and TRUEID != E  and Has_Brem ==1 , use TRACKP for the misID map!
        convert electron P which doesnt stem from MC true electrons to track P if brem == 1
        the maps we use are parametrised with particle momentum, the electron momentum is 
        p_electron(brem1) = p_particle + p_brem
        so using p_particle == p_track might be a better way to go. 
        If TRUEID != electron and Brem == 1 , return P as the TRACK_P 
    */       
    auto _pToUse = []( const int _tidParticle, const int _brem, const double _PT, const double _P, const double _trackPX, const double _trackPY, const double _trackPZ ){ 
       if (TMath::Abs(_tidParticle) != PDG::ID::E) { //Electron MisID case stays at P,Eta parametrization for the moment
            if (_brem == 1) {
                return TMath::Sqrt( TMath::Sq(_trackPX) + TMath::Sq(_trackPY) + TMath::Sq(_trackPZ) );
            }
            else if (_brem == 0) {
                return _P;
            }
        }
        else if (TMath::Abs(_tidParticle) == PDG::ID::E) { //Use PT for Electron ID maps in case that usePT is activated.
            if(SettingDef::Weight::usePIDPTElectron){
                return _PT;
            }
            else {
                return _P;
            }
        }
    };
    /*
        Convert HasBrem column from boolean to integer value
    */    
    auto _convBool = []( const bool _brem ){
        int _ret = _brem  ? 1: 0; 
        return _ret;
    };
    //====================================================================================================
    
    auto _getBranch= []( TString _lepBranchName, TString _appendix){
        return TString::Format( "%s%s", _lepBranchName.Data(), _appendix.Data());
    };
    TString _lep1TrueID = _getBranch(_lepName1 , "_TRUEID" );
    TString _lep1ID     = _getBranch(_lepName1 , "_ID" );
    TString _lep1P      = _getBranch(_lepName1 , "_P" );
    TString _lep1ETA    = _getBranch(_lepName1 , "_ETA");

    TString _lep2TrueID = _getBranch(_lepName2 , "_TRUEID" );
    TString _lep2ID     = _getBranch(_lepName2 , "_ID" );
    TString _lep2P      = _getBranch(_lepName2 , "_P" );
    TString _lep2ETA    = _getBranch(_lepName2 , "_ETA");

    TString _had1TrueID = _getBranch(_had1Name , "_TRUEID" );
    TString _had1ID     = _getBranch(_had1Name , "_ID" );
    TString _had1P      = _getBranch(_had1Name , "_P" );
    TString _had1ETA    = _getBranch(_had1Name , "_ETA");

    TString _had2TrueID = _getBranch(_had2Name , "_TRUEID" );
    TString _had2ID     = _getBranch(_had2Name , "_ID" );
    TString _had2P      = _getBranch(_had2Name , "_P" );
    TString _had2ETA    = _getBranch(_had2Name , "_ETA");
 
    //Define some temporary branches concerning the P, TRUEID, ID to use for the functors 
    /*
        _tidToUse  = force the column to be the ID if TRUEID ==0 
        _idToUse   = just rename it
    */
    auto lastNode = df.Define( "L1_tmpWPIDCalib_TID",  _tidToUse, {_lep1TrueID.Data(), _lep1ID.Data()} ) // switch between the TIDs to use 
	              .Define( "L1_tmpWPIDCalib_ID",      _idToUse,  {_lep1ID.Data()} )                     // switch between the TIDs to use 

	              .Define( "L2_tmpWPIDCalib_TID",  _tidToUse, {_lep2TrueID.Data(), _lep2ID.Data()} )
	              .Define( "L2_tmpWPIDCalib_ID",   _idToUse,  {_lep2ID.Data()} )

	              .Define( "H1_tmpWPIDCalib_TID",  _tidToUse, {_had1TrueID.Data(), _had1ID.Data()} )
	              .Define( "H1_tmpWPIDCalib_ID",   _idToUse,  {_had1ID.Data()} )
	              .Define( "H1_tmpWPIDCalib_Brem", "0" );   // define a common branch for the BREM

    //=============================================================
    // for RPhi and RKst need H2 branches
    //=============================================================
    if (_configHolder.GetNBodies() > 3) {
	    lastNode = lastNode.Define( "H2_tmpWPIDCalib_TID",  _tidToUse, {_had2TrueID.Data(), _had2ID.Data()} )
	                       .Define( "H2_tmpWPIDCalib_ID",   _idToUse,  {_had2ID.Data()} )
	                       .Define( "H2_tmpWPIDCalib_Brem", "0" );
    }

    //=============================================================
    // define a common branch for the BREM (L1)
    //=============================================================
    if (_configHolder.GetAna() == Analysis::MM){
	    lastNode = lastNode.Define( "L1_tmpWPIDCalib_Brem", "0" )
                           .Define( "L2_tmpWPIDCalib_Brem", "0" )
                           .Define( "L1_tmpWPIDCalib_P",    _lep1P.Data() )
                           .Define( "L2_tmpWPIDCalib_P",    _lep2P.Data() );
    } else if (_configHolder.GetAna()== Analysis::EE) {
        /*
            _pToUse : if the electron hasBremAdded and TRUEID != electron, we use the TRACKP for misID maps attaching
        */
	    lastNode = lastNode.Define( "L1_tmpWPIDCalib_Brem", _convBool, {"E1_HasBremAdded"} )
                           .Define( "L2_tmpWPIDCalib_Brem", _convBool, {"E2_HasBremAdded"} )
                           .Define( "L1_tmpWPIDCalib_P",    _pToUse, {"L1_tmpWPIDCalib_TID", "L1_tmpWPIDCalib_Brem", "E1_PT", "E1_P", "E1_TRACK_PX", "E1_TRACK_PY", "E1_TRACK_PZ"} )
                           .Define( "L2_tmpWPIDCalib_P",    _pToUse, {"L2_tmpWPIDCalib_TID", "L2_tmpWPIDCalib_Brem", "E2_PT", "E2_P", "E2_TRACK_PX", "E2_TRACK_PY", "E2_TRACK_PZ"} );
    }
    //==========================================================================================================================
    // Define the columns to be passed to the PIDHistoAdder functors for each particle species. See signature of PIDHistoAdder operator()
    // Default here is P, ETA, nTRACKS, TRUEID, ID, Brem information
    //==========================================================================================================================
    map< TString, 	ROOT::Detail::RDF::ColumnNames_t > _Inputs{
        { "L1" , {"L1_tmpWPIDCalib_P", _lep1ETA.Data(), "nTracks", "L1_tmpWPIDCalib_TID", "L1_tmpWPIDCalib_ID", "L1_tmpWPIDCalib_Brem"}},
        { "L2" , {"L2_tmpWPIDCalib_P", _lep2ETA.Data(), "nTracks", "L2_tmpWPIDCalib_TID", "L2_tmpWPIDCalib_ID", "L2_tmpWPIDCalib_Brem"}},
        { "H1" , {_had1P.Data(),       _had1ETA.Data(), "nTracks", "H1_tmpWPIDCalib_TID", "H1_tmpWPIDCalib_ID", "H1_tmpWPIDCalib_Brem"}}, 
        { "H2" , {_had2P.Data(),       _had2ETA.Data(), "nTracks", "H2_tmpWPIDCalib_TID", "H2_tmpWPIDCalib_ID", "H2_tmpWPIDCalib_Brem"}}
    }; 



    if (_usePID && !_useWeight && !_useInterp) {
        //=============================================================
        // Actual HistoAdders PIDCalib (manual binning)
        //=============================================================    
        PIDHistoAdder _wPIDCalibL1( _weightHolder, TString::Format( "wPIDCalib%s",_lepName1.Data()), "" ); // loads all maps
        PIDHistoAdder _wPIDCalibL2( _wPIDCalibL1,  TString::Format( "wPIDCalib%s",_lepName2.Data()) );     // copies maps from 1. one
        PIDHistoAdder _wPIDCalibH1( _wPIDCalibL1,  TString::Format( "wPIDCalib%s",_had1Name.Data()) );
        PIDHistoAdder _wPIDCalibH2( _wPIDCalibL1,  TString::Format( "wPIDCalib%s",_had2Name.Data()) );

        lastNode = lastNode.Define( _wPIDCalibL1.branchName(), _wPIDCalibL1 , _Inputs.at("L1"))
                           .Define( _wPIDCalibL2.branchName(), _wPIDCalibL2 , _Inputs.at("L2"))
                           .Define( _wPIDCalibH1.branchName(), _wPIDCalibH1 , _Inputs.at("H1"));
        
        if (_configHolder.GetNBodies() > 3) {
            TString _combinedWPIDCalib = TString::Format("%s * %s * %s * %s", _wPIDCalibL1.branchName(), _wPIDCalibL2.branchName(), _wPIDCalibH1.branchName(), _wPIDCalibH2.branchName());
            lastNode = lastNode.Define( _wPIDCalibH2.branchName(), _wPIDCalibH1 , _Inputs.at("H2"))
                               .Define( "wPIDCalib",               _combinedWPIDCalib.Data() );
        } else if (_configHolder.GetNBodies() == 3) {
            TString _combinedWPIDCalib = Form("%s * %s * %s",      _wPIDCalibL1.branchName(), _wPIDCalibL2.branchName(), _wPIDCalibH1.branchName());
            lastNode = lastNode.Define( "wPIDCalib",               _combinedWPIDCalib.Data()  );    
        }
    }

    if (_usePID && _useWeight && !_useInterp) {
        //=============================================================
        // Actual HistoAdders PIDCalib Weight method electrons
        //=============================================================    
        PIDHistoAdder _wPIDCalibL1_Weight( _weightHolder,        TString::Format( "wPIDCalib%s_Weight",_lepName1.Data()), "WeightMapPID" ); // loads all maps
        PIDHistoAdder _wPIDCalibL2_Weight( _wPIDCalibL1_Weight,  TString::Format( "wPIDCalib%s_Weight",_lepName2.Data()) );     // copies maps from 1. one
        PIDHistoAdder _wPIDCalibH1_Weight( _wPIDCalibL1_Weight,  TString::Format( "wPIDCalib%s_Weight",_had1Name.Data()) );
        PIDHistoAdder _wPIDCalibH2_Weight( _wPIDCalibL1_Weight,  TString::Format( "wPIDCalib%s_weight",_had2Name.Data()) );

        lastNode = lastNode.Define( _wPIDCalibL1_Weight.branchName(), _wPIDCalibL1_Weight , _Inputs.at("L1"))
                           .Define( _wPIDCalibL2_Weight.branchName(), _wPIDCalibL2_Weight , _Inputs.at("L2"))
                           .Define( _wPIDCalibH1_Weight.branchName(), _wPIDCalibH1_Weight , _Inputs.at("H1"));
        
        if (_configHolder.GetNBodies() > 3) {
            TString _combinedWPIDCalibWeight = TString::Format("%s * %s * %s * %s", _wPIDCalibL1_Weight.branchName(), _wPIDCalibL2_Weight.branchName(), _wPIDCalibH1_Weight.branchName(), _wPIDCalibH2_Weight.branchName());
            lastNode = lastNode.Define( _wPIDCalibH2_Weight.branchName(), _wPIDCalibH1_Weight , _Inputs.at("H2"))
                               .Define( "wPIDCalib_Weight",               _combinedWPIDCalibWeight.Data() );
        } else if (_configHolder.GetNBodies() == 3) {
            TString _combinedWPIDCalibWeight = Form("%s * %s * %s",      _wPIDCalibL1_Weight.branchName(), _wPIDCalibL2_Weight.branchName(), _wPIDCalibH1_Weight.branchName());
            lastNode = lastNode.Define( "wPIDCalib_Weight",               _combinedWPIDCalibWeight.Data()  );    
        }
    }
    if (_usePID && !_useWeight && _useInterp) {
        //=============================================================
        // Actual HistoAdders PIDCalib (manual binning)
        //=============================================================    
        PIDHistoAdder _wiPIDCalibL1( _weightHolder, TString::Format(  "wiPIDCalib%s",_lepName1.Data()), "interp" ); // loads all maps
        PIDHistoAdder _wiPIDCalibL2( _wiPIDCalibL1,  TString::Format( "wiPIDCalib%s",_lepName2.Data()) );     // copies maps from 1. one
        PIDHistoAdder _wiPIDCalibH1( _wiPIDCalibL1,  TString::Format( "wiPIDCalib%s",_had1Name.Data()) );
        PIDHistoAdder _wiPIDCalibH2( _wiPIDCalibL1,  TString::Format( "wiPIDCalib%s",_had2Name.Data()) );

        lastNode = lastNode.Define( _wiPIDCalibL1.branchName(), _wiPIDCalibL1 , _Inputs.at("L1"))
                           .Define( _wiPIDCalibL2.branchName(), _wiPIDCalibL2 , _Inputs.at("L2"))
                           .Define( _wiPIDCalibH1.branchName(), _wiPIDCalibH1 , _Inputs.at("H1")); 
        
        if (_configHolder.GetNBodies() > 3) {
            TString _combinedWiPIDCalib = Form("%s * %s * %s * %s", _wiPIDCalibL1.branchName(), _wiPIDCalibL2.branchName(), _wiPIDCalibH1.branchName(), _wiPIDCalibH2.branchName());
            lastNode = lastNode.Define( _wiPIDCalibH2.branchName(), _wiPIDCalibH2 , _Inputs.at("H2"))
                               .Define( "wiPIDCalib",              _combinedWiPIDCalib.Data() );
        } else if (_configHolder.GetNBodies() == 3) {
            TString _combinedWiPIDCalib = Form("%s * %s * %s",      _wiPIDCalibL1.branchName(), _wiPIDCalibL2.branchName(), _wiPIDCalibH1.branchName());
            lastNode = lastNode.Define( "wiPIDCalib",              _combinedWiPIDCalib.Data()  );    
        }
    }

    if (_usePID && _useInterp && _useWeight) {
        //=============================================================
        // Actual HistoAdders PIDCalib Weight method electrons
        //=============================================================    
        PIDHistoAdder _wiPIDCalibL1_Weight( _weightHolder,         TString::Format( "wiPIDCalib%s_Weight",_lepName1.Data()), "interp_WeightMapPID" ); // loads all maps
        PIDHistoAdder _wiPIDCalibL2_Weight( _wiPIDCalibL1_Weight,  TString::Format( "wiPIDCalib%s_Weight",_lepName2.Data()) );     // copies maps from 1. one
        PIDHistoAdder _wiPIDCalibH1_Weight( _wiPIDCalibL1_Weight,  TString::Format( "wiPIDCalib%s_Weight",_had1Name.Data()) );
        PIDHistoAdder _wiPIDCalibH2_Weight( _wiPIDCalibL1_Weight,  TString::Format( "wiPIDCalib%s_Weight",_had2Name.Data()) );

        lastNode = lastNode.Define( _wiPIDCalibL1_Weight.branchName(), _wiPIDCalibL1_Weight , _Inputs.at("L1"))
                           .Define( _wiPIDCalibL2_Weight.branchName(), _wiPIDCalibL2_Weight , _Inputs.at("L2"))
                           .Define( _wiPIDCalibH1_Weight.branchName(), _wiPIDCalibH1_Weight , _Inputs.at("H1")); 
        
        if (_configHolder.GetNBodies() > 3) {
            TString _combinedWiPIDCalibWeight = Form("%s * %s * %s * %s", _wiPIDCalibL1_Weight.branchName(), _wiPIDCalibL2_Weight.branchName(), _wiPIDCalibH1_Weight.branchName(), _wiPIDCalibH2_Weight.branchName());
            lastNode = lastNode.Define( _wiPIDCalibH2_Weight.branchName(), _wiPIDCalibH2_Weight , _Inputs.at("H2"))
                               .Define( "wiPIDCalib_Weight",              _combinedWiPIDCalibWeight.Data() );
        } else if (_configHolder.GetNBodies() == 3) {
            TString _combinedWiPIDCalibWeight = Form("%s * %s * %s",      _wiPIDCalibL1_Weight.branchName(), _wiPIDCalibL2_Weight.branchName(), _wiPIDCalibH1_Weight.branchName());
            lastNode = lastNode.Define( "wiPIDCalib_Weight",              _combinedWiPIDCalibWeight.Data()  );    
        }
    }

    if (_useKDE) {
        //=============================================================
        // Actual HistoAdders KDE
        //=============================================================    
        PIDHistoAdder _wPIDCalibKDEL1( _weightHolder,   TString::Format("wPIDCalibKDE%s", _lepName1.Data() ), "KDE" ); // loads all maps
        PIDHistoAdder _wPIDCalibKDEL2( _wPIDCalibKDEL1, TString::Format("wPIDCalibKDE%s", _lepName2.Data()) );        // copies maps from 1. one
        PIDHistoAdder _wPIDCalibKDEH1( _wPIDCalibKDEL1, TString::Format("wPIDCalibKDE%s", _had1Name.Data()) );
        PIDHistoAdder _wPIDCalibKDEH2( _wPIDCalibKDEL1, TString::Format("wPIDCalibKDE%s", _had2Name.Data()) );	
        lastNode = lastNode.Define( _wPIDCalibKDEL1.branchName(), _wPIDCalibKDEL1 , _Inputs.at("L1"))
                           .Define( _wPIDCalibKDEL2.branchName(), _wPIDCalibKDEL2 , _Inputs.at("L2"))
                           .Define( _wPIDCalibKDEH1.branchName(), _wPIDCalibKDEH1 , _Inputs.at("H1")); 
        if (_configHolder.GetNBodies() > 3) {
            TString _combinedWPIDCalibKDE = Form("%s * %s * %s * %s", _wPIDCalibKDEL1.branchName(), _wPIDCalibKDEL2.branchName(), _wPIDCalibKDEH1.branchName(), _wPIDCalibKDEH2.branchName());
            lastNode = lastNode.Define( _wPIDCalibKDEH2.branchName(), _wPIDCalibKDEH2 ,  _Inputs.at("H2"))
                               .Define( "wPIDCalibKDE",               _combinedWPIDCalibKDE.Data() );
        } else if (_configHolder.GetNBodies() == 3) {
            TString _combinedWPIDCalibKDE = Form("%s * %s * %s", _wPIDCalibKDEL1.branchName(), _wPIDCalibKDEL2.branchName(), _wPIDCalibKDEH1.branchName());
            lastNode = lastNode.Define( "wPIDCalibKDE",               _combinedWPIDCalibKDE.Data()  );    
        }
    }
    if (_useKDE && _useAB){
        //=============================================================
        // Actual HistoAdders KDE alternative electron binning
        //=============================================================    
        PIDHistoAdder _wPIDCalibKDEL1_AB( _weightHolder     , TString::Format("wPIDCalibKDE%s_AB", _lepName1.Data()), "KDE_NoOpt" ); // loads all maps
        PIDHistoAdder _wPIDCalibKDEL2_AB( _wPIDCalibKDEL1_AB, TString::Format("wPIDCalibKDE%s_AB", _lepName2.Data()) );        // copies maps from 1. one
        PIDHistoAdder _wPIDCalibKDEH1_AB( _wPIDCalibKDEL1_AB, TString::Format("wPIDCalibKDE%s_AB", _had1Name.Data()) );
        PIDHistoAdder _wPIDCalibKDEH2_AB( _wPIDCalibKDEL1_AB, TString::Format("wPIDCalibKDE%s_AB", _had2Name.Data()) );	                
        lastNode = lastNode.Define( _wPIDCalibKDEL1_AB.branchName(), _wPIDCalibKDEL1_AB , _Inputs.at("L1"))
                           .Define( _wPIDCalibKDEL2_AB.branchName(), _wPIDCalibKDEL2_AB , _Inputs.at("L2"))
                           .Define( _wPIDCalibKDEH1_AB.branchName(), _wPIDCalibKDEH1_AB , _Inputs.at("H1")); 
        
        if (_configHolder.GetNBodies() > 3) {
            TString _combinedWPIDCalibKDEAB = Form("%s * %s * %s * %s",  _wPIDCalibKDEL1_AB.branchName(), _wPIDCalibKDEL2_AB.branchName(), _wPIDCalibKDEH1_AB.branchName(), _wPIDCalibKDEH2_AB.branchName());
            lastNode = lastNode.Define( _wPIDCalibKDEH2_AB.branchName(), _wPIDCalibKDEH2_AB ,_Inputs.at("H2"))
                               .Define( "wPIDCalibKDE_AB",               _combinedWPIDCalibKDEAB.Data() );
        } else if (_configHolder.GetNBodies() == 3) {
            TString _combinedWPIDCalibKDEAB = Form("%s * %s * %s",       _wPIDCalibKDEL1_AB.branchName(), _wPIDCalibKDEL2_AB.branchName(), _wPIDCalibKDEH1_AB.branchName());
            lastNode = lastNode.Define( "wPIDCalibKDE_AB",               _combinedWPIDCalibKDEAB.Data() );
        }
    }

    if (_useKDE && _useAK){
        //=============================================================
        // Actual HistoAdders KDE altkernel
        //=============================================================    
        PIDHistoAdder _wPIDCalibKDEL1_AK( _weightHolder     , TString::Format("wPIDCalibKDE%s_AK", _lepName1.Data()), "KDE_ALTKERNEL" ); // loads all maps
        PIDHistoAdder _wPIDCalibKDEL2_AK( _wPIDCalibKDEL1_AK, TString::Format("wPIDCalibKDE%s_AK", _lepName2.Data() ));        // copies maps from 1. one
        PIDHistoAdder _wPIDCalibKDEH1_AK( _wPIDCalibKDEL1_AK, TString::Format("wPIDCalibKDE%s_AK", _had1Name.Data() ));
        PIDHistoAdder _wPIDCalibKDEH2_AK( _wPIDCalibKDEL1_AK, TString::Format("wPIDCalibKDE%s_AK", _had2Name.Data() ));	    
        lastNode = lastNode.Define( _wPIDCalibKDEL1_AK.branchName(), _wPIDCalibKDEL1_AK ,_Inputs.at("L1"))
                           .Define( _wPIDCalibKDEL2_AK.branchName(), _wPIDCalibKDEL2_AK ,_Inputs.at("L2"))
                           .Define( _wPIDCalibKDEH1_AK.branchName(), _wPIDCalibKDEH1_AK ,_Inputs.at("H1"));
        if (_configHolder.GetNBodies() > 3) {
            TString _combinedWPIDCalibKDEAK = Form("%s * %s * %s * %s",  _wPIDCalibKDEL1_AK.branchName(), _wPIDCalibKDEL2_AK.branchName(), _wPIDCalibKDEH1_AK.branchName(), _wPIDCalibKDEH2_AK.branchName());
            lastNode = lastNode.Define( _wPIDCalibKDEH2_AK.branchName(), _wPIDCalibKDEH2_AK ,  _Inputs.at("H2"))
                               .Define( "wPIDCalibKDE_AK",               _combinedWPIDCalibKDEAK.Data() );
        } else if (_configHolder.GetNBodies() == 3) {
            TString _combinedWPIDCalibKDEAK = Form("%s * %s * %s",       _wPIDCalibKDEL1_AK.branchName(), _wPIDCalibKDEL2_AK.branchName(), _wPIDCalibKDEH1_AK.branchName());
            lastNode = lastNode.Define( "wPIDCalibKDE_AK",               _combinedWPIDCalibKDEAK.Data() );
        }
    }

    if (_useKDE && _useWeight){
        //=============================================================
        // Actual HistoAdders KDE Weight method electrons
        //=============================================================    
        PIDHistoAdder _wPIDCalibKDEL1_Weight( _weightHolder     , TString::Format("wPIDCalibKDE%s_Weight", _lepName1.Data()), "KDE_WeightMapPID" ); // loads all maps
        PIDHistoAdder _wPIDCalibKDEL2_Weight( _wPIDCalibKDEL1_Weight, TString::Format("wPIDCalibKDE%s_Weight", _lepName2.Data() ));                 // copies maps from 1. one
        PIDHistoAdder _wPIDCalibKDEH1_Weight( _wPIDCalibKDEL1_Weight, TString::Format("wPIDCalibKDE%s_Weight", _had1Name.Data() ));
        PIDHistoAdder _wPIDCalibKDEH2_Weight( _wPIDCalibKDEL1_Weight, TString::Format("wPIDCalibKDE%s_Weight", _had2Name.Data() ));
        lastNode = lastNode.Define( _wPIDCalibKDEL1_Weight.branchName(), _wPIDCalibKDEL1_Weight ,_Inputs.at("L1"))
                           .Define( _wPIDCalibKDEL2_Weight.branchName(), _wPIDCalibKDEL2_Weight ,_Inputs.at("L2"))
                           .Define( _wPIDCalibKDEH1_Weight.branchName(), _wPIDCalibKDEH1_Weight ,_Inputs.at("H1"));
        if (_configHolder.GetNBodies() > 3) {
            TString _combinedWPIDCalibKDEWeight = Form("%s * %s * %s * %s",  _wPIDCalibKDEL1_Weight.branchName(), _wPIDCalibKDEL2_Weight.branchName(), _wPIDCalibKDEH1_Weight.branchName(), _wPIDCalibKDEH2_Weight.branchName());
            lastNode = lastNode.Define( _wPIDCalibKDEH2_Weight.branchName(), _wPIDCalibKDEH2_Weight ,  _Inputs.at("H2"))
                               .Define( "wPIDCalibKDE_Weight",               _combinedWPIDCalibKDEWeight.Data() );
        } else if (_configHolder.GetNBodies() == 3) {
            TString _combinedWPIDCalibKDEWeight = Form("%s * %s * %s",       _wPIDCalibKDEL1_Weight.branchName(), _wPIDCalibKDEL2_Weight.branchName(), _wPIDCalibKDEH1_Weight.branchName());
            lastNode = lastNode.Define( "wPIDCalibKDE_Weight",               _combinedWPIDCalibKDEWeight.Data() );
        }
    }

    if (_useKDE && _useAN){
        //=============================================================
        // Actual HistoAdders KDE alt nTracks
        //=============================================================    
        PIDHistoAdder _wPIDCalibKDEL1_AN( _weightHolder     , TString::Format("wPIDCalibKDE%s_AN", _lepName1.Data()), "KDE_ALTNTRACKS" ); // loads all maps
        PIDHistoAdder _wPIDCalibKDEL2_AN( _wPIDCalibKDEL1_AN, TString::Format("wPIDCalibKDE%s_AN", _lepName2.Data() ));
        PIDHistoAdder _wPIDCalibKDEH1_AN( _wPIDCalibKDEL1_AN, TString::Format("wPIDCalibKDE%s_AN", _had1Name.Data() ));
        PIDHistoAdder _wPIDCalibKDEH2_AN( _wPIDCalibKDEL1_AN, TString::Format("wPIDCalibKDE%s_AN", _had2Name.Data() ));
        
        lastNode = lastNode.Define( _wPIDCalibKDEL1_AN.branchName(), _wPIDCalibKDEL1_AN , _Inputs.at("L1"))
                           .Define( _wPIDCalibKDEL2_AN.branchName(), _wPIDCalibKDEL2_AN , _Inputs.at("L2"))
                           .Define( _wPIDCalibKDEH1_AN.branchName(), _wPIDCalibKDEH1_AN , _Inputs.at("H1"));
        if (_configHolder.GetNBodies() > 3) {
            TString _combinedWPIDCalibKDEAN = Form("%s * %s * %s * %s",  _wPIDCalibKDEL1_AN.branchName(), _wPIDCalibKDEL2_AN.branchName(), _wPIDCalibKDEH1_AN.branchName(), _wPIDCalibKDEH2_AN.branchName());
            lastNode = lastNode.Define( _wPIDCalibKDEH2_AN.branchName(), _wPIDCalibKDEH2_AN , _Inputs.at("H2"))
                               .Define( "wPIDCalibKDE_AN",               _combinedWPIDCalibKDEAN.Data() );
        } else if (_configHolder.GetNBodies() == 3) {
            TString _combinedWPIDCalibKDEAN = Form("%s * %s * %s",       _wPIDCalibKDEL1_AN.branchName(), _wPIDCalibKDEL2_AN.branchName(), _wPIDCalibKDEH1_AN.branchName());
            lastNode = lastNode.Define( "wPIDCalibKDE_AN",               _combinedWPIDCalibKDEAN.Data() );
        }
    }

    //=============================================================
    // BS Maps [KDE]
    //=============================================================
    if( _useKDE && _useBS ){
        MessageSvc::Info(Color::Cyan, "AddPIDWeights", "Adding BS branches (KDE setup)");
        PIDHistoAdderBSKDE _wPIDCalibKDEL1_BS( _weightHolder     , TString::Format("wPIDCalibKDE%s_BS", _lepName1.Data()) , "KDE", WeightDefRX::nBS );
        PIDHistoAdderBSKDE _wPIDCalibKDEL2_BS( _wPIDCalibKDEL1_BS, TString::Format("wPIDCalibKDE%s_BS", _lepName2.Data() ));
        PIDHistoAdderBSKDE _wPIDCalibKDEH1_BS( _wPIDCalibKDEL1_BS, TString::Format("wPIDCalibKDE%s_BS", _had1Name.Data() ));
        PIDHistoAdderBSKDE _wPIDCalibKDEH2_BS( _wPIDCalibKDEL1_BS, TString::Format("wPIDCalibKDE%s_BS", _had2Name.Data() ));
        if (_configHolder.GetNBodies() > 3) {
            TString _combinedWPIDCalibKDEBS = Form("%s * %s * %s * %s",  _wPIDCalibKDEL1_BS.branchName(), _wPIDCalibKDEL2_BS.branchName(), _wPIDCalibKDEH1_BS.branchName(), _wPIDCalibKDEH2_BS.branchName());
            lastNode = lastNode.Define(_wPIDCalibKDEL1_BS.branchName(),  _wPIDCalibKDEL1_BS, _Inputs.at("L1"))
                               .Define(_wPIDCalibKDEL2_BS.branchName(), _wPIDCalibKDEL2_BS, _Inputs.at("L2"))
                               .Define(_wPIDCalibKDEH1_BS.branchName(), _wPIDCalibKDEH1_BS, _Inputs.at("H1"))
                               .Define(_wPIDCalibKDEH2_BS.branchName(), _wPIDCalibKDEH2_BS, _Inputs.at("H2"))
                               .Define( "wPIDCalibKDE_BS",              _combinedWPIDCalibKDEBS.Data() );
        } else if (_configHolder.GetNBodies() == 3) {
            //RK case 3 body decay
            TString _combinedWPIDCalibKDEBS = Form("%s * %s * %s",  _wPIDCalibKDEL1_BS.branchName(), _wPIDCalibKDEL2_BS.branchName(), _wPIDCalibKDEH1_BS.branchName());
            lastNode = lastNode.Define(_wPIDCalibKDEL1_BS.branchName(), _wPIDCalibKDEL1_BS, _Inputs.at("L1")) 
                               .Define(_wPIDCalibKDEL2_BS.branchName(), _wPIDCalibKDEL2_BS, _Inputs.at("L2"))  
                               .Define(_wPIDCalibKDEH1_BS.branchName(), _wPIDCalibKDEH1_BS, _Inputs.at("H1"))
                               .Define( "wPIDCalibKDE_BS",              _combinedWPIDCalibKDEBS.Data() );
        }
    }

    //=============================================================
    // BS Maps [KDE] with Weight Method Electrons
    //=============================================================
    if( _useKDE && _useBS && _useWeight ){
        MessageSvc::Info(Color::Cyan, "AddPIDWeights", "Adding BS branches (KDE + weight setup)");
        PIDHistoAdderBSKDE _wPIDCalibKDEL1_WeightBS( _weightHolder           , TString::Format("wPIDCalibKDE%s_WeightBS", _lepName1.Data()) , "KDE_WeightMapPID", WeightDefRX::nBS );
        PIDHistoAdderBSKDE _wPIDCalibKDEL2_WeightBS( _wPIDCalibKDEL1_WeightBS, TString::Format("wPIDCalibKDE%s_WeightBS", _lepName2.Data() ));
        PIDHistoAdderBSKDE _wPIDCalibKDEH1_WeightBS( _wPIDCalibKDEL1_WeightBS, TString::Format("wPIDCalibKDE%s_WeightBS", _had1Name.Data() ));
        PIDHistoAdderBSKDE _wPIDCalibKDEH2_WeightBS( _wPIDCalibKDEL1_WeightBS, TString::Format("wPIDCalibKDE%s_WeightBS", _had2Name.Data() ));
        if (_configHolder.GetNBodies() > 3) {
            TString _combinedWPIDCalibKDEWeightBS = Form("%s * %s * %s * %s", _wPIDCalibKDEL1_WeightBS.branchName(), _wPIDCalibKDEL2_WeightBS.branchName(), _wPIDCalibKDEH1_WeightBS.branchName(), _wPIDCalibKDEH2_WeightBS.branchName());
            lastNode = lastNode.Define(_wPIDCalibKDEL1_WeightBS.branchName(), _wPIDCalibKDEL1_WeightBS, _Inputs.at("L1"))
                               .Define(_wPIDCalibKDEL2_WeightBS.branchName(), _wPIDCalibKDEL2_WeightBS, _Inputs.at("L2"))
                               .Define(_wPIDCalibKDEH1_WeightBS.branchName(), _wPIDCalibKDEH1_WeightBS, _Inputs.at("H1"))
                               .Define(_wPIDCalibKDEH2_WeightBS.branchName(), _wPIDCalibKDEH2_WeightBS, _Inputs.at("H2"))
                               .Define( "wPIDCalibKDE_WeightBS",              _combinedWPIDCalibKDEWeightBS.Data() );
        } else if (_configHolder.GetNBodies() == 3) {
            //RK case 3 body decay
            TString _combinedWPIDCalibKDEWeightBS = Form("%s * %s * %s",      _wPIDCalibKDEL1_WeightBS.branchName(), _wPIDCalibKDEL2_WeightBS.branchName(), _wPIDCalibKDEH1_WeightBS.branchName());
            lastNode = lastNode.Define(_wPIDCalibKDEL1_WeightBS.branchName(), _wPIDCalibKDEL1_WeightBS, _Inputs.at("L1")) 
                               .Define(_wPIDCalibKDEL2_WeightBS.branchName(), _wPIDCalibKDEL2_WeightBS, _Inputs.at("L2"))  
                               .Define(_wPIDCalibKDEH1_WeightBS.branchName(), _wPIDCalibKDEH1_WeightBS, _Inputs.at("H1"))
                               .Define( "wPIDCalibKDE_WeightBS",              _combinedWPIDCalibKDEWeightBS.Data() );
        }
    }

    //=============================================================
    // BS Maps non interpolated
    //=============================================================    
    if( _usePID && _useBS && !_useInterp ){
        MessageSvc::Info(Color::Cyan, "AddPIDWeights", "Adding BS branches (nominal maps setup)");
        PIDHistoAdderBS_SMEAR _wPIDCalibL1_BS( _weightHolder        , TString::Format("wPIDCalib%s_BS", _lepName1.Data()) , "", WeightDefRX::nBS );
        PIDHistoAdderBS_SMEAR _wPIDCalibL2_BS( _wPIDCalibL1_BS, TString::Format("wPIDCalib%s_BS", _lepName2.Data() ));
        PIDHistoAdderBS_SMEAR _wPIDCalibH1_BS( _wPIDCalibL1_BS, TString::Format("wPIDCalib%s_BS", _had1Name.Data() ));
        PIDHistoAdderBS_SMEAR _wPIDCalibH2_BS( _wPIDCalibL1_BS, TString::Format("wPIDCalib%s_BS", _had2Name.Data() ));
        if (_configHolder.GetNBodies() > 3) {
            TString _combinedWPIDCalibBS = Form("%s * %s * %s * %s",  _wPIDCalibL1_BS.branchName(), _wPIDCalibL2_BS.branchName(), _wPIDCalibH1_BS.branchName(), _wPIDCalibH2_BS.branchName());
            lastNode = lastNode.Define(_wPIDCalibL1_BS.branchName(), _wPIDCalibL1_BS, _Inputs.at("L1"))
                               .Define(_wPIDCalibL2_BS.branchName(), _wPIDCalibL2_BS, _Inputs.at("L2"))
                               .Define(_wPIDCalibH1_BS.branchName(), _wPIDCalibH1_BS, _Inputs.at("H1"))
                               .Define(_wPIDCalibH2_BS.branchName(), _wPIDCalibH2_BS, _Inputs.at("H2"))
                               .Define( "wPIDCalib_BS",              _combinedWPIDCalibBS.Data() );
        } else if (_configHolder.GetNBodies() == 3) {
            //RK case 3 body decay
            TString _combinedWPIDCalibBS = Form("%s * %s * %s",  _wPIDCalibL1_BS.branchName(), _wPIDCalibL2_BS.branchName(), _wPIDCalibH1_BS.branchName());
            lastNode = lastNode.Define(_wPIDCalibL1_BS.branchName(), _wPIDCalibL1_BS, _Inputs.at("L1")) 
                               .Define(_wPIDCalibL2_BS.branchName(), _wPIDCalibL2_BS, _Inputs.at("L2"))  
                               .Define(_wPIDCalibH1_BS.branchName(), _wPIDCalibH1_BS, _Inputs.at("H1"))
                               .Define( "wPIDCalib_BS",              _combinedWPIDCalibBS.Data() );
        }
    }

    //=============================================================
    // BS Maps non interpolated with Weight Method Electrons
    //=============================================================    
    if( _usePID && _useBS && _useWeight && !_useInterp ){
        MessageSvc::Info(Color::Cyan, "AddPIDWeights", "Adding BS branches (nominal maps + weight method electrons setup)");
        PIDHistoAdderBS_SMEAR _wPIDCalibL1_WeightBS( _weightHolder        , TString::Format("wPIDCalib%s_Weight_BS", _lepName1.Data()) , "WeightMapPID", WeightDefRX::nBS );
        PIDHistoAdderBS_SMEAR _wPIDCalibL2_WeightBS( _wPIDCalibL1_WeightBS, TString::Format("wPIDCalib%s_Weight_BS", _lepName2.Data() ));
        PIDHistoAdderBS_SMEAR _wPIDCalibH1_WeightBS( _wPIDCalibL1_WeightBS, TString::Format("wPIDCalib%s_Weight_BS", _had1Name.Data() ));
        PIDHistoAdderBS_SMEAR _wPIDCalibH2_WeightBS( _wPIDCalibL1_WeightBS, TString::Format("wPIDCalib%s_Weight_BS", _had2Name.Data() ));
        if (_configHolder.GetNBodies() > 3) {
            TString _combinedWPIDCalibWeightBS = Form("%s * %s * %s * %s",  _wPIDCalibL1_WeightBS.branchName(), _wPIDCalibL2_WeightBS.branchName(), _wPIDCalibH1_WeightBS.branchName(), _wPIDCalibH2_WeightBS.branchName());
            lastNode = lastNode.Define(_wPIDCalibL1_WeightBS.branchName(), _wPIDCalibL1_WeightBS, _Inputs.at("L1"))
                               .Define(_wPIDCalibL2_WeightBS.branchName(), _wPIDCalibL2_WeightBS, _Inputs.at("L2"))
                               .Define(_wPIDCalibH1_WeightBS.branchName(), _wPIDCalibH1_WeightBS, _Inputs.at("H1"))
                               .Define(_wPIDCalibH2_WeightBS.branchName(), _wPIDCalibH2_WeightBS, _Inputs.at("H2"))
                               .Define( "wPIDCalib_Weight_BS",              _combinedWPIDCalibWeightBS.Data() );
        } else if (_configHolder.GetNBodies() == 3) {
            //RK case 3 body decay
            TString _combinedWPIDCalibWeightBS = Form("%s * %s * %s",  _wPIDCalibL1_WeightBS.branchName(), _wPIDCalibL2_WeightBS.branchName(), _wPIDCalibH1_WeightBS.branchName());
            lastNode = lastNode.Define(_wPIDCalibL1_WeightBS.branchName(), _wPIDCalibL1_WeightBS, _Inputs.at("L1")) 
                               .Define(_wPIDCalibL2_WeightBS.branchName(), _wPIDCalibL2_WeightBS, _Inputs.at("L2"))  
                               .Define(_wPIDCalibH1_WeightBS.branchName(), _wPIDCalibH1_WeightBS, _Inputs.at("H1"))
                               .Define( "wPIDCalib_Weight_BS",              _combinedWPIDCalibWeightBS.Data() );
        }
    }

    //=============================================================
    // BS Maps interpolated
    //=============================================================      
    if( _usePID && _useBS && _useInterp && !_useWeight ){
        MessageSvc::Info(Color::Cyan, "AddPIDWeights", "Adding BS branches (nominal maps interpolated setup)");
        PIDHistoAdderBS_SMEAR _wiPIDCalibL1_BS( _weightHolder  ,  TString::Format("wiPIDCalib%s_BS", _lepName1.Data()) , "interp", WeightDefRX::nBS );
        PIDHistoAdderBS_SMEAR _wiPIDCalibL2_BS( _wiPIDCalibL1_BS, TString::Format("wiPIDCalib%s_BS", _lepName2.Data() ));
        PIDHistoAdderBS_SMEAR _wiPIDCalibH1_BS( _wiPIDCalibL1_BS, TString::Format("wiPIDCalib%s_BS", _had1Name.Data() ));
        PIDHistoAdderBS_SMEAR _wiPIDCalibH2_BS( _wiPIDCalibL1_BS, TString::Format("wiPIDCalib%s_BS", _had2Name.Data() ));
        if (_configHolder.GetNBodies() > 3) {
            TString _combinedWPIDCalibBS = Form("%s * %s * %s * %s",  _wiPIDCalibL1_BS.branchName(), _wiPIDCalibL2_BS.branchName(), _wiPIDCalibH1_BS.branchName(), _wiPIDCalibH2_BS.branchName());
            lastNode = lastNode.Define(_wiPIDCalibL1_BS.branchName(), _wiPIDCalibL1_BS, _Inputs.at("L1"))
                               .Define(_wiPIDCalibL2_BS.branchName(), _wiPIDCalibL2_BS, _Inputs.at("L2"))
                               .Define(_wiPIDCalibH1_BS.branchName(), _wiPIDCalibH1_BS, _Inputs.at("H1"))
                               .Define(_wiPIDCalibH2_BS.branchName(), _wiPIDCalibH2_BS, _Inputs.at("H2"))
                               .Define( "wiPIDCalib_BS",              _combinedWPIDCalibBS.Data() );
        } else if (_configHolder.GetNBodies() == 3) {
            //RK case 3 body decay
            TString _combinedWPIDCalibBS = Form("%s * %s * %s",  _wiPIDCalibL1_BS.branchName(), _wiPIDCalibL2_BS.branchName(), _wiPIDCalibH1_BS.branchName());
            lastNode = lastNode.Define(_wiPIDCalibL1_BS.branchName(), _wiPIDCalibL1_BS, _Inputs.at("L1")) 
                               .Define(_wiPIDCalibL2_BS.branchName(), _wiPIDCalibL2_BS, _Inputs.at("L2"))  
                               .Define(_wiPIDCalibH1_BS.branchName(), _wiPIDCalibH1_BS, _Inputs.at("H1"))
                               .Define( "wiPIDCalib_BS",              _combinedWPIDCalibBS.Data() );
        }
    }

    //=============================================================
    // BS Maps interpolated with Weight Method Electrons
    //=============================================================      
    if( _usePID && _useBS && _useInterp  && _useWeight){
        MessageSvc::Info(Color::Cyan, "AddPIDWeights", "Adding BS branches (nominal maps interpolated + weight method electrons setup)");
        PIDHistoAdderBS_SMEAR _wiPIDCalibL1_WeightBS( _weightHolder         , TString::Format("wiPIDCalib%s_Weight_BS", _lepName1.Data()) , "interp_WeightMapPID", WeightDefRX::nBS );
        PIDHistoAdderBS_SMEAR _wiPIDCalibL2_WeightBS( _wiPIDCalibL1_WeightBS, TString::Format("wiPIDCalib%s_Weight_BS", _lepName2.Data() ));
        PIDHistoAdderBS_SMEAR _wiPIDCalibH1_WeightBS( _wiPIDCalibL1_WeightBS, TString::Format("wiPIDCalib%s_Weight_BS", _had1Name.Data() ));
        PIDHistoAdderBS_SMEAR _wiPIDCalibH2_WeightBS( _wiPIDCalibL1_WeightBS, TString::Format("wiPIDCalib%s_Weight_BS", _had2Name.Data() ));
        if (_configHolder.GetNBodies() > 3) {
            TString _combinedWPIDCalibWeightBS = Form("%s * %s * %s * %s",  _wiPIDCalibL1_WeightBS.branchName(), 
                                                                            _wiPIDCalibL2_WeightBS.branchName(), 
                                                                            _wiPIDCalibH1_WeightBS.branchName(), 
                                                                            _wiPIDCalibH2_WeightBS.branchName());
            lastNode = lastNode.Define(_wiPIDCalibL1_WeightBS.branchName(), _wiPIDCalibL1_WeightBS, _Inputs.at("L1"))
                               .Define(_wiPIDCalibL2_WeightBS.branchName(), _wiPIDCalibL2_WeightBS, _Inputs.at("L2"))
                               .Define(_wiPIDCalibH1_WeightBS.branchName(), _wiPIDCalibH1_WeightBS, _Inputs.at("H1"))
                               .Define(_wiPIDCalibH2_WeightBS.branchName(), _wiPIDCalibH2_WeightBS, _Inputs.at("H2"))
                               .Define( "wiPIDCalib_Weight_BS",              _combinedWPIDCalibWeightBS.Data() );
        } else if (_configHolder.GetNBodies() == 3) {
            //RK case 3 body decay
            TString _combinedWPIDCalibWeightBS = Form("%s * %s * %s",  _wiPIDCalibL1_WeightBS.branchName(), _wiPIDCalibL2_WeightBS.branchName(), _wiPIDCalibH1_WeightBS.branchName());
            lastNode = lastNode.Define(_wiPIDCalibL1_WeightBS.branchName(), _wiPIDCalibL1_WeightBS, _Inputs.at("L1")) 
                               .Define(_wiPIDCalibL2_WeightBS.branchName(), _wiPIDCalibL2_WeightBS, _Inputs.at("L2"))  
                               .Define(_wiPIDCalibH1_WeightBS.branchName(), _wiPIDCalibH1_WeightBS, _Inputs.at("H1"))
                               .Define( "wiPIDCalib_Weight_BS",              _combinedWPIDCalibWeightBS.Data() );
        }
    }


    if (_usePID)                                        MessageSvc::Info(Color::Green, "AddPIDWeights", "Adding PIDCalib Weights");
    if (_usePID && _useInterp)                          MessageSvc::Info(Color::Green, "AddPIDWeights", "Adding PIDCalib Weights interpolated");
    if (_usePID && _useBS)                              MessageSvc::Info(Color::Green, "AddPIDWeights", "Adding BS PIDCalib Weights non interpolated");
    if (_usePID && _useBS && _useInterp)                MessageSvc::Info(Color::Green, "AddPIDWeights", "Adding BS PIDCalib Weights interpolated");

    if (_usePID && _useWeight)                          MessageSvc::Info(Color::Green, "AddPIDWeights", "Adding PIDCalib Weights + electron ratio weights setup");
    if (_usePID && _useInterp && _useWeight)            MessageSvc::Info(Color::Green, "AddPIDWeights", "Adding PIDCalib Weights interpolated + electron ratio weights setup");
    if (_usePID && _useBS && _useWeight)                MessageSvc::Info(Color::Green, "AddPIDWeights", "Adding BS PIDCalib Weights non interpolated + electron ratio weights setup");
    if (_usePID && _useBS && _useInterp && _useWeight)  MessageSvc::Info(Color::Green, "AddPIDWeights", "Adding BS PIDCalib Weights interpolated + electron ratio weights setup");

    if (_useKDE)                                        MessageSvc::Info(Color::Green, "AddPIDWeights", "Adding KDE Weights");
    if (_useKDE && _useBS)                              MessageSvc::Info(Color::Green, "AddPIDWeights", "Adding BS KDE Weights");
    if (_useKDE && _useAK)                              MessageSvc::Info(Color::Green, "AddPIDWeights", "Adding alternative Kernel KDE Weights");
    if (_useKDE && _useWeight)                          MessageSvc::Info(Color::Green, "AddPIDWeights", "Adding Weight Map KDE Weights");
    if (_useKDE && _useAB)                              MessageSvc::Info(Color::Green, "AddPIDWeights", "Adding alternative Binning KDE Weights");
    if (_useKDE && _useAN)                              MessageSvc::Info(Color::Green, "AddPIDWeights", "Adding alternative nTracks KDE Weights");
    _wildCards = {"tmpWPIDCalib"};

    // Do not snapshot the verbosity branches
    if (!_PIDVerbose && _useKDE) {
        _wildCards.insert( _wildCards.end(), { 
            TString::Format("wPIDCalibKDE%s", _lepName1.Data()).Data(),
            TString::Format("wPIDCalibKDE%s", _lepName2.Data()).Data(),
            TString::Format("wPIDCalibKDE%s", _had1Name.Data()).Data()
        });       
        if (_configHolder.GetNBodies()>3) {
            _wildCards.push_back( TString::Format("wPIDCalibKDE%s",_had2Name.Data()).Data());
	    }
    }
    if (!_PIDVerbose && _usePID) {
        _wildCards.insert( _wildCards.end(), {
            TString::Format("wPIDCalib%s", _lepName1.Data()).Data(),
            TString::Format("wPIDCalib%s", _lepName2.Data()).Data(),
            TString::Format("wPIDCalib%s", _had1Name.Data()).Data()
        });       
        if (_configHolder.GetNBodies()>3) {
            _wildCards.push_back( TString::Format("wPIDCalib%s",_had2Name.Data()).Data());
	    }
    }
    if (!_PIDVerbose && _usePID && _useInterp) {
        _wildCards.insert(_wildCards.end(), {
            TString::Format("wiPIDCalib%s", _lepName1.Data()).Data(),
            TString::Format("wiPIDCalib%s", _lepName2.Data()).Data(),
            TString::Format("wiPIDCalib%s", _had1Name.Data()).Data()
        });       
        if (_configHolder.GetNBodies()>3) {
            _wildCards.push_back( TString::Format("wiPIDCalib%s",_had2Name.Data()).Data());
	    }
    }
    return lastNode;
}

void HelperProcessing::AddHLTWeights(EventType & _eventType, bool _useET) {

    //=========================
    //Preliminary informations
    //=========================    
    bool _addWHLT   = _eventType.IsMC() && _eventType.GetWeightHolder().IsOption("HLT");
    if( !_addWHLT){ MessageSvc::Info("AddHLTWeights skipped"); return; } //shortcircuit
    if( SettingDef::Weight::useBS){
        MessageSvc::Info("AddHLTWeights, enabled bootstrapping maps adder");
    }    
    auto &_configHolder =  _eventType;  

    //=========================
    //Run up the bookkeping of operatiosn to attach weights.
    //=========================
    ROOT::RDataFrame df( "DecayTuple","TupleProcess.root");
    auto lastNode = AppendHLTColumns( df, _configHolder,_eventType.GetWeightHolder().Option());

    //=====================================
    // Dump tuple to disk
    //=====================================
    MessageSvc::Info("Snapshotting");
    lastNode.Snapshot("DecayTuple", "TupleProcess_tmp.root");
    MessageSvc::Info("Snapshotting done");

    //=====================================
    // Shuffle tuple at exit (entries consistenty)
    //=====================================  
    TFile f1("TupleProcess.root","READ");
    auto  original =  (TTree*)f1.Get("DecayTuple");
    auto  entries = original->GetEntries();
    
    TFile f2("TupleProcess_tmp.root", "READ");
    auto updated = (TTree*)f2.Get("DecayTuple");
    if( original->GetEntries() != updated->GetEntries()) MessageSvc::Error("Severe error on Snapshots", "","EXIT_FAILURE");
    else{ f1.Close(); f2.Close(); }
    
    MessageSvc::Info("rootmv TupleProcess.root:MCDecayTuple TupleProcess_tmp.root");
    moveTupleFromTo( "MCDecayTuple", "TupleProcess.root", "TupleProcess_tmp.root");
    MessageSvc::Info("rm TupleProcess.root");
    IOSvc::runCommand(TString("rm TupleProcess.root"));
    MessageSvc::Info("mv TupleProcess_tmp.root TupleProcess.root");
    IOSvc::runCommand(TString("mv TupleProcess_tmp.root TupleProcess.root"));
    TFile finalFile("TupleProcess.root","READ");
    auto finalTree = (TTree*)finalFile.Get("DecayTuple");
    if( finalTree == nullptr) MessageSvc::Error("Failure HLT weights adder","","EXIT_FAILURE");
    if( finalTree->GetEntries() != entries) MessageSvc::Error("Failure HLT weights adder", "","EXIT_FAILURE");
    MessageSvc::Info("Closing file");
    finalFile.Close();
    return;
}


RNode HelperProcessing::AppendHLTColumns(RNode df,  ConfigHolder & _configHolder, TString _weightOption){
    WeightHolder _weightHolder(_configHolder, _weightOption);
    
    auto lastNode = df.Define( "DUMMYHLT", "1>0");
    // ROOT::DisableImplicitMT();
    //=========================
    //Preliminary informations
    //=========================
    bool _addWHLT_nTracks = _configHolder.IsMC() && _weightHolder.IsOption("nTracks");
    bool _addWHLT_BETA    = _configHolder.IsMC() && _weightHolder.IsOption("BETA");

    bool _ispriorChain    =  SettingDef::Weight::priorChain;

    if( _addWHLT_nTracks ){ 
        MessageSvc::Warning("AddHLTWeights",(TString)"wHLT also for nTracks input variable");
    }

    if( _addWHLT_BETA ){ 
        MessageSvc::Warning("AddHLTWeights",(TString)"wHLT also for B_ETA  input variable");
    }    

    //=========================
    //Functor containers of TH1D for maps read-add HLT (1 container per-input variable to use)
    //=========================       
    vector< TH1DHistoAdder > _hltweights;
    vector< TH1DHistoAdder > _hltweights_nTracks;
    vector< TH1DHistoAdder > _hltweights_ETA;


    vector< BSTH1DHistoAdder > _BShltweights;
    vector< BSTH1DHistoAdder > _BShltweights_nTracks;
    vector< BSTH1DHistoAdder > _BShltweights_ETA;


    //=========================
    //Functor containers of TH2D for maps read-add HLT (1 container per-input variable to use)
    //=========================       
    vector< TH2DHistAdder   > _hltweights2D;
    vector< BSTH2DHistAdder > _BShltweights2D;

    //=========================
    //Preliminaries (name branches) and options
    //========================
    map< TString, TString > _names = _configHolder.GetParticleNames();
    TString _head = _names["HEAD"];
    TString _optionWRatio = "-ratior";
    TString _optionEff    = "-effr";

    //=========================
    //Preliminaries Variants of the maps to read and attach
    //========================    
    map< TString , vector< TString > > HLTVariants{ { "projects" , { "Bp", "B0"}      },  // from Bp ( RK sameples), from B0 ( RKst samples)
                                                    { "type"     , { "effMC","effCL"} }, //, "Ratio"} }, //data eff, MC eff, ratio direct [not default to do]
                                                    { "triggers" , { to_string(Trigger::L0L) + "_incl" , to_string(Trigger::L0I) +"_incl", to_string(Trigger::L0L)+"_excl"} }, // triggers (1 map per trigger)
                                                    { "options"  , { "", "i"} },  //non-interp, interp
                                                    { "vars"     , {"B_PT-1D"} } //variables proxies
    };
    if (_configHolder.GetProject()==Prj::RPhi) HLTVariants["projects"] = {"Bs"}; // set Bs weights for RPhi, cant port HLT
    //If add nTracks based map
    // HLTVariants["vars"] = { "nTracks-1D"}; //do only this! TODO: clean up 
    HLTVariants["triggers"] = { to_string(Trigger::L0L) + "_incl" , to_string(Trigger::L0I) +"_incl" };//only inclusive ones
    if( _addWHLT_nTracks){
      HLTVariants["vars"] = { "nTracks-1D"}; //do only this! TODO: clean up 
      HLTVariants["triggers"] = { to_string(Trigger::L0L) + "_incl" , to_string(Trigger::L0I) +"_incl" };//only inclusive ones
      //HLTVariants["vars"].push_back( "nTracks-1D");
    }
    if( _addWHLT_BETA){
        HLTVariants["vars"].push_back( "B_ETA-1D");
    }

    //If prior also do the 2D ones 
    if(!(_configHolder.GetProject()==Prj::RPhi)){
        //Not RPhi 
        if(_ispriorChain && GetBaseVer(SettingDef::Tuple::gngVer >=10)){
            HLTVariants["vars"] = { "B_PT_nTracks-2D" };
        }
        if(!_ispriorChain && GetBaseVer(SettingDef::Tuple::gngVer >=10) ){                        
            HLTVariants["vars"] = { "nTracks-1D" };
            if(!_addWHLT_nTracks ){
                HLTVariants["vars"] = { "B_PT-1D" };//For systematics! , but also ETA!
            }
            // HLTVariants["options"] = { "i" };
        }
    }
    
    if (!_ispriorChain && _configHolder.GetProject()==Prj::RPhi ) {
        HLTVariants["vars"] = { "B_PT-1D" };	
    }
    //Possibilities to attach --> wHLT, wHLTPT, wHLTnTracks, wHLTPTnTracks
    //An Empty FLAG means that the remapped name of the weight is the default wHLT_CL....
    map< TString, bool   > _addBSMaps = {
        { "B_PT-1D"        ,    !_ispriorChain && SettingDef::Weight::useBS},
        { "nTracks-1D"     ,    !_ispriorChain && SettingDef::Weight::useBS},
        { "B_PT_nTracks-2D",     _ispriorChain && SettingDef::Weight::useBS }//baseline prior        
    };
    map< TString, TString> _branchNameMapper = { 
        { "B_PT-1D"        , "PT"},
        { "nTracks-1D"     , "nTracks"},
        { "B_PT_nTracks-2D", "" }//baseline prior
    };
    if( !_ispriorChain ){
        _branchNameMapper = { 
                { "B_PT-1D"        , ""}, //wHLT 
                { "nTracks-1D"     , "nTracks"}, //wHLT_nTracks
                { "B_ETA-1D"       , "BETA" }//baseline Nominal        
        };
        if (_configHolder.GetProject()==Prj::RPhi) {
            _branchNameMapper = { 
                    { "B_PT-1D"        , ""}, //wHLT for RPhi
            };
        }
            /*_addBSMaps = {
                { "B_PT-1D"        , false },
                { "nTracks-1D"     , false },
                { "B_ETA-1D"       , false }//baseline prior   
            };*/

        if (_configHolder.GetProject()==Prj::RPhi) {
            _addBSMaps = {
            { "B_PT-1D"        , true}, // wHLT for RPhi
            };
        }
    }
    auto cleanOptVar = [&_branchNameMapper] ( const TString & optVar){
        // TString _new = optVar;
        if(_branchNameMapper.find(optVar) == _branchNameMapper.end()){
            MessageSvc::Error("BranchNaming Wrapper invalid for ", optVar, "EXIT_FAILURE");
        }
        return _branchNameMapper.at(optVar);
        // return TString(_new.ReplaceAll("-1D", "").ReplaceAll("-2D",""));
    };
    //TODO : make a special BSHLTVariants for attaching, reduce it to the minimum
    // map< TString , vector< TString > > BSHLTVariants{ { "projects" , { "Bp", "B0"}               },  // from Bp ( RK sameples), from B0 ( RKst samples)
    //                                                   { "type"     , { "effMC","effCL"} }, //data eff, MC eff, ratio direct
    //                                                   { "triggers" , { to_string(Trigger::L0L) + "_incl" , to_string(Trigger::L0I) +"_incl"} }, // triggers (1 map per trigger)
    //                                                   { "options"  , { "i"} },  //non-interp, interp
    //                                                   { "vars"     , {"nTracks"}  } //variables proxies
    //};

    pair<TString, TString> _inputVariable;
    pair<TString, TString> _inputVariableETA = {"NONE","NONE"};
    auto PrintVector =[]( const vector<TString> & vv, const TString type){
        std::cout<< type << " : ( ";
        for (const auto& i: vv){
              std::cout << i << ' ';
        }
        std::cout<<" )"<<std::endl;
        return;
    };
    if( _ispriorChain ){
        HLTVariants["triggers"] = {to_string(Trigger::L0L) + "_incl" };
        if (_weightOption.Contains("fromLOI")) HLTVariants["triggers"] = {to_string(Trigger::L0I) + "_incl" };
        _inputVariable = make_pair("hlt_inputVar1D_priorChain", "{HEAD}_PT" ); //baseline input variable for HLT maps
        _inputVariable.second = _inputVariable.second.ReplaceAll("{HEAD}",_head);
    }else{
        HLTVariants["triggers"] = {to_string(Trigger::L0L) + "_incl"  , 
                                   to_string(Trigger::L0I) + "_incl" };
                                //    to_string(Trigger::L0L) + "_excl"};  
        _inputVariable = make_pair("hlt_inputVar1D_nominalChain", "{HEAD}_PT" ); //baseline input variable for HLT maps
        _inputVariable.second = _inputVariable.second.ReplaceAll("{HEAD}",_head);
        if( _addWHLT_BETA){
            _inputVariableETA = make_pair("hlt_inputVar1D_nominalChainBETA", "{HEAD}_ETA" ); //baseline input variable for HLT maps
            _inputVariableETA.second = _inputVariableETA.second.ReplaceAll("{HEAD}",_head);
        }
    }

    //=========================
    //Cross-product loop for all combinations ( see GetStrMapHLT for dependencies of where maps are grabbed)
    //========================    

    PrintVector(HLTVariants.at("triggers"), "triggers");
    PrintVector(HLTVariants.at("projects"), "projects");
    PrintVector(HLTVariants.at("options"), "options");
    PrintVector(HLTVariants.at("type"), "type");
    PrintVector(HLTVariants.at("vars"), "vars");
    for (const auto && [_project, _trigger, _option , _type, _optionVar] : iter::product(HLTVariants.at("projects"), 
                                                                                         HLTVariants.at("triggers"), 
                                                                                         HLTVariants.at("options"), 
                                                                                         HLTVariants.at("type"),
                                                                                         HLTVariants.at("vars") ) ) { 
        bool is1D = _optionVar.Contains("-1D");
        TString _label = "HLT_" + _project + "_" + _trigger + "_w" + _option + "_"+ _type; 
        pair< TString, TString > _name  =  _weightHolder.GetStrMapHLT( _label+"-"+_optionVar );        
        TString _fileName = _name.first;
        TString _mapKey   = _name.second;


        TString _finalBranchName = _head + "_w" + _option + "HLT"+cleanOptVar(_optionVar)+"_" + _trigger + "_" + _project;
        TString _optionCHECKHISTO  = _optionWRatio;
        if( _type.Contains("eff")){
            _finalBranchName += "_"+_type;
            _optionCHECKHISTO = _optionEff;
        }
        TString _finalBranchNameBS = _finalBranchName+"_BS";
        vector<TString> _allBranches; 
        if( is1D){ 
            MessageSvc::Line();
            MessageSvc::Info("~ Loop 1D variable OPTVAR", _optionVar, ""); 
            MessageSvc::Info("~ Loop 1D variable PRJ   ", _project  , ""); 
            MessageSvc::Info("~ Loop 1D variable TRG   ", _trigger  , ""); 
            MessageSvc::Info("~ Loop 1D variable OPT   ", _option   , ""); 
            MessageSvc::Info("~ Loop 1D variable TYPE  ", _type     , ""); 
            cout<<RED<<"# 1D FILE INPUT : "<< _fileName << endl;
            cout<<RED<<"# 1D KEY        : "<< _mapKey << endl;
            cout<<RED<<"# 1D BRANCH     : "<< _finalBranchName << endl;
            cout<<RED<<"# 1D BRANCH BS  : "<< _finalBranchNameBS << endl;
            cout<<RED<<"# 1D OPTCHECKH  : "<< _optionCHECKHISTO << endl;
        }else{
            MessageSvc::Line();
            MessageSvc::Info("~ Loop 2D variable OPTVAR", _optionVar, ""); 
            MessageSvc::Info("~ Loop 2D variable PRJ   ", _project  , ""); 
            MessageSvc::Info("~ Loop 2D variable TRG   ", _trigger  , ""); 
            MessageSvc::Info("~ Loop 2D variable OPT   ", _option   , ""); 
            MessageSvc::Info("~ Loop 2D variable TYPE  ", _type     , "");             
            cout<<RED<<"# 2D FILE INPUT : "<< _fileName << endl;
            cout<<RED<<"# 2D KEY  : "<< _mapKey << endl;     
            cout<<RED<<"# 2D BRANCH     : "<< _finalBranchName << endl;
            cout<<RED<<"# 2D BRANCH BS  : "<< _finalBranchNameBS << endl;
            cout<<RED<<"# 2D OPTCHECKH  : "<< _optionCHECKHISTO << endl;                   
        }


        if( ! IOSvc::ExistFile(_fileName)) MessageSvc::Error( "GetWeightMapHLT (1D)", _fileName, "EXIT_FAILURE");        
        TFile * _inFile = IOSvc::OpenFile(_fileName, OpenMode::READ);
        if( is1D){
            TH1D  * _histo = _inFile->Get<TH1D>( _mapKey);
            if( _histo == nullptr){
                _inFile->ls();
                MessageSvc::Error("GetWeightMapHLT (1D)", (TString) "Map", _mapKey, "does not exist");
                MessageSvc::Error("GetWeightMapHLT (1D)", (TString) "Map", _mapKey, "EXIT_FAILURE");
            }
            if( TString(_histo->ClassName()) != "TH1D"){
                cout<<RED<<"CLASSNAME : "<< _histo->ClassName() << endl;

                MessageSvc::Error("SEVERE ERROR ! HISTOGRAM PICKED EXPECTED 1D",TString( _histo->ClassName()),"EXIT_FAILURE");
            }

            TH1D * _map = dynamic_cast<TH1D*>( CopyHist(_histo) );
            _map->SetName(_finalBranchName);
            _map->SetDirectory(0);
            CheckHistogram(_map, _optionCHECKHISTO);
            delete _histo;

            //Load the BS histos;
            vector<TH1D> _hists ; 
            if( SettingDef::Weight::useBS && _addBSMaps.at(_optionVar) ){
                for( int bsIDX = 0 ; bsIDX < WeightDefRX::nBS; ++bsIDX){
                    auto *_HISTO_ = _inFile->Get<TH1D>(Form("bs%i/%s", bsIDX, _mapKey.Data()));
                    if( _HISTO_ == nullptr){
                        _inFile->ls();
                        cout<<"->cd(bs"<<bsIDX<<")"<<endl;
                        _inFile->cd(Form("bs%i",bsIDX));
                        _inFile->ls();
                        MessageSvc::Error("GetWeightMapHLTBS",(TString)Form("Map bs%i/%s does not exist", bsIDX, _mapKey.Data()), "EXIT_FAILURE");
                    }
                    TH1D *_MAP_ = dynamic_cast<TH1D*>( CopyHist(_HISTO_) );
                    if( _MAP_== nullptr){ 
                        MessageSvc::Error("Failed casting Histogram", "","EXIT_FAILURE");
                    }
                    _MAP_->SetName(_finalBranchNameBS+"["+to_string(bsIDX)+"]");
                    _MAP_->SetDirectory(0);
                    CheckHistogram( _MAP_, _optionCHECKHISTO);
                    _hists.push_back( * _MAP_);
                    delete _HISTO_;
                }
            }
            IOSvc::CloseFile(_inFile);
            bool _interpolation = _finalBranchName.Contains("wiHLT");
            TString _infoMapSource   = _fileName+":"+_mapKey+":"+ ( _interpolation? "INTERP": "NOINTERP");
            TString _infoMapSourceBS = _fileName+":bsIDX/"+_mapKey+":"+ ( _interpolation? "INTERP": "NOINTERP");
            if( _optionVar == "B_PT-1D"){
                _hltweights.push_back(                              TH1DHistoAdder( *_map, _finalBranchName, _interpolation , _infoMapSource) );
                if( _hists.size() != 0 ) _BShltweights.push_back( BSTH1DHistoAdder( _hists, _finalBranchNameBS, _interpolation, _infoMapSourceBS));
            }else if( _optionVar == "nTracks-1D"){
                _hltweights_nTracks.push_back(                           TH1DHistoAdder( *_map, _finalBranchName, _interpolation , _infoMapSource) );
                if( _hists.size() != 0) _BShltweights_nTracks.push_back( BSTH1DHistoAdder( _hists, _finalBranchNameBS, _interpolation, _infoMapSourceBS));
            }else if( _optionVar == "B_ETA-1D"){
                _hltweights_ETA.push_back(                            TH1DHistoAdder( *_map, _finalBranchName, _interpolation , _infoMapSource) );                 
                if( _hists.size() != 0 ) _BShltweights_ETA.push_back( BSTH1DHistoAdder( _hists, _finalBranchNameBS, _interpolation, _infoMapSourceBS));
            }
        }else{
            TH2D  * _histo = _inFile->Get<TH2D>( _mapKey);
            if( _histo == nullptr){
                _inFile->ls();
                MessageSvc::Error("GetWeightMapHLT (2D)", (TString) "Map", _mapKey, "does not exist");
                MessageSvc::Error("GetWeightMapHLT (2D)", (TString) "Map", _mapKey, "EXIT_FAILURE");
            }
            if( TString(_histo->ClassName()) != "TH2D"){
                MessageSvc::Error("SEVERE ERROR ! HISTOGRAM PICKED EXPECTED 2D","","EXIT_FAILURE");
            }            
            TH2D * _map = dynamic_cast<TH2D*>( CopyHist(_histo) );
            _map->SetName(_finalBranchName);
            _map->SetDirectory(0);
            CheckHistogram(_map, _optionCHECKHISTO);
            delete _histo;
            //Load the BS histos;
            vector<TH2D> _hists ; 
            if( SettingDef::Weight::useBS && _addBSMaps.at(_optionVar) ){
                for( int bsIDX = 0 ; bsIDX < WeightDefRX::nBS; ++bsIDX){
                    auto *_BShisto = _inFile->Get<TH2D>(Form("bs%i/%s", bsIDX, _mapKey.Data()));
                    if( _BShisto == nullptr){
                        _inFile->ls();
                        cout<<"->cd(bs"<<bsIDX<<")"<<endl;
                        _inFile->cd(Form("bs%i",bsIDX));
                        _inFile->ls();
                        MessageSvc::Error("GetWeightMapHLTBS (2D)",(TString)Form("Map bs%i/%s does not exist", bsIDX, _mapKey.Data()), "EXIT_FAILURE");
                    }
                    TH2D *_BSmap = dynamic_cast<TH2D*>( CopyHist(_BShisto) );
                    if( _BSmap== nullptr){ 
                        MessageSvc::Error("Failed casting Histogram (2D)", "","EXIT_FAILURE");
                    }
                    _BSmap->SetName(_finalBranchNameBS+"["+to_string(bsIDX)+"]");
                    _BSmap->SetDirectory(0);
                    CheckHistogram( _BSmap, _optionCHECKHISTO);
                    _hists.push_back( * _BSmap);
                    delete _BShisto;
                }
            }
            IOSvc::CloseFile(_inFile);
            bool _interpolation = _finalBranchName.Contains("wiHLT");
            TString _infoMapSource   = _fileName+":"+_mapKey+":"+ ( _interpolation? "INTERP": "NOINTERP");
            TString _infoMapSourceBS = _fileName+":bsIDX/"+_mapKey+":"+ ( _interpolation? "INTERP": "NOINTERP");
            if( _optionVar == "B_PT_nTracks-2D"){
                _hltweights2D.push_back( TH2DHistAdder( *_map, _finalBranchName, _interpolation , _infoMapSource, _optionCHECKHISTO) );
                if( _hists.size() != 0 ) _BShltweights2D.push_back( BSTH2DHistAdder( _hists, _finalBranchNameBS, _interpolation, _infoMapSourceBS ,_optionCHECKHISTO));
            }
        }
    }
    //=========================
    //Debugging messages
    //========================
    MessageSvc::Line();
    MessageSvc::Info("1D Baseline HLT weights size", to_string(_hltweights.size()));
    for( auto & h : _hltweights){
        cout<<RED<<"*)"<< h.branchName() <<"\n \t"<< h.sourceHisto() << RESET<< endl;
    }
    MessageSvc::Line();
    MessageSvc::Info("1D nTracks HLT weights (nTracks) size", to_string(_hltweights_nTracks.size()));
    for( auto & h : _hltweights_nTracks){
        cout<<RED<<"*)"<< h.branchName() <<"\n \t"<< h.sourceHisto() << RESET<< endl;
    }

    MessageSvc::Line();
    MessageSvc::Info("1D nTracks HLT weights (B_ETA) size", to_string(_hltweights_ETA.size()));
    for( auto & h : _hltweights_ETA){
        cout<<RED<<"*)"<< h.branchName() <<"\n \t"<< h.sourceHisto() << RESET<< endl;
    }

    MessageSvc::Line();
    MessageSvc::Info("2D B_PT-nTracks HLT weights size", to_string(_hltweights2D.size()));
    for( auto & h : _hltweights2D){
        cout<<RED<<"*)"<< h.branchName() <<"\n \t"<< h.sourceHisto() << RESET<< endl;
    }

    if( SettingDef::Weight::useBS){
        MessageSvc::Line();
        MessageSvc::Info("Baseline BSHLT weights (1D) size", to_string(_BShltweights.size()));
        for( auto & h : _BShltweights){
            cout<<RED<<"*)"<< h.branchName() <<"\n \t"<< h.sourceHisto() << RESET<< endl;
        }            
        MessageSvc::Line();
        MessageSvc::Info("Baseline BSHLT weights (1D) size [NTRACKS]", to_string(_BShltweights_nTracks.size()));
        for( auto & h : _BShltweights_nTracks){
            cout<<RED<<"*)"<< h.branchName() <<"\n \t"<< h.sourceHisto() << RESET<< endl;
        }    
        MessageSvc::Line();
        MessageSvc::Info("Baseline BSHLT weights (2D) size", to_string(_BShltweights2D.size()));            
        for( auto & h : _BShltweights2D){
            cout<<RED<<"*)"<< h.branchName() <<"\n \t"<< h.sourceHisto() << RESET<< endl;
        }                  
    }

    //=========================
    //Attach collected maps using the vector functor
    //======================== 
    lastNode = lastNode.Define("hlt_input_var1D_baseline", _inputVariable.second.Data());

    if (_hltweights.size() > 0) {
        lastNode = HelperProcessing::ApplyWeights1D( lastNode, _hltweights , "hlt_input_var1D_baseline");
        MessageSvc::Warning("hlt_input_var1D_baseline", _inputVariable.second);
        if( SettingDef::Weight::useBS) lastNode = HelperProcessing::ApplyBSWeights1D( lastNode, _BShltweights, "hlt_input_var1D_baseline" );
    }

    if( _addWHLT_nTracks){
        lastNode = lastNode.Define("hlt_input_var1D_nTracks","1.*nTracks");
        MessageSvc::Warning("hlt_input_var1D_nTracks", (TString)"nTracks");
        lastNode = HelperProcessing::ApplyWeights1D( lastNode, _hltweights_nTracks , "hlt_input_var1D_nTracks");        
        if( SettingDef::Weight::useBS) lastNode = HelperProcessing::ApplyBSWeights1D( lastNode, _BShltweights_nTracks, "hlt_input_var1D_nTracks");
    }
    if( _addWHLT_BETA ){
        lastNode = lastNode.Define("hlt_input_var1D_BETA", _inputVariableETA.second.Data() );
        MessageSvc::Warning("hlt_input_var1D_BETA used", _inputVariableETA.second);
        lastNode = HelperProcessing::ApplyWeights1D( lastNode, _hltweights_ETA , "hlt_input_var1D_BETA");
        if( SettingDef::Weight::useBS) lastNode = HelperProcessing::ApplyBSWeights1D( lastNode, _BShltweights_ETA , "hlt_input_var1D_BETA"); //B_ETA
    }
    if( _hltweights2D.size() != 0 ){
        lastNode = lastNode.Define("hlt_input_var2D_Y","1.*nTracks");
        MessageSvc::Warning("hlt_input_var2D used (X)", _inputVariable.second);
        MessageSvc::Warning("hlt_input_var2D used (Y)", (TString)"nTracks");
        lastNode = HelperProcessing::ApplyWeights2D( lastNode, _hltweights2D , "hlt_input_var1D_baseline", "hlt_input_var2D_Y" );
        if( SettingDef::Weight::useBS) lastNode = HelperProcessing::ApplyBSWeights2D( lastNode, _BShltweights2D,  "hlt_input_var1D_baseline", "hlt_input_var2D_Y" ); //B_PT,nTracks
    }
    return lastNode; 
}


void HelperProcessing::AddL0Weights( EventType & _eventType, bool _useET){

    //=========================
    //Preliminary informations
    //=========================    
    bool _addWL0   = _eventType.IsMC() && _eventType.GetWeightHolder().IsOption("L0");
    if( !_addWL0){ MessageSvc::Info("AddL0Weights skipped"); return; } //shortcircuit
    if( SettingDef::Weight::useBS){
        MessageSvc::Info("AddL0Weights, enabled bootstrapping maps adder");
    }    
    auto &_configHolder =  _eventType;  

    //=========================
    //Run up the bookkeping of operatiosn to attach weights.
    //=========================
    ROOT::RDataFrame df( "DecayTuple","TupleProcess.root");
    auto lastNode = AppendL0Columns( df, _configHolder,_eventType.GetWeightHolder().Option());    
    //=====================================
    // Dump tuple to disk
    //=====================================       
    MessageSvc::Info("Snapshotting for L0 Weights adding (keep all columns, must drop some? Please add the wildcarding!!!)");
    lastNode.Snapshot("DecayTuple", "TupleProcess_tmp.root");
    MessageSvc::Info("Snapshotting for L0 Weights done");
    //=====================================
    // Shuffle tuple at exit (entries consistenty)
    //=====================================  
    TFile f1("TupleProcess.root","READ");
    auto  original =  (TTree*)f1.Get("DecayTuple");
    auto  entries = original->GetEntries();
    TFile f2("TupleProcess_tmp.root", "READ");
    auto updated = (TTree*)f2.Get("DecayTuple");
    if( original->GetEntries() != updated->GetEntries()) MessageSvc::Error("Severe error on Snapshots", "","EXIT_FAILURE");
    else{ f1.Close(); f2.Close(); }
    
    MessageSvc::Info("rootmv TupleProcess.root:MCDecayTuple TupleProcess_tmp.root");    
    moveTupleFromTo( "MCDecayTuple", "TupleProcess.root", "TupleProcess_tmp.root");
    MessageSvc::Info("rm TupleProcess.root");
    IOSvc::runCommand(TString("rm TupleProcess.root"));
    MessageSvc::Info("mv TupleProcess_tmp.root TupleProcess.root");
    IOSvc::runCommand(TString("mv TupleProcess_tmp.root TupleProcess.root"));
    TFile finalFile("TupleProcess.root","READ");
    auto finalTree = (TTree*)finalFile.Get("DecayTuple");
    if( finalTree == nullptr) MessageSvc::Error("Failure L0 weights adder","","EXIT_FAILURE");
    if( finalTree->GetEntries() != entries) MessageSvc::Error("Failure L0 weights adder", "","EXIT_FAILURE");
    MessageSvc::Info("Closing file");
    finalFile.Close();
    return;
}

RNode HelperProcessing::AppendRW1DColumns(RNode df,  ConfigHolder & _configHolder, TString _weightOption){

    WeightHolder _weightHolder(_configHolder, _weightOption);
    auto lastNode = df.Define( "DUMMYRW1D", "1>0");
    //=========================
    //Functor-containers of TH2D histos (nominal ones)
    //=========================        
    vector< TH1DHistoAdder > _rw1dWeights_B0_MM_L0L_nTracks;
    vector< TH1DHistoAdder > _rw1dWeights_B0_MM_L0L_BPT;
    vector< TH1DHistoAdder > _rw1dWeights_B0_MM_L0L_BENDVTX;
    vector< TH1DHistoAdder > _rw1dWeights_B0_MM_L0L_KPT;
    vector< TH1DHistoAdder > _rw1dWeights_B0_MM_L0L_M1PT;
    vector< TH1DHistoAdder > _rw1dWeights_B0_MM_L0L_M2PT;
    vector< TH1DHistoAdder > _rw1dWeights_Bp_MM_L0L_nTracks;
    vector< TH1DHistoAdder > _rw1dWeights_Bp_MM_L0L_BPT;
    vector< TH1DHistoAdder > _rw1dWeights_Bp_MM_L0L_BENDVTX;
    vector< TH1DHistoAdder > _rw1dWeights_Bp_MM_L0L_KPT;
    vector< TH1DHistoAdder > _rw1dWeights_Bp_MM_L0L_M1PT;
    vector< TH1DHistoAdder > _rw1dWeights_Bp_MM_L0L_M2PT;

    //=========================
    //Preliminaries (name branches)
    //========================
    map< TString, TString > _names = _configHolder.GetParticleNames();
    TString _head = _names["HEAD"];
    TString _lep  = ((TString) _names["L1"]).Remove(1);
    TString _lep1 = ((TString) _names["L1"]);
    TString _lep2 = ((TString) _names["L2"]);
    TString _hadR = ((TString) _names["HH"]);
    TString _had1 = ((TString) _names["H1"]);
    TString _had2 = ((TString) _names["H2"]);

    //The combiner of cases to attach 
    vector< TString > _wRW1D_projects       = {"B0", "Bp"}; // weights from Bp , B0
    vector< TString > _wRW1D_triggers       = {to_string(Trigger::L0L)};
    vector< TString > _wRW1D_triggerConfs   = {"incl"};   //, "incl", "exclOverIncl", "excl", "comb"};

    lastNode = lastNode.Define("rw1d_nTracks", "1. * nTracks");
    // lastNode = lastNode.Define("rw1d_B_ENDVERTEX_CHI2_NDOF", TString("{HEAD}_ENDVERTEX_CHI2/{HEAD}_ENDVERTEX_NDOF").ReplaceAll("{HEAD}", _head).Data());
    // vector< TString > branches = {"nTracks", "{HEAD}_PT", "{HEAD}_ENDVERTEX_CHI2_NDOF", "K_PT", "M1_PT", "M2_PT"};
    vector< TString > branches = {"nTracks", "{HEAD}_PT", "K_PT", "M1_PT", "M2_PT"};

    //=========================
    //Cross-product combinations and filler of Histo2DAdders.
    //========================
    TString _finalBanchDefB0 = "";
    TString _finalBanchDefBp = "";
    MessageSvc::Info("AppendRW1DColumns create hist adder");
    for (TString _project : _wRW1D_projects) {
        for (TString _branch : branches) {
            _branch = _branch.ReplaceAll("{HEAD}", _project);
            TString _mapLabel = _project + "_" + _branch;
            TString _branchLabel = TString("wRW1D_") + _project + "_MM_L0L_" + _branch;
            bool _interpolate = false;
            if (_project == "B0") {
                if (_finalBanchDefB0 == "") _finalBanchDefB0 = _branchLabel;
                else _finalBanchDefB0 += TString(" * ") + _branchLabel;
                if (_branch == "nTracks") {
                    _rw1dWeights_B0_MM_L0L_nTracks.push_back( TH1DHistoAdder( * _weightHolder.GetWeightMapRW1D( _mapLabel ), _branchLabel, _interpolate , "") );
                }
                if (_branch == "B0_PT") {
                    _rw1dWeights_B0_MM_L0L_BPT.push_back( TH1DHistoAdder( * _weightHolder.GetWeightMapRW1D( _mapLabel ), _branchLabel, _interpolate , "") );
                }
                if (_branch == "B0_ENDVERTEX_CHI2_NDOF") {
                    _rw1dWeights_B0_MM_L0L_BENDVTX.push_back( TH1DHistoAdder( * _weightHolder.GetWeightMapRW1D( _mapLabel ), _branchLabel, _interpolate , "") );
                }
                if (_branch == "K_PT") {
                    _rw1dWeights_B0_MM_L0L_KPT.push_back( TH1DHistoAdder( * _weightHolder.GetWeightMapRW1D( _mapLabel ), _branchLabel, _interpolate , "") );
                }
                if (_branch == "M1_PT") {
                    _rw1dWeights_B0_MM_L0L_M1PT.push_back( TH1DHistoAdder( * _weightHolder.GetWeightMapRW1D( _mapLabel ), _branchLabel, _interpolate , "") );
                }
                if (_branch == "M2_PT") {
                    _rw1dWeights_B0_MM_L0L_M2PT.push_back( TH1DHistoAdder( * _weightHolder.GetWeightMapRW1D( _mapLabel ), _branchLabel, _interpolate , "") );
                }
            }
            if (_project == "Bp") {
                if (_finalBanchDefBp == "") _finalBanchDefBp = _branchLabel;
                else _finalBanchDefBp += TString(" * ") + _branchLabel;
                if (_branch == "nTracks") {
                    _rw1dWeights_Bp_MM_L0L_nTracks.push_back( TH1DHistoAdder( * _weightHolder.GetWeightMapRW1D( _mapLabel ), _branchLabel, _interpolate , "") );
                }
                if (_branch == "Bp_PT") {
                    _rw1dWeights_Bp_MM_L0L_BPT.push_back( TH1DHistoAdder( * _weightHolder.GetWeightMapRW1D( _mapLabel ), _branchLabel, _interpolate , "") );
                }
                if (_branch == "Bp_ENDVERTEX_CHI2_NDOF") {
                    _rw1dWeights_Bp_MM_L0L_BENDVTX.push_back( TH1DHistoAdder( * _weightHolder.GetWeightMapRW1D( _mapLabel ), _branchLabel, _interpolate , "") );
                }
                if (_branch == "K_PT") {
                    _rw1dWeights_Bp_MM_L0L_KPT.push_back( TH1DHistoAdder( * _weightHolder.GetWeightMapRW1D( _mapLabel ), _branchLabel, _interpolate , "") );
                }
                if (_branch == "M1_PT") {
                    _rw1dWeights_Bp_MM_L0L_M1PT.push_back( TH1DHistoAdder( * _weightHolder.GetWeightMapRW1D( _mapLabel ), _branchLabel, _interpolate , "") );
                }
                if (_branch == "M2_PT") {
                    _rw1dWeights_Bp_MM_L0L_M2PT.push_back( TH1DHistoAdder( * _weightHolder.GetWeightMapRW1D( _mapLabel ), _branchLabel, _interpolate , "") );
                }
            }
        }
    }

    auto _inputPT_usage = []( double b_pt, double b_truept, int bkgcat){
        if(bkgcat != 60) return b_truept;
        return b_pt;
    };

    MessageSvc::Info("Starting histogram attachment. ");
    if(_rw1dWeights_B0_MM_L0L_nTracks.size() != 0 ){    
        for (TString _branch : branches) {
            _branch = _branch.ReplaceAll("{HEAD}", "B0");
            MessageSvc::Info("Hist", _branch);
            if (_branch == "nTracks") {
                TString _inputVarX = "rw1d_nTracks";
                lastNode = HelperProcessing::ApplyWeights1D(lastNode, _rw1dWeights_B0_MM_L0L_nTracks, _inputVarX);
            }
            else if (_branch == "B0_PT") {
                TString _inputVarX = TString("{HEAD}_PT").ReplaceAll("{HEAD}", _head);
                if (_weightOption.Contains("MCT")) _inputVarX.ReplaceAll("_PT", "_TRUEPT");
                lastNode = HelperProcessing::ApplyWeights1D(lastNode, _rw1dWeights_B0_MM_L0L_BPT, _inputVarX);
            }
            else if (_branch == "B0_ENDVERTEX_CHI2_NDOF") {
                TString _inputVarX = "rw1d_B_ENDVERTEX_CHI2_NDOF";
                lastNode = HelperProcessing::ApplyWeights1D(lastNode, _rw1dWeights_B0_MM_L0L_BENDVTX, _inputVarX);
            }
            else if (_branch == "K_PT") {
                TString _inputVarX = "K_PT";
                if (_weightOption.Contains("MCT")) _inputVarX.ReplaceAll("_PT", "_TRUEPT");
                lastNode = HelperProcessing::ApplyWeights1D(lastNode, _rw1dWeights_B0_MM_L0L_KPT, _inputVarX);
            }
            else if (_branch == "M1_PT") {
                TString _inputVarX = "M1_PT";
                if (_weightOption.Contains("MCT")) _inputVarX.ReplaceAll("_PT", "_TRUEPT");
                lastNode = HelperProcessing::ApplyWeights1D(lastNode, _rw1dWeights_B0_MM_L0L_M1PT, _inputVarX);
            }
            else if (_branch == "M2_PT") {
                TString _inputVarX = "M2_PT";
                if (_weightOption.Contains("MCT")) _inputVarX.ReplaceAll("_PT", "_TRUEPT");
                lastNode = HelperProcessing::ApplyWeights1D(lastNode, _rw1dWeights_B0_MM_L0L_M2PT, _inputVarX);
            }
        }
        TString _finalBanchLabel = "wRW1D_B0_MM_L0L";
        lastNode = lastNode.Define(_finalBanchLabel, _finalBanchDefB0.Data());
    }

    if(_rw1dWeights_Bp_MM_L0L_nTracks.size() != 0 ){    
        for (TString _branch : branches) {
            _branch == _branch.ReplaceAll("{HEAD}", "Bp");
            MessageSvc::Info("Hist", _branch);
            if (_branch == "nTracks") {
                TString _inputVarX = "rw1d_nTracks";
                lastNode = HelperProcessing::ApplyWeights1D(lastNode, _rw1dWeights_Bp_MM_L0L_nTracks, _inputVarX);
            }
            else if (_branch == "Bp_PT") {
                TString _inputVarX = TString("{HEAD}_PT").ReplaceAll("{HEAD}", _head);
                if (_weightOption.Contains("MCT")) _inputVarX.ReplaceAll("_PT", "_TRUEPT");
                lastNode = HelperProcessing::ApplyWeights1D(lastNode, _rw1dWeights_Bp_MM_L0L_BPT, _inputVarX);
            }
            else if (_branch == "Bp_ENDVERTEX_CHI2_NDOF") {
                TString _inputVarX = "rw1d_B_ENDVERTEX_CHI2_NDOF";
                lastNode = HelperProcessing::ApplyWeights1D(lastNode, _rw1dWeights_Bp_MM_L0L_BENDVTX, _inputVarX);
            }
            else if (_branch == "K_PT") {
                TString _inputVarX = "K_PT";
                if (_weightOption.Contains("MCT")) _inputVarX.ReplaceAll("_PT", "_TRUEPT");
                lastNode = HelperProcessing::ApplyWeights1D(lastNode, _rw1dWeights_Bp_MM_L0L_KPT, _inputVarX);
            }
            else if (_branch == "M1_PT") {
                TString _inputVarX = "M1_PT";
                if (_weightOption.Contains("MCT")) _inputVarX.ReplaceAll("_PT", "_TRUEPT");
                lastNode = HelperProcessing::ApplyWeights1D(lastNode, _rw1dWeights_Bp_MM_L0L_M1PT, _inputVarX);
            }
            else if (_branch == "M2_PT") {
                TString _inputVarX = "M2_PT";
                if (_weightOption.Contains("MCT")) _inputVarX.ReplaceAll("_PT", "_TRUEPT");
                lastNode = HelperProcessing::ApplyWeights1D(lastNode, _rw1dWeights_Bp_MM_L0L_M2PT, _inputVarX);
            }
        }
        TString _finalBanchLabel = "wRW1D_Bp_MM_L0L";
        lastNode = lastNode.Define(_finalBanchLabel, _finalBanchDefBp.Data());
    }

    return lastNode; 
}


RNode HelperProcessing::AppendL0Columns(RNode df,  ConfigHolder & _configHolder, TString _weightOption){

    WeightHolder _weightHolder(_configHolder, _weightOption);
    auto lastNode = df.Define( "DUMMYL0", "1>0");
    //=========================
    //Functor-containers of TH2D histos (nominal ones)
    //=========================        
    vector< TH2DHistAdder   > _l0weights_CL_L0I;    //baseline weights (data)
    vector< TH2DHistAdder   > _l0weights_CL_L0L_Comb_L1;    //varied L0L weights (data) [dilepton]
    vector< TH2DHistAdder   > _l0weights_CL_L0L_Comb_L2;    //varied L0L weights (data) [dilepton]
    vector< TH2DHistAdder   > _l0weights_CL_Dist_L1;    //varied L0Le weights (data) ECAL distance instead ECAL region
    vector< TH2DHistAdder   > _l0weights_CL_Dist_L2;    //varied L0Le weights (data) ECAL distance instead ECAL region
    vector< TH2DHistAdder   > _l0weights_CL_L1;    //baseline weights (data)
    vector< TH2DHistAdder   > _l0weights_CL_L2;    //baseline weights (data)
    vector< TH2DHistAdder   > _l0weights_CL_L0H;    //baseline weights (data)

    vector< TH2DHistAdder   > _l0weights_MC_L0I;    //baseline weights (MC)
    vector< TH2DHistAdder   > _l0weights_MC_L0L_Comb_L1;    //varied L0L weights (MC) [dilepton]
    vector< TH2DHistAdder   > _l0weights_MC_L0L_Comb_L2;    //varied L0L weights (MC) [dilepton]
    vector< TH2DHistAdder   > _l0weights_MC_Dist_L1;    //varied L0Le weights (MC) ECAL distance instead ECAL region
    vector< TH2DHistAdder   > _l0weights_MC_Dist_L2;    //varied L0Le weights (MC) ECAL distance instead ECAL region
    vector< TH2DHistAdder   > _l0weights_MC_L1;    //baseline weights (MC)
    vector< TH2DHistAdder   > _l0weights_MC_L2;    //baseline weights (MC)
    vector< TH2DHistAdder   > _l0weights_MC_L0H;    //baseline weights (MC)


    vector< TH2DHistAdderL0EBremSplit   > _l0weights_CL_L1_BremSplit;    //baseline weights (data)
    vector< TH2DHistAdderL0EBremSplit   > _l0weights_CL_L2_BremSplit;    //baseline weights (data)
    vector< TH2DHistAdderL0EBremSplit   > _l0weights_MC_L1_BremSplit;    //baseline weights (MC)
    vector< TH2DHistAdderL0EBremSplit   > _l0weights_MC_L2_BremSplit;    //baseline weights (MC)



    //=========================
    //Functor-containers of vector<TH2D> histos (Bootstrapped maps)
    //=========================        
    vector< BSTH2DHistAdder   > _BS_l0weights_CL_L0I;    //baseline weights (data)
    vector< BSTH2DHistAdder   > _BS_l0weights_CL_L0L_Comb_L1;    //varied L0L weights (data) [dilepton]
    vector< BSTH2DHistAdder   > _BS_l0weights_CL_L0L_Comb_L2;    //varied L0L weights (data) [dilepton]
    vector< BSTH2DHistAdder   > _BS_l0weights_CL_Dist_L1;    //varied L0Le weights (data) ECAL distance instead ECAL region
    vector< BSTH2DHistAdder   > _BS_l0weights_CL_Dist_L2;    //varied L0Le weights (data) ECAL distance instead ECAL region
    vector< BSTH2DHistAdder   > _BS_l0weights_CL_L1;    //baseline weights (data)
    vector< BSTH2DHistAdder   > _BS_l0weights_CL_L2;    //baseline weights (data)
    vector< BSTH2DHistAdder   > _BS_l0weights_CL_L0H;    //baseline weights (data)

    vector< BSTH2DHistAdder   > _BS_l0weights_MC_L0I;    //baseline weights (MC)
    vector< BSTH2DHistAdder   > _BS_l0weights_MC_L0L_Comb_L1;    //varied L0L weights (MC) [dilepton]
    vector< BSTH2DHistAdder   > _BS_l0weights_MC_L0L_Comb_L2;    //varied L0L weights (MC) [dilepton]
    vector< BSTH2DHistAdder   > _BS_l0weights_MC_Dist_L1;    //varied L0Le weights (MC) ECAL distance instead ECAL region
    vector< BSTH2DHistAdder   > _BS_l0weights_MC_Dist_L2;    //varied L0Le weights (MC) ECAL distance instead ECAL region
    vector< BSTH2DHistAdder   > _BS_l0weights_MC_L1;    //baseline weights (MC)
    vector< BSTH2DHistAdder   > _BS_l0weights_MC_L2;    //baseline weights (MC)
    vector< BSTH2DHistAdder   > _BS_l0weights_MC_L0H;    //baseline weights (MC)


    //=========================
    //Preliminaries (name branches)
    //========================
    map< TString, TString > _names = _configHolder.GetParticleNames();
    TString _head   = _names["HEAD"];
    TString _lep    = ((TString) _names["L1"]).Remove(1);
    TString _lep1   = ((TString) _names["L1"]);
    TString _lep2   = ((TString) _names["L2"]);
    TString _hadR   = ((TString) _names["HH"]);
    TString _had1   = ((TString) _names["H1"]);
    TString _had2   = ((TString) _names["H2"]);

    TString _optionWRatio = "-ratior";
    TString _optionEff = "-effr";    
    //The combiner of cases to attach 
    vector< TString > _wL0_projects = {"B0", "Bp"}; // weights from Bp , B0
    if (_configHolder.GetProject() == Prj::RPhi) _wL0_projects = {"Bs"}; // for RPhi use weights from Bs (Bp, planned to be readded)
    const vector< TString > _wL0_options  = {"f"};        //"", "i", "f"};
    vector< TString > _wL0_triggers       = {to_string(Trigger::L0I), to_string(Trigger::L0L)}; // {to_string(Trigger::L0H)};
    vector< TString > _wL0_triggerConfs   = {"incl"};   //, "incl", "exclOverIncl", "excl", "comb"};
    if(SettingDef::Weight::priorChain){
        _wL0_triggers = {to_string(Trigger::L0L)}; // in prior chain only use L0L (muon maps)
        if (_weightOption.Contains("fromLOI"))
            _wL0_triggers = {to_string(Trigger::L0I)};
    }
    // if (_weightHolder.Option().Contains("COMB")) {
    //     _wL0_triggerConfs = {"incl", "comb"}; //add the COMBINED weight map on L0L
    // }
    //=========================
    //Cross-product combinations and filler of Histo2DAdders.
    //========================    
    for (const auto && [_project, _trigger, _triggerConf, _option] : iter::product(_wL0_projects, _wL0_triggers, _wL0_triggerConfs, _wL0_options)) {
        TString _opt = "no";
        if (_option == "i") _opt = "interp";
        if (_option == "f") _opt = "fit";
        TString _bin = "";
        if (_option != "f") _bin = "_coarse";
        TString _mapLabel    = "L0_" + _project + "_" + _trigger + "_" + _triggerConf + "_" + _opt + _bin;
        MessageSvc::Info("AddL0Weights (loop mapLabel)", _mapLabel);
        if (_trigger == to_string(Trigger::L0I)){
            if (_triggerConf == "comb") continue;
            //=========================
            //Grab L0I maps
            //========================    
            TString _branchLabel = _head + "_w" + _option + _trigger + "_" + _triggerConf + "_" + _project;
            bool _interpolate = true; // for L0I maps, we enable interpolation across nTracks slots            
            _l0weights_CL_L0I.push_back( TH2DHistAdder( *  _weightHolder.GetWeightMapL02D(_mapLabel + _optionEff + "-effCL") , 
                                            _branchLabel +"_effCL", //final branch name
                                            _interpolate,           //enable interpolation when GetHistogramVal ?
                                            "",                     //infomaps slots...
                                            "effr" ) );             //option ( it's an efficiency map, values [0,1]) bound
            _l0weights_MC_L0I.push_back( TH2DHistAdder( *  _weightHolder.GetWeightMapL02D(  _mapLabel + _optionEff + "-effMC") , _branchLabel +"_effMC",    _interpolate, "", "effr" ) );

            if (SettingDef::Weight::useBS) {
        		_BS_l0weights_CL_L0I.push_back( BSTH2DHistAdder(  _weightHolder.GetWeightMapL02DBS(_mapLabel + _optionEff + "-effCL") , _branchLabel +"_effCL_BS", _interpolate, "", "effr" ) );
        		_BS_l0weights_MC_L0I.push_back( BSTH2DHistAdder(  _weightHolder.GetWeightMapL02DBS(_mapLabel + _optionEff + "-effMC") , _branchLabel +"_effMC_BS", _interpolate, "", "effr" ) );
    	    }
        }else if( _trigger == to_string(Trigger::L0L)){            
            // if (_triggerConf == "comb" ) {
            if (_weightHolder.Option().Contains("COMB") && _configHolder.GetAna() == Analysis::EE) {
                //=========================
                //Grab L0L comb maps
                //========================
                bool _interpolate = false; // for L0L maps, we disable interpolation across 2D spectrum
                TString _branchLabel = _lep1 + "_w" + _option + _trigger + "_" + "comb" + "_" + _project;
                _l0weights_CL_L0L_Comb_L1.push_back( TH2DHistAdder( *_weightHolder.GetWeightMapL02D(  _mapLabel + _optionEff + "-effCL") , _branchLabel +"_effCL",    _interpolate, "", "effr" ) ); 
                _l0weights_MC_L0L_Comb_L1.push_back( TH2DHistAdder( *_weightHolder.GetWeightMapL02D(  _mapLabel + _optionEff + "-effMC") , _branchLabel +"_effMC",    _interpolate, "", "effr" ) ); //option ( it's an efficiency map, values [0,1]) bound
                if (SettingDef::Weight::useBS) {
                    _BS_l0weights_CL_L0L_Comb_L1.push_back( BSTH2DHistAdder( _weightHolder.GetWeightMapL02DBS(_mapLabel + _optionEff + "-effCL") , _branchLabel +"_effCL_BS", _interpolate, "", "effr" ) ); 
                    _BS_l0weights_MC_L0L_Comb_L1.push_back( BSTH2DHistAdder( _weightHolder.GetWeightMapL02DBS(_mapLabel + _optionEff + "-effMC") , _branchLabel +"_effMC_BS", _interpolate, "", "effr" ) ); //option ( it's an efficiency map, values [0,1]) bound
                }
                _branchLabel = _lep2 + "_w" + _option + _trigger + "_" + "comb" + "_" + _project;
                _l0weights_CL_L0L_Comb_L2.push_back( TH2DHistAdder( *_weightHolder.GetWeightMapL02D(  _mapLabel + _optionEff + "-effCL") , _branchLabel +"_effCL",    _interpolate, "", "effr" ) ); 
                _l0weights_MC_L0L_Comb_L2.push_back( TH2DHistAdder( *_weightHolder.GetWeightMapL02D(  _mapLabel + _optionEff + "-effMC") , _branchLabel +"_effMC",    _interpolate, "", "effr" ) ); //option ( it's an efficiency map, values [0,1]) bound
                if (SettingDef::Weight::useBS) {
                    _BS_l0weights_CL_L0L_Comb_L2.push_back( BSTH2DHistAdder( _weightHolder.GetWeightMapL02DBS(_mapLabel + _optionEff + "-effCL") , _branchLabel +"_effCL_BS", _interpolate, "", "effr" ) ); 
                    _BS_l0weights_MC_L0L_Comb_L2.push_back( BSTH2DHistAdder( _weightHolder.GetWeightMapL02DBS(_mapLabel + _optionEff + "-effMC") , _branchLabel +"_effMC_BS", _interpolate, "", "effr" ) ); //option ( it's an efficiency map, values [0,1]) bound
                }
            } else if (_weightHolder.Option().Contains("DIST") && _configHolder.GetAna() == Analysis::EE) {
                //=========================
                //Grab L0L comb maps
                //========================
                bool _interpolate = false;
                TString _branchLabel_L1 = _lep1 + "_w"+ _option + _trigger + "_" + "dist" + "_" + _triggerConf + "_" + _project;
                _l0weights_CL_Dist_L1.push_back( TH2DHistAdder( *_weightHolder.GetWeightMapL02D(_mapLabel + _optionEff + "-effCL") ,   _branchLabel_L1 +"_effCL",    _interpolate, "", "effr" ) ); //option ( it's an efficiency map, values [0,1]) bound
                _l0weights_MC_Dist_L1.push_back( TH2DHistAdder( *_weightHolder.GetWeightMapL02D(_mapLabel + _optionEff + "-effMC") ,   _branchLabel_L1 +"_effMC",    _interpolate, "", "effr" ) ); //option ( it's an efficiency map, values [0,1]) bound
                if (SettingDef::Weight::useBS) {
                    _BS_l0weights_CL_Dist_L1.push_back( BSTH2DHistAdder( _weightHolder.GetWeightMapL02DBS(_mapLabel + _optionEff + "-effCL") , _branchLabel_L1 +"_effCL_BS", _interpolate, "", "effr" ) ); //option ( it's an efficiency map, values [0,1]) bound
                    _BS_l0weights_MC_Dist_L1.push_back( BSTH2DHistAdder( _weightHolder.GetWeightMapL02DBS(_mapLabel + _optionEff + "-effMC") , _branchLabel_L1 +"_effMC_BS", _interpolate, "", "effr" ) ); //option ( it's an efficiency map, values [0,1]) bound
                }
                TString _branchLabel_L2 = _lep2 + "_w"+ _option + _trigger + "_" + "dist" + "_" + _triggerConf + "_" + _project;
                _l0weights_CL_Dist_L2.push_back( TH2DHistAdder( *_weightHolder.GetWeightMapL02D(_mapLabel + _optionEff + "-effCL"),   _branchLabel_L2 +"_effCL",   _interpolate, "", "effr" ) ); //option ( it's an efficiency map, values [0,1]) bound
                _l0weights_MC_Dist_L2.push_back( TH2DHistAdder( *_weightHolder.GetWeightMapL02D(_mapLabel + _optionEff + "-effMC"),   _branchLabel_L2 +"_effMC",   _interpolate, "", "effr" ) ); //option ( it's an efficiency map, values [0,1]) bound
                if (SettingDef::Weight::useBS) {
                    _BS_l0weights_CL_Dist_L2.push_back( BSTH2DHistAdder( _weightHolder.GetWeightMapL02DBS(_mapLabel + _optionEff + "-effCL"), _branchLabel_L2 +"_effCL_BS",_interpolate, "", "effr" ) ); //option ( it's an efficiency map, values [0,1]) bound
                    _BS_l0weights_MC_Dist_L2.push_back( BSTH2DHistAdder( _weightHolder.GetWeightMapL02DBS(_mapLabel + _optionEff + "-effMC"), _branchLabel_L2 +"_effMC_BS",_interpolate, "", "effr" ) ); //option ( it's an efficiency map, values [0,1]) bound
                }
            }else {
                //========================
                //Grab L0L (L1,L2) maps (both muons and electrons)
                //========================  
                bool _interpolate = false;
                TString _branchLabel_L1 = _lep1 + "_w"+ _option + _trigger + "_" + _triggerConf + "_" + _project;
                _l0weights_CL_L1.push_back( TH2DHistAdder( *_weightHolder.GetWeightMapL02D(_mapLabel + _optionEff + "-effCL") ,   _branchLabel_L1 +"_effCL",    _interpolate, "", "effr" ) ); //option ( it's an efficiency map, values [0,1]) bound
                _l0weights_MC_L1.push_back( TH2DHistAdder( *_weightHolder.GetWeightMapL02D(_mapLabel + _optionEff + "-effMC") ,   _branchLabel_L1 +"_effMC",    _interpolate, "", "effr" ) ); //option ( it's an efficiency map, values [0,1]) bound
                if (SettingDef::Weight::useBS) {
                    _BS_l0weights_CL_L1.push_back( BSTH2DHistAdder( _weightHolder.GetWeightMapL02DBS(_mapLabel + _optionEff + "-effCL") , _branchLabel_L1 +"_effCL_BS", _interpolate, "", "effr" ) ); //option ( it's an efficiency map, values [0,1]) bound
                    _BS_l0weights_MC_L1.push_back( BSTH2DHistAdder( _weightHolder.GetWeightMapL02DBS(_mapLabel + _optionEff + "-effMC") , _branchLabel_L1 +"_effMC_BS", _interpolate, "", "effr" ) ); //option ( it's an efficiency map, values [0,1]) bound
                }
                TString _branchLabel_L2 = _lep2 + "_w"+ _option + _trigger + "_" + _triggerConf + "_" + _project;
                _l0weights_CL_L2.push_back( TH2DHistAdder( *_weightHolder.GetWeightMapL02D(_mapLabel + _optionEff + "-effCL"),   _branchLabel_L2 +"_effCL",   _interpolate, "", "effr" ) ); //option ( it's an efficiency map, values [0,1]) bound
                _l0weights_MC_L2.push_back( TH2DHistAdder( *_weightHolder.GetWeightMapL02D(_mapLabel + _optionEff + "-effMC"),   _branchLabel_L2 +"_effMC",   _interpolate, "", "effr" ) ); //option ( it's an efficiency map, values [0,1]) bound
                if (SettingDef::Weight::useBS) {
                    _BS_l0weights_CL_L2.push_back( BSTH2DHistAdder( _weightHolder.GetWeightMapL02DBS(_mapLabel + _optionEff + "-effCL"), _branchLabel_L2 +"_effCL_BS",_interpolate, "", "effr" ) ); //option ( it's an efficiency map, values [0,1]) bound
                    _BS_l0weights_MC_L2.push_back( BSTH2DHistAdder( _weightHolder.GetWeightMapL02DBS(_mapLabel + _optionEff + "-effMC"), _branchLabel_L2 +"_effMC_BS",_interpolate, "", "effr" ) ); //option ( it's an efficiency map, values [0,1]) bound
                }

                if( _configHolder.GetAna() == Analysis::EE  && _weightOption.Contains("BREM")  ){
                    _l0weights_CL_L1_BremSplit.push_back(  TH2DHistAdderL0EBremSplit( *_weightHolder.GetWeightMapL02D(_mapLabel + _optionEff + "-effCL-Brem0") , 
                                                                                      *_weightHolder.GetWeightMapL02D(_mapLabel + _optionEff + "-effCL-Brem1") , 
                                                                                     _branchLabel_L1+"_brem_effCL", _interpolate, "", "effr" ) );                    
                    _l0weights_MC_L1_BremSplit.push_back(  TH2DHistAdderL0EBremSplit( *_weightHolder.GetWeightMapL02D(_mapLabel + _optionEff + "-effMC-Brem0") , 
                                                                                      *_weightHolder.GetWeightMapL02D(_mapLabel + _optionEff + "-effMC-Brem1") , 
                                                                                     _branchLabel_L1+"_brem_effMC", _interpolate,"",  "effr" )
                    );  
                    _l0weights_CL_L2_BremSplit.push_back(  TH2DHistAdderL0EBremSplit( *_weightHolder.GetWeightMapL02D(_mapLabel + _optionEff + "-effCL-Brem0") , 
                                                                                      *_weightHolder.GetWeightMapL02D(_mapLabel + _optionEff + "-effCL-Brem1") , 
                                                                                     _branchLabel_L2+"_brem_effCL", _interpolate,"",  "effr" )
                    );                   
                    _l0weights_MC_L2_BremSplit.push_back(  TH2DHistAdderL0EBremSplit( *_weightHolder.GetWeightMapL02D(_mapLabel + _optionEff + "-effMC-Brem0") , 
                                                                                      *_weightHolder.GetWeightMapL02D(_mapLabel + _optionEff + "-effMC-Brem1") , 
                                                                                     _branchLabel_L2+"_brem_effMC", _interpolate,"",  "effr" )
                    );
                }
            }
        } else if ( _trigger == to_string(Trigger::L0H)) {
            if (_triggerConf == "comb") continue;
            //=========================
            //Grab L0H maps
            //========================    
            TString _branchLabel = _head + "_w" + _option + _trigger + "_" + _triggerConf + "_" + _project;
            bool _interpolate = false; // for L0H maps, we disable interpolation across 2D spectrum 
            _l0weights_CL_L0H.push_back( TH2DHistAdder( * _weightHolder.GetWeightMapL02D(_mapLabel + _optionEff + "-effCL") , _branchLabel +"_effCL", _interpolate, "", "effr" ) );                                                   
            _l0weights_MC_L0H.push_back( TH2DHistAdder( * _weightHolder.GetWeightMapL02D(_mapLabel + _optionEff + "-effMC") , _branchLabel +"_effMC", _interpolate, "", "effr" ) );
            if( SettingDef::Weight::useBS){
                _BS_l0weights_CL_L0H.push_back( BSTH2DHistAdder( _weightHolder.GetWeightMapL02DBS(_mapLabel + _optionEff + "-effCL") , _branchLabel +"_effCL_BS", _interpolate, "","effr" ) );
                _BS_l0weights_MC_L0H.push_back( BSTH2DHistAdder( _weightHolder.GetWeightMapL02DBS(_mapLabel + _optionEff + "-effMC") , _branchLabel +"_effMC_BS", _interpolate,  "",  "effr" ) );
            }
        }
    }
    auto check_same_size_CL_MC = []( vector<TH2DHistAdder> & a, vector<TH2DHistAdder> & b){
        if(a.size() != b.size()) MessageSvc::Error("Invalid weights (CL and MC same size)");
	MessageSvc::Info("Checked weight vector size of CL and MC.");
        return 0;
    };
    auto check_same_size_CL_MC_BS = []( vector<BSTH2DHistAdder> & a, vector<BSTH2DHistAdder> & b){
        if(a.size() != b.size()) MessageSvc::Error("Invalid weights (CL and MC same size)");
        MessageSvc::Info("Checked BS weight vector size of CL and MC.");
        return 0;
    };    
    
    check_same_size_CL_MC( _l0weights_CL_L0I, _l0weights_MC_L0I);
    check_same_size_CL_MC( _l0weights_CL_L0L_Comb_L1, _l0weights_MC_L0L_Comb_L1);
    check_same_size_CL_MC( _l0weights_CL_L0L_Comb_L2, _l0weights_MC_L0L_Comb_L2);
    check_same_size_CL_MC( _l0weights_CL_Dist_L1, _l0weights_MC_Dist_L1);
    check_same_size_CL_MC( _l0weights_CL_Dist_L2, _l0weights_MC_Dist_L2);
    check_same_size_CL_MC( _l0weights_CL_L1, _l0weights_MC_L1);
    check_same_size_CL_MC( _l0weights_CL_L2, _l0weights_MC_L2);
    check_same_size_CL_MC( _l0weights_CL_L1, _l0weights_MC_L2); //extra check for it.
    check_same_size_CL_MC( _l0weights_CL_L0H, _l0weights_MC_L0H);

    if( SettingDef::Weight::useBS){                
        check_same_size_CL_MC_BS( _BS_l0weights_CL_L0I, _BS_l0weights_MC_L0I);
        check_same_size_CL_MC_BS( _BS_l0weights_CL_L0L_Comb_L1, _BS_l0weights_MC_L0L_Comb_L1);
        check_same_size_CL_MC_BS( _BS_l0weights_CL_L0L_Comb_L2, _BS_l0weights_MC_L0L_Comb_L2);
        check_same_size_CL_MC_BS( _BS_l0weights_CL_Dist_L1, _BS_l0weights_MC_Dist_L1);
        check_same_size_CL_MC_BS( _BS_l0weights_CL_Dist_L2, _BS_l0weights_MC_Dist_L2);
        check_same_size_CL_MC_BS( _BS_l0weights_CL_L1, _BS_l0weights_MC_L1);
        check_same_size_CL_MC_BS( _BS_l0weights_CL_L2, _BS_l0weights_MC_L2);
        check_same_size_CL_MC_BS( _BS_l0weights_CL_L1, _BS_l0weights_MC_L2); //extra check for it.
        check_same_size_CL_MC_BS( _BS_l0weights_CL_L0H, _BS_l0weights_MC_L0H);
    }
    auto _inputPT_usage = []( double b_pt, double b_truept, int bkgcat){
        if(bkgcat != 60) return b_truept;
        return b_pt;
    };
    MessageSvc::Info("Starting histogram attachment. ");
    if(_l0weights_CL_L0I.size() != 0 ){    
        //=========================
        // L0I maps attachment 
        //=========================
        TString _inputVarX     = "{HEAD}_PT"; _inputVarX.ReplaceAll("{HEAD}", _head);        
        TString _inputVarY     = "1.*nTracks";        
        TString _bkgcat        = "{HEAD}_BKGCAT"; _bkgcat.ReplaceAll("{HEAD}", _head);
        TString _inputVarXTRUE = "{HEAD}_TRUEPT"; _inputVarXTRUE.ReplaceAll("{HEAD}", _head);
        lastNode = lastNode.Define("wL0_L0I_Input_VarY", _inputVarY.Data()) ;        
        if( _configHolder.GetAna() == Analysis::EE){ 
            //On EE( for BKGCAT60, the B_TRUEPT --> B_TRUEPT for L0I Maps when ported from muons [baseline])
            //On EE( for Any BKGCAT, use B_PT for L0I Maps when NOT ported from muons [Systematic, linked to logic in WeightHolder.cpp])
            if( !SettingDef::Weight::L0I_EToE){
                MessageSvc::Warning("ElectronMode (L0I), L0I(mu) -> L0I(e) , use TRUE_PT for non BKGCAT60 , use _PT for CAT60");
                lastNode = lastNode.Define("wL0_L0I_Input_VarX", _inputPT_usage, { _inputVarX.Data(), _inputVarXTRUE.Data(), _bkgcat.Data()} );
            }else{
                MessageSvc::Warning("ElectronMode (L0I), L0I(e) -> L0I(e) , use B_PT reconstructed");
                lastNode = lastNode.Define("wL0_L0I_Input_VarX", _inputVarX.Data()); 
            }
        }else{
            lastNode = lastNode.Define("wL0_L0I_Input_VarX", _inputVarX.Data()  );
        }
        lastNode =  ApplyWeights2D( lastNode, _l0weights_CL_L0I, "wL0_L0I_Input_VarX", "wL0_L0I_Input_VarY");
        lastNode =  ApplyWeights2D( lastNode, _l0weights_MC_L0I, "wL0_L0I_Input_VarX", "wL0_L0I_Input_VarY");

        if (SettingDef::Weight::useBS) {
            lastNode =  ApplyBSWeights2D( lastNode, _BS_l0weights_CL_L0I, "wL0_L0I_Input_VarX", "wL0_L0I_Input_VarY");
            lastNode =  ApplyBSWeights2D( lastNode, _BS_l0weights_MC_L0I, "wL0_L0I_Input_VarX", "wL0_L0I_Input_VarY");
        }
    }
    if( _l0weights_CL_L0L_Comb_L1.size()!=0){
        //=========================
        // L0L Comb maps attachment 
        //=========================
        if( _configHolder.GetAna() == Analysis::EE){                        
            lastNode = lastNode.Define("L1_wL0_L0LComb_Input_VarX", "E1_L0Calo_ECAL_realET");
            lastNode = lastNode.Define("L1_wL0_L0LComb_Input_VarY", "1.*E1_L0Calo_ECAL_region");
            lastNode = lastNode.Define("L2_wL0_L0LComb_Input_VarX", "E2_L0Calo_ECAL_realET");
            lastNode = lastNode.Define("L2_wL0_L0LComb_Input_VarY", "1.*E2_L0Calo_ECAL_region");         
        }else if( _configHolder.GetAna() == Analysis::MM){
            lastNode = lastNode.Define("L1_wL0_L0LComb_Input_VarX", "M1_PT");
            lastNode = lastNode.Define("L1_wL0_L0LComb_Input_VarY", "M1_ETA");
            lastNode = lastNode.Define("L2_wL0_L0LComb_Input_VarX", "M2_PT");
            lastNode = lastNode.Define("L2_wL0_L0LComb_Input_VarY", "M2_ETA");
        }

        lastNode = ApplyWeights2D( lastNode, _l0weights_CL_L0L_Comb_L1, "L1_wL0_L0LComb_Input_VarX", "L1_wL0_L0LComb_Input_VarY");
        lastNode = ApplyWeights2D( lastNode, _l0weights_MC_L0L_Comb_L1, "L1_wL0_L0LComb_Input_VarX", "L1_wL0_L0LComb_Input_VarY");
        lastNode = ApplyWeights2D( lastNode, _l0weights_CL_L0L_Comb_L2, "L2_wL0_L0LComb_Input_VarX", "L2_wL0_L0LComb_Input_VarY");
        lastNode = ApplyWeights2D( lastNode, _l0weights_MC_L0L_Comb_L2, "L2_wL0_L0LComb_Input_VarX", "L2_wL0_L0LComb_Input_VarY");
        if (SettingDef::Weight::useBS) {
            lastNode = ApplyBSWeights2D( lastNode, _BS_l0weights_CL_L0L_Comb_L1, "L1_wL0_L0LComb_Input_VarX", "L1_wL0_L0LComb_Input_VarY");
            lastNode = ApplyBSWeights2D( lastNode, _BS_l0weights_MC_L0L_Comb_L1, "L1_wL0_L0LComb_Input_VarX", "L1_wL0_L0LComb_Input_VarY");
            lastNode = ApplyBSWeights2D( lastNode, _BS_l0weights_CL_L0L_Comb_L2, "L2_wL0_L0LComb_Input_VarX", "L2_wL0_L0LComb_Input_VarY");
            lastNode = ApplyBSWeights2D( lastNode, _BS_l0weights_MC_L0L_Comb_L2, "L2_wL0_L0LComb_Input_VarX", "L2_wL0_L0LComb_Input_VarY");
        }
    }    
    if( _l0weights_CL_Dist_L1.size() !=0 ){
        //=====================================
        // L0L ECAL distance (L1,L2) maps attachment 
        //=====================================        
        if( _configHolder.GetAna() == Analysis::EE){                        
            lastNode = lastNode.Define("L1_wL0_L0L_Input_VarX", "E1_L0Calo_ECAL_realET");
            lastNode = lastNode.Define("L1_wL0_L0L_Input_VarY", "TMath::Sqrt(TMath::Sq(E1_L0Calo_ECAL_xProjection - E2_L0Calo_ECAL_xProjection) + TMath::Sq(E1_L0Calo_ECAL_yProjection - E2_L0Calo_ECAL_yProjection))");
            lastNode = lastNode.Define("L2_wL0_L0L_Input_VarX", "E2_L0Calo_ECAL_realET");
            lastNode = lastNode.Define("L2_wL0_L0L_Input_VarY", "TMath::Sqrt(TMath::Sq(E1_L0Calo_ECAL_xProjection - E2_L0Calo_ECAL_xProjection) + TMath::Sq(E1_L0Calo_ECAL_yProjection - E2_L0Calo_ECAL_yProjection))");         
        }else if( _configHolder.GetAna() == Analysis::MM){
            MessageSvc::Error("L0L ECAL distance isnt defined for muons", "","EXIT_FAILURE");
        }
        lastNode = ApplyWeights2D( lastNode, _l0weights_CL_Dist_L1, "L1_wL0_L0L_Input_VarX", "L1_wL0_L0L_Input_VarY");
        lastNode = ApplyWeights2D( lastNode, _l0weights_MC_Dist_L1, "L1_wL0_L0L_Input_VarX", "L1_wL0_L0L_Input_VarY");
        lastNode = ApplyWeights2D( lastNode, _l0weights_CL_Dist_L2, "L2_wL0_L0L_Input_VarX", "L2_wL0_L0L_Input_VarY");
        lastNode = ApplyWeights2D( lastNode, _l0weights_MC_Dist_L2, "L2_wL0_L0L_Input_VarX", "L2_wL0_L0L_Input_VarY");

        if (SettingDef::Weight::useBS) {
            lastNode = ApplyBSWeights2D( lastNode, _BS_l0weights_CL_Dist_L1, "L1_wL0_L0L_Input_VarX", "L1_wL0_L0L_Input_VarY");
            lastNode = ApplyBSWeights2D( lastNode, _BS_l0weights_MC_Dist_L1, "L1_wL0_L0L_Input_VarX", "L1_wL0_L0L_Input_VarY");
            lastNode = ApplyBSWeights2D( lastNode, _BS_l0weights_CL_Dist_L2, "L2_wL0_L0L_Input_VarX", "L2_wL0_L0L_Input_VarY");
            lastNode = ApplyBSWeights2D( lastNode, _BS_l0weights_MC_Dist_L2, "L2_wL0_L0L_Input_VarX", "L2_wL0_L0L_Input_VarY");        
        }
    }
    
    if( _l0weights_CL_L1.size() !=0 ){
        //=====================================
        // L0L (L1,L2) maps attachment 
        //=====================================        
        if( _configHolder.GetAna() == Analysis::EE){                        
            lastNode = lastNode.Define("L1_wL0_L0L_Input_VarX", "E1_L0Calo_ECAL_realET");
            lastNode = lastNode.Define("L1_wL0_L0L_Input_VarY", "1.*E1_L0Calo_ECAL_region");
            lastNode = lastNode.Define("L2_wL0_L0L_Input_VarX", "E2_L0Calo_ECAL_realET");
            lastNode = lastNode.Define("L2_wL0_L0L_Input_VarY", "1.*E2_L0Calo_ECAL_region");         
        }else if( _configHolder.GetAna() == Analysis::MM){
            lastNode = lastNode.Define("L1_wL0_L0L_Input_VarX", "M1_PT");
            lastNode = lastNode.Define("L1_wL0_L0L_Input_VarY", "M1_ETA");
            lastNode = lastNode.Define("L2_wL0_L0L_Input_VarX", "M2_PT");
            lastNode = lastNode.Define("L2_wL0_L0L_Input_VarY", "M2_ETA");
        }
        lastNode = ApplyWeights2D( lastNode, _l0weights_CL_L1, "L1_wL0_L0L_Input_VarX", "L1_wL0_L0L_Input_VarY");
        lastNode = ApplyWeights2D( lastNode, _l0weights_MC_L1, "L1_wL0_L0L_Input_VarX", "L1_wL0_L0L_Input_VarY");
        lastNode = ApplyWeights2D( lastNode, _l0weights_CL_L2, "L2_wL0_L0L_Input_VarX", "L2_wL0_L0L_Input_VarY");
        lastNode = ApplyWeights2D( lastNode, _l0weights_MC_L2, "L2_wL0_L0L_Input_VarX", "L2_wL0_L0L_Input_VarY");

        if (SettingDef::Weight::useBS) {
            lastNode = ApplyBSWeights2D( lastNode, _BS_l0weights_CL_L1, "L1_wL0_L0L_Input_VarX", "L1_wL0_L0L_Input_VarY");
            lastNode = ApplyBSWeights2D( lastNode, _BS_l0weights_MC_L1, "L1_wL0_L0L_Input_VarX", "L1_wL0_L0L_Input_VarY");
            lastNode = ApplyBSWeights2D( lastNode, _BS_l0weights_CL_L2, "L2_wL0_L0L_Input_VarX", "L2_wL0_L0L_Input_VarY");
            lastNode = ApplyBSWeights2D( lastNode, _BS_l0weights_MC_L2, "L2_wL0_L0L_Input_VarX", "L2_wL0_L0L_Input_VarY");        
        }

        //Only EE mode / the L0L with Brem-splitting support
        if( _configHolder.GetAna() == Analysis::EE  && _weightOption.Contains("BREM")  ){
            lastNode = ApplyWeights2D_3Args( lastNode, _l0weights_CL_L1_BremSplit, "L1_wL0_L0L_Input_VarX", "L1_wL0_L0L_Input_VarY", "E1_HasBremAdded");
            lastNode = ApplyWeights2D_3Args( lastNode, _l0weights_MC_L1_BremSplit, "L1_wL0_L0L_Input_VarX", "L1_wL0_L0L_Input_VarY", "E1_HasBremAdded");
            lastNode = ApplyWeights2D_3Args( lastNode, _l0weights_CL_L2_BremSplit, "L2_wL0_L0L_Input_VarX", "L2_wL0_L0L_Input_VarY", "E2_HasBremAdded");
            lastNode = ApplyWeights2D_3Args( lastNode, _l0weights_MC_L2_BremSplit, "L2_wL0_L0L_Input_VarX", "L2_wL0_L0L_Input_VarY", "E2_HasBremAdded");
        }
    }

    if(_l0weights_CL_L0H.size() != 0 ){
        //=========================
        // L0I maps attachment 
        //=========================
        TString _inputVarX = "{HH}_PT"; _inputVarX.ReplaceAll("{HH}", _hadR);
        TString _inputVarY = "1.*({H1}_L0Calo_HCAL_region+{H2}_L0Calo_HCAL_region)"; _inputVarY.ReplaceAll("{H1}", _had1).ReplaceAll("{H2}", _had2);
        if( _configHolder.GetProject() == Prj::RK){ 
            _inputVarX = "K_L0Calo_HCAL_realET";
            _inputVarY = "1.*({H1}_L0Calo_HCAL_region)"; _inputVarY.ReplaceAll("{H1}", _had1);
        }
        lastNode = lastNode.Define("wL0_L0H_Input_VarX", _inputVarX.Data());
        lastNode = lastNode.Define("wL0_L0H_Input_VarY", _inputVarY.Data());        
        lastNode = ApplyWeights2D( lastNode, _l0weights_CL_L0H, "wL0_L0H_Input_VarX", "wL0_L0H_Input_VarY");
        lastNode = ApplyWeights2D( lastNode, _l0weights_MC_L0H, "wL0_L0H_Input_VarX", "wL0_L0H_Input_VarY");
        if( SettingDef::Weight::useBS){
            lastNode = ApplyBSWeights2D( lastNode, _BS_l0weights_CL_L0H, "wL0_L0H_Input_VarX", "wL0_L0H_Input_VarY");
            lastNode = ApplyBSWeights2D( lastNode, _BS_l0weights_MC_L0H, "wL0_L0H_Input_VarX", "wL0_L0H_Input_VarY");
        }

    }
    return lastNode; 
}

RNode HelperProcessing::AttachWeights(RNode df,  ConfigHolder & _configHolder, TString _weightOption) {
    if( SettingDef::Efficiency::option.Contains("OnTheFly")) {
        MessageSvc::Line();
        if(_weightOption.Contains("BS")){
            MessageSvc::Warning("Appending Poisson bootstrapping branches on the fly");
            df = AppendBSColumns(df);
        }
        if (_configHolder.IsMC()) {
            auto this_analysis = _configHolder.GetAna();
            vector<string> _wildCards = {};
            MessageSvc::Line();
            if( _weightOption.Contains("PID")){
                MessageSvc::Warning("Appending PID on the fly for effiency computation");
                df = AppendPIDColumns(df, _configHolder ,_wildCards);
            }
            if( this_analysis == Analysis::EE && _weightOption.Contains("TRK")){
                MessageSvc::Warning("Appending TRK on the fly for effiency computation");
                df = AppendTRKColumns(df, _configHolder);
            }
            if( _weightOption.Contains("RW1D")){
                MessageSvc::Warning("Appending RW1D on the fly for effiency computation");
                df = AppendRW1DColumns(df, _configHolder, _weightOption);
            }
            if( _weightOption.Contains("L0")){
                MessageSvc::Warning("Appending L0 on the fly for effiency computation");
                df = AppendL0Columns(df, _configHolder, _weightOption);
            }
            if( _weightOption.Contains("HLT")){
                MessageSvc::Warning("Appending HLT on the fly for effiency computation");
                df = AppendHLTColumns(df, _configHolder, _weightOption);
            }
            if( _weightOption.Contains("MODEL") && _configHolder.IsSignalMC() ){
                MessageSvc::Warning("Appending MODEL weights on the fly for effiency computation");
                df = AppendDecModelWeights(df);
            }else{
                MessageSvc::Info("MODEL weight not appended not a valid sample");
            }
        }
        MessageSvc::Line();
    }

    return df;
}


RNode HelperProcessing::AttachWeights(RNode df,  const ConfigHolder & _configHolder, TString _weightOption) {
    auto _nonConstConfigHolder( _configHolder);
    _nonConstConfigHolder.Init();
    return AttachWeights( df, _nonConstConfigHolder, _weightOption);
}


void HelperProcessing::PortTrueBMass( EventType & et){
    MessageSvc::Line();
    MessageSvc::Info("PortTrueBMass");
    MessageSvc::Line();
    if( !et.PortingEnabled()){
        MessageSvc::Warning("PortTrueBMass", TString("Nothing to do, Porting is not enabled for this EventType"));
        return;
    }
    bool _SIGNALMC_ = et.IsSignalMC();
    const TString _TRUEB_MASS_FINAL_        = TruePort::B_TRUEM_PREFSR_PORT.Data();
    const TString _TRUEB_MASSPOSTFSR_FINAL_ = TruePort::B_TRUEM_POSTFSR_PORT.Data();
    
    ROOT::DisableImplicitMT();
    //Check that the job has not yet been done...
    ROOT::RDataFrame df("DecayTuple", "TupleProcess.root");
    auto lastNode = RNode(df);
    if(lastNode.HasColumn( TruePort::B_TRUEM_PREFSR_PORT.Data())){
        MessageSvc::Info( "True BMass porting already done, skipping");
        return;
    }
    bool _resnapMCDT = false;
    ROOT::RDataFrame dfMC("MCDecayTuple", "TupleProcess.root");
    auto last_node_MCT = RNode(dfMC);

    //We have to port from MCDT to DT for the "signal MC"
    //B_TRUEM_PORT         = Mass from B_TRUEKIN
    //B_TRUEM_POSTFSR_PORT = Mass from Sum of RECONSTRUCTED Final states
    //Additionally for background modes (B_TRUEM_PR_PORT), in signal they will be the same? 
    //B_TRUEM_PR_PORT      = Mass from B_TRUEKIN - MISSING(TRUEKIN)

    TString _inputToPort_preFRS  = TruePort::B_TRUEM_PREFSR_PORT.Data();         //from B_TRUE_KINEMATICS
    TString _inputToPort_postFSR = TruePort::B_TRUEM_POSTFSR_PORT.Data();        //from K + E + E
    TString _inputToPort_PR      = TruePort::B_TRUEM_PREFSR_PR_PORT.Data();      //from B_TRUE_KINEMATICS - OTHER PARTICLE

    //====================================================================================//
    // Special treatment of background samples 
    //====================================================================================//
    map< Prj, vector< TString > > _bkgSamples_porting{ { Prj::RK , {"Bd2KPiEE",  "Bu2KstEE", "Bu2KPiEE", "Bd2KstEE", "Bu2KEtaPrimeGEE", "Bd2KstEtaGEE" }},
                                                       { Prj::RKst,{"Bu2KPiPiEE"}},
                                                       { Prj::RPhi,{}} };
    //return the vector input needed to create the TLorentzVectors
    auto _TRUEKINLABELS_ =[]( TString _labelParticle){
        vector< std::string > _truekinlabels{};
        for( TString _s : {"TRUEP_X", "TRUEP_Y", "TRUEP_Z", "TRUEP_E"}){
            TString col = _labelParticle+"_"+_s;
            _truekinlabels.push_back(col.Data() );
        }
        return _truekinlabels;
    };


    vector<std::string> _HEAD_TRUEKIN_; 
    if( et.GetSample().BeginsWith("Bu")) _HEAD_TRUEKIN_ = _TRUEKINLABELS_("Bp");
    if( et.GetSample().BeginsWith("Bd")) _HEAD_TRUEKIN_ = _TRUEKINLABELS_("B0");
    if( et.GetSample().BeginsWith("Bs")) _HEAD_TRUEKIN_ = _TRUEKINLABELS_("Bs");
    
    if( et.GetSample().Contains("Bd2KstGEE")){
      last_node_MCT = last_node_MCT.Define("tmp_HEAD_LV",      Functors::make_lorentz, _HEAD_TRUEKIN_)
	.Define("tmp_G_LV", Functors::make_lorentz, _TRUEKINLABELS_("G"))
        .Define("B_TRUEM",          Functors::getmass_1Body, {"tmp_HEAD_LV"} )                                                                                                                             
        .Define("B_TRUEM_MCDT",     Functors::ConstructIt, {"runNumber", "eventNumber", "B_TRUEM"} );      
    }else{
      //Common operations to all projects and samples
      last_node_MCT = last_node_MCT.Define("tmp_HEAD_LV",      Functors::make_lorentz, _HEAD_TRUEKIN_)
	.Define("tmp_L1_LV",        Functors::make_lorentz, _TRUEKINLABELS_("E1"))
	.Define("tmp_L2_LV",        Functors::make_lorentz, _TRUEKINLABELS_("E2"))
	.Define("B_TRUEM",          Functors::getmass_1Body, {"tmp_HEAD_LV"} )
	.Define("B_TRUEM_MCDT",     Functors::ConstructIt, {"runNumber", "eventNumber", "B_TRUEM"} );
    }
    
    if( CheckVectorContains(_bkgSamples_porting.at( et.GetProject()),et.GetSample()) || et.IsSignalMC() ){        
        MessageSvc::Warning(fmt::format("Defining true B mass on tuple for {0}!",et.GetSample().Data()));
        std::map< TString , vector< std::string >  > _MISSINGPARTICLEINPUTS_;
        switch( et.GetProject()){
            case Prj::RK  : _MISSINGPARTICLEINPUTS_ = { { "Bu2KEtaPrimeGEE",  _TRUEKINLABELS_("Gamma")}, 
                                                        { "Bu2KstEE",         _TRUEKINLABELS_("Pi")}, 
                                                        { "Bd2KstEE",         _TRUEKINLABELS_("Pi")}, 
                                                        { "Bu2KPiEE",         _TRUEKINLABELS_("Pi")},
                                                        { "Bd2KPiEE",         _TRUEKINLABELS_("Pi")},
                                                        { "Bd2KstEtaGEE",     _TRUEKINLABELS_("Gamma")} }; break;
            case Prj::RKst : _MISSINGPARTICLEINPUTS_= { { "Bu2KPiPiEE",       _TRUEKINLABELS_("Pi1")}};  break; 
            default : MessageSvc::Warning("Switch not implemented for this project in the portTrueQ2 routine");
        }
        if( !_SIGNALMC_){
            //it's among the part-recoed samples, assuming 1 missing particle ONLY, the 2 missing particle case to be implemented...
            last_node_MCT = last_node_MCT.Define("tmp_MISSING_LV",   Functors::make_lorentz, _MISSINGPARTICLEINPUTS_.at( et.GetSample()) );
        }
	
        //== Define TRUEB mass to extract from MCDT porting for RK samples
        if( et.GetProject() == Prj::RK){
            if(!_SIGNALMC_){
                last_node_MCT = last_node_MCT.Define("tmp_H1_LV",           Functors::make_lorentz,  _TRUEKINLABELS_("K"))
                                             .Define("B_TRUEM_POSTFSR",     Functors::getmass_4Body,  {"tmp_L1_LV", "tmp_L2_LV", "tmp_H1_LV", "tmp_MISSING_LV"})
                                             .Define("B_TRUEM_POSTFSR_MCDT",Functors::ConstructIt,    {"runNumber", "eventNumber", "B_TRUEM_POSTFSR"})
                                             .Define("B_TRUEM_PR",          Functors::getMassDelta,   {"tmp_HEAD_LV", "tmp_MISSING_LV"})
                                             .Define("B_TRUEM_PR_MCDT",     Functors::ConstructIt,    {"runNumber", "eventNumber", "B_TRUEM_PR"} );            
            }else{
                last_node_MCT = last_node_MCT.Define("tmp_H1_LV",           Functors::make_lorentz,  _TRUEKINLABELS_("K"))
                                             .Define("B_TRUEM_POSTFSR",     Functors::getmass_3Body,  {"tmp_L1_LV", "tmp_L2_LV", "tmp_H1_LV"})
                                             .Define("B_TRUEM_POSTFSR_MCDT",Functors::ConstructIt,    {"runNumber", "eventNumber", "B_TRUEM_POSTFSR"});
            }
        }
        //== Define TRUEB mass to extract from MCDT porting for RK samples
        if( et.GetProject() == Prj::RKst){
            if(!_SIGNALMC_){
                last_node_MCT = last_node_MCT.Define("tmp_H1_LV",           Functors::make_lorentz,  _TRUEKINLABELS_("K"))
                                             .Define("tmp_H2_LV",           Functors::make_lorentz,  _TRUEKINLABELS_("Pi2"))  //Pi2 because of Bu2KPiPiEE sample dvDictionaries.py!                                         
                                             .Define("B_TRUEM_POSTFSR",     Functors::getmass_5Body,  {"tmp_L1_LV", "tmp_L2_LV", "tmp_H1_LV", "tmp_H2_LV", "tmp_MISSING_LV"})
                                             .Define("B_TRUEM_POSTFSR_MCDT",Functors::ConstructIt,    {"runNumber", "eventNumber", "B_TRUEM_POSTFSR"})
                                             .Define("B_TRUEM_PR",          Functors::getMassDelta,   {"tmp_HEAD_LV", "tmp_MISSING_LV"})
                                             .Define("B_TRUEM_PR_MCDT",     Functors::ConstructIt,    {"runNumber", "eventNumber", "B_TRUEM_PR"} );
	    }else if( et.GetSample().Contains("Bd2KstGEE")){
		last_node_MCT = last_node_MCT.Define("tmp_H1_LV",Functors::make_lorentz,  _TRUEKINLABELS_("K"))
		                             .Define("tmp_H2_LV",Functors::make_lorentz,  _TRUEKINLABELS_("Pi"))
		                             .Define("B_TRUEM_POSTFSR",Functors::getmass_3Body,  {"tmp_G_LV", "tmp_H1_LV", "tmp_H2_LV"})
		                             .Define("B_TRUEM_POSTFSR_MCDT",Functors::ConstructIt,    {"runNumber", "eventNumber", "B_TRUEM_POSTFSR"});	   
            }else{
                last_node_MCT = last_node_MCT.Define("tmp_H1_LV",           Functors::make_lorentz,  _TRUEKINLABELS_("K"))
                                             .Define("tmp_H2_LV",           Functors::make_lorentz,  _TRUEKINLABELS_("Pi"))
                                             .Define("B_TRUEM_POSTFSR",     Functors::getmass_4Body,  {"tmp_L1_LV", "tmp_L2_LV", "tmp_H1_LV", "tmp_H2_LV"})
                                             .Define("B_TRUEM_POSTFSR_MCDT",Functors::ConstructIt,    {"runNumber", "eventNumber", "B_TRUEM_POSTFSR"});
            }
        }
        //== Define TRUEB mass to extract from MCDT porting for RK samples
        if( et.GetProject() == Prj::RPhi){
            if(!_SIGNALMC_){
                //NOT IMPLEMENTED 
                MessageSvc::Warning("Porting B mass for RPhi-Samples background from MCDT not implemented. do it", (TString)"","EXIT_FAILURE");
            }else{
                last_node_MCT = last_node_MCT.Define("tmp_H1_LV",           Functors::make_lorentz,  _TRUEKINLABELS_("K1"))
                                             .Define("tmp_H2_LV",           Functors::make_lorentz,  _TRUEKINLABELS_("K2")) //Pi2 because of Bu2KPiPiEE sample dvDictionaries.py! How is MCDT done here??? Probably this part of the code will not work out of the box.
                                             .Define("B_TRUEM_POSTFSR",     Functors::getmass_4Body,  {"tmp_L1_LV", "tmp_L2_LV", "tmp_H1_LV", "tmp_H2_LV"})
                                             .Define("B_TRUEM_POSTFSR_MCDT",Functors::ConstructIt,    {"runNumber", "eventNumber", "B_TRUEM_POSTFSR"});
            }
        }
        if( !_SIGNALMC_) _resnapMCDT = true;
    }
    auto port_BMassTrue          = last_node_MCT.Take<BranchPort>( "B_TRUEM_MCDT");
    auto port_BMassTrue_POSTFSR  = last_node_MCT.Take<BranchPort>( "B_TRUEM_POSTFSR_MCDT");
    
    decltype(port_BMassTrue) port_BMassTrue_PR;
    if(!_SIGNALMC_){
        port_BMassTrue_PR = last_node_MCT.Take<BranchPort>( "B_TRUEM_PR_MCDT");
    }   
    MessageSvc::Info("DOING B Mass PORTING, MCDT loop");
    *last_node_MCT.Count();
    MessageSvc::Info("DOING B Mass PORTING, MCDT loop done");
    if( _resnapMCDT){
        ROOT::RDF::RSnapshotOptions _options;
        _options.fLazy = false;        // start the event loop when actually triggered
        _options.fMode = "RECREATE";   // recreate the tuple and the TFile
        //second event loop triggered, maybe the NODE oprations
        //Do it lazy, so Take and snapshot runs in parallel
        MessageSvc::Info("DOING B Mass PORTING, MCDT Resnapshot");
        last_node_MCT.Snapshot("MCDecayTuple", "TupleProcess_tmpMCDT.root", DropColumnsWildcard(last_node_MCT.GetColumnNames(), {"tmp_", "_MCDT"}), _options);
        MessageSvc::Info("DOING B Mass PORTING, MCDT Resnapshot");
    }

    using map_value    = map< pair< UInt_t , ULong64_t> , double >;
    map_value port_Bmass_true;
    map_value port_Bmass_true_POSTFSR;
    map_value port_Bmass_true_PR;
    MessageSvc::Info("Handling port_BMassTrue");
    std::cout<< " Loop over "<<  port_BMassTrue->size() << " elements" << std::endl;
    for( int i =0 ; i < port_BMassTrue->size(); ++i){
        if( i==0){
            std::cout<< "port_BMassTrue"<< std::endl;
        }
        port_Bmass_true[port_BMassTrue->at(i).KeyID()] = port_BMassTrue->at(i).BranchVal();
        if( i==0){
            std::cout<< "port_Bmass_true_POSTFSR"<< std::endl;
        }
        port_Bmass_true_POSTFSR[port_BMassTrue_POSTFSR->at(i).KeyID()] = port_BMassTrue_POSTFSR->at(i).BranchVal();
        if( !_SIGNALMC_ ){
            if( i==0){
                std::cout<< "port_BMassTrue_PR (!SignalMC case)"<< std::endl;
            }                   
            port_Bmass_true_PR[port_BMassTrue_PR->at(i).KeyID()] = port_BMassTrue_PR->at(i).BranchVal();
        }
    }
    
    /* MAIN LAMBDA FUNCTION USING EXTERNAL MAP TO READ AND PORT VIA RunNb,EventNb */
    auto portFromMCDT = [&port_Bmass_true](  const UInt_t & rnNb, const ULong64_t & eNb ){
        if(port_Bmass_true.find( make_pair(rnNb, eNb) ) == port_Bmass_true.end()){
            //this event has not the rnNb, eNb matching to MCDT ( very rare but it (can) happen)
            return -1.;
        }
        return port_Bmass_true.at(make_pair(rnNb, eNb));
    };

    //Lamda function to attach POSTFSR porting
    auto portFromMCDT_POSTFSR = [&port_Bmass_true_POSTFSR](  const UInt_t & rnNb, const ULong64_t & eNb ){
        if(port_Bmass_true_POSTFSR.find( make_pair(rnNb, eNb) ) == port_Bmass_true_POSTFSR.end()){
            //this event has not the rnNb, eNb matching to MCDT ( very rare but it (can) happen)
            return -1.;
        }
        return port_Bmass_true_POSTFSR.at(make_pair(rnNb, eNb));
    };
   //Lamda function to attach PR porting
    auto portFromMCDT_PR = [&port_Bmass_true_PR](  const UInt_t & rnNb, const ULong64_t & eNb ){
        if(port_Bmass_true_PR.find( make_pair(rnNb, eNb) ) == port_Bmass_true_PR.end()){
            //this event has not the rnNb, eNb matching to MCDT ( very rare but it (can) happen)
            return -1.;
        }
        return port_Bmass_true_PR.at(make_pair(rnNb, eNb));
    };
    
    MessageSvc::Info(fmt::format("Define({})", TruePort::B_TRUEM_PREFSR_PORT.Data()));
    MessageSvc::Info(fmt::format("Define({})", TruePort::B_TRUEM_POSTFSR_PORT.Data()));
    lastNode = lastNode.Define( TruePort::B_TRUEM_PREFSR_PORT.Data(), portFromMCDT ,        {"runNumber", "eventNumber"})
                       .Define( TruePort::B_TRUEM_POSTFSR_PORT.Data(),portFromMCDT_POSTFSR ,{"runNumber", "eventNumber"});

    if(!_SIGNALMC_){
        lastNode = lastNode.Define( TruePort::B_TRUEM_PREFSR_PR_PORT.Data(),portFromMCDT_PR ,{"runNumber", "eventNumber"});
    }
    
    //=====================================
    // Dump tuple to disk
    //=====================================       
    MessageSvc::Info("PortTrueBMass [no SMEAR], Snapshotting");
    ROOT::RDF::RSnapshotOptions _options;
    _options.fLazy = false;        // start the event loop when actually triggered
    _options.fMode = "RECREATE";   // recreate the tuple and the TFile
    //second event loop triggered, maybe the NODE oprations
    //Do it lazy, so Take and snapshot runs in parallel    
    lastNode.Snapshot("DecayTuple", "TupleProcess_tmp.root", DropColumnsWildcard( lastNode.GetColumnNames(), {"BMASS_VALID"}), _options);
    MessageSvc::Info("PortTrueBMass [no SMEAR], Snapshotting done");
    //=====================================
    // Shuffle tuple at exit (entries consistenty)
    //=====================================  
    TFile f1("TupleProcess.root","READ");
    auto  original =  (TTree*)f1.Get("DecayTuple");
    auto  entries = original->GetEntries();
    TFile f2("TupleProcess_tmp.root", "READ");
    auto updated = (TTree*)f2.Get("DecayTuple");
    if( original->GetEntries() != updated->GetEntries()) MessageSvc::Error("Severe error on Snapshots", "","EXIT_FAILURE");
    else{ f1.Close(); f2.Close(); }
    if(!_resnapMCDT){
        MessageSvc::Info("rootmv TupleProcess.root:MCDecayTuple TupleProcess_tmp.root");    
        moveTupleFromTo( "MCDecayTuple", "TupleProcess.root", "TupleProcess_tmp.root");
    }else{
        MessageSvc::Info("rootmv TupleProcess_tmpMCDT.root:MCDecayTuple TupleProcess_tmp.root");    
        moveTupleFromTo( "MCDecayTuple", "TupleProcess_tmpMCDT.root", "TupleProcess_tmp.root");        
        IOSvc::runCommand(TString("rm TupleProcess_tmpMCDT.root"));
    }    
    MessageSvc::Info("rm TupleProcess.root"); 
    IOSvc::runCommand(TString("rm TupleProcess.root"));
    MessageSvc::Info("mv TupleProcess_tmp.root TupleProcess.root");
    IOSvc::runCommand(TString("mv TupleProcess_tmp.root TupleProcess.root"));
    TFile finalFile("TupleProcess.root","READ");
    auto finalTree = (TTree*)finalFile.Get("DecayTuple");
    if( finalTree == nullptr) MessageSvc::Error("Failure PortTrueBMass adder","","EXIT_FAILURE");
    if( finalTree->GetEntries() != entries) MessageSvc::Error("Failure PortTrueBMass adder", "","EXIT_FAILURE");    
    MessageSvc::Info("Closing file");
    finalFile.Close();
    return;    
}

void HelperProcessing::PortTrueKstMass( EventType & et){
    bool _doAddit =  et.IsCrossFeedSample() && et.HasMCDecayTuple() ; 
    if(!_doAddit){
        MessageSvc::Warning("PortTrueKstMass", (TString)"Sample not supported, return");
        return ;
    }
    ROOT::DisableImplicitMT();
    //Check that the job has not yet been done...
    ROOT::RDataFrame df("DecayTuple", "TupleProcess.root");
    RNode lastNode(df);
    if(df.HasColumn( "Kst_TRUE_M_FromMCDT")){
        MessageSvc::Info( "PortTrueKstMass", TString("Nothing to do, Kst_TRUE_M_FromMCDT already in tuples") ); 
        return;         
    };
    //Read up the MCDecayTuple and construct the Kst_TRUE_M branches
    ROOT::RDataFrame dfMC("MCDecayTuple", "TupleProcess.root");    
    auto last_node_MCT = dfMC.Define("XYZ", "1>0"); 
    if( dfMC.HasColumn( "Kst_TRUE_M")){     
        last_node_MCT  = dfMC.Define("Kst_TRUE_M"     ,        Functors::getmass,     {"Kst_TRUEP_X", "Kst_TRUEP_Y", "Kst_TRUEP_Z", "Kst_TRUEP_E"})
                             .Define("Kst_TRUE_M_Port",        Functors::ConstructIt, {"runNumber", "eventNumber",  "Kst_TRUE_M"} );
    }else{
        last_node_MCT  = last_node_MCT.Define("K_V",             Functors::make_lorentz, {"K_TRUEP_X", "K_TRUEP_Y", "K_TRUEP_Z", "K_TRUEP_E"})
                                      .Define("Pi_V",            Functors::make_lorentz, {"Pi_TRUEP_X", "Pi_TRUEP_Y", "Pi_TRUEP_Z", "Pi_TRUEP_E"})
                                      .Define("Kst_TRUE_M",      Functors::getmass_2Body,{"K_V","Pi_V"})
                                      .Define("Kst_TRUE_M_Port", Functors::ConstructIt,  {"runNumber", "eventNumber", "Kst_TRUE_M"} );
    }
    auto port_kstmass  = last_node_MCT.Take<BranchPort>( "Kst_TRUE_M_Port");
    using map_value    = map< pair< UInt_t , ULong64_t> , double >;
    map_value port_kstmass_true; 
    for( int i =0 ; i < port_kstmass->size(); ++i){
        port_kstmass_true[port_kstmass->at(i).KeyID()] = port_kstmass->at(i).BranchVal();
    }
    //Lambda function to attach ! ( run single thread only!)
    auto portFromMCDT = [&port_kstmass_true](  const UInt_t & rnNb, const ULong64_t & eNb ){
        if(port_kstmass_true.find( make_pair(rnNb, eNb) ) == port_kstmass_true.end()){
            //this event has not the rnNb, eNb matching to MCDT ( very rare but it (can) happen)
            return -1.;
        }
        return port_kstmass_true.at(make_pair(rnNb, eNb));
    };
    lastNode = lastNode.Define( "Kst_TRUE_M_FromMCDT",portFromMCDT ,{"runNumber", "eventNumber"});
    //=====================================
    // Dump tuple to disk
    //=====================================       
    MessageSvc::Info("PortTrueKstMass, Snapshotting");
    lastNode.Snapshot("DecayTuple", "TupleProcess_tmp.root");
    MessageSvc::Info("PortTrueKstMass, Snapshotting done");
    //=====================================
    // Shuffle tuple at exit (entries consistenty)
    //=====================================  
    TFile f1("TupleProcess.root","READ");
    auto  original =  (TTree*)f1.Get("DecayTuple");
    auto  entries = original->GetEntries();
    TFile f2("TupleProcess_tmp.root", "READ");
    auto updated = (TTree*)f2.Get("DecayTuple");
    if( original->GetEntries() != updated->GetEntries()) MessageSvc::Error("Severe error on Snapshots", "","EXIT_FAILURE");
    else{ f1.Close(); f2.Close(); }
    
    MessageSvc::Info("rootmv TupleProcess.root:MCDecayTuple TupleProcess_tmp.root");    
    moveTupleFromTo( "MCDecayTuple", "TupleProcess.root", "TupleProcess_tmp.root");
    MessageSvc::Info("rm TupleProcess.root");
    IOSvc::runCommand(TString("rm TupleProcess.root"));
    MessageSvc::Info("mv TupleProcess_tmp.root TupleProcess.root");
    IOSvc::runCommand(TString("mv TupleProcess_tmp.root TupleProcess.root"));
    TFile finalFile("TupleProcess.root","READ");
    auto finalTree = (TTree*)finalFile.Get("DecayTuple");
    if( finalTree == nullptr) MessageSvc::Error("Failure PortTrueKstMass adder","","EXIT_FAILURE");
    if( finalTree->GetEntries() != entries) MessageSvc::Error("Failure PortTrueKstMass adder", "","EXIT_FAILURE");    
    MessageSvc::Info("Closing file");
    finalFile.Close();
    return;
}

void HelperProcessing::PortTrueQ2( EventType & et , bool _useET){
    if( !et.PortingEnabled()){
        MessageSvc::Warning("PortTrueQ2", TString("Nothing to do, Porting is not enabled for this EventType"));
        return;
    }
    ROOT::DisableImplicitMT();
    //Check that the job has not yet been done...
    ROOT::RDataFrame df("DecayTuple", "TupleProcess.root");
    if(df.HasColumn( TruePort::JPs_TRUEM_PREFSR.Data()) || df.HasColumn(TruePort::JPs_TRUEM_POSTFSR.Data())){
        MessageSvc::Info( "True Q2 porting already done, skipping"); 
        return;         
    };
    bool _resnapMCDT = false;
    ROOT::RDataFrame dfMC("MCDecayTuple", "TupleProcess.root");
    RNode last_node_MCT( dfMC);
    //it has been computed inside TupleProcess in MCDT, on signal MC all good, if something specific on MCDT has been done, one has to either redoing it or make it from scratch
    TString _inputToPort_preFSR  = "JPs_TRUEM"; 
    TString _inputToPort_postFSR = "JPs_TRUEM_POSTFSR"; 
    //====================================================================================//
    // Special treatment of background samples 
    //====================================================================================//
    map< Prj, vector< TString > > _bkgSamples_redoTrueQ2{ { Prj::RK , {"Bd2KPiEE",  "Bu2KstEE", "Bu2KPiEE", "Bd2KstEE", "Bu2KEtaPrimeGEE", "Bd2KstEtaGEE" }}, 
                                                          { Prj::RKst,{"Bu2KPiPiEE"}}, 
                                                          { Prj::RPhi,{}} };
    //return the vector input needed to create the TLorentzVectors
    auto _TRUEKINLABELS_ =[]( TString labelTuple){
        vector< std::string > _truekinlabels{};
        for( TString _s : {"TRUEP_X", "TRUEP_Y", "TRUEP_Z", "TRUEP_E"}){
            TString col = labelTuple+"_"+_s;
            _truekinlabels.push_back(col.Data() );
        }
        return _truekinlabels;
    }; 

    if( CheckVectorContains(_bkgSamples_redoTrueQ2.at( et.GetProject()),et.GetSample())){
        MessageSvc::Warning(fmt::format("Re-defining true q2 on tuple for {0}!",et.GetSample().Data()));        
        std::map< TString , vector< std::string >  > _MISSINGPARTICLEINPUTS_;             
        switch( et.GetProject()){
            case Prj::RK  : _MISSINGPARTICLEINPUTS_ = { { "Bu2KEtaPrimeGEE",  _TRUEKINLABELS_("Gamma")}, 
                                                        { "Bu2KstEE",         _TRUEKINLABELS_("Pi")}, 
                                                        { "Bd2KstEE",         _TRUEKINLABELS_("Pi")}, 
                                                        { "Bu2KPiEE",         _TRUEKINLABELS_("Pi")},
                                                        { "Bd2KPiEE",         _TRUEKINLABELS_("Pi")},
                                                        { "Bd2KstEtaPrimeGEE",_TRUEKINLABELS_("Gamma")} }; break;
            case Prj::RKst : _MISSINGPARTICLEINPUTS_= { { "Bu2KPiPiEE",  _TRUEKINLABELS_("Pi1")} }; break; 
            default : MessageSvc::Warning("Switch not implemented for this project in the portTrueQ2 routine");
        }
        vector<std::string> _HEAD_TRUEKIN_; 
        if( et.GetSample().BeginsWith("Bu")) _HEAD_TRUEKIN_ = _TRUEKINLABELS_("Bp");
        if( et.GetSample().BeginsWith("Bd")) _HEAD_TRUEKIN_ = _TRUEKINLABELS_("B0");
        if( et.GetSample().BeginsWith("Bs")) _HEAD_TRUEKIN_ = _TRUEKINLABELS_("Bs");
	//Common operations to all projects.
	last_node_MCT = last_node_MCT.Define("tmp_HEAD_LV",                Functors::make_lorentz,  _HEAD_TRUEKIN_)
	  .Define("tmp_L1_LV",                  Functors::make_lorentz,  _TRUEKINLABELS_("E1"))
	  .Define("tmp_L2_LV",                  Functors::make_lorentz,  _TRUEKINLABELS_("E2"))
	  .Define("JPs_TRUEM_POSTFSR_REPLACED", Functors::getmass_2Body , {"tmp_L1_LV" , "tmp_L2_LV"})
	  .Define("tmp_MISSING_LV",             Functors::make_lorentz, _MISSINGPARTICLEINPUTS_.at( et.GetSample()));
        if( et.GetProject() == Prj::RK){
            last_node_MCT = last_node_MCT.Define("tmp_H1_LV",               Functors::make_lorentz,  _TRUEKINLABELS_("K"))                        
                                         .Define("JPs_TRUEM_REPLACED",      Functors::getmass_HEAD_MINUS_H1_MISS , {"tmp_HEAD_LV" , "tmp_H1_LV", "tmp_MISSING_LV"});
        }
        if( et.GetProject() == Prj::RKst){
            last_node_MCT = last_node_MCT.Define("tmp_H1_LV",           Functors::make_lorentz,  _TRUEKINLABELS_("K"))
                                         .Define("tmp_H2_LV",           Functors::make_lorentz,  _TRUEKINLABELS_("Pi2")) //Pi2 because of Bu2KPiPiEE sample dvDictionaries.py!                                         
                                         .Define("JPs_TRUEM_REPLACED",  Functors::getmass_HEAD_MINUS_H1_H2_MISS , {"tmp_HEAD_LV" , "tmp_H1_LV", "tmp_H2_LV", "tmp_MISSING_LV"});
        }
        if( et.GetProject() == Prj::RPhi){
            last_node_MCT = last_node_MCT.Define("tmp_H1_LV",           Functors::make_lorentz,  _TRUEKINLABELS_("K1"))
                                         .Define("tmp_H2_LV",           Functors::make_lorentz,  _TRUEKINLABELS_("K2")) //TODO , verify RPhi logics.                                                
                                         .Define("JPs_TRUEM_REPLACED",  Functors::getmass_HEAD_MINUS_H1_H2_MISS , {"tmp_HEAD_LV" , "tmp_H1_LV", "tmp_H2_LV", "tmp_MISSING_LV"});
        }
        _inputToPort_preFSR  = "JPs_TRUEM_REPLACED";
        _inputToPort_postFSR = "JPs_TRUEM_POSTFSR_REPLACED";
        _resnapMCDT = true; 
    }
    //look up table multi dimensional ? 
    last_node_MCT = last_node_MCT.Define( TruePort::JPs_TRUEM_PREFSR.Data() , Functors::ConstructIt, {"runNumber", "eventNumber", _inputToPort_preFSR.Data()} )
                                 .Define( TruePort::JPs_TRUEM_POSTFSR.Data(), Functors::ConstructIt, {"runNumber", "eventNumber", _inputToPort_postFSR.Data()} );
                                     
    auto port_q2truem         = last_node_MCT.Take<BranchPort>( TruePort::JPs_TRUEM_PREFSR.Data());
    auto port_q2truem_postfsr = last_node_MCT.Take<BranchPort>( TruePort::JPs_TRUEM_POSTFSR.Data());
    MessageSvc::Info("DOING Q2SMEARING PORTING, MCDT loop");
    *last_node_MCT.Count();
    MessageSvc::Info("DOING Q2SMEARING PORTING, MCDT loop done");
    if( _resnapMCDT){
        ROOT::RDF::RSnapshotOptions _options;
        _options.fLazy = false;         // start the event loop when actually triggered
        _options.fMode = "RECREATE";   // recreate the tuple and the TFile
        //second event loop triggered, maybe the NODE oprations
        //Do it lazy, so Take and snapshot runs in parallel...
        MessageSvc::Info("DOING Q2SMEARING PORTING, MCDT Resnapshot");
        last_node_MCT.Snapshot("MCDecayTuple", 
                               "TupleProcess_tmpMCDT.root", 
                               DropColumnsWildcard(last_node_MCT.GetColumnNames(), 
                               {"tmp_", TruePort::JPs_TRUEM_PREFSR.Data(), TruePort::JPs_TRUEM_POSTFSR.Data()}), 
                               _options );
        MessageSvc::Info("DOING Q2SMEARING PORTING, MCDT Resnapshot");
    }

    using map_value    = map< pair< UInt_t , ULong64_t> , double >;
    map_value q2truem; 
    map_value q2truem_postfsr;
    for( int i =0 ; i < port_q2truem->size(); ++i){
        q2truem[                port_q2truem->at(i).KeyID()] = port_q2truem->at(i).BranchVal();
        q2truem_postfsr[port_q2truem_postfsr->at(i).KeyID()] = port_q2truem_postfsr->at(i).BranchVal();
    }
    //Lambda function to attach ! ( run single thread only!)
    auto portFromMCDT_JPS_TRUEM = [&q2truem](  const UInt_t & rnNb, const ULong64_t & eNb ){
        if(q2truem.find( make_pair(rnNb, eNb) ) == q2truem.end()){
            //this event has not the rnNb, eNb matching to MCDT ( very rare but it happens)
            return -1.;
        }
        return q2truem.at(make_pair(rnNb, eNb));
    };
    auto portFromMCDT_JPS_TRUEM_POSTFSR = [&q2truem_postfsr](  const UInt_t & rnNb, const ULong64_t & eNb ){
        if(q2truem_postfsr.find( make_pair(rnNb, eNb) ) == q2truem_postfsr.end()){
            //this event has not the rnNb, eNb matching to MCDT ( very rare but it happens)
            return -1.;
        }
        return q2truem_postfsr.at(make_pair(rnNb, eNb));
    };
    RNode lastNode = df.Define( TruePort::JPs_TRUEM_PREFSR.Data()         ,portFromMCDT_JPS_TRUEM        , {"runNumber", "eventNumber"} )
                       .Define( TruePort::JPs_TRUEM_POSTFSR.Data()        ,portFromMCDT_JPS_TRUEM_POSTFSR, {"runNumber", "eventNumber"} );                
    //=====================================
    // Dump tuple to disk
    //=====================================       
    MessageSvc::Info("Snapshotting");
    lastNode.Snapshot("DecayTuple", "TupleProcess_tmp.root");
    MessageSvc::Info("Snapshotting done");
    //=====================================
    // Shuffle tuple at exit (entries consistenty)
    //=====================================  
    TFile f1("TupleProcess.root","READ");
    auto  original =  (TTree*)f1.Get("DecayTuple");
    auto  entries = original->GetEntries();
    TFile f2("TupleProcess_tmp.root", "READ");
    auto updated = (TTree*)f2.Get("DecayTuple");
    if( original->GetEntries() != updated->GetEntries()) MessageSvc::Error("Severe error on Snapshots", "","EXIT_FAILURE");
    else{ f1.Close(); f2.Close(); }
    if(!_resnapMCDT){
        MessageSvc::Info("rootmv TupleProcess.root:MCDecayTuple TupleProcess_tmp.root");    
        moveTupleFromTo( "MCDecayTuple", "TupleProcess.root", "TupleProcess_tmp.root");
    }else{
        MessageSvc::Info("rootmv TupleProcess_tmpMCDT.root:MCDecayTuple TupleProcess_tmp.root");    
        moveTupleFromTo( "MCDecayTuple", "TupleProcess_tmpMCDT.root", "TupleProcess_tmp.root");        
        IOSvc::runCommand(TString("rm TupleProcess_tmpMCDT.root"));
    }
    MessageSvc::Info("rm TupleProcess.root");
    IOSvc::runCommand(TString("rm TupleProcess.root"));
    MessageSvc::Info("mv TupleProcess_tmp.root TupleProcess.root");
    IOSvc::runCommand(TString("mv TupleProcess_tmp.root TupleProcess.root"));
    TFile finalFile("TupleProcess.root","READ");
    auto finalTree = (TTree*)finalFile.Get("DecayTuple");
    if( finalTree == nullptr) MessageSvc::Error("Failure AddQ2SmearingCorrection adder","","EXIT_FAILURE");
    if( finalTree->GetEntries() != entries) MessageSvc::Error("Failure AddQ2SmearingCorrection adder", "","EXIT_FAILURE");    
    MessageSvc::Info("Closing file");
    finalFile.Close();
    return;
}

RNode HelperProcessing::AppendBSmearColumns( RNode df, const Prj & _prj, const Year & _year){
    if( df.HasColumn("B_DTF_M_SMEAR_Bp_wMC") ){
        MessageSvc::Warning("AppendBSmearColumns", (TString)"BSmearing already performed , do nothing, branch and column already exists");
        return df;    
    }
    if( !df.HasColumn("E1_BremMultiplicity")){ 
        MessageSvc::Warning("AppendBSmearColumns", (TString)"Q2Smearing invalid, not an eletron tuple, return!!!!");
        return df;        
    }

    TString _head_TRUE = TruePort::B_TRUEM_PREFSR_PORT.Data();  //See the Porting of Branches routine in the HelperProcessing class , how this branch is generated, or supposed ot be generated!
    TString _headDTF_M = "ERROR";
    if( !df.HasColumn( _head_TRUE.Data())){
        MessageSvc::Error("AppendBSmearColumns", TString("Cannot perform smearing, true B mass is NOT existing in the tuple"), "EXIT_FAILURE");
    }
    if( df.HasColumn("B0_BKGCAT")) _headDTF_M = "B0_DTF_M"; 
    if( df.HasColumn("Bp_BKGCAT")) _headDTF_M = "Bp_DTF_M";
    if( df.HasColumn("Bs_BKGCAT")) _headDTF_M = "Bs_DTF_M";
    //The variable to smear is the B_DTF_M value which is the one we fit for, around a true B mass being the MCDT ported value via Run-Nb, Evt-Nb
    using ColumnNames = std::vector<std::string>;

    //!!!! IMPORTANT , if the tuple has attached the ```_PR``` variant of TRUE M , let's smear around that ONE! 
    TString _B_TRUE_MASS_SMEAR_AROUND_ = TruePort::B_TRUEM_PREFSR_PORT.Data();
    if( df.HasColumn(TruePort::B_TRUEM_PREFSR_PR_PORT.Data() )){
        _B_TRUE_MASS_SMEAR_AROUND_ = TruePort::B_TRUEM_PREFSR_PR_PORT.Data();
    }

    if( SettingDef::Weight::q2SmearDiffVar == "" ){
        MessageSvc::Warning("Smearing B mass will be performed with TRUE B Mass = ", _B_TRUE_MASS_SMEAR_AROUND_);
        ColumnNames _inputBSmearing        = {_headDTF_M.Data() , _B_TRUE_MASS_SMEAR_AROUND_.Data() , "ValidBmass", "E1_BremMultiplicity", "E2_BremMultiplicity", "Year"  , "JPs_M", TruePort::JPs_TRUEM_PREFSR.Data()};
        MessageSvc::Info( "AppendBSmearColumns", TString::Format("m(reco) B     = %s", _headDTF_M.Data()));
        MessageSvc::Info( "AppendBSmearColumns", TString::Format("m(true) B     = %s", _B_TRUE_MASS_SMEAR_AROUND_.Data()));
        MessageSvc::Info( "AppendBSmearColumns", TString::Format("m(reco) J/Psi = %s", "JPs_M"));
        MessageSvc::Info( "AppendBSmearColumns", TString::Format("m(true) J/Psi = %s", TruePort::JPs_TRUEM_PREFSR.Data()));

        //Create the B mass smearing as if it's JPsi mode
        Q2SmearCorrection _BpMassSmear( Prj::RK  , _year , "B_DTF_M_SMEAR_Bp_wMC" );
        Q2SmearCorrection _B0MassSMear( Prj::RKst, _year , "B_DTF_M_SMEAR_B0_wMC" );
        _BpMassSmear.Print();
        _B0MassSMear.Print();
        df = df.Define( "ValidBmass", "1") //always a valid B mass ! 
                .Define( _BpMassSmear.branchName(),_BpMassSmear,_inputBSmearing)
                .Define( _B0MassSMear.branchName(),_B0MassSMear,_inputBSmearing);
    }else{
        MessageSvc::Warning("Performing differential smearing q2 as a function of ", SettingDef::Weight::q2SmearDiffVar);
        MessageSvc::Warning("Smearing B mass will be performed with TRUE B Mass = ", _B_TRUE_MASS_SMEAR_AROUND_);

        TString L0ISelection = CutDefRX::Trigger::L0I.GetTitle();
        if(df.HasColumn("Bp_BKGCAT")) L0ISelection = L0ISelection.ReplaceAll("{HEAD}", "Bp");
        if(df.HasColumn("B0_BKGCAT")) L0ISelection = L0ISelection.ReplaceAll("{HEAD}", "B0");
        if(df.HasColumn("Bs_BKGCAT")) L0ISelection = L0ISelection.ReplaceAll("{HEAD}", "Bs");

        TString InputVar = TString::Format("1.* %s", SettingDef::Weight::q2SmearDiffVar.Data());
        if(df.HasColumn("Bp_BKGCAT")) InputVar = InputVar.ReplaceAll("B_", "Bp_");
        if(df.HasColumn("B0_BKGCAT")) InputVar = InputVar.ReplaceAll("B_", "B0_");
        if(df.HasColumn("Bs_BKGCAT")) InputVar = InputVar.ReplaceAll("B_", "Bs_");
        
        ColumnNames _inputBSmearing  = {_headDTF_M.Data() , _B_TRUE_MASS_SMEAR_AROUND_.Data() , "ValidBmass", "E1_BremMultiplicity", "E2_BremMultiplicity", "Year"  , "isL0ICatBSmear", "InputVarBSmear" };
        Q2SmearDifferential _BpMassSmear( Prj::RK  , _year , "B_DTF_M_SMEAR_Bp_wMC" );
        Q2SmearDifferential _B0MassSMear( Prj::RKst, _year , "B_DTF_M_SMEAR_B0_wMC" );
        df = df.Define("isL0ICatBSmear", L0ISelection.Data())
               .Define("InputVarBSmear", InputVar.Data())
               .Define( "ValidBmass", "1") //always a valid B mass ! 
               .Define( _BpMassSmear.branchName(),_BpMassSmear,_inputBSmearing)
               .Define( _B0MassSMear.branchName(),_B0MassSMear,_inputBSmearing);
    }
    return df;
}


RNode HelperProcessing::AppendQ2SmearColumns( RNode df , const Prj & _prj, const Year & _year){
    // TODO : remove this hack (_wMC appended everywhere). In practice TupleProcess already had those branches, we attach with a new name or we define the column on the fly
    // if( df.HasColumn("JPs_M_smear_Bp") ){ 
    if( df.HasColumn("JPs_M_smear_Bp_wMC") ){
        MessageSvc::Warning("AppendQ2SmearColumns does nothing, branch and column already exists");
        return df;
    }
    if( !df.HasColumn("E1_BremMultiplicity")){ 
        MessageSvc::Warning("Q2Smearing invalid, not an eletron tuple, return!!!!");
        return df;
    }

    TString _head_BKGCAT = "ERROR";
    MessageSvc::Warning("TODO, overriding inferred from column content... _prj flag in callee to drop");
    if( df.HasColumn("B0_BKGCAT")) _head_BKGCAT = "B0_BKGCAT";
    if( df.HasColumn("Bp_BKGCAT")) _head_BKGCAT = "Bp_BKGCAT";
    if( df.HasColumn("Bs_BKGCAT")) _head_BKGCAT = "Bs_BKGCAT";
    MessageSvc::Warning("Creating the q2 smearing functors...");
    RNode lastNode(df);
    if( SettingDef::Weight::q2SmearDiffVar == "" ){

        Q2SmearCorrection _BpQ2Smear( Prj::RK  , _year , "JPs_M_smear_Bp_wMC" ); //, {"JPs_M", "JPs_TRUEM", "_BKGCAT" , "E1_BremMultiplicity", "E2_BremMultiplicity");
        Q2SmearCorrection _B0Q2Smear( Prj::RKst, _year , "JPs_M_smear_B0_wMC" ); //, {"JPs_M", "JPs_TRUEM", "_BKGCAT" , "E1_BremMultiplicity", "E2_BremMultiplicity");

        Q2SmearCorrection _BpQ2SmearPostFSR( Prj::RK  , _year , "JPs_M_smear_Bp_postFSR_wMC" ); //, {"JPs_M", "JPs_TRUEM", "_BKGCAT" , "E1_BremMultiplicity", "E2_BremMultiplicity");
        Q2SmearCorrection _B0Q2SmearPostFSR( Prj::RKst, _year , "JPs_M_smear_B0_postFSR_wMC" ); //, {"JPs_M", "JPs_TRUEM", "_BKGCAT" , "E1_BremMultiplicity", "E2_BremMultiplicity");


        Q2SmearCorrection _BpQ2Smear_port( Prj::RK  , _year , "JPs_M_smear_Bp_fromMCDT_wMC" );                //, {"JPs_M", TruePort::JPs_TRUEM_PREFSR.Data(), "NOT0" , "E1_BremMultiplicity", "E2_BremMultiplicity");
        Q2SmearCorrection _B0Q2Smear_port( Prj::RKst, _year , "JPs_M_smear_B0_fromMCDT_wMC" );                //, {"JPs_M", TruePort::JPs_TRUEM_PREFSR.Data(), "NOT0" , "E1_BremMultiplicity", "E2_BremMultiplicity");
        Q2SmearCorrection _BpQ2SmearPostFSR_port( Prj::RK  , _year , "JPs_M_smear_Bp_postFSR_fromMCDT_wMC" ); //, {"JPs_M", TruePort::JPs_TRUEM_POSTFSR.Data(), "NOT0" , "E1_BremMultiplicity", "E2_BremMultiplicity");
        Q2SmearCorrection _B0Q2SmearPostFSR_port( Prj::RKst, _year , "JPs_M_smear_B0_postFSR_fromMCDT_wMC" ); //, {"JPs_M", TruePort::JPs_TRUEM_POSTFSR.Data(), "NOT0" , "E1_BremMultiplicity", "E2_BremMultiplicity");    
        if(false){
            _BpQ2Smear.Print();
            _B0Q2Smear.Print();
            _BpQ2SmearPostFSR.Print();
            _B0Q2SmearPostFSR.Print();
            _BpQ2Smear_port.Print();
            _B0Q2Smear_port.Print();
            _BpQ2SmearPostFSR_port.Print();
            _B0Q2SmearPostFSR_port.Print();
        }    
        //JPs_TRUEM_MCDT
        //JPs_TRUEM_MCDT_POSTFSR
        //TODO : auto lastNode = df.Define( "JPS_TRUE_VALID", "1") 
        // if( _year == Year::Run1 || _year == Year::Run2p1 || _year == Year::Run2p2 || _year == Year::Run2 || _year == Year::All ){
        MessageSvc::Warning("Append Columns Q2 smearing passing YEAR branch, SMEARING MC SAMPLE DEPENDING ON YEARS");
        lastNode = lastNode.Define( "JPS_TRUE_VALID2", "1")
                            .Define( _BpQ2Smear.branchName(),             _BpQ2Smear,             {"JPs_M", "JPs_TRUEM", _head_BKGCAT.Data()           , "E1_BremMultiplicity", "E2_BremMultiplicity", "Year" , "JPs_M", "JPs_TRUEM"}) //TRUEM precomputed on DT used
                            .Define( _B0Q2Smear.branchName(),             _B0Q2Smear,             {"JPs_M", "JPs_TRUEM", _head_BKGCAT.Data()           , "E1_BremMultiplicity", "E2_BremMultiplicity", "Year" , "JPs_M", "JPs_TRUEM"}) 
                            .Define( _BpQ2SmearPostFSR.branchName(),      _BpQ2SmearPostFSR,      {"JPs_M", "JPs_TRUEM_POSTFSR", _head_BKGCAT.Data()   , "E1_BremMultiplicity", "E2_BremMultiplicity", "Year" , "JPs_M", "JPs_TRUEM_POSTFSR"}) //TRUEM_POSTFSR precomputed on DT used
                            .Define( _B0Q2SmearPostFSR.branchName(),      _B0Q2SmearPostFSR,      {"JPs_M", "JPs_TRUEM_POSTFSR", _head_BKGCAT.Data()   , "E1_BremMultiplicity", "E2_BremMultiplicity", "Year" , "JPs_M", "JPs_TRUEM_POSTFSR"})                    
                            .Define( _BpQ2Smear_port.branchName(),        _BpQ2Smear_port,        {"JPs_M", TruePort::JPs_TRUEM_PREFSR.Data(),         "JPS_TRUE_VALID2" , "E1_BremMultiplicity", "E2_BremMultiplicity", "Year" , "JPs_M", TruePort::JPs_TRUEM_PREFSR.Data()}) //TRUEM_MCDT precomputed on MCDT ported evtNb,runNb
                            .Define( _B0Q2Smear_port.branchName(),        _B0Q2Smear_port,        {"JPs_M", TruePort::JPs_TRUEM_PREFSR.Data(),         "JPS_TRUE_VALID2" , "E1_BremMultiplicity", "E2_BremMultiplicity", "Year" , "JPs_M", TruePort::JPs_TRUEM_PREFSR.Data()})
                            .Define( _BpQ2SmearPostFSR_port.branchName(), _BpQ2SmearPostFSR_port, {"JPs_M", TruePort::JPs_TRUEM_POSTFSR.Data(),        "JPS_TRUE_VALID2" , "E1_BremMultiplicity", "E2_BremMultiplicity", "Year" , "JPs_M", TruePort::JPs_TRUEM_POSTFSR.Data()}) //TRUEM_MCDT_POSTFSR precomputed on MCDT ported evtNb,runNb
                            .Define( _B0Q2SmearPostFSR_port.branchName(), _B0Q2SmearPostFSR_port, {"JPs_M", TruePort::JPs_TRUEM_POSTFSR.Data(),        "JPS_TRUE_VALID2" , "E1_BremMultiplicity", "E2_BremMultiplicity", "Year" , "JPs_M", TruePort::JPs_TRUEM_POSTFSR.Data()});        


        //THE NORMALIZATION OF THE Q@ SMEARING CORRECTION 
        // Q2SmearNormalizer w_BpQ2Smear( Prj::RK  , _year , "wJPs_M_smear_Bp_wMC" ); //, {"JPs_M", "JPs_TRUEM", "_BKGCAT" , "E1_BremMultiplicity", "E2_BremMultiplicity");
        // Q2SmearNormalizer w_B0Q2Smear( Prj::RKst, _year , "wJPs_M_smear_B0_wMC" ); //, {"JPs_M", "JPs_TRUEM", "_BKGCAT" , "E1_BremMultiplicity", "E2_BremMultiplicity");

        // Q2SmearNormalizer w_BpQ2SmearPostFSR( Prj::RK  , _year , "wJPs_M_smear_Bp_postFSR_wMC" ); //, {"JPs_M", "JPs_TRUEM", "_BKGCAT" , "E1_BremMultiplicity", "E2_BremMultiplicity");
        // Q2SmearNormalizer w_B0Q2SmearPostFSR( Prj::RKst, _year , "wJPs_M_smear_B0_postFSR_wMC" ); //, {"JPs_M", "JPs_TRUEM", "_BKGCAT" , "E1_BremMultiplicity", "E2_BremMultiplicity");


        // Q2SmearNormalizer w_BpQ2Smear_port( Prj::RK  , _year , "wJPs_M_smear_Bp_fromMCDT_wMC" );                //, {"JPs_M", TruePort::JPs_TRUEM_PREFSR.Data(), "NOT0" , "E1_BremMultiplicity", "E2_BremMultiplicity");
        // Q2SmearNormalizer w_B0Q2Smear_port( Prj::RKst, _year , "wJPs_M_smear_B0_fromMCDT_wMC" );                //, {"JPs_M", TruePort::JPs_TRUEM_PREFSR.Data(), "NOT0" , "E1_BremMultiplicity", "E2_BremMultiplicity");
        // Q2SmearNormalizer w_BpQ2SmearPostFSR_port( Prj::RK  , _year , "wJPs_M_smear_Bp_postFSR_fromMCDT_wMC" ); //, {"JPs_M", TruePort::JPs_TRUEM_POSTFSR.Data(), "NOT0" , "E1_BremMultiplicity", "E2_BremMultiplicity");
        // Q2SmearNormalizer w_B0Q2SmearPostFSR_port( Prj::RKst, _year , "wJPs_M_smear_B0_postFSR_fromMCDT_wMC" ); //, {"JPs_M", TruePort::JPs_TRUEM_POSTFSR.Data(), "NOT0" , "E1_BremMultiplicity", "E2_BremMultiplicity");    
        // if(false){
        //     w_BpQ2Smear.Print();
        //     w_B0Q2Smear.Print();
        //     w_BpQ2SmearPostFSR.Print();
        //     w_B0Q2SmearPostFSR.Print();
        //     w_BpQ2Smear_port.Print();
        //     w_B0Q2Smear_port.Print();
        //     w_BpQ2SmearPostFSR_port.Print();
        //     w_B0Q2SmearPostFSR_port.Print();
        // }
        // //JPs_TRUEM_MCDT
        // //JPs_TRUEM_MCDT_POSTFSR
        // //TODO : auto lastNode = df.Define( "JPS_TRUE_VALID", "1") 
        // // if( _year == Year::Run1 || _year == Year::Run2p1 || _year == Year::Run2p2 || _year == Year::Run2 || _year == Year::All ){
        // MessageSvc::Warning("Append Columns Q2 Normalization Factors  passing YEAR branch, SMEARING MC SAMPLE DEPENDING ON YEARS");
        // // lastNode = lastNode.Define( "JPS_TRUE_VALID2", "1")
        // lastNode = lastNode.Define(  w_BpQ2Smear.branchName(),             w_BpQ2Smear,             {"JPs_M", "JPs_TRUEM", _head_BKGCAT.Data()           , "E1_BremMultiplicity", "E2_BremMultiplicity", "Year" , "JPs_M", "JPs_TRUEM"})        //TRUEM precomputed on DT used
        //                     .Define( w_B0Q2Smear.branchName(),             w_B0Q2Smear,             {"JPs_M", "JPs_TRUEM", _head_BKGCAT.Data()           , "E1_BremMultiplicity", "E2_BremMultiplicity", "Year" , "JPs_M", "JPs_TRUEM"}) 
        //                     .Define( w_BpQ2SmearPostFSR.branchName(),      w_BpQ2SmearPostFSR,      {"JPs_M", "JPs_TRUEM_POSTFSR", _head_BKGCAT.Data()   , "E1_BremMultiplicity", "E2_BremMultiplicity", "Year" , "JPs_M", "JPs_TRUEM_POSTFSR"}) //TRUEM_POSTFSR precomputed on DT used
        //                     .Define( w_B0Q2SmearPostFSR.branchName(),      w_B0Q2SmearPostFSR,      {"JPs_M", "JPs_TRUEM_POSTFSR", _head_BKGCAT.Data()   , "E1_BremMultiplicity", "E2_BremMultiplicity", "Year" , "JPs_M", "JPs_TRUEM_POSTFSR"})                    
        //                     .Define( w_BpQ2Smear_port.branchName(),        w_BpQ2Smear_port,        {"JPs_M", TruePort::JPs_TRUEM_PREFSR.Data(),         "JPS_TRUE_VALID2" , "E1_BremMultiplicity", "E2_BremMultiplicity", "Year" , "JPs_M",  TruePort::JPs_TRUEM_PREFSR.Data()}) //TRUEM_MCDT precomputed on MCDT ported evtNb,runNb
        //                     .Define( w_B0Q2Smear_port.branchName(),        w_B0Q2Smear_port,        {"JPs_M", TruePort::JPs_TRUEM_PREFSR.Data(),         "JPS_TRUE_VALID2" , "E1_BremMultiplicity", "E2_BremMultiplicity", "Year" , "JPs_M",  TruePort::JPs_TRUEM_PREFSR.Data()})
        //                     .Define( w_BpQ2SmearPostFSR_port.branchName(), w_BpQ2SmearPostFSR_port, {"JPs_M", TruePort::JPs_TRUEM_POSTFSR.Data(),        "JPS_TRUE_VALID2" , "E1_BremMultiplicity", "E2_BremMultiplicity", "Year" , "JPs_M",  TruePort::JPs_TRUEM_POSTFSR.Data()}) //TRUEM_MCDT_POSTFSR precomputed on MCDT ported evtNb,runNb
        //                     .Define( w_B0Q2SmearPostFSR_port.branchName(), w_B0Q2SmearPostFSR_port, {"JPs_M", TruePort::JPs_TRUEM_POSTFSR.Data(),        "JPS_TRUE_VALID2" , "E1_BremMultiplicity", "E2_BremMultiplicity", "Year" , "JPs_M",  TruePort::JPs_TRUEM_POSTFSR.Data()});                                    
    }else{
        TString L0ISelection = CutDefRX::Trigger::L0I.GetTitle();
        if(lastNode.HasColumn("Bp_BKGCAT")) L0ISelection = L0ISelection.ReplaceAll("{HEAD}", "Bp");
        if(lastNode.HasColumn("B0_BKGCAT")) L0ISelection = L0ISelection.ReplaceAll("{HEAD}", "B0");
        if(lastNode.HasColumn("Bs_BKGCAT")) L0ISelection = L0ISelection.ReplaceAll("{HEAD}", "Bs");

        TString InputVar = TString::Format("1.* %s", SettingDef::Weight::q2SmearDiffVar.Data());
        if(lastNode.HasColumn("Bp_BKGCAT")) InputVar = InputVar.ReplaceAll("B_", "Bp_");
        if(lastNode.HasColumn("B0_BKGCAT")) InputVar = InputVar.ReplaceAll("B_", "B0_");
        if(lastNode.HasColumn("Bs_BKGCAT")) InputVar = InputVar.ReplaceAll("B_", "Bs_");
        
        lastNode = lastNode.Define("isL0ICat", L0ISelection.Data())
                           .Define("InputVar", InputVar.Data());

        

        Q2SmearDifferential _BpQ2Smear( Prj::RK  , _year , "JPs_M_smear_Bp_wMC" ); //, {"JPs_M", "JPs_TRUEM", "_BKGCAT" , "E1_BremMultiplicity", "E2_BremMultiplicity");
        Q2SmearDifferential _B0Q2Smear( Prj::RKst, _year , "JPs_M_smear_B0_wMC" ); //, {"JPs_M", "JPs_TRUEM", "_BKGCAT" , "E1_BremMultiplicity", "E2_BremMultiplicity");

        Q2SmearDifferential _BpQ2SmearPostFSR( Prj::RK  , _year , "JPs_M_smear_Bp_postFSR_wMC" ); //, {"JPs_M", "JPs_TRUEM", "_BKGCAT" , "E1_BremMultiplicity", "E2_BremMultiplicity");
        Q2SmearDifferential _B0Q2SmearPostFSR( Prj::RKst, _year , "JPs_M_smear_B0_postFSR_wMC" ); //, {"JPs_M", "JPs_TRUEM", "_BKGCAT" , "E1_BremMultiplicity", "E2_BremMultiplicity");


        Q2SmearDifferential _BpQ2Smear_port( Prj::RK  , _year , "JPs_M_smear_Bp_fromMCDT_wMC" );                //, {"JPs_M", TruePort::JPs_TRUEM_PREFSR.Data(), "NOT0" , "E1_BremMultiplicity", "E2_BremMultiplicity");
        Q2SmearDifferential _B0Q2Smear_port( Prj::RKst, _year , "JPs_M_smear_B0_fromMCDT_wMC" );                //, {"JPs_M", TruePort::JPs_TRUEM_PREFSR.Data(), "NOT0" , "E1_BremMultiplicity", "E2_BremMultiplicity");
        Q2SmearDifferential _BpQ2SmearPostFSR_port( Prj::RK  , _year , "JPs_M_smear_Bp_postFSR_fromMCDT_wMC" ); //, {"JPs_M", TruePort::JPs_TRUEM_POSTFSR.Data(), "NOT0" , "E1_BremMultiplicity", "E2_BremMultiplicity");
        Q2SmearDifferential _B0Q2SmearPostFSR_port( Prj::RKst, _year , "JPs_M_smear_B0_postFSR_fromMCDT_wMC" ); //, {"JPs_M", TruePort::JPs_TRUEM_POSTFSR.Data(), "NOT0" , "E1_BremMultiplicity", "E2_BremMultiplicity");    
        MessageSvc::Warning("Append Columns Q2 smearing passing YEAR branch, SMEARING MC SAMPLE DEPENDING ON YEARS and differential in L0I and L0L!!!");
        lastNode = lastNode.Define( "JPS_TRUE_VALID2", "1")
                            .Define( _BpQ2Smear.branchName(),             _BpQ2Smear,             {"JPs_M", "JPs_TRUEM", _head_BKGCAT.Data()           , "E1_BremMultiplicity", "E2_BremMultiplicity", "Year" , "isL0ICat", "InputVar"}) //TRUEM precomputed on DT used
                            .Define( _B0Q2Smear.branchName(),             _B0Q2Smear,             {"JPs_M", "JPs_TRUEM", _head_BKGCAT.Data()           , "E1_BremMultiplicity", "E2_BremMultiplicity", "Year" , "isL0ICat", "InputVar"}) 
                            .Define( _BpQ2SmearPostFSR.branchName(),      _BpQ2SmearPostFSR,      {"JPs_M", "JPs_TRUEM_POSTFSR", _head_BKGCAT.Data()   , "E1_BremMultiplicity", "E2_BremMultiplicity", "Year" , "isL0ICat", "InputVar"}) //TRUEM_POSTFSR precomputed on DT used
                            .Define( _B0Q2SmearPostFSR.branchName(),      _B0Q2SmearPostFSR,      {"JPs_M", "JPs_TRUEM_POSTFSR", _head_BKGCAT.Data()   , "E1_BremMultiplicity", "E2_BremMultiplicity", "Year" , "isL0ICat", "InputVar"})                    
                            .Define( _BpQ2Smear_port.branchName(),        _BpQ2Smear_port,        {"JPs_M", TruePort::JPs_TRUEM_PREFSR.Data(),         "JPS_TRUE_VALID2" , "E1_BremMultiplicity", "E2_BremMultiplicity", "Year" , "isL0ICat", "InputVar"}) //TRUEM_MCDT precomputed on MCDT ported evtNb,runNb
                            .Define( _B0Q2Smear_port.branchName(),        _B0Q2Smear_port,        {"JPs_M", TruePort::JPs_TRUEM_PREFSR.Data(),         "JPS_TRUE_VALID2" , "E1_BremMultiplicity", "E2_BremMultiplicity", "Year" , "isL0ICat", "InputVar"})
                            .Define( _BpQ2SmearPostFSR_port.branchName(), _BpQ2SmearPostFSR_port, {"JPs_M", TruePort::JPs_TRUEM_POSTFSR.Data(),        "JPS_TRUE_VALID2" , "E1_BremMultiplicity", "E2_BremMultiplicity", "Year" , "isL0ICat", "InputVar"}) //TRUEM_MCDT_POSTFSR precomputed on MCDT ported evtNb,runNb
                            .Define( _B0Q2SmearPostFSR_port.branchName(), _B0Q2SmearPostFSR_port, {"JPs_M", TruePort::JPs_TRUEM_POSTFSR.Data(),        "JPS_TRUE_VALID2" , "E1_BremMultiplicity", "E2_BremMultiplicity", "Year" , "isL0ICat", "InputVar"});        
        //THE NORMALIZATION OF THE Q@ SMEARING CORRECTION         
    }
    return lastNode;
}

RNode HelperProcessing::AppendDecModelWeights( RNode df){
    MessageSvc::Info("AppendDecModelWeights");
    //deduce decay and lepton type from branches
    Prj _prj = Prj::Error; 
    if(df.HasColumn("B0_TRUEP_E"))      _prj = Prj::RKst; 
    else if(df.HasColumn("Bp_TRUEP_E")) _prj = Prj::RK;
    else MessageSvc::Error("INVALID node, must contains B0_TRUEP_E or Bp_TRUEP_E", "","EXIT_FAILURE");

    Analysis _ana = Analysis::Error;
    if( df.HasColumn("E1_TRUEP_E"))      _ana = Analysis::EE;
    else if( df.HasColumn("M1_TRUEP_E")) _ana = Analysis::MM;
    else  MessageSvc::Error("INVALID node, must contains B0_TRUEP_E or Bp_TRUEP_E", "","EXIT_FAILURE");


    // put in my public for now, can be moved to within framework also
    MessageSvc::Debug( "Apply decay model weights to project",     to_string(_prj) );
    MessageSvc::Debug( "Apply decay model weights to lepton type", to_string(_ana) );
    // temp branches for angles and q2 are mad with this prefix + "q2, ctl" etc
    std::string angle_prefix = "true_";
    auto _IS_B_Z_DECAY_ = [](int id){ 
        //Decided upon the K_MCMOTHER_ID
        bool bzero  = false;//RDataFrame strict on things being bools, not ints
        if (id>0){bzero = true;}
        return bzero;
    };  
    auto _PHOTOS_Q2 = [](const TLorentzVector  & L1, const TLorentzVector  & L2){
        return (L1+L2).M2()/1.0e6;
    };
    auto _Q2_ = [](const TLorentzVector & b, const TLorentzVector & k,const TLorentzVector & pi){
        auto Kst = k+pi;
        return (b-Kst).M2()/1.0e6;
    };    
    auto _Q2RK_ = [](const TLorentzVector & b, const TLorentzVector & k){    
        return (b-k).M2()/1.0e6;
    };        
    if(_prj == Prj::RKst){   

        DecModelWeightsAdder _wModelEluned( _ana);
        ROOT::Detail::RDF::ColumnNames_t _lep1_inputs;
        ROOT::Detail::RDF::ColumnNames_t _lep2_inputs;
        if( _ana == Analysis::EE){
            _lep1_inputs = { "E1_TRUEP_X", "E1_TRUEP_Y", "E1_TRUEP_Z", "E1_TRUEP_E"};
            _lep2_inputs = { "E2_TRUEP_X", "E2_TRUEP_Y", "E2_TRUEP_Z", "E2_TRUEP_E"};
        }else{
            _lep1_inputs = { "M1_TRUEP_X", "M1_TRUEP_Y", "M1_TRUEP_Z", "M1_TRUEP_E"};
            _lep2_inputs = { "M2_TRUEP_X", "M2_TRUEP_Y", "M2_TRUEP_Z", "M2_TRUEP_E"};        
        }
        ROOT::Detail::RDF::ColumnNames_t _pi_inputs{"Pi_TRUEP_X","Pi_TRUEP_Y","Pi_TRUEP_Z","Pi_TRUEP_E"};
        ROOT::Detail::RDF::ColumnNames_t  _k_inputs{"K_TRUEP_X" ,"K_TRUEP_Y" ,"K_TRUEP_Z" ,"K_TRUEP_E"};
        ROOT::Detail::RDF::ColumnNames_t  _B_inputs{"B0_TRUEP_X","B0_TRUEP_Y","B0_TRUEP_Z","B0_TRUEP_E"};

        df = df.Define("true_Pi",Functors::make_lorentz,_pi_inputs)
               .Define("true_K" ,Functors::make_lorentz,_k_inputs)
               .Define("true_L1",Functors::make_lorentz,_lep1_inputs)
               .Define("true_L2",Functors::make_lorentz,_lep2_inputs)
               .Define("bzero"  ,_IS_B_Z_DECAY_,{ "K_MC_MOTHER_ID"})
               .Define("true_B0", Functors::make_lorentz ,_B_inputs);

        // Compute AngularInfo for RKst analysis
        auto make_angular_info=[]( const bool bzero, const TLorentzVector & vPlus, const TLorentzVector & vMinus, const TLorentzVector & kaon, const TLorentzVector & pion){
            return AngularInfos( bzero, vPlus, vMinus, kaon, pion);
        };
        df = df.Define("angles",   make_angular_info, { "bzero", "true_L1", "true_L2", "true_K", "true_Pi"})
               .Define("true_ctk", [&]( const AngularInfos & infoIn){ return infoIn.CosThetaK(); }, { "angles"})
               .Define("true_ctl", [&]( const AngularInfos & infoIn){ return infoIn.CosThetaL(); }, { "angles"})
               .Define("true_phi", [&]( const AngularInfos & infoIn){ return infoIn.Phi();       }, { "angles"})
               .Define("true_photos_q2", _PHOTOS_Q2  , {"true_L1", "true_L2"})
               .Define("true_q2",            _Q2_    , {"true_B0", "true_K", "true_Pi"});               
        df = df.Define("decay_model_w", _wModelEluned, {"true_q2", "true_ctl", "true_ctk", "true_phi"});
        return df;
    }else{
        TString wfilename_1 = TString::Format("/afs/cern.ch/user/e/elsmith/work_home/public/RX_weights/RK_final_weights_%s.root", to_string(_ana).Data());
        auto _file = IOSvc::OpenFile( wfilename_1 , OpenMode::READ);
        auto _hist = _file->Get<TH1F>( "weights_"+to_string(_ana));
        TH1FHistoAdder _wModelEluned( *_hist , "decay_model_w" , false, wfilename_1);
        ROOT::Detail::RDF::ColumnNames_t  _k_inputs{"K_TRUEP_X" ,"K_TRUEP_Y" ,"K_TRUEP_Z" ,"K_TRUEP_E"};
        ROOT::Detail::RDF::ColumnNames_t  _B_inputs{"Bp_TRUEP_X" ,"Bp_TRUEP_Y" ,"Bp_TRUEP_Z" ,"Bp_TRUEP_E"};
        df = df.Define("true_K"  ,Functors::make_lorentz, _k_inputs)
               .Define("true_Bp", Functors::make_lorentz ,_B_inputs)
               .Define("true_q2" , _Q2RK_ , {  "true_Bp", "true_K"})
               .Define("decay_model_w",_wModelEluned, { "true_q2"});
        return df;     
    }
    return df; 
}
#endif
