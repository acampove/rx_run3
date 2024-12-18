#ifndef TOYTUPLECONFIGLOADER_CPP
#define TOYTUPLECONFIGLOADER_CPP

#include "ToyTupleConfigLoader.hpp"
#include "MessageSvc.hpp"

ToyTupleConfigLoader::ToyTupleConfigLoader()
    : m_configHolder() {}

ToyTupleConfigLoader::ToyTupleConfigLoader(const ConfigHolder & _configHolder)
    : m_configHolder(_configHolder) {
    Init();
}

ToyTupleConfigLoader::ToyTupleConfigLoader(const ToyTupleConfigLoader & _other)
    : m_configHolder(_other.m_configHolder) {
    m_configurationsLoaded = _other.m_configurationsLoaded;
}

ToyTupleConfigLoader::ToyTupleConfigLoader(ToyTupleConfigLoader && _other)
    : m_configHolder(_other.m_configHolder) {
    m_configurationsLoaded = _other.m_configurationsLoaded;
}

void ToyTupleConfigLoader::Init() {
    MessageSvc::Info("ToyTupleConfigLoader", (TString) "Initialize ...");

    m_configurationsLoaded.clear();
    return;
}

vector < ToyTupleConfig > ToyTupleConfigLoader::LoadConfigs() {
    MessageSvc::Info("ToyTupleConfigLoader", (TString) "LoadConfigs");
    ClearConfigurations();
    bool _loadPdf = true;
    auto _toyFileHandler = ToyFileHandler(m_configHolder);
    if (_toyFileHandler.ConfigFileExists()) {
        GetConfigTree(_toyFileHandler);
        GetConfigInTree(_loadPdf);
        CloseFile(_toyFileHandler);
    }
    PrintLoaded();
    return m_configurationsLoaded;
}

RooWorkspace * ToyTupleConfigLoader::LoadWorkspace() {
    MessageSvc::Info("ToyTupleConfigLoader", (TString) "LoadWorkspace");
    auto _toyFileHandler = ToyFileHandler(m_configHolder);
    RooWorkspace * _workspace = nullptr;
    if (_toyFileHandler.ConfigFileExists()) {
        _workspace = GetWorkspace(_toyFileHandler);
        CloseFile(_toyFileHandler);
    }
    return _workspace;
}

vector < ToyTupleConfig > ToyTupleConfigLoader::LoadConfigsWithoutPdfs() {
    MessageSvc::Info("ToyTupleConfigLoader", (TString) "LoadConfigsWithoutPdfs");
    ClearConfigurations();
    bool _loadPdf = false;
    auto _toyFileHandler = ToyFileHandler(m_configHolder);
    if (_toyFileHandler.ConfigFileExists()) {
        GetConfigTree(_toyFileHandler);
        GetConfigInTree(_loadPdf);
        CloseFile(_toyFileHandler);
    }
    PrintLoaded();
    return m_configurationsLoaded;
}

void ToyTupleConfigLoader::ClearConfigurations() {
    m_configurationsLoaded.clear();
}

RooWorkspace * ToyTupleConfigLoader::GetWorkspace(ToyFileHandler& _toyFileHandler) {
    _toyFileHandler.SetConfigOpenMode(OpenMode::READ);
    m_configFile = _toyFileHandler.GetTupleConfigFile();
    RooWorkspace * _workspace;
    if (SettingDef::Toy::mergeConfig){
        MessageSvc::Info(TString::Format("Get(%s/%s)", m_configHolder.GetKey().Data(), "workspace"));
        _workspace = m_configFile->Get<RooWorkspace>(m_configHolder.GetKey() + "/" + "workspace");
    }else{
        MessageSvc::Info(TString::Format("Get(%s)", "workspace"));
        _workspace = m_configFile->Get<RooWorkspace>("workspace");
    }
    MessageSvc::Info("ToyTupleConfigLoader", (TString)"Loaded workspace");
    return _workspace;
}

void ToyTupleConfigLoader::GetConfigTree(ToyFileHandler& _toyFileHandler) {
    _toyFileHandler.SetConfigOpenMode(OpenMode::READ);
    m_configFile = _toyFileHandler.GetTupleConfigFile();
    if (SettingDef::Toy::mergeConfig){
        MessageSvc::Info(TString::Format("Get(%s/%s)", m_configHolder.GetKey().Data(), m_configTreeName));
        m_configTree = m_configFile->Get<TTree>(m_configHolder.GetKey() + "/" + m_configTreeName);
    }else{
        MessageSvc::Info(TString::Format("Get(%s)", m_configTreeName));
        m_configTree = m_configFile->Get<TTree>(m_configTreeName);
    }
    MessageSvc::Info("ToyTupleConfigLoader", m_configFile);
    MessageSvc::Info("ToyTupleConfigLoader", (TString)"Loaded config tree", m_configTreeName);
}

void ToyTupleConfigLoader::GetConfigInTree(bool _loadPdf) {
    ToyTupleConfig _configuration = ToyTupleConfig();
    if (m_configTree != nullptr) {
        MessageSvc::Info("ToyTupleConfigLoader", (TString) "Existing Config with", to_string(m_configTree->GetEntries()), "entries");
        _configuration.AttachToTree(m_configTree);
        for (int i = 0; i < m_configTree->GetEntries(); i++) {
            m_configTree->GetEntry(i);
            m_configurationsLoaded.push_back(_configuration);
        }
    }
}

void ToyTupleConfigLoader::CloseFile(ToyFileHandler& _toyFileHandler) {
    m_configFile = nullptr;
    m_configTree = nullptr;
    _toyFileHandler.CloseConfigFile();
}

void ToyTupleConfigLoader::PrintLoaded() const {
    if (m_configurationsLoaded.size() != 0) {
        MessageSvc::Line();
        MessageSvc::Info("ToyTupleConfigLoader", (TString) "Print Loaded", to_string(m_configurationsLoaded.size()));
        for (auto & tupleConfig : m_configurationsLoaded) { tupleConfig.Print(); }
        MessageSvc::Line();
    }
    return;
}

ToyTupleConfigLoader & ToyTupleConfigLoader::operator=(const ToyTupleConfigLoader & _other) { return *this; }

ToyTupleConfigLoader & ToyTupleConfigLoader::operator=(ToyTupleConfigLoader && _other) { return *this; }

#endif