#pragma once

#include "VariableBinning.hpp"
#include "TString.h"
#include "TH2Poly.h"
#include <vector> 
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

};
