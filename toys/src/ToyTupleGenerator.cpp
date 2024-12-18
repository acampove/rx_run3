#ifndef TOYTUPLEGENERATOR_CPP
#define TOYTUPLEGENERATOR_CPP

#include "ToyTupleGenerator.hpp"
#include "MessageSvc.hpp"
#include "ToyTupleConfigLoader.hpp"

ToyTupleGenerator::ToyTupleGenerator()
    : m_configHolder()
    , m_toyFileHandler()
    , m_tupleHeaderHandler() {}

ToyTupleGenerator::ToyTupleGenerator(const ConfigHolder & _configHolder, const vector< ToyYieldConfig > & _yieldConfigs)
    : m_configHolder(_configHolder)
    , m_toyFileHandler(_configHolder)
    , m_tupleHeaderHandler() {
    m_yieldConfigs = _yieldConfigs;
    Init();
}

ToyTupleGenerator::ToyTupleGenerator(const ToyTupleGenerator & _other)
    : m_configHolder(_other.m_configHolder)
    , m_toyFileHandler(_other.m_toyFileHandler)
    , m_sampleToTupleConfigMap(_other.m_sampleToTupleConfigMap)
    , m_yieldConfigs(_other.m_yieldConfigs)
    , m_componentGenerators(_other.m_componentGenerators) {}

ToyTupleGenerator::ToyTupleGenerator(const ToyTupleGenerator && _other)
    : m_configHolder(_other.m_configHolder)
    , m_toyFileHandler(move(_other.m_toyFileHandler))
    , m_sampleToTupleConfigMap(move(_other.m_sampleToTupleConfigMap))
    , m_yieldConfigs(move(_other.m_yieldConfigs))
    , m_componentGenerators(move(_other.m_componentGenerators)) {}

void ToyTupleGenerator::Init() {
    MessageSvc::Info("ToyTupleGenerator", (TString) "Initialize ...");
    m_configHolder.Print();
    if (m_yieldConfigs.size() == 0) MessageSvc::Error("ToyTupleGenerator", (TString) "No ToyYieldConfig passed");
    Print();

    ReadToyTupleConfigs();
    EvaluateGeneratorMean();
    InitialiseToyComponentGenerators();
    return;
}

void ToyTupleGenerator::Print() const {
    MessageSvc::Info("ToyTupleGenerator", (TString) "YieldConfigurations", to_string(m_yieldConfigs.size()));
    MessageSvc::Line();
    for (auto & yieldConfig : m_yieldConfigs) {
        yieldConfig.Print();
        MessageSvc::Line();
    }
    return;
}

void ToyTupleGenerator::ReadToyTupleConfigs() {
    ToyTupleConfigLoader _tupleConfigLoader(m_configHolder);
    m_workspace = _tupleConfigLoader.LoadWorkspace();
    if (m_workspace == nullptr) MessageSvc::Error("ToyTupleGenerator", (TString) "RooWorkspace loaded is a null pointer.");
    for (auto & tupleConfig : _tupleConfigLoader.LoadConfigs()) {
        TString sampleKey                   = tupleConfig.GetSample();
        m_sampleToTupleConfigMap[sampleKey] = tupleConfig;
        m_sampleToTupleConfigMap[sampleKey].LoadPdfFromWorkspace(m_workspace, tupleConfig.GetPdfKey());
    }
    if (m_sampleToTupleConfigMap.size() == 0) MessageSvc::Error("ToyTupleGenerator", (TString) "No ToyTupleConfig exists for the configured configHolder.");
    return;
}

void ToyTupleGenerator::EvaluateGeneratorMean() {
    MessageSvc::Line();
    for (auto & yieldConfig : m_yieldConfigs) {
        const ToyTupleConfig & tupleConfig  = FindMatchingTupleConfig(yieldConfig);
        double                 nominalYield = tupleConfig.GetNominalYield();
        yieldConfig.EvaluateGeneratorMean(nominalYield);
        MessageSvc::Line();
    }
    return;
}

void ToyTupleGenerator::InitialiseToyComponentGenerators() {
    for (auto & yieldConfig : m_yieldConfigs) {
        const ToyTupleConfig & tupleConfig = FindMatchingTupleConfig(yieldConfig);
        m_componentGenerators.emplace_back(tupleConfig, yieldConfig);
    }
    return;
}

const ToyTupleConfig & ToyTupleGenerator::FindMatchingTupleConfig(const ToyYieldConfig & yieldConfig) const {
    TString sampleKey = yieldConfig.GetSample();
    if (!TupleConfigInMap(sampleKey)) { MessageSvc::Error("ToyTupleGenerator", (TString) "Cannot find sample: " + sampleKey + " among the saved ToyTupleConfigs", "logic_error"); }
    return m_sampleToTupleConfigMap.at(sampleKey);
}

bool ToyTupleGenerator::TupleConfigInMap(TString sampleKey) const {
    bool tupleConfigFound = (m_sampleToTupleConfigMap.find(sampleKey) != m_sampleToTupleConfigMap.end());
    return tupleConfigFound;
}

void ToyTupleGenerator::Generate() {
    MessageSvc::Info("ToyTupleGenerator", (TString) "Generating ...");
    DeleteOldHeaders();
    OverwriteOpen();   // We delete the old generated stuff because we cannot ensure no two events in the toy are unique when we append generate
    WriteComponentTreesToCurrentTuple();
    UpdateToyTupleHeader();
    WriteToyTupleHeader();
    CloseFile();
    return;
}

void ToyTupleGenerator::DeleteOldHeaders() {
    m_tupleComponentHeaders.clear();
    return;
}

void ToyTupleGenerator::OverwriteOpen() {
    m_toyFileHandler.SetTupleOpenMode(OpenMode::RECREATE);
    m_file = m_toyFileHandler.GetNextToyTupleFile();
    return;
}

void ToyTupleGenerator::WriteComponentTreesToCurrentTuple() {
    if (SettingDef::Toy::mergeConfig)
        for (auto & componentGenerator : m_componentGenerators) { componentGenerator.WriteComponentTreeToFile(m_file, m_index, m_configHolder.GetKey()); }
    else
        for (auto & componentGenerator : m_componentGenerators) { componentGenerator.WriteComponentTreeToFile(m_file); }
    return;
}

void ToyTupleGenerator::UpdateToyTupleHeader() {
    for (auto & componentGenerator : m_componentGenerators) {
        ToyTupleComponentHeader tupleComponentHeader = componentGenerator.GetLastGeneratedHeader();
        m_tupleComponentHeaders.push_back(tupleComponentHeader);
    }
    return;
}

void ToyTupleGenerator::WriteToyTupleHeader() {
    m_tupleHeaderHandler.SetToyTupleFile(m_file);
    if (SettingDef::Toy::mergeConfig)
        m_tupleHeaderHandler.WriteHeaders(m_tupleComponentHeaders, m_index, m_configHolder.GetKey());
    else
        m_tupleHeaderHandler.WriteHeaders(m_tupleComponentHeaders);
    return;
}

void ToyTupleGenerator::CloseFile() {
    m_toyFileHandler.CloseTupleFile();
    return;
}

ToyTupleGenerator & ToyTupleGenerator::operator=(const ToyTupleGenerator & _other) {
    m_configHolder           = _other.m_configHolder;
    m_toyFileHandler         = _other.m_toyFileHandler;
    m_sampleToTupleConfigMap = _other.m_sampleToTupleConfigMap;
    m_yieldConfigs           = _other.m_yieldConfigs;
    m_componentGenerators    = _other.m_componentGenerators;
    return *this;
}

ToyTupleGenerator & ToyTupleGenerator::operator=(const ToyTupleGenerator && _other) {
    m_configHolder           = _other.m_configHolder;
    m_toyFileHandler         = move(_other.m_toyFileHandler);
    m_sampleToTupleConfigMap = move(_other.m_sampleToTupleConfigMap);
    m_yieldConfigs           = move(_other.m_yieldConfigs);
    m_componentGenerators    = move(_other.m_componentGenerators);
    return *this;
}

#endif