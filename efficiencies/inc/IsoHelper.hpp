#ifndef ISOHELPER_HPP
#define ISOHELPER_HPP

#include "EventType.hpp"

#include "TCut.h"
#include "TString.h"

constexpr int MAXNBINS = 100000;

// The IsoBinning Making produces IsoBinResults.root file with inside the relevant histograms
const TString outRootFile = "IsoBinResults.root";
// The IsoBinning Making produce IsoBinCuts.csv file with inside the vector<TCut> associated to the bins
const TString outCSVFile = "IsoBinCuts.csv";
// The IsoBinning Making produce IsoBinVar.pdf file for checking
const TString checkCanvas = "IsoBinVar.pdf";

using namespace std;

// The struct constructed for BuildBinningSchemens in evaluateIsoBinCuts
struct IsoBinningInputs {
    IsoBinningInputs() = default;
    IsoBinningInputs(const vector< TString > & _vars, const vector< int > & _bins, const vector<pair<double, double> > & _minmax, const vector<TString> & labels , const TString & _ID, const TString &_histType)
        : variables(_vars) 
        , nBins(_bins)
        , MinMax(_minmax)
        , labelAxis(labels)
        , ID(_ID)
        , HistoType(_histType) {}
    vector< TString > variables = {};
    vector< int >     nBins     = {};
    vector< pair< double, double> > MinMax;
    vector< TString > labelAxis = {};
    TString ID        = "ERROR";
    TString HistoType = "ERROR";
    void Print(){
        cout<<BLUE <<" ID   : "<< ID<< " ( "<< HistoType << endl;
        cout<<BLUE <<" varX : "<< variables[0] << " : ( "<< nBins[0] << " , " << MinMax[0].first << " , " << MinMax[0].second  << ") --> " << labelAxis[0] << endl;
        if(variables.size()!=1){
            cout<<BLUE <<" varY : "<< variables[1] << " : ( "<< nBins[1] << " , " << MinMax[1].first << " , " << MinMax[1].second  << ") --> " << labelAxis[1] << endl;
        }
        cout<< RESET << endl;
        return;
    }
};


/**
 * Example of usage of IsoBinReport struct
 *  cout<<RED << " ISOBINNING REPORTING FROM MM "<< RESET<< endl;
    auto IsoBinReportMM = GetIsoBinReport(eventMM, "B0_ETA", eventMM.GetCut(), eventMM.GetWeight(), SettingDef::Tuple::frac); //Get The IsoBin Report from eventMuons
    double CommonScalinSum = 1000;  // A common scaling can be used to "equally weight the 2 Data Informations ". I.e. If Electrons have 100 entries while Muons have 200, you may want to consider the data from Muon as important as from Electrons
    IsoBinReportMM.SortByValue();  //< sort the result to do it
    IsoBinReportMM.ScaleWeights(CommonScalinSum);
    cout<<RED << " ISOBINNING REPORTING FROM MM "<< RESET<< endl;
    auto IsoBinReportEE = GetIsoBinReport(eventEE, "B0_ETA", eventEE.GetCut(), eventEE.GetWeight(), SettingDef::Tuple::frac); //Get IsoBinning for EE mode
    IsoBinReportEE.SortByValue(); //Sort this one as well
    IsoBinReportEE.ScaleWeights(CommonScalinSum);

    //let's merge the 2 results.... Now we scaled both, we merge the results and we re-sort the container

    vector < pair <double, double> > _MergedResults;
    _MergedResults.reserve(IsoBinReportEE.varFill_weightFill.size() +  IsoBinReportMM.varFill_weightFill.size());
    _MergedResults = IsoBinReportEE.varFill_weightFill;
    _MergedResults.insert(_MergedResults.end(), IsoBinReportMM.varFill_weightFill.begin(),IsoBinReportMM.varFill_weightFill.end());

    sort(_MergedResults.begin(), _MergedResults.end(), [&](pair <double, double> & a, pair <double,double> & b){  return a.first < b.first;});

    auto _boundaries = GetBoundariesForPlot(_MergedResults, 10); //10 bins we want, and we Merge the Results, We can overload the Boundary 0 and last to be more conservative, but, this could screw up the ordering of auto-bounds.
    TCanvas c("canvas");
    TH1D * histoIsoBinned1 = new TH1D("hh1", "hh1", 10, _boundaries.data());
    TH1D * histoIsoBinned2 = new TH1D("hh2", "hh2", 10, _boundaries.data());
    c.Divide(2,1);
    c.cd(1);
    eventMM.GetTuple()->Draw("B0_ETA >> hh1", eventMM.GetWCut(), "e");
    c.cd(2);
    eventEE.GetTuple()->Draw("B0_ETA >> hh2", eventEE.GetWCut(), "e");
    c.SaveAs("PLOTISO.pdf"); //see how it comes out
 */

/**
 * \brief      Struct Holding the Informations to do an isobinning. Helper Function to sort by value, the sorted vector is then sliced in equal sizes depending on the nBins one want to use. Also full scale all weights to match a given sum
 * sumOfWeightsPas = sum of all weights passing the selection
 * sumOfEntriesPas = sum of Entries passing the selection
 * sumOfEntries = sum of Entries before selection
 * TODO : sumOfWeights before selection
 */
struct IsoBinReport {
    double                           sumOfWeightsPas;
    int                              sumOfEntriesPas;
    int                              sumOfEntries;
    vector< pair< double, double > > varFill_weightFill;
    void                             SortByValue() {
        cout << "sorting" << endl;
        sort(varFill_weightFill.begin(), varFill_weightFill.end(), [&](pair< double, double > & a, pair< double, double > & b) { return a.first < b.first; });
    };
    void ScaleWeights(double _scaleValue) {
        double sumNew      = 0;
        double scaleFactor = _scaleValue / sumOfWeightsPas;
        for (int i = 0; i < varFill_weightFill.size(); ++i) {
            varFill_weightFill[i].second = varFill_weightFill[i].second * scaleFactor;
            sumNew += varFill_weightFill[i].second;
        }
        if (sumOfWeightsPas != sumNew) { cout << RED << "SCALE VALUE = " << _scaleValue << " New Sum = " << sumNew << endl; }
        sumOfWeightsPas = sumNew;
    }
};

/**
 * @brief      Small helper struct housing (X,Y) values to be used for 2D histogramming and isobinning
 */
struct Var2D {
    double x;
    double y;
    Var2D(double _x, double _y) {
        x = _x;
        y = _y;
    };
    double X() const { return x; }
    double Y() const { return y; }
    Var2D() = default;
};

/**
 * \brief      Gets the iso bin report struct
 *
 * @param      _eventType  The event type to process, the Ntuple is extracted from there
 * @param[in]  _VarName    The variable name You intend to Plot (can be an expression)
 * @param[in]  _Cut        The cut to apply to the Tuple (if left empty it uses _eventType.GetCut())
 * @param[in]  _Weight     The weight to add to each event (if left empty it uses _eventType.GetWeight())
 * @param[in]  frac        The fraction of events (before Cut to process)
 * @param[in]  debug       The debug flag to inspect the method
 *
 * @return     The iso bin report.
 * TODO : extend for vector < _VarName, min, max  > : do a single Loop filling all Variables in one go
 * TODO : extend for vector < _Cuts    > : maybe a map[TString, TCut] to map all isobinning schemes produced.
 * TODO : extend to test different Weights schemes ... TODO here just for speed reason, you want to loop only once on all booked informations to extract ...
 */
IsoBinReport GetIsoBinReport(EventType & _eventType, TString _VarName, double min, double max, const TCut & _Cut = "", const TString & _Weight = "", double frac = -1, bool debug = false);

/**
 * \brief      Gets the boundaries for plot passing a vector of pairs of Values (to plot) and Weights (to plot) both aligned and already sorted by value
 *
 * @param[in]  Value_Weights  The value weights pair housing < ValueToPlot collected, Weights Assigned to that entry)
 * @param[in]  nBins          The number Of Bins to generate
 *
 * @return     The boundaries for plot. You can use this into TH1D("nn","nn", nbins, boundaries.data()); To fast-build your isobinned plot
 */
array< Double_t, MAXNBINS > GetBoundariesForPlot(const vector< pair< double, double > > & Value_Weights, int nBins, double min, double max);

/**
 * @brief      Build a vector<pair<value,weight>> out of values with weight = 1000/values.size()
 *
 * @param      values = flat vector housing double values of a given variable [ assume sorted from small-->high value ! ]
 *
 * @return     a vector< value, weight > , with the weight being 1000./(entries), i.e the integral of weights of this vector is 1000. (used to equal normalize 2 observables)
 *
 */
vector< pair< double, double > > ScaleAndWeightSorted(vector< double > & values);

/**
 * @brief      Build a vector<pair<value,weight>> out of Var2D values with weight = 1000/values.size()
 *
 * @param      values = flat vector housing <x,y> values of a given variable [ not assumed sorted in one direction]
 *
 * @return     a vector< Var2D, weight > , with the weight being 1000./(entries), i.e the integral of weights of this vector is 1000. (used to equal normalize 2 sets of 2D observables)
 */
vector< pair< Var2D, double > > ScaleAndWeightSorted(vector< Var2D > & values);

/**
 * @brief      Gets the vector cut [_boundaries contains list of values which MUST BE ALIGNED TO MIN/MAX and NBINS]
 *
 * @param      _boundaries  The boundaries
 * @param[in]  _varIso      The variable iso
 * @param[in]  _nbins       The nbins
 * @param[in]  _min         The minimum
 * @param[in]  _max         The maximum
 *
 * @return     The vector cut.
 */
vector< TCut > getVectorCut(double * _boundaries, TString _varIso, int _nbins, double _min, double _max);

/**
 * @brief      Write a CSV file with a list of Cuts [assumed in ORDER]
 *
 * @param      _cuts  The cuts
 */
void WriteCSVfile(vector< TCut > & _cuts, TString _ID);

/**
 * @brief      Filter and return a vector slicing it with the range provided
 *
 * @param[in]  invector  The invector
 * @param[in]  range     The range
 *
 * @return     { description_of_the_return_value }
 */
vector< double > filterVector(const vector< double > & invector, pair< double, double > range);

/**
 * @brief      Filter and return a vector slicing it with the range provided [input is a vector<X,Y> values]
 *
 * @param[in]  invector  The invector
 * @param[in]  rangeX    The range x
 *
 * @return     vector sliced in X
 */
vector< Var2D > filterVectorX(const vector< Var2D > & invector, pair< double, double > rangeX);

/**
 * @brief      Filter and return a vector slicing it with the range provided [input is a vector<X,Y> values]
 *
 * @param[in]  invector  The invector
 * @param[in]  rangeX    The range y
 *
 * @return     vector sliced in Y
 */
vector< Var2D > filterVectorY(const vector< Var2D > & invector, pair< double, double > rangeY);

/**
 * @brief      Builds a binning scheme 1 d weighting EE/MM types.
 *
 * @param      _eTypeEE         The EventType for EE
 * @param      _eTypeMM         The EventType for MM
 * @param[in]  _isoBinVariable  The var to use to IsoBin
 * @param[in]  nBins            The nBins to use
 * @param[in]  dumpIt           The Dump the CSV file?
 */
void BuildBinningScheme1D(EventType & _eTypeEE, EventType & _eTypeMM, const vector<IsoBinningInputs> & _isoBinInput, bool OnlyMuon = false, bool OnlyElectron = false); 

/**
 * @brief      Builds a binning scheme 2d for X vs Y : first IsoBin in X then IsoBin in Y for each x-slice
 *
 * @param      _eTypeEE  The eType ee
 * @param      _eTypeMM  The eType mm
 * @param[in]  varX      The variable x [ pair< Name, nBins> ] ( deprecated, use IsoBinInputs struct )
 * @param[in]  varY      The variable y [ pair< Name, nBins>]  ( deprecated, use IsoBinInputs struct )
 * bin scheme is (for a 3x3 for example) , rectangular borders in x-y , split in X , variable Y-binning for each x-slice
 * IsoBinning is done in a "shared" range for x-y, but boundaries are built from the min(x) max(x) min(y),max(y)
 * ------------------
   |__2_| 5   |__8__|
   |    |_____|     |
   |  1 |_4___|  7  |
   |____|     |_____|
   |  0 | 3   |  5  |
   ------------------
*/

void BuildBinningScheme2D(EventType & _eTypeEE, EventType & _eTypeMM, const IsoBinningInputs & _isoBinInput, bool OnlyMuon = false, bool OnlyElectron = false); 

#endif
