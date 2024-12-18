#ifndef COUNTERHELPERS_HPP
#define COUNTERHELPERS_HPP

#include "IOSvc.hpp"
#include "MessageSvc.hpp"

//#include "ROOT/RMakeUnique.hxx"
#include "itertools.hpp"
#include <ROOT/RDataFrame.hxx>
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <fstream>
#include <map>
#include <vector>

/**
 * @brief      Simple Struct holding A weight | selection counting.
 *             Internally store the "SumW" , SumW2 given selection.
 */
struct CounterWeights {
    TString ID;                          // The ID assigned to it
    TString WeightAlias;                 // The WeightAlias used
    TString WeightExpression;            // The weight Expression used
    TString SelectionAlias;              // The selection Alias used
    TString SelectionExpression;         // The selection Expression used
    double  SumWeight         = 0;       // SumW   for this Weight | Selection Sum
    double  SumWeight2        = 0;       // SumWSq for this Weight | Selection Sum
    double  SumWeight_RND     = 0;       // SumW using Random KILLER
    double  SumWeight2_RND    = 0;       // SumW2 using Random KILLER
    int     nPasSelection     = 0;       // NPas selection
    int     nPasSelection_RND = 0;       // NPas selection (RND)
    int     nTotSelection     = 0;       // NTot Selection ( before selection )
    bool    Processed         = false;   // Internal flag for processing status, never reprocess things...
    bool    Processed_rnd     = false;   // Internal flag for processing status RND, never reprocess things
    CounterWeights() {
        ID                  = "ERROR";
        WeightAlias         = "ERROR";
        WeightExpression    = "ERROR";
        SelectionAlias      = "ERROR";
        SelectionExpression = "ERROR";
        SumWeight           = 0;
        SumWeight2          = 0;
        SumWeight_RND       = 0;
        SumWeight2_RND      = 0;
        nPasSelection       = 0;
        nPasSelection_RND   = 0;
        nTotSelection       = 0;
    };
    /**
     * @brief      Constructor
     *
     * @param[in]  _WeightAlias          The weight alias
     * @param[in]  _WeightExpression     The weight expression
     * @param[in]  _SelectionAlias       The selection alias
     * @param[in]  _SelectionExpression  The selection expression
     * @param[in]  _ID                  Unique ID of this Counters
     */
    CounterWeights(const TString & _WeightAlias, const TString & _WeightExpression, const TString & _SelectionAlias, const TString & _SelectionExpression, const TString & _ID) {
        ID                  = _ID;
        WeightAlias         = _WeightAlias;
        WeightExpression    = _WeightExpression;
        SelectionAlias      = _SelectionAlias;
        SelectionExpression = _SelectionExpression;
        SumWeight           = 0;
    };
    /**
     * @brief      Process_BIN
     *
     * @param      _Weights         The vector<double> housing all _Weights to be used (_Weights.size == _Baseselection.size == _Extraselection.size )
     * @param      _Baseselection   The _Baseselection as vector<double> , for value > 0 = true, value < 0 = false
     * @param      _Extraselection  The _Extraselection the selection to apply on top of _Baseselection, value > 0 true, value < 0 false
     */
    void Process_BIN(const vector< double > & _Weights, const vector< bool > & _Baseselection, const vector< bool > & _Extraselection) {
        if (Processed) MessageSvc::Error("ERROR, PROCEESSINGTWICE THE COUNTER!", "", "EXIT_FAILURE");
        if (_Weights.size() != _Baseselection.size()) { MessageSvc::Error("SEVERE ERROR Process BIN vector sizes!", ID, "EXIT_FAILURE"); }
        if (_Weights.size() != _Extraselection.size()) { MessageSvc::Error("SEVERE ERROR Process BIN vector sizes!", ID, "EXIT_FAILURE"); }

        SumWeight     = 0;   // reset counters
        SumWeight2    = 0;   // reset counters
        nPasSelection = 0;   // reset counters

        MessageSvc::Info(TString("Process Bin (_Extraselection) ") + ID, WeightAlias, SelectionAlias);
        MessageSvc::Info(TString("Process Bin (_Extraselection) ") + ID, WeightExpression, SelectionExpression);
        for (int i = 0; i < _Weights.size(); ++i) {
            if (_Baseselection.at(i) > 0 && _Extraselection.at(i) > 0) {
                nPasSelection++;
                SumWeight += _Weights.at(i);
                SumWeight2 += _Weights.at(i) * _Weights.at(i);
            }
            nTotSelection++;
        }
        Processed = true;
    };
    /**
     * @brief      Process_BIN_RND
     *
     * @param[in]  _Weights         The vector<double> housing all _Weights to be used (_Weights.size == _Baseselection.size == _Extraselection.size )
     * @param[in]  _Baseselection   The _Baseselection as vector<double> , for value > 0 = true, value < 0 = false
     * @param[in]  _Extraselection  The _Extraselection associated to the specific Bin
     * @param[in]  _KeepEntries     The keep entries for Multiple Candidate removal ( RANDOM ones )
     */

    // Will be dropped with a Fill HistogramVar
    void Process_BIN_RND(const vector< double > & _Weights, const vector< bool > & _Baseselection, const vector< bool > & _Extraselection, const vector< int > & _KeepEntries) {
        if (Processed_rnd) MessageSvc::Error("ERROR, PROCEESSINGTWICE THE COUNTER (RND)!", "", "EXIT_FAILURE");
        if (_Weights.size() != _Baseselection.size()) { MessageSvc::Error("SEVERE ERROR Process BIN RND vector sizes!", ID, "EXIT_FAILURE"); }
        if (_Weights.size() != _Extraselection.size()) { MessageSvc::Error("SEVERE ERROR Process BIN RND vector sizes!", ID, "EXIT_FAILURE"); }
        SumWeight_RND     = 0;   // reset counters
        SumWeight2_RND    = 0;   // reset counters
        nPasSelection_RND = 0;   // reset counters
        MessageSvc::Info(TString("Process Bin RANDOM KILL ") + ID, WeightAlias, SelectionAlias);
        MessageSvc::Info(TString("Process Bin RANDOM KILL ") + ID, WeightExpression, SelectionExpression);
        for (auto & idx : _KeepEntries) {
            if (_Baseselection.at(idx) == false) { MessageSvc::Error("SEVERE ERROR, keeping Randomly on non-selected events", "", "EXIT_FAILURE"); }
            if (_Extraselection.at(idx) == true) {
                nPasSelection_RND++;
                SumWeight_RND += _Weights.at(idx);
                SumWeight2_RND += _Weights.at(idx) * _Weights.at(idx);
            }
        }
        Processed_rnd = true;
    };

    /**
     * @brief      Process
     *
     * @param[in]  _Weights     The weight to process (as vector<double> )
     * @param[in]  _Selections  The selection vector to process ( as vector<double> , > 0 = pas, < 0 = rejected)
     */
    void Process(const vector< double > & _Weights, const vector< bool > & _Selections) {
        if (Processed) MessageSvc::Error("ERROR, PROCEESSINGTWICE THE COUNTER!", "", "EXIT_FAILURE");
        if (_Weights.size() != _Selections.size()) { MessageSvc::Error("SEVERE ERROR Process vector sizes!", ID, "EXIT_FAILURE"); }
        SumWeight     = 0;
        nPasSelection = 0;
        nTotSelection = 0;
        MessageSvc::Info(TString("Process ") + ID, WeightAlias, SelectionAlias);
        MessageSvc::Info(TString("Process ") + ID, WeightAlias, SelectionAlias);

        for (const auto && [weight, selection] : iter::zip(_Weights, _Selections)) {
            if (selection) {
                nPasSelection++;
                SumWeight += weight;
                SumWeight2 += weight * weight;
            }
            nTotSelection++;
        }
        Processed = true;
    }
    /**
     * @brief      Process using Random killer ( via keepEntries = vector of indexes of entry to keep )
     *
     * @param[in]  _Weights     The weight to process (as vector<double> )
     * @param[in]  _Selections  The selection vector to process ( as vector<double> , > 0 = pas, < 0 = rejected)
     */
    void Process_RND(const vector< double > & _Weights, const vector< bool > & _Selections, const vector< int > & _KeepEntries) {
        if (Processed_rnd) MessageSvc::Error("ERROR, PROCEESSINGTWICE THE COUNTER (RND)!", "", "EXIT_FAILURE");
        if (_Weights.size() != _Selections.size()) { MessageSvc::Error("Process RND SEVERE ERROR!", "", "EXIT_FAILURE"); }
        SumWeight_RND     = 0;
        nPasSelection_RND = 0;
        MessageSvc::Info(TString("Process RANDOM KILL") + ID, WeightAlias, SelectionAlias);
        MessageSvc::Info(TString("Process RANDOM KILL") + ID, WeightAlias, SelectionAlias);
        for (auto & idx : _KeepEntries) {
            if (_Selections.at(idx) == false) { MessageSvc::Error("SEVERE ERROR, keeping Randomly on non-selected events", "", "EXIT_FAILURE"); }
            nPasSelection_RND++;
            SumWeight_RND += _Weights.at(idx);
            SumWeight2_RND += _Weights.at(idx) * _Weights.at(idx);
        }
        Processed_rnd = true;
    }

    void Report() {
        // Some cout...
        std::cout << GREEN << "------ REPORT FOR " << ID << " ---------" << std::endl;
        std::cout << " SumW                 : " << SumWeight << std::endl;
        std::cout << " SumW2                : " << SumWeight2 << std::endl;
        std::cout << " SumW(RNDKILL)        : " << SumWeight_RND << std::endl;
        std::cout << " SumW2(RNDKILL)       : " << SumWeight2_RND << std::endl;
        std::cout << " nPAS Selection       : " << nPasSelection << std::endl;
        std::cout << " nPAS Selection RND   : " << nPasSelection_RND << std::endl;
        std::cout << " Weight    Expression : " << WeightExpression << std::endl;
        std::cout << " Selection Expression : " << SelectionExpression << std::endl;
        std::cout << GREEN << "------------------------------" << RESET << std::endl;
    }
    void ReportOnDisk() {
        // To check
        if (IOSvc::ExistFile(ID + "_Counters.json")) { MessageSvc::Error("Cannot Report On Disk, file exists ", "", "EXIT_FAILURE"); }
        std::ofstream ofs(ID + "_Counters.json", std::ofstream::out);
        ofs << "{ 'Weight'    : '" << WeightExpression << "' , \n "
            << "  'Selection' : '" << SelectionExpression << "' , \n "
            << "  'SumW'      : '" << SumWeight << "' , \n "
            << "  'SumW2'     : '" << SumWeight2 << "' , \n "
            << "  'SumW_RND'  : '" << SumWeight_RND << "' , \n "
            << "  'SumW2_RND' : '" << SumWeight2_RND << "' , \n "
            << "  'nPas'      : '" << nPasSelection << "' , \n "
            << "  'nTot'      : '" << nTotSelection << "' , \n }";
        ofs.close();
    }
};

/**
 * @brief      Class housing public functions to use to shuffle around counting Entries for SumW for efficiencies
 */
class CounterHelpers {
    CounterHelpers() = default;

  public:
    /**
     * @brief      Return the vector<int> pointing to the IDX to keep given a selection and a random killing based on matching runNumber/eventNumber
     *
     * @param[in]  _runNumber    The run number in the tuple , vector<int> is the outcome of DataFrame.Take( "runNumber");
     * @param[in]  _eventNumber  The event number , vector<int> is the outcome of the DataFrame.Take( "eventNumber")
     * @param[in]  _selection    The selection , vector<double> is the outcome of the DataFrame.Take( "selection"), with selection = (Cut) != 0 ? 2.5 : -2.5 , true remapped to >0, false remapped to < 0
     * @return  vector<int> of entries to KEEP to eval SumWeights.
     */
    static vector< int > EntriesToKeepList(const vector< int > & _runNumber, const vector< int > & _eventNumber, const vector< bool > _selection);
    /**
     * @brief      Return vector<int> pointing to the IDX to keep given a selection and a random killing absed on matching runNUmber/eventNumber
     *
     * @param[in]  _runNumber       The run number in the tuple , vector<int> is the outcome of DataFrame.Take( "runNumber");
     * @param[in]  _eventNumber     The event number , vector<int> is the outcome of the DataFrame.Take( "eventNumber")
     * @param[in]  _selection       The selection , vector<double> is the outcome of the DataFrame.Take( "selection"), with selection = (Cut) != 0 ? 2.5 : -2.5 , true remapped to >0, false remapped to < 0
     * @param[in]  _selectionOnTop  The selection on top of _selection , vector<double> is the outcome of the DataFrame.Take( "selection"), with selection = (Cut) != 0 ? 2.5 : -2.5 , true remapped to >0, false remapped to < 0
     * Use case :
     * CounterHelpers::EntriesToKeepList(_runNumber, _eventNumber, _SELECTIONTM, _SELECTION); // TruthMatching --> FULL SELECTION (which contains TruthMatching! )
     * The random killing is done such that pair.first = ( SELECTIONTM random killed ), pair.second = (SELECTION random killed ) contains same IDX after Random killing multiple candidates
     * Saying if MCandidate killer random would kill entry [0,1,2] in The Selection truth matched, but after selection the selection keep them as unique, the TM multiple candidate killer has to contains what is applied afterwards.
     * @return     pair< vector<IDX> (Selection Prior) , vector<IDX> ( SelectioNONTOP) > the 2 Vectors contains SAME IDX ( from the ONTOP to the total container)
     */
    static pair< vector< int >, vector< int > > EntriesToKeepList(const vector< int > & _runNumber, const vector< int > & _eventNumber, const vector< bool > & _selection, const std::vector< bool > & _selectionOnTop);

    /**
     * @brief      ProcessMCDecayTuple, returning the CounterWeights to be used for Efficiencies
     *
     * @param[in]  _baseConfigHolder  The base configuration holder from which the MCTuple is extracted
     *
     * @return     The associated CounterWeights for the MCDecayTuple
     */
    static CounterWeights ProcessMCDecayTuple(const ConfigHolder & _baseConfigHolder, const TString & _weightOption, const TString & _weightConfiguration);

    // static TString BuildCutExpression(const TCut & cut) {
    //     TString _expr = TString("(") + TString(cut) + ") != 0 ? +2.5 : -2.5";
    //     return _expr;
    // }
    // static TString BuildCutExpression(const TString & string) {
    //     TString _expr = TString("(") + TString(string) + ") != 0 ? +2.5 : -2.5";
    //     return _expr;
    // }
};

#endif
