#ifndef EFFICIENCYSVC_CPP
#define EFFICIENCYSVC_CPP
#include "EnumeratorSvc.hpp"
#include "EfficiencySvc.hpp"
#include "ConstDef.hpp"
#include "SettingDef.hpp"
#include "ConfigHolder.hpp"
#include "HelperSvc.hpp"
#include "HistogramSvc.hpp"
#include "IOSvc.hpp"
#include "MessageSvc.hpp"

#include "yamlcpp.h"
#include "zipfor.h"
#include <fmt_ostream.h>

#include "TCanvas.h"
#include "TEfficiency.h"
#include "TFile.h"
#include "TGraphAsymmErrors.h"
#include "TH1.h"
#include "TH2.h"
#include "TH2Poly.h"
#include "TMath.h"
#include "EfficiencyForFitHandler.hpp"
#include "itertools.hpp"
#include "CustomActions.hpp"
#include "WeightDefRX.hpp"
using namespace  std;

//
// SHOULD ACTUALLY BE AN EVENTTYPE OR AT LEAST CONTAIN A CUT/WEIGHT HOLDER TO ACCESS OPTIONS
//
RooRealVar * LoadEfficiencyForFit(ConfigHolder & _configHolder, TString _mode) noexcept {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "LoadEfficiencyForFit", _mode );
    bool _printIt = true;
    if( _configHolder.IsSignalMC() && 
        (_configHolder.GetQ2bin() == Q2Bin::Low || _configHolder.GetQ2bin() == Q2Bin::Central) 
        && SettingDef::Fit::blindEfficiency ){
        _printIt = false;
    }
    _configHolder.PrintInline();

    Prj         _prj      = _configHolder.GetProject();
    Analysis    _ana      = _configHolder.GetAna();
    Q2Bin       _q2bin    = _configHolder.GetQ2bin();
    TString     _sample   = _configHolder.GetSample();
    Polarity    _polarity = _configHolder.GetPolarity();
    Year        _year     = _configHolder.GetYear();
    Trigger     _trg      = _configHolder.GetTrigger();
    TriggerConf _trgConf  = _configHolder.GetTriggerConf();
    Track       _trk      = _configHolder.GetTrack();
    //Maybe to hack here
    // if( _configHolder.IsCrossFeedSample() )             _prj   = Prj::RKst  ; //Hack around cross-feed, efficiency setup has to be the same one from RKst project 
    // if( _configHolder.GetSample() == "Bu2KEtaPrimGEE" ) _q2bin = Q2Bin::JPsi; //Hack around Bu2KEtaPrimeGEE, efficiency setup has to be the same one from RK-J/Psi project, wBx/wBx ! 
    auto InfoEffToUse = SettingDef::Efficiency::fitconfiguration.GetEfficiencyInfo( _prj, _q2bin, _mode);
    auto wOptionUse   = InfoEffToUse.wOption();
    auto wConfUse     = InfoEffToUse.wConfig();
    auto eVerUse      = InfoEffToUse.effVersion();
    auto eVarUse      = InfoEffToUse.effVariable();

    if(_mode == "RRATIO"){
        //Special, we must ensure the Bp/Bp ratio or B0/B0 ratio in efficiencies is obtained when dealing with cross-feed, KEtaPrime and Leakage-Central q2 wrt J/Psi mode 
        if( _q2bin == Q2Bin::Central && (_sample == "Bu2KJPsEE" || _sample=="Bd2KstJPsEE") ){
            //Leakage dealed with 
            auto InfoEffReference = SettingDef::Efficiency::fitconfiguration.GetEfficiencyInfo(_prj, Q2Bin::JPsi, _mode);
            auto wConfReference     = InfoEffReference.wConfig();
            auto wOptionReference   = InfoEffReference.wOption();
            if( wOptionUse != wOptionReference){ 
                MessageSvc::Warning("LoadEfficiencyForFit Leakage mismatched weight Options for Rare/JPsi, Trying to fix it with replace options fully");
                MessageSvc::Warning("Forcing", TString::Format("Change %s --> %s", wOptionUse.Data(), wOptionReference.Data()));
                wOptionUse = wOptionReference;
            }
            if( wConfReference != wConfUse ){
                MessageSvc::Warning("LoadEfficiencyForFit Leakage mismatched weight config for Rare/JPsi, fix on the fly the setup changing the wBx configuration in use");
                MessageSvc::Warning("Forcing", TString::Format("Change %s --> %s", wConfUse.Data(), wConfReference.Data()));
                wConfUse = wConfReference;
            }
        }
        if( _q2bin == Q2Bin::Low && _sample == "Bu2KEtaPrimeGEE" ){
            auto InfoEffReference = SettingDef::Efficiency::fitconfiguration.GetEfficiencyInfo(_prj, Q2Bin::JPsi, _mode);
            auto wConfReference     = InfoEffReference.wConfig();
            auto wOptionReference   = InfoEffReference.wOption();
            if( wOptionUse != wOptionReference){ 
                MessageSvc::Warning("LoadEfficiencyForFit KEtaPrime mismatched weight Options for Rare/JPsi, Trying to fix it with replace options fully");
                MessageSvc::Warning("Forcing", TString::Format("Change %s --> %s", wOptionUse.Data(), wOptionReference.Data()));
                wOptionUse = wOptionReference;
            }
            if( wConfReference != wConfUse ){
                MessageSvc::Warning("LoadEfficiencyForFit KEtaPrime mismatched weight config for Rare/JPsi, fix on the fly the setup changing the wBx configuration in use");
                MessageSvc::Warning("Forcing", TString::Format("Change %s --> %s", wConfUse.Data(), wConfReference.Data()));
                wConfUse = wConfReference;
            }
            MessageSvc::Warning("Evar Use changed from ", eVarUse, (TString)"rnd");
            eVarUse = "rnd";
        }
        if( ( _q2bin == Q2Bin::Central || _q2bin == Q2Bin::Low) && _sample == "Bd2KstEE" && _prj == Prj::RK ){
            //Rare mode low/central cross feed 
            auto InfoEffReference = SettingDef::Efficiency::fitconfiguration.GetEfficiencyInfo(Prj::RKst, _q2bin, _mode);
            auto wConfReference     = InfoEffReference.wConfig();
            auto wOptionReference   = InfoEffReference.wOption();
            if( wOptionUse != wOptionReference){ 
                MessageSvc::Warning("LoadEfficiencyForFit Bd2KstEE mismatched weight Options for Rare/JPsi, Trying to fix it with replace options fully");
                MessageSvc::Warning("Forcing", TString::Format("Change %s --> %s", wOptionUse.Data(), wOptionReference.Data()));
                wOptionUse = wOptionReference;
            }
            if( wConfReference != wConfUse ){
                MessageSvc::Warning("LoadEfficiencyForFit Bd2KstEE mismatched weight config for RK/RKst, fix on the fly the setup changing the wBx configuration in use");
                MessageSvc::Warning("Forcing", TString::Format("Change %s --> %s", wConfUse.Data(), wConfReference.Data()));
                wConfUse = wConfReference;
            }
            MessageSvc::Warning("Evar Use changed from ", eVarUse, (TString)"rnd");
            eVarUse = "rnd";            
        }
    }//end if RRATIO 


    TString _toGrab = "eff_sel;1";
    if( eVarUse != ""){
        _toGrab.ReplaceAll("eff_sel", Form("eff_sel_%s",eVarUse.Data())) ;
    }
    
    auto _BASEEVER = SettingDef::Efficiency::ver;
    SettingDef::Efficiency::ver = eVerUse;

    _year = GetYearForSample(_sample, _year);   // NEED TO BE CAREFULL WHEN DIFFERENT YEARS ARE AVAILABLE BETWEEN EE AND MM
    auto _years      = GetYears(to_string(_year));
    auto _polarities = GetPolarities(to_string(_polarity));
    vector< pair< Year, Polarity > > _yearsAndPolarities;
    for (const auto && [yy, pp] : iter::product(_years, _polarities)) {
        _yearsAndPolarities.push_back(make_pair(hash_year(yy), hash_polarity(pp)));
    }

    if (_years.size() != 1)      MessageSvc::Warning("LoadEfficiencyForFit", (TString) "Combine", to_string(_years.size()), "years");
    if (_polarities.size() != 1) MessageSvc::Warning("LoadEfficiencyForFit", (TString) "Combine", to_string(_polarities.size()), "polarities");
    vector< double > _effs;
    vector< double > _errs;
    vector< double > _weights;
    MessageSvc::Line();
    for (auto & _yearAndPolarity : _yearsAndPolarities) {
        auto _yy      = _yearAndPolarity.first;
        auto _pol     = _yearAndPolarity.second;
        auto _dirName = IOSvc::GetTupleDir("eff", to_string(_prj), to_string(_ana), to_string(_q2bin), to_string(_yy), to_string(_trg));
        _dirName += "/" + to_string(_yy) + to_string(_pol);
        // TMP HACK ....do this to work with your eos user area to efficiency grabbing and RD for tuples...
        // _dirName.ReplaceAll("eos/lhcb/wg/RD/RKstar", "eos/lhcb/user/r/rquaglia/ewp-rkstz");
        // cout<<"Peaking EFF from DIR " << _dirName<< RESET<< endl;
        auto ch = ConfigHolder(_prj, _ana, _sample, _q2bin, _yy, _pol, _trg, Brem::All, _trk);
        TString _fileName = _dirName + "/";
        TString _option = wOptionUse;
        
        TString _config = _option == "no" ? "" : "_" + wConfUse;
        _fileName += _option + "_" + _sample + "_Efficiency_" + ch.GetKey("addtrgconf") + _config;        
        _fileName += ".root";

        auto * _tFile = IOSvc::OpenFile(_fileName, OpenMode::READ);
        if (_tFile == nullptr) {
            MessageSvc::Warning("LoadEfficiencyForFit", (TString) "Trying opposite polarity");
            if (_pol == Polarity::MD) _fileName.ReplaceAll("MD", "MU");
            if (_pol == Polarity::MU) _fileName.ReplaceAll("MU", "MD");
            _tFile = IOSvc::OpenFile(_fileName, OpenMode::READ);
        } else if (_tFile != nullptr) {
            TString eff_gen_Var = "eff_" + ch.GetKey() + "_gen";
            TString eff_flt_Var = "eff_" + ch.GetKey() + "_flt";
            TString eff_sel_Var;
            eff_sel_Var = _toGrab;            
            TString eff_tot_Var = "eff_" + ch.GetKey() + "_tot";
            // RooRealVar * eff_total   = (RooRealVar *) _tFile->Get(eff_tot_Var);
            // RooRealVar * lumival     = (RooRealVar *) _tFile->Get("lumi");

            RooRealVar * eff_gen = GetGeneratorEfficiencyVar(eff_gen_Var, _prj, _yy, _pol, _sample, _q2bin) ;
            if (eff_gen == nullptr) MessageSvc::Error("LoadEfficiencyForFit", (TString) "GeneratorEfficiency var", eff_gen_Var, "is nullptr", "EXIT_FAILURE");
            MessageSvc::Info("GeneratorEfficiency", eff_gen);

            RooRealVar * eff_flt = GetFilteringEfficiencyVar(eff_flt_Var, _prj, _yy, _pol, _sample );
            if (eff_flt == nullptr) MessageSvc::Error("LoadEfficiencyForFit", (TString) "FilteringEfficiency var", eff_flt_Var, "is nullptr", "EXIT_FAILURE");
            MessageSvc::Info("FilteringEfficiency", eff_flt);

            RooRealVar * eff_sel = (RooRealVar *) _tFile->Get(eff_sel_Var);
            if (eff_sel == nullptr) {
                _tFile->ls();
                MessageSvc::Error("LoadEfficiencyForFit", (TString) "SelectionEfficiency var", eff_sel_Var, "is nullptr", "EXIT_FAILURE");
            }
            // eff_gen->Print();
            // eff_flt->Print();
            // eff_sel->Print();
            MessageSvc::Info("GetSelectionEfficiency", _fileName);
            if( !_printIt) MessageSvc::Info("SelectionEfficiency (BLIND)");
            else MessageSvc::Info("SelectionEfficiency", eff_sel);            
	    
            RooRealVar * eff_total = new RooRealVar(eff_tot_Var, eff_tot_Var, 0, 0, 1);
            eff_total->setVal(eff_gen->getVal() * eff_flt->getVal() * eff_sel->getVal());
            eff_total->setError(eff_total->getVal() * TMath::Sqrt(TMath::Sq(eff_gen->getError() / eff_gen->getVal()) + TMath::Sq(eff_flt->getError() / eff_flt->getVal()) + TMath::Sq(eff_sel->getError() / eff_sel->getVal())));
            eff_total->setConstant(0);
            if (eff_total != nullptr) {
                if(_printIt){ MessageSvc::Info("TotalEfficiency", eff_total);}
                else{         MessageSvc::Warning("TotalEfficiency(BLIND)");}                
                if (eff_total->getVal() == 0) MessageSvc::Error("LoadEfficiencyForFit", (TString) "Efficiency for", eff_tot_Var, "is 0", "EXIT_FAILURE");
                if (eff_total->getError() == 0) MessageSvc::Warning("LoadEfficiencyForFit", (TString) "Efficiency for", eff_tot_Var, "is 0");
                if (isnan(eff_total->getVal()) || isnan(eff_total->getError())) MessageSvc::Error("LoadEfficiencyForFit", (TString) "Efficiency for", eff_tot_Var, "is NaN", "EXIT_FAILURE");
                _effs.push_back(eff_total->getVal());
                _errs.push_back(eff_total->getError());
            } else {
                MessageSvc::Error("LoadEfficiencyForFit", (TString) "Efficiency for", eff_tot_Var, "not available", "EXIT_FAILURE");
            }

            RooRealVar * lumival = LoadLuminosityVar("lumi", _prj, _yy, _pol);
            if (lumival != nullptr) {
                MessageSvc::Info("Luminosity", lumival);
                if (lumival->getVal() == 0) MessageSvc::Error("LoadEfficiencyForFit", (TString) "Luminosity for", eff_tot_Var, "is 0", "EXIT_FAILURE");
                if (lumival->getError() == 0) MessageSvc::Warning("LoadEfficiencyForFit", (TString) "Luminosity for", eff_tot_Var, "is 0");
                if (isnan(lumival->getVal()) || isnan(lumival->getError())) MessageSvc::Error("LoadEfficiencyForFit", (TString) "Luminosity for", eff_tot_Var, "is NaN", "EXIT_FAILURE");
                _weights.push_back(lumival->getVal() * PDG::Const::SQS.at(_yy));
            } else {
                MessageSvc::Error("LoadEfficiencyForFit", (TString) "Luminosity for", eff_tot_Var, "not available", "EXIT_FAILURE");
            }
        } else {
            MessageSvc::Error("LoadEfficiencyForFit", _fileName, "does not exist", "EXIT_FAILURE");
        }
        IOSvc::CloseFile(_tFile);
        MessageSvc::Line();
    }

    // Build efficiency to use in the fit.
    RooRealVar * _efficiency = GetAverage(_effs, _errs, _weights);
    ConfigHolder _co(_prj, _ana, _sample, _q2bin, _year, _polarity, _trg, Brem::All, _trk);
    TString _name = "eff_" + _co.GetKey();
    TString _title = "eff_{" + to_string(_prj) + SettingDef::separator + to_string(_ana) + SettingDef::separator + to_string(_trg) + "}^{" + to_string(_q2bin) + "}";
    if(_mode == "RRATIO"){
        _name+="_FULL";
        _title+="_{FULL}";
    }
    _name += TString::Format("_w%s", wConfUse.Data());
    _efficiency->SetName(_name);
    _efficiency->SetTitle(_title);
    _efficiency->setConstant(0);    
    _efficiency->setMin(0);
    _efficiency->setMax(1);   
    //Forcing Efficiency relative error to be 1% ( minUncertainty as global configuration )
    //TODO : know the systematics on this might be useful
    //Tempted to set the eps(Bkg) to be 10% by default and 1-2 % on signal mode efficiency ( adding the MC-stat one we get by default )
    
    auto _updateEfficiencyError = []( RooRealVar *var, double minUncertainty){
        MessageSvc::Debug("AddEfficiency (ERROR UPDATE)", (TString) "INCREASE UNCERTAINTY FROM", to_string(var->getError() / var->getVal() * 100) + "%", "TO", to_string(minUncertainty* 100) + "%");        
        var->setError(var->getVal() * minUncertainty);        
        if( var->getVal() - var->getError() <0.){
            MessageSvc::Warning("AddEfficiency WARNING (ERROR UPDATE)", (TString) "Would allow g-const to negative values, NOT Forcing ErrorUp/ErrorLow, expected limits on var to make the proper job") ;
        }
        if( var->getVal() + var->getError() >1.){
            MessageSvc::Warning("AddEfficiency WARNING (ERROR UPDATE)", (TString) "Would allow g-const to >1 values, should never happen!");
        }
    };
    /*
        DEVS on going, force relative errors on each fit component by hand for g-constraints later
    */
    auto _relUncertaintyForced = []( const ConfigHolder & cH , RooRealVar * var){
        auto _prj_    = cH.GetProject(); 
        auto _q2Bin_  = cH.GetQ2bin(); 
        auto _sample_ = cH.GetSample(); 
        auto _relError = var->getError() / var->getVal();
        map< std::tuple<Prj, Q2Bin, TString> , double> _relErrorsForced = {
            /*RKst, JPsi mode */
            { {Prj::RKst, Q2Bin::JPsi , "Bd2KstJPsEE"}       , SettingDef::Efficiency::minUncertainty }, 
            { {Prj::RKst, Q2Bin::JPsi , "Bd2KstJPsMM"}       , SettingDef::Efficiency::minUncertainty },
            { {Prj::RKst, Q2Bin::JPsi , "Bd2KstSwapJPsEE"}   , TMath::Sqrt( TMath::Sq(0.10) + TMath::Sq(_relError)) }, //double misID RKst (J/Psi) 10% error at least
            { {Prj::RKst, Q2Bin::JPsi , "Bd2KstSwapJPsMM"}   , TMath::Sqrt( TMath::Sq(0.10) + TMath::Sq(_relError)) }, //double misID RKst (J/Psi) 10% error at least
            { {Prj::RKst, Q2Bin::JPsi , "Bs2PhiJPsEE"}       , TMath::Sqrt( TMath::Sq(0.05) + TMath::Sq(_relError)) }, //Bs2PhiJPs misID RKst (J/Psi) 5% error at least
            { {Prj::RKst, Q2Bin::JPsi , "Bs2PhiJPsMM"}       , TMath::Sqrt( TMath::Sq(0.05) + TMath::Sq(_relError)) }, //Bs2PhiJPs misID RKst (J/Psi) 5% error at least
            { {Prj::RKst, Q2Bin::JPsi , "Lb2pKJPsEE"}        , TMath::Sqrt( TMath::Sq(0.50) + TMath::Sq(_relError)) }, //Lb2pKJPs misID RKst (J/Psi) 50% error at least
            { {Prj::RKst, Q2Bin::JPsi , "Lb2pKJPsMM"}        , TMath::Sqrt( TMath::Sq(0.50) + TMath::Sq(_relError)) }, //Lb2pKJPs misID RKst (J/Psi) 50% error at least        
            { {Prj::RKst, Q2Bin::JPsi , "Bd2DNuKstNuEE"}     , TMath::Sqrt( TMath::Sq(0.50) + TMath::Sq(_relError)) }, //Bd2DNuKstNu DSLC RKst (J/Psi) 50% error at least        
            { {Prj::RKst, Q2Bin::JPsi , "Bd2DNuKstNuMM"}     , TMath::Sqrt( TMath::Sq(0.50) + TMath::Sq(_relError)) }, //Bd2DNuKstNu DSLC RKst (J/Psi) 50% error at least         
            /*RKst, Psi mode */            
            { {Prj::RKst, Q2Bin::Psi , "Bd2KstPsiEE"}       , SettingDef::Efficiency::minUncertainty }, 
            { {Prj::RKst, Q2Bin::Psi , "Bd2KstPsiMM"}       , SettingDef::Efficiency::minUncertainty },
            { {Prj::RKst, Q2Bin::Psi , "Bd2KstSwapPsiEE"}   , TMath::Sqrt( TMath::Sq( 0.10) + TMath::Sq( _relError)) }, //double misID RKst (J/Psi) 10% error at least
            { {Prj::RKst, Q2Bin::Psi , "Bd2KstSwapPsiMM"}   , TMath::Sqrt( TMath::Sq( 0.10) + TMath::Sq( _relError)) }, //double misID RKst (J/Psi) 10% error at least
            { {Prj::RKst, Q2Bin::Psi , "Bs2PhiPsiEE"}       , TMath::Sqrt( TMath::Sq( 0.05) + TMath::Sq( _relError)) }, //Bs2PhiJPs misID RKst (J/Psi) 5% error at least
            { {Prj::RKst, Q2Bin::Psi , "Bs2PhiPsiMM"}       , TMath::Sqrt( TMath::Sq( 0.05) + TMath::Sq( _relError)) }, //Bs2PhiJPs misID RKst (J/Psi) 5% error at least
            { {Prj::RKst, Q2Bin::Psi , "Lb2pKPsiEE"}        , TMath::Sqrt( TMath::Sq( 0.50) + TMath::Sq( _relError)) }, //Lb2pKJPs misID RKst (J/Psi) 50% error at least
            { {Prj::RKst, Q2Bin::Psi , "Lb2pKPsiMM"}        , TMath::Sqrt( TMath::Sq( 0.50) + TMath::Sq( _relError)) }, //Lb2pKJPs misID RKst (J/Psi) 50% error at least
            { {Prj::RKst, Q2Bin::Psi , "Bd2KstPsiPiPiJPsEE"}, TMath::Sqrt( TMath::Sq( 0.05) + TMath::Sq( _relError)) }, //Lb2pKJPs misID RKst (J/Psi) 50% error at least
            { {Prj::RKst, Q2Bin::Psi , "Bd2KstPsiPiPiJPsMM"}, TMath::Sqrt( TMath::Sq( 0.05) + TMath::Sq( _relError)) }, //Lb2pKJPs misID RKst (J/Psi) 50% error at least
            { {Prj::RKst, Q2Bin::Psi , "Bd2DNuKstNuEE"}     , TMath::Sqrt( TMath::Sq(0.50) + TMath::Sq(_relError)) }, //Bd2DNuKstNu DSLC RKst (J/Psi) 50% error at least         
            { {Prj::RKst, Q2Bin::Psi , "Bd2DNuKstNuMM"}     , TMath::Sqrt( TMath::Sq(0.50) + TMath::Sq(_relError)) }, //Bd2DNuKstNu DSLC RKst (J/Psi) 50% error at least         
            /*RKst, low q2 mode */
            { {Prj::RKst, Q2Bin::Low , "Bd2KstEE"}       , SettingDef::Efficiency::minUncertainty }, 
            { {Prj::RKst, Q2Bin::Low , "Bd2KstJPsEE"}    , SettingDef::Efficiency::minUncertainty }, //Leakage stuff... to be cleared-out ( 2% error might suffice , TODO, make this with the FULL efficiency ratios probably? )
            { {Prj::RKst, Q2Bin::Low , "Bd2DNuKstNuEE"}  , TMath::Sqrt( TMath::Sq(0.50) + TMath::Sq(_relError)) }, //Bd2DNuKstNu DSLC RKst (J/Psi) 50% error at least         
            { {Prj::RKst, Q2Bin::Low , "Bd2KstMM"}       , SettingDef::Efficiency::minUncertainty },
            /*RKst, central q2 mode */
            { {Prj::RKst, Q2Bin::Central , "Bd2KstEE"}      , _relError } , 
            { {Prj::RKst, Q2Bin::Central , "Bd2KstJPsEE"}   , TMath::Sqrt( TMath::Sq(0.20) + TMath::Sq(_relError))  }, //Leakage stuff... to be cleared-out ( 2% error might suffice , TODO, make this with the FULL efficiency ratios probably? )
            { {Prj::RKst, Q2Bin::Central , "Bd2DNuKstNuEE"} , TMath::Sqrt( TMath::Sq(0.50) + TMath::Sq(_relError)) }, //Bd2DNuKstNu DSLC RKst (J/Psi) 50% error at least         
            { {Prj::RKst, Q2Bin::Central , "Bd2KstMM"}      , _relError },

            /*RK, J/Psi mode */            
            { {Prj::RK,   Q2Bin::JPsi , "Bu2KJPsEE"}  , _relError  }, //Do nothing , keep as it is
            { {Prj::RK,   Q2Bin::JPsi , "Bu2KJPsMM"}  , _relError }, //Do nothing , keep as it is
            { {Prj::RK,   Q2Bin::JPsi , "Bu2PiJPsEE"} , TMath::Sqrt( TMath::Sq(0.05) + TMath::Sq( _relError)) }, //Bu2PiJPs misID RK (J/Psi) 50% add up a 5% relative error to baseline
            { {Prj::RK,   Q2Bin::JPsi , "Bu2PiJPsMM"} , TMath::Sqrt( TMath::Sq(0.05) + TMath::Sq( _relError)) }, //Bu2PiJPs misID RK (J/Psi) 50% add up a 5% relative error to baseline
            /*RK, Psi mode */            
            { {Prj::RK,   Q2Bin::Psi , "Bu2KPsiEE"}  , _relError  }, //Lb2pKJPs misID RKst (J/Psi) 50% error at least
            { {Prj::RK,   Q2Bin::Psi , "Bu2KPsiMM"}  , _relError  }, //Lb2pKJPs misID RKst (J/Psi) 50% error at least
            { {Prj::RK,   Q2Bin::Psi , "Bu2PiPsiEE"} , TMath::Sqrt( TMath::Sq(0.05) + TMath::Sq(_relError) )  }, //Lb2pKJPs misID RKst (J/Psi) 50% error at least
            { {Prj::RK,   Q2Bin::Psi , "Bu2PiPsiMM"} , TMath::Sqrt( TMath::Sq(0.05) + TMath::Sq(_relError) )  }, //Lb2pKJPs misID RKst (J/Psi) 50% error at least      
            { {Prj::RK,   Q2Bin::Psi , "Bu2KPsiPiPiJPsEE"}, TMath::Sqrt( TMath::Sq(0.05) + TMath::Sq(_relError) )}, //Bu2KPsiPiPiJPs 5% error at least
            { {Prj::RK,   Q2Bin::Psi , "Bu2KPsiPiPiJPsMM"}, TMath::Sqrt( TMath::Sq(0.05) + TMath::Sq(_relError) )}, //Bu2KPsiPiPiJPs 5% error at least
            /*RK, low q2 mode */
            { {Prj::RK, Q2Bin::Low , "Bu2KEE"}       , _relError }, 
            { {Prj::RK, Q2Bin::Low , "Bu2KJPsEE"}    , _relError }, //Leakage stuff... to be cleared-out ( 2% error might suffice , TODO, make this with the FULL efficiency ratios probably? )
            { {Prj::RK, Q2Bin::Low , "Bd2KstEE"}     , _relError }, //Leakage stuff... to be cleared-out ( 2% error might suffice , TODO, make this with the FULL efficiency ratios probably? )
            { {Prj::RK, Q2Bin::Low , "Bu2KstEE"}     , _relError }, //Leakage stuff... to be cleared-out ( 2% error might suffice , TODO, make this with the FULL efficiency ratios probably? )
            { {Prj::RK, Q2Bin::Low , "Bu2KMM"}       , _relError },
            /*RK, central q2 mode */
            { {Prj::RK, Q2Bin::Central , "Bu2KEE"}     , _relError }, 
            { {Prj::RK, Q2Bin::Central , "Bu2KJPsEE"}  , TMath::Sqrt( TMath::Sq(0.20) + TMath::Sq(_relError) ) }, //Leakage stuff ( a 25% relative uncertainty to make it float )... to be cleared-out ( 2% error might suffice , TODO, make this with the FULL efficiency ratios probably and constraint using SMEARING on JPsi--> this for sure must be done? )
            { {Prj::RK, Q2Bin::Central , "Bd2KstEE"}   , _relError }, //Leakage stuff... to be cleared-out ( 2% error might suffice , TODO, make this with the FULL efficiency ratios probably? )
            { {Prj::RK, Q2Bin::Central , "Bu2KstEE"}   , _relError }, //Leakage stuff... to be cleared-out ( 2% error might suffice , TODO, make this with the FULL efficiency ratios probably? )            
            { {Prj::RK, Q2Bin::Central , "Bu2KMM"}     , _relError },  
        };
        auto _relUnctowerBound = _relErrorsForced.at( std::make_tuple( _prj_, _q2Bin_, _sample_));
        if(_relError > _relUnctowerBound){
            return _relError;
        }else{
            return _relUnctowerBound;
        }
        return _relError;
    };

    
    //DEAL with the relative uncertainties on those efficiencies!
    if( _printIt){
        MessageSvc::Info("LoadEfficiencyForFit", _efficiency);
    }else{
        MessageSvc::Debug("LoadEfficiencyForFit(BLIND) rel. uncerainty is", _efficiency);
    }
    if(_mode != "RRATIO"){
        //Only do this for "eps(Signal or Backgrounds) entering background constraints!"       
        if( SettingDef::Efficiency::option.Contains("forceEpsErrorsBkg")){
            //TODO : devs properly and coverage test on fits to do... (option not used anywhere so far)
            MessageSvc::Warning("AddEfficiency", (TString) "ForceEpsErrorsBkg mode");
            auto _minUncertainty = _relUncertaintyForced( _configHolder, _efficiency );
            if (_efficiency->getError() / _efficiency->getVal() < _minUncertainty) {
                std::cout<< "Old relative error "<< 100 * _efficiency->getError()/_efficiency->getVal()<<std::endl;
                _updateEfficiencyError(  _efficiency, _minUncertainty );
                std::cout<< "New relative error "<< 100 * _efficiency->getError()/_efficiency->getVal()<<std::endl;
            }else{
                MessageSvc::Warning("AddEfficiency(BKGOVERSIGNAL MODE)", (TString) "Keep relative uncertainty as it is");
            }
        }else{
            //================================================================================================================
            // Baseline by default. Relies on the global setting on SettingDef::Efficiency::minUncertainty (RRATIO parameters)
            //================================================================================================================
            if (_efficiency->getError() / _efficiency->getVal() < SettingDef::Efficiency::minUncertainty) {
                MessageSvc::Warning("AddEfficiency(BKGOVERSIG MODE)", TString::Format("Forcing Relative uncertainty to %.2f per-cent", SettingDef::Efficiency::minUncertainty) );
                std::cout<< "Old relative error "<< 100 * _efficiency->getError()/_efficiency->getVal()<<std::endl;
                _updateEfficiencyError(  _efficiency, SettingDef::Efficiency::minUncertainty );
                std::cout<< "New relative error "<< 100 * _efficiency->getError()/_efficiency->getVal()<<std::endl;
            }else{
                MessageSvc::Warning("AddEfficiency(BKGOVERSIG MODE)", (TString) "Keep relative uncertainty as it is");
            }
            if( _configHolder.IsLeakageSample()){
                //We deal with leakage components , we force the Minimum relative uncertainty to x % ( to do configurable )
                MessageSvc::Warning("AddEfficiency(BKGOVERSIG MODE) Leakage", (TString) "Force Eps Leakage component in rare mode");
                // auto _minUncertainty = 0.15; //15% uncertainty on leakage
                auto _minUncertainty = _relUncertaintyForced( _configHolder, _efficiency );
                std::cout<< "Old relative error "<< 100 * _efficiency->getError()/_efficiency->getVal()<<std::endl;
                _updateEfficiencyError( _efficiency, _minUncertainty);
                std::cout<< "New relative error "<< 100 * _efficiency->getError()/_efficiency->getVal()<<std::endl;
            }            
        }
    }else{
        if( _configHolder.IsLeakageSample()){
            //We deal with leakage components , we force the Minimum relative uncertainty to x % ( to do configurable )
            MessageSvc::Warning("AddEfficiency(RRATIO MODE) Leakage", (TString) "Force Eps Leakage component in rare mode to 20 % uncertainty");
            // auto _minUncertainty = 0.15; //15% uncertainty on leakage
            auto _minUncertainty = _relUncertaintyForced( _configHolder, _efficiency );
            std::cout<< "Old relative error "<< 100 * _efficiency->getError()/_efficiency->getVal()<<std::endl;
            _updateEfficiencyError( _efficiency, _minUncertainty);
            std::cout<< "New relative error "<< 100 * _efficiency->getError()/_efficiency->getVal()<<std::endl;
        }                    
    }
    if(_printIt) MessageSvc::Info("LoadEfficiencyForFit", _efficiency);
    else MessageSvc::Info("LoadEfficiencyForFit(BLIND)");
    
    MessageSvc::Line();


    //Reset global tags...
    SettingDef::Efficiency::ver = _BASEEVER;

    MessageSvc::Warning("Applying scale factor to efficiency value error to let the minimizer digesting the covariance matrix, scale and retweak max", to_string( SettingDef::Efficiency::scaleEfficiency) );
    MessageSvc::Warning("Raw efficiency relative error (before) is  ", TString::Format( "%.3f per-cent" ,100 * _efficiency->getError()/_efficiency->getVal() ));
    // std::cout<<RED << " RAW EFFICIENCY IS " << _efficiency->getVal() << " +/- "  << _efficiency->getError() << RESET <<  std::endl;
    double old_relative = _efficiency->getError()/_efficiency->getVal();
    //Scale efficiencies by 1000.
    double min = 0; 
    double max = 1.0 * SettingDef::Efficiency::scaleEfficiency;
    double val = _efficiency->getVal() * SettingDef::Efficiency::scaleEfficiency;
    double err = old_relative * val;
    
    _efficiency->setRange( min, max);
    _efficiency->setVal(val);
    _efficiency->setError(err);

    MessageSvc::Warning("Raw efficiency relative error (after) is ", TString::Format( "%.3f per-cent" ,100 * _efficiency->getError()/_efficiency->getVal() ));
    return _efficiency;
}

bool IsEfficienciesCorrelated(const ConfigHolder & _configA, const ConfigHolder & _configB) noexcept {
    bool _isCorrelated = _configA.IsSignalMCEfficiencySample() && _configB.IsSignalMCEfficiencySample();
    return _isCorrelated;
}

double GetVariance(const ConfigHolder & _config) noexcept {     
    if( SettingDef::Efficiency::option.Contains("noCov")){     
        double covX = GetCovariance( _config,_config);
        return covX;
    }   
    MessageSvc::Warning("GetCovariance, scaling up the efficiency results by scale factors to make cov-matrix constraint less brutally small");
    double CovXY = GetCovariance( _config,_config);    
    return CovXY; 
}

double GetCovariance(const ConfigHolder & _configA, const ConfigHolder & _configB) noexcept { 
    //Suppress covariances between runs for the Bootstrapping errors
    bool _sameYear =  _configA.GetYear() == _configB.GetYear();
    bool _sameAna  =  _configA.GetAna()  == _configB.GetAna();
    bool _sameTrg  =  _configA.GetTrigger()  == _configB.GetTrigger();
    bool _samePrj  =  _configA.GetProject()  == _configB.GetProject();
    bool _sameQ2   =  _configA.GetQ2bin()  == _configB.GetQ2bin();

    if(  SettingDef::Efficiency::option.Contains("noRunCov") && !_sameYear ){ return 0;} //off-diagonal elements for run i,j with i!=j , set to 0
    if(  SettingDef::Efficiency::option.Contains("noAnaCov") && !_sameAna  ){ return 0;} //off-diagonal elements for ana i,j with i!=j , set to 0
    if(  SettingDef::Efficiency::option.Contains("noQ2Cov")  && !_sameQ2   ){ return 0;} //off-diagonal elements for q2 i,j with i!=j , set to 0
    if(  SettingDef::Efficiency::option.Contains("noPrjCov") && !_samePrj  ){ return 0;} //off-diagonal elements for prj i,j with i!=j , set to 0
    if(  SettingDef::Efficiency::option.Contains("noTrgCov") && !_sameTrg  ){ return 0;} //off-diagonal elements for prj i,j with i!=j , set to 0


    //TODO : more protections for what to make in the covariance . Has to be checked on sample as well
    if( !( _configA.IsSignalMC() && _configB.IsSignalMC())) MessageSvc::Error("GetCovariance can be called only between 2 Signal ConfigHolders", "","EXIT_FAILURE"); 
    /*
        Allowed efficiencies for covariance matrix calculation
    */
    auto CheckValid = [](const ConfigHolder & cH ){
        std::map< tuple< Prj, Q2Bin, Analysis> , vector<TString> >  _AllowedInCovariance{ 
            { { Prj::RK, Q2Bin::JPsi,      Analysis::EE }, { "Bu2KJPsEE"} },
            { { Prj::RK, Q2Bin::Psi,       Analysis::EE }, { "Bu2KPsiEE"} },
            { { Prj::RK, Q2Bin::Low,       Analysis::EE }, { "Bu2KEE"} },
            { { Prj::RK, Q2Bin::Central,   Analysis::EE }, { "Bu2KEE"  , "Bu2KJPsEE"} },//Leakage allowed for covariance! 
            { { Prj::RKst, Q2Bin::JPsi,    Analysis::EE }, { "Bd2KstJPsEE"} },
            { { Prj::RKst, Q2Bin::Psi,     Analysis::EE }, { "Bd2KstPsiEE"} },
            { { Prj::RKst, Q2Bin::Low,     Analysis::EE }, { "Bd2KstEE"} },
            { { Prj::RKst, Q2Bin::Central, Analysis::EE }, { "Bd2KstEE", "Bd2KstJPsEE"} },//Leakage allowed for covariance! 
            { { Prj::RK, Q2Bin::JPsi,      Analysis::MM }, { "Bu2KJPsMM"} },
            { { Prj::RK, Q2Bin::Psi,       Analysis::MM }, { "Bu2KPsiMM"} },
            { { Prj::RK, Q2Bin::Low,       Analysis::MM }, { "Bu2KMM"} },
            { { Prj::RK, Q2Bin::Central,   Analysis::MM }, { "Bu2KMM"} },
            { { Prj::RKst, Q2Bin::JPsi,    Analysis::MM }, { "Bd2KstJPsMM"} },
            { { Prj::RKst, Q2Bin::Psi,     Analysis::MM }, { "Bd2KstPsiMM"} },
            { { Prj::RKst, Q2Bin::Low,     Analysis::MM }, { "Bd2KstMM"} },
            { { Prj::RKst, Q2Bin::Central, Analysis::MM }, { "Bd2KstMM"} }              
            /* 
            TODO : enable it for RPhi 
            { { Prj::RPhi, Q2Bin::JPsi,      Analysis::EE }, { "Bs2PhiJPsEE"} },
            { { Prj::RPhi, Q2Bin::Psi,       Analysis::EE }, { "Bs2PhiPsiEE"} },
            { { Prj::RPhi, Q2Bin::Low,       Analysis::EE }, { "Bs2PhiEE"} },
            { { Prj::RPhi, Q2Bin::Central,   Analysis::EE }, { "Bs2PhiEE"} },
            { { Prj::RPhi, Q2Bin::JPsi,      Analysis::MM }, { "Bs2PhiJPsMM"} },
            { { Prj::RPhi, Q2Bin::Psi,       Analysis::MM }, { "Bs2PhiPsiMM"} },
            { { Prj::RPhi, Q2Bin::Low,       Analysis::MM }, { "Bs2PhiMM"} },
            { { Prj::RPhi, Q2Bin::Central,   Analysis::MM }, { "Bs2PhiMM"} }, 
            */
        };
        auto this_slot = make_tuple<Prj, Q2Bin, Analysis>( cH.GetProject(), cH.GetQ2bin(), cH.GetAna());
        if( _AllowedInCovariance.find( this_slot) == _AllowedInCovariance.end()){
            cH.PrintInline();
            MessageSvc::Error("Prj,Q2Bin,Ana not allowed in covariance matrix calculation (only signal)", "","EXIT_FAILURE");
        }
        if( ! CheckVectorContains( _AllowedInCovariance.at(this_slot), cH.GetSample())){
            cH.PrintInline();
            MessageSvc::Error("Sample for Covariance matrix not allowed", (TString)"Efficiency must be computed also for this sample (not at the moment)","EXIT_FAILURE");            
        }
        return true;
    };
    /*
        This is the covariance matrix calculation you do with BS-efficiency results
        This is not the end of the story, we have to combine several Covariances to have a proper handling 
        Expecially important when combining R1+R2p1+R2p2 fits
        TODO : pre-compute covariances and load/combine them here 
        Move this code to some standalone computer of covariances , being this part only the one dealing with BS-efficiencies,
        i.e efficienciencies and correlations due to statistical MC sample size
    */
    CheckValid(_configA);
    CheckValid(_configB);
    Prj         A_prj      = _configA.GetProject();
    Analysis    A_ana      = _configA.GetAna();
    Q2Bin       A_q2bin    = _configA.GetQ2bin();
    TString     A_sample   = _configA.GetSample();
    Polarity    A_polarity = _configA.GetPolarity();
    Year        A_year     = _configA.GetYear();
    Trigger     A_trg      = _configA.GetTrigger();
    TriggerConf A_trgConf  = _configA.GetTriggerConf();
    Track       A_trk      = _configA.GetTrack();

    Prj         B_prj      = _configB.GetProject();
    Analysis    B_ana      = _configB.GetAna();
    Q2Bin       B_q2bin    = _configB.GetQ2bin();
    TString     B_sample   = _configB.GetSample();
    Polarity    B_polarity = _configB.GetPolarity();
    Year        B_year     = _configB.GetYear();
    Trigger     B_trg      = _configB.GetTrigger();
    TriggerConf B_trgConf  = _configB.GetTriggerConf();
    Track       B_trk      = _configB.GetTrack();

    //Flag to re-use same file , no make Friends. 1 EffTuple should contains directly L0I, L0L, L0L(inc), 
    //no need to chain anything if EE-EE, L0X-L0X, 
    //SameYear, Same Polarity, Same Q2Bin, Same Project
    bool _isSame = (( A_prj == B_prj) &&  
                    ( A_ana == B_ana) && 
                    ( A_year == B_year) && 
                    ( A_polarity == B_polarity) && 
                    ( A_q2bin == B_q2bin) && 
                    ( A_sample == B_sample)); 


    if( SettingDef::Efficiency::option.Contains("noCov") && !_isSame){
        return 0;
    }

    //=== Retrieve EfficiencyInfo to grab with RRatio settings, get the Signal Full correction weights
    auto infoA = SettingDef::Efficiency::fitconfiguration.GetEfficiencyInfo( A_prj,A_q2bin, "RRATIO");
    auto infoB = SettingDef::Efficiency::fitconfiguration.GetEfficiencyInfo( B_prj,B_q2bin, "RRATIO");
    //We have to make a TChain with friends here 

    if( SettingDef::Efficiency::fitconfiguration.CovTuple() != ""){
        MessageSvc::Warning("Loading covariance value from pre-computed ntuple");
        TString _covTuple =  SettingDef::Efficiency::fitconfiguration.CovTuple();
        TFile *_fileCovariance = IOSvc::OpenFile(_covTuple , OpenMode::READ);
        auto *tree = _fileCovariance->Get<TTree>("CovTuple");
        if( tree==nullptr){
            MessageSvc::Error("Cannot read TTree 'CovTuple' from " , _covTuple, "EXIT_FAILURE");
        }
        ROOT::RDataFrame df(*tree);
        //Branches are named as eff_Bd2KstJPsEE_RKst_EE_central_L0I_exclusive_R1
        TString _IDA = _configA.GetKey("addtrgconf-addsample").ReplaceAll("-", "_");
        TString _IDB = _configB.GetKey("addtrgconf-addsample").ReplaceAll("-", "_");
        TString _branchA = TString::Format("eff_%s", _IDA.Data());
        TString _branchB = TString::Format("eff_%s", _IDB.Data());
        auto covi = Covariance<double>();   
        MessageSvc::Line();         
        MessageSvc::Info("GetCovariance", TString::Format("Computing Covariance for %s", _branchA.Data()));
        MessageSvc::Info("GetCovariance", TString::Format("Computing Covariance for %s", _branchB.Data()));
        auto covXY = df.Book<double,double>(std::move(covi), {_branchA.Data(), _branchB.Data() });
        double cov = covXY.GetValue() * SettingDef::Efficiency::scaleEfficiency * SettingDef::Efficiency::scaleEfficiency;
        IOSvc::CloseFile(_fileCovariance);
        MessageSvc::Line();
        return cov;
    }
    MessageSvc::Info("Computing covariance on the fly");

    
    auto ID2Conf = _configA.GetKey("addtrgconf-addsample")+"_"+_configB.GetKey("addtrgconf-addsample")+".root"; 
    ROOT::DisableImplicitMT(); 
    ROOT::RDataFrame df(100);
    df.Define("bsIDX", "rdfentry_").Snapshot("CovTuple", ID2Conf.Data());

    TFile f(ID2Conf.Data());
    TTree *EfficiencyTree  = f.Get<TTree>("CovTuple") ;
    
    struct MappedElements{ 
        TString FileName;
        TString TreeName;
        TString BranchName;
        pair<double,double> Lumi;
        pair<double,double> Gen;
        pair<double,double> Flt;
        double LumiNormTot =1.;
        MappedElements() = default;        
        MappedElements( const TString & fileName, 
                        const TString & treeName, 
                        const TString & branchName, 
                        const pair<double, double> & lumiValWeight, 
                        const pair<double, double> & epsGen , 
                        const pair<double, double> & epsFiltering ){
            
            FileName = fileName;
            TreeName = treeName;
            BranchName = branchName;
            Lumi = lumiValWeight;
            Gen  = epsGen;
            Flt  = epsFiltering;
            LumiNormTot =1.;
        };
        TString GetExpression( TString aliasID) const{
            TString _lumiWeight = TString::Format( "(%f/%f)", lumi().first, LumiNormTot);
            TString _BranchName = TString::Format("%s.%s", aliasID.Data(), branchName().Data());
            TString _epsGeoFlt  = TString::Format("(%f * %f)", gen().first, flt().first);
    	    //Expression for chaining Vector columns is epsGeo * epsFlt * EfficiencyBranchVector
            TString _expression = TString::Format("%s * %s * %s", _lumiWeight.Data(), _epsGeoFlt.Data(), _BranchName.Data() );
            return _expression;
        };
        void SetNormLumiVal( double totLumi){
            LumiNormTot = totLumi;
        };        
        TString fileName()const{        return FileName;}
        TString treeName()const{        return TreeName;}
        TString branchName()const{      return BranchName;}
        pair<double,double> lumi()const{return Lumi;}
        pair<double,double> gen()const{ return Gen;}
        pair<double,double> flt()const{ return Flt;}
        void Print()const{
            std::cout<<GREEN<<"*) InputFile  : "<< fileName()<<"::"<<treeName()<<RESET<<endl;
            std::cout<<GREEN<<"*) BranchName : "<<branchName()<<RESET<<std::endl;
            std::cout<<YELLOW<<"*) eps(gen)  : "<<100*gen().first    <<" +/- "<< 100*gen().second << " %" RESET<<std::endl;
            std::cout<<YELLOW<<"*) eps(flt)  : "<<100*flt().first    <<" +/- "<< 100*flt().second << " %" RESET<<std::endl;
            std::cout<<YELLOW<<"*) Lumi(w)   : "<<lumi().first <<" +/- "<< lumi().second << " 1/pb ; normed with "<< LumiNormTot <<  RESET<<std::endl;
        };
    };
    auto FillMap = []( const ConfigHolder & _config, const InfoEff & info ){
        map< TString , MappedElements > _TUPLEADD;
        Prj         _prj      = _config.GetProject();
        Analysis    _ana      = _config.GetAna();
        Q2Bin       _q2bin    = _config.GetQ2bin();
        TString     _sample   = _config.GetSample();
        Polarity    _polarity = _config.GetPolarity();
        Year        _year     = _config.GetYear();
        Trigger     _trg      = _config.GetTrigger();
        TriggerConf _trgConf  = _config.GetTriggerConf();
        Track       _trk      = _config.GetTrack();        
        auto wOpt  = info.wOption();
        auto wConf = info.wConfig();
        auto eVer  = info.effVersion();   
        TString _gather= TString::Format("effnorm_%s_%s_w%s", to_string(_trg).Data() , to_string(_trgConf).Data(), wConf.Data());
        vector< TString > _Polarities = GetPolarities( to_string(_polarity));
        vector< TString > _Years      = GetYears( to_string(_year));
        vector< pair< Year, Polarity > > _yearsAndPolarities;
        for (const auto && [yy, pp] : iter::product(_Years, _Polarities)) {
            _yearsAndPolarities.push_back(make_pair<Year,Polarity>(hash_year(yy), hash_polarity(pp)));
        }
        if (_Years.size() != 1)      MessageSvc::Warning("GetCovariance::FillMap", (TString) "Combine", to_string(_Years.size()), "years");
        if (_Polarities.size() != 1) MessageSvc::Warning("GetCovariance::FillMap", (TString) "Combine", to_string(_Polarities.size()), "polarities");
        //TODO : SET AND RESET AT THE END! 
        auto _BASEEVER = SettingDef::Efficiency::ver;
        SettingDef::Efficiency::ver = eVer;
        double _lumiTotWeight = 0.;
        vector< pair< TString, TString> > _remappedAliasTupleBranch; 
        for (auto & _yearAndPolarity : _yearsAndPolarities) {
            auto _yy      = _yearAndPolarity.first;
            auto _pol     = _yearAndPolarity.second;
            auto _dirName = IOSvc::GetTupleDir("eff", to_string(_prj), to_string(_ana), to_string(_q2bin), to_string(_yy), to_string(_trg));
            // auto _effTuple= _dirName+ "/" + to_string(_yy) + to_string(_pol)+"/EffTuple.root";       
            //!!!!!!!!!! NEW NAMING SCHEME FOR LEAKAGE NEEDED ! !!!!!!! 
            auto _effTuple= _dirName+ "/" + to_string(_yy) + to_string(_pol)+"/"+_sample+"_EffTuple.root";
            if( !IOSvc::ExistFile(_effTuple)){
                MessageSvc::Error("GetCovariance (file not exist)", _effTuple, "EXIT_FAILURE");
            }
            TString ID = TString::Format( "%s_%s_%s_%s_%s_%s",_sample.Data(), 
                                                              to_string(_prj).Data(), 
                                                              to_string(_q2bin).Data(), 
                                                              to_string(_ana).Data(), 
                                                              to_string(_yy).Data(), 
                                                              to_string(_pol).Data());
            TString _tupleName =  wOpt;
            _tupleName = _tupleName.ReplaceAll("-","_");     
            if( _TUPLEADD.find( ID) != _TUPLEADD.end() ){ 
                MessageSvc::Error("Adding tuple already in with ID", ID,"EXIT_FAILURE");
            }else{
                MessageSvc::Info("Adding ", ID, "to map");
            }  
            auto _lumi = LoadLuminosity( _prj,  _yy, _pol, false, true);
            _lumi.first *=PDG::Const::SQS.at(_yy);
            _lumi.second*=PDG::Const::SQS.at(_yy);
            _lumiTotWeight+= _lumi.first;
            _TUPLEADD[ID] = MappedElements( 
                _effTuple,
                _tupleName,
                _gather,
                _lumi, /*pair< double, double > LoadLuminosity(const Prj & _project, const Year & _year, const Polarity & _polarity, bool _debug, bool _silent)*/
                GetGeneratorEfficiency(_prj ,  _yy, _pol, _sample, _q2bin), 
                GetFilteringEfficiency(_prj ,  _yy, _pol, _sample)
            );
        }
        for( auto & el : _TUPLEADD){
            el.second.SetNormLumiVal( _lumiTotWeight);
        }
        SettingDef::Efficiency::ver = _BASEEVER;
        return _TUPLEADD;
    };
    /*
        Jpsi_Muon_L0I  (R1)       
        eps_sel = vector<100>  
        eps_flt = double 
        eps_gen = double 
        lumi(11-MD)
        eps_ vector * eps_flt * eps_gen * lumi / sum(Lumi)
    */
    auto join = []( const vector<TString> vectorString , TString _delimiter){
        TString el = vectorString.at(0);
        for( int i =1; i < vectorString.size(); ++i){
            el = TString::Format("%s %s %s", el.Data(), _delimiter.Data(), vectorString.at(i).Data());
        }
        return el;
    };
    std::cout<<RED<<"FillMap A"<<RESET<<std::endl;    
    auto mapA = FillMap(_configA, infoA);
    vector<TString> _expressionAveragingA; 
    for( const auto & el : mapA){
        MessageSvc::Line();
        std::cout<<RED<<"- ID :" << el.first<< std::endl;
        el.second.Print();
        MessageSvc::Line();    
        TString friend_aliasing= TString::Format("%s = %s", el.first.Data(), el.second.treeName().Data());
        MessageSvc::Warning("AddFriend", friend_aliasing, el.second.fileName());
        EfficiencyTree->AddFriend( friend_aliasing, el.second.fileName());
        _expressionAveragingA.push_back( el.second.GetExpression( el.first));
    }
    TString _exprA = join( _expressionAveragingA , " + ");
    std::cout<<RED<<"FillMap B"<<RESET<<std::endl;    
    auto mapB = FillMap(_configB, infoB);
    MessageSvc::Line(); 
    vector<TString> _expressionAveragingB; 
    for( const auto & el : mapB){
        MessageSvc::Line();
        std::cout<<RED<<"- ID :" << el.first<< std::endl;
        el.second.Print();
        MessageSvc::Line();    
        TString friend_aliasing= el.first +" = "+el.second.treeName();
        MessageSvc::Warning("AddFriend", friend_aliasing, el.second.fileName());
        if( !_isSame  ){
            EfficiencyTree->AddFriend( friend_aliasing, el.second.fileName());
        }
        _expressionAveragingB.push_back( el.second.GetExpression( el.first));
    }
    TString _exprB = join( _expressionAveragingB , " + ");
    MessageSvc::Info("Making computation of cov(a,b)");
    ROOT::RDataFrame dfFinal( *EfficiencyTree);
    for( auto & e : dfFinal.GetColumnNames()){
        cout<< e<< endl;
    };
    MessageSvc::Warning("Define(A)",_exprA );
    MessageSvc::Warning("Define(B)",_exprB );
    auto covi = Covariance<double>();            
    auto covXY = dfFinal.Define("EpsA", _exprA.Data())
                        .Define("EpsB", _exprB.Data())
                        .Book<double,double>(std::move(covi), {"EpsA", "EpsB"});
    IOSvc::RemoveFile(ID2Conf.Data());
    dfFinal.Snapshot("CovTupleFinal", ID2Conf.Data());
    return covXY.GetValue() * SettingDef::Efficiency::scaleEfficiency * SettingDef::Efficiency::scaleEfficiency;
}

pair< double, double > LoadLuminosity(const Prj & _project, const Year & _year, const Polarity & _polarity, bool _debug, bool _silent) noexcept {
    TString _file = fmt::format("{0}/luminosity/v{1}/LumiInfo_{2}.yaml", SettingDef::IO::gangaDir,GetBaseVer(SettingDef::Tuple::gngVer), to_string(_project));
    if (!_silent) { MessageSvc::Info(Color::Cyan, "LoadLuminosity", (TString) fmt::format("{0} - {1} -- {2}", to_string(_year), to_string(_polarity), _file)); }

    YAML::Node _parser = YAML::LoadFile(_file.Data());
    if (_parser.IsNull()) MessageSvc::Error("LoadLuminosity", (TString) "Invalid", _file, "parser", "EXIT_FAILURE");
    if (_debug) {
        cout << YELLOW;
        MessageSvc::Line();
        MessageSvc::Debug("LoadLuminosity", (TString) "Nodes =", to_string(_parser.size()));
        for (YAML::iterator _it = _parser.begin(); _it != _parser.end(); ++_it) { MessageSvc::Debug("LoadLuminosity", (TString) "SubNodes =", to_string(_it->second.size())); }
        cout << YELLOW;
        cout << _parser << endl;
        MessageSvc::Line();
        cout << RESET;
    }

    auto _years      = GetYears(to_string(_year));
    auto _polarities = GetPolarities(to_string(_polarity));

    map< pair< Year, Polarity >, pair< double, double > > _collected;

    double _integratedLumi    = 0.;
    double _integratedLumiErr = 0.;
    for (auto & _yy : _years) {
        for (auto & _pp : _polarities) {
            if (_debug) MessageSvc::Debug("LoadLuminosity", (TString) "Parsing year =", _yy, ", polarity = ", _pp);
            YAML::Node _yearNode = _parser[_yy.Data()][_pp.Data()];
            if (!_yearNode) continue;
            if (_debug) cout << YELLOW << "LoadLuminosity Year = " << _yy << " Polarity = " << _pp << "   Value = " << _yearNode["value"] << " +/- " << _yearNode["err"] << RESET << endl;
            _collected[make_pair(hash_year(_yy), hash_polarity(_pp))] = make_pair(0., 0.);
            _integratedLumi += _yearNode["value"].as< double >();
            _integratedLumiErr += _yearNode["err"].as< double >();
        }
    }
    if (_collected.size() != _years.size() * _polarities.size()) { MessageSvc::Error("LoadLuminosity", (TString) "Issue in collecting Luminosity", "EXIT_FAILURE"); }

    // MessageSvc::Info("LoadLuminosity", (TString) fmt::format("({0} +/- {1}) fb-1", _integratedLumi / 1000, _integratedLumiErr / 1000));
    return make_pair(_integratedLumi, _integratedLumiErr);
}

pair< double, double > LoadLuminosity(const TString & _project, const TString & _year, const TString & _polarity, bool _debug, bool _silent) noexcept { return LoadLuminosity(hash_project(_project), hash_year(_year), hash_polarity(_polarity), _debug, _silent); }

RooRealVar * LoadLuminosityVar(const TString & _varName, const Prj & _prj, const Year & _year, const Polarity & _polarity, bool _debug) noexcept {
    pair< double, double > _results = LoadLuminosity(_prj, _year, _polarity, _debug);
    RooRealVar *           _lumi    = new RooRealVar(_varName, _varName, _results.first);
    _lumi->setError(_results.second);
    _lumi->setRange(0, 1);
    if ((_lumi->getVal() == 0) || (_lumi->getError() == 0)) MessageSvc::Warning("LoadLuminosityVar", _varName, "is 0");
    if (isnan(_lumi->getVal()) || isnan(_lumi->getError())) MessageSvc::Error("LoadLuminosityVar", _varName, "is NaN", "EXIT_FAILURE");
    return _lumi;
}

double GetMCTEntries(const Prj & _project, const Year & _year, const Polarity & _polarity, const TString & _sample, bool _force, TString _type, bool _debug) noexcept {    
    TString _file = fmt::format("{0}/mct/v{1}/{2}/{3}.yaml", SettingDef::IO::gangaDir, GetBaseVer(SettingDef::Tuple::gngVer), to_string(_project), _sample);
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "GetMCTEntries", (TString) fmt::format("{0} - {1} - {2} -- {3}", to_string(_project), to_string(_year), to_string(_polarity), _file));
    YAML::Node _parser = YAML::LoadFile(_file.Data());
    if (_parser.IsNull()) MessageSvc::Error("GetMCTEntries", (TString) "Invalid", _file, "parser", "EXIT_FAILURE");
    if (_debug) {
        cout << YELLOW;
        MessageSvc::Line();
        MessageSvc::Debug("GetMCTEntries", (TString) "Nodes =", to_string(_parser.size()));
        for (YAML::iterator _it = _parser.begin(); _it != _parser.end(); ++_it) { MessageSvc::Debug("GetMCTEntries", (TString) "SubNodes =", to_string(_it->second.size())); }
        cout << YELLOW;
        cout << _parser << endl;
        MessageSvc::Line();
        cout << RESET;
    }
    vector<TString>   _BLACKLISTYEAR = {};
    if( _sample == "Bd2XJPsEE" && _project == Prj::RK){
        /*
        This sample is bugged in 15,16 year TRUEIDS == 0, bad efficiencies and selections based on TRUEIDS if used. Bugged for 15-16 only RK
        */
        _BLACKLISTYEAR = {"15","16", "R2p1"};
    }
    if( _sample == "Bs2XJPsEE"){
        /*
        This sample is bugged in 15,16 year TRUEIDS == 0, bad efficiencies and selections based on TRUEIDS if used. Bugged for 15-16 both RK/RKst
        */
        _BLACKLISTYEAR = {"15","16", "R2p1"};
    }    
    auto _years      = GetYears(to_string(_year));
    auto _polarities = GetPolarities(to_string(_polarity));

    map< pair< Year, Polarity >, pair< double, double > > _collected;

    double _entries = 0.;
    for (auto & _yy : _years) {
        if(CheckVectorContains(_BLACKLISTYEAR, _yy)){
            MessageSvc::Warning("GetMCTEntries", (TString) "Done with a bugged sample, this must not happen, fix it, count it as 0",_yy );
            continue;
            // MessageSvc::Error("GetMCTEntries", (TString) "Done with a bugged sample, this must not happen, fix it", "EXIT_FAILURE" );
        }

        for (auto & _pp : _polarities) {
            if (_debug) MessageSvc::Debug("GetMCTEntries", (TString) "Parsing year =", _yy, ", polarity = ", _pp);
            YAML::Node _yearNode = _parser[_yy.Data()][_pp.Data()];
            if (!_yearNode) continue;
            const int _nTotal = _yearNode[_type].as< int >();
            if (_debug) cout << YELLOW << "GetMCTEntries Year = " << _yy << " Polarity = " << _pp << "   Value = " << _nTotal << RESET << endl;
            if (_nTotal == -1) {
                if (_debug) MessageSvc::Debug("GetMCTEntries", (TString) "SKIPPING");
                continue;
            }
            _collected[make_pair(hash_year(_yy), hash_polarity(_pp))] = make_pair(0., 0.);
            _entries += _nTotal;
        }
    }
    if ((_collected.size() != _years.size() * _polarities.size()) && !_force) { 
      MessageSvc::Error("GetMCTEntries", (TString) "Issue in collecting MCT entries", "EXIT_FAILURE"); 
    }
    
    MessageSvc::Info("GetMCTEntries", (TString) fmt::format("MCT entries = {0}", _entries));
    MessageSvc::Line();
    return _entries;
}

pair< double, double > GetGeneratorEfficiency(const Prj & _prj, const Year & _year, const Polarity & _polarity, const TString & _sample, const Q2Bin & _q2bin, bool _debug) noexcept {
    /*
      TODO :
        we compute all efficiencies (generator, filtering, reconstruction) split by years and polarity
        for signal modes, we want all the different pieces to be available when we request a combined value (e.g. Run1)
        therefore, trowing an error is the best thing to do
        for background modes, we know some years are not available and we have tweaks in place for the fits (e.g. use only 2012 for Run1)
        therefore, I would trow a warning and return the average of the available pieces.
    */
    TString _sampleCopy = _sample;

    vector<TString> _samples_with_q2BinTag = { 
        "Bu2KMM", "Bu2KEE", 
        "Bd2KstMM", "Bd2KstEE"
    };
    if( CheckVectorContains( _samples_with_q2BinTag, _sample) && _q2bin != Q2Bin::All){
        MessageSvc::Warning("GetGeneratorEfficiency, appending q2 bin file tag", _sampleCopy);
        _sampleCopy = fmt::format("{}_{}", _sampleCopy.Data(), to_string(_q2bin));
        MessageSvc::Warning("GetGeneratorEfficiency, appending q2 bin file tag", _sampleCopy);
    }else{
        MessageSvc::Warning("GetGeneratorEfficiency, loading the integrated over q2 range", _sampleCopy);
    }

    TString _file       = fmt::format("{0}/efficiencies/generator/v{2}/{1}.yaml", SettingDef::IO::gangaDir, _sampleCopy.ReplaceAll("JPs", "JPs_").ReplaceAll("Psi", "Psi_"), GetBaseVer(SettingDef::Tuple::gngVer));

    MessageSvc::Info(Color::Cyan, "GetGeneratorEfficiency", (TString) fmt::format("{0} - {1} -- {2} v({2})", to_string(_year), to_string(_polarity), _file,GetBaseVer(SettingDef::Tuple::gngVer )));
    
    TString _effKey = "eff";
    TString _effErrKey = "err";
    if( _file.Contains("Bu2KPsi_EE") && (SettingDef::Tuple::gngVer == "9") ){
      //Patch, re-use v9 generator effs (hack)
      MessageSvc::Warning("Patch generator level effs due to BUG on dec file, will use the particle decprod cut, ( not anti-particle one)");
      _effKey = "eff_p";
      _effErrKey = "err_p";
    }
    YAML::Node _parser = YAML::LoadFile(_file.Data());
    if (_parser.IsNull()) MessageSvc::Error("GetGeneratorEfficiency", (TString) "Invalid", _file, "parser", "EXIT_FAILURE");
    if (_debug) {
        cout << YELLOW;
        MessageSvc::Line();
        MessageSvc::Debug("GetGeneratorEfficiency", (TString) "Nodes =", to_string(_parser.size()));
        for (YAML::iterator _it = _parser.begin(); _it != _parser.end(); ++_it) { MessageSvc::Debug("GetGeneratorEfficiency", (TString) "SubNodes =", to_string(_it->second.size())); }
        cout << YELLOW;
        cout << _parser << endl;
        MessageSvc::Line();
        cout << RESET;
    }
    
    vector< TString > _years      = GetYears(to_string(_year));
    vector< TString > _polarities = GetPolarities(to_string(_polarity));

    vector< double > _effs    = {};
    vector< double > _errs    = {};
    vector< double > _weights = {};

    for (YAML::iterator _it1 = _parser.begin(); _it1 != _parser.end(); ++_it1) {
        for (auto _yearSelected : _years) {   // loop over years we search
            if (!_it1->first.as< TString >().Contains(_yearSelected)) continue;
            // Only the ndoe of YEAR pass here
            for (auto _pol : _polarities) {                                                              // loop over the polarities to search for
                for (YAML::iterator _it2 = _it1->second.begin(); _it2 != _it1->second.end(); ++_it2) {   // Loop over the sub-node of polaties
                    if (!_it2->first.as< TString >().Contains(_pol)) continue;
                    YAML::Node _effYAML = _it2->second;
                    if (_effYAML[_effKey.Data()]) _effs.push_back(_effYAML[_effKey.Data()].as< double >());
                    if (_effYAML[_effErrKey.Data()]) _errs.push_back(_effYAML[_effErrKey.Data()].as< double >());
                    if ((_years.size() == 1) && (_polarities.size() == 1)) {
                        _weights.push_back(1);
                    } else {
                        auto _Year        = hash_year(_yearSelected);
                        auto _Polarity    = hash_polarity(_pol);
                        auto _lumi_weight = LoadLuminosity(_prj, _Year, _Polarity, _debug);
                        _weights.push_back(_lumi_weight.first * PDG::Const::SQS.at(_Year));
                    }
                }
            }
        }
    }
    return GetAverageVal(_effs, _errs, _weights, _debug);
}

RooRealVar * GetGeneratorEfficiencyVar(const TString & _varName, const Prj & _prj, const Year & _year, const Polarity & _polarity, const TString & _sample, const Q2Bin & _q2bin,  bool _debug) noexcept {
    pair< double, double > _results = GetGeneratorEfficiency(_prj, _year, _polarity, _sample, _q2bin, _debug);
    RooRealVar *           _eff     = new RooRealVar(_varName, _varName, _results.first);
    _eff->setError(_results.second);
    _eff->setRange(0, 1);
    if ((_eff->getVal() == 0) || (_eff->getError() == 0)) MessageSvc::Warning("GetGeneratorEfficiencyVar", _varName, "is 0");
    if (isnan(_eff->getVal()) || isnan(_eff->getError())) MessageSvc::Error("GetGeneratorEfficiencyVar",   _varName, "is NaN", "EXIT_FAILURE");
    return _eff;
}

map< pair< Year, Polarity >, int > GetNPasFiltering(const Prj & _prj, const Year & _year, const Polarity & _polarity, const TString & _sample, bool _debug) noexcept {

    TString    _sampleCopy = _sample;
    TString    _file       = fmt::format("{0}/efficiencies/filtering/v{2}/{1}.yaml", SettingDef::IO::gangaDir, _sampleCopy.ReplaceAll("JPs", "JPs_").ReplaceAll("Psi", "Psi_"), GetBaseVer(SettingDef::Tuple::gngVer));
    if( _debug){
        MessageSvc::Info("GetNPasFiltering Reading ", _file);
    }
    YAML::Node _parser     = YAML::LoadFile(_file.Data());
    if (_parser.IsNull()) MessageSvc::Error("GetNPasFiltering", (TString) "Invalid", _file, "parser", "EXIT_FAILURE");
    if (_debug) {
        cout << YELLOW;
        MessageSvc::Line();
        MessageSvc::Debug("GetNPasFiltering", (TString) "Nodes =", to_string(_parser.size()));
        for (YAML::iterator _it = _parser.begin(); _it != _parser.end(); ++_it) { MessageSvc::Debug("GetNPasFiltering", (TString) "SubNodes =", to_string(_it->second.size())); }
        cout << YELLOW;
        cout << _parser << endl;
        MessageSvc::Line();
        cout << RESET;
    }
    vector< TString > _years      = GetYears(to_string(_year));
    vector< TString > _polarities = GetPolarities(to_string(_polarity));

    map< pair< Year, Polarity >, int > _nPas;
    map< pair< Year, Polarity >, int > _nTot;

    for (YAML::iterator _it1 = _parser.begin(); _it1 != _parser.end(); ++_it1) {
        for (const TString & _YEAR : _years) {   // loop over years we search
            if (!_it1->first.as< TString >().Contains(_YEAR)) continue;
            // Only the ndoe of YEAR pass here
            for (const TString & _POL : _polarities) {                                                   // loop over the polarities to search for
                for (YAML::iterator _it2 = _it1->second.begin(); _it2 != _it1->second.end(); ++_it2) {   // Loop over the sub-node of polaties
                    if (!_it2->first.as< TString >().Contains(_POL)) continue;
                    YAML::Node _countYAML = _it2->second;
                    const int  _nPassed   = _countYAML["npas"].as< int >();
                    const int  _nTotal    = _countYAML["ntot"].as< int >();
                    // cout<<"["<<_year << " - "<< to_string(_polarity)<< " EFF (filtering) from  nPas "<< _pas << " / nTot  "<< _tot << endl;

                    _nPas[make_pair(hash_year(_YEAR), hash_polarity(_POL))] = _nPassed;
                    _nTot[make_pair(hash_year(_YEAR), hash_polarity(_POL))] = _nTotal;

                    const pair< double, double > _lumiVal = LoadLuminosity(_prj, hash_year(_YEAR), hash_polarity(_POL), _debug);
                    MessageSvc::Info("GetNPasFiltering", (TString) fmt::format("[ {0} - {1} : EFF (filtering) from lumi * (nPas / nTot) = {4}* [ ({2}) / ({3}) ]", _YEAR, _POL, _nPassed, _nTotal, _lumiVal.first));
                    // _lumi.push_back(_lumiVal.first); // only value, not error on lumi
                    //---- Matching polarity found among list of years we used
                }
            }
        }
    }
    if (_nPas.size() != _years.size() * _polarities.size() || _nTot.size() != _years.size() * _polarities.size()) { MessageSvc::Error("GetNPasFiltering", (TString) "Invalid Parsing, expected size", "EXIT_FAILURE"); }
    return _nPas;
}

map< pair< Year, Polarity >, int > GetNTotFiltering(const Prj & _prj, const Year & _year, const Polarity & _polarity, const TString & _sample, bool _debug) noexcept {

    TString    _sampleCopy = _sample;
    TString    _file       = fmt::format("{0}/efficiencies/filtering/v{2}/{1}.yaml", SettingDef::IO::gangaDir, _sampleCopy.ReplaceAll("JPs", "JPs_").ReplaceAll("Psi", "Psi_"), GetBaseVer(SettingDef::Tuple::gngVer));
    if( _debug){
        MessageSvc::Info("GetNTotFiltering Reading ", _file);
    }    
    YAML::Node _parser     = YAML::LoadFile(_file.Data());
    if (_parser.IsNull()) MessageSvc::Error("GetNTotFiltering", (TString) "Invalid", _file, "parser", "EXIT_FAILURE");
    if (_debug){
        cout << YELLOW;
        MessageSvc::Line();
        MessageSvc::Debug("GetNTotFiltering", (TString) "Nodes =", to_string(_parser.size()));
        for (YAML::iterator _it = _parser.begin(); _it != _parser.end(); ++_it) { MessageSvc::Debug("GetNTotFiltering", (TString) "SubNodes =", to_string(_it->second.size())); }
        cout << YELLOW;
        cout << _parser << endl;
        MessageSvc::Line();
        cout << RESET;
    }
    vector< TString > _years      = GetYears(to_string(_year));
    vector< TString > _polarities = GetPolarities(to_string(_polarity));

    map< pair< Year, Polarity >, int > _nPas;
    map< pair< Year, Polarity >, int > _nTot;

    for (YAML::iterator _it1 = _parser.begin(); _it1 != _parser.end(); ++_it1) {
        for (const TString & _YEAR : _years) {   // loop over years we search
            if (!_it1->first.as< TString >().Contains(_YEAR)) continue;
            // Only the ndoe of YEAR pass here
            for (const TString & _POL : _polarities) {                                                   // loop over the polarities to search for
                for (YAML::iterator _it2 = _it1->second.begin(); _it2 != _it1->second.end(); ++_it2) {   // Loop over the sub-node of polaties
                    if (!_it2->first.as< TString >().Contains(_POL)) continue;
                    YAML::Node _countYAML = _it2->second;
                    const int  _nPassed   = _countYAML["npas"].as< int >();
                    const int  _nTotal    = _countYAML["ntot"].as< int >();
                    // cout<<"["<<_year << " - "<< to_string(_polarity)<< " EFF (filtering) from  nPas "<< _pas << " / nTot  "<< _tot << endl;

                    _nPas[make_pair(hash_year(_YEAR), hash_polarity(_POL))] = _nPassed;
                    _nTot[make_pair(hash_year(_YEAR), hash_polarity(_POL))] = _nTotal;

                    const pair< double, double > _lumiVal = LoadLuminosity(_prj, hash_year(_YEAR), hash_polarity(_POL), _debug);
                    MessageSvc::Info("GetNTotFiltering", (TString) fmt::format("[ {0} - {1} : EFF (filtering) from lumi * (nPas / nTot) = {4}* [ ({2}) / ({3}) ]", _YEAR, _POL, _nPassed, _nTotal, _lumiVal.first));
                    // _lumi.push_back(_lumiVal.first); // only value, not error on lumi
                    //---- Matching polarity found among list of years we used
                }
            }
        }
    }
    if (_nPas.size() != _years.size() * _polarities.size() || _nTot.size() != _years.size() * _polarities.size()) { MessageSvc::Error("GetNTotFiltering", (TString) "Invalid Parsing, expected size", "EXIT_FAILURE"); }
    return _nTot;
}

pair< double, double > GetFilteringEfficiency(const Prj & _prj, const Year & _year, const Polarity & _polarity, const TString & _sample, bool _debug) noexcept {
    /*
        Hack all samples unfiltered
        return { 1.0,0.}; 
    */
    
    /*
    TODO :
        we compute all efficiencies (generator, filtering, reconstruction) split by years and polarity
        for signal modes, we want all the different pieces to be available when we request a combined value (e.g. Run1)
        therefore, trowing an error is the best thing to do
        for background modes, we know some years are not available and we have tweaks in place for the fits (e.g. use only 2012 for Run1)
        therefore, I would trow a warning and return the average of the available pieces.
    */
    TString _sampleCopy = _sample;
    TString _file       = fmt::format("{0}/efficiencies/filtering/v{2}/{1}.yaml", SettingDef::IO::gangaDir, _sampleCopy.ReplaceAll("JPs", "JPs_").ReplaceAll("Psi", "Psi_"), GetBaseVer(SettingDef::Tuple::gngVer ) );
    MessageSvc::Info(Color::Cyan, "GetFilteringEfficiency", (TString) fmt::format("{0} - {1} -- {2}", to_string(_year), to_string(_polarity), _file));

    YAML::Node _parser = YAML::LoadFile(_file.Data());
    if (_parser.IsNull()) MessageSvc::Error("GetFilteringEfficiency", (TString) "Invalid", _file, "parser", "EXIT_FAILURE");

    if (_debug) {
        cout << YELLOW;
        MessageSvc::Line();
        MessageSvc::Debug("GetFilteringEfficiency", (TString) "Nodes =", to_string(_parser.size()));
        for (YAML::iterator _it = _parser.begin(); _it != _parser.end(); ++_it) { MessageSvc::Debug("GetFilteringEfficiency", (TString) "SubNodes =", to_string(_it->second.size())); }
        cout << YELLOW;
        cout << _parser << endl;
        MessageSvc::Line();
        cout << RESET;
    }

    vector< TString > _years      = GetYears(to_string(_year));
    vector< TString > _polarities = GetPolarities(to_string(_polarity));

    vector< int >    _nPas;
    vector< int >    _nTot;
    vector< double > _lumi;

    for (YAML::iterator _it1 = _parser.begin(); _it1 != _parser.end(); ++_it1) {
        for (const TString & _YEAR : _years) {   // loop over years we search
            if (!_it1->first.as< TString >().Contains(_YEAR)) continue;
            // Only the ndoe of YEAR pass here
            for (const TString & _POL : _polarities) {                                                   // loop over the polarities to search for
                for (YAML::iterator _it2 = _it1->second.begin(); _it2 != _it1->second.end(); ++_it2) {   // Loop over the sub-node of polaties
                    if (!_it2->first.as< TString >().Contains(_POL)) continue;
                    YAML::Node _countYAML = _it2->second;
                    const int  _nPassed   = _countYAML["npas"].as< int >();
                    const int  _nTotal    = _countYAML["ntot"].as< int >();
                    // cout<<"["<<_year << " - "<< to_string(_polarity)<< " EFF (filtering) from  nPas "<< _pas << " / nTot  "<< _tot << endl;
                    _nPas.push_back(_nPassed);
                    _nTot.push_back(_nTotal);

                    if ((_years.size() == 1) && (_polarities.size() == 1)) {
                        _lumi.push_back(1);
                    } else {
                        const pair< double, double > _lumiVal = LoadLuminosity(_prj, hash_year(_YEAR), hash_polarity(_POL), _debug);
                        MessageSvc::Info("GetFilteringEfficiency", (TString) fmt::format("[ {0} - {1} : EFF (filtering) from lumi * (nPas / nTot) = {4}* [ ({2}) / ({3}) ]", _YEAR, _POL, _nPassed, _nTotal, _lumiVal.first));
                        _lumi.push_back(_lumiVal.first * PDG::Const::SQS.at(hash_year(_YEAR)));
                    }
                }
            }
        }
    }
    if (_nPas.size() != _years.size() * _polarities.size() || _nTot.size() != _years.size() * _polarities.size() || _lumi.size() != _years.size() * _polarities.size()) { MessageSvc::Error("ComputeFilteringEfficiency", (TString) "Invalid Parsing, expected size", "EXIT_FAILURE"); }

    vector< double > _effs;
    vector< double > _errs;
    vector< double > _weights;
    for (auto && entry : zip_range(_nPas, _nTot, _lumi)) {
        double pas = entry.get< 0 >();
        // if (_nPasMod > 0 && _nPas.size() != 0){
        //    pas = _nPasMod;
        // }
        double       tot    = entry.get< 1 >();
        double       weight = entry.get< 2 >();
        RooRealVar * eff    = GetEfficiencyVar(pas, tot);
        _effs.push_back(eff->getVal());
        _errs.push_back(eff->getError());
        _weights.push_back(weight);
        delete eff;
    }

    return GetAverageVal(_effs, _errs, _weights, _debug);
}

RooRealVar * GetFilteringEfficiencyVar(const TString & _varName, const Prj & _prj, const Year & _year, const Polarity & _polarity, const TString & _sample, const bool _debug) noexcept {
    pair< double, double > _results = GetFilteringEfficiency(_prj, _year, _polarity, _sample, _debug);
    RooRealVar *           _eff     = new RooRealVar(_varName, _varName, _results.first);
    _eff->setError(_results.second);
    _eff->setRange(0, 1);
    if ((_eff->getVal() == 0) || (_eff->getError() == 0)) MessageSvc::Warning("GetFilteringEfficiencyVar", _varName, "is 0");
    if (isnan(_eff->getVal()) || isnan(_eff->getError())) MessageSvc::Error("GetFilteringEfficiencyVar", _varName, "is NaN", "EXIT_FAILURE");
    return _eff;
}

// Efficiency value from 2 numbers : double!
RooRealVar * GetEfficiencyVar(double _nPas, double _nTot, bool _debug) noexcept {
    // 2 numbers, 1 bin, error = sqrt(nPas) and sqrt(nTot);
    TH1D _hPas = TH1D("pas", "", 1, 0, 1);
    _hPas.SetBinContent(1, floor(_nPas));
    _hPas.SetBinError(1, TMath::Sqrt(floor(_nPas)));

    TH1D _hTot = TH1D("tot", "", 1, 0, 1);
    _hTot.SetBinContent(1, floor(_nTot));
    _hTot.SetBinError(1, TMath::Sqrt(floor(_nTot)));

    TEfficiency * _tEff = new TEfficiency(_hPas, _hTot);

    RooRealVar * _eff = new RooRealVar("eff", "eff", 0, 1);
    // Get efficiency in this single bin
    _eff->setVal(_tEff->GetEfficiency(1));
    // Set Error on the RooRealVar being  (err_low + err_up)/2
    _eff->setError((_tEff->GetEfficiencyErrorLow(1) + _tEff->GetEfficiencyErrorUp(1)) / 2.);
    // Set eventual AsymError from low/up errors
    _eff->setAsymError(_tEff->GetEfficiencyErrorLow(1), _tEff->GetEfficiencyErrorUp(1));

    if (_nPas == _nTot) {
        MessageSvc::Warning("100% efficiency, set error to 0");
        _eff->setError(0.);
        _eff->setAsymError(0., 0.);
    }

    // MessageSvc::Info("GetEfficiencyVar", _eff);
    return _eff;
}

tuple< RooRealVar *, TEfficiency *, TH1D * > GetEfficiencyResults(const TH1 & _hPasIn, const TH1 & _hTotIn) {
    TString _name = _hPasIn.GetName();
    _name.ReplaceAll("pas", "eff");
    TString _title = _hPasIn.GetTitle();
    _title.ReplaceAll("pas", "eff");
    TH1D * _hEff = static_cast< TH1D * >(CopyHist(&_hPasIn, true));
    _hEff->SetName(_name);
    _hEff->SetTitle(_title);
    _hEff->SetMinimum(0);
    _hEff->SetMaximum(1.1);

    // Make a Full Copy of The hPas and hTot to build the new information
    TH1D * _hPasCOPY = static_cast< TH1D * >(CopyHist(&_hPasIn, false));
    TH1D * _hTotCOPY = static_cast< TH1D * >(CopyHist(&_hTotIn, false));

    // N entries of  hPas
    double _nPas = _hPasCOPY->GetEntries();
    double _nTot = _hTotCOPY->GetEntries();
    // Integrals of hPas and  hTot
    // Integral of   hPas
    double _wPas = _hPasCOPY->Integral();
    double _wTot = _hTotCOPY->Integral();

    cout << endl;
    MessageSvc::Info("Events");
    MessageSvc::Info("Pas", to_string(_nPas));
    MessageSvc::Info("Tot", to_string(_nTot));
    cout << endl;
    MessageSvc::Info("Weighted Events");
    MessageSvc::Info("Pas", to_string(_wPas));
    MessageSvc::Info("Tot", to_string(_wTot));
    cout << endl;

    //-----> Here you set for each bin the entry to be equal to the integral.
    //-----> Depends on the way denominator/numerator has to be used to show the efficiencies.
    //-----> For the time being we just divide bin_Num/bin_Denom to get the efficiency of the cuts
    //-----> and reweighting in each bin. Thus we do not overload the content
    // for (int i = 1; i <= m_TotCOPY->back().GetNbinsX(); ++i)
    //     m_TotCOPY->back().SetBinContent(i, _wTot);
    //-----> We have to write what is happening here.

    double _iPas = _wPas;
    double _statPas[10];
    // stats[0] = sumw
    // stats[1] = sumw2
    // stats[2] = sumwx
    // stats[3] = sumwx2
    _hPasCOPY->GetStats(_statPas);
    // Round each bin content to integer
    if (TMath::Abs(_statPas[0] - _statPas[1]) > 1e-5) {
        MessageSvc::Warning("Pas histogram filled with weights");
        _hPasCOPY = static_cast< TH1D * >(RoundToIntAllEntries(_hPasCOPY));
        // _hPas = * GetIntHistogram(& _hPas);
        _iPas = _hPasCOPY->Integral();
    }

    //-----> We have to write what is happening here.
    double _iTot = _wTot;
    double _statTot[10];
    _hTotCOPY->GetStats(_statTot);
    // Round each bin content to integer
    if (TMath::Abs(_statTot[0] - _statTot[1]) > 1e-5) {
        MessageSvc::Warning("Tot histogram filled with weights");
        //--- TAKE the ROUNDED to INT Integral ?
        _hTotCOPY = static_cast< TH1D * >(RoundToIntAllEntries(_hTotCOPY));
        _iTot     = _hTotCOPY->Integral();
    }

    //-----> We have to write what is happening here. "effr"/"eff" options It's a ratio of 2 raw histograms.
    // effr = efficiency-reset
    // CheckHistogram(_hPasCOPY, _hTotCOPY, "eff");
    // CheckHistogram(& _hPas, & _hTot, "eff");
    //-----> Create the TEfficiency object here
    if (!TEfficiency::CheckConsistency(*_hPasCOPY, *_hTotCOPY)) { MessageSvc::Error("GetEfficiencyResults", (TString) "ConsistencyCheck failed", "EXIT_FAILURE"); }

    TEfficiency * _tEff = new TEfficiency(*_hPasCOPY, *_hTotCOPY);
    for (int i = 1; i <= _hEff->GetNbinsX(); ++i) {
        if ((_hPasCOPY->GetBinContent(i) != 0) && (_hTotCOPY->GetBinContent(i) != 0)) {
            MessageSvc::Print("Bin " + to_string(i), (TString) "_tEfficiency.GetEfficiency(" + _hEff->GetBin(i) + ") = " + _tEff->GetEfficiency(_hEff->GetBin(i)));
            _hEff->SetBinContent(i, _tEff->GetEfficiency(_hEff->GetBin(i)));   //< get efficiency in this Bin
            _hEff->SetBinError(i, (_tEff->GetEfficiencyErrorUp(_hEff->GetBin(i)) + _tEff->GetEfficiencyErrorLow(_hEff->GetBin(i))) / 2.);
        }
    }
    _tEff->SetName(((TString) _hEff->GetName()).ReplaceAll("eff", "TEff"));
    _tEff->SetTitle(((TString) _hEff->GetName()).ReplaceAll("eff", "TEff"));

    cout << RED;
    MessageSvc::Print("TEfficiency NAME  =", _tEff->GetName());
    MessageSvc::Print("TEfficiency TITLE =", _tEff->GetTitle());
    // abort();
    MessageSvc::Info("int(Weighted Events)");
    MessageSvc::Info("Pas", to_string(_iPas));
    MessageSvc::Info("Tot", to_string(_iTot));

    RooRealVar * _eff = GetEfficiencyVar(_iPas, _iTot);
    return make_tuple(_eff, _tEff, _hEff);
}

void MakeRatioPlot(pair< TH1D *, TH1D * > & h_num_pair, pair< TH1D *, TH1D * > & h_den_pair, int padID, bool logy) {
    if (h_num_pair.second == nullptr || h_den_pair.second == nullptr) {
        MessageSvc::Warning("MakeRatioPlot", (TString) "Isobinned histos do not exist");
        return;
    }
    if (h_num_pair.first == nullptr || h_den_pair.first == nullptr) {
        MessageSvc::Warning("MakeRatioPlot", (TString) "Raw histos do not exist");
        return;
    }

    TH1D * h_num_raw = h_num_pair.first;
    TH1D * h_num     = h_num_pair.second;

    TH1D * h_den_raw = h_den_pair.first;
    TH1D * h_den     = h_den_pair.second;
    // Create the TEfficiency Object from Num/Den
    TEfficiency * pEff = nullptr;
    if (TEfficiency::CheckConsistency(*h_num, *h_den)) {
        pEff = new TEfficiency(*h_num_pair.second, *h_den_pair.second);
        // this will attach the TEfficiency object to the current directory
        // pEff->SetDirectory(gDirectory);
        // now all objects in gDirectory will be written to "myfile.root"
        // pFile->Write();
    } else {
        MessageSvc::Error("MakeRatioPlot", (TString) "CheckConsistency failed", "EXIT_FAILURE");
    }
    // pEff in bottom pad,
    pEff->SetLineColor(kBlack);
    pEff->SetMarkerStyle(21);
    // h1 settings
    h_num->SetLineColor(kBlue + 1);
    h_num->SetLineWidth(2);

    h_den->SetLineColor(kRed);
    h_den->SetLineWidth(2);
    pEff->SetTitle(TString(fmt::format("{0};{1};{2}", "", h_num->GetTitle(), "ratio hN/hD")));
    pEff->SetTitle("");   // Remove the ratio title

    //-------- Plotting stuff
    //(xlow,yhigh)--> ------------------------  <-- (xhigh,yhigh)
    //               |                        |
    //               |                        |
    //               |       pUpper           |
    //               |                        |
    //               |                        |
    //               |                        |
    //               |                        |
    //               |------------------------| <-- yep (we actually add ysepStep to make space for titles in both up/low plots)
    //               |       pDown            |
    //               |                        |
    //(xlow,ylow)-->  ------------------------  <-- (xhigh,ylow)
    // always padDown = 2/10 of the total, upper 8/10 -> scaling is 2/10
    TCanvas * c = new TCanvas("cdif", "cdif", 800, 600);
    c->Divide(2, 1);
    c->cd(1);
    h_den_raw->Draw();
    c->cd(2);
    h_num_raw->Draw();
    c->SaveAs(TString(h_num->GetName()) + "_TESTDENUG.pdf");
    delete c;
    TCanvas * c1          = new TCanvas("c1", "multipads", 800, 600);
    double    xlow        = 0.05;
    double    xhigh       = 0.95;
    double    ylow        = 0.005;
    double    yhigh       = 0.95;
    double    ysepStep    = 0.001;   // 0.015;
    double    yseparation = ylow + (4.0 / 10.) * (yhigh - ylow);
    double    ratio_sizes = (yhigh - yseparation) / (yseparation - ylow);
    TPad *    pDown       = new TPad("pDown", "pDown", xlow, ylow, xhigh, yseparation - ysepStep, 0, 0, 0);
    pDown->Draw();

    TPad * pUpper = new TPad("p2", "p2", xlow, yseparation + ysepStep, xhigh, yhigh, 0, 0, 0);
    pUpper->SetTopMargin(0.1);
    // pUpper->SetLeftMargin(0.05);
    pUpper->SetBottomMargin(0.12 * (yhigh - yseparation));   // make space fo the tile.
    pUpper->Draw();
    // Plot Upper stuff !
    pUpper->cd();
    // hnum = less entries per bin : draw it later
    // hden = more entries per bin : draw it first
    h_den_raw->SetLineColor(kRed);
    h_den_raw->SetMarkerColor(kRed);
    h_den_raw->SetLineWidth(2);
    h_den_raw->SetLineWidth(2);
    h_den_raw->GetYaxis()->SetLabelColor(kRed);
    h_den_raw->GetYaxis()->CenterTitle();
    h_den_raw->GetYaxis()->SetTitleOffset(0.6 * h_den_raw->GetYaxis()->GetTitleOffset());
    h_den_raw->GetXaxis()->SetTitleOffset(100);

    h_den_raw->SetMinimum(0);
    h_den_raw->Draw("+Re");
    pUpper->SetTicks(1, 0);
    pDown->SetGridx();
    pDown->SetGridy();
    pUpper->Update();

    h_num_raw->GetYaxis()->SetLabelColor(kBlue);
    // h_num_raw->GetYaxis()->SetLineColor(kBlue);
    h_num_raw->SetLineColor(kBlue);
    h_num_raw->SetMarkerColor(kBlue);
    h_num_raw->SetLineWidth(2);
    // pUpper->SetTicks(0 , )
    //    tx = 1 ;  tick marks on top side are drawn (inside)
    //    tx = 2;   tick marks and labels on top side are drawn
    //    ty = 1;   tick marks on right side are drawn (inside)
    //    ty = 2;   tick marks and labels on right side are drawn
    //       Use TPad::SetTicks(tx,ty)

    Double_t right_max    = 1.1 * h_num_raw->GetMaximum();
    Double_t scale        = pUpper->GetUymax() / right_max;
    TH1D *   for_plot_num = new TH1D();
    h_num_raw->Copy(*for_plot_num);
    for_plot_num->Scale(scale);
    for_plot_num->SetMinimum(0);
    for_plot_num->Draw("same");
    // the legend
    // gPad->BuildLegend();

    auto * axis = new TGaxis(pUpper->GetUxmax(), pUpper->GetUymin(), pUpper->GetUxmax(), pUpper->GetUymax(), 0, right_max, 510, "+L");
    axis->SetLineColor(kBlue);
    axis->SetLabelColor(kBlue);
    axis->CenterTitle();
    axis->Draw();
    pDown->cd();
    pDown->SetTopMargin(0.1);
    pDown->SetBottomMargin(0.25);
    pEff->SetTitle("");   // Remove the ratio title
    pEff->Draw();
    gPad->Update();
    auto * xax   = pEff->GetPaintedGraph()->GetXaxis();
    auto * yax   = pEff->GetPaintedGraph()->GetYaxis();
    double x_min = h_num->GetXaxis()->GetXmin();
    double x_max = h_num->GetXaxis()->GetXmax();
    xax->SetLimits(x_min, x_max);
    xax->SetTitleSize(xax->GetTitleSize() * ratio_sizes);
    xax->SetLabelSize(xax->GetLabelSize() * ratio_sizes);
    xax->CenterTitle();
    xax->SetTitle(TString(h_den_raw->GetXaxis()->GetTitle()));
    pEff->SetTitle("");   // Remove the ratio title
    yax->SetTitle("#epsilon [%]");
    yax->SetTitleSize(yax->GetTitleSize() * ratio_sizes);
    yax->SetLabelSize(yax->GetLabelSize() * ratio_sizes);
    yax->SetTitleOffset((0.6 / ratio_sizes) * yax->GetTitleOffset());   // shift yText on left hand side on bottom plot
    yax->CenterTitle();
    pDown->Draw("AP");
    pDown->SetGridx();
    pDown->SetGridy();

    pDown->Update();
    TString name = h_num->GetName();
    name.ReplaceAll("(", "_").ReplaceAll("TMath::", "").ReplaceAll(")", "").ReplaceAll("{", "").ReplaceAll("}", "");
    c1->SaveAs(name + "_RATIOPLOT.pdf");
    c1->SaveAs(name + "_RATIOPLOT.root");

    delete pUpper;
    delete pDown;
    delete c1;

    return;
}

vector< VariableBinning > GetAllBinLabels(const Prj & _prj, const Year & _year, const Trigger & _trigger, const TriggerConf & _triggerConf) {
    vector< VariableBinning > _variable_Binning = {};
    if (!SettingDef::Efficiency::option.Contains("ISO")) { return _variable_Binning; };
    return GetVariableBinning(  _prj,  _year, _trigger, _triggerConf);
    // TString _directory = IOSvc::GetFlatnessDir(_prj, _year, _trigger, _triggerConf);
    // _directory += "/*IsoBinCuts.csv";
    // vector< TString > files = IOSvc::Glob(_directory.Data());
    // MessageSvc::Info("GetAllBinLabels, globbing (n Found = )", _directory, to_string(files.size()));
    // for (auto & file : files) { _variable_Binning.push_back(VariableBinning(file)); }
    // return _variable_Binning;
}

// map< TString, vector< TString > > GetAllBinLabels(const Prj & _prj, const Year & _year, const Trigger & _trigger, const TriggerConf & _triggerConf) {
//     map< TString, vector< TString > > _label_isoExpression;
//     if (!SettingDef::Efficiency::option.Contains("ISO")) { return _label_isoExpression; };
//     TString _directory = IOSvc::GetFlatnessDir("flatness", _prj, _year, _trigger, _triggerConf);
//     _directory += "/*IsoBinCuts.csv";
//     vector< TString > files = IOSvc::Glob(_directory.Data());
//     MessageSvc::Info("GetAllBinLabels, globbing (n Found = )", _directory, to_string(files.size()));
//     for (auto & file : files) {
//         VariableBinning var( file);
//     }
//     if( EffDebug()){
//         for( const auto & collected : _label_isoExpression){
//             std::cout<<RED<<collected.first << std::endl;
//             for( const auto & el : collected.second){
//                 std::cout<< YELLOW<< "\t "<< el << RESET << std::endl;
//             }
//         }
//     }
//     return _label_isoExpression;
// }

/* -------------- Total eff map filler------------*/
void AddTOTEfficienciesToMap(map< TString, EfficiencyContent > & _Efficiencies) {
    if (_Efficiencies.find("flt") == _Efficiencies.end()) { MessageSvc::Error("AddTOTEfficienciesToMap", "Efficiency Map must contain filtering (flt)", "EXIT_FAILURE"); }
    if (_Efficiencies.find("gen") == _Efficiencies.end()) { MessageSvc::Error("AddTOTEfficienciesToMap", "Efficiency Map must contain filtering (flt)", "EXIT_FAILURE"); }
    MessageSvc::Info("Add Tot Efficiency To Existing Map ");
    size_t original_size = _Efficiencies.size();
    int    nSel          = 0;
    for (const auto & _sel_eff : _Efficiencies) {
        if (!_sel_eff.first.Contains("sel")) continue;   // for each Selection efficiency let's make the TOTAL eff
        nSel++;
        TString _id      = _sel_eff.first;
        TString _id_tot  = _id.ReplaceAll("sel", "tot");
        TString _varName = _sel_eff.second.EfficiencyVar->GetName();
        _varName.ReplaceAll("sel", "tot");
        _Efficiencies[_id_tot]               = EfficiencyContent(_varName);
        _Efficiencies[_id_tot].EfficiencyVar = new RooRealVar(_varName, _varName, 0, 1);
        _Efficiencies[_id_tot].EfficiencyVar->setVal(_Efficiencies[_sel_eff.first].getVal() * _Efficiencies["flt"].getVal() * _Efficiencies["gen"].getVal());
        // TODO : WHICH EFFICIENCY TO ASSING TO TOTAL?
        // WE DO USE WEIGHTS IN CALCULATION, how to access the error?
        double err = _Efficiencies[_id_tot].getVal() * sqrt(pow(_Efficiencies[_sel_eff.first].getRelError(), 2) + pow(_Efficiencies["flt"].getRelError(), 2) + pow(_Efficiencies["gen"].getRelError(), 2));
        _Efficiencies[_id_tot].UpdateName();
        _Efficiencies[_id_tot].EfficiencyVar->setError(err);
        _Efficiencies[_id_tot].EfficiencyVar->setConstant();
    }
    MessageSvc::Info("AddTOTEfficienciesToMap", to_string(original_size) + " -> " + to_string(_Efficiencies.size()) + " (" + to_string(nSel) + " sel effs )", "");
    return;
};

/* -------------- Generator level eff map filler------------*/
void AddGeneratorEfficiencyToMap(map< TString, EfficiencyContent > & _Efficiencies, const ConfigHolder & _ConH) {
    MessageSvc::Info("Creating Generator Level Efficiency", _ConH.GetKey());
    if (_Efficiencies.find("gen") != _Efficiencies.end()) { MessageSvc::Error("Generator efficiency already in Map", "", "EXIT_FAILURE"); }
    TString _name_gen    = "eff_" + _ConH.GetKey() + "_gen";
    _Efficiencies["gen"] = EfficiencyContent(_name_gen);

    // if (_ConH.GetYear() == Year::Y2018) {
    //     // all generator are missing here for 2018!
    //     MessageSvc::Warning("GetGeneratorEfficiency (2018) loading 2017 ones");
    //     _Efficiencies["gen"].EfficiencyVar = GetGeneratorEfficiencyVar(_name_gen, _ConH.GetProject(), Year::Y2017, _ConH.GetPolarity(), _ConH.GetSample());
    // } else {
    _Efficiencies["gen"].EfficiencyVar = GetGeneratorEfficiencyVar(_name_gen, _ConH.GetProject(), _ConH.GetYear(), _ConH.GetPolarity(), _ConH.GetSample() , _ConH.GetQ2bin() );
    // }
    _Efficiencies["gen"].UpdateName();
    _Efficiencies["gen"].EfficiencyVar->setConstant();
    MessageSvc::Info("Creating Generator Level Efficiency DONE", _ConH.GetKey());
}
/* --------- FIltering level Eff map filler -------- */
void AddFilteringEfficiencyToMap(map< TString, EfficiencyContent > & _Efficiencies, const ConfigHolder & _ConH) {
    MessageSvc::Info("Creating Filtering Level Efficiency", _ConH.GetKey());
    if (_Efficiencies.find("flt") != _Efficiencies.end()) { MessageSvc::Error("Filtering efficiency already in Map", "", "EXIT_FAILURE"); }
    TString _name_flt = "eff_" + _ConH.GetKey() + "_flt";
    // DO assign a NEW NAME !
    _Efficiencies["flt"]               = EfficiencyContent(_name_flt);
    _Efficiencies["flt"].EfficiencyVar = GetFilteringEfficiencyVar(_name_flt, _ConH.GetProject(), _ConH.GetYear(), _ConH.GetPolarity(), _ConH.GetSample());
    _Efficiencies["flt"].UpdateName();
    _Efficiencies["flt"].EfficiencyVar->setConstant();
    MessageSvc::Info("Creating Filtering Level Efficiency DONE", _ConH.GetKey());
    return;
}
/* -------- Lumi  map filler */
void AddLumiToMap(map< TString, EfficiencyContent > & _Efficiencies, const ConfigHolder & _ConH) {
    /* -------------- Luminosity  value -------------*/
    MessageSvc::Info("Creating Lumi RooRealVar", _ConH.GetKey());
    pair< double, double > Luminosity = LoadLuminosity(_ConH.GetProject(), _ConH.GetYear(), _ConH.GetPolarity());
    if (_Efficiencies.find("lumi") != _Efficiencies.end()) { MessageSvc::Error("Cannot add Lumi to Map, already there", "", "EXIT_FAILURE"); }
    _Efficiencies["lumi"]               = EfficiencyContent("lumi");
    _Efficiencies["lumi"].EfficiencyVar = new RooRealVar("lumi", "lumi", 0, 1000000);
    _Efficiencies["lumi"].EfficiencyVar->setVal(Luminosity.first);
    _Efficiencies["lumi"].EfficiencyVar->setError(Luminosity.second);
    _Efficiencies["lumi"].EfficiencyVar->setAsymError(Luminosity.second, Luminosity.second);
    _Efficiencies["lumi"].UpdateName();
    _Efficiencies["lumi"].EfficiencyVar->setConstant();
    MessageSvc::Info("Creating Lumi RooRealVar Done", _ConH.GetKey());
    return;
}

/* ---- Once TFile creeated, extract the map of it ----- */
map< TString, EfficiencyContent > RetrieveAllResults(TString _base_name, TFile & _file, TString _option) {
    map< TString, EfficiencyContent > _Results;
    MessageSvc::Info("RetrieveAllResults", _base_name + ",  option = " + _option);
    for (auto & _variant : EfficiencyFileContent::Types) {
        if (_option == "" && _variant.Contains("NormBin")) continue;
        TString _name = _base_name + "_" + _variant;
        if (_variant == "lumi") { _name = "lumi"; }
        RooRealVar * _found = (RooRealVar *) _file.Get(_name);
        if (_found == nullptr) {
            MessageSvc::Error(TString("Cannot extract ") + _name + " from input TFile", "", "EXIT_FAILURE");
        } else {
            _Results[_variant]               = EfficiencyContent(_found->GetName());
            _Results[_variant].EfficiencyVar = new RooRealVar(*_found, _found->GetName());
        }
    }
    return _Results;
}
map< TString, TH1 * > GetAllVarTemplateHisto(const Prj & _prj, const Year & _year, const Trigger & _trg, const TriggerConf & _trgConf) {
    map< TString, TH1 * > _histoTemplates;
    if (!SettingDef::Efficiency::option.Contains("ISO")) { return _histoTemplates; };
    // directory type = RK-R1-L0I-exclusive/VarXX/IsoBinCuts.csv
    // TODO : replace with eos/vX/fit/flatness/.....
    TString _directory = SettingDef::IO::anaDir + "/flatness/v" + SettingDef::Tuple::gngVer + "/{PRJ}-{RUN}-{TRG}-{TRGCONF}/*/";
    _directory.ReplaceAll("{PRJ}", to_string(_prj));                    // flatness/RK-{RUN}-{TRG}-{TRGCONF};
    _directory.ReplaceAll("{RUN}", GetRunFromYear(to_string(_year)));   // flatness/RK-{RUN}-{TRG}-{TRGCONF};
    _directory.ReplaceAll("{TRG}", to_string(_trg));
    _directory.ReplaceAll("{TRGCONF}", to_string(_trgConf));

    // map<TString, vector<TCut> >
    vector< TString > directories = IOSvc::Glob(_directory.Data());
    MessageSvc::Info("GetAllBinSplits, globbing", _directory);
    map< TString, vector< TString > > _allBins;
    for (const auto & dir : directories) {
        cout << dir << endl;
        // those are the variables....
        TString varAlias = "ERROR";
        // if (!dir.Contains("PT-ETA")) {
        auto * _strCollection = ((TString) dir).Tokenize("/");
        varAlias              = TString(((TObjString *) (*_strCollection).Last())->String());
        TString isoBin_file   = dir + "IsoBinResults.root";

        cout << "FILE FOUND = " << varAlias << "   FILE : " << isoBin_file << endl;
        TFile * _tFile = IOSvc::OpenFile(isoBin_file, OpenMode::READ);
        TH1 *   histo  = nullptr;
        if (varAlias.Contains("2D")) {
            histo = (TH2Poly *) _tFile->Get("template");
            histo = static_cast< TH2Poly * >(histo);
        } else {
            histo = (TH1D *) _tFile->Get("template");
            histo = static_cast< TH1D * >(histo);
        }
        if (histo == nullptr) { MessageSvc::Error("Cannot Load template histo from file", isoBin_file, "EXIT_FAILURE"); }
        histo->SetDirectory(0);
        _histoTemplates[varAlias] = histo;
        IOSvc::CloseFile(_tFile);
    }
    return _histoTemplates;
}

//--------------------------------------------------------------------------------------------------------
//
//-------- FUNCTIONS  useful for flatness (yields collecting with templates to fill )
//
//--------------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------------
//
//-------- Get Root FileName of Efficineciy File containing histograms efficiencies
//
//--------------------------------------------------------------------------------------------------------
TString RetrieveRootFileEfficiency(const Prj & _prj, const Analysis & _ana, const Q2Bin & _q2Bin, const Year & _yy, const Polarity & _pol, const Trigger & _trg, const TriggerConf & triggerConf, const TString & _sample, const TString & _weightOption, const TString & _weightConfig) {
    MessageSvc::Info("RetrieveRootFileEfficiency");
    auto _dirName = IOSvc::GetTupleDir("eff", to_string(_prj), to_string(_ana), to_string(_q2Bin), to_string(_yy), to_string(_trg));
    _dirName += "/" + to_string(_yy) + to_string(_pol);
    auto original                   = SettingDef::Config::triggerConf;
    SettingDef::Config::triggerConf = to_string(triggerConf);
    auto _ch                        = ConfigHolder(_prj, _ana, _sample, _q2Bin, _yy, _pol, _trg, Brem::All);
    _ch.Init();
    SettingDef::Config::triggerConf = original;
    TString _fileName               = _dirName + "/";
    if (GetBaseVer(SettingDef::Tuple::gngVer).Atoi() < 7) {
        _fileName += _sample + "_Efficiency_" + _ch.GetKey("addtrgconf");
    } else {
        _fileName = _fileName + "/" + _weightOption + "_" + _sample + "_Efficiency_" + _ch.GetKey("addtrgconf");
        if (_weightOption != "no") { _fileName = _fileName + "_" + _weightConfig; }
    }
    _fileName += ".root";
    MessageSvc::Info("RetrieveRootFileEfficiency", _fileName);
    return _fileName;
}

TString RetrieveRootFileEfficiencyCO(const ConfigHolder & co, const TString & _weightOption, const TString & _weightConfig) { return RetrieveRootFileEfficiency(co.GetProject(), co.GetAna(), co.GetQ2bin(), co.GetYear(), co.GetPolarity(), co.GetTrigger(), co.GetTriggerConf(), co.GetSample(), _weightOption, _weightConfig); }

TString RetrieveRootFileEfficiencyS(const TString & _prj, const TString & _ana, const TString & _q2Bin, const TString & _yy, const TString & _pol, const TString & _trg, const TString & _triggerConf, const TString & _sample, const TString & _weightOption, const TString & _weightConfig) { return RetrieveRootFileEfficiency(hash_project(_prj), hash_analysis(_ana), hash_q2bin(_q2Bin), hash_year(_yy), hash_polarity(_pol), hash_trigger(_trg), hash_triggerconf(_triggerConf), _sample, _weightOption, _weightConfig); }

//--------------------------------------------------------------------------------------------------------
//
//-------- Get map<TH1DModel> to fill  by hand with yields  in bins collected from log file (  YIELDS  only  used, EFFS ( TODO , but actually other methods used))
//
//--------------------------------------------------------------------------------------------------------
map< TString, ROOT::RDF::TH1DModel > GetMapHistoTemplate(const Prj & _project, const Year & _year, const Trigger & _trigger, const TriggerConf & _trgConf, TString _type) {
    if (!(_type == "YIELDS" or _type == "EFFS")) { MessageSvc::Error("Call GetMapHistoTemplate with TYPES = 'EFFS' or 'YIELDS'", "", "EXIT_FAILURE"); }
    MessageSvc::Info("GetMapHistoTemplate");
    map< TString, ROOT::RDF::TH1DModel > _returnMap;
    MessageSvc::Info("Collecting map of Histograms", _type);
    // vector< tuple< TString, vector< pair< TString, int > > > >
    auto YEAR = hash_year(GetRunFromYear(to_string(_year)));

    cout << "start loop YEAR" << to_string(YEAR) << endl;
    for (auto && [_IDVAR, _binningInfo] : SettingDef::Tuple::isoBins) {
        cout << "IDVAR " << _IDVAR << endl;
        int iDim = 1;
        vector<TString> _branchNames; 
        for (auto && [branchName, nBinsDim, minVal, maxVal, labelAxis, cType] : _binningInfo) {
            _branchNames.push_back( branchName);
            cout<<"==== Dimension "<< iDim << endl;
            cout<<"\t branchName : " << branchName << endl;
            cout<<"\t nBinsDim   : " << nBinsDim << endl;
            cout<<"\t minVal     : " << minVal << endl;
            cout<<"\t maxVal     : " << maxVal << endl;
            cout<<"\t labelAxis  : " << labelAxis << endl;
            cout<<"\t histType   : " << cType << endl;
            iDim++;
        }
        // auto & [_IDVAR, _label_nBins] = var;      // var = vector< tuple< TString ID VAR, vector< pair< TString ( name Var) , int nbins > > > vector used for multi - D
        if (_binningInfo.size() != 1) continue;   // skip 2D isobin vars for the moment
        int     nBins     = get<1>(_binningInfo[0]);
        TString _csv_file = IOSvc::GetFlatnessDir(_project, YEAR, _trigger, _trgConf) + "/" + _IDVAR + "_IsoBinCuts.csv";
        MessageSvc::Info("CSV FILE = ", _csv_file);
        auto _varBinning = VariableBinning(_csv_file, _branchNames.at(0), _branchNames.at( _branchNames.size()-1 ));
        // _varBinning.Init();
        // _varBinning.Print();
        TString _name     = to_string(_project) + "-" + to_string(YEAR) + "-" + to_string(_trigger) + "-" + to_string(_trgConf);
        TString _ID_Histo = _IDVAR;
        MessageSvc::Info("Loading ", _ID_Histo);
        if (_type == "YIELDS") {
            _returnMap[_ID_Histo + "_YEE"] = _varBinning.GetHisto1DModel(_ID_Histo + "_YEE", _name);
            _returnMap[_ID_Histo + "_YMM"] = _varBinning.GetHisto1DModel(_ID_Histo + "_YMM", _name);
        } else if (_type == "EFFS") {
            vector< TString > _types = {
                "sumW", "sumW_rnd", "sumW_bkgcat", "normN", "normN_rnd", "normN_bkgcat", "normD", "normD_rnd", "normD_bkgcat", "sumW_raw", "sumW_rnd_raw", "sumW_bkgcat_raw", "normN_raw", "normN_rnd_raw", "normN_bkgcat_raw", "normD_raw", "normD_rnd_raw", "normD_bkgcat_raw",
            };
            for (auto & tt : _types) { _returnMap[_ID_Histo + "_" + tt] = _varBinning.GetHisto1DModel(_ID_Histo + "_" + tt, _name + "_" + tt); }
        }
    }
    return _returnMap;
};
map< TString, ROOT::RDF::TH1DModel > GetMapHistoTemplateS(const TString & _project, const TString & _year, const TString & _trigger, const TString & _trgConf, TString _type) {
    cout << "GetMapHistoTemplateS" << endl;
    auto rr = GetMapHistoTemplate(hash_project(_project), hash_year(_year), hash_trigger(_trigger), hash_triggerconf(_trgConf), _type);
    return rr;
};

//--------------------------------------------------------------------------------------------------------
//
// Helper Function building the 3 sets  of  histograms needed
// SumW  is basically  Draw("Var", "weight * (  cut &&  butRangeBin) ")  with weight = fullWeight &  cut == full selection on DecayTuple
// NormN is basically  Draw("Var", "weightNormN * (  cutNorm &&  butRangeBin) ")  with weight = fullWeight &  cut == full selection on DecayTuple
// NormD is basically  Draw("Var", "weightNormN * (  cutNorm &&  butRangeBin) ")  with weight = fullWeight &  cut == full selection on DecayTuple
// Efficiency histo  is  extracted with SumW->Scale( normN(integral)/normD(integral) /  nMCDecayTuple(sumW) )
// Integrals are done in the full range including overflow/underflows. SumW after scaling gets under/overflows also filled.
// Details...if you want to "empty the efficiency in  first/last bin do it by hand moving BinContent/Error(0, nBins+1) " to (  1, nBins)
// We do like thiis because fits are  "in ranges",  in  case we will fit bin0 and last bin up  to -inf,+inf we have to change this code...
//
//--------------------------------------------------------------------------------------------------------
const vector< TString > BinnedEffs::m_modes{"normD", "normN", "sumW"};

const vector< TString > BinnedEffs::m_types{"", "rnd", "bkgcat"};

TString BinnedEffs::ID(const TString & _type, const TString & _mode , bool raw ) {
    if (!CheckVectorContains(m_types, _type)) { MessageSvc::Error("Type not supported in BinnedEffs::types", _type, "EXIT_FAILURE"); }
    if (!CheckVectorContains(m_modes, _mode)) { MessageSvc::Error("Type not supported in BinnedEffs::modes", _mode, "EXIT_FAILURE"); }
    //
    TString _key_return = TString(_mode);
    if (_type != "") {
        _key_return += "_" +_type;
    }
    if( raw ==true){
        _key_return +="_raw";
    }
    return _key_return;
};

TH1D BinnedEffs::EvalEfficiencyHisto(TH1D & sumW, TH1D & normN, TH1D & normD, const pair< double, double > & nMCDecay, bool normBin) {
    // Core functions to evaluate efficiency histogram
    auto nBins   = sumW.GetNbinsX();
    auto nBins_N = normN.GetNbinsX();
    auto nBins_D = normD.GetNbinsX();
    if (nBins != nBins_N || nBins != nBins_D) { MessageSvc::Error("Invalid histograms", "", "EXIT_FAILURE"); }
    
    auto sumW_total     = sumW.Integral(0, nBins + 1);
    auto normalization  = (normN.Integral(0, nBins + 1) / normD.Integral(0, nBins + 1));
    auto nTot           = nMCDecay.first;
    auto Eff_Bins_histo = (TH1D *) sumW.Clone(TString("EfficiencyHisto_") + sumW.GetName());
    //Set everything to 0
    Eff_Bins_histo->Reset("ICES");
    // fill histo bin by bin
    for (int binIDX = 0; binIDX < nBins + 1; ++binIDX) {
        auto sumW_bin     = sumW.GetBinContent(binIDX);
        auto sumW_bin_err = sumW.GetBinError(binIDX);
        auto eff_bin      = sumW_bin     * normalization / nTot;
        auto eff_bin_err  = sumW_bin_err * normalization / nTot;
        if( normBin == false){
            Eff_Bins_histo->SetBinContent(binIDX, eff_bin);
            Eff_Bins_histo->SetBinError(binIDX, eff_bin_err);
        }else{
            auto normN_sumW_BIN     = normN.GetBinContent(binIDX );
            auto normN_sumW_BIN_err = normN.GetBinError(binIDX );
            auto normD_sumW_BIN     = normD.GetBinContent(binIDX );
            auto normD_sumW_BIN_err = normD.GetBinError(binIDX );
            auto normBinVAL     = normN_sumW_BIN/normD_sumW_BIN;
            auto normBin_err = normBinVAL * TMath::Sqrt( TMath::Sq(normN_sumW_BIN/normN_sumW_BIN) + TMath::Sq(normD_sumW_BIN/normD_sumW_BIN)  );
            eff_bin     = sumW_bin * normBinVAL /nTot;
            eff_bin_err = eff_bin * TMath::Sqrt( TMath::Sq(sumW_bin_err/sumW_bin) +  TMath::Sq(normBin_err/normBinVAL) ) ;
            Eff_Bins_histo->SetBinContent(binIDX, eff_bin);
            Eff_Bins_histo->SetBinError(binIDX, eff_bin_err);            
        }
    }
    return *Eff_Bins_histo;
};



map< TString, TH1D > BinnedEffs::GetEfficienciesFromFile(TFile & f, const TString & varID, bool normBin ) {
    if( normBin){
        MessageSvc::Warning("GetEfficienciesFromFile normalizing in Bins", varID);
    }else{
        MessageSvc::Warning("GetEfficienciesFromFile NOT normalizing in Bins", varID);
    }
    // cout<<"LoadNMCDecayTupleEEntries "<<endl;
    auto GetNMCDeayTupleEntries = [&](TFile & inff) {
        auto vv = (RooRealVar *) inff.Get("nMCDecay");
        return make_pair(vv->getVal(), vv->getError());
    };
    map< TString, TH1D > histograms;
    auto                 _all_dirs_in_file = IOSvc::ListOfDirectoryNamesInFile(f, 1);
    if (!CheckVectorContains(_all_dirs_in_file, varID)) { MessageSvc::Error("Directory from file", varID + " in " + f.GetName(), "EXIT_FAILURE"); }
    for (const auto && [_type, _mode] : iter::product(m_types, m_modes)) {
        auto h    = (TH1D *) f.Get(varID + "/" + ID(_type, _mode));
        if (h == nullptr) { MessageSvc::Error("NullPtr loaded (isoBin)", "", "EXIT_FAILURE"); }
        // printoverflows( *h);
        histograms[ID(_type, _mode)] = *(TH1D *) h->Clone();
        histograms[ID(_type, _mode)].SetName(varID + "_" + histograms[ID(_type, _mode)].GetName());
        histograms[ID(_type, _mode)].SetDirectory(0);


        auto hRaw = (TH1D *) f.Get(varID + "/" + ID(_type, _mode, true));
        if (hRaw == nullptr) { MessageSvc::Error("NullPtr loaded (raw)", "", "EXIT_FAILURE"); }
        histograms[ID(_type, _mode, true)] = *(TH1D *) hRaw->Clone();
        histograms[ID(_type, _mode, true)].SetName(varID + "_" + histograms[ID(_type, _mode,true)].GetName());
        histograms[ID(_type, _mode, true)].SetDirectory(0);
        // cout<<" loaded "<< endl;
    }

    auto nMCDecay = GetNMCDeayTupleEntries(f);
    // MessageSvc::Warning("Juggling histograms to make eff-histo");
    // cout<<"Loop Fill "<<endl;
    for (const auto & _mode : m_types){
        TString _eff_ID   = "eff";
        TString _sumW_ID  = "sumW";
        TString _normN_ID = "normN";
        TString _normD_ID = "normD";
        if (_mode != "") {
            _eff_ID   += "_" + _mode;
            _sumW_ID  += "_" + _mode;
            _normN_ID += "_" + _mode;
            _normD_ID += "_" + _mode;
        }
        // cout<<"map Fill EvalEfficiencyHisto "<<endl;
        histograms[_eff_ID] = EvalEfficiencyHisto(histograms[_sumW_ID], histograms[_normN_ID], histograms[_normD_ID], nMCDecay,normBin);
        histograms[_eff_ID].SetDirectory(0);

        histograms[_eff_ID+"_raw"] = EvalEfficiencyHisto(histograms[_sumW_ID +"_raw"], 
                                                         histograms[_normN_ID+"_raw"], 
                                                         histograms[_normD_ID+"_raw"], 
                                                         nMCDecay);
        histograms[_eff_ID+"_raw"].SetDirectory(0);
    }
    return histograms;
}



map< TString, vector<TH1D> > BinnedEffs::GetEfficienciesFromFileBS(TFile & f, const TString & varID, bool normBin ) {
   
    MessageSvc::Warning("GetEfficienciesFromFileBS NOT normalizing in Bins", varID);
   
    map< TString, vector<TH1D> > histograms;
    auto _all_dirs_in_file = IOSvc::ListOfDirectoryNamesInFile(f, 1);
    if (!CheckVectorContains(_all_dirs_in_file, varID)) { MessageSvc::Error("Directory from file", varID + " in " + f.GetName(), "EXIT_FAILURE"); }
    TString _toGet = "eff_sumW";
    histograms["eff"].reserve(WeightDefRX::nBS );
    //this histogram is the binned efficiency value in each BS slot
    for( int iBS = 0; iBS < WeightDefRX::nBS; ++iBS){
        TString _getAt = TString::Format("%s/bs%i/%s", varID.Data(), iBS, _toGet.Data());
        // if( false ) MessageSvc::Debug("GetEfficienciesFromFileBS::Get");
        auto h = f.Get<TH1D>(_getAt);
        if (h == nullptr) { MessageSvc::Error("NullPtr loaded (isoBin)", "", "EXIT_FAILURE"); }
        // if( false )  MessageSvc::Debug("GetEfficienciesFromFileBS::Clone");
        histograms["eff"].push_back( * (TH1D*)h->Clone() );
        // printoverflows( *h);
        // if( false )  MessageSvc::Debug("GetEfficienciesFromFileBS::SetName");
        TString _myName  = histograms["eff"].back().GetName();
        TString _newName = TString::Format("bs%i_%s_%s", iBS, varID.Data(), _myName );
        histograms["eff"].back().SetName(_newName);
        // if( false )  MessageSvc::Debug("GetEfficienciesFromFileBS::SetDirectory(0)");
        histograms["eff"].back().SetDirectory(0);
    }
    return histograms;
}


map< TString, vector<TH1D> > BinnedEffs::GetEfficienciesFromFileBS(const TString & fileName, const TString & csvFile, bool normBin ){
    VariableBinning variableBinning(csvFile , "PASS", "PASS");
    // cout<<"opeening file"<<endl;
    TFile f(fileName, "READ");   // = IOSvc::OpenFile(fileName,  OpenMode::READ);
    auto  varID = variableBinning.varID();
    auto  hists = GetEfficienciesFromFileBS(f, varID, normBin);
    f.Close();
    return hists;    
}

map< TString, TH1D > BinnedEffs::GetEfficienciesFromFile(const TString & fileName, const TString & csvFile, bool normBin) {
    // cout<<"making _varBinning"<<endl;
    VariableBinning variableBinning(csvFile , "PASS", "PASS");
    // cout<<"opeening file"<<endl;
    TFile f(fileName, "READ");   // = IOSvc::OpenFile(fileName,  OpenMode::READ);
    auto  varID = variableBinning.varID();
    auto  hists = GetEfficienciesFromFile(f, varID, normBin);
    f.Close();
    return hists;
}
#endif
