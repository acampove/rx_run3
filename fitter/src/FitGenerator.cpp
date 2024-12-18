#ifndef FITGENERATOR_CPP
#define FITGENERATOR_CPP

#include "FitGenerator.hpp"
#include "FitResultLogger.hpp"

#include "ConstDef.hpp"
#include "SettingDef.hpp"

#include "FitterTool.hpp"
#include "ParserSvc.hpp"

ClassImp(FitGenerator)

FitGenerator::FitGenerator(TString _name, TString _option) {
    if (SettingDef::debug.Contains("FG")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("FitGenerator", (TString) "TString");
    m_name          = _name;
    m_name          = CleanString(m_name);
    m_option        = _option;
    m_parameterPool = RXFitter::GetParameterPool();
    Parse();
    cout << WHITE << *this << RESET << endl;
}

FitGenerator::FitGenerator(const FitGenerator & _fitGenerator) {
    if (SettingDef::debug.Contains("FG")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("FitGenerator", (TString) "FitGenerator");
    m_name           = _fitGenerator.Name();
    m_name           = CleanString(m_name);
    m_option         = _fitGenerator.Option();
    m_configurations = _fitGenerator.Configurations();
    m_managers       = _fitGenerator.Managers();
    m_parameterPool  = RXFitter::GetParameterPool();
    if (_fitGenerator.Fitter() != nullptr) m_fitter = static_cast< FitterTool * >(_fitGenerator.Fitter());
    // cout << WHITE << *this << RESET << endl;
}

FitGenerator::FitGenerator(TString _generatorName, TString _option, TString _name, TString _dir) {
    if (SettingDef::debug.Contains("FG")) SetDebug(true);
    m_name = _generatorName;
    m_name = CleanString(m_name);
    if (m_debug) MessageSvc::Debug("FitGenerator", m_name, "LoadFromDisk");
    LoadFromDisk(_name, _dir);
    if (IsLoaded()) m_parameterPool = RXFitter::GetParameterPool();
    if (_option != m_option) {
        MessageSvc::Line();
        MessageSvc::Info(Color::Cyan, "FitGenerator", m_name, "Resetting fit options");
        MessageSvc::Info("OLD", m_option);
        MessageSvc::Info("NEW", _option);
        MessageSvc::Line();
        m_option = _option;
        if (IsLoaded() && ! SettingDef::Weight::useBS ) Parse();
        cout << WHITE << *this << RESET << endl;
    }
}

ostream & operator<<(ostream & os, const FitGenerator & _fitGenerator) {
    os << WHITE;
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "FitGenerator");
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "name", _fitGenerator.Name());
    MessageSvc::Print((ostream &) os, "option", _fitGenerator.Option());
    MessageSvc::Print((ostream &) os, "loaded", to_string(_fitGenerator.IsLoaded()));
    MessageSvc::Print((ostream &) os, "reduced", to_string(_fitGenerator.IsReduced()));
    MessageSvc::Print((ostream &) os, "configurations", to_string(_fitGenerator.Configurations().size()));
    MessageSvc::Print((ostream &) os, "managers", to_string(_fitGenerator.Managers().size()));
    for (const auto & _manager : _fitGenerator.Managers()) { MessageSvc::Print((ostream &) os, (TString) "manager", _manager.first); }
    MessageSvc::Line(os);
    os << RESET;
    os << "\033[F";
    return os;
}

void FitGenerator::Parse() {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitGenerator", m_name, "Parse");
    MessageSvc::Line();

    ParserSvc parser("quiet");

    m_configurations = SettingDef::Fit::configurations;

    map< pair< Prj, Q2Bin >, pair< TString, TString > > _yamls = SettingDef::Fit::yamls;

    if (_yamls.size() != 0) {
        ConfigHolder _conf = ConfigHolder();
        for (auto _yaml : _yamls) {
            SettingDef::Config::project = to_string(_yaml.first.first);
            SettingDef::Config::q2bin   = to_string(_yaml.first.second);
            //Append to main FitOption the sub-file to load option specified.
            TString _option             = m_option + _yaml.second.second;
            if (m_option.Contains("dry")) {
                if (m_option.Contains("drysig"))
                    _option = "-drysig";
                else
                    _option = "-dry";
            }

            parser.Init(_yaml.second.first);

            vector< FitConfiguration > _configurations = SettingDef::Fit::configurations;
            if (_configurations.size() == 0) MessageSvc::Error("FitGenerator", (TString) "Only supports FitConfiguration", "EXIT_FAILURE");

            /*
            IN CASE YOU useBS, by default the FitGenerator is loaded from Disk, it comes out with baseline ingredients, but we MUST clear eveything inside the 
            ParameterPool beforeHand and reconfigure all parameters/add them with the proper constraints, else the FitterTool will not be able 
            to capture the Branching ratios and Hadronization factors constraints due to pointer-pointer mismatch.            
            if( SettingDef::Weight::useBS) m_parameterPool->ConfigureParameters(_configurations, _option);
            NO NEED OF RECONFIGURING THEM since Parse() is not called for Bootstrapping 
            */
            if (!m_isLoaded) {
                m_parameterPool->ConfigureParameters(_configurations, _option);
                FitManager _fitManager(to_string(_yaml.first.first) + SettingDef::separator + "q2" + to_string(_yaml.first.second), _configurations, _option);
                AddFitManager(_fitManager);
            }
            m_configurations.insert(m_configurations.end(), _configurations.begin(), _configurations.end());
        }
        ResetSettingDefConfig(_conf);
    } else if (m_configurations.size() != 0) {
        TString _name = "";
        if (m_configurations.front().GetQ2bin() == Q2Bin::JPsi){
            _name = to_string(Q2Bin::JPsi);
        }else if (m_configurations.front().GetQ2bin() == Q2Bin::Psi){
            _name = to_string(Q2Bin::Psi);
        }else if (m_configurations.front().GetQ2bin() == Q2Bin::All){
    	  _name = to_string(Q2Bin::All);
        }else{ 
            MessageSvc::Error(SettingDef::IO::exe, (TString) "Please add an extra name switch", "EXIT_FAILURE");
        }
        TString _option = m_option;
        if (m_option.Contains("dry")) {
            if (m_option.Contains("drysig"))
                _option = "-drysig";
            else
                _option = "-dry";
        }
        /*
        IN CASE YOU useBS, by default the FitGenerator is loaded from Disk, it comes out with baseline ingredients, but we MUST clear eveything inside the 
        ParameterPool beforeHand and reconfigure all parameters/add them with the proper constraints, else the FitterTool will not be able 
        to capture the Branching ratios and Hadronization factors constraints due to pointer-pointer mismatch.       
        if( SettingDef::Weight::useBS) m_parameterPool->ConfigureParameters(m_configurations, _option);
        NO NEED OF RECONFIGURING THEM since Parse() is not called for Bootstrapping 
        */
        if (!m_isLoaded) {
            m_parameterPool->ConfigureParameters(m_configurations, _option);
            FitManager _fitManager("q2" + _name, m_configurations, _option);
            AddFitManager(_fitManager);
        }
    } else {
        MessageSvc::Error("FitGenerator", (TString) "Only supports FitConfiguration", "EXIT_FAILURE");
    }
    if (m_option.Contains("dry")) {
        if (m_option.Contains("drysig")){
            m_option = "-drysig";
        }
        else{
            m_option = "-dry";
        }
    }
    SettingDef::Fit::option = m_option;

    SettingDef::Fit::configurations = m_configurations;

    SettingDef::Fit::yamls = _yamls;
    
    //==== DEBUG Parsing and filler of Parameter Pool =====//
    m_parameterPool->PrintParameters();
    
    //We should have loaded everything for the fit, we now initialize the Systematics for EfficiencyRatios factors entering the R-Ratio formulas
    m_parameterPool->InitEffRatioSystematicCovariance();
    return;
}

void FitGenerator::AddFitManager(const FitManager & _fitManager) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitGenerator", m_name, "AddFitManager", _fitManager.Name(), _fitManager.Option());
    MessageSvc::Line();

    m_managers[_fitManager.Name()] = _fitManager;
    _fitManager.PrintConfigurations();

    return;
}

FitHolder FitGenerator::Holder() {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitGenerator", m_name, "Holder");

    if (m_managers.size() != 1) MessageSvc::Error("Holder", (TString) "Only 1 FitManager supported", "EXIT_FAILURE");
    if (m_managers.begin()->second.Holders().size() != 1) MessageSvc::Error("Holder", (TString) "Only 1 FitHolder supported", "EXIT_FAILURE");

    MessageSvc::Info("Manager", m_managers.begin()->first);
    cout << m_managers.begin()->second << endl;
    MessageSvc::Info("Holder", m_managers.begin()->second.Holders().begin()->first);
    cout << m_managers.begin()->second.Holders().begin()->second << endl;

    return m_managers.begin()->second.Holders().begin()->second;
}

void FitGenerator::Init(TCut _cut, int  _BSIDX ) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitGenerator", m_name, "Initialize");
    MessageSvc::Line();
    if( _BSIDX >=0){
        MessageSvc::Warning("FitGenerator Initialize BS FIT " , to_string(_BSIDX));
    }

    auto _start = chrono::high_resolution_clock::now();

    if (!m_isLoaded || m_isReduced) {
        //should load all cached stuff...
        Prepare();
        CreateData();
        if (IsCut(_cut) && !m_isReduced) ReduceComponents(_cut);
        if (m_option.Contains("dry")) return;

        ModifyShapes();
        ModifyYields();
    }

    for (auto & _manager : m_managers) { _manager.second.Init(); }

    m_isInitialized = true;

    CreateFitter();

    m_fitter->PrintParameterComponentRelation();

    auto _stop = chrono::high_resolution_clock::now();
    MessageSvc::Warning("FitGenerator", m_name, "Init took", to_string(chrono::duration_cast< chrono::seconds >(_stop - _start).count()), "seconds");
    return;
}

void FitGenerator::SetStatus(bool _isLoaded, bool _isReduced) {
    m_isLoaded  = _isLoaded;
    m_isReduced = _isReduced;
    for (auto & _manager : m_managers) { _manager.second.SetStatus(m_isLoaded, m_isReduced); }
    return;
}

void FitGenerator::RefreshParameterPool() {
    MessageSvc::Info("RefreshParmaeterPool");
    m_parameterPool = RXFitter::GetParameterPool();
    for (auto & _manager : m_managers) { 
        _manager.second.RefreshParameterPool(); 
    }
    return;
}

void FitGenerator::Close() {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitGenerator", m_name, "Close");
    MessageSvc::Line();

    for (auto & _manager : m_managers) { _manager.second.Close(); }

    if (m_fitter != nullptr) {
        m_fitter->Close();
        MessageSvc::Warning("FitGenerator", m_name, "Delete FitterTool");
        delete m_fitter;
        m_fitter = nullptr;
    }

    for (auto & _configuration : m_configurations) { _configuration.Close(); }

    return;
}

void FitGenerator::Prepare() {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitGenerator", m_name, "Prepare");
    MessageSvc::Line();

    auto _start = chrono::high_resolution_clock::now();

    for (auto & _manager : m_managers) {
        MessageSvc::Line();
        MessageSvc::Info("Prepare", _manager.second.Name());
        MessageSvc::Line();

        _manager.second.PrintHoldersMM();
        _manager.second.PrintHoldersEE();

        _manager.second.Prepare();

        _manager.second.PrintPDFs();
    }

    for (auto & _manager : m_managers) { _manager.second.PrintKeys(); }
    if( m_option.Contains("forceSavebeforemod")){
        SaveToDisk("beforemod",true);
    }else{
        SaveToDisk("beforemod");
    }
    
    auto _stop = chrono::high_resolution_clock::now();
    MessageSvc::Warning("FitGenerator", m_name, "Prepare took", to_string(chrono::duration_cast< chrono::seconds >(_stop - _start).count()), "seconds");
    return;
}

void FitGenerator::CreateData() {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitGenerator", m_name, "CreateData");
    MessageSvc::Line();

    auto _start = chrono::high_resolution_clock::now();

    for (auto & _manager : m_managers) {
        MessageSvc::Line();
        MessageSvc::Info("CreateData", _manager.second.Name(), _manager.second.Option());
        MessageSvc::Line();

        _manager.second.CreateData();
    }

    for (auto & _manager : m_managers) { _manager.second.PrintKeys(); }

    auto _stop = chrono::high_resolution_clock::now();
    MessageSvc::Warning("FitGenerator", m_name, "CreateData took", to_string(chrono::duration_cast< chrono::seconds >(_stop - _start).count()), "seconds");
    return;
}

void FitGenerator::ModifyShapes() {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitGenerator", m_name, "ModifyShapes");
    MessageSvc::Line();
    /**
        Identify the parameters in option specified by a key of search option
        Example 
        OPTION = -OPTION1-OPTION2[A,B,C]-OPTION3
        _GET_PARS_IN_OPTION_(OPTION , OPTION2) returns { A,B,C }
    */
    auto _GET_PARS_IN_OPTION_ = []( TString _MANAGEROPTION, TString _SEARCHOPTION ){
        vector<string> _parsInOption{};
        if(!_MANAGEROPTION.Contains( _SEARCHOPTION)) return _parsInOption;
        TString _stringsearch = "";
        for( auto & el : TokenizeString(_MANAGEROPTION , "-") ){
            if(el.Contains(_SEARCHOPTION)){
                _stringsearch = el;        
                break;
            }
        }        
        if( _stringsearch == "") MessageSvc::Error(TString::Format("_GET_PARS_IN_OPTION_( %s , %s )", _MANAGEROPTION.Data(), _SEARCHOPTION.Data()), "Failed to parse and search", "EXIT_FAILURE");
        vector<TString> _ParametersPassedOnOption = TokenizeString( StripStringBetween( _stringsearch, TString::Format("%s[", _SEARCHOPTION.Data()), "]") ,",");
        for( auto & _par  :  _ParametersPassedOnOption){
            _parsInOption.push_back( _par.Data());
        }
        return _parsInOption;
    };


    auto _ISANALYTICALSHAPE_ = [](FitComponent & _component){
        /*
            Simple Lambda identifying whether the Component has been generated with an Analytical shape
        */
        PdfType        _type              = _component.Type();
        // WARNING : 
        // SignalCopy unfortunately is recognized for some reason as StringToFit...
        return ( (_type == PdfType::StringToFit) || (_type == PdfType::SignalCopy) || (_type == PdfType::ToyPDF) );
    };
    /**
        Identify the parameters in option specified by a key of search option
        Example 
        OPTION = -OPTION1-OPTION2[A,B,C]-OPTION3
        _ALLOW_TO_MODIFY_(A , OPTION, OPTION2) returns true
        OPTION = -OPTION1-OPTION2[B,C]-OPTION3
        _ALLOW_TO_MODIFY_(A , OPTION, OPTION2) returns false
    */    
    auto _ALLOW_TO_MODIFY_ = []( TString _PARAMETERKEY , TString _MANAGEROPTION, TString _SEARCHOPTION){
        /* 
            Usage 
            _ALLOW_TO_MODIFY_ = ( "m", "TheOption", "NOSHIFTEE");
            Example : 
            std::cout<< _ALLOW_TO_MODIFY_( "m", "Opt1-Opt2-NOSCALEMM[mg]-Opt3", "NOSCALEMM" ) << std::endl;
            ==> TRUE
            std::cout<< _ALLOW_TO_MODIFY_( "m", "Opt1-Opt2-NOSCALEMM[mg,m,m2]-Opt3", "NOSCALEMM" ) << std::endl;
            ==> FALSE 
            ( can implement custom options vetoes )
        */
        if(!_MANAGEROPTION.Contains( _SEARCHOPTION)) return true;
        TString _stringsearch = "";
        for( auto & el : TokenizeString(_MANAGEROPTION , "-") ){
            if(el.Contains(_SEARCHOPTION)){
                _stringsearch = el;        
                break;
            }
        }
        if( _stringsearch == "") MessageSvc::Error(TString::Format("ALLOW_TO_MODIFY( %s , %s , %s )", _PARAMETERKEY.Data(), _MANAGEROPTION.Data(), _SEARCHOPTION.Data()), "Failed to parse and search", "EXIT_FAILURE");
        vector<TString> _ParametersPassedOnOption = TokenizeString( StripStringBetween( _stringsearch, TString::Format("%s[", _SEARCHOPTION.Data()), "]") ,",");
        bool _vetoed = false;
        for( auto & _par  :  _ParametersPassedOnOption){
            if ( _par == _PARAMETERKEY){ _vetoed = true; break;}
        }
        return !_vetoed;  
    };


    auto _start = chrono::high_resolution_clock::now();

    TString _dmBsBd = TString(to_string(PDG::Mass::Bs - PDG::Mass::Bd));
    TString _dmBdBs = TString(to_string(PDG::Mass::Bd - PDG::Mass::Bs));

    for (auto & _manager : m_managers) {
        MessageSvc::Line();
        MessageSvc::Info("ModifyShapes", _manager.second.Name(), _manager.second.Option());
        MessageSvc::Line();

        if (_manager.second.Option().Contains("modshape")) {
            //==============================================================================================================================
            // Deal with the Muon MODE ( IF FIT TO SIGNAL IS DONE WITH MC shapes in each Brem Categories)
            //==============================================================================================================================            
            for (const auto & _keyHolderPair : _manager.second.HoldersMM()) {
                FitHolder & _holderMM = _manager.second[_keyHolderPair.first];
                TString     _key      = _holderMM[_holderMM.SignalComponentKey()].Name();
                _key.ReplaceAll(_holderMM.SignalComponentKey() + SettingDef::separator, "");
                _key.ReplaceAll("MC", "").ReplaceAll("CL", "");
                TString _keyInThisQ2 = _key;
                _keyInThisQ2 = CleanString(_keyInThisQ2); 
                if (_manager.second.Option().Contains("modshapejpsmm") || (_manager.second.Option().Contains("modshapejps") && !_manager.second.Option().Contains("modshapejpsee"))) _key.ReplaceAll(to_string(_holderMM.Configuration().GetQ2bin()), to_string(Q2Bin::JPsi));
                _key = CleanString(_key);

                auto _shiftKey = _key;
                auto _scaleKey = _key;
                if (_manager.second.Option().Contains("modshiftrx")) _shiftKey.ReplaceAll(to_string(_holderMM.Configuration().GetProject()) + SettingDef::separator, "");
                if (_manager.second.Option().Contains("modscalerx")) _scaleKey.ReplaceAll(to_string(_holderMM.Configuration().GetProject()) + SettingDef::separator, "");
                if (_manager.second.Option().Contains("modshifttagprobe")) _shiftKey.ReplaceAll(SettingDef::separator + to_string(_holderMM.Configuration().GetTrack()), "");
                if (_manager.second.Option().Contains("modscaletagprobe")) _scaleKey.ReplaceAll(SettingDef::separator + to_string(_holderMM.Configuration().GetTrack()), "");

                _shiftKey = CleanString(_shiftKey);
                _scaleKey = CleanString(_scaleKey);

                FitComponent & _signalComponentMM = _holderMM[_holderMM.SignalComponentKey()];
                if( _ISANALYTICALSHAPE_(_signalComponentMM) ) {
                    _signalComponentMM.SetConstantAllPars();
                    // mass shift
                    _signalComponentMM.ModifyPDF("m", (RooRealVar *) m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey), "-shift");
                    _signalComponentMM.ModifyPDF("mcb", (RooRealVar *) m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey), "-shift");
                    _signalComponentMM.ModifyPDF("mg", (RooRealVar *) m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey), "-shift");
                    _signalComponentMM.ModifyPDF("m2", (RooRealVar *) m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey), "-shift");

                    // sigma scale
                    _signalComponentMM.ModifyPDF("s", (RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");
                    _signalComponentMM.ModifyPDF("s2", (RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");
                    _signalComponentMM.ModifyPDF("scb", (RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");
                    _signalComponentMM.ModifyPDF("sg", (RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");

                    // CB, Ipatia tail , DSCB ( tailLEFT,tailRIGHT )
                    if (_manager.second.Option().Contains("modshapetail")) {		      
                        if( _manager.second.Option().Contains("modshapetail[a]")) _signalComponentMM.ModifyPDF("a", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                        else if( _manager.second.Option().Contains("modshapetail[a2]")){
                            _signalComponentMM.ModifyPDF("a2", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                            //TODO: Really needed ? Same logic of scaling right tail from RK imperial here... 
                            _signalComponentMM.ModifyPDF("n2", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scalinv");
                        }
                        else{ 
                			//do both same scale
                			_signalComponentMM.ModifyPDF("a", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
			                _signalComponentMM.ModifyPDF("a2", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
		                }
		                _signalComponentMM.ModifyPDF("acb", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
            		    _signalComponentMM.ModifyPDF("ag", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");		    
                    }
                    // AndCB, AndGaus fractions
                    if (_manager.second.Option().Contains("modshapefrac")) {
                        _signalComponentMM.ModifyPDF("fcb", (RooRealVar *) m_parameterPool->GetShapeParameter("fcb_scale_" + _key), "-scale");
                        _signalComponentMM.ModifyPDF("fg", (RooRealVar *) m_parameterPool->GetShapeParameter("fg_scale_" + _key), "-scale");
                    }
                }
                if (_holderMM.HasComponent(to_string(Sample::Bs))) {
                    FitComponent & _BsComponentMM = _holderMM[to_string(Sample::Bs)];
                    if( _ISANALYTICALSHAPE_(_BsComponentMM) ){
                        _BsComponentMM.SetConstantAllPars();                       
                        if( _holderMM.Configuration().GetProject() != Prj::RK){
                            _BsComponentMM.ModifyPDF("m", (RooRealVar *) m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey), "-shift-offset[" + _dmBsBd + "]");
                            _BsComponentMM.ModifyPDF("mcb", (RooRealVar *) m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey), "-shift-offset[" + _dmBsBd + "]");
                            _BsComponentMM.ModifyPDF("mg", (RooRealVar *) m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey), "-shift-offset[" + _dmBsBd + "]");   
                            _BsComponentMM.ModifyPDF("m2", (RooRealVar *) m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey), "-shift-offset[" + _dmBsBd + "]");                            

                            _BsComponentMM.ModifyPDF("s", (RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");
                            _BsComponentMM.ModifyPDF("scb", (RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");
                            _BsComponentMM.ModifyPDF("sg", (RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");
                            _BsComponentMM.ModifyPDF("s2", (RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");
                        }
                        // else{
                        //     _BsComponentMM.ModifyPDF("m", (RooRealVar *) m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey), "-offset[" + _dmBsBd + "]");
                        //     _BsComponentMM.ModifyPDF("mcb", (RooRealVar *) m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey), "-shift-offset[" + _dmBsBd + "]");
                        //     _BsComponentMM.ModifyPDF("mg", (RooRealVar *) m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey), "-shift-offset[" + _dmBsBd + "]");                            
                        //     _BsComponentMM.ModifyPDF("s", (RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");
                        //     _BsComponentMM.ModifyPDF("scb", (RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");
                        //     _BsComponentMM.ModifyPDF("sg", (RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");                            
                        // }
                        
                        if (_manager.second.Option().Contains("modshapetail")) {
            			  if( _manager.second.Option().Contains("modshapetail[a]"))      _BsComponentMM.ModifyPDF("a", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
			              else if(_manager.second.Option().Contains("modshapetail[a2]")){
                              _BsComponentMM.ModifyPDF("a2", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                              //Really needed ? 
                              _BsComponentMM.ModifyPDF("n2", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scalinv");
                          }
            			  else{
			                //do both same scale
                            _BsComponentMM.ModifyPDF("a", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                            _BsComponentMM.ModifyPDF("a2", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
            			  }
            			  _BsComponentMM.ModifyPDF("acb", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
			              _BsComponentMM.ModifyPDF("ag", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                        }
                        if (_manager.second.Option().Contains("modshapefrac")) {
                            _BsComponentMM.ModifyPDF("fcb", (RooRealVar *) m_parameterPool->GetShapeParameter("fcb_scale_" + _key), "-scale");
                            _BsComponentMM.ModifyPDF("fg", (RooRealVar *) m_parameterPool->GetShapeParameter("fg_scale_" + _key), "-scale");
                        }
                    }
                }
                /* RPhi specifics*/
                if (_holderMM.HasComponent(to_string(Sample::Bd)) && (_holderMM.Configuration().GetProject() == Prj::RPhi)) {
                    FitComponent & _BdComponentMM = _holderMM[to_string(Sample::Bd)];
                    if(_ISANALYTICALSHAPE_(_BdComponentMM)){
                        _BdComponentMM.SetConstantAllPars();
                        _BdComponentMM.ModifyPDF("m", (RooRealVar *) m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey), "-shift-offset[" + _dmBdBs + "]");
                        _BdComponentMM.ModifyPDF("mcb", (RooRealVar *) m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey), "-shift-offset[" + _dmBdBs + "]");
                        _BdComponentMM.ModifyPDF("mg", (RooRealVar *) m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey), "-shift-offset[" + _dmBdBs + "]");
                        _BdComponentMM.ModifyPDF("s", (RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");
                        _BdComponentMM.ModifyPDF("scb", (RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");
                        _BdComponentMM.ModifyPDF("sg", (RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");

                        if (_manager.second.Option().Contains("modshapetail")) {
                            if (_manager.second.Option().Contains("modshapetail[a]"))           _BdComponentMM.ModifyPDF("a", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                            else if( _manager.second.Option().Contains("modshapetail[a2]")){
                                _BdComponentMM.ModifyPDF("a2", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                                _BdComponentMM.ModifyPDF("n2", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scalinv");
                            }
                            else{
                                _BdComponentMM.ModifyPDF("a", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                                _BdComponentMM.ModifyPDF("a2", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");                                
                            }
                            _BdComponentMM.ModifyPDF("acb", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                            _BdComponentMM.ModifyPDF("ag", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                        }
                        if (_manager.second.Option().Contains("modshapefrac")) {
                            _BdComponentMM.ModifyPDF("fcb", (RooRealVar *) m_parameterPool->GetShapeParameter("fcb_scale_" + _key), "-scale");
                            _BdComponentMM.ModifyPDF("fg", (RooRealVar *) m_parameterPool->GetShapeParameter("fg_scale_" + _key), "-scale");
                        }
                    }
                }

                if (_holderMM.HasComponent(to_string(Sample::MisID))) {
                    FitComponent & _MisIDComponentMM = _holderMM[to_string(Sample::MisID)];
                    if(_ISANALYTICALSHAPE_(_MisIDComponentMM)){
                        _MisIDComponentMM.SetConstantAllPars();
                        _MisIDComponentMM.ModifyPDF("m", (RooRealVar *) m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey), "-shift");
                        _MisIDComponentMM.ModifyPDF("mcb", (RooRealVar *) m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey), "-shift");
                        _MisIDComponentMM.ModifyPDF("mg", (RooRealVar *) m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey), "-shift");
                        _MisIDComponentMM.ModifyPDF("m2", (RooRealVar *) m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey), "-shift");


                        _MisIDComponentMM.ModifyPDF("s", (RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");
                        _MisIDComponentMM.ModifyPDF("scb", (RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");
                        _MisIDComponentMM.ModifyPDF("sg", (RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");
                        _MisIDComponentMM.ModifyPDF("s2", (RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");

                        if (_manager.second.Option().Contains("modshapetail")) {
                            if( _manager.second.Option().Contains("modshapetail[a]"))  _MisIDComponentMM.ModifyPDF("a", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                            else if( _manager.second.Option().Contains("modshapetail[a2]")){
                                _MisIDComponentMM.ModifyPDF("a2", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                                _MisIDComponentMM.ModifyPDF("n2", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scalinv");
                            }
                            else{
                                _MisIDComponentMM.ModifyPDF("a", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                                _MisIDComponentMM.ModifyPDF("a2", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");                                 
                            }
                            _MisIDComponentMM.ModifyPDF("acb", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                            _MisIDComponentMM.ModifyPDF("ag", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                        }
                        if (_manager.second.Option().Contains("modshapefrac")) {
                            _MisIDComponentMM.ModifyPDF("fcb", (RooRealVar *) m_parameterPool->GetShapeParameter("fcb_scale_" + _key), "-scale");
                            _MisIDComponentMM.ModifyPDF("fg", (RooRealVar *) m_parameterPool->GetShapeParameter("fg_scale_" + _key), "-scale");
                        }
                    }
                }

                if (_holderMM.HasComponent(to_string(Sample::CombSS))) {
                    FitComponent & _CombSS = _holderMM[to_string(Sample::CombSS)];
                    if(_ISANALYTICALSHAPE_(_CombSS)){
                        _CombSS.SetConstantAllPars();                                              
                        if(_manager.second.Option().Contains("gconstCombSS")){
                            TString _parsGaussConstraint = StripStringBetween(_manager.second.Option(), "gconstCombSS[", "]");
                            auto parsToGaussConstraint = TokenizeString(_parsGaussConstraint, ",");
                            for( auto & pName : parsToGaussConstraint){
                                TString _shapeParameterName = TString::Format("%s_CombSS-%s", pName.Data(), _key.Data());
                                auto * _pComb = (RooRealVar *) m_parameterPool->GetShapeParameter(_shapeParameterName);
                                if( _pComb->getVal() + 10*_pComb->getError() > _pComb->getMax() ){ 
                                    MessageSvc::Warning("SS data gConstSS adjusting ranges for gauss constraints");
                                    _pComb->setMax(  _pComb->getVal() + 10*_pComb->getError() ); 
                                }
                                if( _pComb->getVal() - 10*_pComb->getError() < _pComb->getMin() ){
                                    MessageSvc::Warning("SS data gConstSS adjusting ranges for gauss constraints");
                                    _pComb->setMin(  _pComb->getVal() - 10*_pComb->getError());                                     
                                    if(_shapeParameterName.Contains("sl_") || _shapeParameterName.Contains("m0") ){
                                        if(_pComb->getMin() <0 ){
                                            _pComb->setMin(0.);
                                        }
                                    }
                                }
                                _pComb->setConstant(0);
                                m_parameterPool->AddConstrainedParameter(_pComb);
                            }
                        }                           
                    }
                }//End of CombSS data shape modifiers MM 
                if (_holderMM.HasComponent(to_string(Sample::Comb)) && SettingDef::Fit::useRatioComb ) {
                    if( 
                         ( _holderMM.Configuration().GetQ2bin() == Q2Bin::Low  ||  _holderMM.Configuration().GetQ2bin() == Q2Bin::Central) && 
                        !( _holderMM.Configuration().GetTrigger() == Trigger::L0I && _holderMM.Configuration().GetYear() == Year::Run2p2)
                    ){
                        //Be aware of modshapejpsmm which set _key to the J/Psi one ( see lines above )
                        FitComponent & _CombComponentMM = _holderMM[to_string(Sample::Comb)];
                        TString _ThisSlotKey  = _keyInThisQ2;                    
                        //Retrieve a reference Key for MM, R2p2, L0I! 
                        TString _ReferenceKey = _ThisSlotKey;
                        _ReferenceKey = _ReferenceKey.ReplaceAll(to_string(_holderMM.Configuration().GetYear()), to_string(Year::Run2p2))
                                                     .ReplaceAll(to_string(_holderMM.Configuration().GetTrigger()), to_string(Trigger::L0I)); 
                        if(!( _ThisSlotKey.Contains( to_string(Q2Bin::Low)) ||  _ThisSlotKey.Contains( to_string(Q2Bin::Central)))){
                            MessageSvc::Error("ModifyPDF(CombMM)", _ThisSlotKey, "EXIT_FAILURE");
                        }
                        if(!( _ReferenceKey.Contains( to_string(Q2Bin::Low)) ||  _ThisSlotKey.Contains( to_string(Q2Bin::Central)))){
                            MessageSvc::Error("ModifyPDF(CombMM)", _ThisSlotKey, "EXIT_FAILURE");
                        }
                        //The R2p2-L0I period Reference Shape Parameter.     
                        TString _OldCombSlope      = TString::Format("b_Comb-%s",            _ThisSlotKey.Data());
                        TString _RefCombSlope      = TString::Format("b_Comb-%s",           _ReferenceKey.Data());
                        TString _RatioCombSlope    = TString::Format("b_RatioComb_%s",       _ThisSlotKey.Data());
                        MessageSvc::Info("Comb(MM) [OLD   Slope]"             , _OldCombSlope);        
                        MessageSvc::Info("Comb(MM) [REF   Slope]"             , _RefCombSlope);         
                        MessageSvc::Info("Comb(MM) [SCALE Slope]"             , _RatioCombSlope);                          
                        //Safety Checks : 1) Reference present in fitter, I.e the fit includes R2p2-L0I category ! 
                        if( !m_parameterPool->ShapeParameterExists( _RefCombSlope))   MessageSvc::Error("Cannot useRatioComb, please add in the fit the R2p2-L0I setup, taken as reference", _RefCombSlope , "EXIT_FAILURE");
                        //Safety Checks : 2) Original Comb Slope done with StringToPDF ( -b [XXX]) 
                        if( !m_parameterPool->ShapeParameterExists( _OldCombSlope))   MessageSvc::Error("Cannot find original StringTOPDF parameter slope to replace, FIXME!", _OldCombSlope , "EXIT_FAILURE");
                        //Safety Checks : 3) ParameterPool ShapeParameter is added properly BeforeHand with a "scale(XX) wrt (R2p2-L0I)"
                        if( !m_parameterPool->ShapeParameterExists( _RatioCombSlope)) MessageSvc::Error("Ratio to R2p2-L0I comb slope not found in ParameterPool, FIXME!", _RatioCombSlope , "EXIT_FAILURE");
       
                        //This FitComponent, Fully Replace ReferenceCombSlope , create a totally new PDF.
                        _CombComponentMM.ModifyPDF( "b", (RooRealVar *) m_parameterPool->GetShapeParameter(_RefCombSlope) , "replace" );
                        //Remove from ParameterPool the Old comb-Slope
                        m_parameterPool->RemoveShapeParameter(_OldCombSlope);
                        if( !m_parameterPool->ShapeParameterExists( _OldCombSlope))   MessageSvc::Debug("Deletition Success", _OldCombSlope);
                        //Re-Define the Comb Slope to be a Scale * Run2p2-L0I value!                      
                        _CombComponentMM.ModifyPDF( "b", (RooRealVar *) m_parameterPool->GetShapeParameter(_RatioCombSlope), "-scale");
                    }
                }//End of Comb shape modifiers MM 

                // Analytical convolution
                // The acronym is pretty bad I know, but this is to avoid collision with "conv"
                // Ask Da Yu about this if you want to use it
                if (_holderMM.HasComponent(to_string(Sample::PartReco))) {
                    FitComponent & _partRecoComponentMM = _holderMM[to_string(Sample::PartReco)];
                    if(_ISANALYTICALSHAPE_(_partRecoComponentMM)){
                        if (_manager.second.Option().Contains("acon")) {
                            _partRecoComponentMM.ModifyPDF("s",  (RooRealVar *) m_parameterPool->GetShapeParameter("s_conv_" + _key), "-acon");
                            _partRecoComponentMM.ModifyPDF("s2", (RooRealVar *) m_parameterPool->GetShapeParameter("s_conv_" + _key), "-acon");
                        }else{
                            /*
                            WHEN is FitTo 
                            m2_PartReco-RK-MM-jps-L0L-R2p2
                            m_PartReco-RK-MM-jps-L0L-R2p2
                            s2_PartReco-RK-MM-jps-L0L-R2p2
                            s_PartReco-RK-MM-jps-L0L-R2p2
                            */
                            _partRecoComponentMM.ModifyPDF("m", (RooRealVar *) m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey), "-shift");
                            _partRecoComponentMM.ModifyPDF("m2", (RooRealVar *) m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey), "-shift");
                            _partRecoComponentMM.ModifyPDF("s", (RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");
                            _partRecoComponentMM.ModifyPDF("s2", (RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");                    
                        }
                    }
                    // TODO MAYBE ON MUONS -dataset setup, shift and scale ? 
                    // if( _partRecoComponentMM.Option().Contains("Shift"){
                    //     _partRecoComponentMM.ModifyPDF("s", (RooRealVar *) m_parameterPool->GetShapeParameter("s_conv_" + _key), "-acon");
                    // }
                }// End of PartReco MM modifications
            }// End of HoldersMM loop , inside modshape if statement

            auto _BREMFROMKEY_=[](const TString & _key){
                if (_key.Contains(to_string(Brem::G0))) return Brem::G0;
                if (_key.Contains(to_string(Brem::G1))) return Brem::G1;
                if (_key.Contains(to_string(Brem::G2))) return Brem::G2;
                MessageSvc::Error("_BREMFROMKEY_", (TString) "Only valid for G0,1,2", "EXIT_FAILURE");
                return Brem::All;
            };
            //==============================================================================================================================
            // Deal with the Electron MODE ( IF FIT TO SIGNAL IS DONE WITH MC shapes in each Brem Categories)
            //==============================================================================================================================
            if (_manager.second.HoldersEEBrem().size() != 0) {
                for (const auto & _holder : _manager.second.HoldersEEBrem()) {
                    for (const auto & _keyBremHolderPair : _holder.second) {
                        FitHolder    &    _bremHolder         = _manager.second[_keyBremHolderPair.first];                        
                        FitComponent & _signalComponentBremEE = _bremHolder[_bremHolder.SignalComponentKey()];
                        TString        _key                   = _signalComponentBremEE.Name();                                       
                        _key.ReplaceAll(_bremHolder.SignalComponentKey() + SettingDef::separator, "");
                        _key.ReplaceAll("MC", "").ReplaceAll("CL", "");
                        if (_manager.second.Option().Contains("modshapejpsee") || (_manager.second.Option().Contains("modshapejps") && !_manager.second.Option().Contains("modshapejpsmm"))) _key.ReplaceAll(to_string(_bremHolder.Configuration().GetQ2bin()), to_string(Q2Bin::JPsi));
                        TString _shiftKey    = _key;
                        TString _scaleKey    = _key;
                        TString _bremFracKey = _key; 
                        if (_manager.second.Option().Contains("modshiftrx")) _shiftKey.ReplaceAll(to_string(_bremHolder.Configuration().GetProject()) + SettingDef::separator, ""); //done to share on RK,RKst same shift
                        if (_manager.second.Option().Contains("modscalerx")) _scaleKey.ReplaceAll(to_string(_bremHolder.Configuration().GetProject()) + SettingDef::separator, ""); //done to share on RK,RKst same scale
                        if (_manager.second.Option().Contains("modshifttagprobe")) _shiftKey.ReplaceAll(SettingDef::separator + to_string(_bremHolder.Configuration().GetTrack()), "");
                        if (_manager.second.Option().Contains("modscaletagprobe")) _scaleKey.ReplaceAll(SettingDef::separator + to_string(_bremHolder.Configuration().GetTrack()), "");
                        if (_manager.second.Option().Contains("modscalejpsmm")) _scaleKey.ReplaceAll("EE", "MM");
                        if (!_manager.second.Option().Contains("modshiftbrem")) _shiftKey.ReplaceAll(to_string(Brem::G0), "").ReplaceAll(to_string(Brem::G1), "").ReplaceAll(SettingDef::separator + to_string(Brem::G2), ""); //done because all brems shares the same
                        if (!_manager.second.Option().Contains("modscalebrem")) _scaleKey.ReplaceAll(to_string(Brem::G0), "").ReplaceAll(to_string(Brem::G1), "").ReplaceAll(SettingDef::separator + to_string(Brem::G2), ""); //done because all brems shares the same

                        _shiftKey = CleanString(_shiftKey);
                        _scaleKey = CleanString(_scaleKey);
                        _bremFracKey = CleanString(_bremFracKey);

                        _signalComponentBremEE.SetConstantAllPars();
                        // mass shift (all masses in the shapes)		
                        if( _ALLOW_TO_MODIFY_( "m",   _manager.second.Option()  , "noMSHIFTEE")  ) _signalComponentBremEE.ModifyPDF("m",  (RooRealVar *) m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey), "-shift");
                        if( _ALLOW_TO_MODIFY_( "mcb", _manager.second.Option()  , "noMSHIFTEE")  ) _signalComponentBremEE.ModifyPDF("mcb",(RooRealVar *) m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey), "-shift");
                        if( _ALLOW_TO_MODIFY_( "mg",   _manager.second.Option() , "noMSHIFTEE") )  _signalComponentBremEE.ModifyPDF("mg", (RooRealVar *) m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey), "-shift");
                        if( _ALLOW_TO_MODIFY_( "m2",   _manager.second.Option() , "noMSHIFTEE") )  _signalComponentBremEE.ModifyPDF("m2", (RooRealVar *) m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey), "-shift");
                        // sigma scale (all sigmas in the shape!)
                        if( _ALLOW_TO_MODIFY_( "s",     _manager.second.Option() , "noSSCALEEE")  ) _signalComponentBremEE.ModifyPDF("s",  (RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");
                        if( _ALLOW_TO_MODIFY_( "s2",    _manager.second.Option() , "noSSCALEEE")  ) _signalComponentBremEE.ModifyPDF("s2", (RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");
                        if( _ALLOW_TO_MODIFY_( "sg",    _manager.second.Option() , "noSSCALEEE")  ) _signalComponentBremEE.ModifyPDF("sg", (RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");
                        if( _ALLOW_TO_MODIFY_( "scb",   _manager.second.Option() , "noSSCALEEE")  ) _signalComponentBremEE.ModifyPDF("scb",(RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");
                        if( _manager.second.Option().Contains("modbremgaussfrac") && _BREMFROMKEY_(_bremFracKey) != Brem::G0 ){
                            // fracGauss (RHS!!!) scale factor applied to accomodate RHS bumps in signal EE shapes (only when Brem-splitted fits) 
                            // NB : we must be sure that the fg refers to the gaussian we place on the RHS of signal, this is dependent on the fit setup.
                            MessageSvc::Warning("Adding modbremgaussfrac on Signal Brem Split");                           
                            // if( _BREMFROMKEY_(_bremFracKey) != Brem::G1){
                            if( m_parameterPool->GetShapeParameter("fgBrem_scale_" + _bremFracKey) == nullptr){                                 
                                std::cout<<RED<< "PARAMETER HAS TO EXIST, NULLPTR get : " <<  "fgBrem_scale_" + _bremFracKey << RESET<< std::endl;
                            }                                
                            //HACK, only G2 
                            _signalComponentBremEE.ModifyPDF("fg",  (RooRealVar *) m_parameterPool->GetShapeParameter("fgBrem_scale_" + _bremFracKey), "-scale");                            
                            
                        }
                        // CB tail
                        if (_manager.second.Option().Contains("modshapetail")) {
            			  if( _manager.second.Option().Contains("modshapetail[a]"))        _signalComponentBremEE.ModifyPDF("a", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
            			  else if( _manager.second.Option().Contains("modshapetail[a2]")){
                              _signalComponentBremEE.ModifyPDF("a2", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                              _signalComponentBremEE.ModifyPDF("n2", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scalinv");
                          }
            			  else{ 
                            _signalComponentBremEE.ModifyPDF("a", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                		    _signalComponentBremEE.ModifyPDF("a2", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
            			  }
                        }
                        if (_bremHolder.HasComponent(to_string(Sample::Bs))) {
                            FitComponent & _BsComponentBremEE = _bremHolder[to_string(Sample::Bs)];
                            if(_ISANALYTICALSHAPE_(_BsComponentBremEE) ){
                                PdfType _type = _BsComponentBremEE.Type();
                                //(TYPE = StringToFit even if SignalCopy!)
                                _BsComponentBremEE.SetConstantAllPars();
                                //We must not shift it for RK        
                                for( auto & p : {"m","m2","mcb","mg"}){
                                    MessageSvc::Warning("Adding mass shifts on Bs signal Copy (brem-by brem)");
                                    if( _ALLOW_TO_MODIFY_( p,    _manager.second.Option() , "noMSHIFTEE")  ){ 
                                        _BsComponentBremEE.ModifyPDF(p,  (RooRealVar *) m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey),   "-shift-offset[" + _dmBsBd + "]");                                
                                    }else{
                                        //if not allowed to modify and it is a SignalCopy, offset it only.
                                        MessageSvc::Warning("Applying shift with no variables (nullptr)");
                                       _BsComponentBremEE.ModifyPDF(p,  nullptr ,   "-offset[" + _dmBsBd + "]");
                                    }
                                }
                                if( _ALLOW_TO_MODIFY_( "s",    _manager.second.Option() , "noSSCALEEE")  )_BsComponentBremEE.ModifyPDF("s",  (RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");
                                if( _ALLOW_TO_MODIFY_( "s2",   _manager.second.Option() , "noSSCALEEE")  )_BsComponentBremEE.ModifyPDF("s2", (RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");
                                if( _ALLOW_TO_MODIFY_( "scb",  _manager.second.Option() , "noSSCALEEE")  )_BsComponentBremEE.ModifyPDF("scb",(RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");
                                if( _ALLOW_TO_MODIFY_( "sg",   _manager.second.Option() , "noSSCALEEE")  )_BsComponentBremEE.ModifyPDF("sg", (RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");
                                if( _manager.second.Option().Contains("modbremgaussfrac") &&  _BREMFROMKEY_(_bremFracKey) != Brem::G0){
                                    // fracGauss (RHS!!!) scale factor applied to accomodate RHS bumps in signal EE shapes (only when Brem-splitted fits)
                                    MessageSvc::Warning("Adding modbremgaussfrac on Bs Brem Component Copy");
                                    // if( _BREMFROMKEY_(_bremFracKey) != Brem::G1){
                                    if( m_parameterPool->GetShapeParameter("fgBrem_scale_" + _bremFracKey) == nullptr){                                 
                                        std::cout<<RED<< "PARAMETER HAS TO EXIST, NULLPTR get : " <<  "fgBrem_scale_" + _bremFracKey << RESET<< std::endl;
                                    }
                                    _BsComponentBremEE.ModifyPDF("fg",  (RooRealVar *) m_parameterPool->GetShapeParameter("fgBrem_scale_" + _bremFracKey), "-scale");                            
                                    // }
                                    
                                }
                            }
                            if (_manager.second.Option().Contains("modshapetail")) {
                                if( _manager.second.Option().Contains("modshapetail[a]") )      _BsComponentBremEE.ModifyPDF("a", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                                else if( _manager.second.Option().Contains("modshapetail[a2]") ){
                                    _BsComponentBremEE.ModifyPDF("a2", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                                    _BsComponentBremEE.ModifyPDF("n2", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scalinv");
                                }
                                else{
                                    _BsComponentBremEE.ModifyPDF("a", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                                    _BsComponentBremEE.ModifyPDF("a2", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                                }
                            }
                        }
                        /* RPhi specifics */
                        if (_bremHolder.HasComponent(to_string(Sample::Bd)) && (_bremHolder.Configuration().GetProject() == Prj::RPhi)) {                            
                            FitComponent & _BdComponentBremEE = _bremHolder[to_string(Sample::Bd)];
                            _BdComponentBremEE.SetConstantAllPars();
                            if(_ISANALYTICALSHAPE_(_BdComponentBremEE)){
                                //TODO : want to adopt the noSSCALE, noMMSHIFT ? 
                                // shifts 
                                if( _ALLOW_TO_MODIFY_( "m",   _manager.second.Option() , "noMSHIFTEE")  ) _BdComponentBremEE.ModifyPDF("m", (RooRealVar *) m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey),   "-shift-offset[" + _dmBdBs + "]");
                                if( _ALLOW_TO_MODIFY_( "mcb", _manager.second.Option() , "noMSHIFTEE")  ) _BdComponentBremEE.ModifyPDF("mcb", (RooRealVar *) m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey), "-shift-offset[" + _dmBdBs + "]");
                                if( _ALLOW_TO_MODIFY_( "mg",  _manager.second.Option() , "noMSHIFTEE")  ) _BdComponentBremEE.ModifyPDF("mg", (RooRealVar *) m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey),  "-shift-offset[" + _dmBdBs + "]");
                                if( _ALLOW_TO_MODIFY_( "m2",  _manager.second.Option() , "noMSHIFTEE")  ) _BdComponentBremEE.ModifyPDF("m2", (RooRealVar *) m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey),  "-shift-offset[" + _dmBdBs + "]");
                                // scales
                                if( _ALLOW_TO_MODIFY_( "s",   _manager.second.Option() , "noSSCALEEE")  ) _BdComponentBremEE.ModifyPDF("s", (RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey),   "-scale");
                                if( _ALLOW_TO_MODIFY_( "s2",  _manager.second.Option() , "noSSCALEEE")  ) _BdComponentBremEE.ModifyPDF("s2", (RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey),  "-scale");
                                if( _ALLOW_TO_MODIFY_( "scb", _manager.second.Option() , "noSSCALEEE")  ) _BdComponentBremEE.ModifyPDF("scb", (RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");
                                if( _ALLOW_TO_MODIFY_( "sg",  _manager.second.Option() , "noSSCALEEE")  ) _BdComponentBremEE.ModifyPDF("sg", (RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey),  "-scale");
                                if (_manager.second.Option().Contains("modshapetail")){         
                                    if(      _manager.second.Option().Contains("modshapetail[a]"))  _BdComponentBremEE.ModifyPDF("a", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                                    else if( _manager.second.Option().Contains("modshapetail[a2]")){
                                        _BdComponentBremEE.ModifyPDF("a2", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                                        _BdComponentBremEE.ModifyPDF("n2", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scalinv");
                                    }
                                    else{
                                        _BdComponentBremEE.ModifyPDF("a", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                                        _BdComponentBremEE.ModifyPDF("a2", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");			                
                                    }   
                                }
                            }
                        }
                    }
                }
                // Scale and shift the misID component stored in Holders EE
                for (const auto & _keyHolderPair : _manager.second.HoldersEE()){
                    FitHolder & _holderEE = _manager.second[_keyHolderPair.first];
                    TString     _key      = _holderEE.Name();
                    _key.ReplaceAll("MC", "").ReplaceAll("CL", "");
                    
                    if (_manager.second.Option().Contains("modshapejpsee") || (_manager.second.Option().Contains("modshapejps") && !_manager.second.Option().Contains("modshapejpsmm"))) _key.ReplaceAll(to_string(_holderEE.Configuration().GetQ2bin()), to_string(Q2Bin::JPsi));
                    _key = CleanString(_key);
                    TString _shiftKey = _key;
                    TString _scaleKey = _key;
                    if (_manager.second.Option().Contains("modshiftrx")) _shiftKey.ReplaceAll(to_string(_holderEE.Configuration().GetProject()) + SettingDef::separator, "");
                    if (_manager.second.Option().Contains("modscalerx")) _scaleKey.ReplaceAll(to_string(_holderEE.Configuration().GetProject()) + SettingDef::separator, "");
                    if (_manager.second.Option().Contains("modshifttagprobe")) _shiftKey.ReplaceAll(SettingDef::separator + to_string(_holderEE.Configuration().GetTrack()), "");
                    if (_manager.second.Option().Contains("modscaletagprobe")) _scaleKey.ReplaceAll(SettingDef::separator + to_string(_holderEE.Configuration().GetTrack()), "");
                    if (_manager.second.Option().Contains("modscalejpsmm")) _scaleKey.ReplaceAll("EE", "MM");
                    _shiftKey = CleanString(_shiftKey);
                    _scaleKey = CleanString(_scaleKey);

                    if (_holderEE.HasComponent(to_string(Sample::MisID))) {
                        FitComponent & _MisIDComponentEE = _holderEE[to_string(Sample::MisID)];
                        if(_ISANALYTICALSHAPE_(_MisIDComponentEE)){
                            _MisIDComponentEE.SetConstantAllPars();
                            //do it regardless? 
                            //shift massses 
                            if( _ALLOW_TO_MODIFY_( "m",     _manager.second.Option() , "noMSHIFTEE")  )_MisIDComponentEE.ModifyPDF("m",   (RooRealVar *) m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey), "-shift");
                            if( _ALLOW_TO_MODIFY_( "mcb",   _manager.second.Option() , "noMSHIFTEE")  )_MisIDComponentEE.ModifyPDF("mcb", (RooRealVar *) m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey), "-shift");
                            if( _ALLOW_TO_MODIFY_( "mg",    _manager.second.Option() , "noMSHIFTEE")  )_MisIDComponentEE.ModifyPDF("mg",  (RooRealVar *) m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey), "-shift");
                            if( _ALLOW_TO_MODIFY_( "m2",    _manager.second.Option() , "noMSHIFTEE")  )_MisIDComponentEE.ModifyPDF("m2",  (RooRealVar *) m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey), "-shift");
                            //scales cores
                            if( _ALLOW_TO_MODIFY_( "s",     _manager.second.Option() , "noSSCALEEE")  )_MisIDComponentEE.ModifyPDF("s",   (RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");
                            if( _ALLOW_TO_MODIFY_( "scb",   _manager.second.Option() , "noSSCALEEE")  )_MisIDComponentEE.ModifyPDF("scb", (RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");
                            if( _ALLOW_TO_MODIFY_( "sg",    _manager.second.Option() , "noSSCALEEE")  )_MisIDComponentEE.ModifyPDF("sg",  (RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");
                            if( _ALLOW_TO_MODIFY_( "s2",    _manager.second.Option() , "noSSCALEEE")  )_MisIDComponentEE.ModifyPDF("s2", (RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");                            

                            if (_manager.second.Option().Contains("modshapetail")) {
                			      if( _manager.second.Option().Contains("modshapetail[a]")) _MisIDComponentEE.ModifyPDF("a", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                			      else if( _manager.second.Option().Contains("modshapetail[a2]")){
                                      _MisIDComponentEE.ModifyPDF("a2", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                                      _MisIDComponentEE.ModifyPDF("n2", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scalinv");
                                  }
                			      else {
                                    _MisIDComponentEE.ModifyPDF("a", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                                    _MisIDComponentEE.ModifyPDF("a2", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                                }
                            }
                        }
                    }

                    if (_holderEE.HasComponent(to_string(Sample::Leakage))){
                        FitComponent & _LeakageComponent = _holderEE[to_string(Sample::Leakage)];
                        if(_ISANALYTICALSHAPE_(_LeakageComponent)){
                            _LeakageComponent.SetConstantAllPars();
                            //---- s-scale / m-shift as main peak ??? --- //
                            //---- MOSTLY RELEVANT FOR THE PART RECO MODELLING IN RK/RKSt for noDTF fits! ----//
                        }
                    }
                    if (_holderEE.HasComponent(to_string(Sample::Psi2JPsX))){
                        FitComponent & _Psi2JPsX = _holderEE[to_string(Sample::Psi2JPsX)];
                        if(_ISANALYTICALSHAPE_(_Psi2JPsX)){                        
                            _Psi2JPsX.SetConstantAllPars();
                            //---- s-scale / m-shift as main peak ??? --- //
                            //---- MOSTLY RELEVANT FOR THE PART RECO MODELLING IN RK/RKSt for noDTF fits! ----//
                        }
                    }
                    if( _holderEE.HasComponent(to_string(Sample::Psi2JPsPiPi))){
                        FitComponent & _Psi2JPsPiPi = _holderEE[to_string(Sample::Psi2JPsPiPi)];
                        if(_ISANALYTICALSHAPE_(_Psi2JPsPiPi)){                        
                            _Psi2JPsPiPi.SetConstantAllPars();
                            //---- s-scale / m-shift as main peak ??? --- //
                            //---- MOSTLY RELEVANT FOR THE PART RECO MODELLING IN RK/RKSt for noDTF fits! ----//
                        }
                    }
                    //SPECIAL 
                    if (_holderEE.HasComponent(to_string(Sample::Comb))) {
                        FitComponent & _Comb = _holderEE[to_string(Sample::Comb)];
                        MessageSvc::Warning("gConstSS merge shape parameters for turn-on curve");
                        if(_manager.second.Option().Contains("gconstComb")){
                            MessageSvc::Warning("gConstSS merge shape parameters for turn-on curve");
                            TString _keyComb = _Comb.Name();
                            _keyComb.ReplaceAll(to_string(_holderEE.Configuration().GetTrigger()), "");
                            _keyComb.ReplaceAll(to_string(_holderEE.Configuration().GetYear()), "");
                            _keyComb           = CleanString(_keyComb);
                            TString _parsGaussConstraint = StripStringBetween(_manager.second.Option(), "gconstComb[", "]");
                            auto parsToGaussConstraint = TokenizeString(_parsGaussConstraint, ",");
                            for( auto & pName : parsToGaussConstraint){
                                TString _shapeParameterNameMod = "";
                                _shapeParameterNameMod = TString::Format("%s_%s", pName.Data(), _keyComb.Data());
                                _Comb.ModifyPDF(pName.Data(), (RooRealVar *) m_parameterPool->GetShapeParameter(_shapeParameterNameMod), "-replace");
                                auto * _pComb = (RooRealVar *) m_parameterPool->GetShapeParameter(_shapeParameterNameMod);
                                _pComb->setConstant(0);
                                m_parameterPool->AddConstrainedParameter(_pComb);
                            }
                        }
                    }
                    if (_holderEE.HasComponent(to_string(Sample::CombSS))) {
                        FitComponent & _CombSS = _holderEE[to_string(Sample::CombSS)];
                        if(_ISANALYTICALSHAPE_(_CombSS)){
                            _CombSS.SetConstantAllPars();
                            if(_manager.second.Option().Contains("gconstCombSS")){
                                TString _parsGaussConstraint = StripStringBetween(_manager.second.Option(), "gconstCombSS[", "]");
                                auto parsToGaussConstraint = TokenizeString(_parsGaussConstraint, ",");
                                for( auto & pName : parsToGaussConstraint){
                                    TString _shapeParameterName = "";
                                    // if (pName.Contains("c1") || pName.Contains("c2")) 
                                    //     _shapeParameterName = TString::Format("%s", pName.Data());
                                    // else
                                    _shapeParameterName = TString::Format("%s_CombSS-%s", pName.Data(), _key.Data());
                                    auto * _pComb = (RooRealVar *) m_parameterPool->GetShapeParameter(_shapeParameterName);
                                    if( _pComb->getVal() + 10*_pComb->getError() > _pComb->getMax() ){ 
                                        MessageSvc::Warning("SS data gConstSS adjusting ranges for gauss constraints (max)");
                                        _pComb->setMax(  _pComb->getVal() + 10*_pComb->getError()); 
                                    }
                                    if( _pComb->getVal() - 10*_pComb->getError() < _pComb->getMin() ){
                                        MessageSvc::Warning("SS data gConstSS adjusting ranges for gauss constraints (min)");
                                        _pComb->setMin(  _pComb->getVal() - 10*_pComb->getError());                                     
                                        if(_shapeParameterName.Contains("sl_") || _shapeParameterName.Contains("m0") ){
                                            if(_pComb->getMin() <0 ){
                                                _pComb->setMin(0.);
                                            }
                                        }                                        
                                    }
                                    _pComb->setConstant(0);
                                    m_parameterPool->AddConstrainedParameter(_pComb);
                                }
                            }
                        }                                        
                    }
                    if (_holderEE.HasComponent(to_string(Sample::Comb)) && SettingDef::Fit::useRatioComb ) {
                        if( 
                             ( _holderEE.Configuration().GetQ2bin()    == Q2Bin::Low   ||  _holderEE.Configuration().GetQ2bin() == Q2Bin::Central) && 
                            !( _holderEE.Configuration().GetTrigger() == Trigger::L0I && _holderEE.Configuration().GetYear() == Year::Run2p2)
                        ){
                            //Be aware of modshapejpsmm which set _key to the J/Psi one ( see lines above )
                            FitComponent & _CombComponentEE = _holderEE[to_string(Sample::Comb)];
                            TString     _keyInThisQ2      = _CombComponentEE.Name();
                            _keyInThisQ2.ReplaceAll(to_string(Sample::Comb) + SettingDef::separator, "");
                            _keyInThisQ2.ReplaceAll("MC", "").ReplaceAll("CL", "");
                            _keyInThisQ2 = CleanString(_keyInThisQ2); 
                            
                            TString _ThisSlotKey  = _keyInThisQ2;                    
                            TString _ReferenceKey = _keyInThisQ2;
                            _ReferenceKey = _ReferenceKey.ReplaceAll(to_string(_holderEE.Configuration().GetYear()), to_string(Year::Run2p2))
                                                                .ReplaceAll(to_string(_holderEE.Configuration().GetTrigger()), to_string(Trigger::L0I)); 
                            if(!( _ThisSlotKey.Contains( to_string(Q2Bin::Low)) ||  _ThisSlotKey.Contains( to_string(Q2Bin::Central)))){
                                MessageSvc::Error("ModifyPDF(CombEE)", _ThisSlotKey, "EXIT_FAILURE");
                            }
                            if(!( _ReferenceKey.Contains( to_string(Q2Bin::Low)) ||  _ReferenceKey.Contains( to_string(Q2Bin::Central)))){
                                MessageSvc::Error("ModifyPDF(CombEE)", _ReferenceKey, "EXIT_FAILURE");
                            }
                            //The R2p2-L0I period Reference Shape Parameter.     
                            TString _OldCombSlope      = TString::Format("b_Comb-%s",            _ThisSlotKey.Data());
                            TString _RefCombSlope      = TString::Format("b_Comb-%s",           _ReferenceKey.Data());
                            TString _RatioCombSlope    = TString::Format("b_RatioComb_%s",       _ThisSlotKey.Data());
                            MessageSvc::Info("Comb(EE) [OLD   Slope]" , _OldCombSlope);        
                            MessageSvc::Info("Comb(EE) [REF   Slope]" , _RefCombSlope);         
                            MessageSvc::Info("Comb(EE) [SCALE Slope]" , _RatioCombSlope);                             
                            //Safety Checks : 1) Reference present in fitter, I.e the fit includes R2p2-L0I category ! 
                            if( !m_parameterPool->ShapeParameterExists( _RefCombSlope) ) MessageSvc::Error("Cannot useRatioComb, please add in the fit the R2p2-L0I setup, taken as reference", _RefCombSlope , "EXIT_FAILURE");
                            //Safety Checks : 2) Original Comb Slope done with StringToPDF ( -b [XXX]) 
                            if( !m_parameterPool->ShapeParameterExists( _OldCombSlope))   MessageSvc::Error("Cannot find original StringTOPDF parameter slope to replace, FIXME!", _OldCombSlope , "EXIT_FAILURE");
                            //Safety Checks : 3) ParameterPool ShapeParameter is added properly BeforeHand with a "scale(XX) wrt (R2p2-L0I)"
                            if( !m_parameterPool->ShapeParameterExists( _RatioCombSlope)) MessageSvc::Error("Ratio to R2p2-L0I comb slope not found in ParameterPool, FIXME!", _RatioCombSlope , "EXIT_FAILURE");
        
                            //This FitComponent, Fully Replace ReferenceCombSlope , create a totally new PDF.
                            _CombComponentEE.ModifyPDF( "b", (RooRealVar *) m_parameterPool->GetShapeParameter(_RefCombSlope) , "replace" );
                            //Remove from ParameterPool the Old comb-Slope
                            m_parameterPool->RemoveShapeParameter(_OldCombSlope);
                            if( !m_parameterPool->ShapeParameterExists( _OldCombSlope))   MessageSvc::Debug("Deletition Success", _OldCombSlope);
                            //Re-Define the Comb Slope to be a Scale * Run2p2-L0I value!                      
                            _CombComponentEE.ModifyPDF( "b", (RooRealVar *) m_parameterPool->GetShapeParameter(_RatioCombSlope), "-scale");
                        }
                    }//End of Comb shape modifiers MM 
                    if ( _holderEE.HasComponent(to_string(Sample::DataDrivenEMisID))){
                        FitComponent & _DataDrivenShape = _holderEE[to_string(Sample::DataDrivenEMisID)];
                        _DataDrivenShape.SetConstantAllPars();
                    }                    
                }
            } else {
                //When there is no BremHolder in the list. Signal not Brem Splitted!
                for (const auto & _keyHolderPair : _manager.second.HoldersEE()) {
                    FitHolder & _holderEE = _manager.second[_keyHolderPair.first];
                    TString     _key      = _holderEE[_holderEE.SignalComponentKey()].Name();
                    _key.ReplaceAll(_holderEE.SignalComponentKey() + SettingDef::separator, "");
                    _key.ReplaceAll("MC", "").ReplaceAll("CL", "");
                    if (_manager.second.Option().Contains("modshapejpsee") || (_manager.second.Option().Contains("modshapejps") && !_manager.second.Option().Contains("modshapejpsmm"))) _key.ReplaceAll(to_string(_holderEE.Configuration().GetQ2bin()), to_string(Q2Bin::JPsi));
                    _key = CleanString(_key);

                    TString _shiftKey = _key;
                    TString _scaleKey = _key;
                    TString _rhsfracKey = _key;
                    if (_manager.second.Option().Contains("modshiftrx")) _shiftKey.ReplaceAll(to_string(_holderEE.Configuration().GetProject()) + SettingDef::separator, "");
                    if (_manager.second.Option().Contains("modscalerx")) _scaleKey.ReplaceAll(to_string(_holderEE.Configuration().GetProject()) + SettingDef::separator, "");
                    if (_manager.second.Option().Contains("modshifttagprobe")) _shiftKey.ReplaceAll(SettingDef::separator + to_string(_holderEE.Configuration().GetTrack()), "");
                    if (_manager.second.Option().Contains("modscaletagprobe")) _scaleKey.ReplaceAll(SettingDef::separator + to_string(_holderEE.Configuration().GetTrack()), "");                    
                    if (_manager.second.Option().Contains("modscalejpsmm")) _scaleKey.ReplaceAll("EE", "MM");
                    _shiftKey = CleanString(_shiftKey);
                    _scaleKey = CleanString(_scaleKey);
                    _rhsfracKey = CleanString(_rhsfracKey);
                    FitComponent & _signalComponentEE = _holderEE[_holderEE.SignalComponentKey()];
                    if(_ISANALYTICALSHAPE_(_signalComponentEE)){
                        _signalComponentEE.SetConstantAllPars();
                        //mass shift (m, mcb, mg, m2)
                        if( _ALLOW_TO_MODIFY_( "m",   _manager.second.Option() ,  "noMSHIFTEE")  ) _signalComponentEE.ModifyPDF("m",  (RooRealVar *) m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey), "-shift");
                        if( _ALLOW_TO_MODIFY_( "mcb", _manager.second.Option() ,  "noMSHIFTEE")  ) _signalComponentEE.ModifyPDF("mcb",(RooRealVar *) m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey), "-shift");
                        if( _ALLOW_TO_MODIFY_( "mg",   _manager.second.Option() , "noMSHIFTEE") ) _signalComponentEE.ModifyPDF("mg", (RooRealVar *) m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey), "-shift");
                        if( _ALLOW_TO_MODIFY_( "m2",   _manager.second.Option() , "noMSHIFTEE") ) _signalComponentEE.ModifyPDF("m2", (RooRealVar *) m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey), "-shift");
                        // sigma scale (s,s2,sg,scb)
                        if( _ALLOW_TO_MODIFY_( "s",     _manager.second.Option() , "noSSCALEEE")  ) _signalComponentEE.ModifyPDF("s",  (RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");
                        if( _ALLOW_TO_MODIFY_( "s2",    _manager.second.Option() , "noSSCALEEE")  ) _signalComponentEE.ModifyPDF("s2", (RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");
                        if( _ALLOW_TO_MODIFY_( "sg",    _manager.second.Option() , "noSSCALEEE")  ) _signalComponentEE.ModifyPDF("sg", (RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");
                        if( _ALLOW_TO_MODIFY_( "scb",   _manager.second.Option() , "noSSCALEEE")  ) _signalComponentEE.ModifyPDF("scb",(RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");                        	
                        if( _manager.second.Option().Contains("modscaleLR")){
                            if( _ALLOW_TO_MODIFY_( "sL",    _manager.second.Option() , "noSSCALEEE")  )_signalComponentEE.ModifyPDF("sL", (RooRealVar *) m_parameterPool->GetShapeParameter("s_scaleL_" + _scaleKey), "-scale");
                            if( _ALLOW_TO_MODIFY_( "sR",    _manager.second.Option() , "noSSCALEEE")  )_signalComponentEE.ModifyPDF("sR", (RooRealVar *) m_parameterPool->GetShapeParameter("s_scaleR_" + _scaleKey), "-scale");                            
                        }
                        // CB, Ipatia tail
                        if (_manager.second.Option().Contains("modshapetail")) {
                            if(     _manager.second.Option().Contains("modshapetail[a]"))   _signalComponentEE.ModifyPDF("a", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                            else if( _manager.second.Option().Contains("modshapetail[a2]")){
                                _signalComponentEE.ModifyPDF("a2", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                                _signalComponentEE.ModifyPDF("n2", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scalinv");
                            }
                            else{
                                _signalComponentEE.ModifyPDF("a", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                                _signalComponentEE.ModifyPDF("a2", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");                                
                            }
                            _signalComponentEE.ModifyPDF("acb", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                            _signalComponentEE.ModifyPDF("ag", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                        }
                        //TODO: add the modfracbrem stuff for RHS tail wgeb Brem merged, check Keys naming....when shared to rare!
                        if( _manager.second.Option().Contains("modbremgaussfrac") ){
                            // fracGauss (RHS!!!) scale factor applied to accomodate RHS bumps in signal EE shapes (only when Brem-splitted fits) 
                            // NB : we must be sure that the fg refers to the gaussian we place on the RHS of signal, this is dependent on the fit setup.
                            MessageSvc::Warning("Adding modbremgaussfrac on signal EE merged");
                            if( m_parameterPool->GetShapeParameter("fgBrem_scale_" + _rhsfracKey) == nullptr){                                 
                                std::cout<<RED<< "PARAMETER HAS TO EXIST, NULLPTR get : " <<  "fgBrem_scale_" + _rhsfracKey << RESET<< std::endl;
                            }
                            _signalComponentEE.ModifyPDF("fg",  (RooRealVar *) m_parameterPool->GetShapeParameter("fgBrem_scale_" + _rhsfracKey), "-scale");                            
                        }
                    }

                    if (_holderEE.HasComponent(to_string(Sample::Bs))) {
                        FitComponent & _BsComponentEE = _holderEE[to_string(Sample::Bs)];
                        if(_ISANALYTICALSHAPE_(_BsComponentEE)){
                            _BsComponentEE.SetConstantAllPars();                          
                            //TODO : it has always been a SignalCopy for RKst, for RK it's from B0 -> KstJPs EE shifted and made as ROoKeyPdf, we should be ok until this is the setup, else we need to update this...                        
                            for( TString  massParameter : {"m", "m2", "mcb","mg"} ) {
                                if( _ALLOW_TO_MODIFY_( massParameter,    _manager.second.Option() , "noMSHIFTEE")  ){ 
                                    //modify shape parameter with the main mass shift factor + a offset for the delta masses
                                    _BsComponentEE.ModifyPDF(massParameter,  (RooRealVar *) m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey),   "-shift-offset[" + _dmBsBd + "]");                                
                                }else{
                                    //modify shape with just a delta mass parameter
                                    _BsComponentEE.ModifyPDF(massParameter,  nullptr ,   "-offset[" + _dmBsBd + "]");
                                }
                            }
                            if( _ALLOW_TO_MODIFY_( "s",    _manager.second.Option() , "noSSCALEEE")  ) _BsComponentEE.ModifyPDF("s",  (RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");
                            if( _ALLOW_TO_MODIFY_( "s2",   _manager.second.Option() , "noSSCALEEE")  ) _BsComponentEE.ModifyPDF("s2", (RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");
                            if( _ALLOW_TO_MODIFY_( "scb",  _manager.second.Option() , "noSSCALEEE")  ) _BsComponentEE.ModifyPDF("scb",(RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");
                            if( _ALLOW_TO_MODIFY_( "sg",   _manager.second.Option() , "noSSCALEEE")  ) _BsComponentEE.ModifyPDF("sg", (RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");
                            if (_manager.second.Option().Contains("modshapetail")) {
                                if (_manager.second.Option().Contains("modshapetail[a]")) _BsComponentEE.ModifyPDF("a", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                                else if( _manager.second.Option().Contains("modshapetail[a2]")){
                                    _BsComponentEE.ModifyPDF("a2", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                                    _BsComponentEE.ModifyPDF("n2", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scalinv");
                                }
                                else{
                                    _BsComponentEE.ModifyPDF("a", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                                    _BsComponentEE.ModifyPDF("a2", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                                }
                                _BsComponentEE.ModifyPDF("acb", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                                _BsComponentEE.ModifyPDF("ag", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                            }
                            //TODO: add the modfracbrem stuff for RHS tail
                            if( _manager.second.Option().Contains("modbremgaussfrac") ){
                                // fracGauss (RHS!!!) scale factor applied to accomodate RHS bumps in signal EE shapes (only when Brem-splitted fits) 
                                // NB : we must be sure that the fg refers to the gaussian we place on the RHS of signal, this is dependent on the fit setup.
                                MessageSvc::Warning("Adding modbremgaussfrac on Bs Copy brem merged");
                                if( m_parameterPool->GetShapeParameter("fgBrem_scale_" + _rhsfracKey) == nullptr){                                 
                                    std::cout<<RED<< "PARAMETER HAS TO EXIST, NULLPTR get : " <<  "fgBrem_scale_" + _rhsfracKey << RESET<< std::endl;
                                }
                                _BsComponentEE.ModifyPDF("fg",  (RooRealVar *) m_parameterPool->GetShapeParameter("fgBrem_scale_" + _rhsfracKey), "-scale");                            
                            }                            
                        }
                    }
                    /* RPhi specific */
                    if (_holderEE.HasComponent(to_string(Sample::Bd)) && (_holderEE.Configuration().GetProject() == Prj::RPhi)) {
                        FitComponent & _BdComponentEE = _holderEE[to_string(Sample::Bd)];
                        if(_ISANALYTICALSHAPE_(_BdComponentEE)){
                            _BdComponentEE.SetConstantAllPars();
                            for( TString  massParameter : {"m", "m2", "mcb","mg"}){
                                if( _ALLOW_TO_MODIFY_( massParameter,    _manager.second.Option() , "noMSHIFTEE")  ){ 
                                    //modify shape parameter with the main mass shift factor + a offset for the delta masses
                                    _BdComponentEE.ModifyPDF(massParameter,  (RooRealVar *) m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey),   "-shift-offset[" + _dmBdBs + "]");                                
                                }else{
                                    //modify shape with just a delta mass parameter
                                    _BdComponentEE.ModifyPDF(massParameter,  nullptr ,   "-offset[" + _dmBdBs + "]");
                                }
                            }     
                            if( _ALLOW_TO_MODIFY_( "s",    _manager.second.Option() , "noSSCALEEE")) _BdComponentEE.ModifyPDF("s", (RooRealVar *)  m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");
                            if( _ALLOW_TO_MODIFY_( "s2",   _manager.second.Option() , "noSSCALEEE")) _BdComponentEE.ModifyPDF("s2", (RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");
                            if( _ALLOW_TO_MODIFY_( "scb",  _manager.second.Option() , "noSSCALEEE")) _BdComponentEE.ModifyPDF("scb", (RooRealVar *)m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");
                            if( _ALLOW_TO_MODIFY_( "sg",   _manager.second.Option() , "noSSCALEEE")) _BdComponentEE.ModifyPDF("sg", (RooRealVar *) m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");
                            if (_manager.second.Option().Contains("modshapetail")) {
                                if(_manager.second.Option().Contains("modshapetail[a]"))       _BdComponentEE.ModifyPDF("a", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                                else if(_manager.second.Option().Contains("modshapetail[a2]")){
                                    _BdComponentEE.ModifyPDF("a2", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                                    _BdComponentEE.ModifyPDF("n2", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scalinv");
                                }
                                else{
                                    _BdComponentEE.ModifyPDF("a", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                                    _BdComponentEE.ModifyPDF("a2", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                                }
                                _BdComponentEE.ModifyPDF("acb", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                                _BdComponentEE.ModifyPDF("ag", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                            }
                        }
                    }

                    if (_holderEE.HasComponent(to_string(Sample::MisID))) {
                        FitComponent & _MisIDComponentEE = _holderEE[to_string(Sample::MisID)];
                        if(_ISANALYTICALSHAPE_(_MisIDComponentEE)){
                            _MisIDComponentEE.SetConstantAllPars();

                            if( _ALLOW_TO_MODIFY_( "m",   _manager.second.Option() , "noMSHIFTEE")  ) _MisIDComponentEE.ModifyPDF("m", (RooRealVar *)     m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey), "-shift");
                            if( _ALLOW_TO_MODIFY_( "mcb", _manager.second.Option() , "noMSHIFTEE")  ) _MisIDComponentEE.ModifyPDF("mcb", (RooRealVar *)   m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey), "-shift");
                            if( _ALLOW_TO_MODIFY_( "mg",  _manager.second.Option() , "noMSHIFTEE")  ) _MisIDComponentEE.ModifyPDF("mg", (RooRealVar *)    m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey), "-shift");
                            if( _ALLOW_TO_MODIFY_( "m2",  _manager.second.Option() , "noMSHIFTEE")  ) _MisIDComponentEE.ModifyPDF("m2", (RooRealVar *)    m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey), "-shift");
                            
                            if( _ALLOW_TO_MODIFY_( "s",   _manager.second.Option() , "noSSCALEEE")  ) _MisIDComponentEE.ModifyPDF("s", (RooRealVar *)     m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");
                            if( _ALLOW_TO_MODIFY_( "scb", _manager.second.Option() , "noSSCALEEE")  ) _MisIDComponentEE.ModifyPDF("scb", (RooRealVar *)   m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");
                            if( _ALLOW_TO_MODIFY_( "sg",  _manager.second.Option() , "noSSCALEEE")  ) _MisIDComponentEE.ModifyPDF("sg", (RooRealVar *)    m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");
                            if( _ALLOW_TO_MODIFY_( "s2",  _manager.second.Option() , "noSSCALEEE")  ) _MisIDComponentEE.ModifyPDF("s2", (RooRealVar *)    m_parameterPool->GetShapeParameter("s_scale_" + _scaleKey), "-scale");
                            if (_manager.second.Option().Contains("modshapetail")) {
                                if( _manager.second.Option().Contains("modshapetail[a]"))        _MisIDComponentEE.ModifyPDF("a", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                                else if( _manager.second.Option().Contains("modshapetail[a2]")){
                                    _MisIDComponentEE.ModifyPDF("a2", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                                    _MisIDComponentEE.ModifyPDF("n2", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scalinv");
                                }
                                else{
                                    _MisIDComponentEE.ModifyPDF("a", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                                    _MisIDComponentEE.ModifyPDF("a2", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                                }
                                _MisIDComponentEE.ModifyPDF("acb", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                                _MisIDComponentEE.ModifyPDF("ag", (RooRealVar *) m_parameterPool->GetShapeParameter("a_scale_" + _key), "-scale");
                            }
                        }
                    }

                    if (_holderEE.HasComponent(to_string(Sample::Leakage))){
                        FitComponent & _LeakageComponent = _holderEE[to_string(Sample::Leakage)];
                        if(_ISANALYTICALSHAPE_(_LeakageComponent)){
                            _LeakageComponent.SetConstantAllPars();
                            //---- s-scale / m-shift as main peak ??? --- //
                            //---- MOSTLY RELEVANT FOR THE PART RECO MODELLING IN RK/RKSt for noDTF fits! ----//
                        }
                    }
                    if (_holderEE.HasComponent(to_string(Sample::Psi2JPsX))){
                        FitComponent & _Psi2JPsX = _holderEE[to_string(Sample::Psi2JPsX)];
                        if(_ISANALYTICALSHAPE_(_Psi2JPsX)){
                            _Psi2JPsX.SetConstantAllPars();
                            //---- s-scale / m-shift as main peak ??? --- //
                            //---- MOSTLY RELEVANT FOR THE PART RECO MODELLING IN RK/RKSt for noDTF fits! ----//
                        }
                    }                    
                    if (_holderEE.HasComponent(to_string(Sample::Psi2JPsPiPi))){
                        FitComponent & _Psi2JPsPiPi = _holderEE[to_string(Sample::Psi2JPsPiPi)];
                        if(_ISANALYTICALSHAPE_(_Psi2JPsPiPi)){
                            _Psi2JPsPiPi.SetConstantAllPars();
                            //---- s-scale / m-shift as main peak ??? --- //
                            //---- MOSTLY RELEVANT FOR THE PART RECO MODELLING IN RK/RKSt for noDTF fits! ----//
                        }
                    }
                    if ( _holderEE.HasComponent(to_string(Sample::DataDrivenEMisID))){
                        FitComponent & _DataDrivenShape = _holderEE[to_string(Sample::DataDrivenEMisID)];
                        _DataDrivenShape.SetConstantAllPars();
                    }
                    if (_holderEE.HasComponent(to_string(Sample::Comb))) {
                        FitComponent & _Comb = _holderEE[to_string(Sample::Comb)];
                        MessageSvc::Warning("gConstSS merge shape parameters for turn-on curve");
                        if(_manager.second.Option().Contains("gconstCombSS")){
                            MessageSvc::Warning("gConstSS merge shape parameters for turn-on curve");
                            TString _keyComb = _Comb.Name();
                            _keyComb.ReplaceAll(to_string(_holderEE.Configuration().GetTrigger()), "");
                            _keyComb.ReplaceAll(to_string(_holderEE.Configuration().GetYear()), "");
                            _keyComb           = CleanString(_keyComb);
                            TString _parsGaussConstraint = StripStringBetween(_manager.second.Option(), "gconstComb[", "]");
                            auto parsToGaussConstraint = TokenizeString(_parsGaussConstraint, ",");
                            for( auto & pName : parsToGaussConstraint){
                                TString _shapeParameterNameMod = "";
                                _shapeParameterNameMod = TString::Format("%s_%s", pName.Data(), _keyComb.Data());
                                _Comb.ModifyPDF(pName.Data(), (RooRealVar *) m_parameterPool->GetShapeParameter(_shapeParameterNameMod), "-replace");
                                auto * _pComb = (RooRealVar *) m_parameterPool->GetShapeParameter(_shapeParameterNameMod);
                                _pComb->setConstant(0);
                                m_parameterPool->AddConstrainedParameter(_pComb);
                            }
                        }
                    }
                    if (_holderEE.HasComponent(to_string(Sample::CombSS))) {
                        FitComponent & _CombSS = _holderEE[to_string(Sample::CombSS)];
                        if(_ISANALYTICALSHAPE_(_CombSS)){
                            _CombSS.SetConstantAllPars();
                            if(_manager.second.Option().Contains("gconstCombSS")){
                                TString _parsGaussConstraint = StripStringBetween(_manager.second.Option(), "gconstCombSS[", "]");
                                auto parsToGaussConstraint = TokenizeString(_parsGaussConstraint, ",");
                                for( auto & pName : parsToGaussConstraint){
                                    TString _shapeParameterName = "";
                                    if (pName.Contains("c1") || pName.Contains("c2")) 
                                        _shapeParameterName = TString::Format("%s", pName.Data());
                                    else
                                        _shapeParameterName = TString::Format("%s_CombSS-%s", pName.Data(), _key.Data());
                                    auto * _pComb = (RooRealVar *) m_parameterPool->GetShapeParameter(_shapeParameterName);

                                    if( _pComb->getVal() + 10*_pComb->getError() > _pComb->getMax() ){ 
                                        MessageSvc::Warning("SS data gConstSS adjusting ranges for gauss constraints (max)");
                                        _pComb->setMax(  _pComb->getVal() + 10*_pComb->getError()); 
                                    }
                                    if( _pComb->getVal() - 10*_pComb->getError() < _pComb->getMin() ){
                                        MessageSvc::Warning("SS data gConstSS adjusting ranges for gauss constraints (min)");
                                        _pComb->setMin(  _pComb->getVal() - 10*_pComb->getError());    
                                        if(_shapeParameterName.Contains("sl_") || _shapeParameterName.Contains("m0") ){
                                            if(_pComb->getMin() <0 ){
                                                _pComb->setMin(0.);
                                            }
                                        }                                                                         
                                    }                                    
                                    _pComb->setConstant(0);
                                    m_parameterPool->AddConstrainedParameter(_pComb);
                                }
                            }                           
                        }
                    }
                }
            }
            /* TODO : what is this? needed by other projects? 
            } else {
                for (const auto & _keyHolderPair : _manager.second.HoldersMM()) {
                    FitHolder & _holderMM = _manager.second[_keyHolderPair.first];
                    _holderMM[_holderMM.SignalComponentKey()].SetConstantAllPars();
                    if (_holderMM.HasComponent(to_string(Sample::Bs))) {
                        FitComponent & _BsComponentMM = _holderMM[to_string(Sample::Bs)];
                        _BsComponentMM.SetConstantAllPars();
                        _BsComponentMM.ModifyPDF("m", (RooRealVar *) m_parameterPool->GetShapeParameter("m_offset"), "-offset[" + _dmBsBd + "]");
                    }
                    if (_holderMM.HasComponent(to_string(Sample::Bd))) {
                        FitComponent & _BdComponentMM = _holderMM[to_string(Sample::Bd)];
                        _BdComponentMM.SetConstantAllPars();
                        _BdComponentMM.ModifyPDF("m", (RooRealVar *) m_parameterPool->GetShapeParameter("m_offset"), "-offset[" + _dmBdBs + "]");
                    }
                }

                if (_manager.second.HoldersEEBrem().size() != 0) {
                    for (const auto & _holder : _manager.second.HoldersEEBrem()) {
                        for (const auto & _keyBremHolderPair : _holder.second) {
                            FitHolder & _holderBremEE = _manager.second[_keyBremHolderPair.first];
                            _holderBremEE[_holderBremEE.SignalComponentKey()].SetConstantAllPars();
                            if (_holderBremEE.HasComponent(to_string(Sample::Bs))) {
                                FitComponent & _BsComponentBremEE = _holderBremEE[to_string(Sample::Bs)];
                                _BsComponentBremEE.SetConstantAllPars();
                                _BsComponentBremEE.ModifyPDF("m", (RooRealVar *) m_parameterPool->GetShapeParameter("m_offset"), "-offset[" + _dmBsBd + "]");
                                _BsComponentBremEE.ModifyPDF("mg", (RooRealVar *) m_parameterPool->GetShapeParameter("m_offset"), "-offset[" + _dmBsBd + "]");
                            }
                            if (_holderBremEE.HasComponent(to_string(Sample::Bd))) {
                                FitComponent & _BdComponentBremEE = _holderBremEE[to_string(Sample::Bd)];
                                _BdComponentBremEE.SetConstantAllPars();
                                _BdComponentBremEE.ModifyPDF("m", (RooRealVar *) m_parameterPool->GetShapeParameter("m_offset"), "-offset[" + _dmBdBs + "]");
                                _BdComponentBremEE.ModifyPDF("mg", (RooRealVar *) m_parameterPool->GetShapeParameter("m_offset"), "-offset[" + _dmBdBs + "]");
                            }
                        }
                    }
                } else {
                    for (const auto & _keyHolderPair : _manager.second.HoldersEE()) {
                        FitHolder & _holderEE = _manager.second[_keyHolderPair.first];
                        _holderEE[_holderEE.SignalComponentKey()].SetConstantAllPars();
                        if (_holderEE.HasComponent(to_string(Sample::Bs))) {
                            FitComponent & _BsComponentEE = _holderEE[to_string(Sample::Bs)];
                            _BsComponentEE.SetConstantAllPars();
                            _BsComponentEE.ModifyPDF("m", (RooRealVar *) m_parameterPool->GetShapeParameter("m_offset"), "-offset[" + _dmBsBd + "]");
                        }
                        if (_holderEE.HasComponent(to_string(Sample::Bd))) {
                            FitComponent & _BdComponentEE = _holderEE[to_string(Sample::Bd)];
                            _BdComponentEE.SetConstantAllPars();
                            _BdComponentEE.ModifyPDF("m", (RooRealVar *) m_parameterPool->GetShapeParameter("m_offset"), "-offset[" + _dmBdBs + "]");
                        }
                    }
                }
            */
        } else if (_manager.second.Option().Contains("fixshape")) {
            for (const auto & _keyHolderPair : _manager.second.HoldersMM()) {
                FitHolder &    _holderMM          = _manager.second[_keyHolderPair.first];
                FitComponent & _signalComponentMM = _holderMM[_holderMM.SignalComponentKey()];
                _signalComponentMM.SetConstantAllPars();
            }
            for (const auto & _holder : _manager.second.HoldersEEBrem()) {
                for (const auto & _keyBremHolderPair : _holder.second) {
                    FitHolder &    _bremHolder            = _manager.second[_keyBremHolderPair.first];
                    FitComponent & _signalComponentBremEE = _bremHolder[_bremHolder.SignalComponentKey()];
                    _signalComponentBremEE.SetConstantAllPars();
                    // _signalComponentBremEE.SetConstantExceptPars( {"fg"} );
                }
            }
            for (const auto & _keyHolderPair : _manager.second.HoldersEE()) {
                FitHolder &    _holderEE          = _manager.second[_keyHolderPair.first];
                FitComponent & _signalComponentEE = _holderEE[_holderEE.SignalComponentKey()];
                _signalComponentEE.SetConstantAllPars();
            }
        }

        for (auto & _keyHolderPair : _manager.second.Holders()) {
            auto &  _holder = _manager.second[_keyHolderPair.first];
            TString _key    = _holder.Name();
            if (_manager.second.Option().Contains("modshapejps")) _key.ReplaceAll(to_string(_holder.Configuration().GetQ2bin()), to_string(Q2Bin::JPsi));
            _key.ReplaceAll("MC", "").ReplaceAll("CL", "");
            _key           = CleanString(_key);
            auto _shiftKey = _key;
            auto _sigmaKey = _key;
            if (_manager.second.Option().Contains("modshiftrx")) _shiftKey.ReplaceAll(to_string(_holder.Configuration().GetProject()) + SettingDef::separator, "");
            for (auto & _keyBkbComponentPair : _holder.BackgroundComponents()) {
                auto & _bkgComponent = _holder[_keyBkbComponentPair.first];
                // Set the convolution mass to a constant
                if (_bkgComponent.Option().Contains("convsigma")) {
                    TString      _varName        = "m_conv_" + _bkgComponent.Name();
                    RooRealVar * _shiftParameter = new RooRealVar(_varName, _varName, 0);
                    m_parameterPool->AddShapeParameter(_shiftParameter);
                    auto * _sigmaParameter = (RooRealVar *) m_parameterPool->GetShapeParameter("s_conv_" + _sigmaKey);
                    _bkgComponent.ConvPDF(_shiftParameter, _sigmaParameter);
                }
                // Set the convolution sigma to a constant
                else if (_bkgComponent.Option().Contains("convmass")) {
                    TString      _varName        = "s_conv_" + _bkgComponent.Name();
                    RooRealVar * _sigmaParameter = new RooRealVar(_varName, _varName, 1);
                    m_parameterPool->AddShapeParameter(_sigmaParameter);
                    auto * _shiftParameter = (RooRealVar *) m_parameterPool->GetShapeParameter("m_conv_" + _shiftKey);
                    // auto * _shiftParameter = (RooRealVar *) m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey);
                    _bkgComponent.ConvPDF(_shiftParameter, _sigmaParameter);
                    // Set the convolution mass to the signal shift
                } else if (_bkgComponent.Option().Contains("conv")) {
                    auto * _shiftParameter = (RooRealVar *) m_parameterPool->GetShapeParameter("m_shift_" + _shiftKey);
                    auto * _sigmaParameter = (RooRealVar *) m_parameterPool->GetShapeParameter("s_conv_" + _sigmaKey);
                    _bkgComponent.ConvPDF(_shiftParameter, _sigmaParameter);
                }
                
                //
                //TODO : not sure if really needed, but this enforce for a given bkg to have all pars constant in the fit, useful in case we do some special fits with all backgrounds
                //
                if( _bkgComponent.Option().Contains("fixshapebkg") ){
                    if (_keyBkbComponentPair.first.Contains(to_string(Sample::Comb)) && !_keyBkbComponentPair.first.Contains(to_string(Sample::CombSS)) ) continue;
                    //for non - combinatorial background type, we fix shape of MC fit.          
                    _bkgComponent.SetConstantAllPars();                
                }
            }
        }
        _manager.second.Finalize();
        _manager.second.InitRanges();
    }

    for (auto & _manager : m_managers) { _manager.second.PrintKeys(); }

    auto _stop = chrono::high_resolution_clock::now();
    MessageSvc::Warning("FitGenerator", m_name, "ModifyShapes took", to_string(chrono::duration_cast< chrono::seconds >(_stop - _start).count()), "seconds");
    return;
}

void FitGenerator::ModifyYields() {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitGenerator", m_name, "ModifyYields");
    MessageSvc::Line();

    auto _start = chrono::high_resolution_clock::now();

    auto ManagerExists = [this](const Prj & _project, const Q2Bin & _q2bin) {
        TString _name          = to_string(_project) + SettingDef::separator + "q2" + to_string(_q2bin);
        bool    _managerExists = (m_managers.find(_name) != m_managers.end());
        return _managerExists;
    };

    auto CheckQ2FitManager = [&](const FitManager & _manager, const Q2Bin & _q2bin) {
        FitConfiguration _conf = _manager.Configurations()[0];
        if (_conf.GetQ2bin() == _q2bin)
            return true;
        else
            return false;
    };
    
    /*
        All manager loops are and MUST be splitted because you need to link RKst to RK yields and do this also on cross-feed backgrounds 
    */


    /*
      Step 1 : For Managers JPsi type of a given Project, if the FitManager had ```modyieldsig``` tag, 
      we redefine Y(ee) as a function of a sinle ratio parameter [only rJPsi because of the veto on CheckQ2FitManager]
    */
    for (auto & _manager : m_managers) {
        if (_manager.second.Option().Contains("modyieldsig") && CheckQ2FitManager(_manager.second, Q2Bin::JPsi)) {
            for (const auto & _keyHolderPair : _manager.second.HoldersEE()) {
                FitHolder & _holderEE     = _manager.second[_keyHolderPair.first];
                auto        _signalConfig = FitParameterConfig::GetSignalConfig(_holderEE.Configuration());
                m_parameterPool->AddSingleRatioYield(_signalConfig, _manager.second.Option());
                _holderEE.SetSignalYield(m_parameterPool->GetYield(_signalConfig));
                _manager.second[_keyHolderPair.first] = _holderEE;
            }
        }
    }

    // Throw an error if the user configure a non-JPsi fit to use single ratios
    for (auto & _manager : m_managers) {
        if (not(CheckQ2FitManager(_manager.second, Q2Bin::JPsi))) {
            if (_manager.second.Option().Contains("modyieldsigsingle")) { MessageSvc::Error("FitGenerator", (TString) "ModifyYields", (TString) "-modyieldsigsingle option is only allowed for JPsi q2bin", "EXIT_FAILURE"); }
        }
    }


    /*
        Step 2 : Deal with double ratio and signal yields definitions such that the next Background dealing
        will absorb double ratios in the Y(BKG) for x-feeds
    */
    for (auto & _manager : m_managers) {
        TString _managerOption = _manager.second.Option();
        if (_managerOption.Contains("modyield")) {
            if (_managerOption.Contains("modyieldsig") && not(CheckQ2FitManager(_manager.second, Q2Bin::JPsi))) {   // Do no repeat ratio creation for JPsi
                MessageSvc::Line();
                MessageSvc::Info("ModifyYields", _manager.second.Name(), _managerOption, "Signal");
                MessageSvc::Line();

                for (const auto & _keyHolderPair : _manager.second.HoldersEE()) {
                    FitHolder & _holderEE     = _manager.second[_keyHolderPair.first];
                    auto        _signalConfig = FitParameterConfig::GetSignalConfig(_holderEE.Configuration());
                    if (_managerOption.Contains("modyieldsigdouble")) m_parameterPool->AddDoubleRatioYield(_signalConfig, _managerOption);
                    MessageSvc::Info("Setting SignalYield");
                    _holderEE.SetSignalYield(m_parameterPool->GetYield(_signalConfig));
                }
            }
        }
    }

    /*
      Step 3 : Deal with background constraints and redefinition of yields on them, 
      Here the modyieldbkgbs        act on Sample::Bs      to properly introduce in the fitter the Bs->Kst Phi component
      Here the modyieldbkgbs2phi    act on Sample::Bs2Phi  to properly introduce in the fitter the Bs->J/Psi(Psi) Phi component and constrained yields 
      Here the modyieldbkghadswap   act on Sample::HadSwap to properly introduce in the fitter the B0->J/Psi(Psi) Kst0 component with double hadronic misID and constrained yields 
      Here the modyieldbkglb        act on Sample::Lb      to properly introduce in the fitter the Lb2pK(J/Psi|Psi) component with constrained eff ratios and integrated fLb/fd
      Here the modyieldbkgmid       act on the misID for RK  to properly introduce in the fitter the B+ -> Pi J/Psi  component with constrained eff ratios
      Here the modyieldbkgpsi2jpsx  act on the Psi2JPsX for RK/RKst to properly introduce in the fitter the B(+,0) -> Psi(->Pi Pi) J/Psi Kst/K  component with constrained eff ratios
      Here the modyieldbkgbd        act on RPhi only (TODO : check it is fine )
      TODO : review leakage
      Special threatment for x-feed components in RK background : 
        Bd2Kst  
        Bu2Kst  
        BdBu         
        

    */
    for (auto & _manager : m_managers) {
        TString _managerOption = _manager.second.Option();
        if (_managerOption.Contains("modyield")) {        
            if (_managerOption.Contains("modyieldbkg")) {
                MessageSvc::Line();
                MessageSvc::Info("ModifyYields", _manager.second.Name(), _managerOption, "Background");
                MessageSvc::Line();
                for (const auto & _keyHolderPair : _manager.second.HoldersMM()) {
                    FitHolder & _holderMM     = _manager.second[_keyHolderPair.first];
                    auto        _signalConfig = FitParameterConfig::GetSignalConfig(_holderMM.Configuration());
                    // if (_holderMM.HasComponent(to_string(Sample::Bd))) {
                    //    if (_managerOption.Contains("modyieldbkgbd")) {
                    //    } else if (_managerOption.Contains("modyieldbkgks")) {
                    //    }
                    //}
                    if (_holderMM.HasComponent(to_string(Sample::Bs))) {
                        if(_managerOption.Contains("modyieldbkgbs")){
                            auto _bkgConfig = FitParameterConfig::GetBackgroundConfig(_holderMM.Configuration(), Sample::Bs);
                            // Get the values for the ratios
                            auto _configHadronisation = GetConfigForRatio(RatioType::FsOverFd, _bkgConfig);
                            auto _configBranching     = GetConfigForRatio(RatioType::BranchingFraction, _bkgConfig);
                            auto _branchingRatio      = m_parameterPool->GetRatioParameter(_configBranching, RatioType::BranchingFraction);
                            auto _hadronisationRatio  = m_parameterPool->GetRatioParameter(_configHadronisation, RatioType::FsOverFd);

                            vector< RooAbsReal * > _numerators     = {_branchingRatio, _hadronisationRatio};
                            vector< RooAbsReal * > _denominators   = {};
                            auto                   _signalYieldVar = (RooRealVar *) m_parameterPool->GetYield(_signalConfig);
                            if( _holderMM.Configuration().GetProject() == Prj::RK ){
                                MessageSvc::Warning("modyieldbkgbs for RK, loading efficiency of Bd2KstJPs with range mass shifted cut");
                                auto _varEfficiencyBs2KstJps = m_parameterPool->GetEfficiency(_bkgConfig);
                                auto _varEfficiencySignal = m_parameterPool->GetEfficiency(_signalConfig);
                                _numerators.push_back( _varEfficiencyBs2KstJps);
                                _denominators.push_back( _varEfficiencySignal);
                            }
                            //ADD IT ! 
                            m_parameterPool->AddYieldFormula(_bkgConfig, _signalYieldVar, _numerators, _denominators);                        
                            _holderMM.SetBackgroundYield(to_string(Sample::Bs), (RooRealVar *) m_parameterPool->GetYield(_bkgConfig));
                        }
                    }
                    if (_holderMM.HasComponent(to_string(Sample::Bs2Phi))) {
                        if (_managerOption.Contains("modyieldbkgbs2phi")) {
                            auto _bkgConfig = FitParameterConfig::GetBackgroundConfig(_holderMM.Configuration(), Sample::Bs2Phi);
                            // Get the values for the ratios
                            auto _configHadronisation = GetConfigForRatio(RatioType::FsOverFd, _bkgConfig);
                            auto _configBranching     = GetConfigForRatio(RatioType::BranchingFraction, _bkgConfig);
                            auto _branchingRatio      = m_parameterPool->GetRatioParameter(_configBranching, RatioType::BranchingFraction);
                            auto _hadronisationRatio  = m_parameterPool->GetRatioParameter(_configHadronisation, RatioType::FsOverFd);
                            auto _varEfficiencyBs2Phi = m_parameterPool->GetEfficiency(_bkgConfig);
                            auto _varEfficiencySignal = m_parameterPool->GetEfficiency(_signalConfig);

                            vector< RooAbsReal * > _numerators     = {_branchingRatio, _hadronisationRatio, _varEfficiencyBs2Phi};
                            vector< RooAbsReal * > _denominators   = {_varEfficiencySignal};
                            auto                   _signalYieldVar = (RooRealVar *) m_parameterPool->GetYield(_signalConfig);

                            m_parameterPool->AddYieldFormula(_bkgConfig, _signalYieldVar, _numerators, _denominators);
                            _holderMM.SetBackgroundYield(to_string(Sample::Bs2Phi), (RooRealVar *) m_parameterPool->GetYield(_bkgConfig));
                        }
                    }
                    if (_holderMM.HasComponent(to_string(Sample::HadSwap))) {
                        if (_managerOption.Contains("modyieldbkghadswap")) {
                            auto _bkgConfig = FitParameterConfig::GetBackgroundConfig(_holderMM.Configuration(), Sample::HadSwap);
                            // Get the values for the ratios
                            auto _varEfficiencyHadSwap = m_parameterPool->GetEfficiency(_bkgConfig);
                            auto _varEfficiencySignal  = m_parameterPool->GetEfficiency(_signalConfig);

                            vector< RooAbsReal * > _numerators     = {_varEfficiencyHadSwap};
                            vector< RooAbsReal * > _denominators   = {_varEfficiencySignal};
                            auto                   _signalYieldVar = (RooRealVar *) m_parameterPool->GetYield(_signalConfig);

                            m_parameterPool->AddYieldFormula(_bkgConfig, _signalYieldVar, _numerators, _denominators);
                            _holderMM.SetBackgroundYield(to_string(Sample::HadSwap), (RooRealVar *) m_parameterPool->GetYield(_bkgConfig));
                        }
                    }
                    if (_holderMM.HasComponent(to_string(Sample::Lb))) {
                        if (_managerOption.Contains("modyieldbkglb")) {
                            auto _bkgConfig = FitParameterConfig::GetBackgroundConfig(_holderMM.Configuration(), Sample::Lb);
                            // Get the values for the ratios
                            auto _configHadronisation = GetConfigForRatio(RatioType::FLbOverFd, _bkgConfig);
                            auto _configBranching     = GetConfigForRatio(RatioType::BranchingFraction, _bkgConfig);
                            auto _branchingRatio      = m_parameterPool->GetRatioParameter(_configBranching, RatioType::BranchingFraction);
                            auto _hadronisationRatio  = m_parameterPool->GetRatioParameter(_configHadronisation, RatioType::FLbOverFd);
                            auto _varEfficiencyLb     = m_parameterPool->GetEfficiency(_bkgConfig);
                            auto _varEfficiencySignal = m_parameterPool->GetEfficiency(_signalConfig);

                            vector< RooAbsReal * > _numerators     = {_branchingRatio, _hadronisationRatio, _varEfficiencyLb};
                            vector< RooAbsReal * > _denominators   = {_varEfficiencySignal};
                            auto                   _signalYieldVar = (RooRealVar *) m_parameterPool->GetYield(_signalConfig);

                            m_parameterPool->AddYieldFormula(_bkgConfig, _signalYieldVar, _numerators, _denominators);
                            _holderMM.SetBackgroundYield(to_string(Sample::Lb), (RooRealVar *) m_parameterPool->GetYield(_bkgConfig));
                        }
                    }
                    if (_holderMM.HasComponent(to_string(Sample::DSLC))) {
                        if (_managerOption.Contains("modyieldbkgdslc")) {
                            auto _bkgConfig = FitParameterConfig::GetBackgroundConfig(_holderMM.Configuration(), Sample::DSLC);
                            // Get the values for the ratios
                            auto _configBranching     = GetConfigForRatio(RatioType::BranchingFraction, _bkgConfig);
                            auto _branchingRatio      = m_parameterPool->GetRatioParameter(_configBranching, RatioType::BranchingFraction);
                            auto _varEfficiencyDSLC   = m_parameterPool->GetEfficiency(_bkgConfig);
                            auto _varEfficiencySignal = m_parameterPool->GetEfficiency(_signalConfig);

                            vector< RooAbsReal * > _numerators     = {_branchingRatio, _varEfficiencyDSLC};
                            vector< RooAbsReal * > _denominators   = {_varEfficiencySignal};
                            auto                   _signalYieldVar = (RooRealVar *) m_parameterPool->GetYield(_signalConfig);

                            m_parameterPool->AddYieldFormula(_bkgConfig, _signalYieldVar, _numerators, _denominators);
                            _holderMM.SetBackgroundYield(to_string(Sample::DSLC), (RooRealVar *) m_parameterPool->GetYield(_bkgConfig));
                        }
                    }
                    if (_holderMM.HasComponent(to_string(Sample::MisID))) {
                        if (_managerOption.Contains("modyieldbkgmid")) {
                            auto _bkgConfig = FitParameterConfig::GetBackgroundConfig(_holderMM.Configuration(), Sample::MisID);
                            // Get the values for the ratios
                            auto _configBranching     = GetConfigForRatio(RatioType::BranchingFraction, _bkgConfig);
                            auto _branchingRatio      = m_parameterPool->GetRatioParameter(_configBranching, RatioType::BranchingFraction);
                            auto _varEfficiencyMisID  = m_parameterPool->GetEfficiency(_bkgConfig);
                            auto _varEfficiencySignal = m_parameterPool->GetEfficiency(_signalConfig);

                            vector< RooAbsReal * > _numerators     = {_branchingRatio, _varEfficiencyMisID};
                            vector< RooAbsReal * > _denominators   = {_varEfficiencySignal};
                            auto                   _signalYieldVar = (RooRealVar *) m_parameterPool->GetYield(_signalConfig);

                            m_parameterPool->AddYieldFormula(_bkgConfig, _signalYieldVar, _numerators, _denominators);
                            _holderMM.SetBackgroundYield(to_string(Sample::MisID), (RooRealVar *) m_parameterPool->GetYield(_bkgConfig));
                        }
                    }
                    MessageSvc::Line();
                }//End of Muon HOLDERS !                 
                // go thorugh each EE mode Holders of fits....
                for (const auto & _keyHolderPair : _manager.second.HoldersEE()) {
                    FitHolder & _holderEE = _manager.second[_keyHolderPair.first];
                    // Retrieve the Signal Configuration from this Holder
                    auto _signalConfig = FitParameterConfig::GetSignalConfig(_holderEE.Configuration());                
                    if (_holderEE.HasComponent(to_string(Sample::Bd)) && (_holderEE.Configuration().GetProject() == Prj::RPhi)) {
                        auto _bkgConfig = FitParameterConfig::GetBackgroundConfig(_holderEE.Configuration(), Sample::Bd);
                        if (_managerOption.Contains("modyieldbkgbd")) {
                        } else if (_managerOption.Contains("modyieldbkgks")) {
                        } else {
                            // Add to the parameter pool a Yield Signal Ratio paramete for it
                            m_parameterPool->AddBackgroundYieldSignalRatio(_bkgConfig, _signalConfig);
                        }
                        // Force the Background Yield for the Bd shape to be linked to the ParameterPool yield of the previously configured Background Configuration
                        _holderEE.SetBackgroundYield(to_string(Sample::Bd), (RooRealVar *) m_parameterPool->GetYield(_bkgConfig));
                    }
                    if (_holderEE.HasComponent(to_string(Sample::Bs))) {
                        auto _bkgConfig = FitParameterConfig::GetBackgroundConfig(_holderEE.Configuration(), Sample::Bs);                        
                        if (_managerOption.Contains("modyieldbkgbs")) {
                            // Get the values for the ratios
                            auto _configHadronisation = GetConfigForRatio(RatioType::FsOverFd, _bkgConfig);
                            auto _configBranching     = GetConfigForRatio(RatioType::BranchingFraction, _bkgConfig);
                            auto _branchingRatio      = m_parameterPool->GetRatioParameter(_configBranching, RatioType::BranchingFraction);
                            auto _hadronisationRatio  = m_parameterPool->GetRatioParameter(_configHadronisation, RatioType::FsOverFd);
                            vector< RooAbsReal * > _numerators     = {_branchingRatio, _hadronisationRatio};
                            vector< RooAbsReal * > _denominators   = {};
                            auto                   _signalYieldVar = (RooRealVar *) m_parameterPool->GetYield(_signalConfig);
                            if( _holderEE.Configuration().GetProject() == Prj::RK ){
                                MessageSvc::Warning("modyieldbkgbs for RK, loading efficiency of Bd2KstJPs with range mass shifted cut");
                                auto _varEfficiencyBs2KstJps = m_parameterPool->GetEfficiency(_bkgConfig);
                                auto _varEfficiencySignal = m_parameterPool->GetEfficiency(_signalConfig);
                                _numerators.push_back( _varEfficiencyBs2KstJps);
                                _denominators.push_back( _varEfficiencySignal);
                				m_parameterPool->AddYieldFormula(_bkgConfig, _signalYieldVar, _numerators, _denominators);
                            }else{
                                //For RKst Bs shape do always a link to MM(Bs)/MM(signal) * EE(signal)!
                                //The Bs fraction gets always absorbed from Muons assuming eps(EE-Signal) == eps(EE-Bs)
                                m_parameterPool->AddBackgroundYieldSignalRatio(_bkgConfig, _signalConfig);
                                // m_parameterPool->AddYieldFormula(_bkgConfig, _signalYieldVar, _numerators, _denominators);
                            }
                        }else{
                            //Link to ratio of Muons here 
                            m_parameterPool->AddBackgroundYieldSignalRatio(_bkgConfig, _signalConfig);
                        }
                        // Force the Background Yield for the Bs shape to be linked to the ParameterPool yield of the previously configured Background Configuration
                        _holderEE.SetBackgroundYield(to_string(Sample::Bs), (RooRealVar *) m_parameterPool->GetYield(_bkgConfig));
                    }
                    if (_holderEE.HasComponent(to_string(Sample::Bs2Phi))) {
                        auto _bkgConfig = FitParameterConfig::GetBackgroundConfig(_holderEE.Configuration(), Sample::Bs2Phi);
                        if (_managerOption.Contains("modyieldbkgbs2phi")) {
                            // Get the values for the ratios
                            auto _configHadronisation = GetConfigForRatio(RatioType::FsOverFd, _bkgConfig);
                            auto _configBranching     = GetConfigForRatio(RatioType::BranchingFraction, _bkgConfig);
                            auto _branchingRatio      = m_parameterPool->GetRatioParameter(_configBranching, RatioType::BranchingFraction);
                            auto _hadronisationRatio  = m_parameterPool->GetRatioParameter(_configHadronisation, RatioType::FsOverFd);
                            auto _varEfficiencyBs2Phi = m_parameterPool->GetEfficiency(_bkgConfig);
                            auto _varEfficiencySignal = m_parameterPool->GetEfficiency(_signalConfig);

                            vector< RooAbsReal * > _numerators     = {_branchingRatio, _hadronisationRatio, _varEfficiencyBs2Phi};
                            vector< RooAbsReal * > _denominators   = {_varEfficiencySignal};
                            auto                   _signalYieldVar = (RooRealVar *) m_parameterPool->GetYield(_signalConfig);

                            m_parameterPool->AddYieldFormula(_bkgConfig, _signalYieldVar, _numerators, _denominators);
                        } else if (_managerOption.Contains("modyieldbkgeff")) {
                            m_parameterPool->AddBackgroundYieldEfficiencyRatio(_bkgConfig);
                        } else {
                            // Add to the parameter pool a Yield Signal Ratio paramete for it
                            m_parameterPool->AddBackgroundYieldSignalRatio(_bkgConfig, _signalConfig);
                        }
                        // Force the Background Yield for the Bs shape to be linked to the ParameterPool yield of the previously configured Background Configuration
                        _holderEE.SetBackgroundYield(to_string(Sample::Bs2Phi), (RooRealVar *) m_parameterPool->GetYield(_bkgConfig));
                    }
                    if (_holderEE.HasComponent(to_string(Sample::HadSwap))) {
                        auto _bkgConfig = FitParameterConfig::GetBackgroundConfig(_holderEE.Configuration(), Sample::HadSwap);
                        if (_managerOption.Contains("modyieldbkghadswap")) {
                            // Get the values for the ratios
                            auto _varEfficiencyHadSwap = m_parameterPool->GetEfficiency(_bkgConfig);
                            auto _varEfficiencySignal = m_parameterPool->GetEfficiency(_signalConfig);
                            vector< RooAbsReal * > _numerators     = {_varEfficiencyHadSwap};
                            vector< RooAbsReal * > _denominators   = {_varEfficiencySignal};
                            auto                   _signalYieldVar = (RooRealVar *) m_parameterPool->GetYield(_signalConfig);

                            m_parameterPool->AddYieldFormula(_bkgConfig, _signalYieldVar, _numerators, _denominators);
                        } else if (_managerOption.Contains("modyieldbkgeff")) {
                            m_parameterPool->AddBackgroundYieldEfficiencyRatio(_bkgConfig);
                        } else {
                            // Add to the parameter pool a Yield Signal Ratio paramete for it
                            m_parameterPool->AddBackgroundYieldSignalRatio(_bkgConfig, _signalConfig);
                        }
                        // Force the Background Yield for the Bs shape to be linked to the ParameterPool yield of the previously configured Background Configuration
                        _holderEE.SetBackgroundYield(to_string(Sample::HadSwap), (RooRealVar *) m_parameterPool->GetYield(_bkgConfig));
                    }
                    if (_holderEE.HasComponent(to_string(Sample::Psi2JPsX))) {
                        if (_managerOption.Contains("modyieldbkgpsi2jpsx")) {                            
                            auto _bkgConfig = FitParameterConfig::GetBackgroundConfig(_holderEE.Configuration(), Sample::Psi2JPsX);
                            // Get the values for the ratios
                            auto _configBranching       = GetConfigForRatio(RatioType::BranchingFraction, _bkgConfig);
                            auto _branchingRatio        = m_parameterPool->GetRatioParameter(_configBranching, RatioType::BranchingFraction);
                            auto _varEfficiencyPsi2JPsX = m_parameterPool->GetEfficiency(_bkgConfig);
                            auto _varEfficiencySignal   = m_parameterPool->GetEfficiency(_signalConfig);

                            vector< RooAbsReal * > _numerators     = {_branchingRatio, _varEfficiencyPsi2JPsX};
                            vector< RooAbsReal * > _denominators   = {_varEfficiencySignal};
                            auto                   _signalYieldVar = (RooRealVar *) m_parameterPool->GetYield(_signalConfig);

                            m_parameterPool->AddYieldFormula(_bkgConfig, _signalYieldVar, _numerators, _denominators);
                            _holderEE.SetBackgroundYield(to_string(Sample::Psi2JPsX), (RooRealVar *) m_parameterPool->GetYield(_bkgConfig));
                        }else if( _managerOption.Contains("modyieldbkgPRpsi2jpsx")){
                            MessageSvc::Debug("ADDING PART RECO CONSTRAINED YIELD OF PSI2XJPs decay mode");
                            auto _bkgConfig = FitParameterConfig::GetBackgroundConfig(_holderEE.Configuration(), Sample::Psi2JPsX);
                            auto _configBranching       = GetConfigForRatio(RatioType::BranchingFraction, _bkgConfig);
                            auto _branchingRatio        = m_parameterPool->GetRatioParameter(_configBranching, RatioType::BranchingFraction);
                            auto _varEfficiencyPsi2JPsX = m_parameterPool->GetEfficiency(_bkgConfig);
                            auto _varEfficiencySignal   = m_parameterPool->GetEfficiency(_signalConfig);
                            vector< RooAbsReal * > _numerators     = {_branchingRatio, _varEfficiencyPsi2JPsX};
                            vector< RooAbsReal * > _denominators   = {_varEfficiencySignal};
                            auto                   _signalYieldVar = (RooRealVar *) m_parameterPool->GetYield(_signalConfig);
                            m_parameterPool->AddYieldFormula(_bkgConfig, _signalYieldVar, _numerators, _denominators);
                            _holderEE.SetBackgroundYield(to_string(Sample::Psi2JPsX), (RooRealVar *) m_parameterPool->GetYield(_bkgConfig));                            
                        }
                    }
                    if( _holderEE.HasComponent(to_string(Sample::Psi2JPsPiPi))){
                        //TODO : add protection to have this for RKst only in J/Psi q2 (EE)                     
                        if (_managerOption.Contains("modyieldbkgPRpsi2jpspipi")) {
                            MessageSvc::Debug("ADDING PART RECO CONSTRAINED YIELD OF B+ => Psi( => J/Psi pipi) K+");
                            auto _bkgConfig = FitParameterConfig::GetBackgroundConfig(_holderEE.Configuration(), Sample::Psi2JPsPiPi);
                            auto _configBranching       = GetConfigForRatio(RatioType::BranchingFraction, _bkgConfig);
                            auto _branchingRatio        = m_parameterPool->GetRatioParameter(_configBranching, RatioType::BranchingFraction);
                            auto _varEfficiencyPsi2JPsPiPi = m_parameterPool->GetEfficiency(_bkgConfig);
                            auto _varEfficiencySignal   = m_parameterPool->GetEfficiency(_signalConfig);
                            vector< RooAbsReal * > _numerators     = {_branchingRatio, _varEfficiencyPsi2JPsPiPi};
                            vector< RooAbsReal * > _denominators   = {_varEfficiencySignal};
                            auto                   _signalYieldVar = (RooRealVar *) m_parameterPool->GetYield(_signalConfig);
                            m_parameterPool->AddYieldFormula(_bkgConfig, _signalYieldVar, _numerators, _denominators);
                            _holderEE.SetBackgroundYield(to_string(Sample::Psi2JPsPiPi), (RooRealVar *) m_parameterPool->GetYield(_bkgConfig));
                        }
                    }
                    if (_holderEE.HasComponent(to_string(Sample::Lb))) {
                        auto _bkgConfig = FitParameterConfig::GetBackgroundConfig(_holderEE.Configuration(), Sample::Lb);
                        if (_managerOption.Contains("modyieldbkglb")) {
                            /*
                                Define Y(Lb-EE) = Y(Signal-EE) * ( eps(Lb-EE)/eps(Signal-EE) ) * fLb/fd * B.R.( Lb decay )/B.R(Signal-XX decay)
                            */
                            auto _configHadronisation = GetConfigForRatio(RatioType::FLbOverFd, _bkgConfig);
                            auto _configBranching     = GetConfigForRatio(RatioType::BranchingFraction, _bkgConfig);
                            auto _branchingRatio      = m_parameterPool->GetRatioParameter(_configBranching, RatioType::BranchingFraction);
                            auto _hadronisationRatio  = m_parameterPool->GetRatioParameter(_configHadronisation, RatioType::FLbOverFd);
                            auto _varEfficiencyLb     = m_parameterPool->GetEfficiency(_bkgConfig);
                            auto _varEfficiencySignal = m_parameterPool->GetEfficiency(_signalConfig);

                            vector< RooAbsReal * > _numerators     = {_branchingRatio, _hadronisationRatio, _varEfficiencyLb};
                            vector< RooAbsReal * > _denominators   = {_varEfficiencySignal};
                            auto                   _signalYieldVar = (RooRealVar *) m_parameterPool->GetYield(_signalConfig);
                            m_parameterPool->AddYieldFormula(_bkgConfig, _signalYieldVar, _numerators, _denominators);
                        } else if (_managerOption.Contains("modyieldbkgeff")) {
                            m_parameterPool->AddBackgroundYieldEfficiencyRatio(_bkgConfig);
                        } else {   
                            if( _bkgConfig.GetProject() != Prj::RK){
                                m_parameterPool->AddBackgroundYieldSignalRatio(_bkgConfig, _signalConfig);
                            }
                        }
                        _holderEE.SetBackgroundYield(to_string(Sample::Lb), (RooRealVar *) m_parameterPool->GetYield(_bkgConfig));
                    }
                    if (_holderEE.HasComponent(to_string(Sample::DSLC))) {
                        if (_managerOption.Contains("modyieldbkgdslc")) {
                            //Same as muon via efficiencies
                            auto _bkgConfig = FitParameterConfig::GetBackgroundConfig(_holderEE.Configuration(), Sample::DSLC);
                            // Get the values for the ratios
                            auto _configBranching     = GetConfigForRatio(RatioType::BranchingFraction, _bkgConfig);
                            auto _branchingRatio      = m_parameterPool->GetRatioParameter(_configBranching, RatioType::BranchingFraction);
                            auto _varEfficiencyDSLC   = m_parameterPool->GetEfficiency(_bkgConfig);
                            auto _varEfficiencySignal = m_parameterPool->GetEfficiency(_signalConfig);

                            vector< RooAbsReal * > _numerators     = {_branchingRatio, _varEfficiencyDSLC};
                            vector< RooAbsReal * > _denominators   = {_varEfficiencySignal};
                            auto                   _signalYieldVar = (RooRealVar *) m_parameterPool->GetYield(_signalConfig);
                            m_parameterPool->AddYieldFormula(_bkgConfig, _signalYieldVar, _numerators, _denominators);
                            _holderEE.SetBackgroundYield(to_string(Sample::DSLC), (RooRealVar *) m_parameterPool->GetYield(_bkgConfig));
                        }        
                        //else it's floating               
                    }
                    if (_holderEE.HasComponent(to_string(Sample::MisID))) {
                        auto _bkgConfig = FitParameterConfig::GetBackgroundConfig(_holderEE.Configuration(), Sample::MisID);
                        if (_managerOption.Contains("modyieldbkgmid")) {
                            // Get the values for the ratios
                            auto _project  =_holderEE.Configuration().GetProject();
                            auto _year     =_holderEE.Configuration().GetYear();
                            auto _q2Bin    =_holderEE.Configuration().GetQ2bin();
                            if( _project == Prj::RK && _q2Bin == Q2Bin::JPsi){ 
			      //Bu2PiJPsEE misID in RK fits J/Psi q2 
			      MessageSvc::Warning("Bu2PiJPsEE constraint inferred from Bu2PiJPsMM/Bu2KJPsMM ratio");
			      m_parameterPool->AddBackgroundYieldSignalRatio(_bkgConfig, _signalConfig);
                            }else{
                                auto _configBranching     = GetConfigForRatio(RatioType::BranchingFraction, _bkgConfig);
                                auto _branchingRatio      = m_parameterPool->GetRatioParameter(_configBranching, RatioType::BranchingFraction);
                                auto _varEfficiencyMisID  = m_parameterPool->GetEfficiency(_bkgConfig);
                                auto _varEfficiencySignal = m_parameterPool->GetEfficiency(_signalConfig);
                                vector< RooAbsReal * > _numerators     = {_branchingRatio, _varEfficiencyMisID};
                                vector< RooAbsReal * > _denominators   = {_varEfficiencySignal};
                                auto                   _signalYieldVar = (RooRealVar *) m_parameterPool->GetYield(_signalConfig);
                                m_parameterPool->AddYieldFormula(_bkgConfig, _signalYieldVar, _numerators, _denominators);
                            }
                        } else if (_managerOption.Contains("modyieldbkgeff")) {                            
                            m_parameterPool->AddBackgroundYieldEfficiencyRatio(_bkgConfig);
                        } else {
                            m_parameterPool->AddBackgroundYieldSignalRatio(_bkgConfig, _signalConfig);
                        }
                        _holderEE.SetBackgroundYield(to_string(Sample::MisID), (RooRealVar *) m_parameterPool->GetYield(_bkgConfig));
                    }
                    
                    if (_holderEE.HasComponent(to_string(Sample::Leakage))){
                        auto _bkgConfig = FitParameterConfig::GetBackgroundConfig(_holderEE.Configuration(), Sample::Leakage);
                        //TODO : review usage of leakage efficiency constraints [constrained in Psi q2, constrained in CentralQ2 ]
                        if (_managerOption.Contains("modyieldbkgeff")) {
                            FitParameterConfig _leakConfig(_signalConfig); 
                            if (_signalConfig.GetQ2bin() == Q2Bin::Central) _leakConfig = _bkgConfig.ReplaceConfig(Q2Bin::JPsi).ReplaceConfig(GetSignalSample(Q2Bin::JPsi));
                            if (_signalConfig.GetQ2bin() == Q2Bin::Psi)     _leakConfig = _bkgConfig.ReplaceConfig(Q2Bin::JPsi).ReplaceConfig(GetSignalSample(Q2Bin::JPsi));
                            if (_signalConfig.GetQ2bin() == Q2Bin::High)    _leakConfig = _bkgConfig.ReplaceConfig(Q2Bin::Psi).ReplaceConfig(GetSignalSample(Q2Bin::Psi));
                            if (_leakConfig == _signalConfig) MessageSvc::Error("ModifyYields", _manager.second.Name(), _managerOption, "Leakage signal not available", "EXIT_FAILURE");
                            m_parameterPool->AddBackgroundYieldSignalEfficiencyRatio(_bkgConfig, _leakConfig);
                        }else if( _managerOption.Contains("modyieldbkgprleak")){
                            //Leakage configuration for Part-Reco J/Psi mode is simply : 
                            //Y(leak) = Y(sig) * eps(leak)/eps(sig) * BF-Ratios[expected!]
                            MessageSvc::Warning("Add Constraint leakage in J/Psi with Psi mode ...");
                            auto _configBranching     = GetConfigForRatio(RatioType::BranchingFraction, _bkgConfig);
                            auto _branchingRatio      = m_parameterPool->GetRatioParameter(_configBranching, RatioType::BranchingFraction);
                            auto _varEfficiencyLeak   = m_parameterPool->GetEfficiency(_bkgConfig);
                            auto _varEfficiencySignal = m_parameterPool->GetEfficiency(_signalConfig);
                            vector< RooAbsReal * > _numerators     = {_branchingRatio, _varEfficiencyLeak };
                            vector< RooAbsReal * > _denominators   = {_varEfficiencySignal};
                            auto                   _signalYieldVar = (RooRealVar *) m_parameterPool->GetYield(_signalConfig);
                            m_parameterPool->AddYieldFormula(_bkgConfig, _signalYieldVar, _numerators, _denominators);
                        }
                        _holderEE.SetBackgroundYield(to_string(Sample::Leakage), (RooRealVar *) m_parameterPool->GetYield(_bkgConfig));
                    }
                    // Direct cross-feed from Y(RKst-->RK) for pWave + sWave in narrow mass region is "EFFICINECY CONSTRAINED"
                    if (_holderEE.HasComponent(to_string(Sample::Bd2Kst)) ) {
                        if (_managerOption.Contains("crossfeed")) {
                            /*
                                This is creating 
                                Y(Bd2Kst)[RK] = Y[RKst] signal * eps(Bd2Kst,RK)/eps(Signal-RKst)
                            */
                            MessageSvc::Line();
                            MessageSvc::Info("ModifyYields", _manager.second.Name(), _manager.second.Option(), "CrossFeed for Bd2Kst0");
                            MessageSvc::Line();

                            auto _bkgConfig = FitParameterConfig::GetBackgroundConfig(_holderEE.Configuration(), Sample::Bd2Kst);
                            if (_bkgConfig.GetProject() != Prj::RK) MessageSvc::Error("Cross feed", _manager.second.Name(), _managerOption, "Only implemented for RK", "EXIT_FAILURE");                            
                            auto _signalConfigXFeedRKst = _bkgConfig.ReplaceConfig("Bd2Kst" + to_string(_bkgConfig.GetAna())) //change Name sample
                                                           .ReplaceConfig(Prj::RKst) //change Project to RKst
                                                           .ReplaceConfig(GetSignalSample(_bkgConfig.GetQ2bin())); //Change 
                            m_parameterPool->AddBackgroundYieldSignalEfficiencyRatio(_bkgConfig, _signalConfigXFeedRKst);
                            _holderEE.SetBackgroundYield(to_string(Sample::Bd2Kst), (RooRealVar *) m_parameterPool->GetYield(_bkgConfig));
                        }
                    }
                    // InDirect cross-feed from pi0 Miss for pWave + sWave in narrow mass region is NOT "EFFICINECY CONSTRAINED", external factors to load
                    if (_holderEE.HasComponent(to_string(Sample::Bu2Kst))){
                        //TODO: WEAK/NO CHECKING, IMPROVE forcing component to exist
                        if (_managerOption.Contains("crossfeed")) {            
                            auto _bkgConfig_Bu2KstEE = FitParameterConfig::GetBackgroundConfig(_holderEE.Configuration(), Sample::Bu2Kst);                        
                            if (_bkgConfig_Bu2KstEE.GetProject() != Prj::RK) MessageSvc::Error("Cross feed Bu2KstEE", _manager.second.Name(), _managerOption, "Only implemented for RK", "EXIT_FAILURE");
                            auto _bkgConfig_Bd2KstEE = FitParameterConfig::GetBackgroundConfig(_holderEE.Configuration(), Sample::Bd2Kst);
                            if (_bkgConfig_Bd2KstEE.GetProject() != Prj::RK)  MessageSvc::Error("Cross feed Bu2KstEE to Bd2KstEE", _manager.second.Name(), _managerOption, "Only implemented for RK", "EXIT_FAILURE");
                            /* do the trick (remove from pool, construct it, add it)*/
                            m_parameterPool->RemoveYieldParameter(_bkgConfig_Bu2KstEE);
                            auto _ratioPar     = m_parameterPool->GetRatioParameter(_bkgConfig_Bu2KstEE, RatioType::YieldRatio);
                            auto _yieldBd2Kst  = m_parameterPool->GetYield(_bkgConfig_Bd2KstEE);
                            m_parameterPool->AddYieldFormula(_bkgConfig_Bu2KstEE, _ratioPar, _yieldBd2Kst );
                            _holderEE.SetBackgroundYield( to_string(Sample::Bu2Kst),(RooRealVar *) m_parameterPool->GetYield(_bkgConfig_Bu2KstEE));                          
                        }else{
                            MessageSvc::Error("Add -crossfeed option to the manager for Bu2Kst component", "","EXIT_FAILURE");
                        }
                    }
                    // InDirect cross-feed from pWave+sWave pi+ and pi0 Miss for pWave + sWave in NOT narrow mass region is NOT "EFFICINECY CONSTRAINED", external factors to load
                    if (_holderEE.HasComponent(to_string(Sample::BdBu)) ) {
                        //TODO: WEAK/NO CHECKING, IMPROVE forcing component to exist
                        if (_managerOption.Contains("crossfeed")){
                            auto _bkgConfig_BdBu = FitParameterConfig::GetBackgroundConfig(_holderEE.Configuration(), Sample::BdBu);                        
                            if (_bkgConfig_BdBu.GetProject()    != Prj::RK) MessageSvc::Error("Cross feed BdBu(s+pWave outside KstNarrow)", _manager.second.Name(), _managerOption, "Only implemented for RK", "EXIT_FAILURE");
                            auto _bkgConfig_Bd2KstEE = FitParameterConfig::GetBackgroundConfig(_holderEE.Configuration(), Sample::Bd2Kst);
                            if (_bkgConfig_Bd2KstEE.GetProject() != Prj::RK) MessageSvc::Error("Cross feed BdBu(s+pWave outside KstNarrow) to Bd2KstEE", _manager.second.Name(), _managerOption, "Only implemented for RK", "EXIT_FAILURE");
                            /* do the trick */
                            m_parameterPool->RemoveYieldParameter(_bkgConfig_BdBu);
                            auto _ratioPar     = m_parameterPool->GetRatioParameter(_bkgConfig_BdBu, RatioType::YieldRatio);
                            auto _yieldBd2Kst  = m_parameterPool->GetYield(_bkgConfig_Bd2KstEE);
                            m_parameterPool->AddYieldFormula(_bkgConfig_BdBu, _ratioPar, _yieldBd2Kst );
                            _holderEE.SetBackgroundYield( to_string(Sample::BdBu),(RooRealVar *) m_parameterPool->GetYield(_bkgConfig_BdBu));                             
                        }else{
                            MessageSvc::Error("Add -crossfeed option to the manager for BdBu component", "","EXIT_FAILURE");
                        }
                    }                
                    /*
                        NB: The BdBu component for cross-feed is Configured with fixed values externally ( MUST PASS IN A initialParamFile.yaml)
                        Choice is to not make the Y(BdBu) = scale * [ Bd2Kst+Bu2Kst] yield with scale gauss constrained, but rather have it streamed from external inputs. 
                        Toys can be run forcing that value to become 2x bigger or smaller for systematics on the sWave/pWave outside Kst mass window

                        NB2 : All the rest which has not been Yield-modified is Free to Float
                    */
                    if (_holderEE.HasComponent(to_string(Sample::KEtaPrime))) {
                        auto _bkgConfig = FitParameterConfig::GetBackgroundConfig(_holderEE.Configuration(), Sample::KEtaPrime);
                        if (_managerOption.Contains("modyieldbkgketaprime")) {
                            /*
                                Define Y(KEtaPrime-EE) = Y(KEtaPrime-EE) * ( eps(Lb-EE)/eps(Signal-EE) ) * fLb/fd * B.R.( Lb decay )/B.R(Signal-XX decay)
                            */
                            auto _jpsiSignalConfig    =  _bkgConfig.ReplaceConfig("Bu2KJPs" + to_string(_bkgConfig.GetAna())) //change Name sample
                                                                   .ReplaceConfig(Q2Bin::JPsi).ReplaceConfig(GetSignalSample(Q2Bin::JPsi));
                            auto _configBranching     = GetConfigForRatio(RatioType::BranchingFraction, _bkgConfig);
                            auto _branchingRatio      = m_parameterPool->GetRatioParameter(_configBranching, RatioType::BranchingFraction);
                            //Use a full/full ratio of efficiency for this constraint : FULL in Rare, FULL in J/Psi mpde! 
                            bool _forRatioEff = true; 
                            auto _varEfficiencyKEtaPrime = m_parameterPool->GetEfficiency(_bkgConfig, _forRatioEff );
                            auto _varEfficiencySignal    = m_parameterPool->GetEfficiency(_jpsiSignalConfig, _forRatioEff );
                            vector< RooAbsReal * > _numerators     = {_branchingRatio, _varEfficiencyKEtaPrime};
                            vector< RooAbsReal * > _denominators   = {_varEfficiencySignal};
                            auto                   _signalYieldVar = (RooRealVar *) m_parameterPool->GetYield(_jpsiSignalConfig);
                            //BR ratio parameter scaled by * 1000., here we scale back by 1E-4                            
                            m_parameterPool->AddYieldFormula(_bkgConfig, _signalYieldVar, _numerators, _denominators); 
                            _holderEE.SetBackgroundYield(to_string(Sample::KEtaPrime), (RooRealVar *) m_parameterPool->GetYield(_bkgConfig));
                        }
                    }
                }
            }
        }
    }
    for (auto & _manager : m_managers) { _manager.second.PrintKeys(); }
    auto _stop = chrono::high_resolution_clock::now();
    MessageSvc::Warning("FitGenerator", m_name, "ModifyYields took", to_string(chrono::duration_cast< chrono::seconds >(_stop - _start).count()), "seconds");
    return;
}

void FitGenerator::CreateFitter() {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitGenerator", m_name, "CreateFitter");
    MessageSvc::Line();

    if (!m_isInitialized) MessageSvc::Error("FitGenerator", m_name, "Not initialized", "EXIT_FAILURE");

    if (m_fitter != nullptr) {
        m_fitter->Close();
        delete m_fitter;
        m_fitter = nullptr;
    }
    m_fitter = new FitterTool(m_name);
    for (auto & _manager : m_managers) {
        if (!_manager.second.IsInitialized()) MessageSvc::Error("FitManager", _manager.second.Name(), "Not initialized", "EXIT_FAILURE");
        if (SettingDef::Fit::rareOnly ){
            if( _manager.second.Name().Contains(to_string(Q2Bin::JPsi))) MessageSvc::Warning("RareOnly True, skipping manager J/Psi adding");
            else m_fitter->AddFitManager(&_manager.second);
        }else{
            m_fitter->AddFitManager(&_manager.second); // In the rare only fit, the fitter tool only considers the rare mode.
        }
    }
    if( m_option.Contains("profileRatios")  ) m_fitter->SetProfile1D(true);
    if( m_option.Contains("profileRatios2D")) m_fitter->SetProfile2D(true);    
    return;
}

void FitGenerator::Fit() {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitGenerator", m_name, "Fit");
    MessageSvc::Line();

    if (!m_fitter) MessageSvc::Error("FitGenerator", m_name, "Fitter not available", "EXIT_FAILURE");
    for (auto & _manager : m_managers) { _manager.second.PrintPDFs(); }
    m_fitter->SetConstraintFlag(m_option.Contains("gconst"));
    m_fitter->SetConstraintFlagEffs(m_option.Contains("gconsteffs"));
    m_fitter->Minos(m_option.Contains("profileRatios") || SettingDef::Fit::RatioParsMinos);
    m_fitter->InitialHesse( m_option.Contains("initialHesse"));
    if(SettingDef::Fit::nCPUDataFit >1 ){
        m_fitter->NCPU( SettingDef::Fit::nCPUDataFit);
    }
    if( m_option.Contains("noLLOffset")){
        m_fitter->Offset(false);
    }else{
        m_fitter->Offset(true);
    }
    m_fitter->Init();
    m_fitter->SetInitialValuesAndErrors();
    m_fitter->Fit();

    // SaveToDisk("afterfit",true); Really needed? commented out , probably we never used it really
    m_parameterPool->PrintParameters();
        
    if (m_option.Contains("saveWS")) m_fitter->SaveToWorkspace( m_name+"_fullFit");
    if (m_option.Contains("splot")) m_fitter->DoSPlot();

    return;
}

void FitGenerator::LogFitResult() {
    //Name of the file where a tuple with results are saved
    TString _fileOut = "FitResult.root";
    //Avoid to dump results to EOS path when running fits.
    TString _resultDir = ".";
    FitResultLogger _logger;
    RooFitResult * _fitResult = m_fitter->Results();
    for (auto * variable : _fitResult->floatParsFinal()){
        _logger.AddVariable(*((RooRealVar*)variable));
    }

    if( SettingDef::Weight::useBS == true){
        //If we are in BS-fits mode , save to tuple also the bsIdx loop done.        
        RooRealVar * bsIdx = new RooRealVar("bsIdx", "bsIdx", SettingDef::Weight::iBS, 0, WeightDefRX::nBS);
        bsIdx->setConstant(0);
        _logger.AddVariable( *(RooRealVar*)bsIdx);
        _fileOut = "FitResultBS.root";
        _resultDir = SettingDef::IO::outDir;
    }else{
        TString _ResDir = SettingDef::IO::useEOS ? _resultDir : IOSvc::GetFitDir("", ConfigHolder());
        if( IsBATCH("DORTMUND") ){ 
           _resultDir = _resultDir; 
        }else{
           _resultDir = _ResDir ; //the same as before
        } 
    }
    _logger.LogFit(*_fitResult, m_fitter->LLOffset() );
    TString _resultPath =  _resultDir +"/"+_fileOut;
    MessageSvc::Info("FitGenerator::LogFitResult", _resultPath);
    _logger.SaveResults(_resultPath, "NominalFit");
}

void FitGenerator::SaveToDisk(TString _name, bool _force) {
    if (!m_saveToDisk && !_force) {
        MessageSvc::Warning("FitGenerator", m_name, "SKIPPING SaveToDisk");
        return;
    }
    if (m_isLoaded) {
        MessageSvc::Warning("FitGenerator", m_name, "LoadedFromDisk SKIPPING SaveToDisk");
        return;
    }
    // SaveToLog(_name);

    if (!SettingDef::IO::outDir.EndsWith("/")) SettingDef::IO::outDir += "/";
    if (_name != "") _name = "_" + _name;
    TString _oname = m_name + _name;
    _name          = SettingDef::IO::outDir + _oname + ".root";

    MessageSvc::Line();
#ifdef STREAMDATA
    MessageSvc::Info(Color::Cyan, "FitGenerator", m_name, "SaveToDisk", _name, "with STREAMDATA");
#else
    MessageSvc::Warning("FitGenerator", m_name, "SaveToDisk", _name, "without STREAMDATA");
#endif
    MessageSvc::Line();

    m_parameterSnapshot.ConfigureSnapshotMap();

    TFile _tFile(_name, to_string(OpenMode::RECREATE));
    (*this).Write(_oname, TObject::kOverwrite);

    _tFile.Close();
    cout << WHITE << *this << RESET << endl;
    return;
}

void FitGenerator::LoadFromDisk(TString _name, TString _dir) {
    MessageSvc::Line();
#ifdef STREAMDATA
    MessageSvc::Info(Color::Cyan, "FitGenerator", m_name, "LoadFromDisk", _name, _dir, "with STREAMDATA");
#else
    MessageSvc::Warning("FitGenerator", m_name, "LoadFromDisk", _name, _dir, "without STREAMDATA");
#endif

    if (_name != "") _name = "_" + _name;
    _name = m_name + _name;

    if ((_dir != "") && (!_dir.EndsWith("/"))) _dir += "/";

    if (IOSvc::ExistFile(_dir + _name + ".root")) {
        TFile _tFile(_dir + _name + ".root", "read");
        MessageSvc::Line();
        _tFile.ls();
        MessageSvc::Line();

        FitGenerator * _fm = (FitGenerator *) _tFile.Get(_name);
        *this              = *_fm;

        SetStatus(true, m_isReduced);
        m_parameterSnapshot.ReloadParameters();

        RefreshParameterPool();

        _tFile.Close();
        cout << WHITE << *this << RESET << endl;
    } else {
        MessageSvc::Error("FitGenerator", _dir + _name + ".root", "does not exist");
    }
    return;
}

void FitGenerator::SaveToLog(TString _name) const noexcept {
    if (!SettingDef::IO::outDir.EndsWith("/")) SettingDef::IO::outDir += "/";
    if (_name != "") _name = "_" + _name;
    _name = SettingDef::IO::outDir + m_name + _name + ".log";

    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitGenerator", m_name, "SaveToLog", _name);

    ofstream _file(_name);
    if (!_file.is_open()) MessageSvc::Error("Unable to open file", _name, "EXIT_FAILURE");
    _file << *this << endl;
    _file.close();

    MessageSvc::Line();
    return;
}

void FitGenerator::SaveToYAML(TString _name, TString _option) const noexcept {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitGenerator", m_name, "SaveToYAML", _name, _option, "Manager(s)", to_string(m_managers.size()));

    if (_name != "") _name = "_" + _name;

    vector< FitConfiguration >                          _configurationsFit = SettingDef::Fit::configurations;
    map< pair< Prj, Q2Bin >, pair< TString, TString > > _yamlsFit          = SettingDef::Fit::yamls;

    vector< FitConfiguration >                          _configurationsToy = SettingDef::Toy::configurations;
    map< pair< Prj, Q2Bin >, pair< TString, TString > > _yamlsToy          = SettingDef::Toy::yamls;
    auto _mainPrj = SettingDef::Config::project;
    auto _mainQ2  = SettingDef::Config::q2bin;
    for (const auto & _manager : m_managers) {
        SettingDef::Fit::configurations.clear();
        SettingDef::Fit::yamls.clear();
        SettingDef::Toy::configurations.clear();
        SettingDef::Toy::yamls.clear();

        Prj   _project = Prj::All;
        Q2Bin _q2bin   = Q2Bin::All;

        // SKIP MULTIPLE CONFIGURATIONS FOR YEARS AND TRIGGERS (SEE ParserSvc::GetConfigurationsYAML)
        if (_manager.second.ConfigurationsMM().size() != 0) {
            SettingDef::Fit::configurations.push_back(_manager.second.ConfigurationsMM()[0]);
            SettingDef::Toy::configurations.push_back(_manager.second.ConfigurationsMM()[0]);
            _project = _manager.second.ConfigurationsMM()[0].GetProject();
            _q2bin   = _manager.second.ConfigurationsMM()[0].GetQ2bin();
        }
        if (_manager.second.ConfigurationsEE().size() != 0) {
            SettingDef::Fit::configurations.push_back(_manager.second.ConfigurationsEE()[0]);
            SettingDef::Toy::configurations.push_back(_manager.second.ConfigurationsEE()[0]);
            _project = _manager.second.ConfigurationsEE()[0].GetProject();
            _q2bin   = _manager.second.ConfigurationsEE()[0].GetQ2bin();
        }

        SettingDef::Config::project = to_string(_project);
        SettingDef::Config::q2bin   = to_string(_q2bin);
        std::cout<< "SaveToYaml "<< SettingDef::Config::project  << " , " << SettingDef::Config::q2bin << std::endl;
        _manager.second.SaveToYAML("", _option + "-smanager");
    }
    SettingDef::Config::project = _mainPrj;
    SettingDef::Config::q2bin = _mainQ2;
    //up to here seems to save fine, but global Project and q2 bin IS from the last iteration. 

    MessageSvc::Info(Color::Cyan, "FitGenerator", m_name, "SaveToYAML", _name, _option, "YAML(s)");

    SettingDef::Fit::configurations.clear();
    SettingDef::Fit::yamls.clear();
    SettingDef::Toy::configurations.clear();
    SettingDef::Toy::yamls.clear();


    for (const auto & _manager : m_managers) {
        Prj   _project = Prj::All;
        Q2Bin _q2bin   = Q2Bin::All;
        if (_manager.second.ConfigurationsMM().size() != 0) {
            _project = _manager.second.ConfigurationsMM()[0].GetProject();
            _q2bin   = _manager.second.ConfigurationsMM()[0].GetQ2bin();
        }
        if (_manager.second.ConfigurationsEE().size() != 0) {
            _project = _manager.second.ConfigurationsEE()[0].GetProject();
            _q2bin   = _manager.second.ConfigurationsEE()[0].GetQ2bin();
        }
        SettingDef::Fit::yamls[make_pair(_project, _q2bin)] = make_pair("config-" + _manager.second.Name() + ".yaml", _manager.second.Option());
        SettingDef::Toy::yamls[make_pair(_project, _q2bin)] = make_pair("config-" + _manager.second.Name() + ".yaml", "");
    }

    MessageSvc::Info(Color::Cyan, "FitGenerator", m_name, "Fit YAML(s)", to_string(SettingDef::Fit::yamls.size()));
    MessageSvc::Info(Color::Cyan, "FitGenerator", m_name, "Toy YAML(s)", to_string(SettingDef::Toy::yamls.size()));

    EventType _et = EventType();
    _et.SaveToYAML(m_name + _name);

    SettingDef::Fit::configurations = _configurationsFit;
    SettingDef::Fit::yamls          = _yamlsFit;
    SettingDef::Toy::configurations = _configurationsToy;
    SettingDef::Toy::yamls          = _yamlsToy;

    MessageSvc::Line();
    return;
}

bool FitGenerator::IsInManagerMap(TString _name) const noexcept {
    if (m_managers.find(_name) == m_managers.end()) return false;
    return true;
}

void FitGenerator::ReduceComponents(TCut _cut) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitGenerator", m_name, "ReduceComponents", TString(_cut));
    MessageSvc::Line();

    for (auto & _manager : m_managers) { _manager.second.ReduceComponents(_cut); }

    SetStatus(m_isLoaded, true);

    cout << WHITE << *this << RESET << endl;
    return;
}

#endif
