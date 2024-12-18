#ifndef TOYSTUDY_HPP
#define TOYSTUDY_HPP

#include "FitGenerator.hpp"
#include "FitterTool.hpp"
#include "FitResultLogger.hpp"
#include "ToyReader.hpp"

#include "VariableResetter.hpp"
#include "VariableSmearer.hpp"
#include "CorrelatedVariablesSmearer.hpp"

/**
 * \class ToyStudy
 * \brief Responsible for the toy fit loop.
 * \brief Fitting is delegated to FitterTool while nTuple reading is ToyReader's responsibility.
 * \brief VariableResetter resets the variable to values before a toy fit while VariableSmearer smears the central values of constraints.
 * \brief FitResultLogger logs the fit results across multuple toy loops and output as a nTuple.
 */
class ToyStudy {
  public:
    /**
     * \brief Default constructor.
     */
    ToyStudy();
    /**
     * \brief Destructor.
     */
    ~ToyStudy(){};

    void Init();

    /**
     * \brief Setup the fitter via the YAML file paths found in SettingDef::Fit::yamls.
     * \brief One FitManager is constructed and added to FitGenerator per YAML file found.
     */
    void SetupFitter();
    /**
     * \brief Setup the ToyReader via YAML ToyReader::GetToyTupleReaders().
     */
    void SetupReader();
    /**
     * \brief Check that FitterTool and ToyReader keys matches.
     * \brief This must be executed before calling ToyStudy::Fit().
     * \brief Checks that all q2Bin matches and ConfigHolder::GeyKey() matches in all the q2Bins.
     */
    void CheckKeys();
    /**
     * \brief Synchronise the observables used in Datasets and Model
     */
    void SynchroniseObservable();
    /**
     * \brief Changes the current index used to read toys.
     */
    void SetIndex(unsigned int newIndex);
    /**
     * \brief Performs one loop of toy fit.
     * \brief Fit variables are reset to their initial values and constraint means are smeared.
     * \brief Toy data is produced by reading toy nTuples.
     * \brief A minimization is performed and the fit result is stored to FitResultLogger.
     * \brief The toy index is incremented so successive calls to this function would read different input nTuples.
     */
    void Fit(uint _index);
    /**
     * \brief Saves all the fit results across all toy loops in a nTuple.
     */
    void SaveResults(TString _name = "");
    /**
     * \brief Changes the seed of the constraint smearer
     */
    void SetSeed(uint32_t seed);

  private:
    void              ResetCheckedStatus();
    void              GenerateFitterTool();
    void              SetupResetterAndLogger();
    void              LoadNominalResult();
    void              LoadConstraintsOverwrite();
    TFile *           GetNominalResultFile();
    vector <TString>  GetResultParNames(TFile * _file) const;
    void              FillNominalResultMap(TFile * _file, const vector <TString> & _parNames);
    void              SetupConstraintSmearer();
    bool              OverwriteConstraint(TString _name);
    bool              IsInNominal(TString _name);
    void              PrintManagerKeys() const;
    void              CheckManagerKeysMatches() const;
    vector< TString > GetFitterManagerKeys() const;
    vector< TString > GetReaderManagerKeys() const;
    void              CheckKeysMatches(const vector< TString > & managerKeys, const vector< TString > & readerKeys) const;
    void              ThrowIfKeyNotFound(TString key, const vector< TString > & keyContainer) const;
    void              PrintFitInfoKeys() const;
    void              PrintKeysInManager(TString _managerKey) const;
    void              CheckFitInfoKeysMatches() const;
    void              CheckKeysInManagerMatches(TString _managerKey) const;
    vector< TString > GetFitterFitInfoKeys(TString _managerKey) const;
    vector< TString > GetReaderFitInfoKeys(TString _managerKey) const;
    void              GetNextToyData(uint _index);
    void              ResetFitter();
    void              ResetPDFs();
    void              SmearVariables();
    void              SaveCurrentToyResult();
    bool              ShouldWriteFullOutput(const RooFitResult & _fitResults) const;
    void              ConfigurePlotsDirectory();
    void              SaveRooFitResults(RooFitResult & _fitResults) const;
    void              CheckKeyMessage() const;

  private:
    OpenMode m_openMode = OpenMode::WARNING;

    FitGenerator m_fitGenerator;
    FitterTool * m_fitter = nullptr;

    ToyReader         m_reader;
    FitResultLogger   m_logger;
    VariableResetter  m_resetter;

    map< TString, QSquareFit > * m_managerMap;
    vector< VariableSmearer >    m_variableSmearers;
    vector< CorrelatedVariablesSmearer > m_correlatedVariablesSmearers;
    map< TString, double > m_nominalResult;
    map< TString, pair<double, double> > m_overwriteConstraints;
    bool m_keysChecked;

    const int m_logFrequency = 50;
    int m_studyIndex;
    int m_toyIndex;
};

#endif