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

typedef vector<TString> v_tstr;

/**
 * \class TupleHolder
 * \brief Tuple info
 */
class TupleHolder : public TObject 
{
  public:
    /**
     * \brief Default constructor
     */
    TupleHolder();

    /**
     * @brief Constructor with ConfigHolder and TString.
     * @param _configHolder Instance of ConfigHolder, all the configuration will be taken from it
     * @param _tupOption    String with extra configuration
     */
    TupleHolder(const ConfigHolder & _configHolder, const TString &_tupleOption);

    /**
     * \brief Copy constructor
     */
    TupleHolder(const TupleHolder & _tupleHolder);

    /**
     * \brief Constructor with TString
     */
    TupleHolder(
            const ConfigHolder & _configHolder, 
            const TString      & _fileName, 
            const TString      & _tupleName, 
            const TString      & _tupleOption);

    /**
     * \brief Equality checkers
     */
    bool operator==(const TupleHolder & _tupleHolder) const { return (GetConfigHolder() == _tupleHolder.GetConfigHolder() && Option() == _tupleHolder.Option() && TupleDir() == _tupleHolder.TupleDir() && TupleName() == _tupleHolder.TupleName() && GetTupleReader() == _tupleHolder.GetTupleReader()); };

    bool operator!=(const TupleHolder & _tupleHolder) const { return !((*this) == _tupleHolder); };

    /**
     * \brief Init Tuple
     * @param  _force     [description]
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
        _Check();
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
     * @brief Will set the aliases of the tuple reader instance.
     * @param  _aliases This is a vector of pairs of strings, where the first one is the original
     * name and the second one is the alias.\n 
     * If the alias is empty, will use the attribute. Otherwise it will override it.
     */
    void SetAliases(const vector< pair< TString, TString > > &_aliases = {});

    /**
     * @brief Get correspondence between branch names, needed for renaming.
     */
    vector< pair< TString, TString > > GetAliases(const TString &_option);

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

    /**
     * @brief Prints tuple holder and config holder options
     */
    void PrintInline() const noexcept;

  private:
    /**
     * Will fill m_branches with all the branch names in tree if both:
     * - m_branches is empty
     * - Branches were found in tree
    */
    void _SetAllBranchesFromTree();

    /**
     * @brief returns name of TTree for cre jobs 
     */
    TString _TupleNameForCRE();

    /**
     * @brief Retrieves name of sample after some renaming
     */
    TString _GetSampleName();

    /**
     * @brief For ganga files, this will rename sample name
     * @return Nothing, it will modify in-place 
     */
    void _RenameGangaSampleName(TString &_sample);

    /**
     * @brief Will make TupleReader if tuple name and file name are non-empty
     * @return True if the tuple and file names are specified and therefore the reader can be made, false otherwise.
     */
    bool _MakeReaderFromPaths();

    /**
     * @brief Will add list of files to reader, when the sample is Ganga
     * @param _sample Sample name, will need to be renamed internally, has to be copied
     * @param _type DT or MC, data or montecarlo
     * @param _years years to loop over
     * @param _polarities polarities to loop over
     */
    bool _AddGangaSamplesToReader(
                  TString _sample, 
            const TString &_type, 
            const v_tstr  &_years,
            const v_tstr  &_polarities);

    /**
     * @brief Helper function to make reader from production (?) samples
     * @param _sample Sample name, will need to be renamed internally, has to be copied
     * @param _type DT or MC, data or montecarlo
     * @param _years years to loop over
     * @param _polarities polarities to loop over
     */
    bool _AddProSamplesToReader(
                  TString _sample, 
            const TString &_type, 
            const v_tstr  &_years,
            const v_tstr  &_polarities);

    /**
     * @brief Helper function to make reader from created (?) samples
     * @param _sample Sample name, will need to be renamed internally, has to be copied
     * @param _type DT or MC, data or montecarlo
     * @param _years years to loop over
     * @param _polarities polarities to loop over
     */
    bool _AddCreSamplesToReader(
                  TString _sample, 
            const TString &_type, 
            const v_tstr  &_years,
            const v_tstr  &_polarities);

    /**
     * @brief Helper function to make reader from splot (?) samples
     * @param _sample Sample name, will need to be renamed internally, has to be copied
     * @param _type DT or MC, data or montecarlo
     * @param _years years to loop over
     * @param _polarities polarities to loop over
     */
    bool _AddSplSamplesToReader(
                  TString _sample, 
            const TString &_type, 
            const v_tstr  &_years,
            const v_tstr  &_polarities);

    /**
     * @brief Helper function to make reader from samples from post_ap, i.e. AP and filtering/slimming step in Run3
     * @param _sample Sample name, will need to be renamed internally, has to be copied
     * @param _type DT or MC, data or montecarlo
     * @param _years years to loop over
     * @param _polarities polarities to loop over
     */
    bool _AddPAPSamplesToReader(
              TString _sample, 
        const TString &_type, 
        const v_tstr  &_years,
        const v_tstr  &_polarities);

    /**
     * @brief Function used to check if samples were added, If samples were cached, check is skipped. 
     * If samples are background MC, check only shows warning
     * @param _addList True if samples were added, false otherwise
     */
    void _CheckAddedList(const bool &_addList);
    /**
     * Will initialize values of m_fileName and m_tupleName
     *
     * @param  _fileName  Name of file, e.g. file.root 
     * @param  _tupleName Name of tree, e.g. DecayTree
    */
    void _SetFileTupleNames(const TString &_fileName, const TString &_tupleName);

    /**
     * @brief Will return the value that m_tupleName will take.
     * @details This should depend on what constructor and what data period, (Run1/2/3) is used
     */
    TString _GetTuplePAPName();

    //const ConfigHolder m_configHolder; // This should be const eventually, currently does not work for unknown reasons
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
    void _Check();

    bool m_isInitialized = false;   //! Initialization flag [false by default]

    /**
     * Sets value of m_tupleName, i.e. location of tree in file, when 
     * the files are "processed", i.e. m_tupleOption.Contains("pro") is true.
    */
    void _CreateTupleProcessName();

    /**
     * Sets value of m_tupleName, i.e. location of tree in file, when 
     * the files are from ganga, i.e. m_tupleOption.Contains("gng") is true.
     */
    void _CreateTupleGangaName();

    /**
     * Sets value of m_tupleName, i.e. location of tree in file, when 
     * the files are used for Rpk and RKShort (?)
     */
    void _CreateTupleRLRKSName();
    /**
     * @brief Returns path to file with aliases, e.g. X -> Y in order to rename branches
     */
    TString _GetAliasFile(const TString &_option);

    /**
     * @brief Set value of m_tupleDir, uses m_fileName to extract directory path
     */
    void _SetTupleDir();

    /**
     * @brief Will ask ConfigHolder attribute for the project and will check if, for the given aliases,
     * it has to be skipped
     * @param Name of branch that will be defined (alias) that might be skipped.
     * @param String with definition for of alias argument
     * @return true (skip), false (define the aliases)
     */
    bool _SkipAliasForProject(const TString & _alias, const TString & _expr);

    /**
     * @brief Will ask ConfigHolder attribute for the analysis and will check if, for the given aliases,
     * it has to be skipped
     * @param Name of branch that will be defined (alias) that might be skipped.
     * @param String with definition for of alias argument
     * @return true (skip), false (define the aliases)
     */
    bool _SKipAliasForAnalysis(const TString & _alias, const TString & _expr);
    /**
     * \brief Create TupleReader
     */
    void _CreateTupleReader();

    ClassDef(TupleHolder, 1);
};

ostream & operator<<(ostream & os, const TupleHolder & _tupleHolder);
