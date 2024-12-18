#include "RooFit.h"

#include "Riostream.h"
#include <math.h>

#include "RooAbsReal.h"
#include "RooDoubleSidedCBShape.h"
#include "RooMath.h"
#include "RooRealVar.h"
#include "TMath.h"

#include "TError.h"

using namespace std;

ClassImp(RooDoubleSidedCBShape);

RooDoubleSidedCBShape::RooDoubleSidedCBShape(const char * name, const char * title, RooAbsReal & _x, RooAbsReal & _mu, RooAbsReal & _sigma, RooAbsReal & _alphaLow, RooAbsReal & _nLow, RooAbsReal & _alphaHigh, RooAbsReal & _nHigh)
    : RooAbsPdf(name, title)
    , x("x", "Dependent", this, _x)
    , mu("mu", "Mu", this, _mu)
    , sigma("sigma", "Sigma", this, _sigma)
    , alphaLow("alphaLow", "AlphaLow", this, _alphaLow)
    , nLow("nLow", "Low Tail Order", this, _nLow)
    , alphaHigh("alphaHigh", "AlphaLow", this, _alphaHigh)
    , nHigh("nHigh", "High Tail Order", this, _nHigh) {}

RooDoubleSidedCBShape::RooDoubleSidedCBShape(const RooDoubleSidedCBShape & other, const char * name)
    : RooAbsPdf(other, name)
    , x("x", this, other.x)
    , mu("mu", this, other.mu)
    , sigma("sigma", this, other.sigma)
    , alphaLow("alphaLow", this, other.alphaLow)
    , nLow("nLow", this, other.nLow)
    , alphaHigh("alphaHigh", this, other.alphaHigh)
    , nHigh("nHigh", this, other.nHigh) {}

Double_t RooDoubleSidedCBShape::evaluate() const {
    double t      = (x - mu) / sigma;
    double result = 0;
    if (t < -alphaLow) {
        double a = exp(-0.5 * alphaLow * alphaLow);
        double b = alphaLow/nLow;
        result   = a / pow(1 - alphaLow*b - b*t, nLow);
    } else if (t >= -alphaLow && t < alphaHigh) {
        result = exp(-0.5 * t * t);
    } else if (t >= alphaHigh) {
        double a = exp(-0.5 * alphaHigh * alphaHigh);
        double b = alphaHigh / nHigh;
        result   = a / pow(1 - b*alphaHigh + b*t, nHigh);
    }
    return result;
}

Int_t RooDoubleSidedCBShape::getAnalyticalIntegral(RooArgSet & allVars, RooArgSet & analVars, const char * rangeName) const {
    if (matchArgs(allVars, analVars, x)) return 1;

    return 0;
}

Double_t RooDoubleSidedCBShape::analyticalIntegral(Int_t code, const char * rangeName) const {
    double tMin = (x.min(rangeName) - mu) / sigma;
    double tMax = (x.max(rangeName) - mu) / sigma;
    double result(0);
    if (tMin <= -alphaLow && tMax <= -alphaLow){
        double a = exp(-0.5 * alphaLow * alphaLow);
        double b = alphaLow/nLow;
        bool useLog = (abs(nLow - 1.0) < 1.0e-5);
        if (useLog){
            double factor = -1. * sigma * a / b;
            double difference = log(1 - alphaLow * b - b * tMax) - log(1 - alphaLow * b - b * tMin);
            result = factor * difference; 
        }
        else{
            double factor = sigma * a / b / (nLow - 1);
            double difference = pow(1 - b * alphaLow - b * tMax, 1 - nLow) - pow(1 - b * alphaLow - b * tMin, 1 - nLow);
            result = factor * difference;
        }
    }
    else if(tMin > -alphaLow && tMax <= alphaHigh) {
        result = sqrtPi * invSqrt2 * sigma * (std::erf(tMax * invSqrt2) - std::erf(tMin * invSqrt2));
    }
    else if(tMin > alphaHigh && tMax > alphaHigh) {
        double aHigh = exp(-0.5 * alphaHigh * alphaHigh);
        double bHigh = alphaHigh/nHigh;
        bool   highUseLog = (abs(nHigh - 1.0) < 1.0e-5);
        if (highUseLog){
            double factor = sigma * aHigh / bHigh;
            double difference = log(1 - alphaHigh * bHigh + bHigh * tMax) - log(1 - alphaHigh * bHigh + bHigh * tMin);
            result = factor * difference;
        }
        else{
            double factor = -1. * sigma * aHigh / bHigh / (nHigh - 1);
            double difference = pow(1 - bHigh * alphaHigh + bHigh * tMax, 1 - nHigh) - pow(1 - bHigh * alphaHigh + bHigh * tMin, 1 - nHigh);
            result = factor * difference;
        }
    }
    else if(tMin <= -alphaLow && tMax > -alphaLow && tMax <= alphaHigh) {
        double aLow = exp(-0.5 * alphaLow * alphaLow);
        double bLow = alphaLow/nLow;
        bool useLog = (abs(nLow - 1.0) < 1.0e-5);
        if (useLog){
            double factor = sigma * aLow / bLow;
            double integral = log(1 - alphaLow * bLow - bLow * tMin);
            result = factor * integral;
        }
        else{
            double factor = sigma * aLow / bLow / (nLow - 1);
            double integral = 1 - pow(1 - bLow * alphaLow - bLow * tMin, 1 - nLow);
            result = factor * integral;
        }
        result += sqrtPi * invSqrt2 * sigma * (std::erf(alphaLow * invSqrt2) + std::erf(tMax * invSqrt2));
    }
    else if(tMin <= -alphaLow && tMax > alphaHigh) {
        // Lower tail integral
        double aLow = exp(-0.5 * alphaLow * alphaLow);
        double bLow = alphaLow/nLow;
        bool   lowUseLog = (abs(nLow - 1.0) < 1.0e-5);
        if (lowUseLog){
            double factor = sigma * aLow / bLow;
            double integral = log(1 - alphaLow * bLow - bLow * tMin);
            result = factor * integral;
        }
        else{
            double factor = sigma * aLow / bLow / (nLow - 1);
            double integral = 1 - pow(1 - bLow * alphaLow - bLow * tMin, 1 - nLow);
            result = factor * integral;
        }
        // Gaussian core integral
        result += sqrtPi * invSqrt2 * sigma * (std::erf(alphaLow * invSqrt2) + std::erf(alphaHigh * invSqrt2));
        // High tail integral
        double aHigh = exp(-0.5 * alphaHigh * alphaHigh);
        double bHigh = alphaHigh/nHigh;
        bool   highUseLog = (abs(nHigh - 1.0) < 1.0e-5);
        if (highUseLog){
            double factor = sigma * aHigh / bHigh;
            double difference = log(1 - alphaHigh * bHigh + bHigh * tMax);
            result += factor * difference;
        }
        else{
            double factor = -1. * sigma * aHigh / bHigh / (nHigh - 1);
            double difference = pow(1 - bHigh * alphaHigh + bHigh * tMax, 1 - nHigh) - 1;
            result += factor * difference;
        }
    }
    else if(tMin > -alphaLow && tMin <= alphaHigh && tMax > alphaHigh) {
        result = sqrtPi * invSqrt2 * sigma * (std::erf(alphaHigh * invSqrt2) - std::erf(tMin * invSqrt2));
        // High tail integral
        double aHigh = exp(-0.5 * alphaHigh * alphaHigh);
        double bHigh = alphaHigh/nHigh;
        bool   highUseLog = (abs(nHigh - 1.0) < 1.0e-5);
        if (highUseLog){
            double factor = sigma * aHigh / bHigh;
            double difference = log(1 - alphaHigh * bHigh + bHigh * tMax);
            result += factor * difference;
        }
        else{
            double factor = -1. * sigma * aHigh / bHigh / (nHigh - 1);
            double difference = pow(1 - bHigh * alphaHigh + bHigh * tMax, 1 - nHigh) - 1;
            result += factor * difference;
        }
    }
    else {
      std::cout<< "PDF::analyticalIntegral() "<< this->GetName() << std::endl;
      std::cout<< "Entered into weird branch, returning 0" << std::endl;	
      std::cout<< " tMin     : "<< tMin << std::endl;
      std::cout<< " tMax     : "<< tMax << std::endl;
      std::cout<< "-alphaLow : "<<-alphaLow << std::endl;
      std::cout<< " alphaHigh: "<< alphaHigh<< std::endl;
      bool _cond1 = tMin <= -alphaLow && tMax <= -alphaLow;
      bool _cond2 = tMin >  -alphaLow && tMax <= alphaHigh;
      bool _cond3 = tMin <= -alphaLow && tMax > -alphaLow && tMax <= alphaHigh;
      bool _cond4 = tMin <= -alphaLow && tMax >  alphaHigh;
      bool _cond5 = tMin >  -alphaLow && tMin <= alphaHigh && tMax > alphaHigh;      
    }
    return result;
}

Int_t RooDoubleSidedCBShape::getMaxVal(const RooArgSet & vars) const {
    RooArgSet dummy;

    if (matchArgs(vars, dummy, x)) { return 1; }
    return 0;
}

Double_t RooDoubleSidedCBShape::maxVal(Int_t code) const {
    R__ASSERT(code == 1);

    // The maximum value for given (m0,alpha,n,sigma)
    return 1.0;
}
