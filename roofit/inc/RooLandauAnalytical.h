/*****************************************************************************
 * Project: RooFit                                                           *
 * Package: RooFitModels                                                     *
 *    File: $Id: RooLandauAnalytical.h,v 1.5 2007/07/12 20:30:49 wouter Exp $
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
#ifndef ROO_LANDAU_ANALYTICAL
#define ROO_LANDAU_ANALYTICAL

#include "RooAbsPdf.h"
#include "RooRealProxy.h"

class RooRealVar;

class RooLandauAnalytical : public RooAbsPdf {
public:
  RooLandauAnalytical() {} ;
  RooLandauAnalytical(const char *name, const char *title, RooAbsReal& _x, RooAbsReal& _mean, RooAbsReal& _sigma);
  RooLandauAnalytical(const RooLandauAnalytical& other, const char* name=0);
  virtual TObject* clone(const char* newname) const { return new RooLandauAnalytical(*this,newname); }
  inline virtual ~RooLandauAnalytical() { }

  Int_t getGenerator(const RooArgSet& directVars, RooArgSet &generateVars, Bool_t staticInitOK=kTRUE) const;
  void generateEvent(Int_t code);

  Int_t getAnalyticalIntegral(RooArgSet& allVars, RooArgSet& analVars, const char* /*rangeName*/) const;
  Double_t analyticalIntegral(Int_t code, const char* rangeName) const;
  
protected:
  
  RooRealProxy x ;
  RooRealProxy mean ;
  RooRealProxy sigma ;
  
  Double_t evaluate() const ;

private:
  
  ClassDef(RooLandauAnalytical,1) // Landau Distribution PDF
};

#endif
