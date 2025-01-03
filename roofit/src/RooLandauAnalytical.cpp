/*****************************************************************************
 * Project: RooFit                                                           *
 * Package: RooFitModels                                                     *
 * @(#)root/roofit:$Id$
 * Authors:                                                                  *
 *   WV, Wouter Verkerke, UC Santa Barbara, verkerke@slac.stanford.edu       *
 *   DK, David Kirkby,    UC Irvine,         dkirkby@uci.edu                 *
 *                                                                           *
 * Copyright (c) 2000-2005, Regents of the University of California          *
 *                          and Stanford University. All rights reserved.    *
 *                                                                           *
 * Redistribution and use in source and binary forms,                        *
 * with or without modification, are permitted according to the terms        *
 * listed in LICENSE (http://roofit.sourceforge.net/license.txt)             *
 *****************************************************************************/

/** \class RooLandauAnalytical
    \ingroup Roofit

Landau distribution p.d.f
\image html RF_Landau.png "PDF of the Landau distribution."
**/

#include "RooLandauAnalytical.h"
#include "RooFit.h"
#include "RooRandom.h"

#include "Math/ProbFunc.h"
#include "TMath.h"

ClassImp(RooLandauAnalytical);

////////////////////////////////////////////////////////////////////////////////

RooLandauAnalytical::RooLandauAnalytical(const char *name, const char *title, RooAbsReal& _x, RooAbsReal& _mean, RooAbsReal& _sigma) :
  RooAbsPdf(name,title),
  x("x","Dependent",this,_x),
  mean("mean","Mean",this,_mean),
  sigma("sigma","Width",this,_sigma)
{
}

////////////////////////////////////////////////////////////////////////////////

RooLandauAnalytical::RooLandauAnalytical(const RooLandauAnalytical& other, const char* name) :
  RooAbsPdf(other,name),
  x("x",this,other.x),
  mean("mean",this,other.mean),
  sigma("sigma",this,other.sigma)
{
}

////////////////////////////////////////////////////////////////////////////////

Double_t RooLandauAnalytical::evaluate() const
{
  return TMath::Landau(x, mean, sigma, kTRUE);
}

////////////////////////////////////////////////////////////////////////////////

Int_t RooLandauAnalytical::getGenerator(const RooArgSet& directVars, RooArgSet &generateVars, Bool_t /*staticInitOK*/) const
{
  if (matchArgs(directVars,generateVars,x)) return 1 ;
  return 0 ;
}

////////////////////////////////////////////////////////////////////////////////

void RooLandauAnalytical::generateEvent(Int_t code)
{
  assert(1 == code); (void)code;
  Double_t xgen ;
  while(1) {
    xgen = RooRandom::randomGenerator()->Landau(mean,sigma);
    if (xgen<x.max() && xgen>x.min()) {
      x = xgen ;
      break;
    }
  }
  return;
}


Int_t RooLandauAnalytical::getAnalyticalIntegral(RooArgSet& allVars, RooArgSet& analVars, const char* /*rangeName*/) const
{
  if (matchArgs(allVars,analVars,x)) return 1 ;
  return 0 ;
}

////////////////////////////////////////////////////////////////////////////////

Double_t RooLandauAnalytical::analyticalIntegral(Int_t code, const char* rangeName) const
{
  assert(code==1 || code==2);

  double max = x.max(rangeName);
  double min = x.min(rangeName);

  return ROOT::Math::landau_cdf(max, sigma, mean) - ROOT::Math::landau_cdf(min, sigma, mean);
}
