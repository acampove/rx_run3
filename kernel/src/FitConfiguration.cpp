#ifndef FITCONFIGURATION_CPP
#define FITCONFIGURATION_CPP

#include "FitConfiguration.hpp"

#include "ConstDef.hpp"
#include "CutDefRX.hpp"
#include "CutDefRKst.hpp"
#include "CutDefRK.hpp"

#include "SettingDef.hpp"

ClassImp(FitConfiguration);

FitConfiguration::FitConfiguration(const Prj & _prj, const Analysis & _ana, const Q2Bin & _q2Bin, const Year & _year, const Polarity & _polarity, const Trigger & _trigger, const Brem & _brem, const Track & _track, const bool & _constrainedMass, const tuple< bool, int, double, double > & _configVarMC, const tuple< bool, int, double, double > & _configVarCL, const vector< TString > & _composition)
    : ConfigHolder(
            _prj, 
            _ana, 
            "", 
            _q2Bin, 
            _year, 
            _polarity, 
            _trigger, 
            hash_triggerconf(SettingDef::Config::triggerConf), 
            _brem, 
            _track)   // <- construct the class it inherits from (the ConfigHolder, A FitConfiguration doesn't need a Tuple/Weight/Cut !)
{
    if (SettingDef::debug.Contains("FF")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("FitConfiguration", (TString) "");

    m_key = this->GetKey();   //< it's the GetKey for the FitHolder we have in hands !

    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitConfiguration", m_key);
    (static_cast< const ConfigHolder & >(*this)).PrintInline();

    m_constrainedMass = _constrainedMass;
    // if (m_constrainedMass && !SettingDef::Cut::option.Contains("noPR")) { MessageSvc::Warning("FitConfiguration", (TString) "When fitting JPs/Psi constrained mass remove part-reco cut using -noPR in CutOption", "EXIT_FAILURE"); }

    m_binnedMC = get< 0 >(_configVarMC);
    m_binnedCL = get< 0 >(_configVarCL);

    m_nBinsMC = get< 1 >(_configVarMC);
    m_nBinsCL = get< 1 >(_configVarCL);

    m_minMC = get< 2 >(_configVarMC);
    m_maxMC = get< 3 >(_configVarMC);

    m_minCL = get< 2 >(_configVarCL);
    m_maxCL = get< 3 >(_configVarCL);

    m_composition = _composition;

    Init();
}

FitConfiguration::FitConfiguration(const Prj & _prj, const Analysis & _ana, const Q2Bin & _q2Bin, const Year & _year, const Polarity & _polarity, const Trigger & _trigger, const Brem & _brem, const Track & _track, const TString & _varName, const tuple< bool, int, double, double > & _configVarMC, const tuple< bool, int, double, double > & _configVarCL, const vector< TString > & _composition)
    : ConfigHolder(
            _prj, 
            _ana, 
            "", 
            _q2Bin, 
            _year, 
            _polarity, 
            _trigger, 
            hash_triggerconf(SettingDef::Config::triggerConf), 
            _brem, 
            _track)   // <- construct the class it inherits from (the ConfigHolder, A FitConfiguration doesn't need a Tuple/Weight/Cut !)
{
    if (SettingDef::debug.Contains("FF")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("FitConfiguration", (TString) "varName");

    m_key = this->GetKey();   //< it's the GetKey for the FitHolder we have in hands !

    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitConfiguration", m_key);
    (static_cast< const ConfigHolder & >(*this)).PrintInline();

    m_varName = _varName;

    m_binnedMC = get< 0 >(_configVarMC);
    m_binnedCL = get< 0 >(_configVarCL);

    m_nBinsMC = get< 1 >(_configVarMC);
    m_nBinsCL = get< 1 >(_configVarCL);

    m_minMC = get< 2 >(_configVarMC);
    m_maxMC = get< 3 >(_configVarMC);

    m_minCL = get< 2 >(_configVarCL);
    m_maxCL = get< 3 >(_configVarCL);

    m_composition = _composition;

    Init();
}

FitConfiguration::FitConfiguration(const Prj & _prj, const Analysis & _ana, const Q2Bin & _q2Bin, const Year & _year, const Polarity & _polarity, const Trigger & _trigger, const Brem & _brem, const Track & _track, const vector< TString > & _composition)
    : ConfigHolder(
            _prj, 
            _ana, 
            "", 
            _q2Bin, 
            _year, 
            _polarity, 
            _trigger, 
            hash_triggerconf(SettingDef::Config::triggerConf), 
            _brem, 
            _track)   // <- construct the class it inherits from (the ConfigHolder, A FitConfiguration doesn't need a Tuple/Weight/Cut !)
{
    if (SettingDef::debug.Contains("FF")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("FitConfiguration", (TString) "composition");

    m_key = this->GetKey();   //< it's the GetKey for the FitHolder we have in hands !

    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitConfiguration", m_key);
    (static_cast< const ConfigHolder & >(*this)).PrintInline();

    m_composition = _composition;

    ParseComposition();
}

// FitConfiguration::FitConfiguration(const FitConfiguration & _fitConfiguration)
//    : ConfigHolder(static_cast< ConfigHolder >(_fitConfiguration)) {
//    if (SettingDef::debug.Contains("FF")) SetDebug(true);
//    if (m_debug) MessageSvc::Debug("FitConfiguration", (TString) "FitConfiguration");
//
//    m_key = this->GetKey();   //< it's the GetKey for the FitHolder we have in hands !
//
//    m_var     = _fitConfiguration.Var();
//    m_varName = _fitConfiguration.VarName();
//    UseBinAndRange(SettingDef::Fit::varSchemeMC);
//    m_binnedMC = _fitConfiguration.IsBinnedMC();
//    m_nBinsMC  = m_var->getBins();
//    m_minMC    = m_var->getMin();
//    m_maxMC    = m_var->getMax();
//    UseBinAndRange(SettingDef::Fit::varSchemeCL);
//    m_binnedCL = _fitConfiguration.IsBinnedCL();
//    m_nBinsCL  = m_var->getBins();
//    m_minCL    = m_var->getMin();
//    m_maxCL    = m_var->getMax();
//
//    m_composition     = _fitConfiguration.Composition();
//    m_constrainedMass = _fitConfiguration.HasConstrainedMass();
//    m_hasBrem         = _fitConfiguration.HasBrem();
//
//    m_signalSample      = _fitConfiguration.SignalSample();
//    m_backgroundSamples = _fitConfiguration.BackgroundSamples();
//
//    m_eventTypeAndOptions = _fitConfiguration.EventTypes();
//
//    m_componentsAndOptions     = _fitConfiguration.Components();
//    m_componentsAndOptionsBrem = _fitConfiguration.ComponentsBrem();
//
//    MessageSvc::Info(Color::Cyan, "FitConfiguration", m_key);
//    (static_cast< const ConfigHolder & >(*this)).PrintInline();
//    // Init();
//}

void FitConfiguration::Init() {
    MessageSvc::Info(Color::Cyan, "FitConfiguration", m_key, "Initialize ...");

    if (m_constrainedMass && (m_q2bin != Q2Bin::JPsi) && (m_q2bin != Q2Bin::Psi)) MessageSvc::Error("Init", (TString) "Created with wrong q2 and mass constrain flag", to_string(m_q2bin), to_string(m_constrainedMass), "EXIT_FAILURE");

    for (const auto & _composition : m_composition) {
        auto * _strCollection = ((TString) _composition).Tokenize("|");
        if (((*_strCollection).GetEntries() < 3) || ((*_strCollection).GetEntries() > 7)) MessageSvc::Error("Init", (TString) "Unable to parse, must have at least 3 items separated by '|' : ['Signal' | 'StringToPDF' | 'Ipatia2-....']", TString(_composition), "EXIT_FAILURE");
    }

    SetHasBrem();
    ParseComposition();

    // The Var is build with the defaults ranges. If parsed values are < 0, use the default ones...
    BuildFitVar();

    // If the argument passe for the min/max CL/MC are less than 0, we build anyway the range and the RooBinning for that, but we use the "built_in" variable ranges.
    if (m_minMC < 0) m_minMC = m_var->getMin();
    if (m_maxMC < 0) m_maxMC = m_var->getMax();
    if (m_minCL < 0) m_minCL = m_var->getMin();
    if (m_maxCL < 0) m_maxCL = m_var->getMax();

    SetBinAndRange(SettingDef::Fit::varSchemeMC, m_binnedMC, m_nBinsMC, m_minMC, m_maxMC);

    int _cacheBins = 10000;
    // SetBinAndRange("cache", m_binnedCL, _cacheBins, m_minCL, m_maxCL);
    _cacheBins = int(2 * m_maxCL / double(m_maxCL - m_minCL) * _cacheBins);
    SetBinAndRange("cache", m_binnedCL, _cacheBins, -m_maxCL, m_maxCL);   // RooFFTConvPdf in FitComponent::ConvPDF needs symmetric cache range

    SetBinAndRange(SettingDef::Fit::varSchemeCL, m_binnedCL, m_nBinsCL, m_minCL, m_maxCL);

    Print();

    return;
}

void FitConfiguration::Close() {
    /*
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitConfiguration", m_key, "Close ...");
    MessageSvc::Line();

    Print();

    for (auto & _eventType : m_eventTypeAndOptions) {
        MessageSvc::Info("Close", _eventType.first);
        get< 0 >(_eventType.second).Close();
        MessageSvc::Line();
    }
    */
    return;
}

void FitConfiguration::CheckTupleEntries(Long64_t _entries, bool _error) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitConfiguration", m_key, "CheckTupleEntries");
    MessageSvc::Line();

    map< TString, Long64_t > _samples;

    auto _data = GetData();
    if ((get< 0 >(_data)).GetTuple() != nullptr) _samples[(get< 0 >(_data)).GetSample()] = (get< 0 >(_data)).GetTupleEntries();
    (get< 0 >(_data)).Close();

    auto _signal = GetSignal(Brem::All);
    if ((get< 0 >(_signal)).GetTuple() != nullptr) _samples[(get< 0 >(_signal)).GetSample()] = (get< 0 >(_signal)).GetTupleEntries();
    (get< 0 >(_signal)).Close();

    for (auto & _component : Components()) {
        if (_component.first == SignalSample()) continue;
        auto _background = GetBackground(_component.first);
        if ((get< 0 >(_background)).GetTuple() != nullptr) _samples[(get< 0 >(_background)).GetSample()] = (get< 0 >(_background)).GetTupleEntries();
        (get< 0 >(_background)).Close();
    }

    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitConfiguration", m_key, "CheckTupleEntries");
    for (auto & _sample : _samples) { MessageSvc::Info(_sample.first, to_string(_sample.second), "entries"); }
    for (auto & _sample : _samples) {
        if (_sample.second <= _entries) {
            if (SettingDef::trowLogicError || _error)
                MessageSvc::Error("Tuple has less than", to_string(_entries), "logic_error");
            else
                MessageSvc::Error("Tuple has less than", to_string(_entries), "EXIT_FAILURE");
        }
    }
    MessageSvc::Line();
    return;
}

void FitConfiguration::SetHasBrem() {
    if (m_debug) MessageSvc::Debug("FitConfiguration", m_key, "SetHasBrem");
    m_hasBrem = false;
    for (const auto & _composition : m_composition) {
        auto *  _strCollection = ((TString) _composition).Tokenize("|");
        TString _sampleID      = TString(((TObjString *) (*_strCollection).At(0))->String());
        if (_sampleID.Contains("Signal") && (_sampleID.Contains(to_string(Brem::G0)) || _sampleID.Contains(to_string(Brem::G1)) || _sampleID.Contains(to_string(Brem::G2)))) m_hasBrem = true;
    }
    if (m_hasBrem && (m_ana == Analysis::MM)) MessageSvc::Error("SetHasBrem", (TString) "Not allowed for MM", "EXIT_FAILURE");
    if (m_hasBrem) MessageSvc::Info("Composition", (TString) "HasBrem");
    return;
}

// MAIN PARSING AND FILLING OF PRIVATE MEMBERS !
void FitConfiguration::ParseComposition() {
    MessageSvc::Info(Color::Cyan, "FitConfiguration", m_key, "ParseComposition", to_string(m_composition.size()));

    // This is a quite evil bit of code to parse what is streamed to the YAML file the idea is simple :
    // You have the shapes passed via YAML in this way:

    double _yieldFractionTot = 0;
    for (const auto & _composition : m_composition) {
        auto * _strCollection = ((TString) _composition).Tokenize("|");

        // Unpack the set of 3 strings separated by "|" string key word
        TString _sampleID      = TString(((TObjString *) (*_strCollection).At(0))->String());
        TString _pdfType       = TString(((TObjString *) (*_strCollection).At(1))->String());
        TString _pdfOption     = TString(((TObjString *) (*_strCollection).At(2))->String());
        TString _cutOption     = (*_strCollection).GetEntries() > 3 ? TString(((TObjString *) (*_strCollection).At(3))->String()) : SettingDef::Cut::option;
        TString _weightOption  = (*_strCollection).GetEntries() > 4 ? TString(((TObjString *) (*_strCollection).At(4))->String()) : SettingDef::Weight::option;
        TString _tupleOption   = (*_strCollection).GetEntries() > 5 ? TString(((TObjString *) (*_strCollection).At(5))->String()) : SettingDef::Tuple::option;
        double  _yieldFraction = (*_strCollection).GetEntries() > 6 ? TString(((TObjString *) (*_strCollection).At(6))->String()).Atof() : m_noYieldFraction;
        _sampleID.ReplaceAll(" ", "");
        _pdfType.ReplaceAll(" ", "");
        _pdfOption.ReplaceAll(" ", "");
        _cutOption.ReplaceAll(" ", "");
        _weightOption.ReplaceAll(" ", "");
        _tupleOption.ReplaceAll(" ", "");        

        //L0H special treatment
        if (this->GetTrigger() == Trigger::L0H && !_sampleID.Contains("Signal")) {
            if (this->GetProject() == Prj::RK){
                if(hash_sample(_sampleID) == Sample::MisID || hash_sample(_sampleID) == Sample::Bd2Kst || hash_sample(_sampleID) == Sample::Bu2Kst) continue;
            }
            if(hash_sample(_sampleID) == Sample::PartReco) _pdfOption = _pdfOption.ReplaceAll("+Bs2XJPsEE*0", "");
            if(hash_sample(_sampleID) == Sample::PartReco) _pdfOption = _pdfOption.ReplaceAll("+Bs2XJPsMM*0", "");
            if (this->GetProject() == Prj::RKst){
                if(hash_sample(_sampleID) == Sample::Lb || hash_sample(_sampleID) == Sample::Bs2Phi || hash_sample(_sampleID) == Sample::PartReco || hash_sample(_sampleID) == Sample::HadSwap) continue;
            }
        }

        if ((_sampleID == "PartRecoH") && !_cutOption.Contains("PR")) _cutOption += "-PRH";
        if ((_sampleID == "PartRecoL") && !_cutOption.Contains("PR")) _cutOption += "-PRL";
        //----- if not passed the global one is passed -----""
        if (_cutOption == "") _cutOption = SettingDef::Cut::option;
        //----- will always be true,  append the custom flag to the "global one"
        if (_cutOption != SettingDef::Cut::option) _cutOption = SettingDef::Cut::option + "-" + _cutOption;

        if (_weightOption == "") _weightOption = SettingDef::Weight::option;
        if (_tupleOption == "")  _tupleOption  = SettingDef::Tuple::option;
        if ((_pdfType == to_string(PdfType::SignalCopy)) || (_pdfType == to_string(PdfType::StringToPDF)) || (_pdfType == to_string(PdfType::RooAbsPDF))) {
            _cutOption    = "";
            _weightOption = "";
            _tupleOption  = "";
        }
        if (_pdfType == to_string(PdfType::ToyPDF)) {
            _cutOption    = "";
            _weightOption = "";
            _tupleOption  = "";
        }
        _cutOption    = CleanString(_cutOption);
        _weightOption = CleanString(_weightOption);
        _tupleOption  = CleanString(_tupleOption);        

        if ((_yieldFraction != m_noYieldFraction) && ((_yieldFraction <= 0) || (_yieldFraction >= 1))) MessageSvc::Error("ParseComposition", (TString) "Invalid", _sampleID, "yield fraction", to_string(_yieldFraction), "EXIT_FAILURE");
        if (_yieldFraction != m_noYieldFraction) {
            if (!m_hasBrem || !_sampleID.Contains("Signal")) _yieldFractionTot += _yieldFraction;
        }

        //===== what we can specify is "Signal-0G/1G/2G or simply Signal"
        Sample _sample = Sample::Empty;
        Brem   _brem   = Brem::All;
        if (_sampleID.Contains("Signal")) {
            // We specified Signal, the Q2Bin will tell us the naming
            // Sample enumeration for Signal shapes

            switch (m_q2bin) {
                case Q2Bin::JPsi: m_signalSample = Sample::JPsi; break;
                case Q2Bin::Psi: m_signalSample = Sample::Psi; break;
                case Q2Bin::Low: m_signalSample = Sample::LL; break;
                case Q2Bin::Central: m_signalSample = Sample::LL; break;
                case Q2Bin::High: m_signalSample = Sample::LL; break;
                case Q2Bin::Gamma: m_signalSample = Sample::Gamma; break;
                default: MessageSvc::Error("ParseComposition", _sampleID, "failed", "EXIT_FAILURE"); break;
            }
            _sample = m_signalSample;
            if (m_hasBrem) {
                auto *  _sampleBremID = _sampleID.Tokenize("-");
                TString _category     = TString(((TObjString *) _sampleBremID->At(1))->String());
                _brem                 = hash_brem(_category.ReplaceAll(" ", ""));
            }
        } else {
            _sample = hash_sample(_sampleID.ReplaceAll(" ", ""));   //.Remove(TString::EStripType::kTrailing));
            m_backgroundSamples.push_back(_sample);
        }
        /*
        WARNING: here picking tuples gets shuffled (needed for using weighted MC in fits and customizing samples proVersion to pick)
        */
        MessageSvc::Info("Composition", _sampleID, GetSampleName(_sample), _pdfType, _pdfOption, _cutOption, _weightOption, _tupleOption, _yieldFraction != -1 ? to_string(_yieldFraction) : "");
        for (const auto & _component : m_componentsAndOptions) {
            if (_component.first == _sample) MessageSvc::Error("ParseComposition", (TString) "Map sample already contains", to_string(_sample), "EXIT_FAILURE");
        }
        if (_brem != Brem::All) {
            for (const auto & _component : m_componentsAndOptionsBrem) {
                if (_component.first == _brem) MessageSvc::Error("ParseComposition", (TString) "Map sample bre already contains", to_string(_brem), "EXIT_FAILURE");
            }
            m_componentsAndOptionsBrem[_brem] = make_tuple(hash_pdftype(_pdfType), _pdfOption, _cutOption, _weightOption, _tupleOption, _yieldFraction);
        } else {
            for (const auto & _component : m_componentsAndOptions) {
                if (_component.first == _sample) MessageSvc::Error("ParseComposition", (TString) "Map sample already contains", to_string(_sample), "EXIT_FAILURE");
            }
            m_componentsAndOptions[_sample] = make_tuple(hash_pdftype(_pdfType), _pdfOption, _cutOption, _weightOption, _tupleOption, _yieldFraction);
        }
    }
    if (m_hasBrem) {
        double _bremFractionSum = 0;
        int    _nBrem           = 0;
        for (auto & _bremTuplePair : m_componentsAndOptionsBrem) {
            auto _fraction = get< 5 >(_bremTuplePair.second);
            if (_fraction != m_noYieldFraction) {
                _bremFractionSum += _fraction;
                _nBrem++;
            }
        }
        double _averageFraction                = _nBrem != 0 ? _bremFractionSum / _nBrem : m_noYieldFraction;
        m_componentsAndOptions[m_signalSample] = make_tuple(hash_pdftype(""), "", "", "", "", _averageFraction);
        if (_averageFraction != m_noYieldFraction) _yieldFractionTot += _averageFraction;
    }
    // NOT SURE WHY Commented out, to review.
    // bool _addTuple              = SettingDef::Tuple::addTuple;
    // SettingDef::Tuple::addTuple = false;
    for (const auto & _component : m_componentsAndOptionsBrem) { GenerateSignal(_component.first); }
    for (const auto & _component : m_componentsAndOptions) {
        if (_component.first == m_signalSample)
            GenerateSignal(Brem::All);
        else
            GenerateBackground(_component.first);
    }
    GenerateData();

    // SettingDef::Tuple::addTuple = _addTuple;

    if ((_yieldFractionTot < 0) || (_yieldFractionTot > 1)) MessageSvc::Error("ParseComposition", (TString) "Invalid total yield fraction", to_string(_yieldFractionTot), "EXIT_FAILURE");

    return;
}

void FitConfiguration::BuildFitVar() {
    MessageSvc::Info(Color::Cyan, "FitConfiguration", m_key, "BuildFitVar");

    vector< pair< TString, TString > > _heads = GetParticleBranchNames(m_project, m_ana, m_q2bin, "onlyhead");

    TString _name  = m_varName;
    TString _title = m_varName;
    TString _unit = "";
    double _val = 0;
    double _min = numeric_limits< double >::min();
    double _max = numeric_limits< double >::max();

    if (m_varName == "") {
        _name  = _heads[0].first + "_";
        _title = "m(" + _heads[0].second + ")";
        switch (m_project) {
            case Prj::RKst: _val = PDG::Mass::Bd; break;
            case Prj::RK: _val = PDG::Mass::Bu; break;
            case Prj::RPhi: _val = PDG::Mass::Bs; break;
            case Prj::RL: _val = PDG::Mass::Lb; break;
            case Prj::RKS: _val = PDG::Mass::Bd; break;
            default: MessageSvc::Error("BuildFitVar", (TString) "Switch for Project failed", "EXIT_FAILURE"); break;
        }
        
        if (m_project == Prj::RL) {
            _name += "DTFLambda";
            _title += " " + Tex::Lambda0;
            if (m_constrainedMass) {
                _name += "Jpsi";
                _title += " & " + Tex::JPsi;
            }
            _name += "PV_";
            _title += " constraint";
        } else if (m_project == Prj::RKS) {
            _name += "DTFKs";
            _title += " " + Tex::KShort;
            if (m_constrainedMass) {
                _name += "Jpsi";
                _title += " & " + Tex::JPsi;
            }
            _name += "PV_";
            _title += " constraint";
        } else {
            _name += "DTF_";
            if (m_constrainedMass) {
                _title += "^{DTF}";
                switch (m_q2bin) {
                    case Q2Bin::JPsi:
                        _name += "JPs_";
                        _title += "_{" + Tex::JPsi + "}";
                        break;
                    case Q2Bin::Psi:
                        _name += "Psi_";
                        _title += "_{" + Tex::Psi + "}";
                        break;
                    default: break;
                }
            }
        }

        _name += "M";
        // OLD approach: _title += " [MeV/c^{2}]";
        _unit = "MeV/c^{2}";
        if (m_constrainedMass && (m_q2bin == Q2Bin::JPsi || m_q2bin == Q2Bin::Psi)) {
            if (m_q2bin == Q2Bin::JPsi) {
                _min = m_ana == Analysis::MM ? CutDefRX::Mass::MinBDTFJPsMM : CutDefRX::Mass::MinBDTFJPsEE;
                _max = m_ana == Analysis::MM ? CutDefRX::Mass::MaxBDTFJPsMM : CutDefRX::Mass::MaxBDTFJPsEE;
            }
            if (m_q2bin == Q2Bin::Psi) {
                _min = m_ana == Analysis::MM ? CutDefRX::Mass::MinBDTFPsiMM : CutDefRX::Mass::MinBDTFPsiEE;
                _max = m_ana == Analysis::MM ? CutDefRX::Mass::MaxBDTFPsiMM : CutDefRX::Mass::MaxBDTFPsiEE;
            }
        } else {
            _min = m_ana == Analysis::MM ? CutDefRX::Mass::MinBMM : CutDefRX::Mass::MinBEE;
            _max = m_ana == Analysis::MM ? CutDefRX::Mass::MaxBMM : CutDefRX::Mass::MaxBEE;
        }        
    }else{
        auto tokens = TokenizeString( m_varName , "|");
        if( tokens.size() == 3){
            //Cusomize the fit var bit in yamls with "VARTOFIT|Label|Unit";
            _name = tokens.at(0);
            _title = tokens.at(1);
            _unit = tokens.at(2);
        }
    }

    // TO BE REVIEWED

    m_var = new RooRealVar(_name, _title, _val, _min, _max, _unit.Data());
    MessageSvc::Info("BuildFitVar", m_var);

    return;
}

void FitConfiguration::SetBinAndRange(TString _scheme, bool _binned, int _nBins, double _min, double _max) {
    MessageSvc::Info(Color::Cyan, "FitConfiguration", m_key, "SetBinAndRange", _scheme, _binned ? "Binned" : "UnBinned");
    if (_scheme != "") {
        if (!m_var->hasBinning(_scheme)) {
            m_var->setBinning(RooBinning(_nBins, _min, _max, _scheme), _scheme);
            m_var->setRange(_scheme, _min, _max);
            m_var->setBins(m_var->getBins(_scheme));
            m_var->setMin(m_var->getMin(_scheme));
            m_var->setMax(m_var->getMax(_scheme));
        } else {
            MessageSvc::Error("SetBinAndRange", (TString) "Already existing", _scheme, "EXIT_FAILURE");
        }
    }
    MessageSvc::Info("RooRealVar", m_var);
    if (m_var->getBinningNames().size() > 1) {
        for (const auto & _it : m_var->getBinningNames()) {
            if (_it != "") MessageSvc::Info("RooBinning", _it, "(", to_string(m_var->getBins((TString) _it)), to_string(m_var->getMin((TString) _it)), to_string(m_var->getMax((TString) _it)), ")");
        }
    }
    return;
}

void FitConfiguration::UseBinAndRange(TString _scheme) {
    MessageSvc::Info(Color::Cyan, "FitConfiguration", m_key, "UseBinAndRange", _scheme);
    if (m_var == nullptr) MessageSvc::Error("UseBinAndRange", (TString) "var is nullptr", "EXIT_FAILURE");
    if (m_var->hasBinning(_scheme) || (_scheme == "")) {
        m_var->setBins(m_var->getBins(_scheme));
        m_var->setMin(m_var->getMin(_scheme));
        m_var->setMax(m_var->getMax(_scheme));
        MessageSvc::Info("RooRealVar", m_var);
        MessageSvc::Info("RooBinning", (TString) "(", to_string(m_var->getBins()), to_string(m_var->getMin()), to_string(m_var->getMax()), ")");
    } else {
        MessageSvc::Error("UseBinAndRange", (TString) "Invalid", _scheme, "EXIT_FAILURE");
    }
    return;
}

TString FitConfiguration::GetSampleName(const Sample & _sample) const noexcept {
    if (m_debug) MessageSvc::Debug("FitConfiguration", m_key, "GetSampleName", to_string(_sample));

    if (_sample == Sample::Error) { MessageSvc::Error("GetSampleName", (TString) "Cannot be called if parser failed before", "You should instruct the FitConfiguration of what is your signal", "EXIT_FAILURE"); }
    if (((_sample == Sample::JPsi) && (GetQ2bin() != Q2Bin::JPsi)) || ((_sample == Sample::Psi) && (GetQ2bin() != Q2Bin::Psi))) {
        MessageSvc::Error("GetSampleName", (TString) "Required", to_string(_sample), "for q2Bin =", to_string(m_q2bin));
        MessageSvc::Error("GetSampleName", (TString) "Inconsistent sample requirement and q2Bin of FitConfiguration", "EXIT_FAILURE");
    }

    TString _key = "";
    if (_sample != Sample::Custom) {
        switch (m_project) {
            case Prj::RKst:
                switch (_sample) {
                    case Sample::LL: _key = "Bd2Kst" + to_string(m_ana); break;
                    case Sample::JPsi: _key = "Bd2KstJPs" + to_string(m_ana); break;
                    case Sample::Psi: _key = "Bd2KstPsi" + to_string(m_ana); break;
                    case Sample::Comb:  _key = "";  break;  // No name for the Combinatorial shape !
                    case Sample::DataDrivenEMisID: _key = ""; break;  // No name for the DataDrivenEMisID shape !
                    case Sample::TemplateMisID_PIDe3: _key = ""; break;  // No name for the TemplateMisID_PIDe3 shape !
                    case Sample::CombSS: _key = "LPTSS";  break;
                    case Sample::Bs:
                        switch (m_q2bin) {
                            case Q2Bin::JPsi: _key = "Bs2KstJPs" + to_string(m_ana); break;
                            case Q2Bin::Psi:  _key = "Bs2KstPsi" + to_string(m_ana); break;
                            default: break;
                        }
                        break;
                    case Sample::Bs2Phi:
                        switch (m_q2bin) {
                            case Q2Bin::JPsi: _key = "Bs2PhiJPs" + to_string(m_ana); break;
                            case Q2Bin::Psi: _key = "Bs2PhiPsi" + to_string(m_ana); break;
                            default: break;
                        }
                        break;
                    case Sample::HadSwap:
                        switch (m_q2bin) {
                            case Q2Bin::JPsi: _key = "Bd2KstSwapJPs" + to_string(m_ana); break;
                            case Q2Bin::Psi:  _key = "Bd2KstSwapPsi" + to_string(m_ana); break;
                            default: break;
                        }
                        break;
                    case Sample::DSLC:
                            _key = "Bd2DNuKstNu" + to_string(m_ana); break;
                    case Sample::Psi2JPsX:
                        switch (m_q2bin) {
                            case Q2Bin::Psi:  _key = "Bd2KstPsiPiPiJPs" + to_string(m_ana); break;
                            case Q2Bin::JPsi: _key = "Bd2KstPsiPiPiJPs" + to_string(m_ana); break;
                            default: break;
                        }
                        break;
                    case Sample::Psi2JPsPiPi:
                        switch (m_q2bin) {
                            case Q2Bin::Psi:  _key = "Bu2KPsiPiPiJPs" + to_string(m_ana); break;
                            case Q2Bin::JPsi: _key = "Bu2KPsiPiPiJPs" + to_string(m_ana); break;
                            default: break;
                        }
                        break;                        
                    case Sample::Lb:
                        switch (m_q2bin) {
                            case Q2Bin::Low: _key = "Lb2pK" + to_string(m_ana); break;
                            case Q2Bin::Central: _key = "Lb2pK" + to_string(m_ana); break;
                            case Q2Bin::High: _key = "Lb2pK" + to_string(m_ana); break;
                            case Q2Bin::JPsi: _key = "Lb2pKJPs" + to_string(m_ana); break;
                            case Q2Bin::Psi: _key = "Lb2pKPsi" + to_string(m_ana); break;
                            default: break;
                        }
                        break;
                    case Sample::Leakage:
                        switch (m_q2bin) {
                            case Q2Bin::Central: _key = "Bd2KstJPs" + to_string(m_ana); break;
                            case Q2Bin::High: _key = "Bd2KstPsi" + to_string(m_ana); break;
                            case Q2Bin::JPsi: _key = "Bd2KstPsi" + to_string(m_ana); break;
                            case Q2Bin::Psi: _key = "Bd2KstJPs" + to_string(m_ana); break;
                            default: break;
                        }
                        break;
                    case Sample::PartRecoHad:
                        switch (m_q2bin) {
                            case Q2Bin::JPsi: _key = "Bu2KPiPiJPs" + to_string(m_ana); break;
                            case Q2Bin::Psi: _key = "Bu2KPiPiPsi" + to_string(m_ana); break;
                            default: break;
                        }
                        break;
                    case Sample::PartRecoK1: _key = "Bu2K1" + to_string(m_ana); break;
                    case Sample::PartRecoK2: _key = "Bu2K2" + to_string(m_ana); break;
                    case Sample::PartRecoH: [[fallthrough]];
                    case Sample::PartRecoL: [[fallthrough]];
                    case Sample::PartReco:
                        switch (m_q2bin) {
                            case Q2Bin::Low: _key = "Bu2KPiPi" + to_string(m_ana); break;
                            case Q2Bin::Central: _key = "Bu2KPiPi" + to_string(m_ana); break;
                            case Q2Bin::High: _key = "Bu2KPiPi" + to_string(m_ana); break;
                            case Q2Bin::JPsi: _key = "Bd2KstJPs" + to_string(m_ana); break; //Will grab always correct year from GetYearForSample and EventTypeCreation, but the cocktail making will update it 
                            case Q2Bin::Psi:  _key = "Bd2KstPsi" + to_string(m_ana); break; //Will grab always correct year from GetYearForSample and EventTypeCreation, but the cocktail making will update it 
                            default: break;
                        }
                        break;

                    case Sample::Data: _key = to_string(Sample::Data); break;

                    default: MessageSvc::Error("GetSampleName", (TString) "Project", to_string(m_project), "Sample", to_string(_sample), "not implemented", "EXIT_FAILURE"); break;
                }
                break;

            case Prj::RK:
                switch (_sample) {
                    case Sample::LL: _key = "Bu2K" + to_string(m_ana); break;
                    case Sample::JPsi: _key = "Bu2KJPs" + to_string(m_ana); break;
                    case Sample::Psi: _key = "Bu2KPsi" + to_string(m_ana); break;
                    // No name for the Combinatorial shape and DataDrivenEMisID !                    
                    case Sample::Comb: _key = "";   break;
                    case Sample::DataDrivenEMisID: _key = ""; break; 
                    case Sample::TemplateMisID_PIDe3: _key = ""; break; 
                    case Sample::DoubleMisID_PiPi: _key = ""; break; 
                    case Sample::DoubleMisID_KK: _key = ""; break; 
                    case Sample::CombSS: _key = "LPTSS";  break;
                    case Sample::Leakage:
                        switch (m_q2bin) {
                            case Q2Bin::Central: _key = "Bu2KJPs" + to_string(m_ana); break;
                            case Q2Bin::High: _key = "Bu2KPsi" + to_string(m_ana); break;
                            case Q2Bin::JPsi: _key = "Bu2KPsi" + to_string(m_ana); break;
                            case Q2Bin::Psi: _key = "Bu2KJPs" + to_string(m_ana); break;
                            default: break;
                        }
                        break;
                    case Sample::Psi2JPsX:
                        switch (m_q2bin) {
                            case Q2Bin::Psi:  _key = "Bu2KPsiPiPiJPs" + to_string(m_ana); break;
                            case Q2Bin::JPsi: _key = "Bu2KPsiPiPiJPs" + to_string(m_ana); break;
                            default: break;
                        }
                        break;
                    case Sample::DSLC:
                            _key = "Bu2DKNuNu" + to_string(m_ana); break;
                    case Sample::Lb:
                        switch (m_q2bin) {
                            case Q2Bin::Low: _key = "Lb2pK" + to_string(m_ana); break;
                            case Q2Bin::Central: _key = "Lb2pK" + to_string(m_ana); break;
                            case Q2Bin::High: _key = "Lb2pK" + to_string(m_ana); break;
                            case Q2Bin::JPsi: _key = "Lb2pKJPs" + to_string(m_ana); break;
                            case Q2Bin::Psi: _key = "Lb2pKPsi" + to_string(m_ana); break;
                            default: break;
                        }
                        break;
                    case Sample::Bs:
                        /*Attempt to model the Bs->Kst J/Psi part reco in RK*/
                        /*Bs -> Kst (J/Psi, Psi)*/
                        switch (m_q2bin) {
                            case Q2Bin::JPsi: _key = "Bd2KstJPs" + to_string(m_ana); break;
                            case Q2Bin::Psi: _key = "Bd2KstPsi" + to_string(m_ana); break;
                            default : break;
                        }
                        break;
                    case Sample::MisID:
                        switch (m_q2bin) {
                            case Q2Bin::JPsi: _key = "Bu2PiJPs" + to_string(m_ana); break;
                            case Q2Bin::Psi: _key = "Bu2PiJPs" + to_string(m_ana); break;
                            default: break;
                        }
                        break;
                    case Sample::PartRecoHad:
                        switch (m_q2bin) {
                            case Q2Bin::JPsi: _key = "Bd2KstJPs" + to_string(m_ana); break;
                            case Q2Bin::Psi: _key = "Bd2KstPsi" + to_string(m_ana); break;
                            default: break;
                        }
                        break;
                    case Sample::PartRecoH: [[fallthrough]];
                    case Sample::PartRecoL: [[fallthrough]];
                    case Sample::PartReco:
                        switch (m_q2bin) {
                            // case Q2Bin::Low: _key = "Bd2Kst" + to_string(m_ana); break;
                            // case Q2Bin::Central: _key = "Bd2Kst" + to_string(m_ana); break;
                            // case Q2Bin::High: _key = "Bd2Kst" + to_string(m_ana); break;
                            case Q2Bin::JPsi: _key = "Bu2KJPs" + to_string(m_ana); break; //Will grab always correct year from GetYearForSample and EventTypeCreation, but the cocktail making will update it 
                            case Q2Bin::Psi: _key =  "Bu2KPsi" + to_string(m_ana); break; //Will grab always correct year from GetYearForSample and EventTypeCreation, but the cocktail making will update it 
                            default: break;
                        }
                        break;
                    case Sample::Bd2Kst:
                        switch (m_q2bin) {
                            case Q2Bin::Low: _key = "Bd2Kst" + to_string(m_ana); break;
                            case Q2Bin::Central: _key = "Bd2Kst" + to_string(m_ana); break;
                            case Q2Bin::High: _key = "Bd2Kst" + to_string(m_ana); break;
                            // case Q2Bin::JPsi: _key = "Bu2KJPs" + to_string(m_ana); break; //Will grab always correct year from GetYearForSample and EventTypeCreation, but the cocktail making will update it 
                            // case Q2Bin::Psi: _key =  "Bu2KPsi" + to_string(m_ana); break; //Will grab always correct year from GetYearForSample and EventTypeCreation, but the cocktail making will update it 
                            default: break;
                        }
                        break;
                    case Sample::Bu2Kst:
                        switch (m_q2bin) {
                            case Q2Bin::Low: _key = "Bu2Kst" + to_string(m_ana); break;
                            case Q2Bin::Central: _key = "Bu2Kst" + to_string(m_ana); break;
                            case Q2Bin::High: _key = "Bu2Kst" + to_string(m_ana); break;
                            // case Q2Bin::JPsi: _key = "Bu2KJPs" + to_string(m_ana); break; //Will grab always correct year from GetYearForSample and EventTypeCreation, but the cocktail making will update it 
                            // case Q2Bin::Psi: _key =  "Bu2KPsi" + to_string(m_ana); break; //Will grab always correct year from GetYearForSample and EventTypeCreation, but the cocktail making will update it 
                            default: break;
                        }
                        break;
                    case Sample::BdBu:
                        switch (m_q2bin) {
                            case Q2Bin::Low: _key = "Bd2KPi" + to_string(m_ana); break;
                            case Q2Bin::Central: _key = "Bd2KPi" + to_string(m_ana); break;
                            case Q2Bin::High: _key = "Bd2KPi" + to_string(m_ana); break;
                            case Q2Bin::JPsi: _key = "Bd2KPiJPs" + to_string(m_ana); break;
                            case Q2Bin::Psi: _key = "Bd2KstPsi" + to_string(m_ana); break;
                            default: break;
                        }
                        break;
                    case Sample::KEtaPrime : 
                        switch (m_q2bin) {
                            case Q2Bin::Low: _key     = "Bu2KEtaPrimeG" + to_string(m_ana); break;
                            case Q2Bin::Central: _key = "Bu2KEtaPrimeG" + to_string(m_ana); break;                          
                            default: break;
                        }
                        break;
                    case Sample::Data: _key = to_string(Sample::Data); break;

                    default: MessageSvc::Error("GetSampleName", (TString) "Project", to_string(m_project), "Sample", to_string(_sample), "not implemented", "EXIT_FAILURE"); break;
                }
                break;

            case Prj::RPhi:
                switch (_sample) {
                    case Sample::LL: _key = "Bs2Phi" + to_string(m_ana); break;
                    case Sample::JPsi: _key = "Bs2PhiJPs" + to_string(m_ana); break;
                    case Sample::Psi: _key = "Bs2PhiPsi" + to_string(m_ana); break;

                    case Sample::Comb:
                        _key = "";   // No name for the Combinatorial shape !
                        break;
                    case Sample::CombSS: _key = "LPTSS";  break;
                    case Sample::Bd:
                        switch (m_q2bin) {
                            case Q2Bin::JPsi: _key = "Bd2PhiJPs" + to_string(m_ana); break;
                            case Q2Bin::Psi: _key = "Bd2PhiPsi" + to_string(m_ana); break;
                            default: break;
                        }
                        break;
                    case Sample::Lb:
                        switch (m_q2bin) {
                            case Q2Bin::Low: _key = "Lb2pK" + to_string(m_ana); break;
                            case Q2Bin::Central: _key = "Lb2pK" + to_string(m_ana); break;
                            case Q2Bin::High: _key = "Lb2pK" + to_string(m_ana); break;
                            case Q2Bin::JPsi: _key = "Lb2pKJPs" + to_string(m_ana); break;
                            case Q2Bin::Psi:
                                _key = "Lb2pKJPs" + to_string(m_ana);
                                MessageSvc::Warning("GetSampleName", (TString) "Lb2pKPsi not existing ... using Lb2pKJPs instead");   // NEED TO TWEAK MASS
                                break;
                            default: break;
                        }
                        break;
                    case Sample::Leakage:
                        switch (m_q2bin) {
                            case Q2Bin::Central: _key = "Bs2PhiJPs" + to_string(m_ana); break;
                            case Q2Bin::High: _key = "Bs2PhiPsi" + to_string(m_ana); break;
                            default: break;
                        }
                        break;
                    case Sample::PartReco:
                        switch (m_q2bin) {
                            case Q2Bin::JPsi: _key = "Lb2XJPs" + to_string(m_ana); break;
                            case Q2Bin::Psi: _key = "Lb2XJPs" + to_string(m_ana); break;
                            default: break;
                        }
                        break;

                    case Sample::Data: _key = to_string(Sample::Data); break;

                    default: MessageSvc::Error("GetSampleName", (TString) "Project", to_string(m_project), "Sample", to_string(_sample), "not implemented", "EXIT_FAILURE"); break;
                }
                break;

            case Prj::RL:
                switch (_sample) {
                    case Sample::LL: _key = "Lb2L" + to_string(m_ana); break;
                    case Sample::JPsi: _key = "Lb2LJPs" + to_string(m_ana); break;
                    case Sample::Psi: _key = "Lb2LPsi" + to_string(m_ana); break;

                    case Sample::Comb:
                        _key = "";   // No name for the Combinatorial shape !
                        break;
                    case Sample::CombSS: _key = "LPTSS";  break;
                    case Sample::Bd:
                        switch (m_q2bin) {
                            case Q2Bin::Low: _key = "Bd2KS" + to_string(m_ana); break;
                            case Q2Bin::Central: _key = "Bd2KS" + to_string(m_ana); break;
                            case Q2Bin::High: _key = "Bd2KS" + to_string(m_ana); break;
                            case Q2Bin::JPsi: _key = "Bd2KSJPs" + to_string(m_ana); break;
                            case Q2Bin::Psi: _key = "Bd2KSPsi" + to_string(m_ana); break;
                            default: break;
                        }
                        break;
                    case Sample::Leakage:
                        switch (m_q2bin) {
                            case Q2Bin::Central: _key = "Lb2LJPs" + to_string(m_ana); break;
                            case Q2Bin::High: _key = "Lb2LPsi" + to_string(m_ana); break;
                            default: break;
                        }
                        break;

                    case Sample::Data: _key = to_string(Sample::Data); break;

                    default: MessageSvc::Error("GetSampleName", (TString) "Project", to_string(m_project), "Sample", to_string(_sample), "not implemented", "EXIT_FAILURE"); break;
                }
                break;

            case Prj::RKS:
                switch (_sample) {
                    case Sample::JPsi: _key = "Bd2KSJPs" + to_string(m_ana); break;

                    case Sample::Comb:
                        _key = "";   // No name for the Combinatorial shape !
                        break;
                    case Sample::CombSS: _key = "LPTSS";  break;
                    case Sample::Bs:
                        switch (m_q2bin) {
                            case Q2Bin::JPsi: _key = "Bs2KSJPs" + to_string(m_ana); break;
                            default: break;
                        }
                        break;
                    case Sample::Lb:
                        switch (m_q2bin) {
                            case Q2Bin::JPsi: _key = "Lb2LJPs" + to_string(m_ana); break;
                            default: break;
                        }
                        break;

                    case Sample::Data: _key = to_string(Sample::Data); break;

                    default: MessageSvc::Error("GetSampleName", (TString) "Project", to_string(m_project), "Sample", to_string(_sample), "not implemented", "EXIT_FAILURE"); break;
                }
                break;

            default: MessageSvc::Error("GetSampleName", (TString) "Project", to_string(m_project), "not implemented", "EXIT_FAILURE"); break;
        }
    }
    return CleanString(_key);
}

ConfigHolder FitConfiguration::GetSignalConfigHolder() const noexcept {
    if (m_componentsAndOptions.find(m_signalSample) == m_componentsAndOptions.end() && !m_hasBrem) MessageSvc::Error("GetSignalConfigHolder", (TString) "Cannot find", to_string(m_signalSample), "among FitComponents", "EXIT_FAILURE");
    TString      _sampleID = GetSampleName(m_signalSample);
    ConfigHolder _config(m_project, m_ana, _sampleID, m_q2bin, m_year, m_polarity, m_trigger, Brem::All, m_track);
    return _config;
}

EventTypeAndOption FitConfiguration::GetSignal(const Brem & _brem) const noexcept {
    MessageSvc::Info(Color::Cyan, "FitConfiguration", m_key, "GetSignal", to_string(m_signalSample), m_hasBrem ? (TString) "HasBrem" : (TString) "HasNoBrem", _brem == Brem::All ? "All" : to_string(_brem));
    TString _name = to_string(m_signalSample);
    if (_brem != Brem::All) _name += SettingDef::separator + to_string(_brem);
    if (m_eventTypeAndOptions.find(_name) == m_eventTypeAndOptions.end()) { MessageSvc::Error("GetSignal", _name, "not in map", "EXIT_FAILURE"); }
    return m_eventTypeAndOptions.at(_name);
}

void FitConfiguration::GenerateSignal(const Brem & _brem) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitConfiguration", m_key, "GenerateSignal", to_string(m_signalSample), m_hasBrem ? (TString) "HasBrem" : (TString) "HasNoBrem", _brem == Brem::All ? "All" : to_string(_brem));

    TString _name = to_string(m_signalSample);
    if (_brem != Brem::All) _name += SettingDef::separator + to_string(_brem);
    if (m_eventTypeAndOptions.find(_name) != m_eventTypeAndOptions.end()) {
        MessageSvc::Warning("GenerateSignal", _name, "already not in map", "SKIPPING");
        return;
    }

    if (m_componentsAndOptions.find(m_signalSample) == m_componentsAndOptions.end() && !m_hasBrem) MessageSvc::Error("GenerateSignal", (TString) "Cannot find", to_string(m_signalSample), "among FitComponents", "EXIT_FAILURE");

    TString _sampleID = GetSampleName(m_signalSample);

    TString _pdfType = "";
    if (m_hasBrem) {
        if (_brem != Brem::All) _pdfType = to_string(get< 0 >(m_componentsAndOptionsBrem.find(_brem)->second));
        if (_brem == Brem::All) _pdfType = to_string(PdfType::RooAbsPDF);
    } else {
        _pdfType = to_string(get< 0 >(m_componentsAndOptions.find(m_signalSample)->second));
    }

    TString _pdfOption = "";
    if (m_hasBrem) {
        if (_brem != Brem::All) _pdfOption = get< 1 >(m_componentsAndOptionsBrem.find(_brem)->second);
        if (_brem == Brem::All) _pdfOption = "";
    } else {
        _pdfOption = get< 1 >(m_componentsAndOptions.find(m_signalSample)->second);
    }

    TString _cutOption = "";
    if (m_hasBrem) {
        if (_brem != Brem::All) _cutOption = get< 2 >(m_componentsAndOptionsBrem.find(_brem)->second);
        if (_brem == Brem::All) _cutOption = "";
    } else {
        _cutOption = get< 2 >(m_componentsAndOptions.find(m_signalSample)->second);
    }

    TString _weightOption = "";
    if (m_hasBrem) {
        if (_brem != Brem::All) _weightOption = get< 3 >(m_componentsAndOptionsBrem.find(_brem)->second);
        if (_brem == Brem::All) _weightOption = "";
    } else {
        _weightOption = get< 3 >(m_componentsAndOptions.find(m_signalSample)->second);
    }

    TString _tupleOption = "";
    if (m_hasBrem) {
        if (_brem != Brem::All) _tupleOption = get< 4 >(m_componentsAndOptionsBrem.find(_brem)->second);
        if (_brem == Brem::All) _tupleOption = "";
    } else {
        _tupleOption = get< 4 >(m_componentsAndOptions.find(m_signalSample)->second);
    }

    if ((_pdfType == to_string(PdfType::StringToPDF)) || (_pdfType == to_string(PdfType::RooAbsPDF))) {
        _sampleID     = "";
        _cutOption    = "";
        _weightOption = "";
        _tupleOption  = "";
    } else {        
        if (_cutOption != "no") {
            if (!_cutOption.Contains("-tm")) {
                _cutOption += "-tmSig";
                MessageSvc::Warning("GenerateSignal", (TString) "Adding TruthMatching", _cutOption);
            }
            if (!_cutOption.Contains("-isSingle") && !_cutOption.Contains("-noIsSingle")) {
                if (_tupleOption == "cre" || _tupleOption.Contains("cre[")) {
                    //TODO: understand the role of m-candidates removal here for MC fits, should be negligible?
                    _cutOption += "-isSingle";
                    MessageSvc::Warning("GenerateSignal", (TString) "Adding IsSingle", _cutOption);
                }
            }
        }
    }

    MessageSvc::Line();
    MessageSvc::Info("Name", _sampleID);
    MessageSvc::Info("Type", _pdfType);
    MessageSvc::Info("PdfOption", _pdfOption);
    MessageSvc::Info("CutOption", _cutOption);
    MessageSvc::Info("WeightOption", _weightOption);
    MessageSvc::Info("TupleOption", _tupleOption);

    if (SettingDef::Fit::option.Contains("chainexctrgsig")) SettingDef::Tuple::chainexctrg = true;


    //Do this when fitting since PHYS tag MC is not a good proxy!.    
    //TODO: helping the HLT fits to work....
    if( m_track != Track::All && !_cutOption.Contains("noTRACK") && _cutOption.Contains("TAGHLTPHYS") ) _cutOption.ReplaceAll("TAGHLTPHYS","TAGHLTORALL"); 
    if( m_track == Track::PRB && !_cutOption.Contains("noTRACK") && _cutOption.Contains("TAGHLTPHYS") ){ 
      auto _track = Track::TAG;
      m_eventTypeAndOptions[_name] = make_tuple(EventType(m_project, m_ana, _sampleID, m_q2bin, m_year, m_polarity, m_trigger, _brem, _track, _cutOption, _weightOption, _tupleOption, false), _pdfType, _pdfOption);
      SettingDef::Tuple::chainexctrg = false;
      return;
    }

    m_eventTypeAndOptions[_name] = make_tuple(EventType(m_project, m_ana, _sampleID, m_q2bin, m_year, m_polarity, m_trigger, _brem, m_track, _cutOption, _weightOption, _tupleOption, false), _pdfType, _pdfOption);

    SettingDef::Tuple::chainexctrg = false;

    return;
}

EventTypeAndOption FitConfiguration::GetBackground(const Sample & _sample) const noexcept {
    MessageSvc::Info(Color::Cyan, "FitConfiguration", m_key, "GetBackground", to_string(_sample));
    TString _name = to_string(_sample);
    if (m_eventTypeAndOptions.find(_name) == m_eventTypeAndOptions.end()) { MessageSvc::Error("GenerateBackground", _name, "not in map", "EXIT_FAILURE"); }
    return m_eventTypeAndOptions.at(_name);
}

void FitConfiguration::GenerateBackground(const Sample & _sample) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitConfiguration", m_key, "GenerateBackground", to_string(_sample));

    TString _name = to_string(_sample);
    if (m_eventTypeAndOptions.find(_name) != m_eventTypeAndOptions.end()) {
        MessageSvc::Warning("GenerateBackground", _name, "already not in map", "SKIPPING");
        return;
    }

    if (_sample == m_signalSample) MessageSvc::Error("GenerateBackground", (TString) "BackgroundSamples", to_string(_sample), "== SignalSample", to_string(m_signalSample), "EXIT_FAILURE");

    if (m_componentsAndOptions.find(_sample) == m_componentsAndOptions.end()) MessageSvc::Error("GenerateBackground", (TString) "Cannot find", to_string(_sample), "among FitComponents", "EXIT_FAILURE");

    TString _sampleID  = GetSampleName(_sample);
    TString _pdfType   = to_string(get< 0 >(m_componentsAndOptions.find(_sample)->second));
    TString _pdfOption = get< 1 >(m_componentsAndOptions.find(_sample)->second);

    if (_sample == Sample::Custom) {
        _sampleID = RemoveStringAfter(RemoveStringBefore(_pdfOption, "custom["), "]");
        _sampleID.ReplaceAll("-custom[", "").ReplaceAll("custom[", "").ReplaceAll("]", "");
    }

    TString _cutOption    = get< 2 >(m_componentsAndOptions.find(_sample)->second);
    TString _weightOption = get< 3 >(m_componentsAndOptions.find(_sample)->second);
    TString _tupleOption  = get< 4 >(m_componentsAndOptions.find(_sample)->second);

    if ((_pdfType == to_string(PdfType::StringToPDF)) || (_pdfType == to_string(PdfType::RooAbsPDF))) {
        _sampleID     = "";
        _cutOption    = "";
        _weightOption = "";
        _tupleOption  = "";
    } else {
        if (_cutOption != "no") {
            if (!_cutOption.Contains("-tm")) {
                if (_sample == Sample::Leakage){ 
                    _cutOption += "-tmSig";
                }else{
                    _cutOption += "-tmBkg";
                }
                MessageSvc::Warning("GenerateBackground", (TString) "Adding TruthMatching", _cutOption);
            }
        }
    }

    MessageSvc::Line();
    MessageSvc::Info("Name", _sampleID);
    MessageSvc::Info("Type", _pdfType);
    MessageSvc::Info("PdfOption", _pdfOption);
    MessageSvc::Info("CutOption", _cutOption);
    MessageSvc::Info("WeightOption", _weightOption);
    MessageSvc::Info("TupleOption", _tupleOption);

    Q2Bin _q2bin = m_q2bin;

    Year _year = GetYearForSample(_sampleID, m_year, m_project);
    
    if (_pdfOption.Contains("combRuns")) {
        /* COMB RUNS REMOVE A LOT OF CUTS! DO WE REALLY WANT THIS TO HAPPEN*/
        MessageSvc::Warning("GenerateBackground", (TString) "Combine all Runs");
        _year = Year::All;
        if (!_cutOption.Contains("noSPD")) _cutOption += "-noSPD";
        if (!_cutOption.Contains("noHLT1")) _cutOption += "-noHLT1";
        if (!_cutOption.Contains("noHLT2")) _cutOption += "-noHLT2";
        if (!_cutOption.Contains("noPID")) _cutOption += "-noPID";
        if (!_cutOption.Contains("noBKG")) _cutOption += "-noBKG";        
        MessageSvc::Warning("GenerateBackground", (TString) "Dropping incompatible cuts", _cutOption);
    }
    Trigger _trigger = m_trigger;
    if (SettingDef::Fit::option.Contains("chainexctrgbkg") || _pdfOption.Contains("chainexctrgbkg") ){
        SettingDef::Tuple::chainexctrg = true;
        if (!SettingDef::Fit::option.Contains("chainexctrgsig")) _trigger = Trigger::All;
        if ( _pdfOption.Contains("chainexctrgbkg")) _trigger = Trigger::All;
    }
    /* TupleProcess parsing usage */
    TString _proVer = SettingDef::Tuple::proVer;
    TString _proVerUse = _proVer; 
    if(_tupleOption.Contains("pro") && _tupleOption.Contains("[")){
        _proVerUse = StripStringBetween(_proVer, "pro[", "]");
    }
    if(_tupleOption.Contains("pro")){
        MessageSvc::Warning("TupleProcess used version : ", _proVerUse);
    }
    SettingDef::Tuple::proVer = _proVerUse;
    m_eventTypeAndOptions[_name] = make_tuple(EventType(m_project, m_ana, _sampleID, _q2bin, _year, m_polarity, _trigger, m_brem, m_track, _cutOption, _weightOption, _tupleOption, false), _pdfType, _pdfOption);

    SettingDef::Tuple::proVer = _proVer;
    
    SettingDef::Tuple::chainexctrg = false;
    return;
}

EventTypeAndOption FitConfiguration::GetData() const noexcept {
    MessageSvc::Info(Color::Cyan, "FitConfiguration", m_key, "GetData");
    TString _name = to_string(Sample::Data);
    if (m_eventTypeAndOptions.find(_name) == m_eventTypeAndOptions.end()) { MessageSvc::Error("GetData", _name, "not in map", "EXIT_FAILURE"); }
    return m_eventTypeAndOptions.at(_name);
}

void FitConfiguration::GenerateData() {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitConfiguration", m_key, "GenerateData");

    TString _name = to_string(Sample::Data);
    if (m_eventTypeAndOptions.find(_name) != m_eventTypeAndOptions.end()) {
        MessageSvc::Warning("GenerateData", _name, "already not in map", "SKIPPING");
        return;
    }

    TString _sampleID  = GetSampleName(Sample::Data);
    TString _pdfOption = "";

    TString _cutOption    = SettingDef::Cut::option;
    TString _weightOption = SettingDef::Weight::option;
    TString _tupleOption  = SettingDef::Tuple::option;

    if (_cutOption != "no"  && !SettingDef::Fit::LPTMCandidates) {
        if (!_cutOption.Contains("-isSingle") && !_cutOption.Contains("-noIsSingle")) {
            if (SettingDef::Tuple::option.Contains("cre")) {
                MessageSvc::Warning("GenerateData", (TString) "Adding IsSingle", _cutOption);
                _cutOption += "-isSingle";
            }
        }
    }else{
        if(SettingDef::Fit::LPTMCandidates ){
            MessageSvc::Warning("USING Multiple Candidate removal on the fly");
        }else{
            MessageSvc::Warning("NOT USING Multiple Candidate removal At ALL ");
        }        
    }
    _weightOption = SettingDef::Weight::useBS ?  _weightOption : ""; //!!!! 

    MessageSvc::Line();
    MessageSvc::Info("Name", _sampleID);
    // MessageSvc::Info("Type", _pdfType);
    // MessageSvc::Info("PdfOption", _pdfOption);
    MessageSvc::Info("CutOption", _cutOption);
    MessageSvc::Info("WeightOption", _weightOption);
    MessageSvc::Info("TupleOption", _tupleOption);

    if (SettingDef::Fit::option.Contains("chainexctrgsig")) SettingDef::Tuple::chainexctrg = true;

    m_eventTypeAndOptions[_name] = make_tuple(EventType(m_project, m_ana, _sampleID, m_q2bin, m_year, m_polarity, m_trigger, m_brem, m_track, _cutOption, _weightOption, _tupleOption, false), _sampleID, _pdfOption);

    SettingDef::Tuple::chainexctrg = false;

    return;
}

vector< Sample > FitConfiguration::GetSamplesWithType(const PdfType & _pdfType) const noexcept {
    vector< Sample > _samplesWithType;
    for (auto & _component : m_componentsAndOptions) {
        if (_pdfType == get< 0 >(_component.second)) _samplesWithType.push_back(_component.first);
    }
    return _samplesWithType;
}

PdfType FitConfiguration::GetTypeFromSample(const Sample & _sample) const noexcept {
    PdfType _type = PdfType::Empty;
    for (auto & _component : m_componentsAndOptions) {
        if (_sample == _component.first) {
            _type = get< 0 >(_component.second);
            break;
        }
    }
    /* thow error if not found ? if(_type == PdfType::Empty){ }*/
    return _type;
}

TString FitConfiguration::GetKeyWithBrem(const Brem & _brem) const noexcept {
    if (!m_hasBrem) MessageSvc::Error("GetKeyWithBrem", (TString) "No brem", "EXIT_FAILURE");
    return m_key + SettingDef::separator + to_string(_brem);
}

Brem FitConfiguration::GetBremFromKey(const TString & _key) const noexcept {
    if (!m_hasBrem) MessageSvc::Error("GetBremFromKey", (TString) "Only valid for HasBrem", "EXIT_FAILURE");
    if (_key.Contains(to_string(Brem::G0))) return Brem::G0;
    if (_key.Contains(to_string(Brem::G1))) return Brem::G1;
    if (_key.Contains(to_string(Brem::G2))) return Brem::G2;
    MessageSvc::Error("GetBremFromKey", (TString) "No Brem found in", _key, "EXIT_FAILURE");
    return Brem::All;
}

void FitConfiguration::Print() const noexcept {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitConfiguration", m_key);
    MessageSvc::Line();
    (static_cast< const ConfigHolder & >(*this)).PrintInline();
    MessageSvc::Line();
    if (m_varName != "") MessageSvc::Info("VarName", m_varName);
    MessageSvc::Info("ConstrainedMass", m_constrainedMass);
    MessageSvc::Info("Binned MC", m_binnedMC);
    MessageSvc::Info("Binned CL", m_binnedCL);
    MessageSvc::Info("HasBrem", m_hasBrem);
    // MessageSvc::Info("EventTypes", to_string(m_eventTypeAndOptions.size()));
    MessageSvc::Info("Components", to_string(m_componentsAndOptions.size()));
    for (auto & _component : m_componentsAndOptions) MessageSvc::Info(to_string(_component.first), to_string(get< 0 >(_component.second)), get< 1 >(_component.second), get< 2 >(_component.second), get< 3 >(_component.second), get< 4 >(_component.second));
    MessageSvc::Info("Brem Components", to_string(m_componentsAndOptionsBrem.size()));
    for (auto & _component : m_componentsAndOptionsBrem) MessageSvc::Info(to_string(_component.first), to_string(get< 0 >(_component.second)), get< 1 >(_component.second), get< 2 >(_component.second), get< 3 >(_component.second), get< 4 >(_component.second));
    if (m_extraRange != "") { MessageSvc::Info("ExtraRange", m_extraRange); }
    MessageSvc::Line();
    return;
}

void FitConfiguration::PrintContent() const noexcept {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitConfiguration", m_key, "PrintContent");
    MessageSvc::Line();
    cout << GREEN;
    cout << m_key << "  [KEY]" << endl;
    cout << "\t Components" << endl;
    for (auto & _component : m_componentsAndOptions) cout << "\t\t" << to_string(_component.first) << " | " << to_string(get< 0 >(_component.second)) << " | " << get< 1 >(_component.second) << endl;
    if (m_hasBrem) {
        cout << "\t Brem Components" << endl;
        for (auto & _component : m_componentsAndOptionsBrem)
            cout << "\t\t" << to_string(_component.first) << " | " << to_string(get< 0 >(_component.second))
                 << " "
                    " "
                 << get< 1 >(_component.second) << endl;
    }
    cout << RESET;
    return;
}

void FitConfiguration::AddExtraRanges(const vector< tuple< TString, double, double > > & _extraRanges) noexcept {
    if (_extraRanges.size() == 0) return;
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitConfiguration", m_key, "AddExtraRanges", to_string(_extraRanges.size()));
    MessageSvc::Line();

    m_extraRange = "";
    for (const auto & _range : _extraRanges) { m_var->setRange(get< 0 >(_range), min(get< 1 >(_range), get< 2 >(_range)), max(get< 1 >(_range), get< 2 >(_range))); }
    // build the rangestring for the fit being "range1,range2,....";
    for (int i = 0; i < _extraRanges.size(); ++i) {
        if (i == 0) {
            m_extraRange += get< 0 >(_extraRanges[i]);
        } else {
            m_extraRange += TString(",");
            m_extraRange += get< 0 >(_extraRanges[i]);
        }
    }
    return;
}

bool FitConfiguration::HasInitialFraction(Sample _sample) {
    bool _hasInitialFraction = get< 5 >(m_componentsAndOptions[_sample]) != m_noYieldFraction;
    return _hasInitialFraction;
}

double FitConfiguration::GetInitialFraction(Sample _sample) {
    double _initialFraction = get< 5 >(m_componentsAndOptions[_sample]);
    return _initialFraction;
}
//======= Labels stuff ======//
void FitConfiguration::SetLabels( const map< Sample, TString> & _labels){
    m_labels = _labels;
}
TString FitConfiguration::GetLabel( Sample & _sample) const{
    if( m_labels.find(_sample) != m_labels.end()){
        return m_labels.at(_sample);
    }
    return "";
}
map<Sample,TString> FitConfiguration::GetLabels()const{
    return m_labels;
}
map<TString, TString> FitConfiguration::GetLabelsNamed() const {
    map<TString, TString> _labelsReturn;
    for( const auto & el : m_labels){
        _labelsReturn[to_string(el.first)] = el.second;
    }
    return _labelsReturn;
}
bool FitConfiguration::HasLabels()const {
    return m_labels.size() != 0; 
}

//======= Colors stuff ======//
void FitConfiguration::SetColors( const map< Sample, TString> & _colors){
    //String is an Hex string 
    for( auto & colorHex : _colors){
      TString _hexColor = colorHex.second.BeginsWith("#")? colorHex.second : TString("#")+colorHex.second.Data();
      //if(colorHex.BeginsWith("#") 
      //TString _hexColor = TString("#")+colorHex.second;
      m_colors[ colorHex.first] = TColor::GetColor(_hexColor.Data());
    }
    return;
}
Int_t FitConfiguration::GetColor( Sample & _sample) const{
    if( m_colors.find(_sample) != m_colors.end()){
        return m_colors.at(_sample);
    }
    return (Int_t)kBlack;
}
map<Sample,Int_t> FitConfiguration::GetColors()const{
    return m_colors;
}
map<TString, Int_t> FitConfiguration::GetColorsNamed() const {
    map<TString, Int_t> _colorsReturn;
    for( const auto & el : m_colors){
        _colorsReturn[to_string(el.first)] = el.second;
    }
    return _colorsReturn;
}
bool FitConfiguration::HasColors()const {
    return m_colors.size() != 0; 
}


pair<double, double> FitConfiguration::RangeDataFit( )const {
    double minV = m_minCL;
    double maxV = m_maxCL;
    return std::pair<double, double>(minV, maxV);
}
#endif
