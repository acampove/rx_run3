#ifndef ROO_DOUBLE_SIDED_CB_SHAPE_H
#define ROO_DOUBLE_SIDED_CB_SHAPE_H

#include "RooAbsPdf.h"
#include "RooRealProxy.h"

class RooRealVar;

class RooDoubleSidedCBShape : public RooAbsPdf {
  public:
    RooDoubleSidedCBShape(){};
    RooDoubleSidedCBShape(const char * name, const char * title, RooAbsReal & _x, RooAbsReal & _mu, RooAbsReal & _sigma, RooAbsReal & _alphaLow, RooAbsReal & _nLow, RooAbsReal & _alphaHigh, RooAbsReal & _nHigh);

    RooDoubleSidedCBShape(const RooDoubleSidedCBShape & other, const char * name = 0);
    virtual TObject * clone(const char * newname) const { return new RooDoubleSidedCBShape(*this, newname); }

    inline virtual ~RooDoubleSidedCBShape() {}

    virtual Int_t    getAnalyticalIntegral(RooArgSet & allVars, RooArgSet & analVars, const char * rangeName = 0) const;
    virtual Double_t analyticalIntegral(Int_t code, const char * rangeName = 0) const;

    // Optimized accept/reject generator support
    virtual Int_t    getMaxVal(const RooArgSet & vars) const;
    virtual Double_t maxVal(Int_t code) const;

  protected:
    const double invSqrt2 = 0.70710678;
    const double sqrtPi   = 1.7724539;

    RooRealProxy x;
    RooRealProxy mu;
    RooRealProxy sigma;
    RooRealProxy alphaLow;
    RooRealProxy nLow;
    RooRealProxy alphaHigh;
    RooRealProxy nHigh;

    Double_t evaluate() const;

  private:
    ClassDef(RooDoubleSidedCBShape, 1)   // Double Sided Crystal Ball lineshape PDF
};

#endif
