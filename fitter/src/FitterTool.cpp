#ifndef FITTERTOOL_CPP
#define FITTERTOOL_CPP

#include "FitterTool.hpp"
#include "SPlot2.hpp"

#include "yamlcpp.h"
#include <fmt_ostream.h>
#include <variant>
#include "RooProduct.h"
#include "RooProfileLL.h"
#include "RooProfileLLRX.hpp"
#include "RooMinimizer.h"
#include <Math/Minimizer.h>
#include "TNtupleD.h"
//================================================================================================
// PLOTTING NAMESPACE IMPLEMENTATION
//================================================================================================
void Plotting::ConfigurePadPlot( TPad * _plotPad){
    _plotPad->SetLeftMargin(0.125);
    _plotPad->SetTopMargin(0.08);
    _plotPad->SetBottomMargin(0.2);  
    return;
};   
void Plotting::ConfigurePullPlot( TPad * _resPad){
    _resPad->SetLeftMargin(0.125);
    _resPad->SetBottomMargin(0.25);
    return;
};
TLegend * Plotting::GetLegend( int _nLabels){
    float _legendYMin = 0.9 - 0.085 * _nLabels;
    if (_nLabels > 5){
        _legendYMin = 0.9 - 0.085 * (_nLabels + 1)/2;
    }
    TLegend *_legend = new TLegend(0.55, _legendYMin, 0.875, 0.9);
    _legend->SetBorderSize(0);
    _legend->SetFillColor(kWhite);
    _legend->SetFillStyle(0);

    if (_nLabels > 5){
        _legend->SetNColumns(2);
    }    
    return _legend;    
};
void Plotting::CustomizeRooPlot( RooPlot * _frame){
    _frame->GetXaxis()->SetLabelFont(132);
    _frame->GetYaxis()->SetLabelFont(132);    
    _frame->GetXaxis()->SetLabelSize(0.08);
    _frame->GetXaxis()->SetTitleSize(0.10);
    _frame->GetXaxis()->SetTitleOffset(0.9);
    _frame->GetXaxis()->SetNdivisions(10, 5, 0);
    _frame->GetYaxis()->SetLabelSize(0.08);
    _frame->GetYaxis()->SetTitleSize(0.10);
    _frame->GetYaxis()->SetTitleOffset(0.6);
    _frame->GetYaxis()->SetNdivisions(6, 5, 0);
    return;
};
void Plotting::CustomizeFramePull( RooPlot *_framePull){
    _framePull->SetTitle("");
    _framePull->GetXaxis()->SetLabelFont(132);
    _framePull->GetYaxis()->SetLabelFont(132);       
    _framePull->SetMarkerSize(1);
    _framePull->SetMaximum(6);
    _framePull->SetMinimum(-6);
    _framePull->SetYTitle("Pulls");
    _framePull->SetXTitle("");
    _framePull->SetLineWidth(1.0);
    TAxis * _yAxis   = _framePull->GetYaxis();
    TAxis * _xAxis   = _framePull->GetYaxis();
    _yAxis->SetNdivisions(5, 5, 0);
    _yAxis->SetLabelSize(0.25);
    _yAxis->SetTitleSize(0.30);
    _yAxis->SetTitleOffset(0.2);
    _yAxis->CenterTitle(1);
    _yAxis->SetTitle("Pulls");
    _xAxis->SetLabelSize(0.25);
    _xAxis->SetTitleSize(0);
    return; 
};
int Plotting::bkgsortidx( const TString &_nameIn){
    TString _name = _nameIn;
    _name.ToLower();
    if (_name.Contains("signal")) return -1;
    if (_name.Contains("comb")) return 0;
    if (_name.Contains("combSS")) return 0;
    if (_name.Contains("dslc")) return 1;
    if (_name.Contains("datadrivenemisid"))  return 1;
    if (_name == "partreco") return 2;
    if (_name == "partrecok1") return 2;
    if (_name == "partrecok2") return 2;
    if (_name == "partrecoh") return 3;
    if (_name == "partrecohad") return 3;
    if (_name.Contains("lb")) return 4;    
    //----------------------------------------
    if (_name.Contains("bd2kst")) return 10;
    if (_name.Contains("bu2kst")) return 11;
    if (_name.Contains("bdbu")) return 12;
    //----------------------------------------
    if (_name.Contains("bd") && !(_name.Contains("bd2kst") || _name.Contains("bdbu"))) return 5;
    // if (_name.Contains("bdbu")) return 4;
    if (_name.Contains("misid")) return 6;
    if (_name.Contains("bs2phi")) return 7;
    if (_name.Contains("bs")) return 8;
    if (_name.Contains("hadswap")) return 9;
    if (_name.Contains("leakage")) return 13;
    if (_name.Contains("psi2jpsx")) return 14;
    if (_name.Contains("psi2jpspipi")) return 15;
    if (_name == "partrecol") return 16;
    if (_name.Contains("custom")) return 17;
    if (_name.Contains("ketaprime")) return 18;
    //
    // failing everything , if it's custom stuff? 
    return numeric_limits<int>::max();
    MessageSvc::Error("bkgsortidx", (TString) "(", _name, ") not supported", "EXIT_FAILURE");
    return numeric_limits< int >::max();
};

void Plotting::SaveAllObjectFromFrame( RooPlot *_frame, TString _outFile, OpenMode _oMode){
    auto oFile = IOSvc::OpenFile( _outFile , _oMode);
    for( int idx = 0; idx < _frame->numItems(); ++idx){
        oFile->cd();
        auto obj = _frame->getObject(idx)->Clone();
        if( obj) oFile->WriteObjectAny( obj, obj->ClassName(), obj->GetName() );
    }
    IOSvc::CloseFile( oFile);
    return ;   
}



//================================================================================================
// FITTERTOOL IMPLEMENTATION
//================================================================================================
FitterTool::FitterTool(TString _name)
    : m_constraints() {
    if (SettingDef::debug.Contains("FT")) SetDebug(true);
    m_name = _name;
    if (m_debug) MessageSvc::Debug("FitterTool", m_name, "TString");
    if( SettingDef::Fit::useMinuit2){
        MessageSvc::Warning("FitterTool", (TString)"Use Minuit2");
        m_minType = "Minuit2";
    }
}

FitterTool::FitterTool(TString _name, FitterInfoContainer * _container)
    : m_constraints() {
    if (SettingDef::debug.Contains("FT")) SetDebug(true);
    m_name = _name;
    if (m_debug) MessageSvc::Debug("FitterTool", m_name, "FitterInfoContainer");
    if( SettingDef::Fit::useMinuit2){
        MessageSvc::Warning("FitterTool", (TString)"Use Minuit2");
        m_minType = "Minuit2";
    }
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitterTool", m_name, "FitInfoContainer");
    MessageSvc::Line();
    if (_container == nullptr)            MessageSvc::Error("FitterTool from FitterInfoContainer", (TString) "FitterInfoContainer is nullptr", "EXIT_FAILURE");
    if (_container->var       == nullptr) MessageSvc::Error("FitterTool from FitterInfoContainer", (TString) "FitterInfoContainer.var is nullptr", "EXIT_FAILURE");
    if (_container->dataset   == nullptr) MessageSvc::Error("FitterTool from FitterInfoContainer", (TString) "FitterInfoContainer.dataset is nullptr", "EXIT_FAILURE");
    if (_container->fullmodel == nullptr) MessageSvc::Error("FitterTool from FitterInfoContainer", (TString) "FitterInfoContainer.fullmodel is nullptr", "EXIT_FAILURE");
    auto * _toFill   = &m_fitManagers[_name];
    _toFill->FitInfo[_name] = *_container;
    m_isInitialized = true;
    return;    
}


FitterTool::FitterTool(TString _name, FitComponent * _fitComponent)
    : m_constraints() {
    if (SettingDef::debug.Contains("FT")) SetDebug(true);
    m_name = _name;
    if (m_debug) MessageSvc::Debug("FitterTool", m_name, "FitComponent");
    if( SettingDef::Fit::useMinuit2){
        MessageSvc::Warning("FitterTool", (TString)"Use Minuit2");
        m_minType = "Minuit2";
    }    
    AddFitComponent(_fitComponent);
}

FitterTool::FitterTool(TString _name, FitHolder * _fitHolder)
    : m_constraints() {
    if (SettingDef::debug.Contains("FT")) SetDebug(true);
    m_name = _name;
    if (m_debug) MessageSvc::Debug("FitterTool", m_name, "FitHolder");
    if( SettingDef::Fit::useMinuit2){
        MessageSvc::Warning("FitterTool", (TString)"Use Minuit2");
        m_minType = "Minuit2";
    }
    AddFitHolder(_fitHolder);
}

FitterTool::FitterTool(TString _name, FitManager * _fitManager)
    : m_constraints() {
    if (SettingDef::debug.Contains("FT")) SetDebug(true);
    m_name = _name;
    if (m_debug) MessageSvc::Debug("FitterTool", m_name, "FitManager");
    if( SettingDef::Fit::useMinuit2){
        MessageSvc::Warning("FitterTool", (TString)"Use Minuit2");
        m_minType = "Minuit2";
    }    
    AddFitManager(_fitManager);
}


void FitterTool::AddFitComponent(FitComponent * _fitComponent) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitterTool", m_name, "AddFitComponent", _fitComponent->IsBinned() ? "Binned" : "Unbinned");
    MessageSvc::Line();

    if (_fitComponent == nullptr) MessageSvc::Error("AddFitComponent", (TString) "FitComponent is nullptr", "EXIT_FAILURE");
    if (_fitComponent->Var() == nullptr) MessageSvc::Error("AddFitComponent", (TString) "Var is nullptr", "EXIT_FAILURE");

    cout << *_fitComponent << endl;

    auto * _toFill   = &m_fitManagers[_fitComponent->Name()];
    _toFill->manager = new FitManager(_fitComponent->Name(), "");

    FitterInfoContainer _container;
    _container.var     = _fitComponent->Var();
    _container.dataset = _fitComponent->DataSet();
    _container.binned  = _fitComponent->IsBinned();
    if (_fitComponent->DataHist() != nullptr) {
        _container.datahist = _fitComponent->DataHist();
    } else {
        _container.datahist = new RooDataHist("dh_" + _fitComponent->Name(), "binned version of " + _fitComponent->Name(), RooArgSet(*_fitComponent->Var()), *_container.dataset);
    }
    _container.ismc                         = _fitComponent->GetEventType().IsMC();
    _container.fullmodel                    = _fitComponent->PDF();
    _toFill->FitInfo[_fitComponent->Name()] = _container;
    m_isInitialized = true;
    return;
}

void FitterTool::AddFitHolder(FitHolder * _fitHolder) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitterTool", m_name, "AddFitHolder", _fitHolder->Name());
    MessageSvc::Line();

    if (_fitHolder == nullptr) MessageSvc::Error("AddFitHolder", (TString) "FitHolder is nullptr", "EXIT_FAILURE");
    if (_fitHolder->Configuration().Var() == nullptr) MessageSvc::Error("AddFitHolder", (TString) "Var is nullptr", "EXIT_FAILURE");

    if (m_fitManagers.find(_fitHolder->Name()) != m_fitManagers.end()) MessageSvc::Error("AddFitHolder", _fitHolder->Name(), "already in map", "EXIT_FAILURE");

    cout << *_fitHolder << endl;

    m_fitManagers[_fitHolder->Name()];
    m_fitManagers[_fitHolder->Name()].manager = new FitManager(_fitHolder->Name(), _fitHolder->Option());
    m_fitManagers[_fitHolder->Name()].manager->AddFitHolder(_fitHolder->Name(), _fitHolder);

    cout << *m_fitManagers[_fitHolder->Name()].manager << endl;

    return;
}

void FitterTool::AddFitManager(FitManager * _fitManager) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitterTool", m_name, "AddFitManager", _fitManager->Name());
    MessageSvc::Line();

    if (_fitManager == nullptr) MessageSvc::Error("AddFitManager", (TString) "FitManager is nullptr", "EXIT_FAILURE");
    if (m_fitManagers.find(_fitManager->Name()) != m_fitManagers.end()) MessageSvc::Error("AddFitManager", "Try to add same fit manager name to the list of managers", "EXIT_FAILURE");

    cout << *_fitManager << endl;

    m_fitManagers[_fitManager->Name()].manager = _fitManager;
    return;
}

void FitterTool::AddGaussConstraint(RooRealVar * par) {
    MessageSvc::Info(Color::Cyan, "FitterTool", m_name, "AddGaussConstraint", (TString) par->GetName());
    m_constraints.GenerateConstraint(*par);
    return;
}

void FitterTool::AddMultiVarGaussConstraint(RooArgList & listOfVariables, TMatrixDSym covMatrix) {
    MessageSvc::Info(Color::Cyan, "FitterTool", m_name, "AddMultiVarGaussConstraint");
    if( SettingDef::Fit::CorrelateConstraintsNoNorm){
        MessageSvc::Debug("CorrelateConstraintsNoNorm used");
        m_constraints.GenerateCorrelatedConstraintsNoNorm(listOfVariables, covMatrix);
    }else{
        m_constraints.GenerateCorrelatedConstraints(listOfVariables, covMatrix);
    }
    return;
}

void FitterTool::Init() {
    if(MessageSvc::SILENCED){
        //maybe Reduced ! 
        //enum PrintLevel { None=-1, Reduced=0, Normal=1, ExtraForProblem=2, Maximum=3 } ;
        m_printLevel      = -1;
        m_printEvalErrors =  3;
        RooMsgService::instance().setGlobalKillBelow(RooFit::FATAL);
    }
    if( SettingDef::Fit::useMinuit2){
        MessageSvc::Warning("FitterTool", (TString)"Use Minuit2");
        m_minType = "Minuit2";
    }    
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitterTool", m_name, "Initialize ...");
    MessageSvc::Line();

    if (m_fitManagers.size() == 0) MessageSvc::Error("Init", (TString) "No FitManagers stored in class", "EXIT_FAILURE");
    if (m_isInitialized) return;

    for (auto & _fitManager : m_fitManagers) {
        MessageSvc::Info("Loading FitManager", _fitManager.first);

        if (_fitManager.second.manager == nullptr) MessageSvc::Error("Init", (TString) "FitManager in nullptr", "EXIT_FAILURE");
        if (_fitManager.second.manager->Holders().size() == 0) MessageSvc::Error("Init", (TString) "No FitHolders stored in class", "EXIT_FAILURE");

        for (auto & _holder : _fitManager.second.manager->Holders()) {
            MessageSvc::Info("Loading FitHolder", _holder.first);

            // if (_holder.first.Contains(to_string(Analysis::MM)) && _holder.first.Contains(to_string(Analysis::EE))) MessageSvc::Error("Init", _holder.first, "not supported because both MM and EE in the name", "EXIT_FAILURE");
            // if (!(_holder.first.Contains(to_string(Analysis::MM)) || _holder.first.Contains(to_string(Analysis::EE)))) MessageSvc::Error("Init", _holder.first, "not supported because MM or EE not in the name", "EXIT_FAILURE");
            if( SettingDef::Weight::useBS && SettingDef::Weight::iBS >=0){
                MessageSvc::Warning("Bootstrapped fit, re-create full model, assumes you refit all MC as Nominal-loaded from disk");
                _holder.second.BuildModel();
            }
            _fitManager.second.FitInfo[_holder.first] = InitHolder(_holder.second, _holder.first);
        }
    }

    InitOptList();
    PrintFitter();

    GetParameterMaps();
    PrintParameterComponentRelation();
    InitStepSize();
    if (SettingDef::Fit::initialParamFile.size() != 0) { LoadInitialParameters(); }

    m_isInitialized = true;

    // We need the likelihood to figure out what to constraint.
    // Avoids constraining unused parameters
    if (m_constraintParameters && not(m_constraints.HasConstraints())) {
        RooArgSet _nLL;
        CreateNLL(_nLL);
        AddConstraints(_nLL);
        DeleteNLL(); 
    }
    return;
}

void FitterTool::Close() {
    MessageSvc::Info(Color::Cyan, "FitterTool", m_name, "Close ...");
    DeleteNLL();
    if (m_fitResults != nullptr) {
        MessageSvc::Warning("FitterTool", m_name, "Delete RooFitResult");
        delete m_fitResults;
        m_fitResults = nullptr;
    }
    return;
}

FitterInfoContainer FitterTool::InitHolder(const FitHolder & _holder, TString _name) {
    if (_name == "") _name = _holder.Name();
    MessageSvc::Info("InitHolder", _name);

    //----- create the fitter info container, for a given FitHolder !
    FitterInfoContainer _container;
    _container.var    = _holder.Configuration().Var();
    _container.binned = _holder.Configuration().IsBinnedCL();
    _container.labels = _holder.Configuration().GetLabelsNamed();
    _container.colors = _holder.Configuration().GetColorsNamed();

    //---- Check the dataset for the fitter is already there
    if (_holder.Data().DataSet() != nullptr) {
        _container.dataset = _holder.Data().DataSet();
        _container.dataset->SetName("ds_" + _name);
        MessageSvc::Info("Loading DataSet", (TString) _holder.Data().DataSet()->GetName());
        _container.datahist = new RooDataHist("dh_" + _name, "binned version of " + _name, RooArgSet(*_container.var), *_container.dataset);
        // IGNORED AS NOT RECOGNIZED IN RooAbsReal::createNLL
        if (_container.dataset->isNonPoissonWeighted()) m_sumW2Error = kTRUE;
        else m_sumW2Error = kFALSE;
    } else if (_holder.Data().DataHist() != nullptr) {
        _container.datahist = _holder.Data().DataHist();
        _container.datahist->SetName("dh_" + _name);
        MessageSvc::Info("Loading DataHist", (TString) _holder.Data().DataHist()->GetName());
        // IGNORED AS NOT RECOGNIZED IN RooAbsReal::createNLL
        if (_container.datahist->isWeighted()) m_sumW2Error = kTRUE;
        else                                   m_sumW2Error = kFALSE;
    } else MessageSvc::Error("InitHolder", (TString) "Data is nullptr", "EXIT_FAILURE");

    _container.ismc       = _holder.Data().GetEventType().IsMC();
    _container.extraRange = _holder.Configuration().ExtraRange();
    //---- Check the Analysis and RooAddPdf model is already there, load the fullmodel of the fit
    if (_holder.GetModel() == nullptr) MessageSvc::Error("InitHolder", (TString) "Model is nullptr", "EXIT_FAILURE");
    _container.fullmodel = _holder.GetModel();
    MessageSvc::Info("Loading Model", (TString) _holder.GetModel()->GetName());

    _container.components["Signal"] = _holder.GetSig();
    _container.yields["Signal"]     = _holder.SignalYield();

    MessageSvc::Info("Loading Signal", (TString) _holder.GetSig()->GetName());

    for (auto & _bkg : _holder.BackgroundComponents()){
        //---- wrapping around the name to store things, tokenizing the underscores
        TString _pdfName = _bkg.second.fitComponent.PDF()->GetName();
        if (!_pdfName.Contains("bkg")){
            // MessageSvc::Error("InitHolder", (TString) "This routine assumes bkg_ in the pdf name", _pdfName, "EXIT_FAILURE");    
            _pdfName = "bkg_"+_pdfName; // this is an hack...not sure why...
        }
        // Strip out the name, keep only the elements between first and second underscore *_*_*....
        auto *  strcollection           = _pdfName.Tokenize("_");
        TString keystore1               = TString(((TObjString *) strcollection->At(1))->String());
        auto *  strcollection2          = keystore1.Tokenize("-");
        TString keystore                = TString(((TObjString *) strcollection2->At(0))->String());
        _container.components[keystore] = _bkg.second.fitComponent.PDF();
        _container.yields[keystore]     = _bkg.second.yield;
        MessageSvc::Info("Loading Background", _pdfName, "->", keystore);
    }
    return move(_container);
}

void FitterTool::InitOptList() {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitterTool", m_name, "InitOptList");

    //---------- m_optList used to create individual likelihoods in EE/MM,
    // here in the initialization we add the Exended(), OffsetLikelihood(), NumCPU and Varbose flag but others are available.
    // From FitTo RooFit routine the likelihood is typically created on a combined dataset through:
    // ProjectedObservables,
    // Extended,
    // Range,
    // RangeWithName,
    // SumCoefRange,
    // NumCPU,
    // SplitRange,
    // Constrained,
    // Constrain,
    // ExternalConstraints,
    // CloneData,
    // GlobalObservables,
    // GlobalObservablesTag,
    // OffsetLikelihood;

    // OptList Filling
    /*
    ConditionalObservables(const RooArgSet& set) – Do not normalize PDF over listed observables
    Extended(Bool_t flag) – Add extended likelihood term, off by default
    Range(const char* name) – Fit only data inside range with given name
    Range(Double_t lo, Double_t hi) – Fit only data inside given range. A range named "fit" is created on the fly on all observables. Multiple comma separated range names can be specified.
    SumCoefRange(const char* name) – Set the range in which to interpret the coefficients of RooAddPdf components
    NumCPU(int num, int strat) – Parallelize NLL calculation on num CPUs
    Strategy 0 = RooFit::BulkPartition (Default) --> Divide events in N equal chunks
    Strategy 1 = RooFit::Interleave --> Process event i%N in process N. Recommended for binned data with
    a substantial number of zero-bins, which will be distributed across processes more equitably in this strategy
    Strategy 2 = RooFit::SimComponents --> Process each component likelihood of a RooSimultaneous fully in a single process
    and distribute components over processes. This approach can be benificial if normalization calculation time
    dominates the total computation time of a component (since the normalization calculation must be performed
    in each process in strategies 0 and 1. However beware that if the RooSimultaneous components do not share many
    parameters this strategy is inefficient: as most minuit-induced likelihood calculations involve changing
    a single parameter, only 1 of the N processes will be active most of the time if RooSimultaneous components
    do not share many parameters
    Strategy 3 = RooFit::Hybrid --> Follow strategy 0 for all RooSimultaneous components, except those with less than
    30 dataset entries, for which strategy 2 is followed.
    Optimize(Bool_t flag)   – Activate constant term optimization (on by default)
    SplitRange(Bool_t flag) – Use separate fit ranges in a simultaneous fit. Actual range name for each subsample is assumed to by rangeName_{indexState} where indexState is the state of the master index category of the simultaneous fit
    Constrain(const RooArgSet&pars) – For p.d.f.s that contain internal parameter constraint terms, only apply constraints to given subset of parameters
    ExternalConstraints(const RooArgSet&) – Include given external constraints to likelihood
    GlobalObservables(const RooArgSet&) – Define the set of normalization observables to be used for the constraint terms. If none are specified the constrained parameters are used
    GlobalObservablesTag(const char* tagName) – Define the set of normalization observables to be used for the constraint terms by a string attribute associated with pdf observables that match the given tagName
    Verbose(Bool_t flag) – Constrols RooFit informational messages in likelihood construction CloneData(Bool flag) – Use clone of dataset in NLL (default is true)
    Offset(Bool_t) – Offset likelihood by initial value (so that starting value of FCN in minuit is zero). This can improve numeric stability in simultaneously fits with components with large likelihood values
    */

    // https://root.cern.ch/doc/master/classRooAbsPdf.html#a8f802a3a93467d5b7b089e3ccaec0fa8

    m_optList.Clear();
    m_optList.subArgs().Clear();

    if (m_isFitTo) {
        MessageSvc::Info("Minimizer", m_minType, m_minAlg);
        m_optList.addArg(RooFit::Minimizer(m_minType, m_minAlg));

        MessageSvc::Info("Strategy", to_string(m_strategyMINUIT));
        m_optList.addArg(RooFit::Strategy(m_strategyMINUIT));
        
        // Do this on MC fits ?
        // MessageSvc::Info("InitialHesse", to_string(m_initialHesse));
        // m_optList.addArg(RooFit::InitialHesse(m_initialHesse));

        MessageSvc::Info("Hesse", to_string(m_hesse));
        m_optList.addArg(RooFit::Hesse(m_hesse));
        
        /*
            MessageSvc::Info("Minos", to_string(m_minos));
            m_optList.addArg(RooFit::Minos(m_minos));
        */
        if (m_constraints.HasConstraints()) {
            MessageSvc::Info("ExternalConstraints");
            m_optList.addArg(RooFit::ExternalConstraints(m_constraints.GetAllConstraints()));
        }

        MessageSvc::Info("Optimize", to_string(m_optimizeConst));
        m_optList.addArg(RooFit::Optimize(m_optimizeConst));

        //Ignored when fitting weighted data.
        MessageSvc::Info("SumW2Error", to_string(m_sumW2Error));
        //Forcing to false, the Results our has always bad covQual, fix passing always FALSE sumW2
        m_optList.addArg(RooFit::SumW2Error(kFALSE));

        MessageSvc::Info("Save", to_string(m_save));
        m_optList.addArg(RooFit::Save(m_save));

        MessageSvc::Info("Timer", to_string(m_timer));
        m_optList.addArg(RooFit::Timer(m_timer));

        MessageSvc::Info("PrintLevel", to_string(m_printLevel));
        m_optList.addArg(RooFit::PrintLevel(m_printLevel));

        MessageSvc::Info("PrintEvalErrors", to_string(m_printEvalErrors));
        m_optList.addArg(RooFit::PrintEvalErrors(m_printEvalErrors));

        MessageSvc::Info("Offset", to_string(m_offsetLikelihood));
        m_optList.addArg(RooFit::Offset(m_offsetLikelihood));

    }

    MessageSvc::Info("Extended", to_string(m_extended));
    m_optList.addArg(RooFit::Extended(m_extended));
    
    //OFFSET IN EACH PIECE ADDED IN LL ? NO, only FIT TO and use enableOffsetting eventually!
    //MessageSvc::Info("Offset", to_string(m_offsetLikelihood));
    //m_optList.addArg(RooFit::Offset(m_offsetLikelihood));

    if (m_nCPU == 1) m_strategyCPU = 2;
    if (m_nCPU > 1) {
        MessageSvc::Info("NumCPU", to_string(m_nCPU), to_string(m_strategyCPU));
        m_optList.addArg(RooFit::NumCPU(m_nCPU, m_strategyCPU));
    }

    MessageSvc::Info("Verbose", to_string(m_verbose));
    m_optList.addArg(RooFit::Verbose(m_verbose));

    if (m_excludeRange) {
        MessageSvc::Info("Fit in splitted range", m_excludedRange);
        m_optList.addArg(RooFit::Range(m_excludedRange));
    }

    PrintOptList();

    return;
}

inline void FitterTool::CreateNLL(RooArgSet & _nLL) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitterTool", m_name, "CreateNLL");
    if (!m_isInitialized) MessageSvc::Error("FitterTool not initialized", "EXIT_FAILURE");
    DeleteNLL();
    // https://root.cern.ch/doc/master/classRooAbsPdf.html#a536396deee60ae88762b306af45ec399
    // RooFit::PrintLevel(0); workaround for fast debug
    for (auto & _fitManager : m_fitManagers){
        MessageSvc::Info("Loading FitManagers", _fitManager.first);
        for (auto & _fitInfo : _fitManager.second.FitInfo) {
            MessageSvc::Info("Loading FitInfo", _fitInfo.first);
            // Append to options for likelihood creation also the Range one
            auto _optList = m_optList;
            if (_fitInfo.second.extraRange != "") {
                MessageSvc::Info("Range", _fitInfo.second.extraRange);
                _optList.addArg(Range(_fitInfo.second.extraRange));
            }
            if( _fitInfo.second.fullmodel == nullptr ) MessageSvc::Error("CreateNLL severe error, fullmodel to make likelihood missing","","EXIT_FAILURE");
            if( _fitInfo.second.dataset   == nullptr ) MessageSvc::Error("CreateNLL severe error, dataset   to make likelihood missing","","EXIT_FAILURE");

            if (_fitInfo.second.binned){
                MessageSvc::Error("Binned likelihood not implemented");
            }
            else{
                MessageSvc::Info("CreateNLL (unbinned) calling", _fitInfo.first);
                _fitInfo.second.nll = _fitInfo.second.fullmodel->createNLL(*_fitInfo.second.dataset, _optList.subArgs());
            }
            //_fitInfo.second.nll.enableOffsetting( m_offsetLikelihood);
            MessageSvc::Info("CreateNLL", _fitInfo.second.nll);
            // m_offsetvalue += _fitInfo.second.nll
            _nLL.add(*_fitInfo.second.nll);
        }
    }
    MessageSvc::Info("CreateNLL", &_nLL);

    if (m_constraints.HasConstraints()) {
        MessageSvc::Info("CreateNLL", (TString) "GenerateConstraintNLL");
        RooConstraintSum & _constraintsNLL = m_constraints.GenerateConstraintNLL("nllConstraintsToyFit");
        // m_offsetvalue += _constraintsNLL.getVal()
        _nLL.add(_constraintsNLL);        
        MessageSvc::Info("CreateNLL", &_nLL);
    }
    //this is the overall offset
    MessageSvc::Line();

    return;
}

void FitterTool::DeleteNLL() {
    MessageSvc::Warning("FitterTool", m_name, "Delete Likelihood");
    for (auto & _fitManager : m_fitManagers) {
        for (auto & _fitInfo : _fitManager.second.FitInfo) {
            if (_fitInfo.second.nll != nullptr) {
                delete _fitInfo.second.nll;
                _fitInfo.second.nll = nullptr;
            }
        }
    }
    return;
}

inline void FitterTool::AddConstraints(const RooArgSet & _nLL) {
    MessageSvc::Info(Color::Cyan, "FitterTool", m_name, (TString) "AddConstraints", "Adding constraints found in FitParameterPool");
    // This is put here since FitParameterPool is the one which figures out which constraints to use given the likelihood
    auto _parameterPool = RXFitter::GetParameterPool();
    for (auto * _constrainedParameter : _parameterPool->GetConstrainedParametersInLikelihood(_nLL)){
      //This should not be an efficiency right?
      AddGaussConstraint(_constrainedParameter); 
    }
    _parameterPool->FillConstrainedEfficiencyContainers(_nLL);
    if( m_constraintEfficiencies){
        for (auto * _uncorrelatedEfficiency : _parameterPool->GetUncorrelatedEfficiencies()) { 
            AddGaussConstraint(_uncorrelatedEfficiency); 
        }
        for (auto & _correlatedEfficiencyHolder : _parameterPool->GetCorrelatedEfficiencies()) {
            auto _correlatedEfficiencies = VectorToRooArglist(_correlatedEfficiencyHolder.GetCorrelatedEfficiencies());
            auto _covarianceMatrix       = MatrixToROOTMatrix(_correlatedEfficiencyHolder.GetCovarianceMatrix());
            MessageSvc::Line();
            MessageSvc::Info(Color::Cyan, "Printing correlated efficiencies and their covariance matrix");
            int i = 0;
            for (auto * _variable : _correlatedEfficiencies) { MessageSvc::Info(to_string(i++), (TString) _variable->GetName()); }
            _covarianceMatrix.Print();
            MessageSvc::Line();
            AddMultiVarGaussConstraint(_correlatedEfficiencies, _covarianceMatrix);
      }
    }
    if( SettingDef::Fit::rJPsiFit || SettingDef::Fit::RPsiFit || SettingDef::Fit::RXFit  ){
        MessageSvc::Warning("Injection of systematics in the fitter");
        pair<RooArgList, TMatrixDSym> _systematics_in_the_fit = _parameterPool->GetSystematicEffRatioListAndCovarianceMatrix();
        _systematics_in_the_fit.first.Print();
        MessageSvc::Line();
        MessageSvc::Info(Color::Cyan, "Printing correlated efficiency ratios systematics and their covariance matrix");
        int i = 0;
        for (auto * _variable : _systematics_in_the_fit.first) { MessageSvc::Info(to_string(i++), (TString) _variable->GetName()); }
        _systematics_in_the_fit.second.Print();
        MessageSvc::Line();
        AddMultiVarGaussConstraint(_systematics_in_the_fit.first, _systematics_in_the_fit.second);
    }
    return;
}

void FitterTool::FitTo(bool _saveResults) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitterTool", m_name, "FitTo");
    MessageSvc::Line();

    if (!m_isInitialized) MessageSvc::Error("FitterTool not initialized", "EXIT_FAILURE");
    if (m_fitManagers.size() != 1) MessageSvc::Error("More than 1 FitInfo stored in class, FitTo done with only 1 dataset and 1 model", "EXIT_FAILURE");

    auto _start = chrono::high_resolution_clock::now();

    m_isFitTo = true;

    InitOptList();

    if (m_fitManagers.size() != 1) MessageSvc::Warning("FitTo", (TString) "More than 1 FitManager not supported (will only fit the first)");
    for (auto & _fitManager : m_fitManagers) {
        if (_fitManager.second.FitInfo.size() == 0) MessageSvc::Warning("FitTo", (TString) "Empty FitInfo in " + _fitManager.first);
        if (_fitManager.second.FitInfo.size() != 1) MessageSvc::Warning("FitTo", (TString) "More than 1 FitInfo in " + _fitManager.first + " not supported");

        for (auto & _fitInfo : _fitManager.second.FitInfo) {
            MessageSvc::Info("Loading FitInfo", _fitInfo.first);

            int _status = 0;

            MessageSvc::Line();
            if (_fitInfo.second.binned && m_sumW2Error == false   ) {
                MessageSvc::Info("FitTo", (TString) "Binned DataHist", (TString) "Calling FitterTool::Fit() instead for binned datasets");
                MessageSvc::Line();
                /*
                    More goodies will be available 
                    BatchMode(bool on)	
                    Experimental batch evaluation mode. This computes a batch of likelihood values at a time, uses faster math functions and possibly auto vectorisation (this depends on the compiler flags). Depending on hardware capabilities, the compiler flags and whether a batch evaluation function was implemented for the PDFs of the model, likelihood computations are 2x to 10x faster. The relative difference of the single log-likelihoods w.r.t. the legacy mode is usually better than 1.E-12, and fit parameters usually agree to better than 1.E-6.
                    IntegrateBins(double precision) 
                    RecoverFromUndefinedRegions , mostly for Ipatia2 if used 
                    SumW2Error and AsymptoticError() 
                    
                */
                //Fit(_saveResults);
                m_fitResults = _fitInfo.second.fullmodel->fitTo(*_fitInfo.second.datahist, m_optList.subArgs());
                _status      = m_fitResults->status();
            } else {
                MessageSvc::Info("FitTo", (TString) "Unbinned DataSet");
                if( m_sumW2Error == true){ 
                    MessageSvc::Info("FitTo", (TString) "Unbinned DataSet forced as weighted MC fit, rely on fitTo from RooFit, bypass custom LL for MC fits");
                }
                MessageSvc::Line();
                m_fitResults = _fitInfo.second.fullmodel->fitTo(*_fitInfo.second.dataset, m_optList.subArgs());
                _status      = m_fitResults->status();
            }

            if (m_reFit || (_status != 0)) {
                int           _strategyMINUIT = m_strategyMINUIT;
                vector< int > _strategies     = {m_strategyMINUIT};
                if (m_strategyMINUIT == 2) _strategies = {2, 1, 2, 0};
                if (m_strategyMINUIT == 1) _strategies = {1, 2, 1, 0};
                if (m_strategyMINUIT == 0) _strategies = {0, 1, 2, 0};
                MessageSvc::Line();
                MessageSvc::Error("Status", to_string(_status));
                MessageSvc::Line();
                MessageSvc::Info("MINUIT strategies", _strategies);
                MessageSvc::Line();
                for (int _strategy : _strategies) {
                    MessageSvc::Line();
                    MessageSvc::Warning("Trying Strategy", to_string(_strategy));
                    m_strategyMINUIT = _strategy;
                    InitOptList();
                    MessageSvc::Warning("FitTo", (TString) "Resetting errors");
                    Str2VarMap _pars = GetPars(_fitInfo.second.fullmodel, _fitInfo.second.var);
                    PrintPars(_pars);
                    MessageSvc::Line();

                    if (_fitInfo.second.binned && m_sumW2Error == false) {
                        //Fit(_saveResults);
                        // NOTE : fitTo directly is biased (DaYu BinnedLikelihood not used in this case, when first fit failed)
                        m_fitResults = _fitInfo.second.fullmodel->fitTo(*_fitInfo.second.datahist, m_optList.subArgs()); 
                        _status      = m_fitResults->status();
                    } else {
                        m_fitResults = _fitInfo.second.fullmodel->fitTo(*_fitInfo.second.dataset, m_optList.subArgs());
                        _status      = m_fitResults->status();
                    }
                    if (_status == 0) {
                        m_strategyMINUIT = _strategyMINUIT;
                        break;
                    } else {
                        MessageSvc::Line();
                        MessageSvc::Error("Status", to_string(_status));
                        MessageSvc::Line();
                    }
                }
            }
            m_status = _status;
            // Dump integral infos only once the fit is really OVER! (CHECK/DEBUG for some cases , removed)
            // MessageSvc::Info("FitTo", (TString) "Integral info");
            // _fitInfo.second.var->setRange("integralRangeFit", _fitInfo.second.var->getMin(), _fitInfo.second.var->getMax()) ;
            // _fitInfo.second.var->setRange("integralRangeFull", 0, 10000);
            // RooAbsReal * _integralFit  = _fitInfo.second.fullmodel->createIntegral(*_fitInfo.second.var, RooFit::NormSet(*_fitInfo.second.var), RooFit::Range("integralRangeFit"));
            // RooAbsReal * _integralFull = _fitInfo.second.fullmodel->createIntegral(*_fitInfo.second.var, RooFit::NormSet(*_fitInfo.second.var), RooFit::Range("integralRangeFull"));
            // double _integralFit_val    = _integralFit->getVal();
            // double _integralFull_val   = _integralFull->getVal();
            // MessageSvc::Info("FitTo", (TString) "Integral fit:",   to_string(_integralFit_val));
            // MessageSvc::Info("FitTo", (TString) "Integral full:",  to_string(_integralFull_val));
            // MessageSvc::Info("FitTo", (TString) "Integral ratio:", to_string(_integralFit_val/_integralFull_val));
            // delete _integralFit;
            // delete _integralFull;

            break;   // 1 loop only accepted
        }
        break;   // 1 loop only accepted
    }
    if (!m_fitResults) MessageSvc::Error("FitTo", (TString) "Invalid FitResults", "EXIT_FAILURE");

    auto _stop = chrono::high_resolution_clock::now();
    MessageSvc::Line();
    MessageSvc::Warning("FitTo", (TString) "Took", to_string(chrono::duration_cast< chrono::seconds >(_stop - _start).count()), "seconds");


    PrintResults();
    if (_saveResults) {
        ofstream _outFile(fmt::format("{0}/{1}_MinimizerStatus.log", SettingDef::IO::outDir, m_name));
        PrintResults(_outFile);
        _outFile.close();
        SaveResults();
        PlotResults();
    }
    return;
}

void FitterTool::Fit(bool _saveResults) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitterTool", m_name, "Fit");
    MessageSvc::Line();
    if (!m_isInitialized) MessageSvc::Error("FitterTool not initialized", "EXIT_FAILURE");
    auto _start = chrono::high_resolution_clock::now();

    //================================================================================================
    // Main fit routine 
    // 1. Create the Likelihood and cache the Likelihood initial offset
    // 2. Create RooMinimizer instance and configure it
    // 3. Run the Fitting Procedure with the Minimizer and the NLL created
    // 4. If we run 2D LL scans, re-cycle the minimized LL and the Minimizer instance to run the contour call
    // 5. Clear up the Minimizer memory hogging and Save Results of the fit 
    // 6. If 1D scans are requested, a minos call is done (SettingDef::Fit::MinosPriorScan) to determine the range else, a +3,-3 sigma is done.
    //================================================================================================


    // if (m_profile1DRatios && SettingDef::Fit::scan1DParameter != "" && SettingDef::Fit::ScanPatching){
    //     ForceAllNuisanceScan(SettingDef::Fit::scan1DParameter);
    //     return;
    // }
    //================================================================================================
    //1. Create Minimizer Instance with the Likelihood
    //================================================================================================
    RooArgSet _nLL;
    CreateNLL(_nLL);
    RooAddition _nllSimultaneous("nllCombined", "-log(likelihood)", _nLL);    
    m_offset = _nllSimultaneous.getVal();
    MessageSvc::Info("Fit", &_nllSimultaneous);
    MessageSvc::Info("Fit", m_name, "Offsetting", to_string(m_offsetLikelihood));
    _nllSimultaneous.enableOffsetting(m_offsetLikelihood);
    MessageSvc::Info("Fit", &_nllSimultaneous);


    //================================================================================================
    //2. Create RooMinimizer Instance with the Likelihood and configure it
    //================================================================================================    
    RooMinimizer _minimizer(_nllSimultaneous);
    ConfigureMinimizer(_minimizer, m_name + "_Fit.log");
    //================================================================================================
    //3. Run Fitting Procedure
    //================================================================================================
    FittingProcedure(_minimizer, &_nllSimultaneous);    
    auto _stop = chrono::high_resolution_clock::now();
    MessageSvc::Line();
    MessageSvc::Warning("Fit", (TString) "Took", to_string(chrono::duration_cast< chrono::seconds >(_stop - _start).count()), "seconds");    
    PrintResults();
    if( m_profile2DRatios){
        //====================================================================================================
        // 4. Profile 2D of rJPsi RK / rJPsi RKst 
        // Since RX-ANA v5.2 contouring is performed with the already minimized NLL and re-using the RooMinimizer itself from the main fit
        // See RooFit bug at https://root-forum.cern.ch/t/possible-bug-in-roominimizer-contour-change-errordef-and-gets-not-restored/46919 about ErrorDef restoring
        //====================================================================================================
        auto _parameterPool = RXFitter::GetParameterPool();
        auto rJPsis = _parameterPool->GetSingleRatios();
        auto RXs    = _parameterPool->GetDoubleRatios();
        _minimizer.fitter()->GetMinimizer()->SetErrorDef(0.5);
        if( SettingDef::Fit::rJPsiFitWithSystematics()) {
            //Scan for rJPsi when systematics are enabled only
            MessageSvc::Info("Contour rJPsi Single Ratios");
            for( int i =0; i< rJPsis.size(); ++i ){
                for( int j = i+1; j < rJPsis.size(); ++j){
                    if( ! ( _nllSimultaneous.dependsOn( *rJPsis[i]) && _nllSimultaneous.dependsOn( *rJPsis[j] ) )) continue;
                    MessageSvc::Info("ProfileLikelihood Ratio 2D for Var1",TString( rJPsis[i]->GetName()));
                    MessageSvc::Info("ProfileLikelihood Ratio 2D for Var2",TString( rJPsis[j]->GetName()));
                    //TODO : how to reset once contour is run already?
                    PlotContour( _minimizer, *rJPsis[i], *rJPsis[j]);
                    //Really restore the ErrorDef to the default one !
                    _minimizer.fitter()->GetMinimizer()->SetErrorDef(0.5);
                }
            }
        }
        if (SettingDef::Fit::RPsiFitWithSystematics() || SettingDef::Fit::RXFitWithSystematics()) {         
            //Scan for RX 2D contouring when systematics are enabled only for RPsi RX pars
            if (SettingDef::Fit::RPsiFitWithSystematics()) MessageSvc::Info("Contour RPsi2S double ratios");
            else MessageSvc::Info("Contour RX double ratios");
            for(int i =0 ; i< RXs.size(); ++i ){
                for( int j = i+1; j < RXs.size(); ++j){
                    //https://root-forum.cern.ch/t/possible-bug-in-roominimizer-contour-change-errordef-and-gets-not-restored/46919
                    if( ! ( _nllSimultaneous.dependsOn( *RXs[i]) && _nllSimultaneous.dependsOn( *RXs[j] ) )) continue;
                    MessageSvc::Info("ProfileLikelihood Ratio 2D for Var1",TString( RXs[i]->GetName())); 
                    MessageSvc::Info("ProfileLikelihood Ratio 2D for Var2",TString( RXs[j]->GetName())); 
                    //TODO : how to reset once contour is run already?
                    PlotContour( _minimizer, *RXs[i], *RXs[j]);
                    _minimizer.fitter()->GetMinimizer()->SetErrorDef(0.5);
                }
            }
        }
    }
    if(false){
        //======================================================================================================
        // This code was a check from DaYu for debugging negative likelihood values in the converged fit
        //======================================================================================================        
        MessageSvc::Info("FitterTool", (TString)"Checking for negative likelihood");
        for (auto & _fitManager : m_fitManagers) {
            for (auto & _fitInfo : _fitManager.second.FitInfo) {
                TString _varName = _fitInfo.second.var->GetName();
                auto _modelVariable = dynamic_cast<RooRealVar*>(_fitInfo.second.fullmodel->getVariables()->find(_varName));
                RooArgSet _normSet(*_modelVariable);
                for (size_t i = 0; i < _fitInfo.second.dataset->numEntries(); i++){
                    auto _datasetVariable = dynamic_cast<RooRealVar*>(_fitInfo.second.dataset->get(i)->find(_varName));
                    _modelVariable->setVal(_datasetVariable->getVal());
                    double _likelihood = _fitInfo.second.fullmodel->getVal(_normSet);
                    if (_likelihood <= 0){
                        MessageSvc::Warning("FitterTool", (TString)"Non-positive likelihood value", _fitInfo.first, to_string(_likelihood));
                    }
                }
            }
        }
    }
    //================================================================================================
    // 5. Clear up the Minimizer memory hogging and Save Results of the fit
    //================================================================================================
    _minimizer.cleanup();
    if (_saveResults) {
      ofstream _outFile(fmt::format("{0}/{1}_MinimizerStatus.log", SettingDef::IO::outDir, m_name));
      PrintResults(_outFile);
      _outFile.close();
      SaveResults(_minimizer.GetName());
      PlotResults();
      DumpInitialParameters();
      if( false){
        DumpYieldsValues();
      }
    }
    DeleteNLL();
    
    //================================================================================================
    // 6. Perform 1D likelihood Scan
    //================================================================================================
    auto _findMinMax_ForProfile = [](  double errLow, double errUp , double cValue){
        double nSigmaAway = 1.5;
        double MIN = cValue - nSigmaAway * abs(errLow); 
        double MAX = cValue + nSigmaAway * abs(errUp); 
        //Could go negative
        while( MIN < 0 && nSigmaAway > 0  ){
            MIN = cValue - nSigmaAway *abs(errLow);
            nSigmaAway-=1;
            if( MIN > cValue) MessageSvc::Error("Have to break, logic is wrong", "","EXIT_FAILURE");
        }
        return make_pair( MIN, MAX);
    };
    //With specific parameter passed as argument.
    if (m_profile1DRatios && SettingDef::Fit::scan1DParameter != ""){
        if (m_independentParameterMap.find(SettingDef::Fit::scan1DParameter) == m_independentParameterMap.end()){
            MessageSvc::Error("FitterTool", (TString)"Likelihood Profile cannot find", SettingDef::Fit::scan1DParameter);
        }
        auto var = m_independentParameterMap[SettingDef::Fit::scan1DParameter];
        pair< double, double>  minmax; 
        TString _name =var->GetName();       
        auto nSigmaLow = 5.5;
        auto nSigmaUp  = 6.5;
        minmax = std::make_pair( var->getVal() - nSigmaLow*fabs(var->getError()) , 
                                 var->getVal() + nSigmaUp* fabs(var->getError()));
        if( var->hasAsymError(false)){
            if( var->getVal() - nSigmaLow * fabs(var->getErrorLo()) <0.  ){
                minmax.first  = 0.1;
            }
            if( var->getVal() +  nSigmaUp * fabs(var->getErrorHi()) >5.0 ){ 
                var->setMax( minmax.second);
            }
        }else{
            if( var->getVal() - nSigmaLow * fabs(var->getError()) <0. ){
                minmax.first   = 0.1;
            }
            if( var->getVal() + nSigmaUp  * fabs(var->getError()) >5.0 ){
                var->setMax( minmax.second);
            }
        }
        //Expand RX range up if it goes out of boundaries.
        if( var->getMax() > minmax.second ) var->setMax( minmax.second);

        MessageSvc::Info("ProfileLikelihood Ratio      1D for ",TString( var->GetName()));
        if( SettingDef::Fit::scanProfileManual){
            PlotProfileLikelihood( SettingDef::Fit::scan1DParameter, minmax.first, minmax.second);
        }else{
            PlotProfileLikelihoodROOT( SettingDef::Fit::scan1DParameter, minmax.first, minmax.second);
        }
        MessageSvc::Info("ProfileLikelihood Ratio Done 1D for ",TString( var->GetName()));
        DeleteNLL();
    }
    /*
    //OLD routine whihc was not eating a specific parameter, scan was donw with a long list of parameters (Obsolete)
    else if( m_profile1DRatios &&  SettingDef::Fit::scan1DParameter == ""){
        //========================================================================
        // Scan profile of all RX parameters together , not on a single one
        //========================================================================
        auto _parameterPool = RXFitter::GetParameterPool();    
        auto rJPsis = _parameterPool->GetSingleRatios(); //retrieve rJPsi/RX parameters
        auto RXs    = _parameterPool->GetDoubleRatios(); //retrieve rJPsi/RX parameters
        vector<pair< double, double> > _min_maxs_singleRatios;    
        vector<pair< double, double> > _min_maxs_doubleRatios;    
        //Cache SingRatio Parameters ranges
        for( auto * var : rJPsis){ 
            if( var->hasAsymError(false)) _min_maxs_singleRatios.push_back( _findMinMax_ForProfile(var->getErrorLo(), var->getErrorHi() , var->getVal()) );
            else  _min_maxs_singleRatios.push_back( _findMinMax_ForProfile(var->getError(), var->getError() , var->getVal()) );        
        }
        //Cache DoubleRatios Parameters ranges
        for( auto * var : RXs){    
            if( var->hasAsymError(false)){
                _min_maxs_doubleRatios.push_back( _findMinMax_ForProfile(var->getErrorLo(), var->getErrorHi(), var->getVal() ) );
            }else{
                _min_maxs_doubleRatios.push_back( _findMinMax_ForProfile(var->getError(), var->getError(), var->getVal() ) );
            }
        }
        int j = 0;            
        for( auto * var : rJPsis){
            //Create a fresh LL for each parameter to scan!
            RooArgSet _nLLScan;
            CreateNLL( _nLLScan);
            RooAddition _nllSimultaneousScan("nllCombined", "-log(likelihood)", _nLLScan);
            _nllSimultaneousScan.enableOffsetting(true);                
            if( !_nllSimultaneousScan.dependsOn( *var)) continue;
            MessageSvc::Info("ProfileLikelihood Ratio 1D for ",TString( var->GetName()));
            double min = _min_maxs_singleRatios[j].first;
            double max = _min_maxs_singleRatios[j].second;
            PlotProfileLikelihood( _nllSimultaneousScan, var, var->getVal() - 5.5* var->getError(), var->getVal() + 5.5 * var->getError() );  
            DeleteNLL();
            j++;
        }
        j = 0; 
        for( auto * var : RXs){
            //Create a fresh LL for each parameter to scan!
            RooArgSet _nLLScan;
            CreateNLL( _nLLScan);
            RooAddition _nllSimultaneousScan("nllCombined", "-log(likelihood)", _nLLScan);
            _nllSimultaneousScan.enableOffsetting(true);
            if( !_nllSimultaneousScan.dependsOn( *var)) continue;
            MessageSvc::Info("ProfileLikelihood Ratio 1D for ",TString( var->GetName())); 
            //TODO : maybe use min-max for this? 
            if( SettingDef::Fit::RPsiFitWithSystematics()){
                PlotProfileLikelihood( _nllSimultaneousScan, var,  var->getVal() - 5.5*var->getError(), var->getVal() + 5.5 * var->getError());
                DeleteNLL();
            }else if(SettingDef::Fit::RXFitWithSystematics() ){
                PlotProfileLikelihood( _nllSimultaneousScan, var,  var->getVal() - 1.3*var->getError(), var->getVal() + 1.3 * var->getError());
                DeleteNLL();
            }else{
                PlotProfileLikelihood( _nllSimultaneousScan, var,  var->getVal() - 5.5*var->getError(), var->getVal() + 5.5 * var->getError());
                DeleteNLL();
            }
            j++;
        }
    }
    */
    return;
}

void FitterTool::InitStepSize() {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitterTool", m_name, (TString) "InitStepSize");
    MessageSvc::Line();
    /*
        The step size tells minuit of how much make the steps around the initial values to reach a minimum 
        So while it minimize it moves by a given step the parameter of interest. 
        We want to skip this step-size for 
        - efficiencies ( they are gauss constrained )        
        - bfRatio ( they are gauss constrained )
        - Over ( ratios of Branchign ratios, gauss constrained )
        - _CombSS ( possibly gauss constrained parameter shape of background CombSS)
        - RLL, rJPsi, rPsi parameters 
        The initial step size is also run when fits are failing and we try to refit. 
        In order to recover from unwanted regions, if the signal or background yield is found to be negative , we assign an error to the yield parameter of 100% 
        This allows the fitter to resurrect from the failed and unwanted region , rather than keep it rolling in that bad configuration of parameters. 
    */
    for (auto & _nameVarPair : m_independentParameterMap) {        
        auto & _var = _nameVarPair.second;        
        bool _skip = false;
        bool _largeError = abs( _var->getError() / _var->getVal()) > 1;
        bool _belowLimit = _var->hasMin() ? (_var->getVal() - _var->getError()) < _var->getMin() : false;
        bool _aboveLimit = _var->hasMax() ? (_var->getVal() + _var->getError()) > _var->getMax() : false;

        bool _forceConfigure = _largeError || _belowLimit || _aboveLimit; // Only reconfigures the errors if they are not sensible

        if (_var->isConstant()){ _skip = true;}
        TString _name = _nameVarPair.first;
        if (_name.Contains("csyst_") && !_forceConfigure){ _skip = true;}
        if (_name.Contains("_CombSS") && !( _name.Contains("nbkg_") || _name.Contains("nsig_")) && !_forceConfigure ){ _skip = true; }    
        if (_name.Contains("eff_") && !_forceConfigure){    _skip = true;} //why ? 
        if (_name.Contains("bfRatio") && !_forceConfigure){ _skip = true;} //why ? 
        if (_name.Contains("Over") && !_forceConfigure){    _skip = true;} //why ? 
        if (_name.Contains("eff")  && !_forceConfigure){     _skip = true;}
        if (_name.Contains("RLL_") && !_forceConfigure){     _skip = true;} //MAYBE COMMENT OUT ?? 
        if (_name.Contains("rJPs_") && !_forceConfigure){    _skip = true;} //MAYBE COMMENT OUT ??         
        if (_name.Contains("rPs_") && !_forceConfigure){    _skip = true;} //MAYBE COMMENT OUT ??         
        if (_name.Contains("fgBrem") && !_forceConfigure){ _skip = true; } //MAYBE COMMENT OUT ?? 
        if( _skip){
            MessageSvc::Warning("Skipping InitStepSize for", _name );
            continue;
        }
        double _error = SettingDef::Fit::stepSize * TMath::Abs(_var->getVal());
        bool    _doIt = true; 
        MessageSvc::Info("Parameterer[Before]", _var);
        if (_name.Contains("nsig_") || _name.Contains("nbkg_")){
            //Yield parameters , if the value is found negative we assign an error which is 3x the value itself, so it can resurrect and come back positive
            if( _var->getVal() < 0 ){
                // Reset the yields if negative
                if (m_initialParams.find(_name) != m_initialParams.end()){
                    auto _initialState = m_initialParams[_name];
                    _var->setVal(_initialState.Value);
                    _var->setError(_initialState.Error);
                    _doIt = false;
                }
                else{
                    _var->setVal(0);
                    _var->setError(1);
                    _doIt = false;
                }
            }else{
                //Yield parameters , if the value has been found and it's positive , let's assign a step size being the sqrt( value ) 
                _error = 0.01 * _var->getVal();
                _var->setError( _error); //set an initial error being sqrt( value ), be careful with initial yields settings.
                _doIt = false;
            }
    	}
        else if( _name.Contains("fgBrem")){
            _var->setError(0.1* _var->getVal());
            _doIt = false; 
        }else if( _name.Contains("m_shift")){
            _var->setError(0.1);        
            _doIt = false ;
        }else if( _name.Contains("s_scale")){
            //s-scales usually known from J/Psi mode at the 1% level, should prevent to go out of bounds qith ranges...
            _var->setError(0.1);
            _doIt = false ;
        }else if( _name.Contains("b_Comb")){
            //if the fit failed , what to do for the next round? (be aware of SS data combSS ? )
            if( (abs((_var->getVal() - _var->getMin())/_var->getVal()) < 0.01) || 
                (abs((_var->getMax() - _var->getVal())/_var->getVal()) < 0.01) ){ 
                // The slope has hit the limit
                _var->setVal(-1.e-3);
            }
            _error = 1e-5;
            //Do this and forget about the nbkg ??
        }else if (m_initialParams.find(_name) != m_initialParams.end()){
            auto _initialState = m_initialParams[_name];
            _error = _initialState.Error;
        }
        else if(_var->hasMin() && _var->hasMax()){            
            _error = SettingDef::Fit::stepSizeRange * TMath::Min(_var->getMax() - _var->getVal(), _var->getVal() - _var->getMin());  //stepSize Forcing this to be TOOOOO SMALL 1% of the range ? 
        }

        if (_var->getError() > _error && _doIt){
            _var->setError(_error);
        }

        // If the variables exceed the limits with the given step sizes, re-initialize the values to be within the limits after stepping in both directions
        if (_var->hasMin() && (_var->getVal() - _var->getError() < _var->getMin())){
            _var->setVal(_var->getMin() + 2 * _var->getError());
        }
        else if (_var->hasMax() && (_var->getVal() + _var->getError() > _var->getMax())){
            _var->setVal(_var->getMax() - 2 * _var->getError());
        }
        MessageSvc::Info("Parameterer[After]", _var);
        //Maybe the logic has to be reversed ? If the current error is smaller than the one we manually enforce, update it to make it bigger? 
        //Or  with an unreasonable SMALL error ??? 
    }
    MessageSvc::Line();
    return;
}

void FitterTool::LoadInitialParameters() {
    MessageSvc::Line();
    for( auto & _file : SettingDef::Fit::initialParamFile ){
        MessageSvc::Info(Color::Cyan, "FitterTool", m_name, (TString) "LoadInitialParameters from list of files:", _file);
    }
    MessageSvc::Line();
    auto _parameterPool = RXFitter::GetParameterPool();
    int  _count         = 0;
    m_initialParams = ParseInitialParams();
    for (auto & _nameVarPair : m_independentParameterMap) {
        const TString & _name     = _nameVarPair.first;
        RooRealVar *    _variable = _nameVarPair.second;
        if (m_initialParams.find(_name) != m_initialParams.end()) {   // We want to tune this variable
            MessageSvc::Info("LoadInitialParameters (before)", _variable);
            auto _initialState = m_initialParams[_name];
            if (_initialState.NoRange) {                
                _variable->removeRange();
            }
            else if (_initialState.RangeSpecified) {
                if( _variable->getVal() < _initialState.LowerLimit){
                    std::cout<< "Variable Value In Load InitialParameters is currently out of bounds (low), placing in the middle!" << std::endl;
                    _variable->setVal( (_initialState.UpperLimit + _initialState.LowerLimit )/2.);
                }
                if( _variable->getVal() > _initialState.UpperLimit){
                    std::cout<< "Variable Value In Load InitialParameters is currently out of bounds (low)!" << std::endl;
                    _variable->setVal( (_initialState.UpperLimit + _initialState.LowerLimit )/2.);
                }
                _variable->setRange(_initialState.LowerLimit, _initialState.UpperLimit); // Only set the ranges is it is specified. Otherwise use default ranges
            }
            if (_initialState.Constant) {
                _variable->setVal(_initialState.Value);
                _variable->setConstant(1);
            }
            if(_initialState.gConst ) { 
                //TODO : what we should do here ? 
                //TODO : is this ENOUGH ? 
                _variable->setVal(  _initialState.Value);
                _variable->setError(_initialState.Error);
                _variable->setConstant(0);
                _parameterPool->AddConstrainedParameter( _variable);
            }
            // Why this is problematic?  (is it really problematic because of the fitter logic?)
            // else{
            //     _variable->setVal(_initialState.Value);
            //     _variable->setError(_initialState.Error);
            //}
            MessageSvc::Info("LoadInitialParameters (after)", _variable);
            _count++;
        }
    }

    MessageSvc::Line();
    MessageSvc::Info("LoadInitialParameters", (TString) to_string(_count));
    MessageSvc::Line();
    return;
}

void FitterTool::SetInitialValuesAndErrors() {
    int _count = 0;
    for (auto & _nameVarPair : m_independentParameterMap) {
        const TString & _name     = _nameVarPair.first;
        RooRealVar *    _variable = _nameVarPair.second;
        if (m_initialParams.find(_name) != m_initialParams.end()) {
            MessageSvc::Info("SetInitialValuesAndErrors[Before]", _variable);
            auto _initialState = m_initialParams[_name];
            if( _variable->hasMin() && _initialState.Value < _variable->getMin() ) MessageSvc::Error("Cannot configure Value, fix yaml input initialParams (min)", TString(_variable->GetName()) );
            if( _variable->hasMax() && _initialState.Value > _variable->getMax() ) MessageSvc::Error("Cannot configure Value, fix yaml input initialParams (max)", TString(_variable->GetName()) );
            _variable->setVal(_initialState.Value);
            _variable->setError(_initialState.Error);
            MessageSvc::Info("SetInitialValuesAndErrors[After]", _variable);
         }
        _count += 1;
    }
    MessageSvc::Line();
    MessageSvc::Info("SetInitialValuesAndErrors", (TString) to_string(_count));
    MessageSvc::Line();
}

map< TString, InitialParamOption > FitterTool::ParseInitialParams() {
    InitialParamOption                 _option;
    map< TString, InitialParamOption > _initialParams;
    for(  auto & initialParamFile :  SettingDef::Fit::initialParamFile){
        MessageSvc::Info("ParseInitialParams from file", initialParamFile);
        YAML::Node _fileNode = YAML::LoadFile(initialParamFile.Data());
        for (YAML::iterator _it = _fileNode.begin(); _it != _fileNode.end(); ++_it) {
            TString _name = _it->first.as< TString >();
            _option.LowerLimit = 0;
            _option.UpperLimit = 0;
            TString _options   = _it->second.as< TString >();
            if (m_debug) MessageSvc::Debug(_name, _options);
            _options.ReplaceAll("[", "");
            _options.ReplaceAll("]", "");
            _options.ReplaceAll(" ", "");
            TObjArray * _strCollection = _options.Tokenize(",");
            if (_strCollection->GetEntries() < 4) { MessageSvc::Error("ParseInitialParams", (TString) _name, "must have at least 4 entries", "EXIT_FAILURE"); }
            TString _valueString  = TString(((TObjString *) (*_strCollection).At(0))->String());
            TString _errorString  = TString(((TObjString *) (*_strCollection).At(1))->String());
            TString _constantFlag = TString(((TObjString *) (*_strCollection).At(2))->String());
            TString _noRangeFlag  = TString(((TObjString *) (*_strCollection).At(3))->String());
            _option.Value    = _valueString.Atof();
            _option.Error    = _errorString.Atof();
            _option.gConst    = false; 
            if( _constantFlag.Atoi() == 2){ 
                _option.gConst   = true; 
                _option.Constant = false; 
            }else{
                _option.Constant = (bool) _constantFlag.Atoi();
            }
            _option.NoRange  = (bool) _noRangeFlag.Atoi();
            if (not(_option.NoRange)) {
                if (_strCollection->GetEntries() == 6){
                    TString _lowerLimitString = TString(((TObjString *) (*_strCollection).At(4))->String());
                    TString _upperLimitString = TString(((TObjString *) (*_strCollection).At(5))->String());
                    _option.LowerLimit        = _lowerLimitString.Atof();
                    _option.UpperLimit        = _upperLimitString.Atof();
                    _option.RangeSpecified    = true;
                }
                else{
                    _option.RangeSpecified    = false;
                }
            }

            if(_initialParams.find(_name) != _initialParams.end()){
                //Avoid replication
                MessageSvc::Error("ParseInitialParams", TString::Format("File %s, Parameter %s  are duplicates, already loaded from another initial file: FIX IT!", initialParamFile.Data(), _name.Data()), "EXIT_FAILURE");
            }
            _initialParams[_name] = _option;
        }
    }//end loop param file list
    MessageSvc::Info("ParseInitialParams", (TString) to_string(_initialParams.size()));
    MessageSvc::Line();
    return _initialParams;
}

void FitterTool::DumpInitialParameters() {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitterTool", m_name, (TString) "DumpInitialParameters to", SettingDef::Fit::dumpParamFile);

    YAML::Emitter _emitter;
    _emitter << YAML::BeginMap;
    _emitter << YAML::Key << TString("Parameter Name ") << TString("[ value, error, constant?, unlimited range?, lower limit, upper limit]");
    for (const auto & _nameVarPair : m_independentParameterMap) {
        const auto & _name = _nameVarPair.first;
        const auto   _var  = _nameVarPair.second;
        if (!_var->isConstant()) {
            TString _emitString = "[" + to_string(_var->getVal());
            _emitString += ", " + to_string(0.02 * abs(_var->getVal()));
            _emitString += ", 0, 0";
            if (_var->hasMin() && _var->hasMax()) {
                _emitString += ", " + to_string(_var->getMin());
                _emitString += ", " + to_string(_var->getMax());
            }
            _emitString += "]";
            _emitter << YAML::Key << _name << _emitString;
        }
    }
    _emitter << YAML::EndMap;
    ofstream _file(SettingDef::Fit::dumpParamFile);
    if (!_file.is_open()) MessageSvc::Error("Unable to open file", SettingDef::Fit::dumpParamFile, "EXIT_FAILURE");
    _file << _emitter.c_str() << endl;
    _file.close();
    return;
}

void FitterTool::ConfigureMinimizer(RooMinimizer & _minimizer, TString _logFile) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitterTool", m_name, "ConfigureMinimizer", _logFile);

    if (!SettingDef::IO::outDir.EndsWith("/")) SettingDef::IO::outDir += "/";

    // https://root.cern.ch/root/html/src/RooAbsPdf.cxx.html#1173

    // Log file for dump
    if (m_logFile) {
        MessageSvc::Info("LogFile", _logFile);
        _minimizer.setLogFile(SettingDef::IO::outDir + _logFile);
    }
    // Choose the minimzer algorithm
    MessageSvc::Info("MinimizerType", m_minType);
    _minimizer.setMinimizerType(m_minType.Data());

    // Change MINUIT fit strategy : 0,1,2 (2 robustness, 1)
    MessageSvc::Info("Strategy", to_string(m_strategyMINUIT));
    _minimizer.setStrategy(m_strategyMINUIT);

    // Change MINUIT epsilon
    // MessageSvc::Info("Eps", to_string(???));
    // _minimizer.setEps(???);

    MessageSvc::Info("EvalErrorWall", to_string(kTRUE));
    _minimizer.setEvalErrorWall(kTRUE);

    /*
    Default RooFit is to do at most 500 * nParameters iterations in minuit and 500 * nParameters function calls for the likelihood. 
    We want to increase it by hand to the value we configure externally with is MUCH bigger. 
    The full RX fit will have O( 300 ) parameters with a 100x100 matrix of efficiencies, 
    therefore is suitable to make A LOT of calls before stopping, quitting too early might lead the fitter to sit in a weird parameter space which due to the logic of errors and step size we have in failure case
    might trigger nasty behaviours. 
    Help the fitter with more function calls and Iterations !!!! 
    */
    MessageSvc::Info("MaxFunctionCalls (500 * parameters is default in roofit)", to_string(m_maxFunctionCalls));
    _minimizer.setMaxFunctionCalls(m_maxFunctionCalls);
    MessageSvc::Info("MaxIterations (500 * parameters is default in roofit)", to_string(m_maxFunctionCalls));
    _minimizer.setMaxIterations(m_maxFunctionCalls);

    // Offsetting Likelihood
    MessageSvc::Info("Offsetting", to_string(m_offsetLikelihood));
    _minimizer.setOffsetting(m_offsetLikelihood);

    // perform constant term optimization of function being minimized
    MessageSvc::Info("OptimizeConst", to_string(m_optimizeConst));
    _minimizer.optimizeConst(m_optimizeConst);

    // Measure timing
    MessageSvc::Info("Timer", to_string(m_timer));
    _minimizer.setProfile(m_timer);

    // PrintLevel
    MessageSvc::Info("PrintLevel", to_string(m_printLevel));
    _minimizer.setPrintLevel(3);
    MessageSvc::Info("PrintEvalErrors", to_string(m_printEvalErrors));
    if( SettingDef::Fit::useMinuit2){
      _minimizer.setPrintEvalErrors(3); //DEBUG 
      gErrorIgnoreLevel = kInfo;      
    }else{
      _minimizer.setPrintEvalErrors(m_printEvalErrors);
    }
    // Verbosity
    MessageSvc::Info("Verbose", to_string(m_verbose));
    _minimizer.setVerbose(m_verbose);

    MessageSvc::Info("ConfigureMinimizer", &_minimizer);
    MessageSvc::Line();
    return;
}

void FitterTool::FittingProcedure(RooMinimizer & _minimizer, RooAbsReal * _nll) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitterTool", m_name, "FittingProcedure");
    int _status = 0;
    //TODO : remove it ? 
    if (m_initialHesse) {
       MessageSvc::Info("FittingProcedure", (TString) "InitialHesse");       
       _minimizer.zeroEvalCount();
       _status = _minimizer.hesse();
    }
    MessageSvc::Line();
    MessageSvc::Info("FittingProcedure", (TString) "Minimize", m_minType, m_minAlg);
    MessageSvc::Line();
    //First fit is done without InitStepSize()  , maybe we should ?   
    _minimizer.zeroEvalCount();
    _status = _minimizer.minimize(m_minType, m_minAlg);
    // map <RooRealVar*, double> _floatingParsErrorMap;
    if ((m_reFit || (_status != 0))) {
        vector< int > _strategies = {m_strategyMINUIT};
        if (m_strategyMINUIT == 2) _strategies = {2, 1, 2};
        if (m_strategyMINUIT == 1) _strategies = {1, 2, 1};
        if (m_strategyMINUIT == 0) _strategies = {0, 1, 2};
        MessageSvc::Line();
        MessageSvc::Error("Status", to_string(_status));
        MessageSvc::Line();
        MessageSvc::Info("MINUIT strategies", _strategies);
        MessageSvc::Line();

        // We save the initial errors (step size) for each floating parameter
        // for (auto * _args : _nll->getVariables()){
        //     bool _notConstant = not(_args->isConstant());
        //     bool _isRooRealVar = _args->ClassName() == "RooRealVar";
        //     if (_notConstant && _isRooRealVar) _floatingParsErrorMap[_args] = _args->getError();
        // }
        for (int _strategy : _strategies) {
            MessageSvc::Line();
            MessageSvc::Warning("FittingProcedure", (TString) "Minimize", m_minType, m_minAlg);
            MessageSvc::Warning("Trying Strategy", to_string(_strategy));
            MessageSvc::Line();
            InitStepSize();
            //Reset the FCN to minmize counter of FCN calls.
            _minimizer.zeroEvalCount();
            //Change Strategy fit and re-minimize
            _minimizer.setStrategy(_strategy);
            _status = _minimizer.minimize(m_minType, m_minAlg);
            if (_status == 0) {
                //succeded, go out of this loop
                break;
                if (_strategy != m_strategyMINUIT) {
                    MessageSvc::Line();
                    MessageSvc::Warning("Status", to_string(_status));
                    MessageSvc::Warning("FittingProcedure", (TString) "Minimize", m_minType, m_minAlg);
                    MessageSvc::Warning("Trying Strategy", to_string(m_strategyMINUIT));
                    MessageSvc::Line();
                    //Really InitStepSize here ? 
                    InitStepSize();
                    _minimizer.setStrategy(m_strategyMINUIT);
                    _minimizer.zeroEvalCount();
                    _status = _minimizer.minimize(m_minType, m_minAlg);
                    break;
                }
            } else {
                MessageSvc::Line();
                MessageSvc::Error("Status", to_string(_status));
                MessageSvc::Line();
            }
        }
    }
    
    if (_status == 0) {
        map< TString, double > _lowerLimits;
        map< TString, double > _upperLimits;
        if (m_hesse || m_minos) {
            /*
                Hesse needs to run removing range limits 
            */           
            MessageSvc::Line();
            MessageSvc::Warning("FittingProcedure", (TString) "Removing range limits for all parameters before running HESSE");
            for (auto & keyParPair : m_independentParameterMap) {
                auto _key = keyParPair.first;
                auto _var = keyParPair.second;               
                MessageSvc::Debug("RemoveRangeLimits Before", _var);                
                if (_var->hasMin()){              
                    _lowerLimits[_key] = _var->getMin();
                    _var->removeMin();
                }
                if (_var->hasMax()){ 
                    _upperLimits[_key] = _var->getMax();
                    _var->removeMax();
                }
                MessageSvc::Debug("RemoveRangeLimits After", _var);
            }
            MessageSvc::Line();
        }
        if (m_hesse) {
            MessageSvc::Line();
            MessageSvc::Info("FittingProcedure", (TString) "Hesse");
            MessageSvc::Line();
            _minimizer.zeroEvalCount();
            _status = _minimizer.hesse();
        }
        if( SettingDef::Fit::RatioParsMinos){
            /*
                Minos must run with parameter limits introduced.
            */
            MessageSvc::Warning("Restoring RangeLimits");            
            for (auto & _keyValuePair : _lowerLimits) {
                const auto & _key        = _keyValuePair.first;
                auto         _var        = m_independentParameterMap[_key];
                MessageSvc::Debug("Before (Low Limit)", _var);
                double       _lowerLimit = _lowerLimits[_key];
                _var->setMin(_lowerLimit);
                MessageSvc::Debug("After (Low Limit)", _var);
            }
            for( auto & _keyValuePair : _upperLimits){
                const auto & _key        = _keyValuePair.first;
                auto         _var        = m_independentParameterMap[_key];
                MessageSvc::Debug("Before (Upper Limit)", _var);
                double       _upperLimit = _upperLimits[_key];
                _var->setMax(_upperLimit);
                MessageSvc::Debug("After (Upper Limit)", _var);
            }
            auto _parameterPool = RXFitter::GetParameterPool();    
            auto rJPsis = _parameterPool->GetSingleRatios();    
            auto RXs = _parameterPool->GetDoubleRatios();    
            RooArgList parsForMinos;
            for( auto * p : RXs){
                if(!_nll->dependsOn( *p )) continue;
                TString _name = p->GetName();
                MessageSvc::Info("Adding to Minos the parameter", p);
                parsForMinos.add(*p);
            }
            // MessageSvc::Warning("FittingProcedure", (TString)"Running Minos over RX parameters");            
            // _minimizer.zeroEvalCount();
            // _status = _minimizer.minos(parsForMinos);
        }
        //Minos must run with parameters ranges restored!
        if (m_minos && SettingDef::Fit::scan1DParameter != "" ){
            /*
                Minos must run with parameter limits introduced.
            */
            MessageSvc::Warning("Restoring RangeLimits");            
            for (auto & _keyValuePair : _lowerLimits) {
                const auto & _key        = _keyValuePair.first;
                auto         _var        = m_independentParameterMap[_key];
                MessageSvc::Debug("Before (Low Limit)", _var);
                double       _lowerLimit = _lowerLimits[_key];
                _var->setMin(_lowerLimit);
                MessageSvc::Debug("After (Low Limit)", _var);
            }
            for( auto & _keyValuePair : _upperLimits){
                const auto & _key        = _keyValuePair.first;
                auto         _var        = m_independentParameterMap[_key];
                MessageSvc::Debug("Before (Upper Limit)", _var);
                double       _upperLimit = _upperLimits[_key];
                _var->setMax(_upperLimit);
                MessageSvc::Debug("After (Upper Limit)", _var);
            }
            //============================================================================
            //If you run Likelihood scans 1D for some specific parameters (rJPsi only), run Minos over it to define the ranges for the later scan!
            //TODO: if scan over other parameters != from r-ratio related , code has to be changed
            //============================================================================
            MessageSvc::Line();
            MessageSvc::Info("FittingProcedure", (TString) "Minos (over only r-Ratio parameters from scan1DParameter name!)");
            MessageSvc::Line();
            auto _parameterPool = RXFitter::GetParameterPool();    
            auto rJPsis = _parameterPool->GetSingleRatios();    
            auto RXs = _parameterPool->GetDoubleRatios();    
            RooArgList parsForMinos;
            for( auto * p : rJPsis){   
                if(!_nll->dependsOn( *p )) continue;
                TString _name = p->GetName();
                if(_name == SettingDef::Fit::scan1DParameter){
                    MessageSvc::Info("Adding to Minos the parameter", p);
                    parsForMinos.add(*p);                
                }                
            }
            for( auto * p : RXs){
                if(!_nll->dependsOn( *p )) continue;
                TString _name = p->GetName();
                if( _name == SettingDef::Fit::scan1DParameter){		
                    MessageSvc::Info("Adding to Minos the parameter", p);
                    parsForMinos.add(*p);                
                }
            }
            // if( parsForMinos.getSize() != 0){
            //     MessageSvc::Warning("FittingProcedure", (TString)"NO Minos running over some parameters");            
            //     _minimizer.zeroEvalCount();
            //     _status = _minimizer.minos(parsForMinos);
            //     //Log error up/down for minos errors....
            // }else{
            //     MessageSvc::Warning("FittingProcedure", (TString)"Minos didn't run since parameters (r-Ratios) are not in the likelihood)");            
            // }
        }
        if (m_hesse || m_minos) {
             MessageSvc::Warning("Restoring RangeLimits");            
             //NOTE : some parameter has lower but no upper limit etc...
             //Deal with bounds independently         
             for (auto & _keyValuePair : _lowerLimits) {
                 const auto & _key        = _keyValuePair.first;
                 auto         _var        = m_independentParameterMap[_key];
                 MessageSvc::Debug("Before (Low Limit)", _var);
                 double       _lowerLimit = _lowerLimits[_key];
                 _var->setMin(_lowerLimit);
                 MessageSvc::Debug("After (Low Limit)", _var);
             }
             for( auto & _keyValuePair : _upperLimits){
                 const auto & _key        = _keyValuePair.first;
                 auto         _var        = m_independentParameterMap[_key];
                 MessageSvc::Debug("Before (Upper Limit)", _var);
                 double       _upperLimit = _upperLimits[_key];
                 _var->setMax(_upperLimit);
                 MessageSvc::Debug("After (Upper Limit)", _var);
             }
        }
    } else {
        MessageSvc::Line();
        MessageSvc::Error("Status", to_string(_status));
        MessageSvc::Line();
    }
    if (m_fitResults != nullptr) {
        delete m_fitResults;
        m_fitResults = nullptr;
    }
    m_fitResults = _minimizer.save(m_name + "_FittingProcedure", "Result of fit of p.d.f for ");
    if (!m_fitResults) MessageSvc::Error("FittingProcedure", (TString) "Invalid FitResults", "EXIT_FAILURE");
    m_status = _status;
    return;
}

void FitterTool::DoSPlot() {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitterTool", m_name, "DoSPlot", SettingDef::Fit::useSPlot2 ? "SPlot2" : "");
    MessageSvc::Line();

    if (m_fitManagers.size() == 0) MessageSvc::Error("DoSPlot", (TString) "No FitManager stored in class", "EXIT_FAILURE");
    if (!m_isInitialized) MessageSvc::Error("FitterTool not initialized", "EXIT_FAILURE");

    auto _start = chrono::high_resolution_clock::now();

    for (auto & _fitManager : m_fitManagers) {
        MessageSvc::Info("Loading FitManager", _fitManager.first);

        for (auto & _holder : _fitManager.second.manager->Holders()) {
            MessageSvc::Info("Loading FitHolder", _holder.first);

            if (_holder.second.GetDataSet() == nullptr) MessageSvc::Error("DoSPlot", (TString) "Only works with RooDataSet", "EXIT_FAILURE");

            MessageSvc::Info("DoSPlot", (TString) "Reset Yields");
            _holder.second.SignalComponent().yield = new RooRealVar(_holder.second.SignalComponent().yield->GetName(), (TString) "N_{" + _holder.second.SignalComponent().yield->GetName() + "}", _holder.second.SignalComponent().yield->getVal());
            for (auto & _bkg : _holder.second.BackgroundComponents()) {
                _bkg.second.yield = new RooRealVar(_bkg.second.yield->GetName(), (TString) "N_{" + _bkg.second.yield->GetName() + "}", _bkg.second.yield->getVal());
                // new const bkg yields
                if (SettingDef::Fit::useSPlot2) { ((RooRealVar *) _bkg.second.yield)->setConstant(); }
            }
            if (SettingDef::Fit::useSPlot2) {
                // constrain background yields to simultaneous fitted values, do re-init only signal yield
                MessageSvc::Warning("DoSPlot", (TString) "Background Yields constrained ");
                _holder.second.InitRanges(true, false);
            } else {
                _holder.second.InitRanges(true, true);
            }
            _holder.second.Init();

            MessageSvc::Info("DoSPlot", (TString) "SetConstantPars");
            vector< string > _names = {((TString) _holder.second.SignalComponent().yield->GetName()).Data()};
            for (auto & _bkg : _holder.second.BackgroundComponents()) { _names.push_back(((TString) _bkg.second.yield->GetName()).Data()); }
            if (_names.size() != 0) _holder.second.SetConstantModelExceptPars(_names);
            _holder.second.PrintModelParameters();
            MessageSvc::Info("DoSPlot", (TString) "ReFit");
            _holder.second.SetName(_holder.second.Name() + "_SPlot");
            _holder.second.CreateFitter();
            _holder.second.Fit();

            MessageSvc::Line();
            MessageSvc::Info("DoSPlot", (TString) "SPlot");

            RooArgSet * _yields = new RooArgSet(*_holder.second.SignalComponent().yield);
            for (auto & _bkg : _holder.second.BackgroundComponents()) { _yields->add(*_bkg.second.yield); }
            MessageSvc::Info("DoSPlot", _yields);

            if (SettingDef::Fit::useSPlot2) {
                // SPlot constrained yields....
                RooArgSet * _constyields = new RooArgSet();
                for (auto & _bkg : _holder.second.BackgroundComponents()) { _constyields->add((*_bkg.second.yield)); }
                MessageSvc::Info("Const Yields", _constyields);

                auto * _sPlot = new SPlot2(_holder.first + "_SPlot", _holder.first + "_SPlot", *_holder.second.GetDataSet(), _holder.second.GetModel(), *_yields, *_constyields);

                RooArgList _sWeights = (RooArgList) _sPlot->GetSWeightVars();

                MessageSvc::Line();
                MessageSvc::Info("DoSPlot", _sPlot->GetSDataSet());
                MessageSvc::Info("DoSPlot", &_sWeights);

                MessageSvc::Line();
                TIterator *  _it = _yields->createIterator();
                RooRealVar * _arg;
                while ((_arg = (RooRealVar *) _it->Next())) {
                    MessageSvc::Info("DoSPlot", _arg);
                    MessageSvc::Info("DoSPlot", (TString) "GetYieldFromSWeight", to_string(_sPlot->GetYieldFromSWeight(_arg->GetName())));
                }

                // TString _nameSPlot = "TupleSPlot" + SettingDef::separator + _holder.first;
                TString _nameSPlot = "TupleSPlot";
                if (_holder.first.Contains(to_string(Analysis::MM)))
                    _nameSPlot += SettingDef::separator + to_string(Analysis::MM);
                else if (_holder.first.Contains(to_string(Analysis::EE)))
                    _nameSPlot += SettingDef::separator + to_string(Analysis::EE);
                else if (_holder.first.Contains(to_string(Analysis::ME)))
                    _nameSPlot += SettingDef::separator + to_string(Analysis::ME);
                else
                    MessageSvc::Error("DoSPlot", _nameSPlot, "invalid name", "EXIT_FAILURE");
                _nameSPlot += ".root";

                TFile _tFile(_nameSPlot, to_string(OpenMode::RECREATE));

                MessageSvc::Info("DoSPlot", (TString) "GetSDataSet");
                TTree * _sTuple = (TTree *) _sPlot->GetSDataSet()->GetClonedTree();   // see https://sft.its.cern.ch/jira/browse/ROOT-9408
                if (_sTuple != nullptr) {
                    _sTuple->SetDirectory(0);
                    _sTuple->SetName("SPlotTuple");
                    _sTuple->Write(_sTuple->GetName(), TObject::kOverwrite);
                    _tFile.Close();

                    MessageSvc::Line();
                    GetHistogram(*(static_cast< TChain * >(_sTuple)), TCut(""), "", *_holder.second.Configuration().Var());
                    for (int i = 0; i < _sWeights.getSize(); ++i) {
                        TString    _name = ((TString) _sWeights[i].GetName()).ReplaceAll("-", "M");   // WEIRD FEATURE OF GetClonedTree
                        RooRealVar _var(_name, _name, _sTuple->GetMinimum(_name), _sTuple->GetMaximum(_name) * 1.01);
                        GetHistogram(*(static_cast< TChain * >(_sTuple)), (TCut) _name, "", _var);
                    }
                    delete _sTuple;
                } else {
                    MessageSvc::Error("DoSPlot", (TString) "Tuple is nullptr", "EXIT_FAILURE");
                }
            } else {
                // SPlot floating all yields
                auto * _sPlot = new SPlot(_holder.first + "_SPlot", _holder.first + "_SPlot", *_holder.second.GetDataSet(), _holder.second.GetModel(), *_yields);

                RooArgList _sWeights = (RooArgList) _sPlot->GetSWeightVars();

                MessageSvc::Line();
                MessageSvc::Info("DoSPlot", _sPlot->GetSDataSet());
                MessageSvc::Info("DoSPlot", &_sWeights);

                MessageSvc::Line();
                TIterator *  _it = _yields->createIterator();
                RooRealVar * _arg;
                while ((_arg = (RooRealVar *) _it->Next())) {
                    MessageSvc::Info("DoSPlot", _arg);
                    MessageSvc::Info("DoSPlot", (TString) "GetYieldFromSWeight", to_string(_sPlot->GetYieldFromSWeight(_arg->GetName())));
                }

                // TString _nameSPlot = "TupleSPlot" + SettingDef::separator + _holder.first;
                TString _nameSPlot = "TupleSPlot";
                if (_holder.first.Contains(to_string(Analysis::MM)))
                    _nameSPlot += SettingDef::separator + to_string(Analysis::MM);
                else if (_holder.first.Contains(to_string(Analysis::EE)))
                    _nameSPlot += SettingDef::separator + to_string(Analysis::EE);
                else if (_holder.first.Contains(to_string(Analysis::ME)))
                    _nameSPlot += SettingDef::separator + to_string(Analysis::ME);
                else
                    MessageSvc::Error("DoSPlot", _nameSPlot, "invalid name", "EXIT_FAILURE");
                _nameSPlot += ".root";
                TFile _tFile(_nameSPlot, to_string(OpenMode::RECREATE));
                MessageSvc::Info("DoSPlot", (TString) "GetSDataSet");
                TTree * _sTuple = (TTree *) _sPlot->GetSDataSet()->GetClonedTree();   // see https://sft.its.cern.ch/jira/browse/ROOT-9408
                if (_sTuple != nullptr) {
                    _sTuple->SetDirectory(0);
                    _sTuple->SetName("SPlotTuple");
                    _sTuple->Write(_sTuple->GetName(), TObject::kOverwrite);
                    _tFile.Close();
                    MessageSvc::Line();
                    GetHistogram(*(static_cast< TChain * >(_sTuple)), TCut(""), "", *_holder.second.Configuration().Var());
                    for (int i = 0; i < _sWeights.getSize(); ++i) {
                        TString    _name = ((TString) _sWeights[i].GetName()).ReplaceAll("-", "M");   // WEIRD FEATURE OF GetClonedTree
                        RooRealVar _var(_name, _name, _sTuple->GetMinimum(_name), _sTuple->GetMaximum(_name) * 1.01);
                        GetHistogram(*(static_cast< TChain * >(_sTuple)), (TCut) _name, "", _var);
                    }
                    delete _sTuple;
                } else {
                    MessageSvc::Error("DoSPlot", (TString) "Tuple is nullptr", "EXIT_FAILURE");
                }
            }   // End Splotting floating all yields
            MessageSvc::Line();
        }
    }
    auto _stop = chrono::high_resolution_clock::now();
    MessageSvc::Line();
    MessageSvc::Warning("DoSPlot", (TString) "Took", to_string(chrono::duration_cast< chrono::seconds >(_stop - _start).count()), "seconds");
    MessageSvc::Line();
    return;
}

void FitterTool::PrintFitter() {
    cout << YELLOW;
    MessageSvc::Line();
    MessageSvc::Print("FitterTool", m_name);
    MessageSvc::Line();
    if (m_fitManagers.size() != 1) {
        cout << "| Simultaneous fit among different FitManagers" << endl;
        cout << "| Likelihood defined as the sum of each contributions" << endl;
    }
    for (const auto & _fitManager : m_fitManagers) {
        cout << "| \t Manager " << _fitManager.first << endl;
        for (const auto & _fitInfo : _fitManager.second.FitInfo) {
            cout << "| \t \t " << _fitInfo.first << " added  " << TString(_fitInfo.second.binned ? " Binned " : " Unbinned ") << endl;
            cout << "| \t \t \t [nBins, min, max] " << _fitInfo.second.var->GetName() << " [" << _fitInfo.second.var->getBins() << ", " << _fitInfo.second.var->getBinning().lowBound() << ", " << _fitInfo.second.var->getBinning().highBound() << "]" << endl;
        }
    }
    MessageSvc::Line();
    cout << RESET;

    return;
}

void FitterTool::PrintOptList() {
    if (m_optList.subArgs().GetSize() > 0) {
        MessageSvc::Line();
        MessageSvc::Debug("FitterTool", m_name, "PrintOptList", to_string(m_optList.subArgs().GetSize()));
        for (int i = 0; i < m_optList.subArgs().GetSize(); ++i) { MessageSvc::Debug(((RooCmdArg *) (m_optList.subArgs().At(i)))->opcode(), ((RooCmdArg *) (m_optList.subArgs().At(i)))->getInt(0)); }
        MessageSvc::Line();
    } else {
        MessageSvc::Warning("PrintOptList", (TString) "Empty OptList");
    }
    MessageSvc::Debug("MinimizerType", (TString)m_minType);
    return;
}

void FitterTool::PrintResults(ostream & _stream) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitterTool", m_name, "PrintResults");
    MessageSvc::Line();
    cout << YELLOW;
    PrintFitter();
    cout << YELLOW;
    m_fitResults->Print("v");
    /*
    RooFitResult::covQual()
        0 Not calculated at all
        1 Diagonal approximation only, not accurate
        2 Full matrix, but forced positive - definite
        3 Full accurate covariance matrix (After MIGRAD, this is the indication of normal convergence.)
    RooFitResult::status()
        status = 0    : OK
        status = 1    : Covariance was mad  epos defined
        status = 2    : Hesse is invalid
        status = 3    : Edm is above max
        status = 4    : Reached call limit
        status = 5    : Any other failure
    MINUIT
        ierflg 0      : command executed normally
        ierflg 1      : command is blank, ignored
        ierflg 2      : command line unreadable, ignored
        ierflg 3      : unknown command, ignored
        ierflg 4      : abnormal termination (e.g., MIGRAD not converged)
        ierflg 9      : reserved
        ierflg 10     : END command
        ierflg 11     : EXIT or STOP command
        ierflg 12     : RETURN command
    HESSE
        status = 1    : hesse failed
        status = 2    : matrix inversion failed
        status = 3    : matrix is not pos defined
    MINOS
        status = 1    : maximum number of function calls exceeded when running for lower error
        status = 2    : maximum number of function calls exceeded when running for upper error
        status = 3    : new minimum found when running for lower error
        status = 4    : new minimum found when running for upper error
        status = 5    : any other failure
    */
    if ((m_fitResults->status() == 0) && (m_fitResults->covQual() == 3))
        cout << YELLOW;
    else
        cout << RED;

    MessageSvc::Line(_stream);
    MessageSvc::Print(_stream, " Status", to_string(m_fitResults->status()));
    MessageSvc::Print(_stream, " CovQual", to_string(m_fitResults->covQual()));
    MessageSvc::Print(_stream, " EDM", to_string(m_fitResults->edm()));
    MessageSvc::Print(_stream, " MinNLL", to_string(m_fitResults->minNll()));
    MessageSvc::Print(_stream, " InvalidNLL", to_string(m_fitResults->numInvalidNLL()));
    for (uint i = 0; i < m_fitResults->numStatusHistory(); ++i) { MessageSvc::Print(_stream, (TString) " " + m_fitResults->statusLabelHistory(i), to_string(m_fitResults->statusCodeHistory(i))); }
    MessageSvc::Line(_stream);
    cout << RESET;

    return;
}

void FitterTool::SaveResults(TString _name) {
    if (!SettingDef::IO::outDir.EndsWith("/")) SettingDef::IO::outDir += "/";
    if (_name != "") _name = "_" + _name;
    _name = SettingDef::IO::outDir + m_name + _name + "_Results.log";

    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitterTool", m_name, "SaveResults", _name);
    MessageSvc::Line();

    auto _file = ofstream(_name);
    m_fitResults->printMultiline(_file, 1111, kTRUE, "\t");
    _file.close();

    if (m_status == 0) {
        SaveToTEX(m_fitResults, _name, m_minos);
        SaveToYAML(m_fitResults, _name);
        // SaveDOTs();
    }
    
    if (m_saveAllExt) {
        _name.ReplaceAll(".log", ".root");
        auto _tFile = TFile::Open(_name, to_string(OpenMode::RECREATE));
        _tFile->cd();
        m_fitResults->Write();
        RooRealVar llOffset("nLLOffset", "nLLOffset", m_offset );
        llOffset.Write();
        IOSvc::CloseFile(_tFile);
    }



    return;
}

void FitterTool::PlotResults() {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitterTool", m_name, "PlotResults");
    MessageSvc::Line();
    bool _singleFit = false;
    for (auto & _fitManager : m_fitManagers) {
        MessageSvc::Info("Loading FitManager", _fitManager.first);
        for (auto & _fitInfo : _fitManager.second.FitInfo) {
            MessageSvc::Info("Loading FitInfo", _fitInfo.first);
            TCanvas _canvas("canvas", "canvas", 1200, 900);
            TPad *  _plotPad = new TPad("plotPad", "", 0, .25, 1, 1);
            TPad *  _resPad  = new TPad("resPad", "", 0, 0, 1, .25);
            Plotting::ConfigurePadPlot(_plotPad);
            Plotting::ConfigurePullPlot(_resPad);
            _plotPad->Draw();
            _resPad->Draw();
            RooRealVar * _var = _fitInfo.second.var;
            if (_var == nullptr) MessageSvc::Error("PlotResults", "SOMETHING WENT TERRIBLY WRONG with VAR passed around", "EXIT_FAILURE");
            //----- pad for plotting
            _plotPad->cd();
            TString _title = _fitInfo.second.binned ? "Binned fit" : "Unbinned fit";
            _title += m_sumW2Error ? " Weighted" : "";
            
            double _varMin = _var->getMin();
            double _varMax = _var->getMax();        
            double _bin_width  =  (_varMax-_varMin)/_var->getBins();
            if( !_fitInfo.second.ismc){
                if( _fitInfo.first.Contains("jps") || _fitInfo.first.Contains("psi") ) _bin_width  =  32;  //assumes it's the B mass we are fitting 32MeV per Bin
                else  _bin_width  =  32;
                // else  _bin_width  =  25;
            }
            int nBinsPlot =  std::floor( (_varMax - _varMin )/_bin_width);
            std::cout<<"Plotting " << _fitInfo.first << " with [ "<< _varMin << " , " << _varMax << " ], bin width ="<< _bin_width << std::endl;
            RooPlot * _frame = _var->frame(Title(" "), Bins( nBinsPlot));
            Plotting::CustomizeRooPlot(_frame);
            int _nLabels = _fitInfo.second.components.size() + 1;
            TLegend *_legend = Plotting::GetLegend( _nLabels);
            double _sumEntries = _fitInfo.second.dataset->sumEntries();
            if (_fitInfo.second.extraRange != "") _sumEntries = _fitInfo.second.dataset->sumEntries("1", _fitInfo.second.extraRange);
            if (m_sumW2Error) {
                if (SettingDef::Fit::blindYield && (_fitInfo.second.extraRange != "")) _fitInfo.second.dataset->plotOn(_frame, Name("data"), DataError(RooAbsData::SumW2), CutRange(_fitInfo.second.extraRange));
                else                                                                   _fitInfo.second.dataset->plotOn(_frame, Name("data"), DataError(RooAbsData::SumW2));
            } else {
                if (SettingDef::Fit::blindYield && (_fitInfo.second.extraRange != "")) _fitInfo.second.dataset->plotOn(_frame, Name("data"), CutRange(_fitInfo.second.extraRange));
                else _fitInfo.second.dataset->plotOn(_frame, Name("data"));
            }
            TString _label_in_plot = _fitInfo.second.ismc ? "MC" : "Real Data";
            _label_in_plot +=  _fitInfo.second.dataset->isNonPoissonWeighted() ? " (w)": "";
            _legend->AddEntry( _frame->findObject("data"),_label_in_plot.Data() , "P");
            if (_fitInfo.second.extraRange != "") {
                if (SettingDef::Fit::blindYield)
                    _fitInfo.second.fullmodel->plotOn(_frame, LineColor(m_colors[0]), LineWidth(5), Precision(1E-5), Name("model"), NormRange(_fitInfo.second.extraRange), Normalization(_sumEntries, RooAbsReal::NumEvent));
                else
                    _fitInfo.second.fullmodel->plotOn(_frame, LineColor(m_colors[0]), LineWidth(5), Precision(1E-5), Name("model"), NormRange(_fitInfo.second.extraRange));
            } else {
                _fitInfo.second.fullmodel->plotOn(_frame, LineColor(m_colors[0]), LineWidth(5), Precision(1E-5), Name("model"));
            }
            RooPlot * _framePull = _var->frame(Title("Pulls"));
            auto * dataHist = (RooHist *) _frame->getHist("data");
            if (_fitInfo.second.extraRange != "") {
                auto nranges = _fitInfo.second.extraRange.Tokenize(",")->GetEntries();
                MessageSvc::Info("Extra ranges", to_string(nranges));
                for (int i = 0; i < nranges; ++i) {
                    _frame->Print("V");
                    auto * curve = (RooCurve *) _frame->getObject(i + 1);
                    auto * _hres = dataHist->makePullHist(*curve, true);
                    _hres->SetFillColor(kGray + 1);
                    _hres->SetLineColor(kGray + 1);
                    _hres->SetMarkerColor(kGray + 1);
                    _hres->SetMarkerSize(0);
                    _framePull->addPlotable(_hres, "BX");
                }
            } else {
                RooHist * _hPulls = _frame->pullHist(0,0,true); //true = use Integral of curve in the bin width( else curve->interpolate() vs curve->)
                _hPulls->SetFillColor(kGray + 1);
                _hPulls->SetLineColor(kGray + 1);
                _hPulls->SetMarkerColor(kGray + 1);
                _hPulls->SetMarkerSize(0);
                _framePull->addPlotable(_hPulls, "BX");
            }
            bool _hasNegativeYield = false; 
            if (m_isFitTo) {
                RooAddPdf * _model = dynamic_cast< RooAddPdf * >(_fitInfo.second.fullmodel);                
                if (_model != nullptr) {   // This MC Shape PDF is a RooAddPdf object, decompose to sub-components for plotting
                    RooArgList _pdflist = _model->pdfList();

                    for (int i = 0; i < _pdflist.getSize(); ++i) {
                        _fitInfo.second.fullmodel->plotOn(_frame, Components(*_pdflist.at(i)), DrawOption("L"), LineColor(m_colors[i + 1]), LineStyle(kDashed), LineWidth(5), Name(_pdflist.at(i)->GetName()), MoveToBack());
                        _legend->AddEntry(_frame->findObject(_pdflist.at(i)->GetName()), _pdflist.at(i)->GetName(), "L");
                    }
                }else{
                    //Give a try to RooRealSumPdf 
                    RooRealSumPdf * _modelSum = dynamic_cast< RooRealSumPdf * >(_fitInfo.second.fullmodel);
                    if( _modelSum != nullptr){
                        RooArgList _pdflist = _modelSum->funcList();
                        for (int i = 0; i < _pdflist.getSize(); ++i) {
                            _fitInfo.second.fullmodel->plotOn(_frame, Components(*_pdflist.at(i)), DrawOption("L"), LineColor(m_colors[i + 1]), LineStyle(kDashed), LineWidth(5), Name(_pdflist.at(i)->GetName()), MoveToBack());
                            _legend->AddEntry(_frame->findObject(_pdflist.at(i)->GetName()), _pdflist.at(i)->GetName(), "L");
                        }
                    }
                }                            
            } else {
                vector< pair< RooAbsPdf *, TString > > _pdfs;
                map< TString, double > _yields;
                for (const auto & component : _fitInfo.second.components) {
                    if (component.first == "Signal") {
                        MessageSvc::Info("PlotResults", (TString) "Integral info Signal");
                        _var->setRange("integralRangeFit", _var->getMin(), _var->getMax()) ;
                        _var->setRange("integralRangeFull", 0, 10000) ;
                        RooAbsReal * _integralFit  = component.second->createIntegral(*_var, RooFit::NormSet(*_var), RooFit::Range("integralRangeFit"));
                        RooAbsReal * _integralFull = component.second->createIntegral(*_var, RooFit::NormSet(*_var), RooFit::Range("integralRangeFull"));
                        double _integralFit_val    = _integralFit->getVal();
                        double _integralFull_val   = _integralFull->getVal();
                        MessageSvc::Info("PlotResults", (TString) "Integral Signal fit:", to_string(_integralFit_val));
                        MessageSvc::Info("PlotResults", (TString) "Integral Signal full:", to_string(_integralFull_val));
                        MessageSvc::Info("PlotResults", (TString) "Integral Signal ratio:", to_string(_integralFit_val/_integralFull_val));
                        delete _integralFit;
                        delete _integralFull;
                        _yields["Signal"] = _fitInfo.second.yields.at("Signal")->getVal();
                        _hasNegativeYield = _hasNegativeYield || _yields.at("Signal")<0;
                        continue;
                    } else {
                        MessageSvc::Info("PlotResults", (TString) "Integral info", component.first);
                        _var->setRange("integralRangeFit", _var->getMin(), _var->getMax()) ;
                        _var->setRange("integralRange5200", 5200, 10000) ;
                        RooAbsReal * _integralFit  = component.second->createIntegral(*_var, RooFit::NormSet(*_var), RooFit::Range("integralRangeFit"));
                        RooAbsReal * _integralFull = component.second->createIntegral(*_var, RooFit::NormSet(*_var), RooFit::Range("integralRange5200"));
                        double _integralFit_val    = _integralFit->getVal();
                        double _integralFull_val   = _integralFull->getVal();
                        MessageSvc::Info("PlotResults", (TString) "Integral Bkg fit:", to_string(_integralFit_val));
                        MessageSvc::Info("PlotResults", (TString) "Integral Bkg 5200:", to_string(_integralFull_val));
                        MessageSvc::Info("PlotResults", (TString) "Integral Bkg ratio:", to_string(_integralFit_val/_integralFull_val));
                        delete _integralFit;
                        delete _integralFull;
                    }
                    if (component.first.Contains(to_string(Sample::Custom))) {
                        TString _name = ((TString) component.second->GetName()).ReplaceAll(to_string(Sample::Custom) + SettingDef::separator, "");
                        _name         = RemoveStringAfter(_name, SettingDef::separator);
                        _name.ReplaceAll("sig_", "").ReplaceAll("bkg_", "");                        
                        _pdfs.push_back(make_pair(component.second, _name));
                        _yields[_name] = _fitInfo.second.yields.at(component.first)->getVal();
                    } else {
                        _pdfs.push_back(make_pair(component.second, component.first));
                        _yields[component.first] = _fitInfo.second.yields.at(component.first)->getVal();
                    }
                    _hasNegativeYield = _hasNegativeYield || _yields.at(component.first)<0;
                }
                // Sort the background pdfs according to the sorting criteria defined in bkgsortidx (lower number plotted first)
                sort(_pdfs.begin(), _pdfs.end(), [&](pair< RooAbsPdf *, TString > & _pdf1, pair< RooAbsPdf *, TString > & _pdf2) { 
                    return Plotting::bkgsortidx(_pdf1.second) < Plotting::bkgsortidx(_pdf2.second); 
                });

                //Small routine a posteriori to assign a label on the legend for each sub-component in the fit (names matching important!)//
                auto _LABEL_SEARCH_ = []( map<TString, TString> & _myLabels, TString _name){
                    for( auto & el : _myLabels){
                        TString _idLabel = ToLower(el.first);
                        TString _idName  = ToLower(_name);
                        if( _idLabel == _idName) return  el.second;
                    }
                    return _name;
                };   
                auto _COLOR_SEARCH_ = []( map<TString, Int_t> & _myColors, TString _name){
                    for( auto & el : _myColors){
                        TString _idLabel = ToLower(el.first);
                        TString _idName  = ToLower(_name);
                        if( _idLabel == _idName) return  el.second;
                    }
                    return (Int_t)kBlack;
                };
                //Usually Sample::to_string for each component//
                //A bit weak setup , but should work, if not found it's named as usually
                auto _LABEL_PLOT_ = _fitInfo.second.labels;                
                map< TString, TString > _labelsLegend; 
                for( auto & _pdf : _pdfs ){
                    _labelsLegend[_pdf.second] = _LABEL_SEARCH_(_LABEL_PLOT_, _pdf.second);
                }
                //Coloring Stuff
                map< TString, Int_t> _colorsPlot;
                auto _COLOR_PLOT_ = _fitInfo.second.colors;
                int IDX = 1;
                for( auto & _pdf : _pdfs){                
                    auto col = _COLOR_SEARCH_( _COLOR_PLOT_, _pdf.second);
                    if( col == (Int_t)kBlack){
                        _colorsPlot[_pdf.second] = (Int_t)m_colors[IDX++];
                    }else{
                        _colorsPlot[_pdf.second] = col;
                    }
                }

                RooArgSet _pdfList = RooArgSet("pdfList");
                int i = 1;
                for (const auto & _pdf : _pdfs) {
                    _pdfList.add(*_pdf.first);
                    if (_fitInfo.second.extraRange != "") {
                        if (SettingDef::Fit::blindYield) {
                            _fitInfo.second.fullmodel->plotOn(_frame, Components(*_pdf.first), DrawOption("F"), FillColor(_colorsPlot[_pdf.second]), FillStyle(1001), VLines(), LineColor(_colorsPlot[_pdf.second]), LineWidth(5), Name(_pdf.second), NormRange(_fitInfo.second.extraRange), Normalization(_sumEntries, RooAbsReal::NumEvent));
                        } else {
                            _fitInfo.second.fullmodel->plotOn(_frame, Components(*_pdf.first), DrawOption("L"), FillStyle(0), VLines(), LineColor(_colorsPlot[_pdf.second]), LineWidth(5), LineStyle(kDashDotted),    Name(_pdf.second), NormRange(_fitInfo.second.extraRange), Range(_var->getMin(), _var->getMax()));
                            _fitInfo.second.fullmodel->plotOn(_frame, Components(*_pdf.first), DrawOption("F"), FillColor(_colorsPlot[_pdf.second]), FillStyle(1001), VLines(), LineColor(_colorsPlot[_pdf.second]), LineWidth(5), Name(_pdf.second), NormRange(_fitInfo.second.extraRange));
                        }
                    } else {
                        //Plotting assume you sorted to plotting order here !
                        if( !_hasNegativeYield){
                            _fitInfo.second.fullmodel->plotOn(_frame, Components(_pdfList), DrawOption("F"), FillColor(_colorsPlot[_pdf.second]), FillStyle(1001), VLines(), LineColor(_colorsPlot[_pdf.second]), LineWidth(5), Name(_pdf.second), MoveToBack());
                        }else{
                            _pdf.first->plotOn(_frame, DrawOption("L"), FillColor(_colorsPlot[_pdf.second]), FillStyle(1001), VLines(), LineColor(_colorsPlot[_pdf.second]), LineWidth(5), Name(_pdf.second), MoveToBack(), Normalization( _yields.at(_pdf.second), RooAbsReal::NumEvent) );
                        }
                    }
                    i++;
                }
                //Signal --> decompose EE into the 3 Brems fractions summed! 
                if (_fitInfo.second.components["Signal"] != nullptr) {
                    _pdfList.add(*_fitInfo.second.components["Signal"]);
                    if (_fitInfo.second.extraRange != "") {
                        if (SettingDef::Fit::blindYield)
                            _fitInfo.second.fullmodel->plotOn(_frame, Components(*_fitInfo.second.components["Signal"]), DrawOption("L"), LineColor(m_colors[0]), LineStyle(kDashed), LineWidth(5), Name("Signal"), NormRange(_fitInfo.second.extraRange), Normalization(_sumEntries, RooAbsReal::NumEvent));
                        else
                            _fitInfo.second.fullmodel->plotOn(_frame, Components(*_fitInfo.second.components["Signal"]), DrawOption("L"), LineColor(m_colors[0]), LineStyle(kDashDotted), LineWidth(5), Name("Signal"), NormRange(_fitInfo.second.extraRange), Range(_var->getMin(), _var->getMax()));
                    } else {
                        if( !_hasNegativeYield){
                            _fitInfo.second.fullmodel->plotOn(_frame, Components(*_fitInfo.second.components["Signal"]), DrawOption("L"), LineColor(m_colors[0]), LineStyle(kDashed), LineWidth(5), Name("Signal"));
                        }else{
                            _fitInfo.second.components["Signal"]->plotOn( _frame, DrawOption("L"), LineColor(m_colors[0]), LineStyle(kDashed), LineWidth(5), Name("Signal"), Normalization( _yields.at("Signal"), RooAbsReal::NumEvent) );
                        }
                        /*
                        THIS IS NEEDED TO FURTHER DEEP THE COMPONENT IN THE FIT WHEN PLOTTING , uncommend if you want to see how sub-PDFs of signal behaves , 2 nested check here 
                        
                        TString _ADDPDFCLASS_ = "RooAddPdf";
                        if( _fitInfo.second.components["Signal"]->ClassName() == _ADDPDFCLASS_){
                            MessageSvc::Warning("AddPdf Signal, decomposing plot (no labels)");
                            RooArgList _signalPdfList = ((RooAddPdf*)_fitInfo.second.components["Signal"])->pdfList();                            
                            for( int j = 0 ; j < _signalPdfList.getSize(); ++j){
                                _fitInfo.second.components["Signal"]->plotOn( _frame, Components(*_signalPdfList.at(j)), DrawOption("L"), LineColor(m_colors[j+1]) , LineStyle(kDashed), LineWidth(4), Name(_signalPdfList.at(j)->GetName()), MoveToBack() );
                                if( _signalPdfList.at(j)->ClassName() == _ADDPDFCLASS_){
                                    MessageSvc::Warning("AddPdf Signal, Sub-Decomposing plot (no labels)");
                                    RooArgList _signalSubPdfList = ((RooAddPdf*)_signalPdfList.at(j))->pdfList();
                                    for(int k =0 ; k <  _signalSubPdfList.getSize(); ++k ){
                                        _fitInfo.second.components["Signal"]->plotOn( _frame, Components(*_signalSubPdfList.at(k)), DrawOption("L"), LineColor(m_colors[j+1]) , LineStyle(kDashDotted), LineWidth(2), Name(_signalSubPdfList.at(k)->GetName()), MoveToBack() );
                                    }
                                }
                            }
                        }
                        */                        
                    }
                    _legend->AddEntry(_frame->findObject("Signal"), "Signal", "L");
                } else {
                    _legend->AddEntry(_frame->findObject("model"), ((TString) _frame->findObject("model")->GetTitle()).ReplaceAll("Projection of ", ""), "L");
                }

                // Show signal legend first before background legends
                for (const auto & _pdf : _pdfs) {
                    _legend->AddEntry(_frame->findObject(_pdf.second), _labelsLegend[_pdf.second], "F");
                }
            }
            _frame->GetYaxis()->CenterTitle(1);
            _frame->Draw();
            if(!_hasNegativeYield && !_frame->GetMinimum()<0.){
                _frame->SetMinimum(1e-9);
            }
            _legend->Draw("SAME");
            //------ pad for pulls
            _resPad->cd();
            Plotting::CustomizeFramePull(_framePull);
            TAxis * _xAxis   = _framePull->GetXaxis();
            TLine * lc       = new TLine(_xAxis->GetXmin(), 0, _xAxis->GetXmax(), 0);
            TLine * lu       = new TLine(_xAxis->GetXmin(), 3, _xAxis->GetXmax(), 3);
            TLine * ld       = new TLine(_xAxis->GetXmin(), -3, _xAxis->GetXmax(), -3);
            _framePull->Draw();
            lc->SetLineColor(kGray + 2);
            lu->SetLineColor(kGray + 1);
            ld->SetLineColor(kGray + 1);
            lc->SetLineStyle(2);
            lu->SetLineStyle(2);
            ld->SetLineStyle(2);
            lc->Draw("SAME");
            lu->Draw("SAME");
            ld->Draw("SAME");
            if (!SettingDef::IO::outDir.EndsWith("/")) SettingDef::IO::outDir += "/";
            TString _name = SettingDef::IO::outDir + m_name;
            if (_fitManager.first == _fitInfo.first) {
                _singleFit = true;
            } else {
                _singleFit = false;
                _name += "_" + _fitInfo.first;
            }
            _canvas.SaveAs(_name + ".pdf");
            if (m_saveAllExt){
                _canvas.SaveAs(_name + ".root");   //<- to re-work plot offline
                Plotting::SaveAllObjectFromFrame( _frame, _name+".root", OpenMode::UPDATE);   
            }
            _plotPad->cd();
            if( !_hasNegativeYield ){
                _plotPad->SetLogy();
                _plotPad->Update();
                RooHist _hist = (*(RooHist *) _frame->getHist("data"));
                double  _min  = numeric_limits< double >::max();
                for (uint i = 0; i < _hist.GetN(); ++i) {
                    if (_hist.GetY()[i] != 0) _min = TMath::Min(_min, _hist.GetY()[i]);
                }
                if (_min == numeric_limits< double >::max()) _min = 1;
                _frame->SetMinimum(1);
                _canvas.SaveAs(_name + "_log.pdf");
            }
        }
    }
    // Construct 2D color plot of correlation matrix
    // Extend this for 2 variables correlation plots
    // https://root.cern.ch/root/html/tutorials/roofit/rf607_fitresult.C.html

    TH2 * _hCorr = m_fitResults->correlationHist();
    _hCorr->SetTitle("CorrelationMatrix");
    PlotCorrelationMatrix(_hCorr, "_CorrelationMatrix");
    delete _hCorr;

    if (m_fitResults->floatParsFinal().getSize() != 0) {
        RooArgList _floatPars = m_fitResults->floatParsFinal();

        RooArgList _sigYields;
        for (int i = 0; i < _floatPars.getSize(); ++i) {
            if (((TString) _floatPars.at(i)->GetName()).BeginsWith("nsig_")) _sigYields.add(*_floatPars.at(i));
        }
        if ((_sigYields.getSize() > 1) && (_sigYields.getSize() != _floatPars.getSize())) { MakeCorrelationMatrix(_sigYields, "Signal"); }

        RooArgList _bkgYields;
        for (int i = 0; i < _floatPars.getSize(); ++i) {
            if (((TString) _floatPars.at(i)->GetName()).BeginsWith("nbkg_")) _bkgYields.add(*_floatPars.at(i));
        }
        if ((_bkgYields.getSize() > 1) && (_bkgYields.getSize() != _floatPars.getSize())) { MakeCorrelationMatrix(_bkgYields, "Background"); }

        RooArgList _yields;
        RooArgList _pars;
        for (int i = 0; i < _floatPars.getSize(); ++i) {
            if (((TString) _floatPars.at(i)->GetName()).BeginsWith("nsig_") || ((TString) _floatPars.at(i)->GetName()).BeginsWith("nbkg_"))
                _yields.add(*_floatPars.at(i));
            else
                _pars.add(*_floatPars.at(i));
        }
        if ((_yields.getSize() > 1) && (_yields.getSize() != _floatPars.getSize())) { MakeCorrelationMatrix(_yields, "Yields"); }
        if ((_pars.getSize() > 1) && (_pars.getSize() != _floatPars.getSize())) { MakeCorrelationMatrix(_pars, "Parameters"); }
    }
    // if (!_singleFit && (m_status == 0)) PlotSumCategories();
    if (!_singleFit ) PlotSumCategories();

    return;
}

void FitterTool::SaveDOTs() {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitterTool", m_name, "SaveDOTs");
    MessageSvc::Line();

    for (auto & _fitManager : m_fitManagers) {
        MessageSvc::Info("Loading FitManager", _fitManager.first);

        for (auto & _fitInfo : _fitManager.second.FitInfo) {
            MessageSvc::Info("Loading FitInfo", _fitInfo.first);

            if (!SettingDef::IO::outDir.EndsWith("/")) SettingDef::IO::outDir += "/";
            TString _name = SettingDef::IO::outDir + m_name;
            if (_fitManager.first != _fitInfo.first) { _name += "_" + _fitInfo.first; }
            _name += ".dot";
            if (m_isFitTo) {
                RooAddPdf * _model = dynamic_cast< RooAddPdf * >(_fitInfo.second.fullmodel);
                if (_model != nullptr) {   // This MC Shape PDF is a RooAddPdf object, decompose to sub-components for plotting
                    SaveToDOT(_model, _name);
                }
            } else {
                for (const auto & component : _fitInfo.second.components) {
                    if (component.first == "Signal") continue;
                    if (component.first.Contains(to_string(Sample::Custom))) continue;
                    SaveToDOT(component.second, _name);
                }
            }
        }
    }
    MessageSvc::Line();
    return;
}

void FitterTool::PlotCorrelationMatrix(TH2 * _hCorr, TString _corrName) {
    TCanvas _canvas("canvas");
    _canvas.SetLeftMargin(0.25);
    _hCorr->GetYaxis()->LabelsOption("v");
    _hCorr->GetYaxis()->SetLabelSize(0.03);
    _hCorr->GetXaxis()->LabelsOption("v");
    _hCorr->GetXaxis()->SetLabelSize(0.03);
    _hCorr->GetXaxis()->SetLabelOffset(0.1);
    _hCorr->Draw("COLZ TEXT");

    if (!SettingDef::IO::outDir.EndsWith("/")) SettingDef::IO::outDir += "/";
    TString _name = SettingDef::IO::outDir + m_name + _corrName;
    _canvas.SetGrid();
    _canvas.SaveAs(_name + ".pdf");
    if (m_saveAllExt) {
        _canvas.SaveAs(_name + ".root");
        _name       = SettingDef::IO::outDir + m_name + _corrName + "_TH2.root";
        auto _tFile = IOSvc::OpenFile(_name, OpenMode::RECREATE);
        _tFile->cd();
        _hCorr->Write();
        IOSvc::CloseFile(_tFile);
    }
    return;
}

void FitterTool::MakeCorrelationMatrix(const RooArgList & _parameterList, TString _name) {
    if (_name == "") _name = "Reduced";

    auto _reducedCovMatrix = m_fitResults->reducedCovarianceMatrix(_parameterList);

    int _nParameters = _reducedCovMatrix.GetNcols();

    // Gets the uncertainties: square root of the diagonal elements
    vector< double > _uncertainties(_nParameters);
    for (int i = 0; i < _nParameters; i++) { _uncertainties[i] = sqrt(_reducedCovMatrix(i, i)); }

    // Gets the correlation matrix from covariance matrix
    for (int i = 0; i < _nParameters; i++) {
        for (int j = 0; j < _nParameters; j++) { _reducedCovMatrix(i, j) = _reducedCovMatrix(i, j) / (_uncertainties[i] * _uncertainties[j]); }
    }

    TH2D * _hCorr = new TH2D(_name + "CorrelationMatrix", _name + "CorrelationMatrix", _nParameters, 0, _nParameters, _nParameters, 0, _nParameters);

    for (int i = 0; i < _nParameters; i++) {
        for (int j = 0; j < _nParameters; j++) { _hCorr->Fill(i + 0.5, _nParameters - j - 0.5, _reducedCovMatrix(i, j)); }
        _hCorr->GetXaxis()->SetBinLabel(i + 1, _parameterList.at(i)->GetName());
        _hCorr->GetYaxis()->SetBinLabel(_nParameters - i, _parameterList.at(i)->GetName());
    }
    _hCorr->SetMinimum(-1);
    _hCorr->SetMaximum(+1);

    PlotCorrelationMatrix(_hCorr, "_CorrelationMatrix" + _name);
    delete _hCorr;
    return;
}

void FitterTool::PlotSumCategories() {
    if (SettingDef::Fit::plotSumCategories) {
        MessageSvc::Line();
        MessageSvc::Info(Color::Cyan, "FitterTool", m_name, "PlotSumCategories");
        MessageSvc::Line();

        vector< TString > _sumFitManagers = {};        
        for (const auto & _fitManager : m_fitManagers) {
            vector< double > mins_MM, maxs_MM;
            vector< double > mins_EE, maxs_EE;
            bool             validEE = false;
            bool             validMM = false;
            int              nEE     = 0;
            int              nMM     = 0;
            
            for (const auto & _fitInfo : _fitManager.second.FitInfo) {
                if (_fitInfo.first.Contains(to_string(Analysis::MM))) {
                    nMM++;
                    mins_MM.push_back(_fitInfo.second.var->getMin());
                    maxs_MM.push_back(_fitInfo.second.var->getMax());
                } else if (_fitInfo.first.Contains(to_string(Analysis::EE))) {
                    nEE++;
                    mins_EE.push_back(_fitInfo.second.var->getMin());
                    maxs_EE.push_back(_fitInfo.second.var->getMax());
                } else {
                    MessageSvc::Error("PlotSumCategories", (TString) "SOMETHING went wrong var check", "EXIT_FAILURE");
                }
            }
            if (nMM > 0 && count(begin(mins_MM), end(mins_MM), mins_MM.front()) == mins_MM.size() && count(begin(maxs_MM), end(maxs_MM), maxs_MM.front()) == maxs_MM.size()) validMM = true;
            if (nMM == 0) validMM = true;
            if (nEE > 0 && count(begin(mins_EE), end(mins_EE), mins_EE.front()) == mins_EE.size() && count(begin(maxs_EE), end(maxs_EE), maxs_EE.front()) == maxs_EE.size()) validEE = true;
            if (nEE == 0) validEE = true;

            if (validEE && validMM) _sumFitManagers.push_back(_fitManager.first);
        }
        for (auto & _fitManager : m_fitManagers) {
            MessageSvc::Info("Loading FitManagers", _fitManager.first);
            if (find(_sumFitManagers.begin(), _sumFitManagers.end(), _fitManager.first) == _sumFitManagers.end()) {
                MessageSvc::Info("INVALID SUM PLOT CATEGORY, RANGE IS NOT CONSISTENT!, SKIPPING");
                continue;
            }
            RooDataSet * combdataMM = MergedPlot::MergeDataSet< Analysis::MM >(_fitManager.second);
            if (combdataMM != nullptr) {
                auto pdfs_MM   = MergedPlot::GetPDFsCoefsCategories< Analysis::MM >(_fitManager.second);
                auto colors_MM = MergedPlot::GetColorsCategories< Analysis::MM>(_fitManager.second);
                RooArgList all_pdfsMM  = MergedPlot::AllPDFsFromDecomposition(pdfs_MM);
                RooArgList all_coefsMM = MergedPlot::AllCoefsFromDecomposition(pdfs_MM);                
                //Make the RooAddPdf here for the "full Model"
                RooAddPdf * merged_pdfMM = new RooAddPdf("merged_pdfMM", "merged_pdfMM", all_pdfsMM, all_coefsMM);
                RooRealVar * varMM = nullptr;
                for (auto & _fitInfo : _fitManager.second.FitInfo) {
                    if (_fitInfo.first.Contains(to_string(Analysis::MM))) {
                        varMM = _fitInfo.second.var;
                        break;
                    }
                }
                gStyle->SetOptTitle(0);

                TCanvas * tp_MM = MergedPlot::PlotAndResidualPad< Analysis::MM >(_fitManager.second, combdataMM, merged_pdfMM, pdfs_MM, varMM, _fitManager.first, colors_MM);
                if (tp_MM != nullptr) {
                    if (!SettingDef::IO::outDir.EndsWith("/")) SettingDef::IO::outDir += "/";
                    TString _name = SettingDef::IO::outDir + m_name + "_" + _fitManager.first + "_CombinedMM";
                    tp_MM->Draw();
                    tp_MM->SaveAs(_name + ".pdf");
                    tp_MM->SaveAs(_name + ".root");   //<- to re-work plot offline
                }

                delete merged_pdfMM;
                delete combdataMM;
                delete tp_MM;
            }

            RooDataSet * combdataEE = MergedPlot::MergeDataSet< Analysis::EE >(_fitManager.second);
            if (combdataEE != nullptr) {
                auto  pdfs_EE  = MergedPlot::GetPDFsCoefsCategories< Analysis::EE >(_fitManager.second);
                auto colors_EE = MergedPlot::GetColorsCategories< Analysis::EE>(_fitManager.second);

                RooArgList all_pdfsEE  = MergedPlot::AllPDFsFromDecomposition(pdfs_EE);
                RooArgList all_coefsEE = MergedPlot::AllCoefsFromDecomposition(pdfs_EE);

                RooAddPdf * merged_pdfEE = new RooAddPdf("merged_pdfEE", "merged_pdfEE", all_pdfsEE, all_coefsEE);

                RooRealVar * varEE = nullptr;
                for (const auto & _fitInfo : _fitManager.second.FitInfo) {
                    if (_fitInfo.first.Contains(to_string(Analysis::EE))) {
                        varEE = _fitInfo.second.var;
                        break;
                    }
                }
                gStyle->SetOptTitle(0); //REMOVE THE "A RooPlot of"
                TCanvas * tp_EE = MergedPlot::PlotAndResidualPad< Analysis::EE >(_fitManager.second, combdataEE, merged_pdfEE, pdfs_EE, varEE, _fitManager.first, colors_EE);
                if (tp_EE != nullptr) {
                    if (!SettingDef::IO::outDir.EndsWith("/")) SettingDef::IO::outDir += "/";
                    TString _name = SettingDef::IO::outDir + m_name + "_" + _fitManager.first + "_CombinedEE";
                    tp_EE->Draw();
                    tp_EE->SaveAs(_name + ".pdf");
                    tp_EE->SaveAs(_name + ".root");   //<- to re-work plot offline
                }
                delete merged_pdfEE;
                delete combdataEE;
                delete tp_EE;
            }
        }
    }
    return;
}
void FitterTool::PlotProfileLikelihoodROOT(const TString &  _varForLikelihoodName , double _minRange, double _maxRange){
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitterTool", m_name, "PlotProfileLikelihood");
    MessageSvc::Line();
    if (!m_isInitialized) MessageSvc::Error("FitterTool not initialized", "EXIT_FAILURE");
    double min = _minRange;
    double max = _maxRange;
    if( SettingDef::Fit::minValScan  > 0 && SettingDef::Fit::maxValScan  >0){
        min=  SettingDef::Fit::minValScan;
        max=  SettingDef::Fit::maxValScan;
    }
    //Do we want to change default minimizer ( minuit vs minuit2 ? ) let's try this like that
    if( min > max){ 
        MessageSvc::Error("Invalid range for likelihood scan, check logic", "","EXIT_FAILURE");
    }
    MessageSvc::Warning("PlotProfileLikelihood", TString::Format("( Min , Max ) = %.2f , %.2f", min , max ) );
    ROOT::Math::MinimizerOptions::SetDefaultMinimizer(m_minType,m_minAlg);
    RooArgSet _nLL;
    CreateNLL(_nLL);
    RooAddition _nllSimultaneous("nllCombined_SCAN", "-log(likelihood)", _nLL);
    _nllSimultaneous.enableOffsetting(m_offsetLikelihood);
    auto _varForLikelihood = m_independentParameterMap[_varForLikelihoodName];
    double _currentMin = _varForLikelihood->getMin();
    double _currentMax = _varForLikelihood->getMax();
    TCanvas _canvas("canvas","canvas", 800,600);
    RooPlot * _frame = _varForLikelihood->frame(Bins(SettingDef::Fit::nScanPointsProfile), Range(min, max), Title(""));
    _frame->SetMinimum(0);
    _frame->SetMaximum(30);
    //ROOT::RooAbsReal::defaultIntegratorConfig().getConfigSection("RooMCIntegrator").setRealValue("nIntPerDim",50000)
    RooProfileLL * pll_var = (RooProfileLL *) _nllSimultaneous.createProfile(*_varForLikelihood);
    pll_var->setAlwaysStartFromMin(SettingDef::Fit::startLLScanFromMin);
    // NB : Normalization(2) required to have -LL --> -2LL (create Profile returns log(Likelihood), not 2 * log(Likelihood) )
    //nll->plotOn(frame2, ShiftToZero(), Normalization(2), Precision(-1));
    pll_var->plotOn(_frame, LineColor(kRed), Precision(-1) , ShiftToZero(), LineWidth(2), Name(_varForLikelihood->GetName()) , Normalization(2) ); 
    auto oneSigma   = RooConst(1);
    auto twoSigma   = RooConst(4);
    auto threeSigma = RooConst(9);
    auto fiveSigma  = RooConst(25);
    oneSigma.plotOn(   _frame, LineColor(kBlack), LineWidth(1), LineStyle(kDashed), MoveToBack(), Name("1sigmaBar") );
    twoSigma.plotOn(   _frame, LineColor(kBlack), LineWidth(1), LineStyle(kDashed), MoveToBack(), Name("2sigmaBar"));
    threeSigma.plotOn( _frame, LineColor(kBlack), LineWidth(1), LineStyle(kDashed), MoveToBack(), Name("3sigmaBar"));        
    fiveSigma.plotOn(  _frame, LineColor(kBlack) , LineWidth(1),LineStyle(kDashed), MoveToBack(), Name("5sigmaBar"));
    _frame->Draw();
    _frame->GetYaxis()->SetTitle("-2 #Delta log(#it{L})");
    _frame->GetYaxis()->CenterTitle();
    _frame->GetXaxis()->CenterTitle();
    if (!SettingDef::IO::outDir.EndsWith("/")) SettingDef::IO::outDir += "/";
    TString _name = SettingDef::IO::outDir + m_name + "_ProfileLikelihood_for_" + _varForLikelihood->GetName();
    _canvas.SaveAs(_name + ".pdf");
    _canvas.SaveAs(_name + ".root");
    Plotting::SaveAllObjectFromFrame( _frame, _name+".root", OpenMode::UPDATE);
    _varForLikelihood->setRange( _currentMin, _currentMax);
    DeleteNLL();
    return;
}


void FitterTool::PlotProfileLikelihood(const TString &  _varForLikelihoodName , double _minRange, double _maxRange){
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitterTool", m_name, "PlotProfileLikelihood");
    MessageSvc::Line();
    if (!m_isInitialized) MessageSvc::Error("FitterTool not initialized", "EXIT_FAILURE");
    if(m_independentParameterMap.find(_varForLikelihoodName) == m_independentParameterMap.end()) MessageSvc::Error("Invalid, found twice in parameter map independent", "","EXIT_FAILURE");
    int nStepScan = SettingDef::Fit::nScanPointsProfile;
    double MIN    = _minRange;
    double MAX    = _maxRange;
    if( SettingDef::Fit::minValScan  > 0 && SettingDef::Fit::maxValScan  >0){
        MIN=  SettingDef::Fit::minValScan;
        MAX=  SettingDef::Fit::maxValScan;
    }

    vector<double> _SCANVALUES_ = LineSpace<double>( MIN, MAX, nStepScan);
    //======================================================================================================
    // Cache in the map the current status of parameters
    //======================================================================================================
    map< TString, double > _values_AtMin, _values_AtMinUseLoop;
    map< TString, double > _errors_AtMin, _errors_AtMinUseLoop;
    for (auto & keyParPair : m_independentParameterMap) {
        auto _key = keyParPair.first;
        auto _var = keyParPair.second;
        if( _var->isConstant()) continue;
        _values_AtMin[_key] = _var->getVal();
        _errors_AtMin[_key] = _var->getError();  
        _values_AtMinUseLoop[_key] = _var->getVal();
        _errors_AtMinUseLoop[_key] = _var->getError();              
    }
    double _MINIMUMPAR_      = m_independentParameterMap[_varForLikelihoodName]->getVal(); //retrieve the value at min 
    double _MINIMUMPARERR_   = m_independentParameterMap[_varForLikelihoodName]->getError(); //retrieve the error
    // _SCANVALUES_.push_back( _MINIMUMPAR_); //it's filling also the re-fitted one min value
    //======================================================================================================
    // Create a Ntuple to store results of the scan 
    //======================================================================================================    
    MessageSvc::Debug("Will perform", TString( TString::Format("%i Steps", _SCANVALUES_.size())), "");
    TFile myFileScan("LLScan.root","RECREATE","ROOT file with scan point values-likelihood values");
    TNtupleD    tuple("ScanTuple", "ScanTuple", "fitstatus:offset:parVal:likelihoodval:likelihoodatmin");


    //======================================================================================================
    // Create a Likelihood with minimized parameter set
    //======================================================================================================
    RooArgSet _nLL;
    CreateNLL(_nLL);
    RooAddition _nllSimultaneous("nllCombined_SCAN", "-log(likelihood)", _nLL);
    _nllSimultaneous.enableOffsetting(m_offsetLikelihood);
    RooMinimizer _minimizer(_nllSimultaneous);
    ConfigureMinimizer(_minimizer, m_name + "_FitScans.log");
    _minimizer.setStrategy(1);
    //_minimizer.setStrategy(2); //Probably it's better for numerical precision
    _minimizer.zeroEvalCount();
    double _likelihoodatmin = _nllSimultaneous.getVal();    
    if( SettingDef::Fit::startLLScanFromMin){
        //========================================================================================================================
        // Run the fit again, validate min with the new offset given initial params at converged values
        //========================================================================================================================
        m_independentParameterMap[_varForLikelihoodName]->setConstant(0);
        m_independentParameterMap[_varForLikelihoodName]->setVal(  _MINIMUMPAR_);
        m_independentParameterMap[_varForLikelihoodName]->setError(_MINIMUMPARERR_);
        double _initialOffset             =  _nllSimultaneous.getVal();
        _minimizer.setStrategy(2);
        int _statusFit =  _minimizer.minimize(m_minType, m_minAlg);
        if( _statusFit != 0 ){
            MessageSvc::Warning("ProflieLikelihood::startLLScanFromMin Invalid Fit back loop !");
        }else{
            _statusFit = _minimizer.hesse();
            if( _statusFit!=0){
                _statusFit = _minimizer.minimize(m_minType, m_minAlg);
                _statusFit = _minimizer.hesse();
            }
        }
        if( _statusFit != 0 ){
            MessageSvc::Error("ProflieLikelihood::startLLScanFromMin Invalid Fit back loop, after retrying!");
        }
        // if(_statusFit   !=0 )  MessageSvc::Error("ProfileLikelihood fit back failed, severe error, check it", "","EXIT_FAILURE");
        auto _results = _minimizer.save(m_name + "_FittingLLScan1D", "Result of fit of p.d.f for ");
        TString _name = SettingDef::IO::outDir + "LLScan_"+_varForLikelihoodName+ "_ValidateAbsMin_Results.log";
        auto _file = ofstream(_name);
        _results->printMultiline(_file, 1111, kTRUE, "\t");
        _file.close();        
        //========================================================================================================================
        // Override them, to check matching with log value
        //========================================================================================================================
        _likelihoodatmin = _nllSimultaneous.getVal();
        _MINIMUMPAR_      = m_independentParameterMap[_varForLikelihoodName]->getVal();   //retrieve the value at min 
        _MINIMUMPARERR_   = m_independentParameterMap[_varForLikelihoodName]->getError(); //retrieve the error
        // tuple.Fill( _statusFit, _initialOffset, _MINIMUMPAR_, LLValStep, _likelihoodatmin);
        //Cache results
        for (auto & keyParPair : m_independentParameterMap) {
            auto _keyTHIS = keyParPair.first;
            auto _var     = keyParPair.second;
            if( _var->isConstant()) continue;
            _values_AtMinUseLoop[_keyTHIS] = _var->getVal();
            _errors_AtMinUseLoop[_keyTHIS] = _var->getError();
        }
        _SCANVALUES_.push_back( _MINIMUMPAR_); //will redo a fit at the very end with the minimum parameter setting
    }
    //======================================================================================================
    int IDX_STEP = 0;
    for(auto & _PARSCANVALUE_ : _SCANVALUES_ ){
        MessageSvc::Warning("Resetting Parameter(s) to values at min found");
        MessageSvc::Line();
        std::cout<<"Step "<< IDX_STEP <<"/"<<_SCANVALUES_.size() << std::endl;
        MessageSvc::Line();
        m_independentParameterMap[_varForLikelihoodName]->setVal(_PARSCANVALUE_);
        m_independentParameterMap[_varForLikelihoodName]->setConstant(1);
        //========================================================================================================================
        // Initialization Routine for fits at each loop scan
        //========================================================================================================================
        for (auto & keyParPair : m_independentParameterMap){
            auto _keyLoop = keyParPair.first;
            if( m_independentParameterMap[_keyLoop]->isConstant()) continue; //skip constant and likelihood scan parameter of interest
            if(                 _keyLoop == _varForLikelihoodName) continue; //skip constant and likelihood scan parameter of interest
            MessageSvc::Debug("(LLScan) SetInitialValuesAndErrors[Before]", m_independentParameterMap[_keyLoop]);
            if( !_keyLoop.Contains("csyst")){
                //Value from previous iteration used on csyst... (try to follow correlation to RX parameter in each fit step?)
                m_independentParameterMap[_keyLoop]->setVal(_values_AtMinUseLoop[_keyLoop]); //set the value to the values At Min to be used for the loop
            }
            if(_keyLoop.Contains("jps") || _keyLoop.Contains("csyst")){
                //Initialize errors for J/Psi related and systematics parameters to be the one at the converged value
                if( _keyLoop.Contains("csyst")){
                    // m_independentParameterMap[_keyLoop]->setError(_errors_AtMinUseLoop[_keyLoop]*2.);
                    m_independentParameterMap[_keyLoop]->setError(_errors_AtMinUseLoop[_keyLoop]);
                // m_independentParameterMap[_keyLoop]->setError(_errors_AtMinUseLoop[_keyLoop]); maybe 0.05 ? 

                }else{
                    m_independentParameterMap[_keyLoop]->setError(_errors_AtMinUseLoop[_keyLoop]);
                }
	            MessageSvc::Debug("(LLScan) SetInitialValuesAndErrors[After, J/Psi parameter to converged val]",  m_independentParameterMap[_keyLoop]);
            }else{
                //Initialize errors for all other parameters to the one fed into the fit at the origin ( InitialParamFile triggered )
                if( m_initialParams.find(_keyLoop) != m_initialParams.end()){                    
                    if(SettingDef::Fit::startLLScanFromMin){
                        m_independentParameterMap[_keyLoop]->setError(_errors_AtMinUseLoop[_keyLoop]);
                    }else{
                        m_independentParameterMap[_keyLoop]->setError(m_initialParams[_keyLoop].Error);
                    }
                    MessageSvc::Debug("(LLScan) SetInitialValuesAndErrors[After, from initialParams]",     m_independentParameterMap[_keyLoop]);
                }else{
                    m_independentParameterMap[_keyLoop]->setError(_errors_AtMinUseLoop[_keyLoop]);
                    MessageSvc::Debug("(LLScan) SetInitialValuesAndErrors[After, from converged at Min]",  m_independentParameterMap[_keyLoop]);
                }
            }
            
            //Shift par from boundary limit a bit to make the fit happy
            for( auto & el : m_independentParameterMap ){
                if( el.second->isConstant()) continue ;
                auto _keyHERE = el.first ; 
                double val = el.second->getVal();
                double err = el.second->getError();
                bool _largeError = abs( el.second->getError() / el.second->getVal()) > 1;
                bool _belowLimit = el.second->hasMin() ? (el.second->getVal() - el.second->getError()) < el.second->getMin() : false;
                bool _aboveLimit = el.second->hasMax() ? (el.second->getVal() + el.second->getError()) > el.second->getMax() : false;
                if( (_keyHERE.Contains("b_Comb") || _keyHERE.Contains("nbkg_")) && (_belowLimit || _aboveLimit)  ){
                //if the fit failed , what to do for the next round? (be aware of SS data combSS ? )
                    if( (abs(( el.second->getVal() -  el.second->getMin())/ el.second->getVal()) < 0.01) || 
                        (abs(( el.second->getMax() -  el.second->getVal())/ el.second->getVal()) < 0.01) ){ 
                        // The slope has hit the limit
                        if( m_initialParams.find(_keyHERE) != m_initialParams.end()){
                            m_independentParameterMap[_keyHERE]->setVal(   m_initialParams[_keyHERE].Value );
                            m_independentParameterMap[_keyHERE]->setError( m_initialParams[_keyHERE].Error );
                        }
                    }
                }
            }
        }
        //========================================================================================================================
        MessageSvc::Debug("Running fit scan");
        _minimizer.zeroEvalCount();
        _minimizer.setStrategy(1) ;
        double _initialOffset = _nllSimultaneous.getVal();
        int   _status         = _minimizer.minimize(m_minType, m_minAlg);
        // if( _status == 0){
        //     for( auto & el : m_independentParameterMap){
        //         if(el.first.Contains("csyst") && fabs(el.second->getError()/el.second->getValV() ) >0.60) {
        //             MessageSvc::Warning("Rerunning fit because csyst parameter has unrealistic errors");
        //             _minimizer.setStrategy(2);
        //             _statusFit = _minimizer.minimize( m_minType, m_minAlg);
        //         }
        //         break;  //hopefully it's enough doing it only once to fix this point
        //     }
        // }
        auto _results         = _minimizer.save(m_name + "_FittingLLScan1D", "Result of fit of p.d.f for ");
        double LLValStep      = _nllSimultaneous.getVal();
        tuple.Fill( _status, _initialOffset, _PARSCANVALUE_, LLValStep , _likelihoodatmin );        
        TString _name = SettingDef::IO::outDir + "LLScan_"+_varForLikelihoodName+ TString::Format("_%i", IDX_STEP)+ "_Results.log";
        auto _file = ofstream(_name);
        _results->printMultiline(_file, 1111, kTRUE, "\t");
        _file.close();
        m_independentParameterMap[_varForLikelihoodName]->setConstant(0);
        m_independentParameterMap[_varForLikelihoodName]->setVal(  _MINIMUMPAR_);
        m_independentParameterMap[_varForLikelihoodName]->setError(_MINIMUMPARERR_);        
        for(auto& keyParPair :m_independentParameterMap){
            auto _key = keyParPair.first;
            if(m_independentParameterMap[_key]->isConstant()) continue; 
            if( _key == _varForLikelihoodName               ) continue; 
            m_independentParameterMap[_key]->setVal(  _values_AtMinUseLoop[_key]);
            m_independentParameterMap[_key]->setError(_errors_AtMinUseLoop[_key]);
        }
        IDX_STEP++;
    }
    _minimizer.cleanup();
    DeleteNLL();
    myFileScan.Write();
    myFileScan.Close();
    /* Restore at end loop */
    m_independentParameterMap[_varForLikelihoodName]->setConstant(0);
    m_independentParameterMap[_varForLikelihoodName]->setVal(  _values_AtMin[_varForLikelihoodName]);
    m_independentParameterMap[_varForLikelihoodName]->setError(_errors_AtMin[_varForLikelihoodName]);
    for (auto & keyParPair : m_independentParameterMap){
        auto _key = keyParPair.first;
        if( m_independentParameterMap[_key]->isConstant()) continue;
        m_independentParameterMap[_key]->setVal(  _values_AtMin[_key]); //from migrad
        m_independentParameterMap[_key]->setError(_errors_AtMin[_key]); //from hesse (with boundaries applied!)
    }
    m_independentParameterMap[_varForLikelihoodName]->setConstant(0);
    return ;
}

void FitterTool::PlotContour(RooMinimizer & _minimizer , RooRealVar & _var1, RooRealVar & _var2) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitterTool", m_name, "PlotContour");
    MessageSvc::Line();
    if (!m_isInitialized) MessageSvc::Error("FitterTool not initialized", "EXIT_FAILURE");
    auto Name = TString::Format("%s_%s_vs_%s_PlotContour.log",m_name.Data(),_var1.GetName(),+_var2.GetName());
    //somehow contour requires migrad before ! 
    
    double v1_min = _var1.getVal() - abs(_var1.getErrorLo() ) *5.5;
    double v1_max = _var1.getVal() + abs(_var1.getErrorHi() ) *5.5;
    double v2_min = _var2.getVal() - abs(_var2.getErrorLo() ) *5.5;
    double v2_max = _var2.getVal() + abs(_var2.getErrorHi() ) *5.5;
    if( !_var1.hasAsymError(false) ){
        v1_min = _var1.getVal() - abs(_var1.getError() ) *5.5;
        v1_max = _var1.getVal() + abs(_var1.getError() ) *5.5;
    }
    if( !_var1.hasAsymError(false) ){
        v2_min = _var2.getVal() - abs(_var2.getError() ) *5.5;
        v2_max = _var2.getVal() + abs(_var2.getError() ) *5.5;
    }    
    auto fixedMinVal = [ ]( double inputMIN, double errorLow , double centralValue){
        double MIN = inputMIN;
        while(MIN < 0  && MIN < centralValue){
            MIN+= 0.5*errorLow;
        }
        if( MIN > centralValue) MessageSvc::Error("Have to break, logic is wrong", "","EXIT_FAILURE");
        return MIN;
    };
    if( !_var1.hasAsymError(false)){
        v1_min = fixedMinVal( v1_min , abs(_var1.getErrorLo()), _var1.getVal() );
    }else{ 
        v1_min = fixedMinVal( v1_min , abs(_var1.getError()), _var1.getVal() );
    }
    if( !_var2.hasAsymError(false)){
        v2_min = fixedMinVal( v2_min , abs(_var2.getErrorLo()), _var2.getVal() );
    }else{ 
        v2_min = fixedMinVal( v2_min , abs(_var2.getError()), _var2.getVal() );
    }
    std::cout<< "Var1 Contour overwrite Range ( "<< v1_min << ","<<v1_max<<")"<<std::endl;
    std::cout<< "Var2 Contour overwrite Range ( "<< v2_min << ","<<v2_max<<")"<<std::endl;
    _var1.setRange(v1_min, v1_max);
    _var2.setRange(v2_min, v2_max);
    TCanvas _canvas("canvas");
    //See https://pdg.lbl.gov/2021/reviews/rpp2020-rev-statistics.pdf
    // double n1 = TMath::Sqrt( TMath::ChisquareQuantile( 0.6827, 2));
    // double n2 = TMath::Sqrt( TMath::ChisquareQuantile( 0.9545, 2));
    // double n3 = TMath::Sqrt( TMath::ChisquareQuantile( 0.9973, 2));
    //68.27
    //95.45
    //99.73
    double n1 = TMath::Sqrt( TMath::ChisquareQuantile(0.6827,2)); //1 sigma for DOF = 2 , TMath::Sqrt( TMath::ChisquareQuantile( 0.6827, 2));
    double n2 = TMath::Sqrt( TMath::ChisquareQuantile(0.9545,2)); //2 sigma for DOF = 2 , TMath::Sqrt( TMath::ChisquareQuantile( 0.9545, 2));
    double n3 = 0;
    //double n3 = TMath::Sqrt( TMath::ChisquareQuantile(0.9973,2)); //3 sigma for DOF = 2 , TMath::Sqrt( TMath::ChisquareQuantile( 0.9973, 2));
    // RooPlot * _frame = _minimizer.contour(_var1, _var2, n1, n2, n3, 0,0,0, 50);
    RooPlot * _frame = _minimizer.contour(_var1, _var2, n1, n2, n3, 0,0,0, 50);
    _frame->SetXTitle(_var1.GetTitle());
    _frame->SetYTitle(_var2.GetTitle());
    _frame->SetZTitle("log(#it{L})");
    _frame->Draw();
    if (!SettingDef::IO::outDir.EndsWith("/")) SettingDef::IO::outDir += "/";
    TString _name = SettingDef::IO::outDir + m_name + "_ContourPlot_for_" + _var1.GetName() + "_" + _var2.GetName();
    _canvas.SaveAs(_name + ".pdf");
    _canvas.SaveAs(_name + ".root");
    Plotting::SaveAllObjectFromFrame( _frame, _name+".root", OpenMode::UPDATE);   
    delete _frame;
    return;    
}

void FitterTool::FullDebug() {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitterTool", m_name, "FullDebug");
    MessageSvc::Line();
    cout << YELLOW << endl;
    vector< RooAbsReal * > list_of_fit_depentends;
    for (auto & _fitManager : m_fitManagers) {
        for (auto & _fitInfo : _fitManager.second.FitInfo) {
            cout << _fitManager.first << " ============ " << endl;
            cout << "|\t " << _fitInfo.first << " Deep-dependences " << endl;
            auto params_model   = _fitInfo.second.fullmodel->getVariables();
            auto iter_par_model = params_model->createIterator();
            auto var_deps       = iter_par_model->Next();
            while (var_deps != nullptr) {
                if (((RooRealVar *) var_deps)->isConstant()) {
                    var_deps = iter_par_model->Next();
                    continue;
                    auto iter_clients_pdfs = ((RooRealVar *) var_deps)->clientIterator();
                    auto clients_pdfs      = iter_clients_pdfs->Next();
                    cout << WHITE << flush;
                    cout << "|\t \t CONSTANT " << flush;
                    var_deps->Print();
                    cout << endl;

                    while (clients_pdfs != nullptr) {
                        cout << "|\t\t\t CLIENT : " << flush;
                        clients_pdfs->Print();
                        clients_pdfs = iter_clients_pdfs->Next();
                    }
                    cout << RESET << flush;
                } else {
                    auto iter_clients_pdfs = ((RooRealVar *) var_deps)->clientIterator();
                    auto clients_pdfs      = iter_clients_pdfs->Next();
                    cout << YELLOW << flush;
                    cout << "|\t \t FLOATING " << flush;
                    var_deps->Print();
                    cout << endl;
                    // var_deps->Print("v");
                    while (clients_pdfs != nullptr) {
                        cout << "|\t\t\t CLIENT : " << flush;
                        clients_pdfs->Print();
                        clients_pdfs = iter_clients_pdfs->Next();
                    }
                    cout << RESET << flush;
                }
                var_deps = iter_par_model->Next();
            }
        }
    }
    cout << RESET << endl;
    return;
}

void FitterTool::GetParameterMaps() {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitterTool", m_name, (TString) "GetParameterMaps");
    // Fills the relation table.
    for (auto & _keyQ2FitPair : m_fitManagers) {
        auto _manager = _keyQ2FitPair.second.manager;
        for (auto & _keyHolderPair : _manager->Holders()) {
            auto &     _fitHolder  = (*_manager)[_keyHolderPair.first];
            const auto _signalName = _keyHolderPair.first + "\t" + _fitHolder.SignalComponentKey();
            auto       _parameters = GetParametersInComponent(_fitHolder.SignalComponent());
            AddParameterComponentRelation(_signalName, _parameters);
            for (auto & _keyComponentPair : _fitHolder.BackgroundComponents()) {
                const auto & _componentKey  = _keyComponentPair.first;
                const auto   _componentName = _keyHolderPair.first + "\t" + _componentKey;
                const auto & _component     = _fitHolder.BackgroundComponent(_componentKey);
                _parameters                 = GetParametersInComponent(_component);
                AddParameterComponentRelation(_componentName, _parameters);
            }
        }
    }
    MessageSvc::Line();
    return;
}

vector< RooAbsReal * > FitterTool::GetParametersInComponent(const FitComponentAndYield & _componentAndYield) {
    MessageSvc::Info(Color::Cyan, "FitterTool", m_name, (TString) "GetParametersInComponent");

    vector< RooAbsReal * > _parameters;
    RooAbsReal *           _parameter = nullptr;
    RooArgSet *            _parSet    = nullptr;
    TIterator *            _iter      = nullptr;

    _parSet = _componentAndYield.fitComponent.PDF()->getVariables();
    auto * observable = _componentAndYield.fitComponent.Var();
    if (_parSet != nullptr) _iter = _parSet->createIterator();
    if (_iter != nullptr) {
        while ((_parameter = (RooAbsReal *) _iter->Next())) {
            if (_parameter != nullptr) {
                _parameters.push_back(_parameter);
                if (not(_parameter->isDerived()) && not(TString(observable->GetName()) == TString(_parameter->GetName()))) {
                    auto _name                       = TString(_parameter->GetName());
                    m_independentParameterMap[_name] = (RooRealVar *) _parameter;
                }
            }
        }
    }
    if (_parSet != nullptr) {
        delete _parSet;
        _parSet = nullptr;
    }
    if (_iter != nullptr) {
        delete _iter;
        _iter = nullptr;
    }

    auto _yield = _componentAndYield.yield;
    if (_yield != nullptr) _parSet = _yield->getVariables();
    if (_parSet != nullptr) _iter = _parSet->createIterator();
    if (_iter != nullptr) {
        while ((_parameter = (RooAbsReal *) _iter->Next())) {
            if (_parameter != nullptr) {
                _parameters.push_back(_parameter);
                if (not(_parameter->isDerived())) {
                    auto _name                       = TString(_parameter->GetName());
                    m_independentParameterMap[_name] = (RooRealVar *) _parameter;
                }
            }
        }
    }
    if (_yield != nullptr) {
        if (not(_yield->isDerived())) {
            auto _name = TString(_yield->GetName());
            _parameters.push_back(_yield);
            m_independentParameterMap[_name] = (RooRealVar *) _yield;
        }
    }
    if (_parSet != nullptr) delete _parSet;
    if (_iter != nullptr) delete _iter;

    return _parameters;
}

void FitterTool::AddParameterComponentRelation(const TString & _componentName, const vector< RooAbsReal * > _parameters) {
    MessageSvc::Info(Color::Cyan, "FitterTool", m_name, (TString) "AddParameterComponentRelation");
    for (auto _par : _parameters) {
        auto _name = TString(_par->GetName());
        m_parameterToComponentsMap[_name].push_back(_componentName);
    }
    return;
}

void FitterTool::PrintParameterComponentRelation() {
    // Sorts so that parameters shared more often are displayed first.
    vector< TString > _parameterNames;
    for (const auto & _parComponentsPair : m_parameterToComponentsMap) { _parameterNames.push_back(_parComponentsPair.first); }
    sort(_parameterNames.begin(), _parameterNames.end(), [this](TString a, TString b) { return m_parameterToComponentsMap[a].size() > m_parameterToComponentsMap[b].size(); });

    // Now print the relation table
    MessageSvc::Line();
    for (const auto & _parameterName : _parameterNames) {
        if (m_parameterToComponentsMap[_parameterName].size() > 1) {   // Print if variable is shared by more than one component
            MessageSvc::Info("Parameter", _parameterName, m_parameterToComponentsMap[_parameterName].size());
            for (const auto & _componentName : m_parameterToComponentsMap[_parameterName]) { MessageSvc::Info("Component", _componentName); }
            MessageSvc::Line();
        }
    }
}

void FitterTool::DumpYieldsValues(){
    auto formula_string = []( const RooFormulaVar & f2){
        TString _name = Form("RooFormulaVar::%s[ actualVars=(", f2.GetName());
        auto SIZE = f2.getVariables()->size();
        for( int IDX =0; IDX< SIZE; ++IDX){            
            _name+= Form("%s",f2.getParameter(IDX)->GetName());            
            _name+= ( SIZE-1 == IDX)? ")" : ",";
        }
        _name+=" formula='";
        _name+=f2.GetTitle();
        _name+="' ] = ";
        _name+=Form("%f", f2.getVal());        
        return _name;
    };
            
    MessageSvc::Warning("FitterTool::DumpYieldsValues Execute it only after fitting !");
    YAML::Emitter _emitter;
    _emitter << YAML::Key <<"Managers";
    _emitter << YAML::BeginMap; //0 
    for (auto & _fitManager : m_fitManagers) {        
        _emitter << YAML::Key << _fitManager.first.Data();        
        _emitter << YAML::BeginMap;  //1 
        for (auto & _holder : _fitManager.second.manager->Holders()) {
            _emitter << YAML::Key << _holder.first.Data();        
            _emitter << YAML::BeginMap; // 2
            // _emitter << YAML::Key << _holder.second.SignalComponent().yield->GetName()  << YAML::Value << _holder.second.SignalComponent().yield->getVal() ;    
            if(_holder.second.SignalComponent().yield == nullptr){ 
                MessageSvc::Warning("Cannot dump, nullptr"); continue; 
            }else{
                if(_holder.second.SignalComponent().yield->ClassName()=="RooFormulaVar"){
                RooFormulaVar *cc = dynamic_cast<RooFormulaVar*>(_holder.second.SignalComponent().yield);
                if( cc != nullptr){              
                    TString keyVal = TString("Formula_") + _holder.second.SignalComponent().yield->GetName();
                    TString forExp = TString(formula_string( *cc ));
                    _emitter << YAML::Key << keyVal.ReplaceAll(" ","").Data() << YAML::Value << forExp.Data();
                }
                }else{
                    _emitter << YAML::Key << _holder.second.SignalComponent().yield->GetName()  << YAML::Value << _holder.second.SignalComponent().yield->getVal() ;        
                }
            }
            for (auto & _bkg : _holder.second.BackgroundComponents()){
                _emitter << YAML::Key << _bkg.second.yield->GetName()  << YAML::Value << _bkg.second.yield->getVal() ;                
                RooFormulaVar *cc = dynamic_cast<RooFormulaVar*>(_bkg.second.yield);
                if( cc != nullptr){
                    TString keyVal = TString("Formula_") +  _bkg.second.yield->GetName();
                    TString forExp = TString(formula_string( *cc ));
                    _emitter << YAML::Key << keyVal.ReplaceAll(" ","").Data() << YAML::Value << forExp.Data();
                }                
                _emitter << YAML::Key << TString("BKGoverSIG")+"_"+_bkg.second.yield->GetName()  << YAML::Value << _bkg.second.yield->getVal()/ _holder.second.SignalComponent().yield->getVal();
            }
            _emitter << YAML::EndMap; // 2 close
        }
        _emitter << YAML::EndMap;  // 1 close
    }
    _emitter << YAML::EndMap;  // 0 close
    auto _name = fmt::format("{0}/{1}_YieldsResults.log", SettingDef::IO::outDir, m_name);
    ofstream _file(_name);
    if (!_file.is_open()) MessageSvc::Error("Unable to open file", _name, "EXIT_FAILURE");
    _file << _emitter.c_str() << endl;
    _file.close();    
}



//================================================================================================
// MERGED PLOT NAMESPACE IMPLEMENTATION
//================================================================================================
/**
 * \brief Create a canvas with the summed PDFs in it splitted by categories
 * @param  Pass the QSquareFit object, the combined dataset, the full model generated summing all categories, the corresponing _mapOFPDFs, th observable of interest, a list of colors for the plot
 */
template < Analysis ana > RooDataSet * MergedPlot::MergeDataSet(const QSquareFit & _fitManagers){
    bool         _flag       = false;
    RooDataSet * _datasetSum = nullptr;
    for (const auto & _fitInfo : _fitManagers.FitInfo) {
        if (!_fitInfo.first.Contains(to_string(ana))) continue;
        if (_fitInfo.second.dataset == nullptr) MessageSvc::Error("DoSPlot", (TString) "Only works with RooDataSet", "EXIT_FAILURE");
        if (_datasetSum == nullptr) {
            _datasetSum = new RooDataSet(*_fitInfo.second.dataset);
            MessageSvc::Info("MergeDataSet", to_string(ana), "Entries =", to_string(_datasetSum->sumEntries()));
        } else {
            // Append extra dataset to the summed one
            _datasetSum->append(*_fitInfo.second.dataset);
            _flag = true;
        }
    }
    if (!_flag) return nullptr;
    if (_datasetSum == nullptr) MessageSvc::Warning("MergeDataSet", (TString) "Unable to create merged dataset", to_string(ana));
    return _datasetSum;
};

template < Analysis ana > 
map< TString, pair<RooArgList,RooArgList> > MergedPlot::GetPDFsCoefsCategories(const QSquareFit & _fitManagers){
    map< TString, pair< RooArgList, RooArgList > > _pdfs;
    for (const auto & _fitInfo : _fitManagers.FitInfo) {
        if (!_fitInfo.first.Contains(to_string(ana))) continue;
        static_cast< RooAddPdf * >(_fitInfo.second.fullmodel)->coefList().Print("v");
        static_cast< RooAddPdf * >(_fitInfo.second.fullmodel)->pdfList().Print("v");
        // Decompose full model into the subcomponents list of pdfs and coefficients
        RooArgList pdflist  = (static_cast< RooAddPdf * >(_fitInfo.second.fullmodel))->pdfList();
        RooArgList coeflist = (static_cast< RooAddPdf * >(_fitInfo.second.fullmodel))->coefList();
        // components is map<TString, RooAbsPdf * > = map [ category shape ] = pdf used . Match this to the pdflist
        int npdfs  = pdflist.getSize();
        int nfound = 0;
        for (auto & pdfs : _fitInfo.second.components){
            bool found = false;
            for (int i = 0; i < pdflist.getSize(); ++i) {
                if( found) continue;
                if (pdfs.second->GetName() == pdflist.at(i)->GetName() ) {
                    found = true;
                    MessageSvc::Line();
                    MessageSvc::Info("GetPDFsCoefsCategories (found matching) in ", _fitInfo.first);
                    MessageSvc::Info("GetPDFsCoefsCategories Fill", TString::Format("ANA: %s, CAT: %s", to_string(ana).Data(), pdfs.first.Data()));
                    pdflist.at(i)->Print("v");
                    coeflist.at(i)->Print("v");
                    MessageSvc::Line();
                    _pdfs[pdfs.first].first.add(*pdflist.at(i));
                    _pdfs[pdfs.first].second.add(*coeflist.at(i));
                    break;
                }
            }
            if (found){ nfound++;}
        }
        if (npdfs != nfound) {
            MessageSvc::Error("NFound != NPDFS","","EXIT_FAILURE");
        }
    }
    return _pdfs;
};

template < Analysis ana > 
map<TString, Int_t>  MergedPlot::GetColorsCategories(const QSquareFit & _fitManagers){
    auto _COLOR_SEARCH_ = []( map<TString, Int_t> & _myColors, TString _name){
        for( auto & el : _myColors){
            TString _idLabel = ToLower(el.first);
            TString _idName  = ToLower(_name);
            if( _idLabel == _idName) return  el.second;
        }
        return (Int_t)kBlack;
    };
    map< TString, Int_t>  _colors;
    for (const auto & _fitInfo : _fitManagers.FitInfo) {
        RooArgList pdflist  = (static_cast< RooAddPdf * >(_fitInfo.second.fullmodel))->pdfList();
        if (!_fitInfo.first.Contains(to_string(ana))) continue;
        auto _COLOR_PLOT_ = _fitInfo.second.colors;
        int npdfs  = pdflist.getSize();
        int nfound = 0;
        for (auto & pdfs : _fitInfo.second.components){
            bool found = false;
            for (int i = 0; i < pdflist.getSize(); ++i) {
                if( found) continue;
                if (pdfs.second->GetName() == pdflist.at(i)->GetName() ) {       
                    found = true;             
                    _colors[pdfs.first] = _COLOR_SEARCH_(_COLOR_PLOT_, pdfs.first );
                    break;
                }
            }
            if (found){ nfound++;}
        }
        if (npdfs != nfound) {
            MessageSvc::Error("NFound != NPDFS","","EXIT_FAILURE");
        }
    }
    return _colors;
}

RooArgList MergedPlot::AllPDFsFromDecomposition(const map<TString, pair< RooArgList, RooArgList > > & _map) {
    RooArgList all_pdfs("myPDFS");
    for (auto & _list : _map) {
        MessageSvc::Debug("AllPDFsFromDecomposition (Category)", TString::Format("%s", _list.first.Data()));
        for (int i = 0; i < _list.second.first.getSize(); ++i) {
            all_pdfs.add(_list.second.first);
            TString _myPrint = TString::Format("%s", _list.second.first.at(i)->GetName());
            MessageSvc::Debug("AllPDFsFromDecomposition", _myPrint);
        }
    }
    return all_pdfs;
}

RooArgList MergedPlot::AllCoefsFromDecomposition(const map< TString, pair< RooArgList, RooArgList > >  & _map) {
    RooArgList all_coefs("myCoeffs");
    for (auto & _list : _map) {
        MessageSvc::Debug("AllCoefsFromDecomposition (Category)", TString::Format("%s", _list.first.Data()));
        for (int i = 0; i < _list.second.first.getSize(); ++i) {
            all_coefs.add(_list.second.second);        
            TString _myPrint = TString::Format("%s", _list.second.second.at(i)->GetName());
            MessageSvc::Debug("AllCoefsFromDecomposition", _myPrint);
        }
    }
    return all_coefs;
}

template < Analysis ana > TCanvas * MergedPlot::PlotAndResidualPad(const QSquareFit & _fitManagers, const RooDataSet * _dataset, const RooAddPdf * _model, const map<TString, pair< RooArgList, RooArgList >> & _mapPDFS, const RooRealVar * _observable, TString _managername, const map<  TString, Int_t > & colors){    
    TCanvas * canvas  = new TCanvas("canvasMerge", "canvasMerge", 1200, 900);
    TPad *   _plotPad = new TPad(TString("plotPad") + _model->GetName(), "", 0, .25, 1, 1);
    TPad *   _pullPad  = new TPad(TString("resPad") + _model->GetName(), "", 0, 0, 1, .25);
    Plotting::ConfigurePadPlot(_plotPad);
    Plotting::ConfigurePullPlot(_pullPad);
    _plotPad->Draw();
    _pullPad->Draw();
    _plotPad->cd();    

    //------------------  FOR PAPER BINNINGS  --------------//
    double _varMin = _observable->getMin();
    double _varMax = _observable->getMax();
    double _bin_width  =  (_varMax-_varMin)/ _observable->getBins();
    if( _managername.Contains("jps") || _managername.Contains("psi") ){
        _bin_width  =  32;  //assumes it's the B mass we are fitting 32MeV per Bin
    }
    int nBinsPlot =  std::floor( (_varMax - _varMin )/_bin_width);
    std::cout<<"Plotting(COMBINED) " << _managername << " with [ "<< _varMin << " , " << _varMax << " ], bin width ="<< _bin_width << std::endl;
    RooPlot * _frame  = _observable->frame(Title(""), Bins( nBinsPlot)); // Bins(_observable->getBins()));
    //------------------------------------------------------//
    // RooPlot * _frame  = _observable->frame(Title("")); // Bins(_observable->getBins()));


    Plotting::CustomizeRooPlot(_frame);
    int  _nLabels = _mapPDFS.size() + 1;
    auto * _legend  = Plotting::GetLegend( _nLabels);
    RooPlot * _framePull = _observable->frame(Title("Pulls"));
    //Plot Data & add Legend label
    _dataset->plotOn(_frame, Name("data"));
    _legend->AddEntry(_frame->findObject("data"), "Real Data" , "P");

    _model->plotOn(   _frame, Name("model") , LineColor(MergedPlot::DefaultColors[0]), LineWidth(5), Precision(1E-5));
    RooHist * _hPulls = _frame->pullHist(0,0,true);
    /* _legend->AddEntry(_frame->findObject("model"), "#Sigma (model)", "L");*/
    
    vector< TString> _elementsPlot; 
    for (const auto & categories : _mapPDFS) {
        _elementsPlot.push_back(categories.first);
    }
    //--- Sort elements to plot ( the MAP key )!
    std::sort( _elementsPlot.begin(), _elementsPlot.end() , []( const TString &a , const TString &b ){
        return Plotting::bkgsortidx(a) < Plotting::bkgsortidx(b);
    });
    for( auto & el : _elementsPlot){
        MessageSvc::Info("Plotting order", el);
    }
    //--- Plot signal Alone
    _model->plotOn(_frame, Components(_mapPDFS.at("Signal").first), DrawOption("L"), LineColor(MergedPlot::DefaultColors[0]), LineStyle(kDashed), LineWidth(5), Name("Signal"));
    _legend->AddEntry(_frame->findObject("Signal"), "#Sigma Signal", "L");
    int i = 1;
    //--- Plot non signal stac
    RooArgSet _pdfList = RooArgSet("pdfListBKGSAMPLES");
    for (const auto & category_ordered : _elementsPlot){
        if(category_ordered.Contains("Signal")){continue;}
        MessageSvc::Warning("PlotAndResidualPad", TString::Format("Plotting category (stacked) %s", category_ordered.Data()));
        _mapPDFS.at(category_ordered).first.Print();
        _mapPDFS.at(category_ordered).second.Print();
        /* ADD TO PDFLIST SO IT STACKS */
        _pdfList.add(_mapPDFS.at(category_ordered).first);
        if( colors.find( category_ordered ) != colors.end() ){
            _model->plotOn(_frame, Components(_pdfList), DrawOption("F") , FillStyle(1001), VLines(), LineColor(colors.at(category_ordered)), FillColor(colors.at(category_ordered)), LineWidth(5),  Name(category_ordered), MoveToBack() );
        }else{
            _model->plotOn(_frame, Components(_pdfList), DrawOption("F") , FillStyle(1001), VLines(), LineColor(MergedPlot::DefaultColors[i]), FillColor(MergedPlot::DefaultColors[i]), LineWidth(5),  Name(category_ordered), MoveToBack() );
        }
        _legend->AddEntry(_frame->findObject(category_ordered), TString::Format("#Sigma %s",category_ordered.Data()), "F");
        i++;
    }

    _frame->GetYaxis()->CenterTitle(1);
    _frame->Draw();
    if(_frame->GetMinimum() >0){
        _frame->SetMinimum(1e-9);
    }
    _legend->Draw("SAME");

    _hPulls->SetFillColor(kGray + 1);
    _hPulls->SetLineColor(kGray + 1);
    _hPulls->SetMarkerColor(kGray + 1);
    _hPulls->SetMarkerSize(0);
    _framePull->addPlotable(_hPulls, "BX");    
    /*
        RooArgSet _pdfList = RooArgSet("pdfList");
        _pdfList.add(*_pdf.first);
        _fitInfo.second.fullmodel->plotOn(_frame, Components(_pdfList), DrawOption("F"), FillColor(m_colors[i]), FillStyle(1001), VLines(), LineColor(m_colors[i]), LineWidth(5), Name(_pdf.second), MoveToBack());
    */
    _pullPad->cd();
    Plotting::CustomizeFramePull(_framePull);
    TAxis * xAxis    = _framePull->GetXaxis();
    TLine * lc       = new TLine(xAxis->GetXmin(), 0, xAxis->GetXmax(), 0);
    TLine * lu       = new TLine(xAxis->GetXmin(), 3, xAxis->GetXmax(), 3);
    TLine * ld       = new TLine(xAxis->GetXmin(), -3, xAxis->GetXmax(), -3);
    _framePull->Draw();
    lc->SetLineColor(kGray + 2);
    lu->SetLineColor(kGray + 1);
    ld->SetLineColor(kGray + 1);
    lc->SetLineStyle(2);
    lu->SetLineStyle(2);
    ld->SetLineStyle(2);
    lc->Draw("same");
    lu->Draw("same");
    ld->Draw("same");
    // plotPad->SetLogy();
    // auto tuple = make_tuple(plotPad, pullPad, legend);
    return canvas;
};


void FitterTool::SaveToWorkspace( TString _name){
    /*
        We have links across yields variables of models, have to fix it , done with this RecycleConflictNodes
        Be sure this is correct...verify WS ( this routine was implemented for Vava to remake plots a posteriori )
        We need to import source code of custom classes to have this workspace fully compatible to any ROOT release
    */            
    RooWorkspace *w = new RooWorkspace(_name);
    w->autoImportClassCode(true);
    //Class Code importation doesn't work apparently...something we are missing
    for (auto & _fitManager : m_fitManagers) {
        for (auto & _fitInfo : _fitManager.second.FitInfo) {
            TString _ID_FINAL = _fitManager.first;  //FullHolder name 
            w->import( *_fitInfo.second.fullmodel, RecycleConflictNodes() );   //hack to get back linked pdfs and datasets  
        }
    }
    for (auto & _fitManager : m_fitManagers) {
        for (auto & _fitInfo : _fitManager.second.FitInfo) {
            TString _ID_FINAL = _fitManager.first;  //FullHolder name 
            if( _fitInfo.second.dataset ){
                w->import( *_fitInfo.second.dataset, RecycleConflictNodes() ); //hack to get back ALL linked pdfs and datasets  
            }else{
                MessageSvc::Warning("Cannot import in Workspace the dataset, probably a binned only fit?");
            }
            if( _fitInfo.second.datahist ){
                w->import( *_fitInfo.second.datahist, RecycleConflictNodes() ); //hack to get back ALL linked pdfs and datasets  
            }else{
                MessageSvc::Warning("Cannot import in Workspace the datahist, probably a un-binned only fit?");
            }
        }
    }
    w->Print() ;
    w->writeToFile(TString::Format("%s_workspace.root", _name.Data() )) ;
    delete w;
    return;
}
#endif
