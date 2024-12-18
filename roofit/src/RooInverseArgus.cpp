#include "RooInverseArgus.h"

#include "RooFit.h"

#include "Riostream.h"
#include <math.h>

#include "RooMath.h"
#include "RooRealConstant.h"
#include "RooRealVar.h"
#include "TMath.h"

#include "TError.h"

using namespace std;

ClassImp(RooInverseArgus)

    ////////////////////////////////////////////////////////////////////////////////
    /// Constructor.

    RooInverseArgus::RooInverseArgus(const char * name, const char * title, RooAbsReal & _m, RooAbsReal & _m0, RooAbsReal & _c, RooAbsReal & _p)
    : RooAbsPdf(name, title)
    , m("m", "Mass", this, _m)
    , m0("m0", "Resonance mass", this, _m0)
    , c("c", "Slope parameter", this, _c)
    , p("p", "Power", this, _p) {}

////////////////////////////////////////////////////////////////////////////////
/// Constructor.

RooInverseArgus::RooInverseArgus(const RooInverseArgus & other, const char * name)
    : RooAbsPdf(other, name)
    , m("m", this, other.m)
    , m0("m0", this, other.m0)
    , c("c", this, other.c)
    , p("p", this, other.p) {}

////////////////////////////////////////////////////////////////////////////////

Double_t RooInverseArgus::evaluate() const {
    Double_t mp = m0 - m + m0;

    Double_t t = mp / m0;
    if (t >= 1) return 0;

    Double_t u = 1 - t * t;
    // cout << "c = " << c << " result = " << m*TMath::Power(u,p)*exp(c*u) << endl ;
    return mp * TMath::Power(u, p) * exp(c * u);
}

//////////////////////////////////////////////////////////////////////////////////
//
// Int_t RooInverseArgus::getAnalyticalIntegral(RooArgSet & allVars, RooArgSet & analVars, const char *rangeName) const {
//    if (p.arg().isConstant()) {
//        // We can integrate over m if power = 0.5
//        if (matchArgs(allVars, analVars, m) && p == 0.5) return 1;
//    }
//    return 0;
//}
//
//////////////////////////////////////////////////////////////////////////////////
//
// Double_t RooInverseArgus::analyticalIntegral(Int_t code, const char * rangeName) const {
//    R__ASSERT(code == 1);
//    // Formula for integration over m when p=0.5
//    static const Double_t pi  = atan2(0.0, -1.0);
//    Double_t              min = (m.min(rangeName) < m0) ? m.min(rangeName) : m0;
//    Double_t              max = (m.max(rangeName) < m0) ? m.max(rangeName) : m0;
//    Double_t              f1  = (1. - TMath::Power(min / m0, 2));
//    Double_t              f2  = (1. - TMath::Power(max / m0, 2));
//    Double_t              aLow, aHigh;
//    if (c < 0.) {
//        aLow  = -0.5 * m0 * m0 * (exp(c * f1) * sqrt(f1) / c + 0.5 / TMath::Power(-c, 1.5) * sqrt(pi) * RooMath::erf(sqrt(-c * f1)));
//        aHigh = -0.5 * m0 * m0 * (exp(c * f2) * sqrt(f2) / c + 0.5 / TMath::Power(-c, 1.5) * sqrt(pi) * RooMath::erf(sqrt(-c * f2)));
//    } else if (c == 0.) {
//        aLow  = -m0 * m0 / 3. * f1 * sqrt(f1);
//        aHigh = -m0 * m0 / 3. * f1 * sqrt(f2);
//    } else {
//        aLow  = 0.5 * m0 * m0 * exp(c * f1) / (c * sqrt(c)) * (0.5 * sqrt(pi) * (RooMath::faddeeva(sqrt(c * f1))).imag() - sqrt(c * f1));
//        aHigh = 0.5 * m0 * m0 * exp(c * f2) / (c * sqrt(c)) * (0.5 * sqrt(pi) * (RooMath::faddeeva(sqrt(c * f2))).imag() - sqrt(c * f2));
//    }
//    Double_t area = aHigh - aLow;
//    // cout << "c = " << c << "aHigh = " << aHigh << " aLow = " << aLow << " area = " << area << endl ;
//    return area;
//}