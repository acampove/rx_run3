/********************************************************************************************
 * Project: RooFit                                                                          *
 * Package: RooFitModels                                                                    *
 * @(#)root/roofit:$Id$
 * Authors:                                                                                 *
 *    T. Skwarnicki modify RooCBShape to Asymmetrical Double-Sided CB                       *
 *    Michael Wilkinson add to RooFit source                                                *
 *    Renato Quagliani for the RX analysis edit code from Root source code for R/L smearing *
*********************************************************************************************/

/** \class RooBifurDSCBShape
    \ingroup Roofit
PDF implementing the Asymmetrical Double-Sided Crystall Ball line shape with double width (left/right)
Inspiration from root source code ( a plain copy paste )
**/

#include "RooBifurDSCBShape.h"
#include "RooFit.h"

#include "RooAbsReal.h"
#include "RooRealVar.h"
#include "RooMath.h"

#include "TMath.h"

#include <exception>
#include <math.h>

using namespace std;

ClassImp(RooBifurDSCBShape);


namespace {
  
  inline double evaluateCrystalBallTail(double t, double alpha, double n){
    double a = std::pow(n / alpha, n) * std::exp(-0.5 * alpha * alpha);
    double b = n / alpha - alpha;

    return a / std::pow(b - t, n);
  }

  inline double integrateGaussian(double sigmaL, double sigmaR, double tmin, double tmax){
    constexpr double sqrtPiOver2 = 1.2533141373;
    constexpr double sqrt2 = 1.4142135624;

    const double sigmaMin = tmin < 0 ? sigmaL : sigmaR;
    const double sigmaMax = tmax < 0 ? sigmaL : sigmaR;

    return sqrtPiOver2 * (sigmaMax * std::erf(tmax / sqrt2) - sigmaMin * std::erf(tmin / sqrt2));
  }

  inline double integrateTailLogVersion(double sigma, double alpha, double n, double tmin, double tmax){
    double a = std::pow(n / alpha, n) * exp(-0.5 * alpha * alpha);
    double b = n / alpha - alpha;

    return a * sigma * (log(b - tmin) - log(b - tmax));
  }

  inline double integrateTailRegular(double sigma, double alpha, double n, double tmin, double tmax){
    double a = std::pow(n / alpha, n) * exp(-0.5 * alpha * alpha);
    double b = n / alpha - alpha;

    return a * sigma / (1.0 - n) * (1.0 / (std::pow(b - tmin, n - 1.0)) - 1.0 / (std::pow(b - tmax, n - 1.0)));
  }
  
 } // namespace

////////////////////////////////////////////////////////////////////////////////

RooBifurDSCBShape::RooBifurDSCBShape(const char *name, const char *title,
		       RooAbsReal& _m, 
		       RooAbsReal& _m0, 
		       RooAbsReal& _sigmaL, 
		       RooAbsReal& _sigmaR, 
		       RooAbsReal& _alphaL, 
		       RooAbsReal& _nL,
		       RooAbsReal& _alphaR, 
		       RooAbsReal& _nR) :
  RooAbsPdf(name, title),
  x_("m", "Dependent", this, _m),
  x0_("m0", "M0", this, _m0),
  sigmaL_("sigmaL", "Sigma Left", this, _sigmaL),
  sigmaR_("sigmaR", "Sigma Right", this, _sigmaR),
  alphaL_("alphaL", "Left Alpha", this, _alphaL),
  nL_("nL", "Left Order", this, _nL),
  alphaR_("alphaR", "Right Alpha", this, _alphaR),
  nR_("nR", "Right Order", this, _nR)
{
}


RooBifurDSCBShape::RooBifurDSCBShape(const char *name, const char *title,
				     RooAbsReal& _m,
				     RooAbsReal& _m0,
				     RooAbsReal& _sigma,				  
				     RooAbsReal& _alphaL,
				     RooAbsReal& _nL,
				     RooAbsReal& _alphaR,
				     RooAbsReal& _nR) :
  RooAbsPdf(name, title),
  x_("m", "Dependent", this, _m),
  x0_("m0", "M0", this, _m0),
  sigmaL_("sigmaL", "Sigma Left", this, _sigma),
  sigmaR_("sigmaR", "Sigma Right", this, _sigma),
  alphaL_("alphaL", "Left Alpha", this, _alphaL),
  nL_("nL", "Left Order", this, _nL),
  alphaR_("alphaR", "Right Alpha", this, _alphaR),
  nR_("nR", "Right Order", this, _nR){
}


////////////////////////////////////////////////////////////////////////////////

RooBifurDSCBShape::RooBifurDSCBShape(const RooBifurDSCBShape& other, const char* name) :
  RooAbsPdf(other, name), 
  x_("m", this, other.x_), 
  x0_("m0", this, other.x0_),
  sigmaL_("sigmaL", this, other.sigmaL_), 
  sigmaR_("sigmaR", this, other.sigmaR_), 
  alphaL_("alphaL", this, other.alphaL_),
  nL_("nL", this, other.nL_), 
  alphaR_("alphaR", this, other.alphaR_),
  nR_("nR", this, other.nR_)
{
}

////////////////////////////////////////////////////////////////////////////////
 Double_t RooBifurDSCBShape::evaluate() const
 {
    const double x = x_;
    const double x0 = x0_;
    const double sigmaL = std::abs(sigmaL_);
    const double sigmaR = std::abs(sigmaR_);
    double alphaL = std::abs(alphaL_);
    double nL = nL_;
    double alphaR = std::abs(alphaR_);
    double nR = nR_ ;
  
    // If alphaL is negative, then the tail will be on the right side.
    // Like this, we follow the convention established by RooCBShape.
    if(alphaL_ < 0.0) {
       std::swap(alphaL, alphaR);
       std::swap(nL, nR);
    }

    const double t = (x - x0) / (x < x0 ? sigmaL : sigmaR);
    if (t < -alphaL) {
       return evaluateCrystalBallTail(t, alphaL, nL);
    } else if (t <= alphaR) {
       return std::exp(-0.5 * t * t);
    } else {
       return evaluateCrystalBallTail(-t, alphaR, nR);
    }
 }

////////////////////////////////////////////////////////////////////////////////

Int_t RooBifurDSCBShape::getAnalyticalIntegral(RooArgSet& allVars, RooArgSet& analVars, const char* /*rangeName*/) const
{
  return matchArgs(allVars, analVars, x_) ? 1 : 0;
}

////////////////////////////////////////////////////////////////////////////////

Double_t RooBifurDSCBShape::analyticalIntegral(Int_t code, const char* rangeName) const{
    R__ASSERT(code == 1);
  
    const double x0 = x0_;
    const double sigmaL = std::abs(sigmaL_);
    const double sigmaR = std::abs(sigmaR_);
    double alphaL = std::abs(alphaL_);
    double nL = nL_;
    double alphaR = alphaR_ ;
    double nR = nR_ ;
  
    // If alphaL is negative, then the tail will be on the right side.
    // Like this, we follow the convention established by RooCBShape.
    if(alphaL_ < 0.0) {
       std::swap(alphaL, alphaR);
       std::swap(nL, nR);
    }
  
    constexpr double switchToLogThreshold = 1.0e-05;
  
    const double xmin = x_.min(rangeName);
    const double xmax = x_.max(rangeName);
    const double tmin = (xmin - x0) / (xmin < x0 ? sigmaL : sigmaR);
    const double tmax = (xmax - x0) / (xmax < x0 ? sigmaL : sigmaR);
  
    double result = 0.0;
  
  
    if (tmin < -alphaL) {
       auto integrateTailL = std::abs(nL - 1.0) < switchToLogThreshold ? integrateTailLogVersion : integrateTailRegular;
       result += integrateTailL(sigmaL, alphaL, nL, tmin, std::min(tmax, -alphaL));
    }
    if (tmax > alphaR) {
       auto integrateTailR = std::abs(nR - 1.0) < switchToLogThreshold ? integrateTailLogVersion : integrateTailRegular;
       result += integrateTailR(sigmaR, alphaR, nR, -tmax, std::min(-tmin, -alphaR));
    }
    if (tmin < alphaR && tmax > -alphaL) {
       result += integrateGaussian(sigmaL, sigmaR, std::max(tmin, -alphaL), std::min(tmax, alphaR));
    }
  
    return result;
}

////////////////////////////////////////////////////////////////////////////////
/// Advertise that we know the maximum of self for given (m0,alpha,n,sigma)

Int_t RooBifurDSCBShape::getMaxVal(const RooArgSet& vars) const
{
  RooArgSet dummy ;

  return matchArgs(vars, dummy, x_) ? 1 : 0;
}

////////////////////////////////////////////////////////////////////////////////

Double_t RooBifurDSCBShape::maxVal(Int_t code) const{
  R__ASSERT(code == 1);

  // The maximum value for given (m0,alpha,n,sigma) is 1./ Integral in the variable range
  // For the crystal ball, the maximum is 1.0 in the current implementation,
  // but it's maybe better to keep this general in case the implementation changes.    
  return 1.0 / analyticalIntegral(code);
}
