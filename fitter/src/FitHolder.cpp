#ifndef FITHOLDER_CPP
#define FITHOLDER_CPP

#include "FitHolder.hpp"

#include "SettingDef.hpp"

#include "FitterTool.hpp"

#include "vec_extends.h"

ClassImp(FitHolder)

    //==============================================================================
    //                              Constructors
    //==============================================================================

    //---- Standard Constructors of the FitHolder class
    FitHolder::FitHolder(TString _name, FitConfiguration _configuration, TString _option) {
    if (SettingDef::debug.Contains("FH")) SetDebug(true);
    m_name = _name;
    m_name = CleanString(m_name);
    if (m_debug) MessageSvc::Debug("FitHolder", m_name, "FitConfiguration");
    m_configuration = _configuration;
    m_option        = _option;
    if (!m_configuration.Var()) MessageSvc::Error("FitHolder", m_name, "Var is nullptr", "EXIT_FAILURE");
    Check();
    m_parameterPool = RXFitter::GetParameterPool();
    cout << WHITE << *this << RESET << endl;
}

FitHolder::FitHolder(TString _name, RooRealVar * _var, TString _option) {
    if (SettingDef::debug.Contains("FH")) SetDebug(true);
    m_name = _name;
    m_name = CleanString(m_name);
    if (m_debug) MessageSvc::Debug("FitHolder", m_name, "RooRealVar");
    m_configuration = FitConfiguration(hash_project(SettingDef::Config::project), 
                                      hash_analysis(SettingDef::Config::ana), 
                                      hash_q2bin(SettingDef::Config::q2bin), 
                                      hash_year(SettingDef::Config::year), 
                                      hash_polarity(SettingDef::Config::polarity), 
                                      hash_trigger(SettingDef::Config::trigger), 
                                      hash_brem(SettingDef::Config::brem), 
                                      hash_track(SettingDef::Config::track), 
                                      ((TString) _var->GetName()).Contains("DTF_JPs") || ((TString) _var->GetName()).Contains("DTF_Psi") ? true : false, 
                                      make_tuple(SettingDef::Fit::doBinned, SettingDef::Fit::nBins, 
                                      _var->getMin(), _var->getMax()), 
                                      make_tuple(SettingDef::Fit::doBinned, SettingDef::Fit::nBins, _var->getMin(), _var->getMax()), 
                                      vector< TString >{" | | | | | "});
    m_option        = _option;
    if (!m_configuration.Var()) MessageSvc::Error("FitHolder", m_name, "Var is nullptr", "EXIT_FAILURE");
    Check();
    m_parameterPool = RXFitter::GetParameterPool();
    cout << WHITE << *this << RESET << endl;
}

//---- Copy Constructors of the FitHolder class
FitHolder::FitHolder(const FitHolder & _fitHolder) {
    if (SettingDef::debug.Contains("FH")) SetDebug(true);
    m_name = _fitHolder.Name();
    m_name = CleanString(m_name);
    if (m_debug) MessageSvc::Debug("FitHolder", m_name, "FitHolder");
    m_configuration   = _fitHolder.Configuration();
    m_option          = _fitHolder.Option();
    m_isInitialized   = _fitHolder.IsInitialized();
    m_isLoaded        = _fitHolder.IsLoaded();
    m_isReduced       = _fitHolder.IsReduced();
    m_sigComponentKey = _fitHolder.SignalComponentKey();
    m_sigComponent    = _fitHolder.SignalComponent();
    m_bkgComponents   = _fitHolder.BackgroundComponents();
    m_data            = _fitHolder.Data();
    if (_fitHolder.GetModel() != nullptr) m_model = (RooAddPdf *) _fitHolder.GetModel()->Clone();
    if (_fitHolder.Fitter() != nullptr) m_fitter = static_cast< FitterTool * >(_fitHolder.Fitter());
    m_parameterPool = RXFitter::GetParameterPool();
    // cout << WHITE << *this << RESET << endl;
}

FitHolder::FitHolder(TString _holderName, TString _option, TString _name, TString _dir) {
    if (SettingDef::debug.Contains("FH")) SetDebug(true);
    m_name = _holderName;
    m_name = CleanString(m_name);
    if (m_debug) MessageSvc::Debug("FitHolder", m_name, "LoadFromDisk");
    LoadFromDisk(_name, _dir);
    m_parameterPool = RXFitter::GetParameterPool();
    if (_option != m_option) {
        MessageSvc::Line();
        MessageSvc::Info(Color::Cyan, "FitHolder", m_name, "Resetting fit options");
        MessageSvc::Info("OLD", m_option);
        MessageSvc::Info("NEW", _option);
        MessageSvc::Line();
        m_option = _option;
        cout << WHITE << *this << RESET << endl;
    }
}

//---- Inline Printing for the (cout<< FitHolder << endl;)
ostream & operator<<(ostream & os, const FitHolder & _fitHolder) {
    os << WHITE;
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "FitHolder");
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "name", _fitHolder.Name());
    MessageSvc::Print((ostream &) os, "option", _fitHolder.Option());
    MessageSvc::Print((ostream &) os, "loaded", to_string(_fitHolder.IsLoaded()));
    MessageSvc::Print((ostream &) os, "reduced", to_string(_fitHolder.IsReduced()));
    MessageSvc::Print((ostream &) os, "configuration", _fitHolder.Configuration().GetKey());
    MessageSvc::Print((ostream &) os, "var", _fitHolder.Configuration().Var()->GetName());
    if (_fitHolder.GetSig() != nullptr) MessageSvc::Print((ostream &) os, "signal", _fitHolder.SignalName());
    for (const auto & _bkg : _fitHolder.BackgroundComponents()) { MessageSvc::Print((ostream &) os, "background", _bkg.second.fitComponent.Name()); }
    if (_fitHolder.GetDataSet() != nullptr) MessageSvc::Print((ostream &) os, "data", _fitHolder.Data().Name());
    MessageSvc::Line(os);
    os << RESET;
    os << "\033[F";
    return os;
}

//---- Internal checks for the FitHolder option
bool FitHolder::Check() {
    for (auto _opt : TokenizeString(m_option, SettingDef::separator)) {
        _opt = RemoveStringAfter(_opt, "[");
        if (!CheckVectorContains(SettingDef::AllowedConf::FitOptions, _opt)) {
            cout << RED << *this << RESET << endl;
            MessageSvc::Error("FitHolder", "\"" + _opt + "\"", "option not in SettingDef::AllowedConf::FitOptions", "EXIT_FAILURE");
        }
    }

    if (m_name.Contains(to_string(Analysis::MM)) && m_name.Contains(to_string(Analysis::EE))) MessageSvc::Error("FitHolder", m_name, "not supported because multiple Analysis in the name", "EXIT_FAILURE");
    if (m_name.Contains(to_string(Analysis::MM)) && m_name.Contains(to_string(Analysis::ME))) MessageSvc::Error("FitHolder", m_name, "not supported because multiple Analysis in the name", "EXIT_FAILURE");
    if (m_name.Contains(to_string(Analysis::EE)) && m_name.Contains(to_string(Analysis::ME))) MessageSvc::Error("FitHolder", m_name, "not supported because multiple Analysis in the name", "EXIT_FAILURE");
    if (!(m_name.Contains(to_string(Analysis::MM)) || m_name.Contains(to_string(Analysis::EE)) || m_name.Contains(to_string(Analysis::ME)))) MessageSvc::Error("FitHolder", m_name, "not supported because Analysis not in the name", "EXIT_FAILURE");

    return false;
}

//========================================================================================================
// Internal operations to dynamically generate and create the objects in the container (Core of the Class)
//========================================================================================================

void FitHolder::CreateSignal(const EventType & _eventType, TString _key, TString _pdf, TString _option) {
    _key = CleanString(_key);
    if (!_option.Contains(m_option)) _option += "-EXTERNAL-" + m_option;

    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitHolder", m_name, "CreateSignal", _key, _pdf, _option);
    MessageSvc::Line();

    TString _name = m_name;
    if (_key != "") _name = _key + SettingDef::separator + _name;

    if (IsSigKey()) MessageSvc::Error("IsSigKey", _key, "already created", "EXIT_FAILURE");

    if (_pdf == to_string(PdfType::StringToFit))
        m_configuration.UseBinAndRange(SettingDef::Fit::varSchemeMC);
    else
        m_configuration.UseBinAndRange(SettingDef::Fit::varSchemeCL);

    m_sigComponentKey           = _key;
    //Why assign and not
    m_sigComponent.fitComponent = FitComponent(_eventType, _name, _pdf, m_configuration.Var(), _option);
    m_sigComponent.yield        = CreateYield(_name, "sig_");

    m_sigComponent.fitComponent.SetBinnedFit(m_configuration.IsBinnedMC());
    m_sigComponent.fitComponent.CreatePDF();
    m_sigComponent.fitComponent.PDF()->SetName(_name);

    m_sigComponent.fitComponent.Close(m_option + "-shape");

    MessageSvc::Line();
    MessageSvc::Info("CreateSignal", SignalName());
    MessageSvc::Info("CreateSignal", SignalPDF());
    MessageSvc::Info("CreateSignal", SignalYield());
    MessageSvc::Line();
    return;
}

void FitHolder::CreateSignal(const EventType & _eventType, TString _key, RooAddPdf * _pdf, TString _option) {
    _key = CleanString(_key);
    if (!_option.Contains(m_option)) _option += "-EXTERNAL-" + m_option;

    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitHolder", m_name, "CreateSignal", _key, _pdf->GetName(), "with RooAddPDF", _option);
    MessageSvc::Line();

    TString _name = m_name;
    if (_key != "") _name = _key + SettingDef::separator + _name;

    if (IsSigKey()) MessageSvc::Error("IsSigKey", _key, "already created", "EXIT_FAILURE");
    m_sigComponentKey           = _key;
    m_sigComponent.fitComponent = FitComponent(_eventType, _name, _pdf, m_configuration.Var(), _option);
    m_sigComponent.yield        = CreateYield(_name, "sig_");

    m_sigComponent.fitComponent.Close(m_option + "-shape");

    MessageSvc::Line();
    MessageSvc::Info("CreateSignal with RooAddPDF", SignalName());
    MessageSvc::Info("CreateSignal with RooAddPDF", SignalPDF());
    MessageSvc::Info("CreateSignal with RooAddPDF", SignalYield());
    MessageSvc::Line();
    return;
}

void FitHolder::CreateSignal(const FitComponent & _component) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitHolder", m_name, "CreateSignal", _component.Name());
    MessageSvc::Line();

    TString _key  = ((TString) _component.Name()).Remove(_component.Name().First(SettingDef::separator), _component.Name().Length());
    TString _name = _component.Name();

    if (IsSigKey()) MessageSvc::Error("IsSigKey", _key, "already created", "EXIT_FAILURE");

    if (_component.Type() == PdfType::StringToFit)
        m_configuration.UseBinAndRange(SettingDef::Fit::varSchemeMC);
    else
        m_configuration.UseBinAndRange(SettingDef::Fit::varSchemeCL);

    m_sigComponentKey           = _key;
    m_sigComponent.fitComponent = _component;
    m_sigComponent.yield        = CreateYield(_name, "sig_");

    m_sigComponent.fitComponent.SetVar(m_configuration.Var());
    m_sigComponent.fitComponent.SetBinnedFit(m_configuration.IsBinnedMC());

    m_sigComponent.fitComponent.Close(m_option + "-shape");

    MessageSvc::Line();
    MessageSvc::Info("CreateSignal", SignalName());
    MessageSvc::Info("CreateSignal", SignalPDF());
    MessageSvc::Info("CreateSignal", SignalYield());
    MessageSvc::Info("CreateSignal", m_sigComponent.fitComponent.Var(), "v");
    MessageSvc::Line();
    return;
}

void FitHolder::CreateBackground(const EventType & _eventType, TString _key, TString _pdf, TString _option) {
    _key = CleanString(_key);
    if (!_option.Contains(m_option)) _option += "-EXTERNAL-" + m_option;

    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitHolder", m_name, "CreateBackground", _key, _pdf, _option);
    MessageSvc::Line();

    TString _name = m_name;
    if (_key != "") _name = _key + SettingDef::separator + _name;

    if (IsInBkgComponentMap(_key)) MessageSvc::Error("IsInBkgComponentMap", _key, "already in map", "EXIT_FAILURE");

    if (_pdf == to_string(PdfType::StringToFit))
        m_configuration.UseBinAndRange(SettingDef::Fit::varSchemeMC);
    else
        m_configuration.UseBinAndRange(SettingDef::Fit::varSchemeCL);

    m_bkgComponents[_key].fitComponent = FitComponent(_eventType, _name, _pdf, m_configuration.Var(), _option);
    m_bkgComponents[_key].yield        = CreateYield(_name, "bkg_");

    m_bkgComponents[_key].fitComponent.SetBinnedFit(m_configuration.IsBinnedMC());
    m_bkgComponents[_key].fitComponent.CreatePDF();

    m_bkgComponents[_key].fitComponent.Close(m_option + "-shape");

    MessageSvc::Line();
    MessageSvc::Info("CreateBackground Name ", BackgroundName(_key));
    MessageSvc::Info("CreateBackground PDF  ", BackgroundPDF(_key));
    MessageSvc::Info("CreateBackground Yield", BackgroundYield(_key));
    MessageSvc::Line();
    return;
}

void FitHolder::CreateBackground(const EventType & _eventType, TString _key, RooAddPdf * _pdf, TString _option) {
    _key = CleanString(_key);
    if (!_option.Contains(m_option)) _option += "-EXTERNAL-" + m_option;    

    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitHolder", m_name, "CreateBackground", _key, _pdf->GetName(), "with RooAddPDF", _option);
    MessageSvc::Line();

    TString _name = m_name;
    if (_key != "") _name = _key + SettingDef::separator + _name;

    m_bkgComponents[_key].fitComponent = FitComponent(_eventType, _name, _pdf, m_configuration.Var(), _option);
    m_bkgComponents[_key].yield        = CreateYield(_name, "bkg_");

    m_bkgComponents[_key].fitComponent.Close(m_option + "-shape");

    MessageSvc::Line();
    MessageSvc::Info("CreateBackground with RooAddPDF", BackgroundName(_key));
    MessageSvc::Info("CreateBackground with RooAddPDF", BackgroundPDF(_key));
    MessageSvc::Info("CreateBackground with RooAddPDF", BackgroundYield(_key));
    MessageSvc::Line();
}

void FitHolder::CreateBackgroundFromSignal(const EventType & _eventType, TString _key) {
    _key = CleanString(_key);
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitHolder", m_name, "CreateBackground", _key, "from", m_sigComponentKey);
    MessageSvc::Line();

    TString _name = m_name;
    if (_key != "") _name = _key + SettingDef::separator + _name;

    if (IsInBkgComponentMap(_key)) MessageSvc::Error("IsInBkgComponentMap", _key, "already in map", "EXIT_FAILURE");
    m_bkgComponents[_key].fitComponent = FitComponent(m_sigComponent.fitComponent);
    cout << WHITE << m_bkgComponents[_key].fitComponent << RESET << endl;
    m_bkgComponents[_key].yield = CreateYield(_name, "bkg_");

    m_bkgComponents[_key].fitComponent.SetEventType(_eventType);
    m_bkgComponents[_key].fitComponent.SetName(_name);
    m_bkgComponents[_key].fitComponent.PDF()->SetName(_name);

    m_bkgComponents[_key].fitComponent.Close(m_option + "-shape");

    MessageSvc::Line();
    MessageSvc::Info("CreateBackgroundFromSignal", BackgroundName(_key));
    MessageSvc::Info("CreateBackgroundFromSignal", BackgroundPDF(_key));
    MessageSvc::Info("CreateBackgroundFromSignal", BackgroundYield(_key));
    MessageSvc::Line();
    return;
}

void FitHolder::CreateBackground(const FitComponent & _component) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitHolder", m_name, "CreateBackground", _component.Name());
    MessageSvc::Line();

    TString _key  = ((TString) _component.Name()).Remove(_component.Name().First(SettingDef::separator), _component.Name().Length());
    TString _name = _component.Name();

    if (IsInBkgComponentMap(_key)) MessageSvc::Error("IsInBkgComponentMap", _key, "already in map", "EXIT_FAILURE");

    if (_component.Type() == PdfType::StringToFit)
        m_configuration.UseBinAndRange(SettingDef::Fit::varSchemeMC);
    else
        m_configuration.UseBinAndRange(SettingDef::Fit::varSchemeCL);

    m_bkgComponents[_key].fitComponent = _component;
    m_bkgComponents[_key].yield        = CreateYield(_name, "bkg_");

    m_bkgComponents[_key].fitComponent.SetVar(m_configuration.Var());
    m_bkgComponents[_key].fitComponent.SetBinnedFit(m_configuration.IsBinnedMC());

    m_bkgComponents[_key].fitComponent.Close(m_option + "-shape");

    MessageSvc::Line();
    MessageSvc::Info("CreateBackground", BackgroundName(_key));
    MessageSvc::Info("CreateBackground", BackgroundPDF(_key));
    MessageSvc::Info("CreateBackground", BackgroundYield(_key));
    MessageSvc::Info("CreateBackground", m_bkgComponents[_key].fitComponent.Var(), "v");
    MessageSvc::Line();
    return;
}

void FitHolder::CreateData(const EventType & _eventType, TString _key, TString _option) {
    _key = CleanString(_key);
    if (!_option.Contains(m_option)) _option += "-EXTERNAL-" + m_option;    

    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitHolder", m_name, "CreateData", _key, _option);
    MessageSvc::Line();

    TString _name = m_name;
    if (_key != "") _name = _key + SettingDef::separator + _name;

    m_configuration.UseBinAndRange(SettingDef::Fit::varSchemeCL);
    
    m_data = FitComponent(_eventType, _name, "", m_configuration.Var(), _option);
    

    m_data.SetBinnedFit(m_configuration.IsBinnedCL());
    m_data.CreateData();

    if (m_data.DataSize() == 0) MessageSvc::Error("CreateData", (TString) "Empty data", "EXIT_FAILURE");

    if (_option.Contains("splot")) {
        if (!SettingDef::IO::outDir.EndsWith("/")) SettingDef::IO::outDir += "/";
        TString _oname = "FitHolder_" + m_name + "_SPlot";
        TString _NAME_  = SettingDef::IO::outDir + _oname + ".root";

        MessageSvc::Line();
        MessageSvc::Info("CreateData", (TString) "SaveToDisk", _NAME_);

        if (SettingDef::Tuple::dataFrame) {
            if (!IOSvc::ExistFile("tmp" + _key + SettingDef::separator + m_name + ".root")) MessageSvc::Error("CreateData", (TString) "tmp" + _key + SettingDef::separator + m_name + ".root", "does not exist", "EXIT_FAILURE");
            IOSvc::runCommand("mv tmp" + _key + SettingDef::separator + m_name + ".root " + _NAME_);
        } else {
            TFile   _tFileTmp("tmp" + _key + SettingDef::separator + m_name + ".root", "read");
            TTree * _tuple = (TTree *) _tFileTmp.Get("DecayTuple");

            TFile _tFile(_NAME_, to_string(OpenMode::RECREATE));
            _tuple->CloneTree()->Write("DecayTuple", TObject::kOverwrite);
            _tFile.Close();
            _tFileTmp.Close();
        }
    }

    m_data.Close(m_option + "-data");

    MessageSvc::Line();
    MessageSvc::Info("CreateData", m_data.Name());
    MessageSvc::Info("CreateData", m_data.DataSet());
    MessageSvc::Line();
    return;
}

void FitHolder::CreateData(const FitComponent & _component) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitHolder", m_name, "CreateData", _component.Name());
    MessageSvc::Line();

    TString _key  = ((TString) _component.Name()).Remove(_component.Name().First(SettingDef::separator), _component.Name().Length());
    TString _name = _component.Name();

    m_configuration.UseBinAndRange(SettingDef::Fit::varSchemeCL);

    m_data = _component;
    m_data.SetVar(m_configuration.Var());
    m_data.SetBinnedFit(m_configuration.IsBinnedCL());

    m_data.Close(m_option + "-data");

    MessageSvc::Line();
    MessageSvc::Info("CreateData", m_data.Name());
    if (m_data.DataSet() != nullptr) MessageSvc::Info("CreateData", m_data.DataSet());
    if (m_data.DataHist() != nullptr) MessageSvc::Info("CreateData", m_data.DataHist());
    MessageSvc::Info("CreateData", m_data.Var(), "v");
    MessageSvc::Line();
    return;
}

// Generate Yields for the given componentKey (ID of the FitCompoennt) name.
// The _componentType flag is used to identify if the the yield to add is assigned to a signal or bkg yield
RooAbsReal * FitHolder::CreateYield(TString _key, TString _componentType) {
    _key.ReplaceAll("MC", "").ReplaceAll("CL", "");
    _key                = CleanString(_key);
    RooRealVar * _yield = new RooRealVar("n" + _componentType + _key, "N_{" + _key + "}", 1);
    _yield->setConstant(0);
    MessageSvc::Info("CreateYield", _yield);
    return (RooRealVar *) _yield;
}

// Generate the Model for the data fit given a list of pdfs and yields.
// Assumes list of pdfs and yields are all in place (same size of the list)
void FitHolder::CreateModel(const RooArgList & _pdfs, const RooArgList & _yields) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitHolder", m_name, "CreateModel");
    MessageSvc::Line();
    MessageSvc::Info("CreateModel", &_pdfs);
    MessageSvc::Info("CreateModel", &_yields);

    if (_yields.getSize() != _pdfs.getSize()) MessageSvc::Error("FitHolder", m_name, "CreateModel", "Illegal sum of PDF, accepted only same size pdfs and list of yields", "EXIT_FAILURE");

    if( SettingDef::Fit::useRooRealSumPDF){
        MessageSvc::Warning("Using RooRealSumPdf models, Extended, UNTESTED");
        abort() ; 
        //maybe illegal pointer?
        m_model = new RooRealSumPdf("model_" + m_name, "model_" + m_name, _pdfs, _yields, kTRUE); //Recursive fraction OFF ! this is each component has a yield
    }else{
        m_model = new RooAddPdf("model_" + m_name, "model_" + m_name, _pdfs, _yields, kFALSE); //Recursive fraction OFF ! this is each component has a yield
    }
    if( m_model->extendMode() == RooAbsPdf::ExtendMode::CanNotBeExtended ){
        MessageSvc::Warning("CreateModel, m_model is NOT extendable!", m_name);
    }

    MessageSvc::Info("CreateModel", m_model, "v");
    MessageSvc::Line();
    return;
}

void FitHolder::SetConstantModelAllPars() {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitHolder", m_name, "SetConstantModelAllPars");
    cout << GREEN;
    Str2VarMap _pars = GetPars(m_model, m_configuration.Var(), "orignames");
    FixPars(&_pars);
    MessageSvc::Line();
    return;
}

void FitHolder::SetConstantModelExceptPars(vector< string > _names) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitHolder", m_name, "SetConstantModelExceptPars", to_string(_names.size()));
    for (size_t i = 0; i < _names.size(); i++) MessageSvc::Info("SetConstantModelExceptPars", _names[i]);
    cout << GREEN;
    Str2VarMap _pars = GetPars(m_model, m_configuration.Var(), "orignames");
    FixPars(&_pars, _names, "except");
    MessageSvc::Line();
    return;
}

void FitHolder::PrintModelParameters() const noexcept {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitHolder", m_name, "PrintModelParameters");
    cout << GREEN;
    Str2VarMap _pars = GetPars(m_model, m_configuration.Var(), "orignames");
    PrintPars(_pars);
    MessageSvc::Line();
    return;
}

//---------------------  External interaction for the YIELDS re-definition -------------------- //

// Force the setting of the SignalYield for this fit Holder given an external variable
// Mechanism used to replace the yield generated with some custom one defined passing via a RooFormulaVar
void FitHolder::SetSignalYield(RooAbsReal * _var) {
    if (!IsSigKey()) MessageSvc::Error("IsSigKey", m_sigComponentKey, "not in map", "EXIT_FAILURE");
    if (_var != nullptr) {
        if (m_debug) MessageSvc::Debug("SetSignalYield", m_sigComponent.yield);
        m_sigComponent.yield = _var;
        MessageSvc::Info("SetSignalYield", _var);
    } else
        MessageSvc::Info(Color::Cyan, "FitHolder", m_name, "SetSignalYield", "Nothing to modify");
    return;
}

// Force the setting of the Background Yield for this fit Holder given an external variable
// Mechanism used to replace the yield generated with some custom one defined passing via a RooFormulaVar
void FitHolder::SetBackgroundYield(TString _key, RooAbsReal * _var) {
    if (!IsInBkgComponentMap(_key)) MessageSvc::Error("IsInBkgComponentMap", _key, "not in map", "EXIT_FAILURE");
    if (_var != nullptr) {
        if (m_debug) MessageSvc::Debug("SetBackgroundYield", m_bkgComponents[_key].yield);
        m_bkgComponents[_key].yield = _var;
        MessageSvc::Info("SetBackgroundYield", _var);
    } else
        MessageSvc::Info(Color::Cyan, "FitHolder", m_name, "SetBackgroundYield", "Nothing to modify");
    return;
}

// We can return also a nullptr and assign by hand a value, be careful with this method, it's used to re-define the yield of a component with a RooFormulaVar
RooAbsReal * FitHolder::BackgroundYield(TString _key) {
    if (!IsInBkgComponentMap(_key)) MessageSvc::Error("IsInBkgComponentMap", _key, "not in map", "EXIT_FAILURE");
    return m_bkgComponents[_key].yield;
}

void FitHolder::FixBackgroundYield(TString _key, double _value) {
    if (IsInBkgComponentMap(_key)) {
        MessageSvc::Warning("FitHolder", m_name, "FixBackgroundYield", _key, to_string(_value));
        ((RooRealVar *) m_bkgComponents[_key].yield)->setVal(_value);
        ((RooRealVar *) m_bkgComponents[_key].yield)->setConstant(1);
    }
    return;
}

TString FitHolder::BackgroundName(TString _key) const noexcept {
    if (!IsInBkgComponentMap(_key)) MessageSvc::Error("IsInBkgComponentMap", _key, "not in map", "EXIT_FAILURE");
    return (*m_bkgComponents.find(_key)).second.fitComponent.Name();
    // return m_bkgComponents[_key].fitComponent.Name();
}

RooAbsPdf * FitHolder::BackgroundPDF(TString _key) {
    if (!IsInBkgComponentMap(_key)) MessageSvc::Error("IsInBkgComponentMap", _key, "not in map", "EXIT_FAILURE");
    return m_bkgComponents[_key].fitComponent.PDF();
}

//====================================================================================
//------------------------------- Interactions with TFile as TObject  ----------------
//====================================================================================
void FitHolder::SaveToDisk(TString _name, bool _verbose) {
    if (m_isLoaded) {
        MessageSvc::Warning("FitHolder", m_name, "LoadedFromDisk SKIPPING SaveToDisk");
        return;
    }
    // SaveToLog(_name);

    if (!SettingDef::IO::outDir.EndsWith("/")) SettingDef::IO::outDir += "/";
    if (_name != "") _name = "_" + _name;
    TString _oname = "FitHolder_" + m_name + _name;
    _name          = SettingDef::IO::outDir + _oname + ".root";

    MessageSvc::Line();
#ifdef STREAMDATA
    MessageSvc::Info(Color::Cyan, "FitHolder", m_name, "SaveToDisk", _name, "with STREAMDATA");
#else
    MessageSvc::Warning("FitHolder", m_name, "SaveToDisk", _name, "without STREAMDATA");
#endif
    PrintKeys();
    MessageSvc::Line();

    m_parameterSnapshot.ConfigureSnapshotMap();

    TFile _tFile(_name, to_string(OpenMode::RECREATE));
    (*this).Write(_oname, TObject::kOverwrite);
    if (_verbose) {
        TDirectory * _componentsSig  = _tFile.mkdir("FitComponent_Sig");
        TDirectory * _componentsBkg  = _tFile.mkdir("FitComponent_Bkg");
        TDirectory * _componentsData = _tFile.mkdir("FitComponent_Data");

        MessageSvc::Info("SaveToDisk", (TString) "Writing", m_sigComponentKey);
        _componentsSig->cd();
        m_sigComponent.fitComponent.Write(m_sigComponentKey, TObject::kOverwrite);
        for (const auto & _component : m_bkgComponents) {
            MessageSvc::Info("SaveToDisk", (TString) "Writing", _component.first);
            _componentsBkg->cd();
            _component.second.fitComponent.Write(_component.first, TObject::kOverwrite);
        }
        _componentsData->cd();
        m_data.Write("Data", TObject::kOverwrite);
    }
    _tFile.Close();
    cout << WHITE << *this << RESET << endl;
    return;
}

void FitHolder::LoadFromDisk(TString _name, TString _dir) {
    MessageSvc::Line();
#ifdef STREAMDATA
    MessageSvc::Info(Color::Cyan, "FitHolder", m_name, "LoadFromDisk", _name, _dir, "with STREAMDATA");
#else
    MessageSvc::Warning("FitHolder", m_name, "LoadFromDisk", _name, _dir, "without STREAMDATA");
#endif

    if (_name != "") _name = "_" + _name;
    _name = "FitHolder_" + m_name + _name;

    if ((_dir != "") && (!_dir.EndsWith("/"))) _dir += "/";

    if (!IOSvc::ExistFile(_dir + _name + ".root")) MessageSvc::Error("FitHolder", _dir + _name + ".root", "does not exist", "EXIT_FAILURE");

    TFile _tFile(_dir + _name + ".root", "read");
    MessageSvc::Line();
    _tFile.ls();
    MessageSvc::Line();

    FitHolder * _fh = (FitHolder *) _tFile.Get(_name);
    *this           = *_fh;

    PrintKeys();
    MessageSvc::Line();

    SetStatus(true, m_isReduced);

    m_parameterSnapshot.ReloadParameters();

    RefreshParameterPool();

    _tFile.Close();
    cout << WHITE << *this << RESET << endl;
    return;
}

void FitHolder::ReduceComponents(TCut _cut) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitHolder", m_name, "ReduceComponents", TString(_cut));
    MessageSvc::Line();

    if (!m_configuration.Var()) MessageSvc::Error("FitHolder", m_name, "Var is nullptr", "EXIT_FAILURE");

    PrintComponents();

    MessageSvc::Info("Signal", m_sigComponent.fitComponent.Name(), to_string(m_sigComponent.fitComponent.Type()));
    m_configuration.UseBinAndRange(SettingDef::Fit::varSchemeMC);
    m_sigComponent.fitComponent.ReduceComponent(_cut);

    if (m_sigComponent.fitComponent.DataSize() != 0) {
        MessageSvc::Line();
        MessageSvc::Info("ReducedSignal", SignalName());
        MessageSvc::Info("ReducedSignal", SignalPDF());
        MessageSvc::Info("ReducedSignal", SignalYield());
        MessageSvc::Line();
    }

    for (auto & _component : m_bkgComponents) {
        MessageSvc::Info("Background", _component.second.fitComponent.Name(), to_string(_component.second.fitComponent.Type()));
        if ((_component.second.fitComponent.Type() == PdfType::SignalCopy) || (_component.first == to_string(Sample::Bd)) || (_component.first == to_string(Sample::Bs))) {
            TString _key  = CleanString(_component.first);
            TString _name = m_name;
            if (_key != "") _name = _key + SettingDef::separator + _name;
            //Why copy and not assignment ? 
            //TODO : review ! 
            _component.second.fitComponent = FitComponent(m_sigComponent.fitComponent);
            _component.second.yield        = CreateYield(_name, "bkg_");
            _component.second.fitComponent.SetEventType(_component.second.fitComponent.GetEventType());
            _component.second.fitComponent.SetName(_name);
            _component.second.fitComponent.PDF()->SetName(_name);
            MessageSvc::Line();
            MessageSvc::Info("ReducedBackgroundFromSignal", BackgroundName(_component.first));
            MessageSvc::Info("ReducedBackgroundFromSignal", BackgroundPDF(_component.first));
            MessageSvc::Info("ReducedBackgroundFromSignal", BackgroundYield(_component.first));
        } else {
            m_configuration.UseBinAndRange(SettingDef::Fit::varSchemeMC);
            _component.second.fitComponent.ReduceComponent(_cut);
            if (_component.second.fitComponent.DataSize() != 0) {
                MessageSvc::Line();
                MessageSvc::Info("ReducedBackground", BackgroundName(_component.first));
                MessageSvc::Info("ReducedBackground", BackgroundPDF(_component.first));
                MessageSvc::Info("ReducedBackground", BackgroundYield(_component.first));
            }
        }
        MessageSvc::Line();
    }

    MessageSvc::Info("Data", m_data.Name());
    if (m_data.DataSize() == 0) MessageSvc::Error("ReducedData", (TString) "Empty data", "EXIT_FAILURE");
    m_configuration.UseBinAndRange(SettingDef::Fit::varSchemeCL);
    m_data.ReduceComponent(_cut);
    if (m_data.DataSize() != 0) {
        MessageSvc::Line();
        MessageSvc::Info("ReducedData", m_data.Name());
        MessageSvc::Info("ReducedData", m_data.DataSet());
        MessageSvc::Line();
    }

    SetStatus(m_isLoaded, true);

    cout << WHITE << *this << RESET << endl;
    return;
}

void FitHolder::SaveToLog(TString _name) {
    if (!SettingDef::IO::outDir.EndsWith("/")) SettingDef::IO::outDir += "/";
    if (_name != "") _name = "_" + _name;
    _name = SettingDef::IO::outDir + "FitHolder_" + m_name + _name + ".log";

    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitHolder", m_name, "SaveToLog", _name);

    ofstream _file(_name);
    if (!_file.is_open()) MessageSvc::Error("Unable to open file", _name, "EXIT_FAILURE");
    _file << *this << endl;
    _file.close();

    MessageSvc::Line();
    return;
}

//====================================================================================
//------------------------------- Initialize the Yield ranges ------------------------
//====================================================================================
// Necessary to have the correct ranges when fitting to data, by default the yields are
// not intitialized in the correct range
void FitHolder::InitRanges(bool _signalYields, bool _backgroundYields) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitHolder", m_name, "InitRanges");
    MessageSvc::Line();

    if ( ( (!m_isLoaded || m_isReduced) && !SettingDef::Weight::useBS) || SettingDef::Weight::useBS) {
        RooAbsData * _data = nullptr;
        if (m_data.DataSet() != nullptr) {
            _data = m_data.DataSet();
        } else if (m_data.DataHist() != nullptr) {
            _data = m_data.DataHist();
        }

        if (_data == nullptr) MessageSvc::Error("InitRanges", (TString) "DataSet is nullptr", "EXIT_FAILURE");

        MessageSvc::Info("InitRanges", _data, "v");

        Sample _signalSample       = m_configuration.SignalSample();
        double _dataEntries        = _data->sumEntries();
        double _max                = (1 + 6*TMath::Sqrt(_dataEntries) / _dataEntries) * _dataEntries;
        double _min                = m_option.Contains("posyield") ? 0 : -0.03 * _dataEntries;  //-3% of n-events is our low bound
        double _defaultSignalYield = _dataEntries / (double) (m_bkgComponents.size() + 2) * (double) (m_bkgComponents.size() + 1);
        if (SettingDef::Tuple::option == "cre"){
            _defaultSignalYield = _dataEntries;
            if (m_configuration.GetQ2bin() == Q2Bin::JPsi) {
                _defaultSignalYield *= 0.9;
            } else if (m_configuration.GetQ2bin() == Q2Bin::Psi) {
                _defaultSignalYield *= 0.9;
            } else {
                if (m_configuration.GetAna() == Analysis::MM) _defaultSignalYield *= 0.7;
                if (m_configuration.GetAna() == Analysis::EE) _defaultSignalYield *= 0.3;
            }
        }

        double _val = m_configuration.HasInitialFraction(_signalSample) ? _dataEntries * m_configuration.GetInitialFraction(_signalSample) : _defaultSignalYield;
	    //Justified such small initial error?
        //double _err = SettingDef::Fit::stepSizeYield * TMath::Sqrt(_val);
        double _err = TMath::Sqrt(_val);
        if (_signalYields) {
            if (m_sigComponent.yield) {
                MessageSvc::Info("InitRanges", (TString) "Signal", m_sigComponentKey);
                ((RooRealVar *) SignalYield())->setRange(_min, _max);
		if( _val < _min || _val > _max) MessageSvc::Error("Invalid configuration value, in min, max range");
                ((RooRealVar *) SignalYield())->setVal(_val);
                ((RooRealVar *) SignalYield())->setError(_err);
                ((RooRealVar *) SignalYield())->setConstant(0);
                ((RooRealVar *) SignalYield())->Print();
            } else {
                MessageSvc::Error("InitRanges", (TString) "Signal", m_sigComponentKey, "Yield is nullptr", "EXIT_FAILURE");
            }
        }
        double _defaultBackgroundYield = _dataEntries - _val;
        if (m_bkgComponents.size() != 0) { _defaultBackgroundYield /= (double) (m_bkgComponents.size() + 1); }
        if (_backgroundYields) {
            for (const auto & _component : m_bkgComponents) {
                auto _bkgKey = _component.first;
                if (BackgroundYield(_bkgKey)) {
                    MessageSvc::Info("InitRanges", (TString) "Background", _bkgKey);
                    auto _backgroundSample = hash_sample(_bkgKey);
                    _val                   = m_configuration.HasInitialFraction(_backgroundSample) ? _dataEntries * m_configuration.GetInitialFraction(_backgroundSample) : _defaultBackgroundYield;
                    // _err                   = SettingDef::Fit::stepSizeYield * TMath::Sqrt(_val);
                    _err                   = 0.10 * TMath::Sqrt(_val);
                    double _myMin = m_option.Contains("posyield") ? 0 :  -8 * abs(TMath::Sqrt(_dataEntries)); //Min Yield for background species is -3% of the dataset size or 0
                    ((RooRealVar *) BackgroundYield(_bkgKey))->setRange(_myMin, _max);
                    if( _val < _myMin || _val > _max) MessageSvc::Error("Invalid configuration value, in min, max range");
                    ((RooRealVar *) BackgroundYield(_bkgKey))->setVal(_val);
                    ((RooRealVar *) BackgroundYield(_bkgKey))->setError(_err);
                    ((RooRealVar *) BackgroundYield(_bkgKey))->setConstant(0);
                    ((RooRealVar *) BackgroundYield(_bkgKey))->Print();
                } else {
                    MessageSvc::Error("InitRanges", (TString) "Background", _component.first, "Yield is nullptr", "EXIT_FAILURE");
                }
            }
        }
    }
    return;
}
void FitHolder::BuildModel() {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitHolder", m_name, "BuildModel");
    MessageSvc::Line(); 
    // Create a list of pdfs and yields
    RooArgList _pdfs   = RooArgList("pdfs");
    RooArgList _yields = RooArgList("yields");

    // Deal with the signal component
    PrintSignalComponent();
    MessageSvc::Info(Color::Cyan, "FitHolder", m_name, "SetSignal", m_sigComponentKey);
    if (m_sigComponent.yield != nullptr) {
        MessageSvc::Info("SetSignal", SignalName());
        MessageSvc::Info("SetSignal", SignalPDF());
        MessageSvc::Info("SetSignal", SignalYield());
        TString _name = m_sigComponent.fitComponent.PDF()->GetName();
        if (!_name.BeginsWith("sig_")) m_sigComponent.fitComponent.PDF()->SetName(TString("sig_") + _name);
        _pdfs.add(*m_sigComponent.fitComponent.PDF());
        _yields.add(*m_sigComponent.yield);
    } else {
        MessageSvc::Error("FitHolder", m_name, "SignalYield is nullptr", "EXIT_FAILURE");
    }
    MessageSvc::Line();

    // Deal with the background components
    if (m_bkgComponents.size() != 0) PrintBackgroundComponents();
    for (const auto & _component : m_bkgComponents) {
        MessageSvc::Info(Color::Cyan, "FitHolder", m_name, "SetBackground", BackgroundName(_component.first));
        if (BackgroundYield(_component.first)) {
            MessageSvc::Info("SetBackground", BackgroundName(_component.first));
            MessageSvc::Info("SetBackground", BackgroundPDF(_component.first));
            MessageSvc::Info("SetBackground", BackgroundYield(_component.first));
            TString _name = _component.second.fitComponent.PDF()->GetName();
            if (!_name.BeginsWith("bkg_")) _component.second.fitComponent.PDF()->SetName(TString("bkg_") + _name);
            _pdfs.add(*_component.second.fitComponent.PDF());
            _yields.add(*_component.second.yield);
        } else {
            MessageSvc::Error("FitHolder", m_name, "BuildModel", "Background not defined", "EXIT_FAILURE");
        }
    }
    // We have collected the list of pdfs and yield, generate a Model
    CreateModel(_pdfs, _yields);
    PrintModelParameters();
    MessageSvc::Line();
    return;    
}



//====================================================================================
//------------------------------- Initialize the FitHolder ---------------------------
//====================================================================================
void FitHolder::Init() {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitHolder", m_name, "Initialize");
    MessageSvc::Line();

    if (!m_isLoaded || m_isReduced) {
        BuildModel();
            
        // Check dataset to fit exists and has been already defined
        if (m_data.DataSet() != nullptr) {
            MessageSvc::Info(Color::Cyan, "FitHolder", m_name, "SetData", m_data.Name());
            MessageSvc::Info("SetData", m_data.Name());
            MessageSvc::Info("SetData", m_data.DataSet());
        } else if (m_data.DataHist() != nullptr) {
            MessageSvc::Info(Color::Cyan, "FitHolder", m_name, "SetHist", m_data.Name());
            MessageSvc::Info("SetHist", m_data.Name());
            MessageSvc::Info("SetHist", m_data.DataHist());
        } else {
            MessageSvc::Error("FitHolder", m_name, "DataSet is nullptr", "EXIT_FAILURE");
        }
        MessageSvc::Line();
    }

    m_isInitialized = true;

    return;
}

void FitHolder::SetStatus(bool _isLoaded, bool _isReduced) {
    m_isLoaded  = _isLoaded;
    m_isReduced = _isReduced;
    m_sigComponent.fitComponent.SetStatus(m_isLoaded, m_isReduced);
    for (auto & _component : m_bkgComponents) { _component.second.fitComponent.SetStatus(m_isLoaded, m_isReduced); }
    return;
}

void FitHolder::RefreshParameterPool() {
    m_parameterPool = RXFitter::GetParameterPool();
    m_sigComponent.fitComponent.RefreshParameterPool();
    for (auto & _component : m_bkgComponents) { _component.second.fitComponent.RefreshParameterPool(); }
    return;
}

void FitHolder::CheckDataSize(Long64_t _entries, bool _error) {
    MessageSvc::Line();
    MessageSvc::Info("FitHolder", (TString) "CheckDataSize");
    MessageSvc::Line();

    map< TString, Long64_t > _samples;

    _samples[m_data.Name()] = m_data.DataSize();

    _samples[m_sigComponentKey] = m_sigComponent.fitComponent.DataSize();
    if ((m_sigComponent.fitComponent.Type() == PdfType::StringToPDF) || (m_sigComponent.fitComponent.Type() == PdfType::RooAbsPDF) || (m_sigComponent.fitComponent.Type() == PdfType::SignalCopy) || (m_sigComponent.fitComponent.Type() == PdfType::ToyPDF)) _samples[m_sigComponentKey] = _entries;

    for (auto & _component : m_bkgComponents) {
        _samples[_component.first] = _component.second.fitComponent.DataSize();
        if ((_component.second.fitComponent.Type() == PdfType::StringToPDF) || (_component.second.fitComponent.Type() == PdfType::RooAbsPDF) || (_component.second.fitComponent.Type() == PdfType::SignalCopy) || (_component.second.fitComponent.Type() == PdfType::ToyPDF)) _samples[_component.first] = _entries;
        if (_component.second.fitComponent.Option().Contains("add")) _samples[_component.first] = _entries;
    }

    MessageSvc::Line();
    MessageSvc::Info("FitHolder", (TString) "CheckDataSize");
    for (auto & _sample : _samples) { MessageSvc::Info(_sample.first, to_string(_sample.second), "entries"); }
    for (auto & _sample : _samples) {
        if (_sample.second < _entries) {
            if (SettingDef::trowLogicError || _error)
                MessageSvc::Error("Data has less than", to_string(_entries), "logic_error");
            else
                MessageSvc::Error("Data has less than", to_string(_entries), "EXIT_FAILURE");
        }
    }
    MessageSvc::Line();
    return;
}

void FitHolder::Close() {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitHolder", m_name, "Close ...");
    MessageSvc::Line();

    if (m_sigComponent.yield != nullptr)
        m_sigComponent.fitComponent.Close();
    else
        MessageSvc::Error("FitHolder", m_name, "SignalYield is nullptr");
    MessageSvc::Line();

    for (auto & _component : m_bkgComponents) {
        if (BackgroundYield(_component.first))
            _component.second.fitComponent.Close();
        else
            MessageSvc::Error("FitHolder", m_name, "BackgroundYield is nullptr");
    }
    MessageSvc::Line();

    if ((m_data.DataSet() != nullptr) || (m_data.DataHist() != nullptr))
        m_data.Close();
    else
        MessageSvc::Error("FitHolder", m_name, "DataSet is nullptr");
    MessageSvc::Line();

    if (m_fitter != nullptr) {
        m_fitter->Close();
        MessageSvc::Warning("FitHolder", m_name, "Delete FitterTool");
        delete m_fitter;
        m_fitter = nullptr;
    }

    if (m_model != nullptr) {
        MessageSvc::Warning("FitHolder", m_name, "Delete RooAddPdf");
        delete m_model;
        m_model = nullptr;
    }

    m_configuration.Close();

    return;
}

void FitHolder::CreateFitter() {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitHolder", m_name, "CreateFitter");
    MessageSvc::Line();

    if (!m_isInitialized) MessageSvc::Error("FitHolder", m_name, "Not initialized", "EXIT_FAILURE");

    m_configuration.UseBinAndRange(SettingDef::Fit::varSchemeCL);

    if (m_fitter != nullptr) {
        m_fitter->Close();
        delete m_fitter;
        m_fitter = nullptr;
    }

    m_fitter = new FitterTool("FitHolder_" + m_name, this);

    return;
}

void FitHolder::Fit() {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitHolder", m_name, "Fit");
    MessageSvc::Line();

    if (!m_fitter) MessageSvc::Error("FitHolder", m_name, "Fitter not available", "EXIT_FAILURE");

    m_fitter->Init();
    m_fitter->Fit();

    m_parameterPool->PrintParameters();

    return;
}

map< TString, pair< double, double > > FitHolder::GetIntegrals(double _min, double _max) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitHolder", m_name, "GetIntegrals", "Range [", to_string(_min), ",", to_string(_max), "]");
    MessageSvc::Line();

    map< TString, pair< double, double > > _map;
    pair< double, double >                 _total = make_pair(0, 0);

    pair< double, double > _pair = GetIntegral(m_sigComponentKey, _min, _max);
    _map[m_sigComponentKey]      = _pair;
    _total.first += _pair.first;

    for (auto & _component : m_bkgComponents) {
        _pair                  = GetIntegral(_component.first, _min, _max);
        _map[_component.first] = _pair;
        _total.first += _pair.first;
    }

    _map["Model"] = _total;

    MessageSvc::Info("GetIntegral", (TString) "Rangee = [", to_string(_min), ",", to_string(_max), "] Model");
    MessageSvc::Info("GetIntegral", (TString) "Integral =", to_string(_map["Model"].first), "+/-", to_string(_map["Model"].second));
    MessageSvc::Line();
    return _map;
}

pair< double, double > FitHolder::GetIntegral(TString _key, double _min, double _max) {
    MessageSvc::Line();
    MessageSvc::Info("GetIntegral", (TString) "Range = [", to_string(_min), ",", to_string(_max), "]", _key);

    if (!m_fitter) MessageSvc::Error("FitHolder", m_name, "Fitter not available", "EXIT_FAILURE");
    if (m_fitter->Results() == nullptr) MessageSvc::Error("GetIntegral", "fitter is nullptr", "EXIT_FAILURE");
    if (_min > _max) MessageSvc::Error("GetIntegral", to_string(_min), ">", to_string(_max), "EXIT_FAILURE");

    TString _rangeName = "GetIntegral";
    m_configuration.Var()->setRange(_rangeName, _min, _max);

    RooAbsReal * _yield = nullptr;
    RooAbsReal * _frac  = nullptr;
    if (_key == m_sigComponentKey) {
        _yield = SignalComponent().yield;
        _frac  = SignalComponent().fitComponent.PDF()->createIntegral(*m_configuration.Var(), NormSet(*m_configuration.Var()), Range(_rangeName));
    } else if (ContainsBackground(hash_sample(_key))) {
        _yield = BackgroundComponent(_key).yield;
        _frac  = BackgroundComponent(_key).fitComponent.PDF()->createIntegral(*m_configuration.Var(), NormSet(*m_configuration.Var()), Range(_rangeName));
    } else {
        MessageSvc::Error("GetIntegral", _key, "not available", "EXIT_FAILURE");
    }

    double _integral      = _yield->getVal() * _frac->getVal();
    double _integralError = 0;   // TO BE DONE

    MessageSvc::Info("GetIntegral", (TString) "Fraction =", to_string(_frac->getVal()));
    MessageSvc::Info("GetIntegral", (TString) "Integral =", to_string(_integral), "+/-", to_string(_integralError));
    MessageSvc::Line();
    return make_pair(_integral, _integralError);
}

pair< double, double > FitHolder::GetIntegralSignal(double _min, double _max) {
    MessageSvc::Line();
    MessageSvc::Info("GetIntegralSignal", (TString) "Range = [", to_string(_min), ",", to_string(_max), "]", m_sigComponentKey);

    map< TString, pair< double, double > > _map = GetIntegrals(_min, _max);

    MessageSvc::Info("GetIntegralSignal", (TString) "Integral =", to_string(_map[m_sigComponentKey].first), "+/-", to_string(_map[m_sigComponentKey].second));
    MessageSvc::Line();
    return _map[m_sigComponentKey];
}

pair< double, double > FitHolder::GetIntegralBackgrounds(double _min, double _max) {
    MessageSvc::Line();
    MessageSvc::Info("GetIntegralBackgrounds", (TString) "Range = [", to_string(_min), ",", to_string(_max), "]");

    map< TString, pair< double, double > > _map = GetIntegrals(_min, _max);

    double _integral      = 0;
    double _integralError = 0;
    for (auto & _component : m_bkgComponents) {
        MessageSvc::Info("GetIntegralBackgrounds", (TString) "Range = [", to_string(_min), ",", to_string(_max), "]", _component.first);
        _integral += _map[_component.first].first;
        _integralError += TMath::Sq(_map[_component.first].second);
    }
    _integralError = TMath::Sqrt(_integralError);

    MessageSvc::Info("GetIntegralBackgrounds", (TString) "Integral =", to_string(_integral), "+/-", to_string(_integralError));
    MessageSvc::Line();
    return make_pair(_integral, _integralError);
}

void FitHolder::DoSPlot() {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitHolder", m_name, "DoSPlot");
    MessageSvc::Line();

    if (!m_fitter) MessageSvc::Error("FitHolder", m_name, "Fitter not available", "EXIT_FAILURE");

    m_fitter->DoSPlot();

    return;
}

//====================================================================================
//------------------------------- The operator access --------------------------------
//====================================================================================
// Supposing you know the Keys you can extract the underneath objects directly from ["key"]
// as you do normally with a map except that this code doesn't support (yet) the assignment operation
FitComponent & FitHolder::operator[](const TString & _keyComponent) {
    if (m_sigComponentKey == _keyComponent) { return m_sigComponent.fitComponent; }
    if (m_bkgComponents.find(_keyComponent) != m_bkgComponents.end()) { return m_bkgComponents[_keyComponent].fitComponent; }
    MessageSvc::Error("FitHolder", m_name, "Cannot find FitComponent key", _keyComponent);
    cout << RED;
    PrintKeys();
    cout << RESET;
    MessageSvc::Error("FitHolder", m_name, "Cannot find FitComponent key", _keyComponent, "EXIT_FAILURE");
    // please the compiler
    auto dummy = new FitComponent();
    return *dummy;
}

//==============================================================================
//------------------------------- DEBUGGING METHODS and internal checks---------
//==============================================================================
bool FitHolder::IsSigKey() const noexcept {
    if (m_sigComponentKey == "") return false;
    return true;
}

bool FitHolder::IsInBkgComponentMap(TString _key) const noexcept {
    if (m_bkgComponents.find(_key) == m_bkgComponents.end()) return false;
    return true;
}

//---- Print on screen
void FitHolder::PrintKeys() const noexcept {
    cout << "\t SigComponent: " << m_sigComponent.fitComponent.Name() << RED << " (KEY = " << m_sigComponentKey << ")" << RESET << endl;
    if (m_sigComponent.yield != nullptr) {
        cout << "\t\t Yield     = [" << m_sigComponent.yield << "] : ";
        PrintVar(m_sigComponent.yield);
    } else {
        cout << "\t\t Yield     = [nullptr]" << endl;
    }
    m_sigComponent.fitComponent.PrintKeys();
    for (auto & _component : m_bkgComponents) {
        cout << "\t BkgComponent: " << _component.second.fitComponent.Name() << RED << " (KEY = " << _component.first << ")" << RESET << endl;
        if (_component.second.yield != nullptr) {
            cout << "\t\t Yield     = [" << _component.second.yield << "] : ";
            PrintVar(_component.second.yield);
        } else {
            cout << "\t\t Yield     = [nullptr]" << endl;
        }
        _component.second.fitComponent.PrintKeys();
    }
    cout << "\t DataComponent: " << m_data.Name() << RED << " (KEY = Data)" << RESET << endl;
    m_data.PrintKeys();
    return;
}

void FitHolder::PrintSignalComponent() const noexcept {
    MessageSvc::Info(Color::Cyan, "FitHolder", m_name, "PrintSignalComponent");
    if (m_sigComponent.fitComponent.PDF() != nullptr)
        MessageSvc::Info(m_sigComponent.fitComponent.Name(), (TString) m_sigComponent.fitComponent.PDF()->GetName());
    else
        MessageSvc::Warning("PrintSignalComponent", (TString) "SIGNAL COMPONENT NOT YET CREATED FOR THIS FITHOLDER !");
    if (m_sigComponent.yield != nullptr)
        MessageSvc::Info(m_sigComponent.fitComponent.Name(), m_sigComponent.yield);
    else
        MessageSvc::Warning("PrintSignalComponent", (TString) "SIGNAL COMPONENT NOT YET CREATED FOR THIS FITHOLDER !");
    return;
}

void FitHolder::PrintBackgroundComponents() const noexcept {
    MessageSvc::Info(Color::Cyan, "FitHolder", m_name, "PrintBackgroundComponents");
    for (const auto & _component : m_bkgComponents) {
        MessageSvc::Info(_component.second.fitComponent.Name(), (TString) _component.second.fitComponent.PDF()->GetName());
        MessageSvc::Info(_component.second.fitComponent.Name(), _component.second.yield);
    }
    return;
}

void FitHolder::PrintDataComponent() const noexcept {
    MessageSvc::Info(Color::Cyan, "FitHolder", m_name, "PrintDataComponent");
    if (m_data.DataSet() != nullptr)
        MessageSvc::Info(m_data.Name(), m_data.DataSet());
    else if (m_data.DataHist() != nullptr)
        MessageSvc::Info(m_data.Name(), m_data.DataHist());
    else
        MessageSvc::Warning("PrintDataComponent", (TString) "EMPTY DATASET !");
    return;
}

void FitHolder::PrintComponents() const noexcept {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitHolder", m_name, "PrintComponents");
    PrintSignalComponent();
    PrintBackgroundComponents();
    PrintDataComponent();
    MessageSvc::Line();
    return;
}

void FitHolder::PrintYields() const noexcept {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitHolder", m_name, "PrintYields");
    MessageSvc::Line();
    cout << " Yield SIG: " << flush;
    PrintVar(m_sigComponent.yield);
    cout << "\t SHAPE SIG " << m_sigComponent.fitComponent.Option() << endl;
    for (const auto & _component : m_bkgComponents) {
        cout << "--------------------------------" << endl;
        cout << " Yield BKG: " << flush;
        PrintVar(_component.second.yield);
        cout << "\t SHAPE BKG " << _component.second.fitComponent.Option() << endl;
    }
    MessageSvc::Line();
    return;
}

#endif
