// @(#)root/roostats:$Id$
// Author: Kyle Cranmer   21/07/2008

/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/
/*
Code stoled from Louis and Eli paper :
https://cds.cern.ch/record/2139626?ln=fr
*/
#ifndef RooStats_SPlot2
#define RooStats_SPlot2

class RooAbsReal;
class RooAbsPdf;
class RooFitResult;
class RooRealVar;
class RooSimultaneous;

#include "RooMsgService.h"

#include "RooDataSet.h"
#include "RooFitResult.h"
#include "RooHist.h"
#include "RooPlot.h"
#include "RooRealVar.h"

class SPlot2 : public TNamed {

  public:
    ~SPlot2();
    SPlot2();
    SPlot2(const SPlot2 & other);
    SPlot2(const char * name, const char * title);
    SPlot2(const char * name, const char * title, const RooDataSet & data);
    SPlot2(const char * name, const char * title, RooDataSet & data, RooAbsPdf * pdf, const RooArgList & yieldsList, const RooArgSet & projDeps = RooArgSet(), bool includeWeights = kTRUE, bool copyDataSet = kFALSE, const char * newName = "");

    SPlot2(const char * name, const char * title, RooDataSet & data, RooAbsPdf * pdf, const RooArgList & allyieldsList, const RooArgList & fixedYields, const RooArgSet & projDeps = RooArgSet(), bool includeWeights = kTRUE, bool copyDataSet = kFALSE, const char * newName = "");

    RooDataSet * SetSData(RooDataSet * data);

    RooDataSet * GetSDataSet() const;

    RooArgList GetSWeightVars() const;

    Double_t GetSWeightCoef(const char * sVariable) const;

    Int_t GetNumSWeightVars() const;

    void AddSWeight(RooAbsPdf * pdf, const RooArgList & yieldsTmp, const RooArgSet & projDeps = RooArgSet(), bool includeWeights = kTRUE);

    void     AddSWeight(RooAbsPdf * pdf, const RooArgList & yieldsTmp, const RooArgList & fixedYield, const RooArgSet & projDeps = RooArgSet(), bool includeWeights = kTRUE);
    Double_t GetSumOfEventSWeight(Int_t numEvent) const;

    Double_t GetYieldFromSWeight(const char * sVariable) const;

    Double_t GetSWeight(Int_t numEvent, const char * sVariable) const;

  protected:
    enum { kOwnData = BIT(20) };

    RooArgList fSWeightVars;

    RooArgList fSWeightCoefs;   // added
    //  RooListProxy fSWeightVars;

    RooDataSet * fSData;

    ClassDef(SPlot2, 1)   // Class used for making sPlots
};

#endif
