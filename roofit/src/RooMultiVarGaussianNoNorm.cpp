#ifndef ROOMULTIVARGAUSSIANNONORM_CPP
#define ROOMULTIVARGAUSSIANNONORM_CPP

#include "RooMultiVarGaussianNoNorm.h"

RooMultiVarGaussianNoNorm::RooMultiVarGaussianNoNorm(){};

RooMultiVarGaussianNoNorm::RooMultiVarGaussianNoNorm(const char * name, const char * title, const RooArgList & xvec, const RooArgList & mu, const TMatrixDSym & cov)
    : RooMultiVarGaussian(name, title, xvec, mu, cov){};

RooMultiVarGaussianNoNorm::RooMultiVarGaussianNoNorm(const RooMultiVarGaussianNoNorm & other, const char * name)
    : RooMultiVarGaussian(other, name){};

TObject * RooMultiVarGaussianNoNorm::clone(const char * newname) const { return new RooMultiVarGaussianNoNorm(*this, newname); };

RooMultiVarGaussianNoNorm::~RooMultiVarGaussianNoNorm(){};

Double_t RooMultiVarGaussianNoNorm::getLogVal(const RooArgSet * nset) const { return log(evaluate()); }
// Double_t RooMultiVarGaussianNoNorm::getLogVal(const RooArgSet * nset) const { return log(getVal(nset)*m_normValue); }

#endif
