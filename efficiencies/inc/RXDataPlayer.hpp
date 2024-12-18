#ifndef RXDATAPLAYER_HPP
#define RXDATAPLAYER_HPP

#include "IsoHelper.hpp"

#include <map>
#include <tuple>

#include "TString.h"

class TTreeFormula;
class TH1D;
class TH2D;
class TProfile;
class TRatioPlot;
class TChain;

using namespace std;

/* By Renato Quagliani , Simone Bifani, Stephan Sescher
 * (rquaglia@cern.ch , Simone.Bifani@cern.ch , stephan.escher@cern.ch )
 * February 2018
 *  [v0 for RX analysis in LHCB software]
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
/**
 * \brief      Class for order in the tuple< Var,Sel,Weight> when wrapping the containers
 */
enum class Order : int { Selection = 0, Weight = 1, Variable = 2 };

/**
 * \brief      Class housing infos for the rx weight processing/plotting
 */
class RXWeight {
  public:
    /**
     * \brief Default constructor
     */
    RXWeight(){};

    /**
     * \brief      Constructs the object with an IDX, an Expression, The Actual Label
     *
     * @param[in]  _idx        The index
     * @param[in]  _expression The weight expression
     * @param[in]  _label      The weight label
     */
    RXWeight(int _idx, TString _expression, TString _label) {
        m_idx       = _idx;
        m_weightExp = _expression;
        m_weightLab = _label;
    };

    /**
     * \brief The WeightExpression to use in TTreeFormula when loading the value
     */
    TString m_weightExp;

    /**
     * \brief The WeightLabel associated to this Weight, it will populate the final Histogram
     */
    TString m_weightLab;

    /**
     * \brief The unique IDX identifying this object, the idx is used such that Weight1 == Weight2 if idx == idx, useful to store map and retrieve object associated to a given objec
     */
    int m_idx;

    /**
     * \brief      Returnm this oject Weight Expression
     *
     * @return     { description_of_the_return_value }
     */
    TString Expression() const { return m_weightExp; };

    /**
     * \brief      Return the label of This Object
     *
     * @return     { description_of_the_return_value }
     */
    TString Label() const { return m_weightLab; };

    /**
     * \brief      Print for Debugging
     */
    void Print() const {
        MessageSvc::Line();
        MessageSvc::Info("Weight", (TString) "ID =", to_string(Index()));
        MessageSvc::Info("Weight", (TString) "Label =", m_weightLab);
        MessageSvc::Info("Weight", (TString) "Expression =", m_weightExp);
        MessageSvc::Line();
    };

    /**
     * \brief      The Weeight Index
     *
     * @return    the Weight INdex
     */
    int Index() const { return m_idx; };

    bool operator<(const RXWeight & rhs) const { return m_idx < rhs.Index(); };

    bool operator==(const RXWeight & rhs) { return m_idx == rhs.Index(); };
};

/**
 * \brief      Class housing info for the rx selection to apply when plotting
 */
class RXSelection {
  public:
    /**
     * \brief Default constructor
     */
    RXSelection(){};

    /**
     * \brief      Constructs the object. with IDX, the actual selection and the SelectionLabel
     *
     * @param[in]  _idx        The index
     * @param[in]  _expression The rx selection
     * @param[in]  _label      The rx selection label
     */
    RXSelection(int _idx, TCut _expression, TString _label) {
        m_idx          = _idx;   // idx*MAXSLOTS;
        m_selectionExp = _expression;
        m_selectionLab = _label;
    };

    /**
     * \brief The actual cut to apply
     */
    TCut m_selectionExp;

    /**
     * \brief The label for the cut
     */
    TString m_selectionLab;

    /**
     * \brief The unique IDX identifying this Cut in a given sequence of cuts
     */
    int m_idx;

    /**
     * \brief      Cut Index of this Selection
     *
     * @return     The cut index of the selection
     */
    int Index() const { return m_idx; };

    /**
     * \brief      Print this RXSelection
     */
    void Print() const {
        MessageSvc::Line();
        MessageSvc::Info("Selection", (TString) "ID =", to_string(Index()));
        MessageSvc::Info("Selection", (TString) "Label =", m_selectionLab);
        MessageSvc::Info("Selection", (TString) "Expression =", TString(m_selectionExp));
        MessageSvc::Line();
    };

    /**
     * \brief      Expression Getter
     *
     * @return     Get The Expression in form of TString
     */
    TString Expression() const { return (TString) m_selectionExp; };

    /**
     * \brief      Label Getter
     *
     * @return     Get the Label in form of TString
     */
    TString Label() const { return m_selectionLab; };

    /**
     * \brief      Operator equality
     * @return     true if they have the same internal Index
     */
    bool operator==(const RXSelection & a) { return m_idx == a.Index(); };

    /**
     * \brief      Operator needed when storing map[RXSelection, ...]; order in map is given by the < operator, thus the idx sorting
     *
     * @param[in]  rhs  Another RXSelection
     *
     * @return     the idx comparison
     */
    bool operator<(const RXSelection & rhs) const { return m_idx < rhs.Index(); };
};

class RXVarPlot {
  public:
    /**
     * \brief Default constructor
     */
    RXVarPlot(){};

    /**
     * \brief      Constructs the object.
     *
     * @param[in]  _idx         The index to assign
     * @param[in]  _expression  The variable expression
     * @param[in]  _label       The variable label
     * @param[in]  _nBins       The bins
     * @param[in]  _min         The minimum
     * @param[in]  _max         The maximum
     * @param[in]  _units       The units
     * @param[in]  _doIsoBinned The do iso binned [not handled in RXDataPlayer]
     * @param[in]  _useLogY     The use y log [not supported]
     */

    RXVarPlot(int _idx, TString _expression, TString _label, int _nBins, double _min, double _max, TString _units = "", bool _doIsoBinned = false, bool _useLogY = false) {
        m_idx         = _idx;
        m_varExp      = _expression;
        m_varLab      = _label != "" ? _label : _expression;
        m_nBins       = _nBins;
        m_min         = _min;
        m_max         = _max;
        m_units       = _units;
        m_useLogY     = _useLogY;
        m_doIsobinned = _doIsoBinned;
    };
    TString m_varExp;
    TString m_varLab;
    TString m_units;
    int     m_idx;
    bool    m_useLogY;
    bool    m_doIsobinned;
    double  m_min;
    double  m_max;
    int     m_nBins;
    /**
     * \brief      GetExpression to use in Draw
     *
     * @return     The Expression for Drawing this RXVarPlot
     */
    TString Expression() const { return m_varExp; };

    /**
     * \brief      The Label (used to assign x-axis name)
     *
     * @return     The Label of this Variable
     */
    TString Label() const { return m_varLab; };

    /**
     * \brief      The units in which this observable is looked for (attached to y and x axis)
     *
     * @return     { description_of_the_return_value }
     */
    TString Units() { return m_units; };

    /**
     * \brief      The nBins one wants to use
     *
     * @return     The number of bins
     */
    int NBins() { return m_nBins; };

    /**
     * \brief      Min boundary of this variable
     *
     * @return     the min
     */
    double Min() { return m_min; };

    /**
     * \brief      Max boundary of this variable
     *
     * @return     the max
     */
    double Max() { return m_max; };

    /**
     * \brief      Logs on y-axis for plot?
     *
     * @return     doLogY on canvas?
     */
    bool LogY() { return m_useLogY; };

    /**
     * \brief      IsoBin this observable?
     *
     * @return     True/False
     */
    bool IsoBin() { return m_doIsobinned; };

    /**
     * \brief      The INdex of this RXVarPlot
     *
     * @return     The Index used for bookkeping in RXDataPlayer
     */
    int Index() const { return m_idx; };

    /**
     * \brief      Print this RXVarPlot
     */
    void Print() const {
        MessageSvc::Line();
        MessageSvc::Info("Variable", (TString) "ID =", to_string(Index()));
        MessageSvc::Info("Variable", (TString) "Label =", m_varLab, m_units);
        MessageSvc::Info("Variable", (TString) "Expression =", m_varExp);
        MessageSvc::Info("Variable", (TString) "nBins[min,max] =", to_string(m_nBins), "[", to_string(m_min), ",", to_string(m_max), "]");
        MessageSvc::Info("Variable", (TString) "iso =", to_string(m_doIsobinned));
        MessageSvc::Info("Variable", (TString) "logY =", to_string(m_useLogY));
        MessageSvc::Line();
    };

    /**
     * \brief      Oparator equality
     *
     * @param[in]  a     one RXVarPlot
     * @param[in]  b     one RXVarPlot
     *
     * @return     equal if they share the smae index
     */
    bool operator==(const RXVarPlot & a) { return m_idx == a.Index(); };

    /**
     * \brief      Operator <, useful for map<RXVarPlot,...>
     *
     * @param[in]  rhs   Another RXVarPlot to compare with
     *
     * @return     Index comparison
     */
    bool operator<(const RXVarPlot & rhs) const { return m_idx < rhs.Index(); };
};

/**
 * \brief      The RXDataPlayer handling and bookkeping histograms-cuts-selections to look at for a given input TChain
 */
class RXDataPlayer {
  public:
    /**
     * \brief Default constructor
     */
    RXDataPlayer();

    /**
     * \briefConstructor with TString
     */
    RXDataPlayer(TString _name);

    /**
     * @brief      Set a cut to filter before doing all the job after
     *
     * @param[in]  _cut  The cut
     * @param[in]  _var  The variable
     */
    void SetBaseCut(TString _cut) {
        if (_cut != "") m_cutUpFront = _cut;
        return;
    }

    void SetName(TString _name) { m_name = _name; };

    TString GetName() { return m_name; };

    /**
     * \brief BookVariable
     *
     * @param[in]  _expression  The variable expression to use in Drawing
     * @param[in]  _label       The variable label to attach to the X-Axis
     * @param[in]  _nBins       The bins on which to plot
     * @param[in]  _min         The minimum on which to plot
     * @param[in]  _max         The maximum on which to plot
     * @param[in]  _units       The units to use on this variable
     * @param[in]  _isoBin      The iso bin flag [not used yet]
     * @param[in]  _useLogY     The use logy [not used yet, no "full-plot report available at the moment"]
     */
    void BookVariable(TString _expression, TString _label, int _nBins, double _min, double _max, TString _units = "", bool _isoBin = false, bool _useLogY = false);

    /**
     * \brief      BookVariable passing a RooRealVar, we will use the name, title, nBins, units to book the histogram
     *
     * @param[in]  _var   The variable
     */
    void BookVariable(const RooRealVar & _var);

    /**
     * \brief      BookVariable passing a RXVarPlot to be parsed
     *
     * @param[in]  _var   The RXVar
     */
    void BookVariable(RXVarPlot _var);

    /**
     * \brief      BookVariable passing a fileName to be parsed
     *
     * @param[in]  _var   The file name
     */
    void BookVariables(TString _filaName = "");

    /**
     * \brief      BookVariable passing a vector
     *
     * @param[in]  _variables   The vector
     */
    void BookVariables(vector< vector< TString > > _variables);

    // void BookVariable(TString _expression, int nBins, double min, double max  ,TString units, bool isoBin = false, bool _useLogY = false);
    /**
     * \brief      Add a Selection Switcher for all the variables in the pool
     *
     * @param[in]  _selection  The selection expression (TCut)
     * @param[in]  _label      The label of the selection
     */
    void BookSelection(TCut _selection, TString _label);

    /**
     * \brief      BookWeight
     *
     * @param[in]  _weight The weight
     * @param[in]  _label  The rx weight label
     */
    void BookWeight(TString _weight, TString _label);

    /**
     * \brief      Gets the n variable plot booked internally.
     *
     * @return     The n variables to plot.
     */
    int GetNVariables() const { return m_nVars; };

    /**
     * \brief      Gets the n selections one has booked internally
     *
     * @return     The n selections.
     */
    int GetNSelections() const { return m_nSelections; };

    /**
     * \brief      Gets the n weight one has booked internally.
     *
     * @return     The n weight exponent.
     */
    int GetNWeights() const { return m_nWeights; };

    /**
     * \brief      Gets the vector<values> of the Observable added at the INDEX = _idx
     *
     * @param[in]  _idx  The index variable to retrieve the vector<double>, having size = nEntries Tuple processed
     *
     * @return     The vector<double> sorted by "entry" processing order.
     */

    vector< double > GetVariableValues(int _idx);
    vector< double > GetVariableValues(TString _labelVar);

    /**
     * \brief      Gets the name of the variable with index.
     *
     * @return     The name of variable.
     */
    TString GetVariableName(int _idx) { return m_bookedVariable.at(_idx).Expression(); };
    TString GetVariableLabel(int _idx) { return m_bookedVariable.at(_idx).Label(); };

    TString GetSelectionName(int _idx) { return m_bookedSelections.at(_idx).Label(); };
    TString GetSelectionLabel(int _idx) { return m_bookedSelections.at(_idx).Label(); };

    TString GetWeightName(int _idx) { return m_bookedWeights.at(_idx).Expression(); };
    TString GetWeightLabel(int _idx) { return m_bookedWeights.at(_idx).Label(); };

    /**
     * \brief      Gets the vector<bool> associated to the booked selection with INDEX = _idx.
     *
     * @param[in]  _idx  The index of the selection to retrieve
     *
     * @return     The vector<bool> sorted by "entry" processing order.
     */
    vector< bool > GetSelectionValues(int _idx);
    vector< bool > GetSelectionValues(TString _selectionName);

    /**
     * \brief      Gets the vector<double> associated to the booked weight with INDEX = _idx.
     *
     * @param[in]  _idx  The index weight
     *
     * @return     The vector<double> sorted by "entry" processing order.
     */
    vector< double > GetWeightValues(int _idx);
    vector< double > GetWeightValues(TString _weightVar);

    void AddAlias(TString _alias, TString _expression) { m_aliases.push_back(std::make_pair(_alias, _expression)); }

    void AddAliases(vector< pair< TString, TString > > & _aliases) {
        for (auto & a : _aliases) { AddAlias(a.first, a.second); }
    }
    /**
     * \brief      Print the Booked Stuff
     */
    void Print() {
        MessageSvc::Line();
        MessageSvc::Info("RXDataPlayer", m_name);
        MessageSvc::Line();
        MessageSvc::Info("Booked Variable(s)", to_string(GetNVariables()));
        for (const auto & vv : m_bookedVariable) { vv.second.Print(); }
        MessageSvc::Info("Booked Selection(s)", to_string(GetNSelections()));
        for (const auto & vv : m_bookedSelections) { vv.second.Print(); }
        MessageSvc::Info("Booked Weight(s)", to_string(GetNWeights()));
        for (const auto & vv : m_bookedWeights) { vv.second.Print(); }
        MessageSvc::Line();
    };

    /**
     * \brief      Builds the cross Product of combinations, Selection, Weights : size nSel x nObs x nWeights
     *
     * @return     The combination.
     */
    vector< tuple< RXSelection, RXWeight, RXVarPlot > > BuildCombination();

    /**
     * \brief      Process This
     *
     * @param      _tuple  The input TChain to process
     */
    void Process(TChain & _tuple) noexcept;

    /**
     * \brief      Build and return an Histogram with the Booked properties of RXSelection bookkeped for the Variable(_idx_var), with Cut(_idx_cut) and Weight(_idx_weight)
     *
     * @param[in]  _idx_var     The index variable
     * @param[in]  _idx_cut     The index cut
     * @param[in]  _idx_weight  The index weight
     *
     * @return     The histogram.
     */
    TH1D * GetHistogram1D(int _idx_var, int _idx_cut, int _idx_weight, EColor _color = kBlack);
    TH1D * GetHistogram1D(TString _name_var, TString _name_cut, TString _name_weight, EColor _color = kBlack);

    TRatioPlot * GetHistogramRatio1D(int _idx1_var, int _idx1_cut, int _idx1_weight, int _idx2_var, int _idx2_cut, int _idx2_weight, TString _option = "");
    TRatioPlot * GetHistogramRatio2D(pair< int, int > _idx_XY_var, int _idxNum_cut, int _idxNum_weight, int _idxDen_cut, int _idxDen_weight, TString _option = "");

    /**
     * \brief SaveHistogram1D
     * @param _option     s: superimpose different selections with the same weight; w: superimpose different weights with the same selection
     * @param _idx_cut    [description]
     * @param _idx_weight [description]
     */
    void SaveHistogram1D(TString _option, int _idx_cut = 0, int _idx_weight = 0);

    /**
     * \brief      Gets the histogram 2D
     *
     * @param[in]  _idx_varX    The index of  variable for x-axis
     * @param[in]  _idx_varY    The index of  variable for y-axis
     * @param[in]  _idx_cut     The index of the selection to apply cut
     * @param[in]  _idx_weight  The index of the weight to apply
     *
     * @return     The histogram built with the specifics (bins wrapped from the "binner"); TODO : optional nBinsX, nBinsY to overload the current values
     */
    TH2D * GetHistogram2D(pair< int, int > _idx_var, int _idx_cut, int _idx_weight, EColor _color = kBlack);

    /**
     * \brief SaveHistogram2D
     * @param _option     s: superimpose different selections with the same weight; w: superimpose different weights with the same selection
     * @param _idx_cut    [description]
     * @param _idx_weight [description]
     */
    void SaveHistogram2D(TString _option, int _idx_cut = 0, int _idx_weight = 0);

    /**
     * \brief      Gets the histogram Profile
     *
     * @param[in]  _idx_varX    The index of  variable for x-axis
     * @param[in]  _idx_varY    The index of  variable for y-axis
     * @param[in]  _idx_cut     The index of the selection to apply cut
     * @param[in]  _idx_weight  The index of the weight to apply
     *
     * @return     The histogram built with the specifics (bins wrapped from the "binner"); TODO : optional nBinsX, nBinsY to overload the current values
     */
    TProfile * GetHistogramProfile(pair< int, int > _idx_var, int _idx_cut, int _idx_weight, EColor _color = kBlack, TString _option = "");
    TProfile * GetHistogramProfile(pair< TString, TString > _name_var, TString _name_cut, TString _name_weight, EColor _color = kBlack, TString _option = "");

    /**
     * \brief SaveHistogramProfile
     * @param _option     s: superimpose different selections with the same weight; w: superimpose different weights with the same selection
     * @param _idx_cut    [description]
     * @param _idx_weight [description]
     */
    void SaveHistogramProfile(TString _option, int _idx_cut = 0, int _idx_weight = 0);

    /**
     * \brief      Clean-Up all maps, restore to default, can re-use same object.
     */
    void Reset() {
        m_bookedWeights.clear();
        m_bookedSelections.clear();
        m_bookedVariable.clear();
        m_bookedWeightResults.clear();
        m_bookedSelectionResults.clear();
        m_bookedVariableValues.clear();
        m_nSelections = 0;   // 0-99
        m_nWeights    = 0;   // 0-99
        m_nVars       = 0;   // 0-99
        m_CachedHistoPool.clear();
        m_CachedHisto2DPool.clear();
        return;
    };

    /**
     * \brief      Clean-up the stored from processed TChain, one can ClearContent and re-fill from TChain.
     */
    void ClearContent() {
        m_bookedWeightResults.clear();
        m_bookedSelectionResults.clear();
        m_bookedVariableValues.clear();
    };

    void SaveAll(TString _fileName = "");

    TString CleanUpName(TString & string) {
        // Handy function to clean-up the name of the plot removing all brackets TMath:: product, minus, plus
        TString _cp = string;
        _cp.ReplaceAll("(", "_").ReplaceAll("TMath::", "").ReplaceAll(")", "").ReplaceAll("{", "").ReplaceAll("}", "").ReplaceAll("*", "_times_").ReplaceAll("/", "_").ReplaceAll(",2", "").ReplaceAll(",", "").ReplaceAll("1", "").ReplaceAll("(double)", "").ReplaceAll("+", "_plus_").ReplaceAll("-", "_minus_");
        return _cp;
    };

  private:
    vector< pair< TString, TString > > m_aliases = {};
    map< int, RXWeight >               m_bookedWeights;
    map< int, RXSelection >            m_bookedSelections;
    map< int, RXVarPlot >              m_bookedVariable;

    map< RXWeight, vector< double > >  m_bookedWeightResults;
    map< RXSelection, vector< bool > > m_bookedSelectionResults;
    map< RXVarPlot, vector< double > > m_bookedVariableValues;

    int m_nSelections = 0;
    int m_nWeights    = 0;
    int m_nVars       = 0;

    TString m_cutUpFront = "(1>0)";

    TString m_name = "RXDataPlayer";

    bool m_debug = false;
    /**
     * \brief Activate debug
     * @param _debug [description]
     */
    void SetDebug(bool _debug) { m_debug = _debug; };

  public:
    map< tuple< int, int, int >, TH1D >                        m_CachedHistoPool;
    map< tuple< pair< int, int >, int, int >, TH2D >           m_CachedHisto2DPool;
    map< tuple< pair< int, int >, int, int, int, int >, TH2D > m_CachedRatioPlot2D;
};

#endif
