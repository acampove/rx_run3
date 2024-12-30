//-----------------------------------------------------------------------------
// Implementation file for target : efficiencyCreate
//
// 2018-11-15 : Renato Quagliani, Simone Bifani
//-----------------------------------------------------------------------------

#include "CounterHelpers.hpp"
#include "CutDefRX.hpp"
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

const vector< Year >     PROCESSABLE_YEAR = {Year::Y2011, Year::Y2012, Year::Y2015, Year::Y2016, Year::Y2017, Year::Y2018};
const vector< Prj >      PROCESSABLE_PRJ  = {Prj::RKst, Prj::RK, Prj::RPhi};
const vector< Polarity > PROCESSABLE_POL  = {Polarity::MD, Polarity::MU};
const vector< Analysis > PROCESSABLE_ANA  = {Analysis::EE, Analysis::MM};
const vector< Q2Bin >    PROCESSABLE_Q2   = {Q2Bin::All, Q2Bin::Low, Q2Bin::Central, Q2Bin::High, Q2Bin::JPsi, Q2Bin::Psi};


/**
 * @brief Custom class parsed for eficiency slot generations
 *
 */
class EffSlot {
  private:
    TString           m_wOpt                 = "";   // to use
    TString           m_cOpt                 = "";   // to append ( to baseline definition of a given sample )
    TString           m_wOptNormN            = "";   // to use for NormN
    TString           m_wOptNormD            = "";   // to use for NormD
    TString           m_wOptMCDecay          = "";   // to use for MCDecayTuple
    vector< TString > m_weightConfigurations = {};
    TString           m_cutSetNorm           = "";   // the cut-set map to use for NormCUT    
  public:
    EffSlot() = default;
    EffSlot(TString _wOpt, TString _cOpt, TString _wMCDecayOpt, TString _wOptNormN, TString _wOptNormD, TString _cutSetNormalization, vector< TString > _weightConfigurations)
        : m_wOpt(_wOpt)
        , m_cOpt(_cOpt)
        , m_wOptMCDecay(_wMCDecayOpt)
        , m_wOptNormN(_wOptNormN)
        , m_wOptNormD(_wOptNormD)
        , m_cutSetNorm(_cutSetNormalization)
        , m_weightConfigurations(_weightConfigurations){};
    void Print() const {
        MessageSvc::Info("EffSlot");
        cout << "\t wOpt       :    " << wOpt() << endl;
        cout << "\t cOpt       :    " << cOpt() << endl;
        cout << "\t wOptMCDecay:    " << wOptMCDecay() << endl;
        cout << "\t wOptNormN  :    " << wOptNormN() << endl;
        cout << "\t wOptNormD  :    " << wOptNormD() << endl;
        cout << "\t cutSetNorm :    " << cutSetNorm() << endl;
        cout << "\t weightConfs:   [";
        int i = 0;
        for (auto & cnorm : weightConfigs()) {
            if (i == 0 && weightConfigs().size() !=1)
                cout << " " << cnorm << "," << flush;
            else if (i == 0 && weightConfigs().size() ==1 ){
                cout << " " << cnorm << " ]" << flush;
            }
            else if (i != m_weightConfigurations.size() - 1)
                cout << cnorm << " , " << flush;
            else
                cout << cnorm << " ] " << endl;
            i++;
        }
        cout << endl;
        return;
    }
    TString           wOpt() const { return m_wOpt; }
    TString           cOpt() const { return m_cOpt; }
    TString           wOptNormN() const { return m_wOptNormN; }
    TString           wOptNormD() const { return m_wOptNormD; }
    TString           wOptMCDecay() const { return m_wOptMCDecay; }
    TString           cutSetNorm() const { return m_cutSetNorm; }
    vector< TString > ListAndCutsNorm() const {
        vector< TString > _listCuts;
        TString           _raw = m_cutSetNorm;
        if (_raw.Contains("&")) {
            _listCuts = TokenizeString(_raw.ReplaceAll(" ", ""), "&");
        } else {
            _listCuts = {_raw.ReplaceAll(" ", "")};
        }
        return _listCuts;
    }
    vector< TString > weightConfigs() const { return m_weightConfigurations; }
};

vector< EffSlot > LoadEffScanOption(TString yaml_ConfigFile, TString q2, TString ana, TString sample, bool signal = true) {
    MessageSvc::Info("LoadEffScanOption", q2, ana, sample);
    auto parserYaml = YAML::LoadFile(yaml_ConfigFile.Data());
    //Nodes must exist
    auto _node      = signal ? "scanStepsSig" : "scanStepsBkg";
    YAML::Node        _Efficiency_nodeYaml    = parserYaml["Efficiency"][_node];
    YAML::Node        _Samples_nodeYamlSample = parserYaml["Create"][q2.Data()][ana.Data()];
    TString _tmOptUse = "";
    
    for (auto const & _slot : _Samples_nodeYamlSample) {
        if (sample == _slot["sample"].as< TString >()){
            if( _slot["tmOption"] ) _tmOptUse = _slot["tmOption"] .as< TString >();
        }
    }
    vector< EffSlot > _effSlots;
    if (_Efficiency_nodeYaml.size() == 0) { MessageSvc::Error("Cannot load slots Efficiencies for", TString(_node), "EXIT_FAILURE"); }
    for (auto const & _slot : _Efficiency_nodeYaml){
        auto _cutOption = _slot["cOpt"].as< TString >();
        if( _tmOptUse != ""){
            MessageSvc::Warning("LoadEffScanOption, replacing truth matching cut from Samples Node (tmSig->tmOption, tmBkg->tmOption)");
            MessageSvc::Info( "Sample" , sample );
            MessageSvc::Info( "Ana", ana);
            MessageSvc::Info( "Q2" , q2);
            MessageSvc::Warning("OLD cutOption", _cutOption);
            _cutOption.ReplaceAll("tmSig", _tmOptUse).ReplaceAll("tmBkg", _tmOptUse);
            MessageSvc::Warning("NEW cutOption", _cutOption);
        }else{
            MessageSvc::Warning("LoadEffScanOption, using truth matching defined in ScanStep Yaml slots");
        }
        _effSlots.push_back(EffSlot(_slot["wOpt"].as< TString >(),
                                    _cutOption,
                                    _slot["wOptMCDecay"].as< TString >(),
                                    _slot["wOptNormN"].as< TString >(),
                                    _slot["wOptNormD"].as< TString >(),
                                    _slot["cutSetNorm"].as< TString >(),
                                    _slot["wConfigs"].as< vector< TString > >()));
        /* Efficiency::option of the job has BremFrac  , EE only enabled */                                     
        if( SettingDef::Efficiency::option.Contains("BremFrac")){  //&& ana == "EE"){ OVERDO IT ON MUONS, so naming is aligned 
                MessageSvc::Info("Adding for EE the 0G,1G,2G split on top of merged setup via CutOptions Brem0,Brem1,Brem2");
                _effSlots.push_back(EffSlot(_slot["wOpt"].as< TString >()+"-0G",
                                    _cutOption+"-Brem0",
                                    _slot["wOptMCDecay"].as< TString >(),
                                    _slot["wOptNormN"].as< TString >(),
                                    _slot["wOptNormD"].as< TString >(),
                                    _slot["cutSetNorm"].as< TString >(),
                                    _slot["wConfigs"].as< vector< TString > >()));                
                _effSlots.push_back(EffSlot(_slot["wOpt"].as< TString >()+"-1G",
                                    _cutOption+"-Brem1",
                                    _slot["wOptMCDecay"].as< TString >(),
                                    _slot["wOptNormN"].as< TString >(),
                                    _slot["wOptNormD"].as< TString >(),
                                    _slot["cutSetNorm"].as< TString >(),
                                    _slot["wConfigs"].as< vector< TString > >()));    
                _effSlots.push_back(EffSlot(_slot["wOpt"].as< TString >()+"-2G",
                                    _cutOption+"-Brem2",
                                    _slot["wOptMCDecay"].as< TString >(),
                                    _slot["wOptNormN"].as< TString >(),
                                    _slot["wOptNormD"].as< TString >(),
                                    _slot["cutSetNorm"].as< TString >(),
                                    _slot["wConfigs"].as< vector< TString > >()));                                                                                        
        }
    }
    int j = 1;
    for (auto ee : _effSlots) {
      cout<< BLUE << j++ <<") : " << RESET << endl;
      ee.Print();
    }
    return _effSlots;
}

vector< pair< string, string > > to_string_pairs(const vector< pair< TString, TString > > & _pairs) {
    vector< pair< string, string > > _pairs_out;
    for (auto const & el : _pairs) { _pairs_out.push_back(make_pair(el.first.Data(), el.second.Data())); }
    return _pairs_out;
}

inline bool CheckExecutable(const Year & _year, const Prj & _project, const Analysis & _analysis, const Polarity & _polarity, const Q2Bin & _q2Bin) {
    // You can process Only 1 Q2 at the time, 1 year per time, 1 analysis per time...
    if (!CheckVectorContains(PROCESSABLE_YEAR, _year)) MessageSvc::Error((TString) SettingDef::IO::exe,    (TString) "Invalid Year", "EXIT_FAILURE");
    if (!CheckVectorContains(PROCESSABLE_POL, _polarity)) MessageSvc::Error((TString) SettingDef::IO::exe, (TString) "Invalid Polarity", "EXIT_FAILURE");
    if (!CheckVectorContains(PROCESSABLE_ANA, _analysis)) MessageSvc::Error((TString) SettingDef::IO::exe, (TString) "Invalid Analysis", "EXIT_FAILURE");
    if (!CheckVectorContains(PROCESSABLE_PRJ, _project)) MessageSvc::Error((TString) SettingDef::IO::exe,  (TString) "Invalid Project", "EXIT_FAILURE");
    if (!CheckVectorContains(PROCESSABLE_Q2, _q2Bin)) MessageSvc::Error((TString) SettingDef::IO::exe,     (TString) "Invalid Q2Bin", "EXIT_FAILURE");
    return true;
}

void FillHisto1D(TH1D & _histo, const vector< double > & _X, const vector< double > & _W, const vector< bool > & _Sel, const vector< int > & _toKeep = {}) {
    if (_histo.GetEntries() != 0) { MessageSvc::Error("Refilling histo not admitted", "", "EXIT_FAILURE"); }
    if (_toKeep.size() == 0) {
        for (auto const && [x, w, s] : iter::zip(_X, _W, _Sel)) {
            if (s) _histo.Fill(x, w);
        }
        return;
    } else {
        for (auto const & _idx : _toKeep) {
            if (_Sel[_idx]) { _histo.Fill(_X[_idx], _W[_idx]); }
        }
        return;
    }
}

vector<TH1D> UnpackBSHistos1D( const TH2D & myH2){
    vector<TH1D> _histos;
    _histos.reserve(WeightDefRX::nBS);
    myH2.Print();
    //do the 100 projectionX for each y-bin which is a BSindex
    for( int i =0; i < WeightDefRX::nBS; ++i ){
        MessageSvc::Info("ProjectionX" , TString::Format("Bin Y %i", i));
        _histos.push_back(* (myH2.ProjectionX( TString::Format("%s_%i", myH2.GetTitle(),i), i+1, i+1, "e")));//Check option here (error recomputed what it means?)
    
        _histos[i].SetName( TString::Format("%s_%i",     myH2.GetName(), i));
        _histos[i].SetTitle( TString::Format("%s bs[%i]", myH2.GetTitle(),i));
    }
    return _histos;
}

void FillHisto2D(TH2D & _histo, const vector< double > & _X, const vector< double > & _Y, const vector< double > & _W, const vector< bool > & _Sel, const vector< int > & _toKeep = {}) {
    if (_histo.GetEntries() != 0) { MessageSvc::Error("Refilling histo not admitted", "", "EXIT_FAILURE"); }
    if (_toKeep.size() == 0) {
        for (auto const && [x, y, w, s] : iter::zip(_X, _Y, _W, _Sel)) {
            if (s) _histo.Fill(x, y, w);
        }
        return;
    } else {
        for (auto const & _idx : _toKeep) {
            if (_Sel[_idx]) { _histo.Fill(_X[_idx], _Y[_idx], _W[_idx]); }
        }
        return;
    }
}

void FillHisto2DPoly(TH2Poly & _histo, const vector< double > & _X, const vector< double > & _Y, const vector< double > & _W, const vector< bool > & _Sel, const vector< int > & _toKeep = {}) {
    if (_histo.GetEntries() != 0) { MessageSvc::Error("Refilling histo not admitted", "", "EXIT_FAILURE"); }
    if (_toKeep.size() == 0) {
        for (auto const && [x, y, w, s] : iter::zip(_X, _Y, _W, _Sel)) {
            if (s) _histo.Fill(x, y, w);
        }
        return;
    } else {
        for (auto const & _idx : _toKeep) {
            if (_Sel[_idx]) { _histo.Fill(_X[_idx], _Y[_idx], _W[_idx]); }
        }
        return;
    }
}

TString build_product_weight_string(const vector< TString > & _string) {
    if (_string.size() == 0) return "1.";
    TString _final = "(" + _string.front();
    for (int i = 1; i < _string.size(); ++i) { _final += ")*(" + _string.at(i); }
    _final += ")";
    return _final;
}

inline void PrintAndTestMap(const map< TString, pair< TupleHolder, vector< tuple< ConfigHolder, CutHolder, WeightHolder > > > > & mymap) {
    MessageSvc::Line();
    MessageSvc::Warning((TString) SettingDef::IO::exe, (TString) "Samples in map =", to_string(mymap.size()));
    vector< TString > all_sample_produced = {};
    for (auto && _sample : mymap) {
        MessageSvc::Line();
        MessageSvc::Warning((TString) SettingDef::IO::exe, (TString) "Sample to process =", _sample.first, TString(fmt::format("(nEntries = {0}, nSplits = {1})", _sample.second.first.GetTuple()->GetEntries(), _sample.second.second.size())));
        _sample.second.first.PrintInline();
        vector< TString > _names_assigned = {};
        for (auto & tp : _sample.second.second) {
            TString _name = get< 0 >(tp).GetTupleName(get< 1 >(tp).Option() + "-" + get< 2 >(tp).Option() + "-" + _sample.second.first.Option());
            MessageSvc::Warning((TString) SettingDef::IO::exe, (TString) "Booking", _name);
            get< 0 >(tp).PrintInline();
            get< 1 >(tp).PrintInline();
            get< 2 >(tp).PrintInline();
            if (find(_names_assigned.begin(), _names_assigned.end(), _name) != _names_assigned.end()) MessageSvc::Error((TString) SettingDef::IO::exe, _name, "already booked", "EXIT_FAILURE");
            _names_assigned.push_back(_name);
        }
        all_sample_produced.insert(all_sample_produced.end(), _names_assigned.begin(), _names_assigned.end());
    }
    MessageSvc::Line();
    MessageSvc::Warning((TString) SettingDef::IO::exe, (TString) "Map");
    for (auto & ss : all_sample_produced) { MessageSvc::Warning((TString) SettingDef::IO::exe, (TString) "Sample", ss); }
    MessageSvc::Line();
    return;
}

inline void PrintMap(const map< TString, TString > & _map) {
    std::cout << RED << "Label "
              << " \t " << YELLOW << " Expression " << std::endl;
    for (auto const & el : _map) { std::cout << RED << el.first << " \t " << YELLOW << el.second << std::endl; }
}

inline void PrintMapS(const map< TString, TCut > & _map) {
    std::cout << RED << "Label "
              << " \t " << YELLOW << " Expression " << std::endl;
    for (auto const & el : _map) { std::cout << RED << el.first << " \t " << YELLOW << el.second << std::endl; }
}

void LoadTH2DFlatness(map< TString, map< TString, TH2Poly * > > & _histo2D, map< TString, map< TString, TH2D * > > & _histo2D_raw, const vector< VariableBinning > & _vars, TString _effWeight, TString _normNumWeight, TString _normDenWeight) {
    if (EffDebug()) MessageSvc::Debug("Booking TH1D Histos for flatness");
    for (auto const & var : _vars) {
        if (!var.is1D()) {
            _histo2D[var.varID()]     = {{"sumW", (TH2Poly *) var.GetHistoClone(var.varID() + "_sumW", _effWeight + " | full")}, {"normN", (TH2Poly *) var.GetHistoClone(var.varID() + "_normN", _normNumWeight + " | norm")}, {"normD", (TH2Poly *) var.GetHistoClone(var.varID() + "_normD", _normDenWeight + " | norm")}, {"sumW_rnd", (TH2Poly *) var.GetHistoClone(var.varID() + "_sumW_rnd", _effWeight + " | full")}, {"normN_rnd", (TH2Poly *) var.GetHistoClone(var.varID() + "_normN_rnd", _normNumWeight + " | norm")}, {"normD_rnd", (TH2Poly *) var.GetHistoClone(var.varID() + "_normD_rnd", _normDenWeight + " | norm")}, {"sumW_bkgcat", (TH2Poly *) var.GetHistoClone(var.varID() + "_sumW_bkgcat", _effWeight + " | full")}, {"normN_bkgcat", (TH2Poly *) var.GetHistoClone(var.varID() + "_normN_bkgcat", _normNumWeight + " | norm")}, {"normD_bkgcat", (TH2Poly *) var.GetHistoClone(var.varID() + "_normD_bkgcat", _normDenWeight + " | norm")}};
            _histo2D_raw[var.varID()] = {{"sumW_raw", (TH2D *) var.GetRawHisto2D(var.varID() + "_sumW_raw", _effWeight + " | full", 100, 100)},
                                         {"normN_raw", (TH2D *) var.GetRawHisto2D(var.varID() + "_normN_raw", _normNumWeight + " | norm", 100, 100)},
                                         {"normD_raw", (TH2D *) var.GetRawHisto2D(var.varID() + "_normD_raw", _normDenWeight + " | norm", 100, 100)},
                                         {"sumW_rnd_raw", (TH2D *) var.GetRawHisto2D(var.varID() + "_sumW_rnd_raw", _effWeight + " | full", 100, 100)},
                                         {"normN_rnd_raw", (TH2D *) var.GetRawHisto2D(var.varID() + "_normN_rnd_raw", _normNumWeight + " | norm", 100, 100)},
                                         {"normD_rnd_raw", (TH2D *) var.GetRawHisto2D(var.varID() + "_normD_rnd_raw", _normDenWeight + " | norm", 100, 100)},
                                         {"sumW_bkgcat_raw", (TH2D *) var.GetRawHisto2D(var.varID() + "_sumW_bkgcat_raw", _effWeight + " | full", 100, 100)},
                                         {"normN_bkgcat_raw", (TH2D *) var.GetRawHisto2D(var.varID() + "_normN_bkgcat_raw", _normNumWeight + " | norm", 100, 100)},
                                         {"normD_bkgcat_raw", (TH2D *) var.GetRawHisto2D(var.varID() + "_normD_bkgcat_raw", _normDenWeight + " | norm", 100, 100)}};
        }
    }
    return;
}

void LoadTH1DFlatness(map< TString, map< TString, TH1D * > > & _histo1D, const vector< VariableBinning > & _vars, TString _effWeight, TString _normNumWeight, TString _normDenWeight) {
    if (EffDebug()) MessageSvc::Debug("Booking TH1D Histos for flatness");
    for (auto const & var : _vars) {
        if (var.is1D()) {
            _histo1D[var.varID()] = {{"sumW_rnd", (TH1D *) var.GetHistoClone(var.varID() + "_sumW_rnd", _effWeight + " | full")},
                                     {"normN_rnd", (TH1D *) var.GetHistoClone(var.varID() + "_normN_rnd", _normNumWeight + " | norm")},
                                     {"normD_rnd", (TH1D *) var.GetHistoClone(var.varID() + "_normD_rnd", _normDenWeight + " | norm")},
                                     {"sumW_bkgcat", (TH1D *) var.GetHistoClone(var.varID() + "_sumW_bkgcat", _effWeight + " | full")},
                                     {"normN_bkgcat", (TH1D *) var.GetHistoClone(var.varID() + "_normN_bkgcat", _normNumWeight + " | norm")},
                                     {"normD_bkgcat", (TH1D *) var.GetHistoClone(var.varID() + "_normD_bkgcat", _normDenWeight + " | norm")},

                                     {"sumW_rnd_raw", (TH1D *) var.GetRawHisto1D(var.varID() + "_sumW_rnd_raw", _effWeight + " | full", 100)},
                                     {"normN_rnd_raw", (TH1D *) var.GetRawHisto1D(var.varID() + "_normN_rnd_raw", _normNumWeight + " | norm", 100)},
                                     {"normD_rnd_raw", (TH1D *) var.GetRawHisto1D(var.varID() + "_normD_rnd_raw", _normDenWeight + " | norm", 100)},

                                     {"sumW_bkgcat_raw", (TH1D *) var.GetRawHisto1D(var.varID() + "_sumW_bkgcat_raw", _effWeight + " | full", 100)},
                                     {"normN_bkgcat_raw", (TH1D *) var.GetRawHisto1D(var.varID() + "_normN_bkgcat_raw", _normNumWeight + " | norm", 100)},
                                     {"normD_bkgcat_raw", (TH1D *) var.GetRawHisto1D(var.varID() + "_normD_bkgcat_raw", _normDenWeight + " | norm", 100)}};
        }        
    }
    return;
}

void LoadTH1DModels(
        map< TString, map< TString, ROOT::RDF::TH1DModel > > & _histo1D, 
        const vector< VariableBinning > & _vars, 
        TString _effWeight, 
        TString _normNumWeight, 
        TString _normDenWeight, 
        bool isBS = false) 
{
    if (EffDebug()) 
        MessageSvc::Debug("Booking TH1D Histos for flatness");

    for (auto const & var : _vars) 
    {
        if (!var.is1D()) 
        {
            MessageSvc::Warning("LoadTH1DModels not supported for > 1D yet");
            continue;
        }

        _histo1D[var.varID()] = 
        {
            {"sumW",      var.GetHisto1DModel(var.varID()    + "_sumW"     , _effWeight     + " | full"     )}, 
            {"normN",     var.GetHisto1DModel(var.varID()    + "_normN"    , _normNumWeight + " | norm"     )}, 
            {"normD",     var.GetHisto1DModel(var.varID()    + "_normD"    , _normDenWeight + " | norm"     )},
            {"sumW_raw",  var.GetRawHisto1DModel(var.varID() + "_sumW_raw" , _effWeight     + " | full", 100)}, 
            {"normN_raw", var.GetRawHisto1DModel(var.varID() + "_normN_raw", _normNumWeight + " | norm", 100)}, 
            {"normD_raw", var.GetRawHisto1DModel(var.varID() + "_normD_raw", _normDenWeight + " | norm", 100)},
        };
    }
}

vector< pair< string, string > > GetVariablesForPlot(const vector< VariableBinning > & _vars) 
{
    vector< pair< string, string > > _variables_forPlot;
    for (auto const & var : _vars) 
    {
        TString _varDefine = var.varID() + "_X";
        _variables_forPlot.push_back(make_pair( _varDefine.Data(), var.varX().Data()));
        if (!var.is1D()) 
        { 
            TString _varDefine2 = var.varID() + "_Y";
            _variables_forPlot.push_back(make_pair(_varDefine2.Data(), var.varY().Data())); 
        }
    }

    return _variables_forPlot;
}

typedef pair< Trigger, TriggerConf > trigger_slice;
typedef pair< TString, TString >     weight_weightConfig;

typedef map< TString, double >                          sumW_values;
typedef map< TString, TString >                         expressions;
typedef map< TString, ROOT::RDF::RResultPtr< double > > sumWeights;

// juggler for columns  to enable rnd killing and bestbkgcat logics.
typedef map< TString, ROOT::RDF::RResultPtr< vector< double > > >                  column_double;
typedef map< TString, ROOT::RDF::RResultPtr< vector< bool > > >                    column_bool;
typedef map< TString, ROOT::RDF::RResultPtr< vector< pair< Int_t, Long64_t > > > > column_uniqueIDs;
typedef map< TString, ROOT::RDF::RResultPtr< vector< int > > >                     column_particleIDs;   // holder of vectors  Particle IDs...
// jugglers for histograms storage for various variables
typedef map< TString, ROOT::RDF::RResultPtr< TH1D > > histoEffs1DTYPES;   // holder of a single histogram [variable][histo]
typedef map< TString, map< TString, TH1D * > >        histoEffs1DTYPES_HISTO;

typedef map< TString, histoEffs1DTYPES >       histoEffs1D;
typedef map< TString, histoEffs1DTYPES_HISTO > histoEffs1D_HISTO;


auto bookkepingName(
        const EffSlot      & _effStepType, 
        const ConfigHolder & _ConH_BASE, 
        const TString      & _weightConfiguration, 
        bool clean         = false, 
        bool rootfile      = false) 
{
    TString _bookkepingName = _effStepType.wOpt() + "_Efficiency_" + _ConH_BASE.GetKey("addtrgconf");
    if (_effStepType.wOpt() != "no") 
        _bookkepingName = _bookkepingName + "_" + _weightConfiguration; 

    if (clean) 
        _bookkepingName.ReplaceAll("-", "_"); 

    if (rootfile) 
    {
        _bookkepingName = _effStepType.wOpt() + "_" + _ConH_BASE.GetSample() + "_Efficiency_" + _ConH_BASE.GetKey("addtrgconf");
        if (_effStepType.wOpt() != "no") 
            _bookkepingName = _bookkepingName + "_" + _weightConfiguration;

        _bookkepingName += ".root";
    }
    return _bookkepingName;
};

//Add to the Interpreter of ROOT when parsing strings operation the MAXV call which is used for the L0-Comb Systematic formula 
//MAXV( RVec<double> , RVec<double> ) is going to be used inside the Define("a","....MAXV"); See the ReplaceAll for the Weight at the end of the executable.
ROOT::VecOps::RVec<double> MAXV( const ROOT::VecOps::RVec<double>  &a, const ROOT::VecOps::RVec<double>  & b )
{
     ROOT::VecOps::RVec<double> vecOut(a.size());
     for( int i = 0 ; i < a.size() ; ++i)
         vecOut[i] = a[i]>b[i] ? a[i] : b[i];    

     return vecOut;                              
}

// Small helper function flattening  nested WeightOption - WeightConfig loop
auto flatten_effStepType (const vector< EffSlot > & slots) 
{
    vector< pair< EffSlot, TString > > _slot_weightConfig;
    for (auto & _effStepType : slots) 
    {
        for (auto & _weightConfiguration : _effStepType.weightConfigs()) 
            _slot_weightConfig.push_back(make_pair(_effStepType, _weightConfiguration)); 
    }

    return _slot_weightConfig;
}

// Bookkeping naming schemes...
// Boookkeping navigator methods.
auto IDTRG(const ConfigHolder & _ConH_BASE) 
{
    auto trg      = _ConH_BASE.GetTrigger();
    auto trg_conf = _ConH_BASE.GetTriggerConf();

    return make_pair(trg, trg_conf);
}

auto IDWEIGHT(const EffSlot & _slot, const TString _weightConfiguration) 
{ 
    return make_pair(_slot.wOpt(), _weightConfiguration); 
}

int main(int argc, char ** argv) 
{
    auto tStart = chrono::high_resolution_clock::now();

    // Save expressions
    auto toNamed = [](const TString & savingname, const TString & expression) {
        TNamed f(savingname.Data(), expression.Data());
        return f;
    };
    auto toNamedRooRealVar = [](const TString & savingname, const RooRealVar * var) {
        TNamed v(savingname, fmt::format("{0} +- {1}", var->getVal(), var->getError()));
        return v;
    };

    auto tobool        = [](const double & v) { return v == 1 ? true : false; };
    auto makeUniqueIDs = [](Int_t runN, Long64_t evtN) { return make_pair(runN, evtN); };

    //---  Start program
    //--------  Yaml file loading ------- //
    ParserSvc parser("");
    parser.Init(argc, argv);
    if (parser.Run(argc, argv) != 0) return 1;
    bool _testMode       = SettingDef::debug.Contains("TC") ? true : false;
    auto this_year       = hash_year(SettingDef::Config::year);
    auto this_polarity   = hash_polarity(SettingDef::Config::polarity);
    auto this_project    = hash_project(SettingDef::Config::project);
    auto this_analysis   = hash_analysis(SettingDef::Config::ana);
    auto this_q2binSlice = hash_q2bin(SettingDef::Config::q2bin);
    CheckExecutable(this_year, this_project, this_analysis, this_polarity, this_q2binSlice);
    TString _yaml_ConfigFile = parser.YAML();

    //--------  Get list of Samples to process and  Efficiency Slots  ------- //
    SettingDef::trowLogicError          = true;
    auto              samples_toprocess = ParserSvc::GetListOfSamples(_yaml_ConfigFile, this_q2binSlice, this_analysis);
    SettingDef::trowLogicError          = false;
    PrintAndTestMap(samples_toprocess);

    // Driver for usage of  MCDecayTuple denominator
    auto fUSEMCDECAYTUPLE = [&](const TupleHolder & hold) { return hold.GetConfigHolder().HasMCDecayTuple(); };
    auto fUSESIGNALSLOT   = [&](const TupleHolder & hold) { return hold.GetConfigHolder().IsSignalMC() ; };
    auto tos              = [&](const TString & string)   { return string.Data(); };

    auto getQ2BinEdges=[](){
        vector<double> edges{ 0.1,0.98,1.1,1.8,2.5,3.2,3.9,4.6,5.3,6.0, 7.0,8.0, 11.0,12.5,15,16,17,18,19,20,22};
        return edges;
    };

    auto getBPTBinEdges=[](){
        vector<double> edges{0.0,2500.0,3500.0,4000.0,4500.0,5000.0,5500.0,6000.0,6500.0,7500.0,10000.0,12500.0,15000.0,40000.0};
        return edges;
    };

    auto getBETABinEdges=[](){
        vector<double> edges{1.4,1.8,2.2,2.6,3.0,3.4,3.8,4.2,4.6,5.0,5.4,5.8,6.2};
        return edges;
    };

    for (auto && s : samples_toprocess) {
        bool _useSignal = fUSESIGNALSLOT( s.second.first) ? true : false;
        bool USEMCDECAYTUPLE = fUSEMCDECAYTUPLE(s.second.first);
        if(USEMCDECAYTUPLE){
            MessageSvc::Warning("USEMCDECAY");
        }else{
            MessageSvc::Warning("!USEMCDECAY");
        }
        auto _slot           =  LoadEffScanOption(_yaml_ConfigFile, to_string(this_q2binSlice), to_string(this_analysis), s.first, _useSignal);
        vector< pair< string, string > > _branches_weights_selections;
        vector< pair< string, string > > _branches_weights_selections_MCDecayTuple;
        TupleHolder                      _tupleShared = s.second.first;
        _tupleShared.Init();
        cout << RED << "DecayTuple (shared) Entries " << _tupleShared.TupleEntries() << RESET << endl;
        // set of extra variables to keep memory of
        vector< pair< string, string > > _extras = {{"runN", "(Int_t)runNumber"}, {"evtN", "(Long64_t)eventNumber"}};
        // set of variables to keep track of given seet of variables needeed for flatness plots...
        vector< pair< string, string > > _variables_forPlot = {};
        // set of extra variables to keep memory of , for truth matching infos for bkgcat logics
        auto                             _labels             = s.second.first.GetConfigHolder().GetParticleNames();
        TString                          _head               = _labels.at("HEAD");
        vector< pair< string, string > > _truthmatchinginfos = {{"HEAD_TRUEID", (string) _head + "_TRUEID"},
                                                                {"HEAD_BKGCAT", (string) _head + "_BKGCAT"},
                                                                {"L1_TRUEID", (string) _labels.at("L1") + "_TRUEID"},
                                                                {"L2_TRUEID", (string) _labels.at("L2") + "_TRUEID"},
                                                                {"H1_TRUEID", (string) _labels.at("H1") + "_TRUEID"},
                                                                {"H2_TRUEID", (string) _labels.at("H2") + "_TRUEID"}};
        auto                             aliases      = to_string_pairs(GetAllAlias(static_cast< TTree * >(_tupleShared.GetTuple())));
        // The maps holding all  results....

        map< TString, ROOT::RDF::RNode > NODESFILTERS;

        map< trigger_slice, map< weight_weightConfig, sumWeights > >             SUMW;
        map< trigger_slice, map< weight_weightConfig, expressions > >            SUMWExpr;
        map< trigger_slice, map< weight_weightConfig, sumW_values > >            SUMW_MCDECAY;
        map< trigger_slice, map< weight_weightConfig, histoEffs1D > >            HISTOFLATNESS_MCDecaySpecial;
        map< trigger_slice, map< weight_weightConfig, histoEffs1D > >            HISTOFLATNESS_MCDecayBPT;
        map< trigger_slice, map< weight_weightConfig, histoEffs1D > >            HISTOFLATNESS_MCDecayBETA;
        map< trigger_slice, map< weight_weightConfig, histoEffs1D > >            HISTOFLATNESS;
        map< trigger_slice, map< weight_weightConfig, histoEffs1DTYPES_HISTO > > HISTOFLATNESS_RNDKILL;

        map< trigger_slice, map< weight_weightConfig, column_bool > >        COLUMNS_SELECTION;
        map< trigger_slice, map< weight_weightConfig, column_double > >      COLUMNS_VALUES;
        map< trigger_slice, map< weight_weightConfig, column_uniqueIDs > >   COLUMNS_UNIQUEID;
        map< trigger_slice, map< weight_weightConfig, column_particleIDs > > COLUMNS_IDS;
        // Create a RDataFrame for DecayTuple and MCDecayTuple
        // ROOT::EnableImplicitMT(4);   //(4);
        ROOT::DisableImplicitMT();//(4);   //(4);
        ROOT::RDataFrame   dfDecay(*_tupleShared.GetTuple());
        ROOT::RDataFrame * dfMCDecay = nullptr;
        if (USEMCDECAYTUPLE) {
            TString _tupleName           = SettingDef::Tuple::tupleName;
            SettingDef::Tuple::tupleName = "MCDT";
            TupleHolder _tupleSharedMCDecay(_tupleShared.GetConfigHolder(), "", "MCT", _tupleShared.Option());
            _tupleSharedMCDecay.Init();
            SettingDef::Tuple::tupleName = _tupleName;
            cout << RED << "MCDecayTuple (tuple Entries)  " << _tupleSharedMCDecay.GetTuple()->GetEntries() << RESET << endl;
            _tupleSharedMCDecay.GetTuple()->SetBranchStatus("*",1);
            dfMCDecay = new ROOT::RDataFrame(*_tupleSharedMCDecay.GetTuple());                                   
        }
        
        vector< VariableBinning > _variables = GetVariableBinning(_tupleShared.GetConfigHolder().GetProject(), _tupleShared.GetConfigHolder().GetYear(), Trigger::L0L, TriggerConf::Exclusive);
        for( const auto &  v : _variables){
            MessageSvc::Debug("VARIABLE LABEL LOADED");
            v.Print();
        }


        

        _variables_forPlot = GetVariablesForPlot(_variables);
        if (_variables_forPlot.size() == 0) { _variables_forPlot.push_back(make_pair("DUMMYBRANCH", "1.")); }
        int nOperations        = 0;
        int nOperationsMCDecay = 0;
        nOperations += _truthmatchinginfos.size();
        nOperations += _variables_forPlot.size();
        auto last_node = ApplyDefines(dfDecay, aliases, true);
        last_node = ApplyDefines(last_node, _truthmatchinginfos, false);   // int casting !, do-not force double
        last_node = ApplyDefines(last_node, _extras, false);                // int casting !, do-not force double

        // Only if q2 histo added later ...
        if(SettingDef::Efficiency::option.Contains("HistoQ2")){
            MessageSvc::Debug("Adding q2_X , TMath::Sq(JPs_M/1000.) definition to tuples for the plotting!");
            last_node = last_node.Define("q2_X","TMath::Sq(JPs_M/1000.)");
        }
        // Only for BPT histo added later ...
        if(SettingDef::Efficiency::option.Contains("HistoBPT")){
            MessageSvc::Debug("Adding BPT_X, ETA_X definition to tuples for the plotting!");
            last_node = last_node.Define("BPT_X",(string) _head + "_TRUEPT");   
            last_node = last_node.Define("BETA_X",(string) _head + "_TRUEETA");
        }
        // MessageSvc::Info("Append Q2 Smearing values ");
        bool _doSmearing = false;
        TString _wOptAppend = "";
        for (auto && [_effStepType, _weightConfiguration] : flatten_effStepType(_slot)) {
            _doSmearing = _effStepType.wOpt().Contains("SMEAR") ? true : false;             
            if (_effStepType.wOpt().Contains("SMEAR")) _doSmearing = true;
            if (_effStepType.wOpt().Contains("PID") && !_wOptAppend.Contains("PID")) _wOptAppend += "-PID";
            if (_effStepType.wOpt().Contains("TRK") && !_wOptAppend.Contains("TRK")) _wOptAppend += "-TRK";
            if (_effStepType.wOpt().Contains("RW1D") && !_wOptAppend.Contains("RW1D")) _wOptAppend += "-RW1D";
            if (_effStepType.wOpt().Contains("L0") && !_wOptAppend.Contains("L0")) _wOptAppend += "-L0";
            if (_effStepType.wOpt().Contains("COMB") && !_wOptAppend.Contains("COMB")) _wOptAppend += "-COMB";
            if (_effStepType.wOpt().Contains("DIST") && !_wOptAppend.Contains("DIST")) _wOptAppend += "-DIST";
            if (_effStepType.wOpt().Contains("HLT-nTracks") && !_wOptAppend.Contains("HLT")) _wOptAppend += "-HLT-nTracks";
            if (_effStepType.wOpt().Contains("HLT") && !_wOptAppend.Contains("HLT")) _wOptAppend += "-HLT";
            if (_effStepType.wOpt().Contains("MODEL") && !_wOptAppend.Contains("MODEL")) _wOptAppend += "-MODEL";
        }
        if( _doSmearing && this_analysis == Analysis::EE){
            //TODO : COMMENT WHEN v11 append to tuple the correct Q2Smearing columns. (ask @Renato)        
            if( !last_node.HasColumn("JPs_M_smear_B0_fromMCDT_wMC") && 
                ( _tupleShared.GetConfigHolder().PortingEnabled() ) &&  last_node.HasColumn("JPs_TRUEM_MCDT") ){
                MessageSvc::Warning("Appending Q2 smearing columns sample", _tupleShared.GetConfigHolder().GetSample() );
               last_node = HelperProcessing::AppendQ2SmearColumns( last_node, this_project, this_year); //UNCOMMENT WHEN v10 Q2Smearing is available            
            }else{
                MessageSvc::Warning("Failing codition for appending Q2 smearing columns");                
            }
            if(!last_node.HasColumn("B_DTF_M_SMEAR_Bp_wMC") && 
               !last_node.HasColumn("B_DTF_M_SMEAR_B0_wMC") && 
               _tupleShared.GetConfigHolder().PortingEnabled()){
                MessageSvc::Warning("Appending B_DTF_M smearing columns sample", _tupleShared.GetConfigHolder().GetSample() );
                last_node = HelperProcessing::AppendBSmearColumns( last_node, this_project, this_year); //UNCOMMENT WHEN v10 Q2Smearing is available
            }else{
                MessageSvc::Warning("Failing codition for appending B smearing columns");                
            }
        }

        if( SettingDef::Efficiency::option.Contains("OnTheFly")){
            MessageSvc::Line();
            MessageSvc::Warning("Appending weights on the fly for effiency computation", _wOptAppend);
            auto _cHBasic =  s.second.first.GetConfigHolder();
            bool _useBStmp = SettingDef::Weight::useBS;
            SettingDef::Weight::useBS = false;
            last_node = HelperProcessing::AttachWeights(last_node, _cHBasic, _wOptAppend);
            SettingDef::Weight::useBS = _useBStmp;
        }
        bool slotHasSmearing = false; 
        for (auto && [_effStepType, _weightConfiguration] : flatten_effStepType(_slot)){ 
            if(_effStepType.wOpt().Contains("SMEAR")) slotHasSmearing = true;
        };
        last_node = last_node.Define("uniqueID", makeUniqueIDs, {"runN", "evtN"});
        last_node = ApplyDefines(last_node, _variables_forPlot, true);   // int casting !, do-not force double        
        for (auto && [_ConH_BASE, _CutH_BASE, _WeiH_BASE] : s.second.second) {
            vector< VariableBinning > _vars = GetVariableBinning(_ConH_BASE.GetProject(), _ConH_BASE.GetYear(), _ConH_BASE.GetTrigger(), _ConH_BASE.GetTriggerConf());
            //====== ADD THIS BY HAND! =====//           
            if(SettingDef::Efficiency::option.Contains("HistoQ2")){
                vector<double> q2BinEdges= getQ2BinEdges(); 
                ROOT::RDF::TH1DModel modelq2("q2Distributions","q2;q^{2} [GeV^{2}];#epsilon", (int)q2BinEdges.size()-1, q2BinEdges.data() );
                auto Q2Histo = VariableBinning(modelq2, "q2");
                _vars.push_back(Q2Histo);
            }
            if(SettingDef::Efficiency::option.Contains("HistoBPT")){
                vector<double> BPTBinEdges= getBPTBinEdges(); 
                ROOT::RDF::TH1DModel modelBPT("BPTDistributions","BPT;B_{PT} [MeV];#epsilon", (int)BPTBinEdges.size()-1, BPTBinEdges.data() );
                VariableBinning BPTHisto(modelBPT, "BPT");
                _vars.push_back(BPTHisto);

                vector<double> BETABinEdges= getBETABinEdges();
                ROOT::RDF::TH1DModel modelBETA("BETADistributions","ETA;ETA;#epsilon", (int)BETABinEdges.size()-1, BETABinEdges.data() );
                VariableBinning BETAHisto(modelBETA, "BETA");
                _vars.push_back(BETAHisto);
            }
            //=============================//
            if (IsCutInMap("cutEXTRA", _CutH_BASE.Cuts())) {
                SettingDef::Cut::extraCut = TString(_CutH_BASE.Cuts().at("cutEXTRA"));
                MessageSvc::Warning("Settting Global ExtraCut to ", SettingDef::Cut::extraCut);
            }
            // Loop for effStep and Weight Configurations
            for (auto && [_effStepType, _weightConfiguration] : flatten_effStepType(_slot)) {
                // build fullSeleection, normSelection and weights  string expressions
                auto _CutH_slice = _CutH_BASE.Clone(_effStepType.cOpt());
                _CutH_slice.Init();
                auto _fullSelection = TString(_CutH_slice.Cut());
                //------ HAT TRICK REQUIRES THE B_DTF_M replacer of smeared B mass branches on TUPLES ! TODO : drop all this when reprocessing fully samples , porting properly B mass from MCDT -----//               
                TCut _NormCut       = TCut("");
                for (auto & _cutElement : _effStepType.ListAndCutsNorm()) { _NormCut = _NormCut && _CutH_slice.Cuts().at(_cutElement); }
                auto _normSelection = TString(_NormCut);
                if( this_analysis == Analysis::EE){
                    MessageSvc::Debug("Applying Variable replacing for B mass cut-with-smearing. wOption", _effStepType.wOpt());
                    TString _headUse = "NONE";
                    if(last_node.HasColumn("B0_DTF_M")) _headUse = "B0";
                    if(last_node.HasColumn("Bp_DTF_M")) _headUse = "Bp";
                    if(last_node.HasColumn("Bs_DTF_M")) _headUse = "Bs";                    
                    if( _headUse == "NONE"){ 
                        MessageSvc::Error("Please fix Head in EE-mode logic (effiencyCreateFast)", "","EXIT_FAILURE");
                    }
                    TString _varBMass = TString::Format("%s_DTF_M", _headUse.Data());
                    MessageSvc::Debug("Before (fullSele)", _fullSelection);
                    MessageSvc::Debug("Before (normSele)", _normSelection);
                    if( !_ConH_BASE.IsLeakageSample()){
                        if( _effStepType.wOpt().Contains("SMEARBP")){
                            _fullSelection = _fullSelection.ReplaceAll( _varBMass, "B_DTF_M_SMEAR_Bp_wMC");
                            _normSelection = _normSelection.ReplaceAll( _varBMass, "B_DTF_M_SMEAR_Bp_wMC");
                        }
                        if( _effStepType.wOpt().Contains("SMEARB0")){
                            _fullSelection = _fullSelection.ReplaceAll( _varBMass, "B_DTF_M_SMEAR_B0_wMC");
                            _normSelection = _normSelection.ReplaceAll( _varBMass, "B_DTF_M_SMEAR_B0_wMC");
                        }
                        MessageSvc::Debug("After (fullSele)", _fullSelection);
                        MessageSvc::Debug("After (normSele)", _normSelection);
                    }
                }
                MessageSvc::Info("cloning for weight Num");
                auto _WeiH_NUMERATOR = _WeiH_BASE.Clone(_weightConfiguration, _effStepType.wOpt());
                _WeiH_NUMERATOR.Init();
                MessageSvc::Info("cloning for weight Norm Num");
                auto _WeiH_NORMN = _WeiH_BASE.Clone(_weightConfiguration, _effStepType.wOptNormN());
                _WeiH_NORMN.Init();
                MessageSvc::Info("cloning for weight Norm Den");
                auto _WeiH_NORMD = _WeiH_BASE.Clone(_weightConfiguration, _effStepType.wOptNormD());
                _WeiH_NORMD.Init();
                TString _weightMCDecay        = "NONE";
                TString _selectionMCDecay     = "NONE";
                TString _selectionMCDecayNOQ2 = "NONE";
                if (USEMCDECAYTUPLE){
                    auto _WeiH_MCDecay = _WeiH_BASE.Clone(_weightConfiguration, _effStepType.wOptMCDecay());
                    EventType::Check(_ConH_BASE, _CutH_slice, _WeiH_MCDecay);
                    _weightMCDecay        = _WeiH_MCDecay.Weight()  + " * (1./(1.*totCandidates))"; //use this to "mult-cand kill on MCDecayTuple"
                    _selectionMCDecay     = _CutH_slice.Cuts().at("cutSPD");
                    _selectionMCDecayNOQ2 = _CutH_slice.Cuts().at("cutSPD");           
                    if (SettingDef::Efficiency::option.Contains("noSPD")) { _selectionMCDecay = "1>0"; }
                    if (SettingDef::Efficiency::option.Contains("noSPD")) { _selectionMCDecay = "1>0"; }
                    if (SettingDef::Efficiency::option.Contains("trueQ2")){
                        //Compute efficiencies so that MCDT used cuts at denominator with TRUE Q2 masses ! 
                        //This is needed to account for the "bin-migration" effect  [ pas ( q2 smeared ) / tot ( q2 true region of interest ! )]
                        //Such routine is OK only for "real" efficiencies in a given q2 region. For leakage samples ( JPSi -> central , or JPsi -> psi) , we just take all events at denominator
                        if( ( this_q2binSlice == Q2Bin::Low || this_q2binSlice == Q2Bin::Central) && !_ConH_BASE.IsLeakageSample()){
                            //Whatever didn't migrate and has trueQ2 cut gets normalized wrt the true q2! 
                            MessageSvc::Warning("ADDING TO DENOMINATOR THE TRUE Q2 CUT");
                            _selectionMCDecay = TString::Format("(%s) && (%s)"  , _selectionMCDecay.Data() ,   _CutH_slice.Cuts().at("cutQ2TRUE").GetTitle());
                            if(SettingDef::Efficiency::option.Contains("trueQ2PostFSR")){
                                MessageSvc::Warning("Using PostFSR true q2 cut");
                                _selectionMCDecay = _selectionMCDecay.ReplaceAll("JPs_TRUEM", "JPs_TRUEM_POSTFSR");
                            }
                            if( !_ConH_BASE.IsSignalMC()){                                
                                //JPs_TRUEM --> JPs_TRUEM_REPLACED for background samples ( v5 ana note onwards, before doesn't work on other samples! )
                                //This because we had bugged TupleProcess.cpp computer of JPs_TRUEM , we do now with the porting routine, see HelperProcessing.cpp, some samples ( not signal ones ) have part-recoed True q2 masses to be defined                                
                                _selectionMCDecay = _selectionMCDecay.ReplaceAll("JPs_TRUEM", "JPs_TRUEM_REPLACED"); //This is a cut on JPs_TRUEM applied at MCDT level ( bug-patch fix )
                            }
                        }
                    }
                }
                EventType::Check(_ConH_BASE, _CutH_slice, _WeiH_NUMERATOR);
                EventType::Check(_ConH_BASE, _CutH_slice, _WeiH_NUMERATOR);
                EventType::Check(_ConH_BASE, _CutH_slice, _WeiH_NORMN);
                EventType::Check(_ConH_BASE, _CutH_slice, _WeiH_NORMD);
                TString _effWeight      = _WeiH_NUMERATOR.Weight();
                TString _normNumWeight  = _WeiH_NORMN.Weight();
                TString _normDenWeight  = _WeiH_NORMD.Weight();
                TString _bookkepingName = bookkepingName(_effStepType, _ConH_BASE, _weightConfiguration, true);
                // df.Define(....).Define(....selection).Filter(selection).Sum<double>(  weight)
                auto _IDTRG = IDTRG(_ConH_BASE);
                auto _IDWEI = IDWEIGHT(_effStepType, _weightConfiguration);
                MessageSvc::Line();
                cout << CYAN << "Booking for " << _tupleShared.GetConfigHolder().GetSample() << "  " << to_string(_IDTRG.first) << "-" << to_string(_IDTRG.second) << RESET << endl;
                cout << CYAN << "wEnabled =  " << _IDWEI.first << "  wConfig = " << _IDWEI.second << RESET << endl;
                MessageSvc::Line();
                cout << MAGENTA << "FullSele    : " << CYAN << _fullSelection.Data() << RESET << endl;
                cout << MAGENTA << "NormSele    : " << CYAN << _normSelection.Data() << RESET << endl;
                cout << MAGENTA << "FullWeight  : " << CYAN << _effWeight.Data() << RESET << endl;
                cout << MAGENTA << "NormWeightN : " << CYAN << _normNumWeight.Data() << RESET << endl;
                cout << MAGENTA << "NormWeightD : " << CYAN << _normDenWeight.Data() << RESET << endl;
                MessageSvc::Line();
                // Results Sum
                MessageSvc::Info("Adding NodeFILTER");
                bool _fullSeleNode_alreadyExists = false;
                bool _normSeleNode_alreadyExists = false;
                if (NODESFILTERS.find(_fullSelection) == NODESFILTERS.end()) {
                    MessageSvc::Warning("Adding Filter for fullSelection ", _fullSelection);
                    NODESFILTERS.insert(std::pair< TString, ROOT::RDF::RNode >(_fullSelection, last_node.Define("fullSelection", _fullSelection.Data()).Filter("fullSelection")));
                } else {
                    MessageSvc::Warning("No NODEFILTER added (fullSelection)");
                }
                if (NODESFILTERS.find(_normSelection) == NODESFILTERS.end()) {
                    MessageSvc::Warning("Adding Filter for normSelection ", _normSelection);
                    NODESFILTERS.insert(std::pair< TString, ROOT::RDF::RNode >(_normSelection, last_node.Define("normSelection", _normSelection.Data()).Filter("normSelection")));
                } else {
                    MessageSvc::Warning("No NODEFILTER added (normSelection)");
                }
                auto df_fullSele = NODESFILTERS.find(_fullSelection)->second;
                auto df_normSele = NODESFILTERS.find(_normSelection)->second;

                auto df_fullSele_weights = df_fullSele.Define("weight", _effWeight.Data());
                auto df_normSele_weights = df_normSele.Define("weightNormN", _normNumWeight.Data()).Define("weightNormD", _normDenWeight.Data());
                // Extract columns...... Heavy vectors memory allocation.... for fullSelected Ntuple NODE
                COLUMNS_VALUES[_IDTRG][_IDWEI]["weight"]     = df_fullSele_weights.Take< double >("weight");
                COLUMNS_UNIQUEID[_IDTRG][_IDWEI]["uniqueID"] = df_fullSele_weights.Take< pair< Int_t, Long64_t > >("uniqueID");
                COLUMNS_IDS[_IDTRG][_IDWEI]["HEAD_BKGCAT"]   = df_fullSele_weights.Take< int >("HEAD_BKGCAT");
                COLUMNS_IDS[_IDTRG][_IDWEI]["L1_TRUEID"]     = df_fullSele_weights.Take< int >("L1_TRUEID");
                COLUMNS_IDS[_IDTRG][_IDWEI]["L2_TRUEID"]     = df_fullSele_weights.Take< int >("L2_TRUEID");
                COLUMNS_IDS[_IDTRG][_IDWEI]["H1_TRUEID"]     = df_fullSele_weights.Take< int >("H1_TRUEID");
                COLUMNS_IDS[_IDTRG][_IDWEI]["H2_TRUEID"]     = df_fullSele_weights.Take< int >("H2_TRUEID");

                // Extract columns...... Heavy vectors memory allocation.... for normSelected Ntuple NODE
                COLUMNS_VALUES[_IDTRG][_IDWEI]["weightNormN"]     = df_normSele_weights.Take< double >("weightNormN");
                COLUMNS_VALUES[_IDTRG][_IDWEI]["weightNormD"]     = df_normSele_weights.Take< double >("weightNormD");
                COLUMNS_UNIQUEID[_IDTRG][_IDWEI]["uniqueID_norm"] = df_normSele_weights.Take< pair< Int_t, Long64_t > >("uniqueID");
                COLUMNS_IDS[_IDTRG][_IDWEI]["HEAD_BKGCAT_norm"]   = df_normSele_weights.Take< int >("HEAD_BKGCAT");
                COLUMNS_IDS[_IDTRG][_IDWEI]["L1_TRUEID_norm"]     = df_normSele_weights.Take< int >("L1_TRUEID");
                COLUMNS_IDS[_IDTRG][_IDWEI]["L2_TRUEID_norm"]     = df_normSele_weights.Take< int >("L2_TRUEID");
                COLUMNS_IDS[_IDTRG][_IDWEI]["H1_TRUEID_norm"]     = df_normSele_weights.Take< int >("H1_TRUEID");
                COLUMNS_IDS[_IDTRG][_IDWEI]["H2_TRUEID_norm"]     = df_normSele_weights.Take< int >("H2_TRUEID");

                map< TString, map< TString, ROOT::RDF::TH1DModel > > HistoModels;
                map< TString, map< TString, TH1D * > >               HistoToFill;
                LoadTH1DFlatness(HistoToFill, _vars, _effWeight, _normNumWeight, _normDenWeight);
                LoadTH1DModels(HistoModels, _vars, _effWeight, _normNumWeight, _normDenWeight);

                HISTOFLATNESS_RNDKILL[_IDTRG][_IDWEI] = HistoToFill;
                if (_vars.size() != 0) {
                    for (auto & var : _vars) {
                        if (var.is1D()) {
                            TString branch = var.varID()+"_X"; //.first;
                            MessageSvc::Warning("Booking histograms for ", TString(branch));
                            // Load the column Values as well !
                            COLUMNS_VALUES[_IDTRG][_IDWEI][var.varID()]           = df_fullSele_weights.Take< double >(branch.Data());
                            COLUMNS_VALUES[_IDTRG][_IDWEI][var.varID() + "_norm"] = df_normSele_weights.Take< double >(branch.Data());
                            for (auto & el : {"sumW", "sumW_raw"}) {
                                HISTOFLATNESS[_IDTRG][_IDWEI][var.varID()][el] = df_fullSele_weights.Histo1D(HistoModels[var.varID()][el], branch.Data(), "weight");
                                nOperations++;
                            }
                            for (auto & el : {"normN", "normN_raw"}) {
                                HISTOFLATNESS[_IDTRG][_IDWEI][var.varID()][el] = df_normSele_weights.Histo1D(HistoModels[var.varID()][el], branch.Data(), "weightNormN");
                                nOperations++;
                            }
                            for (auto & el : {"normD", "normD_raw"}) {
                                HISTOFLATNESS[_IDTRG][_IDWEI][var.varID()][el] = df_normSele_weights.Histo1D(HistoModels[var.varID()][el], branch.Data(), "weightNormD");
                                nOperations++;
                            }
                        }
                    }
                }
                SUMW[_IDTRG][_IDWEI]["weightFull"]  = df_fullSele_weights.Sum< double >("weight");
                nOperations++;
                SUMW[_IDTRG][_IDWEI]["weightNormN"] = df_normSele_weights.Sum< double >("weightNormN");
                nOperations++;
                SUMW[_IDTRG][_IDWEI]["weightNormD"] = df_normSele_weights.Sum< double >("weightNormD");
                nOperations++;
                SUMWExpr[_IDTRG][_IDWEI]["normSelection"] = _normSelection;

                // Expressions bookkeping
                SUMWExpr[_IDTRG][_IDWEI]["fullSelection"] = _fullSelection;
                SUMWExpr[_IDTRG][_IDWEI]["weightFull"]    = _effWeight;
                SUMWExpr[_IDTRG][_IDWEI]["weightNormN"]   = _normNumWeight;
                SUMWExpr[_IDTRG][_IDWEI]["weightNormD"]   = _normDenWeight;
                // MCDecayTuple
                auto _mcdtentries = GetMCTEntries(_ConH_BASE.GetProject(), _ConH_BASE.GetYear(), _ConH_BASE.GetPolarity(), _ConH_BASE.GetSample(), true, "ngng_evt");
                SUMW_MCDECAY[_IDTRG][_IDWEI]["weightMCDecay_ngng"] = _mcdtentries;
                if (USEMCDECAYTUPLE) {
                    ROOT::RDF::RNode nodeMCT( *dfMCDecay);
                    if( SettingDef::Efficiency::option.Contains("OnTheFly")){
                        if( _effStepType.wOptMCDecay().Contains("RW1D")){
                            TString _wOptMCDTAppend = "-RW1D-MCT";
                            MessageSvc::Line();
                            MessageSvc::Warning("Appending MCDT weights on the fly for effiency computation", _wOptMCDTAppend);
                            bool _useBStmp = SettingDef::Weight::useBS;
                            SettingDef::Weight::useBS = false;
                            nodeMCT = HelperProcessing::AttachWeights(nodeMCT, _ConH_BASE, _wOptMCDTAppend);
                            SettingDef::Weight::useBS = _useBStmp;
                        }
                        if( _effStepType.wOptMCDecay().Contains("MODEL")) nodeMCT = HelperProcessing::AppendDecModelWeights( nodeMCT);
                    }
                    SUMW[_IDTRG][_IDWEI]["weightMCDecay"] = nodeMCT.Filter(_selectionMCDecay.Data()).Define("weight", _weightMCDecay.Data()).Sum< double >("weight");
                    if( SettingDef::Efficiency::option.Contains("HistoQ2")){
                        vector<double> edges = getQ2BinEdges();
                        HISTOFLATNESS_MCDecaySpecial[_IDTRG][_IDWEI]["q2"]["sumW_MCDT_q2preFSR"] = nodeMCT.Filter(_selectionMCDecayNOQ2.Data())
                                                                                                .Define("weight", _weightMCDecay.Data())
                                                                                                .Define("q2PreFSR", "TMath::Sq(JPs_TRUEM/1000.)")
                                                                                                .Histo1D( { "MCDT_q2PreFSR","MCDT_q2PreFSR", (int)edges.size()-1, edges.data() }, "q2PreFSR", "weight" );
                        HISTOFLATNESS_MCDecaySpecial[_IDTRG][_IDWEI]["q2"]["sumW_MCDT_q2postFSR"] = nodeMCT.Filter(_selectionMCDecayNOQ2.Data())
                                                                                                .Define("weight", _weightMCDecay.Data())
                                                                                                .Define("q2PostFSR", "TMath::Sq(JPs_TRUEM_POSTFSR/1000.)")
                                                                                                .Histo1D( { "MCDT_q2PostFSR","MCDT_q2PostFSR", (int)edges.size()-1, edges.data() }, "q2PostFSR", "weight" );
                    }
                    if( SettingDef::Efficiency::option.Contains("HistoBPT")){
                        vector<double> edges_PT = getBPTBinEdges();
                        HISTOFLATNESS_MCDecayBPT[_IDTRG][_IDWEI]["BPT"]["sumW_MCDT_BPT"] = nodeMCT.Filter(_selectionMCDecay.Data())
                                                                                                .Define("weight", _weightMCDecay.Data())
                                                                                                .Histo1D( { "MCDT_BPT","MCDT_BPT", (int)edges_PT.size()-1, edges_PT.data() }, _head + "_TRUEPT", "weight" );
                        vector<double> edges_ETA = getBETABinEdges();
                        HISTOFLATNESS_MCDecayBETA[_IDTRG][_IDWEI]["BETA"]["sumW_MCDT_BETA"] = nodeMCT.Filter(_selectionMCDecay.Data())
                                                                                                .Define("weight", _weightMCDecay.Data())
                                                                                                .Histo1D( { "MCDT_BETA","MCDT_BETA", (int)edges_ETA.size()-1, edges_ETA.data() }, _head + "_TRUEETA", "weight" );
                    }
                    nOperationsMCDecay++;
                    SUMWExpr[_IDTRG][_IDWEI]["weightMCDecay"]    = _weightMCDecay;
                    SUMWExpr[_IDTRG][_IDWEI]["MCDecaySelection"] = _selectionMCDecay;
                } else {
                    if( _mcdtentries <=0){
                        _ConH_BASE.PrintInline();
                        MessageSvc::Error("INVALID MCDTENTRIES USED! Efficiencies will make no sense","","EXIT_FAILURE");
                    }
                    SUMW_MCDECAY[_IDTRG][_IDWEI]["weightMCDecay"] = SUMW_MCDECAY[_IDTRG][_IDWEI]["weightMCDecay_ngng"];
                    SUMWExpr[_IDTRG][_IDWEI]["weightMCDecay"]     = "noWeight_ngngEvents";
                    SUMWExpr[_IDTRG][_IDWEI]["MCDecaySelection"]  = "noCut_ngngEvents";
                }
            }
        }
        MessageSvc::Info("Processing  DecayTuple             ", _tupleShared.GetConfigHolder().GetSample());
        auto lac = last_node.Count();

        std::string progressBar;
        std::mutex  barMutex;
        const auto  barWidth = 100.;
        const auto  everyN   = static_cast< long long unsigned int >(_tupleShared.GetTuple()->GetEntries() / 100.);

        //// ROOT::Internal::RDF::SaveGraph(df, _graph_computation.Data());   // dump the computational graph of define/filter/snapshot
        // lac.OnPartialResultSlot(everyN, [&barWidth, &progressBar, &barMutex](unsigned int, auto &) {
        //    std::lock_guard< std::mutex > l(barMutex);   // lock_guard locks the mutex at construction, releases it at destruction
        //    progressBar.push_back('#');
        //    // re-print the line with the progress bar
        //    std::cout << "\r[" << std::left << std::setw(100) << progressBar << ']' << std::flush;
        //});
        if (dfMCDecay != nullptr) {
            MessageSvc::Info("Processing  MCDecayTuple", _tupleShared.GetConfigHolder().GetSample());
            MessageSvc::Info("Processing  MCDecayTuple  nOperations", to_string(nOperationsMCDecay));
            *(*dfMCDecay).Count();
            MessageSvc::Info("Processing  MCDecayTuple done", _tupleShared.GetConfigHolder().GetSample());
        }

        MessageSvc::Info("Processing  DecayTuple  nOperations", to_string(nOperations));
        *lac;
        MessageSvc::Info("Processing  DecayTuple done", _tupleShared.GetConfigHolder().GetSample());


        for (auto && [_ConH_BASE, _CutH_BASE, _WeiH_BASE] : s.second.second) {
            auto _IDTRG = IDTRG(_ConH_BASE);
            cout << CYAN << "---- " << to_string(_IDTRG.first) << "-" << to_string(_IDTRG.second) << RESET << endl;
            for (auto && [_effStepType, _weightConfiguration] : flatten_effStepType(_slot)) {
                auto _IDWEI = IDWEIGHT(_effStepType, _weightConfiguration);
                // Get results reference concent....
                auto & w_results         = SUMW[_IDTRG][_IDWEI];           // results datadrame on  DecayTuple as RResultPtr<double>                                
                auto & w_results_mcdecay = SUMW_MCDECAY[_IDTRG][_IDWEI];   // results dataframe on  MCDecayTuple as double
                auto & w_exprs           = SUMWExpr[_IDTRG][_IDWEI];       // expressions used
                cout << CYAN << "\t  w" << _IDWEI.first << " # " << _IDWEI.second << " ----" << RESET << endl;
                if (USEMCDECAYTUPLE) {
                    w_results_mcdecay["weightMCDecay"] = w_results.at("weightMCDecay").GetValue();
                    MessageSvc::Info("SumW(weightMCDecay) ", w_results_mcdecay["weightMCDecay"]);
                    if( std::isnan(w_results_mcdecay["weightMCDecay"]) || std::isinf(w_results_mcdecay["weightMCDecay"] )  ){
                        MessageSvc::Error("SumW(MCDECAYTUPLE) is Nan or Inf FAILURE", "","EXIT_FAILURE");
                    }
                }else{
                    w_results_mcdecay["weightMCDecay"] = w_results_mcdecay["weightMCDecay_ngng"];
                }
                // cout<<"getting  sumW"<<  endl;
                auto SumW    = w_results.at("weightFull").GetValue();
                auto NormNum = w_results.at("weightNormN").GetValue();
                auto NormDen = w_results.at("weightNormD").GetValue();
                // cout<<"getting  sumW  MCDecayTuples"<<  endl;
                auto _TOTMCDECAYTUPLE     = w_results_mcdecay.at("weightMCDecay");
                auto _TOTMCDECAYTUPLE_gng = w_results_mcdecay.at("weightMCDecay_ngng");
                if(std::isnan( _TOTMCDECAYTUPLE)){
                    MessageSvc::Error("sumW(MCDecayTuple) is nan", "","EXIT_FAILURE");
                }
                //--------------- DEAl WITH FULLY SELECTED Entries -------------- //
                map< pair< Int_t, Long64_t >, vector< int > >                    _indexes_fullSelection;
                map< pair< Int_t, Long64_t >, vector< tuple< int, int, int > > > _indexes_fullSelection_nMatches;
                auto                                                             _pdgL = this_analysis == Analysis::MM ? PDG::ID::M : PDG::ID::E;
                // retrieve columns for FullSelection
                auto fS_weightEff = COLUMNS_VALUES[_IDTRG][_IDWEI]["weight"].GetValue();
                auto fS_IDs_Eff   = COLUMNS_UNIQUEID[_IDTRG][_IDWEI]["uniqueID"].GetValue();
                auto fS_L1_TRUEID = COLUMNS_IDS[_IDTRG][_IDWEI]["L1_TRUEID"].GetValue();
                auto fS_L2_TRUEID = COLUMNS_IDS[_IDTRG][_IDWEI]["L2_TRUEID"].GetValue();
                auto fS_H1_TRUEID = COLUMNS_IDS[_IDTRG][_IDWEI]["H1_TRUEID"].GetValue();
                auto fS_H2_TRUEID = COLUMNS_IDS[_IDTRG][_IDWEI]["H2_TRUEID"].GetValue();
                auto fS_B_bkgcat  = COLUMNS_IDS[_IDTRG][_IDWEI]["HEAD_BKGCAT"].GetValue();
                // retrieve columnd for NormSelection    (BE CAREFUL! _norm in map is needed!)
                map< pair< Int_t, Long64_t >, vector< int > >                    _indexes_normSelection;
                map< pair< Int_t, Long64_t >, vector< tuple< int, int, int > > > _indexes_normSelection_nMatches;
                auto                                                             nS_normWeighNum = COLUMNS_VALUES[_IDTRG][_IDWEI]["weightNormN"].GetValue();
                auto                                                             nS_normWeighDen = COLUMNS_VALUES[_IDTRG][_IDWEI]["weightNormD"].GetValue();
                auto                                                             nS_IDs_Eff      = COLUMNS_UNIQUEID[_IDTRG][_IDWEI]["uniqueID_norm"].GetValue();
                auto                                                             nS_L1_TRUEID    = COLUMNS_IDS[_IDTRG][_IDWEI]["L1_TRUEID_norm"].GetValue();
                auto                                                             nS_L2_TRUEID    = COLUMNS_IDS[_IDTRG][_IDWEI]["L2_TRUEID_norm"].GetValue();
                auto                                                             nS_H1_TRUEID    = COLUMNS_IDS[_IDTRG][_IDWEI]["H1_TRUEID_norm"].GetValue();
                auto                                                             nS_H2_TRUEID    = COLUMNS_IDS[_IDTRG][_IDWEI]["H2_TRUEID_norm"].GetValue();
                auto                                                             nS_B_bkgcat     = COLUMNS_IDS[_IDTRG][_IDWEI]["HEAD_BKGCAT_norm"].GetValue();
                // Dummy vectors
                vector< bool > _fullselectionPass;
                _fullselectionPass.resize(fS_weightEff.size());
                vector< bool > _normSelectionPass;
                _normSelectionPass.resize(nS_normWeighNum.size());
                if (true) {
                    cout << "------------------ FULL SELECTION SIZE CHECKERS ---------------" << endl;
                    cout << "Weight     size " << fS_weightEff.size() << endl;
                    cout << "IDS_Eff    size " << fS_IDs_Eff.size() << endl;
                    cout << "L1 trueID  size " << fS_L1_TRUEID.size() << endl;
                    cout << "L2 trueID  size " << fS_L2_TRUEID.size() << endl;
                    cout << "H1 trueID  size " << fS_H1_TRUEID.size() << endl;
                    cout << "H2 trueID  size " << fS_H2_TRUEID.size() << endl;
                    cout << "bkgcat     size " << fS_B_bkgcat.size() << endl;
                }
                if (true) {
                    cout << "------------------ NORM SELECTION SIZE CHECKERS ---------------" << endl;
                    cout << "normWNum   size " << nS_normWeighNum.size() << endl;
                    cout << "normWDen   size " << nS_normWeighDen.size() << endl;
                    cout << "IDS_Eff    size " << nS_IDs_Eff.size() << endl;
                    cout << "L1 trueID  size " << nS_L1_TRUEID.size() << endl;
                    cout << "L2 trueID  size " << nS_L2_TRUEID.size() << endl;
                    cout << "H1 trueID  size " << nS_H1_TRUEID.size() << endl;
                    cout << "H2 trueID  size " << nS_H2_TRUEID.size() << endl;
                    cout << "bkgcat     size " << nS_B_bkgcat.size() << endl;
                }
                // deal with full Selection and count the BKGCAT passing selection
                int             nPassFull = 0;
                map< int, int > nFullPassBKGCAT;
                // zip-for loop over trimmed-out columns for this slice of efficiency ( each column has length equal to the Full selection pass ! )
                // Perform multiple candidate killing filling the map( (RunNb, EvntNb) , vector<int> entries column index)
                for (auto const && [i_uniqueID, idx, l1_trueid, l2_trueid, h1_trueid, h2_trueid, b_bkgcat] : zip(fS_IDs_Eff, range(fS_weightEff.size()), fS_L1_TRUEID, fS_L2_TRUEID, fS_H1_TRUEID, fS_H2_TRUEID, fS_B_bkgcat)) {
                    nPassFull++;
                    _fullselectionPass[idx] = true;
                    int nMatches            = 0;
                    nMatches += abs(l1_trueid) == abs(_pdgL);
                    nMatches += abs(l2_trueid) == abs(_pdgL);
                    nMatches += abs(h1_trueid) == abs(PDG::ID::K);
                    if (_tupleShared.GetConfigHolder().GetSample().Contains("Bu2PiJPs")) {
                        // For background mode Bu2PiJps count as nMatches if K is Pion.
                        nMatches += abs(h1_trueid) == abs(PDG::ID::Pi);
                    }
                    if (_tupleShared.GetConfigHolder().GetSample().Contains("Lb2pK")) {
                        // For background mode Lb2pKJPs count as nMatches if Pion is Proton.
                        nMatches += abs(h2_trueid) == abs(PDG::ID::P);
                    }
                    if (this_project == Prj::RKst) nMatches += abs(h2_trueid) == abs(PDG::ID::Pi);
                    if (this_project == Prj::RPhi) nMatches += abs(h2_trueid) == abs(PDG::ID::K);
                    if (nFullPassBKGCAT.find(b_bkgcat) == nFullPassBKGCAT.end()) {
                        nFullPassBKGCAT[b_bkgcat] = 1;
                    } else {
                        nFullPassBKGCAT[b_bkgcat]++;
                    }
                    auto _bkgcatEVT = make_tuple(b_bkgcat, idx, nMatches);
                    _indexes_fullSelection[i_uniqueID].push_back(idx);
                    _indexes_fullSelection_nMatches[i_uniqueID].push_back(_bkgcatEVT);
                }

                int             nPassNorm = 0;
                map< int, int > nNormPassBKGCAT;

                for (auto const && [i_uniqueID, idx, l1_trueid, l2_trueid, h1_trueid, h2_trueid, b_bkgcat] : zip(nS_IDs_Eff, range(nS_normWeighNum.size()), nS_L1_TRUEID, nS_L2_TRUEID, nS_H1_TRUEID, nS_H2_TRUEID, nS_B_bkgcat)) {
                    nPassNorm++;
                    _normSelectionPass[idx] = true;
                    int nMatches            = 0;
                    nMatches += abs(l1_trueid) == abs(_pdgL);
                    nMatches += abs(l2_trueid) == abs(_pdgL);
                    nMatches += abs(h1_trueid) == abs(PDG::ID::K);
                    if (_tupleShared.GetConfigHolder().GetSample().Contains("Bu2PiJPs")) {
                        // For background mode Bu2PiJps count as nMatches if K is Pion.
                        nMatches += abs(h1_trueid) == abs(PDG::ID::Pi);
                    }
                    if (_tupleShared.GetConfigHolder().GetSample().Contains("Lb2pK")) {
                        // For background mode Lb2pKJPs count as nMatches if Pion is Proton.
                        nMatches += abs(h2_trueid) == abs(PDG::ID::P);
                    }
                    if (this_project == Prj::RKst) nMatches += abs(h2_trueid) == abs(PDG::ID::Pi);
                    if (this_project == Prj::RPhi) nMatches += abs(h2_trueid) == abs(PDG::ID::K);
                    if (nNormPassBKGCAT.find(b_bkgcat) == nNormPassBKGCAT.end()) {
                        nNormPassBKGCAT[b_bkgcat] = 1;
                    } else {
                        nNormPassBKGCAT[b_bkgcat]++;
                    }
                    auto _bkgcatEVT = make_tuple(b_bkgcat, idx, nMatches);
                    _indexes_normSelection[i_uniqueID].push_back(idx);
                    _indexes_normSelection_nMatches[i_uniqueID].push_back(_bkgcatEVT);
                }
                MessageSvc::Info("===== Stats on counters (Init maps for Mult_cand_killing) ===== ");
                cout << CYAN << "Tot fullSele           : " << nPassFull << endl;
                cout << CYAN << "Tot normSele           : " << nPassNorm << endl;
                for (auto & el : nFullPassBKGCAT) { cout << CYAN << "(f) #Pass BKGCAT " << el.first << " : " << el.second << endl; }
                for (auto & el : nNormPassBKGCAT) { cout << CYAN << "(n) #Pass BKGCAT " << el.first << " : " << el.second << endl; }
                // Random killing , generate list to keep from overlap
                map< pair< int, long >, int > _to_keep;
                map< pair< int, long >, int > _to_keep_norm;
                map< pair< int, long >, int > _to_keep_bestIDMatch;
                map< pair< int, long >, int > _to_keep_bestIDMatch_norm;
                TRandom3                      rnd;
                // PICK RANDOM single candidates (all entries in fullSelection must be in normSelection ! )
                MessageSvc::Debug("pick random single candidate from FullSelection Pass");
                for (auto const & _id_idxs : _indexes_fullSelection) {
                    // do MultCand Killing on fully selected entries
                    if (_id_idxs.second.size() == 1) {
                        _to_keep[_id_idxs.first] = _id_idxs.second[0];   // stoe the index to keep (ONLY 1 ! )
                    } else {
                        _to_keep[_id_idxs.first] = _id_idxs.second[std::floor(rnd.Uniform(0, _id_idxs.second.size()))];
                    }
                }
                for (auto const & _idx_idxs_norm : _indexes_normSelection) {
                    if (_idx_idxs_norm.second.size() == 1) {
                        _to_keep_norm[_idx_idxs_norm.first] = _idx_idxs_norm.second[0];
                        // store the index to keep (ONLY 1 ! )
                    } else {
                        _to_keep_norm[_idx_idxs_norm.first] = _idx_idxs_norm.second[std::floor(rnd.Uniform(0, _idx_idxs_norm.second.size()))];
                    }
                }
                MessageSvc::Debug("pick random bestbkgcat cands");
                for (auto const & [evt_ID, list_bkgcatevent] : _indexes_fullSelection_nMatches) {
                    if (list_bkgcatevent.size() == 1) {
                        _to_keep_bestIDMatch[evt_ID] = get< 1 >(list_bkgcatevent.front());
                    } else {
                        vector< tuple< int, int, int > > to_sort = list_bkgcatevent;
                        stable_sort(to_sort.begin(), to_sort.end(), [](const tuple< int, int, int > & a, const tuple< int, int, int > & b) {
                            auto a_bkgcat   = get< 0 >(a);
                            auto b_bkgcat   = get< 0 >(b);
                            auto a_nMatches = get< 2 >(a);
                            auto b_nMatches = get< 2 >(b);
                            if (a_bkgcat < b_bkgcat) {
                                return true;
                            } else if (a_bkgcat > b_bkgcat) {
                                return false;
                            } else {
                                return a_nMatches > b_nMatches;
                            }
                            // left with first/second having same bkgcat and nmatchees
                            return false;
                        });
                        if (get< 0 >(to_sort[0]) == get< 0 >(to_sort[1]) && get< 2 >(to_sort[0]) == get< 2 >(to_sort[1])) {
                            int  nEqualComparisons = 1;
                            auto _bkgcat           = get< 0 >(to_sort[0]);
                            auto _nMatch           = get< 2 >(to_sort[0]);
                            for (int element_index = 1; element_index < to_sort.size(); ++element_index) {
                                auto el = to_sort[element_index];
                                if (get< 0 >(el) == _bkgcat && get< 2 >(el) == _nMatch) { nEqualComparisons++; }
                            }
                            _to_keep_bestIDMatch[evt_ID] = get< 1 >(to_sort.at(std::floor(rnd.Uniform(0, nEqualComparisons))));
                        } else {
                            _to_keep_bestIDMatch[evt_ID] = get< 1 >(to_sort.front());
                        }
                    }
                }
                MessageSvc::Debug("pick random bestbkgcat cands toKeep BestIDMATCH FILL");
                for (const auto & [evt_ID, list_bkgcatevent] : _indexes_normSelection_nMatches) {
                    if (list_bkgcatevent.size() == 1) {
                        _to_keep_bestIDMatch_norm[evt_ID] = get< 1 >(list_bkgcatevent.front());   //.idx;
                    } else {
                        // Sort the bkgcatEVENT being in the same "EVTNB, RUNNB" with : smaller bkgcat in front. If Same bkgcat, highest NMATCH in front, if Same bkgcat and same NMATCH, coinflip random keep
                        vector< tuple< int, int, int > > to_sort = list_bkgcatevent;
                        stable_sort(to_sort.begin(), to_sort.end(), [](const tuple< int, int, int > & a, const tuple< int, int, int > & b) {
                            auto a_bkgcat   = get< 0 >(a);
                            auto b_bkgcat   = get< 0 >(b);
                            auto a_nMatches = get< 2 >(a);
                            auto b_nMatches = get< 2 >(b);
                            if (a_bkgcat < b_bkgcat) {
                                return true;
                            } else if (a_bkgcat > b_bkgcat) {
                                return false;
                            } else {
                                return a_nMatches > b_nMatches;
                            }
                            return true;
                        });
                        if (get< 0 >(to_sort[0]) == get< 0 >(to_sort[1]) && get< 2 >(to_sort[0]) == get< 2 >(to_sort[1])) {
                            int  nEqualComparisons = 1;
                            auto _bkgcat           = get< 0 >(to_sort[0]);
                            auto _nMatch           = get< 2 >(to_sort[0]);
                            for (int element_index = 1; element_index < to_sort.size(); ++element_index) {
                                auto el = to_sort[element_index];
                                if (get< 0 >(el) == _bkgcat && get< 2 >(el) == _nMatch) { nEqualComparisons++; }
                            }
                            _to_keep_bestIDMatch_norm[evt_ID] = get< 1 >(to_sort.at(std::floor(rnd.Uniform(0, nEqualComparisons))));
                        } else {
                            _to_keep_bestIDMatch_norm[evt_ID] = get< 1 >(to_sort.front());
                        }
                    }
                    // }
                }

                auto FillListMultCand = [](const map< pair< int, long >, int > & toKeepMap, const vector< bool > & selectioCheck) {
                    vector< int > list_of_indexes;
                    for (auto const & indexes : toKeepMap) {
                        auto _entry = indexes.second;
                        if (selectioCheck.at(_entry)) {
                            list_of_indexes.push_back(_entry);
                        } else {
                            MessageSvc::Error("Invalid Loop ", "", "EXIT_FAILURE");
                        }
                    }
                    return list_of_indexes;
                };
                auto CountWeights = [](const map< pair< int, long >, int > & toKeepMap, const vector< bool > & selectioCheck, const vector< double > & weightVector) {
                    if (weightVector.size() != selectioCheck.size()) { MessageSvc::Error("CountWeights", (TString) "Illegal vectors", "EXIT_FAILURE"); }
                    double sumW = 0;
                    for (auto const & indexes : toKeepMap) {
                        auto _entry = indexes.second;
                        if (selectioCheck.at(_entry)) {
                            sumW += weightVector.at(_entry);
                        } else {
                            MessageSvc::Error("Invalid Loop ", "", "EXIT_FAILURE");
                        }
                    }
                    return sumW;
                };
                auto DummyWeightCount = [](const vector< double > & weightVector, const vector< bool > & selectioCheck) {
                    if (weightVector.size() != selectioCheck.size()) { MessageSvc::Error("DummyWeightCounter", (TString) "Illegal vectors", "EXIT_FAILURE"); }
                    double sumW = 0;
                    for (auto const && [selection, weight] : iter::zip(selectioCheck, weightVector)) {
                        if (selection) sumW += weight;
                    }
                    return sumW;
                };
                auto SumW_dummy    = DummyWeightCount(fS_weightEff, _fullselectionPass);
                auto NormNum_dummy = DummyWeightCount(nS_normWeighNum, _normSelectionPass);
                auto NormDen_dummy = DummyWeightCount(nS_normWeighDen, _normSelectionPass);

                auto entries_rnd         = FillListMultCand(_to_keep, _fullselectionPass);
                auto entries_rnd_norm    = FillListMultCand(_to_keep_norm, _normSelectionPass);
                auto SumW_rnd            = CountWeights(_to_keep, _fullselectionPass, fS_weightEff);
                auto NormNum_rnd         = CountWeights(_to_keep_norm, _normSelectionPass, nS_normWeighNum);
                auto NormDen_rnd         = CountWeights(_to_keep_norm, _normSelectionPass, nS_normWeighDen);
                auto entries_bkgcat      = FillListMultCand(_to_keep_bestIDMatch, _fullselectionPass);
                auto entries_bkgcat_norm = FillListMultCand(_to_keep_bestIDMatch_norm, _normSelectionPass);
                auto SumW_bkgcat         = CountWeights(_to_keep_bestIDMatch, _fullselectionPass, fS_weightEff);
                auto NormNum_bkgcat      = CountWeights(_to_keep_bestIDMatch_norm, _normSelectionPass, nS_normWeighNum);
                auto NormDen_bkgcat      = CountWeights(_to_keep_bestIDMatch_norm, _normSelectionPass, nS_normWeighDen);

                auto & _histo1D = HISTOFLATNESS_RNDKILL[_IDTRG][_IDWEI];
                for (auto & var : _histo1D) {
                    // For each Variablee... we have to Take<double>
                    auto varID = var.first;
                    MessageSvc::Info("Filling histograms by hand after multiple candidate removal  for   ", varID);
                    auto varColumn      = COLUMNS_VALUES[_IDTRG][_IDWEI][varID].GetValue();
                    auto varColumn_Norm = COLUMNS_VALUES[_IDTRG][_IDWEI][varID + "_norm"].GetValue();
                    cout << "-------------- FULL -----------------" << endl;
                    cout << "column loaded varColumn            with size =  " << varColumn.size() << endl;
                    cout << "column loaded fullselectionPass    with size =  " << _fullselectionPass.size() << endl;
                    cout << "column loaded WeightEff(rnd)       with size =  " << fS_weightEff.size() << endl;
                    cout << "column loaded Entries(rnd)         with size =  " << entries_rnd.size() << endl;
                    cout << "column loaded Entries(bkgcat)      with size =  " << entries_bkgcat.size() << endl;
                    cout << "-------------- NORM -----------------" << endl;
                    cout << "column loaded varColumn            with size =  " << varColumn_Norm.size() << endl;
                    cout << "column loaded normselectionPass    with size =  " << _normSelectionPass.size() << endl;
                    cout << "column loaded WeightEffNum(rnd)    with size =  " << nS_normWeighNum.size() << endl;
                    cout << "column loaded WeightEffDen(rnd)    with size =  " << nS_normWeighDen.size() << endl;
                    cout << "column loaded Entries(rnd)         with size =  " << entries_rnd_norm.size() << endl;
                    cout << "column loaded Entries(bkgcat)      with size =  " << entries_bkgcat_norm.size() << endl;

                    FillHisto1D(*_histo1D.at(varID).at("sumW_rnd"), varColumn,              fS_weightEff,    _fullselectionPass, entries_rnd);
                    FillHisto1D(*_histo1D.at(varID).at("sumW_rnd_raw"), varColumn,          fS_weightEff,    _fullselectionPass, entries_rnd);
                    FillHisto1D(*_histo1D.at(varID).at("normN_rnd"), varColumn_Norm,        nS_normWeighNum, _normSelectionPass, entries_rnd_norm);
                    FillHisto1D(*_histo1D.at(varID).at("normN_rnd_raw"), varColumn_Norm,    nS_normWeighNum, _normSelectionPass, entries_rnd_norm);
                    FillHisto1D(*_histo1D.at(varID).at("normD_rnd"), varColumn_Norm,        nS_normWeighDen, _normSelectionPass, entries_rnd_norm);
                    FillHisto1D(*_histo1D.at(varID).at("normD_rnd_raw"), varColumn_Norm,    nS_normWeighDen, _normSelectionPass, entries_rnd_norm);
                    FillHisto1D(*_histo1D.at(varID).at("sumW_bkgcat"), varColumn,           fS_weightEff,    _fullselectionPass, entries_bkgcat);
                    FillHisto1D(*_histo1D.at(varID).at("sumW_bkgcat_raw"), varColumn,       fS_weightEff,    _fullselectionPass, entries_bkgcat);
                    FillHisto1D(*_histo1D.at(varID).at("normN_bkgcat"), varColumn_Norm,     nS_normWeighNum, _normSelectionPass, entries_bkgcat_norm);
                    FillHisto1D(*_histo1D.at(varID).at("normN_bkgcat_raw"), varColumn_Norm, nS_normWeighNum, _normSelectionPass, entries_bkgcat_norm);
                    FillHisto1D(*_histo1D.at(varID).at("normD_bkgcat"), varColumn_Norm,     nS_normWeighDen, _normSelectionPass, entries_bkgcat_norm);
                    FillHisto1D(*_histo1D.at(varID).at("normD_bkgcat_raw"), varColumn_Norm, nS_normWeighDen, _normSelectionPass, entries_bkgcat_norm);
                }
                
                if (true) {
                    cout << RED << "[def]     sumW       " << SumW << endl;
                    cout << "[def]            sumW norm N       " << NormNum << endl;
                    cout << "[def]            sumW norm D       " << NormDen << endl;
                    cout << "[defDummy]       sumW       " << SumW_dummy << endl;
                    cout << "[defDummy]       sumW norm N       " << NormNum_dummy << endl;
                    cout << "[defDummy]       sumW norm D       " << NormDen_dummy << endl;
                    cout << "" << endl;
                    cout << "[rnd]            sumW              " << SumW_rnd << endl;
                    cout << "[rnd]            sumW norm N       " << NormNum_rnd << endl;
                    cout << "[rnd]            sumW norm D       " << NormDen_rnd << endl;
                    cout << "" << endl;
                    cout << "[bkgcat]         sumW           " << SumW_bkgcat << endl;
                    cout << "[bkgcat]         sumW norm N    " << NormNum_bkgcat << endl;
                    cout << "[bkgcat]         sumW norm D    " << NormDen_bkgcat << endl;
                    cout << "" << endl;
                    cout << "[def]  sumW mcdecayTuple    " << _TOTMCDECAYTUPLE << endl;
                    cout << "[def]  sumW MCT (gng)       " << _TOTMCDECAYTUPLE_gng << RESET << endl;
                }
                // Persistency to disk  of  RooRealVar and summary efficiencies
                double _norm        = (double) NormNum / (double) NormDen;
                double _norm_rnd    = (double) NormNum_rnd / (double) NormDen_rnd;
                double _norm_bkgcat = (double) NormNum_bkgcat / (double) NormDen_bkgcat;

                double _efficieny        = ((SumW) * (NormNum) / (NormDen)) / (double) _TOTMCDECAYTUPLE;
                double _efficieny_rnd    = ((SumW_rnd) * (NormNum_rnd) / (NormDen_rnd)) / (double) _TOTMCDECAYTUPLE;            // to update with multiple candidate killer in MCDecayTuple... maybe or maybe not
                double _efficieny_bkgcat = ((SumW_bkgcat) * (NormNum_bkgcat / (NormDen_bkgcat))) / (double) _TOTMCDECAYTUPLE;   // to update with multiple candidate killer in MCDecayTuple... maybe or maybe not

                TString _OUTFILEname = bookkepingName(_effStepType, _ConH_BASE, _weightConfiguration, false, true);
                if (IOSvc::ExistFile(_OUTFILEname)) { MessageSvc::Error("Naming error (already done) for ", _OUTFILEname, "EXIT_FAILURE"); }
                TFile *                           fileN = IOSvc::OpenFile(_OUTFILEname, OpenMode::RECREATE);
                map< TString, EfficiencyContent > _Efficiencies;
                _Efficiencies["norm"]       = EfficiencyContent("norm", {_norm, 1.});
                _Efficiencies["sel"]        = EfficiencyContent("eff_sel", SumW * (NormNum / NormDen), _TOTMCDECAYTUPLE);
                _Efficiencies["sel_noNORM"] = EfficiencyContent("eff_sel_noNORM", (SumW),              _TOTMCDECAYTUPLE);
                _Efficiencies["SUMW"]       = EfficiencyContent("SUMW",  {SumW , 1.});
                _Efficiencies["NORMN"]    = EfficiencyContent("NORMN", {NormNum , 1.});
                _Efficiencies["NORMD"]    = EfficiencyContent("NORMD", {NormDen , 1.});


                _Efficiencies["norm_rnd"]       = EfficiencyContent("norm_rnd", {_norm_rnd, 1.});
                _Efficiencies["sel_rnd"]        = EfficiencyContent("eff_sel_rnd", SumW_rnd * (NormNum_rnd / NormDen_rnd), _TOTMCDECAYTUPLE);
                _Efficiencies["sel_noNORM_rnd"] = EfficiencyContent("eff_sel_noNORM_rnd", SumW_rnd, _TOTMCDECAYTUPLE);
                _Efficiencies["SUMW_rnd"]       = EfficiencyContent("SUMW_rnd",  {SumW_rnd , 1.});
                _Efficiencies["NORMN_rnd"]      = EfficiencyContent("NORMN_rnd", {NormNum_rnd , 1.});
                _Efficiencies["NORMD_rnd"]      = EfficiencyContent("NORMD_rnd", {NormDen_rnd , 1.});


                _Efficiencies["norm_bkgcat"]       = EfficiencyContent("norm_bkgcat", {_norm_bkgcat, 1.});
                _Efficiencies["sel_bkgcat"]        = EfficiencyContent("eff_sel_bkgcat", SumW_bkgcat * (NormNum_bkgcat / NormDen_bkgcat), _TOTMCDECAYTUPLE);
                _Efficiencies["sel_noNORM_bkgcat"] = EfficiencyContent("eff_sel_noNORM_bkgcat", SumW_bkgcat, _TOTMCDECAYTUPLE);
                _Efficiencies["nMCDecayTuple"]     = EfficiencyContent("nMCDecay",     make_pair(_TOTMCDECAYTUPLE, (double) sqrt(_TOTMCDECAYTUPLE)));
                _Efficiencies["nMCDecayTuple_gng"] = EfficiencyContent("nMCDecay_gng", make_pair(_TOTMCDECAYTUPLE_gng, (double) sqrt(_TOTMCDECAYTUPLE_gng)));
                _Efficiencies["SUMW_bkgcat"]       = EfficiencyContent("SUMW_bkgcat",  {SumW_bkgcat , 1.});
                _Efficiencies["NORMN_bkgcat"]      = EfficiencyContent("NORMN_bkgcat", {NormNum_bkgcat , 1.});
                _Efficiencies["NORMD_bkgcat"]      = EfficiencyContent("NORMD_bkgcat", {NormDen_bkgcat , 1.});

                fileN->cd();
                for (auto & eff : _Efficiencies) { 
                    eff.second.EfficiencyVar->Write(eff.second.EfficiencyVar->GetName(), TObject::kOverwrite); 
                }
                // renaming done for backward compatibility
                MessageSvc::Debug("Storing Expressions");
                toNamed("fullSelection",    w_exprs.at("fullSelection")).Write();
                toNamed("normSelection",    w_exprs.at("normSelection")).Write();
                toNamed("weight_full",      w_exprs.at("weightFull")).Write();
                toNamed("weight_normNum",   w_exprs.at("weightNormN")).Write();
                toNamed("weight_normDen",   w_exprs.at("weightNormD")).Write();
                toNamed("weight_MCDECAY",   w_exprs.at("weightMCDecay")).Write();
                toNamed("MCDecaySelection", w_exprs.at("MCDecaySelection")).Write();
                
                // Persist histograms flatness to disk
                // if (USEMCDECAYTUPLE) {
                auto & histoFlatness     = HISTOFLATNESS[_IDTRG][_IDWEI];
                auto & histoFlatness_rnd = HISTOFLATNESS_RNDKILL[_IDTRG][_IDWEI];
                for (auto & varID : histoFlatness) {
                    fileN->cd();
                    fileN->mkdir(varID.first);
                    fileN->cd(varID.first);
                    for (auto & hists : varID.second) { hists.second->Write(hists.first, TObject::kOverwrite); }
                }
                for (auto & varID : histoFlatness_rnd) {
                    fileN->cd();
                    fileN->cd(varID.first);
                    for (auto & hists : varID.second) { hists.second->Write(hists.first, TObject::kOverwrite); }
                }
                if( SettingDef::Efficiency::option.Contains("HistoQ2")){
                    //save the MCDT plots of q2 with/without corrections on the various steps!
                    auto & histoFlatnessMCDT     = HISTOFLATNESS_MCDecaySpecial[_IDTRG][_IDWEI];
                    for (auto & varID : histoFlatnessMCDT) {
                        fileN->cd();
                        fileN->cd(varID.first);
                        for (auto & hists : varID.second) { hists.second->Write(hists.first, TObject::kOverwrite); }
                    }
                }
                if( SettingDef::Efficiency::option.Contains("HistoBPT")){
                    //save the MCDT plots of BPT,BETA with/without corrections on the various steps!
                    auto & histoFlatnessMCDT_BPT     = HISTOFLATNESS_MCDecayBPT[_IDTRG][_IDWEI];
                    for (auto & varID : histoFlatnessMCDT_BPT) {
                        fileN->cd();
                        fileN->cd(varID.first);
                        for (auto & hists : varID.second) { hists.second->Write(hists.first, TObject::kOverwrite); }
                    }

                    auto & histoFlatnessMCDT_BETA     = HISTOFLATNESS_MCDecayBETA[_IDTRG][_IDWEI];
                    for (auto & varID : histoFlatnessMCDT_BETA) {
                        fileN->cd();
                        fileN->cd(varID.first);
                        for (auto & hists : varID.second) { hists.second->Write(hists.first, TObject::kOverwrite); }
                    }
                }
                fileN->Close();
                for (auto & eff : _Efficiencies) { delete eff.second.EfficiencyVar; }
            }   // end loop wconfig weightoption
        }       // end loop  trigger slice
    }           // end loop   Sample
    
    //======================================================
    // Compute BOOTSTRAPPED EFFICIENCIES (only for signal)
    //======================================================

    //The nodes of cuts and cutsMCDT to avoid several event loops triggered
    auto SLOTID = []( const TString & _wOpt, const Trigger & trg, const TriggerConf & trgConf, const TString & _weightConfig){
        TString _toret = _wOpt+"_"+to_string(trg)+"_"+to_string(trgConf) +"_"+ _weightConfig;
        return _toret;
    };
    if( SettingDef::Weight::useBS == true){
        // ROOT::DisableImplicitMT(); //MUST DISABLE IT, unfortunately branch RndPoisson is not always a vector column ! CRAP!
        MessageSvc::Info("Performing computation and dumping of efficiency ntuples for signal modes");
        //what we should do is to re-construct the names of the files dumped by this iteration
        for (auto && s : samples_toprocess) {
            // Make 1 TFile Efficiency_Sample, then make directories in the fileitself storing results, hadd them at the end
            vector< EffSlot > _SignalEffsSlots  = LoadEffScanOption(_yaml_ConfigFile, to_string(this_q2binSlice), to_string(this_analysis), s.first, true);
            auto _slots = _SignalEffsSlots;
            // Get the Shared Tuple
            TupleHolder _tupleShared = s.second.first;
            _tupleShared.PrintInline();
            _tupleShared.Init();
            auto _baseConfigHolder = _tupleShared.GetConfigHolder();
            TString _SAMPLE        = _baseConfigHolder.GetSample();

            //Sample must have MCDecayTuple to normalize against  
            bool _doIt           = s.second.first.GetConfigHolder().HasMCDecayTuple() ; 
            //All cross Feed samples are enabled for it
            bool _crossFeed      = s.second.first.GetConfigHolder().IsCrossFeedSample();
            //All signal MC samples are enabled for it ( this include Leakage ! )
            bool _signalMC       = s.second.first.GetConfigHolder().IsSignalMC();
            //Leakage tag 
            bool _leakage        = s.second.first.GetConfigHolder().IsLeakageSample();
            //Special KEtaPrime for low RK 
            bool _KetaPrimeGEE   = s.second.first.GetConfigHolder().GetSample() == "Bu2KEtaPrimeGEE";

            // bool _MakeBSTuple = (_KetaPrimeGEE ||  _leakage || _signalMC || _crossFeed) && _doIt; 
            // 2D constraints only for leakage 
            bool _MakeBSTuple = (  _leakage || _signalMC ) && _doIt; 
            if( !_MakeBSTuple ){           
                MessageSvc::Warning("No bootstrapping for this sample, not a sample flagged to go in BS mode!!!!");
                continue;
            }

            TString _tupleName           = SettingDef::Tuple::tupleName;
            SettingDef::Tuple::tupleName = "MCDT";   
            TupleHolder _tupleSharedMCDecay(_tupleShared.GetConfigHolder(), "", "MCT", _tupleShared.Option());
            _tupleSharedMCDecay.Init();
            SettingDef::Tuple::tupleName = _tupleName;

            using RNode = ROOT::RDF::RNode;
            using WeightOption = TString;
            using TriggerSlot_weightConfig  = tuple<Trigger,TriggerConf , TString>;


            // ==================================================================
            //Wraps the .root efficiency files expected to exist locally to coodinate and orchestrate the BS-weight vector column definition.
            // ==================================================================
            map< TString, map<TString, EfficiencyInfos> > infos; 
            map< pair<Trigger,TriggerConf>, vector< VariableBinning> > _Variables;
            for (auto && [_ConH_BASE, _CutH_BASE, _WeiH_BASE] : s.second.second) {
                pair<Trigger,TriggerConf> _ID(_ConH_BASE.GetTrigger(),_ConH_BASE.GetTriggerConf());
                _Variables[_ID] = GetVariableBinning(_ConH_BASE.GetProject(), _ConH_BASE.GetYear(), _ConH_BASE.GetTrigger(), _ConH_BASE.GetTriggerConf());
            }

            // ==================================================================
            // Decay Tuple handling 
            // ==================================================================
            // ROOT::EnableImplicitMT(4);        
            // Collect up the Variables to "Define"  columns for plotting BS-histos (sometimes aliases are requested)
            vector< pair< string, string > > _variables_forPlot = {};
            const vector< VariableBinning > _variables = GetVariableBinning(_tupleShared.GetConfigHolder().GetProject(), _tupleShared.GetConfigHolder().GetYear(), Trigger::L0L, TriggerConf::Exclusive);
            _variables_forPlot = GetVariablesForPlot(_variables);
            auto aliases = to_string_pairs(GetAllAlias(static_cast< TTree * >(_tupleShared.GetTuple())));
            
            ROOT::RDataFrame  dfDecay(*_tupleShared.GetTuple());
            RNode last_node      = dfDecay.Define("CRAPDEFINE","1>0");
            last_node = ApplyDefines(last_node, aliases, true);
            last_node = ApplyDefines(last_node, _variables_forPlot, true);   // int casting !, do-not force double   
            bool _doSmearingBS    = false;
            TString _wOptAppendBS = "-BS";
            for (auto & _effStepType : _slots) {
                if (_effStepType.wOpt().Contains("SMEAR")) _doSmearingBS = true;
                if (_effStepType.wOpt().Contains("PID") && !_wOptAppendBS.Contains("PID"))         _wOptAppendBS += "-PID";
                if (_effStepType.wOpt().Contains("TRK") && !_wOptAppendBS.Contains("TRK"))         _wOptAppendBS += "-TRK";
                if (_effStepType.wOpt().Contains("RW1D") && !_wOptAppendBS.Contains("RW1D"))       _wOptAppendBS += "-RW1D";
                if (_effStepType.wOpt().Contains("L0") && !_wOptAppendBS.Contains("L0"))           _wOptAppendBS += "-L0";
                if (_effStepType.wOpt().Contains("COMB") && !_wOptAppendBS.Contains("COMB"))       _wOptAppendBS += "-COMB";
                if (_effStepType.wOpt().Contains("DIST") && !_wOptAppendBS.Contains("DIST"))       _wOptAppendBS += "-DIST";
                if (_effStepType.wOpt().Contains("HLT-nTracks") && !_wOptAppendBS.Contains("HLT")) _wOptAppendBS += "-HLT-nTracks";
                if (_effStepType.wOpt().Contains("HLT") && !_wOptAppendBS.Contains("HLT"))         _wOptAppendBS += "-HLT";
                if (_effStepType.wOpt().Contains("MODEL") && !_wOptAppendBS.Contains("MODEL"))     _wOptAppendBS += "-MODEL";
            }
            if( _doSmearingBS && this_analysis == Analysis::EE){
                //TODO : COMMENT WHEN v11 append to tuple the correct Q2Smearing columns. (ask @Renato)            
                if( !last_node.HasColumn("JPs_M_smear_B0_fromMCDT_wMC") &&  _tupleShared.GetConfigHolder().PortingEnabled() ){
                    MessageSvc::Warning("Appending Q2 smearing columns");
                    last_node = HelperProcessing::AppendQ2SmearColumns( last_node, this_project, this_year); //UNCOMMENT WHEN v10 Q2Smearing is available                
                }else{
                    MessageSvc::Warning("Appending Q2 smearing columns Not possible, Fix tuples if you want it (and you do cut on it ) porting the JPs_TRUEM_MCDT");
                }
                if(!last_node.HasColumn("B_DTF_M_SMEAR_Bp_wMC") && !last_node.HasColumn("B_DTF_M_SMEAR_B0_wMC")  &&  _tupleShared.GetConfigHolder().PortingEnabled()){
                    MessageSvc::Warning("Appending B_DTF_M smearing columns");
                    last_node = HelperProcessing::AppendBSmearColumns( last_node, this_project, this_year); //UNCOMMENT WHEN v10 Q2Smearing is available
                }else{
                    MessageSvc::Warning("Appending B smearing columns Not possible, Fix tuples if you want it (and you do cut on it ) porting the B_TRUEM_MCDT (or PR one for background)");
                }
            }
            
            if( SettingDef::Efficiency::option.Contains("OnTheFly")){
                MessageSvc::Line();
                MessageSvc::Warning("Appending weights on the fly for DT", _wOptAppendBS);
                auto _cHBasic =  s.second.first.GetConfigHolder();
                last_node = HelperProcessing::AttachWeights(last_node, _cHBasic, _wOptAppendBS);
            }
            // ===================================================================
            // The Map of results of sumW< > with weight from bootstrapping 
            // KEY is the SLOTID for a given weightSlot
            // ===================================================================            
            map<  TString , ROOT::RDF::RResultPtr<RVec<double>> > _sumWBSWeights;         
            map<  TString , ROOT::RDF::RResultPtr<RVec<double>> > _sumWBSWeights_NORMN;            
            map<  TString , ROOT::RDF::RResultPtr<RVec<double>> > _sumWBSWeights_NORMD;                        
            map<  TString,  ROOT::RDF::RNode>  nodes_cuts; 
            // ===================================================================


            // ==================================================================
            // MCDecayTuple handling 
            // ==================================================================
            ROOT::RDataFrame dfMCDecay(*_tupleSharedMCDecay.GetTuple());
            RNode last_node_MCDT(dfMCDecay);
            if( SettingDef::Efficiency::option.Contains("OnTheFly")){
                MessageSvc::Line();
                MessageSvc::Warning("Appending BS weights on the fly for MCDT");
                auto _cHBasic =  s.second.first.GetConfigHolder();
                if( _wOptAppendBS.Contains("MODEL")){
                    //BS and MODEL
                    last_node_MCDT = HelperProcessing::AttachWeights(last_node_MCDT, _cHBasic, "MODEL-BS");
                }else{
                    //Only BS for RndPoisson2
                    last_node_MCDT = HelperProcessing::AttachWeights(last_node_MCDT, _cHBasic, "-BS");
                }
            }
            map<  TString , ROOT::RDF::RResultPtr<RVec<double>> > _sumWBSWeightsMCDecay;
            map<TString, ROOT::RDF::RNode>  nodes_cuts_MCDT;                        
            /*[SLOTID][VAR]["sumW"]--> Histo2D */
            map< TString, map< TString , map< TString , ROOT::RDF::RResultPtr<TH2D> > > > _BSFlatness1D;                         
            for (auto & _effStepType : _slots) {
                //loop weight option
                auto _weightOption = _effStepType.wOpt() ;                
                if(infos.count(_weightOption) != 0) MessageSvc::Error("BootStrapping eff-calculation, weightOption already booked",_weightOption,"EXIT_FAILURE");
                infos[_weightOption];
                for (auto && [_ConH_BASE, _CutH_BASE, _WeiH_BASE] : s.second.second) {
                    //trigger/triggerConf loop
                    auto _Trigger     = _ConH_BASE.GetTrigger();
                    auto _TriggerConf = _ConH_BASE.GetTriggerConf();
                    for (auto & _weightConfiguration : _effStepType.weightConfigs()) {
                        auto _slot =  SLOTID( _weightOption , _Trigger, _TriggerConf, _weightConfiguration);
                        if( infos[_weightOption].count( _slot) !=0 ){ 
                            MessageSvc::Error("BootStrapping eff-calculation, Trigger-TriggerConf-WeightConf already booked",_weightOption,"EXIT_FAILURE"); 
                        }
                        auto _KEY = _ConH_BASE.GetKey("addtrgconf-nobrem-notrack") + "_" +_weightConfiguration;
                        _KEY = _KEY.ReplaceAll("-","_");
                        TString _OUTFILEname = bookkepingName(_effStepType, _ConH_BASE, _weightConfiguration, false, true);
                        MessageSvc::Info("loading file ", _OUTFILEname) ;
                        auto info  = EfficiencyInfos( _OUTFILEname, _KEY);
                        cout<< RED << "~~~~~~~~~~~~~~~~~~~~~ nodes wSlot ~~~~~~~~~~~~~~~~~~~~~" << RESET<< endl;                    
                        info.Print();
                        infos[_weightOption][_slot] = info;
                        cout<< RED << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << RESET<< endl;                    
                        //=========== Deal with DecayTuple =============//
                        auto FullSelectionDT = info.fullSelection();
                        auto NormSelectionDT = info.normSelection();
                        if( nodes_cuts.find( FullSelectionDT ) == nodes_cuts.end()){
                            // NODESFILTERS.insert(std::pair< TString, ROOT::RDF::RNode >(_fullSelection, last_node.Define("fullSelection", _fullSelection.Data()).Filter("fullSelection")));
                            nodes_cuts.insert( std::pair<TString, ROOT::RDF::RNode>( FullSelectionDT , last_node.Define("fullSeleDT", FullSelectionDT.Data()).Filter("fullSeleDT")) );
                        }else{
                            MessageSvc::Warning("Reusing cached Node FullSelection for DT");
                        }
                        if( nodes_cuts.find( NormSelectionDT ) == nodes_cuts.end()){
                            nodes_cuts.insert( std::pair<TString, ROOT::RDF::RNode>( NormSelectionDT , last_node.Define("normSeleDT", NormSelectionDT.Data()).Filter("normSeleDT")) );
                        }else{
                            MessageSvc::Warning("Reusing cached Node CutSelection NORM for DT");
                        }
                        //Drop the TMath::Max for scalar in the weight formula, use the "Interpreter declared MAXV"
                        auto weightBS_FULL   = info.fullWeightBS().ReplaceAll("TMath::Max", "MAXV");
                        auto weightBS_NORMN  = info.normNumWeightBS().ReplaceAll("TMath::Max", "MAXV");
                        auto weightBS_NORMD  = info.normDenWeightBS().ReplaceAll("TMath::Max", "MAXV");
                        //---- Schedule up on MCDT the Booking of sumW for the vector column!  
                        /* 
                            DEPRECATED, use Reduce           
                            auto sumWBSFULL  = SumVecCol<RVec<double>, WeightDefRX::nBS>(_slot+ " BSFull"); 
                            auto sumWBSNORMN = SumVecCol<RVec<double>, WeightDefRX::nBS>(_slot+ " BSNormN"); 
                            auto sumWBSNORMD = SumVecCol<RVec<double>, WeightDefRX::nBS>(_slot+ " BSNormD"); 
                        */
                        //Define and Reduce With VecSum out of an initialized vector column of zeroes.
                        _sumWBSWeights[_slot]       = nodes_cuts.find(FullSelectionDT)->second.Define("weightBSFull" ,  weightBS_FULL.Data()).Reduce( VecSum, {"weightBSFull"},  RVec<double>( WeightDefRX::nBS,0));
                        //Define and Reduce With VecSum out of an initialized vector column of zeroes.
                        _sumWBSWeights_NORMN[_slot] = nodes_cuts.find(NormSelectionDT)->second.Define("weightBSNormN", weightBS_NORMN.Data()).Reduce( VecSum, {"weightBSNormN"}, RVec<double>( WeightDefRX::nBS,0));
                        //Define and Reduce With VecSum out of an initialized vector column of zeroes.
                        _sumWBSWeights_NORMD[_slot] = nodes_cuts.find(NormSelectionDT)->second.Define("weightBSNormD", weightBS_NORMD.Data()).Reduce( VecSum, {"weightBSNormD"}, RVec<double>( WeightDefRX::nBS,0));

                        //Let's deal with histograms for BS efficiencies 
                        pair<Trigger,TriggerConf> MyID(_ConH_BASE.GetTrigger(), _ConH_BASE.GetTriggerConf());
                        if( _Variables.at( MyID).size() != 0){
                            map< TString, map< TString, ROOT::RDF::TH1DModel > > HistoModels;
                            LoadTH1DModels(HistoModels, 
                                        _Variables.at(MyID), 
                                        weightBS_FULL, 
                                        weightBS_NORMN, 
                                        weightBS_NORMD, true);                                     
                            for( auto & var : _Variables.at( MyID)){
                                TString _branch = var.varID()+"_X";
                                BS1DHistoFiller<WeightDefRX::nBS>  sumW(HistoModels.at(var.varID()).at("sumW"));
                                BS1DHistoFiller<WeightDefRX::nBS> normN(HistoModels.at(var.varID()).at("normN"));
                                BS1DHistoFiller<WeightDefRX::nBS> normD(HistoModels.at(var.varID()).at("normD"));

                                _BSFlatness1D[_slot][var.varID()].insert(std::pair<TString, ROOT::RDF::RResultPtr<TH2D>>("sumW", nodes_cuts.find(FullSelectionDT)->second.Define("weightBSFull" , weightBS_FULL.Data())
                                                                                                                                                                         .Book<double, RVec<double>>( std::move(sumW), {_branch.Data(), "weightBSFull"} )));
                                _BSFlatness1D[_slot][var.varID()].insert(std::pair<TString, ROOT::RDF::RResultPtr<TH2D>>("normN", nodes_cuts.find(NormSelectionDT)->second.Define("weightBSNormN" , weightBS_NORMN.Data())
                                                                                                                                                                         .Book<double, RVec<double>>( std::move(normN), {_branch.Data(), "weightBSNormN"} )));
                                _BSFlatness1D[_slot][var.varID()].insert(std::pair<TString, ROOT::RDF::RResultPtr<TH2D>>("normD", nodes_cuts.find(NormSelectionDT)->second.Define("weightBSNormD" , weightBS_NORMD.Data())
                                                                                                                                                                         .Book<double, RVec<double>>( std::move(normD), {_branch.Data(), "weightBSNormD"} )));                                                                                                                                                                                                                                                                                                                                                  
                            }
                        }
                        //=========== Deal with MCDecayTuple =============//
                        auto SelectionMCDT   = info.MCDecaySelection();
                        auto weightBS_MCDT   = info.weightMCDecayBS();
                        if( nodes_cuts_MCDT.find( SelectionMCDT) == nodes_cuts_MCDT.end() ){
                            nodes_cuts_MCDT.insert( std::pair<TString, ROOT::RDF::RNode>(SelectionMCDT , last_node_MCDT.Define("seleMCDT", SelectionMCDT.Data()).Filter("seleMCDT")));
                        }else{
                            MessageSvc::Warning("Reusing cached Node CutSelection for MCDT");
                        }
                        //---- Schedule up on MCDT the Booking of sumW for the vector column ! 
                        _sumWBSWeightsMCDecay[_slot] = nodes_cuts_MCDT.find(SelectionMCDT)->second.Define("weightMCDTBS",  weightBS_MCDT.Data()).Reduce( VecSum, {"weightMCDTBS"}, RVec<double>( WeightDefRX::nBS,0));
                    }//end WeightConfig loop 
                }//end (Trigger-Cut) Configuration loop 
            }//end weight Option slot loop
            MessageSvc::Info("Computing and dumping ntuple with efficiencies from the Bootstrapping slices in MC");    
            MessageSvc::Info("1.) Event loop MCDT start");                                
            *dfMCDecay.Count();
            MessageSvc::Info("Event loop MCDT end");                                
            MessageSvc::Info("2.) Event loop DT start");                                
            *dfDecay.Count();   
            MessageSvc::Info("2.) Event loop DT done");                            
            if( _variables.size() != 0 ){
                MessageSvc::Info("2.0) Updating BS histos on files");                            
                for (auto & _effStepType : _slots) {
                    auto _weightOption = _effStepType.wOpt() ;
                    for (auto && [_ConH_BASE, _CutH_BASE, _WeiH_BASE] : s.second.second) {
                        auto _Trigger     = _ConH_BASE.GetTrigger();
                        auto _TriggerConf = _ConH_BASE.GetTriggerConf();
                        for (auto & _weightConfiguration : _effStepType.weightConfigs()) {
                            auto _KEY = _ConH_BASE.GetKey("addtrgconf-nobrem-notrack") + "_" +_weightConfiguration;
                            _KEY = _KEY.ReplaceAll("-","_");
                            auto _slot =  SLOTID( _weightOption , _Trigger, _TriggerConf, _weightConfiguration);
                            TString _OUTFILEname = bookkepingName(_effStepType, _ConH_BASE, _weightConfiguration, false, true);
                            //--- UPDATE EXISTING TFILE 
                            TFile fileN(_OUTFILEname, "UPDATE");
                            if(! IOSvc::ExistFile(_OUTFILEname)) MessageSvc::Error("Cannot acces this file!", _OUTFILEname, "EXIT_FAILURE");
                            MessageSvc::Warning("Updating File for BS histograms", _OUTFILEname);                                                                                
                            pair<Trigger,TriggerConf> MyID(_Trigger,_TriggerConf);    
                            auto sumW_MCDT = _sumWBSWeightsMCDecay.at(_slot).GetValue(); //->second);
                            for( auto & var : _Variables.at( MyID)){
                                MessageSvc::Debug("Dealing with", var.varID());        
                                MessageSvc::Debug("Unpacking BS Histos1D");
                                auto  HistSumW  = UnpackBSHistos1D( *(_BSFlatness1D.at(_slot).at(var.varID()).at("sumW"))); //->GetResultPtr()) );
                                MessageSvc::Debug("Unpacking BS Histos1D");
                                auto  HistNormN = UnpackBSHistos1D( *(_BSFlatness1D.at(_slot).at(var.varID()).at("normN")));
                                MessageSvc::Debug("Unpacking BS Histos1D");
                                auto  HistNormD = UnpackBSHistos1D( *(_BSFlatness1D.at(_slot).at(var.varID()).at("normD")));
                                MessageSvc::Debug("Looping and writing to files");
                                fileN.cd();
                                
                                if( fileN.GetDirectory(var.varID()) == 0){
                                    MessageSvc::Error("Cannot update directory", TString::Format("%s:%s", _OUTFILEname.Data(), var.varID().Data()), "EXIT_FAILURE");
                                }     
                                auto _VARDIRINFILE = fileN.GetDirectory(var.varID());                            
                                MessageSvc::Info("Writing SumW_BS histogram");
                                _VARDIRINFILE->cd();
                                _BSFlatness1D.at(_slot).at(var.varID()).at("sumW")->Write("sumW_BS"  , TObject::kOverwrite);
                                MessageSvc::Info("Writing SumW[i] histograms");
                                for( int i = 0; i < HistSumW.size(); ++i){                                    
                                    auto *bsDir = _VARDIRINFILE->mkdir(TString::Format("bs%i", i));
                                    bsDir->cd();
                                    // fileN.cd(TString::Format("bs%i", i));
                                    HistSumW.at(i).Write("sumW", TObject::kOverwrite);                                    
                                    HistNormN.at(i).Write("normN", TObject::kOverwrite);
                                    HistNormD.at(i).Write("normD", TObject::kOverwrite);
                                    //compute the sumW * Int(NormN)/Int(NormD) / sumW( MCDT[i]) directly
                                    auto Eff_Hist = BinnedEffs::EvalEfficiencyHisto(HistSumW.at(i), HistNormN.at(i), HistNormD.at(i), make_pair(sumW_MCDT[i], 0));                                  
                                    Eff_Hist.Write( "eff_sumW", TObject::kOverwrite );                                    
                                }
                            }
                            MessageSvc::Warning("Updating File for BS histograms, Close", _OUTFILEname);                                                                                
                            fileN.Close();
                            //--- CLOSE EXISTING TFILE 
                        }
                    }
                }
            }
            /*
                //We should go in "update" mode for the input files used in the computation 

                (Also add nominal histos back ? maybe not)
                WEIGHTSTEP_WeightConfig/VariableX/{2DHistoFlatness, {bs0/sumW, bs0/normN, bs0/normD, bs0/MCDT}}

            */            

            MessageSvc::Info("Generating ntuples EffTuple for each weight option in sequence");            
            auto snapopts = ROOT::RDF::RSnapshotOptions();
            snapopts.fMode = "UPDATE";
            int SNAPLOOP =0;                
            for (auto & _effStepType : _slots){   
                //recreate a tfile with eff tuple or update existing one, depends on loop iteration done. 
                snapopts.fMode =  SNAPLOOP ==0 ? "RECREATE" : "UPDATE";             
                SNAPLOOP++;            
                auto _weightOption = _effStepType.wOpt() ;
                MessageSvc::Info("making new df with 100 entries");
                ROOT::DisableImplicitMT(); //!!!!! IMPORTANT FOR CORRECT INDEXING.
                ROOT::RDataFrame dfEFFTUPLE(WeightDefRX::nBS);
                auto lastNode_epsTuple = dfEFFTUPLE.Define( "bsIDX", "rdfentry_");            
                map< TString,  RVec<double> > _results; 
                vector<VecDoubleAdder> _epsColumnsAdding;
                for (auto && [_ConH_BASE, _CutH_BASE, _WeiH_BASE] : s.second.second){
                    auto _Trigger     = _ConH_BASE.GetTrigger();
                    auto _TriggerConf = _ConH_BASE.GetTriggerConf();
                    auto _ID = std::make_pair<Trigger,TriggerConf>(_ConH_BASE.GetTrigger(),_ConH_BASE.GetTriggerConf());
                    for (auto & _weightConfiguration : _effStepType.weightConfigs()) {
                        auto _slot =  SLOTID( _weightOption, _Trigger, _TriggerConf, _weightConfiguration);
                        if( _sumWBSWeights.find(_slot) == _sumWBSWeights.end()){
                            MessageSvc::Error("Slot is invalid...","","EXIT_FAILURE");
                        }
                        MessageSvc::Info("Gathering RVec double results (full sele)");
                        if( _sumWBSWeights.find(_slot)->second == nullptr){
                            std::cout<<"NULL???"<<std::endl;
                            MessageSvc::Error("NULL SUM ? ", "","EXIT_FAILURE");
                        }
                        auto sumW_DT_FullSelection     =        _sumWBSWeights.at(_slot).GetValue() ;//->second);
                        MessageSvc::Info("Gathering RVec double results (normNum)");
                        auto sumW_DT_NormSelection_NUM =        _sumWBSWeights_NORMN.at(_slot).GetValue() ;//->second);
                        MessageSvc::Info("Gathering RVec double results (normDen)");
                        auto sumW_DT_NormSelection_DEN =        _sumWBSWeights_NORMD.at(_slot).GetValue() ;//->second);
                        MessageSvc::Info("Gathering RVec double results (mcDT)");
                        auto sumW_MCDT                 =        _sumWBSWeightsMCDecay.at(_slot).GetValue(); //->second);                        
                        //Check it 
                        if(sumW_DT_FullSelection.size()      != WeightDefRX::nBS || 
                            sumW_DT_NormSelection_NUM.size() != WeightDefRX::nBS ||
                            sumW_DT_NormSelection_DEN.size() != WeightDefRX::nBS ||
                                            sumW_MCDT.size() != WeightDefRX::nBS ){
                                                MessageSvc::Error("Invalid size weights", "","EXIT_FAILURE");
                        }
                        //RVec<double> operations are like numpy array operations!
                        if( true){
                            auto printIt = []( const RVec<double>& vec, TString _key){
                                MessageSvc::Debug("Content of",_key);
                                std::cout<<"SIZE : "<< vec.size()<<" , Content : "<< vec<< std::endl;
                            };
                            MessageSvc::Debug("Content of sumW_DT_FullSelection");
                            printIt(sumW_DT_FullSelection, "sumW_DT_FullSelection");
                            printIt(sumW_DT_NormSelection_NUM, "sumW_DT_NormSelection_NUM");
                            printIt(sumW_DT_NormSelection_DEN, "sumW_DT_NormSelection_DEN");
                            printIt(sumW_MCDT, "sumW_MCDT");
                        }
                        MessageSvc::Info("Products and divisions");
                        RVec<double> efficiencyResultNORM   = sumW_DT_FullSelection * (sumW_DT_NormSelection_NUM/sumW_DT_NormSelection_DEN) / sumW_MCDT;
                        RVec<double> efficiencyResultnoNORM = sumW_DT_FullSelection/ sumW_MCDT;
                        RVec<double> NORMFACTOR             = sumW_DT_NormSelection_NUM/sumW_DT_NormSelection_DEN;
                        //NEW Naming scheme to facilitate things later?
                        // auto _KEYBRANCHNAME = _ConH_BASE.GetKey("-addtrgconf-notrack")+"_"+_weightConfiguration; 
                        // _KEYBRANCHNAME = _KEYBRANCHNAME.ReplaceAll("-","_");
                        // _epsColumnsAdding.emplace_back(  efficiencyResultNORM ,   Form("eps_%s",         _KEYBRANCHNAME.Data()    ));
                        // _epsColumnsAdding.emplace_back(  efficiencyResultnoNORM , Form("eps_%s_noNORM",  _KEYBRANCHNAME.Data()    ));
                        // _epsColumnsAdding.emplace_back(  NORMFACTOR ,             Form("norm_%s",        _KEYBRANCHNAME.Data()    ));                        
                        TString _wConfig  = _weightConfiguration;
                        TString _trgKey = to_string(_Trigger)+"_"+to_string(_TriggerConf);
                        TString _branchName       = Form("eps_%s_w%s", _trgKey.Data()     , _wConfig.Data());
                        TString _branchNormName   = Form("norm_%s_w%s", _trgKey.Data()    , _wConfig.Data());
                        TString _branchEffNormName= Form("effnorm_%s_w%s", _trgKey.Data() , _wConfig.Data());

                        _epsColumnsAdding.emplace_back(  efficiencyResultNORM ,   _branchEffNormName.Data() );
                        _epsColumnsAdding.emplace_back(  efficiencyResultnoNORM , _branchName.Data() );
                        _epsColumnsAdding.emplace_back(  NORMFACTOR ,             _branchNormName.Data() );                        
                    }
                }
                auto treeName = _weightOption ; 
                treeName.ReplaceAll("-","_");                                 
                for( int i = 0 ; i < _epsColumnsAdding.size() ; ++i){
                    lastNode_epsTuple = lastNode_epsTuple.Define( _epsColumnsAdding[i].branchName(), _epsColumnsAdding[i], {"bsIDX"});
                }                
                TString _ofile = TString::Format("%s_EffTuple.root", _SAMPLE.Data());
                // lastNode_epsTuple.Snapshot( treeName.Data(), "EffTuple.root", "", snapopts );
                lastNode_epsTuple.Snapshot( treeName.Data(), _ofile, "", snapopts );
                MessageSvc::Info("Snapshotting eff(tuple) -> EffTuple.root for", treeName, "DONE");
            }//end loop for EffTuple making on this WeightOption
            _tupleShared.Close();
            _tupleSharedMCDecay.Close();
        }//end loop for Samples
    }//end if(useBS)
    auto tEnd = chrono::high_resolution_clock::now();
    MessageSvc::Line();
    MessageSvc::Warning((TString) SettingDef::IO::exe, "Took", to_string(chrono::duration_cast< chrono::seconds >(tEnd - tStart).count()), "seconds");
    MessageSvc::Line();
    return 0;
}

