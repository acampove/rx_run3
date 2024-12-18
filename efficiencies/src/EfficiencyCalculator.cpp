#ifndef EFFICIENCYCALCULATOR_CPP
#define EFFICIENCYCALCULATOR_CPP

#include "EfficiencyCalculator.hpp"

#include "SettingDef.hpp"

#include "EfficiencySvc.hpp"
#include "FitterSvc.hpp"
#include "HelperSvc.hpp"
#include "HistogramSvc.hpp"
#include "IOSvc.hpp"
#include "MessageSvc.hpp"
#include "ParserSvc.hpp"

#include "EventType.hpp"
#include "IsoHelper.hpp"

#include "core.h"
#include "zipfor.h"
#include <fmt_ostream.h>
#include <iostream>
#include <vector>

#include "TEfficiency.h"
#include "TFile.h"
#include "TGraphAsymmErrors.h"
#include "TH1.h"
#include "TH2.h"
#include "TH2Poly.h"
#include "TMath.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TObject.h"
#include "TString.h"

EfficiencyCalculator::EfficiencyCalculator() {
    if (SettingDef::debug.Contains("EH")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("EfficiencyCalculator", (TString) "Default");
}

EfficiencyCalculator::EfficiencyCalculator(const EfficiencyCalculator & _efficiencyCalculator) {
    // Assign the Numerator / Denominator EventTypes ...
    if (SettingDef::debug.Contains("EH")) SetDebug(true);
    m_pas          = _efficiencyCalculator.EventPas();
    m_tot          = _efficiencyCalculator.EventTot();
    m_efficiencies = _efficiencyCalculator.Efficiencies();
    MessageSvc::Info("EfficiencyCalculator", (TString) "EfficiencyCalculator");
}

ostream & operator<<(ostream & os, const EfficiencyCalculator & _efficiencyCalculator) {
    os << WHITE;
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "EfficiencyCalculator");
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "efficiencies", to_string(_efficiencyCalculator.Efficiencies().size()));
    MessageSvc::Line(os);
    os << RESET;
    os << "\033[F";
    return os;
}

void EfficiencyCalculator::SetRatio(EventType & _numeratorEventType, EventType & _denominatorEventType) {
    // Assign the Numerator / Denominator EventTypes. One can use here one cut-version vs another cut-version.
    // Rather sub-obtimal tough. Better to do a CutScanner ?
    MessageSvc::Info("EfficiencyCalculator, construct Ratio EventTypes");
    m_tot = _denominatorEventType;
    m_pas = _numeratorEventType;
    m_tot.Init(true);
    m_pas.Init(true);
    MessageSvc::Line();
    cout << RED << "============== NUMERATOR   \n" << m_pas << RESET << endl;
    cout << RED << "---> Numerator   Entries Raw (no Cut/no Weight)  :" << m_pas.GetTupleHolder().GetTuple()->GetEntries() << endl;
    cout << RED << "---> Numerator   Cut + weight applied            :" << m_pas.GetWCut();
    MessageSvc::Line();
    cout << BLUE << "============== DENOMINATOR \n" << m_tot << RESET << endl;
    cout << BLUE << "---> Denominator Entries Raw (no Cut/no Weight)  :" << m_tot.GetTupleHolder().GetTuple()->GetEntries() << endl;
    cout << BLUE << "---> Denominator Cut + weight applied            :" << m_tot.GetWCut();
}

void EfficiencyCalculator::BookEffRatioHist1D(const RooRealVar & _varPas, const RooRealVar & _varTot) noexcept {
    MessageSvc::Info("EfficiencyCalculator::BookEfficiency1D", (TString) fmt::format("[VarNum, VarDen] (1D) = [ {0}, {1} ] ", _varPas.GetName(), _varTot.GetName()));
    m_dataPas.BookVariable(_varPas);
    m_dataTot.BookVariable(_varTot);

    m_varPas.push_back(_varPas);
    m_varTot.push_back(_varTot);
    // TODO : check the 2 variables are fine for efficiencies .... : i.e. same low/up and nBins  to build the histogram
    // m_histoCollector[_varPas->GetName()].numerator = make_pair(new TH1D(_varPas->GetName()+"_pas", _varPas->GetTitle() +" (pas) ", _varPas.GetBins(), _varPas.GetMin(), _varPas.GetMax()),
    //                                                            new TH1D(_varTot->GetName()+"_tot", _varTot->GetTitle() +" (tot) ", _varTot.GetBins(), _varTot.GetMin(), _varPas.GetMax())
    //                                                             );
    //---- we book the hisotgram to evaluate the efficiency over here.
}

void EfficiencyCalculator::SetVariables(RooRealVar _varPas, RooRealVar _varTot, int nBins) {
    // Define the set of variables to use when dealing with efficiencies, will plot efficiency vs the kinematic itself
    if (nBins != 0) {
        _varPas.setBins(nBins);
        _varTot.setBins(nBins);
    }
    MessageSvc::Info("EfficiencyCalculator::SetVariables", (TString) "(Numerator,Denominator) var = " + _varPas.GetName() + "," + _varTot.GetName());
    m_dataPas.BookVariable(_varPas);
    m_dataTot.BookVariable(_varTot);

    m_varPas.emplace_back(_varPas);
    m_varTot.emplace_back(_varTot);
    return;
}

void EfficiencyCalculator::DumpPlots() {
    // TODO rework what to report, the RatioPlot is fine for (Num/Denominator) , the rest is a bit tricky
    TFile _file(m_pas.GetKey() + "_HISTOS.root", "RECREATE");
    for (unsigned i = 0; i < m_varPas.size(); i++) {
        MessageSvc::Line();
        MessageSvc::Info("DumpPlots for ", TString(m_varPas[i].GetName()), "");
        MessageSvc::Line();
        TCanvas * c1 = new TCanvas(Form("c_%i", i));
        c1->Divide(2, 3);
        c1->cd(1);
        m_hPas[i].first->Draw();
        c1->cd(2);
        m_hTot[i].first->Draw();
        c1->cd(3);
        m_hEff[i].first->Draw();
        c1->cd(4);
        // TEfficiencyDrawing
        m_tEff[i].first->SetFillStyle(3005);
        // m_tEff[i].first->SetFillColor(kBlue);
        m_tEff[i].first->SetConfidenceLevel(0.683);   //< 1 sigma confidence level error bar
        // By default it uses the kFCP (=0)(default): using the Clopper-Pearson interval (recommended by PDG) sets kIsBayesian = false see also ClopperPearson
        m_tEff[i].first->Draw("A4");
        c1->Write(m_pas.GetKey() + "Efficiency_" + TString(m_varPas[i].GetName()) + ".root");
        c1->SaveAs(m_pas.GetKey() + "Efficiency_" + TString(m_varPas[i].GetName()) + ".pdf");
        delete c1;
        MakeRatioPlot(m_hPas[i], m_hTot[i], 5, false);
        // abort();
    }
    _file.Close();
}

void EfficiencyCalculator::ComputeEfficiency() {
    MessageSvc::Line();
    MessageSvc::Info("ComputeEfficiency");

    if (m_varPas.size() != m_varTot.size()) MessageSvc::Error("ComputeEfficiency", (TString) "Incompatible Pas and Tot variable size", "EXIT_FAILURE");

    bool _varListContainsEventNumber = false;
    for (const auto & _var : m_varPas) {
        if (_var.GetName() == TString(ALWAYSEXISTINGVAR)) _varListContainsEventNumber = true;
    }
    if (!_varListContainsEventNumber) {
        MessageSvc::Info("Adding ALWAYSEXISTINGVAR", TString(ALWAYSEXISTINGVAR));
        MessageSvc::Line();
        RooRealVar _var(ALWAYSEXISTINGVAR, ALWAYSEXISTINGVAR, 0, 1e9);
        _var.setBins(100);
        BookEffRatioHist1D(_var, _var);
    }
    pair< TCut, TString > _cut_weightNumerator   = make_pair(m_pas.GetCut(), m_pas.GetWeight());
    pair< TCut, TString > _cut_weightDenominator = make_pair(m_tot.GetCut(), m_tot.GetWeight());   // WEIGHT = ("1") for None
    TCut                  _numeratorWeightCut    = m_pas.GetWCut();
    TCut                  _denominatorWeightCut  = m_tot.GetWCut();

    TCut    _pasCut    = m_pas.GetCut();
    TString _pasWeight = m_pas.GetWeight();
    m_dataPas.BookSelection(_pasCut, m_pas.GetKey() + " [Cut]");
    m_dataPas.BookWeight(_pasWeight, m_pas.GetKey() + " [Weight]");

    TCut    _denCut    = m_tot.GetCut();
    TString _denWeight = m_tot.GetWeight();   //+"*(1./nCandidates)";  //maybe here since it's with MultipleCandidates 1/totCandidates as weight, very tiny Nb of multiple candidates in MCDecayTuple!
    m_dataTot.BookSelection(_denCut, m_tot.GetKey() + " [Cut]");
    m_dataTot.BookWeight(_denWeight, m_tot.GetKey() + " [Weight]");

    /*
        The MCDecayTree tuple has multiple candidates as well
        _denominatorWeightCut = (1/totCandidates) * _denominatorWeightCut; //< should work in case we want to drop multiple candidates in the denominator from MCDecayTuple. Reason, there are few mult-cand also in MCDecayTuple
        _denominatorCut = ....hack here for eventual extra truth matching, this will also be the nPas to use in Filtering
    */

    //--- ALL VARIABLES MUST HAVE THE SAME MIN/MAX-NBINS to make possible the "Division"
    for (unsigned i = 0; i < m_varPas.size(); i++) {
        if (m_varPas[i].getMin() != m_varTot[i].getMin()) { MessageSvc::Error("MIN of Num-Den not aligned", "EXIT_FAILURE"); }
        if (m_varPas[i].getMax() != m_varTot[i].getMax()) { MessageSvc::Error("MAX of Num-Den not aligned", "EXIT_FAILURE"); }
        if (m_varPas[i].getBins() != m_varTot[i].getBins()) { MessageSvc::Error("nBINS of Num-Den not aligned", "EXIT_FAILURE"); }
    }

    //--- THE HEAVY JOB , fill all selection status, weight values, variable values in the 2 tuples
    MessageSvc::Warning("PROCESSING NUMERATOR ", fmt::format("{0} {1}", m_pas.GetKey(), m_pas.GetTuple()->GetName()), "");
    m_dataPas.Process(*m_pas.GetTuple());
    MessageSvc::Warning("PROCESSING DENOMINATOR ", fmt::format("{0} {1}", m_pas.GetKey(), m_pas.GetTuple()->GetName()), "");
    m_dataTot.Process(*m_tot.GetTuple());

    for (unsigned i = 0; i < m_varPas.size(); i++) {
        MessageSvc::Info("Pas", &m_varPas[i]);
        MessageSvc::Info("Tot", &m_varTot[i]);
        bool    _eventNumberVar = false;
        TString _name           = "eff_" + m_pas.GetKey();
        if (m_varPas[i].GetName() != TString(ALWAYSEXISTINGVAR)) {
            _name += (TString) "_" + m_varPas[i].GetName();
        } else {
            _eventNumberVar = true;
        }
        //---- we do here the business of getting istograms .... 2 Numerator : RAW-ISO , 2 Denominator RAW-ISO (except for eventNumber Var , always filled)
        TH1D * hhPIso = nullptr;
        TH1D * hhPRaw = nullptr;
        TH1D * hhTRaw = nullptr;
        TH1D * hhTIso = nullptr;

        hhPRaw = m_dataPas.GetHistogram1D(i, 0, 0);   // get for booked-i var (1 cut, 1 weight)
        hhPRaw->SetName(TString(fmt::format("{0}_pas", m_varPas[i].GetName())));
        hhPRaw->SetTitle(TString(fmt::format("{0} {1} (pas)", m_varPas[i].GetTitle(), m_pas.GetKey())));

        hhTRaw = m_dataTot.GetHistogram1D(i, 0, 0);   // get for booked-i var (1 cut, 1 weight)
        hhTRaw->SetName(TString(fmt::format("{0}_tot", m_varTot[i].GetName())));
        hhTRaw->SetTitle(TString(fmt::format("{0} {1} (tot)", m_varTot[i].GetTitle(), m_pas.GetKey())));   // APPEND HERE THE KEY OF THE HISTO

        // double min = std::min(m_varPas[i].getMin(), m_varTot[i].getMin());
        // double max = std::max(m_varPas[i].getMax(), m_varTot[i].getMax());
        // ensure min-max here for the 2 variables num/den
        if (_eventNumberVar) {   // for eventNumber , no need to do anything else
            RooRealVar * _eff = nullptr;
            m_hPas.push_back(make_pair(hhPRaw, hhPIso));
            m_hTot.push_back(make_pair(hhTRaw, hhTIso));
            // do efficiency with the RAW HISTOGRAM
            tuple< RooRealVar *, TEfficiency *, TH1D * > _effCont = GetEfficiencyResults(*hhPRaw, *hhTRaw);
            get< 2 >(_effCont)->SetName((TString) "t" + _name);
            _eff = get< 0 >(_effCont);
            _eff->SetName(_name);
            _eff->SetTitle(_name);
            _eff->setRange(0, 1);
            m_efficiencies[_name.Data()] = _eff;
            m_tEff.push_back(make_pair(get< 1 >(_effCont), nullptr));
            m_hEff.push_back(make_pair(get< 2 >(_effCont), nullptr));
            continue;
        }
        // For all other variables we want to IsoBin the final histogram based on Numerator
        //---- Let's run here the IsoBinner stuff to guarantee a nice view of Numerator/Denominator rendering output, ready for the Note!
        double min   = std::min(m_varPas[i].getMin(), m_varTot[i].getMin());
        double max   = std::max(m_varPas[i].getMax(), m_varTot[i].getMax());
        int    nBins = m_varPas[i].getBins("iso");   // retrieve the bin-scheme for this RooRealVar for IsoBinning
        MessageSvc::Warning("ISOBINNER ROUTINE ON VARIABLE ", (TString) fmt::format("Var : {0} , nBins {1} , [{2},{3}]", m_varPas[i].GetName(), nBins, min, max));

        // Numerator vector<entry> tuple for values of Var-i, Weight-Version0 , Weeight Values0
        vector< double > values_Pas    = m_dataPas.GetVariableValues(i);
        vector< double > weight_Pas    = m_dataPas.GetWeightValues(0);
        vector< bool >   selection_Pas = m_dataPas.GetSelectionValues(0);

        vector< double > values_Tot    = m_dataTot.GetVariableValues(i);
        vector< double > weight_Tot    = m_dataTot.GetWeightValues(0);
        vector< bool >   selection_Tot = m_dataTot.GetSelectionValues(0);

        vector< pair< double, double > > _Pas_ForISOBINNING = {};

        //--- IN case you want to isobin on denominator, exchange code here
        vector< pair< double, double > > _Tot_ForISOBINNING = {};
        double                           sum_Tot_weights    = 0;
        for (auto && entry : zip_range(values_Tot, weight_Tot, selection_Tot)) {
            if (!entry.get< 2 >()) continue;   // this entry has not passed the selection
            _Tot_ForISOBINNING.push_back(make_pair(entry.get< 0 >(), entry.get< 1 >()));
            sum_Tot_weights += entry.get< 1 >();
        }

        // Sort by Values done here. Now Building IsoBinning and then Populate Iso-Bins with comulative equal-Weights
        //----------------- ISOBINNING ROUTINE HERE
        double sum_Pas_weights = 0;
        for (auto && entry : zip_range(values_Pas, weight_Pas, selection_Pas)) {
            if (!entry.get< 2 >()) continue;   // this entry has not passed the selection
            _Pas_ForISOBINNING.push_back(make_pair(entry.get< 0 >(), entry.get< 1 >()));
            sum_Pas_weights += entry.get< 1 >();
        };

        // now scale both vector to same weight-total-sum of saying 1000;
        for (int i_i = 0; i_i < _Pas_ForISOBINNING.size(); ++i_i) { _Pas_ForISOBINNING[i_i].second *= 1000. / sum_Pas_weights; }
        for (int i_i = 0; i_i < _Tot_ForISOBINNING.size(); ++i_i) { _Tot_ForISOBINNING[i_i].second *= 1000. / sum_Tot_weights; }
        vector< pair< double, double > > scaled_entries_isobinning = _Pas_ForISOBINNING + _Tot_ForISOBINNING;
        if (scaled_entries_isobinning.size() != _Pas_ForISOBINNING.size() + _Tot_ForISOBINNING.size()) { MessageSvc::Error("Merged vectors for iso-binning is wrong", "EXIT_FAILURE"); }
        sort(scaled_entries_isobinning.begin(), scaled_entries_isobinning.end(), [&](pair< double, double > & a, pair< double, double > & b) { return a.first < b.first; });
        // The array of boundaries for iso-bins between min/max ( using comulative weights ).
        array< double, MAXNBINS > _boundaries = GetBoundariesForPlot(scaled_entries_isobinning, nBins, min, max);
        cout << RED << "MIN " << min << "MAX " << max << endl;
        _boundaries[0]         = min;
        _boundaries[nBins]     = max;
        _boundaries[nBins + 1] = max + 0.0001;   // just in case
        //-------------------
        hhPIso = new TH1D((TString) fmt::format("{0}_{1}", m_varPas[i].GetName(), "pas-iso"), m_varPas[i].GetTitle(), nBins, _boundaries.data());
        hhTIso = new TH1D((TString) fmt::format("{0}_{1}", m_varTot[i].GetName(), "tot-iso"), m_varTot[i].GetTitle(), nBins, _boundaries.data());
        for (int i_i = 0; i < nBins; i_i++) { cout << "Bin " << i << " [ " << _boundaries[i_i] << ", " << _boundaries[i + 1] << endl; }
        // time to fill...
        for (auto && entry : zip_range(values_Pas, weight_Pas, selection_Pas)) {
            if (!entry.get< 2 >()) continue;   // this entry has not passed the selection
            hhPIso->Fill(entry.get< 0 >(), entry.get< 1 >());
        }
        for (auto && entry : zip_range(values_Tot, weight_Tot, selection_Tot)) {
            if (!entry.get< 2 >()) continue;   // this entry has not passed the selection
            hhTIso->Fill(entry.get< 0 >(), entry.get< 1 >());
        }
        // Surely RAW always done ( see before )
        m_hPas.push_back(make_pair(hhPRaw, hhPIso));
        m_hTot.push_back(make_pair(hhTRaw, hhTIso));
        // Returnt a RooRealVar which is the final efficiency. What to pass here : The numerator Histogram, The denominator Histogram, the TEfficiency Object, the EfficiencyHistogram object
        // Single Efficiency
        RooRealVar *                                 _eff        = nullptr;
        tuple< RooRealVar *, TEfficiency *, TH1D * > _effContIso = GetEfficiencyResults(*hhPIso, *hhTIso);
        tuple< RooRealVar *, TEfficiency *, TH1D * > _effContRaw = GetEfficiencyResults(*hhPRaw, *hhTRaw);
        get< 2 >(_effContIso)->SetName((TString) "t" + _name);
        _eff = get< 0 >(_effContIso);
        _eff->SetName(_name);
        _eff->SetTitle(_name);
        _eff->setRange(0, 1);
        m_efficiencies[_name.Data()] = _eff;
        m_tEff.push_back(make_pair(get< 1 >(_effContRaw), get< 1 >(_effContIso)));
        m_hEff.push_back(make_pair(get< 2 >(_effContRaw), get< 2 >(_effContIso)));

        MessageSvc::Line();
    }
    PrintEfficiencies();
    return;
}

RooRealVar * EfficiencyCalculator::GetEfficiency(TString _name) {
    MessageSvc::Line();
    MessageSvc::Info("EfficiencyCalculator", (TString) "GetEfficiency", _name);

    if (!IsVarInMap(_name.Data(), m_efficiencies)) {
        cout << endl;
        cout << RED;
        PrintPars(m_efficiencies);
        cout << RESET;
        MessageSvc::Error("IsVarInMap", _name, "not in map", "EXIT_FAILURE");
    }
    if (m_efficiencies[_name.Data()] == nullptr) MessageSvc::Error("GetEfficiency", _name, "is nullptr", "EXIT_FAILURE");

    MessageSvc::Info("Efficiency", (RooRealVar *) m_efficiencies[_name.Data()]);
    MessageSvc::Line();
    return (RooRealVar *) m_efficiencies[_name.Data()];
}

void EfficiencyCalculator::ComputeGeneratorEfficiency() {
    MessageSvc::Line();
    MessageSvc::Warning("EfficiencyCalculator", (TString) "ComputeGeneratorEfficiency", "");
    TString      _name = "eff_" + m_pas.GetKey() + "_gen";
    RooRealVar * _eff  = GetGeneratorEfficiencyVar(_name, m_pas.GetProject(), m_pas.GetYear(), m_pas.GetPolarity(), m_pas.GetSample(), m_pas.GetQ2bin(), m_debug);
    _eff->setRange(0, 1);
    _eff->Print();
    // if (SettingDef::Fit::blindEfficiency) m_efficiencies[_name.Data()] = BlindParameter(_eff); ? do we need this ?
    m_efficiencies[_name.Data()] = _eff;
    MessageSvc::Line();
    return;
}

void EfficiencyCalculator::ComputeFilteringEfficiency() {
    MessageSvc::Warning("EfficiencyCalculator", (TString) "ComputeFilteringEfficiency", "");
    TString      _name = "eff_" + m_pas.GetKey() + "_flt";
    RooRealVar * _eff  = GetFilteringEfficiencyVar(_name, m_pas.GetProject(), m_pas.GetYear(), m_pas.GetPolarity(), m_pas.GetSample(), m_debug);
    _eff->setRange(0, 1);
    _eff->Print();
    m_efficiencies[_name.Data()] = _eff;
    // if (SettingDef::Fit::blindEfficiency) m_efficiencies[_name.Data()] = BlindParameter(_eff); ? do we need this ?
    MessageSvc::Line();
    return;
}

void EfficiencyCalculator::PrintEfficiencies() const {
    MessageSvc::Line();
    MessageSvc::Info("PrintEfficiencies", (TString) "nEfficiencies =", to_string(m_efficiencies.size()));
    cout << GREEN;
    // Do not print the efficiency calculated for the final step
    if (m_efficiencies.size() > 0) {
        for (const auto & _par : m_efficiencies) {
            TString _key = _par.first;
            if (_key.Contains("_gen") || _key.Contains("_flt")) {
                // We do print the generator and filtering efficiency
                MessageSvc::Info(_par.first, _par.second);
            } else {
                if (!SettingDef::Fit::blindEfficiency && false) {   //< never ever print it
                    MessageSvc::Info(_par.first, _par.second);
                }
                cout << "UNPRINTED EFFICIENCY WE CALCULATED, THAT'S BLINDED" << endl;
            }
        }
    }
    cout << RESET;
    MessageSvc::Line();
    return;
}

void EfficiencyCalculator::SaveToDisk(TString _nameO) const {
    // We save to disk this Version Of the efficiencies RooRealVars. To Do : implement something which grabs and merge stuff for final Efficiency
    if (!SettingDef::IO::outDir.EndsWith("/")) SettingDef::IO::outDir += "/";
    TString EfficiencyID = m_pas.GetKey();
    TString _name        = SettingDef::IO::outDir + "Efficiency_" + m_pas.GetKey();
    // SaveToLog(_name);    < fix me
    TString _texName  = _name + ".tex";
    TString _fileName = _name + ".root";
    MessageSvc::Line();
    MessageSvc::Info("EfficiencyCalculator", (TString) "SaveToDisk", _name);
    MessageSvc::Line();
    if (IOSvc::ExistFile(_fileName)) {
        MessageSvc::Warning("MUST BE NEW FILE --> Trying to overwrite an existing one");
        MessageSvc::Error("SaveToDisk", TString("Efficiencies already saved for this EventType, remove them"), "EXIT_FAILURE");
    }
    TFile     _oFile(_fileName, to_string(OpenMode::RECREATE));
    RooArgSet params(_name + "_gen");
    for (auto & _var : m_efficiencies) {
        TString _nn = ((RooRealVar *) _var.second)->GetName();
        _var.second->Print("v");
        _var.second->Write(_nn, TObject::kOverwrite);
        params.add(*_var.second);
    }
    params.printLatex(OutputFile(_texName));
    return;
}

#endif
