
#ifndef BINNEDLIKELIHOOD_CPP
#define BINNEDLIKELIHOOD_CPP

#include "BinnedLikelihood.h"

#include "RooAddition.h"
#include "RooCmdConfig.h"
#include "RooMsgService.h"
#include "RooConstraintSum.h"
#include "RooAbsBinning.h"
#include "RooAbsDataStore.h"
#include "RooParamBinning.h"
#include "RooAddPdf.h"

#include <string>
#include <iostream>
#include <stdio.h>

// Models RooAbsPdf::createNLL behaviour

RooAbsReal * BinnedLikelihood::CreateNLL(RooAbsPdf& _model, RooDataHist& _binnedData, RooRealVar& observable, const RooLinkedList& cmdList) {
    // Select the pdf-specific commands 
    RooCmdConfig pc(Form("RooAbsPdf::createNLL(%s)",_model.GetName())) ;

    pc.defineString("rangeName","RangeWithName",0,"",kTRUE) ;
    pc.defineString("addCoefRange","SumCoefRange",0,"") ;
    pc.defineString("globstag","GlobalObservablesTag",0,"") ;
    pc.defineDouble("rangeLo","Range",0,-999.) ;
    pc.defineDouble("rangeHi","Range",1,-999.) ;
    pc.defineInt("splitRange","SplitRange",0,0) ;
    pc.defineInt("ext","Extended",0,2) ;
    pc.defineInt("numcpu","NumCPU",0,1) ;
    pc.defineInt("interleave","NumCPU",1,0) ;
    pc.defineInt("verbose","Verbose",0,0) ;
    pc.defineInt("optConst","Optimize",0,0) ;
    pc.defineInt("cloneData","CloneData",2,0) ;
    pc.defineSet("projDepSet","ProjectedObservables",0,0) ;
    pc.defineSet("cPars","Constrain",0,0) ;
    pc.defineSet("glObs","GlobalObservables",0,0) ;
    pc.defineInt("constrAll","Constrained",0,0) ;
    pc.defineInt("doOffset","OffsetLikelihood",0,0) ;
    pc.defineSet("extCons","ExternalConstraints",0,0) ;
    pc.defineMutex("Range","RangeWithName") ;
    pc.defineMutex("Constrain","Constrained") ;
    pc.defineMutex("GlobalObservables","GlobalObservablesTag") ;

    // Process and check varargs 
    pc.process(cmdList) ;
    if (!pc.ok(kTRUE)) {
        return 0 ;
    }


    // Decode command line arguments
    const char* rangeName = pc.getString("rangeName",0,kTRUE) ;
    const char* addCoefRangeName = pc.getString("addCoefRange",0,kTRUE) ;
    const char* globsTag = pc.getString("globstag",0,kTRUE) ;
    Int_t ext      = pc.getInt("ext") ;
    Int_t numcpu   = pc.getInt("numcpu") ;
    RooFit::MPSplit interl = (RooFit::MPSplit) pc.getInt("interleave") ;

    Int_t splitr    = pc.getInt("splitRange") ;
    Bool_t verbose  = pc.getInt("verbose") ;
    Int_t optConst  = pc.getInt("optConst") ;
    Int_t cloneData = pc.getInt("cloneData") ;
    Int_t doOffset  = pc.getInt("doOffset") ;

    // If no explicit cloneData command is specified, cloneData is set to true if optimization is activated
    if (cloneData==2) {
        cloneData = optConst ;
    }

    RooArgSet* cPars = pc.getSet("cPars") ;
    RooArgSet* glObs = pc.getSet("glObs") ;
    if (pc.hasProcessed("GlobalObservablesTag")) {
    if (glObs) delete glObs ;
        RooArgSet* allVars = _model.getVariables() ;
        glObs = (RooArgSet*) allVars->selectByAttrib(globsTag,kTRUE) ;
        RooMsgService::instance().log(&_model,RooFit::INFO,RooFit::Minimization) << "User-defined specification of global observables definition with tag named '" <<  globsTag << "'" << std::endl ;
        delete allVars ;
    } else if (!pc.hasProcessed("GlobalObservables")) {

        // Neither GlobalObservables nor GlobalObservablesTag has been processed - try if a default tag is defined in the head node
        // Check if head not specifies default global observable tag
        const char* defGlobObsTag = _model.getStringAttribute("DefaultGlobalObservablesTag") ;
        if (defGlobObsTag) {
            RooMsgService::instance().log(&_model,RooFit::INFO,RooFit::Minimization) << "p.d.f. provides built-in specification of global observables definition with tag named '" <<  defGlobObsTag << "'" << std::endl ;
              if (glObs) delete glObs ;
              RooArgSet* allVars = _model.getVariables() ;
              glObs = (RooArgSet*) allVars->selectByAttrib(defGlobObsTag,kTRUE) ;
        }
    }

    
    Bool_t doStripDisconnected=kFALSE ;

    // If no explicit list of parameters to be constrained is specified apply default algorithm
    // All terms of RooProdPdfs that do not contain observables and share a parameters with one or more
    // terms that do contain observables are added as constraints.
    if (!cPars) {    
        cPars = _model.getParameters(_binnedData,kFALSE) ;
        doStripDisconnected=kTRUE ;
    }
    const RooArgSet* extCons = pc.getSet("extCons") ;

    // Process automatic extended option
    if (ext==2) {
        ext = ((_model.extendMode()==RooAbsPdf::ExtendMode::CanBeExtended || _model.extendMode()==RooAbsPdf::ExtendMode::MustBeExtended)) ? 1 : 0 ;
        if (ext) {
            RooMsgService::instance().log(&_model,RooFit::INFO,RooFit::Minimization) << "p.d.f. provides expected number of events, including extended term in likelihood." << std::endl ;
        }
    }

    if (pc.hasProcessed("Range")) {
        Double_t rangeLo = pc.getDouble("rangeLo") ;
        Double_t rangeHi = pc.getDouble("rangeHi") ;

        // Create range with name 'fit' with above limits on all observables
        RooArgSet* obs = _model.getObservables(&_binnedData) ;
        TIterator* iter = obs->createIterator() ;
        RooAbsArg* arg ;
        while((arg=(RooAbsArg*)iter->Next())) {
            RooRealVar* rrv =  dynamic_cast<RooRealVar*>(arg) ;
            if (rrv) rrv->setRange("fit",rangeLo,rangeHi) ;
        }
        // Set range name to be fitted to "fit"
        rangeName = "fit" ;
    }


    RooArgSet projDeps ;
    RooArgSet* tmp = pc.getSet("projDepSet") ;  
    if (tmp) {
        projDeps.add(*tmp) ;
    }

    // Construct NLL
    RooAbsReal::setEvalErrorLoggingMode(RooAbsReal::CollectErrors) ;
    RooAbsReal* nll ;
    std::string baseName = Form("nll_%s_%s",_model.GetName(),_binnedData.GetName()) ;
    if (!rangeName || strchr(rangeName,',')==0) {
        // Simple case: default range, or single restricted range
        //cout<<"FK: Data test 1: "<<data.sumEntries()<<std::endl;
        RooArgSet * allVars = _model.getVariables();
        RooArgSet analVars(observable);
        if (AnalyticalNLL::hasFullAnalyticalIntegral(_model, *allVars, analVars, "") == 1){
            std::cout << "Creating analytical integral for " << _model.GetName() << std::endl;
            nll = new AnalyticalNLL(baseName.c_str(),"-log(likelihood)", _model,_binnedData,observable,
                                    projDeps,ext,rangeName,addCoefRangeName,numcpu,interl,verbose,splitr,cloneData);
        }
        else{
            nll = new AdaptiveNLL(baseName.c_str(),"-log(likelihood)", _model,_binnedData,observable,
                                  projDeps,ext,rangeName,addCoefRangeName,numcpu,interl,verbose,splitr,cloneData);
        }
        delete allVars;
        // nll = new RooNLLVar(baseName.c_str(),"-log(likelihood)",_model,_binnedData,projDeps,ext,rangeName,addCoefRangeName,numcpu,interl,verbose,splitr,cloneData) ;
    } else {
        // Composite case: multiple ranges
        RooArgList nllList ;
        const size_t bufSize = strlen(rangeName)+1;
        char* buf = new char[bufSize] ;
        strlcpy(buf,rangeName,bufSize) ;
        char* token = strtok(buf,",") ;
        while(token) {
            RooAbsReal * nllComp;
            RooArgSet * allVars = _model.getVariables();
            RooArgSet analVars(observable);
            if (AnalyticalNLL::hasFullAnalyticalIntegral(_model, *allVars, analVars, "") == 1){
                std::cout << "Creating analytical integral for " << _model.GetName() << std::endl;
                nllComp = new AnalyticalNLL(Form("%s_%s",baseName.c_str(),token),"-log(likelihood)",_model,_binnedData,observable,
                                            projDeps,ext,rangeName,addCoefRangeName,numcpu,interl,verbose,splitr,cloneData);
            }
            else{
                nllComp = new AdaptiveNLL(Form("%s_%s",baseName.c_str(),token),"-log(likelihood)",_model,_binnedData,observable,
                                          projDeps,ext,rangeName,addCoefRangeName,numcpu,interl,verbose,splitr,cloneData);
            }
            nllList.add(*nllComp) ;
            token = strtok(0,",") ;
            delete allVars;
        }
        delete[] buf ;
        nll = new RooAddition(baseName.c_str(),"-log(likelihood)",nllList,kTRUE) ;
    }
    RooAbsReal::setEvalErrorLoggingMode(RooAbsReal::PrintErrors) ;
  

    // Collect internal and external constraint specifications
    RooArgSet allConstraints ;

    // We have no way to access protected data member _myws (RooWorkspace) of RooArbArg
    // if (_myws && _myws->set(Form("CACHE_CONSTR_OF_PDF_%s_FOR_OBS_%s", GetName(), RooNameSet(*data.get()).content()))) {

    //  // retrieve from cache
    //  const RooArgSet *constr =
    //     _myws->set(Form("CACHE_CONSTR_OF_PDF_%s_FOR_OBS_%s", GetName(), RooNameSet(*data.get()).content()));
    //  coutI(Minimization) << "createNLL picked up cached consraints from workspace with " << constr->getSize()
    //                      << " entries" << std::endl;
    //  allConstraints.add(*constr);

    // } else {

     if (cPars && cPars->getSize() > 0) {
        RooArgSet *constraints = getAllConstraints(_model, *_binnedData.get(), *cPars, doStripDisconnected);
        allConstraints.add(*constraints);
        delete constraints;
     }
     if (extCons) {
        allConstraints.add(*extCons);
     }

    // // write to cache
    // if (_myws) {
    // // cout << "createNLL: creating cache for allconstraints=" << allConstraints << std::endl ;
    // coutI(Minimization) << "createNLL: caching constraint set under name "
    //                     << Form("CONSTR_OF_PDF_%s_FOR_OBS_%s", GetName(), RooNameSet(*data.get()).content())
    //                     << " with " << allConstraints.getSize() << " entries" << std::endl;
    // _myws->defineSetInternal(
    //    Form("CACHE_CONSTR_OF_PDF_%s_FOR_OBS_%s", GetName(), RooNameSet(*data.get()).content()), allConstraints);
    // }
    // }

  // Include constraints, if any, in likelihood
    RooAbsReal* nllCons(0) ;
    if (allConstraints.getSize()>0 && cPars) {   

        RooMsgService::instance().log(&_model,RooFit::INFO,RooFit::Minimization) << " Including the following contraint terms in minimization: " << allConstraints << std::endl ;
        if (glObs) {
            RooMsgService::instance().log(&_model,RooFit::INFO,RooFit::Minimization) << "The following global observables have been defined: " << *glObs << std::endl ;
        }
        nllCons = new RooConstraintSum(Form("%s_constr",baseName.c_str()),"nllCons",allConstraints,glObs ? *glObs : *cPars) ;
        nllCons->setOperMode(_model.operMode()) ;
        RooAbsReal* orignll = nll ;

        nll = new RooAddition(Form("%s_with_constr",baseName.c_str()),"nllWithCons",RooArgSet(*nll,*nllCons)) ;
        nll->addOwnedComponents(RooArgSet(*orignll,*nllCons)) ;
    }

    if (optConst) {
        nll->constOptimizeTestStatistic(RooAbsArg::Activate,optConst>1) ;
    }

    if (doStripDisconnected) {
        delete cPars ;
    }

    if (doOffset) {
        nll->enableOffsetting(kTRUE) ;
    }

    return nll ;
}

RooAbsReal* BinnedLikelihood::CreateNLL(RooAbsPdf& _model, RooDataHist& _binnedData, RooRealVar& observable, const RooCmdArg& arg1, const RooCmdArg& arg2, const RooCmdArg& arg3, const RooCmdArg& arg4, 
                                             const RooCmdArg& arg5, const RooCmdArg& arg6, const RooCmdArg& arg7, const RooCmdArg& arg8) 
{
  RooLinkedList l ;
  l.Add((TObject*)&arg1) ;  l.Add((TObject*)&arg2) ;  
  l.Add((TObject*)&arg3) ;  l.Add((TObject*)&arg4) ;
  l.Add((TObject*)&arg5) ;  l.Add((TObject*)&arg6) ;  
  l.Add((TObject*)&arg7) ;  l.Add((TObject*)&arg8) ;
  return CreateNLL(_model, _binnedData, observable, l) ;
}

RooArgSet* BinnedLikelihood::getAllConstraints(RooAbsPdf& model, const RooArgSet& observables, RooArgSet& constrainedParams, Bool_t stripDisconnected) {
    RooArgSet* ret = new RooArgSet("AllConstraints") ;
    std::unique_ptr<RooArgSet> comps(model.getComponents());
    for (const auto arg : *comps) {
        auto pdf = dynamic_cast<const RooAbsPdf*>(arg) ;
        if (pdf && !ret->find(pdf->GetName())) {
            std::unique_ptr<RooArgSet> compRet(pdf->getConstraints(observables,constrainedParams,stripDisconnected));
            if (compRet) {
                ret->add(*compRet,kFALSE) ;
            }
        }
    }
    return ret ;
}

double BinnedLikelihood::evaluatePDF(double x, void * params){
    ((RooRealVar**)params)[0]->setVal(x);
    double normval = ((RooAbsPdf**)params)[1]->getVal(((RooArgSet**)params)[2]);
    return normval;
}


//ClassImp(AdaptiveNLL); methods to add to get it working 
AdaptiveNLL::AdaptiveNLL() : RooNLLVar() {
}


AdaptiveNLL::AdaptiveNLL(const char *name, const char* title, RooAbsPdf& pdf, RooDataHist& data, RooRealVar& observable,
                             const RooCmdArg& arg1, const RooCmdArg& arg2,const RooCmdArg& arg3,
                             const RooCmdArg& arg4, const RooCmdArg& arg5,const RooCmdArg& arg6,
                             const RooCmdArg& arg7, const RooCmdArg& arg8,const RooCmdArg& arg9) :
                   RooNLLVar(name, title, pdf, data, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9){
    SetObservable(observable);
    StoreBinning();
    InitGSLWorkspace();
    InitGSLFunction();
}

AdaptiveNLL::AdaptiveNLL(const char *name, const char *title, RooAbsPdf& pdf, RooDataHist& data, RooRealVar& observable,
                             Bool_t extended, const char* rangeName, const char* addCoefRangeName, 
                             Int_t nCPU, RooFit::MPSplit interleave, Bool_t verbose, 
                             Bool_t splitRange, Bool_t cloneData, Bool_t binnedL) :
                   RooNLLVar(name, title, pdf, data, extended, rangeName, addCoefRangeName,
                             nCPU, interleave, verbose, splitRange, cloneData, binnedL){
    SetObservable(observable);
    StoreBinning();
    InitGSLWorkspace();
    InitGSLFunction();
}
  
AdaptiveNLL::AdaptiveNLL(const char *name, const char *title, RooAbsPdf& pdf, RooDataHist& data, RooRealVar& observable,
                             const RooArgSet& projDeps, Bool_t extended, const char* rangeName, 
                             const char* addCoefRangeName, Int_t nCPU, RooFit::MPSplit interleave, Bool_t verbose, 
                             Bool_t splitRange, Bool_t cloneData, Bool_t binnedL) : 
                   RooNLLVar(name, title, pdf, data, projDeps, extended, rangeName, addCoefRangeName, 
                             nCPU, interleave, verbose, splitRange, cloneData, binnedL){
    SetObservable(observable);
    StoreBinning();
    InitGSLWorkspace();
    InitGSLFunction();
}

AdaptiveNLL::AdaptiveNLL(const AdaptiveNLL& other, const char* name) : 
                   RooNLLVar(other, name), 
                   binBoundaries(other.binBoundaries), 
                   m_observable(other.m_observable) {
    InitGSLWorkspace();
    InitGSLFunction();
}

AdaptiveNLL::~AdaptiveNLL(){
        gsl_integration_workspace_free(m_integrationWorkspace);
}

void AdaptiveNLL::SetObservable(RooRealVar& observable){
    auto * pdf = (RooAbsPdf*)_funcClone;
    auto * variables = pdf->getVariables();
    variables->Print("V");
    auto & clonedObservable = (*variables)[observable.GetName()];
    m_observable = (RooRealVar*)&clonedObservable;
    delete variables;
}

void AdaptiveNLL::StoreBinning(){
    const auto& binning = m_observable->getBinning();
    size_t nBins = binning.numBins();
    binBoundaries.reserve(nBins+1);
    for (size_t i = 0; i < nBins; i++){
        binBoundaries.push_back(binning.binLow(i));
    }
    binBoundaries.push_back(binning.binHigh(nBins-1));
}

void AdaptiveNLL::InitGSLWorkspace(){
    m_integrationWorkspace = gsl_integration_workspace_alloc(1000);
}

void AdaptiveNLL::InitGSLFunction(){
    m_gslFunction.function = &BinnedLikelihood::evaluatePDF;
    m_gslParams.push_back((void*)m_observable);
    m_gslParams.push_back((void*)_funcClone);
    m_gslParams.push_back((void*)_normSet);
    // m_gslParams.push_back((void*)&m_gslNormalisation);
    m_gslFunction.params = &m_gslParams[0];
}

Double_t AdaptiveNLL::evaluatePartition(Int_t firstEvent, Int_t lastEvent, Int_t stepSize) const{
    RooAbsPdf* pdfClone = (RooAbsPdf*) _funcClone ;
    _dataClone->store()->recalculateCache(_projDeps, firstEvent, lastEvent, stepSize, true) ;
    size_t nBins = (lastEvent - firstEvent)/stepSize;
    m_mu.reserve(nBins);
    m_n.reserve(nBins);

    double integral, abserr, result(0), carry(0);

    for (auto i=firstEvent; i<lastEvent; i+=stepSize) {
        _dataClone->get(i);
        m_n.push_back(_dataClone->weight());
        double _binLow = binBoundaries[i];
        double _binHigh = binBoundaries[i+1];
        // m_gslNormalisation = pdfClone->getVal(_normSet)/pdfClone->getVal();
        gsl_integration_qag(&m_gslFunction, _binLow, _binHigh, 0, 1e-4, 100, 1, m_integrationWorkspace, &integral, &abserr);
        m_mu.push_back(integral);
    }

    // Kahan summation of likelihoods
    for (size_t i=0; i<m_mu.size(); i++){
        double y = masked_likelihood(m_mu[i],m_n[i]) - carry;
        double t = result + y;
        carry = (t - result) - y;
        result = t;
    }

    // include the extended maximum likelihood term, if requested
    if(_extended && _setNum==_extSet) {
        if (_weightSq) {
            // Calculate sum of weights-squared here for extended term
            double sumW2(0), sumW2carry(0);
            for (size_t i=0 ; i<_dataClone->numEntries() ; i++) {
                _dataClone->get(i);
                double y = _dataClone->weightSquared() - sumW2carry;
                double t = sumW2 + y;
                sumW2carry = (t - sumW2) - y;
                sumW2 = t;
            }
            double expected = pdfClone->expectedEvents(_dataClone->get());
            double expectedW2 = expected * sumW2 / _dataClone->sumEntries() ;
            double extra = expectedW2 - sumW2*log(expected);
            double y = extra - carry;

            double t = result + y;
            carry = (t - result) - y;
            result = t;
        } 
        else {
            double y = pdfClone->extendedTerm(_dataClone->sumEntries(), _dataClone->get()) - carry;
            double t = result + y;
            carry = (t - result) - y;
            result = t;
        }
    }

    if (_first) {
        _first = kFALSE ;
        _funcClone->wireAllCaches() ;
    }

  // Check if value offset flag is set.
    if (_doOffset) {

        // If no offset is stored enable this feature now
        if (_offset==0 && result !=0 ) {
            coutI(Minimization) << "AdaptiveNLL::evaluatePartition(" << GetName() << ") first = "<< firstEvent << " last = " << lastEvent << " Likelihood offset now set to " << result << std::endl ;
            _offset = result ;
            _offsetCarry = carry;
        }

        // Substract offset
        double y = -_offset - (carry + _offsetCarry);
        double t = result + y;
        carry = (t - result) - y;
        result = t;
    }
    m_n.clear();
    m_mu.clear();

    return result;
}

//ClassImp(AnalyticalNLL); methods to add to get it working 
AnalyticalNLL::AnalyticalNLL() : RooNLLVar() {
}

AnalyticalNLL::AnalyticalNLL(const char *name, const char* title, RooAbsPdf& pdf, RooDataHist& data, RooRealVar& observable,
                             const RooCmdArg& arg1, const RooCmdArg& arg2,const RooCmdArg& arg3,
                             const RooCmdArg& arg4, const RooCmdArg& arg5,const RooCmdArg& arg6,
                             const RooCmdArg& arg7, const RooCmdArg& arg8,const RooCmdArg& arg9) :
                   RooNLLVar(name, title, pdf, data, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9){
    SetObservable(observable);
    StoreBinning();
    InitBinIntegral();
}

AnalyticalNLL::AnalyticalNLL(const char *name, const char *title, RooAbsPdf& pdf, RooDataHist& data, RooRealVar& observable,
                             Bool_t extended, const char* rangeName, const char* addCoefRangeName, 
                             Int_t nCPU, RooFit::MPSplit interleave, Bool_t verbose, 
                             Bool_t splitRange, Bool_t cloneData, Bool_t binnedL) :
                   RooNLLVar(name, title, pdf, data, extended, rangeName, addCoefRangeName,
                             nCPU, interleave, verbose, splitRange, cloneData, binnedL){
    SetObservable(observable);
    StoreBinning();
    InitBinIntegral();
}
  
AnalyticalNLL::AnalyticalNLL(const char *name, const char *title, RooAbsPdf& pdf, RooDataHist& data, RooRealVar& observable,
                             const RooArgSet& projDeps, Bool_t extended, const char* rangeName, 
                             const char* addCoefRangeName, Int_t nCPU, RooFit::MPSplit interleave, Bool_t verbose, 
                             Bool_t splitRange, Bool_t cloneData, Bool_t binnedL) : 
                   RooNLLVar(name, title, pdf, data, projDeps, extended, rangeName, addCoefRangeName, 
                             nCPU, interleave, verbose, splitRange, cloneData, binnedL){
    SetObservable(observable);
    StoreBinning();
    InitBinIntegral();
}

AnalyticalNLL::AnalyticalNLL(const AnalyticalNLL& other, const char* name) : 
                   RooNLLVar(other, name), 
                   binBoundaries(other.binBoundaries), 
                   m_observable(other.m_observable) {
    InitBinIntegral();
}

void AnalyticalNLL::SetObservable(RooRealVar& observable){
    auto * pdf = (RooAbsPdf*)_funcClone;
    auto * variables = pdf->getVariables();
    variables->Print("V");
    auto & clonedObservable = (*variables)[observable.GetName()];
    m_observable = (RooRealVar*)&clonedObservable;
    delete variables;
}

void AnalyticalNLL::StoreBinning(){
    const auto& binning = m_observable->getBinning();
    size_t nBins = binning.numBins();
    binBoundaries.reserve(nBins+1);
    for (size_t i = 0; i < nBins; i++){
        binBoundaries.push_back(binning.binLow(i));
    }
    binBoundaries.push_back(binning.binHigh(nBins-1));
}

void AnalyticalNLL::InitBinIntegral(){
    m_lowObservable.reset((RooRealVar*) m_observable->clone(Form("%s_low",m_observable->GetName())));
    m_highObservable.reset((RooRealVar*) m_observable->clone(Form("%s_high",m_observable->GetName())));
    RooParamBinning _binning(*m_lowObservable, *m_highObservable, 100);
    m_observable->setBinning(_binning, "binIntegral");
    m_binIntegral.reset(_funcClone->createIntegral(RooArgSet(*m_observable), RooArgSet(*m_observable), "binIntegral"));
    m_binIntegral->Print("V");
}

Double_t AnalyticalNLL::evaluatePartition(Int_t firstEvent, Int_t lastEvent, Int_t stepSize) const{
    RooAbsPdf* pdfClone = (RooAbsPdf*) _funcClone ;
    _dataClone->store()->recalculateCache(_projDeps, firstEvent, lastEvent, stepSize, true) ;
    size_t nBins = (lastEvent - firstEvent)/stepSize;
    m_mu.reserve(nBins);
    m_n.reserve(nBins);

    double result(0), carry(0);

    for (auto i=firstEvent; i<lastEvent; i+=stepSize) {
        _dataClone->get(i);
        m_n.push_back(_dataClone->weight());
        double _binLow = binBoundaries[i];
        double _binHigh = binBoundaries[i+1];
        m_lowObservable->setVal(_binLow);
        m_highObservable->setVal(_binHigh);
        double integral = m_binIntegral->getVal(RooArgSet(*m_observable));
        // m_gslNormalisation = pdfClone->getVal(_normSet)/pdfClone->getVal();
        m_mu.push_back(integral);
    }

    // Kahan summation of likelihoods
    for (size_t i=0; i<m_mu.size(); i++){
        double y = masked_likelihood(m_mu[i],m_n[i]) - carry;
        double t = result + y;
        carry = (t - result) - y;
        result = t;
    }

    // include the extended maximum likelihood term, if requested
    if(_extended && _setNum==_extSet) {
        if (_weightSq) {
            // Calculate sum of weights-squared here for extended term
            double sumW2(0), sumW2carry(0);
            for (size_t i=0 ; i<_dataClone->numEntries() ; i++) {
                _dataClone->get(i);
                double y = _dataClone->weightSquared() - sumW2carry;
                double t = sumW2 + y;
                sumW2carry = (t - sumW2) - y;
                sumW2 = t;
            }
            double expected = pdfClone->expectedEvents(_dataClone->get());
            double expectedW2 = expected * sumW2 / _dataClone->sumEntries() ;
            double extra = expectedW2 - sumW2*log(expected);
            double y = extra - carry;

            double t = result + y;
            carry = (t - result) - y;
            result = t;
        } 
        else {
            double y = pdfClone->extendedTerm(_dataClone->sumEntries(), _dataClone->get()) - carry;
            double t = result + y;
            carry = (t - result) - y;
            result = t;
        }
    }

    if (_first) {
        _first = kFALSE ;
        _funcClone->wireAllCaches() ;
    }

  // Check if value offset flag is set.
    if (_doOffset) {

        // If no offset is stored enable this feature now
        if (_offset==0 && result !=0 ) {
            coutI(Minimization) << "AdaptiveNLL::evaluatePartition(" << GetName() << ") first = "<< firstEvent << " last = " << lastEvent << " Likelihood offset now set to " << result << std::endl ;
            _offset = result ;
            _offsetCarry = carry;
        }

        // Substract offset
        double y = -_offset - (carry + _offsetCarry);
        double t = result + y;
        carry = (t - result) - y;
        result = t;
    }
    m_n.clear();
    m_mu.clear();

    return result;
}

int AnalyticalNLL::hasFullAnalyticalIntegral(const RooAbsPdf& model, RooArgSet& allVars, RooArgSet& analVars, const char * rangeName){
    if (TString(model.ClassName()) == TString("RooAddPdf")){ 
        const auto& addPdf = static_cast<const RooAddPdf&>(model);
        const auto& pdfList = addPdf.pdfList();
        std::vector <int> hasAnalytical;
        for (auto iter = pdfList.begin(); iter != pdfList.end(); iter++){
            const auto * pdf = static_cast<RooAbsPdf*>(*iter);
            int code = hasFullAnalyticalIntegral(*pdf, allVars, analVars, rangeName);
            hasAnalytical.push_back(code);
        }
        int return_code = 1;
        for (auto& code : hasAnalytical){
            return_code *= code;
        }
        return return_code;
    }
    else{
        int code = model.getAnalyticalIntegral(allVars, analVars, rangeName) != 0 ? 1 : 0;
        return code;
    }
}


//ClassImp(RombergNLL); methods to add to get it working
RombergNLL::RombergNLL() : RooNLLVar() {
}

RombergNLL::RombergNLL(const char *name, const char* title, RooAbsPdf& pdf, RooDataHist& data, RooRealVar& observable,
                             const RooCmdArg& arg1, const RooCmdArg& arg2,const RooCmdArg& arg3,
                             const RooCmdArg& arg4, const RooCmdArg& arg5,const RooCmdArg& arg6,
                             const RooCmdArg& arg7, const RooCmdArg& arg8,const RooCmdArg& arg9) :
                   RooNLLVar(name, title, pdf, data, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9){
    SetObservable(observable);
    StoreBinning();
    InitGSLWorkspace();
    InitGSLFunction();
}

RombergNLL::RombergNLL(const char *name, const char *title, RooAbsPdf& pdf, RooDataHist& data, RooRealVar& observable,
                             Bool_t extended, const char* rangeName, const char* addCoefRangeName, 
                             Int_t nCPU, RooFit::MPSplit interleave, Bool_t verbose, 
                             Bool_t splitRange, Bool_t cloneData, Bool_t binnedL) :
                   RooNLLVar(name, title, pdf, data, extended, rangeName, addCoefRangeName,
                             nCPU, interleave, verbose, splitRange, cloneData, binnedL){
    SetObservable(observable);
    StoreBinning();
    InitGSLWorkspace();
    InitGSLFunction();
}
  
RombergNLL::RombergNLL(const char *name, const char *title, RooAbsPdf& pdf, RooDataHist& data, RooRealVar& observable,
                             const RooArgSet& projDeps, Bool_t extended, const char* rangeName, 
                             const char* addCoefRangeName, Int_t nCPU, RooFit::MPSplit interleave, Bool_t verbose, 
                             Bool_t splitRange, Bool_t cloneData, Bool_t binnedL) : 
                   RooNLLVar(name, title, pdf, data, projDeps, extended, rangeName, addCoefRangeName, 
                             nCPU, interleave, verbose, splitRange, cloneData, binnedL){
    SetObservable(observable);
    StoreBinning();
    InitGSLWorkspace();
    InitGSLFunction();
}

RombergNLL::RombergNLL(const RombergNLL& other, const char* name) : 
                   RooNLLVar(other, name), 
                   binBoundaries(other.binBoundaries), 
                   m_observable(other.m_observable) {
    InitGSLWorkspace();
    InitGSLFunction();
}

RombergNLL::~RombergNLL(){
        gsl_integration_romberg_free(m_integrationWorkspace);
}

void RombergNLL::SetObservable(RooRealVar& observable){
    auto * pdf = (RooAbsPdf*)_funcClone;
    auto * variables = pdf->getVariables();
    auto & clonedObservable = (*variables)[observable.GetName()];
    m_observable = (RooRealVar*)&clonedObservable;
    delete variables;
}

void RombergNLL::StoreBinning(){
    const auto& binning = m_observable->getBinning();
    size_t nBins = binning.numBins();
    binBoundaries.reserve(nBins+1);
    for (size_t i = 0; i < nBins; i++){
        binBoundaries.push_back(binning.binLow(i));
    }
    binBoundaries.push_back(binning.binHigh(nBins-1));
}

void RombergNLL::InitGSLWorkspace(){
    m_integrationWorkspace = gsl_integration_romberg_alloc(8);
}

void RombergNLL::InitGSLFunction(){
    m_gslFunction.function = &BinnedLikelihood::evaluatePDF;
    m_gslParams.push_back((void*)m_observable);
    m_gslParams.push_back((void*)_funcClone);
    m_gslParams.push_back((void*)_normSet);
    // m_gslParams.push_back((void*)&m_gslNormalisation);
    m_gslFunction.params = &m_gslParams[0];
}

Double_t RombergNLL::evaluatePartition(Int_t firstEvent, Int_t lastEvent, Int_t stepSize) const{
    RooAbsPdf* pdfClone = (RooAbsPdf*) _funcClone ;
    _dataClone->store()->recalculateCache(_projDeps, firstEvent, lastEvent, stepSize, true) ;
    size_t nBins = (lastEvent - firstEvent)/stepSize;
    m_mu.reserve(nBins);
    m_n.reserve(nBins);

    double integral, result(0), carry(0);
    size_t nEval;

    for (auto i=firstEvent; i<lastEvent; i+=stepSize) {
        _dataClone->get(i);
        m_n.push_back(_dataClone->weight());
        double _binLow = binBoundaries[i];
        double _binHigh = binBoundaries[i+1];
        // m_gslNormalisation = pdfClone->getVal(_normSet)/pdfClone->getVal();
        gsl_integration_romberg(&m_gslFunction, _binLow, _binHigh, 0, 1e-4, &integral, &nEval, m_integrationWorkspace);
        m_mu.push_back(integral);
    }

    // Kahan summation of likelihoods
    for (size_t i=0; i<m_mu.size(); i++){
        double y = masked_likelihood(m_mu[i],m_n[i]) - carry;
        double t = result + y;
        carry = (t - result) - y;
        result = t;
    }

    // include the extended maximum likelihood term, if requested
    if(_extended && _setNum==_extSet) {
        if (_weightSq) {
            // Calculate sum of weights-squared here for extended term
            double sumW2(0), sumW2carry(0);
            for (size_t i=0 ; i<_dataClone->numEntries() ; i++) {
                _dataClone->get(i);
                double y = _dataClone->weightSquared() - sumW2carry;
                double t = sumW2 + y;
                sumW2carry = (t - sumW2) - y;
                sumW2 = t;
            }
            double expected = pdfClone->expectedEvents(_dataClone->get());
            double expectedW2 = expected * sumW2 / _dataClone->sumEntries() ;
            double extra = expectedW2 - sumW2*log(expected);
            double y = extra - carry;

            double t = result + y;
            carry = (t - result) - y;
            result = t;
        } 
        else {
            double y = pdfClone->extendedTerm(_dataClone->sumEntries(), _dataClone->get()) - carry;
            double t = result + y;
            carry = (t - result) - y;
            result = t;
        }
    }

    if (_first) {
        _first = kFALSE ;
        _funcClone->wireAllCaches() ;
    }

  // Check if value offset flag is set.
    if (_doOffset) {

        // If no offset is stored enable this feature now
        if (_offset==0 && result !=0 ) {
            coutI(Minimization) << "RombergNLL::evaluatePartition(" << GetName() << ") first = "<< firstEvent << " last = " << lastEvent << " Likelihood offset now set to " << result << std::endl ;
            _offset = result ;
            _offsetCarry = carry;
        }

        // Substract offset
        double y = -_offset - (carry + _offsetCarry);
        double t = result + y;
        carry = (t - result) - y;
        result = t;
    }
    m_n.clear();
    m_mu.clear();

    return result;
}

#endif
