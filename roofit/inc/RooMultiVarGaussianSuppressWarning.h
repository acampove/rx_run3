#ifndef ROOMULTIVARGAUSSIANSUPPRESSWARNING_H
#define ROOMULTIVARGAUSSIANSUPPRESSWARNING_H

#include "RooMultiVarGaussian.h"

class RooMultiVarGaussianSuppressWarning : public RooMultiVarGaussian {

  public:
    RooMultiVarGaussianSuppressWarning();
    RooMultiVarGaussianSuppressWarning(const char * name, const char * title, const RooArgList & xvec, const RooArgList & mu, const TMatrixDSym & cov);
    RooMultiVarGaussianSuppressWarning(const RooMultiVarGaussianSuppressWarning & other, const char * name);
    TObject * clone(const char * newname) const;
    ~RooMultiVarGaussianSuppressWarning();
    Double_t getLogVal(const RooArgSet * nset = 0) const;

  private:
    ClassDef(RooMultiVarGaussianSuppressWarning, 1);
};

#endif
