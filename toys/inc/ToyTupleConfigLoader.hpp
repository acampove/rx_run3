#ifndef TOYTUPLECONFIGLOADER_HPP
#define TOYTUPLECONFIGLOADER_HPP

#include "ConfigHolder.hpp"
#include "SettingDef.hpp"
#include "TTree.h"
#include "ToyFileHandler.hpp"
#include "ToyTupleConfig.hpp"
#include <vector>

class TTree;

/**
 * \class ToyTupleConfigLoader
 * \brief Loads all the ToyTupleConfig stored within a ROOT file.
 * \brief The file path to the ROOT file is configured by passing an instance of ConfigHolder.
 */
class ToyTupleConfigLoader {

  public:
    /**
     * \brief Default constructor.
     */
    ToyTupleConfigLoader();
    /**
     * \brief Constructs an instance of ToyTupleConfigLoader and configures it's file path.
     */
    ToyTupleConfigLoader(const ConfigHolder & _configHolder);
    /**
     * \brief Copy constructor. Previously loaded configurations are not copied.
     */
    ToyTupleConfigLoader(const ToyTupleConfigLoader & _other);
    /**
     * \brief Move constructor. Previously loaded configurations are not moved.
     */
    ToyTupleConfigLoader(ToyTupleConfigLoader && _other);
    /**
     * \brief Destructor.
     */
    ~ToyTupleConfigLoader(){};

    void Init();

    /**
     * \brief Loads the ToyTupleConfig found in the file path configured.
     */
    vector< ToyTupleConfig > LoadConfigs();
    /**
     * \brief Loads the RooWorkspace in the toy configuration file.
     */
    RooWorkspace * LoadWorkspace();
    /**
     * \brief Loads the ToyTupleConfig found in the file path configured without loading the PDFs.
     */
    vector< ToyTupleConfig > LoadConfigsWithoutPdfs();
    /**
     * \brief Prints the configurations previously loaded by this instance of ToyTupleConfigLoader.
     */
    void PrintLoaded() const;
    /**
     * \brief Equal operator. Previously loaded configurations are not copied.
     */
    ToyTupleConfigLoader & operator=(const ToyTupleConfigLoader & _other);
    /**
     * \brief Move equal operator. Previously loaded configurations are not moved.
     */
    ToyTupleConfigLoader & operator=(ToyTupleConfigLoader && _other);

  private:
    ConfigHolder m_configHolder;

    vector< ToyTupleConfig > m_configurationsLoaded;

    TTree * m_configTree = nullptr;
    TFile * m_configFile = nullptr;

    RooWorkspace * GetWorkspace(ToyFileHandler& _toyFileHandler);
    void ClearConfigurations();
    void GetConfigTree(ToyFileHandler& _fileHandler);
    void GetConfigInTree(bool _loadPdf);
    void CloseFile(ToyFileHandler& _toyFileHandler);
    const char * m_configTreeName = SettingDef::Tuple::TT;
};

#endif
