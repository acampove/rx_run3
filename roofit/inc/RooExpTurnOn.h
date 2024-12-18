/*****************************************************************************
 * Project: RooFit                                                           *
 *                                                                           *
 * This code was autogenerated by RooClassFactory                            *
 *****************************************************************************/

#ifndef RooExpTurnOn_H
#define RooExpTurnOn_H

#include "RooAbsCategory.h"
#include "RooAbsPdf.h"
#include "RooAbsReal.h"
#include "RooCategoryProxy.h"
#include "RooRealProxy.h"

class RooAbsReal;
 
class RooExpTurnOn : public RooAbsPdf {
public:
  RooExpTurnOn() {    } ;
  RooExpTurnOn(const char *name, const char *title, RooAbsReal& _x, RooAbsReal& _mTurnOn, RooAbsReal& _sTurnOn, RooAbsReal & _bCombo);
  RooExpTurnOn(const RooExpTurnOn& other, const char* name=0) ;
  virtual TObject* clone(const char* newname) const { return new RooExpTurnOn(*this,newname); }
  inline virtual ~RooExpTurnOn() {  }
  double hyperg_z_GT1( double a, double b, double c, double z) const;
  double hyperg_z( double a, double b, double c, double z) const ;
  double getA()const ;
  double getB()const ;
  double getC()const ;
  double getZ( double MASS )const ;
  double IntegralVal( double xVal) const;
  Int_t getAnalyticalIntegral(RooArgSet& allVars, RooArgSet& analVars, const char* rangeName=0) const ;
  double analyticalIntegral(Int_t code, const char* rangeName=0) const ;
protected:
  RooRealProxy x ;
  RooRealProxy mO ;
  RooRealProxy sE ;
  RooRealProxy bComb ;
  double evaluate() const ;
private:
  ClassDef(RooExpTurnOn,1) // RooExpTurnOn PDF
};
 
#endif
