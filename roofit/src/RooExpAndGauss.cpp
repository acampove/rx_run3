#include "RooExpAndGauss.h"
#include "RooRealVar.h"

ClassImp(RooExpAndGauss)

    // default constructor to make RooFit workspaces happy
    /*
    RooExpAndGauss::RooExpAndGauss():
          RooAbsPdf("RooExpAndGaussDefault", "RooExpAndGauss Default"),
          x("xRV", "x", 0.0, 1.0),
          sh_mean("sh_mean", "sh_mean", 0.0, 1.0),
          sh_sigma("sh_sigma","sh_sigma", 0.0, 1.0),
          sh_trans("sh_trans","sh_trans", 0.0, 1.0)
    {
    }
    */

    RooExpAndGauss::RooExpAndGauss() {
    RooRealVar xRV("xRV", "x", 0.0, 1.0);
    RooRealVar sh_meanRV("sh_meanRV", "sh_mean", 0.0, 1.0);
    RooRealVar sh_sigmaRV("sh_sigmaRV", "sh_sigma", 0.0, 1.0);
    RooRealVar sh_transRV("sh_transRV", "sh_trans", 0.0, 1.0);

    RooExpAndGauss("RooExpAndGauss", "RooExpAndGauss", xRV, sh_meanRV, sh_sigmaRV, sh_transRV);
}

RooExpAndGauss::RooExpAndGauss(const char * name, const char * title, RooAbsReal & _x, RooAbsReal & _sh_mean, RooAbsReal & _sh_sigma, RooAbsReal & _sh_trans)
    : RooAbsPdf(name, title)
    , x("x", "x", this, _x)
    , sh_mean("sh_mean", "sh_mean", this, _sh_mean)
    , sh_sigma("sh_sigma", "sh_sigma", this, _sh_sigma)
    , sh_trans("sh_trans", "sh_trans", this, _sh_trans) {}

RooExpAndGauss::RooExpAndGauss(const RooExpAndGauss & other, const char * name)
    : RooAbsPdf(other, name)
    , x("x", this, other.x)
    , sh_mean("sh_mean", this, other.sh_mean)
    , sh_sigma("sh_sigma", this, other.sh_sigma)
    , sh_trans("sh_trans", this, other.sh_trans) {}

Double_t RooExpAndGauss::evaluate() const {
    double beta = (sh_mean - sh_trans) / (sh_sigma * sh_sigma);
    double c    = exp(-0.5 * pow((sh_trans - sh_mean) / sh_sigma, 2)) * exp(-beta * sh_trans);
    if (x <= sh_trans) { return c * exp(beta * x); };
    return exp(-0.5 * pow((x - sh_mean) / sh_sigma, 2));   // think a 1/2 was missed before BsMuMuPy v4.4
}
