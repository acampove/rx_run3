// @(#)root/roostats:$Id$
// Author: Kyle Cranmer   28/07/2008

/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

/*****************************************************************************
 * Project: RooStats
 * Package: RooFit/RooStats
 *
 * Authors:
 *   Original code from M. Pivk as part of MLFit package from BaBar.
 *   Modifications:
 *     Giacinto Piacquadio, Maurizio Pierini: modifications for new RooFit version
 *     George H. Lewis, Kyle Cranmer: generalized for weighted events
 *
 * Porting to RooStats (with permission) by Kyle Cranmer, July 2008
 *
 *****************************************************************************/

/** \class RooStats::SPlot2
    \ingroup Roostats

   This class calculates sWeights used to create an sPlot.
   The code is based on
   ``SPlot2: A statistical tool to unfold data distributions,''
   Nucl. Instrum. Meth. A 555, 356 (2005) [arXiv:physics/0402083].

   An SPlot2 gives us  the distribution of some variable, x in our
   data sample for a given species (eg. signal or background).
   The result is similar to a likelihood projection plot, but no cuts are made,
   so every event contributes to the distribution.

   To use this class, you first must have a pdf that includes
   yields for (possibly several) different species.
   Create an instance of the class by supplying a data set,
   the pdf, and a list of the yield variables.  The SPlot2 Class
   will calculate SWeights and include these as columns in the RooDataSet.

*/

#include <map>
#include <vector>

#include "RooAbsPdf.h"
#include "RooDataSet.h"
#include "RooGlobalFunc.h"
#include "RooRealVar.h"
#include "RooStats/RooStatsUtils.h"
#include "SPlot2.hpp"
#include "TTree.h"

#include "TMatrixD.h"

ClassImp(SPlot2);

using namespace RooStats;
using namespace std;

////////////////////////////////////////////////////////////////////////////////

SPlot2::~SPlot2() {
    if (TestBit(kOwnData) && fSData) delete fSData;
}

////////////////////////////////////////////////////////////////////////////////
/// Default constructor

SPlot2::SPlot2()
    : TNamed() {
    RooArgList Args;

    fSWeightVars = Args;

    fSData = NULL;
}

////////////////////////////////////////////////////////////////////////////////

SPlot2::SPlot2(const char * name, const char * title)
    : TNamed(name, title) {
    RooArgList Args;

    fSWeightVars = Args;

    fSData = NULL;
}

////////////////////////////////////////////////////////////////////////////////
/// Constructor from a RooDataSet
/// No sWeighted variables are present

SPlot2::SPlot2(const char * name, const char * title, const RooDataSet & data)
    : TNamed(name, title) {
    RooArgList Args;

    fSWeightVars = Args;

    fSData = (RooDataSet *) &data;
}

////////////////////////////////////////////////////////////////////////////////
/// Copy Constructor from another SPlot2

SPlot2::SPlot2(const SPlot2 & other)
    : TNamed(other) {
    RooArgList Args = (RooArgList) other.GetSWeightVars();

    fSWeightVars.addClone(Args);

    fSData = (RooDataSet *) other.GetSDataSet();
}

////////////////////////////////////////////////////////////////////////////////

SPlot2::SPlot2(const char * name, const char * title, RooDataSet & data, RooAbsPdf * pdf, const RooArgList & yieldsList, const RooArgSet & projDeps, bool includeWeights, bool cloneData, const char * newName)
    : TNamed(name, title) {
    if (cloneData == 1) {
        fSData = (RooDataSet *) data.Clone(newName);
        SetBit(kOwnData);
    } else
        fSData = (RooDataSet *) &data;

    // Add check that yieldsList contains all RooRealVars
    TIterator * iter = yieldsList.createIterator();
    RooAbsArg * arg;
    while ((arg = (RooAbsArg *) iter->Next())) {
        if (!dynamic_cast< RooRealVar * >(arg)) {
            coutE(InputArguments) << "SPlot2::SPlot2(" << GetName() << ") input argument " << arg->GetName() << " is not of type RooRealVar " << endl;
            throw string(Form("SPlot2::SPlot2(%s) input argument %s is not of type RooRealVar", GetName(), arg->GetName()));
        }
    }
    delete iter;

    // Construct a new SPlot2 class,
    // calculate sWeights, and include them
    // in the RooDataSet of this class.

    this->AddSWeight(pdf, yieldsList, projDeps, includeWeights);
}

// OK DONE
SPlot2::SPlot2(const char * name, const char * title, RooDataSet & data, RooAbsPdf * pdf, const RooArgList & allYieldList, const RooArgList & fixedYields, const RooArgSet & projDeps, bool includeWeights, bool cloneData, const char * newName) {
    if (cloneData == 1) {
        fSData = (RooDataSet *) data.Clone(newName);
        SetBit(kOwnData);
    } else {
        fSData = (RooDataSet *) &data;
    }

    TIterator * iter = allYieldList.createIterator();
    RooAbsArg * arg;
    while ((arg = (RooAbsArg *) iter->Next())) {
        if (!dynamic_cast< RooRealVar * >(arg)) {
            coutE(InputArguments) << "SPlot2::SPlot2(" << GetName() << ") input argument " << arg->GetName() << " is not of type RooRealVar " << endl;
            throw string(Form("SPlot2::SPlot2(%s) input argument %s is not of type RooRealVar", GetName(), arg->GetName()));
        }
    }
    delete iter;
    // Check that allYieldList contains only varying yields
    TIterator * iter_fixedYields = fixedYields.createIterator();
    RooAbsArg * arg2;
    while ((arg2 = (RooAbsArg *) iter->Next())) {
        if (!dynamic_cast< RooRealVar * >(arg2)) {
            coutE(InputArguments) << "SPlot2::SPlot2(" << GetName() << ") input argument fixed Yield" << arg2->GetName() << " is not of type RooRealVar " << endl;
            throw string(Form("SPlot2::SPlot2(%s) input argument %s is not of type RooRealVar", GetName(), arg2->GetName()));
        }
        if (!(allYieldList.contains(*arg2))) {
            coutE(InputArguments) << "SPlot2::SPlot2(" << GetName() << ") input argument fixed Yield" << arg2->GetName() << " is not in list of AllYields " << endl;
            throw string(Form("SPlot2::SPlot2(%s) input argument fixedYield %s is not present in allYieldList", GetName(), arg2->GetName()));
        }
    }
    delete iter_fixedYields;
    this->AddSWeight(pdf, allYieldList, fixedYields, projDeps, includeWeights);
}
////////////////////////////////////////////////////////////////////////////////

RooDataSet * SPlot2::SetSData(RooDataSet * data) {
    if (data) {
        fSData = (RooDataSet *) data;
        return fSData;
    } else
        return NULL;
}

////////////////////////////////////////////////////////////////////////////////

RooDataSet * SPlot2::GetSDataSet() const { return fSData; }

////////////////////////////////////////////////////////////////////////////////

Double_t SPlot2::GetSWeight(Int_t numEvent, const char * sVariable) const {
    if (numEvent > fSData->numEntries()) {
        coutE(InputArguments) << "Invalid Entry Number" << endl;
        return -1;
    }

    if (numEvent < 0) {
        coutE(InputArguments) << "Invalid Entry Number" << endl;
        return -1;
    }

    Double_t totalYield = 0;

    std::string varname(sVariable);
    varname += "_sw";

    if (fSWeightVars.find(sVariable)) {
        RooArgSet Row(*fSData->get(numEvent));
        totalYield += Row.getRealValue(sVariable);

        return totalYield;
    }

    if (fSWeightVars.find(varname.c_str())) {

        RooArgSet Row(*fSData->get(numEvent));
        totalYield += Row.getRealValue(varname.c_str());

        return totalYield;
    }

    else
        coutE(InputArguments) << "InputVariable not in list of sWeighted variables" << endl;

    return -1;
}

////////////////////////////////////////////////////////////////////////////////
/// Sum the SWeights for a particular event.
/// This sum should equal the total weight of that event.
/// This method is intended to be used as a check.

Double_t SPlot2::GetSumOfEventSWeight(Int_t numEvent) const {
    if (numEvent > fSData->numEntries()) {
        coutE(InputArguments) << "Invalid Entry Number" << endl;
        return -1;
    }

    if (numEvent < 0) {
        coutE(InputArguments) << "Invalid Entry Number" << endl;
        return -1;
    }

    Int_t numSWeightVars = this->GetNumSWeightVars();

    Double_t eventSWeight = 0;

    RooArgSet Row(*fSData->get(numEvent));

    for (Int_t i = 0; i < numSWeightVars; i++) eventSWeight += Row.getRealValue(fSWeightVars.at(i)->GetName());

    return eventSWeight;
}

////////////////////////////////////////////////////////////////////////////////
/// Sum the SWeights for a particular specie over all events
/// This should equal the total (weighted) yield of that specie
/// This method is intended as a check.

Double_t SPlot2::GetYieldFromSWeight(const char * sVariable) const {

    Double_t totalYield = 0;

    std::string varname(sVariable);
    varname += "_sw";

    if (fSWeightVars.find(sVariable)) {
        for (Int_t i = 0; i < fSData->numEntries(); i++) {
            RooArgSet Row(*fSData->get(i));
            totalYield += Row.getRealValue(sVariable);
        }

        return totalYield;
    }

    if (fSWeightVars.find(varname.c_str())) {
        for (Int_t i = 0; i < fSData->numEntries(); i++) {
            RooArgSet Row(*fSData->get(i));
            totalYield += Row.getRealValue(varname.c_str());
        }

        return totalYield;
    }

    else
        coutE(InputArguments) << "InputVariable not in list of sWeighted variables" << endl;

    return -1;
}

////////////////////////////////////////////////////////////////////////////////
/// Return a RooArgList containing the SWeights

RooArgList SPlot2::GetSWeightVars() const {

    RooArgList Args = fSWeightVars;

    return Args;
}

Double_t SPlot2::GetSWeightCoef(const char * sVariable) const {
    // Gets the c(i) coefficient that is used for correcting fixed yields.
    std::string varname(sVariable);
    varname += "_c";
    UInt_t index = fSWeightCoefs.index(varname.c_str());
    if (index == -1) {
        coutE(InputArguments) << "InputVariable not in list of the calculated c coefficients." << endl;
        return 0;
    }
    RooRealVar * c = dynamic_cast< RooRealVar * >(&(fSWeightCoefs[index]));

    return c->getVal();
}

////////////////////////////////////////////////////////////////////////////////
/// Return the number of SWeights
/// In other words, return the number of
/// species that we are trying to extract.

Int_t SPlot2::GetNumSWeightVars() const {
    RooArgList Args = fSWeightVars;

    return Args.getSize();
}

////////////////////////////////////////////////////////////////////////////////
/// Method which adds the sWeights to the dataset.
/// Input is the PDF, a RooArgList of the yields (floating)
/// and a RooArgSet of the projDeps.
///
/// The projDeps will not be normalized over when calculating the SWeights
/// and will be considered parameters, not observables.
///
/// The SPlot2 will contain two new variables for each specie of name "varname":
///
/// L_varname is the value of the pdf for the variable "varname" at values of this event
/// varname_sw is the value of the sWeight for the variable "varname" for this event
///
/// Find Parameters in the PDF to be considered fixed when calculating the SWeights
/// and be sure to NOT include the yields in that list

void SPlot2::AddSWeight(RooAbsPdf * pdf, const RooArgList & yieldsTmp, const RooArgSet & projDeps, bool includeWeights) {

    RooFit::MsgLevel currentLevel = RooMsgService::instance().globalKillBelow();

    RooArgList * constParameters = (RooArgList *) pdf->getParameters(fSData);
    constParameters->remove(yieldsTmp, kTRUE, kTRUE);

    // Set these parameters constant and store them so they can later
    // be set to not constant
    std::vector< RooRealVar * > constVarHolder;

    for (Int_t i = 0; i < constParameters->getSize(); i++) {
        RooRealVar * varTemp = (dynamic_cast< RooRealVar * >(constParameters->at(i)));
        if (varTemp && varTemp->isConstant() == 0) {
            varTemp->setConstant();
            constVarHolder.push_back(varTemp);
        }
    }

    // Fit yields to the data with all other variables held constant
    // This is necessary because SPlot assumes the yields minimise -Log(likelihood)

    pdf->fitTo(*fSData, RooFit::Extended(kTRUE), RooFit::SumW2Error(kTRUE), RooFit::PrintLevel(-1), RooFit::PrintEvalErrors(-1));

    // Hold the value of the fitted yields
    std::vector< double > yieldsHolder;

    for (Int_t i = 0; i < yieldsTmp.getSize(); i++) yieldsHolder.push_back(((RooRealVar *) yieldsTmp.at(i))->getVal());

    Int_t      nspec  = yieldsTmp.getSize();
    RooArgList yields = *(RooArgList *) yieldsTmp.snapshot(kFALSE);

    if (currentLevel <= RooFit::DEBUG) {
        coutI(InputArguments) << "Printing Yields" << endl;
        yields.Print();
    }

    // The list of variables to normalize over when calculating PDF values.

    RooArgSet vars(*fSData->get());
    vars.remove(projDeps, kTRUE, kTRUE);

    // Attach data set

    // const_cast<RooAbsPdf*>(pdf)->attachDataSet(*fSData);

    pdf->attachDataSet(*fSData);

    // first calculate the pdf values for all species and all events
    std::vector< RooRealVar * > yieldvars;
    RooArgSet *                 parameters = pdf->getParameters(fSData);

    std::vector< Double_t > yieldvalues;
    for (Int_t k = 0; k < nspec; ++k) {
        RooRealVar * thisyield = dynamic_cast< RooRealVar * >(yields.at(k));
        if (thisyield) {
            RooRealVar * yieldinpdf = dynamic_cast< RooRealVar * >(parameters->find(thisyield->GetName()));

            if (yieldinpdf) {
                coutI(InputArguments) << "yield in pdf: " << yieldinpdf->GetName() << " " << thisyield->getVal() << endl;

                yieldvars.push_back(yieldinpdf);
                yieldvalues.push_back(thisyield->getVal());
            }
        }
    }

    Int_t numevents = fSData->numEntries();

    std::vector< std::vector< Double_t > > pdfvalues(numevents, std::vector< Double_t >(nspec, 0));

    // set all yield to zero
    for (Int_t m = 0; m < nspec; ++m) yieldvars[m]->setVal(0);

    // For every event and for every specie,
    // calculate the value of the component pdf for that specie
    // by setting the yield of that specie to 1
    // and all others to 0.  Evaluate the pdf for each event
    // and store the values.

    RooArgSet * pdfvars = pdf->getVariables();

    for (Int_t ievt = 0; ievt < numevents; ievt++) {
        //   if (ievt % 100 == 0)
        //  coutP(Eval)  << ".";

        // FIX THIS PART, EVALUATION PROGRESS!!

        RooStats::SetParameters(fSData->get(ievt), pdfvars);

        //   RooArgSet row(*fSData->get(ievt));

        for (Int_t k = 0; k < nspec; ++k) {
            // Check that range of yields is at least (0,1), and fix otherwise
            if (yieldvars[k]->getMin() > 0) {
                coutW(InputArguments) << "Minimum Range for " << yieldvars[k]->GetName() << " must be 0.  ";
                coutW(InputArguments) << "Setting min range to 0" << std::endl;
                yieldvars[k]->setMin(0);
            }

            if (yieldvars[k]->getMax() < 1) {
                coutW(InputArguments) << "Maximum Range for " << yieldvars[k]->GetName() << " must be 1.  ";
                coutW(InputArguments) << "Setting max range to 1" << std::endl;
                yieldvars[k]->setMax(1);
            }

            // set this yield to 1
            yieldvars[k]->setVal(1);
            // evaluate the pdf
            Double_t f_k       = pdf->getVal(&vars);
            pdfvalues[ievt][k] = f_k;
            if (!(f_k > 1 || f_k < 1)) coutW(InputArguments) << "Strange pdf value: " << ievt << " " << k << " " << f_k << std::endl;
            yieldvars[k]->setVal(0);
        }
    }
    delete pdfvars;

    // check that the likelihood normalization is fine
    std::vector< Double_t > norm(nspec, 0);
    for (Int_t ievt = 0; ievt < numevents; ievt++) {
        Double_t dnorm(0);
        for (Int_t k = 0; k < nspec; ++k) dnorm += yieldvalues[k] * pdfvalues[ievt][k];
        for (Int_t j = 0; j < nspec; ++j) norm[j] += pdfvalues[ievt][j] / dnorm;
    }

    coutI(Contents) << "likelihood norms: ";

    for (Int_t k = 0; k < nspec; ++k) coutI(Contents) << norm[k] << " ";
    coutI(Contents) << std::endl;

    // Make a TMatrixD to hold the covariance matrix.
    TMatrixD covInv(nspec, nspec);
    for (Int_t i = 0; i < nspec; i++)
        for (Int_t j = 0; j < nspec; j++) covInv(i, j) = 0;

    coutI(Contents) << "Calculating covariance matrix";

    // Calculate the inverse covariance matrix, using weights
    for (Int_t ievt = 0; ievt < numevents; ++ievt) {

        fSData->get(ievt);

        // Calculate contribution to the inverse of the covariance
        // matrix. See BAD 509 V2 eqn. 15

        // Sum for the denominator
        Double_t dsum(0);
        for (Int_t k = 0; k < nspec; ++k) dsum += pdfvalues[ievt][k] * yieldvalues[k];

        for (Int_t n = 0; n < nspec; ++n)
            for (Int_t j = 0; j < nspec; ++j) {
                if (includeWeights == kTRUE)
                    covInv(n, j) += fSData->weight() * pdfvalues[ievt][n] * pdfvalues[ievt][j] / (dsum * dsum);
                else
                    covInv(n, j) += pdfvalues[ievt][n] * pdfvalues[ievt][j] / (dsum * dsum);
            }

        // ADDED WEIGHT ABOVE
    }

    // Covariance inverse should now be computed!

    // Invert to get the covariance matrix
    if (covInv.Determinant() <= 0) {
        coutE(Eval) << "SPlot Error: covariance matrix is singular; I can't invert it!" << std::endl;
        covInv.Print();
        return;
    }

    TMatrixD covMatrix(TMatrixD::kInverted, covInv);

    // check cov normalization
    if (currentLevel <= RooFit::DEBUG) {
        coutI(Eval) << "Checking Likelihood normalization:  " << std::endl;
        coutI(Eval) << "Yield of specie  Sum of Row in Matrix   Norm" << std::endl;
        for (Int_t k = 0; k < nspec; ++k) {
            Double_t covnorm(0);
            for (Int_t m = 0; m < nspec; ++m) covnorm += covInv[k][m] * yieldvalues[m];
            Double_t sumrow(0);
            for (Int_t m = 0; m < nspec; ++m) sumrow += covMatrix[k][m];
            coutI(Eval) << yieldvalues[k] << " " << sumrow << " " << covnorm << endl;
        }
    }

    // calculate for each event the sWeight (BAD 509 V2 eq. 21)
    coutI(Eval) << "Calculating sWeight" << std::endl;
    std::vector< RooRealVar * > sweightvec;
    std::vector< RooRealVar * > pdfvec;
    RooArgSet                   sweightset;

    // Create and label the variables
    // used to store the SWeights

    fSWeightVars.Clear();

    for (Int_t k = 0; k < nspec; ++k) {
        std::string  wname = std::string(yieldvars[k]->GetName()) + "_sw";
        RooRealVar * var   = new RooRealVar(wname.c_str(), wname.c_str(), 0);
        sweightvec.push_back(var);
        sweightset.add(*var);
        fSWeightVars.add(*var);

        wname = "L_" + std::string(yieldvars[k]->GetName());
        var   = new RooRealVar(wname.c_str(), wname.c_str(), 0);
        pdfvec.push_back(var);
        sweightset.add(*var);
    }

    // Create and fill a RooDataSet
    // with the SWeights

    RooDataSet * sWeightData = new RooDataSet("dataset", "dataset with sWeights", sweightset);

    for (Int_t ievt = 0; ievt < numevents; ++ievt) {

        fSData->get(ievt);

        // sum for denominator
        Double_t dsum(0);
        for (Int_t k = 0; k < nspec; ++k) dsum += pdfvalues[ievt][k] * yieldvalues[k];
        // covariance weighted pdf for each specief
        for (Int_t n = 0; n < nspec; ++n) {
            Double_t nsum(0);
            for (Int_t j = 0; j < nspec; ++j) nsum += covMatrix(n, j) * pdfvalues[ievt][j];

            // Add the sWeights here!!
            // Include weights,
            // ie events weights are absorbed into sWeight

            if (includeWeights == kTRUE)
                sweightvec[n]->setVal(fSData->weight() * nsum / dsum);
            else
                sweightvec[n]->setVal(nsum / dsum);

            pdfvec[n]->setVal(pdfvalues[ievt][n]);

            if (!(fabs(nsum / dsum) >= 0)) {
                coutE(Contents) << "error: " << nsum / dsum << endl;
                return;
            }
        }

        sWeightData->add(sweightset);
    }

    // Add the SWeights to the original data set

    fSData->merge(sWeightData);

    delete sWeightData;

    // Restore yield values

    for (Int_t i = 0; i < yieldsTmp.getSize(); i++) ((RooRealVar *) yieldsTmp.at(i))->setVal(yieldsHolder.at(i));

    // Make any variables that were forced to constant no longer constant

    for (Int_t i = 0; i < (Int_t) constVarHolder.size(); i++) constVarHolder.at(i)->setConstant(kFALSE);

    return;
}

void SPlot2::AddSWeight(RooAbsPdf * pdf, const RooArgList & yieldsTmp, const RooArgList & fixedYield, const RooArgSet & projDeps, bool includeWeights) {

    //
    // Method which adds the sWeights to the dataset.
    // Input is the PDF, a RooArgList of the yields (floating)
    // and a RooArgSet of the projDeps.
    //
    // Fixed yields still have to be accounted for using the method described in Appendix B.1 of the
    // sWeights paper.
    //
    // The projDeps will not be normalized over when calculating the SWeights
    // and will be considered parameters, not observables.
    //
    // The SPlot2 will contain two new variables for each specie of name "varname":
    //
    // L_varname is the value of the pdf for the variable "varname" at values of this event
    // varname_sw is the value of the sWeight for the variable "varname" for this event
    //
    // If the fixed distribution is not known, calculates the c0 as per B.2., and replaces sWeights by their
    // extended counterparts.

    // Find Parameters in the PDF to be considered fixed when calculating the SWeights
    // and be sure to NOT include the yields in that list

    RooFit::MsgLevel currentLevel = RooMsgService::instance().globalKillBelow();

    RooArgList * constParameters = (RooArgList *) pdf->getParameters(fSData);
    constParameters->remove(yieldsTmp, kTRUE, kTRUE);

    // Set these parameters constant and store them so they can later
    // be set to not constant
    std::vector< RooRealVar * > constVarHolder;

    for (Int_t i = 0; i < constParameters->getSize(); i++) {
        RooRealVar * varTemp = (dynamic_cast< RooRealVar * >(constParameters->at(i)));
        if (varTemp && varTemp->isConstant() == 0) {
            varTemp->setConstant();
            constVarHolder.push_back(varTemp);
        }
    }

    // Fit yields to the data with all other variables held constant
    // This is necessary because SPlot2 assumes the yields minimise -Log(likelihood)
    pdf->fitTo(*fSData, RooFit::Extended(kTRUE), RooFit::SumW2Error(kTRUE), RooFit::PrintLevel(-1), RooFit::PrintEvalErrors(-1));

    // Store indexes of the allYieldsList corresponding to variable Yields
    TIterator *                 it = yieldsTmp.createIterator();
    RooAbsArg *                 arg;
    unsigned int                iArg(0);
    std::vector< unsigned int > varIndexes;
    while ((arg = (RooAbsArg *) it->Next()) != nullptr) {
        if (!fixedYield.find(arg->GetName())) {
            varIndexes.push_back(iArg);
            iArg++;
        }
    }
    delete it;

    // Hold the value of the fitted yields
    std::vector< double > yieldsHolder;

    for (Int_t i = 0; i < yieldsTmp.getSize(); i++) { yieldsHolder.push_back(((RooRealVar *) yieldsTmp.at(i))->getVal()); }

    // Int_t nspec = yieldsTmp.getSize();
    Int_t nAllSpec = yieldsTmp.getSize();
    Int_t nVarSpec = yieldsTmp.getSize() - fixedYield.getSize();

    RooArgList yields = *(RooArgList *) yieldsTmp.snapshot(kFALSE);

    if (currentLevel <= RooFit::DEBUG) {
        coutI(InputArguments) << "Printing Yields" << endl;
        yields.Print();
    }

    // The list of variables to normalize over when calculating PDF values.

    RooArgSet vars(*fSData->get());
    vars.remove(projDeps, kTRUE, kTRUE);

    // Attach data set

    // const_cast<RooAbsPdf*>(pdf)->attachDataSet(*fSData);

    pdf->attachDataSet(*fSData);

    // first calculate the pdf values for all species and all events
    std::vector< RooRealVar * > yieldvars;
    RooArgSet *                 parameters = pdf->getParameters(fSData);

    std::vector< Double_t > yieldvalues;
    for (Int_t k = 0; k < nAllSpec; ++k) {
        RooRealVar * thisyield = dynamic_cast< RooRealVar * >(yields.at(k));
        if (thisyield) {
            RooRealVar * yieldinpdf = dynamic_cast< RooRealVar * >(parameters->find(thisyield->GetName()));

            if (yieldinpdf) {
                coutI(InputArguments) << "yield in pdf: " << yieldinpdf->GetName() << " " << thisyield->getVal() << endl;
                yieldvars.push_back(yieldinpdf);
                yieldvalues.push_back(thisyield->getVal());
            }
        }
    }

    Int_t numevents = fSData->numEntries();

    std::vector< std::vector< Double_t > > pdfvalues(numevents, std::vector< Double_t >(nAllSpec, 0));

    // set all yield to zero
    for (Int_t m = 0; m < nAllSpec; ++m) yieldvars[m]->setVal(0);

    // For every event and for every specie,
    // calculate the value of the component pdf for that specie
    // by setting the yield of that specie to 1
    // and all others to 0.  Evaluate the pdf for each event
    // and store the values.

    RooArgSet * pdfvars = pdf->getVariables();

    for (Int_t ievt = 0; ievt < numevents; ievt++) {
        //   if (ievt % 100 == 0)
        //  coutP(Eval)  << ".";
        // FIX THIS PART, EVALUATION PROGRESS!!
        RooStats::SetParameters(fSData->get(ievt), pdfvars);
        //   RooArgSet row(*fSData->get(ievt));
        for (Int_t k = 0; k < nAllSpec; ++k) {
            // Check that range of yields is at least (0,1), and fix otherwise
            if (yieldvars[k]->getMin() > 0) {
                coutW(InputArguments) << "Minimum Range for " << yieldvars[k]->GetName() << " must be 0.  ";
                coutW(InputArguments) << "Setting min range to 0" << std::endl;
                yieldvars[k]->setMin(0);
            }
            if (yieldvars[k]->getMax() < 1) {
                coutW(InputArguments) << "Maximum Range for " << yieldvars[k]->GetName() << " must be 1.  ";
                coutW(InputArguments) << "Setting max range to 1" << std::endl;
                yieldvars[k]->setMax(1);
            }
            // set this yield to 1
            yieldvars[k]->setVal(1);
            // evaluate the pdf
            Double_t f_k       = pdf->getVal(&vars);
            pdfvalues[ievt][k] = f_k;
            if (!(f_k > 1 || f_k < 1)) coutW(InputArguments) << "Strange pdf value: " << ievt << " " << k << " " << f_k << std::endl;
            yieldvars[k]->setVal(0);
        }
    }
    delete pdfvars;

    // check that the likelihood normalization is fine
    // TEMP : will not work anymore since there are fixed Yields
    std::vector< Double_t > norm(nVarSpec, 0);
    for (Int_t ievt = 0; ievt < numevents; ievt++) {
        Double_t dnorm(0);
        for (Int_t k = 0; k < nAllSpec; ++k) dnorm += yieldvalues[k] * pdfvalues[ievt][k];
        for (Int_t j = 0; j < nVarSpec; ++j) norm[j] += pdfvalues[ievt][j] / dnorm;
    }
    coutI(Contents) << "The following likelihood norms will not be equal to 1 due to fixed yields." << std::endl;
    coutI(Contents) << "likelihood norms: ";

    for (Int_t k = 0; k < nVarSpec; ++k) { coutI(Contents) << norm[k] << " "; }
    coutI(Contents) << std::endl;

    // Make a TMatrixD to hold the covariance matrix.
    TMatrixD covInv(nVarSpec, nVarSpec);
    // TMatrixD covInv(nspec, nspec);
    // for (Int_t i = 0; i < nspec; i++) for (Int_t j = 0; j < nspec; j++) covInv(i,j) = 0;
    for (Int_t i = 0; i < nVarSpec; i++) {
        for (Int_t j = 0; j < nVarSpec; j++) { covInv(i, j) = 0; }
    }

    coutI(Contents) << "Calculating covariance matrix";

    // Calculate the inverse covariance matrix, using weights
    for (Int_t ievt = 0; ievt < numevents; ++ievt) {
        fSData->get(ievt);
        // Calculate contribution to the inverse of the covariance
        // matrix. See BAD 509 V2 eqn. 15
        // Sum for the denominator
        Double_t dsum(0);
        for (Int_t k = 0; k < nAllSpec; ++k) { dsum += pdfvalues[ievt][k] * yieldvalues[k]; }
        // Loop only over variable species which are floating!
        for (Int_t n = 0; n < nVarSpec; ++n) {
            for (Int_t j = 0; j < nVarSpec; ++j) {

                /*
                    if(includeWeights == kTRUE)
                        covInv(n,j) +=  fSData->weight()*pdfvalues[ievt][varIndexes[n]]
                                                        *pdfvalues[ievt][varIndexes[j]]/(dsum*dsum) ;
                                                                else
                                                                        covInv(n,j) +=  pdfvalues[ievt][varIndexes[n]]*pdfvalues[ievt][varIndexes[j]]/\
                (dsum*dsum) ;
                */
                if (includeWeights == kTRUE) {
                    covInv(n, j) += fSData->weight() * pdfvalues[ievt][varIndexes[n]] * pdfvalues[ievt][varIndexes[j]] / (dsum * dsum);
                    // covInv(n,j) +=  fSData->weight()*pdfvalues[ievt][n]*pdfvalues[ievt][j]/(dsum*dsum) ;
                } else {
                    // covInv(n,j) +=  pdfvalues[ievt][n]*pdfvalues[ievt][j]/(dsum*dsum) ;
                    covInv(n, j) += pdfvalues[ievt][varIndexes[n]] * pdfvalues[ievt][varIndexes[j]] / (dsum * dsum);
                }
            }
        }
        // ADDED WEIGHT ABOVE
    }

    // Covariance inverse should now be computed!

    // Invert to get the covariance matrix
    if (covInv.Determinant() <= 0) {
        coutE(Eval) << "SPlot2 Error: covariance matrix is singular; I can't invert it!" << std::endl;
        covInv.Print();
        return;
    }

    TMatrixD covMatrix(TMatrixD::kInverted, covInv);

    // check cov normalization
    if (currentLevel <= RooFit::DEBUG) {
        coutI(Eval) << "Checking Likelihood normalization:  " << std::endl;
        coutI(Eval) << "Yield of specie  Sum of Row in Matrix   Norm" << std::endl;
        for (Int_t k = 0; k < nVarSpec; ++k) {
            Double_t covnorm(0);
            for (Int_t m = 0; m < nVarSpec; ++m) covnorm += covInv[k][m] * yieldvalues[m];
            Double_t sumrow(0);
            for (Int_t m = 0; m < nVarSpec; ++m) sumrow += covMatrix[k][m];
            coutI(Eval) << yieldvalues[k] << " " << sumrow << " " << covnorm << endl;
        }
    }

    // calculate for each event the sWeight (BAD 509 V2 eq. 21)
    coutI(Eval) << "Calculating sWeight" << std::endl;
    std::vector< RooRealVar * > sweightvec;
    std::vector< RooRealVar * > pdfvec;
    RooArgSet                   sweightset;

    // Create and label the variables
    // used to store the SWeights
    fSWeightVars.Clear();

    for (Int_t k = 0; k < nVarSpec; ++k) {
        std::string  wname = std::string(yieldvars[varIndexes[k]]->GetName()) + "_sw";
        RooRealVar * var   = new RooRealVar(wname.c_str(), wname.c_str(), 0);
        sweightvec.push_back(var);
        sweightset.add(*var);
        fSWeightVars.add(*var);

        wname = "L_" + std::string(yieldvars[varIndexes[k]]->GetName());
        var   = new RooRealVar(wname.c_str(), wname.c_str(), 0);
        pdfvec.push_back(var);
        sweightset.add(*var);

        // C coefficients
        wname = std::string(yieldvars[varIndexes[k]]->GetName()) + "_c";
        var   = new RooRealVar(wname.c_str(), wname.c_str(), 0);
        // Calculate the c right now
        double cVal = yieldvalues[varIndexes[k]];
        for (Int_t n = 0; n < nVarSpec; ++n) cVal -= covMatrix[k][n];
        var->setVal(cVal);
        fSWeightCoefs.add(*var);
    }

    // Create and fill a RooDataSet
    // with the SWeights

    RooDataSet * sWeightData = new RooDataSet("dataset", "dataset with sWeights", sweightset);

    for (Int_t ievt = 0; ievt < numevents; ++ievt) {
        fSData->get(ievt);
        // sum for denominator
        Double_t dsum(0);
        for (Int_t k = 0; k < nAllSpec; ++k) { dsum += pdfvalues[ievt][k] * yieldvalues[k]; }
        // covariance weighted pdf for each specief
        for (Int_t n = 0; n < nVarSpec; ++n) {
            Double_t nsum(0);
            for (Int_t j = 0; j < nVarSpec; ++j) nsum += covMatrix(n, j) * pdfvalues[ievt][varIndexes[j]];

            // Add the sWeights here!!
            // Include weights,
            // ie events weights are absorbed into sWeight
            if (includeWeights == kTRUE)
                sweightvec[n]->setVal(fSData->weight() * nsum / dsum);
            else
                sweightvec[n]->setVal(nsum / dsum);

            pdfvec[n]->setVal(pdfvalues[ievt][varIndexes[n]]);

            if (!(fabs(nsum / dsum) >= 0)) {
                coutE(Contents) << "error: " << nsum / dsum << endl;
                return;
            }
        }

        sWeightData->add(sweightset);
    }

    // Add the SWeights to the original data set

    fSData->merge(sWeightData);

    delete sWeightData;

    // Restore yield values

    for (Int_t i = 0; i < yieldsTmp.getSize(); i++) ((RooRealVar *) yieldsTmp.at(i))->setVal(yieldsHolder.at(i));

    // Make any variables that were forced to constant no longer constant

    for (Int_t i = 0; i < (Int_t) constVarHolder.size(); i++) constVarHolder.at(i)->setConstant(kFALSE);

    return;
}
