#pragma once

#include "EnumeratorSvc.hpp"
#include "HelperSvc.hpp"
#include "HistogramSvc.hpp"
#include "IOSvc.hpp"
#include "MessageSvc.hpp"

#include "ConfigHolder.hpp"
#include "TupleReader.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "TObjArray.h"
#include "TObjString.h"
#include "TObject.h"
#include "TString.h"

#include "RooRealVar.h"

/**
 * \class TupleHolder
 * \brief Tuple info
 */
class TupleHolder : public TObject {

  public:
    /**
     * \brief Default constructor
     */
    TupleHolder();

    /**
     * \brief Constructor with ConfigHolder and TString
     */
    TupleHolder(const ConfigHolder & _configHolder, TString _tupleOption);

    /**
     * \brief Copy constructor
     */
    TupleHolder(const TupleHolder & _tupleHolder);

    /**
     * \brief Constructor with TString
     */
    TupleHolder(const ConfigHolder & _configHolder, TString _fileName, TString _tupleName, TString _tupleOption);

    /**
     * \brief Equality checkers
     */
    bool operator==(const TupleHolder & _tupleHolder) const { return (GetConfigHolder() == _tupleHolder.GetConfigHolder() && Option() == _tupleHolder.Option() && TupleDir() == _tupleHolder.TupleDir() && TupleName() == _tupleHolder.TupleName() && GetTupleReader() == _tupleHolder.GetTupleReader()); };

    bool operator!=(const TupleHolder & _tupleHolder) const { return !((*this) == _tupleHolder); };

    /**
     * \brief Init Tuple
     * @param  _fileName  [description]
     * @param  _tupleName [description]
     */
    void Init(bool _force = false, TString _fileName = "", TString _tupleName = "");

    void Close();

    void Reset();

    bool IsInitialized() const { return m_isInitialized; };

    /**
     * @brief      Useful call to move from DecayTuple to MCDecayTuple.
     *
     * @param[in]  _tupleName  The tuple name
     *
     * @return     Copy of this object.
     */
    TupleHolder GetMCDecayTupleHolder();

    /**
     * \brief Get option used to create Tuple
     */
    const TString Option() const { return m_tupleOption; };

    /**
     * \brief Get Tuple location
     */
    const TString TupleDir() const { return m_tupleDir; };

    /**
     * \brief Get Tuple name
     */
    const TString TupleName() const { return m_tupleName; };

    /**
     * \brief Get File name
     */
    const TString FileName() const { return m_fileName; };

    /**
     * \brief Get TupleReader
     */
    const TupleReader GetTupleReader() const { return m_tupleReader; };
    TupleReader &     GetTupleReader() { return m_tupleReader; };

    /**
     * \brief Get Tuple
     */
    TChain * GetTuple() const { return m_tupleReader.Tuple(); };

    TTree * GetTuple(TCut _cut, TString _tupleName = "");

    /**
     * \brief Get Reader
     */
    TTreeReader * GetReader() const { return m_tupleReader.Reader(); };

    /**
     * \brief Set option used to create Tuple
     * @param  _tupleOption [description]
     */
    void SetOption(TString _tupleOption) {
        m_tupleOption = _tupleOption;
        Check();
        return;
    };

    bool IsOption(TString _tupleOption) { return m_tupleOption.Contains(_tupleOption); };

    ConfigHolder GetConfigHolder() const { return m_configHolder; };

    void SetTupleName(TString _tupleName) {
        m_tupleName = _tupleName;
        return;
    };

    void SetFileName(TString _fileName) {
        m_fileName = _fileName;
        return;
    };

    /**
     * \brief Set Branches
     * @param  _branches [description]
     */
    void SetBranches(vector< TString > _branches = {});

    /**
     * \brief Get Branches to keep active
     * @param  _option [description]
     */
    vector< TString > GetBranches(TString _option);

    void CheckBranches(vector< TString > _branches = {}, bool _deep = false);

    const vector< TString > Branches() const { return m_branches; };

    void ResetBranches() {
        SetBranches(m_branches);
        return;
    };

    const bool IsSampleInCreVer(TString _creVer, TString _prj, TString _ana, TString _q2bin, TString _year , TString _trigger, TString _triggerConf, TString _sample);

    /**
     * \brief Set Aliases
     * @param  _aliases [description]
     */
    void SetAliases(vector< pair< TString, TString > > _aliases = {});

    /**
     * \brief Get Aliases
     */
    vector< pair< TString, TString > > GetAliases(TString _option);

    const vector< pair< TString, TString > > Aliases() const { return m_aliases; };

    void ResetAliases() {
        SetAliases(m_aliases);
        return;
    };

    const Long64_t TupleEntries(TCut _cut = "");

    const int TupleFiles() const { return m_tupleReader.GetNFiles(); };

    void CreateSubTupleReader(int _iFile);

    /**
     * \brief Check if Var is in Tuple
     * @param  _name [description]
     */
    bool CheckVarInTuple(TString _name) { return m_tupleReader.CheckVarInTuple(_name); };

    /**
     * \brief Returns the luminosity and its error
     */
    pair< double, double > GetLuminosity();

    vector<TString> GetFileNames() const; 

    void PrintInline() const noexcept;

  private:
    ConfigHolder m_configHolder = ConfigHolder();   // The underlying ConfigHolder

    TupleReader m_tupleReader = TupleReader();   //! The tuple Reader object , not persified, recreated on the fly

    TString m_tupleOption = "";   // Tuple option driving

    TString m_tupleDir = "";   // Tuple directory type

    TString m_tupleName = "";   // Tuple Name

    TString m_fileName = "";   // File Name

    TString m_fileList = "eos.list";   // File list name to use in case of eos parsed list of files
  
    vector< TString >                  m_branches = {};   // Branch status
    vector< pair< TString, TString > > m_aliases  = {};   // Aliases

    /**
     * \brief Check allowed arguments
     */
    bool Check();

    bool m_debug = false;
    /**
     * \brief Activate debug
     * @param  _debug [description]
     */
    void SetDebug(bool _debug) { m_debug = _debug; };

    bool m_isInitialized = false;   //! Initialization flag [false by default]

    /**
     * \brief Create Tuple name
     */
    void CreateTupleName();

    /**
     * \brief Create Tuple location
     * @param  _fileName [description]
     */
    void CreateTupleDir(TString _fileName = "");

    /**
     * \brief Create TupleReader
     * @param  _fileName  [description]
     * @param  _tupleName [description]
     */
    void CreateTupleReader(TString _fileName = "", TString _tupleName = "");

    void UpdateBranches();

    void UpdateAliases();

    ClassDef(TupleHolder, 1);
};

ostream & operator<<(ostream & os, const TupleHolder & _tupleHolder);
