#ifndef HISTOGRAMSVC_CPP
#define HISTOGRAMSVC_CPP

#include "HistogramSvc.hpp"
#include "HelperSvc.hpp"

#include "SettingDef.hpp"
#include "WeightDefRX.hpp"

#include "core.h"
#include <fmt_ostream.h>

#include "TCanvas.h"
#include "TChain.h"
#include "TCut.h"
#include "TH1.h"
#include "TH1D.h"
#include "TH2.h"
#include "TH2D.h"
#include "TH2Poly.h"
#include "TH3D.h"
#include "TMath.h"
#include "TRandom3.h"
#include "TString.h"

#include "RooAbsReal.h"
#include "RooRealVar.h"

bool AdaptingBounds(TH1D * _histo) noexcept {
    vector< double > bin_bounds = {};
    int              nCells     = _histo->GetNcells();
    for (int i = 1; i < nCells - 1; ++i) { bin_bounds.push_back(_histo->GetBinLowEdge(i)); }
    auto   bin_idx_last        = make_pair(nCells - 2, nCells - 1);
    auto   bin_idx_last_prev   = make_pair(nCells - 3, nCells - 2);
    double last_bin_width      = abs(_histo->GetBinLowEdge(bin_idx_last.first) - _histo->GetBinLowEdge(bin_idx_last.second));
    double last_bin_width_prev = abs(_histo->GetBinLowEdge(bin_idx_last_prev.first) - _histo->GetBinLowEdge(bin_idx_last_prev.second));

    if (last_bin_width / last_bin_width_prev > 4) return true;

    return false;
}

TH1D * SquezeBoundaries(TH1D * _histo, bool _delete) noexcept {
    MessageSvc::Warning("Adapting boundaries of Histogram (TH1D) on last bin for better drawing, keeping content intact", TString(_histo->GetName()), "");
    vector< pair< int, pair< double, double > > > bin_bounds = {};
    int                                           nCells     = _histo->GetNcells();
    for (int i = 1; i < nCells - 1; ++i) { bin_bounds.push_back(make_pair(i, make_pair(_histo->GetBinLowEdge(i), _histo->GetBinLowEdge(i + 1)))); }
    auto                    last_bin_width      = abs(_histo->GetBinLowEdge(nCells - 1) - _histo->GetBinLowEdge(nCells - 2));
    auto                    prev_last_bin_width = abs(_histo->GetBinLowEdge(nCells - 2) - _histo->GetBinLowEdge(nCells - 3));
    auto                    last_bin_newbound   = _histo->GetBinLowEdge(nCells - 2) + 2 * prev_last_bin_width;   // at most 2 times the last bin-width.
    array< double, 100000 > _boundaries;
    int                     i = 0;
    for (auto & el : bin_bounds) {
        // fill _boundaries[0] = bounds( bin1 low)
        _boundaries[i] = el.second.first;
        i++;
    }
    _boundaries[i] = last_bin_newbound;
    auto * h       = new TH1D(TString(_histo->GetName()) + " rebinned", TString(_histo->GetTitle()) + " rebinned", nCells - 2, _boundaries.data());
    for (int j = 1; j < nCells - 1; ++j) {
        h->SetBinContent(j, _histo->GetBinContent(j));
        h->SetBinError(j, _histo->GetBinError(j));
    }
    h->SetDirectory(0);

    if (_delete) delete _histo;
    return h;
}

TH1 * GetHistogram(TChain & _tuple, const TCut & _cut, const TString & _extraName, const RooRealVar & _varX, const RooRealVar & _varY, const RooRealVar & _varZ) {
    TString _addName = _extraName;
    if (_addName != "") _addName = "_" + _addName;

    TCut _cutTmp = _cut;

    vector< pair< TString, TString > > _allAliases = GetAllAlias(&_tuple);
    if (_allAliases.size() != 0) {
        TString _cutExpr = TString(_cutTmp);
        for (auto & _alias : _allAliases) {
            // NB : do make aliases such that you don't have repetition or extension names
            // For example LL_XY , LL_XYZ are going to "contain" in the cut the LL_XY and wrongly replace
            if (_cutExpr.Contains(_alias.first)) {
                // MessageSvc::Warning("GetHistogram , expaning alias for Cut", _cutExpr);
                _cutExpr.ReplaceAll(_alias.first, _alias.second);
                // MessageSvc::Warning("GetHistogram , new Cut", _cutExpr);
            }
        }
        _cutTmp = _cutExpr;
        if (_cutTmp != _cut) {
            MessageSvc::Info("Cut", &_cut);
            MessageSvc::Info("Cut", (TString) "Expanding aliases");
            MessageSvc::Info("Cut", &_cutTmp);
        }
    }

    // simple function returning the name of histo and the draw for Tree->Draw("") to fill that histo (1,2,3 D)
    auto generateHistoName = [&](TString _addNameLocal, TString _x = DUMMYNAME, TString _y = DUMMYNAME, TString _z = DUMMYNAME) -> pair< TString, TString > {
        TString _name = "histo";
        TString _draw = "";
        if (_z != DUMMYNAME) {
            _name = (TString) fmt::format("histo_{0}_{1}_{2}{3}", _x, _y, _z, _addNameLocal);
            _name.ReplaceAll(":", "_").ReplaceAll("(", "_").ReplaceAll(")", "_").ReplaceAll("+", "_").ReplaceAll("-", "_").ReplaceAll("*", "_").ReplaceAll("/", "_").ReplaceAll(",", "_");
            _draw = (TString) fmt::format("{0}:{1}:{2}>>{3}", _z, _y, _x, _name);
        } else if (_y != DUMMYNAME) {
            _name = (TString) fmt::format("histo_{0}_{1}{2}", _x, _y, _addNameLocal);
            _name.ReplaceAll(":", "_").ReplaceAll("(", "_").ReplaceAll(")", "_").ReplaceAll("+", "_").ReplaceAll("-", "_").ReplaceAll("*", "_").ReplaceAll("/", "_").ReplaceAll(",", "_");
            _draw = (TString) fmt::format("{0}:{1}>>{2}", _y, _x, _name);
        } else {
            _name = (TString) fmt::format("histo_{0}{1}", _x, _addNameLocal);
            _name.ReplaceAll(":", "_").ReplaceAll("(", "_").ReplaceAll(")", "_").ReplaceAll("+", "_").ReplaceAll("-", "_").ReplaceAll("*", "_").ReplaceAll("/", "_").ReplaceAll(",", "_");
            _draw = (TString) fmt::format("{0}>>{1}", _x, _name);
        }
        return make_pair(_name, _draw);
    };

    // int nDim = (_varX.GetName() != TString(DUMMYNAME)) + (_varY.GetName() != TString(DUMMYNAME)) + (_varZ.GetName() != TString(DUMMYNAME));

    pair< TString, TString > _name_draw = generateHistoName(_addName, _varX.GetName(), _varY.GetName(), _varZ.GetName());

    TH1 * _histo = nullptr;
    if (_varZ.GetName() != TString(DUMMYNAME)) {
        // 3D draw
        _histo = new TH3D(_name_draw.first, "", _varX.getBins(), _varX.getMin(), _varX.getMax(), _varY.getBins(), _varY.getMin(), _varY.getMax(), _varZ.getBins(), _varZ.getMin(), _varZ.getMax());
        _histo->SetXTitle(_varX.GetName());
        _histo->SetYTitle(_varY.GetName());
        _histo->SetZTitle(_varZ.GetName());

    } else if (_varY.GetName() != TString(DUMMYNAME)) {
        // 2D draw
        _histo = new TH2D(_name_draw.first, "", _varX.getBins(), _varX.getMin(), _varX.getMax(), _varY.getBins(), _varY.getMin(), _varY.getMax());
        _histo->SetXTitle(_varX.GetName());
        _histo->SetYTitle(_varY.GetName());
    } else {
        // 1D draw
        _histo = new TH1D(_name_draw.first, "", _varX.getBins(), _varX.getMin(), _varX.getMax());
        _histo->SetXTitle(_varX.GetName());
    }
    _histo->Sumw2();

    MessageSvc::Info("Tuple", (TString) _tuple.GetName());
    if (_varZ.GetName() != TString(DUMMYNAME))
        MessageSvc::Info("DrawZ", _name_draw.second, "(" + to_string(_histo->GetNbinsZ()) + ", " + to_string(_histo->GetZaxis()->GetXmin()) + ", " + to_string(_histo->GetZaxis()->GetXmax()) + ")");
    else if (_varY.GetName() != TString(DUMMYNAME))
        MessageSvc::Info("DrawY", _name_draw.second, "(" + to_string(_histo->GetNbinsY()) + ", " + to_string(_histo->GetYaxis()->GetXmin()) + ", " + to_string(_histo->GetYaxis()->GetXmax()) + ")");
    else if (_varX.GetName() != TString(DUMMYNAME))
        MessageSvc::Info("DrawX", _name_draw.second, "(" + to_string(_histo->GetNbinsX()) + ", " + to_string(_histo->GetXaxis()->GetXmin()) + ", " + to_string(_histo->GetXaxis()->GetXmax()) + ")");

    MessageSvc::Info("Entries (tot)", to_string(_tuple.GetEntries()));
    MessageSvc::Warning("Frac", to_string(SettingDef::Tuple::frac));
    MessageSvc::Info("Cut", &_cutTmp);

    Long64_t _maxEntries = _tuple.GetEntries();
    // Plot fixed number of Entries
    if ((SettingDef::Tuple::frac != -1) && (SettingDef::Tuple::frac >= 1)) _maxEntries = (int) floor(SettingDef::Tuple::frac);
    // Plot fraction of Entries
    if ((SettingDef::Tuple::frac >= 0.f) && (SettingDef::Tuple::frac < 1.0f)) _maxEntries = (int) floor(SettingDef::Tuple::frac * _maxEntries);

    TCanvas  _canvas("canvas");
    Long64_t _entries = _tuple.Draw(_name_draw.second, _cutTmp, "e", _maxEntries);
    if (_entries < 0) MessageSvc::Error("GetHistogram", (TString) "EXIT_FAILURE");
    // MessageSvc::Info("GetHistogram", _histo);

    MessageSvc::Info("Entries (cut)", to_string(_histo->GetEntries()));
    if (_histo->Integral() != _histo->GetEntries()) MessageSvc::Info("Integral", to_string(_histo->Integral()));
    if (_histo->GetBinContent(0) != 0) MessageSvc::Warning("Underflows", to_string(_histo->GetBinContent(0)));
    if (_histo->GetBinContent(_histo->GetNbinsX() + 1) != 0) MessageSvc::Warning("Overflows", to_string(_histo->GetBinContent(_histo->GetNbinsX() + 1)));
    // cout << endl;

    return _histo;
}

void CheckHistogram(TH1 * _histo, TString _option) {    
    if (_histo != nullptr) {
        if (!_option.Contains("q") && (_option.Contains("effr") || _option.Contains("ratior"))) MessageSvc::Warning("CheckHistogram", (TString) "Resetting", _histo->GetName(), _option);
        int _count1 = 0;
        int _count2 = 0;
        int _count3 = 0;
        int _count4 = 0;
        for (int i = 0; i <= _histo->GetNcells() + 1; ++i) {
            int _x, _y, _z;
            if (_option.Contains("eff") || _option.Contains("ratio")) {
                if (_histo->GetBinContent(i) < 0) {
                    /*
                    if (!_option.Contains("q")) {
                        _histo->GetBinXYZ(i, _x, _y, _z);
                        MessageSvc::Warning("<0", _histo->GetName(), to_string(_histo->GetBinContent(i)) + " +/- " + to_string(_histo->GetBinError(i)), "(" + to_string(TMath::Abs(_histo->GetBinError(i) / (double) _histo->GetBinContent(i)) * 100) + "%)", "@ (" + to_string(_x) + ", " + to_string(_y) + ", " + to_string(_z) + ")");
                    }
                    */
                    if (_option.Contains("effr") || _option.Contains("ratior")) {
                        MessageSvc::Info("effr,ratior applied, bin content,error set to 0");
                        _histo->SetBinContent(i, 0);
                        _histo->SetBinError(i, 0);
                    }
                    _count1++;
                }
                if (_option.Contains("eff")) {
                    if (_histo->GetBinContent(i) > 1) {
                        /*
                        if (!_option.Contains("q")) {
                            _histo->GetBinXYZ(i, _x, _y, _z);
                            MessageSvc::Warning(">1", _histo->GetName(), to_string(_histo->GetBinContent(i)) + " +/- " + to_string(_histo->GetBinError(i)), "(" + to_string(TMath::Abs(_histo->GetBinError(i) / (double) _histo->GetBinContent(i)) * 100) + "%)", "@ (" + to_string(_x) + ", " + to_string(_y) + ", " + to_string(_z) + ")");
                        }
                        */
                        MessageSvc::Info("effr,ratior applied, bin content >1 , bin value set to 1.");
                        if (_option.Contains("effr")) {
                            _histo->SetBinContent(i, 1);
                            _histo->SetBinError(i, 0);
                        }
                        _count2++;
                    }
                }
            }
            if (TMath::Abs(_histo->GetBinError(i) / (double) _histo->GetBinContent(i)) > 0.5) {
                _count3++;
            }
        }
        if (!_option.Contains("q")) {
            if (_count1 != 0) MessageSvc::Warning("Bins <0", to_string(_count1), "(" + to_string(_count1 / (double) (_histo->GetNcells()) * 100) + "%)");
            if (_count2 != 0) MessageSvc::Warning("Bins >1", to_string(_count2), "(" + to_string(_count2 / (double) (_histo->GetNcells()) * 100) + "%)");
            if (_count3 != 0) MessageSvc::Warning("Bins >50%", to_string(_count3), "(" + to_string(_count3 / (double) (_histo->GetNcells()) * 100) + "%)");                                    
        }
    }
    return;
}

void CheckHistogram(TH1 * _hPas, TH1 * _hTot, TString _option) {
    if ((_hPas != nullptr) && (_hTot != nullptr)) {
        if (!_option.Contains("q") && (_option.Contains("effr") || _option.Contains("ratior"))) MessageSvc::Warning("CheckHistogram", (TString) "Resetting", _hPas->GetName(), _hTot->GetName(), _option);
        int _count1 = 0;
        int _count2 = 0;
        int _count3 = 0;
        for (int i = 0; i <= _hPas->GetNcells() + 1; ++i) {
            if (_option.Contains("eff")) {
                int _x, _y, _z;
                if (_hPas->GetBinContent(i) < 0) {
                    if (!_option.Contains("q")) {
                        _hPas->GetBinXYZ(i, _x, _y, _z);
                        MessageSvc::Warning("Pas<0", _hPas->GetName(), to_string(_hPas->GetBinContent(i)) + " +/- " + to_string(_hPas->GetBinError(i)), "@ (" + to_string(_x) + ", " + to_string(_y) + ", " + to_string(_z) + ")");
                    }
                    if (_option.Contains("effr")) {
                        _hPas->SetBinContent(i, 0);
                        _hPas->SetBinError(i, 0);
                    }
                    _count1++;
                }
                if (_hTot->GetBinContent(i) < 0) {
                    if (!_option.Contains("q")) {
                        _hTot->GetBinXYZ(i, _x, _y, _z);
                        MessageSvc::Warning("Tot<0", _hTot->GetName(), to_string(_hTot->GetBinContent(i)) + " +/- " + to_string(_hTot->GetBinError(i)), "@ (" + to_string(_x) + ", " + to_string(_y) + ", " + to_string(_z) + ")");
                    }
                    if (_option.Contains("effr")) {
                        _hTot->SetBinContent(i, 0);
                        _hTot->SetBinError(i, 0);
                    }
                    _count2++;
                }
                if (_hPas->GetBinContent(i) > _hTot->GetBinContent(i)) {
                    if (!_option.Contains("q")) {
                        _hPas->GetBinXYZ(i, _x, _y, _z);
                        MessageSvc::Warning("Pas", _hPas->GetName(), to_string(_hPas->GetBinContent(i)) + " +/- " + to_string(_hPas->GetBinError(i)), "@ (" + to_string(_x) + ", " + to_string(_y) + ", " + to_string(_z) + ")");
                        _hTot->GetBinXYZ(i, _x, _y, _z);
                        MessageSvc::Warning("Tot", _hTot->GetName(), to_string(_hTot->GetBinContent(i)) + " +/- " + to_string(_hTot->GetBinError(i)), "@ (" + to_string(_x) + ", " + to_string(_y) + ", " + to_string(_z) + ")");
                    }
                    if (_option.Contains("effr")) {
                        _hPas->SetBinContent(i, _hTot->GetBinContent(i));
                        _hPas->SetBinError(i, _hTot->GetBinError(i));
                    }
                    _count3++;
                }
            }
        }
        if (!_option.Contains("q")) {
            if (_count1 != 0) MessageSvc::Warning("Bins Pas <0", to_string(_count1), "(" + to_string(_count1 / (double) (_hPas->GetNbinsX() * _hPas->GetNbinsY() * _hPas->GetNbinsZ()) * 100) + "%)");
            if (_count1 != 0) MessageSvc::Warning("Bins Tot <0", to_string(_count2), "(" + to_string(_count2 / (double) (_hTot->GetNbinsX() * _hPas->GetNbinsY() * _hPas->GetNbinsZ()) * 100) + "%)");
            if (_count1 != 0) MessageSvc::Warning("Bins Pas > Tot", to_string(_count3), "(" + to_string(_count3 / (double) (_hPas->GetNbinsX() * _hPas->GetNbinsY() * _hPas->GetNbinsZ()) * 100) + "%)");
        }
    }
    return;
}

void CompareHistogram(TH1 * _histo1, TH1 * _histo2, TString _option) {
    if ((_histo1 != nullptr) && (_histo2 != nullptr)) {
        MessageSvc::Line();
        MessageSvc::Info("CompareHistogram", _histo1->GetName(), "vs", _histo2->GetName());
        /*
        "D" Put out a line of "Debug" printout
        "T" Return the normalized A-D test statistic
        */
        // MessageSvc::Info("AndersonDarlingTest", to_string(_histo1->AndersonDarlingTest(_histo2, _option)));
        /*
        "UU" = experiment experiment comparison (unweighted-unweighted)
        "UW" = experiment MC comparison (unweighted-weighted). Note that the first histogram should be unweighted
        "WW" = MC MC comparison (weighted-weighted)
        "NORM" = to be used when one or both of the histograms is scaled but the histogram originally was unweighted
        by default underflows and overflows are not included:
        "OF" = overflows included
        "UF" = underflows included
        "P" = print chi2, ndf, p_value, igood
        "CHI2" = returns chi2 instead of p-value
        "CHI2/NDF" = returns χ2/ndf
        */
        MessageSvc::Info("Chi2Test", to_string(_histo1->Chi2Test(_histo2, _option)));
        /*
        "U" include Underflows in test (also for 2-dim)
        "O" include Overflows (also valid for 2-dim)
        "N" include comparison of normalizations
        "D" Put out a line of "Debug" printout
        "M" Return the Maximum Kolmogorov distance instead of prob
        "X" Run the pseudo experiments post-processor with the following procedure: make pseudoexperiments based on random values from the parent distribution, compare the KS distance of the pseudoexperiment to the parent distribution, and count all the KS values above the value obtained from the original data to Monte Carlo distribution. The number of pseudo-experiments nEXPT is currently fixed at 1000. The function returns the probability. (thanks to Ben Kilminster to submit this procedure).
        Note that this option "X" is much slower.
        */
        MessageSvc::Info("KolmogorovTest", to_string(_histo1->KolmogorovTest(_histo2, _option)));
        MessageSvc::Line();
    }
    return;
}

void PrintHistogram(TH1 * _histo, TString _option) {
    if (_histo != nullptr) {
        MessageSvc::Line();
        MessageSvc::Info("PrintHistogram", _histo);
        int _count = 0;
        for (int i = 0; i <= _histo->GetNcells() + 1; ++i) {
            if ((_histo->GetBinContent(i) != 0) && (_histo->GetBinError(i) != 0)) _count++;
            if (!_option.Contains("q")) {
                int _x, _y, _z;
                _histo->GetBinXYZ(i, _x, _y, _z);
                MessageSvc::Info(to_string(i), to_string(_histo->GetBinContent(i)) + " +/- " + to_string(_histo->GetBinError(i)), "(" + to_string(TMath::Abs(_histo->GetBinError(i) / (double) _histo->GetBinContent(i)) * 100) + "%)", "@ (" + to_string(_x) + ", " + to_string(_y) + ", " + to_string(_z) + ")");
            }
        }
        if (_count != 0) MessageSvc::Warning("Bins != 0", to_string(_count), "(" + to_string(_count / (double) (_histo->GetNbinsX() * _histo->GetNbinsY() * _histo->GetNbinsZ()) * 100) + "%)");
        MessageSvc::Line();
    }
    return;
}

double GetEntries(TChain & _tuple, const TCut & _cut) {
    TString    _varName = "eventNumber";
    RooRealVar _var(_varName, _varName, 0, 1e9);
    _var.setBins(100);
    bool _status = _tuple.GetBranchStatus(_varName);
    _tuple.SetBranchStatus(_varName, true);
    TH1D * _histo = static_cast< TH1D * >(GetHistogram(_tuple, _cut, "", _var));
    _tuple.SetBranchStatus(_varName, _status);
    double _integral = _histo->Integral();
    delete _histo;
    return _integral;
}

double GetEntriesDF(TChain & _tuple, const TCut & _cut) {
    // Pre-shuffle defined Aliases to a list for Define
    auto _listAliases = GetAllAlias(&_tuple);
    EnableMultiThreads();
    ROOT::RDataFrame df(_tuple);
    double           _integral = 0.;
    if (_listAliases.size() == 0) {
        auto _count = df.Filter(TString(_cut).Data()).Count();
        _integral   = *_count;
    } else {
        auto dd      = df.Define("dummy", "1>0");
        auto ddAlias = ApplyDefines(dd, _listAliases, false);
        auto _count  = ddAlias.Filter(TString(_cut).Data()).Count();
        _integral    = *_count;
    }
    DisableMultiThreads();
    return _integral;
}

double GetHistogramVal(TH1D * _histo, double _var, TString _option) {
    double _val = 1;
    if (_histo != nullptr) {
        int _bin = _histo->FindFixBin(_var);
        if (_histo->IsBinUnderflow(_bin) || _histo->IsBinOverflow(_bin)) {
            if (_var <= _histo->GetXaxis()->GetXmin()) _var = _histo->GetXaxis()->GetXmin() + _histo->GetXaxis()->GetBinWidth(1) / 100.;
            if (_var >= _histo->GetXaxis()->GetXmax()) _var = _histo->GetXaxis()->GetXmax() - _histo->GetXaxis()->GetBinWidth(_histo->GetNbinsX()) / 100.;
            _bin = _histo->FindFixBin(_var);
        }
        if (_option.Contains("INTERP")) {
            _val = _histo->Interpolate(_var);
            if (_val == 0) {
                _val = _histo->GetBinContent(_bin);
                // MessageSvc::Warning("GetHistogramVal", (TString) "Cannot interpolate bin", to_string(_bin), "with X =", to_string(_var), ", assigning bin content", to_string(_val));
            }
        } else
            _val = _histo->GetBinContent(_bin);
        if (_option.Contains("effr") && ((_val < 0) || (_val > 1))) MessageSvc::Warning("GetHistogramVal", _histo->GetName(), "bin " + to_string(_bin) + " with X = " + to_string(_var) + " and value =", to_string(_val));
        if (_option.Contains("ratior") && (_val < 0)) MessageSvc::Warning("GetHistogramVal", _histo->GetName(), "bin " + to_string(_bin) + " with X = " + to_string(_var) + " and value =", to_string(_val));
    }
    return _val;
}

double GetHistogramVal(TH2 * _histo, double _varX, double _varY, TString _option) {
    double _val = 1;
    if (_histo != nullptr) {
        //cout << "##### nBins: " << _histo->GetNumberOfBins() << endl;
        int _bin = _histo->FindFixBin(_varX, _varY);
        if (_histo->IsBinUnderflow(_bin) || _histo->IsBinOverflow(_bin)) {
            if (_varX <= _histo->GetXaxis()->GetXmin()) _varX = _histo->GetXaxis()->GetXmin() + _histo->GetXaxis()->GetBinWidth(1) / 100.;
            if (_varX >= _histo->GetXaxis()->GetXmax()) _varX = _histo->GetXaxis()->GetXmax() - _histo->GetXaxis()->GetBinWidth(_histo->GetNbinsX()) / 100.;
            if (_varY <= _histo->GetYaxis()->GetXmin()) _varY = _histo->GetYaxis()->GetXmin() + _histo->GetYaxis()->GetBinWidth(1) / 100.;
            if (_varY >= _histo->GetYaxis()->GetXmax()) _varY = _histo->GetYaxis()->GetXmax() - _histo->GetYaxis()->GetBinWidth(_histo->GetNbinsY()) / 100.;
            _bin = _histo->FindFixBin(_varX, _varY);
        }
        //double _xMin = _histo->GetXaxis()->GetXmin();
        //double _xMax = _histo->GetXaxis()->GetXmax();
        //double _yMin = _histo->GetYaxis()->GetXmin();
        //double _yMax = _histo->GetYaxis()->GetXmax();
        //double _epsilon = 1e-9;
        //if (_varX <= _xMin) _varX = _xMin + _epsilon;
        //if (_varX >= _xMax) _varX = _xMax - _epsilon;
        //if (_varY <= _yMin) _varY = _yMin + _epsilon;
        //if (_varY >= _yMax) _varY = _yMax - _epsilon;
        //_bin = _histo->FindBin(_varX, _varY);
        if (_option.Contains("INTERP")) {
            _val = _histo->Interpolate(_varX, _varY);
            if (_val == 0) {
                _val = _histo->GetBinContent(_bin);
                // MessageSvc::Warning("GetHistogramVal", (TString) "Cannot interpolate bin " + to_string(_bin) + " with (X,Y) = (" + to_string(_varX) + "," + to_string(_varY) + "), assigning bin content", to_string(_val));
            }
        } else
            _val = _histo->GetBinContent(_bin);
        if (_option.Contains("effr") && ((_val < 0) || (_val > 1))) MessageSvc::Warning("GetHistogramVal", _histo->GetName(), "bin " + to_string(_bin) + " with (X,Y) = (" + to_string(_varX) + "," + to_string(_varY) + ") and value =", to_string(_val));
        if (_option.Contains("ratior") && (_val < 0)) MessageSvc::Warning("GetHistogramVal", _histo->GetName(), "bin " + to_string(_bin) + " with (X,Y) = (" + to_string(_varX) + "," + to_string(_varY) + ") and value =", to_string(_val));
    }
    //if (_val < 0.) _val = 0.;
    //if (_val > 1.) _val = 1.;
    return _val;
}
double GetHistogramVal(TH3 * _histo, double _varX, double _varY, double _varZ, TString _option) {
    // careful, this does not catch entries outside of [0,1]!
    // also the TH3 interpolate needs the values to be between between bin centers of the first and last bins in the cube
    double _val = 1;
    if (_histo != nullptr) {
	if (_varX <= _histo->GetXaxis()->GetBinCenter( 1 ))                 _varX = _histo->GetXaxis()->GetBinCenter(1) + _histo->GetXaxis()->GetBinWidth(1) / 100.;
	if (_varX >= _histo->GetXaxis()->GetBinCenter(_histo->GetNbinsX())) _varX = _histo->GetXaxis()->GetBinCenter(_histo->GetNbinsX()) - _histo->GetXaxis()->GetBinWidth(_histo->GetNbinsX()) / 100.;
	if (_varY <= _histo->GetYaxis()->GetBinCenter( 1 ))                 _varY = _histo->GetYaxis()->GetBinCenter(1) + _histo->GetYaxis()->GetBinWidth(1) / 100.;
	if (_varY >= _histo->GetYaxis()->GetBinCenter(_histo->GetNbinsY())) _varY = _histo->GetYaxis()->GetBinCenter(_histo->GetNbinsY()) - _histo->GetYaxis()->GetBinWidth(_histo->GetNbinsY()) / 100.;
	if (_varZ <= _histo->GetZaxis()->GetBinCenter( 1 ))                 _varZ = _histo->GetZaxis()->GetBinCenter(1) + _histo->GetZaxis()->GetBinWidth(1) / 100.;
	if (_varZ >= _histo->GetZaxis()->GetBinCenter(_histo->GetNbinsZ())) _varZ = _histo->GetZaxis()->GetBinCenter(_histo->GetNbinsZ()) - _histo->GetZaxis()->GetBinWidth(_histo->GetNbinsZ()) / 100.;

        int _bin = _histo->FindFixBin(_varX, _varY, _varZ);

        if (_option.Contains("INTERP")) {
            _val = _histo->Interpolate(_varX, _varY, _varZ);
            if (_val == 0) {
                _val = _histo->GetBinContent(_bin);
            }
        } else
            _val = _histo->GetBinContent(_bin);
    }
    return _val;
}

double GetHistogramVal_3D(pair<TH1D*, vector<TH2D*>> & _histos, double _varX, double _varY, double _varZ, TString _option) {
    /*
    https://root.cern.ch/doc/master/classTH1.html :
    bin = 0;       underflow bin
    bin = 1;       first bin with low-edge xlow INCLUDED
    bin = nbins;   last bin with upper-edge xup EXCLUDED    
    bin = nbins+1; overflow bin
    */
    Int_t nTracks_slices = -1;
    if (_histos.first != nullptr) {
	nTracks_slices = _histos.first->GetNbinsX();
    }
    else {
	MessageSvc::Error("GetHistogramVal_3D", "nTracks Histogram is nullptr", "EXIT_FAILURE");
    }

    double _val = 0;
    if(_histos.first != nullptr) {
        double slices = _histos.first->GetNbinsX();
        if( nTracks_slices !=  _histos.second.size() || nTracks_slices != _histos.first->GetNbinsX()) {
            MessageSvc::Error("GetHistogramVal_3D", "nTracks bins are incompatible with histogram vector length! Name:", _histos.first->GetName(), "EXIT_FAILURE");
	}

        double z_min = _histos.first->GetXaxis()->GetXmin();
        double z_max = _histos.first->GetXaxis()->GetXmax();
        
        int slice = -1;
        if (_varZ < z_min) {
            slice = 1; //Underflow bin +1
        }
        else if (_varZ >= z_max) {
            slice = slices; //Overflow bin - 1
        }
        else {
            slice = _histos.first->FindFixBin(_varZ);
        }

        if( slice < 0 || slice > nTracks_slices) //N_nTracks to include highest bin
            MessageSvc::Error("Wrong nTracks slice number ", "", "EXIT_FAILURE");
        TH2D * _histo = _histos.second.at(slice-1); //Convert from binning counting to cpp vector counting
        if (_histo != nullptr) {
            if (_varX <= _histo->GetXaxis()->GetBinCenter( 1 ))                 _varX = _histo->GetXaxis()->GetBinCenter(1) + _histo->GetXaxis()->GetBinWidth(1) / 100.;
            if (_varX >= _histo->GetXaxis()->GetBinCenter(_histo->GetNbinsX())) _varX = _histo->GetXaxis()->GetBinCenter(_histo->GetNbinsX()) - _histo->GetXaxis()->GetBinWidth(_histo->GetNbinsX()) / 100.;
            if (_varY <= _histo->GetYaxis()->GetBinCenter( 1 ))                 _varY = _histo->GetYaxis()->GetBinCenter(1) + _histo->GetYaxis()->GetBinWidth(1) / 100.;
            if (_varY >= _histo->GetYaxis()->GetBinCenter(_histo->GetNbinsY())) _varY = _histo->GetYaxis()->GetBinCenter(_histo->GetNbinsY()) - _histo->GetYaxis()->GetBinWidth(_histo->GetNbinsY()) / 100.;

            int _bin = _histo->FindFixBin(_varX, _varY);
            if (_option.Contains("INTERP")) {
                _val = _histo->Interpolate(_varX, _varY);
                if (_val == 0) {
                    _val = _histo->GetBinContent(_bin);
                }
            } else
                _val = _histo->GetBinContent(_bin);
        }
    }
    return _val;
}

void ScaleHistogram(TH1 & _histo, double _norm, double _normE) {
    if (((_norm == 0) || (_norm == -1)) && (_normE == 0)) {
        if (_norm == 0) _norm = 1. / _histo.Integral();
        if (_norm == -1) _norm = 1. / _histo.GetBinContent(_histo.GetMaximumBin());
        _histo.SetMinimum(0);
        _histo.SetMaximum(1);
    }
    int _entries = _histo.GetEntries();
    for (int i = 0; i <= _histo.GetNcells() + 1; ++i) {
        _histo.SetBinError(i, TMath::Sqrt(TMath::Sq(_norm * _histo.GetBinError(i)) + TMath::Sq(_histo.GetBinContent(i) * _normE)));
        _histo.SetBinContent(i, _histo.GetBinContent(i) * _norm);
    }
    _histo.SetEntries(_entries);
    return;
}

TH1 * CopyHist(const TH1 * _hist, bool _empty) noexcept {
    auto    nbinsx = _hist->GetNbinsX();
    auto    upx    = _hist->GetXaxis()->GetBinUpEdge(nbinsx);
    auto    lowx   = _hist->GetXaxis()->GetBinLowEdge(1);
    TString name   = (TString) _hist->GetName() + "_copy";
    TString title  = (TString) _hist->GetTitle() + " copy";
    if (_hist->ClassName() == TString("TH1D") || _hist->ClassName() == TString("TH1F")) {
        TH1D * _newhist = new TH1D(name, title, nbinsx, lowx, upx);
        _hist->Copy(*_newhist);
        if (_empty) {
            _newhist->Reset("ICES");
            // for (int i = 0; i < nbinsx + 1; ++i) {
            //     _newhist->SetBinContent(i + 1, 0);
            //     _newhist->SetBinError(i + 1, 0);
            //     _newhist->SetEntries(0);
            // }
        }
        return _newhist;
    }
    if (_hist->ClassName() == TString("TH2D") || _hist->ClassName() == TString("TH2F")) {
        auto   nbinsy   = _hist->GetNbinsY();
        auto   upy      = _hist->GetYaxis()->GetBinUpEdge(nbinsy);
        auto   lowy     = _hist->GetYaxis()->GetBinLowEdge(1);
        TH2D * _newhist = new TH2D(name, title, nbinsx, lowx, upx, nbinsy, lowy, upy);
        _hist->Copy(*_newhist);
        if (_empty) {
            _newhist->Reset("ICES");
            // for (int i = 0; i < nbinsx + 1; ++i) {
            //     for (int j = 0; j < nbinsy + 1; ++j) {
            //         _newhist->SetBinContent(i + 1, j + 1, 0);
            //         _newhist->SetBinError(i + 1, j + 1, 0);
            //         _newhist->SetEntries(0);
            //     }
            // }
        }
        return _newhist;
    }
    if (_hist->ClassName() == TString("TH2Poly")) {
        TH2Poly * _newhist = (TH2Poly *) _hist->Clone(name);
        // TH2Poly * _newhist = static_cast< TH2Poly * >(_hist->Clone(name));
        if (_empty) {
            /*
            if "ICE" is specified, resets only Integral, Contents and Errors.
            if "ICES" is specified, resets only Integral, Contents, Errors and Statistics This option is used
            if "M" is specified, resets also Minimum and Maximum
            */
            _newhist->Reset("ICES");
            //_newhist->ClearBinContents();
        }
        _newhist->SetNameTitle(name, title);
        //_hist->Copy(*_newhist);
        return _newhist;
    }
    if (_hist->ClassName() == TString("TH3D")) {
        auto nbinsy = _hist->GetNbinsY();
        auto upy    = _hist->GetYaxis()->GetBinUpEdge(nbinsy);
        auto lowy   = _hist->GetYaxis()->GetBinLowEdge(1);

        auto   nbinsz   = _hist->GetNbinsZ();
        auto   upz      = _hist->GetZaxis()->GetBinUpEdge(nbinsz);
        auto   lowz     = _hist->GetZaxis()->GetBinLowEdge(1);
        TH3D * _newhist = new TH3D(title, title, nbinsx, lowx, upx, nbinsy, lowy, upy, nbinsz, lowz, upz);
        _hist->Copy(*_newhist);
        if (_empty) {
            _newhist->Reset("ICES");
            // Loop-wise : copy underflow/overflow bin contents! [ thus 0, 1, ...., nbins ] .
            // for (int i = 0; i < nbinsx + 1; ++i) {
            //     for (int j = 0; j < nbinsy + 1; ++j) {
            //         for (int k = 0; k < nbinsz + 1; ++k) {
            //             _newhist->SetBinContent(i + 1, j + 1, k + 1, 0);
            //             _newhist->SetBinError(i + 1, j + 1, k + 1, 0);
            //             _newhist->SetEntries(0);
            //         }
            //     }
            // }
        }
        return _newhist;
    }
    MessageSvc::Error("CopyHist", "Class", _hist->ClassName(), "not supported", "EXIT_FAILURE");
    return nullptr;
}

TH1 * RandomizeAllEntries(TH1 * _histo, int _seed, TString _option) noexcept {
    TH1 *    _histoR = (TH1 *) _histo->Clone();
    TRandom3 _rnd(((TString) _histo->GetName()).Atoi());
    if (_seed != -1) _rnd.SetSeed(_seed);
    for (int i = 0; i <= _histoR->GetNcells() + 1; ++i) { 
        auto _value = _rnd.Gaus(_histoR->GetBinContent(i), _histoR->GetBinError(i));
        if( _option == "eff"){
            //We are randomizing an efficiency histogram, therefore, the new value filled must remain [0,1] bound
            int trial =0;
            while(_value <0 || _value > 1){
                trial +=1;
                _value = _rnd.Gaus(_histoR->GetBinContent(i), _histoR->GetBinError(i));
                if( trial > 1000) break;
            }
            if(trial >1000 && _value > 1.)  _value = 1.;
            if(trial >1000 && _value < 0.)  _value = 0.;
        }
        if( _option == "effratio"){
            if( _value < 0){
                MessageSvc::Info("Eff Ratio is < 0, forcing to 0");
                _value = 0;
            }
        }
        _histoR->SetBinContent(i, _rnd.Gaus(_histoR->GetBinContent(i), _histoR->GetBinError(i))); 
    }
    return _histoR;
}

TH1 * RoundToIntAllEntries(TH1 * _histo) noexcept {
    if (TString(_histo->ClassName()) == "TH1D" || TString(_histo->ClassName()) == "TH1F") {
        double _mins[100000] = {0};
        for (int i = 1; i <= _histo->GetNbinsX(); ++i) _mins[i - 1] = _histo->GetBinLowEdge(i);

        _mins[_histo->GetNbinsX()] = _histo->GetBinLowEdge(_histo->GetNbinsX() + 1);
        TH1D * _histoI             = new TH1D("", "", _histo->GetNbinsX(), _mins);
        _histoI->SetXTitle(_histo->GetXaxis()->GetTitle());
        _histoI->SetYTitle(_histo->GetYaxis()->GetTitle());
        for (int i = 1; i <= _histoI->GetNbinsX(); ++i) {
            if ((_histo->GetBinContent(i) - (int) _histo->GetBinContent(i)) < 0.5) {
                _histoI->SetBinContent(i, (int) _histo->GetBinContent(i));
            } else {
                _histoI->SetBinContent(i, (int) _histo->GetBinContent(i) + 1);
            }
        }
        _histoI->SetEntries(_histo->GetEntries());
        return _histoI;
    } else {
        MessageSvc::Error("RoundToIntAllEntries for 2-D & 3-D Histograms not supported ! ", "", "EXIT_FAILURE");
    }
    return nullptr;
}

Color_t GetColorFromGradient(int _idx, int _total) {
    int _step = TColor::GetNumberOfColors() / _total;
    return TColor::GetColorPalette(_idx * _step);
}

#endif
