#ifndef ROOMULTIVARGAUSSIANSUPPRESSWARNING_CPP
#define ROOMULTIVARGAUSSIANSUPPRESSWARNING_CPP

#include "RooMultiVarGaussianSuppressWarning.h"

RooMultiVarGaussianSuppressWarning::RooMultiVarGaussianSuppressWarning(){};

RooMultiVarGaussianSuppressWarning::RooMultiVarGaussianSuppressWarning(const char * name, const char * title, const RooArgList & xvec, const RooArgList & mu, const TMatrixDSym & cov)
    : RooMultiVarGaussian(name, title, xvec, mu, cov){};

RooMultiVarGaussianSuppressWarning::RooMultiVarGaussianSuppressWarning(const RooMultiVarGaussianSuppressWarning & other, const char * name)
    : RooMultiVarGaussian(other, name){};

TObject * RooMultiVarGaussianSuppressWarning::clone(const char * newname) const { return new RooMultiVarGaussianSuppressWarning(*this, newname); };

RooMultiVarGaussianSuppressWarning::~RooMultiVarGaussianSuppressWarning(){};

Double_t RooMultiVarGaussianSuppressWarning::getLogVal(const RooArgSet * nset) const { return log(getVal(nset)); }

#endif
