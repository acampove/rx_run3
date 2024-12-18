#ifndef TOYFILEHANDLER_CPP
#define TOYFILEHANDLER_CPP

#include "ToyFileHandler.hpp"
#include "IOSvc.hpp"
#include "MessageSvc.hpp"
#include "SettingDef.hpp"
#include "HelperSvc.cpp"
#include "core.h"
#include <fmt_ostream.h>

ToyFileHandler::ToyFileHandler()
    : m_configHolder() {}

ToyFileHandler::ToyFileHandler(const ConfigHolder & _configHolder)
    : m_configHolder(_configHolder) {
    Init();
}

ToyFileHandler::ToyFileHandler(const ToyFileHandler & _other)
    : m_configHolder(_other.m_configHolder) {
    m_dirOpenMode    = _other.m_dirOpenMode;
    m_configOpenMode = _other.m_configOpenMode;
    m_tupleOpenMode  = _other.m_tupleOpenMode;
    m_toyPath        = _other.m_toyPath;
    m_configPath     = _other.m_configPath;
    m_localPath      = _other.m_localPath;
    m_toyIndex       = _other.m_toyIndex;
}

ToyFileHandler::ToyFileHandler(ToyFileHandler && _other)
    : m_configHolder(_other.m_configHolder) {
    m_dirOpenMode    = move(_other.m_dirOpenMode);
    m_configOpenMode = move(_other.m_configOpenMode);
    m_tupleOpenMode  = move(_other.m_tupleOpenMode);
    m_toyPath        = _other.m_toyPath;
    m_configPath     = _other.m_configPath;
    m_localPath      = _other.m_localPath;
    m_toyIndex       = move(_other.m_toyIndex);
}

void ToyFileHandler::Init() {
    MessageSvc::Info("ToyFileHandler", (TString) "Initialize ...");

    // m_configHolder.Print();
    m_toyPath = IOSvc::GetTupleDir("toy", m_configHolder);
    SetLocalPath();

    m_configPath = m_toyPath + "/Configuration";

    m_toyIndex = SettingDef::Toy::jobIndex * SettingDef::Toy::nToysPerJob;

    if (SettingDef::Toy::mergeConfig)
        m_toyIndex = SettingDef::Toy::jobIndex;

    Print();
    return;
}

void ToyFileHandler::SetLocalPath() {
    TString _pathTail = m_toyPath;
    TString _pathHead = IOSvc::GetTupleDirHead("toy");
    _pathTail.ReplaceAll(_pathHead, "");
    m_localPath = "./toyTuples" + _pathTail;
    if (SettingDef::Toy::mergeConfig) {
        MessageSvc::Info("ToyFileHandler", (TString) "Use same file for all configs");
        m_localPath = "./TupleToy_" + SettingDef::Toy::tupleVer;
    }
    return;
}

void ToyFileHandler::Print() {
    MessageSvc::Line();
    MessageSvc::Info("ToyFileHandler");
    MessageSvc::Info("toys", m_toyPath);
    MessageSvc::Info("configuration", m_configPath);
    MessageSvc::Info("local", m_localPath);
    MessageSvc::Info("index", to_string(m_toyIndex));
    MessageSvc::Info("configFileName", m_configPathTail);
    MessageSvc::Info("tupleFileName", m_tupleFileName);

    MessageSvc::Line();
    return;
}

void ToyFileHandler::SetConfigOpenMode(OpenMode openMode) {
    m_configOpenMode = openMode;
    return;
}

void ToyFileHandler::SetTupleOpenMode(OpenMode openMode) {
    m_tupleOpenMode = openMode;
    return;
}

TFile * ToyFileHandler::GetTupleConfigFile() {
    ThrowIfConfigHolderNotSet();
    IOSvc::CloseFile(m_configFile);
    CreateConfigDirectoryIfWritable();
    OpenConfigFile();
    return m_configFile;
}

bool ToyFileHandler::ConfigFileExists() const {
    bool configExists = LocalCopyExists(m_configPathTail) || IOSvc::ExistFile(m_configPath + m_configPathTail);
    if (!configExists) {
        MessageSvc::Warning("ToyFileHandler", (TString) "ConfigFile", m_configPath + "/ToyConfiguration.root", "does not exist");
        return false;
    }
    return true;
}

void ToyFileHandler::ThrowIfConfigHolderNotSet() const {
    if (m_toyPath == "") MessageSvc::Error("ToyFileHandler", "The ConfigHolder is not set but a toy file is requested!", "EXIT_FAILURE");
    return;
}

void ToyFileHandler::CreateConfigDirectoryIfWritable() const {
    if (ShouldMakeDirectory(m_configOpenMode)) {
        IOSvc::MakeDir(m_toyPath, m_dirOpenMode);
        IOSvc::MakeDir(m_configPath, m_dirOpenMode);
    }
}

bool ToyFileHandler::ShouldMakeDirectory(OpenMode openMode) const {
    bool notCernBatch = not(HasEOS());
    bool writeMode    = ((openMode == OpenMode::UPDATE) || (openMode == OpenMode::RECREATE));
    return (notCernBatch && writeMode);
}

bool ToyFileHandler::ShouldWriteLocally(OpenMode openMode) const {
    bool cernBatch = HasEOS();
    bool writeMode = ((openMode == OpenMode::UPDATE) || (openMode == OpenMode::RECREATE));
    return (cernBatch && writeMode);
}

void ToyFileHandler::OpenConfigFile() {
    TString _configPath;
    TString _localConfigDir = m_localPath + "/Configuration";
    if (LocalCopyExists("/Configuration" + m_configPathTail)) {
        _configPath = _localConfigDir + m_configPathTail;
    } else if (ShouldWriteLocally(m_configOpenMode)) {
        _configPath = _localConfigDir + m_configPathTail;
        IOSvc::MakeDir(_localConfigDir, m_dirOpenMode);
        CreateLocalCopy("/Configuration" + m_configPathTail);
    } else {
        _configPath = m_configPath + m_configPathTail;
    }
    if(SettingDef::Toy::CopyLocally){
        if( SettingDef::Toy::mergeConfig && !IOSvc::ExistFile("CopyLocally_ConfigFile.root")){
            IOSvc::CopyFile( _configPath, "CopyLocally_ConfigFile.root");
        }else{
            MessageSvc::Warning("Expected copy local to be already done");
        }
        _configPath = "CopyLocally_ConfigFile.root";
        if( m_configFile != nullptr){
            IOSvc::CloseFile(m_configFile);
        }
    }
    if (SettingDef::Toy::mergeConfig) {
        if (m_configOpenMode == OpenMode::READ)  m_configFile = IOSvc::OpenFile(_configPath, OpenMode::READ);
        else if (IOSvc::ExistFile(_configPath))  m_configFile = IOSvc::OpenFile(_configPath, OpenMode::UPDATE);
        else                                     m_configFile = IOSvc::OpenFile(_configPath, OpenMode::RECREATE);
    } else {
        m_configFile = IOSvc::OpenFile(_configPath, m_configOpenMode);
    }
    return;
}

bool ToyFileHandler::LocalCopyExists(TString pathTail) const {
    TString _localPath = m_localPath + pathTail;
    if (IOSvc::ExistFile(_localPath)) {
        return true;
    } else {
        return false;
    }
}

void ToyFileHandler::CreateLocalCopy(TString pathTail) const {
    TString _fileSystemPath = m_toyPath + pathTail;   // either EOS or afs
    TString _localPath      = m_localPath + pathTail;
    if (IOSvc::ExistFile(_fileSystemPath)) { IOSvc::CopyFile(_fileSystemPath ,_localPath); }
    return;
}

TFile * ToyFileHandler::GetNextToyTupleFile() {
    ThrowIfConfigHolderNotSet();
    IOSvc::CloseFile(m_tupleFile);
    CreateNextTupleDirectoryIfWritable();
    OpenTupleFile();
    return m_tupleFile;
}

void ToyFileHandler::CreateNextTupleDirectoryIfWritable() const {
    if (ShouldMakeDirectory(m_tupleOpenMode)) { IOSvc::MakeDir(m_toyPath, m_dirOpenMode); }
}

void ToyFileHandler::OpenTupleFile() {
    TString _pathTail = TString(fmt::format(m_tupleFileName.Data(), GetNextTupleIndex()));
    TString _currentToyPath;
    if (LocalCopyExists(_pathTail)) {
        _currentToyPath = m_localPath + _pathTail;      // Reads the local copy in the cwd if it exists
    } else if (ShouldWriteLocally(m_tupleOpenMode)) {   // Sometimes we cannot write to eos
        IOSvc::MakeDir(m_localPath);
        _currentToyPath = m_localPath + _pathTail;
        CreateLocalCopy(_pathTail);
    } else {
        _currentToyPath = m_toyPath + _pathTail;
    }
    if( SettingDef::Toy::CopyLocally){
        auto _tokens_file_ = TokenizeString(_currentToyPath, "/");
        TString _toyFileName  = "CopyLocally";
        _toyFileName+= "_"+_tokens_file_.at(_tokens_file_.size()-1);
        _toyFileName = _toyFileName.ReplaceAll("/","");
        if(IOSvc::ExistFile( _toyFileName)){
            _currentToyPath = _toyFileName;
        }else{
            IOSvc::CopyFile(_currentToyPath, _toyFileName);
            _currentToyPath = _toyFileName;
        }
    }
    if (SettingDef::Toy::mergeConfig) {
        if (m_tupleOpenMode == OpenMode::READ)      m_tupleFile = IOSvc::OpenFile(_currentToyPath, OpenMode::READ);
        else if (IOSvc::ExistFile(_currentToyPath)) m_tupleFile = IOSvc::OpenFile(_currentToyPath, OpenMode::UPDATE);
        else                                        m_tupleFile = IOSvc::OpenFile(_currentToyPath, OpenMode::RECREATE);
    } else {
        m_tupleFile = IOSvc::OpenFile(_currentToyPath, m_tupleOpenMode);
    }
    return;
}

int ToyFileHandler::GetNextTupleIndex() {
    //TString _currentToyIndex = to_string(m_toyIndex);
    int _currentToyIndex = m_toyIndex;
    if (!SettingDef::Toy::mergeConfig) m_toyIndex++;
    return _currentToyIndex;
}

void ToyFileHandler::CloseConfigFile() {
    IOSvc::CloseFile(m_configFile);
    return;
}

void ToyFileHandler::CloseTupleFile() {
    IOSvc::CloseFile(m_tupleFile);
    return;
}

void ToyFileHandler::SetIndex(int toyIndex) {
    m_toyIndex = toyIndex;
    return;
}

ToyFileHandler & ToyFileHandler::operator=(const ToyFileHandler & _other) {
    m_configHolder   = _other.m_configHolder;
    m_dirOpenMode    = _other.m_dirOpenMode;
    m_configOpenMode = _other.m_configOpenMode;
    m_tupleOpenMode  = _other.m_tupleOpenMode;
    m_toyPath        = _other.m_toyPath;
    m_configPath     = _other.m_configPath;
    m_localPath      = _other.m_localPath;
    m_toyIndex       = _other.m_toyIndex;
    return *this;
}

ToyFileHandler & ToyFileHandler::operator=(ToyFileHandler && _other) {
    m_configHolder   = _other.m_configHolder;
    m_dirOpenMode    = move(_other.m_dirOpenMode);
    m_configOpenMode = move(_other.m_configOpenMode);
    m_tupleOpenMode  = move(_other.m_tupleOpenMode);
    m_toyPath        = _other.m_toyPath;
    m_configPath     = _other.m_configPath;
    m_localPath      = _other.m_localPath;
    m_toyIndex       = move(_other.m_toyIndex);
    return *this;
}

ToyFileHandler::~ToyFileHandler() {
    IOSvc::CloseFile(m_configFile);
    IOSvc::CloseFile(m_tupleFile);
}

#endif