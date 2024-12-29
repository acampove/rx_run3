#pragma once

#include "EnumeratorSvc.hpp"
#include "HelperSvc.hpp"
#include "IOSvc.hpp"
#include "MessageSvc.hpp"

#include <glob.h>
#include <iostream>
#include <vector>

#include "TChain.h"
#include "TChainElement.h"
#include "TEnv.h"
#include "TFile.h"
#include "TRandom3.h"
#include "TString.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreeReaderArray.h"
#include "TTreeReaderValue.h"

#include <ROOT/RDataFrame.hxx>

/**
 * \typedef FUNC_PTR
 * \brief Template function for MultipleCandidates
 */
typedef Long64_t (*FUNC_PTR)(TChain *, vector< Long64_t >, TRandom3 &);

/**
 * \class TupleReader
 * \brief Tuple info
 */
class TupleReader {

  public:
    /**
     * \brief Default constructor
     */
    TupleReader();

    /**
     * \brief Constructor with TString
     */
    TupleReader(const TString &_tupleName, const TString &_fileName = "");

    /**
     * \brief Constructor with TChain
     */
    TupleReader(TChain * _tuple);

    /**
     * \brief Copy constructor
     */
    TupleReader(const TupleReader & _tupleReader);

    /**
     * \brief Equality checkers
     */
    bool operator==(const TupleReader & _tupleReader) const { return 1; };

    bool operator!=(const TupleReader & _tupleReader) const { return !((*this) == _tupleReader); };

    /**
     * \brief Init Tuple
     */
    void Init();

    void Close();

    bool IsInitialized() const { return m_isInitialized; };

    /**
     * Check if Var is in Tuple
     * @param  _name [description]
     */
    bool CheckVarInTuple(TString _name);

    /**
     * Check if Var is in Tuple
     * @param  _name [description]
     */    
    
    bool CheckVarsInChain(const vector<TString> & _columns, const TString _theTupleName);

    /**
     * \brief Get Tuple
     */
    TChain * Tuple() const { return m_tuple; };

    /**
     * \brief Get TTreeReader
     */
    TTreeReader * Reader() const { return m_reader; };

    template < typename T > TTreeReaderArray< T > GetArray(TString _name) { return TTreeReaderArray< T >(*m_reader, _name); };

    template < typename T > TTreeReaderValue< T > GetValue(TString _name) { return TTreeReaderValue< T >(*m_reader, _name); };

    /**
    * \brief Get TTreeReaderValue Pointer.
       It overcome the issue of not having the Branch in the Tuple but still be able to declare a TTreeReaderValue which doesn't get called in the ->Next() when looping.
       This is a rather risky method since if in the loop you try to access the nullptr, you will get segfaults. (Mainly used in TupleProcess)
    */
    template < typename T > TTreeReaderValue< T > * GetValuePtr(TString _name) {
        if (CheckVarInTuple(_name)) {
            auto _readerV = new TTreeReaderValue< T >(*m_reader, _name);
            return _readerV;
        }
        MessageSvc::Warning("TupleReader", (TString) "GetValuePtr", _name, "not in tuple");
        return nullptr;
    };

    template < typename T > TTreeReaderArray< T > * GetArrayPtr(TString _name) {
        if (CheckVarInTuple(_name)) {
            auto _readerV = new TTreeReaderArray< T >(*m_reader, _name);
            return _readerV;
        }
        MessageSvc::Warning("TupleReader", (TString) "GetArrayPtr", _name, "not in tuple");
        return nullptr;
    }

    /**
     * \brief Returns a vector which contains all values of all events in a variabler branch in the TChain/TTree. Very bad and very slow, avoid if possible.
     */
    vector< vector< double > > GetVariableVector(vector< TString > & _variableNames, TCut _cut = "(1)", int _nCores = 1);

    /**
     * @brief Will take a TChain, unpack the paths to files and add it to current instance's TChain
     * @param  _tuple Pointer to TChain, whose files need to be appended to m_tuple TChain 
     */
    void AddTuple(TChain * _tuple);

    /**
     * @brief Will take path to ROOT file and add it to member TChain if checks passed
     * @param  Path to ROOT file 
     */
    bool AddFile(const TString &_fileName);

    /**
     * Add file
     * @param  _fileName  [description]
     * @param  _tupleName [description]
     */
    bool AddFile(TString _fileName, TString _tupleName);

    /**
     * @brief Takes wildcard to ROOT files, it will be globed and the files will be added to member TChain 
     * @param  Wildcard to ROOT files 
     */
    void AddFiles(const TString &_fileName);

    /**
     * Add file
     * @param  _fileName [description]
     * @param  _iFile    [description]
     */
    void AddFile(TString _fileName, int _iFile);

    /**
     * @brief Adds ROOT files to TupleReader object 
     * @param  _fileName Path to text file where each line is the path to a ROOT file
     */
    bool AddList(const TString &_fileName);

    /**
     * Add friend
     * @param  _tuple [description]
     */
    void AddFriend(TChain * _tuple);

    /**
     * Add friend
     * @param  _fileName [description]
     * @param  _tupleName [description]
     */
    void AddFriend(TString _fileName, TString _tupleName = "");

    /**
     * Set branch status
     * @param  _branches [description]
     * @param  _status   [description]
     * @param  _option   [description]
     */
    int SetBranches(vector< TString > _branches, bool _status = true, TString _option = "");

    /**
     * Set aliases
     * @param  _aliases [description]
     * @param  _option  [description]
     */
    int SetAliases(vector< pair< TString, TString > > _aliases, TString _option = "");

    TTree * CopyTuple(TCut _cuts = "", TString _tupleName = "", double _frac = -1, bool _replace = false);

    TTree * SnapshotTuple(TCut _cuts = "", TString _tupleName = "", double _frac = -1, bool _replace = false, vector< TString > _branchesToKeep = {}, vector< TString > _aliasesToKeep = {});

    TTree * CloneTuple(TString _tupleName = "");

    map< TString, TTree * > SplitTuple(const map< TString, SplitInfo > & _selections, double _frac = -1, bool _hadd = false, vector< TString > _branchesToKeep = {}, vector< TString > _aliasesToKeep = {}, bool _replace = false, TString _selection = "");

    TTree * GetMultCandTuple(vector< FUNC_PTR > & _function, vector< TString > & _name);
    TTree * GetMultCandTuple(FUNC_PTR & _function, TString & _name) { return GetMultCandTuple({_function}, {_name}); };

    Long64_t GetEntries(TCut _cut = "") const { return m_tuple->GetEntries(_cut); };

    void SetEntries(Long64_t _maxEntries = 0);

    inline int GetEntry(Long64_t _entry) { return m_tuple->GetEntry(_entry); };

    bool GetAddTuple() const { return m_addTuple; };

    void SetAddTuple(bool _flag) { m_addTuple = _flag; };

    double GetFrac() const { return m_frac; };

    vector< TString > GetFileNames() const { return m_fileNames; };

    int GetNFiles() const { return m_fileNames.size(); };

    TString GetFileName(int _iFile);

    int GetNFiles(TString _fileName);

    void PrintInline() const noexcept;

    /**
     * @brief Print list of files
     */
    void PrintListOfFiles() const noexcept;

  private:
    TChain * m_tuple = nullptr;

    TTreeReader * m_reader = nullptr;   //! do not persist

    bool m_addTuple = true;

    double m_frac = 0;

    vector< TString > m_fileNames = {};

    TRandom3 m_rnd;

    bool m_fileRecover = false;

    bool _CheckAddFile(const TString &_fileName);
    void SetFileRecover();

    bool m_useXrootD = true;

    TString GetXrootD(TString _fileName);

    bool m_debug = false;
    /**
     * \brief Activate debug
     * @param  _debug [description]
     */
    void SetDebug(bool _debug) { m_debug = _debug; };

    /**
     * @brief If the tuple has friends, will check:\n
     * - Does friend and main tree have same number of entries?
     */
    void _CheckFriends();

    bool m_isInitialized = false;   //! Initialization flag [false by default]

    void Size();
};

ostream & operator<<(ostream & os, const TupleReader & _tupleReader);
