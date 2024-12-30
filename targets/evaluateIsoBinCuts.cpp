#include "ParserSvc.hpp"

#include "IsoHelper.hpp"
#include "RXDataPlayer.hpp"
#include "TCanvas.h"
#include "TH2Poly.h"
#include <fmt_ostream.h>



int main(int argc, char ** argv) 
{
    auto tStart = chrono::high_resolution_clock::now();

    ParserSvc parser("");
    parser.Init(argc, argv);
    if (parser.Run(argc, argv) != 0) return 1;
    gStyle->SetOptStat(111111); //enable over & under flow checker for histograms.
    ConfigHolder sharedConfig;

    if (SettingDef::Fit::option.Contains("chainexctrg")) 
    {
        MessageSvc::Warning("Merging Trigger Tuples (create tuple tuple::option)");
        SettingDef::Tuple::chainexctrg = true;
    }

    ConfigHolder configMM(
            sharedConfig.GetProject(), 
            Analysis::MM, 
            sharedConfig.GetSample(), 
            sharedConfig.GetQ2bin(), 
            sharedConfig.GetYear(), 
            sharedConfig.GetPolarity(), 
            sharedConfig.GetTrigger(), 
            hash_triggerconf(SettingDef::Config::triggerConf),
            sharedConfig.GetBrem(), 
            sharedConfig.GetTrack());

    ConfigHolder configEE(
            sharedConfig.GetProject(), 
            Analysis::EE, 
            sharedConfig.GetSample(), 
            sharedConfig.GetQ2bin(), 
            sharedConfig.GetYear(), 
            sharedConfig.GetPolarity(), 
            sharedConfig.GetTrigger(), 
            hash_triggerconf(SettingDef::Config::triggerConf),
            sharedConfig.GetBrem(), 
            sharedConfig.GetTrack());

    EventType eventTypeMM(configMM, CutHolder(configMM, SettingDef::Cut::option), WeightHolder(configMM, SettingDef::Weight::option), TupleHolder(configMM, SettingDef::Tuple::option), true);
    EventType eventTypeEE(configEE, CutHolder(configEE, SettingDef::Cut::option), WeightHolder(configEE, SettingDef::Weight::option), TupleHolder(configEE, SettingDef::Tuple::option), true);

    bool _onlyMuon = false;
    bool _onlyElectrons = false;
    if( SettingDef::Config::ana == "MM"){
      MessageSvc::Warning("Doing only Muon!");
      _onlyMuon= true;
    }
    else if( SettingDef::Config::ana == "EE"){
      MessageSvc::Warning("Doing only Electrons!");
      _onlyElectrons = true;
    }
    
    if( _onlyElectrons && _onlyMuon ) {
      MessageSvc::Error("Invalid, either only MM or only EE , or both ");
    }


    // 1D iso-binning
    map< Prj, TString > _headNames = {{Prj::RK, "Bp"}, {Prj::RKst, "B0"}, {Prj::RPhi, "Bs"}};

    // Collect all the infors needed to produce all IsoBinning(S)
    vector< IsoBinningInputs > _isobinningTasks2D = {};
    vector< IsoBinningInputs > _isobinningTasks1D = {};
    for (auto && [ _uniqueID, _binningInfo] : SettingDef::Tuple::isoBins) {
        vector< TString >               isoBinVariables   = {};
        vector< pair<double, double >>  variables_min_max = {};
        vector< TString >               axisLabels = {};
        vector< int >                   nIsoBins        = {};
        TString classType   = get< 5 >(_binningInfo[0]); //must exist...
        MessageSvc::Info("IsoBinning infos for ", _uniqueID);
        if (_binningInfo.size() == 0 || _binningInfo.size() > 2) { MessageSvc::Error("IsoBinnning cannot be done for 0-vars or >2D : IF YOU NEED IT, IMPLEMENT IT! ", "", "EXIT_FAILURE"); }            
        //Loop over dimensionalities of the histogram to make                
        for (auto && [branchName, nBinsDim, minVal, maxVal, labelAxis, cType] : _binningInfo) {
            TString _branchLabel = branchName.Contains("{HEAD}")? branchName.ReplaceAll("{HEAD}",  _headNames.at(sharedConfig.GetProject()) ) : branchName ;
            pair<double, double> minMax = make_pair( minVal, maxVal);
            isoBinVariables.push_back( _branchLabel);
            nIsoBins.push_back( nBinsDim);
            variables_min_max.push_back( make_pair( minVal, maxVal));
            axisLabels.push_back( labelAxis);
        }
        MessageSvc::Info("Adding task for isoBinning");
        if( _binningInfo.size() == 1){ 
            _isobinningTasks1D.emplace_back(isoBinVariables, nIsoBins, variables_min_max, axisLabels , _uniqueID, classType);
            _isobinningTasks1D.back().Print();
        }else{
            _isobinningTasks2D.emplace_back(isoBinVariables, nIsoBins, variables_min_max, axisLabels , _uniqueID, classType);
            _isobinningTasks2D.back().Print();            
        }
    }
    BuildBinningScheme1D(eventTypeEE, eventTypeMM, _isobinningTasks1D , _onlyMuon, _onlyElectrons);

    for (const auto & _isoBin : _isobinningTasks2D) {
        // if (_isoBin.variables.size() == 1 && _isoBin.nBins.size() == 1) {
        if (_isoBin.variables.size() == 2 && _isoBin.nBins.size() == 2) {
            // implicitely assume you first bin on the first variable, then the second one for 2D
            // tuple< TString, int , double, double, TString > VarX = make_tuple(_isoBin.variables[0], _isoBin.nBins[0], _isoBin.MinMax[0].first, _isoBin.MinMax[0].second, _isoBin.labelAxis[0]);
            // tuple< TString, int , double, double, TString > VarY = make_tuple(_isoBin.variables[1], _isoBin.nBins[1], _isoBin.MinMax[1].first, _isoBin.MinMax[1].second, _isoBin.labelAxis[1]);
            TString _classType = _isoBin.HistoType;
            BuildBinningScheme2D(eventTypeEE, eventTypeMM, _isoBin, _onlyMuon, _onlyElectrons);
        } else {
            MessageSvc::Error("More than 2 D IsoBinning not implemented, try other solution", "", "EXIT_FAILURE");
        }
    }

    return 0;
}
