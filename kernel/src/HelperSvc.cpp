#ifndef HELPERSVC_CPP
#define HELPERSVC_CPP

#define DEBUG_TREE 
#include "TLorentzVector.h"
#include "HelperSvc.hpp"
#include "HistogramSvc.hpp"
#include "MessageSvc.hpp"
#include "ParserSvc.hpp"
#include "ConfigHolder.hpp"
#include "ConstDef.hpp"
#include "CutDefRX.hpp"
#include "SettingDef.hpp"

#include "TupleReader.hpp"

#include "TCanvas.h"
#include "TChain.h"
#include "TH1.h"
#include "TMath.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TRandom3.h"
#include "TString.h"
#include "TTreeFormula.h"
#include "TTreeReader.h"
#include "TTreeReaderArray.h"
#include "TTreeReaderValue.h"
#include "TLorentzVector.h"
#include "vec_extends.h"
#include "zipfor.h"
#include <wordexp.h>

#include "RooAbsReal.h"
#include "RooFormulaVar.h"
#include "RooRealVar.h"
#include "HelperProcessing.hpp"
#include "RooDataSet.h"
#include "XJPsRescaler.hpp"

vector< TString > TokenizeString(const TString & _string, const TString & _separator) {
    vector< TString > _tokens = {};
    for (int i = 0; i < _string.Tokenize(_separator)->GetEntries(); ++i) { _tokens.push_back(((TObjString *) _string.Tokenize(_separator)->At(i))->String()); }
    return _tokens;
}

TString RemoveStringAfter(TString _string, const TString & _after) {
    if (_string.Index(_after) != -1) _string.Remove(_string.Index(_after), _string.Length());
    return _string;
}

TString RemoveStringBefore(TString _string, const TString & _before) {
    if (_string.Index(_before) != -1) _string.Remove(0, _string.Index(_before));
    return _string;
}

TString RemoveStringBetween(TString _string, const TString & _startString, const TString & _endString) {
    TString _returnString     = _string;
    auto    _removeStartIndex = _string.Index(_startString);
    if (_removeStartIndex != -1) {
        auto _removeEndIndex = _string.Index(_endString, _removeStartIndex + _startString.Length()) + _endString.Length();
        if (_removeEndIndex != -1) { _returnString = _string(0, _removeStartIndex) + _string(_removeEndIndex, _string.Length()); }
    }
    return _returnString;
}

TString StripStringBetween(const TString & _string, const TString & _startString, const TString & _endString) {
    TString _returnString    = "";
    auto    _stripStartIndex = _string.Index(_startString) + _startString.Length();
    if ((_stripStartIndex - _startString.Length()) != -1) {   // If a match was found
        auto _stripEndIndex = _string.Index(_endString, _stripStartIndex);
        if (_stripEndIndex != -1) { _returnString = _string(_stripStartIndex, _stripEndIndex - _stripStartIndex); }
    }
    return _returnString;
}

TString ExpandEnvironment(const TString & _string) {
    wordexp_t p;
    char **w;
    wordexp(_string.Data(), &p, 0);
    w = p.we_wordv;
    TString _expanded(*w);
    wordfree(&p);
    return _expanded;
}

void ConfigureFromYAML(TString _fileYAML) {
    ParserSvc parser("");
    parser.Init(_fileYAML);
    return;
}

void EnableMultiThreads(int _nThreads) {
    if (!ROOT::IsImplicitMTEnabled() && SettingDef::useMultiThread) {
        MessageSvc::Warning("EnableMultiThreads", _nThreads == 0 ? to_string(ROOT::GetThreadPoolSize()) : to_string(_nThreads));
        ROOT::EnableThreadSafety();
        if (_nThreads == 0)
            ROOT::EnableImplicitMT();
        else
            ROOT::EnableImplicitMT(_nThreads);
    }
    return;
}

void DisableMultiThreads() {
    if (ROOT::IsImplicitMTEnabled()) {
        MessageSvc::Warning("DisableMultiThreads");
        ROOT::DisableImplicitMT();
    }
    return;
}

bool ContainString(vector< TString > _vector, TString _string) {
    for (const auto & _el : _vector) {
        if (_el.Contains(_string)) return true;
    }
    return false;
}

TString CleanString(TString _string) {
    while (_string.Contains(SettingDef::separator + SettingDef::separator)) { _string.ReplaceAll(SettingDef::separator + SettingDef::separator, SettingDef::separator); }
    if (_string.EndsWith(SettingDef::separator)) _string.Remove(_string.Last(*SettingDef::separator.Data()));
    return _string;
}

TString CleanAnalysis(TString _string) {
    for (const auto & _str : SettingDef::AllowedConf::Analyses) _string.ReplaceAll(_str, "");
    return CleanString(_string);
}

TString CleanQ2Bin(TString _string) {
    for (const auto & _str : SettingDef::AllowedConf::Q2Bins) _string.ReplaceAll(_str, "");
    return CleanString(_string);
}

TString CleanYear(TString _string) {
    for (const auto & _str : SettingDef::AllowedConf::Years) _string.ReplaceAll(_str, "");
    return CleanString(_string);
}

TString CleanTrigger(TString _string) {
    for (const auto & _str : SettingDef::AllowedConf::L0Categories) _string.ReplaceAll(_str, "");
    return CleanString(_string);
}

TString CleanBrem(TString _string) {
    for (const auto & _str : SettingDef::AllowedConf::BremCategories) _string.ReplaceAll(_str, "");
    return CleanString(_string);
}

TCut ReplaceCut(TCut _cut, TString _string1, TString _string2) {
    TString sCut(_cut);
    sCut.ReplaceAll(_string1, _string2);
    _cut = TCut(sCut);
    return _cut;
}
TCut CustomizedCut(TString _cutOption, TString _separator, TString _cutVariable ){
    if( TokenizeString(StripStringBetween( _cutOption, _separator,"]"), ",").size()!=2){
        MessageSvc::Error(TString::Format("Cannot tokenize cutoption (%s) with separator (%s)", _cutOption.Data(), _separator.Data()), "","EXIT_FAILURE");
    }
    double MIN = TokenizeString( StripStringBetween( _cutOption, _separator,"]"), ",")[0].Atof();
    double MAX = TokenizeString( StripStringBetween( _cutOption, _separator,"]"), ",")[1].Atof();    
    TCut _myCut = TCut(TString::Format("%s>%.4f && %s<%.4f", _cutVariable.Data(), MIN, _cutVariable.Data(), MAX));
    return _myCut;
}

TCut ReplaceProject(TCut _cut, Prj _project) {
    vector< pair< TString, TString > > _heads = GetParticleBranchNames(_project, Analysis::All, Q2Bin::All, "onlyhead");
    vector< pair< TString, TString > > _hadrons = GetParticleBranchNames(_project, Analysis::All, Q2Bin::All, "onlyhadrons");
    //TODO : add L1, L2 replacer {L1} , {L2} wildcards...(need Ana in or adapt the ReplaceWildcards call)
    TString                            _head  = _heads[0].first;
    TString                            _h1    = _hadrons[0].first;
    TString                            _h2    = _project != Prj::RK ? _hadrons[1].first : _hadrons[0].first;

    TString                            _headMass;
    switch (_project) {
        case Prj::RKst: _headMass = Form("%f", PDG::Mass::Bd); break;
        case Prj::RK: _headMass = Form("%f", PDG::Mass::Bu); break;
        case Prj::RPhi: _headMass = Form("%f", PDG::Mass::Bs); break;
        case Prj::RL: _headMass = Form("%f", PDG::Mass::Lb); break;
        case Prj::RKS: _headMass = Form("%f", PDG::Mass::Bd); break;
        default: MessageSvc::Error("ReplaceProject", (TString) "Invalid project", to_string(_project), "EXIT_FAILURE"); break;
    }
    _cut = ReplaceCut(_cut, "{HEAD}_", _head + "_");
    _cut = ReplaceCut(_cut, "{H1}_", _h1 + "_");
    _cut = ReplaceCut(_cut, "{H2}_", _h2 + "_");
    _cut = ReplaceCut(_cut, "{HEADMASS}", _headMass);
    _cut = ReplaceCut(_cut, "{H1}_", _h1 + "_");
    _cut = ReplaceCut(_cut, "{H2}_", _h2 + "_");
    return _cut;
}

TString ReplaceWildcards(TString _string, map< TString, TString > _names) {
    if (_string.Contains("{HEAD}")) _string.ReplaceAll("{HEAD}", _names["HEAD"]);
    if (_string.Contains("{HH}")) _string.ReplaceAll("{HH}", _names["HH"]);
    if (_string.Contains("{H1}")) _string.ReplaceAll("{H1}", _names["H1"]);
    if (_string.Contains("{H2}")) _string.ReplaceAll("{H2}", _names["H2"]);
    if (_string.Contains("{LL}")) _string.ReplaceAll("{LL}", _names["LL"]);
    if (_string.Contains("{L1}")) _string.ReplaceAll("{L1}", _names["L1"]);
    if (_string.Contains("{L2}")) _string.ReplaceAll("{L2}", _names["L2"]);
    return _string;
}

TCut CleanCut(TCut _cut) {
    _cut = ReplaceCut(_cut, "(!((1)||1))", TString(NOCUT));
    _cut = ReplaceCut(_cut, "((1)||(1))||(1)", TString(NOCUT));
    _cut = ReplaceCut(_cut, " && 1)", ")");
    _cut = ReplaceCut(_cut, "(1)&&");
    _cut = ReplaceCut(_cut, "((1))&&");
    _cut = ReplaceCut(_cut, "&&(1)");
    _cut = ReplaceCut(_cut, "&&((1))");
    _cut = ReplaceCut(_cut, "&&(((1)))");
    _cut = ReplaceCut(_cut, "1&&");
    _cut = ReplaceCut(_cut, "((!(1)))&&");
    _cut = ReplaceCut(_cut, "((!((1)||((!(1))))))&&");
    _cut = ReplaceCut(_cut, "&&(!((1)||((!(1)))))");
    return _cut;
}

TString CleanWeight(TString _weight) {
    _weight.ReplaceAll("(1.)", TString(NOWEIGHT));
    _weight.ReplaceAll("(1.)", TString(NOWEIGHT));
    _weight.ReplaceAll("(1. * ", "(");
    _weight.ReplaceAll("(1.  * ", "(");
    _weight.ReplaceAll(" * 1.", "");
    _weight.ReplaceAll("1. * ", "");

    _weight.ReplaceAll("(1)", TString(NOWEIGHT));
    _weight.ReplaceAll("(1)", TString(NOWEIGHT));
    _weight.ReplaceAll("(1 * ", "(");
    _weight.ReplaceAll("(1  * ", "(");
    _weight.ReplaceAll(" * 1", "");
    _weight.ReplaceAll("1 * ", "");
    return _weight;
}

TCut UpdateDTFCut(TCut _cut) {
    if (SettingDef::Tuple::option == "gng") {
        _cut = ReplaceCut(_cut, "_DTF_", "_DTF_PV_");
        _cut = ReplaceCut(_cut, "_DTF_PV_M", "_DTF_PV_M[0]");
        _cut = ReplaceCut(_cut, "_DTF_PV_JPs_M", "_DTF_PV_JPs_M[0]");
        _cut = ReplaceCut(_cut, "_DTF_PV_Psi_M", "_DTF_PV_Psi_M[0]");
    }
    return _cut;
}

Year GetRunFromYear(const Year & _year) {   
    switch (_year) {   // TO BE CHECKED
        case Year::Y2011: return Year::Run1; break;
        case Year::Y2012: return Year::Run1; break;
        case Year::Run1: return Year::Run1; break;
        case Year::Y2015: return Year::Run2p1; break;
        case Year::Y2016: return Year::Run2p1; break;
        case Year::Run2p1: return Year::Run2p1; break;
        case Year::Y2017: return Year::Run2p2; break;
        case Year::Y2018: return Year::Run2p2; break;
        case Year::Run2p2: return Year::Run2p2; break;
        case Year::All: return Year::All; break;
        // case Year::All: MessageSvc::Warning("GetRunFromYear", (TString) "Invalid year", year); break;
        default: MessageSvc::Error("GetRunFromYear", (TString) "Invalid year", to_string(_year), "EXIT_FAILURE"); break;
    }
    return  Year::Error;
}

TCut JoinCut( const vector<TString> & list_of_cuts , TString _logicalOperator) {

    if (!( _logicalOperator == "&&" || _logicalOperator == "||")) {
        MessageSvc::Error("HelperSvc::JoinCut", "Please specify '&&' or '||' as separator for join cuts", "EXIT_FAILURE");
    }
    if ( list_of_cuts.size() == 0) return TCut(NOCUT);
    TCut _retCut = TCut(list_of_cuts[0]);
    for ( int i = 1; i< list_of_cuts.size(); ++i) {
        if ( _logicalOperator == "||"){
            _retCut = _retCut || TCut( list_of_cuts[i]);
        }else if ( _logicalOperator == "&&") {
            _retCut = _retCut && TCut( list_of_cuts[i]);
        }
    }
    return _retCut;
};

TCut GetHLT1TCKCut( Year _year) {
    vector<int> _tcks = CutDefRX::Quality::HLT1TCK.at(_year).second;
    vector<TString> _tckCuts;
    for (const auto & _tck : _tcks)
        _tckCuts.push_back(Form("HLT1TCK == %i", _tck));
    TCut _cut = JoinCut(_tckCuts, "||");

    return _cut;
};

TCut UpdateHLT1Cut(TCut _cut, Prj _project, Analysis _analysis, Year _year, TString _option) {
    if (SettingDef::Tuple::option != "gng") {
        if (_option.Contains("HLT1AllTrackL0AlignedTOS") && ((TString)_cut).Contains("Hlt1TrackAllL0Decision_TOS") && _year == Year::Y2012 && (_project== Prj::RK || _project == Prj::RKst) ) {
            TString _check = TString(_cut);
            if( ! _check.Contains( "Hlt1TrackAllL0Decision_TOS_update") ) {
                _cut = ReplaceCut(_cut, "Hlt1TrackAllL0Decision_TOS", "Hlt1TrackAllL0Decision_TOS_update");        
            }
        }
        //HLT1 alignment for Run2 MC....
        if (((TString) _cut).Contains("Hlt1TrackMVADecision_TOS") && (_year == Year::Y2016) && !_option.Contains("noTCKCat")) {
            //////////////////////////////////////////////
            // 2016
            //
            // TCKCat 0, b = 1.1 (default on MC)
            // TCKCat 1, b = 1.6
            // TCKCat 2, b = 2.3
            //
            // MD : 89.8% (1.1) :  0.0% (1.6) : 10.2% (2.3)
            // MU : 30.0% (1.1) : 15.5% (1.6) : 54.5% (2.3)
            //////////////////////////////////////////////

            TCut _cutOLD = CutDefRX::Trigger::Run2p1::Hlt1TrkMVA;
            TCut _cut2D  = CutDefRX::Trigger::Run2p1::Y2016::Hlt1TrkMVA_Track;
            TCut _cutNEW = "1";

            vector< pair< TString, TString > > _hadrons = GetParticleBranchNames(_project, _analysis, Q2Bin::All, "onlyhadrons");
            TString                            _h1      = _hadrons[0].first;
            TString                            _h2      = _hadrons[0].first;
            if (_project != Prj::RK) _h2 = _hadrons[1].first;

            if (_project == Prj::RK) {
                _cutNEW = ReplaceCut(_cut2D, "{PARTICLE}", _h1);
            } else {
                _cutNEW = ReplaceCut(_cut2D, "{PARTICLE}", _h1) || ReplaceCut(_cut2D, "{PARTICLE}", _h2);
            }

            vector< pair< TString, TString > > _leptons = GetParticleBranchNames(_project, _analysis, Q2Bin::All, "onlyleptons");
            TString                            _l1      = _leptons[0].first;
            TString                            _l2      = _leptons[1].first;
            switch (_analysis) {
                case Analysis::MM: _cutNEW = _cutNEW || ReplaceCut(_cut2D, "{PARTICLE}", _l1) || ReplaceCut(_cut2D, "{PARTICLE}", _l2); break;
                case Analysis::EE: _cutNEW = _cutNEW || ReplaceCut(ReplaceCut(_cut2D, "{PARTICLE}_P", _l1 + "_TRACK_P"), "{PARTICLE}", _l1) || ReplaceCut(ReplaceCut(_cut2D, "{PARTICLE}_P", _l2 + "_TRACK_P"), "{PARTICLE}", _l2); break;
                default: MessageSvc::Error("UpdateHLT1Cut", (TString) "Invalid analysis", to_string(_analysis), "EXIT_FAILURE"); break;
            }
            TCut _cutTCKcat0 = "TCKCat == 0" && ReplaceCut(_cutNEW, "{b}", "1.1");
            TCut _cutTCKcat1 = "TCKCat == 1" && ReplaceCut(_cutNEW, "{b}", "1.6");
            TCut _cutTCKcat2 = "TCKCat == 2" && ReplaceCut(_cutNEW, "{b}", "2.3");
            _cutNEW          = _cutTCKcat0 || _cutTCKcat1 || _cutTCKcat2;

            if (_cutOLD != "1") _cut = ReplaceCut(_cut, TString(_cutOLD), TString(_cutNEW));
        }
    }
    return _cut;
}

TCut UpdatePIDTune(TCut _cut, TString _year) {
    _cut = ReplaceCut(_cut, "K_{TUNE}", "K_" + GetPIDTune(_year, "H"));
    _cut = ReplaceCut(_cut, "Pi_{TUNE}", "Pi_" + GetPIDTune(_year, "H"));
    _cut = ReplaceCut(_cut, "K1_{TUNE}", "K1_" + GetPIDTune(_year, "H"));
    _cut = ReplaceCut(_cut, "K2_{TUNE}", "K2_" + GetPIDTune(_year, "H"));
    _cut = ReplaceCut(_cut, "M1_{TUNE}", "M1_" + GetPIDTune(_year, "L"));
    _cut = ReplaceCut(_cut, "M2_{TUNE}", "M2_" + GetPIDTune(_year, "L"));
    _cut = ReplaceCut(_cut, "E1_{TUNE}", "E1_" + GetPIDTune(_year, "L"));
    _cut = ReplaceCut(_cut, "E2_{TUNE}", "E2_" + GetPIDTune(_year, "L"));
    return _cut;
}

TCut UpdatePIDMeerkat(TCut _cut, TString _year) {

    vector< TString  > _pid_vars = { "Pi_{TUNE}_ProbNNpi", "Pi_{TUNE}_ProbNNk", "Pi_{TUNE}_ProbNNp ", "Pi_{TUNE}_ProbNNp)",
                                 "K_PIDK", "K_{TUNE}_ProbNNpi", "K_{TUNE}_ProbNNk", "K_{TUNE}_ProbNNp ", "K_{TUNE}_ProbNNp)",
                                 "K1_PIDK", "K1_{TUNE}_ProbNNpi", "K1_{TUNE}_ProbNNk", "K1_{TUNE}_ProbNNp ", "K1_{TUNE}_ProbNNp)",
                                 "K2_PIDK", "K2_{TUNE}_ProbNNpi", "K2_{TUNE}_ProbNNk", "K2_{TUNE}_ProbNNp ", "K2_{TUNE}_ProbNNp)",
                                 "E1_PIDe", "E1_{TUNE}_ProbNNe", "E2_PIDe", "E2_{TUNE}_ProbNNe",
                                 "M1_{TUNE}_ProbNNmu","M2_{TUNE}_ProbNNmu", 
    };
    for ( const auto &_pid_var : _pid_vars ) {
        TString _pid_var_updated = (TString) UpdatePIDTune(TCut(_pid_var), _year).GetTitle();
        /* Because of a problem with PIDCorr samples for Run 2, the corrected ProbNN branches for Pions
            agree worse to sWeighted data than uncorrected branches. This is why we decided
            to use the uncorrected branches in this case for the time being. So only run 1 uses Meerkat corrected
            branches for pions.
        */
        if ( hash_year(GetRunFromYear(_year)) != Year::Run1 && _pid_var_updated.Contains("Pi_") ) continue;

        TString replace = "";
        if ( _pid_var.Contains("_ProbNNp)")) {
            replace = _pid_var_updated.ReplaceAll(")","") +  "_Meerkat)";
            _pid_var_updated += ")";
        }
        else if ( _pid_var.Contains("_ProbNNp ")) {
            replace = _pid_var_updated.ReplaceAll(" ","") + "_Meerkat ";
            _pid_var_updated += " ";
        }
        else {
            replace = _pid_var_updated + "_Meerkat";
        }
        _cut = ((TString)_cut.GetTitle()).ReplaceAll(_pid_var_updated, replace);
    }
    return _cut;
}

TCut UpdateMVACut(TCut _cut) {
    TString _sCut(_cut);
    if ((SettingDef::Cut::mvaVer != "") && _sCut.Contains("_wMVA_") && !_sCut.Contains(SettingDef::Cut::mvaVer)) {
        MessageSvc::Warning("UpdateMVACut", &_cut);
        _cut = ReplaceCut(_cut, "_lowcen", "_lowcen_v" + SettingDef::Cut::mvaVer);
        MessageSvc::Warning("UpdateMVACut", &_cut);
    }
    return _cut;
}

vector< TString > GetAnalyses(TString _analysis) {
    vector< TString > _analyses = {_analysis};
    if (hash_analysis(_analysis) == Analysis::All) _analyses = {to_string(Analysis::MM), to_string(Analysis::EE)};
    return _analyses;
}

vector< TString > GetYears(TString _year, TString _option) {
    vector< TString > _years = {_year};
    if (_option == "runs") {
        if (hash_year(_year) == Year::All) _years = {to_string(Year::Run1), to_string(Year::Run2p1), to_string(Year::Run2p2)};
    } else if (_option == "comb") {
        //Not working for many reasons...
        if (hash_year(_year) == Year::All) _years = {to_string(Year::All)};
    } else if (_option == "noSplit"){
        _years = {_year};
    }else{
        if (hash_year(_year) == Year::All) _years = {to_string(Year::Y2011), to_string(Year::Y2012), to_string(Year::Y2015), to_string(Year::Y2016), to_string(Year::Y2017), to_string(Year::Y2018)};
        if (hash_year(_year) == Year::Run1) _years = {to_string(Year::Y2011), to_string(Year::Y2012)};
        if (hash_year(_year) == Year::Run2p1) _years = {to_string(Year::Y2015), to_string(Year::Y2016)};
        if (hash_year(_year) == Year::Run2p2) _years = {to_string(Year::Y2017), to_string(Year::Y2018)};
    }
    return _years;
}

TString GetRunFromYear(TString _year) {
    Year _enum = hash_year(_year);
    switch (_enum) {   // TO BE CHECKED
        case Year::Y2011: return to_string(Year::Run1); break;
        case Year::Y2012: return to_string(Year::Run1); break;
        case Year::Run1: return to_string(Year::Run1); break;
        case Year::Y2015: return to_string(Year::Run2p1); break;
        case Year::Y2016: return to_string(Year::Run2p1); break;
        case Year::Run2p1: return to_string(Year::Run2p1); break;
        case Year::Y2017: return to_string(Year::Run2p2); break;
        case Year::Y2018: return to_string(Year::Run2p2); break;
        case Year::Run2p2: return to_string(Year::Run2p2); break;
        case Year::All: return to_string(Year::All); break;
        // case Year::All: MessageSvc::Warning("GetRunFromYear", (TString) "Invalid year", year); break;
        default: MessageSvc::Error("GetRunFromYear", (TString) "Invalid year", _year, "EXIT_FAILURE"); break;
    }
    return TString("");
}

Year GetYearForSample(TString _sample, Year _year , Prj _prj) {
    //COMMENT : availability of this sample in the framework (v10 so far)
    //BASELINE : asked year --> returned year
    //Sample listed here with switches set for Fit on _year, the sample to grab year ( so that sample becomes valid )
    //NB : the year set here is the year used on the efficiency constraint !  Ideally mapping of YEAR:YEAR
    //NB : year set here is relevant when using efficiency constraints in the fit!
    Year _yearTmp     = _year;
    Year _runFromYear = GetRunFromYear( _year);
    if( _sample == "Bs2PhiPsiMM"){
      /*
	    RKst : 11,12,15,16,17
      */
      _year = _runFromYear == Year::Run2p2 ? Year::Y2017 : _year ;
    }
    if( _sample == "Bs2PhiPsiEE"){
      /*
	    RKst : 11,12,15,16,17 
      */
      _year = _runFromYear == Year::Run2p2 ? Year::Y2017 : _year;
    }
    if (_sample == "Bu2PiJPsEE"){ 
      /*
	    RK : 16,17,18
      */
      if( _runFromYear == Year::Run2p1) _year = Year::Y2016; 
      if( _runFromYear == Year::Run1)   _year = Year::Y2016;
      if( _runFromYear == Year::Run2p2) _year = _year; 
    }

    if(_sample == "Bu2K1EE"){
        if((_year == Year::Y2011) || (_year == Year::Y2012) || (_year == Year::Run1)) _year = Year::Run2p2;
    }
    
    if(_sample == "Bu2K2EE"){
        if((_year == Year::Y2011) || (_year == Year::Y2012) || (_year == Year::Run1)) _year = Year::Run2p2;
    }
    
    if(_sample == "Lb2pKJPsEE"){      
      /* 
        RKst : 11,12,16,17,18
        RK   : 11,12,16
      */
        if( _prj != Prj::RK ){  
            if( _runFromYear == Year::Run2p1) _year = Year::Y2016;
            else _year = _year ;
        }else{ 
            //RK : 11,12,16 
            if(_runFromYear == Year::Run2p2) _year = Year::Y2016;
            if(_runFromYear == Year::Run2p1) _year = Year::Y2016;
            if(_runFromYear == Year::Run1)   _year = _year;
        }
    }    
    if(_sample == "Lb2pKPsiEE"){  
      /*
	    RKst : 11,12,16,17
      */
      if( _runFromYear == Year::Run2p1) _year = Year::Y2016;
      if( _runFromYear == Year::Run2p2) _year = Year::Y2017;
    }

    if(_sample == "Lb2pKPsiMM"){ 
      /*
    	RKst : 11,12,15,16,17
      */
      if( _runFromYear == Year::Run2p2) _year = Year::Y2017;
    }
    if( _sample == "Bd2KPiEE" ){
      /*
    	RK : 12,16,17 
	    This component is NOT something Effiicency constrained , we can shuffle years around. 
      */
      //TODO: if( _runFromYear(Year::Run1) _year = Year::Y2012;  SHOULD DO. Old baseline used (probably for stat?) 
      if( _runFromYear == Year::Run1)   _year = Year::Y2016;
      if( _runFromYear == Year::Run2p1) _year = Year::Y2016;
      if( _runFromYear == Year::Run2p2) _year = Year::Y2017;
    }

    if( _sample == "Bd2DNuKstNuEE"){
        //RKst has only 12,16,18 years for this 
        if( GetRunFromYear(_year) == Year::Run1) _year = Year::Y2012;
        if( GetRunFromYear(_year) == Year::Run2p1) _year = Year::Y2016;
        if( GetRunFromYear(_year) == Year::Run2p2) _year = Year::Y2018;
    }
    //TODO : revisit For RooKeyPDFS making and enough stats on MM, EE, bypass bugs! 
    if ((_sample.Contains("2X"))   && ((_year == Year::Y2011) || (_year == Year::Y2012) || (_year == Year::Run1)))   _year = Year::Run1;
    if ((_sample.Contains("2X"))   && ((_year == Year::Y2015) || (_year == Year::Y2016) || (_year == Year::Run2p1))) _year = Year::Run2p1;
    if ((_sample.Contains("2X"))   && ((_year == Year::Y2017) || (_year == Year::Y2018) || (_year == Year::Run2p2))) _year = Year::Run2p2;    
    if(_sample == "Bd2XJPsMM"){
        if((_year == Year::Y2017) || (_year == Year::Y2018) || (_year == Year::Run2p2)) _year = Year::Y2016;
        if((_year == Year::Y2015) || (_year == Year::Run2p1)) _year = Year::Y2016;        
    }
    if(_sample == "Bd2XJPsEE"){
        switch(_prj){
            case Prj::RKst: if( (_year == Year::Y2015) || (_year == Year::Y2016) || (_year == Year::Run2p1)) _year = Year::Run2p1; break; //TODO : R2p1 family on RKst is usable for 2XJPsEE (TRUEIDS  ==0 bug)
            case Prj::RK  : if( (_year == Year::Y2015) || (_year == Year::Y2016) || (_year == Year::Run2p1)) _year = Year::Run2p2; break; //TODO : R2p1 family on RK   is usable for 2XJPsEE (TRUEIDS != 0 bug)
            default : if( (_year == Year::Y2015) || (_year == Year::Y2016) || (_year == Year::Run2p1)) _year = Year::Run2p1; break;
        }
    }
    if ((_sample == "Bu2XJPsEE")  && ((_year == Year::Y2015) || (_year == Year::Y2016) || (_year == Year::Run2p1))) _year = Year::Run2p2;
    if ((_sample == "Bs2XJPsEE")  && ((_year == Year::Y2015) || (_year == Year::Y2016) || (_year == Year::Run2p1))) _year = Year::Run2p2; //TODO : R2p1 family on RK/RKst is unusable for Bs2XJPsEE (TRUEIDS ==0 bug)


    if(_sample == "Bu2XJPsMM"){
        if((_year == Year::Y2017) || (_year == Year::Y2018)  || (_year == Year::Run2p2)) _year = Year::Y2016;
        if((_year == Year::Y2015) || (_year == Year::Run2p1)) _year = Year::Y2016;        
    }
    
    if ((_sample == "Bs2XJPsMM")  && ((_year == Year::Y2017) || (_year == Year::Y2018) || (_year == Year::Run2p2))) _year = Year::Y2016;
    if ((_sample == "Bs2XJPsMM")  && ((_year == Year::Y2015) || (_year == Year::Run2p1))) _year = Year::Y2016;


    //FINAL SUPER HACK, BYPASS ALL DONE BEFORE FORCE 2XJPs cocktails ALWAYS FROM R2p2 , R2p1 available, but noHLT2 can be an issue. 
    //Run1 2XJPs samples have 1. Very low stat, 2. Sim08 and Stripping20. 
    if( _prj == Prj::RKst && (_sample.Contains("2XJPsEE") || _sample == "Bs2KsKstJPsEE" ) ){
        //FLATTEN ALL COCKTAILS TO R2p2 ones for RKst 
        //R2p1 available as well... maybe ok there 
        _year = Year::Run2p2;
    }
    if( _prj == Prj::RK && (_sample.Contains("2XJPsMM") ) ){
        //FLATTEN ALL COCKTAILS TO R2p2 ones for RK! 
        //R2p1 available as well... maybe ok there 
        _year = Year::Y2016;
    }

    if( _prj == Prj::RK && _sample.Contains("2XJPsEE")  ){
        //FLATTEN ALL COCKTAILS TO R2p2 ones for RK 
        //R2p1 available as well... maybe ok there 
        _year = Year::Run2p2;
    }

    if(_sample.Contains("Bu2KEtaPrimeGEE")){
        //RK : 12, 16, 17, 18 
        if( GetRunFromYear(_year) == Year::Run1)   _year = Year::Y2012;
        if( GetRunFromYear(_year) == Year::Run2p1) _year = Year::Y2016;
    }
    
    //2016 sample is quite bad unfortunately S26 instead of S28 plus Sim09b with wrong calo constants.
    if (_year != _yearTmp) MessageSvc::Warning("GetYearForSample", to_string(_yearTmp), "not existing ... using", to_string(_year), "instead");
    return _year;
}

vector< TString > GetPolarities(TString _polarity) {
    vector< TString > _polarities = {_polarity};
    if (hash_polarity(_polarity) == Polarity::All) _polarities = {to_string(Polarity::MD), to_string(Polarity::MU)};
    return _polarities;
}

vector< TString > GetTriggers(TString _trigger, bool _split) {
    vector< TString > _triggers = {_trigger};
    if (!_split) return _triggers;
    if (hash_trigger(_trigger) == Trigger::All) { _triggers = {to_string(Trigger::L0I), to_string(Trigger::L0L)}; }
    return _triggers;
}

vector< TString > GetTracks(TString _track, TString _project, bool _split) {
    vector< TString > _tracks = {_track};
    if (!_split) return _tracks;
    if ((hash_track(_track) == Track::All)) {
        switch (hash_project(_project)) {
            case Prj::RL: _tracks = {to_string(Track::LL), to_string(Track::DD)}; break;
            case Prj::RKS: _tracks = {to_string(Track::LL), to_string(Track::DD)}; break;
            default:
                _tracks = {to_string(Track::TAG), to_string(Track::PRB)};
                // MessageSvc::Error("GetTracks", (TString) "Invalid project", _project, "EXIT_FAILURE");
                break;
        }
    }
    return _tracks;
}

vector< pair< TString, TString > > GetAliasHLT1(TString _project, TString _ana, TString _year) {
    // MessageSvc::Info("GetAliasHLT1", _project, _ana, _year);

    vector< pair< TString, TString > > _heads   = GetParticleBranchNames(hash_project(_project), hash_analysis(_ana), Q2Bin::All, "onlyhead");
    vector< pair< TString, TString > > _hadrons = GetParticleBranchNames(hash_project(_project), hash_analysis(_ana), Q2Bin::All, "onlyhadrons");
    vector< pair< TString, TString > > _leptons = GetParticleBranchNames(hash_project(_project), hash_analysis(_ana), Q2Bin::All, "onlyleptons");

    TString _head = _heads[0].first;
    TString _h1   = _hadrons[0].first;
    TString _h2   = _hadrons[0].first;
    if (_project != to_string(Prj::RK)) _h2 = _hadrons[1].first;
    TString _l1 = _leptons[0].first;
    TString _l2 = _leptons[1].first;

    vector< pair< TString, TString > > _alias;
    if (hash_year(_year) == Year::Y2015) {
        _alias.emplace_back(_h1 + "_Hlt1TrackMVADecision_TOS", _head + "Hlt1TrackMVADecision_TOS");
        if (hash_project(_project) == Prj::RKst || hash_project(_project) == Prj::RPhi) { _alias.emplace_back(_h2 + "_Hlt1TrackMVADecision_TOS", _head + "Hlt1TrackMVADecision_TOS"); }
        if (hash_analysis(_ana) == Analysis::MM) {
            _alias.emplace_back(_l1 + "_Hlt1TrackMVADecision_TOS", _head + "Hlt1TrackMVADecision_TOS");
            _alias.emplace_back(_l2 + "_Hlt1TrackMVADecision_TOS", _head + "Hlt1TrackMVADecision_TOS");
        }
        if (hash_analysis(_ana) == Analysis::EE) {
            _alias.emplace_back(_l1 + "_Hlt1TrackMVADecision_TOS", _head + "Hlt1TrackMVADecision_TOS");
            _alias.emplace_back(_l2 + "_Hlt1TrackMVADecision_TOS", _head + "Hlt1TrackMVADecision_TOS");
        }
    }
    // MessageSvc::Info("GetAliasHLT1", to_string(_alias.size()));
    return _alias;
}

vector< pair< TString, TString > > GetAliasHLT2(TString _project, TString _ana, TString _year) {
    // MessageSvc::Info("GetAliasHLT2", _project, _ana, _year);
    vector< pair< TString, TString > > _heads = GetParticleBranchNames(hash_project(_project), hash_analysis(_ana), Q2Bin::All, "onlyhead");
    TString                            _head  = _heads[0].first;

    vector< pair< TString, TString > > _alias;
    if (hash_year(_year) == Year::Y2015) {
        if (hash_analysis(_ana) == Analysis::MM) {
            _alias.emplace_back(_head + "_Hlt2TopoMuMu2BodyDecision_TOS", _head + "_Hlt2Topo2BodyDecision_TOS");
            _alias.emplace_back(_head + "_Hlt2TopoMuMu3BodyDecision_TOS", _head + "_Hlt2Topo3BodyDecision_TOS");
            _alias.emplace_back(_head + "_Hlt2TopoMuMu4BodyDecision_TOS", _head + "_Hlt2Topo4BodyDecision_TOS");
        }
        if (hash_analysis(_ana) == Analysis::EE) {
            _alias.emplace_back(_head + "_Hlt2TopoE2BodyDecision_TOS", _head + "_Hlt2Topo2BodyDecision_TOS");
            _alias.emplace_back(_head + "_Hlt2TopoE3BodyDecision_TOS", _head + "_Hlt2Topo3BodyDecision_TOS");
            _alias.emplace_back(_head + "_Hlt2TopoE4BodyDecision_TOS", _head + "_Hlt2Topo4BodyDecision_TOS");
            _alias.emplace_back(_head + "_Hlt2TopoEE2BodyDecision_TOS", _head + "_Hlt2Topo2BodyDecision_TOS");
            _alias.emplace_back(_head + "_Hlt2TopoEE3BodyDecision_TOS", _head + "_Hlt2Topo3BodyDecision_TOS");
            _alias.emplace_back(_head + "_Hlt2TopoEE4BodyDecision_TOS", _head + "_Hlt2Topo4BodyDecision_TOS");
        }
    }
    // MessageSvc::Info("GetAliasHLT2", to_string(_alias.size()));
    return _alias;
}

pair< double, double > GetWPID(int _particleID, double _x, double _y, TString _option, TH2 * _mapK, TH2 * _mapPi, TH2 * _mapMu, TH2 * _mapE, TH2 * _mapP) {
    double _w  = 0.;
    double _wi = 0.;
    if (_mapK && (_particleID == PDG::ID::K)) {
        _w  = GetHistogramVal(_mapK, _x, _y, _option);
        _wi = GetHistogramVal(_mapK, _x, _y, _option + "-INTERP");
    }
    if (_mapPi && (_particleID == PDG::ID::Pi)) {
        _w  = GetHistogramVal(_mapPi, _x, _y, _option);
        _wi = GetHistogramVal(_mapPi, _x, _y, _option + "-INTERP");
    }
    if (_mapMu && (_particleID == PDG::ID::M)) {
        _w  = GetHistogramVal(_mapMu, _x, _y, _option);
        _wi = GetHistogramVal(_mapMu, _x, _y, _option + "-INTERP");
    }
    if (_mapE && (_particleID == PDG::ID::E)) {
        _w  = GetHistogramVal(_mapE, _x, _y, _option);
        _wi = GetHistogramVal(_mapE, _x, _y, _option + "-INTERP");
    }
    if (_mapP && (_particleID == PDG::ID::P)) {
        _w  = GetHistogramVal(_mapP, _x, _y, _option);
        _wi = GetHistogramVal(_mapP, _x, _y, _option + "-INTERP");
    }
    return make_pair(_w, _wi);
}

pair< double, double > GetWPID3D_Muon(int _particleID, double _x, double _y, int _z, TString _option, pair< TH1D *, vector < TH2D * > > & _mapsK, pair< TH1D *, vector < TH2D * > > & _mapsPi, pair< TH1D *, vector < TH2D * > > & _mapsMu) {
    double _w  = 0.;
    double _wi = 0.;
    if (_particleID == PDG::ID::K) {
        _w  = GetHistogramVal_3D(_mapsK, _x, _y, _z*1.0, _option);
        _wi = GetHistogramVal_3D(_mapsK, _x, _y, _z*1.0, _option + "-INTERP");
    }
    if (_particleID == PDG::ID::Pi) {
        _w  = GetHistogramVal_3D(_mapsPi, _x, _y, _z*1.0, _option);
        _wi = GetHistogramVal_3D(_mapsPi, _x, _y, _z*1.0, _option + "-INTERP");
    }
    if (_particleID == PDG::ID::M) {
        _w  = GetHistogramVal_3D(_mapsMu, _x, _y, _z*1.0, _option);
        _wi = GetHistogramVal_3D(_mapsMu, _x, _y, _z*1.0, _option + "-INTERP");
    }
    return make_pair(_w, _wi);
}

pair< double, double > GetWPID3D_Electron(int _particleID, double _x, double _y, int _z, TString _option, pair< TH1D *, vector < TH2D * > > & _mapsK, pair< TH1D *, vector < TH2D * > > & _mapsPi, pair< TH1D *, vector < TH2D * > > & _mapsMu, pair< TH1D *, vector < TH2D * > > & _mapsE) {
    double _w  = 0.;
    double _wi = 0.;
    if (_particleID == PDG::ID::K) {
        _w  = GetHistogramVal_3D(_mapsK, _x, _y, _z*1.0, _option);
        _wi = GetHistogramVal_3D(_mapsK, _x, _y, _z*1.0, _option + "-INTERP");
    }
    if (_particleID == PDG::ID::Pi) {
        _w  = GetHistogramVal_3D(_mapsPi, _x, _y, _z*1.0, _option);
        _wi = GetHistogramVal_3D(_mapsPi, _x, _y, _z*1.0, _option + "-INTERP");
    }
    if (_particleID == PDG::ID::M) {
	// We dont have these maps! in the past nulls were passed around here!
	// Either we include this MISID possibility or we ignore it imo (should be basically 0 anyways)
        _w  = 0.;//GetHistogramVal_3D(_mapsMu, _x, _y, _z*1.0, _option);
	_wi = 0.;//GetHistogramVal_3D(_mapsMu, _x, _y, _z*1.0, _option + "-INTERP");
    }
    if (_particleID == PDG::ID::E) {
        _w  = GetHistogramVal_3D(_mapsE, _x, _y, _z*1.0, _option + "-FAC");
        _wi = GetHistogramVal_3D(_mapsE, _x, _y, _z*1.0, _option + "-INTERP" + "-FAC");
    }
    return make_pair(_w, _wi);
}

pair< double, double > GetWPID3D_Hadron(int _particleID, double _x, double _y, int _z, TString _option, pair< TH1D *, vector < TH2D * > > & _mapsK, pair< TH1D *, vector < TH2D * > > & _mapsPi, pair< TH1D *, vector < TH2D * > > & _mapsMu, pair< TH1D *, vector < TH2D * > > & _mapsE, pair< TH1D *, vector < TH2D * > > & _mapsP) {
    double _w  = 0.;
    double _wi = 0.;
    if (_particleID == PDG::ID::K) {
        _w  = GetHistogramVal_3D(_mapsK, _x, _y, _z*1.0, _option);
        _wi = GetHistogramVal_3D(_mapsK, _x, _y, _z*1.0, _option + "-INTERP");
    }
    if (_particleID == PDG::ID::Pi) {
        _w  = GetHistogramVal_3D(_mapsPi, _x, _y, _z*1.0, _option);
        _wi = GetHistogramVal_3D(_mapsPi, _x, _y, _z*1.0, _option + "-INTERP");
    }
    if (_particleID == PDG::ID::M) {
        _w  = GetHistogramVal_3D(_mapsMu, _x, _y, _z*1.0, _option);
        _wi = GetHistogramVal_3D(_mapsMu, _x, _y, _z*1.0, _option + "-INTERP");
    }
    if (_particleID == PDG::ID::E) {
        _w  = GetHistogramVal_3D(_mapsE, _x, _y, _z*1.0, _option + "-FAC");
        _wi = GetHistogramVal_3D(_mapsE, _x, _y, _z*1.0, _option + "-INTERP" + "-FAC");
    }
    if (_particleID == PDG::ID::P) {
        _w  = GetHistogramVal_3D(_mapsP, _x, _y, _z*1.0, _option);
        _wi = GetHistogramVal_3D(_mapsP, _x, _y, _z*1.0, _option + "-INTERP");
    }
    return make_pair(_w, _wi);
}

TString GetBaseVer(TString _ver) {
    if (_ver.Contains("_")) _ver.Remove(_ver.Last('_'), _ver.Length());
    if (_ver.Contains("-")) _ver.Remove(_ver.Last('-'), _ver.Length());
    if (_ver.Contains(".")) _ver.Remove(_ver.Last('.'), _ver.Length());
    return _ver;
}

TString GetPIDTune(TString _year, TString _option) {
    TString _tune = "";
    if (_option == "H") {
        switch (hash_year(_year)) {
            case Year::Y2011: _tune = "MC12TuneV2"; break;
            case Year::Y2012: _tune = "MC12TuneV2"; break;
            case Year::Run1: _tune = "MC12TuneV2"; break;
            case Year::Y2015: _tune = "MC15TuneV1"; break;
            case Year::Y2016: _tune = "MC15TuneV1"; break;
            case Year::Run2p1: _tune = "MC15TuneV1"; break;
            case Year::Y2017: _tune = "MC15TuneV1"; break;
            case Year::Y2018: _tune = "MC15TuneV1"; break;
            case Year::Run2p2: _tune = "MC15TuneV1"; break;
            case Year::All: MessageSvc::Warning("GetTunePID", (TString) "Invalid year", _year); break;
            default: MessageSvc::Error("GetTunePID", (TString) "Invalid year", _year, "EXIT_FAILURE"); break;
        }
    }
    if (_option == "L") {
        switch (hash_year(_year)) {
            case Year::Y2011: _tune = "MC12TuneV3"; break;
            case Year::Y2012: _tune = "MC12TuneV3"; break;
            case Year::Run1: _tune = "MC12TuneV3"; break;
            case Year::Y2015: _tune = "MC15TuneV1"; break;
            case Year::Y2016: _tune = "MC15TuneV1"; break;
            case Year::Run2p1: _tune = "MC15TuneV1"; break;
            case Year::Y2017: _tune = "MC15TuneV1"; break;
            case Year::Y2018: _tune = "MC15TuneV1"; break;
            case Year::Run2p2: _tune = "MC15TuneV1"; break;
            case Year::All: MessageSvc::Warning("GetTunePID", (TString) "Invalid year", _year); break;
            default: MessageSvc::Error("GetTunePID", (TString) "Invalid year", _year, "EXIT_FAILURE"); break;
        }
    }
    return _tune;
}

void PrintVar(RooAbsReal * var) {
    // try to cast the var to item
    if (TString(var->IsA()->GetName()).Contains("RooFormulaVar")) {
        RooFormulaVar formula(*dynamic_cast< RooFormulaVar * >(var), "");
        formula.Print();
    } else if (TString(var->IsA()->GetName()).Contains("RooRealVar")) {
        RooRealVar item(*dynamic_cast< RooRealVar * >(var), "");
        item.Print();
    } else {
        var->Print();
    }
    return;
}

string IsVarInMap(string _par, Str2VarMap _map, string vname) {
    string namepar = _par.substr(0, _par.find("_"));
    for (const auto & _iter : _map) {
        size_t pos    = _iter.first.find("_");
        bool   haspar = (namepar == _iter.first.substr(0, pos));
        if (vname != "" && ((_iter.first).find(vname) != string::npos) && haspar) return _iter.first;
        if (haspar) return _iter.first;
    }
    return (string) "";
}

bool IsVarInMap(TString _par, Str2VarMap _map) {
    if (_map.find(_par.Data()) == _map.end()) return false;
    return true;
}

bool IsCutInMap(TString _cut, Str2CutMap _map) {
    if (_map.find(_cut.Data()) == _map.end()) return false;
    return true;
}

bool IsWeightInMap(TString _weight, Str2WeightMap _map) {
    if (_map.find(_weight.Data()) == _map.end()) return false;
    return true;
}

bool IsCut(TCut _cut) {
    if (_cut == "") return false;
    if (_cut == "1") return false;
    if (_cut == "(1)") return false;
    return true;
}

bool IsWeight(TString _weight) {
    if (_weight == "") return false;
    if (_weight == "1.") return false;
    if (_weight == "(1.)") return false;
    if (_weight == "1") return false;
    if (_weight == "(1)") return false;
    return true;
}

bool HasWeight(TCut _cut) {
    TString _sCut(_cut);

    bool _flag = false;
    // MC
    if (_sCut.Contains("wPID")) _flag = true;
    if (_sCut.Contains("wBKIN")) _flag = true;
    if (_sCut.Contains("wMULT")) _flag = true;
    if (_sCut.Contains("wRECO")) _flag = true;
    if (_sCut.Contains("wBDT")) _flag = true;
    if (_sCut.Contains("wL0")) _flag = true;
    if (_sCut.Contains("wTOS")) _flag = true;
    if (_sCut.Contains("wTIS")) _flag = true;
    if (_sCut.Contains("wHLT")) _flag = true;
    if (_sCut.Contains("wdp")) _flag = true;
    if (_sCut.Contains("wPTRECO")) _flag = true;
    if (_sCut.Contains("wDecFile")) _flag = true;
    if (_sCut.Contains("wkin_RpK"))  _flag = true;
    if (_sCut.Contains("wkin"))  _flag = true;


    if (_sCut.Contains("wiPID")) _flag = true;
    if (_sCut.Contains("wiBKIN")) _flag = true;
    if (_sCut.Contains("wiMULT")) _flag = true;
    if (_sCut.Contains("wiRECO")) _flag = true;
    if (_sCut.Contains("wiBDT")) _flag = true;
    if (_sCut.Contains("wiL0")) _flag = true;
    if (_sCut.Contains("wiTOS")) _flag = true;
    if (_sCut.Contains("wiTIS")) _flag = true;
    if (_sCut.Contains("wiHLT")) _flag = true;
    if (_sCut.Contains("widp")) _flag = true;
    if (_sCut.Contains("wiPTRECO")) _flag = true;
    if (_sCut.Contains("wiDecFile")) _flag = true;
    if (_sCut.Contains("wikin_RpK"))  _flag = true;
    if (_sCut.Contains("wikin"))  _flag = true;

    if (_sCut.Contains("wfPID")) _flag = true;
    if (_sCut.Contains("wfBKIN")) _flag = true;
    if (_sCut.Contains("wfMULT")) _flag = true;
    if (_sCut.Contains("wfRECO")) _flag = true;
    if (_sCut.Contains("wfBDT")) _flag = true;
    if (_sCut.Contains("wfL0")) _flag = true;
    if (_sCut.Contains("wfTOS")) _flag = true;
    if (_sCut.Contains("wfTIS")) _flag = true;
    if (_sCut.Contains("wfHLT")) _flag = true;
    if (_sCut.Contains("wfdp")) _flag = true;
    if (_sCut.Contains("wfPTRECO")) _flag = true;
    if (_sCut.Contains("wfDecFile")) _flag = true;
    if (_sCut.Contains("wfkin_RpK"))  _flag = true;
    if (_sCut.Contains("wfkin"))  _flag = true;
    // CL
    if (_sCut.Contains("nsig_") && _sCut.Contains("_sw")) _flag = true;

    // if (_sCut.Contains("*")) _flag = true;
    return _flag;
}

void PrintPars(Str2VarMap _map, string _option) {
    if (_map.size() > 0) {
        MessageSvc::Info("PrintPars", to_string(_map.size()), _option);
        for (const auto & _par : _map) MessageSvc::Info(_par.first, _par.second);
    }
    return;
}

bool IsVarInTuple(TTree * _tuple, TString _name) {
    if (_tuple->GetListOfBranches() != nullptr) {
        if (_tuple->GetListOfBranches()->Contains(_name)) return true;
    }
    return false;
    /*
    bool _found = false;
    for (auto * _branch : *_tuple->GetListOfBranches()) {
        if (_branch->GetName() == _name) {
            _found = true;
            break;
        }
    }
    return _found;
    */
}

void PlotBranches(TTree * _tuple, TString _name) {
    Long64_t _entries = _tuple->GetEntries();
    if (_entries == 0) {
        MessageSvc::Warning("PlotBranches", (TString) "Empty tuple", _name);
        return;
    }
    if (_name != "") _name = "_" + _name;
    MessageSvc::Line();
    MessageSvc::Info("PlotBranches", (TString) "Branches", _name, to_string(_tuple->GetListOfBranches()->GetSize()));
    _name = "Branch" + _name;
    IOSvc::runCommand((TString)"rm -rf " + _name + ".pdf");
    vector< TString > _branches;
    for (auto * _branch : *_tuple->GetListOfBranches()) { _branches.push_back(_branch->GetName()); }
    TCanvas _canvas("canvas");
    gStyle->SetOptStat("emr");
    _canvas.SaveAs(_name + ".pdf[");
    for (auto _branch : _branches) {
        _tuple->Draw(_branch, "", "", _entries > 1e4 ? 1e4 : _entries);
        _canvas.SetLogy(0);
        _canvas.SaveAs(_name + ".pdf");
        _canvas.SetLogy(1);
        _canvas.SaveAs(_name + ".pdf");
    }
    _canvas.SaveAs(_name + ".pdf]");
    IOSvc::runCommand((TString)"tar -czf " + _name + ".tar.gz " + _name + ".pdf");
    IOSvc::runCommand((TString)"rm -rf " + _name + ".pdf");
    MessageSvc::Line();
    return;
}

Long64_t MultCandRandomKill(TChain * _tuple, vector< Long64_t > _entry, TRandom3 & _rnd) {
    if (_entry.size() == 1) return _entry[0];
    return _entry[(int) floor(_rnd.Rndm() * _entry.size())];
}

Long64_t MultCandBestBkgCat(TChain * _tuple, vector< Long64_t > _entry, TRandom3 & _rnd) {
    if (_entry.size() == 1) return _entry[0];

    vector< pair< TString, TString > > _heads   = GetParticleBranchNames(hash_project(SettingDef::Config::project), hash_analysis(SettingDef::Config::ana), Q2Bin::All, "onlyhead");
    vector< pair< TString, TString > > _hadrons = GetParticleBranchNames(hash_project(SettingDef::Config::project), hash_analysis(SettingDef::Config::ana), Q2Bin::All, "onlyhadrons");
    vector< pair< TString, TString > > _leptons = GetParticleBranchNames(hash_project(SettingDef::Config::project), hash_analysis(SettingDef::Config::ana), Q2Bin::All, "onlyleptons");

    TString _head = _heads[0].first;
    TString _h1   = _hadrons[0].first;
    TString _h2   = _hadrons[0].first;
    if (SettingDef::Config::project != to_string(Prj::RK)) _h2 = _hadrons[1].first;
    TString _l1 = _leptons[0].first;
    TString _l2 = _leptons[1].first;

    TupleReader               _tupleReader = TupleReader(_tuple);
    TTreeReaderValue< int >   _bkgCat      = _tupleReader.GetValue< int >(_head + "_BKGCAT");
    TTreeReaderValue< int > * _h1ID        = _tupleReader.GetValuePtr< int >(_h1 + "_TRUEID");
    TTreeReaderValue< int > * _h2ID        = _tupleReader.GetValuePtr< int >(_h2 + "_TRUEID");
    TTreeReaderValue< int > * _l1ID        = _tupleReader.GetValuePtr< int >(_l1 + "_TRUEID");
    TTreeReaderValue< int > * _l2ID        = _tupleReader.GetValuePtr< int >(_l2 + "_TRUEID");

    int                _current = 0;
    int                _iBest   = 0;
    double             _best    = 1e9;
    vector< Long64_t > _multi;

    for (size_t e = 0; e < _entry.size(); e++) {
        _tupleReader.Reader()->SetEntry(_entry[e]);
        _current = *_bkgCat;
        if ((_current >= 0) && (_current <= _best)) {
            if (_current != _best) {
                _best  = _current;
                _iBest = e;
                _multi.clear();
            }
            _multi.emplace_back(_entry[e]);
        }
    }

    if ((_best == 60) && (_multi.size() > 1)) {
        _current = 0;
        _best    = 0;
        for (size_t e = 0; e < _entry.size(); e++) {
            _tupleReader.Reader()->SetEntry(_entry[e]);
            if (*_bkgCat == 60) {
                if (_h2 != "") {
                    _current = GetSignalLikeness(**_h1ID, **_h2ID, **_l1ID, **_l2ID);
                } else {
                    _current = GetSignalLikeness(**_h1ID, 0, **_l1ID, **_l2ID);
                }
                if (_current >= _best) {
                    if (_current != _best) {
                        _best  = _current;
                        _iBest = e;
                        _multi.clear();
                    }
                    _multi.emplace_back(_entry[e]);
                }
            }
        }
    }

    int _mBest = (int) floor(_rnd.Rndm() * _multi.size());
    if (_multi.size() > 1) return _multi[_mBest];
    return _entry[_iBest];
}

int GetSignalLikeness(int _h1, int _h2, int _l1, int _l2) {
    int _pdgK  = PDG::ID::K;
    int _pdgPi = PDG::ID::Pi;
    int _pdgP  = PDG::ID::P;
    int _pdgL;
    switch (hash_analysis(SettingDef::Config::ana)) {
        case Analysis::MM: _pdgL = PDG::ID::M; break;
        case Analysis::EE: _pdgL = PDG::ID::E; break;
        default: MessageSvc::Error("Wrong analysis", SettingDef::Config::ana, "EXIT_FAILURE"); break;
    }

    // BIT MAP OF DECAY DAUGHTERS : H1-H2-L1-L2
    TString _map = "";
    switch (hash_project(SettingDef::Config::project)) {
        case Prj::RKst: _map = to_string(TMath::Abs(_h1) == _pdgK) + to_string(TMath::Abs(_h2) == _pdgPi) + to_string(TMath::Abs(_l1) == _pdgL) + to_string(TMath::Abs(_l2) == _pdgL); break;
        case Prj::RK: _map = to_string(TMath::Abs(_h1) == _pdgK) + "0" + to_string(TMath::Abs(_l1) == _pdgL) + to_string(TMath::Abs(_l2) == _pdgL); break;
        case Prj::RPhi: _map = to_string(TMath::Abs(_h1) == _pdgK) + to_string(TMath::Abs(_h2) == _pdgK) + to_string(TMath::Abs(_l1) == _pdgL) + to_string(TMath::Abs(_l2) == _pdgL); break;
        case Prj::RL: _map = to_string(TMath::Abs(_h1) == _pdgP) + to_string(TMath::Abs(_h2) == _pdgPi) + to_string(TMath::Abs(_l1) == _pdgL) + to_string(TMath::Abs(_l2) == _pdgL); break;
        case Prj::RKS: _map = to_string(TMath::Abs(_h1) == _pdgPi) + to_string(TMath::Abs(_h2) == _pdgPi) + to_string(TMath::Abs(_l1) == _pdgL) + to_string(TMath::Abs(_l2) == _pdgL); break;
        default: MessageSvc::Error("Wrong project", SettingDef::Config::project, "EXIT_FAILURE"); break;
    }

    vector< TString > _1maps = {"1000", "0100", "0010", "0001"};                   // 1 MATCH
    vector< TString > _2maps = {"1100", "1010", "1001", "0101", "0011", "0110"};   // 2 MATCHES
    vector< TString > _3maps = {"1110", "1101", "1011", "0111"};                   // 3 MATCHES
    vector< TString > _4maps = {"1111"};                                           // 4 MATCHES

    int _signalLike = 0;
    if (find(_1maps.begin(), _1maps.end(), _map) != _1maps.end()) _signalLike = 1;
    if (find(_2maps.begin(), _2maps.end(), _map) != _2maps.end()) _signalLike = 2;
    if (find(_3maps.begin(), _3maps.end(), _map) != _3maps.end()) _signalLike = 3;
    if (find(_4maps.begin(), _4maps.end(), _map) != _4maps.end()) _signalLike = 4;

    return _signalLike;
}

pair< double, double > GetAverageVal(const vector< double > & _values, const vector< double > & _errors, const vector< double > & _weights, bool _debug) {
    if (_values.size() != 1) MessageSvc::Info("GetAverageVal", to_string(_values.size()));
    if (_values.size() != _errors.size()) MessageSvc::Error("GetAverageVal", (TString) "values/size size not matching", "EXIT_FAILURE");
    if (_values.size() != _weights.size()) MessageSvc::Error("GetAverageVal", (TString) "values/weights not matching", "EXIT_FAILURE");

    double _mean   = 0;
    double _sumW   = 0;
    double _error2 = 0;
    // For generator level efficiency we      mean = value * weight / (sum(weights))
    // For generator level efficiency we      eff  = sqrt(SUM([ weight/sum(weight) * err ] ^{2}))
    for (auto && entry : zip_range(_values, _errors, _weights)) {
        auto value  = entry.get< 0 >();
        auto weight = entry.get< 2 >();
        auto error  = entry.get< 1 >();
        if (_debug) {
            MessageSvc::Debug("********************************************");
            MessageSvc::Debug("Value", to_string(value), "");
            MessageSvc::Debug("Error", to_string(error), "");
            MessageSvc::Debug("Weight", to_string(weight), "");
        }
        _mean += value * weight;
        _sumW += weight;
        _error2 += TMath::Sq(weight * error);
    }
    double _error = TMath::Sqrt(_error2 / (_sumW * _sumW));
    _mean         = _mean / _sumW;
    return make_pair(_mean, _error);
}

RooRealVar * GetAverage(const vector< double > & _values, const vector< double > & _errors, const vector< double > & _weights) {
    auto         mean_err = GetAverageVal(_values, _errors, _weights);
    RooRealVar * _var     = new RooRealVar("var", "var", mean_err.first);
    _var->setError(mean_err.second);
    return _var;
}



vector< TString > GetBranchesFromExpression(TChain * _tuple, const TString & _expression) {
    vector< TString > _branches = {};
    if (IsCut(TCut(_expression)) || IsWeight(_expression)) {
        TString                            _exp     = _expression;
        vector< pair< TString, TString > > _aliases = GetAliasesFromExpression(_tuple, _exp);
        for (auto _alias : _aliases) { _exp.ReplaceAll(_alias.first, "1"); }
        // MessageSvc::Info("GetBranchesFromExpression", (TString) _tuple->GetName(), _exp);
        TTreeFormula _formula("formula", _exp, _tuple);
        //_tuple->SetNotify(& _formula);
        _branches.reserve(_formula.GetNcodes());
        for (size_t _v = 0; _v < _formula.GetNcodes(); ++_v) { _branches.push_back((TString) _formula.GetLeaf(_v)->GetName()); }
        RemoveVectorDuplicates(_branches);
        MessageSvc::Info("GetBranchesFromExpression", (TString) _tuple->GetName(), to_string(_branches.size()));
    }
    return _branches;
}

vector< pair< TString, TString > > GetAliasesFromExpression(TChain * _tuple, const TString & _expression) {
    vector< pair< TString, TString > > _aliases = {};
    if (IsCut(TCut(_expression)) || IsWeight(_expression)) {
        TList * _list = (TList *) _tuple->GetListOfAliases();
        if (_list != nullptr) {
            // MessageSvc::Info("GetAliasesFromExpression", (TString) _tuple->GetName(), _expression);
            for (auto _alias : *_list) {
                if (_expression.Contains(_alias->GetName())) _aliases.push_back(make_pair(_alias->GetName(), _alias->GetTitle()));
            }
            RemoveVectorDuplicates(_aliases);
            MessageSvc::Info("GetAliasesFromExpression", (TString) _tuple->GetName(), to_string(_aliases.size()));
        }
    }
    return _aliases;
}

vector< TString > GetBranchesAndAliasesFromExpression(TChain * _tuple, const TString & _expression) {
    vector< TString >                  _branches = GetBranchesFromExpression(_tuple, _expression);
    vector< pair< TString, TString > > _aliases  = GetAliasesFromExpression(_tuple, _expression);
    for (auto _alias : _aliases) {
        vector< TString > _vars = GetBranchesFromExpression(_tuple, _alias.second);
        _branches.insert(_branches.end(), _vars.begin(), _vars.end());
    }
    RemoveVectorDuplicates(_branches);
    MessageSvc::Info("GetBranchesAndAliasesFromExpression", (TString) _tuple->GetName(), to_string(_branches.size()));
    return _branches;
}

vector< pair< TString, TString > > GetAllAlias(TTree * _tuple, bool _clean) {
    if (_tuple == nullptr) { MessageSvc::Error("GetAllAlias", (TString) "tuple is nullptr", "EXIT_FAILURE"); }
    vector< pair< TString, TString > > _allAliases = {};
    if (_tuple->GetListOfAliases() != nullptr) {
        TList * _aliases = (TList *) _tuple->GetListOfAliases();
        if (_aliases != nullptr) {
            for (auto _alias : *_aliases) {
                if (_clean) {
                    _allAliases.push_back(make_pair(_alias->GetName(), ((TString) _alias->GetTitle()).ReplaceAll("<Double_t>", "").ReplaceAll(")+0", ")")));
                } else {
                    _allAliases.push_back(make_pair(_alias->GetName(), ((TString) _alias->GetTitle())));
                }
            }
        }
    }
    return _allAliases;
}

vector< pair< TString, TString > > GetAllAliases(TChain * _tuple) {
    if (_tuple == nullptr) { MessageSvc::Error("GetAllAliases", (TString) "tuple is nullptr", "EXIT_FAILURE"); }
    vector< pair< TString, TString > > _packedAliases = {};
    if (_tuple->GetListOfAliases() != nullptr) {
        TObjArray * _aliases = (TObjArray *) _tuple->GetListOfAliases();
        for (int i = 0; i < _aliases->GetEntries(); ++i) {
            TString _name = _aliases->At(i)->GetName();
            TString _expr = _aliases->At(i)->GetTitle();
            _packedAliases.push_back(make_pair(_name, _expr));
        }
    }
    return _packedAliases;
}

RooDataSet * GetRooDataSet(TChain * _tuple, const TString & _name, RooArgList _varList, const TCut & _cut, const TString & _weight, double _frac, TString _option) {
    MessageSvc::Line();
    MessageSvc::Info("GetRooDataSet [option]", _name, _option);
    MessageSvc::Info("GetRooDataSet [cut]",    _name, TString(_cut));
    MessageSvc::Info("GetRooDataSet [weight]", _name, TString(_weight));
    TCut _cutTmp = _cut;
    MessageSvc::Info("Cut", &_cutTmp);
    if (IsCut(_cutTmp)) {
        vector< pair< TString, TString > > _aliases = GetAliasesFromExpression(_tuple, TString(_cutTmp));
        if (_aliases.size() != 0) {
            MessageSvc::Info("GetRooDataSet", (TString) "Replacing aliases");
            for (auto _alias : _aliases) {
                TString _expression = ((TString) _alias.second).ReplaceAll("<Double_t>", "").ReplaceAll(")+0", ")");
                MessageSvc::Info("GetRooDataSet", _alias.first, "->", _expression);
                _cutTmp = ReplaceCut(_cutTmp, _alias.first, _expression);
            }
            MessageSvc::Info("Cut", &_cutTmp);
        }
    }
    if(_option.Contains("leakage") || IsWeight(_weight) ||  _option.Contains("reweightXJPs") ){
        ROOT::DisableImplicitMT();
        MessageSvc::Warning("RooDataSet maker by hand done routine applied");
        /* HACK FOR LEAKAGE FITS : 
            Ntuples are from TupleProcess, the JPs_Smearing has to be "appended" and the B_MASS variable has been attached. 
            ONCE we reprocess and we attach both, we don't need anything about year-jump around and project loading
        */
        RooRealVar * var  = static_cast<RooRealVar*>(_varList.at(0));//this is the plotting variable ? 
        TString _theBMass = var->GetName();    
        TString _originalMass = _theBMass;
        ROOT::RDataFrame df(*_tuple); 
        auto myNode = ROOT::RDF::RNode( df);       
        /* THIS IS AN HACK , CLEAN UP AND HAVE BRANCH ON TUPLE DIRECTLY*/
        if( _option.Contains("leakage")){
            auto _allopts = TokenizeString(_option, "-");
            TString _LeakageBit = "";
            for( auto & el : _allopts){
                if( el.Contains("leakage")){
                    _LeakageBit = el; break;
                }
            }
            _theBMass = StripStringBetween(_LeakageBit, "leakage[", "]");
            MessageSvc::Warning("BMassSmearedVar", _theBMass);
            var->Print("v");
            MessageSvc::Warning("Special DataSetMaking on Leakage Component");
            MessageSvc::Warning("Cut", TString(_cut.GetTitle()));
            MessageSvc::Warning("Weight", _weight);
            MessageSvc::Warning("BVAR", _theBMass);        
            Year _yearSmear = Year::Y2017; 
            if(_name.Contains("R2p2")) _yearSmear = Year::Y2018;
            if(_name.Contains("R2p1")) _yearSmear = Year::Y2016;
            if(_name.Contains("R1")) _yearSmear = Year::Y2012;
            if(_name.Contains("11")) _yearSmear = Year::Y2011;
            if(_name.Contains("12")) _yearSmear = Year::Y2012;
            if(_name.Contains("15")) _yearSmear = Year::Y2015;        
            if(_name.Contains("16")) _yearSmear = Year::Y2016;
            if(_name.Contains("17")) _yearSmear = Year::Y2017;
            if(_name.Contains("18")) _yearSmear = Year::Y2018;
            Prj _prj = Prj::RK;
            if( myNode.HasColumn("B0_BKGCAT")) _prj = Prj::RKst; 
            if( myNode.HasColumn("Bp_BKGCAT")) _prj = Prj::RK; 
            if( myNode.HasColumn("Bs_BKGCAT")) _prj = Prj::RPhi; 	
            MessageSvc::Warning("Node Filter Cut");
            std::cout<<RED<<"[BYHAND_DataSet] Setup Fill by hand (PRJ) with smearing setup... " << to_string(_prj) << "  [ YEAR = " << to_string(_yearSmear) << " ]" << RESET<< std::endl;
            std::cout<<RED<<"[BYHAND_DataSet] Setup Fill by hand (PRJ) with smearing setup... Expect B var \n \t " << _theBMass << RESET<< std::endl;

            myNode = HelperProcessing::AppendQ2SmearColumns( myNode, _prj,_yearSmear);
            myNode = HelperProcessing::AppendBSmearColumns( myNode, _prj,_yearSmear);
        }
        //!!!!!!!!!!!!! FILTER !!!!!!!!!!!!!//
        TString _selection(_cut.GetTitle());
        if( _option.Contains("leakage") && !_theBMass.BeginsWith("JPs")  ){   
            std::cout<<RED<< "Replacing B invariant mass cut for ranges , before " << _originalMass << " --> " <<  _theBMass <<RESET<< std::endl;
            _selection = _selection.ReplaceAll(_originalMass, _theBMass );
        }     
        //TODO : fix me for this with alias branches cuts.
        vector<pair<TString,TString>> _aliasesAdd;
        if (_tuple->GetListOfAliases() != nullptr) {                                                                                                                                                       
            TList * _aliases = (TList *) _tuple->GetListOfAliases();
            if (_aliases != nullptr) {                                                                                                                                                                     
                for (auto _alias : *_aliases){	      
                _aliasesAdd.push_back(make_pair(_alias->GetName(), ((TString) _alias->GetTitle()).ReplaceAll("<Double_t>", "").ReplaceAll(")+0", ")")));                                                
                }                                                                                                                                                                                         
            }                                                                                                                                                                                             
        }
        for( auto & alias : _aliasesAdd){                                                                                                                                                                  
            myNode = myNode.Define( alias.first.Data(),alias.second.Data());                                                                                                                     
        }       
        MessageSvc::Info("GetRooDataSet [cut applied]",    _name, _selection);
        myNode = myNode.Filter(_selection.Data());
        //!!!!!!!!!!!!! FILTER !!!!!!!!!!!!!//
        TString _weightUse = _weight;
        if( _option.Contains("reweightXJPs") && !_option.Contains("leakage") ) {
            MessageSvc::Warning("Applying 2XJPs inner-weighting! (2)");
            _weightUse = "wDecFile";
            myNode = Reweight2XJPs(myNode, _weightUse );           
        }
        //Must cut on JPs/Smeared mass and B_Smeared B mass
        std::cout<<RED<<"[BYHAND_DataSet] CUT    : "<< _selection << RESET<< std::endl;
        std::cout<<RED<<"[BYHAND_DataSet] WEIGHT : "<< _weightUse.Data()  << RESET<< std::endl;       

        RooDataSet * _data = nullptr; 
        RooRealVar * _varW = new RooRealVar("weight", _weightUse, 1);
        bool _isWeighted = IsWeight(_weightUse);
        bool _addWeight = false;
        if (IsWeight(_weightUse)){
            _varW->setConstant(0);
            _data = new RooDataSet("dataSet_" + _name, "dataSet_" + _name,RooArgSet(*var,*_varW), RooFit::WeightVar(*_varW));
            _addWeight = true;
        }else{
            auto args = RooArgSet(*var);
            _data = new RooDataSet("dataSet_" + _name, "dataSet_" + _name,RooArgSet(*var));
        }
        //!!!!!!!!!!!!!!!!!! RUN IT SINGLE THREAD , ELSE IT CRASH !!!!!!!!!!!!!!!!!
        std::cout<< "RANGE CUTTED ON OBSERVABLES = "<< var->getMin() << " , "<< var->getMax() << std::endl;
        //TODO : expand if var is a list of columns! 
        auto makeDataSet=[ _data , var, _varW, _addWeight]( std::pair< double, double> & myPair){
            double xObs      = myPair.first;
            double weightVal = myPair.second;
            if( xObs< var->getMin() ||  xObs > var->getMax() ) return;
            var->setVal( xObs);
            if(_addWeight){
                _varW->setVal(weightVal);
                _data->add(RooArgSet(*var, *_varW), _varW->getVal() );
            }else{
                _data->add(*var);
            }
        };
        MessageSvc::Info("Extract Vars for DataSet, Event Loop start");
        std::cout<<"[BYHAND_DataSet] Before (entries) = "<< std::endl;
        std::cout<<"[BYHAND_DataSet] BMassVar         = "<< _theBMass << std::endl;
        std::cout<<"[BYHAND_DataSet] Weight           = "<< _weightUse << std::endl;
        _data->Print("v");
        myNode.Define("weight", _weightUse.Data())
              .Define("pair", [](double varVal, double _weightVal){ std::pair<double,double> v(varVal,_weightVal); return v;}, {_theBMass.Data(),"weight"})
              .Foreach(makeDataSet, {"pair"});
        MessageSvc::Info("Extract Vars for DataSet, Event Loop end");
        delete _varW;
        MessageSvc::Info("RooDataSet maker by hand done, print final dataset");
        _data->Print("v");
        return _data;    
    }else{
        MessageSvc::Warning("RooDataSet maker (baseline approach) applied");
    }
    vector< TString > _branches = GetBranchesFromExpression(_tuple, TString(_cutTmp));
    MessageSvc::Info("Weight", _weight);
    if (IsWeight(_weight)) {
        vector< TString > _weights = GetBranchesFromExpression(_tuple, _weight);
        if (_weights.size() == 0) MessageSvc::Error("GetRooDataSet", (TString) "Weight not in tuple", _weight, "EXIT_FAILURE");
        _branches.insert(_branches.end(), _weights.begin(), _weights.end());
        RemoveVectorDuplicates(_branches);
    }

    vector< RooRealVar * > _vars;
    for (auto _branch : _branches) {
        RooRealVar * _var = new RooRealVar(_branch, _branch, 1);
        _var->setConstant(0);
        if (!_varList.contains(*_var)) _varList.add(*_var);
        _vars.push_back(_var);
    }

    MessageSvc::Info("VarList", to_string(_varList.getSize()));
    MessageSvc::Info("VarList", &_varList);

    Long64_t _entries = _tuple->GetEntries();
    MessageSvc::Info("Entries", to_string(_entries));

    if (_frac == 0) _frac = SettingDef::Tuple::frac;
    MessageSvc::Warning("Frac", to_string(_frac));

    if (_option.Contains("rescale") && (_frac != -1)) {
        MessageSvc::Info("Option", _option);
        Long64_t _centries = _tuple->GetEntries(_cutTmp);
        double   _scale    = (double) _entries / (double) _centries;
        MessageSvc::Info("Entries Cut", to_string(_centries));
        MessageSvc::Info("Scaling", to_string(_scale));
        if ((_frac >= 0.f) && (_frac < 1.0f) && (_frac * _scale > 1)) _frac = -1;
        if ((_frac >= 1) && (_frac > _centries)) _frac = -1;
        MessageSvc::Warning("Frac", to_string(_frac));
    }

    Long64_t _maxEntries = _entries;
    if ((_frac >= 0.f) && (_frac < 1.0f)) _maxEntries = (int) floor(_frac * _maxEntries);
    if (_frac >= 1) _maxEntries = (int) floor(_frac);
    if (_maxEntries < _entries) {
        MessageSvc::Info("SetEntries", to_string(_maxEntries));
        _tuple->SetEntries(_maxEntries);
    }

    auto _start = chrono::high_resolution_clock::now();

    MessageSvc::Info("GetRooDataSet", (TString) "Importing ...");
    RooDataSet * _data = nullptr;
    if (IsWeight(_weight)) {
        if (_weight.Contains("*")) MessageSvc::Error("GetRooDataSet", (TString) "Only works if \"Weight\" contains a single branch name", _weight, "EXIT_FAILURE");
        RooRealVar * _varW = new RooRealVar(_weight, _weight, 1);
        _varW->setConstant(0);
        _data = new RooDataSet("dataSet_" + _name, "dataSet_" + _name, _varList, Import(*_tuple), Cut(TString(_cutTmp)), WeightVar(_varW->GetName()));
        delete _varW;
    } else {
        _data = new RooDataSet("dataSet_" + _name, "dataSet_" + _name, _varList, Import(*_tuple), Cut(TString(_cutTmp)));
    }

    for (auto _var : _vars) { delete _var; }

    if (_maxEntries < _entries) _tuple->SetEntries(_entries);

    auto _stop = chrono::high_resolution_clock::now();
    MessageSvc::Warning("GetRooDataSet", (TString) "Took", to_string(chrono::duration_cast< chrono::seconds >(_stop - _start).count()), "seconds");

    MessageSvc::Info("GetRooDataSet", _data);
    MessageSvc::Line();
    return _data;
}

RooDataSet * GetRooDataSetCopy(TupleReader & _tupleReader, const TString & _name, RooArgList _varList, const TCut & _cut, const TString & _weight, double _frac, TString _option) {
    MessageSvc::Line();
    MessageSvc::Info("GetRooDataSetCopy", _name, _option);

    MessageSvc::Info("Cut", &_cut);
    vector< TString > _branches = GetBranchesAndAliasesFromExpression(_tupleReader.Tuple(), TString(_cut));

    vector< RooRealVar * > _vars;
    if (_option.Contains("reduce")) {
        for (auto _branch : _branches) {
            RooRealVar * _var = new RooRealVar(_branch, _branch, 1);
            _var->setConstant(0);
            if (!_varList.contains(*_var)) _varList.add(*_var);
            _vars.push_back(_var);
        }
    }
    MessageSvc::Info("VarList", to_string(_varList.getSize()));
    MessageSvc::Info("VarList", &_varList);
    TString _tmpFileName = "tmp" + _name + ".root";
    if(!SettingDef::IO::outDir.EndsWith("/")){
        _tmpFileName = SettingDef::IO::outDir +"/"+_tmpFileName;
    }else{
        _tmpFileName = SettingDef::IO::outDir + _tmpFileName;        
    }
    TFile   _tFile(_tmpFileName, to_string(OpenMode::RECREATE));
    TTree * _tuple = _tupleReader.CopyTuple(_cut, "", _frac);
    _tuple->Write("DecayTuple", TObject::kOverwrite);

    RooDataSet * _data = GetRooDataSet((TChain *) _tuple, _name, _varList, TCut(NOCUT), _weight, _frac, _option);

    for (auto _var : _vars) { delete _var; }

    delete _tuple;
    _tFile.Close();

    return _data;
}

RooDataSet * GetRooDataSetSnapshot( TChain * _tuple, const TString & _name,   ConfigHolder  _configHolder, TString _weightOption, RooArgList _varList, const TCut & _cut, const TString & _weight, double _frac, TString _option ){            
    MessageSvc::Line();
    MessageSvc::Info("GetRooDataSetSnapshot [option]", _name, _option);
    MessageSvc::Info("GetRooDataSetSnapshot [cut]",    _name, TString(_cut));
    MessageSvc::Info("GetRooDataSetSnapshot [weight]", _name, TString(_weight));
    ROOT::DisableImplicitMT();
    if(_configHolder.GetSample() == "LPT" && SettingDef::Fit::LPTMCandidates && SettingDef::Weight::iBS< 0 ){         
        //Not in BS mode fit...
        MessageSvc::Warning("GetRooDataSetSnapshot REAL DATA!, apply Multiple Candidate Removal On The Fly, via EvtNb, RunNb [TO IMPLEMENT]");     
        _tuple->SetBranchStatus("runNumber"  , 1);
        _tuple->SetBranchStatus("eventNumber", 1);
        _tuple->SetBranchStatus("Bp_PT", 1);
        _tuple->SetBranchStatus("B0_PT", 1);
        int _totCandidates = _tuple->GetEntries();
        RooDataSet * _data = nullptr; 
        RooRealVar * var  = static_cast<RooRealVar*>(_varList.at(0));//this is the plotting variable ? 
        ROOT::RDataFrame df(*_tuple); 
        auto myNode = ROOT::RDF::RNode( df);
        TString _bmass = var->GetName();
        //get out the mass range cut from the varMin and var Max range ...
        TString _selection = TString(_cut + TCut(TString(fmt::format("{0}>{1}&&{0}<{2}", var->GetName(), var->getMin(), var->getMax()))));
        
        //=============== Append All aliases as new columns first ===============// 
        vector<pair<TString,TString>> _aliasesAdd;
        if (_tuple->GetListOfAliases() != nullptr){
            TList * _aliases = (TList *) _tuple->GetListOfAliases();
            if (_aliases != nullptr){
                for (auto _alias : *_aliases){
                    _aliasesAdd.push_back(make_pair(_alias->GetName(), ((TString) _alias->GetTitle()).ReplaceAll("<Double_t>", "").ReplaceAll(")+0", ")")));
                }
            }
        }
        for( auto & alias : _aliasesAdd){
            myNode = myNode.Define( alias.first.Data(),alias.second.Data());
        }
        //=============== Filter out dataset ===============// 
        myNode = myNode.Filter( _selection.Data() );

        //=============== InfoEvent for multiple candidate removal ===============// 
        struct InfoEvent{ 
            InfoEvent() = default;
            InfoEvent( double Bmass,  UInt_t runNumber, ULong64_t  eNumber){
                MASS = Bmass; 
                RUNNB = runNumber;
                EVTNB = eNumber;
            };
            double MASS; 
            UInt_t RUNNB; 
            ULong64_t EVTNB; 
            double mass()const{ return MASS;}
            pair< UInt_t, ULong64_t > ID()const{ return make_pair(RUNNB, EVTNB);}
        };
        auto makeInfo=[]( double _mass,UInt_t _runNumber, ULong64_t _evtNumber ){
            return InfoEvent( _mass, _runNumber, _evtNumber);
        };
        //=============== Create the infoEvent out of which we filter M-candidates ( RANDOMLY )  ===============// 
        auto infos = myNode.Define("infoEvent", makeInfo , {_bmass.Data(), "runNumber", "eventNumber"} ).Take<InfoEvent>( "infoEvent");
        std::map< pair< UInt_t, ULong64_t > , vector< double> > _candidatesValues; 
        for( int i =0 ; i < infos->size(); ++i){
            _candidatesValues[ infos->at(i).ID() ].push_back( infos->at(i).mass() );
        }
        //=============== Fill Dataset entry by entry , with m-candidates randomly removed ===============// 
        _data = new RooDataSet("dataSet_" + _name, "dataSet_" + _name,RooArgSet(*var));        
        std::cout<< "RANGE CUTTED ON OBSERVABLES = "<< var->getMin() << " , "<< var->getMax() << std::endl;
        TRandom3 rnd; 
        int _nCandidatesPass= infos->size() ;
        int _nEvents =       _candidatesValues.size() ; 
        int _nEventsWithSingleCandidates=0;
        int _nEventsWithMultipleCandidates=0;
        int _nCandidatesAfterRemoval = 0; 
        int _nCandidatesRemoved = 0; 
        for( auto & el : _candidatesValues){
            auto IDEvent = el.first;         
            if( el.second.size() > 1 ){ 
                _nEventsWithMultipleCandidates++;
                _nCandidatesRemoved+= el.second.size() -1;
                rnd.SetSeed( IDEvent.first * IDEvent.second);
                int indexToPick = floor( rnd.Uniform( 0,el.second.size()  ) ) ;
                double picked = el.second.at( indexToPick);
                var->setVal(picked);
                _data->add(RooArgSet(*var) );
                _nCandidatesAfterRemoval++;
            }else{
                _nEventsWithSingleCandidates++;
                double picked = el.second.at(0);
                var->setVal(picked);
                _data->add(RooArgSet(*var) );
                _nCandidatesAfterRemoval++;
            }
        }
        //=============== Some printing ===============// 
        //Candidates pas/tot
        MessageSvc::Warning("MultipleCandidatesRemoval", TString::Format("nPass/nTot            = %i/%i", _nCandidatesPass, _totCandidates));
        //nEvents with all candidates 
        MessageSvc::Warning("MultipleCandidatesRemoval", TString::Format("nEvents               = %i", _nEvents));
        //nEvents having a single candidate
        MessageSvc::Warning("MultipleCandidatesRemoval", TString::Format("nSingle               = %i", _nEventsWithSingleCandidates));
        //nEvents having multiple candidates
        MessageSvc::Warning("MultipleCandidatesRemoval", TString::Format("nWithMult             = %i", _nEventsWithMultipleCandidates));
        //nCandidates after removing multiple candidates
        MessageSvc::Warning("MultipleCandidatesRemoval", TString::Format("nCandidates(kept/pass)= %i/%i", _nCandidatesAfterRemoval, _nCandidatesPass));
        //nCandidates removed
        MessageSvc::Warning("MultipleCandidatesRemoval", TString::Format("nCandidates(drop)  = %i", _nCandidatesRemoved));
        return _data; 
    }

    //=============== If one is not dealing with Data ===============// 
    if( _option.Contains("-RK") || _option.Contains("-RKst")){
        /*
            Used for the q2 smearing tweaking for refitting and making 
            dataset with JPs_M_smear_XXX
        */
        //=============== Passed flag with year ===============// 
        if( _option.Contains("-11") || _option.Contains("-12") ||
            _option.Contains("-15") || _option.Contains("-16") ||
            _option.Contains("-17") || _option.Contains("-18")){
                MessageSvc::Warning("Readding q2 smearing columns should happen");
        }                
    }
    if (_option.Contains("rescale")) _frac = -1;
    //=============== Trick for fitting leakage component or reweight the 2XJPs samples on the fly ===============// 
    /* HACK FOR LEAKAGE FITS : 
        Ntuples are from TupleProcess, the JPs_Smearing has to be "appended" and the B_MASS variable has been attached. 
        ONCE we reprocess and we attach both, we don't need anything about year-jump around and project loading
    */
    if(_option.Contains("leakage") || IsWeight(_weight) ||  _option.Contains("reweightXJPs") ){
        ROOT::DisableImplicitMT();
        MessageSvc::Warning("RooDataSet maker by hand done routine applied");
        /* HACK FOR LEAKAGE FITS : 
            Ntuples are from TupleProcess, the JPs_Smearing has to be "appended" and the B_MASS variable has been attached. 
            ONCE we reprocess and we attach both, we don't need anything about year-jump around and project loading
        */
        RooRealVar * var  = static_cast<RooRealVar*>(_varList.at(0));//this is the plotting variable ? 
        TString _theBMass = var->GetName();   
        TString _originalMass = _theBMass; 
        ROOT::RDataFrame df(*_tuple); 
        auto myNode = ROOT::RDF::RNode( df);       
        /* THIS IS AN HACK , CLEAN UP AND HAVE BRANCH ON TUPLE DIRECTLY*/
        if( _option.Contains("leakage")){
            auto _allopts = TokenizeString(_option, "-");
            TString _LeakageBit = "";
            for( auto & el : _allopts){
                if( el.Contains("leakage")){ _LeakageBit = el; break;}
            }
            _theBMass = StripStringBetween(_LeakageBit, "leakage[", "]");
            MessageSvc::Warning("BMassSmearedVar", _theBMass);
            var->Print("v");
            MessageSvc::Warning("Special DataSetMaking on Leakage Component");
            MessageSvc::Warning("Cut", TString(_cut.GetTitle()));
            MessageSvc::Warning("Weight", _weight);
            MessageSvc::Warning("BVAR", _theBMass);        
            Year _yearSmear = Year::Y2017; 
            if(_name.Contains("R2p2")) _yearSmear = Year::Y2018;
            if(_name.Contains("R2p1")) _yearSmear = Year::Y2016;
            if(_name.Contains("R1")) _yearSmear = Year::Y2012;
            if(_name.Contains("11")) _yearSmear = Year::Y2011;
            if(_name.Contains("12")) _yearSmear = Year::Y2012;
            if(_name.Contains("15")) _yearSmear = Year::Y2015;        
            if(_name.Contains("16")) _yearSmear = Year::Y2016;
            if(_name.Contains("17")) _yearSmear = Year::Y2017;
            if(_name.Contains("18")) _yearSmear = Year::Y2018;
            Prj _prj = Prj::RK;
            if( myNode.HasColumn("B0_BKGCAT")) _prj = Prj::RKst; 
            if( myNode.HasColumn("Bp_BKGCAT")) _prj = Prj::RK; 
            if( myNode.HasColumn("Bs_BKGCAT")) _prj = Prj::RPhi; 	
            MessageSvc::Warning("Node Filter Cut");
            std::cout<<RED<<"[BYHAND_DataSet] Setup Fill by hand (PRJ) with smearing setup... " << to_string(_prj) << "  [ YEAR = " << to_string(_yearSmear) << " ]" << RESET<< std::endl;
            std::cout<<RED<<"[BYHAND_DataSet] Setup Fill by hand (PRJ) with smearing setup... Expect B var \n \t " << _theBMass << RESET<< std::endl;
            //Has become agnostic to the year, the previous code is useless....
            myNode = HelperProcessing::AppendQ2SmearColumns( myNode, _prj,_yearSmear);
            myNode = HelperProcessing::AppendBSmearColumns( myNode, _prj,_yearSmear);
        }
        TString _selection(_cut.GetTitle());
        if( _option.Contains("leakage") && !_theBMass.BeginsWith("JPs") ){   
            std::cout<<RED<< "Replacing B invariant mass cut for ranges , before "         << _originalMass << " --> " <<  _theBMass <<RESET<< std::endl;
            _selection = _selection.ReplaceAll(_originalMass, _theBMass );
        }       
        TString _weightUse = _weight;
        MessageSvc::Info("GetRooDataSetSnapshot [cut applied]",    _name, _selection);
        std::cout<<RED<<"[BYHAND_DataSet] CUT    : "<< _selection.Data() << RESET<< std::endl;
        std::cout<<RED<<"[BYHAND_DataSet] WEIGHT : "<< _weightUse.Data()  << RESET<< std::endl;
        //=============== Append All aliases as new columns first ===============// 
	    vector<pair<TString,TString>> _aliasesAdd;
        if (_tuple->GetListOfAliases() != nullptr) {                                                                                                                                                       
          TList * _aliases = (TList *) _tuple->GetListOfAliases();                                                                                                                                         
          if (_aliases != nullptr) {                                                                                                                                                                       
            for (auto _alias : *_aliases){                                                                                                                                                                 
              _aliasesAdd.push_back(make_pair(_alias->GetName(), ((TString) _alias->GetTitle()).ReplaceAll("<Double_t>", "").ReplaceAll(")+0", ")")));                                                     
            }                                                                                                                                                                                              
          }                                                                                                                                                                                                
        }                                                                                                                                                                                                  
        for( auto & alias : _aliasesAdd){                                                                                                                                                                  
	        myNode = myNode.Define( alias.first.Data(),alias.second.Data());                                                                                                                          
        }
        //=============== Filter and select ===============// 
        myNode = myNode.Filter(_selection.Data());

        //=============== Attach weights (OnTheFlyFitter needed)===============// 
        if( IsWeight(_weight) && _weightOption!= "" && SettingDef::Efficiency::option.Contains("OnTheFlyFitter")){
            //Sample is weighted, WeightOption of Event Type calling the input, and OnTheFly for the fitting
            myNode = HelperProcessing::AttachWeights( myNode, _configHolder , _weightOption);
        }
        //=============== Attach weights 2XJPs sample ===============// 
        if( _option.Contains("reweightXJPs") && !_option.Contains("leakage") ) {
            _weightUse = "wDecFile";
            myNode = Reweight2XJPs( myNode,_weightUse);
        }
        //=============== Fill dataset entry by entry with weights/no weights ===============// 
        RooDataSet * _data = nullptr; 
        RooRealVar * _varW = new RooRealVar("weight", _weightUse, 1);
        bool _isWeighted = IsWeight(_weightUse);
        bool _addWeight = false;
        if (IsWeight(_weightUse)) {
            _varW->setConstant(0);
            _data = new RooDataSet("dataSet_" + _name, "dataSet_" + _name,RooArgSet(*var,*_varW), RooFit::WeightVar(*_varW));
            _addWeight = true;
        }else{
            auto args = RooArgSet(*var);
            _data = new RooDataSet("dataSet_" + _name, "dataSet_" + _name,RooArgSet(*var));
        }
        std::cout<< "RANGE CUTTED ON OBSERVABLES = "<< var->getMin() << " , "<< var->getMax() << std::endl;
        //=============== Fill dataset entry by entry ===============// 
        auto makeDataSet=[ _data , var, _varW, _addWeight]( std::pair< double, double> & myPair){
            double xObs = myPair.first;
            double weightVal = myPair.second;
            if( xObs< var->getMin() ||  xObs > var->getMax() ) return ;
            var->setVal( xObs);
            if(_addWeight){
                _varW->setVal(weightVal);
                _data->add(RooArgSet(*var, *_varW), _varW->getVal() );
            }else{
                _data->add(*var);
            }
        };
        MessageSvc::Info("Extract Vars for DataSet, Event Loop start");
        std::cout<<"[BYHAND_DataSet] Before (entries) = "<< std::endl;
        std::cout<<"[BYHAND_DataSet] BMassVar         = "<< _theBMass << std::endl;
        std::cout<<"[BYHAND_DataSet] Weight           = "<< _weightUse << std::endl;        
        _data->Print("v");
        myNode.Define("weight", _weightUse.Data())
              .Define("pair", [](double varVal, double _weightVal){ std::pair<double,double> v(varVal,_weightVal); return v;}, {_theBMass.Data(),"weight"})
              .Foreach(makeDataSet, {"pair"});
        MessageSvc::Info("Extract Vars for DataSet, Event Loop end");
        MessageSvc::Info("RooDataSet maker by hand done, print final dataset");
        _data->Print("v");
        return _data;    
    }else{
        MessageSvc::Warning("RooDataSet maker (baseline approach) applied");
    }    
    vector< RooRealVar * > _vars;
    
    MessageSvc::Info("Cut", &_cut);
    vector< TString >                  _branches = GetBranchesFromExpression(_tuple, TString(_cut));
    vector< pair< TString, TString > > _aliases  = GetAliasesFromExpression(_tuple, TString(_cut));

    if (_option.Contains("reduce")) {
        MessageSvc::Warning("MakeSnapshots", (TString) "Only add extraCut vars to ReduceComponent");
        _branches = GetBranchesFromExpression(_tuple, TString(SettingDef::Cut::extraCut));
        _aliases  = GetAliasesFromExpression(_tuple, TString(SettingDef::Cut::extraCut));
    }

    for (auto _var : _varList) {
        if (find(_branches.begin(), _branches.end(), _var->GetName()) == _branches.end()) _branches.push_back(_var->GetName());
    }

    MessageSvc::Info("Weight", _weight);
    if (IsWeight(_weight)) {
        vector< TString > _weightsB = GetBranchesFromExpression(_tuple, _weight);
        _branches.insert(_branches.end(), _weightsB.begin(), _weightsB.end());
        RemoveVectorDuplicates(_branches);
        for (auto _branch : _weightsB) {
            RooRealVar * _var = new RooRealVar(_branch, _branch, 1);
            _var->setConstant(0);
            if (!_varList.contains(*_var)) _varList.add(*_var);
            _vars.push_back(_var);
        }
        vector< pair< TString, TString > > _weightsA = GetAliasesFromExpression(_tuple, _weight);
        _aliases.insert(_aliases.end(), _weightsA.begin(), _weightsA.end());
        RemoveVectorDuplicates(_aliases);
        for (auto _alias : _weightsA) {
            RooRealVar * _var = new RooRealVar(_alias.first, _alias.first, 1);
            _var->setConstant(0);
            if (!_varList.contains(*_var)) _varList.add(*_var);
            _vars.push_back(_var);
        }
        RooRealVar * _var = new RooRealVar(_weight, _weight, 1);
        _var->setConstant(0);
        if (!_varList.contains(*_var)) _varList.add(*_var);
        _vars.push_back(_var);
    }
    if (_option.Contains("reduce")) {
        for (auto _branch : _branches) {
            RooRealVar * _var = new RooRealVar(_branch, _branch, 1);
            _var->setConstant(0);
            if (!_varList.contains(*_var)) _varList.add(*_var);
            _vars.push_back(_var);
        }
        for (auto _alias : _aliases) {
            RooRealVar * _var = new RooRealVar(_alias.first, _alias.first, 1);
            _var->setConstant(0);
            if (!_varList.contains(*_var)) _varList.add(*_var);
            _vars.push_back(_var);
        }
    }
    if (_option.Contains("splot")) {
        for (auto * _branch : *_tuple->GetListOfBranches()) {
            if (!((TString) _branch->GetTitle()).Contains("]/")) _branches.push_back(_branch->GetName());
        }
        RemoveVectorDuplicates(_branches);
    }
    MessageSvc::Info("VarList", to_string(_varList.getSize()));
    MessageSvc::Info("VarList", &_varList);
    if(SettingDef::Fit::option.Contains("SnpFitVarOnly")){
        MessageSvc::Warning("Cleaning up, keeping only fitting variable for snapshots!");
        _branches.clear();
        for (auto _var : _varList) {
            if (find(_branches.begin(), _branches.end(), _var->GetName()) == _branches.end()) _branches.push_back(_var->GetName());
        }        
    }

    map< TString, SplitInfo > _mapSplit = {{"Snapshot", SplitInfo("tmp" + _name + ".root", (TString) _tuple->GetName(), _cut, "Snapshot")}};
    map< TString, TTree * >   _mapTuple = GetSplitTuples(_tuple, _mapSplit, _frac, false, _branches, GetVectorFirst(_aliases), _option);

    if (_mapTuple["Snapshot"] == nullptr) MessageSvc::Error("GetRooDataSetSnapshot", (TString) "Tuple is nullptr", "EXIT_FAILURE");

    RooDataSet * _data = GetRooDataSet((TChain *) _mapTuple["Snapshot"], _name, _varList, TCut(NOCUT), _weight, _frac, _option);

    delete _mapTuple["Snapshot"];

    for (auto _var : _vars) { delete _var; }

    return _data;
}

RooDataSet * GetRooDataSetSnapshotBSData(TChain * _tuple, const TString & _name, RooArgList _varList, const TCut & _cut, const TString & _weight, double _frac, TString _option) {
    DisableMultiThreads();
    //Assumption of the method.... _varList contains only "fitting branch (1D), _weight is the RndPoisson[i] converted to double ? ". Return a dataset which has Only 1 variable filled, based on RndPoisson filler.
    std::cout<< "GetRooDataSetSnapshotBSData iBS       :"<<  SettingDef::Weight::iBS <<std::endl;
    std::cout<< "GetRooDataSetSnapshotBSData name      :"<< _name << std::endl;
    std::cout<< "GetRooDataSetSnapshotBSData cut       :"<< _cut << std::endl;
    std::cout<< "GetRooDataSetSnapshotBSData weight    :"<< _weight << std::endl;
    std::cout<< "GetRooDataSetSnapshotBSData VarList   :"<< std::endl;
    if( _tuple != nullptr){
        std::cout<< "GetRooDataSetSnapshotBSData entries   :"<< _tuple->GetEntries() << std::endl;
        std::cout<< "GetRooDataSetSnapshotBSData TupleName :"<< _tuple->GetName() << std::endl;
    }
    _varList.Print();
    if( _tuple!=nullptr){
        _tuple->SetBranchStatus("runNumber"  , 1);
        _tuple->SetBranchStatus("eventNumber", 1);
    }else{
        std::cout<< "GetRooDataSetSnapshotBSData Tuple NULLPTR :"<< std::endl;
    }
    RooRealVar * var = static_cast<RooRealVar*>(_varList.at(0));
    using  RNode = ROOT::RDF::RNode;
    TString _weightUse =_weight;
    if( !_weightUse.Contains("RndPoisson2") ){
      MessageSvc::Error("GetRooDataSetSnapshotBS", "to call only with Weight having RndPoisson");
    }
    if( _weightUse.Contains("RndPoisson2[-1]")){
        MessageSvc::Error("Invalid DataSet creation", "", "EXIT_FAILURE"); 
    }
    auto cache_file = _name+"_BS.root";    
    std::cout<< "GetRooDataSetSnapshotBSData WeightUse :"<< _weightUse << std::endl;    
	if(  SettingDef::Weight::iBS>=0 ){
	  std::cout<< "GetRooDataSetSnapshotBSData (DF from cache) Weight :"<< _weightUse << std::endl;  
	}    
    if( SettingDef::Weight::iBS == 0 ){
        MessageSvc::Warning("GetRooDataSetSnapshotBS iBS 0", (TString)Form("Filter( %s )"         , _cut.GetTitle()));
        MessageSvc::Warning("GetRooDataSetSnapshotBS iBS 0", (TString)Form("Define( weight , %s )", _weightUse.Data()) );
        MessageSvc::Warning("GetRooDataSetSnapshotBS iBS 0", (TString)Form("Define( var    , %s )", var->GetName()) );        
        if(IOSvc::ExistFile( cache_file))MessageSvc::Error("Already done",cache_file, "EXIT_FAILURE");
        DisableMultiThreads();
        vector<pair<TString,TString>> _aliasesAdd; 
        if (_tuple->GetListOfAliases() != nullptr) {
            TList * _aliases = (TList *) _tuple->GetListOfAliases();
            if (_aliases != nullptr) {
                for (auto _alias : *_aliases) { 
                    _aliasesAdd.push_back(make_pair(_alias->GetName(), ((TString) _alias->GetTitle()).ReplaceAll("<Double_t>", "").ReplaceAll(")+0", ")")));             
                }
            }
        }
        ROOT::Detail::RDF::ColumnNames_t columnsKEEP = {"runNumber", "eventNumber", var->GetName()};
        ROOT::RDataFrame df( *_tuple);
        if( df.HasColumn( "RndPoisson2") ){
            MessageSvc::Info("BS LOOP 0 : Dataframe has RndPoisson branch available, will remove and recompute");
        }
        TString _cached_weightBS = "RndPoisson2";
        if( !_weightUse.Contains("RndPoisson2") ){ 
            _cached_weightBS = "dummyStuff";
        }
        RNode last_node = df.Define( "dummyStuff", "1");
        for( auto & alias : _aliasesAdd){
            last_node = last_node.Define( alias.first.Data(),alias.second.Data());
        }
        last_node = last_node.Filter( _cut.GetTitle());
        if( !_weightUse.Contains("RndPoisson2")){
            MessageSvc::Warning("GetRooDataSetSnapshotBS !Contains");       
            last_node.Snapshot( _name.Data() , cache_file.Data(), { var->GetName(), _cached_weightBS.Data()} );
            MessageSvc::Info("DataSet Snapshotter (first loop)");
            *last_node.Count();
        }


        if( _weightUse.Contains("RndPoisson2")){
            MessageSvc::Info("DataSet Snapshotter (first loop) creatint tuple local cache for later BS fits");
            //Re-add RndPoisson Branch              
            ROOT::DisableImplicitMT();
            auto cache_file1 = TString("tmpBS_")+cache_file;
            MessageSvc::Info("DataSet Snapshotter (first loop) CACHE(rNb,eNb,variable)", cache_file1);
            last_node.Snapshot( _name.Data() , cache_file1.Data(), columnsKEEP );
            MessageSvc::Info("DataSet Snapshotter (first loop) [with runNumber,eventNumber] done");
            
            ROOT::RDataFrame dfFinal( _name.Data() , cache_file1.Data() );        
            MessageSvc::Info("DataSet Snapshotter Remaking Var + Readd(RndPoisson,{runNumber,eventNumber})");
            TRandom3 rnd;   
            //BE SURE THIS GO SINGLE THREADED , TRandom3 is not MT friendly.
            auto  addRndPoisson = [ &rnd]( const UInt_t & runNumber, const ULong64_t & eNumber ){
                rnd.SetSeed(runNumber * eNumber);
                vector<int> rndPoisson;
                rndPoisson.reserve(100);
                for( int i =0; i < 100; ++i){
                    rndPoisson.push_back((int)rnd.PoissonD(1));
                }
                return rndPoisson;                
            };
            dfFinal.Define("RndPoisson2",addRndPoisson, {"runNumber","eventNumber"})
                   .Snapshot( _name.Data() , cache_file.Data(), { var->GetName(), "RndPoisson2"} );            
        }else{
            MessageSvc::Warning("WeightUse doesn't contain RndPoisson");
        }
    }else if( ! IOSvc::ExistFile(cache_file)){
        MessageSvc::Error("File cached, not existing, but MUST exist", cache_file,"EXIT_FAILURE");
    }else{
        MessageSvc::Warning("GetRooDataSetSnapshotBS", (TString)Form("Filter( NOTHING ) [cached file loaded]"));
        MessageSvc::Warning("GetRooDataSetSnapshotBS", (TString)Form("Define( weight , %s )", _weightUse.Data()) );
        MessageSvc::Warning("GetRooDataSetSnapshotBS", (TString)Form("Define( var    , %s )", var->GetName()) );         
    }    
    DisableMultiThreads();    
    std::cout<< "GetRooDataSetSnapshotBSData (DF by hand) making :"<< std::endl;  
    std::cout<< "GetRooDataSetSnapshotBSData (DF by hand) Var    :"<<  var->GetName() << std::endl;  
    std::cout<< "GetRooDataSetSnapshotBSData (DF by hand) Weight :"<< _weightUse << std::endl;  

    //Hard-wire it...Tuple input can or cannot be used.
    ROOT::RDataFrame df( _name.Data(),cache_file.Data());

    auto dd = df.Define( "weight", _weightUse.Data())
                .Define( "var",     var->GetName());
    auto varColumn       = dd.Take<double>("var");
    auto BsWeightColumn  = dd.Take<int>("weight");
    MessageSvc::Info("Fill DataSet, Event Loop start");
    *df.Count();
    MessageSvc::Info("Fill DataSet, Event Loop stop");
    // RooDataSet * _data = GetRooDataSet((TChain *) _mapTuple["Snapshot"], _name, _varList, TCut(NOCUT), _weight, _frac, _option);
    RooDataSet * _data = new RooDataSet(_name, _name, RooArgSet( *var));
    for( int i =0 ; i < varColumn->size(); ++i){
        int poisWeight_forEntry = BsWeightColumn->at(i);
        for( int j = 0; j < poisWeight_forEntry; ++j){
            var->setVal( varColumn->at(i));
            _data->add( RooArgSet(*var));
        }
    }
    MessageSvc::Info("RooDataSet maker by hand done, print final dataset");
    _data->Print("v");
    return _data;
}

void MakeSnapshots(TChain * _tuple, const map< TString, SplitInfo > & _selections, double _frac, bool _hadd, vector< TString > _branchesToKeep, vector< TString > _aliasesToKeep, bool _report, TString _reportName, TString _option, ConfigHolder _configHolder) {
    MessageSvc::Info("MakeSnapshots Option",_option);
    bool ReaddSmearColumns = false;    
    auto _prj = Prj::All; 
    auto _year = Year::All;
    if( _option.Contains("-RKst"))   _prj = Prj::RKst;
    else if(_option.Contains("-RK")) _prj = Prj::RK;
    if(_option.Contains("-11")) _year = Year::Y2011;
    if(_option.Contains("-12")) _year = Year::Y2012;
    if(_option.Contains("-15")) _year = Year::Y2015;
    if(_option.Contains("-16")) _year = Year::Y2016;
    if(_option.Contains("-17")) _year = Year::Y2017;
    if(_option.Contains("-18")) _year = Year::Y2018;
    // if( _prj != Prj::All && _year != Year::All){
    //     ReaddSmearColumns = true;
    // }
    if( ReaddSmearColumns){
        MessageSvc::Info("MakeSnapshots define wMC smeared branches on the fly");
    }else{
        MessageSvc::Info("Readding smear columns disabled");
    }
    //TODO : hack for Smearing fits , recomment this out and fix logic when doing refits to data checks with Q2 smear columns added on the fly
    // ReaddSmearColumns = false; 


    if (_tuple->GetListOfFriends() == nullptr) EnableMultiThreads();

    // the snapshotting Options!
    // the RDataFrame snapshot return type
    using SnapRet_t = ROOT::RDF::RResultPtr< ROOT::RDF::RInterface< ROOT::Detail::RDF::RLoopManager > >;
    auto _start     = chrono::high_resolution_clock::now();

    if (_tuple->GetListOfBranches() == nullptr) MessageSvc::Warning("MakeSnapshots", (TString) "ListOfBranches is nullptr");
    
    TString _toPrint = fmt::format("Split {0} into {1} slice(s) keeping {2} branches {3} aliases (HADD = {4}) [option = {5}]", _tuple->GetName(), _selections.size(), _branchesToKeep.size() == 0 ? _tuple->GetListOfBranches()->GetEntries() : _branchesToKeep.size(), _aliasesToKeep.size(), _hadd, _option.Data());
    MessageSvc::Line();
    MessageSvc::Info("MakeSnapshots", _toPrint);

    ROOT::RDF::RSnapshotOptions _options;
    _options.fLazy = true;
    _options.fMode = to_string(OpenMode::RECREATE);

    if (_tuple == nullptr) MessageSvc::Error("MakeSnapshots", (TString) "Tuple is nullptr", "EXIT_FAILURE");

    Long64_t _entries = _tuple->GetEntries();
    MessageSvc::Info("Entries", to_string(_entries));

    MessageSvc::Warning("Frac", to_string(_frac));

    Long64_t _maxEntries = _entries;
    if ((_frac >= 0.f) && (_frac < 1.0f)) _maxEntries = (int) floor(_frac * _maxEntries);
    if (_frac >= 1) _maxEntries = (int) floor(_frac);
    if (_maxEntries < _entries) {
        MessageSvc::Info("Range", to_string(_maxEntries));
        DisableMultiThreads();
    } else {
        _maxEntries = _entries;
    }


    DisableMultiThreads();
    ROOT::DisableImplicitMT();
    ROOT::RDataFrame df(*_tuple);

    vector< SnapRet_t > rets;   // the collector of snapshots
    auto                logStart = [](ULong64_t e) {
        if (e == 0) MessageSvc::Warning("MakeSnapshots", (TString) "Event loop - PROCESSING");
        return true;
    };

    // ROOT::RDF::RNode latestDF = df.Define("SnapChecker", [] { return 42; });

    ROOT::RDF::RNode latestDF( df);
    latestDF = HelperProcessing::AttachWeights(latestDF, _configHolder, _option);
    // if( SettingDef::Efficiency::option.Contains("OnTheFly") && _configHolder.IsMC()) {
    //     auto this_analysis = hash_analysis(SettingDef::Config::ana);
    //     vector<string> _wildCards = {};
    //     MessageSvc::Line();
    //     MessageSvc::Warning("Appending PID on the fly for effiency computation");
    //     if( _option.Contains("PID")){                
    //         latestDF = HelperProcessing::AppendPIDColumns(latestDF, _configHolder ,_wildCards);
    //     }
    //     if( this_analysis == Analysis::EE && _option.Contains("TRK")){
    //         latestDF = HelperProcessing::AppendTRKColumns(latestDF, _configHolder);
    //     }
    //     MessageSvc::Warning("Appending PID on the fly for effiency computation");
    //     MessageSvc::Line();
    // }
    /*
    Dropping, let's use full stat directly. Frac not usable.
    ROOT::RDF::RNode firstDF = df; 
    if (_maxEntries < _entries) firstDF = firstDF.Range(_maxEntries);
    ROOT::RDF::RNode latestDF = df.Define("SnapChecker", [] { return 42; });
    */
    
    vector< TString > _fileNames;
    vector< TString > _treeNames;
    vector< TString > _expressions;
    vector< TString > _names;
    for (const auto & _selection : _selections) {
        _fileNames.push_back(_selection.second.FileName);
        _treeNames.push_back(_selection.second.TreeName);
        _expressions.push_back(_selection.second.Selection);
        _names.push_back(_selection.second.SelectionName);
    }
    // You want to be sure to do a snapshot for "DIFFERENT" selections, with "DIFFERENT" alias of the selection, On different TTreeNames, and Different TFiles...
    if (!(_names.end() == unique(_names.begin(), _names.end()))) { MessageSvc::Error("MakeSnapshots", (TString) "Booked SelectionNamees has duplicates", "EXIT_FAILURE"); }
    // if (!(_expressions.end() == unique(_expressions.begin(), _expressions.end()))) { MessageSvc::Error("MakeSnapshots", (TString) "Booked SelectionExpressions has duplicates", "EXIT_FAILURE"); }
    if (_hadd && !(_treeNames.end() == unique(_treeNames.begin(), _treeNames.end()))) { MessageSvc::Error("MakeSnapshots", (TString) "Booked TreeNames has duplicates", "EXIT_FAILURE"); }
    if (!(_fileNames.end() == unique(_fileNames.begin(), _fileNames.end()))) { MessageSvc::Error("MakeSnapshots", (TString) "Booked FileNames has duplicates", "EXIT_FAILURE"); }

    ROOT::Detail::RDF::ColumnNames_t _toKeep;
    //This flag gets in ONLY WHEN SPLOT IS EXECUTED, carefully reproduced in a second time for LPT only.
    bool _hasRndPoissonKEEP = false;
    auto _columnsOnFrame = latestDF.GetColumnNames();
    if( SettingDef::debug.Contains("FitLbMisID")){
        //Very special setup for misID fits and recomputation of mis-id branches (for RKst only)
        latestDF = EvalSwapsByHand( latestDF);
    }
    if( ReaddSmearColumns){
        MessageSvc::Warning("Recalculating q2 smear columns");
        latestDF = HelperProcessing::AppendQ2SmearColumns( latestDF, _prj, _year);
        latestDF = HelperProcessing::AppendBSmearColumns( latestDF, _prj, _year);
    }

    if (_branchesToKeep.size() != 0) {
        for (auto & _branch : _branchesToKeep) {
            MessageSvc::Info("Branch", _branch);
            bool _isInFrame = latestDF.HasColumn( _branch.Data() );
            if( !_isInFrame){
                MessageSvc::Warning("Branch", _branch, "not in TTree");
            }
            //TODO: remove this logic
            if( _branch.Contains("_DTF_") && _branch.Contains("_status") ){
                MessageSvc::Warning("SKIPPING For snapshot the branch [misaligned in production, TODO fix it]", _branch);
                continue;
            }
            if( _branch.Contains("B0_M0123_Subst23_Kpi2pK")) {
                MessageSvc::Warning("SKIPPING For snapshot the branch [misaligned in production, TODO fix it]]", _branch);
                continue; //this branch as well is problematic ...
            }
            if( _branch.Contains("B0_M23_Subst23_Kpi2piK")){
                MessageSvc::Warning("SKIPPING For snapshot the branch [misaligned in production, TODO fix it]", _branch);
                continue;  //this branch as well is problematic ...
            }
            if( _branch == "RndPoisson" && !_hasRndPoissonKEEP){
                MessageSvc::Warning("SKIPPING For snapshot the branch [misaligned in production, TODO fix it]", _branch);
                _hasRndPoissonKEEP = true;
                continue;//Skipping the RndPoisson to keep branch, re-added in sPlot mode
            }
            _toKeep.push_back(_branch.Data());
        }
    }

    if (_tuple->GetListOfAliases() != nullptr) {
        TList * _aliases = (TList *) _tuple->GetListOfAliases();
        if (_aliases != nullptr) {
            vector< pair< TString, TString > > _unpackedAliases;
            for (auto _alias : *_aliases) { _unpackedAliases.push_back(make_pair(_alias->GetName(), ((TString) _alias->GetTitle()).ReplaceAll("<Double_t>", "").ReplaceAll(")+0", ")"))); }
            for (auto & _alias : _unpackedAliases){ 
                latestDF = latestDF.Define(_alias.first.Data(), _alias.second.Data()); 
            }
            _unpackedAliases.clear();
        }
        if (_aliasesToKeep.size() != 0) {
            for (auto & _alias : _aliasesToKeep) {
                MessageSvc::Info("Alias", _alias);
                _toKeep.push_back(_alias.Data());
            }
        }
    }

    for (auto & _selection : _selections) {
        auto Selection = _selection.second.Selection;
        auto Name      = _selection.second.SelectionName;
        MessageSvc::Info("Selection", Name, Selection);
        latestDF = latestDF.Define(Name.Data(), Selection.Data());
    }


    MessageSvc::Line();
    MessageSvc::Info("MakeSnapshots, list of branches to keep size : ", TString( to_string(_toKeep.size())));

    //TODO : remove this logic blacklisting stuff fixing the ntuples!
    // a lambda that checks if String is in the blacklist
    auto is_blacklisted = [&](const std::string &s)  { 
        //Those branches must be blacklisted
        const std::vector<std::string> blacklist = {"B0_DTF_JPs_status", 
                                                    "B0_DTF_Psi_status", 
                                                    "RndPoisson",
                                                    "B0_DTF_status", 
                                                    "B0_M0123_Subst23_Kpi2pK",
                                                    "B0_M23_Subst23_Kpi2piK" };
        return std::find(blacklist.begin(), blacklist.end(), s) != blacklist.end(); 
    };
    auto DropColumns = [&](std::vector<std::string> &&good_cols){
        // removing elements from std::vectors is not pretty, see https://en.wikipedia.org/wiki/Erase%E2%80%93remove_idiom
        MessageSvc::Info("DropColums (before) = ", TString(to_string( good_cols.size() )));
        good_cols.erase(std::remove_if(good_cols.begin(), good_cols.end(), is_blacklisted), good_cols.end());
        MessageSvc::Info("DropColums (after)  = ", TString(to_string( good_cols.size() )));
        return good_cols;
    };
    auto to_vecstr = []( const ROOT::Detail::RDF::ColumnNames_t & cols ){
        vector<std::string> cols_out;
        for( auto & c : cols ){
            cols_out.push_back(c);
        }
        return cols_out;
    };
    for (auto & _selection : _selections) {
        auto    ss_filter = latestDF.Filter([](double wcut) { return wcut > 0; }, {_selection.second.SelectionName.Data()});
        TString _fileName = _selection.second.FileName;
        TString _treeName = _selection.second.TreeName;
        if (_toKeep.size() != 0) {
            MessageSvc::Warning("Snapshot specific branches kept");   
            if(_hasRndPoissonKEEP){
                _fileName = "./HACK_"+_fileName.ReplaceAll("./", ""); //TODO: clean this logic
            }
            rets.emplace_back(ss_filter.Snapshot(_treeName.Data(), _fileName.Data(), DropColumns( to_vecstr(_toKeep) ), _options));
        } else {
            MessageSvc::Warning("Snapshot .* branches BLACKLISTED incompatible setup of branches ( see Lambda function DropColumns in HelperSvc.cpp file )");            
            /*
                Comment from @Renato
                Seems to be NEVER used anyway , even for sPlots.
                Snapshot ALL branches in TTree here
                We have to drop some columns for sPlot to work 
                since 17MU and 17MD seems to have incompatible branch lengh set (v10). If we don't drop them, Snapshot will keep only the first TTree added to the list
            */
        }
    }

    MessageSvc::Warning("MakeSnapshots", (TString) "Event loop - START");
    auto _size = *latestDF.Count();
    MessageSvc::Warning("MakeSnapshots", (TString) "Event loop - STOP");

    if(_hasRndPoissonKEEP){
        MessageSvc::Warning("MakeSnapshots",(TString)"RndPoisson branch has been dropped from local snapshots, will remake it now");
        for (auto & _selection : _selections) {
            ROOT::DisableImplicitMT();
            TString _fileName = _selection.second.FileName;
            _fileName = "./HACK_"+_fileName.ReplaceAll("./", ""); //TODO: clean this logic
            TString _treeName = _selection.second.TreeName;
            TRandom3 rnd;   
            auto  addRndPoisson = [ &rnd]( const UInt_t & runNumber, const ULong64_t & eNumber ){
                rnd.SetSeed(runNumber * eNumber);
                vector<int> rndPoisson;
                rndPoisson.reserve(100);
                for( int i =0; i < 100; ++i){
                    rndPoisson.push_back((int)rnd.PoissonD(1));
                }
                return rndPoisson;
            };
            MessageSvc::Warning("MakeSnapshots",(TString)"RndPoisson branch has been dropped from local snapshots remaking it transparently");
            ROOT::RDataFrame dfNew(_treeName.Data(), _fileName.Data());
            RNode last_node = dfNew.Define("DUMMYSTUFFSNAP","1>0");
            if( _hasRndPoissonKEEP){
                last_node = last_node.Define("RndPoisson", addRndPoisson , {"runNumber","eventNumber"});
            }           
            last_node.Snapshot( _treeName.Data(), _fileName.ReplaceAll("HACK_","").Data() , ".*");
            MessageSvc::Warning("MakeSnapshots",(TString)"RndPoisson branch columns have been dropped from local snapshots remaking it transparently DONE");
        }
    }


    map< TString, pair<int,int>> _stats;
    for (auto & _selection : _selections) {
        TString _fileName = _selection.second.FileName;
        TString _treeName = _selection.second.TreeName;
        auto Selection = _selection.second.Selection;
        TFile f(_fileName);
        auto t = f.Get<TTree>(_treeName);
        if( t == nullptr){
            MessageSvc::Warning("Tree Name", _treeName);
            MessageSvc::Warning("File Name", _fileName);
            MessageSvc::Error("Snapshot has failed");
        }
        TString msg =  Form("%s::%s (Pas/Tot) %i/%i", _fileName.Data(),_treeName.Data(),(int)t->GetEntries(), (int)_entries);
        MessageSvc::Warning( "MakeSnapshots (selection)", Selection);
        MessageSvc::Warning( "MakeSnapshots (stats)", msg);
        _stats[ Form("%s::%s", _fileName.Data(),_treeName.Data())] =  make_pair((int)t->GetEntries(), (int)_entries);
        f.Close();
    }
    MessageSvc::Warning("MakeSnapshots", (TString) "Max Entries =", to_string(_maxEntries));

    auto _stop = chrono::high_resolution_clock::now();
    MessageSvc::Line();
    MessageSvc::Warning("MakeSnapshots", (TString) "took", to_string(chrono::duration_cast< chrono::seconds >(_stop - _start).count()), "seconds");
    MessageSvc::Line();

    if (_tuple->GetListOfFriends() == nullptr) DisableMultiThreads();

    TString _tpName = _tuple->GetName();
    if (_report || _tpName.Contains("LPT")) {
        ofstream myfile;           
        if( _tpName.Contains("LPT")){
            _reportName = SettingDef::IO::outDir+"/LPTSelections.log";
            //Does it go in append mode ? 
        }
        myfile.open(_reportName);
        myfile << "List of cuts applied for TTree created .\n";
        for (auto & _selection : _selections){
            auto    Selection = _selection.second.Selection;       // actual selection
            auto    Name      = _selection.second.SelectionName;   // aliased name on TTree
            TString _fileName = _selection.second.FileName;
            TString _treeName = _selection.second.TreeName;
            myfile << "FILETREE ("  << _fileName << "::"<< _treeName<<")\n" ;
            myfile << "Selection (Name): " << Name << "\n";
            myfile << "Selection (Expr): \n\t" << Selection << "\n";
            myfile << "Selection (Filt): " << Name << " > 0" <<"\n";
            TString ID = Form("%s::%s", _fileName.Data(),_treeName.Data());
            myfile << "Pas       (Filt): " << _stats[ID].first << "\n";
            myfile << "Tot       (Filt): " << _stats[ID].second << "\n";
        }
        myfile << "-----------------------------------------------------------------\n";
        myfile.close();
    }
    return;
}

map< TString, TTree * > GetSplitTuples(TChain * _tuple, const map< TString, SplitInfo > & _selections, double _frac, bool _hadd, vector< TString > _branchesToKeep, vector< TString > _aliasesToKeep, TString _option) {    
    MessageSvc::Info("GetSplitTuples nEntries source tuple", TString(to_string(_tuple->GetEntries())));
    MessageSvc::Info("GetSplitTuples Branches To Keep Size", TString(to_string(_branchesToKeep.size())));
    MessageSvc::Info("GetSplitTuples Aliases  To Keep Size", TString(to_string(_aliasesToKeep.size())));
    MessageSvc::Info("GetSplitTuples Option", _option);
    map< TString, TTree * > _returnTrees;
    MakeSnapshots(_tuple, _selections, _frac, _hadd, _branchesToKeep, _aliasesToKeep, false, "",  _option);
    for (auto & _selection : _selections) {
        TFile * _file = TFile::Open(_selection.second.FileName);
        if (_file == nullptr) {
            MessageSvc::Warning("GetSplitTuples", _selection.second.FileName, "is nullptr");
            continue;
        }
        TTree * _tree = (TTree *) _file->Get(_selection.second.TreeName);
        if (_tree == nullptr) {
            MessageSvc::Warning("GetSplitTuples", _selection.second.TreeName, "is nullptr");
            _file->ls();
            continue;
        }
        _returnTrees[_selection.first] = _tree;
    }
    return _returnTrees;
}

vector< pair< TString, TString > > GetParticleBranchNames(const Prj _project, const Analysis _analysis, const Q2Bin _q2bin, TString _option) {
    //"onlyLeptons, will return M1,M2 or E1,E2, "onlyhadrons"  returns "K, Pi" or K1,K2....or "K"]

    TString _lep1, _lep2;
    TString _lep1tex, _lep2tex;
    switch (_analysis) {
        case Analysis::All: break;
        case Analysis::MM:
            _lep1    = "M";
            _lep2    = "M";
            _lep1tex = Tex::Muon;
            _lep2tex = Tex::Muon;
            break;
        case Analysis::EE:
            _lep1    = "E";
            _lep2    = "E";
            _lep1tex = Tex::Electron;
            _lep2tex = Tex::Electron;
            break;
        case Analysis::ME:
            _lep1    = "M";
            _lep2    = "E";
            _lep1tex = Tex::Muon;
            _lep2tex = Tex::Electron;
            break;
        default: MessageSvc::Error("Wrong analysis", to_string(_analysis), "EXIT_FAILURE"); break;
    }
    if ((_project == Prj::RL) || (_project == Prj::RKS)) {
        _lep1 = "l1";
        _lep1 = "l2";
    }

    vector< pair< TString, TString > > _leptons = {{_lep1 + "1", _lep1tex + "_{1}"}, {_lep2 + "2", _lep2tex + "_{2}"}};

    vector< pair< TString, TString > > _hadrons;
    vector< pair< TString, TString > > _intermediate;
    vector< pair< TString, TString > > _head;
    switch (_project) {
        case Prj::RKst:
            _hadrons      = {{"K", Tex::Kaon}, {"Pi", Tex::Pion}};
            _intermediate = {{"Kst", Tex::Kstar0}};
            _head         = {{"B0", Tex::B0}};
            break;
        case Prj::RK:
            _hadrons      = {{"K", Tex::Kaon}};
            _intermediate = {};
            _head         = {{"Bp", Tex::Bp}};
            break;
        case Prj::RPhi:
            _hadrons      = {{"K1", Tex::Kaon + "_{1}"}, {"K2", Tex::Kaon + "_{2}"}};
            _intermediate = {{"Phi", Tex::Phi}};
            _head         = {{"Bs", Tex::Bs}};
            break;
        case Prj::RL:
            _hadrons      = {{"p", Tex::Proton}, {"pi", Tex::Pion}};
            _intermediate = {{"L0", Tex::Lambda0}};
            _head         = {{"Lb", Tex::Lambdab}};
            break;
        case Prj::RKS:
            _hadrons      = {{"pi1", Tex::Pion + "_{1}"}, {"pi2", Tex::Pion + "_{2}"}};
            _intermediate = {{"KS", Tex::KShort}};
            _head         = {{"B0", Tex::B0}};
            // TEMPORARY FIX
            _head    = {{"Lb", Tex::B0}};
            _hadrons = {{"p", Tex::Pion + "_{1}"}, {"pi", Tex::Pion + "_{2}"}};
            break;
        default: MessageSvc::Error("Wrong project", to_string(_project), "EXIT_FAILURE"); break;
    }

    if (_q2bin == Q2Bin::JPsi)
        _intermediate.push_back({"JPs", Tex::JPsi});
    else if (_q2bin == Q2Bin::Psi)
        _intermediate.push_back({"JPs", Tex::Psi});   // branch name is JPsi, tex is Psi
    else
        _intermediate.push_back({"JPs", ""});   // branch name is JPsi, no tex

    if (_option == "onlyleptons") return _leptons;
    if (_option == "onlyhadrons") return _hadrons;
    if (_option == "onlyintermediate") return _intermediate;
    if (_option == "onlyhead") return _head;

    return _leptons + _hadrons + _intermediate + _head;
}

vector< pair< string, string > > to_string_pairs(const vector< pair< TString, TString > > & _pairs) {
    vector< pair< string, string > > _pairs_out;
    for (const auto & el : _pairs) { _pairs_out.push_back(make_pair(el.first.Data(), el.second.Data())); }
    return _pairs_out;
};

ROOT::RDF::RNode ApplyDefines(ROOT::RDF::RNode df, const vector< pair< TString, TString > > & names_expressions, bool _forceDoubleCast, unsigned int i) {
    if (i == names_expressions.size()) return df;
    TString _expr = TString(names_expressions[i].second);
    if (_forceDoubleCast) { _expr = TString("(double)(") + _expr + ")"; }
    // MessageSvc::Debug("ApplyDefines", names_expressions[i].first, _expr);
    return ApplyDefines(df.Define(names_expressions[i].first.Data(), _expr.Data()), names_expressions, _forceDoubleCast, i + 1);
}

ROOT::RDF::RNode ApplyDefines(ROOT::RDF::RNode df, const vector< pair< string, string > > & names_expressions, bool _forceDoubleCast, unsigned int i) {
    if (i == names_expressions.size()) return df;
    TString _expr = TString(names_expressions[i].second);
    if (_forceDoubleCast) { _expr = TString("(double)(") + _expr + ")"; }
    // MessageSvc::Debug("ApplyDefines", names_expressions[i].first, _expr);
    return ApplyDefines(df.Define(names_expressions[i].first, _expr.Data()), names_expressions, _forceDoubleCast, i + 1);
}

void CheckPIDMapsForNullPtrs(pair<TH1D*, vector <TH2D*> > _map, TString _name, bool _vbs) {
    MessageSvc::Info(Color::Cyan, "CheckPIDMaps For NullPtrs", _name);
    
    TH1D* _nTracksHisto = _map.first;
    if (_nTracksHisto == nullptr) MessageSvc::Error("CheckPIDMapsForNullPtrs", "nTracks Map " + _name + " is nullptr", "EXIT_FAILLURE");
    else {
	MessageSvc::Info(Color::Cyan, "number of nTracks bins: " + to_string(_nTracksHisto->GetNbinsX()));
	if (_vbs) {_nTracksHisto->Print();}
    }
    for (auto const& _2DMap: _map.second) {
	if (_2DMap == nullptr) MessageSvc::Error("CheckPIDMapsForNullPtrs", "One 2D Map " + _name + " is nullptr", "EXIT_FAILLURE");
	if (_vbs) {_2DMap->Print();}
    }
}

ROOT::RDF::RNode EvalSwapsByHand( ROOT::RDF::RNode df , const Prj & _prj, const Analysis & _ana){
    /*
    If not RKst project, do not recompute any swaps
    */
    if( _prj != Prj::RKst){
        MessageSvc::Warning("EvalSwapsByHand", (TString)"Nothing to do on Prj != RKst");
        return df;
    }else{
        MessageSvc::Warning("EvalSwapsByHand", (TString)"Try adding B0_M0123_Subst23_FIX_Kpi2pK,B0_M0123_Subst3_FIX_pi2p columns to DataFrame");
    }

    /*
    Series of functors to construct lorentz vectors with different mass hypotesis
    */
    auto make_lorentz = [&]( double & px, double & py, double & pz, double & pe){
        //Raw lorentz maker
        TLorentzVector v;
        v.SetPxPyPzE( px,py,pz,pe);
        return v;
    };
    auto make_lorentz_proton = [&](double & px, double &py, double &pz){
        //Lorentz maker with proton mass 
        TLorentzVector v;
        v.SetXYZM( px, py, pz, PDG::Mass::P);
        return v;        
    };
    auto make_lorentz_kaon = [&]( double & px, double &py, double &pz){
        //Lorentz maker with kaon mass 
        TLorentzVector v;
        v.SetXYZM( px, py, pz, PDG::Mass::K);
        return v;        
    };
    auto make_lorentz_pion = [&]( double & px, double &py, double &pz){
        //Lorentz maker with pion mass 
        TLorentzVector v;
        v.SetXYZM( px, py, pz, PDG::Mass::Pi);
        return v;        
    };
    auto make_B_mass_swap = []( TLorentzVector & v1, TLorentzVector & v2, TLorentzVector & v3 , double & jps_m){
        //Combiner with 3 lorentz vectors and the reconstructed Jps mass to improve resolution
        auto v = v1+v2+v3;
        return v.Mag() - jps_m + PDG::Mass::JPs;
    };
    
    /*
    The core code creating 2 branches  (for the moment )
    B0_M0123_Subst23_FIX_Kpi2pK = mass of the B with a K->pion , pion->Proton to get the Lb2pKJpsMM/EE double Swap
    B0_M0123_Subst3_FIX_pi2p    = mass of the B with a K->K    , pion->Proton to get the Lb2pKJpsMM/EE single Swap
    */    
    if( !df.HasColumn("B0_M0123_Subst23_FIX_Kpi2pK") && !df.HasColumn("B0_M0123_Subst3_FIX_pi2p")){
        MessageSvc::Warning("EvalSwapsByHand", (TString)"Added B0_M0123_Subst23_FIX_Kpi2pK");
        MessageSvc::Warning("EvalSwapsByHand", (TString)"Added B0_M0123_Subst3_FIX_pi2p");
        df =   df.Define("K_Swap_P_LV"                , make_lorentz_proton, {"K_PX", "K_PY","K_PZ"})   //make a kaon with proton mass hypo 
                 .Define("Pi_Swap_K_LV"               , make_lorentz_kaon,   {"Pi_PX","Pi_PY","Pi_PZ"})  //make a pion with kaon mass hypo
                 .Define("Pi_Swap_P_LV"               , make_lorentz_proton, {"Pi_PX","Pi_PY","Pi_PZ"})  //make a pion with proton mass hypo
                 .Define("LL_LV"                      , make_lorentz,        {"JPs_PX","JPs_PY","JPs_PZ", "JPs_PE"})  //make the dilepton Lorentz Vector
                 .Define("K_LV"                       , make_lorentz,        {"K_PX", "K_PY","K_PZ", "K_PE"})
                 .Define("B0_M0123_Subst23_FIX_Kpi2pK",  make_B_mass_swap ,  {"K_Swap_P_LV", "Pi_Swap_K_LV" , "LL_LV", "JPs_M"})
                 .Define("B0_M0123_Subst3_FIX_pi2p"   ,  make_B_mass_swap ,  {"K_LV"      , "Pi_Swap_P_LV", "LL_LV", "JPs_M"})
                 .Define("B0_M0123_Subst3_FIX_pi2K"   ,  make_B_mass_swap ,  {"K_LV"      , "Pi_Swap_K_LV", "LL_LV", "JPs_M"});
    }
    return df;
}



ROOT::RDF::RNode Reweight2XJPs( ROOT::RDF::RNode myNode, TString _weightName ){
    MessageSvc::Warning("Reweight2XJPs, weightName", _weightName);
    Prj _prj = Prj::All; 
    if( myNode.HasColumn( "B0_BKGCAT")) _prj = Prj::RKst;
    if( myNode.HasColumn( "Bp_BKGCAT")) _prj = Prj::RK;
    if( myNode.HasColumn( "Bs_BKGCAT")) _prj = Prj::RPhi;
    if( _prj == Prj::All) MessageSvc::Error("Invalid smearing of 2XJPs samples", "","EXIT_FAILURE");    
    auto MyScaler = RescalerRXSamples(_prj);
    auto makeIDs = []( const int & ID, const int & MID , const int  & GMOTHERID , const int & GGMOTHERID){
        return IdChains(ID, MID, GMOTHERID, GGMOTHERID);
    };
    //Doable only on RKst!
    std::cout<<RED<< "Reweight2XJPs on the fly..."<< RESET<< std::endl;
    if( myNode.HasColumn("E1_TRUEID")){
        myNode = myNode.Define( "E1_IDs", makeIDs, { "E1_TRUEID", "E1_MC_MOTHER_ID", "E1_MC_GD_MOTHER_ID", "E1_MC_GD_GD_MOTHER_ID"} )
                    .Define( "E2_IDs", makeIDs, { "E2_TRUEID", "E2_MC_MOTHER_ID", "E2_MC_GD_MOTHER_ID", "E2_MC_GD_GD_MOTHER_ID"} );
        if( _prj == Prj::RKst){
            myNode = myNode.Define( "K_IDs", makeIDs,  { "K_TRUEID", "K_MC_MOTHER_ID"  , "K_MC_GD_MOTHER_ID" , "K_MC_GD_GD_MOTHER_ID"} )
                        .Define( "Pi_IDs", makeIDs, { "Pi_TRUEID", "Pi_MC_MOTHER_ID", "Pi_MC_GD_MOTHER_ID", "Pi_MC_GD_GD_MOTHER_ID"} )
                        .Define( _weightName.Data() , MyScaler, {"E1_IDs", "E2_IDs", "K_IDs", "Pi_IDs"});
        }
        if( _prj == Prj::RK){
            myNode = myNode.Define( "K_IDs", makeIDs,  { "K_TRUEID", "K_MC_MOTHER_ID"  , "K_MC_GD_MOTHER_ID" , "K_MC_GD_GD_MOTHER_ID"} )
                        .Define( "Pi_IDs", makeIDs, { "K_TRUEID", "K_MC_MOTHER_ID", "K_MC_GD_MOTHER_ID", "K_MC_GD_GD_MOTHER_ID"} )
                        .Define( _weightName.Data() , MyScaler, {"E1_IDs", "E2_IDs", "K_IDs", "Pi_IDs"});
        }
        if( _prj == Prj::RPhi){
            myNode = myNode.Define( "K1_IDs", makeIDs,  { "K1_TRUEID", "K1_MC_MOTHER_ID"  , "K1_MC_GD_MOTHER_ID" , "K1_MC_GD_GD_MOTHER_ID"} )
                        .Define( "K2_IDs", makeIDs, { "K2_TRUEID", "K2_MC_MOTHER_ID", "K2_MC_GD_MOTHER_ID", "K2_MC_GD_GD_MOTHER_ID"} )
                        .Define( _weightName.Data() , MyScaler, {"E1_IDs", "E2_IDs", "K1_IDs", "K2_IDs"});                
        }
        //Electron samples 
    }else if( myNode.HasColumn("M1_TRUEID")){
        myNode = myNode.Define( "M1_IDs", makeIDs, { "M1_TRUEID", "M1_MC_MOTHER_ID", "M1_MC_GD_MOTHER_ID", "M1_MC_GD_GD_MOTHER_ID"} )
                    .Define( "M2_IDs", makeIDs, { "M2_TRUEID", "M2_MC_MOTHER_ID", "M2_MC_GD_MOTHER_ID", "M2_MC_GD_GD_MOTHER_ID"} );
        if( _prj == Prj::RKst){
            myNode = myNode.Define( "K_IDs", makeIDs,  { "K_TRUEID", "K_MC_MOTHER_ID"  , "K_MC_GD_MOTHER_ID" , "K_MC_GD_GD_MOTHER_ID"} )
                        .Define( "Pi_IDs", makeIDs, { "Pi_TRUEID", "Pi_MC_MOTHER_ID", "Pi_MC_GD_MOTHER_ID", "Pi_MC_GD_GD_MOTHER_ID"} )
                        .Define( _weightName.Data() , MyScaler, {"M1_IDs", "M2_IDs", "K_IDs", "Pi_IDs"});
        }
        if( _prj == Prj::RK){
            myNode = myNode.Define( "K_IDs", makeIDs,  { "K_TRUEID", "K_MC_MOTHER_ID"  , "K_MC_GD_MOTHER_ID" , "K_MC_GD_GD_MOTHER_ID"} )
                        .Define( "Pi_IDs", makeIDs, { "K_TRUEID", "K_MC_MOTHER_ID", "K_MC_GD_MOTHER_ID", "K_MC_GD_GD_MOTHER_ID"} )
                        .Define( _weightName.Data() , MyScaler, {"M1_IDs", "M2_IDs", "K_IDs", "Pi_IDs"});
        }
        if( _prj == Prj::RPhi){
            myNode = myNode.Define( "K1_IDs", makeIDs,  { "K1_TRUEID", "K1_MC_MOTHER_ID"  , "K1_MC_GD_MOTHER_ID" , "K1_MC_GD_GD_MOTHER_ID"} )
                        .Define( "K2_IDs", makeIDs, { "K2_TRUEID", "K2_MC_MOTHER_ID", "K2_MC_GD_MOTHER_ID", "K2_MC_GD_GD_MOTHER_ID"} )
                        .Define( _weightName.Data() , MyScaler, {"M1_IDs", "M2_IDs", "K1_IDs", "K2_IDs"});                
        }//Muon samples
    }
    return myNode;
}

#endif
