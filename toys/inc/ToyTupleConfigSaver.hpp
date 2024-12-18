#ifndef TOYTUPLECONFIGSAVER_HPP
#define TOYTUPLECONFIGSAVER_HPP

#include "ConfigHolder.hpp"
#include "SettingDef.hpp"
#include "TFile.h"
#include "ToyFileHandler.hpp"
#include "ToyTupleConfig.hpp"

/**
 * \class ToyTupleConfigSaver
 * \brief Saves a vector of ToyTupleConfig belonging to the same category.
 * \brief File path is configured using an instance of ConfigHolder passed.
 * \brief Internal checks are made to ensure no \c Sample keys and \c PDF keys of the ToyTupleConfig are not duplicated.
 */
class ToyTupleConfigSaver {

  public:
    /**
     * \brief Default constructor.
     */
    ToyTupleConfigSaver();
    /**
     * \brief Constructs an instance of ToyTupleConfigSaver and configures it with the ConfigHolder passed.
     */
    ToyTupleConfigSaver(const ConfigHolder & _configHolder);
    /**
     * \brief Copy constructor.
     */
    ToyTupleConfigSaver(const ToyTupleConfigSaver & _other);
    /**
     * \brief Move constructor.
     */
    ToyTupleConfigSaver(ToyTupleConfigSaver && _other);
    /**
     * \brief Destructor.
     */
    ~ToyTupleConfigSaver(){};

    void Init();
    /**
     * \brief Print the configurations saved and new configurations added.
     */
    void PrintSaved();
    void PrintToSave();
    /**
     * \brief Adds a vector of ToyTupleConfig to the new ToyTupleConfig queue to save.
     */
    void AddConfigurations(const vector< ToyTupleConfig > & componentConfigs);
    /**
     * \brief Adds a single ToyTupleConfig to the new ToyTupleConfig queue to save.
     * \brief Internal checks are made so that \c Sample and \c PDF keys are not duplicated among saved ToyTupleConfig and ToyTupleConfig queued.
     */
    void AddConfiguration(const ToyTupleConfig & componentConfig);
    /**
     * \brief Updates the ROOT file with new entries of ToyTupleConfig.
     */
    void Update(TString _key = "");
    /**
     * \brief Updates the ROOT file with new entries of ToyTupleConfig.
     */
    void Append(TString _key);
    /**
     * \brief Overwrites the ROOT file
     */
    void Overwrite();
    /**
     * \brief Equal operator.
     */
    ToyTupleConfigSaver & operator=(const ToyTupleConfigSaver & _other);
    /**
     * \brief Move equal operator.
     */
    ToyTupleConfigSaver & operator=(ToyTupleConfigSaver && _other);

  private:
    bool                     NotAdded(const ToyTupleConfig & tupleConfig) const;
    bool                     TupleConfigNotFoundIn(const ToyTupleConfig & tupleConfig, const vector< ToyTupleConfig > & listOfTupleConfigs) const;
    bool                     SampleMatches(const ToyTupleConfig & allocatedConfig, const ToyTupleConfig & newConfig) const;
    void                     MatchWarning(TString keyType, TString key) const;
    bool                     PdfKeyMatches(const ToyTupleConfig & allocatedConfig, const ToyTupleConfig & newConfig) const;
    bool                     ObservableKeyMatches(const ToyTupleConfig & tupleConfig);
    bool                     ObservableKeySet() const;
    bool                     CheckObservableKey(const ToyTupleConfig & tupleConfig) const;
    void                     SetObservableKey(const ToyTupleConfig & tupleConfig);
    void                     WarnKeyNotSame(TString configObservableKey) const;
    vector< ToyTupleConfig > GetConfigurationsToUpdate() const;
    bool                     ConfigNotSaved(const ToyTupleConfig & _tupleConfig) const;
    void                     MainConfigTree(TString _key);
    void                     UpdateOpenConfigTree(TString _key);
    void                     AttachConfigurationToTree();
    void                     WriteConfigurations(const vector< ToyTupleConfig > & _configurations);
    void                     CloseFile();
    void                     UpdateSavedConfigurations();
    void                     RecreateOpenConfigTree();
    void                     OverwriteSavedConfigurations();

  private:
    ConfigHolder   m_configHolder;
    ToyFileHandler m_fileHandler;

    vector< ToyTupleConfig > m_configurationsToSave;
    vector< ToyTupleConfig > m_configurationsSaved;

    ToyTupleConfig   m_configurationInTree;
    TTree *          m_configTree = nullptr;
    TDirectoryFile * m_dir = nullptr;

    const char * m_configTreeName = SettingDef::Tuple::TT;

    const TString m_observableKeyUnsetFlag = "observableUnset";
    TString       m_observableKey;
};

#endif
