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
 
/** \class RooExpTurnOnNumerical
    \ingroup Renato Quagliani
**/
 
#include "RooFit.h"
#include "Riostream.h"
#include "Riostream.h"
#include <math.h>
#include "RooExpTurnOnNumerical.h"
#include "RooAbsReal.h"
#include "RooRealVar.h"
#include "RooRandom.h"
#include "RooMath.h"
#include "TMath.h"
#include "Math/SpecFunc.h"
#include "gsl/gsl_sf_bessel.h"
using namespace std;
 
ClassImp(RooExpTurnOnNumerical);
 
////////////////////////////////////////////////////////////////////////////////
 
RooExpTurnOnNumerical::RooExpTurnOnNumerical(const char *name, const char *title,
          RooAbsReal& _x, RooAbsReal& _mO,
          RooAbsReal& _sE, RooAbsReal & _bComb) :
  RooAbsPdf(name,title),
  x("x","Observable",this,_x),
  mO("mO","mO",this,_mO),
  sE("sE","sE",this,_sE),
  bComb("b","b",this,_bComb){

}
 
 
RooExpTurnOnNumerical::RooExpTurnOnNumerical(const RooExpTurnOnNumerical& other, const char* name) :
  RooAbsPdf(other,name), x("x",this,other.x), 
                         mO("mO",this,other.mO),
                         sE("sE",this,other.sE),
                         bComb("b",this,other.bComb){
}

Double_t RooExpTurnOnNumerical::evaluate() const{
  const double  num =     TMath::Exp( bComb * x);
  const double  den = 1.+ TMath::Exp( sE * (mO - x));
  return num/den;
}
