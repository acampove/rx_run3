#ifndef TUPLEPROCESS_HPP
#define TUPLEPROCESS_HPP

#include "ConstDef.hpp"
#include "CutDefRX.hpp"
#include "WeightDefRX.hpp"

#include "HelperSvc.hpp"
#include "HistogramSvc.hpp"
#include "IOSvc.hpp"
#include "MessageSvc.hpp"
#include "PhysicsSvc.hpp"

#include "EventType.hpp"
#include "VeloMaterial.hpp"

#include <iostream>

#include "TH2Poly.h"
#include "TMath.h"
#include "TRandom3.h"
#include "TString.h"

/**
 * \struct BranchInfo
 * \brief Branch Info
 */
template < typename T > struct BranchInfo {
    TString name;
    T *     addr;
    int     size;
};

/**
 * \typedef Str2BranchMapI
 * \brief Template map for TString to BranchInfo<int>
 */
typedef map< TString, BranchInfo< int > > Str2BranchMapI;

/**
 * \typedef Str2BranchMapD
 * \brief Template map for TString to BranchInfo<double>
 */
typedef map< TString, BranchInfo< double > > Str2BranchMapD;

/**
 * \typedef Str2BranchMapVI
 * \brief Template map for TString to BranchInfo<vector<int>>
 */
typedef map< TString, BranchInfo< vector< int > > > Str2BranchMapVI;

/**
 * \typedef Str2BranchMapVD
 * \brief Template map for TString to BranchInfo<vector<double>>
 */
typedef map< TString, BranchInfo< vector< double > > > Str2BranchMapVD;

/**
 * \struct SetBranchInfo
 * \brief Branch Info
 */
template < typename T, typename S > void SetBranchInfo(T & _map, TString _name, S & _addr, TTree * _tree, int _size = 1) {
    if (!_tree->GetListOfBranches()->Contains(_name)) {
        _map[_name].name = _name;
        _map[_name].addr = &_addr;
        _map[_name].size = _size;
    } else {
        MessageSvc::Warning(_name, (TString) "Already present in tree", "SKIPPING");    
    }
    return;
};

/**
 * \class TupleProcess
 * \brief Tuple info
 */
class TupleProcess {

  public:
    /**
     * \brief Default constructor
     */
    TupleProcess() = default;

    /**
     * \brief Constructor with EventType
     */
    TupleProcess(const EventType & _eventType, TString _option);

    /**
     * \brief Constructor with TString
     */
    TupleProcess(TString _fileName, TString _option);

    /**
     * \brief Copy constructor
     */
    TupleProcess(const TupleProcess & _tupleProcess);

    /**
     * \brief Init Tuple
     */
    void Init();

    /**
     * \brief Process Tuple
     */
    void Process();

    /**
     * \brief Get EventType
     */
    const EventType GetEventType() const { return m_eventType; };

    /**
     * \brief Get file name
     */
    const TString FileName() const { return m_fileName; };

    /**
     * \brief Set file name
     * @param  _option [description]
     */
    void SetFileName(TString _fileName) { m_fileName = _fileName; };

    /**
     * \brief Get option used to process Tuple
     */
    const TString Option() const { return m_option; };

    /**
     * \brief Set option used to process Tuple
     * @param  _option [description]
     */
    void SetOption(TString _option) { m_option = _option; };

    bool IsOption(TString _option) { return m_option.Contains(_option); };

  private:
    EventType m_eventType;

    TString m_option;

    TString m_optionWEFF   = "-effr";
    TString m_optionWRATIO = "-ratior";

    TString m_fileName = TString("");

    vector< TString > m_branches     = {};
    vector< TString > m_branchesSKIM = {};

    vector< pair< TString, TString > > m_aliasesSKIM = {};


    TRandom3 m_rnd;
    TRandom3 m_rndPoisson;
    TRandom3 m_rndTCK;

    const int m_kFolds = 10;

    bool m_bootstrapEvt = true;
    bool m_bootstrapMap = SettingDef::Weight::useBS;

    bool m_copyMode = false;
    /**
     * \brief Check allowed arguments
     */
    bool Check();

    bool m_debug = true;
    /**
     * \brief Activate debug
     * @param  _debug [description]
     */
    void SetDebug(bool _debug) { m_debug = _debug; };

    template < typename T > bool AddMap(T & _map, TString _name, TTree * _tuple);

    bool AddAlias(vector< pair< TString, TString > > & _aliases, TString _name, TTree * _tuple);

    TTree * GetTupleFiltered(EventType & _eventType, TString _tupleName);

    TTree * GetTupleProcessDT(EventType & _eventType, TupleReader & _tupleReader, TFile & _tFile, TString _tupleName);
    TTree * GetTupleProcessMCT(EventType & _eventType, TupleReader & _tupleReader, TFile & _tFile, TString _tupleName);
    TTree * GetTupleProcessRST(EventType & _eventType, TupleReader & _tupleReader, TFile & _tFile, TString _tupleName);
};

ostream & operator<<(ostream & os, const TupleProcess & _tupleProcess);

#endif
