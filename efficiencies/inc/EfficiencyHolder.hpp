#ifndef EFFICIENCYHOLDER_HPP
#define EFFICIENCYHOLDER_HPP

#include "EfficiencySvc.hpp"
#include "HelperSvc.hpp"
#include "HistogramSvc.hpp"
#include "IOSvc.hpp"
#include "MessageSvc.hpp"
#include "ParserSvc.hpp"

#include "EventType.hpp"

#include <iostream>
#include <vector>

#include "TCanvas.h"
#include "TEfficiency.h"
#include "TFile.h"
#include "TGraphAsymmErrors.h"
#include "TH1.h"
#include "TMath.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TObject.h"
#include "TString.h"

#include "RooRealVar.h"

/**
 * \class EfficiencyHolder
 * \brief Efficiency info
 */
class EfficiencyHolder : public TObject {

  public:
    /**
     * \brief Default constructor
     */
    EfficiencyHolder();

    /**
     * \briefConstructor with EventType and TString
     */
    EfficiencyHolder(EventType & _eventType, TString _option);

    EfficiencyHolder(EventType & _eventTypePas, EventType & _eventTypeTot, TString _option);

    /**
     * \brief Copy constructor
     */
    EfficiencyHolder(const EfficiencyHolder & _efficiencyHolder);

    /**
     * \brief Get option used to generate Efficiency
     */
    const TString Option() const { return m_option; };

    /**
     * \brief Init Efficiency
     */
    void Init();

    /**
     * \brief Get EventPas
     */
    const EventType EventPas() const { return m_pas; };

    /**
     * \brief Get EventTot
     */
    const EventType EventTot() const { return m_tot; };

    void SetVariables(RooRealVar _varPas, RooRealVar _varTot, int nBins = 0);

    const Str2VarMap Efficiencies() const { return m_efficiencies; };

    void PrintEfficiencies();

    void PlotEfficiencies(TString _name = "");

    /**
     * \brief Compute Efficiency
     */
    void ComputeEfficiency();

    /**
     * \brief Get Efficiency
     * @param _name [description]
     */
    RooRealVar * GetEfficiency(TString _name);

    /**
     * \brief Save EfficiencyHolder to disk
     */
    void SaveToDisk(TString _name = "");

    /**
     * \brief Load EfficiencyHolder from disk
     */
    void LoadFromDisk(TString _name = "", TString _dir = "");

    /**
     * \brief Save EfficiencyHolder to log
     */
    void SaveToLog(TString _name = "");

  private:
    TString m_option;

    EventType m_pas;
    EventType m_tot;

    TString m_var = "eventNumber";

    vector< RooRealVar > m_varPas = {};
    vector< RooRealVar > m_varTot = {};

    vector< TH1D > m_hPas = {};
    vector< TH1D > m_hTot = {};

    vector< TH1D >        m_hEff = {};
    vector< TEfficiency > m_tEff = {};

    Str2VarMap m_efficiencies;

    /**
     * \brief Check allowed arguments
     * @param  _option [description]
     */
    bool Check(TString _option);

    bool m_debug = false;
    /**
     * \brief Activate debug
     * @param _debug [description]
     */
    void SetDebug(bool _debug) { m_debug = _debug; };

    /**
     * \brief Get Efficiency
     * @param  _hPas   [description]
     * @param  _hTot   [description]
     * @param  _tEff   [description]
     * @param  _hEff   [description]
     * @param  _option [description]
     */
    RooRealVar * GetEfficiency(TH1D _hPas, TH1D _hTot, TEfficiency & _tEff, TH1D & _hEff, TString _option = "");

    /**
     * \brief Get Efficiency
     * @param _nPas [description]
     * @param _nTot [description]
     */
    RooRealVar * GetEfficiency(double _nPas, double _nTot);

    ClassDef(EfficiencyHolder, 1);
};

ostream & operator<<(ostream & os, const EfficiencyHolder & _efficiencyHolder);

#endif
