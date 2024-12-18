/*****************************************************************************
 * Reimplementation of RooProfileLL from RooFit (root 6.18)                  *
 * Can initialise the likelihood pointer if it's null                        *
 *****************************************************************************/
 
#ifndef ROOPROFILELLRX
#define ROOPROFILELLRX
 
#include "RooAbsReal.h"
#include "RooRealProxy.h"
#include "RooSetProxy.h"
#include "RooMinimizer.h"
#include <map>
#include <string>
#include "RooAddition.h"
class RooProfileLLRX : public RooAbsReal {
public:
 
  RooProfileLLRX() ;
  RooProfileLLRX(const char *name, const char *title, RooAddition& nll, const RooArgSet& observables);
  RooProfileLLRX(const RooProfileLLRX& other, const char* name=0) ;
  virtual TObject* clone(const char* newname) const { return new RooProfileLLRX(*this,newname); }
  virtual ~RooProfileLLRX() ;
 
  void setAlwaysStartFromMin(Bool_t flag) { _startFromMin = flag ; }
  Bool_t alwaysStartFromMin() const { return _startFromMin ; }
 
  RooAddition & nll() { return _nll ; }
  const RooArgSet& bestFitParams() const ;
  const RooArgSet& bestFitObs() const ;
 
  virtual RooAbsReal* createProfile(const RooArgSet& paramsOfInterest) ;
  static RooProfileLLRX* createProfileLLRX(RooAddition & nLL , const RooArgSet & paramsOfInterest);

  virtual Bool_t redirectServersHook(const RooAbsCollection& /*newServerList*/, Bool_t /*mustReplaceAll*/, Bool_t /*nameChange*/, Bool_t /*isRecursive*/) ;
 
  void clearAbsMin() { _absMinValid = kFALSE ; }
 
  Int_t numEval() const { return _neval ; }
 
  void initializeMinimizer() const ;
  RooMinimizer* minimizer() {
    if(_minimizer == nullptr) initializeMinimizer();
    return _minimizer.get();  
  }
  
  void SetMinTypeAlg( TString _type, TString _algo){
    m_minType = _type;
    m_minAlg = _algo;
    return;
  }
  void PrintSettings(){
    std::cout<<"RooProfileLLRX::Type   "<<  m_minType<<std::endl;
    std::cout<<"RooProfileLLRX::Algo   "<<  m_minAlg<<std::endl;
  }
  void SetIsAtMin(){
    _absMinValid = true;
  }
protected:
 
  void validateAbsMin() const ;
  //mutable it's a work-around from ROOT to allow calls to method
  mutable RooAddition _nll ;    // Input -log(L) function
  RooSetProxy _obs ;     // Parameters of profile likelihood
  RooSetProxy _par ;     // Marginialized parameters of likelihood
  Bool_t _startFromMin ; // Always start minimization for global minimum?
  TIterator* _piter ; //! Iterator over profile likelihood parameters to be minimized
  TIterator* _oiter ; //! Iterator of profile likelihood output parameter(s)

  mutable std::unique_ptr<RooMinimizer> _minimizer = nullptr ; ///<! Internal minimizer instance
 
  mutable Bool_t _absMinValid ; // flag if absmin is up-to-date
  mutable Double_t _absMin ; // absolute minimum of -log(L)
  mutable RooArgSet _paramAbsMin ; // Parameter values at absolute minimum
  mutable RooArgSet _obsAbsMin ; // Observable values at absolute minimum
  mutable std::map<std::string,bool> _paramFixed ; // Parameter constant status at last time of use
  mutable Int_t _neval ; // Number evaluations used in last minimization
  Double_t evaluate() const ;
  
  TString m_minType          = "Minuit";   //< allowed Minuit2
  TString m_minAlg           = "migrad";
  mutable int m_nFits = 0;
private:
  ClassDef(RooProfileLLRX,0) // Real-valued function representing profile likelihood of external (likelihood) function
};
 
#endif
