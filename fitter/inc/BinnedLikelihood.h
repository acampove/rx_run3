#ifndef BINNEDLIKELIHOOD_H
#define BINNEDLIKELIHOOD_H

#include "RooRealVar.h"
#include "RooNLLVar.h"
#include "RooDataHist.h"
#include "RooCmdArg.h"
#include "RooAbsReal.h"
#include "RooAbsPdf.h"
#include "RooAbsData.h"
#include "RooLinkedList.h"

#include <memory>
#include <functional>
#include <cmath>
#include <gsl/gsl_integration.h>


namespace BinnedLikelihood{
    RooAbsReal * CreateNLL(RooAbsPdf& _model, RooDataHist& _binnedData, RooRealVar& observable, const RooLinkedList& cmdList);

    RooAbsReal * CreateNLL(RooAbsPdf& _model, RooDataHist& _binnedData, RooRealVar& observable, 
                const RooCmdArg& arg1=RooCmdArg::none(),  const RooCmdArg& arg2=RooCmdArg::none(),  
                const RooCmdArg& arg3=RooCmdArg::none(),  const RooCmdArg& arg4=RooCmdArg::none(), const RooCmdArg& arg5=RooCmdArg::none(),  
                const RooCmdArg& arg6=RooCmdArg::none(),  const RooCmdArg& arg7=RooCmdArg::none(), const RooCmdArg& arg8=RooCmdArg::none());

    RooArgSet* getAllConstraints(RooAbsPdf& model, const RooArgSet& observables, RooArgSet& constrainedParams, Bool_t stripDisconnected);

    double evaluatePDF(double x, void * params);
}

class AdaptiveNLL : public RooNLLVar {

public:
    // Constructors, assignment etc
    AdaptiveNLL();
    AdaptiveNLL(const char *name, const char* title, RooAbsPdf& pdf, RooDataHist& data, RooRealVar& observable,
                const RooCmdArg& arg1=RooCmdArg::none(), const RooCmdArg& arg2=RooCmdArg::none(),const RooCmdArg& arg3=RooCmdArg::none(),
                const RooCmdArg& arg4=RooCmdArg::none(), const RooCmdArg& arg5=RooCmdArg::none(),const RooCmdArg& arg6=RooCmdArg::none(),
                const RooCmdArg& arg7=RooCmdArg::none(), const RooCmdArg& arg8=RooCmdArg::none(),const RooCmdArg& arg9=RooCmdArg::none());

    AdaptiveNLL(const char *name, const char *title, RooAbsPdf& pdf, RooDataHist& data, RooRealVar& observable,
                Bool_t extended, const char* rangeName=0, const char* addCoefRangeName=0, 
                Int_t nCPU=1, RooFit::MPSplit interleave=RooFit::BulkPartition, Bool_t verbose=kTRUE, 
                Bool_t splitRange=kFALSE, Bool_t cloneData=kTRUE, Bool_t binnedL=kFALSE);
  
    AdaptiveNLL(const char *name, const char *title, RooAbsPdf& pdf, RooDataHist& data, RooRealVar& observable,
                const RooArgSet& projDeps, Bool_t extended=kFALSE, const char* rangeName=0, 
                const char* addCoefRangeName=0, Int_t nCPU=1, RooFit::MPSplit interleave=RooFit::BulkPartition, Bool_t verbose=kTRUE, 
                Bool_t splitRange=kFALSE, Bool_t cloneData=kTRUE, Bool_t binnedL=kFALSE);

    AdaptiveNLL(const AdaptiveNLL& other, const char* name=0);

    ~AdaptiveNLL();

protected:
    Double_t evaluatePartition(Int_t firstEvent, Int_t lastEvent, Int_t stepSize) const;

private:
    void SetObservable(RooRealVar& observable);
    void StoreBinning();
    void InitGSLWorkspace();
    void InitGSLFunction();
    double EvaluatePDF(double x, void * params);

private:
    std::vector <double> binBoundaries;
    mutable std::vector <double> m_mu;
    mutable std::vector <double> m_n;
    mutable RooRealVar * m_observable;
    
    mutable std::vector <void *> m_gslParams;
    gsl_function m_gslFunction;
    mutable gsl_integration_workspace * m_integrationWorkspace;

    const std::function<double(double, double)> masked_likelihood = [] (double mu, double n) { return (mu > 1e-15 && n > 1e-10) ? -n * log(mu) : 0; };
    //ClassDef( AdaptiveNLL,0);//0 no stremer to disk enabling (some methods to add for Buffers 
};

class AnalyticalNLL : public RooNLLVar {

public:
    // Constructors, assignment etc
    AnalyticalNLL();
    AnalyticalNLL(const char *name, const char* title, RooAbsPdf& pdf, RooDataHist& data, RooRealVar& observable,
                const RooCmdArg& arg1=RooCmdArg::none(), const RooCmdArg& arg2=RooCmdArg::none(),const RooCmdArg& arg3=RooCmdArg::none(),
                const RooCmdArg& arg4=RooCmdArg::none(), const RooCmdArg& arg5=RooCmdArg::none(),const RooCmdArg& arg6=RooCmdArg::none(),
                const RooCmdArg& arg7=RooCmdArg::none(), const RooCmdArg& arg8=RooCmdArg::none(),const RooCmdArg& arg9=RooCmdArg::none());

    AnalyticalNLL(const char *name, const char *title, RooAbsPdf& pdf, RooDataHist& data, RooRealVar& observable,
                Bool_t extended, const char* rangeName=0, const char* addCoefRangeName=0, 
                Int_t nCPU=1, RooFit::MPSplit interleave=RooFit::BulkPartition, Bool_t verbose=kTRUE, 
                Bool_t splitRange=kFALSE, Bool_t cloneData=kTRUE, Bool_t binnedL=kFALSE);
  
    AnalyticalNLL(const char *name, const char *title, RooAbsPdf& pdf, RooDataHist& data, RooRealVar& observable,
                const RooArgSet& projDeps, Bool_t extended=kFALSE, const char* rangeName=0, 
                const char* addCoefRangeName=0, Int_t nCPU=1, RooFit::MPSplit interleave=RooFit::BulkPartition, Bool_t verbose=kTRUE, 
                Bool_t splitRange=kFALSE, Bool_t cloneData=kTRUE, Bool_t binnedL=kFALSE);

    AnalyticalNLL(const AnalyticalNLL& other, const char* name=0);

    static int hasFullAnalyticalIntegral(const RooAbsPdf& model, RooArgSet& allVars, RooArgSet& analVars, const char * rangeName);

protected:
    Double_t evaluatePartition(Int_t firstEvent, Int_t lastEvent, Int_t stepSize) const;

private:
    void SetObservable(RooRealVar& observable);
    void StoreBinning();
    void InitBinIntegral();

private:
    std::vector <double> binBoundaries;
    mutable std::vector <double> m_mu;
    mutable std::vector <double> m_n;
    mutable RooRealVar * m_observable;
    mutable std::unique_ptr<RooRealVar> m_lowObservable;
    mutable std::unique_ptr<RooRealVar> m_highObservable;
    std::unique_ptr<RooAbsReal> m_binIntegral;

    const std::function<double(double, double)> masked_likelihood = [] (double mu, double n) { return (mu > 1e-15 && n > 1e-10) ? -n * log(mu) : 0; };
    //ClassDef(AnalyticalNLL,0);//0 no stremer to disk enabling (some methods to add for Buffers 
};

class RombergNLL : public RooNLLVar {

public:
    // Constructors, assignment etc
    RombergNLL();
    RombergNLL(const char *name, const char* title, RooAbsPdf& pdf, RooDataHist& data, RooRealVar& observable,
                  const RooCmdArg& arg1=RooCmdArg::none(), const RooCmdArg& arg2=RooCmdArg::none(),const RooCmdArg& arg3=RooCmdArg::none(),
                  const RooCmdArg& arg4=RooCmdArg::none(), const RooCmdArg& arg5=RooCmdArg::none(),const RooCmdArg& arg6=RooCmdArg::none(),
                  const RooCmdArg& arg7=RooCmdArg::none(), const RooCmdArg& arg8=RooCmdArg::none(),const RooCmdArg& arg9=RooCmdArg::none());

    RombergNLL(const char *name, const char *title, RooAbsPdf& pdf, RooDataHist& data, RooRealVar& observable,
                 Bool_t extended, const char* rangeName=0, const char* addCoefRangeName=0, 
                 Int_t nCPU=1, RooFit::MPSplit interleave=RooFit::BulkPartition, Bool_t verbose=kTRUE, 
                 Bool_t splitRange=kFALSE, Bool_t cloneData=kTRUE, Bool_t binnedL=kFALSE);
  
    RombergNLL(const char *name, const char *title, RooAbsPdf& pdf, RooDataHist& data, RooRealVar& observable,
                  const RooArgSet& projDeps, Bool_t extended=kFALSE, const char* rangeName=0, 
                  const char* addCoefRangeName=0, Int_t nCPU=1, RooFit::MPSplit interleave=RooFit::BulkPartition, Bool_t verbose=kTRUE, 
                  Bool_t splitRange=kFALSE, Bool_t cloneData=kTRUE, Bool_t binnedL=kFALSE);

    RombergNLL(const RombergNLL& other, const char* name=0);

    ~RombergNLL();

protected:
    Double_t evaluatePartition(Int_t firstEvent, Int_t lastEvent, Int_t stepSize) const;

private:
    void SetObservable(RooRealVar& observable);
    void StoreBinning();
    void InitGSLWorkspace();
    void InitGSLFunction();
    double EvaluatePDF(double x, void * params);

private:
    std::vector <double> binBoundaries;
    mutable std::vector <double> m_mu;
    mutable std::vector <double> m_n;
    mutable RooRealVar * m_observable;
    
    mutable std::vector <void *> m_gslParams;
    gsl_function m_gslFunction;
    mutable gsl_integration_romberg_workspace * m_integrationWorkspace;
    // mutable double m_gslNormalisation;

    const std::function<double(double, double)> masked_likelihood = [] (double mu, double n) { return (mu > 1e-15 && n > 1e-10) ? -n * log(mu) : 0; };
    //ClassDef( RombergNLL,0);//0-no writing streamer defined
};

#endif
