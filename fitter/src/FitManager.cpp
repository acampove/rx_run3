#ifndef FITMANAGER_CPP
#define FITMANAGER_CPP

#include "FitManager.hpp"

#include "ConstDef.hpp"
#include "SettingDef.hpp"

#include "FitterTool.hpp"
#include "HelperProcessing.hpp"
#include "vec_extends.h"

ClassImp(FitManager)

FitManager::FitManager(TString _name, vector< FitConfiguration > _configurations, TString _option) {
    if (SettingDef::debug.Contains("FM")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("FitManager", (TString) "FitConfiguration");
    m_name   = _name;
    m_name   = CleanString(m_name);
    m_option = _option;
    Check();
    for (const auto & _configuration : _configurations) AddFitConfiguration(_configuration);
    m_parameterPool = RXFitter::GetParameterPool();
    cout << WHITE << *this << RESET << endl;
}

FitManager::FitManager(TString _name, TString _option) {
    if (SettingDef::debug.Contains("FM")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("FitManager", (TString) "TString");
    m_name   = _name;
    m_name   = CleanString(m_name);
    m_option = _option;
    Check();
    m_parameterPool = RXFitter::GetParameterPool();
    cout << WHITE << *this << RESET << endl;
}

FitManager::FitManager(const FitManager & _fitManager) {
    if (SettingDef::debug.Contains("FM")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("FitManager", (TString) "FitManager");
    m_name             = _fitManager.Name();
    m_name             = CleanString(m_name);
    m_option           = _fitManager.Option();
    m_isInitialized    = _fitManager.IsInitialized();
    m_isLoaded         = _fitManager.IsLoaded();
    m_isReduced        = _fitManager.IsReduced();
    m_holdersMM        = _fitManager.HoldersMM();
    m_holdersEE        = _fitManager.HoldersEE();
    m_holdersEEBrem    = _fitManager.HoldersEEBrem();
    m_configurationsEE = _fitManager.ConfigurationsEE();
    m_configurationsMM = _fitManager.ConfigurationsMM();
    m_parameterPool    = RXFitter::GetParameterPool();
    if (_fitManager.Fitter() != nullptr) m_fitter = static_cast< FitterTool * >(_fitManager.Fitter());
    // cout << WHITE << *this << RESET << endl;
}

FitManager::FitManager(TString _managerName, TString _option, TString _name, TString _dir) {
    if (SettingDef::debug.Contains("FM")) SetDebug(true);
    m_name = _managerName;
    m_name = CleanString(m_name);
    if (m_debug) MessageSvc::Debug("FitManager", _managerName, "LoadFromDisk");
    LoadFromDisk(_name, _dir);
    m_parameterPool = RXFitter::GetParameterPool();
    if (_option != m_option) {
        MessageSvc::Line();
        MessageSvc::Info(Color::Cyan, "FitManager", m_name, "Resetting fit options");
        MessageSvc::Info("OLD", m_option);
        MessageSvc::Info("NEW", _option);
        MessageSvc::Line();
        m_option = _option;
        cout << WHITE << *this << RESET << endl;
    }
}

ostream & operator<<(ostream & os, const FitManager & _fitManager) {
    os << WHITE;
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "FitManager");
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "name", _fitManager.Name());
    MessageSvc::Print((ostream &) os, "option", _fitManager.Option());
    MessageSvc::Print((ostream &) os, "loaded", to_string(_fitManager.IsLoaded()));
    MessageSvc::Print((ostream &) os, "reduced", to_string(_fitManager.IsReduced()));
    // MessageSvc::Print((ostream &) os, "parameters",  to_string(_fitManager.Parameters().Parameters().size()));
    if (_fitManager.HoldersMM().size() != 0) MessageSvc::Print((ostream &) os, "N MM holders", to_string(_fitManager.HoldersMM().size()));
    if (_fitManager.HoldersEE().size() != 0) MessageSvc::Print((ostream &) os, "N EE holders", to_string(_fitManager.HoldersEE().size()));
    if (_fitManager.HoldersEEBrem().size() != 0) MessageSvc::Print((ostream &) os, "N EE holders brem", to_string(_fitManager.HoldersEEBrem().size()));
    /*
    for (const auto & _mmholders : _fitManager.HoldersMM()) {
        MessageSvc::Print((ostream &) os, TString("varMM for Holder KEY ") + _mmholders.first);
        _mmholders.second.Configuration().Var()->Print("v");
    }
    for (const auto & _eeholders : _fitManager.HoldersEE()) {
        MessageSvc::Print((ostream &) os, TString("varMM for Holder KEY ") + _eeholders.first);
        _eeholders.second.Configuration().Var()->Print("v");
    }
    */
    MessageSvc::Line(os);
    os << RESET;
    os << "\033[F";
    return os;
}

bool FitManager::Check() const noexcept {
    for (auto _opt : TokenizeString(m_option, SettingDef::separator)) {
        _opt = RemoveStringAfter(_opt, "[");
        if (!CheckVectorContains(SettingDef::AllowedConf::FitOptions, _opt)) {
            cout << RED << *this << RESET << endl;
            MessageSvc::Error("FitManager", "\"" + _opt + "\"", "option not in SettingDef::AllowedConf::FitOptions", "EXIT_FAILURE");
        }
    }
    return false;
}

void FitManager::Prepare() {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitManager", m_name, "Prepare");
    MessageSvc::Line();

    for (const FitConfiguration & _configuration : m_configurationsMM) {
        MessageSvc::Info("Prepare", (TString) "Creating HoldersMM [", _configuration.GetKey(), "]  [KEY]");
        FitHolder _holder(_configuration.GetKey(), _configuration, m_option);
        Prepare(_holder, _configuration);
        if (!m_option.Contains("drysig")) AddBackgrounds(_holder, _configuration);
        m_holdersMM[_configuration.GetKey()] = _holder;
    }

    for (const FitConfiguration & _configuration : m_configurationsEE) {
        MessageSvc::Info("Prepare", (TString) "Creating HoldersEE [", _configuration.GetKey(), "]  [KEY]");
        FitHolder _holder(_configuration.GetKey(), _configuration, m_option);
        if (_configuration.HasBrem()) {
            PrepareBrem(_holder, _configuration);
        } else {
            Prepare(_holder, _configuration);
        }
        if (!m_option.Contains("drysig")) AddBackgrounds(_holder, _configuration);
        m_holdersEE[_configuration.GetKey()] = _holder;
    }
    return;
}

void FitManager::Prepare(FitHolder & _holder, const FitConfiguration & _configuration) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitManager", m_name, "Prepare", _holder.Name(), _configuration.GetKey());
    MessageSvc::Line();

    EventTypeAndOption _info = _configuration.GetSignal(_configuration.GetBrem());
    _holder.CreateSignal(get< 0 >(_info), to_string(_configuration.SignalSample()), get< 1 >(_info), get< 2 >(_info));
    m_parameterPool->AddYieldParameter(FitParameterConfig::GetSignalConfig(_configuration), _holder.SignalYield());
    return;
}

void FitManager::PrepareBrem(FitHolder & _holder, const FitConfiguration & _configuration) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitManager", m_name, "PrepareBrem", _holder.Name(), _configuration.GetKey());
    MessageSvc::Line();

    if (!_configuration.HasBrem()) MessageSvc::Error("PrepareBrem", (TString) "Only for FitConfiguration with the Signal-XG specified", "EXIT_FAILURE");
    if ((_configuration.GetAna() != Analysis::EE) && (_configuration.GetAna() != Analysis::ME)) MessageSvc::Error("PrepareBrem", (TString) "Only for FitConfiguration being Ana = EE or ME type", "EXIT_FAILURE");

    vector< Brem > _brems = {Brem::G0, Brem::G1, Brem::G2};
    if (_configuration.GetAna() == Analysis::ME) _brems = {Brem::G0, Brem::G1};
    for (const auto & _brem : _brems) {
        MessageSvc::Line();
        MessageSvc::Info("PrepareBrem", _configuration.GetKeyWithBrem(_brem));
        MessageSvc::Line();

        EventTypeAndOption _info = _configuration.GetSignal(_brem);

        MessageSvc::Info("PrepareBrem", get< 1 >(_info), get< 2 >(_info));

        FitHolder _fh(_configuration.GetKeyWithBrem(_brem), _configuration, m_option);
        _fh.CreateSignal(get< 0 >(_info), to_string(_configuration.SignalSample()), get< 1 >(_info), get< 2 >(_info));
        m_holdersEEBrem[_configuration.GetKey()][_configuration.GetKeyWithBrem(_brem)] = _fh;
        if (!m_option.Contains("drysig")) {
            if (_configuration.HasPdfType(PdfType::SignalCopy)) {
                vector< Sample > _signalCopies = _configuration.GetSignalCopies();
                for (const auto & _sigCopy : _signalCopies) {
                    MessageSvc::Info("PrepareBrem", to_string(_sigCopy));
                    EventTypeAndOption _infoCopy = _configuration.GetBackground(_sigCopy);
                    m_holdersEEBrem[_configuration.GetKey()][_configuration.GetKeyWithBrem(_brem)].CreateBackgroundFromSignal(get< 0 >(_infoCopy), to_string(_sigCopy));
                }
            }
        }
    }
    return;
}

void FitManager::AddBackgrounds(FitHolder & _holder, const FitConfiguration & _configuration) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitManager", m_name, "AddBackgrounds", _holder.Name(), _configuration.GetKey());
    MessageSvc::Line();

    // Get the list of Components of the Fit and remove the Signal one for the next loop...
    Smp2CompAndOptMap _components = _configuration.Components();

    Sample _signal = _configuration.SignalSample();
    _components.erase(_signal);

    if (_configuration.HasBrem()) {
        vector< Sample > _signalCopies = _configuration.GetSignalCopies();
        for (const auto & _signalCopy : _signalCopies) { _components.erase(_signalCopy); }
    }

    for (auto & _component : _components) { MessageSvc::Info("AddBackgrounds", to_string(_component.first), "|", to_string(get< 0 >(_component.second)), "|", get< 1 >(_component.second)); }

    for (const auto & _component : _components) {
        MessageSvc::Line();
        MessageSvc::Info("AddBackgrounds", (TString) "Background =", to_string(_component.first));

        Sample _sample = _component.first;

        EventTypeAndOption _info = _configuration.GetBackground(_sample);
        if (get< 0 >(_component.second) == PdfType::SignalCopy) {
            if (_configuration.HasBrem()) MessageSvc::Error("AddBackgrounds", (TString) "Not allowed, should be filtered if SignalCopy for Brem, this background will be work-out from PrepareBrem, not HERE!!", "EXIT_FAILURE");
            _holder.CreateBackgroundFromSignal(get< 0 >(_info), to_string(_sample));
        } else {            
            _holder.CreateBackground(get< 0 >(_info), to_string(_sample), get< 1 >(_info), get< 2 >(_info));
        }

        m_parameterPool->AddYieldParameter(FitParameterConfig::GetBackgroundConfig(_configuration, _sample), _holder.BackgroundYield(to_string(_sample)));
    }
    return;
}

void FitManager::Finalize() {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitManager", m_name, "Finalize");
    MessageSvc::Line();

    if (m_holdersEEBrem.size() == 0) {
        MessageSvc::Warning("Finalize", (TString) "Empty m_holdersEEBrem", "SKIPPING");
        return;
    }

    for (const auto & _configuration : m_configurationsEE) {
        if (!_configuration.HasBrem()) {
            MessageSvc::Warning("Finalize", (TString) "m_configurationsEE has no Brem", "SKIPPINGDO");
            continue;
        }
	//auto data_fit_range = _configuration.RangeDataFit(); TODO... a bit tricky, we do anyway do a syst for the brem-fractions       
        map< TString, map< Brem, double > > _fractions;
        double                              _sum = 0;        
        for (auto & _holder : m_holdersEEBrem[_configuration.GetKey()]){
            EventType & _event                                     = _holder.second.SignalComponent().fitComponent.GetEventType();
            // if( _event.GetTuple() == nullptr){
            //     SettingDef::Tuple::ConfigureTupleLoading();
            //     _event.Init(true,true);
            //     SettingDef::Tuple::ConfigureNoTupleLoading();
            // Done internally if needed ! 
            // }
            Brem        _brem                                      = _event.GetBrem();
	    
            double      _value                                     = GetBremFraction(_event);
            /* HACK:  uncomment below to manipulate BYHAND ratios of bram fractions on MC
            if( _brem == Brem::G0){ 
                substrEvents = _value * 0.2;
                _value += substrEvents;
            }  
            if( _brem == Brem::G2){ 
                _value -= substrEvents;
            }
            */
            _fractions[_holder.second.SignalComponentKey()][_brem] = _value;
            _sum += _value;
        }
        MessageSvc::Info("Finalize", (TString) "Entries Total", to_string(_sum));
        for (auto && _fraction : _fractions) {
            if (((_configuration.GetAna() == Analysis::EE) && (_fraction.second.size() > 3)) || ((_configuration.GetAna() == Analysis::ME) && (_fraction.second.size() > 2))){
                MessageSvc::Error("Finalize", (TString) "Too many Brem categories", to_string(_fraction.second.size()), "EXIT_FAILURE");
            }
            for (auto && _brem : _fraction.second) {
                _brem.second /= _sum;
                if ((_brem.second < 0) || (_brem.second > 1)) MessageSvc::Error("Finalize", (TString) "Invalid Brem fraction", to_string(_brem.second), "EXIT_FAILURE");
            }
        }
            
        RooArgList _pdfListSignal, _coefListSignal;
        for (const auto & _holder : m_holdersEEBrem[_configuration.GetKey()]) {
            MessageSvc::Info("Finalize", _holder.second.SignalComponentKey());
            MessageSvc::Info("Finalize", _holder.second.SignalComponent().fitComponent.PDF());
            _pdfListSignal.add(*_holder.second.SignalComponent().fitComponent.PDF());
            if (((_configuration.GetAna() == Analysis::EE) && (_coefListSignal.getSize() < 2)) || ((_configuration.GetAna() == Analysis::ME) && (_coefListSignal.getSize() < 1))) {
                Brem         _brem = _configuration.GetBremFromKey(_holder.first);
                TString      _name = "frac" + to_string(_brem) + "_" + _configuration.GetKey();
                RooRealVar * _tmp  = nullptr;
                if (_sum != 0) {
                    _tmp = new RooRealVar(_name, _name, _fractions[_holder.second.SignalComponentKey()][_brem], 0, 1);
                    _tmp->setConstant(1);
                    m_parameterPool->AddShapeParameter(_tmp);
                } else {
                    MessageSvc::Info("Finalize", (TString) "GetShapeParameter", _name);
                    _tmp = (RooRealVar *) m_parameterPool->GetShapeParameter(_name);
                    _tmp->setConstant(1);
                }
                if (_tmp == nullptr) MessageSvc::Error("Finalize", (TString) "Invalid Brem fraction", _name, "EXIT_FAILURE");
                double _value = _tmp->getVal();
                if ((_value <= 0) || (_value >= 1)) MessageSvc::Error("Finalize", (TString) "Invalid Brem fraction", _name, to_string(_value), "EXIT_FAILURE");
                MessageSvc::Info("Finalize", to_string(_brem), to_string(_value * 100), "%");
                if (m_option.Contains("modbremfrac")) {
                    TString _key = _holder.second.SignalComponent().fitComponent.Name();
                    _key.ReplaceAll(_holder.second.SignalComponentKey() + SettingDef::separator, "");
                    _key.ReplaceAll("MC", "").ReplaceAll("CL", "");
                    RooFormulaVar * _ftmp = new RooFormulaVar(_name + "_scaled", "@0*@1", RooArgSet(*(RooRealVar *) m_parameterPool->GetShapeParameter("g_scale_" + _key), *_tmp));
                    MessageSvc::Info("ModifyBremFraction", _ftmp);
                    _coefListSignal.add(*_ftmp);
                } else {
                    _coefListSignal.add(*_tmp);
                }
            }
        }
        MessageSvc::Line();
        MessageSvc::Info("Finalize", &_pdfListSignal);
        MessageSvc::Info("Finalize", &_coefListSignal);
        // ENTER IN UPDATE MODE FOR THE FITHOLDER ! (Backgrounds already added...)
        auto * _holder = &m_holdersEE[_configuration.GetKey()];
	
        // Now we have the ADDPDF, the CoefLists, let's build the final signal without Brem flag
        EventTypeAndOption _infoSignal = _configuration.GetSignal(Brem::All);

        // 3pdfs, 2 or 3 coefList, Sum will be ill-formed breaking code later, RooAddPdf do no throw error if sum is ill-formed, just a text
        if (_pdfListSignal.getSize() - _coefListSignal.getSize() != 1) MessageSvc::Error("Finalize", (TString) "Invalid list of PDFs and brem fractions", "EXIT_FAILURE");


        //TODO : 
        // IF you ever will want to run a fit with floating brem-fractions, we must use a RooFormulaVar for the scaling brem fractions so that the PDF ( 3 components ) created with 2 RooRealVar arguments still normalize to 1. 
        // We will need to "scale Brem-Frac0" and absorb the rescaling in Frac1, 2 via RooFormulaVar, actually only for Frac1 ! 
        RooAddPdf * _pdfSignal = new RooAddPdf(to_string(_configuration.SignalSample()) + SettingDef::separator + _configuration.GetKey() + "_SumBrem", to_string(_configuration.SignalSample()) + SettingDef::separator + _configuration.GetKey() + "_SumBrem", _pdfListSignal, _coefListSignal);

        _holder->CreateSignal(get< 0 >(_infoSignal), to_string(_configuration.SignalSample()), _pdfSignal, get< 2 >(_infoSignal));
        m_parameterPool->AddYieldParameter(FitParameterConfig::GetSignalConfig(_configuration), _holder->SignalYield());

        Sample _signalCopy;
        Prj    prj = _configuration.GetProject();
        switch (prj) {
            case Prj::RKst: _signalCopy = Sample::Bs; break;
            case Prj::RK: break;
            case Prj::RPhi: _signalCopy = Sample::Bd; break;
            case Prj::RL: break;
            case Prj::RKS: break;
            default: MessageSvc::Error("Finalize", (TString) "Invalid prj", to_string(prj), "EXIT_FAILURE"); break;
        }

        if (_configuration.HasSample(_signalCopy) && _configuration.HasPdfType(PdfType::SignalCopy)) {
            MessageSvc::Info("Finalize", to_string(_signalCopy));

            RooArgList _pdfListSignalCopy;
            for (auto & _holder_ : m_holdersEEBrem[_configuration.GetKey()]) {
                TString _key = _configuration.GetSampleName(_signalCopy);
                MessageSvc::Info("Finalize", _key);
                MessageSvc::Info("Finalize", _holder_.second.BackgroundComponent(to_string(_signalCopy)).fitComponent.PDF());
                _pdfListSignalCopy.add(*_holder_.second.BackgroundComponent(to_string(_signalCopy)).fitComponent.PDF());
                MessageSvc::Line();
            }
            MessageSvc::Info("Finalize", &_pdfListSignalCopy);
            MessageSvc::Line();

            //Recursive fraction routinealready done.
            RooArgList _coefListSignalCopy = _coefListSignal;
            MessageSvc::Info("Finalize", &_coefListSignalCopy);
            EventTypeAndOption _infoSignalCopy = _configuration.GetBackground(_signalCopy);

            // 3pdfs, 2 or 3 coefList, Sum will be ill-formed breaking code later, RooAddPdf do no throw error if sum is ill-formed, just a text
            if (_pdfListSignalCopy.getSize() - _coefListSignalCopy.getSize() != 1) MessageSvc::Error("Finalize", (TString) "Invalid list of PDFs and brem fractions", "EXIT_FAILURE");

            //TODO : check the fractions are the actual things we want 
            RooAddPdf * _pdfSignalCopy = new RooAddPdf(to_string(_signalCopy) + _configuration.GetKey() + "_sum", to_string(_signalCopy) + _configuration.GetKey() + "_sum", _pdfListSignalCopy, _coefListSignalCopy);

            _holder->CreateBackground(get< 0 >(_infoSignalCopy), to_string(_signalCopy), _pdfSignalCopy, get< 2 >(_infoSignalCopy));
            m_parameterPool->AddYieldParameter(FitParameterConfig::GetBackgroundConfig(_configuration, _signalCopy), _holder->BackgroundYield(to_string(_signalCopy)));
        }
    }
    return;
}

double FitManager::GetBremFraction(EventType & _eventType) {
    double _brem;
    MessageSvc::Line();
    if (SettingDef::Fit::useBremFracCache) {   // Get the
        MessageSvc::Info("BremFracCache used");
        TString      _cutString(_eventType.GetCut());
        TString      _weightString(_eventType.GetWeight());
        TString _cutHash     = HashString(_cutString);
        TString _weightHash  = HashString(_weightString);
        // unsigned int _cutHash     = _cutString.Hash();
        // unsigned int _weightHash  = _weightString.Hash();
        TString      _tupleOption = _eventType.GetTupleHolder().Option();
        TString      _cacheFolder = IOSvc::GetFitCacheDir("", _eventType, _cutHash, _weightHash, _tupleOption, "BremFracCache");
        TString      _cachePath   = _cacheFolder + "/" + _eventType.GetKey();
        MessageSvc::Debug("CutString", _cutString);
        MessageSvc::Debug("WeightString", _weightString);
        MessageSvc::Debug("CacheFolder", _cacheFolder);
        if (IOSvc::ExistFile(_cachePath) && !SettingDef::Fit::redoBremFracCache){   // Load the fraction from cached files
            std::vector< double > buffer(1);
            double *              bufferPointer = &buffer[0];
            fstream               _bremCacheFile;
            _bremCacheFile.open(_cachePath.Data(), std::ios::in | std::ios::binary);
            _bremCacheFile.read((char *) bufferPointer, sizeof(double));
            _brem = buffer[0];
            _bremCacheFile.close();
            MessageSvc::Info("FitManager", (TString) "Loaded brem frac from", _cachePath);
        } else {
            if(IOSvc::ExistFile(_cachePath)){
                MessageSvc::Warning("Deleting and remaking Cache for Brem Fraction");
                IOSvc::RemoveFile(_cachePath);
            }
	    //TODO : add mass B range on the cut!
            _brem = ComputeBremFraction(_eventType);
            IOSvc::MakeDir(_cacheFolder);
            fstream _outFile;
            _outFile.open(_cachePath.Data(), std::ios::out | std::ios::app | std::ios::binary);
            _outFile.write((char *) &_brem, sizeof(double));
            _outFile.close();
            MessageSvc::Info("FitManager", (TString) "Saved brem frac to", _cachePath);
        }
    } else {
        _brem = ComputeBremFraction(_eventType);
    }
    MessageSvc::Info("GetBremFraction", (TString) "Entries Add", _eventType.GetKey(), to_string(_brem));
    MessageSvc::Line();
    return _brem;
}

double FitManager::ComputeBremFraction(EventType & _eventType) {
    if ( _eventType.GetTuple() == nullptr){
      SettingDef::Tuple::ConfigureTupleLoading();
      _eventType.Init(true,true);
      SettingDef::Tuple::ConfigureNoTupleLoading();
    }
    if (!_eventType.IsInitialized()){
        SettingDef::Tuple::ConfigureTupleLoading();
        _eventType.Init();
        SettingDef::Tuple::ConfigureNoTupleLoading();
    }
    // Finalizing GetTuple from Signal Component, check nullptr tuple first
    if (_eventType.GetTuple() == nullptr) { MessageSvc::Error("ComputeBremFraction", (TString) "Tuple is nullptr", _eventType.GetKey(), "EXIT_FAILURE"); }
    cout << WHITE << _eventType << RESET << endl;
    double _value = 0; 
    if( _eventType.IsWeighted()){
        //TODO : SPEED IT UP.... , maybe switch to caching the Brem Fractions and re-use the cache all the time? For fits in bins it might be very tricky. 
        //TODO : Better, if we are 100% sure the weight Options etc..are correctly computed with the proper weights, we need to store the datasets somewhere
        //TODO : Fits in Bootstrapped mode has to use the BremCache probably , so this step is never re-run and we always re-use cached Brem fractions, do it once, never do it again? 
        //This stuff is very heavy computationally wise, we can make 1 snapshots locally only, and reuse it with the Brem cuts?             
        ROOT::RDataFrame df(*_eventType.GetTuple());
        ROOT::RDF::RNode node(df);
        TString _cut    =  _eventType.GetCut().GetTitle();        
        TString _weight =  _eventType.GetWeight();
        if( _eventType.HasWeightOption("SMEARBP") || _eventType.HasWeightOption("SMEARB0")){
            node = HelperProcessing::AppendQ2SmearColumns( node , _eventType.GetProject(), _eventType.GetYear());
            node = HelperProcessing::AppendBSmearColumns(  node , _eventType.GetProject(), _eventType.GetYear());
            //Tough, we might need to cut for the fit range here with the B_DTF_M_SMEAR terms to be 100% correct.
        }
        MessageSvc::Info("ComputeBremFraction (DataFrame) Cut", _cut);
        MessageSvc::Info("ComputeBremFraction (DataFrame) Weight", _weight);
        auto count = node.Filter( _cut.Data()).Define("weight", _weight.Data()).Sum<double>("weight");
	//to do cut on B mass!	
        _value = count.GetValue();
    }else{
        _value = (double) _eventType.GetTupleEntries();
    }

    // double _valueW = (double) _holder.second.SignalComponent().fitComponent.GetEventType().GetTupleEntries("noPID-noL0-noMVA", "PID-L0-BKIN-MULT-RECO", "pro");
    // if (_valueW != 0) _value = _valueW;
    _eventType.Close();
    return _value;
}

void FitManager::CreateData() {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitManager", m_name, "CreateData");
    MessageSvc::Line();

    vector< FitConfiguration > _configurations;
    _configurations.insert(_configurations.end(), m_configurationsMM.begin(), m_configurationsMM.end());
    _configurations.insert(_configurations.end(), m_configurationsEE.begin(), m_configurationsEE.end());
    MessageSvc::Info("CreateData", (TString) "MM configurations =", to_string(m_configurationsMM.size()));
    MessageSvc::Info("CreateData", (TString) "EE configurations =", to_string(m_configurationsEE.size()));
    for (auto & _configuration : _configurations) {
        FitHolder * _holder = nullptr;
        switch (_configuration.GetAna()) {
            case Analysis::MM: _holder = &m_holdersMM[_configuration.GetKey()]; break;
            case Analysis::EE: _holder = &m_holdersEE[_configuration.GetKey()]; break;
            case Analysis::ME: _holder = &m_holdersEE[_configuration.GetKey()]; break;
            default: MessageSvc::Error("CreateData", (TString) "FAILING TO CREATE DATA", "EXIT_FAILURE"); break;
        }
        EventTypeAndOption _info   = _configuration.GetData();
        TString            _option = get< 2 >(_info);
        if (m_option.Contains("splot")) _option += "-splot";
        _holder->CreateData(get< 0 >(_info), get< 1 >(_info), _option);
    }
    return;
}

void FitManager::Init() {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitManager", m_name, "Initialize ...");
    MessageSvc::Line();

    if (!m_isLoaded || m_isReduced) {
        if (m_holdersMM.size() >= 1) {
            PrintHoldersMM();
            MessageSvc::Line();
            MessageSvc::Info(Color::Cyan, "FitManager", m_name, "Initialize MM ...");
            MessageSvc::Line();
            for (auto & _holder : m_holdersMM) {
                MessageSvc::Line();
                MessageSvc::Info(Color::Cyan, "FitManager", m_name, "Initialize", _holder.second.Name(), "...");
                MessageSvc::Line();
                _holder.second.Init();
            }
        }
        if (m_holdersEE.size() >= 1) {
            PrintHoldersEE();
            MessageSvc::Line();
            MessageSvc::Info(Color::Cyan, "FitManager", m_name, "Initialize EE ...");
            MessageSvc::Line();
            for (auto & _holder : m_holdersEE) {
                MessageSvc::Line();
                MessageSvc::Info(Color::Cyan, "FitManager", m_name, "Initialize", _holder.second.Name(), "...");
                MessageSvc::Line();
                _holder.second.Init();
            }
        }
    }

    m_isInitialized = true;

    return;
}

void FitManager::SetStatus(bool _isLoaded, bool _isReduced) {
    m_isLoaded  = _isLoaded;
    m_isReduced = _isReduced;
    for (auto & _holder : m_holdersMM) { _holder.second.SetStatus(m_isLoaded, m_isReduced); }
    for (auto & _holder : m_holdersEE) { _holder.second.SetStatus(m_isLoaded, m_isReduced); }
    // for (auto & _holder : m_holdersEEBrem) { _holder.second.SetStatus(m_isLoaded, m_isReduced); }
    return;
}

void FitManager::RefreshParameterPool() {
    m_parameterPool = RXFitter::GetParameterPool();
    for (auto & _holder : m_holdersMM) { _holder.second.RefreshParameterPool(); }
    for (auto & _holder : m_holdersEE) { _holder.second.RefreshParameterPool(); }
    // for (auto & _holder : m_holdersEEBrem) { _holder.second.RefreshParameterPool(); }
    return;
}

void FitManager::Close() {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitManager", m_name, "Close ...");
    MessageSvc::Line();

    if (m_holdersMM.size() >= 1) {
        MessageSvc::Line();
        MessageSvc::Info(Color::Cyan, "FitManager", m_name, "Close MM ...");
        MessageSvc::Line();
        for (auto & _holder : m_holdersMM) { _holder.second.Close(); }
    }
    if (m_holdersEE.size() >= 1) {
        MessageSvc::Line();
        MessageSvc::Info(Color::Cyan, "FitManager", m_name, "Close EE ...");
        MessageSvc::Line();
        for (auto & _holder : m_holdersEE) { _holder.second.Close(); }
    }

    if (m_fitter != nullptr) {
        m_fitter->Close();
        MessageSvc::Warning("FitManager", m_name, "Delete FitterTool");
        delete m_fitter;
        m_fitter = nullptr;
    }

    for (auto & _configuration : m_configurationsMM) { _configuration.Close(); }
    for (auto & _configuration : m_configurationsEE) { _configuration.Close(); }

    return;
}

void FitManager::CreateFitter() {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitManager", m_name, "CreateFitter");
    MessageSvc::Line();

    if (!m_isInitialized) MessageSvc::Error("FitManager", m_name, "Not initialized", "EXIT_FAILURE");
    for (auto & _holder : m_holdersEE) _holder.second.Configuration().UseBinAndRange(SettingDef::Fit::varSchemeCL);
    for (auto & _holder : m_holdersMM) _holder.second.Configuration().UseBinAndRange(SettingDef::Fit::varSchemeCL);
    if (m_fitter != nullptr) {
        m_fitter->Close();
        delete m_fitter;
        m_fitter = nullptr;
    }
    m_fitter = new FitterTool("FitManager_" + m_name, this);
    return;
}

void FitManager::Fit() {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitManager", m_name, "Fit");
    MessageSvc::Line();
    if (!m_fitter) MessageSvc::Error("FitManager", m_name, "Fitter not available", "EXIT_FAILURE");

    m_fitter->Init();
    m_fitter->SetInitialValuesAndErrors();
    m_fitter->Fit();

    m_parameterPool->PrintParameters();

    return;
}

void FitManager::DoSPlot() {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitManager", m_name, "DoSPlot");
    MessageSvc::Line();

    if (!m_fitter) MessageSvc::Error("FitManager", m_name, "Fitter not available", "EXIT_FAILURE");

    m_fitter->DoSPlot();

    return;
}

vector< FitConfiguration > FitManager::Configurations() const noexcept {
    if ((m_holdersMM.size() != 0) && (m_holdersEE.size() == 0)) return m_configurationsMM;
    if ((m_holdersMM.size() == 0) && (m_holdersEE.size() != 0)) return m_configurationsEE;
    vector< FitConfiguration > _configurations = m_configurationsMM;
    _configurations.insert(_configurations.end(), m_configurationsEE.begin(), m_configurationsEE.end());
    return _configurations;
}

Str2HolderMap FitManager::Holders() const {
    if ((m_holdersMM.size() != 0) && (m_holdersEE.size() == 0)) return m_holdersMM;
    if ((m_holdersMM.size() == 0) && (m_holdersEE.size() != 0)) return m_holdersEE;
    Str2HolderMap _holders = m_holdersMM;
    _holders.insert(m_holdersEE.begin(), m_holdersEE.end());
    return _holders;
}

void FitManager::PrintHoldersMM() const noexcept {
    for (const auto & _holder : m_holdersMM) {
        MessageSvc::Line();
        cout << GREEN;
        cout << setw(25) << left << "PrintHoldersMM";
        cout << setw(20) << left << _holder.first << " -- ";
        cout << setw(50) << left << _holder.second.Name();
        cout << endl;
        _holder.second.PrintComponents();
    }
    return;
}

void FitManager::PrintHoldersEE() const noexcept {
    bool _printBrem = false;
    for (const auto & _holder : m_holdersEE) {
        MessageSvc::Line();
        cout << GREEN;
        cout << setw(25) << left << "PrintHoldersEE";
        cout << setw(20) << left << _holder.first << " -- ";
        cout << setw(50) << left << _holder.second.Name();
        cout << endl;
        _holder.second.PrintComponents();
        /*
        for (auto & _holderBrem : m_holdersEEBrem[_holder.first]) {
            cout << GREEN;
            cout << setw(25) << left << "PrintHoldersEEBrem";
            cout << setw(20) << left << _holderBrem.first << " -- ";
            cout << setw(50) << left << _holderBrem.second.Name();
            cout << endl;
            _holderBrem.second.PrintComponents();
        }
        */
    }
    _printBrem = m_holdersEEBrem.size() != 0 ? true : false;

    if (_printBrem) {
        for (const auto & _holder : m_holdersEEBrem) {

            for (auto & _holderBrem : _holder.second) {
                MessageSvc::Line();
                cout << GREEN;
                cout << setw(25) << left << "PrintHoldersEEBrem [" << _holder.first << " ][" << _holderBrem.first << "] -- ";
                cout << setw(50) << left << "[" << _holder.first << " ][" << _holderBrem.first << "]  Name = " << _holderBrem.second.Name();
                cout << endl;
                _holderBrem.second.PrintComponents();
                MessageSvc::Line();
            }
        }
    }
    return;
}

bool FitManager::IsInMMHolderMap(TString _name) const noexcept {
    if (m_holdersMM.find(_name) == m_holdersMM.end()) return false;
    return true;
}

bool FitManager::IsInEEHolderMap(TString _name) const noexcept {
    if (m_holdersEE.find(_name) == m_holdersEE.end()) return false;
    return true;
}

void FitManager::InitRanges() {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitManager", m_name, "InitRanges");
    MessageSvc::Line();

    if( ( (!m_isLoaded || m_isReduced) && !SettingDef::Weight::useBS) || SettingDef::Weight::useBS){
        for (auto & _holder : m_holdersMM) {
            MessageSvc::Info("InitRanges", _holder.first);
            _holder.second.InitRanges();
        }
        for (auto & _holder : m_holdersEE) {
            MessageSvc::Info("InitRanges", _holder.first);
            _holder.second.InitRanges();
        }
    }
    return;
}

FitHolder & FitManager::operator[](const TString & _keyHolder) {
    // throw an error if _keyHolder is in both MM and EE maps
    if (m_holdersEE.find(_keyHolder) != m_holdersEE.end() && m_holdersMM.find(_keyHolder) != m_holdersMM.end()) { MessageSvc::Error("FitManager", (TString) "FitHolder key exists in both MM and EE holders", _keyHolder, "EXIT_FAILURE"); }
    if (m_holdersMM.find(_keyHolder) != m_holdersMM.end()) { return m_holdersMM[_keyHolder]; }
    if (m_holdersEE.find(_keyHolder) != m_holdersEE.end()) { return m_holdersEE[_keyHolder]; }
    TString _keyHolderBrem = _keyHolder;
    _keyHolderBrem.ReplaceAll(SettingDef::separator + "0G", "").ReplaceAll(SettingDef::separator + "1G", "").ReplaceAll(SettingDef::separator + "2G", "");
    if (m_holdersEEBrem[_keyHolderBrem].find(_keyHolder) != m_holdersEEBrem[_keyHolderBrem].end()) { return m_holdersEEBrem[_keyHolderBrem][_keyHolder]; }
    MessageSvc::Error("FitManager", m_name, "Cannot find FitHolder key", _keyHolder);
    cout << RED;
    PrintKeys();
    cout << RESET;
    MessageSvc::Error("FitManager", m_name, "Cannot find FitHolder key", _keyHolder, "EXIT_FAILURE");
    // please the compiler
    auto dummy = new FitHolder();
    return *dummy;
}

void FitManager::PrintKeys() const noexcept {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitManager", m_name, "PrintKeys");
    MessageSvc::Line();
    for (const auto & _holder : m_holdersMM) {
        cout << "HoldersMM = " << _holder.first << endl;
        _holder.second.PrintKeys();
        MessageSvc::Line();
    }
    for (const auto & _holder : m_holdersEE) {
        cout << "HoldersEE = " << _holder.first << endl;
        _holder.second.PrintKeys();
        /*
        for (auto & _holderBrem : m_holdersEEBrem[_holder.first]) {
            cout << "HoldersEEBrem = " << _holderBrem.first << endl;
            _holderBrem.second.PrintKeys();
        }
        */
        MessageSvc::Line();
    }
    return;
}

void FitManager::PrintPDFs(TString _option) {
    if (m_holdersMM.size() >= 1) {
        MessageSvc::Line();
        MessageSvc::Info(Color::Cyan, "FitManager", m_name, "HoldersMM");
        MessageSvc::Line();
        for (auto & _holder : m_holdersMM) {
            MessageSvc::Info("PrintPDFs", (TString) "FitHolderKey", _holder.first);
            for (auto & _key : _holder.second.KeyMap()) { MessageSvc::Info(_holder.second[_key].Name(), _holder.second[_key].PDF(), _option); }
            MessageSvc::Line();
        }
    }
    if (m_holdersEE.size() >= 1) {
        MessageSvc::Line();
        MessageSvc::Info(Color::Cyan, "FitManager", m_name, "HoldersEE");
        MessageSvc::Line();
        for (auto & _holder : m_holdersEE) {
            MessageSvc::Info("PrintPDFs", (TString) "FitHolderKey", _holder.first);
            for (auto & _key : _holder.second.KeyMap()) { MessageSvc::Info(_holder.second[_key].Name(), _holder.second[_key].PDF(), _option); }
            MessageSvc::Line();
            /*
            if (m_holdersEEBrem[_holder.first].size() > 0) {
                MessageSvc::Line();
                MessageSvc::Info(Color::Cyan, "FitManager", m_name, "HoldersEEBrem");
                MessageSvc::Line();
                for (auto & _holderBrem : m_holdersEEBrem[_holder.first]) {
                    MessageSvc::Info("PrintPDFs", (TString) "FitHolderKey", _holderBrem.first);
                    for (auto & _keyBrem : _holderBrem.second.KeyMap()) {
                        MessageSvc::Info(_holderBrem.second[_keyBrem].Name(), _holderBrem.second[_keyBrem].PDF(), _option);
                    }
                    MessageSvc::Line();
                }
            }
            */
        }
    }
    return;
}

void FitManager::SaveToDisk(TString _name, bool _verbose) {
    if (m_isLoaded) {
        MessageSvc::Warning("FitManager", m_name, "LoadedFromDisk SKIPPING SaveToDisk");
        return;
    }
    // SaveToLog(_name);

    if (!SettingDef::IO::outDir.EndsWith("/")) SettingDef::IO::outDir += "/";
    if (_name != "") _name = "_" + _name;
    TString _oname = "FitManager_" + m_name + _name;
    _name          = SettingDef::IO::outDir + _oname + ".root";

    MessageSvc::Line();
#ifdef STREAMDATA
    MessageSvc::Info(Color::Cyan, "FitManager", m_name, "SaveToDisk", _name, "with STREAMDATA");
#else
    MessageSvc::Warning("FitManager", m_name, "SaveToDisk", _name, "without STREAMDATA");
#endif
    PrintKeys();
    PrintPDFs();
    MessageSvc::Line();

    m_parameterSnapshot.ConfigureSnapshotMap();

    TFile _tFile(_name, to_string(OpenMode::RECREATE));
    (*this).Write(_oname, TObject::kOverwrite);
    if (_verbose) {
        TDirectory * _holdersMM     = _tFile.mkdir("FitHolder_MM");
        TDirectory * _holdersEE     = _tFile.mkdir("FitHolder_EE");
        TDirectory * _holdersEEBrem = _tFile.mkdir("FitHolder_EEBrem");

        for (const auto & _holder : m_holdersMM) {
            MessageSvc::Info("SaveToDisk", (TString) "Writing", _holder.first);
            _holdersMM->cd();
            _holder.second.Write(_holder.first, TObject::kOverwrite);
        }
        for (const auto & _holder : m_holdersEE) {
            MessageSvc::Info("SaveToDisk", (TString) "Writing", _holder.first);
            _holdersEE->cd();
            _holder.second.Write(_holder.first, TObject::kOverwrite);
        }
        for (const auto & _bremholder : m_holdersEEBrem) {
            for (auto & _holder : _bremholder.second) {
                _holdersEEBrem->cd();
                MessageSvc::Info("SaveToDisk", (TString) "Writing", _holder.first);
                _holder.second.Write(_holder.first, TObject::kOverwrite);
            }
        }
    }

    _tFile.Close();
    cout << WHITE << *this << RESET << endl;
    return;
}

void FitManager::LoadFromDisk(TString _name, TString _dir) {
    MessageSvc::Line();
#ifdef STREAMDATA
    MessageSvc::Info(Color::Cyan, "FitManager", m_name, "LoadFromDisk", _name, _dir, "with STREAMDATA");
#else
    MessageSvc::Warning("FitManager", m_name, "LoadFromDisk", _name, _dir, "without STREAMDATA");
#endif

    if (_name != "") _name = "_" + _name;
    _name = "FitManager_" + m_name + _name;

    if ((_dir != "") && (!_dir.EndsWith("/"))) _dir += "/";

    if (!IOSvc::ExistFile(_dir + _name + ".root")) MessageSvc::Error("FitManager", _dir + _name + ".root", "does not exist", "EXIT_FAILURE");

    TFile _tFile(_dir + _name + ".root", "read");
    MessageSvc::Line();
    _tFile.ls();
    MessageSvc::Line();

    FitManager * _fm = (FitManager *) _tFile.Get(_name);
    *this            = *_fm;

    PrintKeys();
    PrintPDFs();
    MessageSvc::Line();

    SetStatus(true, m_isReduced);

    m_parameterSnapshot.ReloadParameters();

    RefreshParameterPool();

    _tFile.Close();
    cout << WHITE << *this << RESET << endl;
    return;
}

void FitManager::ReduceComponents(TCut _cut) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitManager", m_name, "ReduceComponents", TString(_cut));
    MessageSvc::Line();

    if (m_holdersMM.size() >= 1) {
        MessageSvc::Line();
        MessageSvc::Info(Color::Cyan, "FitManager", m_name, "ReduceComponents MM");
        MessageSvc::Line();
        for (auto & _holder : m_holdersMM) {
            MessageSvc::Line();
            MessageSvc::Info(Color::Cyan, "FitManager", m_name, "ReduceComponents", _holder.second.Name());
            MessageSvc::Line();
            _holder.second.ReduceComponents(_cut);
        }
    }
    if (m_holdersEE.size() >= 1) {
        MessageSvc::Line();
        MessageSvc::Info(Color::Cyan, "FitManager", m_name, "ReduceComponents EE");
        MessageSvc::Line();
        for (auto & _holder : m_holdersEE) {
            MessageSvc::Line();
            MessageSvc::Info(Color::Cyan, "FitManager", m_name, "ReduceComponents", _holder.second.Name());
            MessageSvc::Line();
            _holder.second.ReduceComponents(_cut);
        }
    }

    SetStatus(m_isLoaded, true);

    cout << WHITE << *this << RESET << endl;
    return;
}

void FitManager::SaveToLog(TString _name) const noexcept {
    if (!SettingDef::IO::outDir.EndsWith("/")) SettingDef::IO::outDir += "/";
    if (_name != "") _name = "_" + _name;
    _name = SettingDef::IO::outDir + "FitManager_" + m_name + _name + ".log";

    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitManager", m_name, "SaveToLog", _name);

    ofstream _file(_name);
    if (!_file.is_open()) MessageSvc::Error("Unable to open file", _name, "EXIT_FAILURE");
    _file << *this << endl;
    _file.close();

    MessageSvc::Line();
    return;
}

void FitManager::SaveToYAML(TString _name, TString _option) const noexcept {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitManager", m_name, "SaveToYAML", _name, _option);
    MessageSvc::Info(Color::Cyan, "FitManager", m_name, "Fit Configuration(s)", to_string(SettingDef::Fit::configurations.size()));
    MessageSvc::Info(Color::Cyan, "FitManager", m_name, "Toy Configuration(s)", to_string(SettingDef::Toy::configurations.size()));

    if (_name != "") _name = "_" + _name;

    EventType _et = EventType();
    _et.SaveToYAML(m_name + _name, _option);

    MessageSvc::Line();
    return;
}

void FitManager::AddFitHolderMM(TString _name, FitHolder * _holder) {
    if (_holder != nullptr) {
        if (!IsInMMHolderMap(_name)) {
            m_holdersMM[_name] = *_holder;
            MessageSvc::Info("AddFitHolderEE", _name, to_string(m_holdersMM.size()));
        } else {
            MessageSvc::Error("AddFitHolderMM", _name, "already in map", "EXIT_FAILURE");
        }
    }
    return;
}

void FitManager::AddFitHolderEE(TString _name, FitHolder * _holder) {
    if (_holder != nullptr) {
        if (!IsInEEHolderMap(_name)) {
            m_holdersEE[_name] = *_holder;
            MessageSvc::Info("AddFitHolderEE", _name, to_string(m_holdersEE.size()));
        } else {
            MessageSvc::Error("AddFitHolderEE", _name, "already in map", "EXIT_FAILURE");
        }
    }
    return;
}

void FitManager::AddFitHolder(TString _name, FitHolder * _holder) {
    if (_holder != nullptr) {
        if (_name.Contains("MM"))
            AddFitHolderMM(_name, _holder);
        else if (_name.Contains("EE"))
            AddFitHolderEE(_name, _holder);
        else
            MessageSvc::Error("AddFitHolder", _name, "invalid name", "EXIT_FAILURE");
    }
    return;
}

void FitManager::AddFitConfiguration(const FitConfiguration & _configuration) {
    MessageSvc::Info(Color::Cyan, "FitManager", m_name, "AddFitConfiguration", _configuration.GetKey());
    switch (_configuration.GetAna()) {
        case Analysis::MM: m_configurationsMM.push_back(_configuration); break;
        case Analysis::EE: m_configurationsEE.push_back(_configuration); break;
        case Analysis::ME: m_configurationsEE.push_back(_configuration); break;
        case Analysis::Error: MessageSvc::Error("AddFitConfiguration", (TString) "ERROR ANALYSIS FOR ADDING FitConfiguration", "EXIT_FAILURE"); break;
        default: MessageSvc::Error("AddFitConfiguration", (TString) "FAILING TO ADD FIT CONFIGURATION", "EXIT_FAILURE"); break;
    }
    return;
}

void FitManager::PrintConfigurations() const noexcept {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitManager", m_name, "PrintConfigurations");
    MessageSvc::Line();

    if (m_configurationsMM.size() != 0) MessageSvc::Info("PrintConfigurations", (TString) "Configuration MM");
    for (const auto & _configuration : m_configurationsMM) {
        _configuration.Print();
        _configuration.PrintContent();
        MessageSvc::Line();
    }

    if (m_configurationsEE.size() != 0) MessageSvc::Info("PrintConfigurations", (TString) "Configuration EE");
    for (const auto & _configuration : m_configurationsEE) {
        _configuration.Print();
        _configuration.PrintContent();
        MessageSvc::Line();
    }
    return;
}

#endif
