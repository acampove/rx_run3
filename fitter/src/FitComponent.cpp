#ifndef FITCOMPONENT_CPP
#define FITCOMPONENT_CPP

#include "FitComponent.hpp"

#include "ConstDef.hpp"
#include "SettingDef.hpp"
#include "WeightDefRX.hpp"

#include "FitterSvc.hpp"
#include "RooNDKeysPdf2.h"
#include "FitterTool.hpp"
#include "TRegexp.h"

#include "vec_extends.h"

#include <functional>
#include "TKDE.h"
#include "RooTFnPdfBinding.h"
ClassImp(FitComponent)

    FitComponent::FitComponent(const EventType & _eventType, TString _name, TString _pdf, RooRealVar * _var, TString _option) {
    if (SettingDef::debug.Contains("FC")) SetDebug(true);
    m_eventType = _eventType;
    m_name      = _name;
    m_name      = CleanString(m_name);
    if (m_debug) MessageSvc::Debug("FitComponent", m_name, "TString");
    m_type   = hash_pdftype(_pdf);
    m_var    = _var;
    m_option = _option;
    Check();
    Init();
    cout << WHITE << *this << RESET << endl;
}

FitComponent::FitComponent(const EventType & _eventType, TString _name, PdfType _pdf, RooRealVar * _var, TString _option) {
    if (SettingDef::debug.Contains("FC")) SetDebug(true);
    m_eventType = _eventType;
    m_name      = _name;
    m_name      = CleanString(m_name);
    if (m_debug) MessageSvc::Debug("FitComponent", m_name, "PdfType");
    m_type   = _pdf;
    m_var    = _var;
    m_option = _option;
    Check();
    Init();
    cout << WHITE << *this << RESET << endl;
}

FitComponent::FitComponent(const EventType & _eventType, TString _name, RooAbsPdf * _pdf, RooRealVar * _var, TString _option) {
    if (SettingDef::debug.Contains("FC")) SetDebug(true);
    m_eventType = _eventType;
    m_name      = _name;
    if (m_debug) MessageSvc::Debug("FitComponent", m_name, "RooAbsPdf");
    if (_pdf != nullptr) {
        m_pdf.reset((RooAbsPdf *) _pdf->Clone(_name));
        m_shapeParameters = GetPars(m_pdf.get(), _var);

        TCanvas   _canvas("canvas");
        RooPlot * _frame = _var->frame(Title("RooAbsPDF for " + _name), Bins(_var->getBins()));
        m_pdf->plotOn(_frame);
        _frame->Draw();

        if (!SettingDef::IO::outDir.EndsWith("/")) SettingDef::IO::outDir += "/";
        _name = SettingDef::IO::outDir + "FitComponent_" + _name + "_RooAbsPDF";
        _canvas.Print(_name + ".pdf");
        if (m_saveAllExt) { _canvas.Print(_name + ".root"); }
    } else {
        MessageSvc::Error("FitComponent", m_name, "PDF is nullptr", "EXIT_FAILURE");
    }
    m_type   = PdfType::RooAbsPDF;
    m_var    = _var;
    m_option = _option;
    Check();
    CreateDatasetCachePath();
    m_parameterPool = RXFitter::GetParameterPool();
    cout << WHITE << *this << RESET << endl;
}

FitComponent::FitComponent(const EventType & _eventType, TString _name, RooAbsData * _data, TString _pdf, RooRealVar * _var, TString _option) {
    if (SettingDef::debug.Contains("FC")) SetDebug(true);
    m_eventType = _eventType;
    m_name      = _name;
    if (m_debug) MessageSvc::Debug("FitComponent", m_name, "RooAbsData");
    if (_data != nullptr) {
        if (m_debug) MessageSvc::Debug("FitComponent", m_name, _data->ClassName());
        if (((TString) _data->ClassName()) == "RooDataSet") m_data = make_shared<RooDataSet>(*dynamic_cast<RooDataSet*>(_data), m_name);
        if (((TString) _data->ClassName()) == "RooDataHist") m_hist = make_shared<RooDataHist>(*dynamic_cast<RooDataHist*>(_data), m_name);
    } else {
        MessageSvc::Error("FitComponent", m_name, "Data is nullptr", "EXIT_FAILURE");
    }
    m_type   = hash_pdftype(_pdf);
    m_var    = _var;
    m_option = _option;
    Check();
    m_parameterPool = RXFitter::GetParameterPool();
    cout << WHITE << *this << RESET << endl;
    if (m_data) MessageSvc::Info("FitComponent", m_data.get()); // Returns true if the shared_ptr does not contain a nullptr
    if (m_hist) MessageSvc::Info("FitComponent", m_hist.get()); // Returns true if the shared_ptr does not contain a nullptr
}

FitComponent::FitComponent(const EventType & _eventType, TString _name, RooAbsData * _data, PdfType _pdf, RooRealVar * _var, TString _option) {
    if (SettingDef::debug.Contains("FC")) SetDebug(true);
    m_eventType = _eventType;
    m_name      = _name;
    if (m_debug) MessageSvc::Debug("FitComponent", m_name, "RooAbsData");
    if (_data != nullptr) {
        if (m_debug) MessageSvc::Debug("FitComponent", m_name, _data->ClassName());
        if (((TString) _data->ClassName()) == "RooDataSet") m_data = make_shared<RooDataSet>(*dynamic_cast<RooDataSet*>(_data), m_name);
        if (((TString) _data->ClassName()) == "RooDataHist") m_hist = make_shared<RooDataHist>(*dynamic_cast<RooDataHist*>(_data), m_name);
    } else {
        MessageSvc::Error("FitComponent", m_name, "Data is nullptr", "EXIT_FAILURE");
    }
    m_type   = _pdf;
    m_var    = _var;
    m_option = _option;
    Check();
    m_parameterPool = RXFitter::GetParameterPool();
    cout << WHITE << *this << RESET << endl;
    if (m_data) MessageSvc::Info("FitComponent", m_data.get()); // Returns true if the shared_ptr does not contain a nullptr
    if (m_hist) MessageSvc::Info("FitComponent", m_hist.get()); 
}

FitComponent::FitComponent(const EventType & _eventType, TString _name, RooAbsData * _data, RooAbsPdf * _pdf, RooRealVar * _var, TString _option) {
    if (SettingDef::debug.Contains("FC")) SetDebug(true);
    m_eventType = _eventType;
    m_name      = _name;
    if (m_debug) MessageSvc::Debug("FitComponent", m_name, "RooAbsData RooAbsPdf");
    if (_data != nullptr) {
        if (m_debug) MessageSvc::Debug("FitComponent", m_name, _data->ClassName());
        if (((TString) _data->ClassName()) == "RooDataSet") m_data = make_shared<RooDataSet>(*dynamic_cast<RooDataSet*>(_data), m_name);
        if (((TString) _data->ClassName()) == "RooDataHist") m_hist = make_shared<RooDataHist>(*dynamic_cast<RooDataHist*>(_data), m_name);
    } else {
        MessageSvc::Error("FitComponent", m_name, "Data is nullptr", "EXIT_FAILURE");
    }
    if (_pdf != nullptr) {
        m_pdf.reset((RooAbsPdf *) _pdf->Clone(_name));
        m_shapeParameters = GetPars(m_pdf.get(), _var);

        TCanvas   _canvas("canvas");
        RooPlot * _frame = _var->frame(Title("RooAbsPDF for " + _name), Bins(_var->getBins()));
        m_pdf->plotOn(_frame);
        _frame->Draw();

        if (!SettingDef::IO::outDir.EndsWith("/")) SettingDef::IO::outDir += "/";
        _name = SettingDef::IO::outDir + "FitComponent_" + _name + "_RooAbsPDF";
        _canvas.Print(_name + ".pdf");
        if (m_saveAllExt) { _canvas.Print(_name + ".root"); }
    } else {
        MessageSvc::Error("FitComponent", m_name, "PDF is nullptr", "EXIT_FAILURE");
    }
    m_type   = PdfType::RooAbsPDF;
    m_var    = _var;
    m_option = _option;
    Check();
    m_parameterPool = RXFitter::GetParameterPool();
    cout << WHITE << *this << RESET << endl;
    if (m_data) MessageSvc::Info("FitComponent", m_data.get()); // Returns true if the shared_ptr does not contain a nullptr
    if (m_hist) MessageSvc::Info("FitComponent", m_hist.get());
    if (m_pdf) MessageSvc::Info("FitComponent", m_pdf.get());
}

FitComponent::FitComponent(const FitComponent & _fitComponent) {
    if (SettingDef::debug.Contains("FC")) SetDebug(true);
    m_name = _fitComponent.Name();
    if (m_debug) MessageSvc::Debug("FitComponent", m_name, "FitComponent");
    MessageSvc::Info("FitComponent", m_name, "Copy Constructor");

    m_eventType       = _fitComponent.GetEventType();
    m_type            = _fitComponent.Type();
    m_var             = _fitComponent.Var();
    m_option          = _fitComponent.Option();
    m_shapeParameters = _fitComponent.ShapeParameters();

    m_data = _fitComponent.m_data;
    m_hist = _fitComponent.m_hist;
    m_pdf = _fitComponent.m_pdf;
    if (m_pdf){
        m_shapeParameters = GetPars(m_pdf.get(), m_var);
    }

    m_binned       = _fitComponent.IsBinned();
    m_isModified   = _fitComponent.IsModified();
    m_isLoaded     = _fitComponent.IsLoaded();
    m_isReduced    = _fitComponent.IsReduced();
    m_ratioReduced = _fitComponent.RatioReduced();
    m_isAdded      = _fitComponent.IsAdded();
    m_datasetCache = _fitComponent.DataSetCache();

    CreateDatasetCachePath();
    m_parameterPool = RXFitter::GetParameterPool();
    // cout << WHITE << *this << RESET << endl;
}

FitComponent::FitComponent(TString _componentName, TString _option, TString _name, TString _dir) {
    if (SettingDef::debug.Contains("FC")) SetDebug(true);
    m_name = _componentName;
    m_name = CleanString(m_name);
    if (m_debug) MessageSvc::Debug("FitComponent", m_name, "LoadFromDisk");
    LoadFromDisk(_name, _dir);
    CreateDatasetCachePath();
    m_parameterPool = RXFitter::GetParameterPool();
    if (_option != m_option) {
        MessageSvc::Line();
        MessageSvc::Info(Color::Cyan, "FitComponent", m_name, "Resetting fit options");
        MessageSvc::Info("OLD", m_option);
        MessageSvc::Info("NEW", _option);
        MessageSvc::Line();
        m_option = _option;
        cout << WHITE << *this << RESET << endl;
    }
}

ostream & operator<<(ostream & os, const FitComponent & _fitComponent) {
    os << WHITE;
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "FitComponent");
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "name", _fitComponent.Name());
    MessageSvc::Print((ostream &) os, "type", to_string(_fitComponent.Type()));
    if (_fitComponent.Type() != PdfType::Empty) MessageSvc::Print((ostream &) os, "pars", to_string(_fitComponent.ShapeParameters().size()));
    MessageSvc::Print((ostream &) os, "var", _fitComponent.Var()->GetName());
    MessageSvc::Print((ostream &) os, "option", _fitComponent.Option());
    MessageSvc::Print((ostream &) os, "modified", to_string(_fitComponent.IsModified()));
    MessageSvc::Print((ostream &) os, "loaded", to_string(_fitComponent.IsLoaded()));
    MessageSvc::Print((ostream &) os, "reduced", to_string(_fitComponent.IsReduced()));
    MessageSvc::Print((ostream &) os, "added", to_string(_fitComponent.IsAdded()));
    // MessageSvc::Line(os);
    os << _fitComponent.GetEventType() << endl;
    os << RESET;
    os << "\033[F";
    return os;
}

bool FitComponent::Check() const noexcept {
    for (auto _opt : SettingDef::AllowedConf::FitComponents) {
        if (m_option.Contains(_opt)) return true;
    }
    cout << RED << *this << RESET << endl;
    MessageSvc::Error("FitComponent", m_name, "\"" + m_option + "\"", "option not in SettingDef::AllowedConf::FitComponents", "EXIT_FAILURE");
    return false;
}

void FitComponent::Init() {
    if (m_name.Contains(to_string(Sample::Custom))) { m_name.ReplaceAll(to_string(Sample::Custom), to_string(Sample::Custom) + SettingDef::separator + m_eventType.GetSample()); }
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitComponent", m_name, "Initialize ...");
    MessageSvc::Line();
    if (m_var == nullptr) MessageSvc::Error("FitComponent", m_name, "var is nullptr", "EXIT_FAILURE");
    if (!m_option.Contains("add")) {
        if ((m_type != PdfType::StringToPDF) && (m_type != PdfType::SignalCopy)) {            
            m_eventType.Init();                        
            CreateDatasetCachePath();
            if(  !SettingDef::Fit::useDatasetCache ||  //Either you don't use dataset caches, or you failed to make that because it was missing. 
                (SettingDef::Fit::useDatasetCache && !IOSvc::ExistFile(m_datasetCache) && (m_type != PdfType::Template) )){
                SettingDef::Tuple::ConfigureTupleLoading();
                MessageSvc::Warning("DataSet cache has failed to be loaded (not existing), We try to Reinitialize the EventType, eventType.GetTuple()==nullptr");
                m_eventType.Init(true, true);
                SettingDef::Tuple::ConfigureNoTupleLoading();
                if (! SettingDef::Weight::useBS && !m_eventType.GetTupleHolder().CheckVarInTuple(m_var->GetName())) {
                    if (SettingDef::Tuple::aliases) {
                        MessageSvc::Warning("FitComponent", m_name, "var", m_var->GetName(), "does not exists in Tuple, will try alias");
                    } else {
                        MessageSvc::Error("FitComponent", m_name, "var", m_var->GetName(), "does not exists in Tuple", "EXIT_FAILURE");
                    }
                }            
            }
        }
    }
    // if( m_type != PdfType::Template){
    if (((TString) m_eventType.GetCut()).Contains(m_var->GetName())) {
        MessageSvc::Warning("FitComponent", m_name, "var found in TCut");
        MessageSvc::Warning("FitComponent", m_var);
        MessageSvc::Warning("FitComponent", (TString) m_eventType.GetCut());
    }
    m_parameterPool = RXFitter::GetParameterPool();
    //
    return;
}

void FitComponent::SetStatus(bool _isLoaded, bool _isReduced) {
    m_isLoaded  = _isLoaded;
    m_isReduced = _isReduced;
    return;
}

void FitComponent::SetAdded(bool _isAdded) {
    m_isAdded = _isAdded;
    return;
}

void FitComponent::RefreshParameterPool() {
    m_parameterPool = RXFitter::GetParameterPool();
    return;
}

void FitComponent::Close(TString _option) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitComponent", m_name, "Close", _option, "...");

    if (m_fitter != nullptr) {
        m_fitter->Close();
        MessageSvc::Warning("FitComponent", (TString) "Delete FitterTool");
        delete m_fitter;
        m_fitter = nullptr;
    }
    // These are managed by shared_ptr now
    if (!_option.Contains("shape")) {
        if (m_pdf) {
            MessageSvc::Warning("FitComponent", (TString) "Delete RooAbsPdf");
            m_pdf.reset();
        }
    }
    if (!_option.Contains("data") && !_option.Contains("reduce") && !m_option.Contains("RooHistPDF")){
        if (m_data) {
            MessageSvc::Warning("FitComponent", (TString) "Delete RooDataSet");
            m_data.reset();
        }
        if (m_hist) {
            MessageSvc::Warning("FitComponent", (TString) "Delete RooDataHist");
            m_hist.reset();
        }
    }
    return;
}

void FitComponent::CreatePDF() {
    auto _start = chrono::high_resolution_clock::now();
    if (SettingDef::Fit::rareOnly && m_eventType.GetQ2bin() == Q2Bin::JPsi){
        m_type = PdfType::StringToPDF;
        m_option = "Exp-b[-3e-3,-5e-2,5e-2]";
    }
    if( SettingDef::Fit::loadFitComponentCaches && m_type == PdfType::StringToFit && !m_option.Contains("noCache")){
        m_shapeParameters.clear();
        bool loaded = LoadFitComponentCache();
        if( loaded ){           
            MessageSvc::Info("CreatePDF(LoadDisk)", m_pdf.get());
            PrintParameters(); 
            m_shapeParameters = GetPars(m_pdf.get(), m_var);
            PrintParameters(); 
            for (auto & _shapeParameter : m_shapeParameters) { m_parameterPool->AddShapeParameter((RooRealVar *) _shapeParameter.second); }        
            auto _stop = chrono::high_resolution_clock::now();
            MessageSvc::Warning("CreatePDF(LoadDisk)", (TString) "Took", to_string(chrono::duration_cast< chrono::seconds >(_stop - _start).count()), "seconds");
            MessageSvc::Line();
            return; 
        }else{
            MessageSvc::Warning("Failed to load from Disk, Proceed as usual, do the MC fits");
        }
    }else if(SettingDef::Fit::loadFitComponentCaches && m_type == PdfType::RooKeysPDF && !m_option.Contains("noCache") && !m_option.Contains("add")){ // && !m_option.Contai
        // Loading fit component caches for RooKeysPDF is disabled when we save the PDF into another workspace in order parts of the code execution
        bool loaded = LoadFitComponentCacheRooKeysPdf();
        if(loaded){
            auto _stop = chrono::high_resolution_clock::now();
            MessageSvc::Info("CreatePDF(LoadDisk)", m_pdf.get());
            MessageSvc::Warning("CreatePDF(LoadDisk)", (TString) "Took", to_string(chrono::duration_cast< chrono::seconds >(_stop - _start).count()), "seconds");
            MessageSvc::Line();
            return;
        }
        else{
            MessageSvc::Warning("Failed to load from Disk, Proceed as usual, recreate");
        }
    }
    m_shapeParameters.clear();
    MessageSvc::Info(Color::Cyan, "FitComponent", m_name, "CreatePDF", to_string(m_type));
    if (!m_option.Contains("add")) {
        switch (m_type) {
            case PdfType::StringToPDF: CreateStringToPDF(); break;
            case PdfType::StringToFit: CreateStringToFit(); break;
            case PdfType::RooHistPDF: CreateRooHistPDF(); break;
            case PdfType::RooKeysPDF: CreateRooKeysPDF(); break;
            case PdfType::Template :  CreatePdfFromTemplate(); break;
            default: MessageSvc::Error("CreatePDF", (TString) "PdfType", to_string(m_type), "not available", "EXIT_FAILURE"); break;
        }
    } else {
        AddPDFs();
    }
    MessageSvc::Info("CreatePDF", m_pdf.get());
    auto _stop = chrono::high_resolution_clock::now();
    MessageSvc::Warning("CreatePDF", (TString) "Took", to_string(chrono::duration_cast< chrono::seconds >(_stop - _start).count()), "seconds");
    MessageSvc::Line();

    for (auto & _shapeParameter : m_shapeParameters) { 
        std::cout<<  _shapeParameter.first << std::endl;
        _shapeParameter.second->Print("v");
        m_parameterPool->AddShapeParameter((RooRealVar *) _shapeParameter.second); 
    }        
    // if (m_option.Contains("conv") && !m_isAdded) { ConvPDF(); }
    if ( (m_type == PdfType::StringToFit || m_type == PdfType::RooKeysPDF) && SettingDef::Fit::saveFitComponentCaches && !m_option.Contains("noCache") && !m_option.Contains("add")){
        bool _saved = SaveFitComponentCache();
    }
    return;
}

Long64_t FitComponent::DataSize(TString _option) const {
    Long64_t _size = numeric_limits< Long64_t >::max();
    if (m_data){
        _size = TMath::Min(_size, (Long64_t) m_data->sumEntries());
    }
    else if (m_hist != nullptr){
        _size = TMath::Min(_size, (Long64_t) m_hist->sumEntries());
    }
    if (_option.Contains("cutRangeCL")){
        double pass_range = m_data->sumEntries(0, SettingDef::Fit::varSchemeCL);     
        MessageSvc::Warning("DataSize, RangeCL applied");

        std::cout<< RED<< "Old sumEntries() = "<< _size << std::endl; 
        std::cout<< RED<< "NEW sumEntries(cutRangeCL) = "<< pass_range << std::endl; 
        _size = pass_range;
    }
    if (_size == numeric_limits< Long64_t >::max()) _size = 0;
    if (_option.Contains("original")){
        MessageSvc::Warning("DataSize, scaled using m_ratioReduced", to_string(m_ratioReduced));
        _size *= m_ratioReduced;
    }
    return _size;
}

void FitComponent::SetData(RooAbsData * _data) {
    MessageSvc::Info(Color::Cyan, "FitComponent", m_name, "SetData", _data->ClassName());
    if (_data != nullptr) {
        if (((TString) _data->ClassName()) == "RooDataSet") m_data = make_shared<RooDataSet>(*dynamic_cast<RooDataSet*>(_data), m_name);
        if (((TString) _data->ClassName()) == "RooDataHist") m_hist = make_shared<RooDataHist>(*dynamic_cast<RooDataHist*>(_data), m_name);
    } else {
        MessageSvc::Warning("FitComponent", m_name, "Data is nullptr");
    }
    return;
}

void FitComponent::CreateStringToPDF() {
    MessageSvc::Info(Color::Cyan, "FitComponent", m_name, "CreateStringToPDF");
    if (!m_shapeParameters.empty()) {
        MessageSvc::Info("CreateStringToPDF", (TString) "Using shape parameters");
        PrintParameters();
        m_pdf.reset(StringToPdf(m_option.Data(), m_name.Data(), m_var, m_shapeParameters));
    } else {
        MessageSvc::Info("CreateStringToPDF", (TString) "Not using shape parameters");
        m_pdf.reset(StringToPdf(m_option.Data(), m_name.Data(), m_var, Str2VarMap()));
        m_shapeParameters = GetPars(m_pdf.get(), m_var);
    }    
    MessageSvc::Info("CreateStringToPDF", m_pdf.get());
    TCanvas   _canvas("canvas");
    RooPlot * _frame = m_var->frame(Title("StringToPDF for " + m_name), Bins(m_var->getBins()));
    m_pdf->plotOn(_frame);
    _frame->Draw();

    if (!SettingDef::IO::outDir.EndsWith("/")) SettingDef::IO::outDir += "/";
    TString _name = SettingDef::IO::outDir + "FitComponent_" + m_name + "_StringToPDF";
    _canvas.Print(_name + ".pdf");
    if (m_saveAllExt) { _canvas.Print(_name + ".root"); }

    return;
}

void FitComponent::CreateStringToFit(){
    MessageSvc::Info(Color::Cyan, "FitComponent", m_name, "CreateStringToFit");
    pair< double, double > _range = GetRange();
    m_var->setMin(_range.first);
    m_var->setMax(_range.second);
    m_pdf.reset(StringToPdf(m_option.Data(), m_name.Data(), m_var, m_shapeParameters));
    m_shapeParameters = GetPars(m_pdf.get(), m_var);
    PrintParameters();
    if (!(m_data) && !(m_hist) ){ 
        CreateData();
    }
    if (DataSize() == 0) {
        MessageSvc::Warning("CreateStringToFit", (TString) "Empty data", "SKIPPING");
        return;
    }

    MessageSvc::Line();
    MessageSvc::Info("CreateStringToFit", (TString) "CreateFitter");

    if (m_fitter != nullptr) {
        m_fitter->Close();
        delete m_fitter;
        m_fitter = nullptr;
    }

    if (m_option.Contains("exclude") || m_option.Contains("unbinned")){
        MessageSvc::Warning("Setting fit to unbinned");
        m_binned = false;
    }
    m_fitter = new FitterTool("FitComponent_" + m_name, this);
    m_fitter->Extended(kFALSE);
    m_fitter->Hesse(kTRUE);
    m_fitter->Minos(kFALSE);
    m_fitter->Offset(kTRUE);
    m_fitter->OptimizeConst(kFALSE);
    m_fitter->Save(kTRUE);
    m_fitter->StrategyMINUIT(1);
    m_fitter->ReFit(kFALSE);
    m_fitter->SumW2Error(kFALSE);
    if ((m_hist) && !(m_data)) {
        MessageSvc::Info("CreateStringToFit", (TString) "IsWeighted Hist", to_string(m_hist->isNonPoissonWeighted()));
        if (m_hist->isNonPoissonWeighted()) m_fitter->SumW2Error(kTRUE);
    }
    if (m_data) {
        MessageSvc::Info("CreateStringToFit", (TString) "IsWeighted Data", to_string(m_data->isNonPoissonWeighted()));
        if (m_data->isNonPoissonWeighted()) m_fitter->SumW2Error(kTRUE);
    }
    if(m_type == PdfType::Template){
        m_fitter->SumW2Error(kTRUE);
        m_fitter->Extended(kFALSE);
        //IF you want an extended term in the likelihood RooExtendPdf to be used...
        m_fitter->OptimizeConst(kTRUE);
        m_fitter->Offset(kTRUE);
        m_fitter->ReFit(kTRUE);    
        m_fitter->StrategyMINUIT(2);
        m_fitter->Hesse(kTRUE);
    }
    m_fitter->PrintLevel(1);
    m_fitter->PrintEvalErrors(0);
    m_fitter->Verbose(kFALSE);
    m_fitter->Warnings(kFALSE);

    if (m_option.Contains("exclude")) {
        pair< double, double > _excludedRange = GetExcludedRange();
        m_fitter->ExcludeRange(kTRUE);
        m_var->setRange("belowExcluded", _range.first, _excludedRange.first);
        m_var->setRange("aboveExcluded", _excludedRange.second,  _range.second);
        m_fitter->SetExcludedRange(TString("belowExcluded,aboveExcluded"));
    }
    m_fitter->FitTo();
    m_shapeParameters = GetPars(m_pdf.get(), m_var);
    PrintParameters();
    //delete the dataset!
    return;
}

void FitComponent::ModifyPDF(TString _varToModify, RooRealVar * _modifyingVar, TString _modifyOption) {
    MessageSvc::Info(Color::Cyan, "FitComponent", m_name, "ModifyPDF", _varToModify, _modifyOption);

    if (!IsVarInMap(_varToModify.Data(), m_shapeParameters)) {
        MessageSvc::Warning("ModifyPDF", _varToModify, "not present in current PDF");
        return;
    }

    if (_modifyingVar != nullptr || (_modifyingVar==nullptr && _modifyOption.Contains("offset") && !_modifyOption.Contains("shift"))) {        
        if( _modifyingVar  != nullptr){            
            //Nullptr accepted ONLY AND ONLY if offsetting only applied
            MessageSvc::Info("ModifyPDF", _varToModify, "modified by", _modifyingVar->GetName());
        }
        if( _modifyingVar == nullptr){
            MessageSvc::Info("ModifyPDF(nullptr)", (TString)"modified by option",_modifyOption);
        }
        if( _modifyOption.Contains("replace") ){             
            MessageSvc::Info("ModifyPDF", _varToModify, "replace with", _modifyingVar->GetName());
            m_shapeParameters[_varToModify.Data()] = _modifyingVar;
            m_pdf.reset(StringToPdf(m_option.Data(), m_name.Data(), m_var, m_shapeParameters, ""));
            m_isModified      = true;
        }else{
            m_shapeParameters = ModifyPars(&m_shapeParameters, _varToModify.Data(), _modifyingVar, _modifyOption.Data());
            m_pdf.reset(StringToPdf(m_option.Data(), m_name.Data(), m_var, m_shapeParameters, ""));
            m_isModified      = true;
        }
    } else {
        MessageSvc::Error("ModifyPDF", (TString) "Modifying var is nullptr");
        MessageSvc::Error("ModifyPDF", (TString) "Nothing to modify", _varToModify, _modifyOption, "EXIT_FAILURE");
    }
    return;
}

void FitComponent::CreateRooHistPDF() {
    MessageSvc::Info(Color::Cyan, "FitComponent", m_name, "CreateRooHistPDF");

    if ((!m_data) && (!m_hist)) CreateData();
    if (DataSize() == 0) {
        MessageSvc::Warning("CreateRooHistPDF", (TString) "Empty data", "SKIPPING");
        return;
    }

    m_pdf = make_shared<RooHistPdf>(m_name.Data(), m_name.Data(), RooArgSet(*m_var), *m_hist);
    MessageSvc::Info("CreateRooHistPDF", m_pdf.get());

    TCanvas   _canvas("canvas");
    RooPlot * _frame = m_var->frame(Title("RooHistPDF for " + m_name + (m_hist->isNonPoissonWeighted() ? " weighted" : " not weighted")), Bins(m_var->getBins()));
    m_pdf->plotOn(_frame);
    _frame->Draw();
    if (!SettingDef::IO::outDir.EndsWith("/")) SettingDef::IO::outDir += "/";
    TString _name = SettingDef::IO::outDir + "FitComponent_" + m_name + "_RooHistPDF";
    _canvas.Print(_name + ".pdf");
    if (m_saveAllExt) { _canvas.Print(_name + ".root"); }

    return;
}

void FitComponent::CreatePdfFromTemplate(){
    MessageSvc::Info(Color::Cyan, "FitComponent", m_name, "CreatePdfFromTemplate");
    TString _templateFilePath = StripStringBetween(m_option, "templatePath[", "]");
    TString _workspaceKey = "";
    bool isPDF = false; 
    if( m_option.Contains("workspaceKey")){
        isPDF = true;
        _workspaceKey     = StripStringBetween(m_option, "workspaceKey[", "]");
    }else if( m_option.Contains("histoKey")){
        isPDF = false;
        _workspaceKey     = StripStringBetween(m_option, "histoKey[", "]");
    }
    if (_templateFilePath == "" || _workspaceKey == ""){
        MessageSvc::Error("FitComponent", 
        (TString) "-templatePath[<path to template>]-workspaceKey[<key>] (or -histoKey[<key>]) option must be specified when using PdfType::Template", 
        "EXIT_FAILURE");
    }
    MessageSvc::Debug("Formatting (CreatePdfFromTemplate)", _templateFilePath);    
    _templateFilePath = TString(fmt::format(_templateFilePath.Data(), fmt::arg("run", to_string(m_eventType.GetYear()).Data())));
    //--- patch naming --//
    if( _workspaceKey.Contains( "{RUN}")){
        if( GetRunFromYear( m_eventType.GetYear()) == Year::Run2p1 || 
            GetRunFromYear( m_eventType.GetYear()) == Year::Run2p2 || 
            GetRunFromYear( m_eventType.GetYear()) == Year::Run2){ 
                _workspaceKey =    _workspaceKey.ReplaceAll("{RUN}", "Run2");
                _templateFilePath= _templateFilePath.ReplaceAll("{RUN}", "Run2");
        }
        if( GetRunFromYear( m_eventType.GetYear()) == Year::Run1){ 
            _workspaceKey = _workspaceKey.ReplaceAll("{RUN}", "Run1");
            _templateFilePath= _templateFilePath.ReplaceAll("{RUN}", "Run1");
        }
    }
    if( _workspaceKey.Contains("{BSIDX}") && SettingDef::Fit::IndexBootTemplateMisID >=0 ){
        MessageSvc::Debug("BSIDX filling, before", _workspaceKey);
        _workspaceKey = _workspaceKey.ReplaceAll( "{BSIDX}", TString::Format("%i",SettingDef::Fit::IndexBootTemplateMisID ) );
        MessageSvc::Debug("BSIDX filling, after ", _workspaceKey);
    }
    MessageSvc::Debug("Formatting (CreatePdfFromTemplate)", _templateFilePath);
    _templateFilePath = ExpandEnvironment(_templateFilePath);
    MessageSvc::Debug("Formatting (CreatePdfFromTemplate)", _workspaceKey);
    _workspaceKey     = TString(fmt::format(    _workspaceKey.Data(), fmt::arg("run", to_string(m_eventType.GetYear()).Data())));


    if( m_option.Contains("unifyRun2")){
        MessageSvc::Debug("UnifyRun2, pre", _templateFilePath);
        MessageSvc::Debug("UnifyRun2, pre", _workspaceKey);
        _templateFilePath = _templateFilePath.ReplaceAll("R2p2","R2");
        _workspaceKey     = _workspaceKey.ReplaceAll("R2p2","R2");
        _templateFilePath = _templateFilePath.ReplaceAll("R2p1","R2");
        _workspaceKey     = _workspaceKey.ReplaceAll("R2p1","R2");
        MessageSvc::Debug("UnifyRun2, after", _templateFilePath);
        MessageSvc::Debug("UnifyRun2, after", _workspaceKey);        
    }        

    if( _workspaceKey.Contains("{BSIDX}") && SettingDef::Fit::IndexBootTemplateMisID <0 ){
        MessageSvc::Debug("BSIDX filling Failure", _workspaceKey);
        MessageSvc::Error("Invalid, {BSIDX} in workspaceKey but SettingDef::Fit::IndexBootTemplateMisID not configured!");
    }
    if(isPDF){
        MessageSvc::Info("CreatePdfFromTemplate(LoadTemplate)", _templateFilePath, _workspaceKey);
        bool loaded = LoadRooKeysPDF(_templateFilePath, _workspaceKey);
        if(loaded){
            MessageSvc::Info("(CreatePdfFromTemplate) Successfully loaded template", m_pdf.get());
            MessageSvc::Line();
            TCanvas   _canvas("canvas");            
            RooPlot * _frame = m_var->frame(Title("RooTemplatePDF"), Bins(m_var->getBins()));
            std::cout<<"plotOn"<<std::endl;
            m_pdf->plotOn(_frame);
            _frame->Draw();
            if (!SettingDef::IO::outDir.EndsWith("/")) SettingDef::IO::outDir += "/";
            TString _name = SettingDef::IO::outDir + "FitComponent_" + m_name + "_RooKeysPDF";
            _canvas.Print(_name + ".pdf");
            if (m_saveAllExt) { _canvas.Print(_name + ".root"); }
            std::cout<<"done"<<std::endl;
            return;
        }
        else{
            MessageSvc::Error("FitComponent", (TString) "CreatePDF unable to load template", "EXIT_FAILURE");
        }
    }else{
        //small inlined function to make roodataset from histogram (finely binned!)
        auto  ConvertHistogramToRooDataSet=[](const TH1D & inputHistogram , RooRealVar & MassVar, RooRealVar & WeightVar , TString _name){
            RooDataSet * _data = new RooDataSet("dataSet_" + _name, "dataSet_" + _name,RooArgSet(MassVar,WeightVar), RooFit::WeightVar(WeightVar));                
            double Min = MassVar.getMin();
            double Max = MassVar.getMax();
            for( int i = 1;  i <= inputHistogram.GetNbinsX(); ++i){
                double binContent = inputHistogram.GetBinContent(i);
                double binCenter  = inputHistogram.GetBinCenter(i);
                if( binContent == 0.0 ) continue;                 
                if( binCenter < Min   ) continue;
                if( binCenter > Max   ) continue;
                WeightVar.setVal( binContent);
                MassVar.setVal( binCenter);
                //weight uncertainty should be drop in the filling
                _data->add( RooArgSet( MassVar   , WeightVar), binContent); //30% erorr on weight!
            }
            return _data;
        };
        MessageSvc::Info("CreatePDF(Fit 1D histogram, supplied PDF needed)", _templateFilePath, _workspaceKey);
        auto inFile = IOSvc::OpenFile( _templateFilePath, OpenMode::READ);
        auto HistoInput = inFile->Get<TH1D>(_workspaceKey);
        if(HistoInput == nullptr) MessageSvc::Error("Invalid Loading", _workspaceKey, "EXIT_FAILURE");        
        if( m_option.Contains("-rho")){
            double _rho = 1.1;
            GetValue< double >(m_option, TString("-rho"), _rho);
            MessageSvc::Debug("FitComponent", (TString)"Create RooKeyPdf from dataset");
            RooRealVar weightVar( "weight"+m_name, "weight"+m_name, 1.);
            m_data.reset( ConvertHistogramToRooDataSet( *HistoInput, *m_var, weightVar, m_name ));
            CreateRooKeysPDF();
        }else if( m_option.Contains("RooHistPDF")){
            MessageSvc::Debug("FitComponent" , (TString)"Create RooHistPDF from histogram templateMisID");
            double minOriginal    = m_var->getMin();
            double maxOriginal    = m_var->getMax();
            int    nBinsOriginal  = m_var->getBins();

            auto Key_KDE        = _workspaceKey + "_kde";
            auto Key_Rebinned   = _workspaceKey + "_rebinned";
            auto HistoKDE       = inFile->Get<TH1D>( Key_KDE);
            auto HistoRebinned  = inFile->Get<TH1D>( Key_Rebinned);
            if( HistoKDE == nullptr) MessageSvc::Error("Invalid Loading from file of (KDE hist)", Key_KDE, "EXIT_FAILURE");
            if( HistoRebinned == nullptr) MessageSvc::Error("Invalid Loading from file of (Rebinned hist)", Key_Rebinned, "EXIT_FAILURE");
    	    HistoKDE->SetDirectory(0);
    	    HistoRebinned->SetDirectory(0);
            m_var->setMin(HistoRebinned->GetXaxis()->GetXmin()); //<- ** hachking ranges ** > 
            m_var->setMax(HistoRebinned->GetXaxis()->GetXmax()); //<- ** hachking ranges ** > 
            MessageSvc::Debug("FitComponent", (TString)"Create Fit Shape from dataset");
            RooRealVar weightVar( "weight_"+m_name, "weight_"+m_name, 1.);	   
            m_data.reset( ConvertHistogramToRooDataSet( *HistoInput, *m_var, weightVar, m_name )); //with the original datase
            m_hist = make_shared<RooDataHist>( "dataHistForKDE_"+ m_name , "dataHistForKDE_"+m_name,  *m_var, HistoKDE, 1.0);
            bool _isWeighted = m_data->isNonPoissonWeighted();
            m_pdf = make_shared<RooHistPdf>(m_name.Data(), m_name.Data(), RooArgSet(*m_var), *m_hist);
            //---- Plotting PDF made out of the histPDF in sample ----- //
            TCanvas _canvas("canvas", "canvas", 1200, 900);
            TPad *  _plotPad = new TPad("plotPad", "", 0, .25, 1, 1);
            TPad *  _resPad  = new TPad("resPad", "", 0, 0, 1, .25);
            Plotting::ConfigurePadPlot(_plotPad);
            Plotting::ConfigurePullPlot(_resPad);
            _plotPad->Draw();
            _resPad->Draw();
            double _varMin = m_var->getMin(); //<- ** hachking ranges ** > 
            double _varMax = m_var->getMax(); //<- ** hachking ranges ** > 
            double _bin_width  =  32;        //Force bin width as in FitterTool to easily manipulate Plot ranges later on...
            int    nBinsPlot   =  std::floor( (_varMax - _varMin )/_bin_width);
            //Take Care of PlotPAD
            _plotPad->cd();
            RooPlot * _frame = m_var->frame(Title(" "), Bins(m_var->getBins()));
            Plotting::CustomizeRooPlot(_frame);
            TLegend *_legend = Plotting::GetLegend( 1 );
            TString _label_in_plot = "Tight/Fail weighted data";
            if (_isWeighted) m_data->plotOn(_frame, RooFit::DataError(RooAbsData::SumW2), RooFit::Name("data") );
            else             m_data->plotOn(_frame, RooFit::Name("data"));            
            m_pdf->plotOn(_frame, LineColor(kRed),  RooFit::Name("model"), LineWidth(5), Precision(1e-5));
            _legend->AddEntry(_frame->findObject("data"), _label_in_plot.Data() , "P");
            _legend->AddEntry(_frame->findObject("model"), "RooHistPDF from KDE", "L");
            RooPlot * _framePull = m_var->frame(Title("Pulls"));
            auto * dataHist = (RooHist *) _frame->getHist("data");       
            auto * curve = (RooCurve *) _frame->getObject(1);
            auto * _hres = dataHist->makePullHist(*curve, true);
            _hres->SetFillColor(kGray + 1);
            _hres->SetLineColor(kGray + 1);
            _hres->SetMarkerColor(kGray + 1);
            _hres->SetMarkerSize(0);
            _framePull->addPlotable(_hres, "BX");
            _resPad->cd();
            Plotting::CustomizeFramePull(_framePull);
            TAxis * _xAxis   = _framePull->GetXaxis();
            TLine * lc       = new TLine(_xAxis->GetXmin(), 0, _xAxis->GetXmax(), 0);
            TLine * lu       = new TLine(_xAxis->GetXmin(), 3, _xAxis->GetXmax(), 3);
            TLine * ld       = new TLine(_xAxis->GetXmin(), -3, _xAxis->GetXmax(), -3);
            lc->SetLineColor(kGray + 2);
            lu->SetLineColor(kGray + 1);
            ld->SetLineColor(kGray + 1);
            lc->SetLineStyle(2);
            lu->SetLineStyle(2);
            ld->SetLineStyle(2);
            lc->Draw("SAME");
            lu->Draw("SAME");
            ld->Draw("SAME");
            _plotPad->cd();
            _frame->GetYaxis()->CenterTitle(1);
            _frame->GetXaxis()->CenterTitle(1);
            _frame->Draw();
            _legend->Draw("SAME");	   
            if (!SettingDef::IO::outDir.EndsWith("/")) SettingDef::IO::outDir += "/";
    	    //_canvas.Print(_name + "pdf")
            TString _name = SettingDef::IO::outDir + "FitComponent_" + m_name + "_RooHistPDF";
    	    _canvas.Print(_name+".pdf");
            m_var->setMin(minOriginal);
            m_var->setMax(maxOriginal);
            m_var->setBins(nBinsOriginal);	    
        }else{
            //Will do a fit, we need to set bins and range 
            double minOriginal  = m_var->getMin();
            double maxOriginal  = m_var->getMax();
            int    nBins = m_var->getBins();
            if( m_option.Contains("-range[")){
                pair<double,double> _histo_re_range = GetRange();
                m_var->setMin(_histo_re_range.first);
                m_var->setMax(_histo_re_range.second);                      
            }else{
	            m_var->setMin(HistoInput->GetXaxis()->GetXmin());
	            m_var->setMax(HistoInput->GetXaxis()->GetXmax());
            }
            nBins = 32;
            if(m_option.Contains("-bins")) GetValue< int >(m_option, TString("-bins"), nBins);            
            m_var->setBins(nBins);
            MessageSvc::Debug("FitComponent", (TString)"Create Fit Shape from dataset");
            RooRealVar weightVar( "weight_"+m_name, "weight_"+m_name, 1.);
            m_data.reset( ConvertHistogramToRooDataSet( *HistoInput, *m_var, weightVar, m_name ));
            CreateStringToFit();
            m_var->setMin( minOriginal);
            m_var->setMax( maxOriginal);
            m_var->setBins(nBins);
        }
        IOSvc::CloseFile(inFile);
    }
    return;
}

void FitComponent::CreateRooKeysPDF() {
    MessageSvc::Info(Color::Cyan, "FitComponent", m_name, "CreateRooKeysPDF");

    double _min = m_var->getMin();
    double _max = m_var->getMax();

    pair< double, double > _range = GetRange();
    m_var->setMin(_range.first);
    m_var->setMax(_range.second);

    MessageSvc::Warning("CreateRooKeysPDF", (TString) "Change range");
    MessageSvc::Warning("CreateRooKeysPDF", (TString) "[" + to_string(_min) + "," + to_string(_max) + "]", "->", "[" + to_string(m_var->getMin()) + "," + to_string(m_var->getMax()) + "]");
    bool _isWeighted = false;
    if(m_data) {
      _isWeighted = m_data->isNonPoissonWeighted();
    } else if (m_hist) {
      MessageSvc::Error("CreateRooKeysPDF", (TString) "Cannot use RooDataHist", "EXIT_FAILURE");
    } else {
      CreateData();
      if (m_data) _isWeighted = m_data->isNonPoissonWeighted();
    }  
    if (DataSize() == 0) {
        MessageSvc::Warning("CreateRooKeysPDF", (TString) "Empty data", "SKIPPING");
        m_var->setMin(_min);
        m_var->setMax(_max);
        return;
    }
    double _rho = 1;
    if (m_option.Contains("-rho")) GetValue< double >(m_option, TString("rho"), _rho);

    MessageSvc::Info("CreateRooKeysPDF", m_data.get());
    MessageSvc::Info("CreateRooKeysPDF", (TString) "rho =", to_string(_rho));

    TString _pdfName = m_name + "[NM]";
    auto _mirrorOption = RooKeysPdf::NoMirror;
    if (m_option.Contains("MB")){
        _pdfName = m_name + "[MB]";
        _mirrorOption = RooKeysPdf::MirrorBoth;
    }
    if (m_option.Contains("NM")){
        _pdfName = m_name + "[NM]";
        _mirrorOption = RooKeysPdf::NoMirror;
    }

    MessageSvc::Info("CreateRooKeysPDF", m_data.get());
    MessageSvc::Info("CreateRooKeysPDF", (TString) "rho =", to_string(_rho));

    //should apply shift here i think!     
    if( m_option.Contains("-ShiftMassPar")){
        MessageSvc::Warning("(ShiftMassPar) Applying Mass Shift to RooKeyPdf at construction");
        //Retrieve from string the name of the parmameter in the middle of the option
        if( m_option.Contains("-ShiftMassPar[")){
            TString _parsedShiftKeyTAGS = StripStringBetween(m_option, "ShiftMassPar[", "]");
            MessageSvc::Warning("(ShiftMassPar) Mass Shift variable loaded TAG OPTION", _parsedShiftKeyTAGS );
            //Trick done here in order to Share the global shift variable
            if( _parsedShiftKeyTAGS.Contains("noTRG")) MessageSvc::Warning("The shift has noTRG tag  , IMPLEMENT IT, mass shift is the global one for the fit, string SENSITIVE!");
            if( _parsedShiftKeyTAGS.Contains("noANA")) MessageSvc::Warning("The shift has noANA tag  , IMPLEMENT IT, mass shift is the global one for the fit, string SENSITIVE!");
            if( _parsedShiftKeyTAGS.Contains("noPRJ")) MessageSvc::Warning("The shift has noPRJ tag  , IMPLEMENT IT, mass shift is the global one for the fit, string SENSITIVE!");
            if( _parsedShiftKeyTAGS.Contains("noQ2"))  MessageSvc::Warning("The shift has noQ2  tag  , IMPLEMENT IT, mass shift is the global one for the fit, string SENSITIVE!");
            if( _parsedShiftKeyTAGS.Contains("noYEAR"))  MessageSvc::Warning("The shift has noYEAR  tag  , IMPLEMENT IT, mass shift is the global one for the fit, string SENSITIVE!");
        }else{
            MessageSvc::Warning("(ShiftMassPar) RooKeyPdf will be shifted according to the FitHolder information it belongs to, inferred M-Shift global parameter from PDF name", m_name);
        }
        TString _KeyBegin = TokenizeString(m_name, "-").front();
        TString _KeyEnd   = TokenizeString(m_name, "-").back();
        TString _ID  = m_name;
        _ID = _ID.ReplaceAll( TString::Format("%s-",_KeyBegin.Data()) , "");
        _ID = _ID.ReplaceAll( TString::Format("-%s",_KeyEnd.Data())   , "");
        MessageSvc::Warning("(ShiftMassPar) For this RooKeyPdf will look up the m-shift parameter name after clearing up name of THIS PDF: ", _ID );
        //At this stage we should have something like RK-EE-jps-L0I-R2p2 
        //We can steer the noTRG stuff if we wreally want to via the option string
        //Let's check if ParameterPool CONTAINS the mass shift parameter to apply
        TString _KeyParShiftRooKeyPdf = TString::Format("%s_RooKeyPdfShift", _ID.Data());        

        if( m_option.Contains("-ShiftMassParBs")){
            _KeyParShiftRooKeyPdf+="_Bs";
            double bs_mass_shift =   PDG::Mass::Bs - PDG::Mass::Bd;
            MessageSvc::Warning("(ShiftMassPar) Applying Mass Shift to RooKeyPdf at construction for m(Bd)-m(Bs) !!!!! ");
            ShiftDataset(bs_mass_shift);
            m_pdf = make_shared<RooKeysPdf>(_pdfName, _pdfName, *m_var, *m_data, _mirrorOption, _rho);
            // int _nEntries = m_data->sumEntries();
            // std::cout<<" Entries = "<< _nEntries<<std::endl;
            // vector<double> _data;
            // for( int i = 0; i < _nEntries ; ++i){
            //     double _theVal = m_data->get(i)->getRealValue(m_var->GetName(), true);                
            //     double _shifted = bs_mass_shift + _theVal;                 
            //     _data.push_back(_shifted);
            // }
            // MessageSvc::Warning("Creating TKDE with data shifted with Bs-B0 mass range shifted (max not shifted)");
            // if(_isWeighted){
            //     MessageSvc::Error("Cannot make this specific RooKeyPdf with Weighted events, abort", "","EXIT_FAILURE");
            // }
            // TKDE * kde = new TKDE( _data.size(), & _data[0], m_var->getMin()+bs_mass_shift ,m_var->getMax(), 
            //                      "KernelType:Gaussian;Iteration:Adaptive;Mirror:NoMirror;Binning:RelaxedBinning",
            //                      0.08);//THIS RHO IS DANGEROUS (FORCED TO BE 0.08 to have a good tail description)
            // TF1 * f1  = new TF1("fKDE",kde,m_var->getMin()+bs_mass_shift ,m_var->getMax(),0);
            // TF1 * _F1 = kde->GetFunction(1000,m_var->getMin()+bs_mass_shift ,m_var->getMax() );
            // m_pdf = new RooTFnPdfBinding(_pdfName, _pdfName, f1, RooArgList(*m_var));
        }else{    
            TString _MassShiftSearch =  TString::Format("m_shift_%s", _ID.Data());
            if( ! m_parameterPool->ShapeParameterExists(_MassShiftSearch)){
                MessageSvc::Error("(ShiftMassPar) Trying to mass-shift a RooKeyPDfs creation, the mass shift to apply is not found!", _MassShiftSearch, "EXIT_FAILURE");
            }
            auto *theMassShift = m_parameterPool->GetShapeParameter( _MassShiftSearch);
            //Let's add a shifted parameter for THIS RooKeyPdf 
            //TODO, replace wildcars logic for <TRG>-<Q2BIN> etc...?
            if( ! m_parameterPool->ShapeParameterExists( _KeyParShiftRooKeyPdf)){
                MessageSvc::Warning(TString::Format("(ShiftMassPar) Retrieving from Parameter Pool the %s Failed, Adding the RooAddition to construct RooKey With a mass shift",_KeyParShiftRooKeyPdf.Data() ));
                // m_parameterPool->AddShapeParameter( new RooAddition(_KeyParShiftRooKeyPdf.Data(),_KeyParShiftRooKeyPdf.Data(),  RooArgList(*m_var, *theMassShift )));     
                m_parameterPool->AddShapeParameter( new RooFormulaVar(_KeyParShiftRooKeyPdf.Data(),"@0-@1",  RooArgList(*m_var, *theMassShift ))); //EXTREMELY IMPORTANT - sign, usually mshift applied to fitting parameters as a ```+shift```. Here we shift wrt the observable so the minus! 
                //WTF ? which shift sign to be compatible to what we do in data ? 
            }else{
                MessageSvc::Warning(TString::Format("(ShiftMassPar) Parameter Pool has already the variable for the mass shift of the RooKeyPdf named",_KeyParShiftRooKeyPdf.Data() ));
            }
            RooFormulaVar * shiftedX = (RooFormulaVar*) m_parameterPool->GetShapeParameter( _KeyParShiftRooKeyPdf);
            MessageSvc::Info("(ShiftMassPar) Creating RooKeyPdf with ShiftedX observable input");        
            m_pdf = make_shared<RooKeysPdf>(_pdfName, _pdfName, *shiftedX, *m_var, *m_data, _mirrorOption, _rho);
        }
    }else{
        m_pdf = make_shared<RooKeysPdf>(_pdfName, _pdfName, *m_var, *m_data, _mirrorOption, _rho);
    }
    MessageSvc::Info("CreateRooKeysPDF", m_pdf.get());        
    m_shapeParameters = GetPars(m_pdf.get(), m_var, m_option);
    TCanvas   _canvas("canvas", "canvas", 800, 600);
    _canvas.SetTopMargin(0.10);
    _canvas.SetRightMargin(0.15);
    _canvas.SetLeftMargin(0.15);
    _canvas.SetBottomMargin(0.2);

    RooPlot * _frame = m_var->frame(Title(" "), Bins(m_var->getBins()));
    _frame->GetXaxis()->SetLabelSize(0.08);
    _frame->GetXaxis()->SetTitleSize(0.08);
    _frame->GetXaxis()->SetTitleOffset(0.9);
    _frame->GetXaxis()->SetNdivisions(6, 5, 0);
    _frame->GetYaxis()->SetLabelSize(0.08);
    _frame->GetYaxis()->SetTitleSize(0.08);
    // _frame->GetYaxis()->SetTitleOffset(0.6);
    _frame->GetYaxis()->SetNdivisions(6, 5, 0);
    if (_isWeighted) m_data->plotOn(_frame, DataError(RooAbsData::SumW2));
    else m_data->plotOn(_frame);
    m_pdf->plotOn(_frame, LineColor(kRed));
    _frame->Draw();
    TLegend _legend(0.5, 0.6, 0.8, 0.85);
    _legend.SetBorderSize(0);
    _legend.SetFillColor(kWhite);
    _legend.SetLineColor(kWhite);
    _legend.SetFillStyle(0);
    _legend.AddEntry(_frame->findObject((TString) m_pdf->GetName() + "_Norm[" + m_var->GetName() + "]"), "RooKeysPDF", "L");
    _legend.Draw("SAME");

    if (!SettingDef::IO::outDir.EndsWith("/")) SettingDef::IO::outDir += "/";
    TString _name = SettingDef::IO::outDir + "FitComponent_" + m_name + "_RooKeysPDF";
    //If RooKeyPdf done in a range, plot the _ext.pdf file as well : which is the range over which the RooKeyPDF is actually defined 
    if (m_option.Contains("range")) _canvas.Print(_name + "_ext.pdf");

    MessageSvc::Warning("CreateRooKeysPDF", (TString) "Reset range");
    MessageSvc::Warning("CreateRooKeysPDF", (TString) "[" + to_string(m_var->getMin()) + "," + to_string(m_var->getMax()) + "]", "->", "[" + to_string(_min) + "," + to_string(_max) + "]");

    m_var->setMin(_min);
    m_var->setMax(_max);
    _frame->GetXaxis()->SetRangeUser(_min, _max);
    _frame->Draw();
    _legend.Draw("SAME");
    _canvas.Print(_name + ".pdf");
    if (m_saveAllExt){ _canvas.Print(_name + ".root"); }
    return;
}

void FitComponent::AddPDFs(TCut _cut) {
    TString _option = RemoveStringAfter(RemoveStringBefore(m_option, "add["), "]");
    _option.ReplaceAll("-add[", "").ReplaceAll("add[", "").ReplaceAll("]", "");
    MessageSvc::Line();
    MessageSvc::Info("AddPDFs", _option);

    vector< TString > _comps;
    vector<  double >  _fracs;

    if (!_option.Contains("+")) MessageSvc::Error("AddPDFs", m_name, "Cannot split (+)", _option, "EXIT_FAILURE");

    vector< TString > _split1 = TokenizeString(_option, "+");
    for (auto _it : _split1) {
        if (!_it.Contains("*")) MessageSvc::Error("AddPDFs", m_name, "Cannot split (*)", _it, "EXIT_FAILURE");

        vector< TString > _split2 = TokenizeString(_it, "*");                

        if( _it.Contains("<") && _it.Contains(">")){                   
            std::cout<< "SpecialParsing!"<<std::endl;
            TString _name   = _split2[0];
            TString _confs  = _split2[1];
            if( _name.Contains("<") && _name.Contains(">")){
                std::swap( _name, _confs);
            }
            TString _myComponentBase     = _name; 
            std::cout<< " My Component Base = "<< _myComponentBase << std::endl;
            TString _setupsSplittingCuts = StripStringBetween( _confs, "<", ">");
            std::cout<< " SetupSplittings   = "<< _setupsSplittingCuts << std::endl;
            vector<TString> _split3 = TokenizeString(_setupsSplittingCuts, ",");
            for( auto  _MyInternalConfig : _split3){
                std::cout<< " InternalConfigParsed   = "<< _MyInternalConfig << std::endl;
                TString _extraCutOption = TokenizeString(_MyInternalConfig, "=")[1];
                TString value           = TokenizeString(_MyInternalConfig, "=")[0];
                if( ! value.IsFloat() ){
                    std::cout<< "Value = "<< value << "  (extra CutOption = " << _extraCutOption << std::endl;
                    MessageSvc::Error("Something went wrong in parsin ghte special setup with <FRACTION=CUTOPTION>", "","EXIT_FAILURE");
                }
                std::cout<< "Value = "<< value << "  (extra CutOption = " << _extraCutOption << std::endl;
                TString _component_specialname= _name+"<"+_extraCutOption+">";
                _comps.push_back( _component_specialname);
                _fracs.push_back( value.Atof());
            }
        }else if(_split2[0].IsAlnum()) {
            _comps.push_back(_split2[0]);
            _fracs.push_back(_split2[1].Atof());
        }else if ( _split2[1].IsAlnum() ) {
            _comps.push_back(_split2[1]);
            _fracs.push_back(_split2[0].Atof());
        }else{
            MessageSvc::Error("Cannot parse, fix it");
        }
    }

    auto ClearSample = []( TString _sample){
        return RemoveStringBetween(_sample,"<", ">");
    };
    auto CutOptionSampleAdd = []( TString _sample){
        return StripStringBetween(_sample, "<", ">");
    };    
    MessageSvc::Info("AddPDFs", _comps);
    MessageSvc::Info("AddPDFs", _fracs);

    _option = RemoveStringBetween(m_option, "-add[", "]");
    RooArgList _pdfs;
    vector<TString> _names;
    for (size_t i = 0; i < _comps.size(); i++) {
        MessageSvc::Line();
        MessageSvc::Info("AddPDFs", _comps[i]);
        ZippedEventType _zip = m_eventType.Zip();
        auto _yearUse = GetYearForSample( ClearSample(_comps[i]), m_eventType.GetYear(), m_eventType.GetProject() );

        if( GetRunFromYear(_yearUse) != GetRunFromYear(m_eventType.GetYear())){        
            //When we switch Runs for the -comb option, we must ensure same MVA cut is applied, thus we force the MVA per run picking ( see CutHolders )
            MessageSvc::Warning("FitComponent AddPDFs", _comps[i], "YearUsed != Year FitComponent, forcing MVA cut to specific year" );
            switch(GetRunFromYear(m_eventType.GetYear())) {
                case Year::Run1   : _zip.cutOption+="-MVAR1"  ; break;
                case Year::Run2p1 : _zip.cutOption+="-MVAR2p1"; break;
                case Year::Run2p2 : _zip.cutOption+="-MVAR2p2"; break;                
                default : MessageSvc::Error("Invalid pick MVAR1/2p1/2p2 option","","EXIT_FAILURE");
            }
            MessageSvc::Warning("FitComponent AddPDFs cOpt used", _zip.cutOption );        
        }
        if( CutOptionSampleAdd(_comps[i]) != "") {
            MessageSvc::Warning("FitComponent AddPDFs", _comps[i], "Customized CutOption added" );
            _zip.cutOption+= TString::Format("-%s", CutOptionSampleAdd(_comps[i]).Data());
        }
        _zip.year            = _yearUse;
        _zip.sample = ClearSample(_comps[i]);
        EventType _et        = EventType(_zip, false);        
        TString _name = m_name + SettingDef::separator + _comps[i];
        _names.push_back( _comps[i]);

        FitComponent _ft(_et, _name, m_type, m_var, _option);
        _ft.SetAdded(true);
        // if (IsCut(_cut)) {
        //     _ft.ReduceComponent(_cut);
        // } else {
            _ft.CreatePDF();
        // }
        if (_ft.PDF() == nullptr) MessageSvc::Error("AddPDFs", _name, "PDF is nullptr", "EXIT_FAILURE");
        auto * _pdf = (RooAbsPdf*) _ft.PDF()->Clone();
        _pdfs.add(*_pdf);

        if (_ft.DataSet() == nullptr){
            _ft.CreateData();
        }

        if(_fracs[i] == 0 || _fracs[i] >=0. ){
            double _num = _ft.DataSize("original-cutRangeCL"); //IMPORTANT ! the eps must be in fit range ! 
            if (_num == 0){ 
                if( _ft.DataSize("original") != 0){
                    MessageSvc::Warning("In the fit range there is nothing left, will consider this fraction to be 0.");
                    _fracs[i] =0; 
                    continue;
                }else{
                    MessageSvc::Error("AddPDFs", _comps[i], "Invalid DataSize", "EXIT_FAILURE");
                }     
            }
            
            double _den = GetMCTEntries(_et.GetProject(), _et.GetYear(), _et.GetPolarity(), ClearSample(_comps[i]), false, "ngng_evt");
            if (_den == 0) MessageSvc::Error("AddPDFs", _comps[i], "Invalid GetMCTEntries", "EXIT_FAILURE");                    
            if( _fracs[i] != 0){
                _fracs[i] = _fracs[i] *  _num / _den;
            }else{
                _fracs[i] = _num/_den;
            }
            MessageSvc::Debug("AddPDFs", TString::Format( "Pass/Tot = %.1f/%.1f" , _num, _den));
            /*
            NOTE: We have different components for the cocktail making of the inclusive samples
            We want to merge 
              frac[0] * Bd2X 
            + frac[1] * Bu2X 
            + frac[2] * Bs2X 
            Therefore, each frac absorb an efficiency term computed as nPas/nTot[generated] [Assumes eps(generator the same among all )] and eps(filtering == 1)
            What we want to have is expected[0] * RooKey + expected[1] * RooKey + expected[2] * RooKey ...
            1. nPas / nTot = (frac(decays considered) / all decays ) * eps( decay considered) 
            2. expected[decay considered] = f_{x}/f_{d} *  [B.R. ( decay considered )]  * eps( decay considered) 
            3. expected[decay considered] = f_{x}/f_{d} *  [B.R. ( 1 decay in dec file )/frac( 1 decay in dec file) * frac( decay considered) ]  * eps( decay considered)       
            4. expected[decay considered] = f_{x}/f_{d} *  [B.R. ( 1 decay in dec file )/frac( 1 decay in dec file) * frac( decay considered) ]  * eps( decay considered) 
            5. expected[decay considered] = f_{x}/f_{d} *  [B.R. ( 1 decay in dec file )/frac( 1 decay in dec file) * nPas/nTot !           
            */
    
            auto _prj = _et.GetProject();
            if (_comps[i].BeginsWith("Bd2X")) {
                _fracs[i] /= PDG::DEC::fracBd2KstJPs;
                _fracs[i] *= PDG::BF::Bd2KstJPs;      
                MessageSvc::Debug("AddPDFs", _comps[i]);
                MessageSvc::Debug("AddPDFs(Bd2X)", TString::Format( "/=Frac (%.2f) percent" , 100*PDG::DEC::fracBd2KstJPs));
                MessageSvc::Debug("AddPDFs(Bd2X)", TString::Format( "*=BF   (%.2f) percent" , 100*PDG::BF::Bd2KstJPs));
            }
            if (_comps[i].BeginsWith("Bu2X")) {
                /*
                    Which sample to normalize agains the B+ => X J/Psi ? 
                    The K* -> K+ pi- branching ratios .... electrons/muons .. ? 
                */
                _fracs[i] /= PDG::DEC::fracBu2KJPs;
                _fracs[i] *= PDG::BF::Bu2KJPs;
                MessageSvc::Debug("AddPDFs", _comps[i]);
                MessageSvc::Debug("AddPDFs(Bu2X)", TString::Format( "/=Frac (%.2f) percent" ,   100*PDG::DEC::fracBu2KJPs));
                MessageSvc::Debug("AddPDFs(Bu2X)", TString::Format( "*=BF   (%.2f) percent" , 100*PDG::BF::Bu2KJPs));                
                // _fracs[i] /= PDG::DEC::fracBu2KJPs;
                // _fracs[i] *= PDG::BF::Bu2KJPs;
                // MessageSvc::Debug("AddPDFs", _comps[i]);
                // MessageSvc::Debug("AddPDFs(Bu2X)", TString::Format( "/=Frac (%.2f) percent" , 100*PDG::DEC::fracBu2KJPs));
                // MessageSvc::Debug("AddPDFs(Bu2X)", TString::Format( "*=BF   (%.2f) percent" , 100*PDG::BF::Bu2KstJPs));
            }
            if (_comps[i].BeginsWith("Bs2X")) {
                _fracs[i] /= PDG::DEC::fracBs2PhiJPs;
                _fracs[i] *= PDG::BF::Bs2PhiJPs;
                MessageSvc::Debug("AddPDFs", _comps[i]);
                MessageSvc::Debug("AddPDFs(Bs2X)", TString::Format( "/=Frac (%.2f) percent" ,   100*PDG::DEC::fracBs2PhiJPs));
                MessageSvc::Debug("AddPDFs(Bs2X)", TString::Format( "*=BF   (%.2f) percent" ,   100*PDG::BF::Bs2PhiJPs));
            }
            if (_comps[i].BeginsWith("Bs2KsKst")){
                MessageSvc::Debug("AddPDFs", _comps[i]);
                MessageSvc::Debug("AddPDFs(Bs2KsKst)", TString::Format( "/=Frac (%.2f) percent" ,   100*0.0427));
                MessageSvc::Debug("AddPDFs(Bs2KsKst)", TString::Format( "*=BF   (%.2f) percent" ,   100*0.92E-3 * 0.7));
                _fracs[i] /= 0.0427 ; //Efficiencies is 1/3 of what it should be, so we need a factor 3 at least! and an extra factor 2 for the KLong case (it's in the B.R already ? ).
                _fracs[i] *= 0.92E-3 * 0.7;//70 % of this B.R from Plot is in K*0 mass window
            }

            if (_comps[i].BeginsWith("Bs2X") || _comps[i].BeginsWith("Bs2KsKst")) {
                if ((m_eventType.GetYear() == Year::Y2011) || (m_eventType.GetYear() == Year::Y2012) || (m_eventType.GetYear() == Year::Run1)) {
                    _fracs[i] *= PDG::Const::fsOverfd7;
                    MessageSvc::Debug("AddPDFs(Bs2X)", TString::Format( "*= fsOverfd7 (%.2f) percent" ,   100*PDG::Const::fsOverfd7));                
                } else {
                    _fracs[i] *= PDG::Const::fsOverfd13;
                    MessageSvc::Debug("AddPDFs(Bs2X)", TString::Format( "*= fsOverfd13 (%.2f) percent" ,   100*PDG::Const::fsOverfd13));
                }
            }
        }
    }
    MessageSvc::Line();
    MessageSvc::Info("AddPDFs", _comps);
    MessageSvc::Info("AddPDFs", _fracs);

    double  _norm = 0;
    for (double _frac : _fracs) { _norm += _frac; }
    for( int i = 0; i < _fracs.size() ; ++i){ 
        _fracs[i]/=_norm;
    }
    MessageSvc::Info("AddPDFs normalized", _fracs);

    double  _sum =0.; 
    for( double  el : _fracs){
        _sum+=(double)el;
    }
    MessageSvc::Info("AddPDFs new sum", to_string(_sum));
    
    RooArgList _vfracs;
    RooArgList _vfracs_Free;
    for (size_t i = 0; i < _comps.size(); i++) {        
        if (i != _comps.size() - 1) {
            //---- mustbe sure this is done correctly , expecially when freeing them!             
            RooRealVar * _frac = new RooRealVar("frac" + m_name + SettingDef::separator + _comps[i], "frac" + m_name + SettingDef::separator + _comps[i], _fracs[i], 0, 1);
            _frac->setConstant(1);            
            _vfracs.add(*_frac);
        }
    }
    if (m_option.Contains("free2XPR")) {
        //if any recursive fraction would be needed, we must convert the parameters to a RooFormulaVar.
        if( _comps.size() == 3 ){
            RooRealVar * _frac0 = new RooRealVar("fracFree" + m_name + SettingDef::separator + _comps[0], "fracFree" + m_name + SettingDef::separator + _comps[0], _fracs[0], 0, 1);
            //currently a * A + b * B  + c * C
            //we want b--> ( 1-a )* bnew, for bnew 
            double frac1_new = _fracs[1]/(1.- _fracs[0]);
            RooRealVar * _frac1Start = new RooRealVar("fracStart" + m_name + SettingDef::separator + _comps[1], "fracStart" + m_name + SettingDef::separator + _comps[1], _fracs[frac1_new], 0, 1);
            RooFormulaVar * _frac1 = new RooFormulaVar( "fracRec" + m_name + SettingDef::separator + _comps[1] , "(1.-@0)*@1", RooArgList( *_frac0 ,  *_frac1Start) );            

            RooFormulaVar * _frac2 = new RooFormulaVar( "fracRec" + m_name + SettingDef::separator + _comps[2] , "(1.-@0 -@1)", RooArgList( *_frac0 ,  *_frac1) );            
            _vfracs_Free.add( *_frac0);
            _vfracs_Free.add( *_frac1);
            _vfracs_Free.add( *_frac2);
            MessageSvc::Warning("Free Part Reco fractions, forcing normalization to 1");
            m_pdf = make_shared<RooAddPdf>(m_name, m_name, _pdfs, _vfracs_Free) ; // RECURSIVE FRACTION ENABLED sum is not >1 ! 
            //Support only size = 3 [Bs + Bu + Bd]...            
        }else{
            m_pdf = make_shared<RooAddPdf>(m_name, m_name, _pdfs, _vfracs) ; // RECURSIVE FRACTION ENABLED sum is not >1 ! 
        }
    }
    else{
        m_pdf = make_shared<RooAddPdf>(m_name, m_name, _pdfs, _vfracs) ; // RECURSIVE FRACTION ENABLED sum is not >1 ! 
    }
    TCanvas   _canvas("canvas", "canvas", 800, 600);
    _canvas.SetTopMargin(0.10);
    _canvas.SetRightMargin(0.12);
    _canvas.SetLeftMargin(0.15);
    _canvas.SetBottomMargin(0.2);
    RooPlot * _frame = m_var->frame(Title(" "), Bins(m_var->getBins()));
    m_pdf->plotOn(_frame, LineColor(kBlack));
    TLegend _legend(0.55, 0.60, 0.85, 0.85);
    _legend.SetBorderSize(0);
    _legend.SetFillColor(kWhite);
    _legend.SetLineColor(kWhite);
    _legend.SetFillStyle(0);
    RooAddPdf *             _model   = dynamic_cast<RooAddPdf*>(m_pdf.get());
    RooArgList              _pdflist = _model->pdfList();
    const vector< Color_t > _colors  = {kRed, kBlue, kGreen, kMagenta, kCyan, kYellow, kGray + 2, kPink, kAzure, kSpring, kViolet, kTeal, kOrange};
    for (int i = 0; i < _pdflist.getSize(); ++i) {
        m_pdf->plotOn(_frame, Components(*_pdflist.at(i)), DrawOption("L"), LineColor(_colors[i]), LineStyle(kDashed), LineWidth(5), RooFit::Name(_pdflist.at(i)->GetName()), MoveToBack());
        TString _label = _pdflist.at(i)->GetName();
        if( _label.Contains( _comps[i])){
            _label = _comps[i];
            _legend.AddEntry(_frame->findObject(_pdflist.at(i)->GetName()), TString(_label), "L");
        }else{
            TRegexp _pattern("B.");
            _legend.AddEntry(_frame->findObject(_pdflist.at(i)->GetName()), TString(_label(_pattern)), "L");
        }

    }
    _frame->GetXaxis()->SetLabelSize(0.08);
    _frame->GetXaxis()->SetTitleSize(0.08);
    _frame->GetYaxis()->SetTitle("A.U.");
    _frame->GetXaxis()->SetTitleOffset(0.9);
    _frame->GetXaxis()->SetNdivisions(8, 5, 0);
    _frame->GetYaxis()->SetLabelSize(0.08);
    _frame->GetYaxis()->SetTitleSize(0.08);
    _frame->GetYaxis()->SetNdivisions(6, 5, 0);
    _frame->Draw("SAME");
    _legend.Draw("SAME");
    if (!SettingDef::IO::outDir.EndsWith("/")) SettingDef::IO::outDir += "/";
    TString _name = SettingDef::IO::outDir + "FitComponent_" + m_name + "_AddPDFs";
    _canvas.Print(_name + ".pdf");
    if (m_saveAllExt) { _canvas.Print(_name + ".root"); }

    return;
}

void FitComponent::AddDatasets() {
    TString _option = RemoveStringAfter(RemoveStringBefore(m_option, "datasets["), "]");
    _option.ReplaceAll("-datasets[", "").ReplaceAll("datasets[", "").ReplaceAll("]", "");
    MessageSvc::Line();
    MessageSvc::Info("AddDatasets", _option);

    vector< TString >      _comps;
    vector< TString >      _cutOptionExtras;
    vector< double >       _fracs;
    vector< RooDataSet * > _datasets;    
    if (!_option.Contains("+")) MessageSvc::Error("AddDatasets", m_name, "Cannot split (+)", _option, "EXIT_FAILURE");


    vector< TString > _split1 = TokenizeString(_option, "+");
    for (auto _it : _split1) {
        if (!_it.Contains("*")) MessageSvc::Error("AddDatasets", m_name, "Cannot split (*)", _it, "EXIT_FAILURE");
        vector< TString > _split2 = TokenizeString(_it, "*");
        if (_split2[0].IsAlnum()) {
            _comps.push_back(_split2[0]);
            _fracs.push_back(_split2[1].Atof());
        } else {
            _comps.push_back(_split2[1]);
            _fracs.push_back(_split2[0].Atof());
        }
    }

    MessageSvc::Info("AddDatasets", _comps);
    MessageSvc::Info("AddDatasets", _fracs);

    _option = RemoveStringBetween(m_option, "-datasets[", "]");

    RooArgList _pdfs;
    for (size_t i = 0; i < _comps.size(); i++) {
        MessageSvc::Line();
        MessageSvc::Info("AddDatasets", _comps[i]);        
        ZippedEventType _zip = m_eventType.Zip();
        
        auto _yearUse = GetYearForSample(_comps[i], m_eventType.GetYear(), m_eventType.GetProject());
        if( GetRunFromYear(_yearUse) != GetRunFromYear(m_eventType.GetYear())){
            MessageSvc::Warning("FitComponent AddPDFs", _comps[i], "YearUsed != Year FitComponent, forcing MVA cut to specific year" );
            switch(GetRunFromYear(m_eventType.GetYear())) {
                case Year::Run1   : _zip.cutOption+="-MVAR1"; break;
                case Year::Run2p1 : _zip.cutOption+="-MVAR2p1"; break;
                case Year::Run2p2 : _zip.cutOption+="-MVAR2p2"; break;                
                default : MessageSvc::Error("Invalid pick MVAR1/2p1/2p2 option","","EXIT_FAILURE");
            }
            MessageSvc::Warning("FitComponent AddPDFs cOpt used", _zip.cutOption );
        }
                
        _zip.year            = _yearUse;
        _zip.sample          = _comps[i];
        EventType _et        = EventType(_zip, false);

        TString _name = m_name + SettingDef::separator + _comps[i];

        FitComponent _ft(_et, _name, m_type, m_var, _option);
        _ft.CreateData();

        if (_ft.DataSet() == nullptr) MessageSvc::Error("AddDatasets", _name, "RooDataSet is nullptr", "EXIT_FAILURE");

        _datasets.push_back((RooDataSet *) _ft.DataSet()->Clone());

        if(_fracs[i] == 0 || _fracs[i] >=1 ){
            double _num = _ft.DataSize("original-cutRangeCL"); //IMPORTANT ! the eps must be in fit range ! 
            if (_num == 0){ 
                if( _ft.DataSize("original") != 0){
                    MessageSvc::Warning("In the fit range there is nothing left, will consider this fraction to be 0.");
                    _fracs[i] =0; 
                    continue;
                }else{
                    MessageSvc::Error("AddDatasets", _comps[i], "Invalid DataSize", "EXIT_FAILURE");
                }     
            }
            
            double _den = GetMCTEntries(_et.GetProject(), _et.GetYear(), _et.GetPolarity(), _comps[i], false, "ngng_evt");
            if (_den == 0) MessageSvc::Error("AddDatasets", _comps[i], "Invalid GetMCTEntries", "EXIT_FAILURE");                    
            if( _fracs[i] != 0){
                _fracs[i] = _fracs[i] *  _num / _den;
            }else{
                _fracs[i] = _num/_den;
            }
            MessageSvc::Debug("AddDatasets", TString::Format( "Pass/Tot = %.1f/%.1f" , _num, _den));
            /*
            NOTE: We have different components for the cocktail making of the inclusive samples
            We want to merge 
              frac[0] * Bd2X 
            + frac[1] * Bu2X 
            + frac[2] * Bs2X 
            Therefore, each frac absorb an efficiency term computed as nPas/nTot[generated] [Assumes eps(generator the same among all )] and eps(filtering == 1)
            What we want to have is expected[0] * RooKey + expected[1] * RooKey + expected[2] * RooKey ...
            1. nPas / nTot = (frac(decays considered) / all decays ) * eps( decay considered) 
            2. expected[decay considered] = f_{x}/f_{d} *  [B.R. ( decay considered )]  * eps( decay considered) 
            3. expected[decay considered] = f_{x}/f_{d} *  [B.R. ( 1 decay in dec file )/frac( 1 decay in dec file) * frac( decay considered) ]  * eps( decay considered)       
            4. expected[decay considered] = f_{x}/f_{d} *  [B.R. ( 1 decay in dec file )/frac( 1 decay in dec file) * frac( decay considered) ]  * eps( decay considered) 
            5. expected[decay considered] = f_{x}/f_{d} *  [B.R. ( 1 decay in dec file )/frac( 1 decay in dec file) * nPas/nTot !           
            */
                

            if (_comps[i].BeginsWith("Bd2X")) {        
                MessageSvc::Debug("AddDatasets", _comps[i]);
                _fracs[i] *= PDG::DEC::fracBd2KstJPs;
                _fracs[i] /= PDG::BF::Bd2KstJPs;
                MessageSvc::Debug("AddDatasets(Bd2X)", TString::Format( "/=Frac (%.2f) percent" , 100*PDG::DEC::fracBd2KstJPs));
                MessageSvc::Debug("AddDatasets(Bd2X)", TString::Format( "*=BF   (%.2f) percent" , 100*PDG::BF::Bd2KstJPs));       
            }
            if (_comps[i].BeginsWith("Bu2X")) {
                _fracs[i] *= PDG::DEC::fracBu2KJPs;
                _fracs[i] /= PDG::BF::Bu2KJPs;
                MessageSvc::Debug("AddDatasets", _comps[i]);
                MessageSvc::Debug("AddDatasets(Bu2X)", TString::Format( "/=Frac (%.2f) percent" , 100*PDG::DEC::fracBu2KJPs));
                MessageSvc::Debug("AddDatasets(Bu2X)", TString::Format( "*=BF   (%.2f) percent" , 100*PDG::BF::Bu2KJPs));                   
            }
            if (_comps[i].BeginsWith("Bs2X")) {
                _fracs[i] *= PDG::DEC::fracBs2PhiJPs;
                _fracs[i] /= PDG::BF::Bs2PhiJPs;
                MessageSvc::Debug("AddDatasets", _comps[i]);
                MessageSvc::Debug("AddDatasets(Bs2X)", TString::Format( "/=Frac (%.2f) percent" , 100*PDG::DEC::fracBs2PhiJPs));
                MessageSvc::Debug("AddDatasets(Bs2X)", TString::Format( "*=BF   (%.2f) percent" , 100*PDG::BF::Bs2PhiJPs));                     
            }
            if (_comps[i].BeginsWith("Bs2KsKst")){
                _fracs[i] /= 0.0427 ; //Efficiencies is 1/3 of what it should be, so we need a factor 3 at least! and an extra factor 2 for the KLong case (it's in the B.R already ! ).
                _fracs[i] *= 0.92E-3 * 0.7;//70 % of this B.R from Plot is in K*0 mass window
            }

            if (_comps[i].BeginsWith("Bs2X") || _comps[i].BeginsWith("Bs2KsKst")){
                if ((m_eventType.GetYear() == Year::Y2011) || (m_eventType.GetYear() == Year::Y2012) || (m_eventType.GetYear() == Year::Run1)) {
                    _fracs[i] *= PDG::Const::fsOverfd7;
                    MessageSvc::Debug("AddDatasets(Bs2X)", TString::Format( "*= fsOverfd7 (%.2f) percent" ,   100*PDG::Const::fsOverfd7));                
                } else {
                    _fracs[i] *= PDG::Const::fsOverfd13;
                    MessageSvc::Debug("AddDatasets(Bs2X)", TString::Format( "*= fsOverfd13 (%.2f) percent" ,   100*PDG::Const::fsOverfd13));
                }
            }
        }
        // Deletes RooDataSet and RooDataHist to avoid memory leaks.
        _ft.Close();
    }

    MessageSvc::Line();
    MessageSvc::Info("AddDatasets", _comps);
    MessageSvc::Info("AddDatasets", _fracs);

    double _norm = 0;
    for (auto & _frac : _fracs) { _norm += _frac; }
    for( int i = 0; i < _fracs.size(); ++i){
        _fracs[i]/=_norm;
    }
    for (auto & _frac : _fracs) { _frac /= _norm; }
    RooRealVar * _varWeight = new RooRealVar("weight", "weight", 1.);
    m_data = make_shared<RooDataSet>("dataSet_" + m_name, "dataSet_" + m_name, RooArgList(*m_var), _varWeight->GetName());

    for (size_t i = 0; i < _comps.size(); i++) {
        for (size_t j = 0; j < _datasets[i]->numEntries(); j++) {
            bool _isWeighted = _datasets[i]->isNonPoissonWeighted();
            // We need to clone the dataset to add a weight
            auto argset = (RooArgSet *) _datasets[i]->get(j)->Clone();
            argset->add(*_varWeight);
            // weight the datapoint with the fraction            
            double _weight = _isWeighted ? _fracs[i] * _datasets[i]->weight() : _fracs[i];
            m_data->add(*argset, _weight);
        }
    }

    return;
}

void FitComponent::ConvPDF(RooRealVar * _mean, RooRealVar * _sigma, double _order) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitComponent", m_name, "ConvPDF");

    TString _option = StripStringBetween(m_option, "conv[", "]");
    _option.ReplaceAll("-conv[", "").ReplaceAll("conv[", "").ReplaceAll("]", "");
    MessageSvc::Info("ConvPDF", _option);

    MessageSvc::Info("ConvPDF", m_pdf.get());
    m_pdf->SetName(m_name + "_org");

    if (_mean == nullptr) {
        _mean = new RooRealVar("c_mean_" + m_name, "c_mean_" + m_name, 0, -50, 50);
        m_parameterPool->AddShapeParameter(_mean);
        if (_option.Contains("mean"))
            _mean->setConstant(0);
        else
            _mean->setConstant(1);
    }

    if (_sigma == nullptr) {
        _sigma = new RooRealVar("c_sigma_" + m_name, "c_sigma_" + m_name, 10, 0, 100);
        m_parameterPool->AddShapeParameter(_sigma);
        if (_option.Contains("sigma"))
            _sigma->setConstant(0);
        else
            _sigma->setConstant(1);
    }

    _option.ReplaceAll("mean", "").ReplaceAll("sigma", "").ReplaceAll(",", "").ReplaceAll(" ", "");
    if ((_option != "") && _option.IsFloat()) _order = _option.IsFloat();

    MessageSvc::Info("Mean", _mean);
    MessageSvc::Info("Sigma", _sigma);
    MessageSvc::Info("Order", to_string(_order));

    RooGaussian * _gauss = new RooGaussian(m_name + "_gauss", m_name + "_gauss", *m_var, *_mean, *_sigma);
    MessageSvc::Info("ConvPDF", _gauss);

    if (m_option.Contains("convfft"))
        m_pdf = make_shared<RooFFTConvPdf>(m_name + "_cgauss", m_name + "_cgauss", *m_var, *m_pdf, *_gauss, _order);
    else if (m_option.Contains("convnum"))
        m_pdf = make_shared<RooNumConvPdf>(m_name + "_cgauss", m_name + "_cgauss", *m_var, *m_pdf, *_gauss);
    else
        m_pdf = make_shared<RooFFTConvPdf>(m_name + "_cgauss", m_name + "_cgauss", *m_var, *m_pdf, *_gauss, _order);
    if (!m_pdf) MessageSvc::Warning("FitComponent", m_name, "ConvPDF is nullptr", "EXIT_FAILURE");

    return;
}

void FitComponent::CreateData(){
    auto _start = chrono::high_resolution_clock::now();    
    double _maxEvt = SettingDef::Tuple::frac;
    switch (m_type) {
        case PdfType::StringToFit: _maxEvt = 1e6; break;
        case PdfType::RooHistPDF:  _maxEvt = 1e6; break;
        case PdfType::RooKeysPDF:  _maxEvt = ( SettingDef::Fit::reduceRooKeysPDF || m_option.Contains("reduceRooKeyPdf") ) ? 80e3 : SettingDef::Tuple::frac; break;
        default: break;
    }
    if ((SettingDef::Tuple::frac != -1) && (SettingDef::Tuple::frac < _maxEvt)) _maxEvt = SettingDef::Tuple::frac;
    TString _head = "";
    bool _isDataFitWithSmear = m_eventType.GetSample() == "LPT" && TString(m_var->GetName()).Contains("JPs_M_smear");
    if (_isDataFitWithSmear ) { 
        /*
            The JPs_M_smear_B0/Bp in data if is the fitted variable is simply a placeholder to JPs_M
            Needed for the smearing fit-back check
        */
        _head += TString(m_var->GetName()).ReplaceAll("JPs_M_smear_","");
        // TODO: v11 when attaching the correctly defined one (wMC will be dropped ) @renato
        // m_eventType.GetTuple()->SetAlias("JPs_M_smear_B0","JPs_M");
        // m_eventType.GetTuple()->SetAlias("JPs_M_smear_Bp","JPs_M");
        // m_eventType.GetTuple()->SetAlias("JPs_M_smear_B0_fromMCDT","JPs_M");
        // m_eventType.GetTuple()->SetAlias("JPs_M_smear_Bp_fromMCDT","JPs_M");
        m_eventType.GetTuple()->SetAlias("JPs_M_smear_B0_wMC","JPs_M");
        m_eventType.GetTuple()->SetAlias("JPs_M_smear_Bp_wMC","JPs_M");
        m_eventType.GetTuple()->SetAlias("JPs_M_smear_B0_fromMCDT_wMC","JPs_M");
        m_eventType.GetTuple()->SetAlias("JPs_M_smear_Bp_fromMCDT_wMC","JPs_M");
        SettingDef::Tuple::aliases = true;
    }
    bool _FOUNDANDLOADED_ = false; 
    if( !SettingDef::Fit::redoDatasetCache && SettingDef::Fit::useDatasetCache && m_data == nullptr && !m_option.Contains("noCache")){
        LoadDatasetCache();
        if(m_data) _FOUNDANDLOADED_ = true;
    }
    if(!m_data){    
        MessageSvc::Info(Color::Cyan, "FitComponent", m_name, "CreateData", to_string(_maxEvt));
        if( ( m_eventType.IsLeakageSample() && m_option.Contains("leakage") )      || 
            ( m_eventType.IsSignalMC()  )                                          ||
            ( m_eventType.IsMC()            && m_option.Contains("reweightXJPs") ) || 
            ( m_eventType.IsMC()            && m_eventType.IsWeighted() ) ) {
            MessageSvc::Warning("CreateData Leakage (or weighted one). Maybe the cached one has been loaded, be sure it picks the correct one!!!!");
        }
    }

    if ((!m_data) && m_option.Contains("datasets")) { AddDatasets(); }

    if (!m_data) {
        RooArgList _varList("varList_" + m_name);
        _varList.add(*m_var);
        
        TCut _rangeCut = TCut(Form((TString) "(%f < " + m_var->GetName() + " && " + m_var->GetName() + " < %f)", m_var->getMin(), m_var->getMax()));
        if (_isDataFitWithSmear){ 
            TString _myVar =  "JPs_M";//in data it's this variable to be used on the range cut.       
            _rangeCut = TCut(Form((TString) "(%f < " + _myVar + " && " + _myVar + " < %f)", m_var->getMin(), m_var->getMax()));
        }

        if (m_eventType.IsCutted() && (SettingDef::Tuple::frac == -1) && !m_option.Contains("rescale")) m_option += "-rescale";        

        MessageSvc::Info("CreateData", &_rangeCut);

        if (m_eventType.IsLeakageSample() && (m_option.Contains("leakage"))){
            MessageSvc::Warning("leakage option enabled for DataSetMaking... Remove range cut, leakage option does the cut automatically, min max inferred");
            _rangeCut =  TCut(NOCUT);
            MessageSvc::Info("CreateData (UPDATE)", &_rangeCut);
        }
        if( m_eventType.GetTuple()==nullptr){
            //
            // We tried to load the dataset caches, but it didn't manage to, we have to LOAD the ntuple properly if not already done, reinitialize the EventType and their ntuples
            //
            MessageSvc::Warning("DataSet cache has failed to be loaded, We try to Reinitialize the EventType, eventType.GetTuple()==nullptr");
            SettingDef::Tuple::ConfigureTupleLoading();
            m_eventType.Init(true, true);
            SettingDef::Tuple::ConfigureNoTupleLoading();
        }
        m_data.reset(m_eventType.GetDataSet(m_name, _varList, _rangeCut, _maxEvt, m_option));
        if( !_FOUNDANDLOADED_ &&  SettingDef::Fit::useDatasetCache && !m_option.Contains("noCache") ){
            MessageSvc::Warning("Failed to load, remaking and saving dataset Cache, next time you will be faster...");
            SaveDatasetCache();
        }
    }
    if (!m_data) {
        MessageSvc::Warning("FitComponent", m_name, "Data is nullptr");
    } else {
        if (SettingDef::Fit::redoDatasetCache){ 
            SaveDatasetCache();       
        }
        if (m_type == PdfType::RooKeysPDF) { ReduceData(_maxEvt, true); }
        MessageSvc::Info("CreateData", m_data.get());
        //the RooDataHist creation here knows ANYTHING ABOUT WEIGHTS!!!! FUCK! 
        m_hist = make_shared<RooDataHist>("dataHist_" + m_name, "dataHist_" + m_name, RooArgSet(*m_var), *m_data);
        MessageSvc::Info("CreateData", m_hist.get());
    }

    if (DataSize() == 0) MessageSvc::Warning("CreateData", (TString) "Empty data");

    auto _stop = chrono::high_resolution_clock::now();
    MessageSvc::Warning("CreateData", (TString) "Took", to_string(chrono::duration_cast< chrono::seconds >(_stop - _start).count()), "seconds");

    /*
    TCanvas _canvas("canvas");
    RooPlot * _frame = m_var->frame(Title("CreateData for " + m_name), Bins(m_var->getBins()));
    m_data->plotOn(_frame);
    _frame->Draw();

    if (! SettingDef::IO::outDir.EndsWith("/")) SettingDef::IO::outDir += "/";
    TString _name = SettingDef::IO::outDir + "FitComponent_" + m_name + "_CreateData";
    _canvas.Print(_name + ".pdf");
    if (m_saveAllExt) {
        _canvas.Print(_name + ".root");
    }
    */
    return;
}

void FitComponent::ReduceData(Long64_t _maxEvt, bool _random) {

    if (m_data->isNonPoissonWeighted()) {
        MessageSvc::Warning("ReduceData", (TString) "Nothing to reduce (weighted RooDataSet)");
        return;
    }

    Long64_t _size = DataSize();

    if ((_size < _maxEvt) || (_maxEvt == -1)) {
        MessageSvc::Warning("ReduceData", (TString) "Nothing to reduce (", to_string(_size), "->", to_string(_maxEvt), ")");
        return;
    }

    double _ratio = _size / (double) _maxEvt;
    if (m_ratioReduced != 1) _ratio = 1;

    if (_ratio != 1) {
        MessageSvc::Warning("ReduceData", (TString) "Reduce size to speedup", to_string(m_type), "creation to", to_string(_maxEvt), "entries");
        MessageSvc::Warning("Original size", to_string(_size));
        MessageSvc::Warning("Ratio", to_string(_ratio));
        if (!_random) {
            SetData((RooDataSet *) DataSet()->reduce(EventRange(0, _maxEvt)));
        } else {
            RooDataSet * _data = (RooDataSet *) m_data->emptyClone(m_name + "_random");
            for (Long64_t i = 0; i < _size; ++i) {
                if (round(fmod(i, _ratio != int(_ratio) ? _ratio / 2. : _ratio)) != 0) continue;
                // if (m_data->isNonPoissonWeighted()) {
                //    _data->add(*m_data->get(i), m_data->get(i)->getRealValue("w" + WeightDefRX::ID::FULL));
                //} else {
                if (m_data->get(i) != nullptr) {
                    _data->add(*m_data->get(i));
                } else {
                    MessageSvc::Warning("ReduceData", (TString) "get(", to_string(i), ") is nullptr", "SKIPPING");
                }
                //}
            }
            if (_data->sumEntries() == 0) {
                MessageSvc::Warning("ReduceData", (TString) "Empty data", "SKIPPING");
                return;
            }
            SetData((RooDataSet *) _data);
        }
        MessageSvc::Warning("Reduced size", to_string(DataSize()));

        m_ratioReduced = _ratio;
        SetStatus(m_isLoaded, true);
    } else {
        MessageSvc::Warning("ReduceData", (TString) "Nothing to reduce (", to_string(_size), "->", to_string(_maxEvt), ")");
    }

    return;
}

void FitComponent::ShiftDataset(double _shift) {
    if (m_data){
        shared_ptr < RooDataSet > _shiftedDataset = make_shared<RooDataSet>(m_data->GetName(), m_data->GetTitle(), RooArgSet(*m_var));
        for (int i = 0; i < m_data->numEntries(); i++){
            const RooArgSet * _entry = m_data->get(i);
            RooRealVar * _var = dynamic_cast<RooRealVar*>(_entry->first());
            double _shiftedValue = _var->getVal() + _shift;
            m_var->setVal(_shiftedValue);
            _shiftedDataset->add(RooArgSet(*m_var));
        }
        m_data = _shiftedDataset;
    }
    else{
        MessageSvc::Error("FitComponent", "ShiftDataset", "m_data is a nullptr!");
    }
}

pair< double, double > FitComponent::GetRange() {
    double _min = m_var->getMin();
    double _max = m_var->getMax();
    if (m_type == PdfType::RooKeysPDF) {
        double _delta = (_max - _min) * .2;
        _min -= _delta;
        _max += _delta;
    }
    if (m_option.Contains("range")) {
        TString _option = StripStringBetween(m_option, "range[", "]");
        vector< TString > _split = TokenizeString(_option, ",");
        if (_split.size() != 2) MessageSvc::Error("GetRange", m_name, "Cannot split (size !=2)", _option, "EXIT_FAILURE");
        if (!_split[0].IsFloat() || !_split[1].IsFloat()) MessageSvc::Error("GetRange", m_name, "Cannot split (not Float)", _option, "EXIT_FAILURE");
        if (_split[0].IsFloat() > _split[1].IsFloat()) MessageSvc::Error("GetRange", m_name, "Cannot split (Order equality)", _option, "EXIT_FAILURE");
        _min = _split[0].Atof();
        _max = _split[1].Atof();
    }
    return make_pair(_min, _max);
}

pair< double, double > FitComponent::GetExcludedRange() {
    double _min;
    double _max;
    if (m_option.Contains("exclude")) {
        TString _option  = StripStringBetween(m_option, "exclude[", "]").ReplaceAll(" ","");
        vector< TString > _split = TokenizeString(_option, ",");
        if (_split.size() != 2) MessageSvc::Error("GetExcludedRange", m_name, "Cannot split, size !=2", _option, "EXIT_FAILURE");
        if (!_split[0].IsFloat() || !_split[1].IsFloat()) MessageSvc::Error("GetExcludedRange", m_name, "Cannot split (not Float)", _option, "EXIT_FAILURE");
        if (_split[0].IsFloat() > _split[1].IsFloat()) MessageSvc::Error("GetExcludedRange", m_name, "Cannot split (Order equality)", _option, "EXIT_FAILURE");
        _min = _split[0].Atof();
        _max = _split[1].Atof();
    }
    return make_pair(_min, _max);
}

void FitComponent::CreateDatasetCachePath() {
    if( m_type == PdfType::Template) return;  //FORCE NO CACHING FOR Template ( external supplied from option )
    if (!m_isLoaded && !m_isReduced) {
        if (m_datasetCache == "") {
            MessageSvc::Info(Color::Cyan, "FitComponent", m_name, "CreateDatasetCachePath");
            TString _cutString(m_eventType.GetCut());
            TString _weightString(m_eventType.GetWeight());
            TString _cutHash     = HashString(_cutString);
            TString _weightHash  = HashString(_weightString);
            TString _tupleOption = m_eventType.GetTupleHolder().Option();
            MessageSvc::Debug("CreateDatasetCachePath, CutString", _cutString );
            MessageSvc::Debug("CreateDatasetCachePath, WeightString", _weightString );            
            MessageSvc::Debug("CreateDatasetCachePath, TupleOption", _tupleOption );
            TString _cacheDir    = IOSvc::GetFitCacheDir("", m_eventType, _cutHash, _weightHash, _tupleOption, "DataSetCache");
            if (m_debug) MessageSvc::Debug("DatasetCacheDir", _cacheDir);
            TString _cacheName = ((TString) m_name).ReplaceAll(SettingDef::separator + m_eventType.GetKey(), "");
            MessageSvc::Debug("DatasetCacheName, no ranges", _cacheName);
            if( m_var != nullptr ){
                TString _fittinVarName =  m_var->GetName();
                if( m_option.Contains("leakage")){
                    auto _allopts = TokenizeString(m_option, "-");
                    TString _LeakageBit = "";
                    for( auto & el : _allopts){ if( el.Contains("leakage")){ _LeakageBit = el; break;} }
                    MessageSvc::Warning("FitComponent is with B mass smeared, updated cache names");
                    _LeakageBit = _LeakageBit.ReplaceAll("[", "_").ReplaceAll("]","_").ReplaceAll("-","_");
                    _fittinVarName+= _LeakageBit;             
                }
                if( m_option.Contains("reweightXJPs")){
                    _fittinVarName += "_reweightXJPs";
                    MessageSvc::Warning("FitComponent is with reweightXJPs sample, updated cache names");
                }
                pair< double, double > _range = GetRange();
                _cacheName += SettingDef::separator + _fittinVarName + SettingDef::separator + to_string(_range.first) + SettingDef::separator + to_string(_range.second) + ".root";
                MessageSvc::Debug("DatasetCacheName", _cacheName);                
            }       
            MessageSvc::Debug("DatasetCacheName", _cacheName);                
            // if (m_debug) MessageSvc::Debug("DatasetCacheName", _cacheName);
            m_datasetCache = _cacheDir + "/" + _cacheName;
            if( SettingDef::Fit::LocalCaches == true){
                _cacheDir = "./LocalCache";
                if( SettingDef::Weight::iBS >=0 && _cacheName.Contains("LPT") ){
                    //force remaking dataset cache for fit in BS mode.
                    _cacheName+= TString::Format("_BS%i",SettingDef::Weight::iBS);
                }
                m_datasetCache = _cacheDir + "/" + _cacheName;            
            }
            if (SettingDef::Fit::useDatasetCache && (m_eventType.GetAna() != Analysis::All) && ((m_type != PdfType::StringToPDF) && (m_type != PdfType::RooAbsPDF) && (m_type != PdfType::SignalCopy))) {
                IOSvc::MakeDir(_cacheDir, OpenMode::UPDATE);
                MessageSvc::Info("DatasetCache", m_datasetCache);
                if (SettingDef::Fit::redoDatasetCache) { DeleteDatasetCache(); }
            }
        } else {
            if (m_debug) MessageSvc::Debug("FitComponent", m_name, "DatasetCache already set", m_datasetCache);
        }
    }
    return;
}

void FitComponent::DeleteDatasetCache() {
    if (IOSvc::ExistFile(m_datasetCache)) {
        MessageSvc::Warning("DeleteDatasetCache", m_datasetCache);
        IOSvc::RemoveFile(m_datasetCache);
    }
    return;
}

void FitComponent::SaveDatasetCache() {
    if (!m_isLoaded && !m_isReduced) {
        if (m_datasetCache == "") {
            MessageSvc::Warning("SaveDatasetCache", (TString) "DatasetCache not specified");
        } else{ 
            bool _doIT =   !IOSvc::ExistFile(m_datasetCache) || SettingDef::Fit::redoDatasetCache;
            if( IOSvc::ExistFile(m_datasetCache) ){
                auto _tFile = IOSvc::OpenFile(m_datasetCache, OpenMode::READ);
                auto * _workspace = _tFile->Get<RooWorkspace>("DatasetCache");
                if( _workspace == nullptr){
                    //to fix backward incompatible things
                    _doIT = true;
                }
                IOSvc::CloseFile(_tFile);
            }
            if( _doIT){ 
                MessageSvc::Info(Color::Cyan, "FitComponent", m_name, "SaveDatasetCache");
                MessageSvc::Info("SaveDatasetCache", m_datasetCache);
                MessageSvc::Info("DataSet", DataSet());

                TString _name = SettingDef::IO::outDir+"/tmpDatasetCache.root";

                TFile * _tFile = nullptr;
                if (SettingDef::IO::useEOS) {
                    _tFile = IOSvc::OpenFile(_name, OpenMode::RECREATE);
                } else {
                    _tFile = IOSvc::OpenFile(m_datasetCache, OpenMode::RECREATE);
                }
                RooWorkspace datasetWorkspace("DatasetCache");
                datasetWorkspace.import(*m_data);
                datasetWorkspace.Write();
                TNamed   cutOpt(        TString("cutOpt"),        TString(m_eventType.GetCutHolder().Option())) ;
                TNamed   cut(           TString("cut"),           TString(m_eventType.GetCut()));
                TNamed   weightOpt(     TString("weightOpt"),     TString(m_eventType.GetWeightHolder().Option()) );
                TNamed   weight(        TString("weight"),        TString(m_eventType.GetWeight()));
                TNamed   tupleOpt(      TString("tupleOpt"),      TString(m_eventType.GetTupleHolder().Option()));
                cutOpt.Write();
                cut.Write();
                weightOpt.Write();
                weight.Write();
                tupleOpt.Write();
                IOSvc::CloseFile(_tFile);
                if (SettingDef::IO::useEOS) {
                    if (!IOSvc::ExistFile(_name)) MessageSvc::Error("SaveDatasetCache", _name, "does not exist", "EXIT_FAILURE");
                    IOSvc::CopyFile(_name, m_datasetCache);
                }
            }
        }
    }
    return;
}

void FitComponent::LoadDatasetCache() {
    if (!m_isLoaded && !m_isReduced) {
        if (m_datasetCache == "") {
            MessageSvc::Warning("LoadDatasetCache", (TString) "DatasetCache not specified");
        } else if (IOSvc::ExistFile(m_datasetCache)) {
            MessageSvc::Info(Color::Cyan, "FitComponent", m_name, "LoadDatasetCache");
            MessageSvc::Info("LoadDatasetCache", m_datasetCache);
            auto _tFile = IOSvc::OpenFile(m_datasetCache, OpenMode::READ);        
            if (_tFile != nullptr) {                
                if (_tFile->GetSize() == 0) {
                    MessageSvc::Warning("LoadDatasetCache", (TString) "Size 0 file", m_datasetCache);
                } else {
                    if(_tFile->Get< TNamed>("cut")== nullptr ){
                        MessageSvc::Debug("The dataset cache file HAS NOT the cut stored in...TO FIX code for RooKeysPDFS...");
                        IOSvc::CloseFile(_tFile);
                        return;
                    }else{
                        TString cut_FILE(_tFile->Get< TNamed>("cut")->GetTitle() );
                        TString cut( m_eventType.GetCut().GetTitle());
                        if(cut != cut_FILE){
                            //Hashing algorithm IS NOT PERFECT
                            MessageSvc::Debug("Cut(ET)", cut);
                            MessageSvc::Debug("Cut(FileRead)", cut_FILE);
                            MessageSvc::Error("Incompatible cuts on file and current processing", "","");
                            IOSvc::CloseFile(_tFile);
                            return; 
                        }else{
                            MessageSvc::Info("OK CUT MATCH!");
                        }
                    }
                    auto * _workspace = _tFile->Get<RooWorkspace>("DatasetCache");
                    if( _workspace == nullptr){
                        MessageSvc::Warning("Backward incompatible dataset Cache, remaking it");
                    }else{
                        TString _datasetName = "dataSet_" + m_name;
                        auto * _oldDataset = (RooDataSet*) _workspace->data(_datasetName);
                        m_data = make_shared<RooDataSet>(*_oldDataset, _datasetName);
                        delete _workspace; // _oldDataset is owned by _workspace, will be deleted with _workspace
                        if (!m_data) {
                            MessageSvc::Warning("LoadDatasetCache", (TString) "Cannot find dataSet_" + m_name + "in", m_datasetCache);
                            _tFile->ls();
                        } else if (DataSize() == 0) {
                            MessageSvc::Warning("LoadDatasetCache", (TString) "Empty dataSet_" + m_name + "in", m_datasetCache);
                            m_data.reset();
                        }
                    }
                }
                if(m_data) MessageSvc::Info("LoadDatasetCache", "Load", to_string(m_data->sumEntries()), "Entries");
                IOSvc::CloseFile(_tFile);
            }
            if (m_data) MessageSvc::Info("DataSetCache", DataSet());
        }
    }
    return;
}

void FitComponent::ReduceComponent(TCut _cut) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitComponent", m_name, "ReduceComponent");
    if ((m_type == PdfType::StringToPDF) || (m_type == PdfType::RooAbsPDF) || (m_type == PdfType::RooHistPDF) || (m_type == PdfType::ToyPDF)) {
        if (m_type == PdfType::StringToPDF) { CreatePDF(); }
        MessageSvc::Warning("ReduceComponent", (TString) "Nothing to reduce");
        MessageSvc::Line();
        return;
    }

    if (!m_option.Contains("add")) {
        if (!m_data) CreateData();
        MessageSvc::Info("Cut", &_cut);
        MessageSvc::Info("DataSet", DataSet());
        if (DataSize() == 0) {
            MessageSvc::Warning("ReduceComponent", (TString) "Empty data", "SKIPPING");
            return;
        }
        MessageSvc::Warning("Original size", to_string(DataSize()));
        m_ratioReduced *= DataSize();
        SetData((RooDataSet *) DataSet()->reduce(Cut(TString(_cut))));
        m_ratioReduced /= DataSize();
        MessageSvc::Warning("Reduced size", to_string(DataSize()));
        MessageSvc::Line();

        if (DataSize() == 0) {
            MessageSvc::Warning("ReduceComponent", (TString) "Empty data", "SKIPPING");
            return;
        }

        CreateData();
        if (DataSize() == 0) {
            MessageSvc::Warning("ReduceComponent", (TString) "Empty data", "SKIPPING");
            return;
        }

        if (m_eventType.IsMC()) { CreatePDF(); }
    } else {
        AddPDFs(_cut);
    }

    SetStatus(m_isLoaded, true);

    return;
}

RooAbsReal * FitComponent::operator[](const TString & _keyPar) {
    if (m_shapeParameters.find(_keyPar.Data()) != m_shapeParameters.end()) { return m_shapeParameters[_keyPar.Data()]; }
    MessageSvc::Error("FitComponent", m_name, "Cannot find parameter key", _keyPar, "EXIT_FAILURE");
    return nullptr;
}

void FitComponent::PrintKeys() const noexcept {
    cout << "\t\t " << m_name << RED << " (TYPE = " << to_string(m_type) << ")" << RESET << endl;
    if (m_data) {
        cout << "\t\t\t DataSet   = [size] : " << DataSize() << endl;
    } else if (m_hist) {
        cout << "\t\t\t DataHist  = [size] : " << DataSize() << endl;
    } else {
        cout << "\t\t\t Data      = [nullptr]" << endl;
    }
    for (const auto & _par : m_shapeParameters) {
        cout << "\t\t\t Parameter = " << _par.first << " [" << _par.second << "] : ";
        PrintVar(_par.second);
    }
    return;
}

void FitComponent::SetBinnedFit(bool _flag) {
    MessageSvc::Line();
    if (_flag)
        MessageSvc::Info(Color::Cyan, "FitComponent", m_name, "SetBinnedFit");
    else
        MessageSvc::Info(Color::Cyan, "FitComponent", m_name, "SetUnbinnedFit");
    m_binned = _flag;
    MessageSvc::Line();
    return;
}

void FitComponent::SetConstantAllPars() {
    // MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitComponent", m_name, "SetConstantAllPars");
    cout << GREEN;
    FixPars(&m_shapeParameters);
    // MessageSvc::Line();
    return;
}

void FitComponent::SetConstantExceptPars(vector< string > _names) {
    // MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitComponent", m_name, "SetConstantExceptPars", to_string(_names.size()));
    for (size_t i = 0; i < _names.size(); i++) MessageSvc::Info("SetConstantExceptPars", _names[i]);
    cout << GREEN;
    FixPars(&m_shapeParameters, _names, "-except");
    // MessageSvc::Line();
    return;
}

void FitComponent::SetConstantExceptParsChangeRange(vector< string > _names, double min, double max) {
    // MessageSvc::Line();
    
    MessageSvc::Info(Color::Cyan, "FitComponent", m_name, "SetConstantExceptParsChangeRange", to_string(_names.size()));
    for (size_t i = 0; i < _names.size(); i++) MessageSvc::Info("SetConstantExceptParsChangeRange", _names[i]);
    cout << GREEN;
    FixPars(&m_shapeParameters, _names, "-except-RangeUpdate", min ,max );
    return;
}


void FitComponent::PrintParameters() const noexcept {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitComponent", m_name, "PrintParameters");
    cout << GREEN;
    PrintPars(m_shapeParameters);
    MessageSvc::Line();
    return;
}

bool FitComponent::LoadFitComponentCache(){
    if( m_option.Contains("dry")) return false; 
    auto _savingInfos = FitComponentCacheInfos();
    TString _path     = std::get<0>(_savingInfos);
    TString _fileName = std::get<1>(_savingInfos);
    TString _savingKey= std::get<2>(_savingInfos);
    TString _cacheFilePath = _path +"/"+ _fileName; 
    MessageSvc::Warning("LoadFitComponentCache", m_name, "LoadFromDisk Cache", _savingKey, _cacheFilePath);
    if (IOSvc::ExistFile(_cacheFilePath)){
        //this is for StringToFit cases 
        TFile _tFile(_cacheFilePath, "READ");
        MessageSvc::Line(); _tFile.ls(); MessageSvc::Line();
        // Kind of hacky solution for now, optimal is to just save shape parameters
        auto * _workspace = _tFile.Get<RooWorkspace>(_savingKey);
        if( _workspace == nullptr){ 
            //to fix backward incompatible things
            MessageSvc::Warning("Backward incompatible thing tried to load , unable to load, redoing it");
            _tFile.Close();
            return false;
        }
        auto * _pdf = (RooAbsPdf*) _workspace->allPdfs().first();
        auto * _oldVar = _workspace->var(m_var->GetName());
        auto _parameters = GetPars(_pdf, _oldVar);
        Str2VarMap _cloned;
        for (auto& _strParPair : _parameters){
            string _newKey = _strParPair.first.substr(0, _strParPair.first.find_first_of("_"));// Make a new string of every char until the first _
            auto * _oldPar = (RooRealVar*)_strParPair.second;
            auto * _newPar = new RooRealVar(*_oldPar, _oldPar->GetName());
            _cloned[_newKey] = (RooAbsReal*) _newPar;
        }
        m_shapeParameters = _cloned;
        m_pdf.reset(StringToPdf(m_option.Data(), m_name.Data(), m_var,  m_shapeParameters));
        MessageSvc::Info(Color::Yellow, "FitComponent (DISK)", m_name, "PrintParameters");
        cout << CYAN;
        cout << RESET;
        PrintParameters();
        MessageSvc::Line();
        SetStatus(true, m_isReduced);
        delete _workspace;
        _tFile.Close();
        cout << CYAN << *this << RESET << endl;
        MessageSvc::Warning("LoadFitComponentCache", _cacheFilePath);
        return true ;       
        //if we use RooKeysPdf loaded from disk we should save them via RooWorkspace import it and redirectServers to a list of arguments we 
        //use in this fitting execution
    }else{
        MessageSvc::Warning("LoadFitComponentCache", _cacheFilePath, "Failed to load, MC re-fitting will be done");       
        return false; 
    }
    return true;     
}

bool FitComponent::LoadFitComponentCacheRooKeysPdf(){
    auto _savingInfos = FitComponentCacheInfos();
    TString _path     = std::get<0>(_savingInfos);
    TString _fileName = std::get<1>(_savingInfos);
    TString _savingKey= std::get<2>(_savingInfos);
    TString _cacheFilePath = _path +"/"+ _fileName; 
    MessageSvc::Warning("LoadFitComponentCacheRooKeysPdf", m_name, "LoadFromDisk Cache", _savingKey, _cacheFilePath);

    bool _loaded = LoadRooKeysPDF(_cacheFilePath, _savingKey);

    if(not(_loaded)){
        MessageSvc::Warning("LoadFitComponentCacheRooKeysPdf", _cacheFilePath, "Failed to load, MC re-fitting will be done");
    }

    return _loaded;     
}

bool FitComponent::LoadRooKeysPDF(TString _fullpath, TString _workspaceKey){
    if (IOSvc::ExistFile(_fullpath)){
        TFile _tFile(_fullpath, "READ");
        MessageSvc::Line(); _tFile.ls(); MessageSvc::Line();
        auto * _workspace = _tFile.Get<RooWorkspace>(_workspaceKey);
        if(not(_workspace)){
            MessageSvc::Error("FitComponent::LoadRooKeysPDF", _workspaceKey, "is not found in", _fullpath, "EXIT_FAILURE");
        }
        auto * _pdf = (RooAbsPdf*) _workspace->allPdfs().first();
        if(_pdf == nullptr){ 
            MessageSvc::Error("FitComponent::LoadRooKeysPDF pdf not loaded", _workspaceKey, "allPdfs().first return nullptr in ", _fullpath, "EXIT_FAILURE");
        }
        _pdf->recursiveRedirectServers(RooArgSet(*m_var));
        if( m_type == PdfType::Template){
            MessageSvc::Warning("Template loading as RooNDKeysPdf");
            auto * cloned = new RooNDKeysPdf2(*dynamic_cast<RooNDKeysPdf2*>(_pdf), m_name);
            m_pdf.reset(cloned);
            if (!m_option.Contains("toyconf") && !m_option.Contains("saveWS")){
                delete _workspace; // We need to persist original workspace if we want to save RooKeysPDF to another workspace
            }
            _tFile.Close();
            cout << CYAN << *this << RESET << endl;
            MessageSvc::Warning("LoadRooKeysPDF", _fullpath);
            return true;                
        }else{
            auto * cloned = new RooKeysPdf(*dynamic_cast<RooKeysPdf*>(_pdf), m_name);
            m_pdf.reset(cloned);
            if (!m_option.Contains("toyconf") && !m_option.Contains("saveWS")){
                delete _workspace; // We need to persist original workspace if we want to save RooKeysPDF to another workspace
            }
            _tFile.Close();
            cout << CYAN << *this << RESET << endl;
            MessageSvc::Warning("LoadRooKeysPDF", _fullpath);
            return true;    
        }
    }else{
        MessageSvc::Warning("LoadRooKeysPDF", _fullpath, "Failed to load RooKeysPDF");       
        return false; 
    }
}

bool FitComponent::SaveFitComponentCache(){
    //What is important to save is the m_shapeParameters ONLY! 
    //What the loadFitComponentCaches does is Only FOR StringToFit shapes, to recreate the PDF using the m_shapeParameters saved to disk
    //We don't proceed as the usual, dealing with ParameterPool , refreshing/reloading, 
    //We StringToPdf using the loaded from disk "fitted" shapeParameters.
    if( m_option.Contains("dry")) return false;
    auto _savingInfos = FitComponentCacheInfos();
    TString _path     = std::get<0>(_savingInfos);
    TString _fileName = std::get<1>(_savingInfos);
    TString _savingKey= std::get<2>(_savingInfos);
    TString _cacheFilePath = _path +"/"+ _fileName;
    bool _fileNotGood = false;
    if (IOSvc::ExistFile(_cacheFilePath)){
        //this is for StringToFit cases 
        TFile _tFile(_cacheFilePath, "READ");
        MessageSvc::Line(); _tFile.ls(); MessageSvc::Line();
        // Kind of hacky solution for now, optimal is to just save shape parameters
        auto * _workspace = _tFile.Get<RooWorkspace>(_savingKey);
        if( _workspace == nullptr){ 
            MessageSvc::Warning("Redoing it even if it exists");
            _fileNotGood = true; 
            _tFile.Close();            
        }
    }
    bool _saveIt =  SettingDef::Fit::redoFitComponentCaches || !IOSvc::ExistFile( _cacheFilePath) || _fileNotGood;
    //TODO : a forcing routine of this! 
    if( _saveIt ){
        MessageSvc::Warning("SaveFitComponentCache", TString::Format("%s::%s", _cacheFilePath.Data(), _savingKey.Data()), "will be created"); 
        MessageSvc::Warning("SaveFitComponentCache", TString::Format("%s::%s", _fileName.Data(), _savingKey.Data()), "will be created (local)"); 
        PrintKeys();
        MessageSvc::Line();
        MessageSvc::Warning("SaveFitComponentCache", TString::Format("%s::%s", _fileName.Data(), _savingKey.Data()), "make-ing"); 
        TFile _tFile( "./"+_fileName, to_string(OpenMode::RECREATE));
        RooWorkspace _workspace("pdfWorkspace");
        _workspace.import(*m_pdf);
        _workspace.Write(_savingKey, TObject::kOverwrite);
        _tFile.Close();
        if (SettingDef::IO::useEOS) {
            //TODO : maybe at clusters something different needs to be done here ! 
            if (!IOSvc::ExistFile(_cacheFilePath) || _fileNotGood){
                MessageSvc::Warning("SaveFitComponentCache", TString::Format("%s::%s", _cacheFilePath.Data(), _savingKey.Data()), "Copying"); 
                if(_fileNotGood ){
                    MessageSvc::Warning("SaveFitComponentCache", TString::Format("%s::%s", _cacheFilePath.Data(), _savingKey.Data()), "Copying Overwrite [backward incompatible]"); 
                }
                IOSvc::CopyFile(_fileName, _cacheFilePath);
                IOSvc::RemoveFile(_fileName);
                return true; 
            }else{
                if( SettingDef::Fit::redoFitComponentCaches){
                    MessageSvc::Warning("SaveFitComponentCache", TString::Format("%s::%s", _cacheFilePath.Data(), _savingKey.Data()), "Copying OverWrite"); 
                    IOSvc::CopyFile(_fileName, _cacheFilePath);
                    IOSvc::RemoveFile(_fileName);
                    return true; 
                }
                MessageSvc::Warning("SaveFitComponentCache", TString::Format("%s::%s", _cacheFilePath.Data(), _savingKey.Data()), "Not copied, enable SettingDef::Fit::redoFitComponentCaches to overwrite"); 
            }
        }
        return true;           
    }else{
        MessageSvc::Error("Please delete by hand the FitComponentCaches, or Enable OVERRIDE FC Cache in your fit",_cacheFilePath,"EXIT_FAILURE");
        return false; 
    }
    return true; 
}

void FitComponent::SaveToDisk(TString _name) {
    if (m_isLoaded) {
        MessageSvc::Warning("FitComponent", m_name, "LoadedFromDisk SKIPPING SaveToDisk");
        return;
    }
    // SaveToLog(_name);   
    if (!SettingDef::IO::outDir.EndsWith("/")) SettingDef::IO::outDir += "/";
    if (_name != "") _name = "_" + _name;
    TString _oname = "FitComponent_" + m_name + _name;
    _name          = SettingDef::IO::outDir + _oname + "_LoadFromDisk.root";
    MessageSvc::Line();
#ifdef STREAMDATA
    MessageSvc::Info(Color::Cyan, "FitComponent", m_name, "SaveToDisk", _name, "with STREAMDATA");
#else
    MessageSvc::Warning("FitComponent", m_name, "SaveToDisk", _name, "without STREAMDATA");
#endif
    PrintKeys();
    MessageSvc::Line();

    m_parameterSnapshot.ConfigureSnapshotMap();

    TFile _tFile(_name, to_string(OpenMode::RECREATE));
    (*this).Write(_oname, TObject::kOverwrite);

    _tFile.Close();
    cout << WHITE << *this << RESET << endl;
    return;
}

void FitComponent::LoadFromDisk(TString _name, TString _dir) {
    MessageSvc::Line();
#ifdef STREAMDATA
    MessageSvc::Info(Color::Cyan, "FitComponent", m_name, "LoadFromDisk", _name, _dir, "with STREAMDATA");
#else
    MessageSvc::Warning("FitComponent", m_name, "LoadFromDisk", _name, _dir, "without STREAMDATA");
#endif

    if (_name != "") _name = "_" + _name;
    TString _oname = "FitComponent_" + m_name + _name;
    _name          = _oname + "_LoadFromDisk";

    if ((_dir != "") && (!_dir.EndsWith("/"))) _dir += "/";

    if (!IOSvc::ExistFile(_dir + _name + ".root")) MessageSvc::Error("FitComponent", _dir + _name + ".root", "does not exist", "EXIT_FAILURE");

    TFile _tFile(_dir + _name + ".root", "read");
    MessageSvc::Line();
    _tFile.ls();
    MessageSvc::Line();

    FitComponent * _fc = _tFile.Get<FitComponent>(_oname);
    *this              = *_fc;

    PrintKeys();
    MessageSvc::Line();

    SetStatus(true, m_isReduced);

    m_parameterSnapshot.ReloadParameters();

    RefreshParameterPool();

    _tFile.Close();
    cout << WHITE << *this << RESET << endl;
    return;
}

void FitComponent::SaveToLog(TString _name) const noexcept {
    if (!SettingDef::IO::outDir.EndsWith("/")) SettingDef::IO::outDir += "/";
    if (_name != "") _name = "_" + _name;
    _name = SettingDef::IO::outDir + "FitComponent_" + m_name + _name + ".log";

    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitComponent", m_name, "SaveToLog", _name);

    ofstream _file(_name);
    if (!_file.is_open()) MessageSvc::Error("SaveToLog", (TString) "Unable to open file", _name, "EXIT_FAILURE");
    _file << *this << endl;
    _file.close();

    MessageSvc::Line();
    return;
}


tuple<TString,TString, TString > FitComponent::FitComponentCacheInfos(){
    //Assumes CreateDataSetCache has been already done , the m_datasetCache is "configured" alredy! 
    TString _pathStore =  m_datasetCache;
    if( m_datasetCache== ""){ MessageSvc::Error("FitComponentCacheInfos()", "Something bad happened", "EXIT_FAILURE");}
    TString _dsInputName = TokenizeString(_pathStore, "/").back(); 
    TString _pathToFile  = _pathStore.ReplaceAll( _dsInputName, "");
    _dsInputName.ReplaceAll(".root", "");
    TString _etKey = m_eventType.GetKey(); 
    TString _fCName = m_name; 
    TString _type = to_string(m_type);
    TString _option = m_option; 
    //TODO , 
    //improve the "caching" to use ONLY the StringToPDF option , not the full fitter option, 
    //TRICK IN FITHOLDER WITH -EXTERNAL- PASSED TO SEPARATE THE OPTIONS, 
    //CACHING DONE WITH ONLY FIRST PART OF IT, which is the FitComponent PDF option
    if(_option.Contains("EXTERNAL")){ 
        _option = TokenizeString( m_option, "EXTERNAL")[0];
    }
    _fCName.ReplaceAll("-","_").ReplaceAll("[", "-").ReplaceAll("]","-").ReplaceAll(",", "_").ReplaceAll(".", "_");
    _type.ReplaceAll("-","_").ReplaceAll("[", "-").ReplaceAll("]","-").ReplaceAll(",", "_").ReplaceAll(".", "_");
    //OptionPDF too long string ! 
    _option = to_string(_option.ReplaceAll("-","_").ReplaceAll("[", "-").ReplaceAll("]","-").ReplaceAll(",", "_").ReplaceAll(".", "_").Hash());
    //EventTypeKey is Hashed Here...
    _etKey = to_string(_etKey.ReplaceAll("-","_").ReplaceAll("[", "-").ReplaceAll("]","-").ReplaceAll(",", "_").ReplaceAll(".", "_").Hash());
    TString _FILENAME_ = TString::Format("%s_%s_%s_%s.root", _type.Data(), _fCName.Data() , _etKey.Data() , _option.Data() );
    TString _SaveKey_ = _fCName; 
    MessageSvc::Info("FitComponentCacheName Path"    , _pathToFile);
    MessageSvc::Info("FitComponentCacheName FileName", _FILENAME_);
    MessageSvc::Info("FitComponentCacheName Key",      _SaveKey_);
    std::tuple<TString, TString, TString> _return = make_tuple( _pathToFile, _FILENAME_, _SaveKey_ );
    return _return;
}
#endif
