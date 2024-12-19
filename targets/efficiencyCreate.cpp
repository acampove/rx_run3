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

using namespace iter;
using namespace std;

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
            if (i == 0)
                cout << " " << cnorm << "," << flush;
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

vector< EffSlot > LoadEffScanOption(TString yaml_ConfigFile, bool signal = true) {
    auto parserYaml = YAML::LoadFile(yaml_ConfigFile.Data());
    auto _node      = "scanStepsSig";
    if (!signal) { _node = "scanStepsBkg"; }
    YAML::Node        _nodeYaml = parserYaml["Efficiency"][_node];
    vector< EffSlot > _effSlots;
    if (_nodeYaml.size() == 0) { MessageSvc::Error("Cannot load slots Efficiencies for", TString(_node), "EXIT_FAILURE"); }
    for (auto const & _slot : _nodeYaml) { _effSlots.push_back(EffSlot(_slot["wOpt"].as< TString >(), _slot["cOpt"].as< TString >(), _slot["wOptMCDecay"].as< TString >(), _slot["wOptNormN"].as< TString >(), _slot["wOptNormD"].as< TString >(), _slot["cutSetNorm"].as< TString >(), _slot["wConfigs"].as< vector< TString > >())); }
    for (auto ee : _effSlots) { ee.Print(); }
    return _effSlots;
}

vector< pair< string, string > > to_string_pairs(const vector< pair< TString, TString > > & _pairs) {
    vector< pair< string, string > > _pairs_out;
    for (auto const & el : _pairs) { _pairs_out.push_back(make_pair(el.first.Data(), el.second.Data())); }
    return _pairs_out;
}

inline bool CheckExecutable(const Year & _year, const Prj & _project, const Analysis & _analysis, const Polarity & _polarity, const Q2Bin & _q2Bin) {
    // You can process Only 1 Q2 at the time, 1 year per time, 1 analysis per time...
    if (!CheckVectorContains(PROCESSABLE_YEAR, _year)) MessageSvc::Error((TString) SettingDef::IO::exe, (TString) "Invalid Year", "EXIT_FAILURE");
    if (!CheckVectorContains(PROCESSABLE_POL, _polarity)) MessageSvc::Error((TString) SettingDef::IO::exe, (TString) "Invalid Polarity", "EXIT_FAILURE");
    if (!CheckVectorContains(PROCESSABLE_ANA, _analysis)) MessageSvc::Error((TString) SettingDef::IO::exe, (TString) "Invalid Analysis", "EXIT_FAILURE");
    if (!CheckVectorContains(PROCESSABLE_PRJ, _project)) MessageSvc::Error((TString) SettingDef::IO::exe, (TString) "Invalid Project", "EXIT_FAILURE");
    if (!CheckVectorContains(PROCESSABLE_Q2, _q2Bin)) MessageSvc::Error((TString) SettingDef::IO::exe, (TString) "Invalid Q2Bin", "EXIT_FAILURE");
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
            _histo1D[var.varID()] = {{"sumW", (TH1D *) var.GetHistoClone(var.varID() + "_sumW", _effWeight + " | full")},
                                     {"normN", (TH1D *) var.GetHistoClone(var.varID() + "_normN", _normNumWeight + " | norm")},
                                     {"normD", (TH1D *) var.GetHistoClone(var.varID() + "_normD", _normDenWeight + " | norm")},

                                     {"sumW_rnd", (TH1D *) var.GetHistoClone(var.varID() + "_sumW_rnd", _effWeight + " | full")},
                                     {"normN_rnd", (TH1D *) var.GetHistoClone(var.varID() + "_normN_rnd", _normNumWeight + " | norm")},
                                     {"normD_rnd", (TH1D *) var.GetHistoClone(var.varID() + "_normD_rnd", _normDenWeight + " | norm")},

                                     {"sumW_bkgcat", (TH1D *) var.GetHistoClone(var.varID() + "_sumW_bkgcat", _effWeight + " | full")},
                                     {"normN_bkgcat", (TH1D *) var.GetHistoClone(var.varID() + "_normN_bkgcat", _normNumWeight + " | norm")},
                                     {"normD_bkgcat", (TH1D *) var.GetHistoClone(var.varID() + "_normD_bkgcat", _normDenWeight + " | norm")},

                                     {"sumW_raw", (TH1D *) var.GetRawHisto1D(var.varID() + "_sumW_raw", _effWeight + " | full", 100)},
                                     {"normN_raw", (TH1D *) var.GetRawHisto1D(var.varID() + "_normN_raw", _normNumWeight + " | norm", 100)},
                                     {"normD_raw", (TH1D *) var.GetRawHisto1D(var.varID() + "_normD_raw", _normDenWeight + " | norm", 100)},

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

vector< pair< string, string > > GetVariablesForPlot(const vector< VariableBinning > & _vars) {
    vector< pair< string, string > > _variables_forPlot;
    for (auto const & var : _vars) {
        _variables_forPlot.push_back(make_pair(var.Label_X().first.Data(), var.varX().Data()));
        if (!var.is1D()) { _variables_forPlot.push_back(make_pair(var.Label_Y().first.Data(), var.varY().Data())); }
    }
    return _variables_forPlot;
}

int main(int argc, char ** argv) {
    MessageSvc::Info("Executing efficiencyCreate");
    auto tStart = chrono::high_resolution_clock::now();
    // ROOT::EnableThreadSafety();
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
    if (_testMode) {
        SettingDef::trowLogicError         = true;
        vector< EffSlot > _SignalEffsSlots = LoadEffScanOption(_yaml_ConfigFile, true);
        vector< EffSlot > _bkgcatEffSlots  = LoadEffScanOption(_yaml_ConfigFile, false);
        // for (auto && [pol, this_analysis, year, this_q2binSlice] : iter::product(PROCESSABLE_POL, PROCESSABLE_ANA, PROCESSABLE_YEAR, PROCESSABLE_Q2)) {
        SettingDef::Config::year     = to_string(this_year);
        SettingDef::Config::polarity = to_string(this_polarity);
        MessageSvc::Info((TString) SettingDef::IO::exe, (TString) "TESTMODE::GetListOfSamples", (TString) "", "");
        auto samples_toprocess = ParserSvc::GetListOfSamples(_yaml_ConfigFile, this_q2binSlice, this_analysis);   // Year, Polarity, Projects picked from the DEFAULT in the List-parsing
        PrintAndTestMap(samples_toprocess);
        vector< TString > _outFiles = {};
        for (auto && s : samples_toprocess) {
            // Go through samples, get 1 tuple
            auto        _slots       = _SignalEffsSlots;
            TupleHolder _tupleShared = s.second.first;
            _tupleShared.PrintInline();
            _tupleShared.Init();
            auto _baseConfigHolder = _tupleShared.GetConfigHolder();
            bool USEMCDECAYTUPLE   = true;
            if (s.second.first.GetConfigHolder().IsSignalMC() && s.second.first.GetConfigHolder().HasMCDecayTuple()) {
                USEMCDECAYTUPLE = true;
                _slots          = _SignalEffsSlots;
            } else {
                _slots          = _bkgcatEffSlots;
                USEMCDECAYTUPLE = false;
            }
            for (auto && tp : s.second.second) {
                auto _ConH_BASE = get< 0 >(tp);
                auto _CutH_BASE = get< 1 >(tp);
                auto _WeiH_BASE = get< 2 >(tp);
                for (auto & _effStepType : _slots) {
                    auto _CutH_slice = _CutH_BASE.Clone(_effStepType.cOpt());
                    _CutH_slice.Init();
                    auto _fullSelection = TString(_CutH_slice.Cut()) + " != 0";
                    TCut _NormCut       = TCut("");
                    for (auto & _cutElement : _effStepType.ListAndCutsNorm()) _NormCut = _NormCut && _CutH_slice.Cuts().at(_cutElement);
                    auto _normSelection = TString(_NormCut) + "!= 0";
                    for (auto & _weightConfiguration : _effStepType.weightConfigs()) {
                        // Grab all Variables used to do flatness plots
                        TString _OUTFILEname = _effStepType.wOpt() + "_" + _ConH_BASE.GetSample() + "_Efficiency_" + _ConH_BASE.GetKey("addtrgconf");
                        if (_effStepType.wOpt() != "no") { _OUTFILEname = _OUTFILEname + "_" + _weightConfiguration; }
                        if (CheckVectorContains(_outFiles, _OUTFILEname)) { MessageSvc::Error("Trying to make twice same file (fix yaml file) ", _OUTFILEname, "EXIT_FAILURE"); }
                        _outFiles.push_back(_OUTFILEname);
                    }
                }
            }
        }
        // sort failed ones
        sort(SettingDef::Events::fails.begin(), SettingDef::Events::fails.end());
        MessageSvc::Warning((TString) SettingDef::IO::exe, (TString) "Failed size = ", to_string(SettingDef::Events::fails.size()));
        for (auto & failing : SettingDef::Events::fails) { cout << RED << "FAILED Tuple (not Existing and SKIPPING THEM in processing (maybe missing year, polarity, or sample ) ) : " << failing << endl; }
        cout << RED << "----- IF ALL IS GOOD, you can proceed with Submission " << RESET << endl;
        return 0;
    }
    SettingDef::trowLogicError          = true;
    auto              samples_toprocess = ParserSvc::GetListOfSamples(_yaml_ConfigFile, this_q2binSlice, this_analysis);
    vector< EffSlot > _SignalEffsSlots  = LoadEffScanOption(_yaml_ConfigFile, true);
    vector< EffSlot > _bkgcatEffSlots   = LoadEffScanOption(_yaml_ConfigFile, false);

    SettingDef::trowLogicError = false;
    PrintAndTestMap(samples_toprocess);
    for (auto && s : samples_toprocess) {
        // Make 1 TFile Efficiency_Sample, then make directories in the fileitself storing results, hadd them at the end
        auto _slots = _SignalEffsSlots;
        // Get the Shared Tuple
        TupleHolder _tupleShared = s.second.first;
        _tupleShared.PrintInline();
        _tupleShared.Init();
        auto _baseConfigHolder = _tupleShared.GetConfigHolder();
        bool USEMCDECAYTUPLE   = true;
        if (s.second.first.GetConfigHolder().IsSignalMC() && s.second.first.GetConfigHolder().HasMCDecayTuple()) {
            USEMCDECAYTUPLE = true;
            _slots          = _SignalEffsSlots;
        } else {
            _slots          = _bkgcatEffSlots;
            USEMCDECAYTUPLE = false;
        }
        for (auto && tp : s.second.second) {
            // Get base ConfigHolder of the list of Samples.
            auto _ConH_BASE = get< 0 >(tp);
            auto _CutH_BASE = get< 1 >(tp);
            auto _WeiH_BASE = get< 2 >(tp);
            // For this LOOP use a global extraCut definition (CutHolder::Clone) to transport fit-range extraCut in the Cloning of CutHolders
            if (IsCutInMap("cutEXTRA", _CutH_BASE.Cuts())) {
                SettingDef::Cut::extraCut = TString(_CutH_BASE.Cuts().at("cutEXTRA"));
                MessageSvc::Warning("Settting Global ExtraCut to ", SettingDef::Cut::extraCut);
            }
            // Go for each Trigger-TriggerConf (CutH , WeigthH , ConH)
            for (auto & _effStepType : _slots) {

                auto _CutH_slice = _CutH_BASE.Clone(_effStepType.cOpt());
                _CutH_slice.Init();
                auto _fullSelection = TString(_CutH_slice.Cut()) + " != 0";
                TCut _NormCut       = TCut("");
                for (auto & _cutElement : _effStepType.ListAndCutsNorm()) { _NormCut = _NormCut && _CutH_slice.Cuts().at(_cutElement); }
                auto _normSelection = TString(_NormCut) + "!= 0";
                for (auto & _weightConfiguration : _effStepType.weightConfigs()) {
                    // Grab all Variables used to do flatness plots
                    TString _OUTFILEname = _effStepType.wOpt() + "_" + _ConH_BASE.GetSample() + "_Efficiency_" + _ConH_BASE.GetKey("addtrgconf");
                    if (_effStepType.wOpt() != "no") { _OUTFILEname = _OUTFILEname + "_" + _weightConfiguration; }
                    if (IOSvc::ExistFile(_OUTFILEname + ".root")) { MessageSvc::Error("Naming error (already done) for ", _OUTFILEname, "EXIT_FAILURE"); }
                    const vector< VariableBinning > _vars = GetVariableBinning(_ConH_BASE.GetProject(), _ConH_BASE.GetYear(), _ConH_BASE.GetTrigger(), _ConH_BASE.GetTriggerConf());
                    MessageSvc::Warning("Variables for Flatness collected size ", to_string(_vars.size()));
                    // i need for each VariableBinning to "double ... Take< double >( "alias") ", and then fill( alias, weight) or fill( x, y , weight ) in the "TEMPLATED HIST" and "RAW HIST

                    // Build weight expressions ( total w , norm w Num, norm w Den )
                    auto _WeiH_NUMERATOR = _WeiH_BASE.Clone(_weightConfiguration, _effStepType.wOpt());
                    _WeiH_NUMERATOR.Init();
                    auto _WeiH_NORMN = _WeiH_BASE.Clone(_weightConfiguration, _effStepType.wOptNormN());
                    _WeiH_NORMN.Init();
                    auto _WeiH_NORMD = _WeiH_BASE.Clone(_weightConfiguration, _effStepType.wOptNormD());
                    _WeiH_NORMD.Init();
                    EventType::Check(_ConH_BASE, _CutH_slice, _WeiH_NUMERATOR);
                    EventType::Check(_ConH_BASE, _CutH_slice, _WeiH_NUMERATOR);
                    EventType::Check(_ConH_BASE, _CutH_slice, _WeiH_NORMN);
                    EventType::Check(_ConH_BASE, _CutH_slice, _WeiH_NORMD);
                    TString                                   _effWeight     = _WeiH_NUMERATOR.Weight();
                    TString                                   _normNumWeight = _WeiH_NORMN.Weight();
                    TString                                   _normDenWeight = _WeiH_NORMD.Weight();
                    map< TString, map< TString, TH1D * > >    _histo1D;
                    map< TString, map< TString, TH2Poly * > > _histo2D;
                    map< TString, map< TString, TH2D * > >    _histo2D_raw;
                    LoadTH1DFlatness(_histo1D, _vars, _effWeight, _normNumWeight, _normDenWeight);
                    LoadTH2DFlatness(_histo2D, _histo2D_raw, _vars, _effWeight, _normNumWeight, _normDenWeight);

                    double  _TOTMCDECAYTUPLE, _TOTMCDECAYTUPLE_RND;
                    TString _weightMCDecayTuple, _cutMCDecayTuple;
                    if (USEMCDECAYTUPLE) {
                        ROOT::EnableImplicitMT();
                        CounterWeights _MCDECAY_COUNTER = CounterHelpers::ProcessMCDecayTuple(_tupleShared.GetConfigHolder(), _effStepType.wOptMCDecay(), _weightConfiguration);
                        ROOT::DisableImplicitMT();
                        _MCDECAY_COUNTER.Report();
                        _TOTMCDECAYTUPLE     = std::floor(_MCDECAY_COUNTER.SumWeight);
                        _TOTMCDECAYTUPLE_RND = std::floor(_MCDECAY_COUNTER.SumWeight_RND);
                        _cutMCDecayTuple     = _MCDECAY_COUNTER.SelectionExpression;
                        _weightMCDecayTuple  = _MCDECAY_COUNTER.WeightExpression;
                    } else {
                        // For background mode, which implies no MCDecayTuple is available, we can only use as denominator the ngng processed events.
                        _TOTMCDECAYTUPLE     = GetMCTEntries(_baseConfigHolder.GetProject(), _baseConfigHolder.GetYear(), _baseConfigHolder.GetPolarity(), _baseConfigHolder.GetSample(), true, "ngng_evt");
                        _TOTMCDECAYTUPLE_RND = _TOTMCDECAYTUPLE;
                        _weightMCDecayTuple  = "noWeight_ngngEvents";
                        _cutMCDecayTuple     = "noCut_ngngEvents";
                    }
                    // Columns to add to DataFrame weightExpression as new column
                    vector< pair< string, string > > _weights_attach = {{"total_weight", _effWeight.Data()}, {"normNum_weight", _normNumWeight.Data()}, {"normDen_weight", _normDenWeight.Data()}};

                    // The main work is done here ! [ 1 Data Frame for each WeightConfiguration, maybe more optimal to do it outside, but fair enough...]
                    ROOT::EnableImplicitMT();

                    ROOT::RDataFrame df(*_tupleShared.GetTuple());
                    // collect all aliases defined in tuple
                    auto aliases = to_string_pairs(GetAllAlias(static_cast< TTree * >(_tupleShared.GetTuple())));
                    // collect all variables used for Plotting RDataFrame (will add a new column for each of them, with unique name)
                    auto _variables_forPlot = GetVariablesForPlot(_vars);
                    if (_variables_forPlot.size() == 0) { _variables_forPlot.push_back(make_pair("dummyFORPLOT", "1.")); }
                    // add extra variables used to do MultipLeCandidates counting
                    // eventNumber/l
                    vector< pair< string, string > > _extras = {{"runN", "(int)runNumber"}, {"evtN", "(long)eventNumber"}};

                    auto df_aliases    = ApplyDefines(df, aliases, true);
                    auto df_extras     = ApplyDefines(df_aliases, _extras, false);         // int casting !, do-not force doubble
                    auto df_weights    = ApplyDefines(df_extras, _weights_attach, true);   // force double casting
                    auto df_plots      = ApplyDefines(df_weights, _variables_forPlot, true);
                    auto makeUniqueIDs = [](int runN, long evtN) { return make_pair(runN, evtN); };
                    auto df_final      = df_plots.Define("uniqueID", makeUniqueIDs, {"runN", "evtN"}).Define("fullSelection", _fullSelection.Data()).Define("normSelection", _normSelection.Data());
                    // Count SumW, NormNum, NormDen
                    MessageSvc::Debug("fullSelection", _fullSelection);
                    MessageSvc::Debug("normSelection", _normSelection);
                    MessageSvc::Debug("fullweight", _effWeight);
                    MessageSvc::Debug("normNweight", _normNumWeight);
                    MessageSvc::Debug("normDweight", _normDenWeight);

                    auto df_FinalSelection = df_final.Filter("fullSelection");
                    auto df_Normalization  = df_final.Filter("normSelection");

                    auto _SumW    = df_FinalSelection.Sum< double >("total_weight");
                    auto _NormNum = df_Normalization.Sum< double >("normNum_weight");
                    auto _NormDen = df_Normalization.Sum< double >("normDen_weight");

                    auto _fullselectionPass = df_final.Take< bool >("fullSelection");
                    auto _normSelectionPass = df_final.Take< bool >("normSelection");
                    auto IDs_Eff            = df_final.Take< pair< int, long > >("uniqueID");
                    auto weightEff          = df_final.Take< double >("total_weight");
                    auto normWeighNum       = df_final.Take< double >("normNum_weight");
                    auto normWeighDen       = df_final.Take< double >("normDen_weight");
                    using TAKEDOUBLETYPE    = decltype(weightEff);
                    map< TString, TAKEDOUBLETYPE > _Vars_Content;
                    for (auto const & var : _vars) {
                        _Vars_Content[var.Label_X().first] = df_final.Take< double >(var.Label_X().first.Data());
                        if (!var.is1D()) { _Vars_Content[var.Label_Y().first] = df_final.Take< double >(var.Label_Y().first.Data()); }
                    }
                    // Extract the particles TRUEIDS and bkgcat flags
                    auto    _labels   = _ConH_BASE.GetParticleNames();
                    TString _head     = _labels.at("HEAD");
                    auto    B_TRUEID  = df_final.Take< int >((string) _head + "_TRUEID");
                    auto    B_bkgcat  = df_final.Take< int >((string) _head + "_BKGCAT");
                    auto    L1_TRUEID = df_final.Take< int >((string) _labels.at("L1") + "_TRUEID");   // L1 (MM/EE)
                    auto    L2_TRUEID = df_final.Take< int >((string) _labels.at("L2") + "_TRUEID");   // L2 (MM/EE)
                    auto    H1_TRUEID = df_final.Take< int >((string) _labels.at("H1") + "_TRUEID");   // K
                    auto    H2_TRUEID = df_final.Take< int >((string) _labels.at("H2") + "_TRUEID");   // Pi

                    MessageSvc::Info("initializing DF processing");
                    auto nDone = *df.Count();
                    if (nDone == 0) { MessageSvc::Error("NO ENTRIES PROCESSED, Some issues in bookkeped observables and your computation graph! ", "", "EXIT_FAILURE"); }
                    cout << RED " Processeed " << nDone << "  entries " << RESET << endl;
                    MessageSvc::Info("Done         DF processing");

                    const double SumW    = _SumW.GetValue();
                    const double NormNum = _NormNum.GetValue();
                    const double NormDen = _NormDen.GetValue();
                    ROOT::DisableImplicitMT();

                    // NORM SELECTION is "BEFORE" FULL SELECTION.... RND killing properly handled here with the various strategies
                    // NORM SELECTION is "BEFORE" FULL SELECTION
                    map< pair< int, long >, vector< int > > _indexes_fullSelection;
                    map< pair< int, long >, vector< int > > _indexes_normSelection;
                    // vector< int >                          _entriesToKeep; _entriesToKeep.reserve(nDone);
                    map< pair< int, long >, vector< tuple< int, int, int > > > _indexes_normSelection_nMatches;
                    map< pair< int, long >, vector< tuple< int, int, int > > > _indexes_fullSelection_nMatches;
                    MessageSvc::Debug("handling mult candidate loop");
                    auto _pdgL = _baseConfigHolder.GetAna() == Analysis::MM ? PDG::ID::M : PDG::ID::E;
                    if (EffDebug()) {
                        cout << "IDS_Eff    size " << IDs_Eff->size() << endl;
                        cout << "FullSelection Pass size " << _fullselectionPass->size() << endl;
                        cout << "NormSelection Pass size " << _normSelectionPass->size() << endl;
                        cout << "L1 trueID  size " << L1_TRUEID->size() << endl;
                        cout << "L2 trueID  size " << L2_TRUEID->size() << endl;
                        cout << "H1 trueID  size " << H1_TRUEID->size() << endl;
                        cout << "H2 trueID  size " << H2_TRUEID->size() << endl;
                        cout << "bkgcat     size " << B_bkgcat->size() << endl;
                    }
                    for (auto const && [i_uniqueID, i_fullSele, i_normSele, idx, l1_trueid, l2_trueid, h1_trueid, h2_trueid, b_bkgcat] : zip(IDs_Eff.GetValue(), _fullselectionPass.GetValue(), _normSelectionPass.GetValue(), range(_fullselectionPass.GetValue().size()), L1_TRUEID.GetValue(), L2_TRUEID.GetValue(), H1_TRUEID.GetValue(), H2_TRUEID.GetValue(), B_bkgcat.GetValue())) {
                        // deal with fullSelectection for RND killing & bkgcat killing
                        int nMatches = 0;
                        if (abs(l1_trueid) == abs(_pdgL)) nMatches++;
                        if (abs(l2_trueid) == abs(_pdgL)) nMatches++;
                        if (abs(h1_trueid) == abs(PDG::ID::K)) nMatches++;
                        if (abs(h2_trueid) == abs(PDG::ID::Pi) && _ConH_BASE.GetProject() == Prj::RKst) nMatches++;
                        if (abs(h2_trueid) == abs(PDG::ID::K) && _ConH_BASE.GetProject() == Prj::RPhi) nMatches++;
                        if (i_fullSele) {
                            auto _bkgcatEVT = make_tuple(b_bkgcat, idx, nMatches);
                            _indexes_fullSelection[i_uniqueID].push_back(idx);
                            _indexes_fullSelection_nMatches[i_uniqueID].push_back(_bkgcatEVT);
                        }
                        // deal with normSelectection for RND killing & bkgcat killing
                        if (i_normSele) {
                            auto _bkgcatEVT = make_tuple(b_bkgcat, idx, nMatches);
                            _indexes_normSelection[i_uniqueID].push_back(idx);
                            _indexes_normSelection_nMatches[i_uniqueID].push_back(_bkgcatEVT);
                        }
                        if (!i_normSele && i_fullSele) {
                            // The full selection is fully contained in normSelection!
                            MessageSvc::Error("Normalization Selection does not fully contain The full selection, LOGIC ERROR", "", "EXIT_FAILURE");
                        }
                    }
                    // Random killing , generate list to keep from overlap
                    map< pair< int, long >, int > _to_keep;
                    map< pair< int, long >, int > _to_keep_bestIDMatch;

                    TRandom3 rnd;
                    // PICK RANDOM single candidates (all entries in fullSelection must be in normSelection ! )
                    MessageSvc::Debug("pick random single candidate");
                    for (auto const & _id_idxs : _indexes_fullSelection) {
                        // do MultCand Killing on fully selected entries
                        if (_id_idxs.second.size() == 1) {
                            _to_keep[_id_idxs.first] = _id_idxs.second[0];   // stoe the index to keep (ONLY 1 ! )
                        } else {
                            _to_keep[_id_idxs.first] = _id_idxs.second[std::floor(rnd.Uniform(0, _id_idxs.second.size()))];
                        }
                    }
                    // Random killing , generate list to keep from overlap with normSelection
                    for (auto const & _idx_idxs_norm : _indexes_normSelection) {
                        // Pick single candidates in the normalization selection set. If multiple candidates has been choosen after :selection (seee before)
                        // Keep that, such that Normalization and Selection single candidates are aligned!
                        if (_to_keep.find(_idx_idxs_norm.first) != _to_keep.end()) {
                            continue;   // this event has been already selected to be considered
                        } else {
                            if (_idx_idxs_norm.second.size() == 1) {
                                _to_keep[_idx_idxs_norm.first] = _idx_idxs_norm.second[0];   // stoe the index to keep (ONLY 1 ! )
                            } else {
                                _to_keep[_idx_idxs_norm.first] = _idx_idxs_norm.second[std::floor(rnd.Uniform(0, _idx_idxs_norm.second.size()))];
                            }
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
                        // Pick single candidates in the normalization selection set. If multiple candidates has been choosen after :selection (seee before)
                        // Keep that, such that Normalization and Selection single candidates are aligned!
                        if (_to_keep_bestIDMatch.find(evt_ID) != _to_keep_bestIDMatch.end()) {
                            continue;   // this event has been already selected to be considered
                        } else {
                            if (list_bkgcatevent.size() == 1) {
                                _to_keep_bestIDMatch[evt_ID] = get< 1 >(list_bkgcatevent.front());   //.idx;
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
                                    _to_keep_bestIDMatch[evt_ID] = get< 1 >(to_sort.at(std::floor(rnd.Uniform(0, nEqualComparisons))));
                                } else {
                                    _to_keep_bestIDMatch[evt_ID] = get< 1 >(to_sort.front());
                                }
                            }
                        }
                    }

                    vector< int > _list_of_indexes_after_multiple_candidate_removal_rnd;
                    vector< int > _list_of_indexes_after_multiple_candidate_removal_bestbkgcat;

                    // Now we have the indexes of events to keep ...
                    double SumW_rnd    = 0.;
                    double NormNum_rnd = 0.;
                    double NormDen_rnd = 0.;
                    if (EffDebug()) MessageSvc::Debug("counting (tokeep)");
                    for (auto const & indexes : _to_keep) {
                        auto _entry = indexes.second;
                        if (_fullselectionPass->at(_entry)) { SumW_rnd += weightEff->at(_entry); }
                        if (_normSelectionPass->at(_entry)) {
                            NormNum_rnd += normWeighNum->at(_entry);
                            NormDen_rnd += normWeighDen->at(_entry);
                        }
                        _list_of_indexes_after_multiple_candidate_removal_rnd.push_back(_entry);
                    }
                    double SumW_bkgcat    = 0.;
                    double NormNum_bkgcat = 0.;
                    double NormDen_bkgcat = 0.;
                    for (auto const & indexes : _to_keep_bestIDMatch) {
                        auto _entry = indexes.second;
                        if (_fullselectionPass->at(_entry)) { SumW_bkgcat += weightEff->at(_entry); }
                        if (_normSelectionPass->at(_entry)) {
                            NormNum_bkgcat += normWeighNum->at(_entry);
                            NormDen_bkgcat += normWeighDen->at(_entry);
                        }
                        _list_of_indexes_after_multiple_candidate_removal_bestbkgcat.push_back(_entry);
                    }

                    MessageSvc::Debug("vars looping");
                    for (auto const & var : _vars) {
                        // For each Variablee... we have to Take<double>
                        MessageSvc::Info("Processing ", var.varID());
                        if (var.is1D()) {
                            FillHisto1D(*_histo1D.at(var.varID()).at("sumW"), *_Vars_Content.at(var.Label_X().first), *weightEff, *_fullselectionPass, {});
                            FillHisto1D(*_histo1D.at(var.varID()).at("sumW_raw"), *_Vars_Content.at(var.Label_X().first), *weightEff, *_fullselectionPass, {});
                            FillHisto1D(*_histo1D.at(var.varID()).at("sumW_rnd"), *_Vars_Content.at(var.Label_X().first), *weightEff, *_fullselectionPass, _list_of_indexes_after_multiple_candidate_removal_rnd);
                            FillHisto1D(*_histo1D.at(var.varID()).at("sumW_rnd_raw"), *_Vars_Content.at(var.Label_X().first), *weightEff, *_fullselectionPass, _list_of_indexes_after_multiple_candidate_removal_rnd);
                            FillHisto1D(*_histo1D.at(var.varID()).at("sumW_bkgcat"), *_Vars_Content.at(var.Label_X().first), *weightEff, *_fullselectionPass, _list_of_indexes_after_multiple_candidate_removal_bestbkgcat);
                            FillHisto1D(*_histo1D.at(var.varID()).at("sumW_bkgcat_raw"), *_Vars_Content.at(var.Label_X().first), *weightEff, *_fullselectionPass, _list_of_indexes_after_multiple_candidate_removal_bestbkgcat);

                            FillHisto1D(*_histo1D.at(var.varID()).at("normN"), *_Vars_Content.at(var.Label_X().first), *normWeighNum, *_normSelectionPass, {});
                            FillHisto1D(*_histo1D.at(var.varID()).at("normN_raw"), *_Vars_Content.at(var.Label_X().first), *normWeighNum, *_normSelectionPass, {});
                            FillHisto1D(*_histo1D.at(var.varID()).at("normN_rnd"), *_Vars_Content.at(var.Label_X().first), *normWeighNum, *_normSelectionPass, _list_of_indexes_after_multiple_candidate_removal_rnd);
                            FillHisto1D(*_histo1D.at(var.varID()).at("normN_rnd_raw"), *_Vars_Content.at(var.Label_X().first), *normWeighNum, *_normSelectionPass, _list_of_indexes_after_multiple_candidate_removal_rnd);
                            FillHisto1D(*_histo1D.at(var.varID()).at("normN_bkgcat"), *_Vars_Content.at(var.Label_X().first), *normWeighNum, *_normSelectionPass, _list_of_indexes_after_multiple_candidate_removal_bestbkgcat);
                            FillHisto1D(*_histo1D.at(var.varID()).at("normN_bkgcat_raw"), *_Vars_Content.at(var.Label_X().first), *normWeighNum, *_normSelectionPass, _list_of_indexes_after_multiple_candidate_removal_bestbkgcat);

                            FillHisto1D(*_histo1D.at(var.varID()).at("normD"), *_Vars_Content.at(var.Label_X().first), *normWeighDen, *_normSelectionPass, {});
                            FillHisto1D(*_histo1D.at(var.varID()).at("normD_raw"), *_Vars_Content.at(var.Label_X().first), *normWeighDen, *_normSelectionPass, {});
                            FillHisto1D(*_histo1D.at(var.varID()).at("normD_rnd"), *_Vars_Content.at(var.Label_X().first), *normWeighDen, *_normSelectionPass, _list_of_indexes_after_multiple_candidate_removal_rnd);
                            FillHisto1D(*_histo1D.at(var.varID()).at("normD_rnd_raw"), *_Vars_Content.at(var.Label_X().first), *normWeighDen, *_normSelectionPass, _list_of_indexes_after_multiple_candidate_removal_rnd);
                            FillHisto1D(*_histo1D.at(var.varID()).at("normD_bkgcat"), *_Vars_Content.at(var.Label_X().first), *normWeighDen, *_normSelectionPass, _list_of_indexes_after_multiple_candidate_removal_bestbkgcat);
                            FillHisto1D(*_histo1D.at(var.varID()).at("normD_bkgcat_raw"), *_Vars_Content.at(var.Label_X().first), *normWeighDen, *_normSelectionPass, _list_of_indexes_after_multiple_candidate_removal_bestbkgcat);
                        }
                        if (!var.is1D()) {
                            FillHisto2DPoly(*_histo2D.at(var.varID()).at("sumW"), *_Vars_Content.at(var.Label_X().first), *_Vars_Content.at(var.Label_Y().first), *weightEff, *_fullselectionPass, {});
                            FillHisto2D(*_histo2D_raw.at(var.varID()).at("sumW_raw"), *_Vars_Content.at(var.Label_X().first), *_Vars_Content.at(var.Label_Y().first), *weightEff, *_fullselectionPass, {});
                            FillHisto2DPoly(*_histo2D.at(var.varID()).at("sumW_rnd"), *_Vars_Content.at(var.Label_X().first), *_Vars_Content.at(var.Label_Y().first), *weightEff, *_fullselectionPass, _list_of_indexes_after_multiple_candidate_removal_rnd);
                            FillHisto2D(*_histo2D_raw.at(var.varID()).at("sumW_rnd_raw"), *_Vars_Content.at(var.Label_X().first), *_Vars_Content.at(var.Label_Y().first), *weightEff, *_fullselectionPass, _list_of_indexes_after_multiple_candidate_removal_rnd);
                            FillHisto2DPoly(*_histo2D.at(var.varID()).at("sumW_bkgcat"), *_Vars_Content.at(var.Label_X().first), *_Vars_Content.at(var.Label_Y().first), *weightEff, *_fullselectionPass, _list_of_indexes_after_multiple_candidate_removal_bestbkgcat);
                            FillHisto2D(*_histo2D_raw.at(var.varID()).at("sumW_bkgcat_raw"), *_Vars_Content.at(var.Label_X().first), *_Vars_Content.at(var.Label_Y().first), *weightEff, *_fullselectionPass, _list_of_indexes_after_multiple_candidate_removal_bestbkgcat);

                            FillHisto2DPoly(*_histo2D.at(var.varID()).at("normN"), *_Vars_Content.at(var.Label_X().first), *_Vars_Content.at(var.Label_Y().first), *normWeighNum, *_normSelectionPass, {});
                            FillHisto2D(*_histo2D_raw.at(var.varID()).at("normN_raw"), *_Vars_Content.at(var.Label_X().first), *_Vars_Content.at(var.Label_Y().first), *normWeighNum, *_normSelectionPass, {});
                            FillHisto2DPoly(*_histo2D.at(var.varID()).at("normN_rnd"), *_Vars_Content.at(var.Label_X().first), *_Vars_Content.at(var.Label_Y().first), *normWeighNum, *_normSelectionPass, _list_of_indexes_after_multiple_candidate_removal_rnd);
                            FillHisto2D(*_histo2D_raw.at(var.varID()).at("normN_rnd_raw"), *_Vars_Content.at(var.Label_X().first), *_Vars_Content.at(var.Label_Y().first), *normWeighNum, *_normSelectionPass, _list_of_indexes_after_multiple_candidate_removal_rnd);
                            FillHisto2DPoly(*_histo2D.at(var.varID()).at("normN_bkgcat"), *_Vars_Content.at(var.Label_X().first), *_Vars_Content.at(var.Label_Y().first), *normWeighNum, *_normSelectionPass, _list_of_indexes_after_multiple_candidate_removal_bestbkgcat);
                            FillHisto2D(*_histo2D_raw.at(var.varID()).at("normN_bkgcat_raw"), *_Vars_Content.at(var.Label_X().first), *_Vars_Content.at(var.Label_Y().first), *normWeighNum, *_normSelectionPass, _list_of_indexes_after_multiple_candidate_removal_bestbkgcat);

                            FillHisto2DPoly(*_histo2D.at(var.varID()).at("normD"), *_Vars_Content.at(var.Label_X().first), *_Vars_Content.at(var.Label_Y().first), *normWeighDen, *_normSelectionPass, {});
                            FillHisto2D(*_histo2D_raw.at(var.varID()).at("normD_raw"), *_Vars_Content.at(var.Label_X().first), *_Vars_Content.at(var.Label_Y().first), *normWeighDen, *_normSelectionPass, {});
                            FillHisto2DPoly(*_histo2D.at(var.varID()).at("normD_rnd"), *_Vars_Content.at(var.Label_X().first), *_Vars_Content.at(var.Label_Y().first), *normWeighDen, *_normSelectionPass, _list_of_indexes_after_multiple_candidate_removal_rnd);
                            FillHisto2D(*_histo2D_raw.at(var.varID()).at("normD_rnd_raw"), *_Vars_Content.at(var.Label_X().first), *_Vars_Content.at(var.Label_Y().first), *normWeighDen, *_normSelectionPass, _list_of_indexes_after_multiple_candidate_removal_rnd);
                            FillHisto2DPoly(*_histo2D.at(var.varID()).at("normD_bkgcat"), *_Vars_Content.at(var.Label_X().first), *_Vars_Content.at(var.Label_Y().first), *normWeighDen, *_normSelectionPass, _list_of_indexes_after_multiple_candidate_removal_bestbkgcat);
                            FillHisto2D(*_histo2D_raw.at(var.varID()).at("normD_bkgcat_raw"), *_Vars_Content.at(var.Label_X().first), *_Vars_Content.at(var.Label_Y().first), *normWeighDen, *_normSelectionPass, _list_of_indexes_after_multiple_candidate_removal_bestbkgcat);
                        }
                    }
                    map< TString, EfficiencyContent > _Efficiencies;
                    double                            _norm        = (double) NormNum / (double) NormDen;
                    double                            _norm_rnd    = (double) NormNum_rnd / (double) NormDen_rnd;
                    double                            _norm_bkgcat = (double) NormNum_bkgcat / (double) NormDen_bkgcat;

                    double _efficieny        = ((SumW) * (NormNum) / (NormDen)) / (double) _TOTMCDECAYTUPLE;
                    double _efficieny_rnd    = ((SumW_rnd) * (NormNum_rnd) / (NormDen_rnd)) / (double) _TOTMCDECAYTUPLE_RND;
                    double _efficieny_bkgcat = ((SumW_bkgcat) * (NormNum_bkgcat / (NormDen_bkgcat))) / (double) _TOTMCDECAYTUPLE_RND;

                    TFile * fileN                = IOSvc::OpenFile(_OUTFILEname + ".root", OpenMode::RECREATE);
                    _Efficiencies["norm"]        = EfficiencyContent("norm", {_norm, 1.});
                    _Efficiencies["norm_rnd"]    = EfficiencyContent("norm_rnd", {_norm_rnd, 1.});
                    _Efficiencies["norm_bkgcat"] = EfficiencyContent("norm_bkgcat", {_norm_bkgcat, 1.});

                    _Efficiencies["sel"]        = EfficiencyContent("eff_sel", SumW * (NormNum / NormDen), _TOTMCDECAYTUPLE);
                    _Efficiencies["sel_rnd"]    = EfficiencyContent("eff_sel_rnd", SumW_rnd * (NormNum_rnd / NormDen_rnd), _TOTMCDECAYTUPLE_RND);
                    _Efficiencies["sel_bkgcat"] = EfficiencyContent("eff_sel_bkgcat", SumW_bkgcat * (NormNum_bkgcat / NormDen_bkgcat), _TOTMCDECAYTUPLE_RND);

                    _Efficiencies["sel_noNORM"]        = EfficiencyContent("eff_sel_noNORM", (SumW), _TOTMCDECAYTUPLE);
                    _Efficiencies["sel_noNORM_rnd"]    = EfficiencyContent("eff_sel_noNORM_rnd", SumW_rnd, _TOTMCDECAYTUPLE_RND);
                    _Efficiencies["sel_noNORM_bkgcat"] = EfficiencyContent("eff_sel_noNORM_bkgcat", SumW_bkgcat, _TOTMCDECAYTUPLE_RND);

                    // Which errors on weightted MCDecayTuple?
                    _Efficiencies["nMCDecayTuple"]     = EfficiencyContent("nMCDecay", make_pair(_TOTMCDECAYTUPLE, (double) sqrt(_TOTMCDECAYTUPLE)));
                    _Efficiencies["nMCDecayTuple_rnd"] = EfficiencyContent("nMCDecay_rnd", make_pair(_TOTMCDECAYTUPLE_RND, (double) sqrt(_TOTMCDECAYTUPLE_RND)));
                    // TODO : remove, adapt code coming after to always load Filtering numbers / Generator Numbers ( some jobs fails in different q2 as missing generator yamls or filtering eff yamls )
                    // AddGeneratorEfficiencyToMap(_Efficiencies, _ConH);
                    // AddFilteringEfficiencyToMap(_Efficiencies, _ConH);
                    std::cout << " efficiency     = " << SumW << " *  ( " << NormNum << " / " << NormDen << " )  / " << _TOTMCDECAYTUPLE << " = " << _efficieny << std::endl;
                    std::cout << " efficiency(rnd)= " << SumW_rnd << " *  ( " << NormNum_rnd << " / " << NormDen_rnd << " )  / " << _TOTMCDECAYTUPLE_RND << " = " << _efficieny_rnd << std::endl;
                    std::cout << " efficiency(bkgcat)= " << SumW_bkgcat << " *  ( " << NormNum_bkgcat << " / " << NormDen_bkgcat << " )  / " << _TOTMCDECAYTUPLE_RND << " = " << _efficieny_bkgcat << std::endl;
                    fileN->cd();

                    for (auto & eff : _Efficiencies) { eff.second.EfficiencyVar->Write(eff.second.EfficiencyVar->GetName(), TObject::kOverwrite); }

                    TNamed fs("fullSelection", _fullSelection.Data());
                    fs.Write();
                    TNamed ns("normSelection", _normSelection.Data());
                    ns.Write();
                    TNamed w("weight_full", _effWeight.Data());
                    w.Write();
                    TNamed wn("weight_normNum", _normNumWeight.Data());
                    wn.Write();
                    TNamed wd("weight_normDen", _normDenWeight.Data());
                    wd.Write();
                    TNamed wMCDec("weight_MCDECAY", _weightMCDecayTuple.Data());
                    wMCDec.Write();
                    TNamed sMCDec("MCDecaySelection", _cutMCDecayTuple.Data());
                    sMCDec.Write();
                    TNamed normV("norm_v", fmt::format("{0} +- {1}", _Efficiencies["norm"].EfficiencyVar->getVal(), _Efficiencies["norm"].EfficiencyVar->getError()));
                    normV.Write();
                    TNamed normV_rnd("norm_rnd_v", fmt::format("{0} +- {1}", _Efficiencies["norm_rnd"].EfficiencyVar->getVal(), _Efficiencies["norm_rnd"].EfficiencyVar->getError()));
                    normV_rnd.Write();
                    TNamed normV_bkgcat("norm_bkgcat_v", fmt::format("{0} +- {1}", _Efficiencies["norm_bkgcat"].EfficiencyVar->getVal(), _Efficiencies["norm_bkgcat"].EfficiencyVar->getError()));
                    normV_bkgcat.Write();
                    fileN->cd();
                    // cout<<"Storing histos"<<endl;
                    for (auto const & var : _vars) {
                        fileN->mkdir(var.varID());
                        fileN->cd(var.varID());
                        if (var.is1D()) {
                            for (auto & hists : _histo1D.at(var.varID())) { hists.second->Write(hists.first, TObject::kOverwrite); }
                        } else {
                            for (auto & hists : _histo2D.at(var.varID())) { hists.second->Write(hists.first, TObject::kOverwrite); }
                            for (auto & hists : _histo2D_raw.at(var.varID())) { hists.second->Write(hists.first, TObject::kOverwrite); }
                        }
                    }   // end loop _vars
                    // cleaning of histo pointers
                    fileN->Close();
                    for (auto & eff : _Efficiencies) { delete eff.second.EfficiencyVar; }
                    _Efficiencies.clear();
                    for (auto & el : _histo1D) {
                        for (auto & hist : el.second) { delete hist.second; }
                    }
                    _histo1D.clear();
                    for (auto & el : _histo2D) {
                        for (auto & hist : el.second) { delete hist.second; }
                    }   // end loop _histo2D
                    _histo2D.clear();
                    for (auto & el : _histo2D_raw) {
                        for (auto & hist : el.second) { delete hist.second; }
                    }   // end loop _histo2D_raw
                    _histo2D_raw.clear();
                }
            }
        }   // end loop _WeightOption_CutOption_SLOTS
    }
    MessageSvc::Warning((TString) SettingDef::IO::exe, (TString) "Failed size = ", to_string(SettingDef::Events::fails.size()));
    for (auto & failing : SettingDef::Events::fails) { cout << RED << "FAILED Tuple (not Existing and SKIPPING THEM in processing (maybe missing year, polarity, or sample ) ) : " << failing << endl; }
    cout << RED << "----- IF ALL IS GOOD, you can proceed with Submission " << RESET << endl;
    // Creating executable token.done
    if (SettingDef::Events::fails.size() == 0) {
        std::ofstream ofs("Token.done", std::ofstream::out);
        ofs << " SUCCESS ";
        ofs.close();
    } else {
        TString       name = TString(fmt::format("Token_MISSAMPLE{0}.done", SettingDef::Events::fails.size()));
        std::ofstream ofs(name.Data(), std::ofstream::out);
        ofs << " SUCCESS ";
        ofs.close();
    }
    ROOT::DisableImplicitMT();
    return 0;
}
