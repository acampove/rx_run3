#ifndef EVENTTYPE_CPP
#define EVENTTYPE_CPP

#include "EventType.hpp"

#include "FitConfiguration.hpp"
#include "SettingDef.hpp"
#include "WeightDefRX.hpp"

#include "HistogramSvc.hpp"
#include "IOSvc.hpp"
#include "MessageSvc.hpp"
#include "ParserSvc.hpp"

#include <iostream>
#include <map>

#include "TString.h"
#include "TDirectory.h"
#include "TH1.h"
#include "TString.h"
#include "TTreeFormula.h"
#include "EfficiencyForFitHandler.hpp"

ClassImp(EventType)

    EventType::EventType()
    : ConfigHolder()
    , m_cutHolder()
    , m_weightHolder()
    , m_tupleHolder() {
    if (SettingDef::debug.Contains("ET")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("EventType", (TString) "Default");
}

EventType::EventType(
        const ConfigHolder &_conf, 
        const TString      &_cut_opt,
        const TString      &_wgt_opt,
        const TString      &_tup_opt) : ConfigHolder(_conf), 
    m_cutHolder   (_conf, _cut_opt), 
    m_weightHolder(_conf, _wgt_opt), 
    m_tupleHolder (_conf, _tup_opt) 
{
    MessageSvc::Debug("EventType", "Using map constructor");
}

EventType::EventType(TString _project, TString _ana, TString _sample, TString _q2bin, TString _year, TString _polarity, TString _trigger, TString _brem, TString _track, TString _cutOption, TString _weightOption, TString _tupleOption, bool _init)
    : ConfigHolder(
            hash_project(_project), 
            hash_analysis(_ana), 
            _sample, 
            hash_q2bin(_q2bin), 
            hash_year(_year), 
            hash_polarity(_polarity), 
            hash_trigger(_trigger), 
            hash_triggerconf(SettingDef::Config::triggerConf), 
            hash_brem(_brem), 
            hash_track(_track))
    , m_cutHolder(   *this, _cutOption)
    , m_weightHolder(*this, _weightOption)
    , m_tupleHolder( *this, _tupleOption) 
{
    MessageSvc::Debug("EventType", "Using string constructor");

    if (_init) 
        Init();
}

EventType::EventType(Prj _project, Analysis _ana, TString _sample, Q2Bin _q2bin, Year _year, Polarity _polarity, Trigger _trigger, Brem _brem, Track _track, TString _cutOption, TString _weightOption, TString _tupleOption, bool _init)
    : ConfigHolder(
            _project, 
            _ana, 
            _sample, 
            _q2bin, 
            _year, 
            _polarity, 
            _trigger, 
            hash_triggerconf(SettingDef::Config::triggerConf), 
            _brem, 
            _track)
    , m_cutHolder(*this, _cutOption)
    , m_weightHolder(*this, _weightOption)
    , m_tupleHolder(*this, _tupleOption) {

    if (SettingDef::debug.Contains("ET")) 
        SetDebug(true);

    if (m_debug) 
        MessageSvc::Debug("EventType", (TString) "Enumerator");

    if (_init) 
        Init();
}

EventType::EventType(const ConfigHolder & _configHolder, const CutHolder & _cutHolder, const WeightHolder & _weightHolder, const TupleHolder & _tupleHolder, bool _init)
    : ConfigHolder(_configHolder) {
    if (SettingDef::debug.Contains("ET")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("EventType", (TString) "Holder");
    m_cutHolder    = _cutHolder;
    m_weightHolder = _weightHolder;
    m_tupleHolder  = _tupleHolder;
    if (_configHolder != _cutHolder.GetConfigHolder()) MessageSvc::Error("EventType", (TString) "Different ConfigHolder in CutHolder", "EXIT_FAILURE");
    if (_configHolder != _weightHolder.GetConfigHolder()) MessageSvc::Error("EventType", (TString) "Different ConfigHolder in WeightHolder", "EXIT_FAILURE");
    if (_configHolder != _tupleHolder.GetConfigHolder()) MessageSvc::Error("EventType", (TString) "Different ConfigHolder in TupleHolder", "EXIT_FAILURE");
    if (_init) Init();
}

EventType::EventType(ZippedEventType & _zip, bool _init, bool _reset)
    : ConfigHolder(_zip.project, _zip.ana, _zip.sample, _zip.q2bin, _zip.year, _zip.polarity, _zip.trigger, _zip.triggerConf, _zip.brem, _zip.track)
    , m_cutHolder(*this, _zip.cutOption)
    , m_weightHolder(*this, _zip.weightOption)
    , m_tupleHolder(*this, _zip.tupleOption) {
    if (SettingDef::debug.Contains("ET")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("EventType", (TString) "ZippedEventType");

    m_tupleHolder.SetTupleName(_zip.tupleName);
    m_tupleHolder.SetFileName(_zip.fileName);

    if (_reset) {
        MessageSvc::Info("EventType", (TString) "Resetting ...");
        m_tupleHolder.SetFileName("");
    }
    if (_init) Init();
}

EventType::EventType(const EventType & _eventType)
    : ConfigHolder(static_cast< ConfigHolder >(_eventType))
    , m_cutHolder(_eventType.GetCutHolder())
    , m_weightHolder(_eventType.GetWeightHolder())
    , m_tupleHolder(_eventType.GetTupleHolder()) {
    if (SettingDef::debug.Contains("ET")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("EventType", (TString) "EventType");
    m_isInitialized = _eventType.IsInitialized();
    Init();
}

bool EventType::operator==(const EventType & _eventType) const { return (GetProject() == _eventType.GetProject() && GetAna() == _eventType.GetAna() && GetSample() == _eventType.GetSample() && GetQ2bin() == _eventType.GetQ2bin() && GetYear() == _eventType.GetYear() && GetPolarity() == _eventType.GetPolarity() && GetTrigger() == _eventType.GetTrigger() && GetTriggerConf() == _eventType.GetTriggerConf() && GetBrem() == _eventType.GetBrem() && GetTrack() == _eventType.GetTrack() && GetCutHolder() == _eventType.GetCutHolder() && GetWeightHolder() == _eventType.GetWeightHolder() && GetTupleHolder() == _eventType.GetTupleHolder()); }

bool EventType::operator!=(const EventType & _eventType) const { return !((*this) == _eventType); }

ostream & operator<<(ostream & os, const EventType & _eventType) {
    os << WHITE;
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "EventType");
    // MessageSvc::Line(os);
    os << static_cast< const ConfigHolder & >(_eventType);
    os << _eventType.GetCutHolder();
    os << _eventType.GetWeightHolder();
    os << _eventType.GetTupleHolder();
    os << WHITE;
    MessageSvc::Line(os);
    os << "\033[F";
    os << RESET;
    return os;
}

void EventType::Init(const bool &_force_initialization, const bool &_initialize_tuple_holder) 
{
    if (!_force_initialization && IsInitialized()) 
        return;

    if (m_cutHolder.IsOption("PIDMeerkat") && !m_weightHolder.IsOption("Meerkat")) 
        m_weightHolder.SetOption(m_weightHolder.Option() + SettingDef::separator + "Meerkat");

    if (m_tupleHolder.Option() == "") 
        return;

    MessageSvc::Line();
    MessageSvc::Info("Init", "EventType", "Initializing EventType"); 
    PrintInline();
    Check();

    if (SettingDef::Tuple::tupleName == "MCT") 
        m_weightHolder.SetOptionMCT(); 

    auto is_lumi_tree = (SettingDef::Tuple::tupleName != "LT") && !SettingDef::Tuple::tupleName.Contains("Lumi");
    if ( is_lumi_tree ) 
    {
        m_cutHolder.Init();
        m_weightHolder.Init();
    }

    if (_initialize_tuple_holder) 
        m_tupleHolder.Init(_force_initialization);

    m_isInitialized = true;
    MessageSvc::Line();
}

void EventType::Check(ConfigHolder & _configHolder, CutHolder & _cutHolder, WeightHolder & _weightHolder) {
    if (_weightHolder.IsOption("PID")) {    
        bool _VALIDCASE = _cutHolder.IsOption("no") || _cutHolder.IsOption("noPID") ;
        if( SettingDef::Weight::useMCRatioPID && _configHolder.GetAna() == Analysis::EE ){
            _VALIDCASE = _VALIDCASE && _cutHolder.IsOption("ePID");
        }
        // _VALIDCASE = _VALIDCASE || _cutHolder.IsOption("FORCE"); //TODO , remove 
        if ( !_VALIDCASE ) {
            MessageSvc::Error("EventType", (TString) "Cut    Option =", _cutHolder.Option());
            MessageSvc::Error("EventType", (TString) "Weight Option =", _weightHolder.Option());
            if (!SettingDef::Cut::force){
                if( !_cutHolder.IsOption("FORCE")){
                    MessageSvc::Error("EventType", (TString) "Incompatible Cut/Weight Holder PID options", "EXIT_FAILURE");
                }else{
                    MessageSvc::Warning("EventType", (TString) "Incompatible Cut/Weight Holder PID options", "EXIT_FAILURE");
                }                
            }else{
                MessageSvc::Warning("EventType", (TString) "Incompatible Cut/Weight Holder PID options", "FORCING CUT");
            }
        }
    }

    if (_weightHolder.IsOption("L0")) {
        // if (!(_cutHolder.IsOption("no") || _cutHolder.IsOption("noL0") || _cutHolder.IsOption("noTRG"))) {
        if (_cutHolder.IsOption("noL0") || _cutHolder.IsOption("noTRG")) {
            MessageSvc::Error("EventType", (TString) "Cut    Option =", _cutHolder.Option());
            MessageSvc::Error("EventType", (TString) "Weight Option =", _weightHolder.Option());
            if (!SettingDef::Cut::force)
                MessageSvc::Error("EventType", (TString) "Incompatible Cut/Weight Holder L0 options", "EXIT_FAILURE");
            else
                MessageSvc::Warning("EventType", (TString) "Incompatible Cut/Weight Holder L0 options", "FORCING CUT");
        }
    }

    if (_weightHolder.IsOption("HLT")) {        
        // if (!(_cutHolder.IsOption("no") || _cutHolder.IsOption("noHLT") || _cutHolder.IsOption("noTRG"))) {
        if (_cutHolder.IsOption("noHLT") || _cutHolder.IsOption("noTRG")) {
            MessageSvc::Error("EventType", (TString) "Cut    Option =", _cutHolder.Option());
            MessageSvc::Error("EventType", (TString) "Weight Option =", _weightHolder.Option());
            if( _cutHolder.IsOption("noHLT2") &&  _cutHolder.IsOption("noHLT1") ){
                MessageSvc::Error("EventType", (TString) "Incompatible Cut/Weight Holder HLT options (noHLT2 passed in the fitter to bypass the issue! CORRECT FOR CREVER WITH HLT CUTS APPLIED BEFOREHAND!)", "FORCING CUT (THIS IS A HACK, NOT A FIX)");
            }else{
                if (!SettingDef::Cut::force){
                    if( _cutHolder.IsOption("noHLT2") ){
                        MessageSvc::Error("EventType", (TString) "Incompatible Cut/Weight Holder HLT options (noHLT2 passed in the fitter to bypass the issue....)", "FORCING CUT (THIS IS A HACK, NOT A FIX)");
                    }else{
                        MessageSvc::Error("EventType", (TString) "Incompatible Cut/Weight Holder HLT options", "EXIT_FAILURE");
                    }
                }
                else
                    MessageSvc::Warning("EventType", (TString) "Incompatible Cut/Weight Holder HLT options", "FORCING CUT");
            }
        }
    }

    if ((_configHolder != _cutHolder.GetConfigHolder()) || (_configHolder != _weightHolder.GetConfigHolder())) {
        _configHolder.PrintInline();
        _cutHolder.PrintInline();
        _weightHolder.PrintInline();
        MessageSvc::Error("EventType", (TString) "EventType/Cut/Weight ConfigHolder not aligned", "EXIT_FAILURE");
    }
    if (_cutHolder.GetConfigHolder() != _weightHolder.GetConfigHolder()) { MessageSvc::Error("EventType", (TString) "Cut/Weight ConfigHolder not aligned", "EXIT_FAILURE"); }
    return;
}

void EventType::Check(ConfigHolder & _configHolder, CutHolder & _cutHolder, WeightHolder & _weightHolder, TupleHolder & _tupleHolder) {
    Check(_configHolder, _cutHolder, _weightHolder);
    if ((_configHolder != _cutHolder.GetConfigHolder()) || (_configHolder != _weightHolder.GetConfigHolder()) || (_configHolder != _tupleHolder.GetConfigHolder())) {
        _configHolder.PrintInline();
        _cutHolder.PrintInline();
        _weightHolder.PrintInline();
        _tupleHolder.PrintInline();
        MessageSvc::Error("EventType", (TString) "EventType/Cut/Weight/Tuple ConfigHolder not aligned", "EXIT_FAILURE");
    }
    if ((_cutHolder.GetConfigHolder() != _weightHolder.GetConfigHolder()) || (_cutHolder.GetConfigHolder() != _tupleHolder.GetConfigHolder()) || (_weightHolder.GetConfigHolder() != _tupleHolder.GetConfigHolder())) { MessageSvc::Error("EventType", (TString) "Cut/Weight/Tuple ConfigHolder not aligned", "EXIT_FAILURE"); }
    return;
}

void EventType::Check() {
    Check(static_cast< ConfigHolder & >(*this), m_cutHolder, m_weightHolder, m_tupleHolder);
    return;
}

void EventType::Close() {
    MessageSvc::Info(Color::Cyan, "EventType", (TString) "Close ...");
    m_tupleHolder.Close();
    m_isInitialized = false;
    return;
}

ZippedEventType EventType::Zip() {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "EventType", (TString) "Zip");
    ZippedEventType _zip = {m_project, m_ana, m_sample, m_q2bin, m_year, m_polarity, m_trigger, m_triggerConf, m_brem, m_track, m_cutHolder.Option(), m_weightHolder.Option(), m_tupleHolder.Option(), m_tupleHolder.TupleName(), m_tupleHolder.FileName()};
    return _zip;
}

TString EventType::GetLatex() {
    TString _latex = "";
    if (m_sample.Contains("LPT"))
        _latex = "Data : ";
    else
        _latex = "MC   : ";

    if (m_sample.Contains("LPT")) {
        switch (m_project) {
            case Prj::RKst: _latex += Tex::KPlus + Tex::PiMinus; break;
            case Prj::RK: _latex += Tex::KPlus; break;
            case Prj::RPhi: _latex += Tex::KPlus + Tex::KMinus; break;
            case Prj::RL: _latex += Tex::Proton + Tex::PiMinus; break;
            case Prj::RKS: _latex += Tex::PiPlus + Tex::PiMinus; break;
            default: MessageSvc::Error("GetLatex", (TString) "Invalid prj", to_string(m_project), "EXIT_FAILURE"); break;
        }
        _latex += to_tex(m_ana);
        // append EE/MM string and [q2Bin]
    } else {
        // All MC
        if (SettingDef::AllowedConf::TexSamples.find(m_sample) == SettingDef::AllowedConf::TexSamples.end()) {
            MessageSvc::Warning("GetLatex", (TString) "Sample", m_sample, "not supported");
            _latex += "NOT SUPPORTED";
            return _latex;
        }

        _latex += (*SettingDef::AllowedConf::TexSamples.find(m_sample)).second;
    }

    if ((m_q2bin != Q2Bin::All) || (m_trigger != Trigger::All) || (m_year != Year::All) || (m_polarity != Polarity::All) || (m_brem != Brem::All) || (m_track != Track::All)) {
        _latex += " #scale[0.5]{[}";
        _latex += to_tex(m_q2bin);
        _latex += to_tex(m_trigger);
        _latex += to_tex(m_year);
        _latex += to_tex(m_polarity);
        _latex += to_tex(m_brem);
        _latex += to_tex(m_track);
        _latex += "#scale[0.5]{]}";
    }

    return _latex;
}

TCut EventType::GetCut(const TString _extraCut) const {
    TCut _cut = m_cutHolder.Cut();
    if (IsCut(TCut(_extraCut))) _cut = _cut && TCut(_extraCut);
    return _cut;
}

TCut EventType::GetWCut(const TString _extraCut) const {
    TString _cut = TString(GetCut(_extraCut));
    if (IsWeighted()) {
        if (IsCutted())
            _cut = "(" + GetWeight() + ") * (" + _cut + ")";
        else
            _cut = "(" + GetWeight() + ")";
    }
    return TCut(_cut);
}

const Long64_t EventType::GetTupleEntries(TCut _cut){
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "EventType", (TString) "GetTupleEntries(TCut) [tpl]", GetTuple()->GetName());
    MessageSvc::Info(Color::Cyan, "EventType", (TString) "GetTupleEntries(TCut) [cut]", _cut.GetTitle());

    if (_cut == "TOT") {
        _cut = TCut(NOCUT);
    } else {
        if (_cut != GetWCut()) _cut = GetWCut(TString(_cut));
    }
    MessageSvc::Info(Color::Cyan, "EventType", (TString) "GetTupleEntries(TCut) [cut[update]]", _cut.GetTitle());
    if( SettingDef::Weight::useBS ){
        auto base = SettingDef::Tuple::addTuple;
        SettingDef::Tuple::addTuple = true;
        m_tupleHolder.Init(true);
        SettingDef::Tuple::addTuple = base;
    }
    vector< TString > _branchesTmp = m_tupleHolder.Branches();
    vector< TString > _branches    = GetBranchesAndAliasesFromExpression(GetTuple(), TString(_cut));
    m_tupleHolder.SetBranches(_branches);
    
    if( SettingDef::Weight::useBS ){
        _branches.push_back("RndPoisson2");
        m_tupleHolder.SetBranches(_branchesTmp + _branches);
    }

    Long64_t _entries = 0;
    if (_cut == TCut(NOCUT)) {
        MessageSvc::Info("GetEntriesFast");
        _entries = m_tupleHolder.TupleEntries(_cut);
    } else if (IsWeighted()) {
        MessageSvc::Info("GetEntries");
        _entries = round(GetEntries(*m_tupleHolder.GetTuple(), _cut));
    } else {
        MessageSvc::Info("GetTupleEntries", &_cut);
        if (SettingDef::Tuple::dataFrame) {
            MessageSvc::Info("GetEntriesDF");
            _entries = GetEntriesDF(*m_tupleHolder.GetTuple(), _cut);
        } else {
            MessageSvc::Info("TupleEntries");
            _entries = m_tupleHolder.TupleEntries(_cut);
        }
    }

    m_tupleHolder.SetBranches(_branchesTmp);

    MessageSvc::Info("GetTupleEntries", to_string(_entries));
    MessageSvc::Line();
    return _entries;
}

const Long64_t EventType::GetTupleEntries(TString _cutOption, TString _weightOption, TString _tupleOption, TCut _cut) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "EventType", (TString) "GetTupleEntries(cOpt,wOpt,tOpt)", _cutOption, _weightOption, _tupleOption);
    MessageSvc::Info(Color::Cyan, "EventType", (TString) "GetTupleEntries(cut)           ", _cut.GetTitle());

    ZippedEventType _zip = Zip();
    _zip.cutOption       = _cutOption;
    _zip.weightOption    = _weightOption;
    _zip.tupleOption     = _tupleOption;
    _zip.tupleName       = "DT";

    bool _trowLogicError       = SettingDef::trowLogicError;
    SettingDef::trowLogicError = true;
    bool _useEOS               = SettingDef::IO::useEOS;
    SettingDef::IO::useEOS     = true;

    Long64_t _entries = 0;
    try {
        EventType _et = EventType(_zip);
        cout << _et << endl;

        _cut = _et.GetWCut(TString(_cut));

        vector< TString > _branches = GetBranchesAndAliasesFromExpression(_et.GetTupleHolder().GetTuple(), TString(_cut));
        _branches.push_back("eventNumber");
        _et.GetTupleHolder().SetBranches(_branches);

        _entries = _et.GetTupleHolder().TupleEntries(_cut);
        _et.Close();

    } catch (const exception & e) { cout << e.what() << endl; }

    MessageSvc::Info("GetTupleEntries", to_string(_entries));
    MessageSvc::Line();

    SettingDef::trowLogicError = _trowLogicError;
    SettingDef::IO::useEOS     = _useEOS;

    return _entries;
}

void EventType::ReduceTuple(double _frac) {
    GetTupleReader().CopyTuple(GetWCut(), "", _frac, true);
    m_tupleHolder.SetAliases();
    return;
}

void EventType::ScanTuple(TString _option) {
    if (SettingDef::Tuple::branchList.size() != 0) {
        map< TString, TString > _names = GetParticleNames();
        TString                 _scan  = "";
        for (auto _branch : SettingDef::Tuple::branchList) {
            _branch = ReplaceWildcards(_branch, _names);
            _scan += _branch + ":";
        }
        _scan.Remove(_scan.Length() - 1);
        MessageSvc::Line();
        MessageSvc::Info(Color::Cyan, "ScanTuple", _option);
        MessageSvc::Line();
        GetTuple()->Scan(_scan, GetCut(), _option);
        MessageSvc::Line();
    }
    return;
}

void EventType::SetBranches(vector< TString > _branches) {
    vector< TString > _branchesTmp = m_tupleHolder.Branches();
    if (_branchesTmp.size() != 0) _branches.insert(_branches.end(), _branchesTmp.begin(), _branchesTmp.end());
    if (_branches.size() != 0) {
        m_tupleHolder.SetBranches(_branches);
        if (m_debug) {
            for (auto _branch : _branches) { MessageSvc::Debug("SetBranches ON", _branch); }
        }
    }
    return;
}

vector< TString > EventType::GetBranches(TCut _extraCut) {
    TCut _cut = GetWCut();
    if (IsCut(_extraCut)) _cut = _cut && _extraCut;
    return GetBranchesAndAliasesFromExpression(GetTuple(), TString(_cut));
}

TH1D * EventType::GetHisto(const RooRealVar & _varX, TCut _extraCut, const TString & _extraName) {
    TCut _cut = GetWCut();
    if (IsCut(_extraCut)) _cut = _cut && _extraCut;
    vector< TString > _branches = GetBranchesAndAliasesFromExpression(GetTuple(), TString(_cut));
    if (!CheckVectorContains(_branches, (TString) _varX.GetName())) _branches.push_back(_varX.GetName());
    RemoveVectorDuplicates(_branches);
    m_tupleHolder.SetBranches(_branches);
    TH1D * _histo = static_cast< TH1D * >(GetHistogram(*GetTuple(), _cut, _extraName, _varX));
    m_tupleHolder.ResetBranches();
    return _histo;
}

TH2D * EventType::GetHisto(const RooRealVar & _varX, const RooRealVar & _varY, TCut _extraCut, const TString & _extraName) {
    TCut _cut = GetWCut();
    if (IsCut(_extraCut)) _cut = _cut && _extraCut;
    vector< TString > _branches = GetBranchesAndAliasesFromExpression(GetTuple(), TString(_cut));
    if (!CheckVectorContains(_branches, (TString) _varX.GetName())) _branches.push_back(_varX.GetName());
    if (!CheckVectorContains(_branches, (TString) _varY.GetName())) _branches.push_back(_varY.GetName());
    RemoveVectorDuplicates(_branches);
    m_tupleHolder.SetBranches(_branches);
    TH2D * _histo = static_cast< TH2D * >(GetHistogram(*GetTuple(), _cut, _extraName, _varX, _varY));
    m_tupleHolder.ResetBranches();
    return _histo;
}

RooDataSet * EventType::GetDataSet(TString _name, RooArgList _varList, TCut _extraCut, double _frac, TString _option) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "EventType", (TString) "GetDataSet", _name, _option);

    TCut _cut = GetCut();
    if (IsCut(_extraCut)) _cut = _cut && _extraCut;
    MessageSvc::Info("Cut", &_cut);
    TString _weight = GetWeight();
    MessageSvc::Info("Weight", _weight);

    if( (_option.Contains("leakage") || IsWeight(_weight) || _option.Contains("reweightXJPs"))  && ( SettingDef::Weight::iBS <0 && m_sample != "LPT")    ){
        RooDataSet * _data = nullptr;
        _data = GetRooDataSetSnapshot(GetTuple(), _name, static_cast<  ConfigHolder &>(*this), GetWeightHolder().Option() , _varList, _cut, _weight, _frac, _option );             
        return _data;
    }
    //---------------------------------------------------------------------------------------------------------------------
    //Shortcut the loading of RooDataSets for BS loop >=1 ( cached dataset locally present, no EventType::tuple available )
    //---------------------------------------------------------------------------------------------------------------------
    if( SettingDef::Weight::useBS == true && SettingDef::Weight::iBS >0){
        /*
            Bootstrapping of fits are performed in iterative loops
            The next-to-first loop doesn't need any input tuple to be used, since locally the small ntuple with only
            VarToFit + RndPoisson [vector] column are available (see code in few lines down for iBS==0)
            So the fitter just need to load and make a DataSet with the RndPoisson[i>0] as weight
            Weight is forced, for some reason when reloading fits SavedToDisk and we set Weight::option = "BS", the weight is not created here, 
            HACK is to force this to happen.             
        */
        MessageSvc::Warning("EventType::GetDataSet()", TString("Bootstrapping DataSet re-loader for BS-loop >=1 (LPT only expected)"));
	    _weight = TString::Format("RndPoisson2[%i]",SettingDef::Weight::iBS);	
        MessageSvc::Warning("EventType::GetDataSet()", TString::Format("Regardless of WeightOption, Will use %s weight", _weight.Data()));
        RooDataSet * _data = GetRooDataSetSnapshotBSData(nullptr, _name, _varList, _cut, _weight, _frac, _option);
        MessageSvc::Line();
        return _data;
    }

    vector< TString > _branches = GetBranchesAndAliasesFromExpression(GetTuple(), TString(_cut));

    if (IsWeight(GetWeight())) {
        vector< TString > _weights = GetBranchesAndAliasesFromExpression(GetTuple(), _weight);
        _branches.insert(_branches.end(), _weights.begin(), _weights.end());
    }

    MessageSvc::Info("VarList", to_string(_varList.getSize()));
    MessageSvc::Info("VarList", &_varList);
    for (int i = 0; i < _varList.getSize(); ++i) {
        if (!CheckVectorContains(_branches, (TString) _varList.at(i)->GetName())) _branches.push_back(_varList.at(i)->GetName());
    }

    RemoveVectorDuplicates(_branches);

    if (_option.Contains("splot") && !IsMC()) {
        MessageSvc::Info("GetDataSet", (TString) "SetBranches", "{}", "true", "all");
        GetTupleReader().SetBranches({}, true, "all");
    } else {
        _option.ReplaceAll("-splot", "").ReplaceAll("splot", "");
        m_tupleHolder.SetBranches(_branches);
    }
    
    RooDataSet * _data;
    if (SettingDef::Tuple::dataFrame) {
        m_tupleHolder.CheckBranches( _branches, true);        
        if( SettingDef::Weight::useBS == true && SettingDef::Weight::iBS ==0) {            	        
            /*
                If you are using the bootstrapping flag enabled when fitting AND first loop, the ntuple Snapshot is produced. 
	            Reloaded in next loops from function on top passing a nullptr (the code loads it)
                Bootstrapping of fits are performed in iterative loops
                The first loop need an input tuple to be used, and it caches locally a new tuple with only
                VarToFit + RndPoisson [vector] column are available (see code in few lines up for iBS>0, reused in a next iteration)
                Weight is forced, for some reason when reloading fits SavedToDisk and we set Weight::option = "BS", the weight is not created here, 
                HACK is to force this to happen. 
            */            
        	_weight = "RndPoisson2[0]";
            MessageSvc::Warning("EventType::GetDataSet()", TString("Bootstrapping DataSet first BS-loop==0 (LPT only expected)"));
            MessageSvc::Warning("EventType::GetDataSet()", TString::Format("Regardless of WeightOption, Will use %s weight", _weight.Data()));
	        _data = GetRooDataSetSnapshotBSData(GetTuple(), _name, _varList, _cut, _weight, _frac, _option);
        }else{
            if (IsWeighted()){
                //Weighted fits with BS off
                SetAliases();
                _weight = "w" + WeightDefRX::ID::FULL;
            }
            //If EE and SignalMC handled, we compute the smearing in the cut adding a piece to the option being
            //-RKst-11 --> use Smearing for RKst 11 
            if( GetAna() == Analysis::EE && IsSignalMC() ){
                _option +="-"+to_string( GetProject() )+"-"+to_string(GetYear());
            }
            _data = GetRooDataSetSnapshot(GetTuple(), _name, static_cast<ConfigHolder &>(*this), GetWeightHolder().Option(),  _varList, _cut, _weight, _frac, _option );
        }
    } else {
        //BS Loop not supported here.
        if (_option.Contains("splot") && !IsMC())
            _data = GetRooDataSetCopy(GetTupleReader(), _name, _varList, _cut, _weight, _frac, _option);
        else
            _data = GetRooDataSet(GetTuple(), _name, _varList, _cut, _weight, _frac, _option);
    }

    m_tupleHolder.ResetBranches();

    MessageSvc::Line();
    return _data;
}

void EventType::SetAliases() {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "EventType", (TString) "SetAliases");

    Str2CutMap _cuts = GetCutHolder().Cuts();
    MessageSvc::Info("EventType", (TString) "Cuts", to_string(_cuts.size()));

    Str2WeightMap _weights = GetWeightHolder().Weights();
    MessageSvc::Info("EventType", (TString) "Weights", to_string(_weights.size()));

    vector< pair< TString, TString > > _aliases;
    for (auto _cut : _cuts) {
        if (_cut.first.BeginsWith("cut"))
            _aliases.push_back(make_pair(_cut.first, TString(_cut.second)));
        else
            _aliases.push_back(make_pair("cut" + _cut.first, TString(_cut.second)));
    }
    for (auto _weight : _weights) {
        if (_weight.first.BeginsWith("w"))
            _aliases.push_back(make_pair(_weight.first, TString(_weight.second)));
        else
            _aliases.push_back(make_pair("w" + _weight.first, TString(_weight.second)));
    }

    GetTupleHolder().SetAliases(_aliases);

    if (GetTuple()->GetListOfAliases() != nullptr) {
        MessageSvc::Line();
        GetTuple()->GetListOfAliases()->Print();
        MessageSvc::Line();
    }
    return;
}

void EventType::SaveToDisk(TString _name, bool _verbose) {
    // SaveToLog(_name);

    if (!SettingDef::IO::outDir.EndsWith("/")) SettingDef::IO::outDir += "/";
    if (_name != "") _name = "_" + _name;
    TString _oname = "EventType" + _name;
    _name          = SettingDef::IO::outDir + "EventType" + _name + ".root";

    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "EventType", (TString) "SaveToDisk", _name);

    TFile _tFile(_name, to_string(OpenMode::RECREATE));
    (*this).Write(_oname, TObject::kOverwrite);
    if (_verbose) SaveToNamed(&_tFile, _oname);
    _tFile.Close();
    MessageSvc::Line();
    // cout << WHITE << *this << RESET << endl;
    return;
}

void EventType::SaveToNamed(TFile * _tFile, TString _name) {
    MessageSvc::Info(Color::Cyan, "EventType", (TString) "SaveToNamed", _name);
    TDirectory * _folder = _tFile->mkdir(_name + "_Info");
    _folder->cd();
    (*this).GetNamedProject().Write();
    (*this).GetNamedAna().Write();
    (*this).GetNamedSample().Write();
    (*this).GetNamedQ2bin().Write();
    (*this).GetNamedYear().Write();
    (*this).GetNamedPolarity().Write();
    (*this).GetNamedTrigger().Write();
    (*this).GetNamedTriggerConf().Write();
    (*this).GetNamedBrem().Write();
    (*this).GetNamedTrack().Write();
    (*this).GetNamedCutOption().Write();
    (*this).GetNamedWeightOption().Write();
    (*this).GetNamedTupleOption().Write();
    (*this).GetCut().Write("Cut");
    (*this).GetWCut().Write("WCut");
    (*this).GetNamedWeight().Write();
    _tFile->cd();
    return;
}

void EventType::LoadFromDisk(TString _name, TString _dir) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "EventType", (TString) "LoadFromDisk", _name, _dir);
    MessageSvc::Line();

    if (_name != "") _name = "_" + _name;
    _name = "EventType" + _name;

    if ((_dir != "") && (!_dir.EndsWith("/"))) _dir += "/";

    if (!IOSvc::ExistFile(_dir + _name + ".root")) MessageSvc::Error("EventType", _dir + _name + ".root", "does not exist", "EXIT_FAILURE");

    TFile _tFile(_dir + _name + ".root", "read");
    _tFile.ls();

    EventType * _et = (EventType *) _tFile.Get(_name);
    *this           = *_et;

    _tFile.Close();
    MessageSvc::Line();
    // cout << WHITE << *this << RESET << endl;
    return;
}

void EventType::LoadFromNamed(TString _name, TString _dir) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "EventType", (TString) "LoadFromNamed", _name, _dir);
    MessageSvc::Line();

    if (_name != "") _name = "_" + _name;
    _name = "EventType" + _name;

    if ((_dir != "") && (!_dir.EndsWith("/"))) _dir += "/";

    if (!IOSvc::ExistFile(_dir + _name + ".root")) MessageSvc::Error("EventType", _dir + _name + ".root", "does not exist", "EXIT_FAILURE");

    TFile _tFile(_dir + _name + ".root", "read");
    _tFile.cd(_name + "_Info");
    _tFile.ls();

    SettingDef::Config::project     = _tFile.Get(_name + "_Info/Project")->GetTitle();
    SettingDef::Config::ana         = _tFile.Get(_name + "_Info/Analysis")->GetTitle();
    SettingDef::Config::sample      = _tFile.Get(_name + "_Info/Sample")->GetTitle();
    SettingDef::Config::q2bin       = _tFile.Get(_name + "_Info/Q2bin")->GetTitle();
    SettingDef::Config::year        = _tFile.Get(_name + "_Info/Year")->GetTitle();
    SettingDef::Config::polarity    = _tFile.Get(_name + "_Info/Polarity")->GetTitle();
    SettingDef::Config::trigger     = _tFile.Get(_name + "_Info/Trigger")->GetTitle();
    SettingDef::Config::triggerConf = _tFile.Get(_name + "_Info/TriggerConf")->GetTitle();
    SettingDef::Config::brem        = _tFile.Get(_name + "_Info/Brem")->GetTitle();
    SettingDef::Config::track       = _tFile.Get(_name + "_Info/Track")->GetTitle();
    SettingDef::Cut::option         = _tFile.Get(_name + "_Info/CutOption")->GetTitle();
    SettingDef::Weight::option      = _tFile.Get(_name + "_Info/WeightOption")->GetTitle();
    SettingDef::Tuple::option       = _tFile.Get(_name + "_Info/TupleOption")->GetTitle();

    EventType * _et = new EventType();
    *this           = *_et;

    _tFile.Close();
    MessageSvc::Line();
    // cout << WHITE << *this << RESET << endl;
    return;
}

void EventType::SaveToLog(TString _name) {
    if (!SettingDef::IO::outDir.EndsWith("/")) SettingDef::IO::outDir += "/";
    if (_name != "") _name = "_" + _name;
    _name = SettingDef::IO::outDir + "EventType" + _name + ".log";

    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "EventType", (TString) "SaveToLog", _name);

    ofstream _file(_name);
    if (!_file.is_open()) MessageSvc::Error("Unable to open file", _name, "EXIT_FAILURE");
    _file << *this << endl;
    _file.close();

    MessageSvc::Line();
    return;
}

TString EventType::SaveToYAML(TString _name, TString _option) {
    TString _baseName = _name;
    if (!SettingDef::IO::outDir.EndsWith("/")) SettingDef::IO::outDir += "/";
    if (_name != "") _name = "-" + _name;
    _name = SettingDef::IO::outDir + "config" + _name + ".yaml";

    if (_option.Contains("smanager")) _option = "-smanager-noSetting-noConfig-noCut-noWeight-noTuple-noEvents";

    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "EventType", (TString) "SaveToYAML", _name, _option);

    YAML::Emitter _emitter;
    _emitter << YAML::BeginMap;

    if (!_option.Contains("noSetting")) {
        _emitter << YAML::Key << "Setting";
        _emitter << YAML::BeginMap;
        _emitter << YAML::Key << "name" << SettingDef::name;
        //_emitter << YAML::Key << "blindString" << SettingDef::blindString;
        _emitter << YAML::Key << "debug" << SettingDef::debug;
        _emitter << YAML::Key << "eosHome" << SettingDef::IO::useEOSHome;
        _emitter << YAML::EndMap;
    }

    if (!_option.Contains("noConfig")) {
        _emitter << YAML::Key << "Config";
        _emitter << YAML::BeginMap;
        _emitter << YAML::Key << "project" << to_string(GetProject());
        _emitter << YAML::Key << "ana" << to_string(GetAna());
        _emitter << YAML::Key << "sample" << GetSample();
        _emitter << YAML::Key << "q2bin" << to_string(GetQ2bin());
        _emitter << YAML::Key << "year" << to_string(GetYear());
        _emitter << YAML::Key << "polarity" << to_string(GetPolarity());
        _emitter << YAML::Key << "trigger" << to_string(GetTrigger());
        _emitter << YAML::Key << "triggerConf" << to_string(GetTriggerConf());
        _emitter << YAML::Key << "brem" << to_string(GetBrem());
        _emitter << YAML::Key << "track" << to_string(GetTrack());
        _emitter << YAML::EndMap;
    }

    if (_option.Contains("smanager")) {
        _emitter << YAML::Key << "Config";
        _emitter << YAML::BeginMap;        
        _emitter << YAML::Key << "q2bin" << to_string(GetQ2bin());
        _emitter << YAML::EndMap;
    }

    if (!_option.Contains("noCut")) {
        _emitter << YAML::Key << "Cut";
        _emitter << YAML::BeginMap;
        _emitter << YAML::Key << "option"      << GetCutHolder().Option();
        _emitter << YAML::Key << "mvaVer"      << SettingDef::Cut::mvaVer;
        _emitter << YAML::Key << "extraCut"    << SettingDef::Cut::extraCut;
        _emitter << YAML::Key << "extraEEOnly" << SettingDef::Cut::extraEEOnly;
        _emitter << YAML::Key << "extraMMOnly" << SettingDef::Cut::extraMMOnly;
        _emitter << YAML::Key << "tightLowQ2"  << SettingDef::Cut::tightLowQ2;
        _emitter << YAML::Key << "force"       << SettingDef::Cut::force;
        _emitter << YAML::EndMap;
    }

    if (!_option.Contains("noWeight")) {
        _emitter << YAML::Key << "Weight";
        _emitter << YAML::BeginMap;
        _emitter << YAML::Key << "option" << GetWeightHolder().Option();
        _emitter << YAML::Key << "config" << GetWeightHolder().Config();
        _emitter << YAML::Key << "trkVer" << SettingDef::Weight::trkVer;
        _emitter << YAML::Key << "pidVer" << SettingDef::Weight::pidVer;
        _emitter << YAML::Key << "l0Ver" << SettingDef::Weight::l0Ver;
        _emitter << YAML::Key << "hltVer" << SettingDef::Weight::hltVer;
        _emitter << YAML::Key << "mcVer" << SettingDef::Weight::mcVer;
        _emitter << YAML::Key << "q2SmearFileTag" << SettingDef::Weight::q2SmearFileTag;
        if(  SettingDef::Weight::q2SmearDiffVar != "" ){
            _emitter << YAML::Key << "q2SmearDiffVar" << SettingDef::Weight::q2SmearDiffVar;
        }

        _emitter << YAML::Key << "priorChain" << SettingDef::Weight::priorChain;
        if (SettingDef::Weight::iBS != -1) _emitter << YAML::Key << "iBS" << SettingDef::Weight::iBS;
        _emitter << YAML::Key << "useBS" << SettingDef::Weight::useBS;        
        _emitter << YAML::Key << "usePIDPTElectron" << SettingDef::Weight::usePIDPTElectron;
        _emitter << YAML::Key << "useMCRatioPID" << SettingDef::Weight::useMCRatioPID;
        _emitter << YAML::Key << "TrkFromRKst" << SettingDef::Weight::TrkFromRKst;
        _emitter << YAML::Key << "L0I_EToE" << SettingDef::Weight::L0I_EToE;

        _emitter << YAML::EndMap;
    }

    if (!_option.Contains("noTuple")) {
        _emitter << YAML::Key << "Tuple";
        _emitter << YAML::BeginMap;
        _emitter << YAML::Key << "option" << GetTupleHolder().Option();
        _emitter << YAML::Key << "gngVer" << SettingDef::Tuple::gngVer;
        _emitter << YAML::Key << "proVer" << SettingDef::Tuple::proVer;
        _emitter << YAML::Key << "creVer" << SettingDef::Tuple::creVer;
        _emitter << YAML::Key << "splVer" << SettingDef::Tuple::splVer;
        _emitter << YAML::Key << "fileName" << SettingDef::Tuple::fileName;
        _emitter << YAML::Key << "tupleName" << SettingDef::Tuple::tupleName;
        _emitter << YAML::Key << "dataFrame" << SettingDef::Tuple::dataFrame;
        _emitter << YAML::Key << "branches" << SettingDef::Tuple::branches;
        if (SettingDef::Tuple::branchList.size() != 0) {
            _emitter << YAML::Key << "branchList";
            _emitter << YAML::Flow << SettingDef::Tuple::branchList;
        }
        _emitter << YAML::Key << "aliases" << SettingDef::Tuple::aliases;
        _emitter << YAML::Key << "frac" << SettingDef::Tuple::frac;
        _emitter << YAML::Key << "datasetCache" << SettingDef::Tuple::datasetCache;
        _emitter << YAML::Key << "addTuple" << SettingDef::Tuple::addTuple;
        _emitter << YAML::Key << "noInit" << SettingDef::Tuple::noInit;
        _emitter << YAML::Key << "useURLS" << SettingDef::Tuple::useURLS;
        _emitter << YAML::EndMap;
    }

    if (!_option.Contains("noEvents")) {
        if (SettingDef::Events::types.size() != 0) {
            _emitter << YAML::Key << "Events";
            _emitter << YAML::BeginMap;
            MessageSvc::Info("EventType(s) parsed", to_string(SettingDef::Events::types.size()));
            if (SettingDef::Events::fails.size() != 0) MessageSvc::Warning("EventType(s) failed", to_string(SettingDef::Events::fails.size()));
            for (const auto & _event : SettingDef::Events::types) {
                if ((_event.GetQ2bin() != Q2Bin::All) && (_event.GetQ2bin() != GetQ2bin())) continue;
                if ((_event.GetBrem() != Brem::All) && ((_event.GetSample().Contains("JPs") && (GetQ2bin() != Q2Bin::JPsi)) || (_event.GetSample().Contains("Psi") && (GetQ2bin() != Q2Bin::Psi)) || (!_event.GetSample().Contains("JPs") && !_event.GetSample().Contains("Psi") && (GetQ2bin() != Q2Bin::Low) && (GetQ2bin() != Q2Bin::Central) && (GetQ2bin() != Q2Bin::High)))) continue;
                MessageSvc::Info(_event.GetSample(), to_string(_event.GetTrigger()), to_string(_event.GetBrem()), to_string(_event.GetTrack()), _event.GetCutHolder().Option(), _event.GetWeightHolder().Option(), _event.GetTupleHolder().Option());
                _emitter << YAML::Key << "event";
                _emitter << YAML::BeginMap;
                _emitter << YAML::Key << "sample" << _event.GetSample();
                _emitter << YAML::Key << "q2bin" << to_string(GetQ2bin());
                _emitter << YAML::Key << "year" << ((to_string(_event.GetYear()) != "") ? to_string(_event.GetYear()) : to_string(GetYear()));
                _emitter << YAML::Key << "polarity" << ((to_string(_event.GetPolarity()) != "") ? to_string(_event.GetPolarity()) : to_string(GetPolarity()));
                _emitter << YAML::Key << "trigger" << ((to_string(_event.GetTrigger()) != "") ? to_string(_event.GetTrigger()) : to_string(GetTrigger()));
                _emitter << YAML::Key << "triggerConf" << ((to_string(_event.GetTriggerConf()) != "") ? to_string(_event.GetTriggerConf()) : to_string(GetTriggerConf()));
                _emitter << YAML::Key << "brem" << ((to_string(_event.GetBrem()) != "") ? to_string(_event.GetBrem()) : to_string(GetBrem()));
                _emitter << YAML::Key << "track" << ((to_string(_event.GetTrack()) != "") ? to_string(_event.GetTrack()) : to_string(GetTrack()));
                _emitter << YAML::Key << "cutOption" << ((_event.GetCutHolder().Option() != "") ? _event.GetCutHolder().Option() : GetCutHolder().Option());
                _emitter << YAML::Key << "weightOption" << ((_event.GetWeightHolder().Option() != "") ? _event.GetWeightHolder().Option() : GetWeightHolder().Option());
                _emitter << YAML::Key << "tupleOption" << ((_event.GetTupleHolder().Option() != "") ? _event.GetTupleHolder().Option() : GetTupleHolder().Option());
                _emitter << YAML::EndMap;
            }
            _emitter << YAML::EndMap;
        }
    }

    if (!_option.Contains("noEfficiency")) {
        _emitter << YAML::Key << "Efficiency";
        _emitter << YAML::BeginMap;
        _emitter << YAML::Key << "option" << SettingDef::Efficiency::option;
        //TODO : Obsolete for fitter
        // _emitter << YAML::Key << "ver" << SettingDef::Efficiency::ver;
        _emitter << YAML::Key << "flatnessVer" << SettingDef::Efficiency::flatnessVer;
        _emitter << YAML::Key << "scaleEfficiency" << SettingDef::Efficiency::scaleEfficiency;
        _emitter << YAML::Key << "scaleSystematics" << SettingDef::Efficiency::scaleSystematics;
        SettingDef::Efficiency::fitconfiguration.EmitToYaml(_emitter);
        _emitter << YAML::EndMap;
    }

    if (!_option.Contains("noFit")) {
        _emitter << YAML::Key << "Fit";
        _emitter << YAML::BeginMap;
        _emitter << YAML::Key << "option" << SettingDef::Fit::option;
        _emitter << YAML::Key << "ver" << SettingDef::Fit::ver;
        _emitter << YAML::Key << "startLLScanFromMin" << SettingDef::Fit::startLLScanFromMin;
        _emitter << YAML::Key << "scan1DParameter" << SettingDef::Fit::scan1DParameter;
        _emitter << YAML::Key << "nScanPointsProfile" << SettingDef::Fit::nScanPointsProfile;
        _emitter << YAML::Key << "scanProfileManual" << SettingDef::Fit::scanProfileManual;
        _emitter << YAML::Key << "minValScan" << SettingDef::Fit::minValScan;
        _emitter << YAML::Key << "maxValScan" << SettingDef::Fit::maxValScan;
        _emitter << YAML::Key << "binned" << SettingDef::Fit::doBinned;
        _emitter << YAML::Key << "nBins" << SettingDef::Fit::nBins;
        _emitter << YAML::Key << "splitL0Categories" << SettingDef::Fit::splitL0Categories;
        _emitter << YAML::Key << "splitRunPeriods" << SettingDef::Fit::splitRunPeriods;
        _emitter << YAML::Key << "splitTrackCategories" << SettingDef::Fit::splitTrackCategories;
        _emitter << YAML::Key << "plotSumCategories" << SettingDef::Fit::plotSumCategories;
        _emitter << YAML::Key << "blindYield" << SettingDef::Fit::blindYield;
        _emitter << YAML::Key << "blindEfficiency" << SettingDef::Fit::blindEfficiency;
        _emitter << YAML::Key << "blindRatio" << SettingDef::Fit::blindRatio;
        _emitter << YAML::Key << "reduceRooKeysPDF" << SettingDef::Fit::reduceRooKeysPDF;
        _emitter << YAML::Key << "useDatasetCache" << SettingDef::Fit::useDatasetCache;
        _emitter << YAML::Key << "redoDatasetCache" << SettingDef::Fit::redoDatasetCache;
        _emitter << YAML::Key << "useBremFracCache" << SettingDef::Fit::useBremFracCache;
        _emitter << YAML::Key << "redoBremFracCache" << SettingDef::Fit::redoBremFracCache;
        _emitter << YAML::Key << "useRecursiveFractions" << SettingDef::Fit::useRecursiveFractions;
        _emitter << YAML::Key << "useRooRealSumPDF" << SettingDef::Fit::useRooRealSumPDF;
        _emitter << YAML::Key << "useMinuit2"       << SettingDef::Fit::useMinuit2;
        _emitter << YAML::Key << "RatioParsMinos"   << SettingDef::Fit::RatioParsMinos;
        _emitter << YAML::Key << "initialParamFile" << YAML::BeginSeq ;
        for( auto & el :SettingDef::Fit::initialParamFile ){
            _emitter << el;        
        }
        _emitter << YAML::EndSeq;
        _emitter << YAML::Key << "dumpParamFile" << SettingDef::Fit::dumpParamFile;
        _emitter << YAML::Key << "splot2" << SettingDef::Fit::useSPlot2;
        _emitter << YAML::Key << "IndexBootTemplateMisID" << SettingDef::Fit::IndexBootTemplateMisID;
        _emitter << YAML::Key << "saveFitComponentCaches" << SettingDef::Fit::saveFitComponentCaches;
        _emitter << YAML::Key << "loadFitComponentCaches" << SettingDef::Fit::loadFitComponentCaches;
        _emitter << YAML::Key << "redoFitComponentCaches" << SettingDef::Fit::redoFitComponentCaches;
        _emitter << YAML::Key << "rareOnly" << SettingDef::Fit::rareOnly;
        _emitter << YAML::Key << "useRatioComb" << SettingDef::Fit::useRatioComb;
        _emitter << YAML::Key << "useNumericalExpTurnOn" << SettingDef::Fit::useNumericalExpTurnOn;
        _emitter << YAML::Key << "LocalCaches" << SettingDef::Fit::LocalCaches;
        _emitter << YAML::Key << "nCPUDataFit" << SettingDef::Fit::nCPUDataFit;    
        _emitter << YAML::Key << "LPTMCandidates" << SettingDef::Fit::LPTMCandidates;
        _emitter << YAML::Key << "CorrelateConstraintsNoNorm" << SettingDef::Fit::LPTMCandidates;
        if(SettingDef::Fit::rJPsiFit || SettingDef::Fit::RPsiFit || SettingDef::Fit::RXFit ){
            _emitter << YAML::Key << "rJPsiFit" << SettingDef::Fit::rJPsiFit;
            _emitter << YAML::Key << "RPsiFit" << SettingDef::Fit::RPsiFit;
            _emitter << YAML::Key << "RXFit" << SettingDef::Fit::RXFit;
            _emitter << YAML::Key << "RatioSystFile" << YAML::BeginSeq ;
            for( auto & el :SettingDef::Fit::RatioSystFile ){
                _emitter << el;
            }
            _emitter << YAML::EndSeq;


        }

        if (SettingDef::Fit::configurations.size() != 0) {
            _emitter << YAML::Key << "configuration";
            _emitter << YAML::BeginMap;
            for (const auto & _configuration : SettingDef::Fit::configurations) {
                _emitter << YAML::Key << to_string(_configuration.GetAna());
                _emitter << YAML::BeginMap;
                if (_configuration.VarName() != "") _emitter << YAML::Key << "varName" << _configuration.VarName();
                _emitter << YAML::Key << "massConstrained" << _configuration.HasConstrainedMass();
                _emitter << YAML::Key << "binAndRangeMC";
                _emitter << YAML::Flow;
                _emitter << YAML::BeginSeq << (int) _configuration.IsBinnedMC() << _configuration.Var()->getBins(SettingDef::Fit::varSchemeMC) << _configuration.Var()->getMin(SettingDef::Fit::varSchemeMC) << _configuration.Var()->getMax(SettingDef::Fit::varSchemeMC) << YAML::EndSeq;
                _emitter << YAML::Key << "binAndRangeCL";
                _emitter << YAML::Flow;
                _emitter << YAML::BeginSeq << (int) _configuration.IsBinnedCL() << _configuration.Var()->getBins(SettingDef::Fit::varSchemeCL) << _configuration.Var()->getMin(SettingDef::Fit::varSchemeCL) << _configuration.Var()->getMax(SettingDef::Fit::varSchemeCL) << YAML::EndSeq;
                _emitter << YAML::Key << "composition";
                _emitter << YAML::Flow;
                _emitter << YAML::BeginSeq;
                for (const auto & _composition : _configuration.Composition()) { _emitter << _composition; }
                _emitter << YAML::EndSeq;      
                //labels emitter configs          
                if( _configuration.HasLabels() ){
                    _emitter << YAML::Key << "labels";
                    _emitter << YAML::BeginMap;
                    for( auto & label : _configuration.GetLabelsNamed() ){
                        _emitter<< YAML::Key << label.first << label.second; 
                    }
                    _emitter << YAML::EndMap;
                }
                if( _configuration.HasColors() ){
                    _emitter << YAML::Key << "colors";
                    _emitter << YAML::BeginMap;
                    for( auto & color : _configuration.GetColorsNamed() ){
                        TString _colorHex(gROOT->GetColor(color.second)->AsHexString());
                        _emitter<< YAML::Key << color.first <<_colorHex.Data(); 
                    }
                    _emitter << YAML::EndMap;
                }                
                _emitter << YAML::EndMap;
            }
            _emitter << YAML::EndMap;
        } else if (SettingDef::Fit::yamls.size() != 0) {
            _emitter << YAML::Key << "yaml";
            _emitter << YAML::BeginMap;
            for (const auto & _yaml : SettingDef::Fit::yamls) {
                _emitter << YAML::Key << to_string(_yaml.first.first);
                _emitter << YAML::BeginMap;
                _emitter << YAML::Key << to_string(_yaml.first.second);
                _emitter << YAML::Flow;
                _emitter << YAML::BeginSeq << _yaml.second.first << _yaml.second.second << YAML::EndSeq;
                _emitter << YAML::EndMap;
            }
            _emitter << YAML::EndMap;
        }
        _emitter << YAML::EndMap;
    }

    if (!_option.Contains("noToy")) {
        _emitter << YAML::Key << "Toy";
        _emitter << YAML::BeginMap;
        _emitter << YAML::Key << "option"                  << SettingDef::Toy::option;
        _emitter << YAML::Key << "tupleVer"                << SettingDef::Toy::tupleVer;
        _emitter << YAML::Key << "studyVer"                << SettingDef::Toy::studyVer;
        _emitter << YAML::Key << "nJobs"                   << SettingDef::Toy::nJobs;
        _emitter << YAML::Key << "jobIndex"                << SettingDef::Toy::jobIndex;
        _emitter << YAML::Key << "nToysPerJob"             << SettingDef::Toy::nToysPerJob;
        _emitter << YAML::Key << "constraintOverwriteFile" << YAML::BeginSeq ;
        for( auto & el :SettingDef::Toy::constraintOverwriteFile ){
            _emitter << el;        
        }
        _emitter << YAML::EndSeq;    
        _emitter << YAML::Key << "frozenOverwrite" << SettingDef::Toy::frozenOverwrite;
        _emitter << YAML::Key << "mergeConfig" << SettingDef::Toy::mergeConfig;
        _emitter << YAML::Key << "Silent" << SettingDef::Toy::Silent;
        _emitter << YAML::Key << "CopyLocally" << SettingDef::Toy::CopyLocally;

        _emitter << YAML::Key << "ReadFractionToysComponents" << SettingDef::Toy::ReadFractionToysComponents;
        _emitter << YAML::Key << "configurationOverrideFile" << SettingDef::Toy::configurationOverrideFile;

        if (SettingDef::Fit::configurations.size() != 0) {
            _emitter << YAML::Key << "configuration";
            _emitter << YAML::BeginMap;
            for (const auto & _configuration : SettingDef::Fit::configurations) {
                if (_baseName.Contains(to_string(_configuration.GetProject()) + SettingDef::separator)) {
                    _emitter << YAML::Key << to_string(_configuration.GetAna());
                    _emitter << YAML::BeginMap;
                    _emitter << YAML::Key << "composition";
                    _emitter << YAML::Flow;
                    _emitter << YAML::BeginSeq;
                    for (const auto & _composition : _configuration.Composition()) {
                        auto *  _strCollection = ((TString) _composition).Tokenize("|");
                        TString _sampleID      = TString(((TObjString *) (*_strCollection).At(0))->String());
                        if (_sampleID.Contains("-0G")) _sampleID.ReplaceAll("-0G", "");
                        if (_sampleID.Contains("-1G") || _sampleID.Contains("-2G")) continue;
                        TString _pdf    = to_string(PdfType::ToyPDF);
                        TString _optionEmit = "-scale[1]-yield[0]";
                        _emitter << _sampleID + " | " + _pdf + " | " + _optionEmit;
                    }
                    _emitter << YAML::EndSeq;
                    _emitter << YAML::EndMap;
                }
            }
            _emitter << YAML::EndMap;
        } else if (SettingDef::Toy::yamls.size() != 0) {
            _emitter << YAML::Key << "yaml";
            _emitter << YAML::BeginMap;
            for (const auto & _yaml : SettingDef::Toy::yamls) {
                _emitter << YAML::Key << to_string(_yaml.first.first);
                _emitter << YAML::BeginMap;
                _emitter << YAML::Key << to_string(_yaml.first.second);
                _emitter << YAML::Flow;
                _emitter << YAML::BeginSeq << _yaml.second.first << _yaml.second.second << YAML::EndSeq;
                _emitter << YAML::EndMap;
            }
            _emitter << YAML::EndMap;
        }
        _emitter << YAML::EndMap;
    }

    if (_option.Contains("L0Config")) {
        string _L0options = _option.Data();
        _L0options        = _L0options.substr(_L0options.find("=") + 1);
        string _TOSopt    = _L0options.substr(0, _L0options.find("_"));
        _L0options.erase(0, _L0options.find("_") + 1);
        string _TISopt = _L0options.substr(0, _L0options.find("_"));
        _L0options.erase(0, _L0options.find("_") + 1);
        string _INTERPopt = _L0options.substr(0, _L0options.find("_"));
        _L0options.erase(0, _L0options.find("_") + 1);
        string _L0LVARopt = _L0options.substr(0, _L0options.find("_"));
        _L0options.erase(0, _L0options.find("_") + 1);
        string _L0IVARopt = _L0options.substr(0, _L0options.find("_"));
        _L0options.erase(0, _L0options.find("_") + 1);
        string _L0HVARopt = _L0options.substr(0, _L0options.find("_"));
        _L0options.erase(0, _L0options.find("_") + 1);
        string _BINopt = _L0options.substr(0, _L0options.find("_"));
        _L0options.erase(0, _L0options.find("_") + 1);
        string _FITopt = _L0options.substr(0, _L0options.find("_"));

        _emitter << YAML::Key << "L0Config";
        _emitter << YAML::BeginMap;
        _emitter << YAML::Key << "TOSopt" << _TOSopt;
        _emitter << YAML::Key << "TISopt" << _TISopt;
        _emitter << YAML::Key << "INTERPopt" << _INTERPopt;
        _emitter << YAML::Key << "L0LVARopt" << _L0LVARopt;
        _emitter << YAML::Key << "L0IVARopt" << _L0IVARopt;
        _emitter << YAML::Key << "L0HVARopt" << _L0HVARopt;
        _emitter << YAML::Key << "BINopt" << _BINopt;
        _emitter << YAML::Key << "FITopt" << _FITopt;
    }

    _emitter << YAML::EndMap;

    ofstream _file(_name);
    if (!_file.is_open()) MessageSvc::Error("Unable to open file", _name, "EXIT_FAILURE");
    _file << _emitter.c_str() << endl;
    _file.close();

    MessageSvc::Line();
    return _name;
}

const TNamed EventType::GetNamedCutOption() const { return TNamed((TString) "CutOption", GetCutHolder().Option()); }

const TNamed EventType::GetNamedWeightOption() const { return TNamed((TString) "WeightOption", GetWeightHolder().Option()); }

const TNamed EventType::GetNamedTupleOption() const { return TNamed((TString) "TupleOption", GetTupleHolder().Option()); }

const TNamed EventType::GetNamedWeight() const { return TNamed((TString) "Weight", GetWeight()); }

#endif
