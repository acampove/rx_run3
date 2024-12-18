/*****************************************************************************
 * Project: RooFit                                                           *
 * Package: RooFitModels                                                     *
 *    File: $Id: RooBifurDSCBShape.h$                                             *
 * Authors:                                                                  *
 *    T. Skwarnicki modify RooCBShape to Asymmetrical Double-Sided CB        *
 *    Michael Wilkinson add to RooFit source   
 *    Renato Quagliani (edit https://gitlab.cern.ch/lhcb/Erasmus/blob/b58c1441bb4233b11b0b5bb5816fa268add7c97c/Bu2D0H/Bu2D0H_FIT/src/RooDoubleCB.h 
 *                      to match https://root.cern.ch/doc/master/classRooCrystalBall.html *
 *****************************************************************************/
#ifndef ROO_BIFURDSCB_SHAPE
#define ROO_BIFURDSCB_SHAPE

#include "RooAbsPdf.h"
#include "RooRealProxy.h"
#include "RooAbsReal.h"

class RooRealVar;

class RooBifurDSCBShape : public RooAbsPdf {
public:
  RooBifurDSCBShape() {} ;
  RooBifurDSCBShape(const char *name, 
       const char *title,
       RooAbsReal& _m,
       RooAbsReal& _m0, 
       RooAbsReal& _sigmaL, 
       RooAbsReal& _sigmaR, 
       RooAbsReal& _alphaL, 
       RooAbsReal& _nL,
       RooAbsReal& _alphaR, 
       RooAbsReal& _nR);
  RooBifurDSCBShape( const char * name, 
		     const char * title, 
		     RooAbsReal& _m, 
		     RooAbsReal& _m0, 
		     RooAbsReal& _sigma, 
		     RooAbsReal& _alphaL,
		     RooAbsReal& _nL, 
		     RooAbsReal& _alphaR, 
		     RooAbsReal& _nR);
		     
  RooBifurDSCBShape(const RooBifurDSCBShape& other, const char* name = 0);
  virtual TObject* clone(const char* newname) const { return new RooBifurDSCBShape(*this,newname); }

  inline virtual ~RooBifurDSCBShape() { }

  virtual Int_t getAnalyticalIntegral( RooArgSet& allVars,  RooArgSet& analVars, const char* rangeName=0 ) const;
  virtual Double_t analyticalIntegral( Int_t code, const char* rangeName=0 ) const;

  // Optimized accept/reject generator support
  virtual Int_t getMaxVal(const RooArgSet& vars) const ;
  virtual Double_t maxVal(Int_t code) const ;

protected:

  RooRealProxy x_;
  RooRealProxy x0_;
  RooRealProxy sigmaL_;
  RooRealProxy alphaL_;
  RooRealProxy nL_;
  RooRealProxy alphaR_;
  RooRealProxy sigmaR_;
  RooRealProxy nR_;

  Double_t evaluate() const;

private:

  ClassDef(RooBifurDSCBShape,1) // Asymmetrical Double-Sided Crystal Ball lineshape PDF
};

#endif
