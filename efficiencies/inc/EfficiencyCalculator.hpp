#ifndef EFFICIENCYCALCULATOR_HPP
#define EFFICIENCYCALCULATOR_HPP

#include "EventType.hpp"
#include "RXDataPlayer.hpp"
#include "RooRealVar.h"

class RooRealVar;
class TH1D;
class TH2;
class TH2D;
class TEfficiency;

/**
 * \brief      Struct Holding the plotting INformation for Numerator/Denominator evaluation
 */
struct PlotInfo {
    TString nameNum;      // The Name Of the Expression to use at the Numerator
    TString nameDen;      // The Name of the Expression to use at the Denominator
    double  min;          // The Min of the Observable Expression
    double  max;          // The Max of the Observable Expression
    int     nbins;        // The nBins to use
    int     nbins_iso;    // Nbins for Iso-Binning to use when doing ratio of them inside EfficiencyCalculator
    TString unit;         // The unit of the variable ["MeV", "mm", "ertc.."]
    TString latexTitle;   // The title to populate the x-Axis on the ratio
    bool    doyLog;       // The Y-Log Scale( not yet fully implemented )
    /**
     * \brief      Constructor
     *
     * @param[in]  _exprNum     The expression for numerator
     * @param[in]  _exprDen     The expression for denominator
     * @param[in]  _min         The minimum over which the plot will be created
     * @param[in]  _max         The maximum over which the plot will be created
     * @param[in]  _nbins       The nbins for the raw plot
     * @param[in]  _nbinsIso    The nbins iso for the isobinning routine
     * @param[in]  _unit        The units of this observable
     * @param[in]  _latexTitle  The latex title to assing on x-axis
     * @param[in]  _doLogy      The do logy, enable logY plot
     */
    PlotInfo(TString _exprNum, TString _exprDen, double _min, double _max, int _nbins, int _nbinsIso, TString _unit, TString _latexTitle, bool _doLogy) {
        nameNum    = _exprNum;
        nameDen    = _exprDen;
        min        = _min;
        max        = _max;
        nbins      = _nbins;
        nbins_iso  = _nbinsIso;
        unit       = _unit;
        latexTitle = _latexTitle;
        doyLog     = _doLogy;
    }
    // Produce a Pair of RooRealVar to use to instruct Efficiency Calculation return < Numerator, Denominator> RooRealVars
    pair< RooRealVar, RooRealVar > MakeRooRealVar_ForEFFS() {
        RooRealVar nn(nameNum, latexTitle, min, max, unit);
        nn.setBins(nbins);
        nn.setBins(nbins_iso, "iso");
        RooRealVar dd(nameDen, latexTitle, min, max, unit);
        dd.setBins(nbins);
        dd.setBins(nbins_iso, "iso");
        return make_pair(nn, dd);
    }
};

//---- eventNumber is a valid Branch for ANY TUPLE in LHCb, which can be easily used to compute integrated efficiencies in a single bin! We use this
constexpr char ALWAYSEXISTINGVAR[] = "eventNumber*1.";

/**
 * \class EfficiencyCalculator
 * \brief Efficiency info
 */
class EfficiencyCalculator {

  public:
    /**
     * \brief      Constructs the object.
     */
    EfficiencyCalculator();

    /**
     * \brief      Sets the ratio Info for the 2 EventTypes
     *
     * @param      _numeratorEventType    The numerator   event type (Cut + Weights are implicitly used)
     * @param      _denominatorEventType  The denominator event type (Cut + Weights are implicitly used)
     */
    void SetRatio(EventType & _numeratorEventType, EventType & _denominatorEventType);

    /**
     * \brief      Constructs the object (Copy of it)
     *
     * @param[in]  _efficiencyCalculator  The efficiency holder to Copy
     */
    EfficiencyCalculator(const EfficiencyCalculator & _efficiencyCalculator);

    /**
     * \brief      Get the Numerator EventType
     *
     * @return    Return the underlying EventType
     */
    const EventType EventPas() const { return m_pas; };

    /**
     * \brief      Get the Denominator EventType
     *
     * @return     Return the EventTot
     */
    const EventType EventTot() const { return m_tot; };

    /**
     * \brief      Book Efficiency For 1D plots of Efficiency vs Variable (num Var can differ from name in Denominator Var)
     *
     * @param[in]  _varPas  The variable For Numerator , put for them same nBins, min, max
     * @param[in]  _varTot  The variable For Denominator, put for them same nBins, min, max
     *
     */
    void BookEffRatioHist1D(const RooRealVar & _varPas, const RooRealVar & _varTot) noexcept;

    /**
     * \brief      Sets internal Variable to plot against the Efficiency (1D only)
     *
     * @param[in]  _varPas  The variable pas
     * @param[in]  _varTot  The variable total
     * @param[in]  nBins    The bins
     */
    void SetVariables(RooRealVar _varPas, RooRealVar _varTot, int nBins = 0);

    /**
     * \brief      Return the map <String, RooRealVar > of stored Efficiencies
     *
     * @return     { description_of_the_return_value }
     */
    const Str2VarMap Efficiencies() const { return m_efficiencies; };

    /**
     * \brief      Print on screen the efficiencies
     */
    void PrintEfficiencies() const;

    /**
     * \brief Compute Efficiency, populating histograms and producing final Efficiency Value to save on Disk (later re-loaded on Fitter)
     */
    void ComputeEfficiency();

    /**
     * \brief      Retrieve the Efficiency of This from teh Str2VarMap
     *
     * @param[in]  _name  The name which has bookkeped the efficiency
     *
     * @return     The efficiency RooRealVar
     */
    RooRealVar * GetEfficiency(TString _name);

    /**
     * \brief      Saves to disk this object
     *
     * @param[in]  _name  The name
     */
    void SaveToDisk(TString _name = "") const;

    /**
     * \brief      Loads a from disk this Object
     *
     * @param[in]  _name  The name
     * @param[in]  _dir   The dir
     */
    void LoadFromDisk(TString _name = "", TString _dir = "");
    /**
     * \brief      Saves to log.
     *
     * @param[in]  _name  The name
     */
    void SaveToLog(TString _name = "") const;

    /**
     * \brief      Calculates the generator efficiency, used in tandem also weighting for Luminosity !
     */
    void ComputeGeneratorEfficiency();

    /**
     * \brief      Calculates the generator efficiency, used in tandem also weighting for Luminosity !
     */
    void ComputeFilteringEfficiency();

    /**
     * \brief      Dumps plots :  Numerator, Denominator, Numerator-Binning, TEfficiency Numerator/Denominator (iso-binned)
     */
    void DumpPlots();

    /**
     * \brief      Dumps a latex table for the evaluated Efficiencies on this calculation
     */
    // void DumpLatexTable();
  private:
    // Numerator EventType to use.
    EventType m_pas;
    // Denominator EventType to use
    EventType m_tot;

    RXDataPlayer m_dataPas;
    RXDataPlayer m_dataTot;

    vector< RooRealVar >                           m_varPas = {};   // List of Variables to use in "Division"
    vector< RooRealVar >                           m_varTot = {};   // List of Variables to use in "Division"
    vector< pair< TH1D *, TH1D * > >               m_hPas   = {};   // < first is the RAW one, second IsoBinned one
    vector< pair< TH1D *, TH1D * > >               m_hTot   = {};   // < first is the RAW one, second IsoBinned one
    vector< pair< TH1D *, TH1D * > >               m_hEff   = {};   // < first is the RAW one, second IsoBinned one
    vector< pair< TEfficiency *, TEfficiency * > > m_tEff   = {};   // < first is the RAW one, second IsoBinned one

    // Final Efficiencies Map to dump on disk
    Str2VarMap m_efficiencies;

    /**
     * \brief Check allowed arguments
     * @param  _option [description]
     */
    bool Check(TString _option) const;

    bool m_debug = false;
    /**
     * \brief Activate debug
     * @param _debug [description]
     */
    void SetDebug(bool _debug) { m_debug = _debug; };
};

ostream & operator<<(ostream & os, const EfficiencyCalculator & _efficiencyCalculator);

#endif
