#ifndef HELPERSVC_HPP
#define HELPERSVC_HPP

#include "SettingDef.hpp"
#include "ConfigHolder.hpp"
#include "EnumeratorSvc.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string.h>
#include <typeinfo>
#include <vector>

#include <boost/units/detail/utility.hpp>

//#include "ROOT/RMakeUnique.hxx"
#include <ROOT/RDataFrame.hxx>

class TupleReader;

class TRandom3;
class TChain;
class TString;
class RooRealVar;
class RooAbsReal;

const TCut    NOCUT    = "1";
const TString NOWEIGHT = "1.";

struct SplitInfo {
    TString FileName;
    TString TreeName;
    TString Selection;
    TString SelectionName;
    SplitInfo() {
        FileName      = "";
        TreeName      = "";
        Selection     = "";
        SelectionName = "";
    }
    SplitInfo(const TString & _fileName, const TString & _treeName, const TCut & _selection, const TString & _selectionName) {
        FileName = _fileName;
        if (!FileName.Contains(".root")) {
            // MessageSvc::Info("Appending .root to filename for the split");
            FileName += ".root";
        }
        TreeName = _treeName;
        if (TreeName.Contains("-") || TreeName.Contains("+") || TreeName.Contains("*") || TreeName.Contains("/")) {
            // MessageSvc::Info("Converting special caracthers to underscores");
            TreeName.ReplaceAll("-", "_").ReplaceAll("+", "_").ReplaceAll("*", "_").ReplaceAll("/", "_");
        }
        Selection     = TString("(") + TString(_selection) + " ) != 0 ? +1. :-1. ";
        SelectionName = _selectionName;
    }
};

/**
 * \typedef Str2VarMap
 * \brief Template map for TString to Var
 */
typedef map< string, RooAbsReal * > Str2VarMap;

/**
 * \typedef Str2BoolMap
 * \brief Template map for TString to bool
 */
typedef map< TString, bool > Str2BoolMap;

/**
 * \typedef Str2CutMap
 * \brief Template map for TString to Cut
 */
typedef map< TString, TCut > Str2CutMap;

/**
 * \typedef Str2WeightMap
 * \brief Template map for TString to Weight
 */
typedef map< TString, TString > Str2WeightMap;

template < typename T > TString GetTypename(const T & _type) { return (TString) boost::units::detail::demangle(typeid(_type).name()); };

using namespace RooFit;

vector< TString > TokenizeString(const TString & _string, const TString & _separator);

TString RemoveStringAfter(TString _string, const TString & _after);

TString RemoveStringBefore(TString _string, const TString & _before);

TString RemoveStringBetween(TString _string, const TString & _startString, const TString & _endString);

TString StripStringBetween(const TString & _string, const TString & _startString, const TString & _endString);

TString ExpandEnvironment(const TString & _string);
/**
 * \brief Configure everything in the Framework from a given YAML file
    Use the Parser as a TOOL to configure default system configurations
 * @param  _fileYAML [yaml file from which one want to load the settings]
 */  
void ConfigureFromYAML(TString _fileYAML);

void EnableMultiThreads(int _nThreads = 0);

void DisableMultiThreads();

bool ContainString(vector< TString > _vector, TString _string);

/**
 * \brief Clean string
 * @param  _string [description]
 */
TString CleanString(TString _string);

/**
 * \brief Clean string
 * @param  _string [description]
 */
TString CleanAnalysis(TString _string);

/**
 * \brief Clean string
 * @param  _string [description]
 */
TString CleanQ2Bin(TString _string);

/**
 * \brief Clean string
 * @param  _string [description]
 */
TString CleanYear(TString _string);

/**
 * \brief Clean string
 * @param  _string [description]
 */
TString CleanTrigger(TString _string);

/**
 * \brief Clean string
 * @param  _string [description]
 */
TString CleanBrem(TString _string);

/**
 * \brief Check PID Maps for nullptrs
 * @param _map [map to be checked]
 */
void CheckPIDMapsForNullPtrs(pair< TH1D*, vector < TH2D*> > _map, TString _name, bool _vbs);

TCut CustomizedCut(TString _cutOption, TString _separator, TString _cutVariable );

/**
 * \brief Replace TCut
 * @param  _cut     [description]
 * @param  _string1 [description]
 * @param  _string2 [description]
 */
TCut ReplaceCut(TCut _cut, TString _string1, TString _string2 = "");

/**
 * \brief Replace TCut based on Project
 * @param  _cut     [description]
 * @param  _project [description]
 */
TCut ReplaceProject(TCut _cut, Prj _project);

TString ReplaceWildcards(TString _string, map< TString, TString > _names);

/**
 * \brief Clean TCut
 * @param  _cut [description]
 */
TCut CleanCut(TCut _cut);

/**
 * \brief Clean Weight
 * @param  _weight [description]
 */
TString CleanWeight(TString _weight);

/**
 * \brief Convert DTF TCut (vector <-> element[0])
 * @param  _cut [description]
 */
TCut UpdateDTFCut(TCut _cut);

Year GetRunFromYear(const Year & _year);

TCut JoinCut( const vector<TString> & list_of_cuts , TString _logicalOperator);

TCut GetHLT1TCKCut( Year _year);

TCut UpdateHLT1Cut(TCut _cut, Prj _project, Analysis _analysis, Year _year, TString _option);

TCut UpdatePIDTune(TCut _cut, TString _year);

TCut UpdatePIDMeerkat(TCut _cut, TString _year);

TCut UpdateMVACut(TCut _cut);

vector< TString > GetAnalyses(TString _analysis);

vector< TString > GetYears(TString _year, TString _option = "");

TString GetRunFromYear(TString _year);

Year GetYearForSample(TString _sample, Year _year , Prj _prj = Prj::All);

vector< TString > GetPolarities(TString _polarity);

vector< TString > GetTriggers(TString _trigger, bool _split);

vector< TString > GetTracks(TString _track, TString _project, bool _split);

vector< pair< TString, TString > > GetAliasHLT1(TString _project, TString _ana, TString _year);

vector< pair< TString, TString > > GetAliasHLT2(TString _project, TString _ana, TString _year);

pair< double, double > GetWPID(int _particleID, double _x, double _y, TString _option, TH2 * _mapK, TH2 * _mapPi, TH2 * _mapMu, TH2 * _mapE, TH2 * _mapP);

pair< double, double > GetWPID3D_Hadron(int _particleID, double _x, double _y, int _z, TString _option, pair< TH1D *, vector < TH2D * > >& _mapsK, pair< TH1D *, vector < TH2D * > >& _mapsPi, pair< TH1D *, vector < TH2D * > >& _mapsMu, pair< TH1D *, vector < TH2D * > >& _mapsE, pair< TH1D *, vector < TH2D * > >& _mapsP);

pair< double, double > GetWPID3D_Muon(int _particleID, double _x, double _y, int _z, TString _option, pair< TH1D *, vector < TH2D * > >& _mapsK, pair< TH1D *, vector < TH2D * > >& _mapsPi, pair< TH1D *, vector < TH2D * > >& _mapsMu);

pair< double, double > GetWPID3D_Electron(int _particleID, double _x, double _y, int _z, TString _option, pair< TH1D *, vector < TH2D * > >& _mapsK, pair< TH1D *, vector < TH2D * > >& _mapsPi, pair< TH1D *, vector < TH2D * > >& _mapsMu, pair< TH1D *, vector < TH2D * > >& _mapsE);

TString GetBaseVer(TString _ver);

TString GetPIDTune(TString _year, TString _option);

void PrintVar(RooAbsReal * _var);

string IsVarInMap(string par, Str2VarMap _map, string vname);

bool IsVarInMap(TString _par, Str2VarMap _map);

bool IsCutInMap(TString _cut, Str2CutMap _map);

bool IsWeightInMap(TString _weight, Str2WeightMap _map);

bool IsCut(TCut _cut);

bool IsWeight(TString _weight);

bool HasWeight(TCut _cut);

void PrintPars(Str2VarMap _map, string opt = "");

bool IsVarInTuple(TTree * _tuple, TString _name);

void PlotBranches(TTree * _tuple, TString _name = "");

Long64_t MultCandRandomKill(TChain * _tuple, vector< Long64_t > _entry, TRandom3 & _rnd);

Long64_t MultCandBestBkgCat(TChain * _tuple, vector< Long64_t > _entry, TRandom3 & _rnd);

int GetSignalLikeness(int _h1, int _h2, int _l1, int _l2);

pair< double, double > GetAverageVal(const vector< double > & _values, const vector< double > & _errors, const vector< double > & _weights, bool _debug = false);

RooRealVar * GetAverage(const vector< double > & _values, const vector< double > & _errors, const vector< double > & _weights);


// vector<double> ConvertToRecursiveFractions( const vector<double> & inputValues);
// void RecursiveFraction( RooArgList & inputValues);
/**
 * \brief  Gets the branches from expression.
 * @param  _tuple  The tree on which to compile the expression (as pointer or as Reference)
 * @param  _expression  The expression from which to extract the list of Branches
 * @return The branches set contained in the expression.
 */
vector< TString > GetBranchesFromExpression(TChain * _tuple, const TString & _expression);

vector< pair< TString, TString > > GetAliasesFromExpression(TChain * _tuple, const TString & _expression);

vector< TString > GetBranchesAndAliasesFromExpression(TChain * _tuple, const TString & _expression);

vector< pair< TString, TString > > GetAllAliases(TChain * _tuple);

vector< pair< TString, TString > > GetAllAlias(TTree * _tree, bool _clean = true);

RooDataSet * GetRooDataSet(TChain * _tuple, const TString & _name, RooArgList _varList, const TCut & _cut = TCut(NOCUT), const TString & _weight = TString(NOWEIGHT), double _frac = -1, TString _option = "");

RooDataSet * GetRooDataSetCopy(TupleReader & _tupleReader, const TString & _name, RooArgList _varList, const TCut & _cut = TCut(NOCUT), const TString & _weight = TString(NOWEIGHT), double _frac = -1, TString _option = "");

RooDataSet * GetRooDataSetSnapshot(TChain * _tuple,   const TString & _name, ConfigHolder  _cHolderInput  , TString _weightOption , RooArgList _varList, const TCut & _cut = TCut(NOCUT), const TString & _weight = TString(NOWEIGHT), double _frac = -1, TString _option = "" );

RooDataSet * GetRooDataSetSnapshotBSData(TChain * _tuple, const TString & _name, RooArgList _varList, const TCut & _cut = TCut(NOCUT), const TString & _weight = TString(NOWEIGHT), double _frac = -1, TString _option = "");

/**
 * \brief      Makes snapshots in parallel for the n-selections you bookkeped
 * @param  _selections     The selections bookkeeped in the map, each with a name
 * @param  _hadd           To hadd after , add safety check that each "tuple" in different TFile has different names, so merging back is safe, default , the check is not done
 * @param  _branchesToKeep The branches to keep, if you want to "keep" only few branches when doing the snapshot, empty list --> keep ALL
 * @param  _aliasesToKeep  The aliases  to keep, if you want to "keep" only few branches when doing the snapshot (but they are aliases), empty list --> keep ALL
 * @param  report Save to disk a log file with the cut applied
 * @param  _reportName name of the report to save to disk 
 * @param  _option Option to customize Snapshot making . We can (should) recompute some branch on the fly for some specific use - cases. option would allow to patch the behaviour of dataset creation
 * By default, all aliases defined are added as an extra Branch.
 */
void MakeSnapshots(TChain * _tuple, const map< TString, SplitInfo > & _selections, double _frac = -1, bool _hadd = false, vector< TString > _branchesToKeep = {}, vector< TString > _aliasesToKeep = {}, bool report = false, TString _reportName = "reportSnapshot", TString _option ="", ConfigHolder _configHolder = ConfigHolder());

/**
 * \brief      Split the tuple using RDataFrame
 * @param  _selections     Map< KEY, < TTreeName to create , Selection for the TTreeName to create >  _selections  The selections bookkeped fo the parallel snapshots
 * @param  _hadd           "Ensure each TTree saved to disk has different names"
 * @param  _branchesToKeep vector<TString> , list of the branches to create the snapshot. If empty all are kept. Full CopyTree with selection.
 * @param  _option         Allow to customize the snapshotting, usally forwarded by MakeSnapshots method (_option flag)
 * @return     Locally the snapshots are created, the TTree associated to the splits, are returned in the Map.
 */
map< TString, TTree * > GetSplitTuples(TChain * _tuple, const map< TString, SplitInfo > & _selections, double _frac = -1, bool _hadd = false, vector< TString > _branchesToKeep = {}, vector< TString > _aliasesToKeep = {}, TString _option = "");

/**
 * \brief GetParticleBranchNames
 * @param  _project  [project switcher , RKst, RK, RPhi....]
 * @param  _analysis [EE/MM ? ]
 * @param  _q2bin [ q2bin to use]
 * #param  _option [ "onlyLeptons , will return M1,M2 or E1,E2 , "onlyhadrons"  returns "K, Pi" or K1,K2....or "K"]
 * @retrun the set of TString of Particles Branch names of a given Project, Analysis, q2Bin. And the Latex Name in the pair
 */
vector< pair< TString, TString > > GetParticleBranchNames(const Prj _project, const Analysis _analysis, const Q2Bin _q2bin, TString _option = "");

/**
 * @brief      Convert pair<TString,TString> to <string, string>
 *
 * @param[in]  _pairs  The pairs
 *
 * @return     { description_of_the_return_value }
 */
vector< pair< string, string > > to_string_pairs(const vector< pair< TString, TString > > & _pairs);

/**
 * Recursive Define adding things on top of each other.
 * Steer a vector< pair< NameAlias, Expression> > , passa a node of RDataFrame, get a new Node with all Defines In
 * FoceDoubleCast , to force casting to double the Alias
 * i = 0, to allow recursive call of the function
 */
ROOT::RDF::RNode ApplyDefines(ROOT::RDF::RNode df, const vector< pair< TString, TString > > & names_expressions, bool _forceDoubleCast = false, unsigned int i = 0);
ROOT::RDF::RNode ApplyDefines(ROOT::RDF::RNode df, const vector< pair< string, string > > & names_expressions, bool _forceDoubleCast = false, unsigned int i = 0);



/**
 * Recompute misID branches on the fly, some branches are ill defined and must be recomputed if used
*/
ROOT::RDF::RNode EvalSwapsByHand( ROOT::RDF::RNode df, const Prj & prj = Prj::RKst, const Analysis & ana = Analysis::All);

ROOT::RDF::RNode Reweight2XJPs( ROOT::RDF::RNode df, TString _weightName );

#endif
