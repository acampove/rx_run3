#ifndef TOYFILEHANDLER_HPP
#define TOYFILEHANDLER_HPP

#include "ConfigHolder.hpp"

#include "TString.h"
class TFile;
/**
 * \class ToyFileHandler
 * \brief Handles all the file opening and closing for the toy classes.
 * \brief Handles two types of files:
 * \li Toy nTuple files (TupleFile)
 * \li ToyTupleConfig files (ConfigFile)
 *
 * \brief Ownership of the file objects belongs to this class.
 * \brief All _other classes should not close or delete the file objects.
 */
class ToyFileHandler {
  public:
    /**
     * \brief Default constructor.
     */
    ToyFileHandler();
    /**
     * \brief Calls the default constructor and configures the ToyFileHandler using the ConfigHolder passed.
     */
    ToyFileHandler(const ConfigHolder & _configHolder);
    /**
     * \brief Copy constructor.
     */
    ToyFileHandler(const ToyFileHandler & _other);
    /**
     * \brief Move constructor.
     */
    ToyFileHandler(ToyFileHandler && _other);
    /**
     * \brief Destructor. All files opened by an instance of this class is closed automatically.
     */
    ~ToyFileHandler();

    void Init();

    void Print();

    /**
     * \brief Configures the open mode used for opening Toy configuration .root files.
     */
    void SetConfigOpenMode(OpenMode openMode);
    
    /**
     * \brief Configures the open mode used for opening Toy nTuple .root files.
     */
    void SetTupleOpenMode(OpenMode openMode);
    /**
     * \brief Returns the .root file which stores ToyTupleConfig.
     */
    TFile * GetTupleConfigFile();
    /**
     * \brief Evaluates if the ToyTupleConfig file exists.
     */
    TFile * GetNextToyTupleFile();
    /**
     * \brief Closes the ToyTupleConfig file.
     */
    bool ConfigFileExists() const;
    /**
     * \brief Opens the next toy nTuple file.
     * \brief Also increments the toy index so the next call to this function reads the next toy nTuple.
     */
    void CloseConfigFile();
    /**
     * \brief Closes the toy nTuple file.
     */
    void CloseTupleFile();

    /**
     * \brief Set the toy index.
     */
    void SetIndex(int toyIndex);

    /**
     * \brief Equal operator.
     */
    ToyFileHandler & operator=(const ToyFileHandler & _other);
    /**
     * \brief Move equal operator.
     */
    ToyFileHandler & operator=(ToyFileHandler && _other);

    TString GetToyPath() { return m_toyPath; };
    int     GetToyIndex() { return m_toyIndex; };

  private:
    void    SetLocalPath();
    void    ThrowIfConfigHolderNotSet() const;
    void    CreateConfigDirectoryIfWritable() const;
    void    OpenConfigFile();
    bool    ShouldMakeDirectory(OpenMode openMode) const;
    bool    ShouldWriteLocally(OpenMode openMode) const;
    void    CreateLocalCopy(TString pathTail) const;
    bool    LocalCopyExists(TString pathTail) const;
    void    CreateNextTupleDirectoryIfWritable() const;
    void    OpenTupleFile();
    int     GetNextTupleIndex();

  private:
    ConfigHolder m_configHolder;

    OpenMode m_dirOpenMode    = OpenMode::WARNING;
    OpenMode m_configOpenMode = OpenMode::UPDATE;
    OpenMode m_tupleOpenMode  = OpenMode::NONE;

    TString m_toyPath;
    TString m_localPath;
    TString m_configPath;

    const TString m_configPathTail = "/ToyConfiguration.root";
    const TString m_tupleFileName  = "/TupleToy{0}.root";

    TFile * m_configFile = nullptr;
    TFile * m_tupleFile  = nullptr;

    int m_toyIndex;
};

#endif