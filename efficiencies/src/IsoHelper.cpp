#ifndef ISOHELPER_CPP
#define ISOHELPER_CPP

#include "IsoHelper.hpp"

#include "HelperSvc.hpp"

#include "EventType.hpp"
#include "RXDataPlayer.hpp"

#include "boost/program_options.hpp"
#include "core.h"
#include "fmt_ostream.h"
#include "vec_extends.h"
#include <algorithm>
//#include <boost/progress.hpp>
#include <boost/timer/timer.hpp>
#include <iostream>
#include <numeric>
#include <vector>

#include "TCanvas.h"
#include "TChain.h"
#include "TH2Poly.h"
#include "TTreeFormula.h"

IsoBinReport GetIsoBinReport(EventType & _eventType, TString _VarName, double min, double max, const TCut & _Cut, const TString & _Weight, double frac, bool debug) {
    IsoBinReport Report;
    Report.sumOfWeightsPas    = 0;
    Report.sumOfEntriesPas    = 0;
    Report.varFill_weightFill = {};
    debug                     = true;
    Long64_t _nEntries        = _eventType.GetTuple()->GetEntries();
    if (frac > 1 && frac < _nEntries) { _nEntries = frac; }
    if (frac > 0 && frac < 1) { _nEntries = floor(_nEntries * frac); }

    TString _CutExpression    = _Cut == "" ? (TString) _eventType.GetCut() : (TString) _Cut;
    TString _WeightExpression = _Weight == "" ? _eventType.GetWeight() : _Weight;

    auto branches = GetBranchesFromExpression(_eventType.GetTuple(), _CutExpression) + GetBranchesFromExpression(_eventType.GetTuple(), _WeightExpression) + GetBranchesFromExpression(_eventType.GetTuple(), _VarName);
    _eventType.GetTuple()->SetBranchStatus("*", 0);
    for (auto & b : branches) {
        if (debug) { MessageSvc::Warning("ENABLING BRANCH ", b); }
        _eventType.GetTuple()->SetBranchStatus(b, 1);
    }

    bool _isTChain = false;
    if (TString(_eventType.GetTuple()->ClassName()) == "TChain") {
        MessageSvc::Warning("Processing a TChain");
        _isTChain = true;
        _eventType.GetTuple()->LoadTree(0);
    } else {
        MessageSvc::Warning("Processing a TTree");
    }

    TTreeFormula * cutFormula = new TTreeFormula("CUT", _CutExpression, _eventType.GetTuple());
    if (_isTChain) _eventType.GetTuple()->SetNotify(cutFormula);

    TTreeFormula * weightFormula = new TTreeFormula("WEIGHT", _WeightExpression, _eventType.GetTuple());
    if (_isTChain) _eventType.GetTuple()->SetNotify(weightFormula);

    TTreeFormula * toPlot = new TTreeFormula("VAR", _VarName, _eventType.GetTuple());

    if (_isTChain) _eventType.GetTuple()->SetNotify(toPlot);

    MessageSvc::Info("Processing ", (TString) fmt::format("ET KEY {0}, Entries = {1} ", _eventType.GetKey(), _nEntries));
    // boost::progress_display show_progress_evtloop(_nEntries);
    Report.varFill_weightFill.reserve(_nEntries);
    int underflow = 0;
    int overflow  = 1;
    for (Long64_t entry = 0; entry < _nEntries; ++entry) {
        //++show_progress_evtloop;
        if (_isTChain) {
            _eventType.GetTuple()->LoadTree(entry);
            _eventType.GetTuple()->GetEntry(entry);
        } else {
            _eventType.GetTuple()->GetEntry(entry);
        }
        if (_isTChain) {
            cutFormula->UpdateFormulaLeaves();
            toPlot->UpdateFormulaLeaves();
            weightFormula->UpdateFormulaLeaves();
        }

        // cout<<"EVALCUT("<<entry<<")"<<endl;
        bool _cutISPas = (bool) cutFormula->EvalInstance(0);
        // cout<<"EVALVALUE("<<entry<<")"<<endl;
        double _val = toPlot->EvalInstance(0);
        // cout<<"EVALWEIGHT("<<entry<<")"<<endl;
        double _weight = weightFormula->EvalInstance(0);
        Report.sumOfEntries++;

        if (_cutISPas == false) { continue; }
        if (_val < min) {
            underflow++;
            continue;
        }
        if (_val > max) {
            overflow++;
            continue;
        }
        Report.sumOfEntriesPas++;
        Report.varFill_weightFill.push_back(make_pair(_val, _weight));
        Report.sumOfWeightsPas += _weight;
    }
    if (underflow > 0) { MessageSvc::Warning((TString) fmt::format("ISOBINREPORT : UNDERFLOW N Entries = {0}", underflow)); }
    if (overflow > 0) { MessageSvc::Warning((TString) fmt::format("ISOBINREPORT : OVERFLOW N Entries = {0}", overflow)); }
    MessageSvc::Info("Processing Completed", (TString) fmt::format("ET KEY {0}, Entries = {1}, EntriesPas = {2} ", _eventType.GetKey(), _nEntries, Report.sumOfEntriesPas));
    _eventType.GetTuple()->SetBranchStatus("*", 1);
    if (_isTChain) {
        _eventType.GetTuple()->LoadTree(0);
        // _eventType.GetTuple()->Reset();
    }
    delete cutFormula;
    delete weightFormula;
    delete toPlot;
    return Report;
};

array< Double_t, MAXNBINS > GetBoundariesForPlot(const vector< pair< double, double > > & Value_Weights, int nBins, double min, double max) {
    if (nBins > MAXNBINS) { MessageSvc::Error("Boundaries for plot", (TString) fmt::format("You asked {0} bins, supported MAX {1}", nBins, MAXNBINS), "EXIT_FAILURE"); }
    // we want nBins, _boundaries[0], ......... _boundaries[nBins+1] should be filled
    array< double, MAXNBINS > _boundaries;
    double                    sumOfWeightsTotal = 0;
    for (const auto & el : Value_Weights) { sumOfWeightsTotal += el.second; }
    cout << " SUM OF WEIGHTS = " << sumOfWeightsTotal << endl;
    /* forced boundaries ! */
    _boundaries[0]     = min;
    _boundaries[nBins] = max;
    // if (Value_Weights.front().first < min) {
    //     MessageSvc::Warning("Rectifying min, enlarging it, new min = ", TString(fmt::format("{0}", min)));
    //     _boundaries[0] = min;
    // }
    // if (Value_Weights.back().first > max) {
    //     MessageSvc::Warning("Rectifying max, enlarging it, new max = ", TString(fmt::format("{0}", max)));
    //     _boundaries[nBins] = max;
    // }

    // let's say we have Total Sum of Weights = 300, want this into 3 bins, we have to slice is in 100 comulative stuff
    // |----- 100 ---- |  --- 100 ---- | ---- 100 ---- |
    // 4 boundaries, we just have to find 2 of them for 3 bins, i.e. NBins-1
    //
    int    idx        = 1;
    double Size_Slice = (double) sumOfWeightsTotal / (double) nBins;
    //< actually we may need SumOfAllWeights / nBins ?
    double sumWeight = 0;
    for (int idx_loop = 0; idx_loop < Value_Weights.size(); ++idx_loop) {
        if (idx == nBins && Value_Weights[idx_loop].first <= max) continue;
        sumWeight += Value_Weights[idx_loop].second;
        if (sumWeight >= Size_Slice) {
            cout << " Filling boundary IDX = " << idx << " value " << Value_Weights[idx_loop].first << endl;
            _boundaries[idx] = Value_Weights[idx_loop].first;   // value for it
            idx++;
            // restart it moving to search for new idx.
            sumWeight = Value_Weights[idx_loop].second;
        }
    }
    _boundaries[nBins] = max;
    // cout<<" Last IDX filled = "<< idx << endl;
    return _boundaries;
}

vector< pair< double, double > > ScaleAndWeightSorted(vector< double > & values) {
    vector< pair< double, double > > value_weights;
    value_weights.reserve(values.size());
    if (!std::is_sorted(values.begin(), values.end())) { MessageSvc::Error("ScaleAnaWeightSorted", (TString) "Input Vector not Sorted", "EXIT_FAILURE"); }
    for (auto & el : values) { value_weights.push_back(make_pair(el, 1000. / values.size())); }
    return value_weights;
};
vector< pair< Var2D, double > > ScaleAndWeightSorted(vector< Var2D > & values) {
    vector< pair< Var2D, double > > value_weights;
    value_weights.reserve(values.size());
    // if( !std::is_sorted( values.begin(), values.end())){
    // MessageSvc::Error( "ScaleAnaWeightSorted",(TString)"Input Vector not Sorted","EXIT_FAILURE");
    // }
    for (auto & el : values) { value_weights.push_back(make_pair(Var2D(el.x, el.Y()), 1000. / values.size())); }
    return value_weights;
};

vector< double > filterVector(const vector< double > & invector, pair< double, double > range) {
    vector< double > fitleredVector;
    fitleredVector.reserve(invector.size());
    for (auto & el : invector) {
        if (el <= range.first) continue;
        if (el > range.second) continue;
        fitleredVector.push_back(el);
    }
    return fitleredVector;
};

vector< Var2D > filterVectorX(const vector< Var2D > & invector, pair< double, double > rangeX) {
    vector< Var2D > fitleredVector;
    fitleredVector.reserve(invector.size());
    for (auto & el : invector) {
        if (el.x <= rangeX.first) continue;
        if (el.x > rangeX.second) continue;
        fitleredVector.push_back(el);
    }
    return fitleredVector;
};
vector< Var2D > filterVectorY(const vector< Var2D > & invector, pair< double, double > rangeY) {
    vector< Var2D > fitleredVector;
    fitleredVector.reserve(invector.size());
    for (auto & el : invector) {
        if (el.Y() <= rangeY.first) continue;
        if (el.Y() > rangeY.second) continue;
        fitleredVector.push_back(el);
    }
    return fitleredVector;
};
vector< TCut > getVectorCut(double * _boundaries, TString _varIso, int _nbins, double _min, double _max) {
    vector< TCut > _cuts;
    // Update boundaries!
    for (int i = 0; i < _nbins; ++i){
        double  min        = i == 0 ? _min : _boundaries[i];
        double  max        = i == _nbins - 1 ? _max : _boundaries[i + 1];
        TString _selection = Form("( (%s>=%.8f) && (%s<%.8f) )", _varIso.Data(), min, _varIso.Data(), max);
        if( CheckVectorContains( _cuts, TCut(_selection) ) ){ MessageSvc::Error("CSV writing failed, double precision reach error", "","EXIT_FAILURE");}
        _cuts.push_back(TCut(_selection));
    }
    return _cuts;
};
void WriteCSVfile(vector< TCut > & _cuts, TString _ID) {
    int     binIndex    = 0;
    TString CSVFileName = _ID + "_" + outCSVFile;
    if (IOSvc::ExistFile(CSVFileName)) { MessageSvc::Error("File Exists ", CSVFileName, "EXIT_FAILURE"); }
    ofstream file(CSVFileName.Data());
    for (int i = 0; i < _cuts.size(); ++i) {
        if (!file.is_open()) MessageSvc::Error("Unable to open file", CSVFileName, "EXIT_FAILURE");
        TString binCut = TString(_cuts[i]);
        file << "Bin " << to_string(binIndex) << " : " << binCut << endl;
        binIndex++;
    }
    file.close();
};

// void BuildBinningScheme1D(EventType & _eTypeEE, EventType & _eTypeMM, const IsoBinningInputs & _isoBinInput, bool OnlyMuon , bool OnlyElectron ){

void BuildBinningScheme1D(EventType & _eTypeEE, EventType & _eTypeMM, const vector<IsoBinningInputs> & _isoBinInputs, bool OnlyMuon , bool OnlyElectron ){

   if( OnlyMuon == true && OnlyElectron == true){
     MessageSvc::Error("Invalid","","EXIT_FAILURE");
   }
    // Lamda fucntion to Process the data Player for this Building of 1D binning
    auto processThem = [](RXDataPlayer & dp, EventType & et, vector<TString> & _isoBinVariables) {
        auto BMass = et.GetParticleNames()["HEAD"] + "_DTF_JPs_M";
        dp.BookVariable(BMass, "m(B)", 100, 4800, 5600, "MeV", false, false);   // delegate bookkeping
        for( auto & _isoBinVariable : _isoBinVariables ){
            dp.BookVariable(_isoBinVariable, _isoBinVariable, 100, std::numeric_limits< double >::min(), std::numeric_limits< double >::max(), "", false, false);
        }
        dp.BookWeight("1.", "noW");
        auto alias = GetAllAliases(et.GetTuple());
        dp.AddAliases(alias);
        dp.SetBaseCut(TString(et.GetCut()));
        dp.Process(*et.GetTuple());
        return;
    };

    //===== Build List of Variables to Define column for ( to make IsoBinning operations later ) ====//
    vector<TString> _isoBinVariables = {}; 
    for( const auto & _isoBinInput : _isoBinInputs ){
        TString _isoBinVariable = _isoBinInput.variables.at(0);
        _isoBinVariables.push_back(_isoBinVariable);       
    }

    //===== Use RXDataPlayer =====//
    RXDataPlayer dpEE("EE");
    RXDataPlayer dpMM("MM");
    if( OnlyMuon == true && OnlyElectron == false){
        processThem(dpEE, _eTypeMM, _isoBinVariables);        
        processThem(dpMM, _eTypeMM, _isoBinVariables);        
    }else if( OnlyMuon == false && OnlyElectron == true){
        processThem(dpEE, _eTypeEE, _isoBinVariables);        
        processThem(dpMM, _eTypeEE, _isoBinVariables);        
    }else if( OnlyMuon == false && OnlyElectron ==false){
        processThem(dpEE, _eTypeEE, _isoBinVariables);        
        processThem(dpMM, _eTypeMM, _isoBinVariables);        
    }else{
      MessageSvc::Error("Invalid switch","","EXIT_FAILURE");
    }

    //===== Use RXDataPlayer Columns extracted to make isobinning around =====//

    for( auto & _isoBinInput : _isoBinInputs ){
        TString _isoBinVariable = _isoBinInput.variables.at(0);
        int nBins = _isoBinInput.nBins.at(0);
        pair<double,double> min_max = _isoBinInput.MinMax.at(0);
        TString labelAxis = _isoBinInput.labelAxis.at(0);
        TString _ID = _isoBinInput.ID;
        TString _histoType = _isoBinInput.HistoType;

        vector< double > _eeValues = dpEE.GetVariableValues(_isoBinVariable);
        vector< double > _mmValues = dpMM.GetVariableValues(_isoBinVariable);
        std::stable_sort(_eeValues.begin(), _eeValues.end(), [](double a, double b) { return a <= b; });
        std::stable_sort(_mmValues.begin(), _mmValues.end(), [](double a, double b) { return a <= b; });
        std::pair< double, double >      commonRange        = make_pair(std::max(_eeValues.front(), _mmValues.front()), std::min(_eeValues.back(), _mmValues.back()));
        std::pair< double, double >      fullRange          = make_pair(std::min(_eeValues.front(), _mmValues.front()), std::max(_eeValues.back(), _mmValues.back()));

        //Force axis ranges
        commonRange.first = min_max.first; commonRange.second = min_max.second;
        fullRange.first   = min_max.first; fullRange.second   = min_max.second;

        vector< double >                 ranged_EE          = filterVector(_eeValues, commonRange);
        vector< double >                 ranged_MM          = filterVector(_mmValues, commonRange);
        auto                             weightedEntries_EE = ScaleAndWeightSorted(ranged_EE);
        auto                             weightedEntries_MM = ScaleAndWeightSorted(ranged_MM);
        vector< pair< double, double > > _merged;
        for (auto & el : weightedEntries_MM) { _merged.push_back(el); }
        for (auto & el : weightedEntries_EE) { _merged.push_back(el); }
        std::stable_sort(_merged.begin(), _merged.end(), [](const pair< double, double > & a, const pair< double, double > & b) { return a.first <= b.first; });   // sort the weighted merged sample by value (first), weight  = second
        array< double, MAXNBINS > _boundaries = GetBoundariesForPlot(_merged, nBins, fullRange.first, fullRange.second);
        //--- let's plot some stuff
        vector< TCut > _binCut = getVectorCut(_boundaries.data(), _isoBinVariable, nBins, fullRange.first, fullRange.second);
        WriteCSVfile(_binCut, _ID);
        // Make some histogam
        if (IOSvc::ExistFile(_ID + "_" + checkCanvas)) { MessageSvc::Error("File Exists ", _ID + "_" + checkCanvas, "EXIT_FAILURE"); }
        if (IOSvc::ExistFile(_ID + "_" + outRootFile)) { MessageSvc::Error("File Exists ", _ID + "_" + outRootFile, "EXIT_FAILURE"); }
        TFile * f = new TFile(_ID + "_" + outRootFile, "RECREATE");

        TH1D hEE("hist1DEE", "hist1DEE", nBins, _boundaries.data());
        TH1D hMM("hist1DMM", "hist1DMM", nBins, _boundaries.data());

        TH1D hEERaw("hist1DEE_Raw", "hist1DEE_Raw",       200, fullRange.first, fullRange.second);
        TH1D hMMRaw("hist1DMM_Raw", "hist1DMM_Raw",       200, fullRange.first, fullRange.second);
        TH1D hTemplateRaw("template_Raw", "template_Raw", 200, fullRange.first, fullRange.second);

        TH1D hTemplate("template", "template", nBins, _boundaries.data());
        hTemplate.GetXaxis()->SetTitle(labelAxis);
        hEE.GetXaxis()->SetTitle(labelAxis);
        hMM.GetXaxis()->SetTitle(labelAxis);
        hTemplateRaw.GetXaxis()->SetTitle( labelAxis );
        for (auto & el : _eeValues) {
            hEERaw.Fill(el);
            hEE.Fill(el);
        }
        for (auto & el : _mmValues) {
            hMMRaw.Fill(el);
            hMM.Fill(el);
        }
        TCanvas * cc = new TCanvas();
        cc->Divide(3, 2);

        auto SetColor = [](TH1D & h1D, unsigned int color) {
            h1D.SetLineColor(color);
            h1D.SetMarkerColor(color);
            return;
        };
        cc->cd(1);
        SetColor(hEE, kBlue);
        hEE.Draw();
        cc->cd(4);
        SetColor(hEERaw, kBlue);
        hEERaw.Draw();
        cc->cd(2);
        SetColor(hMM, kRed);
        hMM.Draw();
        cc->cd(5);
        SetColor(hMMRaw, kRed);
        hMMRaw.Draw();
        hTemplate.Draw("same");
        cc->cd(3);
        hEERaw.Scale(1. / hEERaw.GetEntries());
        hMMRaw.Scale(1. / hMMRaw.GetEntries());
        if (hEERaw.GetMaximum() < hMMRaw.GetMaximum()) {
            hMMRaw.Draw();
            hEERaw.Draw("same");
            hTemplate.Draw("same");
        } else {
            hEERaw.Draw();
            hMMRaw.Draw("same");
            hTemplate.Draw("same");
        }
        cc->cd(6);
        hEE.Scale(1. / hEE.GetEntries());
        hMM.Scale(1. / hMM.GetEntries());
        if (hEE.GetMaximum() < hMM.GetMaximum()) {
            hMM.SetMinimum(0);
            hMM.Draw();
            hEE.Draw("same");
        } else {
            hEE.SetMinimum(0);
            hEE.Draw();
            hMM.Draw("same");
        }
        cc->SaveAs(_ID + "_" + checkCanvas);
        cc->Write("canvas", TObject::kOverwrite);
        hEERaw.Write("hist1DEE_Raw", TObject::kOverwrite);
        hMMRaw.Write("hist1DMM_Raw", TObject::kOverwrite);
        hEE.Write("hist1DEE", TObject::kOverwrite);
        hMM.Write("hist1DMM", TObject::kOverwrite);
        hTemplate.Write("template", TObject::kOverwrite);
        hTemplateRaw.Write("template_Raw", TObject::kOverwrite);
        f->Close();
    }
    return;
};


void BuildBinningScheme2D(EventType & _eTypeEE, EventType & _eTypeMM, const IsoBinningInputs & _isoBinInput, bool OnlyMuon , bool OnlyElectron ){
    // void BuildBinningScheme2D(EventType & _eTypeEE, EventType & _eTypeMM, pair< TString, int > varX, pair< TString, int > varY, TString _ID, bool onlyMuon, bool onlyElectron) {
    /*
        vector< TString > variables = {};
        vector< int >     nBins     = {};
        vector< pair< double, double> > MinMax;
        vector< TString > labelAxis = {};
        TString ID        = "ERROR";
        TString HistoType = "ERROR";
    */    
    /* x -axis related */

   if( OnlyMuon == true && OnlyElectron == true){
     MessageSvc::Error("Invalid","","EXIT_FAILURE");
   }
    TString x_var_name          = _isoBinInput.variables.at(0);
    pair<double,double> x_range = _isoBinInput.MinMax.at(0);
    int x_nBins                 = _isoBinInput.nBins.at(0);
    TString x_label            = _isoBinInput.labelAxis.at(0);
    /* y -axis related */
    TString y_var_name          = _isoBinInput.variables.at(1);
    pair<double,double> y_range = _isoBinInput.MinMax.at(1);
    int y_nBins                 = _isoBinInput.nBins.at(1);
    TString y_label            = _isoBinInput.labelAxis.at(1);        
    TString _ID = _isoBinInput.ID;
    TString _HistType = _isoBinInput.HistoType;
    auto processIt2D = [](RXDataPlayer & dp, EventType & et, TString & varX, TString & varY){
        auto BMass = et.GetParticleNames()["HEAD"] + "_DTF_JPs_M";
        dp.BookVariable(BMass, "m(B)", 100, 4800, 5600, "MeV", false, false);   // delegate bookkeping
        dp.BookVariable(varX, varX, 100, std::numeric_limits< double >::min(), std::numeric_limits< double >::max(), "", false, false);
        dp.BookVariable(varY, varY, 100, std::numeric_limits< double >::min(), std::numeric_limits< double >::max(), "", false, false);
        dp.BookWeight("1.", "noW");
        auto alias = GetAllAliases(et.GetTuple());
        dp.AddAliases(alias);
        dp.SetBaseCut(TString(et.GetCut()));
        dp.Process(*et.GetTuple());
        return;
    };    
        // lambda creating the vector< x, y> values
    auto Make2D = [](vector< double > && x, vector< double > && y) {
        if (x.size() != y.size()) { MessageSvc::Error("Cannot bind x,y", "", "EXIT_FAILURE"); }
        vector< Var2D > _returnVec;
        for (int i = 0; i < x.size(); ++i) { _returnVec.push_back(Var2D(x[i], y[i])); }
        return _returnVec;
    };    
    auto Make2D_v1 = [](vector< double > & x, vector< double > & y) {
        if (x.size() != y.size()) { MessageSvc::Error("Cannot bind x,y", "", "EXIT_FAILURE"); }
        vector< Var2D > _returnVec;
        for (int i = 0; i < x.size(); ++i) { _returnVec.push_back(Var2D(x[i], y[i])); }
        return _returnVec;
    };        
    if( _HistType == "TH2D"){
        pair< TString, int> varX = make_pair(x_var_name, x_nBins);
        pair< TString, int> varY = make_pair(y_var_name, y_nBins);
        RXDataPlayer dpEE("EE");
        RXDataPlayer dpMM("MM");
	if( OnlyMuon == true && OnlyElectron == false){
	  processIt2D(dpEE, _eTypeMM,  varX.first, varY.first);        
	  processIt2D(dpMM, _eTypeMM,  varX.first, varY.first);        
	}else if( OnlyMuon == false && OnlyElectron == true){
	  processIt2D(dpEE, _eTypeEE,  varX.first, varY.first);        
	  processIt2D(dpMM, _eTypeEE,  varX.first, varY.first);        
	}else if( OnlyMuon == false && OnlyElectron ==false){
	  processIt2D(dpEE, _eTypeEE,  varX.first, varY.first);        
	  processIt2D(dpMM, _eTypeMM,  varX.first, varY.first);        
	}else{
	  MessageSvc::Error("Invalid switch","","EXIT_FAILURE");
	}
        vector< double > _eeValues_X = dpEE.GetVariableValues(x_var_name);
        vector< double > _mmValues_X = dpMM.GetVariableValues(x_var_name);
        vector< double > _eeValues_Y = dpEE.GetVariableValues(y_var_name);
        vector< double > _mmValues_Y = dpMM.GetVariableValues(y_var_name);

        vector< Var2D > _eeValues = Make2D_v1(_eeValues_X, _eeValues_Y);
        vector< Var2D > _mmValues = Make2D_v1(_mmValues_X, _mmValues_Y);

        std::stable_sort(_eeValues_X.begin(), _eeValues_X.end(), [](double a, double b) { return a <= b; });
        std::stable_sort(_mmValues_X.begin(), _mmValues_X.end(), [](double a, double b) { return a <= b; });
        std::stable_sort(_eeValues_Y.begin(), _eeValues_Y.end(), [](double a, double b) { return a <= b; });
        std::stable_sort(_mmValues_Y.begin(), _mmValues_Y.end(), [](double a, double b) { return a <= b; });
        std::pair< double, double >      commonRangeX        = make_pair(std::max(_eeValues_X.front(), _mmValues_X.front()), std::min(_eeValues_X.back(), _mmValues_X.back()));
        std::pair< double, double >      fullRangeX          = make_pair(std::min(_eeValues_X.front(), _mmValues_X.front()), std::max(_eeValues_X.back(), _mmValues_X.back()));
        std::pair< double, double >      commonRangeY        = make_pair(std::max(_eeValues_Y.front(), _mmValues_Y.front()), std::min(_eeValues_Y.back(), _mmValues_Y.back()));
        std::pair< double, double >      fullRangeY          = make_pair(std::min(_eeValues_Y.front(), _mmValues_Y.front()), std::max(_eeValues_Y.back(), _mmValues_Y.back()));
        commonRangeX.first = x_range.first; commonRangeX.second = x_range.second;
        fullRangeX.first   = x_range.first; fullRangeX.second   = x_range.second;
        commonRangeY.first = y_range.first; commonRangeY.second = y_range.second;
        fullRangeY.first   = y_range.first; fullRangeY.second   = y_range.second;

        // x - iso binning
        vector< double >                 X_ranged_EE          = filterVector(_eeValues_X, commonRangeX);
        vector< double >                 X_ranged_MM          = filterVector(_mmValues_X, commonRangeX);

        vector< double >                 Y_ranged_EE          = filterVector(_eeValues_Y, commonRangeY);
        vector< double >                 Y_ranged_MM          = filterVector(_mmValues_Y, commonRangeY);

        auto                             X_weightedEntries_EE = ScaleAndWeightSorted(X_ranged_EE);
        auto                             X_weightedEntries_MM = ScaleAndWeightSorted(X_ranged_MM);

        auto                             Y_weightedEntries_EE = ScaleAndWeightSorted(Y_ranged_EE);
        auto                             Y_weightedEntries_MM = ScaleAndWeightSorted(Y_ranged_MM);

        vector< pair< double, double > > _merged_X;
        vector< pair< double, double > > _merged_Y;
        for (auto & el : X_weightedEntries_MM) { _merged_X.push_back(el); }
        for (auto & el : X_weightedEntries_EE) { _merged_X.push_back(el); }
        for (auto & el : Y_weightedEntries_MM) { _merged_Y.push_back(el); }
        for (auto & el : Y_weightedEntries_EE) { _merged_Y.push_back(el); }        
        std::stable_sort(_merged_X.begin(), _merged_X.end(), [](const pair< double, double > & a, const pair< double, double > & b) { return a.first <= b.first; });   // sort the weighted merged sample by value (first), weight  = second
        std::stable_sort(_merged_Y.begin(), _merged_Y.end(), [](const pair< double, double > & a, const pair< double, double > & b) { return a.first <= b.first; });   // sort the weighted merged sample by value (first), weight  = second

        array< double, MAXNBINS > _boundaries_X = GetBoundariesForPlot(_merged_X, x_nBins, fullRangeX.first, fullRangeX.second);
        array< double, MAXNBINS > _boundaries_Y = GetBoundariesForPlot(_merged_Y, y_nBins, fullRangeY.first, fullRangeY.second);
        
        //--- let's plot some stuff
        vector<TCut> _allBins;
        vector< TCut > _binCut_X = getVectorCut(_boundaries_X.data(), x_var_name, x_nBins, fullRangeX.first, fullRangeX.second);
        vector< TCut > _binCut_Y = getVectorCut(_boundaries_Y.data(), y_var_name, y_nBins, fullRangeY.first, fullRangeY.second);
        int bIdx =0;
        for( auto & xCut : _binCut_X){
            for(auto & yCut : _binCut_Y){
                _allBins.push_back( xCut && yCut);
                cout<< " bin "<< bIdx << " cut : "<< _allBins.back() << endl;
            }
        }
        WriteCSVfile(_allBins, _ID);
        if (IOSvc::ExistFile(_ID + "_" + checkCanvas)) { MessageSvc::Error("File Exists ", _ID + "_" + checkCanvas, "EXIT_FAILURE"); }
        if (IOSvc::ExistFile(_ID + "_" + outRootFile)) { MessageSvc::Error("File Exists ", _ID + "_" + outRootFile, "EXIT_FAILURE"); }

        TH2D h2PolyEE("hist2DEE", (TString)"hist2DEE;"+x_label+";"+y_label, x_nBins,  _boundaries_X.data() , y_nBins, _boundaries_Y.data() );
        TH2D h2PolyMM("hist2DMM", (TString)"hist2DMM;"+x_label+";"+y_label, x_nBins,  _boundaries_X.data(),  y_nBins, _boundaries_Y.data() );
        TH2D hTemplate("template", (TString)"template;"+x_label+";"+y_label, x_nBins, _boundaries_X.data(), y_nBins, _boundaries_Y.data() );

        TH2D h2RawEE("hist2DEE_Raw", (TString)"hist2DEE_Raw;"+x_label+";"+y_label, 50, fullRangeX.first, fullRangeX.second, 50, fullRangeY.first, fullRangeY.second);
        TH2D h2RawMM("hist2DMM_Raw", (TString)"hist2DMM_Raw;"+x_label+";"+y_label, 50, fullRangeX.first, fullRangeX.second, 50, fullRangeY.first, fullRangeY.second);
        TH2D h2RawTemplate("template_Raw", (TString)"template_Raw;"+x_label+";"+y_label, 50, fullRangeX.first, fullRangeX.second, 50, fullRangeY.first, fullRangeY.second);
    
        for (auto & el : _eeValues) {
            h2PolyEE.Fill(el.x, el.Y());
            h2RawEE.Fill(el.x, el.Y());
        }
        for (auto & el : _mmValues) {
            h2PolyMM.Fill(el.x, el.Y());
            h2RawMM.Fill(el.x, el.Y());
        }
        if (IOSvc::ExistFile(_ID + "_" + checkCanvas)) { MessageSvc::Error("File Exists ", _ID + "_" + checkCanvas, "EXIT_FAILURE"); }
        if (IOSvc::ExistFile(_ID + "_" + outRootFile)) { MessageSvc::Error("File Exists ", _ID + "_" + outRootFile, "EXIT_FAILURE"); }
        TFile file(_ID + "_" + outRootFile, "RECREATE");
        // 2x2 canvas [1 = EE poly, 2 = MM poly, 3 = EE Raw 2D, 4 = MM Raw 2D]
        TCanvas c1("canvas", "canvas", 800, 800);
        c1.Divide(3, 2);
        c1.cd(1);
        h2PolyEE.Draw("COLZ");
        c1.cd(2);
        h2PolyMM.Draw("COLZ");
        c1.cd(3);
        h2RawEE.Draw("colz");
        hTemplate.Draw("same");
        c1.cd(4);
        h2RawMM.Draw("colz");
        hTemplate.Draw("same");

        h2PolyMM.GetXaxis()->SetTitle(x_label);
        h2PolyMM.GetYaxis()->SetTitle(y_label);

        h2PolyEE.GetXaxis()->SetTitle(x_label);
        h2PolyEE.GetYaxis()->SetTitle(y_label);

        h2RawMM.GetXaxis()->SetTitle(x_label);
        h2RawMM.GetYaxis()->SetTitle(y_label);

        h2RawEE.GetXaxis()->SetTitle(x_label);
        h2RawEE.GetYaxis()->SetTitle(y_label);

        hTemplate.GetXaxis()->SetTitle(x_label);
        hTemplate.GetYaxis()->SetTitle(y_label);

        h2RawTemplate.GetXaxis()->SetTitle(x_label);
        h2RawTemplate.GetYaxis()->SetTitle(y_label);

        c1.SaveAs(_ID + "_" + checkCanvas);
        //---- write Histos to disk (for later usage [template is the relevant one])
        h2PolyEE.Write("hist2DEE", TObject::kOverwrite);
        h2PolyMM.Write("hist2DMM", TObject::kOverwrite);
        h2RawEE.Write("hist2DEE_Raw", TObject::kOverwrite);
        h2RawMM.Write("hist2DMM_Raw", TObject::kOverwrite);

        hTemplate.Write("template", TObject::kOverwrite);
        h2RawTemplate.Write("template_Raw", TObject::kOverwrite);
        c1.Write("canvas", TObject::kOverwrite);
        file.Close();
    }
    if( _HistType == "TH2Poly"){
        pair< TString, int> varX = make_pair(x_var_name, x_nBins);
        pair< TString, int> varY = make_pair(y_var_name, y_nBins);
        
        // Make the dataPlayer
        RXDataPlayer dpEE("EE");
        RXDataPlayer dpMM("MM");
        // Load the varX and varY in processing...
        if( OnlyMuon == true && OnlyElectron == false){
            processIt2D(dpEE, _eTypeMM,  varX.first, varY.first);        
            processIt2D(dpMM, _eTypeMM,  varX.first, varY.first);        
        }else if( OnlyMuon == false && OnlyElectron == true){
            processIt2D(dpEE, _eTypeEE,  varX.first, varY.first);        
            processIt2D(dpMM, _eTypeEE,  varX.first, varY.first);        
        }else if( OnlyMuon == false && OnlyElectron ==false){
            processIt2D(dpEE, _eTypeEE,  varX.first, varY.first);        
            processIt2D(dpMM, _eTypeMM,  varX.first, varY.first);        
        }else{
            MessageSvc::Error("Invalid switch","","EXIT_FAILURE");
        }
        vector< Var2D > _eeValues = Make2D(dpEE.GetVariableValues(varX.first), dpEE.GetVariableValues(varY.first));
        vector< Var2D > _mmValues = Make2D(dpMM.GetVariableValues(varX.first), dpMM.GetVariableValues(varY.first));
        // Sorty by Y ( first isobinning) and by X to find the min/max [common Range, full Range]
        std::stable_sort(_eeValues.begin(), _eeValues.end(), [](const Var2D & a, const Var2D & b) { return a.Y() <= b.Y(); });
        std::stable_sort(_mmValues.begin(), _mmValues.end(), [](const Var2D & a, const Var2D & b) { return a.Y() <= b.Y(); });
        std::pair< double, double > commonRangeY = make_pair(std::max(_eeValues.front().Y(), _mmValues.front().Y()), std::min(_eeValues.back().Y(), _mmValues.back().Y()));
        std::pair< double, double > fullRangeY   = make_pair(std::min(_eeValues.front().Y(), _mmValues.front().Y()), std::max(_eeValues.back().Y(), _mmValues.back().Y()));
        std::stable_sort(_eeValues.begin(), _eeValues.end(), [](const Var2D & a, const Var2D & b) { return a.x <= b.x; });
        std::stable_sort(_mmValues.begin(), _mmValues.end(), [](const Var2D & a, const Var2D & b) { return a.x <= b.x; });
        std::pair< double, double > commonRangeX = make_pair(std::max(_eeValues.front().x, _mmValues.front().x), std::min(_eeValues.back().x, _mmValues.back().x));
        std::pair< double, double > fullRangeX   = make_pair(std::min(_eeValues.front().x, _mmValues.front().x), std::max(_eeValues.back().x, _mmValues.back().x));
        /* Bypass everything , force ranges */
        commonRangeX.first = x_range.first;
        fullRangeX.first   = x_range.first;
        commonRangeX.second = x_range.second;
        fullRangeX.second   = x_range.second;
        commonRangeY.first = y_range.first;
        fullRangeY.first   = y_range.first;
        commonRangeY.second = y_range.second;
        fullRangeY.second   = y_range.second;

        // Bin in X [ reduce range on vector XY]
        auto ranged_EE = filterVectorX(_eeValues, commonRangeX);
        auto ranged_MM = filterVectorX(_mmValues, commonRangeX);
        // Weight and scale it!
        auto weightedEntries_EE = ScaleAndWeightSorted(ranged_EE);
        auto weightedEntries_MM = ScaleAndWeightSorted(ranged_MM);
        // Merge EE and MM after weighting
        vector< pair< Var2D, double > > _merged;
        for (auto & el : weightedEntries_MM) { _merged.push_back(make_pair(Var2D(el.first.x, el.first.Y()), el.second)); }
        for (auto & el : weightedEntries_EE) { _merged.push_back(make_pair(Var2D(el.first.x, el.first.Y()), el.second)); }
        // Sort it by X
        std::sort(_merged.begin(), _merged.end(), [](const pair< Var2D, double > & a, const pair< Var2D, double > & b) { return a.first.x < b.first.x; });   // sort the weighted merged sample by value (first), weight  = second
        // extract plain vector of x-values (auto-sorted)
        vector< pair< double, double > > xvars;
        for (auto & el : _merged) { xvars.push_back(make_pair(el.first.x, el.second)); }
        // Get boundaries of Iso-Bins
        array< double, MAXNBINS > _boundaries = GetBoundariesForPlot(xvars, varX.second, fullRangeX.first, fullRangeX.second);
        // Get the TCuts associated to the x-bins boxes
        _boundaries[0]           = fullRangeX.first;
        _boundaries[varX.second] = fullRangeX.second;
        vector< TCut > _binCut   = getVectorCut(_boundaries.data(), varX.first, varX.second, fullRangeX.first, fullRangeX.second);
        vector< TCut > _allBins = {};
        // let's slice things in Y for each bin
        vector< pair< pair< double, double >, pair< double, double > > > bin_X1X2_Y1Y2;
        for (int _binXIDX = 0; _binXIDX < varX.second; ++_binXIDX) {
            double minX = _binXIDX == 0 ? fullRangeX.first : _boundaries[_binXIDX];
            double maxX = _binXIDX == varX.second - 1 ? fullRangeX.second : _boundaries[_binXIDX + 1];
            // get EE / MM in this X-bin flat y values
            vector< double > _eeValuesYBin;
            for (auto & el : _eeValues) {
                if ((el.X() < minX) || (el.X() >= maxX)) continue;
                _eeValuesYBin.push_back(el.Y());
            }
            vector< double > _mmValuesYBin;
            for (auto & el : _mmValues) {
                if ((el.X() < minX) || (el.X() >= maxX)) continue;
                _mmValuesYBin.push_back(el.Y());
            }
            // Sort by Y
            std::sort(_eeValuesYBin.begin(), _eeValuesYBin.end(), [](double a, double b) { return a < b; });
            std::sort(_mmValuesYBin.begin(), _mmValuesYBin.end(), [](double a, double b) { return a < b; });
            // Find commonRange in Y between EE/MM in this x-bin
            std::pair< double, double > commonRange = make_pair(std::max(_eeValuesYBin.front(), _mmValuesYBin.front()), std::min(_eeValuesYBin.back(), _mmValuesYBin.back()));
            // Find fullRange in Y between EE/MM in this x-bin
            std::pair< double, double > fullRange = make_pair(std::min(_eeValuesYBin.front(), _mmValuesYBin.front()), std::max(_eeValuesYBin.back(), _mmValuesYBin.back()));
            // Filter out the things in the common range
            vector< double > ranged_EE_Y = filterVector(_eeValuesYBin, commonRange);
            vector< double > ranged_MM_Y = filterVector(_mmValuesYBin, commonRange);
            // Scale the 2 to a common normalization
            auto weightedEntries_EE_Y = ScaleAndWeightSorted(ranged_EE_Y);
            auto weightedEntries_MM_Y = ScaleAndWeightSorted(ranged_MM_Y);
            // Merge the "common-range" values
            vector< pair< double, double > > _mergedY;
            for (auto & el : weightedEntries_EE_Y) { _mergedY.push_back(el); }
            for (auto & el : weightedEntries_MM_Y) { _mergedY.push_back(el); }
            // Sort by y the common range
            std::sort(_mergedY.begin(), _mergedY.end(), [](const pair< double, double > & a, const pair< double, double > & b) { return a.first < b.first; });   // sort the weighted merged sample by value (first), weight  = second
            // Get the boundaries in this x-bin
            array< double, MAXNBINS > _boundariesY = GetBoundariesForPlot(_mergedY, varY.second, fullRangeY.first, fullRangeY.second);
            // Built this vector of cuts
            vector< TCut > _binCutY = getVectorCut(_boundariesY.data(), varY.first, varY.second, fullRangeY.first, fullRangeY.second);
            // binX (time) [ nBins in Y ]
            for (int _binYIDX = 0; _binYIDX < varY.second; ++_binYIDX) {
                _allBins.push_back(_binCut[_binXIDX] && _binCutY[_binYIDX]);
                double X1 = minX;
                double X2 = maxX;
                double Y1 = _binYIDX == 0 ? fullRangeY.first : _boundariesY[_binYIDX];
                double Y2 = _binYIDX == varY.second - 1 ? fullRangeY.second : _boundariesY[_binYIDX + 1];
                // Boundaries of bin
                bin_X1X2_Y1Y2.push_back(make_pair(make_pair(X1, X2), make_pair(Y1, Y2)));
            }
        }
        WriteCSVfile(_allBins, _ID);
        TH2Poly h2PolyEE("hist2DEE", "hist2DEE", varX.second, fullRangeX.first, fullRangeX.second, varY.second, fullRangeY.first, fullRangeY.second);
        TH2Poly h2PolyMM("hist2DMM", "hist2DMM", varX.second, fullRangeX.first, fullRangeX.second, varY.second, fullRangeY.first, fullRangeY.second);
        TH2Poly hTemplate("template", "template", varX.second, fullRangeX.first, fullRangeX.second, varY.second, fullRangeY.first, fullRangeY.second);

        TH2D h2RawEE("hist2DEE_Raw", "hist2DEE_Raw", 50, fullRangeX.first, fullRangeX.second, 50, fullRangeY.first, fullRangeY.second);
        TH2D h2RawMM("hist2DMM_Raw", "hist2DMM_Raw", 50, fullRangeX.first, fullRangeX.second, 50, fullRangeY.first, fullRangeY.second);
        TH2D h2RawTemplate("template_Raw", "template_Raw", 50, fullRangeX.first, fullRangeX.second, 50, fullRangeY.first, fullRangeY.second);

        for (auto & bin : bin_X1X2_Y1Y2) {
            double x1 = bin.first.first;
            double x2 = bin.first.second;
            double y1 = bin.second.first;
            double y2 = bin.second.second;
            std::cout << " (X1,X2),(Y1,Y2) =  [" << x1 << " , " << x2 << "] , [" << y1 << " , " << y2 << " ]" << std::endl;
            // add a box in TH2Poly
            h2PolyEE.AddBin(x1, y1, x2, y2);
            h2PolyMM.AddBin(x1, y1, x2, y2);
            hTemplate.AddBin(x1, y1, x2, y2);
        }
        for (auto & el : _eeValues) {
            h2PolyEE.Fill(el.x, el.Y());
            h2RawEE.Fill(el.x, el.Y());
        }
        for (auto & el : _mmValues) {
            h2PolyMM.Fill(el.x, el.Y());
            h2RawMM.Fill(el.x, el.Y());
        }
        if (IOSvc::ExistFile(_ID + "_" + checkCanvas)) { MessageSvc::Error("File Exists ", _ID + "_" + checkCanvas, "EXIT_FAILURE"); }
        if (IOSvc::ExistFile(_ID + "_" + outRootFile)) { MessageSvc::Error("File Exists ", _ID + "_" + outRootFile, "EXIT_FAILURE"); }
        TFile file(_ID + "_" + outRootFile, "RECREATE");
        // 2x2 canvas [1 = EE poly, 2 = MM poly, 3 = EE Raw 2D, 4 = MM Raw 2D]
        TCanvas c1("canvas", "canvas", 800, 800);
        c1.Divide(2, 2);
        c1.cd(1);
        h2PolyEE.Draw("COLZ");
        c1.cd(2);
        h2PolyMM.Draw("COLZ");
        c1.cd(3);
        h2RawEE.Draw("colz");
        hTemplate.Draw("same");
        c1.cd(4);
        h2RawMM.Draw("colz");
        hTemplate.Draw("same");
        h2PolyMM.GetXaxis()->SetTitle(x_label);
        h2PolyMM.GetYaxis()->SetTitle(y_label);
        h2PolyEE.GetXaxis()->SetTitle(x_label);
        h2PolyEE.GetYaxis()->SetTitle(y_label);
        h2RawMM.GetXaxis()->SetTitle(x_label);
        h2RawMM.GetYaxis()->SetTitle(y_label);
        h2RawEE.GetXaxis()->SetTitle(x_label);
        h2RawEE.GetYaxis()->SetTitle(y_label);
        hTemplate.GetXaxis()->SetTitle(x_label);
        hTemplate.GetYaxis()->SetTitle(y_label);
        h2RawTemplate.GetXaxis()->SetTitle(x_label);
        h2RawTemplate.GetYaxis()->SetTitle(y_label);
        c1.SaveAs(_ID + "_" + checkCanvas);
        //---- write Histos to disk (for later usage [template is the relevant one])
        h2PolyEE.Write("hist2DEE", TObject::kOverwrite);
        h2PolyMM.Write("hist2DMM", TObject::kOverwrite);
        h2RawEE.Write("hist2DEE_Raw", TObject::kOverwrite);
        h2RawMM.Write("hist2DMM_Raw", TObject::kOverwrite);
        hTemplate.Write("template", TObject::kOverwrite);
        h2RawTemplate.Write("template_Raw", TObject::kOverwrite);
        c1.Write("canvas", TObject::kOverwrite);
        file.Close();
    }
};
#endif
