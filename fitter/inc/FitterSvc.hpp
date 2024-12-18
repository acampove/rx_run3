#ifndef FITTERSVC_HPP
#define FITTERSVC_HPP

#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds

#include "EnumeratorSvc.hpp"
#include "HelperSvc.hpp"
#include "MessageSvc.hpp"

#include "EventType.hpp"
#include "UnblindParameter.hpp"

#include "yamlcpp.h"
#include <algorithm>
#include <iostream>
#include <map>
#include <vector>

#include "TCanvas.h"
#include "TF1.h"
#include "TLegend.h"
#include "TString.h"

#include "RooAbsPdf.h"
#include "RooAbsReal.h"
#include "RooArgSet.h"
#include "RooBinning.h"
#include "RooDataSet.h"
#include "RooFitResult.h"
#include "RooFormulaVar.h"
#include "RooHist.h"
#include "RooPlot.h"
#include "RooRealSumPdf.h"
#include "RooRealVar.h"

#include "RooAddPdf.h"
#include "RooArgusBG.h"
#include "RooBifurGauss.h"
#include "RooBreitWigner.h"
#include "RooCBShape.h"
#include "RooChebychev.h"
#include "RooDoubleSidedCBShape.h"
#include "RooBifurDSCBShape.h"

#include "RooExpGaussExp.h"
#include "RooGenericPdf.h"
#include "RooExponential.h"
#include "RooGExpModel.h"
#include "RooGamma.h"
#include "RooGaussian.h"
#include "RooJohnson.h"
#include "RooLandau.h"
#include "RooKeysPdf.h"
#include "RooNDKeysPdf.h"
#include "RooPolynomial.h"
#include "RooTFnBinding.h"
#include "RooTFnPdfBinding.h"
#include "RooLandauAnalytical.h"
#include "RooDoubleCB.h"
#include "RooExpAndGauss.h"
#include "RooInverseArgus.h"
#include "RooIpatia.h"
#include "RooIpatia2.h"
#include "RooExpTurnOn.h"
#include "RooExpTurnOnNumerical.h"

class FitComponentAndYield;
class FitHolder;
class FitManager;

/**
 * \typedef Str2ComponentMap
 * \brief Template map for TString to FitComponentAndYield
 */
typedef map< TString, FitComponentAndYield > Str2ComponentMap;

/**
 * \typedef Str2HolderMap
 * \brief Template map for TString to FitHolder
 */
typedef map< TString, FitHolder > Str2HolderMap;

/**
 * \typedef Str2HolderBremMap
 * \brief Template map for TString to Str2HolderMap
 */
typedef map< TString, Str2HolderMap > Str2HolderBremMap;

/**
 * \typedef Str2ManagerMap
 * \brief Template map for TString to FitManager
 */
typedef map< TString, FitManager > Str2ManagerMap;

/**
 * \struct FitterInfoContainer
 * \brief Struct containing all the necessary info from one fit, dataset, datahist components of the fitmodel, the fitmodel and the likelihood for the binned/unbinned fit
          Internal representation  of a RooCategory for a give fit Tree (B0 ->  JPs Kst) one for each [EE-L0L, EE-L0I, ...] object for a given fit
 */
struct FitterInfoContainer {
    RooRealVar * var;
    bool         binned = false;
    bool         ismc   = false;

    TString extraRange = "";   // no named ranges for a given FitHolder

    RooDataSet *  dataset;
    RooDataHist * datahist;


    RooAbsPdf *                 fullmodel;
    map< TString, RooAbsPdf  * > components;
    map< TString, RooAbsReal * > yields;

    map< TString, TString >     labels;
    map< TString, Int_t   >     colors; 
    RooAbsReal * nll = nullptr;
};

/**
 * \struct QSquareFit
 * \brief Struct holding  the baseline information of a given fitter [ fit to JPs or Psi or JPsK (EE) + JPsK(MM) ]
 */
struct QSquareFit {
    FitManager *                        manager;   // Pointer to the fit manager to easily extract informations of the various categories for the fit
    map< TString, FitterInfoContainer > FitInfo;   // <- map holding the FitterInfoContainer for EE FitHolders stripped from FitManager
};

struct CorrelatedConstraintsInfo {
    vector <RooRealVar*> variables;
    vector <RooRealVar*> constraintMeans;
    TMatrixDSym          covariance;

    CorrelatedConstraintsInfo(){ };
    CorrelatedConstraintsInfo(const vector<RooRealVar*> & _variables, const vector <RooRealVar*> & _constraintMeans, const TMatrixDSym & _covariance) :
        variables(_variables),
        constraintMeans(_constraintMeans),
        covariance(_covariance) { };
};

Str2VarMap GetPars(RooAbsPdf * pdf, RooArgSet obs, vector< string > pnames, TString _option = "");
Str2VarMap GetPars(RooAbsPdf * pdf, RooAbsReal * var, TString _option = "");

Str2VarMap FixPars(Str2VarMap * _map, vector< string > names = vector< string >(), TString _option = "" , double min = std::numeric_limits<double>::min(), double max = std::numeric_limits<double>::max() );
Str2VarMap FixPars(Str2VarMap * _map, string name, TString _option = "");

Str2VarMap ModifyPars(Str2VarMap * _map, vector< string > names, vector< RooRealVar * > c, vector< string > opt);
Str2VarMap ModifyPars(Str2VarMap * _map, string name, RooRealVar * c, string opt = "-scale");

string GetOptionVal(string _option, string _string);

RooRealVar * AddPar(string par, string parstr, Str2VarMap stval_list, Str2VarMap _map, string pmode = "v");

TString GetPrintParName(TString namepdf_, string opt);

Str2VarMap GetStr2VarMap(string typepdf_, TString namepdf_, RooRealVar * val, Str2VarMap _map = Str2VarMap(), string pmode = "v", TString title = "");

RooAbsPdf * StringToPdf(const char * typepdf, const char * namepdf, RooRealVar * var, Str2VarMap _map = Str2VarMap(), string opt = "", TString title = "");

RooAbsPdf * TF1ToPdf(TString _name, TString _formula, const RooArgList & _pars, RooRealVar * _var = nullptr);

RooAbsReal * BlindParameter(RooRealVar * _par, BlindMode _mode = BlindMode::OffsetScale, double _scaleLimit = 10.);//IMPORTANT SCALE LIMIT ( if < 0)--> SAME SCALE everywhere

vector< double > GetFractions(EventType & _eventType, const RooRealVar & _var, vector< TCut > _cuts);

RooAddPdf * SumPDFs(TString _name, RooArgList _pdfs, vector< double > _coefs, RooRealVar * _var, TString _option = "const");

void SaveToDOT(RooAbsArg * _rooAbsArg, TString _file, TString _option = "");

void SaveToTEX(RooFitResult * _fitResults, TString _file, bool _minos = false);

void SaveToYAML(RooFitResult * _fitResults, TString _file, TString _option = "");

vector< RooRealVar * > LoadFromYAML(TString _file);

pair< double, double > GetSPlotYield(TString _project, TString _ana, TString _year, TString _trigger);

TMatrixDSym MatrixToROOTMatrix(const vector< vector< double > > & _CMatrix);


template < typename T > RooArgList VectorToRooArglist(const vector< T * > & _vectorOfPointers) {
    RooArgList _returnList;
    // Check if object has correct class
    for (T * _object : _vectorOfPointers) {
        if (not(is_base_of< RooAbsArg, T >::value)) { MessageSvc::Error("VectorToRooArglist", (TString) "Trying to insert an object not inherited RooAbsArg to a RooArgList", "EXIT_FAILURE"); }
    }
    for (T * _object : _vectorOfPointers) { _returnList.add(*_object); }
    return _returnList;
}

TString HashString(const TString& _string);

#endif
