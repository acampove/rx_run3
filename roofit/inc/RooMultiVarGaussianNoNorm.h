#ifndef ROOMULTIVARGAUSSIANNONORM_H
#define ROOMULTIVARGAUSSIANNONORM_H

#include "RooMultiVarGaussian.h"
#include "TMath.h"

class RooMultiVarGaussianNoNorm : public RooMultiVarGaussian {

  public:
    RooMultiVarGaussianNoNorm();
    RooMultiVarGaussianNoNorm(const char * name, const char * title, const RooArgList & xvec, const RooArgList & mu, const TMatrixDSym & cov);
    RooMultiVarGaussianNoNorm(const RooMultiVarGaussianNoNorm & other, const char * name);
    TObject * clone(const char * newname) const;
    ~RooMultiVarGaussianNoNorm();
    Double_t getLogVal(const RooArgSet * nset = 0) const;

  private:
    ClassDef(RooMultiVarGaussianNoNorm, 1);
    // _covI: Inverted covariance Matrix, inherited from RooMultiVarGaussian
    // _cov : covariance Matrix, inherited from RooMultiVarGaussian
    Double_t m_normValue = TMath::Sqrt(TMath::Abs(_cov.Determinant())); 
};

#endif
