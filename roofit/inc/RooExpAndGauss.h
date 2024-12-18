#ifndef ROOEXPANDGAUSS
#define ROOEXPANDGAUSS

#include "RooAbsPdf.h"
#include "RooAbsReal.h"
#include "RooRealProxy.h"

class RooExpAndGauss : public RooAbsPdf {

  public:
    RooExpAndGauss();   // default constructor to make RooFit workspaces happy
    RooExpAndGauss(const char * name, const char * title, RooAbsReal & _x, RooAbsReal & _sh_mean, RooAbsReal & _sh_sigma, RooAbsReal & _sh_trans);
    RooExpAndGauss(const RooExpAndGauss & other, const char * name = 0);
    virtual TObject * clone(const char * newname) const { return new RooExpAndGauss(*this, newname); }
    inline virtual ~RooExpAndGauss() {}

  protected:
    RooRealProxy x;
    RooRealProxy sh_mean;
    RooRealProxy sh_sigma;
    RooRealProxy sh_trans;
    Double_t     evaluate() const;

  public:
    ClassDef(RooExpAndGauss, 1)
};

#endif
