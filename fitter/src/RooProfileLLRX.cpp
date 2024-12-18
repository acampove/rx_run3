/*****************************************************************************
 * Reimplementation of RooProfileLL from RooFit (root 6.18)                  *
 * Can initialise the likelihood pointer if it's null                        *
 *****************************************************************************/
 
#include "Riostream.h" 
 
#include "RooFit.h"
#include "RooProfileLL.h" 
#include "RooProfileLLRX.hpp" 
#include "RooAbsReal.h" 
#include "RooMinimizer.h"
#include "RooMsgService.h"
#include "RooRealVar.h"
#include "RooMsgService.h"
#include "SettingDef.hpp"
#include "RooFitResult.h"

using namespace std ;
 
ClassImp(RooProfileLLRX);
////////////////////////////////////////////////////////////////////////////////
/// Default constructor 
/// Should only be used by proof. 
 
 RooProfileLLRX::RooProfileLLRX() : 
   RooAbsReal("RooProfileLLRX","RooProfileLLRX"), 
   _nll(), 
   _obs("paramOfInterest","Parameters of interest",this), 
   _par("nuisanceParam","Nuisance parameters",this,kFALSE,kFALSE), 
   _startFromMin(kTRUE), 
   _absMinValid(kFALSE), 
   _absMin(0),
   _neval(0){
   std::cout<<"Default Constructor!"<<std::endl;
  _piter = _par.createIterator() ; 
  _oiter = _obs.createIterator() ; 
    //  _minimizer(0), 
}

////////////////////////////////////////////////////////////////////////////////
/// Constructor of profile likelihood given input likelihood nll w.r.t
/// the given set of variables. The input log likelihood is minimized w.r.t
/// to all other variables of the likelihood at each evaluation and the
/// value of the global log likelihood minimum is always subtracted.
 
RooProfileLLRX::RooProfileLLRX(const char *name, const char *title, 
			       RooAddition& nllIn, const RooArgSet& observables) :
  RooAbsReal(name,title), 
  _nll(nllIn, "-log(likelihood,profile)"),
  _obs("paramOfInterest","Parameters of interest",this),
  _par("nuisanceParam","Nuisance parameters",this,kFALSE,kFALSE),
  _startFromMin(kTRUE),
  _absMinValid(kFALSE),
  _absMin(0),
  _neval(0){ 
  // _nll.enableOffsetting();
  // Determine actual parameters and observables
  RooArgSet* actualObs = nllIn.getObservables(observables) ;
  RooArgSet* actualPars = nllIn.getParameters(observables) ; 
  _obs.add(*actualObs) ;
  _par.add(*actualPars) ;
  delete actualObs ;
  delete actualPars ;
  _piter = _par.createIterator() ;
  _oiter = _obs.createIterator() ;
  m_minType="Minuit2";m_minAlg="migrad";initializeMinimizer();
}
////////////////////////////////////////////////////////////////////////////////
/// Copy constructor
 
RooProfileLLRX::RooProfileLLRX(const RooProfileLLRX& other, const char* name) :  
  RooAbsReal(other,name), 
  _nll(other._nll, "log(likelihood-copyconstructed"),
  _obs("obs",this,other._obs),
  _par("par",this,other._par),
  _startFromMin(other._startFromMin),  
  _absMinValid(kFALSE),
  _absMin(0),
  _paramFixed(other._paramFixed),
  _neval(0)
{ 
  _piter = _par.createIterator() ;
  _oiter = _obs.createIterator() ;
  _paramAbsMin.addClone(other._paramAbsMin) ;
  _obsAbsMin.addClone(other._obsAbsMin) ;
  initializeMinimizer();}
 
 
////////////////////////////////////////////////////////////////////////////////
/// Destructor
 
RooProfileLLRX::~RooProfileLLRX()
{
  // Delete instance of minuit if it was ever instantiated
  if (_minimizer) {
    _minimizer.reset(nullptr);
    // delete _minimizer ;
  }
 
  delete _piter ;
  delete _oiter ;
}
 
 
 
 
////////////////////////////////////////////////////////////////////////////////
 
const RooArgSet& RooProfileLLRX::bestFitParams() const{
  validateAbsMin() ;
  return _paramAbsMin ;
}
////////////////////////////////////////////////////////////////////////////////
 
const RooArgSet& RooProfileLLRX::bestFitObs() const 
{
  validateAbsMin() ;
  return _obsAbsMin ;
}
 
 
 
 
////////////////////////////////////////////////////////////////////////////////
/// Optimized implementation of createProfile for profile likelihoods.
/// Return profile of original function in terms of stated parameters 
/// of interest rather than profiling recursively.
 
RooAbsReal* RooProfileLLRX::createProfile(const RooArgSet& paramsOfInterest) 
{
  return nll().createProfile(paramsOfInterest) ;
}
 
 
RooProfileLLRX* RooProfileLLRX::createProfileLLRX(RooAddition & nLL , const RooArgSet & paramsOfInterest){ 
  auto name = std::string(nLL.GetName()) + "_Profile[";
  bool first = true;
  for (auto const& arg : paramsOfInterest) {
    if (first) {
      first = false ;
    } else {
      name.append(",") ;
    }
    name.append(arg->GetName()) ;
  }
  name.append("]") ;
  // Create and return profile object
  return new RooProfileLLRX(name.c_str(),(std::string("Profile of ") + nLL.GetTitle()).c_str(), nLL ,paramsOfInterest);
}

void RooProfileLLRX::initializeMinimizer() const{
  coutI(Minimization) << "RooProfileLLRX::initializeMinimizer(" << GetName() << ") Creating instance of MINUIT" << endl ;
  Bool_t smode = RooMsgService::instance().silentMode() ;
  RooMsgService::instance().setSilentMode(kFALSE) ;
  _nll.enableOffsetting(kTRUE);
  _minimizer = std::make_unique<RooMinimizer>(_nll);
  _minimizer->setMinimizerType("Minuit2");
  _minimizer->setEvalErrorWall(kTRUE);
  _minimizer->setStrategy(1);
  _minimizer->setOffsetting(1);
  _minimizer->setMaxFunctionCalls(10000000);
  _minimizer->setMaxIterations(10000000);
  _minimizer->setOffsetting(kTRUE);
  _minimizer->optimizeConst(kFALSE);
  if (!smode) RooMsgService::instance().setSilentMode(kFALSE) ;  
  return;
}
 
Double_t RooProfileLLRX::evaluate() const{ 
  std::cout<<"evaluate() call number" << m_nFits<<std::endl;
  // Instantiate minimizer if we haven't done that already
  if (_minimizer==nullptr){
    std::cout<< "Minimizer is nullptr, initialize it"<<std::endl;
    initializeMinimizer() ;
  }
  // Save current value of observables
  RooArgSet* obsSetOrig = (RooArgSet*) _obs.snapshot() ;
  validateAbsMin();
  // Set all observables constant in the minimization
  const_cast<RooSetProxy&>(_obs).setAttribAll("Constant",kTRUE);
  ccoutP(Eval) << "." ; ccoutP(Eval).flush();
  // If requested set initial parameters to those corresponding to absolute minimum
  if (_startFromMin) {
    const_cast<RooProfileLLRX&>(*this)._par = _paramAbsMin ;
  }
  _minimizer->zeroEvalCount() ;
  int m_maxFunctionCalls= 10000000;
  _minimizer->setMaxFunctionCalls(m_maxFunctionCalls);
  _minimizer->setMaxIterations(m_maxFunctionCalls);
  _minimizer->setEvalErrorWall(1);
  int _status = _minimizer->minimize("Minuit2", m_minAlg);
  if(_status != 0) std::cout<< "ProfileLLRX minimize call has failed (evaluate) status !=0"<<std::endl;  
  m_nFits +=1;
  _neval = _minimizer->evalCounter() ;
  // Restore original values and constant status of observables
  TIterator* iter = obsSetOrig->createIterator() ;
  RooRealVar* var ;
  while((var=dynamic_cast<RooRealVar*>(iter->Next()) ) )  {
    RooRealVar* target = (RooRealVar*) _obs.find(var->GetName()) ;
    target->setVal(var->getVal());
    target->setConstant(var->isConstant()) ;
  }
  delete iter ;
  delete obsSetOrig ;
  return _nll.getVal() - _absMin;
}
////////////////////////////////////////////////////////////////////////////////
/// Check that parameters and likelihood value for 'best fit' are still valid. If not,
/// because the best fit has never been calculated, or because constant parameters have
/// changed value or parameters have changed const/float status, the minimum is recalculated
void RooProfileLLRX::validateAbsMin() const{
  std::cout<<"validateAbsMin "<<std::endl;
  // Check if constant status of any of the parameters have changed
  if(_absMinValid){
    _piter->Reset();
    RooAbsArg* par ;
    while((par=(RooAbsArg*)_piter->Next())) {
      if (_paramFixed[par->GetName()] != par->isConstant()) {
        cxcoutI(Minimization) << "RooProfileLLRX::evaluate(" << GetName() << ") constant status of parameter " << par->GetName() << " has changed from " 
			      << (_paramFixed[par->GetName()]?"fixed":"floating") << " to " << (par->isConstant()?"fixed":"floating") 
			      << ", recalculating absolute minimum" << endl ;
        _absMinValid = kFALSE ;
        break ;
      }
    }
  }
  // If we don't have the absolute minimum w.r.t all observables, calculate that first
  if (!_absMinValid) {
    cxcoutI(Minimization) << "RooProfileLLRX::evaluate(" << GetName() << ") determining minimum likelihood for current configurations w.r.t all observable, validateAbsMin" << endl ;    
    if (_minimizer==nullptr) {
      std::cout<<"initializeMinimizer() from validateAbsMin"<<std::endl;
      initializeMinimizer() ;
    }
    // Save current values of non-marginalized parameters
    RooArgSet* obsStart = (RooArgSet*) _obs.snapshot(kFALSE) ;
    // Start from previous global minimum 
    if (_paramAbsMin.getSize()>0) {
      const_cast<RooSetProxy&>(_par).assignValueOnly(_paramAbsMin) ;
    }
    if (_obsAbsMin.getSize()>0) {
      const_cast<RooSetProxy&>(_obs).assignValueOnly(_obsAbsMin) ;
    }
    // Find minimum with all observables floating
    const_cast<RooSetProxy&>(_obs).setAttribAll("Constant",kFALSE);
    auto _status = _minimizer->minimize("Minuit2",m_minAlg.Data());    
    m_nFits +=1;
    if(_status != 0) std::cout<< "ProfileLLRX minimize (validateAbsMin) call has failed (initialize Minimizer) status !=0"<<std::endl;    
    auto _FITRESULTS_ = _minimizer->save("ProfileLLRX minimize (validateAbsMin)");
    if (!SettingDef::IO::outDir.EndsWith("/")) SettingDef::IO::outDir += "/";
    auto _name = SettingDef::IO::outDir + "ProfileLLRX_validateAbsMin_Results.log";
    auto _file = ofstream(_name);
    _FITRESULTS_->printMultiline(_file, 1111, kTRUE, "\t");
    _file.close();
    // Save value and remember
    _absMin = _nll.getVal();
    _absMinValid = kTRUE;
    // Save parameter values at abs minimum as well
    _paramAbsMin.removeAll() ;
    // Only store non-constant parameters here!
    RooArgSet* tmp = (RooArgSet*) _par.selectByAttrib("Constant",kFALSE) ;
    _paramAbsMin.addClone(*tmp) ;
    delete tmp ;
    _obsAbsMin.addClone(_obs) ;
    // Save constant status of all parameters
    _piter->Reset() ;
    RooAbsArg* par;
    while((par=(RooAbsArg*)_piter->Next())) {
      _paramFixed[par->GetName()] = par->isConstant() ;
    }  
    if (dologI(Minimization)) {
      cxcoutI(Minimization) << "RooProfileLLRX::evaluate(" << GetName() << ") minimum found at (" ;
      RooAbsReal* arg ;
      Bool_t first=kTRUE ;
      _oiter->Reset() ;
      while ((arg=(RooAbsReal*)_oiter->Next())) {
        ccxcoutI(Minimization) << (first?"":", ") << arg->GetName() << "=" << arg->getVal();
        first=kFALSE ;
      }
      ccxcoutI(Minimization) << ")" << endl;
    }
    // Restore original parameter values
    const_cast<RooSetProxy&>(_obs) = *obsStart ;
    delete obsStart ; 
  }
}
 
 
 
////////////////////////////////////////////////////////////////////////////////
 
Bool_t RooProfileLLRX::redirectServersHook(const RooAbsCollection& /*newServerList*/, Bool_t /*mustReplaceAll*/, 
                Bool_t /*nameChange*/, Bool_t /*isRecursive*/) 
{ 

  if (_minimizer) {
    std::cout<<"RedirectServersHook, resetting minimizer pointer"<<std::endl;
    _minimizer.reset(nullptr);    
  }  
  return kFALSE ;
}
 
