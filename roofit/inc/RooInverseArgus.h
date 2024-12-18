#ifndef ROOINVERSEARGUS
#define ROOINVERSEARGUS

#include "RooAbsCategory.h"
#include "RooAbsPdf.h"
#include "RooAbsReal.h"
#include "RooCategoryProxy.h"
#include "RooRealProxy.h"

class RooInverseArgus : public RooAbsPdf {

  public:
    RooInverseArgus(){};
    RooInverseArgus(const char * name, const char * title, RooAbsReal & _m, RooAbsReal & _m0, RooAbsReal & _c, RooAbsReal & _p);
    RooInverseArgus(const RooInverseArgus & other, const char * name = 0);
    virtual TObject * clone(const char * newname) const { return new RooInverseArgus(*this, newname); }
    inline virtual ~RooInverseArgus() {}

    // Int_t    getAnalyticalIntegral(RooArgSet & allVars, RooArgSet & analVars, const char * rangeName = 0) const;
    // Double_t analyticalIntegral(Int_t code, const char * rangeName = 0) const;

  protected:
    RooRealProxy m;
    RooRealProxy m0;
    RooRealProxy c;
    RooRealProxy p;
    Double_t     evaluate() const;

  public:
    ClassDef(RooInverseArgus, 1)
};

#endif
