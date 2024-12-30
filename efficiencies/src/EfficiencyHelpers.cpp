#include "EfficiencyHelpers.hpp"
#include "VariableBinning.hpp"
#include "ConfigHolder.hpp"
#include "EffSlot.hpp"
#include "MessageSvc.hpp"
#include "TString.h"
#include "TH2Poly.h"
#include <vector> 
#include <map> 

using namespace std;

void EfficiencyHelpers::LoadTH2DFlatness(
        map< TString, map< TString, TH2Poly * > > & _histo2D, 
        map< TString, map< TString, TH2D    * > > & _histo2D_raw, 
        const vector< VariableBinning > & _vars, 
        TString _effWeight, 
        TString _normNumWeight, 
        TString _normDenWeight) 
{
    MessageSvc::Debug("Booking TH1D Histos for flatness");

    for (auto const & var : _vars) 
    {
        if (var.is1D()) 
            continue;

        _histo2D[var.varID()]     = {
            {"sumW"            , (TH2Poly *) var.GetHistoClone(var.varID() + "_sumW"        , _effWeight     + " | full")}, 
            {"normN"           , (TH2Poly *) var.GetHistoClone(var.varID() + "_normN"       , _normNumWeight + " | norm")}, 
            {"normD"           , (TH2Poly *) var.GetHistoClone(var.varID() + "_normD"       , _normDenWeight + " | norm")}, 
            {"sumW_rnd"        , (TH2Poly *) var.GetHistoClone(var.varID() + "_sumW_rnd"    , _effWeight     + " | full")}, 
            {"normN_rnd"       , (TH2Poly *) var.GetHistoClone(var.varID() + "_normN_rnd"   , _normNumWeight + " | norm")}, 
            {"normD_rnd"       , (TH2Poly *) var.GetHistoClone(var.varID() + "_normD_rnd"   , _normDenWeight + " | norm")}, 
            {"sumW_bkgcat"     , (TH2Poly *) var.GetHistoClone(var.varID() + "_sumW_bkgcat" , _effWeight     + " | full")}, 
            {"normN_bkgcat"    , (TH2Poly *) var.GetHistoClone(var.varID() + "_normN_bkgcat", _normNumWeight + " | norm")}, 
            {"normD_bkgcat"    , (TH2Poly *) var.GetHistoClone(var.varID() + "_normD_bkgcat", _normDenWeight + " | norm")}};

        _histo2D_raw[var.varID()] = {
            {"sumW_raw"        , (TH2D *)    var.GetRawHisto2D(var.varID() + "_sumW_raw"        , _effWeight     + " | full", 100, 100)},
            {"normN_raw"       , (TH2D *)    var.GetRawHisto2D(var.varID() + "_normN_raw"       , _normNumWeight + " | norm", 100, 100)},
            {"normD_raw"       , (TH2D *)    var.GetRawHisto2D(var.varID() + "_normD_raw"       , _normDenWeight + " | norm", 100, 100)},
            {"sumW_rnd_raw"    , (TH2D *)    var.GetRawHisto2D(var.varID() + "_sumW_rnd_raw"    , _effWeight     + " | full", 100, 100)},
            {"normN_rnd_raw"   , (TH2D *)    var.GetRawHisto2D(var.varID() + "_normN_rnd_raw"   , _normNumWeight + " | norm", 100, 100)},
            {"normD_rnd_raw"   , (TH2D *)    var.GetRawHisto2D(var.varID() + "_normD_rnd_raw"   , _normDenWeight + " | norm", 100, 100)},
            {"sumW_bkgcat_raw" , (TH2D *)    var.GetRawHisto2D(var.varID() + "_sumW_bkgcat_raw" , _effWeight     + " | full", 100, 100)},
            {"normN_bkgcat_raw", (TH2D *)    var.GetRawHisto2D(var.varID() + "_normN_bkgcat_raw", _normNumWeight + " | norm", 100, 100)},
            {"normD_bkgcat_raw", (TH2D *)    var.GetRawHisto2D(var.varID() + "_normD_bkgcat_raw", _normDenWeight + " | norm", 100, 100)}};
    }
}

void EfficiencyHelpers::LoadTH1DFlatness(
        map< TString, map< TString, TH1D * > > & _histo1D, 
        const vector< VariableBinning >        & _vars, 
        TString _effWeight, 
        TString _normNumWeight, 
        TString _normDenWeight) 
{
    MessageSvc::Debug("Booking TH1D Histos for flatness");

    for (auto const & var : _vars)
    {
        if (!var.is1D()) 
            continue;

        _histo1D[var.varID()] = 
        {
            {"sumW_rnd", (TH1D *) var.GetHistoClone(var.varID() + "_sumW_rnd", _effWeight + " | full")},
            {"normN_rnd", (TH1D *) var.GetHistoClone(var.varID() + "_normN_rnd", _normNumWeight + " | norm")},
            {"normD_rnd", (TH1D *) var.GetHistoClone(var.varID() + "_normD_rnd", _normDenWeight + " | norm")},
            {"sumW_bkgcat", (TH1D *) var.GetHistoClone(var.varID() + "_sumW_bkgcat", _effWeight + " | full")},
            {"normN_bkgcat", (TH1D *) var.GetHistoClone(var.varID() + "_normN_bkgcat", _normNumWeight + " | norm")},
            {"normD_bkgcat", (TH1D *) var.GetHistoClone(var.varID() + "_normD_bkgcat", _normDenWeight + " | norm")},

            {"sumW_rnd_raw", (TH1D *) var.GetRawHisto1D(var.varID() + "_sumW_rnd_raw", _effWeight + " | full", 100)},
            {"normN_rnd_raw", (TH1D *) var.GetRawHisto1D(var.varID() + "_normN_rnd_raw", _normNumWeight + " | norm", 100)},
            {"normD_rnd_raw", (TH1D *) var.GetRawHisto1D(var.varID() + "_normD_rnd_raw", _normDenWeight + " | norm", 100)},

            {"sumW_bkgcat_raw", (TH1D *) var.GetRawHisto1D(var.varID() + "_sumW_bkgcat_raw", _effWeight + " | full", 100)},
            {"normN_bkgcat_raw", (TH1D *) var.GetRawHisto1D(var.varID() + "_normN_bkgcat_raw", _normNumWeight + " | norm", 100)},
            {"normD_bkgcat_raw", (TH1D *) var.GetRawHisto1D(var.varID() + "_normD_bkgcat_raw", _normDenWeight + " | norm", 100)}};
    }
}

void EfficiencyHelpers::LoadTH1DModels(
        map< TString, map< TString, ROOT::RDF::TH1DModel > > & _histo1D, 
        const vector< VariableBinning > & _vars, 
        TString _effWeight, 
        TString _normNumWeight, 
        TString _normDenWeight, 
        bool    isBS) 
{
    MessageSvc::Debug("Booking TH1D Histos for flatness");

    for (auto const & var : _vars) 
    {
        if (!var.is1D()) 
        {
            MessageSvc::Warning("LoadTH1DModels not supported for > 1D yet");
            continue;
        }

        _histo1D[var.varID()] = 
        {
            {"sumW",      var.GetHisto1DModel(var.varID()    + "_sumW"     , _effWeight     + " | full"     )}, 
            {"normN",     var.GetHisto1DModel(var.varID()    + "_normN"    , _normNumWeight + " | norm"     )}, 
            {"normD",     var.GetHisto1DModel(var.varID()    + "_normD"    , _normDenWeight + " | norm"     )},
            {"sumW_raw",  var.GetRawHisto1DModel(var.varID() + "_sumW_raw" , _effWeight     + " | full", 100)}, 
            {"normN_raw", var.GetRawHisto1DModel(var.varID() + "_normN_raw", _normNumWeight + " | norm", 100)}, 
            {"normD_raw", var.GetRawHisto1DModel(var.varID() + "_normD_raw", _normDenWeight + " | norm", 100)},
        };
    }
}

auto EfficiencyHelpers::bookkepingName(
        const EffSlot      & _effStepType, 
        const ConfigHolder & _ConH_BASE, 
        const TString      & _weightConfiguration, 
        bool clean, 
        bool rootfile) 
{
    TString _bookkepingName = _effStepType.wOpt() + "_Efficiency_" + _ConH_BASE.GetKey("addtrgconf");
    if (_effStepType.wOpt() != "no") 
        _bookkepingName = _bookkepingName + "_" + _weightConfiguration; 

    if (clean) 
        _bookkepingName.ReplaceAll("-", "_"); 

    if (rootfile) 
    {
        _bookkepingName = _effStepType.wOpt() + "_" + _ConH_BASE.GetSample() + "_Efficiency_" + _ConH_BASE.GetKey("addtrgconf");
        if (_effStepType.wOpt() != "no") 
            _bookkepingName = _bookkepingName + "_" + _weightConfiguration;

        _bookkepingName += ".root";
    }
    return _bookkepingName;
};

