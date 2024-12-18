
#include "CounterHelpers.hpp"
#include "CutDefRX.hpp"
#include "EfficiencyCalculator.hpp"
#include "EfficiencySvc.hpp"
#include "EnumeratorSvc.hpp"
#include "EventType.hpp"
#include "HelperSvc.hpp"
#include "IOSvc.hpp"
#include "ParserSvc.hpp"
#include "SettingDef.hpp"
#include "TH1.h"
#include "TH2D.h"
#include "TH2Poly.h"
#include "TMath.h"
#include "TRandom3.h"
#include "TupleHolder.hpp"
#include "VariableBinning.hpp"
#include "WeightDefRX.hpp"
#include "core.h"
#include "itertools.hpp"
#include <ROOT/RDFHelpers.hxx>
#include <ROOT/RDataFrame.hxx>
#include <fmt_ostream.h>
#include <fstream>
#include <iostream>
#include "HelperProcessing.hpp"
#include "CustomActions.hpp"
#include "Functors.hpp"
using namespace iter;
using namespace std;
#include "TInterpreter.h"
#include "TInterpreterValue.h"

// void ConfigureTuple(){
//   SettingDef::IO::useEOS = true;
//   SettingDef::Tuple::option           = "pro";
//   SettingDef::Tuple::gngVer           = "10";
//   SettingDef::Tuple::creVer           = "NONE";
//   SettingDef::Tuple::proVer           = "BDT2-ECAL100_PIDDataSimRatio_PORT";
//   SettingDef::Tuple::branches         = false;
//   SettingDef::Tuple::aliases          = false;
//   SettingDef::Tuple::frac             = -1;
//   return; 
// }
// void ConfigureWeightsMapsLoading(){
//   SettingDef::Weight::option             = "";
//   SettingDef::Weight::config             = "B0_fit";
//   SettingDef::Weight::pidVer             = "DataSimRatio_June01_BS";
//   SettingDef::Weight::l0Ver              = "L0_PIDDataSimRatio_ECAL100_BS";
//   SettingDef::Weight::hltVer             = "NominalChain_PIDDataSimRatio_ECAL100_BS";
//   SettingDef::Weight::q2SmearFileTag     = "TailA2Scale";
//   SettingDef::Weight::useBS              = false ;
//   SettingDef::Weight::iBS                = -1;
//   SettingDef::Weight::priorChain         = false ;
//   SettingDef::Weight::usePIDPTElectron   = false ;
//   SettingDef::Weight::useMCRatioPID      = false;
//   SettingDef::Weight::useStatusL0Formula = false;
//   SettingDef::Efficiency::option         = "OnTheFly";
//   return;
// }
// /*
// void ConfigureSampleLoading(){
//   SettingDef::Config::sample= "Bu2KMM";
//   SettingDef::Config::q2bin = "central";
//   SettingDef::Cut::option   = "tmSig-MAXP-OneGhostNMatches-HLT1AllTrackL0AlignedTOS-vetoPsi-vetoJPs-HOP-cutECALDistance-noPID-ePID-q2SmearMCDTB0-noQ2";
//   SettingDef::Cut::extraCut = "Bp_DTF_M > 5150 && Bp_DTF_M < 5850";
// }
// */
// void ConfigureSampleLoading(){
//   SettingDef::Config::sample= "Bu2KMM";
//   SettingDef::Config::q2bin = "central";
//   SettingDef::Cut::option   = "tmSig-MAXP-OneGhostNMatches-HLT1AllTrackL0AlignedTOS-vetoPsi-vetoJPs-HOP-cutECALDistance-noPID-ePID-q2SmearMCDTB0-noQ2";
//   SettingDef::Cut::extraCut = "Bp_DTF_M > 5150 && Bp_DTF_M < 5850";
// }

int main(int argc, char ** argv) {
  //Histogramming stuff lambda simplify titling 
  auto _get_title_ = []( TString _label_ , Year _year , Polarity _polarity){
    return TString::Format("%s_%s_%s", _label_.Data(), 
                                        to_string( _year).Data(), 
                                        to_string(_polarity).Data());
  };
  auto _get_nameplot_ = []( TString _label_ ,  TString _xAxis_, TString _yAxis_, Year _year , Polarity _polarity){
    return TString::Format("%s %s %s; %s;%s", _label_.Data(), 
                                              to_string( _year).Data(), 
                                              to_string(_polarity).Data(),
                                              _xAxis_.Data(), 
                                              _yAxis_.Data());
  };    


  //======== BEGIN HERE ========//
  ParserSvc parser("");
  parser.Init(argc, argv);
  if (parser.Run(argc, argv) != 0) return 1;
  TString _yaml_ConfigFile = parser.YAML();
  auto PARSER = YAML::LoadFile(_yaml_ConfigFile.Data());
 
  //===== Take the MC sample =====//  
  vector< Year>     _years      = { Year::Y2011, Year::Y2012, Year::Y2015, Year::Y2016, Year::Y2017, Year::Y2018};
  vector< Polarity> _polarities = { Polarity::MD, Polarity::MU};
  if(IOSvc::ExistDir(PARSER["Setting"]["outDir"].as<TString>() )){
    IOSvc::MakeDir(PARSER["Setting"]["outDir"].as<TString>(), OpenMode::RECREATE);
  }
  TString _OUT_DIR_ = PARSER["Setting"]["outDir"].as<TString>();
  
  for (const auto && [yy, pp] : iter::product(_years, _polarities)){

    //Basic ConfigHolder and DecayTuple grabber
    ConfigHolder cH(hash_project(PARSER["Config"]["project"].as<TString>()), 
                    hash_analysis(PARSER["Config"]["ana"].as<TString>()), 
                    PARSER["Config"]["sample"].as<TString>(),
                    hash_q2bin(PARSER["Config"]["q2bin"].as<TString>()), 
                    yy,
                    pp,
                    Trigger::All,
                    TriggerConf::Exclusive,
                    Brem::All,
                    Track::All);
    cH.Init();
    TupleHolder tH( cH, "pro"); tH.Init();
    ROOT::RDataFrame dfDecayTuple( *tH.GetTuple());
    ROOT::RDF::RNode DT_NODE( dfDecayTuple);


    //ConfigHolders Split By Triggers
    ConfigHolder _CONFHOLD_L0I(
                    hash_project(PARSER["Config"]["project"].as<TString>()), 
                    Analysis::MM, 
                    PARSER["Config"]["sample"].as<TString>(),
                    hash_q2bin(SettingDef::Config::q2bin), 
                    yy, pp, Trigger::L0I ,
                    TriggerConf::Exclusive,
                    Brem::All,
                    Track::All);
    _CONFHOLD_L0I.Init();
    ConfigHolder _CONFHOLD_L0L(
                    hash_project(PARSER["Config"]["project"].as<TString>()), 
                    Analysis::MM,  
                    PARSER["Config"]["sample"].as<TString>(),
                    hash_q2bin(PARSER["Config"]["q2bin"].as<TString>()),
                    yy, pp, Trigger::L0L , 
                    TriggerConf::Exclusive, 
                    Brem::All,
                    Track::All);
    _CONFHOLD_L0L.Init();
    ConfigHolder _CONFHOLD_L0L2(
                    hash_project(PARSER["Config"]["project"].as<TString>()), 
                    Analysis::MM, 
                    PARSER["Config"]["sample"].as<TString>(),
                    hash_q2bin(PARSER["Config"]["q2bin"].as<TString>()),
                    yy, pp, Trigger::L0L ,
                    TriggerConf::Exclusive2 ,
                    Brem::All,
                    Track::All);
    _CONFHOLD_L0L2.Init();

    CutHolder _CUTHOLD_L0I(_CONFHOLD_L0I,   PARSER["Cut"]["option"].as<TString>() ); _CUTHOLD_L0I.Init();
    CutHolder _CUTHOLD_L0L(_CONFHOLD_L0L,   PARSER["Cut"]["option"].as<TString>() ); _CUTHOLD_L0L.Init();
    CutHolder _CUTHOLD_L0L2(_CONFHOLD_L0L2, PARSER["Cut"]["option"].as<TString>() ); _CUTHOLD_L0L2.Init();
    vector<TString> _cutSetNorm{ "cutSPD" ,"cutMCT", "cutPS" , "cutTRG" , "cutPID"};
    //TODO : add cut NORM from the list of cuts...

    //Create the WeightHolders to use around full Selection
    TString _weightOption     =  PARSER["Weight"]["option"].as<TString>();
    TString _weightOptionMCDT =  PARSER["Weight"]["optionMCDT"].as<TString>();
    TString _weightOptionNormN=  PARSER["Weight"]["optionNORMN"].as<TString>();
    TString _weightOptionNormD=  PARSER["Weight"]["optionNORMD"].as<TString>();

    TString _extraCut = PARSER["Cut"]["extraCut"].as<TString>();
    WeightHolder _WEIHOLD_L0I(_CONFHOLD_L0I,   _weightOption ); _WEIHOLD_L0I.Init();
    WeightHolder _WEIHOLD_L0L(_CONFHOLD_L0L,   _weightOption ); _WEIHOLD_L0L.Init();
    WeightHolder _WEIHOLD_L0L2(_CONFHOLD_L0L2, _weightOption ); _WEIHOLD_L0L2.Init();

    WeightHolder _WEIHOLD_L0I_NORMN(_CONFHOLD_L0I,   _weightOptionNormN ); _WEIHOLD_L0I_NORMN.Init();
    WeightHolder _WEIHOLD_L0L_NORMN(_CONFHOLD_L0L,   _weightOptionNormN ); _WEIHOLD_L0L_NORMN.Init();
    WeightHolder _WEIHOLD_L0L2_NORMN(_CONFHOLD_L0L2, _weightOptionNormN ); _WEIHOLD_L0L2_NORMN.Init();


    WeightHolder _WEIHOLD_L0I_NORMD(_CONFHOLD_L0I,   _weightOptionNormD ); _WEIHOLD_L0I_NORMD.Init();
    WeightHolder _WEIHOLD_L0L_NORMD(_CONFHOLD_L0L,   _weightOptionNormD ); _WEIHOLD_L0L_NORMD.Init();
    WeightHolder _WEIHOLD_L0L2_NORMD(_CONFHOLD_L0L2, _weightOptionNormD ); _WEIHOLD_L0L2_NORMD.Init();

    WeightHolder _WEIHOLD_L0I_MCDT(_CONFHOLD_L0I,   _weightOptionMCDT ); _WEIHOLD_L0I_MCDT.Init();
    WeightHolder _WEIHOLD_L0L_MCDT(_CONFHOLD_L0L,   _weightOptionMCDT ); _WEIHOLD_L0L_MCDT.Init();
    WeightHolder _WEIHOLD_L0L2_MCDT(_CONFHOLD_L0L2, _weightOptionMCDT ); _WEIHOLD_L0L2_MCDT.Init();

    //---- We need the cutSetNORM to make histos of normalization---- //
    TCut _cutNORM_L0I("1>0");
    TCut _cutNORM_L0L("1>0");
    TCut _cutNORM_L0L2("1>0");
    for( auto  cutBit : PARSER["Cut"]["optionNORMSET"].as<vector<std::string>>() ){
      MessageSvc::Debug("Cut bit included", TString(cutBit));
      _cutNORM_L0I   = _cutNORM_L0I  && _CUTHOLD_L0I.Cuts().at(   TString(cutBit));
      _cutNORM_L0L   = _cutNORM_L0L  && _CUTHOLD_L0L.Cuts().at(   TString(cutBit));
      _cutNORM_L0L2  = _cutNORM_L0L2 && _CUTHOLD_L0L2.Cuts().at(  TString(cutBit));
    }
    

    //Debugging messages around...
    MessageSvc::Line();
    _CONFHOLD_L0I.PrintInline();
    MessageSvc::Warning( " Cut         L0I  ", (TString)_CUTHOLD_L0I.Cut().GetTitle() );
    MessageSvc::Warning( " EXTRA Cut   L0I  ", _extraCut );
    MessageSvc::Warning( " Weight      L0I  ", (TString)_WEIHOLD_L0I.Weight().Data() );
    MessageSvc::Warning( " WeightNormN L0I  ", (TString)_WEIHOLD_L0I_NORMN.Weight().Data() );
    MessageSvc::Warning( " WeightNormD L0I  ", (TString)_WEIHOLD_L0I_NORMD.Weight().Data() );
    MessageSvc::Warning( " WeightMCDT  L0I  ", (TString)_WEIHOLD_L0I_MCDT.Weight().Data() );
    MessageSvc::Warning( " CutNorm     L0I  ", (TString)_cutNORM_L0I.GetTitle() );
    MessageSvc::Line();
    _CONFHOLD_L0L.PrintInline();
    MessageSvc::Warning( " Cut         L0L  ", (TString)_CUTHOLD_L0L.Cut().GetTitle() );
    MessageSvc::Warning( " EXTRA Cut   L0L  ", _extraCut );
    MessageSvc::Warning( " Weight      L0L  ", (TString)_WEIHOLD_L0L.Weight().Data() );
    MessageSvc::Warning( " WeightNormN L0L  ", (TString)_WEIHOLD_L0L_NORMN.Weight().Data() );
    MessageSvc::Warning( " WeightNormD L0L  ", (TString)_WEIHOLD_L0L_NORMD.Weight().Data() );
    MessageSvc::Warning( " WeightMCDT  L0L  ", (TString)_WEIHOLD_L0L_MCDT.Weight().Data() );
    MessageSvc::Warning( " CutNorm     L0L  ", (TString)_cutNORM_L0L.GetTitle() );
    _CONFHOLD_L0L2.PrintInline();
    MessageSvc::Warning( " Cut         L0L2 ", (TString)_CUTHOLD_L0L2.Cut().GetTitle() );
    MessageSvc::Warning( " EXTRA Cut   L0L2 ", _extraCut );
    MessageSvc::Warning( " Weight      L0L2 ", (TString)_WEIHOLD_L0L2.Weight().Data() );
    MessageSvc::Warning( " WeightNormN L0L2 ", (TString)_WEIHOLD_L0L2_NORMN.Weight().Data() );
    MessageSvc::Warning( " WeightNormD L0L2 ", (TString)_WEIHOLD_L0L2_NORMD.Weight().Data() );
    MessageSvc::Warning( " WeightMCDT  L0L2 ", (TString)_WEIHOLD_L0L2_MCDT.Weight().Data() );
    MessageSvc::Warning( " CutNorm     L0L2 ", (TString)_cutNORM_L0L2.GetTitle() );
    MessageSvc::Line();


    // The q2 plot bin edges declaration 
    vector<double> _bins_edges{ 0.1, 0.98, 1.1, 2, 3, 4, 5, 6 , 7, 8, 9, 10, 11, 11.75, 12.50, 15.0, 16.0,17.0,18.0,19.0,20.0, 21.0, 22.0};
    MessageSvc::Warning("Dealing with DT tuple");
    


    //RDataFrame NODE weight attaching ! 
    DT_NODE = HelperProcessing::AttachWeights(DT_NODE, cH, "PID-TRK-L0-HLT-nTracks-BDT-BKIN-MULT-RECO");
    //RDataFrame NODE Selection BOOL and Weight and q2 Definition attaching
    DT_NODE = DT_NODE.Define("q2Reconstructed", "TMath::Sq(JPs_M/1000.)")
                     .Define( "wL0I_full",  _WEIHOLD_L0I.Weight().Data())
                     .Define( "wL0L_full",  _WEIHOLD_L0L.Weight().Data())
                     .Define( "wL0L2_full", _WEIHOLD_L0L2.Weight().Data())
                     .Define( "cutL0I_full",_CUTHOLD_L0I.Cut().GetTitle())
                     .Define( "cutL0L_full",_CUTHOLD_L0L.Cut().GetTitle())
                     .Define( "cutL0L2_full",_CUTHOLD_L0L2.Cut().GetTitle())
                     .Define( "cut_L0I_norm",_cutNORM_L0I.GetTitle())
                     .Define( "cut_L0L_norm",_cutNORM_L0L.GetTitle())
                     .Define( "cut_L0L2_norm", _cutNORM_L0L2.GetTitle())
                     .Define( "wNORM_L0I_N",    _WEIHOLD_L0I_NORMN.Weight().Data())
                     .Define( "wNORM_L0L_N",  _WEIHOLD_L0L_NORMN.Weight().Data())
                     .Define( "wNORM_L0L2_N", _WEIHOLD_L0L2_NORMN.Weight().Data())
                     .Define( "wNORM_L0I_D",  _WEIHOLD_L0I_NORMD.Weight().Data())
                     .Define( "wNORM_L0L_D",  _WEIHOLD_L0L_NORMD.Weight().Data())
                     .Define( "wNORM_L0L2_D", _WEIHOLD_L0L2_NORMD.Weight().Data());                 

    //Plots for sumW | full selection 
    auto hL0I_Num = DT_NODE.Filter( "cutL0I_full").Filter(_extraCut.Data() )
                           .Histo1D( {_get_title_( "sumW_L0I", yy, pp), 
                                      _get_nameplot_( "L0I", "q^{2}", "Counts", yy, pp) , 
                                      int(_bins_edges.size())-1 , _bins_edges.data()}, 
                                      "q2Reconstructed" , "wL0I_full" );

    auto hL0L_Num = DT_NODE.Filter( "cutL0L_full").Filter(_extraCut.Data() )
                           .Histo1D( {_get_title_( "sumW_L0L", yy, pp), 
                                      _get_nameplot_( "L0L", "q^{2}", "Counts", yy, pp) , 
                                      int(_bins_edges.size())-1 , _bins_edges.data()}, 
                                      "q2Reconstructed" , "wL0L_full" );

    auto hL0L2_Num = DT_NODE.Filter( "cutL0L2_full").Filter(_extraCut.Data() )
                           .Histo1D( {_get_title_( "sumW_L0L2", yy, pp), 
                                      _get_nameplot_( "L0L2", "q^{2}", "Counts", yy, pp) , 
                                      int(_bins_edges.size())-1 , _bins_edges.data()}, 
                                      "q2Reconstructed" , "wL0L2_full" );

    //NORM N 
    auto NormN_hL0I = DT_NODE.Filter( "cut_L0I_norm")
                           .Histo1D( {_get_title_( "normN_L0I", yy, pp), 
                                      _get_nameplot_( "normN L0I", "q^{2}", "Counts", yy, pp) , 
                                      int(_bins_edges.size())-1 , _bins_edges.data()}, 
                                      "q2Reconstructed" , "wNORM_L0I_N" );

    auto NormN_hL0L = DT_NODE.Filter( "cut_L0L_norm")
                           .Histo1D( {_get_title_( "normN_L0L", yy, pp), 
                                      _get_nameplot_( "normN L0L", "q^{2}", "Counts", yy, pp) , 
                                      int(_bins_edges.size())-1 , _bins_edges.data()}, 
                                      "q2Reconstructed" , "wNORM_L0L_N" );

    auto NormN_hL0L2 = DT_NODE.Filter( "cut_L0L2_norm")
                           .Histo1D( {_get_title_( "normN_L0L2", yy, pp), 
                                      _get_nameplot_( "normN L0L2", "q^{2}", "Counts", yy, pp) , 
                                      int(_bins_edges.size())-1 , _bins_edges.data()}, 
                                      "q2Reconstructed" , "wNORM_L0L2_N" );    
    //NORM D
    auto NormD_hL0I      = DT_NODE.Filter( "cut_L0I_norm")
                           .Histo1D( {_get_title_( "normD_L0I", yy, pp), 
                                      _get_nameplot_( "normD L0I", "q^{2}", "Counts", yy, pp) , 
                                      int(_bins_edges.size())-1 , _bins_edges.data()}, 
                                      "q2Reconstructed" , "wNORM_L0I_D" );

    auto NormD_hL0L      = DT_NODE.Filter( "cut_L0L_norm")
                           .Histo1D( {_get_title_( "normD_L0L", yy, pp), 
                                      _get_nameplot_( "normD L0L", "q^{2}", "Counts", yy, pp) , 
                                      int(_bins_edges.size())-1 , _bins_edges.data()}, 
                                      "q2Reconstructed" , "wNORM_L0L_D" );

    auto NormD_hL0L2       = DT_NODE.Filter( "cut_L0L2_norm")
                           .Histo1D( {_get_title_( "normD_L0L2", yy, pp), 
                                      _get_nameplot_( "normD L0L2", "q^{2}", "Counts", yy, pp) , 
                                      int(_bins_edges.size())-1 , _bins_edges.data()}, 
                                      "q2Reconstructed" , "wNORM_L0L2_D" ); 
    //Basic ConfigHolder and MCDecayTuple grabber
    auto _orig = SettingDef::Tuple::tupleName;
    SettingDef::Tuple::tupleName = "MCDT";
    TupleHolder tHMCDT(cH, "", "MCT", SettingDef::Tuple::option);
    tHMCDT.Init();
    SettingDef::Tuple::tupleName = _orig;     
    ROOT::RDataFrame dfMCDecayTuple( *tHMCDT.GetTuple());
    ROOT::RDF::RNode MCDT_NODE( dfMCDecayTuple);

    TString _cutMCDT("");
    if( yy == Year::Y2011 || yy ==Year::Y2012){
      _cutMCDT = "nSPDHits<600";
    }else{
      _cutMCDT = "nSPDHits<450";
    }    


    MessageSvc::Warning("Dealing with MCDT tuple");
    MCDT_NODE = MCDT_NODE.Filter( _cutMCDT.Data());
    MCDT_NODE = MCDT_NODE.Define("q2PostFSR_TRUE", "TMath::Sq(JPs_TRUEM_POSTFSR/1000.)")
                         .Define("wL0I_full", _WEIHOLD_L0I_MCDT.Weight().Data() )
                         .Define("wL0L_full", _WEIHOLD_L0L_MCDT.Weight().Data() )
                         .Define("wL0L2_full", _WEIHOLD_L0L2_MCDT.Weight().Data() );

    auto MCDT_hL0I = MCDT_NODE.Filter(_cutMCDT.Data() )
                           .Histo1D( {
                                      _get_title_( "sumWMCDT_L0I", yy, pp), _get_nameplot_( "MCDTL0I", "q^{2}_{postFSR}", "Counts", yy, pp) , int( _bins_edges.size())-1 , _bins_edges.data()}, 
                                      "q2PostFSR_TRUE" , "wL0I_full" );

    auto MCDT_hL0L = MCDT_NODE.Filter(_cutMCDT.Data() )
                                  .Histo1D( {
                                              _get_title_( "sumWMCDT_L0L", yy, pp), _get_nameplot_( "MCDTL0L", "q^{2}_{postFSR}", "Counts", yy, pp) , int( _bins_edges.size())-1 , _bins_edges.data()
                                            }, "q2PostFSR_TRUE" , "wL0L_full" );

    auto MCDT_hL0L2 = MCDT_NODE.Filter(_cutMCDT.Data() )
                           .Histo1D( {
                                              _get_title_( "sumWMCDT_L0L2", yy, pp), _get_nameplot_( "MCDTL0L2", "q^{2}_{postFSR}", "Counts", yy, pp) , int( _bins_edges.size())-1 , _bins_edges.data()
                                      }, "q2PostFSR_TRUE" , "wL0L2_full" );
    TFile _out_file( TString::Format("%s/Eff_%s_%s_%s.root",  
                                    PARSER["Setting"]["outDir"].as<TString>().Data(),
                                    PARSER["Config"]["sample"].as<TString>().Data(),
                                    to_string(yy).Data(), to_string(pp).Data()), "RECREATE");
    _out_file.WriteObjectAny( MCDT_hL0I.GetPtr() ,      MCDT_hL0I.GetPtr()->ClassName(),      MCDT_hL0I.GetPtr()->GetName() );
    _out_file.WriteObjectAny( MCDT_hL0L.GetPtr() ,      MCDT_hL0L.GetPtr()->ClassName(),      MCDT_hL0L.GetPtr()->GetName() );
    _out_file.WriteObjectAny( MCDT_hL0L2.GetPtr() ,     MCDT_hL0L2.GetPtr()->ClassName(),     MCDT_hL0L2.GetPtr()->GetName() );
    _out_file.WriteObjectAny( hL0I_Num.GetPtr()      ,  hL0I_Num.GetPtr()->ClassName(),       hL0I_Num.GetPtr()->GetName() );
    _out_file.WriteObjectAny( hL0L_Num.GetPtr()      ,  hL0L_Num.GetPtr()->ClassName(),       hL0L_Num.GetPtr()->GetName() );
    _out_file.WriteObjectAny( hL0L2_Num.GetPtr()     ,  hL0L2_Num.GetPtr()->ClassName(),      hL0L2_Num.GetPtr()->GetName());     

    _out_file.WriteObjectAny( NormN_hL0I.GetPtr(), NormN_hL0I.GetPtr()->ClassName(),   NormN_hL0I.GetPtr()->GetName());
    _out_file.WriteObjectAny( NormN_hL0L.GetPtr(), NormN_hL0L.GetPtr()->ClassName(),   NormN_hL0L.GetPtr()->GetName());
    _out_file.WriteObjectAny( NormN_hL0L2.GetPtr(), NormN_hL0L2.GetPtr()->ClassName(), NormN_hL0L2.GetPtr()->GetName());
    _out_file.WriteObjectAny( NormD_hL0I.GetPtr(), NormD_hL0I.GetPtr()->ClassName(),   NormD_hL0I.GetPtr()->GetName());
    _out_file.WriteObjectAny( NormD_hL0L.GetPtr(), NormD_hL0L.GetPtr()->ClassName(),   NormD_hL0L.GetPtr()->GetName());
    _out_file.WriteObjectAny( NormD_hL0L2.GetPtr(), NormD_hL0L2.GetPtr()->ClassName(), NormD_hL0L2.GetPtr()->GetName());
    
    _out_file.Close();

    std::cout<< "DONE, MOVE to next" << std::endl;

    // break;
  }
}