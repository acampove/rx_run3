#ifndef COUNTERHELPERS_CPP
#define COUNTERHELPERS_CPP

#include "CounterHelpers.hpp"
#include "CutDefRX.hpp"
#include "RXDataPlayer.hpp"

#include "EventType.hpp"
#include "HelperSvc.hpp"

#include "vec_extends.h"

#include "TRandom3.h"

vector< int > CounterHelpers::EntriesToKeepList(const vector< int > & _runNumber, const vector< int > & _eventNumber, const vector< bool > _selection) {
    if (_runNumber.size() != _eventNumber.size() || _runNumber.size() != _selection.size() || _runNumber.size() == 0) { MessageSvc::Error("SIZE ISSUE EntriesToKeepList", "", "EXIT_FAILURE"); }
    vector< int > _keep_entries_RND = {};
    _keep_entries_RND.reserve(_runNumber.size());
    TRandom3                                    rnd = TRandom3();
    std::map< pair< int, int >, vector< int > > _CounterMultipleCand_RND;

    for (const auto && [_runN, _eventN, _sel, _idxElem] : iter::zip(_runNumber, _eventNumber, _selection, iter::range(_runNumber.size()))) {
        if (!_sel) continue;
        // Only selected candidates for MCand counting
        auto _uniqueID = make_pair(_runN, _eventN);
        if (_CounterMultipleCand_RND.find(_uniqueID) == _CounterMultipleCand_RND.end()) { _CounterMultipleCand_RND[_uniqueID].reserve(10); }
        _CounterMultipleCand_RND[_uniqueID].push_back(_idxElem);   // entry i filled
    }
    for (auto & el : _CounterMultipleCand_RND) {
        if (el.second.size() != 1) {
            int to_keep = std::floor(rnd.Uniform(0, el.second.size()));
            _keep_entries_RND.push_back(el.second.at(to_keep));
        } else {
            _keep_entries_RND.push_back(el.second.front());   // first int index of entry to keep
        }
    }
    std::sort(_keep_entries_RND.begin(), _keep_entries_RND.end());
    MessageSvc::Info("MultCandidate finder entries process, single candidate found ", to_string(_keep_entries_RND.size()));
    return _keep_entries_RND;
}

// this will be "dropped!"
pair< vector< int >, vector< int > > CounterHelpers::EntriesToKeepList(const vector< int > & _runNumber, const vector< int > & _eventNumber, const vector< bool > & _selection, const vector< bool > & _selectionOnTop) {
    // We remove multiple candidates in the FINAL selected sub-sample of MC
    MessageSvc::Info("MultCandidate finder entries process ", to_string(_runNumber.size()));
    if (_runNumber.size() != _eventNumber.size() || _runNumber.size() != _selection.size() || _runNumber.size() != _selectionOnTop.size() || _runNumber.size() == 0) { MessageSvc::Error("SIZE ISSUE EntriesToKeepList align", "", "EXIT_FAILURE"); }
    vector< int > _keep_entries_RND       = {};
    vector< int > _keep_entries_RND_onTop = {};

    _keep_entries_RND.reserve(_runNumber.size());
    _keep_entries_RND_onTop.reserve(_runNumber.size());
    TRandom3                                    rnd = TRandom3();
    std::map< pair< int, int >, vector< int > > _CounterMultiPleCand_SELECTION_RND;
    std::map< pair< int, int >, vector< int > > _CounterMultiPleCand_SELECTION_ONTOP_RND;

    for (const auto && [_runN, _evtN, _sel, _selOnTop, _idxEvt] : iter::zip(_runNumber, _eventNumber, _selection, _selectionOnTop, iter::range(_selection.size()))) {
        if (!_sel) continue;
        auto _uniqueID = make_pair(_runN, _evtN);
        if (_CounterMultiPleCand_SELECTION_RND.find(_uniqueID) == _CounterMultiPleCand_SELECTION_RND.end()) { _CounterMultiPleCand_SELECTION_RND[_uniqueID].reserve(10); }
        _CounterMultiPleCand_SELECTION_RND[_uniqueID].push_back(_idxEvt);
        if (!_selOnTop) continue;

        if (_CounterMultiPleCand_SELECTION_ONTOP_RND.find(_uniqueID) == _CounterMultiPleCand_SELECTION_ONTOP_RND.end()) { _CounterMultiPleCand_SELECTION_ONTOP_RND[_uniqueID].reserve(10); }
        _CounterMultiPleCand_SELECTION_ONTOP_RND[_uniqueID].push_back(_idxEvt);
    }
    // Decide which one to keep given selection & selection On TOP
    for (auto & el : _CounterMultiPleCand_SELECTION_ONTOP_RND) {
        if (el.second.size() > 1) {
            // If more than 1 entries, this is a [evtNumber, runNumber] pair having multiple candidates... choose randomly it
            int to_keep = std::floor(rnd.Uniform(0, el.second.size()));
            _keep_entries_RND_onTop.push_back(el.second.at(to_keep));
        } else {
            _keep_entries_RND_onTop.push_back(el.second.front());   // first int index of entry to keep
        }
    }
    std::sort(_keep_entries_RND_onTop.begin(), _keep_entries_RND_onTop.end());

    // we have a list of entries To Keep randomly given the full Range-Selected candidates
    for (auto & el : _CounterMultiPleCand_SELECTION_RND) {
        if (el.second.size() != 1) {
            // in full range this entry has a multiple candidate, keep one randomly, is the one
            int  to_keep = std::floor(rnd.Uniform(0, el.second.size()));
            bool overlap = false;
            for (auto & entry : el.second) {
                if (CheckVectorContains(_keep_entries_RND_onTop, entry)) {
                    _keep_entries_RND.push_back(entry);
                    overlap = true;
                }
                if (overlap) break;
            }
            if (!overlap) { _keep_entries_RND.push_back(el.second.at(to_keep)); }
        } else {
            _keep_entries_RND.push_back(el.second.front());   // first int index of entry to keep
        }
    }
    if (_keep_entries_RND.size() < _keep_entries_RND_onTop.size()) {
        cout << RED << "SEVERE ERROR, onTop greater than before" << RESET << endl;
        abort();
    }
    std::sort(_keep_entries_RND.begin(), _keep_entries_RND.end());
    MessageSvc::Info("MultCandidate finder entries process, single candidate found ", to_string(_keep_entries_RND.size()));
    return make_pair(_keep_entries_RND, _keep_entries_RND_onTop);
}

CounterWeights CounterHelpers::ProcessMCDecayTuple(const ConfigHolder & _baseConfigHolder, const TString & _weightOption, const TString & _weightConfigSet) {
    TupleHolder MCDecayTuple(_baseConfigHolder, "", "MCT", "pro");
    MCDecayTuple.Init();
    if (MCDecayTuple.GetTuple() == nullptr) { MessageSvc::Error((TString) SettingDef::IO::exe, (TString) "Cannot initialize MCDecayTuple" + _baseConfigHolder.GetKey(), "EXIT_FAILURE"); }

    MessageSvc::Debug("ProcessMCDecayTuple", _weightOption);

    // HEAVY rely on WeightHolder doing the right things
    WeightHolder MCDecayTupleWeight_Base(_baseConfigHolder, _weightOption);
    WeightHolder MCDecayTupleWeight = MCDecayTupleWeight_Base.Clone(_weightConfigSet);
    MCDecayTupleWeight.PrintInline();
    Year    _year                          = _baseConfigHolder.GetYear();
    TString _WeightMCDecayTuple_Expression = TString("(double)") + MCDecayTupleWeight.Weight();
    TString _Selection_Expression          = "(1>0)";   //>0 all passed
    if (_year == Year::Y2011 || _year == Year::Y2012 || _year == Year::Run1) {
        _Selection_Expression = TString(CutDefRX::Trigger::Run1::nSPD);
    } else if (_year == Year::Y2015 || _year == Year::Y2016 || _year == Year::Run2p1) {
        _Selection_Expression = TString(CutDefRX::Trigger::Run2p1::nSPD);
    } else if (_year == Year::Y2017 || _year == Year::Y2018 || _year == Year::Run2p2) {
        _Selection_Expression = TString(CutDefRX::Trigger::Run2p2::nSPD);
    } else {
        MessageSvc::Error((TString) SettingDef::IO::exe, (TString) "Not suppoerted YEAR MCDECAYTUPLE", "EXIT_FAILURE");
    }
    MessageSvc::Info("ProcessMCDecayTuple, WEIGHT       for MCDECAYTUPLE ", _WeightMCDecayTuple_Expression);
    MessageSvc::Info("ProcessMCDecayTuple, WEIGHTConfig for MCDECAYTUPLE ", _weightConfigSet);
    MessageSvc::Info("ProcessMCDecayTuple, Selection    for MCDECAYTUPLE ", _Selection_Expression);

    ROOT::RDataFrame                 df(*MCDecayTuple.GetTuple());
    vector< pair< string, string > > variables   = {{"RUN_NUMBER", "(int)runNumber"}, {"EVENT_NUMBER", "(int)eventNumber"}, {"WEIGHT", _WeightMCDecayTuple_Expression.Data()}, {"SELECTION", _Selection_Expression.Data()}};
    auto                             df_defines  = ApplyDefines(df, variables);
    auto                             df_selected = df_defines.Filter(_Selection_Expression.Data());

    CounterWeights _MCDECAY_COUNTER("WEIGHT", _WeightMCDecayTuple_Expression, "SELECTION", _Selection_Expression, "MCDECAYTUPLE_DENOMINATOR");
    /*
     * take the vector<double> associated to the selection ( >0 = pas, < 0 = not pas )
     * take the vector<double> associated to the weights   ( 1. for noWeight         )
     */
    auto _Selection = df_selected.Take< bool >("SELECTION");
    auto _Weight    = df_selected.Take< double >("WEIGHT");
    /*
     * take the vector<int> for RunNumber  , EventNumber for Mult-Candidate inspection
     */
    auto _runNumber   = df_selected.Take< int >("RUN_NUMBER");
    auto _eventNumber = df_selected.Take< int >("EVENT_NUMBER");

    MessageSvc::Info("Processing MCDECAYTUPLE .... ");
    auto size = _eventNumber->size();
    MessageSvc::Info("Processing MCDECAYTUPLE .... DONE ", to_string(size), "");
    // Fill counters Of MCDecayTuple gls iven Weight and Selection ( no Mult-Candidates Removed... )
    _MCDECAY_COUNTER.Process(*_Weight, *_Selection);
    // Given RunNumber, EventNumber and Selection, find out the list of entries to keep!
    vector< int > _keep_entries_RND = CounterHelpers::EntriesToKeepList(*_runNumber, *_eventNumber, *_Selection);   // , nullptr);
    _MCDECAY_COUNTER.Process_RND(*_Weight, *_Selection, _keep_entries_RND);
    MCDecayTuple.Close();
    return _MCDECAY_COUNTER;
}

#endif
