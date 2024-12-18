#ifndef RXDATAPLAYER_CPP
#define RXDATAPLAYER_CPP

#include "RXDataPlayer.hpp"

#include "HelperSvc.hpp"
#include "HistogramSvc.hpp"
#include "IOSvc.hpp"

#include "boost/program_options.hpp"
#include "core.h"
#include "vec_extends.h"
#include "zipfor.h"
#include <algorithm>
//#include <boost/progress.hpp>
#include <boost/timer/timer.hpp>
#include <iostream>
#include <numeric>
#include <vector>

#include "TApplication.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TH2.h"
#include "TLegend.h"
#include "TProfile.h"
#include "TRatioPlot.h"
#include "TTreeFormula.h"

#include <ROOT/RDataFrame.hxx>

RXDataPlayer::RXDataPlayer() {
    if (SettingDef::debug.Contains("DP")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("RXDataPlayer", (TString) "Default");
    MessageSvc::Line();
    MessageSvc::Info("RXDataPlayer", m_name);
    MessageSvc::Line();
}

RXDataPlayer::RXDataPlayer(TString _name) {
    if (SettingDef::debug.Contains("DP")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("RXDataPlayer", (TString) "TString");
    m_name = _name;
    MessageSvc::Line();
    MessageSvc::Info("RXDataPlayer", m_name);
    MessageSvc::Line();
}

void RXDataPlayer::BookVariable(TString _expression, TString _label, int _nBins, double _min, double _max, TString _units, bool _isoBin, bool _useLogY) {
    MessageSvc::Info("BookVariable", _expression);
    m_bookedVariable[m_nVars] = RXVarPlot(m_nVars, _expression, _label, _nBins, _min, _max, _units, _isoBin, _useLogY);
    m_nVars++;
    return;
}

void RXDataPlayer::BookVariable(const RooRealVar & _var) {
    TString _expression = _var.GetName();
    TString _label      = _var.GetTitle();
    TString _units      = TString(_var.getUnit());
    int     _nBins      = _var.numBins();
    double  _min        = _var.getMin();
    double  _max        = _var.getMax();
    BookVariable(_expression, _label, _nBins, _min, _max, _units, false, false);   // delegate bookkeping
    return;
}

void RXDataPlayer::BookVariable(RXVarPlot _var) {
    TString _expression = _var.Expression();
    TString _label      = _var.Label();
    TString _units      = _var.Units();
    int     _nBins      = _var.NBins();
    double  _min        = _var.Min();
    double  _max        = _var.Max();
    BookVariable(_expression, _label, _nBins, _min, _max, _units, false, false);
    return;
}

void RXDataPlayer::BookVariables(TString _filaName) {
    MessageSvc::Line();
    MessageSvc::Info("BookVariables", _filaName);
    vector< vector< TString > > _variables = IOSvc::ParseFile(_filaName, ",");
    BookVariables(_variables);
    MessageSvc::Line();
    return;
}

void RXDataPlayer::BookVariables(vector< vector< TString > > _variables) {
    MessageSvc::Info("BookVariables", to_string(_variables.size()));
    for (auto & _variable : _variables) {
        TString _expression = _variable[0];
        TString _label      = _variable[1];
        TString _units      = _variable[2];
        int     _nBins      = _variable[3].Atoi();
        double  _min        = _variable[4].Atof();
        double  _max        = _variable[5].Atof();
        BookVariable(_expression, _label, _nBins, _min, _max, _units, false, false);   // delegate bookkeping
    }
    return;
}

void RXDataPlayer::BookSelection(TCut _selection, TString _label) {
    MessageSvc::Info("BookSelection", _label);
    m_bookedSelections[m_nSelections] = RXSelection(m_nSelections, _selection, _label);
    m_nSelections++;
    return;
}

void RXDataPlayer::BookWeight(TString _weight, TString _label) {
    MessageSvc::Info("BookWeight", _label);
    m_bookedWeights[m_nWeights] = RXWeight(m_nWeights, _weight, _label);
    m_nWeights++;
    return;
}

vector< tuple< RXSelection, RXWeight, RXVarPlot > > RXDataPlayer::BuildCombination() {
    // map< int, tuple<RXSelection, RXWeight, RXVarPlot  >  > RXDataPlayer::BuildCombination(){
    vector< tuple< RXSelection, RXWeight, RXVarPlot > > _final;
    if (GetNSelections() * GetNWeights() * GetNVariables() != 0) {
        // const map<int, RXSelection> & _RXSelections, const map<int, RXWeight>    & _RXWeights, const map<int, RXVarPlot> & _Variables){
        for (const auto & _vv : m_bookedVariable) {
            for (const auto & _ww : m_bookedWeights) {
                for (const auto & _cc : m_bookedSelections) { _final.push_back(make_tuple(_cc.second, _ww.second, _vv.second)); }
            }
        }
        return _final;
    } else {
        MessageSvc::Error("BuildCombination", (TString) "ILLEGAL PROCEDURE: You must book at least 1 Selection, 1 Variable, 1 Weight", "EXIT_FAILURE");
    }
    return _final;
}

void RXDataPlayer::Process(TChain & _tuple) noexcept {
    MessageSvc::Line();
    MessageSvc::Info("RXDataPlayer", m_name);
    MessageSvc::Info("Processing", (TString) _tuple.GetName());

    EnableMultiThreads();
    auto tStart = chrono::high_resolution_clock::now();

    Long64_t _entries = _tuple.GetEntries();
    MessageSvc::Info("Entries", to_string(_entries));

    double _frac = SettingDef::Tuple::frac;
    // MessageSvc::Warning("Frac", to_string(_frac));

    Long64_t _maxEntries = _entries;
    if ((_frac >= 0.f) && (_frac < 1.0f)) _maxEntries = (int) floor(_frac * _maxEntries);
    if (_frac >= 1) _maxEntries = (int) floor(_frac);
    /*
    if (_maxEntries < _entries) {
        MessageSvc::Info("Range", to_string(_maxEntries));
        MessageSvc::Info("RXDataPlayer", (TString) "ROOT::DisableImplicitMT");
        ROOT::DisableImplicitMT();
    }
    */

    ROOT::RDataFrame df(_tuple);

    auto firstDF = df.Define("DUMMY", " 1>0 ?  1. :0.");
    // auto firstDF = df.Range(_maxEntries).Define("DUMMY", " 1>0 ?  1. :0.");

    auto aliases  = GetAllAlias(static_cast< TTree * >(&_tuple));
    using DFTypeA = decltype(firstDF);
    // Add all aliases before cutting etc... [ if there is an alias named for example MinPT_L , the column exists and can be "used" when bookkeped and properly mapped in the "expression Cut"]
    auto aliasedDF = make_unique< DFTypeA >(firstDF);
    if (aliases.size() != 0) { MessageSvc::Info("RXDataPlayer", (TString) "Define columns for all aliases (nAliases = ", to_string(aliases.size())); }
    for (auto & aa : aliases) {
        // MessageSvc::Debug("RXDataPlayer", (TString)"Define "+aa.first+" , "+aa.second, "");
        aliasedDF = make_unique< DFTypeA >(aliasedDF->Define(aa.first.Data(), aa.second.Data()));
    }
    // Apply a filter with cutUpFront if there is a cut shared to all ( and if set, default = take all )
    if (m_cutUpFront != "(1>0)") MessageSvc::Info("RXDataPlayer", (TString) "Apply base cut (after aliases added): ", m_cutUpFront);
    auto filteredDF = aliasedDF->Filter(m_cutUpFront.Data());

    using DFType  = decltype(filteredDF);
    auto latestDF = make_unique< DFType >(filteredDF);

    MessageSvc::Info("Book", (TString) "Variable(s)", to_string(m_bookedVariable.size()));
    for (auto & el : m_bookedVariable) {
        TString vv   = fmt::format("Observable{0}", el.first);
        TString expr = fmt::format("(double){0}", el.second.Expression());
        latestDF     = make_unique< DFType >(latestDF->Define(vv.Data(), expr.Data()));
    }

    MessageSvc::Info("Book", (TString) "Selection(s)", to_string(m_bookedSelections.size()));
    for (auto & el : m_bookedSelections) {
        TString selection = TString(el.second.Expression()) + " ? 1.5 : -1.5";   // passed cut = > 0, failed cut < 0
        TString vv        = fmt::format("Selection{0}", el.first);
        latestDF          = make_unique< DFType >(latestDF->Define(vv.Data(), selection.Data()));
    }

    MessageSvc::Info("Book", (TString) "Weight(s)", to_string(m_bookedWeights.size()));
    for (auto & el : m_bookedWeights) {
        TString vv = fmt::format("Weight{0}", el.first);
        latestDF   = make_unique< DFType >(latestDF->Define(vv.Data(), el.second.Expression().Data()));
    }

    auto dummy  = latestDF->Take< double >("DUMMY");
    using DTYPE = decltype(dummy);
    map< RXSelection, DTYPE > _selectionValues;
    map< RXWeight, DTYPE >    _weightValues;
    map< RXVarPlot, DTYPE >   _variableValues;
    for (auto & el : m_bookedSelections) {
        TString vv                  = fmt::format("Selection{0}", el.first);
        _selectionValues[el.second] = latestDF->Take< double >(vv.Data());
    }
    for (auto & el : m_bookedWeights) {
        TString vv               = fmt::format("Weight{0}", el.first);
        _weightValues[el.second] = latestDF->Take< double >(vv.Data());
    }
    for (auto & el : m_bookedVariable) {
        TString vv                 = fmt::format("Observable{0}", el.first);
        _variableValues[el.second] = latestDF->Take< double >(vv.Data());
    }

    MessageSvc::Info("Processing", (TString) "");
    auto size = dummy->size();
    MessageSvc::Info("Processed", (TString) "Events =", to_string(dummy->size()));
    // cout<< "LOADED ENTRIES = "<< size << endl;  //this trigger all parallel Take we booked

    MessageSvc::Info("Filling", (TString) "");   // this trigger all parallel Take we booked
    for (auto & _results : m_bookedVariable) {
        m_bookedVariableValues[_results.second].reserve(size);
        m_bookedVariableValues[_results.second] = move(*_variableValues[_results.second]);
    }
    for (auto & _results : m_bookedWeights) {
        m_bookedWeightResults[_results.second].reserve(size);
        m_bookedWeightResults[_results.second] = move(*_weightValues[_results.second]);
    }
    for (auto & _results : m_bookedSelections) { m_bookedSelectionResults[_results.second].reserve(size); }
    // cout<<"Populating by element "<< endl;
    for (int i = 0; i < size; ++i) {
        for (auto & _results : m_bookedSelections) {
            // _results.first.Print();
            // cout<<  _selectionValues[_results.second]->at(i) << endl;
            if (_selectionValues[_results.second]->at(i) > 0) {              // Linked to selection 1.5 : -1.5 OF selection line
                m_bookedSelectionResults[_results.second].push_back(true);   // Selection passed
            } else {
                m_bookedSelectionResults[_results.second].push_back(false);   // Selection reject this entry
            }
        }
    }

    auto tEnd      = chrono::high_resolution_clock::now();
    auto timeTotal = chrono::duration_cast< chrono::seconds >(tEnd - tStart).count();
    auto timeEvent = (double) size / (double) timeTotal;
    MessageSvc::Line();
    MessageSvc::Warning("RXDataPlayer", m_name, "took", fmt::format("{0} seconds [{1} evts/s] booking {2} DataInformation", timeTotal, timeEvent, GetNSelections() * GetNWeights() * GetNVariables()), "");
    MessageSvc::Line();
    DisableMultiThreads();
    return;
}

TH1D * RXDataPlayer::GetHistogram1D(TString _name_var, TString _name_cut, TString _name_weight, EColor _color) {
    int _idx_var    = -1;
    int _idx_cut    = -1;
    int _idx_weight = -1;
    for (const auto & _var : m_bookedVariable) {
        if (_var.second.Expression() == _name_var) _idx_var = _var.first;
    }
    for (const auto & _cut : m_bookedSelections) {
        if (_cut.second.Label() == _name_cut) _idx_cut = _cut.first;
    }
    for (const auto & _weight : m_bookedWeights) {
        if (_weight.second.Label() == _name_weight) _idx_weight = _weight.first;
    }
    if (_idx_var == -1) MessageSvc::Error("GetHistogram1D", _name_var, "not existing Variable", "EXIT_FAILURE");
    if (_idx_cut == -1) MessageSvc::Error("GetHistogram1D", _name_cut, "not existing Selection", "EXIT_FAILURE");
    if (_idx_weight == -1) MessageSvc::Error("GetHistogram1D", _name_weight, "not existing Weight", "EXIT_FAILURE");

    return GetHistogram1D(_idx_var, _idx_cut, _idx_weight, _color);
}

TH1D * RXDataPlayer::GetHistogram1D(int _idx_var, int _idx_cut, int _idx_weight, EColor _color) {
    if (_idx_var >= GetNVariables()) MessageSvc::Error("GetHistogram1D", to_string(_idx_var), to_string(GetNVariables()), "not existing Variable", "EXIT_FAILURE");
    if (_idx_cut >= GetNSelections()) MessageSvc::Error("GetHistogram1D", to_string(_idx_cut), to_string(GetNSelections()), "not existing Selection", "EXIT_FAILURE");
    if (_idx_weight >= GetNWeights()) MessageSvc::Error("GetHistogram1D", to_string(_idx_weight), to_string(GetNWeights()), "not existing Weight", "EXIT_FAILURE");

    auto idx_combo = make_tuple(_idx_var, _idx_cut, _idx_weight);
    if (m_CachedHistoPool.find(idx_combo) != m_CachedHistoPool.end()) {
        MessageSvc::Info("Histogram already created, return cached one changing color");
        m_CachedHistoPool[idx_combo].SetLineColor(_color);
        return &m_CachedHistoPool[idx_combo];
    }

    auto _toFillValues     = m_bookedVariableValues[m_bookedVariable[_idx_var]];
    auto _toFillSelections = m_bookedSelectionResults[m_bookedSelections[_idx_cut]];
    auto _toFillWeights    = m_bookedWeightResults[m_bookedWeights[_idx_weight]];
    if (_toFillValues.size() != _toFillSelections.size() || _toFillValues.size() != _toFillWeights.size() || _toFillWeights.size() != _toFillSelections.size()) MessageSvc::Error("GetHistogram1D", (TString) "ERROR FILLING EVENT LOOP HAPPENED, FATAL ERROR", "EXIT_FAILURE");

    int    _nBins = m_bookedVariable[_idx_var].NBins();
    double _min   = m_bookedVariable[_idx_var].Min();
    double _max   = m_bookedVariable[_idx_var].Max();

    TString _nameVar    = m_bookedVariable[_idx_var].Expression();
    TString _nameCut    = m_bookedSelections[_idx_cut].Label();
    TString _nameWeight = m_bookedWeights[_idx_weight].Label();

    TString _nameHisto  = _nameVar + " [ " + _nameCut + "," + _nameWeight + " ]" + m_name;
    TString _titleHisto = _nameCut + _nameWeight + m_name;

    int _nUnderflow = 0;
    int _nOverFlow  = 0;

    // cout<<RED<< fmt::format("{0}, {1}, bins {2}, min {3}, max{4} ", _nameHisto, _titleHisto, _nBins, _min, _max) << endl;
    m_CachedHistoPool[idx_combo] = TH1D(_nameHisto, _nameHisto, _nBins, _min, _max);
    m_CachedHistoPool[idx_combo].Sumw2();

    for (int i = 0; i < _toFillValues.size(); ++i) {
        const double _value  = _toFillValues[i];
        const bool   _sel    = _toFillSelections[i];
        const double _weight = _toFillWeights[i];
        if (!_sel) continue;
        if (_value < _min) {
            _nUnderflow++;
            // continue;
        }
        if (_value > _max) {
            _nOverFlow++;
            // continue;
        }
        m_CachedHistoPool[idx_combo].Fill(_value, _weight);
    }

    MessageSvc::Line();
    MessageSvc::Info("GetHistogram1D", (TString) "Variable  =", m_bookedVariable[_idx_var].Expression());
    MessageSvc::Info("GetHistogram1D", (TString) "Selection =", m_bookedSelections[_idx_cut].Label());
    MessageSvc::Info("GetHistogram1D", (TString) "Entries   =", to_string(m_CachedHistoPool[idx_combo].GetEntries()));
    if (m_CachedHistoPool[idx_combo].Integral() != m_CachedHistoPool[idx_combo].GetEntries()) MessageSvc::Info("GetHistogram1D", (TString) "Integral  =", to_string(m_CachedHistoPool[idx_combo].Integral()));
    if (_nUnderflow > 0) MessageSvc::Warning("GetHistogram1D", (TString) "UnderFlow =", to_string(_nUnderflow), "below", to_string(_min));
    if (_nOverFlow > 0) MessageSvc::Warning("GetHistogram1D", (TString) "OverFlow  =", to_string(_nOverFlow), "above", to_string(_max));

    // Re-work axis labels here
    TString _xAxis = m_bookedVariable[_idx_var].Label();
    if (m_bookedVariable[_idx_var].Units() != "") { _xAxis += " [" + m_bookedVariable[_idx_var].Units() + "]"; }
    m_CachedHistoPool[idx_combo].GetXaxis()->SetTitle(_xAxis);
    TString _yAxis = Form("Counts/%.2f", abs(_max - _min) / m_bookedVariable[_idx_var].NBins());
    if (m_bookedVariable[_idx_var].Units() != "") { _yAxis += " [" + m_bookedVariable[_idx_var].Units() + "]"; }
    m_CachedHistoPool[idx_combo].GetYaxis()->SetTitle(_yAxis);

    m_CachedHistoPool[idx_combo].SetLineColor(_color);
    m_CachedHistoPool[idx_combo].SetMarkerColor(_color);

    return &m_CachedHistoPool[idx_combo];
}

TRatioPlot * RXDataPlayer::GetHistogramRatio1D(int _idx1_var, int _idx1_cut, int _idx1_weight, int _idx2_var, int _idx2_cut, int _idx2_weight, TString _option) {
    TH1D * _histo1 = (TH1D *) GetHistogram1D(_idx1_var, _idx1_cut, _idx1_weight, kBlack)->Clone();
    TH1D * _histo2 = (TH1D *) GetHistogram1D(_idx2_var, _idx2_cut, _idx2_weight, kBlue)->Clone();

    _histo1->SetName(TString(_histo1->GetName()) + " forRatio");
    _histo2->SetName(TString(_histo2->GetName()) + " forRatio");
    if (_option.Contains("scale")) {
        ScaleHistogram(*_histo1);
        ScaleHistogram(*_histo2);
    }
    TRatioPlot * _hratio = new TRatioPlot(_histo1, _histo2, _option);
    return _hratio;
}

void RXDataPlayer::SaveHistogram1D(TString _option, int _idx_cut, int _idx_weight) {
    TString _name = m_name;
    if (_option.Contains("s")) _name += "_Selections";
    if (_option.Contains("w")) _name += "_Weights";
    _name += fmt::format("_S{0}_W{1}", _idx_cut, _idx_weight);
    _name += "_TH1.pdf";

    EColor _color = kBlack;
    if (_option.Contains("s")) _color = (EColor) GetColorFromGradient(0, GetNSelections());
    if (_option.Contains("w")) _color = (EColor) GetColorFromGradient(0, GetNWeights());

    TString _draw = "hist";

    TCanvas _canvas("canvas");
    _canvas.SaveAs(_name + "[");
    for (int i = 0; i < GetNVariables(); ++i) {
        if (_option.Contains("n"))
            GetHistogram1D(i, _idx_cut, _idx_weight, _color)->DrawNormalized(_draw);
        else
            GetHistogram1D(i, _idx_cut, _idx_weight, _color)->Draw(_draw);

        if (_option.Contains("s")) {
            for (int j = 1; j < GetNSelections(); ++j) {
                if (_option.Contains("n"))
                    GetHistogram1D(i, j, _idx_weight, (EColor) GetColorFromGradient(j, GetNSelections()))->DrawNormalized(_draw + "same");
                else
                    GetHistogram1D(i, j, _idx_weight, (EColor) GetColorFromGradient(j, GetNSelections()))->Draw(_draw + "same");
            }
        } else if (_option.Contains("w")) {
            for (int j = 1; j < GetNWeights(); ++j) {
                if (_option.Contains("n"))
                    GetHistogram1D(i, _idx_cut, j, (EColor) GetColorFromGradient(j, GetNWeights()))->DrawNormalized(_draw + "same");
                else
                    GetHistogram1D(i, _idx_cut, j, (EColor) GetColorFromGradient(j, GetNWeights()))->Draw(_draw + "same");
            }
        }

        _canvas.BuildLegend()->Draw();
        _canvas.SaveAs(_name);
    }
    _canvas.SaveAs(_name + "]");

    return;
}

TH2D * RXDataPlayer::GetHistogram2D(pair< int, int > _idx_var, int _idx_cut, int _idx_weight, EColor _color) {
    int _idx_varX = _idx_var.first;
    int _idx_varY = _idx_var.second;

    tuple< pair< int, int >, int, int > idx_combo = make_tuple(_idx_var, _idx_cut, _idx_weight);
    if (m_CachedHisto2DPool.find(idx_combo) != m_CachedHisto2DPool.end()) { return &m_CachedHisto2DPool[idx_combo]; }
    if (_idx_varX >= GetNVariables()) MessageSvc::Error("GetHistogram2D", to_string(_idx_varX), to_string(GetNVariables()), "not existing Variable", "EXIT_FAILURE");
    if (_idx_varY >= GetNVariables()) MessageSvc::Error("GetHistogram2D", to_string(_idx_varY), to_string(GetNVariables()), "not existing Variable", "EXIT_FAILURE");
    if (_idx_cut >= GetNSelections()) MessageSvc::Error("GetHistogram2D", to_string(_idx_cut), to_string(GetNSelections()), "not existing Selection", "EXIT_FAILURE");
    if (_idx_weight >= GetNWeights()) MessageSvc::Error("GetHistogram2D", to_string(_idx_weight), to_string(GetNWeights()), "not existing Weight", "EXIT_FAILURE");

    // Retrieve Histogram from Var-IDX_X
    int     _nBinsX   = m_bookedVariable[_idx_varX].NBins();
    double  _minX     = m_bookedVariable[_idx_varX].Min();
    double  _maxX     = m_bookedVariable[_idx_varX].Max();
    TString _nameVarX = m_bookedVariable[_idx_varX].Expression();
    int     _nBinsY   = m_bookedVariable[_idx_varY].NBins();
    double  _minY     = m_bookedVariable[_idx_varY].Min();
    double  _maxY     = m_bookedVariable[_idx_varY].Max();
    TString _nameVarY = m_bookedVariable[_idx_varY].Expression();

    TString _nameCut    = m_bookedSelections[_idx_cut].Label();
    TString _nameWeight = m_bookedWeights[_idx_weight].Label();

    TString _nameVarY_clean = CleanUpName(_nameVarY);
    TString _nameVarX_clean = CleanUpName(_nameVarX);

    TString _nameHisto  = fmt::format("{0}_{1}_{2}_{3}", _nameVarY_clean, _nameVarX_clean, _nameCut, _nameWeight);
    TString _titleHisto = fmt::format("{0} vs {1} [{2}, {3}]", _nameVarY, _nameVarX, _nameCut, _nameWeight);

    int _nUnderFlowX = 0;
    int _nOverFlowX  = 0;

    int _nUnderFlowY = 0;
    int _nOverFlowY  = 0;

    // cout<<RED<< fmt::format("{0}, {1}, bins {2}, min {3}, max{4} ", _nameHisto, _titleHisto, _nBins, _min, _max) << endl;
    auto             _xVar   = GetVariableValues(_idx_varX);
    auto             _yVar   = GetVariableValues(_idx_varY);
    auto             _sel    = GetSelectionValues(_idx_cut);
    auto             _weight = GetWeightValues(_idx_weight);
    vector< size_t > _sizes  = {_xVar.size(), _yVar.size(), _sel.size(), _weight.size()};

    bool _equal = all_of(_sizes.begin(), _sizes.end(), [&_sizes](size_t _item) { return _item == _sizes[0]; });
    if (!_equal) MessageSvc::Error("GetHistogram2D", (TString) "Something went wrong, aborting", "EXIT_FAILURE");

    // TH2D(const char* name, const char* title, Int_t nbinsx, Double_t xlow, Double_t xup, Int_t nbinsy, Double_t ylow, Double_t yup)
    m_CachedHisto2DPool[idx_combo] = TH2D(_nameHisto, _titleHisto, _nBinsX, _minX, _maxX, _nBinsY, _minY, _maxY);

    for (auto && _entry : zip_range(_xVar, _yVar, _sel, _weight)) {
        if (!_entry.get< 2 >()) { continue; }
        double _x      = _entry.get< 0 >();
        double _y      = _entry.get< 1 >();
        double _weightVal  = _entry.get< 3 >();
        if (_x < _minX) { _nUnderFlowX++; }
        if (_x > _maxX) { _nOverFlowX++; }
        if (_y < _minY) { _nUnderFlowY++; }
        if (_y > _maxY) { _nOverFlowY++; }
        m_CachedHisto2DPool[idx_combo].Fill(_x, _y, _weightVal );
    }
    m_CachedHisto2DPool[idx_combo].Sumw2();

    MessageSvc::Line();
    MessageSvc::Info("GetHistogram2D", (TString) "Variable   =", m_bookedVariable[_idx_varX].Expression(), ":", m_bookedVariable[_idx_varY].Expression());
    MessageSvc::Info("GetHistogram2D", (TString) "Selection  =", m_bookedSelections[_idx_cut].Label());
    MessageSvc::Info("GetHistogram2D", (TString) "Entries    =", to_string(m_CachedHisto2DPool[idx_combo].GetEntries()));
    if (m_CachedHisto2DPool[idx_combo].Integral() != m_CachedHisto2DPool[idx_combo].GetEntries()) MessageSvc::Info("GetHistogram2D", (TString) "Integral   =", to_string(m_CachedHisto2DPool[idx_combo].Integral()));
    if (_nUnderFlowX > 0) MessageSvc::Warning("GetHistogram2D", (TString) "UnderFlowX =", to_string(_nUnderFlowX), "below", to_string(_minX));
    if (_nOverFlowX > 0) MessageSvc::Warning("GetHistogram2D", (TString) "OverFlowX  =", to_string(_nOverFlowX), "above", to_string(_maxX));
    if (_nUnderFlowY > 0) MessageSvc::Warning("GetHistogram2D", (TString) "UnderFlowY =", to_string(_nUnderFlowY), "below", to_string(_minY));
    if (_nOverFlowY > 0) MessageSvc::Warning("GetHistogram2D", (TString) "OverFlowY  =", to_string(_nOverFlowY), "above", to_string(_maxY));

    // Re-work axis labels here
    TString _xAxis = m_bookedVariable[_idx_varX].Label();
    TString _yAxis = m_bookedVariable[_idx_varY].Label();
    if (m_bookedVariable[_idx_varX].Units() != "") { _xAxis += " [" + m_bookedVariable[_idx_varX].Units() + "]"; }
    if (m_bookedVariable[_idx_varY].Units() != "") { _yAxis += " [" + m_bookedVariable[_idx_varY].Units() + "]"; }
    m_CachedHisto2DPool[idx_combo].GetXaxis()->SetTitle(_xAxis);
    m_CachedHisto2DPool[idx_combo].GetYaxis()->SetTitle(_yAxis);
    TString _zAxis = "Counts";
    m_CachedHisto2DPool[idx_combo].GetZaxis()->SetTitle(_zAxis);

    m_CachedHisto2DPool[idx_combo].SetFillColor(_color);
    m_CachedHisto2DPool[idx_combo].SetLineColor(_color);
    m_CachedHisto2DPool[idx_combo].SetMarkerColor(_color);

    return &m_CachedHisto2DPool[idx_combo];
}

TRatioPlot * RXDataPlayer::GetHistogramRatio2D(pair< int, int > _idx_XY_var, int _idxNum_cut, int _idxNum_weight, int _idxDen_cut, int _idxDen_weight, TString _option) {
    TH2D * _XYHistNum = (TH2D *) GetHistogram2D(_idx_XY_var, _idxNum_cut, _idxNum_weight)->Clone();
    TH2D * _XYHistDen = (TH2D *) GetHistogram2D(_idx_XY_var, _idxDen_cut, _idxDen_weight)->Clone();

    _XYHistNum->SetName(TString(_XYHistNum->GetName()) + " forRatio");
    _XYHistDen->SetName(TString(_XYHistDen->GetName()) + " forRatio");
    if (_option.Contains("scale")) {
        ScaleHistogram(*_XYHistNum);
        ScaleHistogram(*_XYHistDen);
    }
    TRatioPlot * _hratio = new TRatioPlot(_XYHistNum, _XYHistDen, _option);
    return _hratio;
}

void RXDataPlayer::SaveHistogram2D(TString _option, int _idx_cut, int _idx_weight) {
    TString _name = m_name;
    if (_option.Contains("s")) _name += "_Selections";
    if (_option.Contains("w")) _name += "_Weights";
    _name += fmt::format("_S{0}_W{1}", _idx_cut, _idx_weight);
    _name += "_TH2.pdf";

    EColor _color = kBlack;
    if (_option.Contains("s")) _color = (EColor) GetColorFromGradient(0, GetNSelections());
    if (_option.Contains("w")) _color = (EColor) GetColorFromGradient(0, GetNWeights());

    TString _draw = "box";

    TCanvas _canvas("canvas");
    _canvas.SaveAs(_name + "[");
    for (int i = 0; i < GetNVariables(); ++i) {
        for (int j = i + 1; j < GetNVariables(); ++j) {
            if (_option.Contains("n"))
                GetHistogram2D(make_pair(i, j), _idx_cut, _idx_weight, _color)->DrawNormalized(_draw);
            else
                GetHistogram2D(make_pair(i, j), _idx_cut, _idx_weight, _color)->Draw(_draw);

            if (_option.Contains("s")) {
                for (int k = 1; k < GetNSelections(); ++k) {
                    if (_option.Contains("n"))
                        GetHistogram2D(make_pair(i, j), k, _idx_weight, (EColor) GetColorFromGradient(k, GetNSelections()))->DrawNormalized(_draw + "same");
                    else
                        GetHistogram2D(make_pair(i, j), k, _idx_weight, (EColor) GetColorFromGradient(k, GetNSelections()))->Draw(_draw + "same");
                }
            } else if (_option.Contains("w")) {
                for (int k = 1; k < GetNWeights(); ++k) {
                    if (_option.Contains("n"))
                        GetHistogram2D(make_pair(i, j), _idx_cut, k, (EColor) GetColorFromGradient(k, GetNWeights()))->DrawNormalized(_draw + "same");
                    else
                        GetHistogram2D(make_pair(i, j), _idx_cut, k, (EColor) GetColorFromGradient(k, GetNWeights()))->Draw(_draw + "same");
                }
            }

            //_canvas.BuildLegend()->Draw();
            _canvas.SaveAs(_name);
        }
    }
    _canvas.SaveAs(_name + "]");

    return;
}

TProfile * RXDataPlayer::GetHistogramProfile(pair< int, int > _idx_var, int _idx_cut, int _idx_weight, EColor _color, TString _option) {
    int _idx_varX = _idx_var.first;
    int _idx_varY = _idx_var.second;

    if (_idx_varX >= GetNVariables()) MessageSvc::Error("GetHistogramProfile", to_string(_idx_varX), to_string(GetNVariables()), "not existing Variable", "EXIT_FAILURE");
    if (_idx_varY >= GetNVariables()) MessageSvc::Error("GetHistogramProfile", to_string(_idx_varY), to_string(GetNVariables()), "not existing Variable", "EXIT_FAILURE");
    if (_idx_cut >= GetNSelections()) MessageSvc::Error("GetHistogramProfile", to_string(_idx_cut), to_string(GetNSelections()), "not existing Selection", "EXIT_FAILURE");
    if (_idx_weight >= GetNWeights()) MessageSvc::Error("GetHistogramProfile", to_string(_idx_weight), to_string(GetNWeights()), "not existing Weight", "EXIT_FAILURE");

    // Retrieve Histogram from Var-IDX_X
    int     _nBinsX   = m_bookedVariable[_idx_varX].NBins();
    double  _minX     = m_bookedVariable[_idx_varX].Min();
    double  _maxX     = m_bookedVariable[_idx_varX].Max();
    TString _nameVarX = m_bookedVariable[_idx_varX].Expression();
    /* not needed for profile  as
        int     _nBinsY   = m_bookedVariable[_idx_varY].NBins();
        ( TProfile (const char *name, const char *title, Int_t nbinsx, Double_t xlow, Double_t xup, Double_t ylow, Double_t yup, Option_t *option=""))
    */
    double  _minY     = m_bookedVariable[_idx_varY].Min();
    double  _maxY     = m_bookedVariable[_idx_varY].Max();
    TString _nameVarY = m_bookedVariable[_idx_varY].Expression();

    TString _nameCut    = m_bookedSelections[_idx_cut].Label();
    TString _nameWeight = m_bookedWeights[_idx_weight].Label();

    TString _nameVarY_clean = CleanUpName(_nameVarY);
    TString _nameVarX_clean = CleanUpName(_nameVarX);

    TString _nameHisto  = fmt::format("{0}_{1}_{2}_{3}", _nameVarY_clean, _nameVarX_clean, _nameCut, _nameWeight);
    TString _titleHisto = fmt::format("{0} vs {1} [{2}, {3}]", _nameVarY, _nameVarX, _nameCut, _nameWeight);

    int _nUnderFlowX = 0;
    int _nOverFlowX  = 0;

    int _nUnderFlowY = 0;
    int _nOverFlowY  = 0;

    // cout<<RED<< fmt::format("{0}, {1}, bins {2}, min {3}, max{4} ", _nameHisto, _titleHisto, _nBins, _min, _max) << endl;
    auto             _xVar   = GetVariableValues(_idx_varX);
    auto             _yVar   = GetVariableValues(_idx_varY);
    auto             _sel    = GetSelectionValues(_idx_cut);
    auto             _weight = GetWeightValues(_idx_weight);
    vector< size_t > _sizes  = {_xVar.size(), _yVar.size(), _sel.size(), _weight.size()};

    bool _equal = all_of(_sizes.begin(), _sizes.end(), [&_sizes](size_t _item) { return _item == _sizes[0]; });
    if (!_equal) MessageSvc::Error("GetHistogramProfile", (TString) "Something went wrong, aborting", "EXIT_FAILURE");

    TString _error = "";
    if (_option.Contains("s")) _error = "s";

    // TProfile (const char *name, const char *title, Int_t nbinsx, Double_t xlow, Double_t xup, Double_t ylow, Double_t yup, Option_t *option="")
    TProfile * _histo = nullptr;
    if (_option.Contains("y"))
        _histo = new TProfile(_nameHisto, _titleHisto, _nBinsX, _minX, _maxX, _minY, _maxY, _error);
    else
        _histo = new TProfile(_nameHisto, _titleHisto, _nBinsX, _minX, _maxX, _error);
    _histo->Sumw2();

    for (auto && _entry : zip_range(_xVar, _yVar, _sel, _weight)) {
        if (!_entry.get< 2 >()) { continue; }
        double _x      = _entry.get< 0 >();
        double _y      = _entry.get< 1 >();
        double _weightVAL = _entry.get< 3 >();
        if (_x < _minX) { _nUnderFlowX++; }
        if (_x > _maxX) { _nOverFlowX++; }
        if (_y < _minY) { _nUnderFlowY++; }
        if (_y > _maxY) { _nOverFlowY++; }
        _histo->Fill(_x, _y, _weightVAL);
    }

    MessageSvc::Line();
    MessageSvc::Info("GetHistogramProfile", (TString) "Variable   =", m_bookedVariable[_idx_varX].Expression(), ":", m_bookedVariable[_idx_varY].Expression());
    MessageSvc::Info("GetHistogramProfile", (TString) "Selection  =", m_bookedSelections[_idx_cut].Label());
    MessageSvc::Info("GetHistogramProfile", (TString) "Entries    =", to_string(_histo->GetEntries()));
    if (_histo->Integral() != _histo->GetEntries()) MessageSvc::Info("GetHistogramProfile", (TString) "Integral   =", to_string(_histo->Integral()));
    if (_nUnderFlowX > 0) MessageSvc::Debug("GetHistogramProfile", (TString) "UnderFlowX =", to_string(_nUnderFlowX), "below", to_string(_minX));
    if (_nOverFlowX > 0) MessageSvc::Debug("GetHistogramProfile", (TString) "OverFlowX  =", to_string(_nOverFlowX), "above", to_string(_maxX));
    if (_nUnderFlowY > 0) MessageSvc::Debug("GetHistogramProfile", (TString) "UnderFlowY =", to_string(_nUnderFlowY), "below", to_string(_minY));
    if (_nOverFlowY > 0) MessageSvc::Debug("GetHistogramProfile", (TString) "OverFlowY  =", to_string(_nOverFlowY), "above", to_string(_maxY));

    // Re-work axis labels here
    TString _xAxis = m_bookedVariable[_idx_varX].Label();
    TString _yAxis = m_bookedVariable[_idx_varY].Label();
    if (m_bookedVariable[_idx_varX].Units() != "") { _xAxis += " [" + m_bookedVariable[_idx_varX].Units() + "]"; }
    if (m_bookedVariable[_idx_varY].Units() != "") { _yAxis += " [" + m_bookedVariable[_idx_varY].Units() + "]"; }
    _histo->GetXaxis()->SetTitle(_xAxis);
    _histo->GetYaxis()->SetTitle(_yAxis);

    _histo->SetLineColor(_color);
    _histo->SetMarkerColor(_color);

    return _histo;
}

TProfile * RXDataPlayer::GetHistogramProfile(pair< TString, TString > _name_var, TString _name_cut, TString _name_weight, EColor _color, TString _option) {
    int _idx_varX   = -1;
    int _idx_varY   = -1;
    int _idx_cut    = -1;
    int _idx_weight = -1;
    for (const auto & _var : m_bookedVariable) {
        if (_var.second.Expression() == _name_var.first) _idx_varX = _var.first;
        if (_var.second.Expression() == _name_var.second) _idx_varY = _var.first;
    }
    for (const auto & _cut : m_bookedSelections) {
        if (_cut.second.Label() == _name_cut) _idx_cut = _cut.first;
    }
    for (const auto & _weight : m_bookedWeights) {
        if (_weight.second.Label() == _name_weight) _idx_weight = _weight.first;
    }
    if (_idx_varX == -1) MessageSvc::Error("GetHistogramProfile", _name_var.first, "not existing Variable", "EXIT_FAILURE");
    if (_idx_varY == -1) MessageSvc::Error("GetHistogramProfile", _name_var.second, "not existing Variable", "EXIT_FAILURE");
    if (_idx_cut == -1) MessageSvc::Error("GetHistogramProfile", _name_cut, "not existing Selection", "EXIT_FAILURE");
    if (_idx_weight == -1) MessageSvc::Error("GetHistogramProfile", _name_weight, "not existing Weight", "EXIT_FAILURE");

    return GetHistogramProfile(make_pair(_idx_varX, _idx_varY), _idx_cut, _idx_weight, _color, _option);
}

void RXDataPlayer::SaveHistogramProfile(TString _option, int _idx_cut, int _idx_weight) {
    TString _name = m_name;
    if (_option.Contains("s")) _name += "_Selections";
    if (_option.Contains("w")) _name += "_Weights";
    _name += fmt::format("_S{0}_W{1}", _idx_cut, _idx_weight);
    _name += "_TProfile.pdf";

    EColor _color = kBlack;
    if (_option.Contains("s")) _color = (EColor) GetColorFromGradient(0, GetNSelections());
    if (_option.Contains("w")) _color = (EColor) GetColorFromGradient(0, GetNWeights());

    TString _draw = "box";

    TCanvas _canvas("canvas");
    _canvas.SaveAs(_name + "[");
    for (int i = 0; i < GetNVariables(); ++i) {
        for (int j = i + 1; j < GetNVariables(); ++j) {
            GetHistogramProfile(make_pair(i, j), _idx_cut, _idx_weight, _color)->Draw(_draw);

            if (_option.Contains("s")) {
                for (int k = 1; k < GetNSelections(); ++k) { GetHistogramProfile(make_pair(i, j), k, _idx_weight, (EColor) GetColorFromGradient(k, GetNSelections()))->Draw(_draw + "same"); }
            } else if (_option.Contains("w")) {
                for (int k = 1; k < GetNWeights(); ++k) { GetHistogramProfile(make_pair(i, j), _idx_cut, k, (EColor) GetColorFromGradient(k, GetNWeights()))->Draw(_draw + "same"); }
            }

            _canvas.BuildLegend()->Draw();
            _canvas.SaveAs(_name);
        }
    }
    _canvas.SaveAs(_name + "]");

    return;
}

vector< double > RXDataPlayer::GetVariableValues(int _idx) {
    if (_idx > GetNVariables()) MessageSvc::Error("GetVariableValues", (TString) "Requesting IDX > stored ones", "EXIT_FAILURE");
    return m_bookedVariableValues[m_bookedVariable[_idx]];
}
vector< double > RXDataPlayer::GetVariableValues(TString _variableLabel) {
    int _idx = -1;
    for (int i = 0; i < GetNVariables(); ++i) {
        if (GetVariableLabel(i) == _variableLabel) { _idx = i; }
    }
    if (_idx == -1) {
        MessageSvc::Error("Cannot retrieve", _variableLabel, "Have you bookkeped it?");
        Print();
        MessageSvc::Error("", "", "EXIT_FAILURE");
    }
    return GetVariableValues(_idx);
}

vector< bool > RXDataPlayer::GetSelectionValues(int _idx) {
    if (_idx > GetNSelections()) MessageSvc::Error("GetSelectionValues", (TString) "Requesting IDX > stored ones", "EXIT_FAILURE");
    return m_bookedSelectionResults[m_bookedSelections[_idx]];
}
vector< bool > RXDataPlayer::GetSelectionValues(TString _selectionLabel) {
    int _idx = -1;
    for (int i = 0; i < GetNSelections(); ++i) {
        if (GetSelectionLabel(i) == _selectionLabel) { _idx = i; }
    }
    if (_idx == -1) {
        MessageSvc::Warning("GetSelection Values, Cannot retrieve", _selectionLabel, "Have you bookkeped it?");
        Print();
        MessageSvc::Error("", "", "EXIT_FAILURE");
    }
    return GetSelectionValues(_idx);
}

vector< double > RXDataPlayer::GetWeightValues(int _idx) {
    if (_idx > GetNWeights()) MessageSvc::Error("GetWeightValues", (TString) "Requesting IDX > stored ones", "EXIT_FAILURE");
    return m_bookedWeightResults[m_bookedWeights[_idx]];
}

vector< double > RXDataPlayer::GetWeightValues(TString _weightLabel) {
    int _idx = -1;
    for (int i = 0; i < GetNWeights(); ++i) {
        if (GetWeightLabel(i) == _weightLabel) { _idx = i; }
    }
    if (_idx == -1) {
        MessageSvc::Error("GetWeightValues, Cannot retrieve", _weightLabel, "Have you bookkeped it?");
        Print();
        MessageSvc::Error("", "", "EXIT_FAILURE");
    }
    return m_bookedWeightResults[m_bookedWeights[_idx]];
}

void RXDataPlayer::SaveAll(TString _fileName) {
    // Experiemntal features USE AND REPORT ERRORS/ISSUES , save all histograms to a File to browse afterwards, some issues in directory structures to fix
    MessageSvc::Info("RXDataPlayer SaveAll ", _fileName);
    TFile * file = nullptr;
    if (_fileName != "") {
        if (_fileName.Contains(".root")) { _fileName.ReplaceAll(".root", ""); }
        file = new TFile(_fileName + ".root", "RECREATE");
    } else {
        file = new TFile(m_name + ".root", "RECREATE");
    }

    file->cd();
    map< TString, TDirectory * > all_directories;
    // 1D plots
    for (int idx_var = 0; idx_var < GetNVariables(); ++idx_var) {
        TString vv     = m_bookedVariable[idx_var].Expression();
        auto    VARDIR = CleanUpName(vv);   // The VarDIR directory
        file->mkdir(VARDIR);
        all_directories[VARDIR] = file->mkdir(VARDIR);
        all_directories[VARDIR]->cd();
        if (GetNSelections() == 1) {
            for (int idx_weight = 0; idx_weight < GetNWeights(); ++idx_weight) {
                TString WEIDIR       = m_bookedWeights[idx_weight].Label();
                TString dir          = VARDIR + "/" + WEIDIR;
                all_directories[dir] = all_directories[VARDIR]->mkdir(WEIDIR);
                all_directories[dir]->cd();
                GetHistogram1D(idx_var, 0, idx_weight)->Write("", TObject::kOverwrite);
            }
            continue;
        } else {
            for (int idx_cut = 0; idx_cut < GetNSelections(); ++idx_cut) {
                TString CUTDIR                         = m_bookedSelections[idx_cut].Label();
                CUTDIR                                 = CleanUpName(CUTDIR);
                all_directories[VARDIR + "/" + CUTDIR] = all_directories[VARDIR]->mkdir(CUTDIR);
                if (GetNWeights() == 1) {
                    all_directories[VARDIR + "/" + CUTDIR]->cd();
                    GetHistogram1D(idx_var, idx_cut, 0)->Write("", TObject::kOverwrite);
                    continue;
                } else {
                    for (int idx_weight = 0; idx_weight < GetNWeights(); ++idx_weight) {
                        TString WEIDIR                                        = m_bookedWeights[idx_weight].Label();
                        all_directories[VARDIR + "/" + CUTDIR + "/" + WEIDIR] = all_directories[VARDIR + "/" + CUTDIR]->mkdir(WEIDIR);
                        all_directories[VARDIR + "/" + CUTDIR + "/" + WEIDIR]->cd();
                        GetHistogram1D(idx_var, idx_cut, idx_weight)->Write("", TObject::kOverwrite);
                    }
                }
            }
        }
    }
    for (auto & dd : all_directories) { MessageSvc::Info("Plot is saved inside ", dd.first); }
}

#endif
