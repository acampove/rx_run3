#pragma once

#include "VariableBinning.hpp"
#include "ConfigHolder.hpp"
#include "WeightHolder.hpp"
#include "TupleHolder.hpp"
#include "CutHolder.hpp"
#include "EffSlot.hpp"
#include "TString.h"
#include "TH2Poly.h"
#include <vector> 
#include <string> 
#include <map> 

using namespace std;

/**
 * @brief Class meant to contain static methods used in efficiency calculation
 */
class EfficiencyHelpers
{
    public:

        static void LoadTH2DFlatness(
                map< TString, map< TString, TH2Poly * > > & _histo2D, 
                map< TString, map< TString, TH2D    * > > & _histo2D_raw, 
                const vector< VariableBinning > & _vars, 
                TString _effWeight, 
                TString _normNumWeight, 
                TString _normDenWeight);

        static void LoadTH1DFlatness(
                map< TString, map< TString, TH1D * > > & _histo1D, 
                const vector< VariableBinning >        & _vars, 
                TString _effWeight, 
                TString _normNumWeight, 
                TString _normDenWeight);

        static void LoadTH1DModels(
                map< TString, map< TString, ROOT::RDF::TH1DModel > > & _histo1D, 
                const vector< VariableBinning > & _vars, 
                TString _effWeight, 
                TString _normNumWeight, 
                TString _normDenWeight, 
                bool isBS = false);

        static auto bookkepingName(
                const EffSlot      & _effStepType, 
                const ConfigHolder & _ConH_BASE, 
                const TString      & _weightConfiguration, 
                bool clean         = false, 
                bool rootfile      = false);


        static vector< pair< string, string > > GetVariablesForPlot(const vector< VariableBinning > & _vars);

        static void PrintAndTestMap(const map< TString, pair< TupleHolder, vector< tuple< ConfigHolder, CutHolder, WeightHolder > > > > & mymap);
};
