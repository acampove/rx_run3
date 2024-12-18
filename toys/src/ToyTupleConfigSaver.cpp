#ifndef TOYTUPLECONFIGSAVER_CPP
#define TOYTUPLECONFIGSAVER_CPP

#include "ToyTupleConfigSaver.hpp"

#include "MessageSvc.hpp"
#include "ToyTupleConfigLoader.hpp"
#include "RooWorkspace.h"

ToyTupleConfigSaver::ToyTupleConfigSaver()
    : m_configHolder()
    , m_fileHandler() {}

ToyTupleConfigSaver::ToyTupleConfigSaver(const ConfigHolder & _configHolder)
    : m_configHolder(_configHolder)
    , m_fileHandler(_configHolder) {
    Init();
}

ToyTupleConfigSaver::ToyTupleConfigSaver(const ToyTupleConfigSaver & _other)
    : m_configHolder(_other.m_configHolder)
    , m_fileHandler(_other.m_fileHandler) {
    m_observableKey        = _other.m_observableKey;
    m_configurationsSaved  = _other.m_configurationsSaved;
    m_configurationsToSave = _other.m_configurationsToSave;
}

ToyTupleConfigSaver::ToyTupleConfigSaver(ToyTupleConfigSaver && _other)
    : m_configHolder(_other.m_configHolder) {
    m_observableKey        = _other.m_observableKey;
    m_fileHandler          = move(_other.m_fileHandler);
    m_configurationsSaved  = move(_other.m_configurationsSaved);
    m_configurationsToSave = move(_other.m_configurationsToSave);
}

void ToyTupleConfigSaver::Init() {
    MessageSvc::Info("ToyTupleConfigSaver", (TString) "Initialize ...");

    m_observableKey = m_observableKeyUnsetFlag;
    m_configurationsToSave.clear();
    m_configurationsSaved.clear();

    ToyTupleConfigLoader _loader = ToyTupleConfigLoader(m_configHolder);
    m_configurationsSaved        = _loader.LoadConfigs();
    return;
}

void ToyTupleConfigSaver::PrintSaved() {
    if (m_configurationsSaved.size() != 0) {
        MessageSvc::Line();
        MessageSvc::Info("ToyTupleConfigSaver", (TString) "Print Saved", to_string(m_configurationsSaved.size()));
        for (auto & tupleConfig : m_configurationsSaved) { tupleConfig.Print(); }
        MessageSvc::Line();
    }
    return;
}

void ToyTupleConfigSaver::PrintToSave() {
    if (m_configurationsToSave.size() != 0) {
        MessageSvc::Line();
        MessageSvc::Info("ToyTupleConfigSaver", (TString) "Print ToSave", to_string(m_configurationsToSave.size()));
        for (auto & tupleConfig : m_configurationsToSave) { tupleConfig.Print(); }
        MessageSvc::Line();
    }
    return;
}

void ToyTupleConfigSaver::AddConfigurations(const vector< ToyTupleConfig > & tupleConfigs) {
    for (auto & tupleConfig : tupleConfigs) { AddConfiguration(tupleConfig); }
    return;
}

void ToyTupleConfigSaver::AddConfiguration(const ToyTupleConfig & tupleConfig) {
    if (NotAdded(tupleConfig) && ObservableKeyMatches(tupleConfig)) { m_configurationsToSave.push_back(tupleConfig); }
    return;
}

bool ToyTupleConfigSaver::NotAdded(const ToyTupleConfig & tupleConfig) const {
    bool notAdded = TupleConfigNotFoundIn(tupleConfig, m_configurationsToSave);
    return notAdded;
}

bool ToyTupleConfigSaver::TupleConfigNotFoundIn(const ToyTupleConfig & tupleConfig, const vector< ToyTupleConfig > & listOfTupleConfigs) const {
    for (auto & comparedTupleConfig : listOfTupleConfigs) {
        bool sampleMatch = SampleMatches(comparedTupleConfig, tupleConfig);
        if (sampleMatch) { return false; }
        bool pdfKeyMatch = PdfKeyMatches(comparedTupleConfig, tupleConfig);
        if (pdfKeyMatch) { return false; }
    }
    return true;
}

bool ToyTupleConfigSaver::SampleMatches(const ToyTupleConfig & allocatedConfig, const ToyTupleConfig & newConfig) const {
    TString allocatedSample = allocatedConfig.GetSample();
    TString newSample       = newConfig.GetSample();
    bool    sampleMatch     = (allocatedSample == newSample);
    if (sampleMatch) { MatchWarning("Sample", newSample); }
    return sampleMatch;
}

void ToyTupleConfigSaver::MatchWarning(TString keyType, TString key) const {
    MessageSvc::Line();
    MessageSvc::Warning("ToyTupleConfigSaver", "The " + keyType + " " + key + " has been used.");
    MessageSvc::Warning("ToyTupleConfigSaver", (TString) "This ToyTupleConfig will not be saved.");
    MessageSvc::Line();
    return;
}

bool ToyTupleConfigSaver::PdfKeyMatches(const ToyTupleConfig & allocatedConfig, const ToyTupleConfig & newConfig) const {
    TString allocatedPdfKey = allocatedConfig.GetPdfKey();
    TString newPdfKey       = newConfig.GetPdfKey();
    bool    pdfKeyMatch     = (allocatedPdfKey == newPdfKey);
    if (pdfKeyMatch) { MatchWarning("PdfKey", newPdfKey); }
    return pdfKeyMatch;
}

bool ToyTupleConfigSaver::ObservableKeyMatches(const ToyTupleConfig & tupleConfig) {
    if (ObservableKeySet()) {
        bool keyMatch = CheckObservableKey(tupleConfig);
        return keyMatch;
    }
    SetObservableKey(tupleConfig);
    return true;
}

bool ToyTupleConfigSaver::ObservableKeySet() const { return (m_observableKey != m_observableKeyUnsetFlag); }

bool ToyTupleConfigSaver::CheckObservableKey(const ToyTupleConfig & tupleConfig) const {
    TString configObservableKey = tupleConfig.GetObservableKey();
    bool    keyMatch            = (configObservableKey == m_observableKey);
    if (!keyMatch) { WarnKeyNotSame(configObservableKey); }
    return keyMatch;
}

void ToyTupleConfigSaver::SetObservableKey(const ToyTupleConfig & tupleConfig) {
    m_observableKey = tupleConfig.GetObservableKey();
    return;
}

void ToyTupleConfigSaver::WarnKeyNotSame(TString configObservableKey) const {
    MessageSvc::Line();
    MessageSvc::Warning("ToyTupleConfigSaver", "The ObservableKey: " + configObservableKey + " does not match the one used by _other ToyTupleConfig");
    MessageSvc::Warning("ToyTupleConfigSaver", (TString) "This ToyTupleConfig will not be saved.");
    MessageSvc::Line();
    return;
}

void ToyTupleConfigSaver::Update(TString _key) {
    MessageSvc::Info("ToyTupleConfigSaver", (TString) "Update");
    PrintToSave();
    auto _configurations = GetConfigurationsToUpdate();
    if (_configurations.size() != 0) {
        UpdateOpenConfigTree(_key);
        AttachConfigurationToTree();
        WriteConfigurations(_configurations);
        CloseFile();
        UpdateSavedConfigurations();
        PrintSaved();
    }
}

void ToyTupleConfigSaver::Append(TString _key) {
    MessageSvc::Info("ToyTupleConfigSaver", (TString) "Append");
    PrintToSave();
    if (m_configurationsToSave.size() != 0) {
        MainConfigTree(_key);
        AttachConfigurationToTree();
        WriteConfigurations(m_configurationsToSave);
        CloseFile();
        UpdateSavedConfigurations();
        PrintSaved();
    }
}

vector< ToyTupleConfig > ToyTupleConfigSaver::GetConfigurationsToUpdate() const {
    vector< ToyTupleConfig > _configurationsToUpdate;
    for (auto & _tupleConfig : m_configurationsToSave) {
        if (ConfigNotSaved(_tupleConfig)) { _configurationsToUpdate.push_back(_tupleConfig); }
    }
    return move(_configurationsToUpdate);
}

bool ToyTupleConfigSaver::ConfigNotSaved(const ToyTupleConfig & _tupleConfig) const {
    bool _notSaved = TupleConfigNotFoundIn(_tupleConfig, m_configurationsSaved);
    return _notSaved;
}

void ToyTupleConfigSaver::MainConfigTree(TString _key) {
    m_fileHandler.SetConfigOpenMode(OpenMode::UPDATE);
    TFile * _file = m_fileHandler.GetTupleConfigFile();
    MessageSvc::Info("ToyTupleConfigSaver", _file);
    _file->cd();
    if (_key != ""){
        _file->mkdir(_key);
        m_dir = _file->Get<TDirectoryFile>(_key);
    }
    else{
        m_dir = dynamic_cast<TDirectoryFile*>(_file);
    }
    m_dir->cd();
    m_configTree = (TTree *) _file->Get(m_configTreeName);
}

void ToyTupleConfigSaver::UpdateOpenConfigTree(TString _key) {
    m_fileHandler.SetConfigOpenMode(OpenMode::UPDATE);
    TFile * _file = m_fileHandler.GetTupleConfigFile();
    MessageSvc::Info("ToyTupleConfigSaver", _file);
    _file->cd();
    if (_key != ""){
        m_dir = _file->Get<TDirectoryFile>(_key);
    }
    else{
        m_dir = dynamic_cast<TDirectoryFile*>(_file);
    }
    m_dir->cd();
    m_configTree = (TTree *) m_dir->Get(m_configTreeName);
}

void ToyTupleConfigSaver::AttachConfigurationToTree() {
    if (m_configTree == nullptr) {
        MessageSvc::Info("ToyTupleConfigSaver", (TString) "Create new Config");
        m_configTree = new TTree(m_configTreeName, "");
        m_configurationInTree.CreateBranches(m_configTree);
    } else {
        MessageSvc::Info("ToyTupleConfigSaver", (TString) "Existing Config with", to_string(m_configTree->GetEntries()), "entries");
        // m_configTree = m_configTree.CloneTree();
        m_configurationInTree.AttachToTree(m_configTree);
    }
}

void ToyTupleConfigSaver::WriteConfigurations(const vector< ToyTupleConfig > & _configurations) {
    RooWorkspace * workspace = m_dir->Get<RooWorkspace>("workspace");
    if (workspace == nullptr){
        workspace = new RooWorkspace("workspace", "workspace");
        std::cout << "Recreate workspace" << std::endl;
    }
    else{
        workspace->Print();
        std::cout << "Found workspace "<< std::endl;
    }
    for (const auto & _tupleConfig : _configurations) {
        m_configurationInTree = _tupleConfig;
        RooAbsPdf * pdf       = _tupleConfig.GetPdf();
        workspace->import(*pdf, RooFit::RenameConflictNodes(_tupleConfig.GetSample()));
        m_configTree->Fill();
        std::cout << "ConfigTree has entries " << m_configTree->GetEntries() << std::endl;
    }
    workspace->Write(0, TObject::kOverwrite);
    m_configTree->Write(0, TObject::kOverwrite);
    delete workspace;
}

void ToyTupleConfigSaver::CloseFile() { m_fileHandler.CloseConfigFile(); }

void ToyTupleConfigSaver::UpdateSavedConfigurations() {
    m_configurationsSaved.insert(m_configurationsSaved.end(), m_configurationsToSave.begin(), m_configurationsToSave.end());
    m_configurationsToSave.clear();
}

void ToyTupleConfigSaver::Overwrite() {
    MessageSvc::Info("ToyTupleConfigSaver", (TString) "Update");
    PrintToSave();
    if (m_configurationsToSave.size() != 0) {
        RecreateOpenConfigTree();
        AttachConfigurationToTree();
        WriteConfigurations(m_configurationsToSave);
        CloseFile();
        OverwriteSavedConfigurations();
        PrintSaved();
    }
}

void ToyTupleConfigSaver::RecreateOpenConfigTree() {
    m_fileHandler.SetConfigOpenMode(OpenMode::RECREATE);
    TFile * _file = m_fileHandler.GetTupleConfigFile();
    MessageSvc::Info("ToyTupleConfigSaver", _file);
    _file->cd();
    _file->Delete("*;*");
    m_dir = dynamic_cast<TDirectoryFile*>(_file);
    m_configTree = nullptr;
}

void ToyTupleConfigSaver::OverwriteSavedConfigurations() {
    m_configurationsSaved.clear();
    m_configurationsSaved = m_configurationsToSave;
    m_configurationsToSave.clear();
}

ToyTupleConfigSaver & ToyTupleConfigSaver::operator=(const ToyTupleConfigSaver & _other) {
    m_observableKey        = _other.m_observableKey;
    m_configurationsSaved  = _other.m_configurationsSaved;
    m_configurationsToSave = _other.m_configurationsToSave;
    m_fileHandler          = _other.m_fileHandler;
    return *this;
}

ToyTupleConfigSaver & ToyTupleConfigSaver::operator=(ToyTupleConfigSaver && _other) {
    m_observableKey        = _other.m_observableKey;
    m_configurationsSaved  = move(_other.m_configurationsSaved);
    m_configurationsToSave = move(_other.m_configurationsToSave);
    m_fileHandler          = move(_other.m_fileHandler);
    return *this;
}

#endif