#ifndef TOYTUPLEREADER_CPP
#define TOYTUPLEREADER_CPP

#include "ToyTupleReader.hpp"
#include "MessageSvc.hpp"

#include "RooArgSet.h"

ToyTupleReader::ToyTupleReader()
    : m_configHolder()
    , m_toyFileHandler()
    , m_tupleConfigLoader()
    , m_tupleHeaderHandler() {}

ToyTupleReader::ToyTupleReader(const ConfigHolder & _configHolder, const vector< ToyYieldConfig > & _yieldConfigs)
    : m_configHolder(_configHolder)
    , m_toyFileHandler(_configHolder)
    , m_tupleConfigLoader(_configHolder)
    , m_tupleHeaderHandler() {
    m_yieldConfigs = _yieldConfigs;
    Init();
}

ToyTupleReader::ToyTupleReader(const ToyTupleReader & _other)
    : m_configHolder(_other.m_configHolder)
    , m_toyFileHandler(_other.m_toyFileHandler)
    , m_sampleToTupleConfigMap(_other.m_sampleToTupleConfigMap)
    , m_yieldConfigs(_other.m_yieldConfigs)
    , m_componentReaders(_other.m_componentReaders)
    , m_componentHeaders(_other.m_componentHeaders)
    , m_tupleConfigLoader(_other.m_tupleConfigLoader)
    , m_tupleHeaderHandler(_other.m_tupleHeaderHandler) {
    m_datasetName  = _other.m_datasetName;
    m_datahistName = _other.m_datahistName;
    m_key          = _other.m_key;
}

ToyTupleReader::ToyTupleReader(ToyTupleReader && _other)
    : m_configHolder(_other.m_configHolder)
    , m_toyFileHandler(move(_other.m_toyFileHandler))
    , m_sampleToTupleConfigMap(move(_other.m_sampleToTupleConfigMap))
    , m_yieldConfigs(move(_other.m_yieldConfigs))
    , m_componentReaders(move(_other.m_componentReaders))
    , m_componentHeaders(move(_other.m_componentHeaders))
    , m_tupleConfigLoader(move(_other.m_tupleConfigLoader))
    , m_tupleHeaderHandler(move(_other.m_tupleHeaderHandler)) {
    m_datasetName  = _other.m_datasetName;
    m_datahistName = _other.m_datahistName;
    m_key          = _other.m_key;
}

void ToyTupleReader::Init() {
    MessageSvc::Info("ToyTupleReader", (TString) "Initialize ...");

    if (m_yieldConfigs.size() == 0) MessageSvc::Error("ToyTupleGenerator", (TString) "No ToyYieldConfig passed");
    Print();

    m_key = m_configHolder.GetKey();

    SetDataObjectsName();
    ReadToyTupleConfigs();
    EvaluateGeneratorMean();
    InitialiseToyTupleComponentReaders();
    return;
}

void ToyTupleReader::Print() const {
    MessageSvc::Info("ToyTupleReader", (TString) "ConfigHolder", m_key);
    MessageSvc::Info("ToyTupleReader", (TString) "YieldConfigurations", to_string(m_yieldConfigs.size()));
    MessageSvc::Line();
    for (auto & yieldConfig : m_yieldConfigs) {
        yieldConfig.Print();
        MessageSvc::Line();
    }
    return;
}

void ToyTupleReader::SetDataObjectsName() {
    TString nameTail = "_" + m_configHolder.GetSample() + "_" + to_string(m_configHolder.GetTrigger());
    m_datasetName    = "dataset" + nameTail;
    m_datahistName   = "datahist" + nameTail;
}

void ToyTupleReader::ReadToyTupleConfigs() {
    for (auto & tupleConfig : m_tupleConfigLoader.LoadConfigsWithoutPdfs()) {
        TString sampleKey                   = tupleConfig.GetSample();
        m_sampleToTupleConfigMap[sampleKey] = tupleConfig;
    }
    m_observables = (m_sampleToTupleConfigMap.begin()->second).GetObservables();
    if (m_sampleToTupleConfigMap.size() == 0) MessageSvc::Error("ToyTupleReader", "No ToyTupleConfig exists for the configured configHolder.");
    return;
}

void ToyTupleReader::EvaluateGeneratorMean() {
    for (auto & yieldConfig : m_yieldConfigs) {
        const ToyTupleConfig & tupleConfig  = FindMatchingTupleConfig(yieldConfig);
        double                 nominalYield = tupleConfig.GetNominalYield();
        /*
        if( nominalYield <0 ) MessageSvc::Error("CANNOT GENERATE NEGATIVE YIELD, FIX YOUR FITS!", "","EXIT_FAILURE");
        */
        yieldConfig.EvaluateGeneratorMean(nominalYield);
    }
}

const ToyTupleConfig & ToyTupleReader::FindMatchingTupleConfig(const ToyYieldConfig & yieldConfig) const {
    TString sampleKey = yieldConfig.GetSample();
    if (TupleConfigInMap(sampleKey)) {
        return m_sampleToTupleConfigMap.at(sampleKey);
    } else {
        TupleConfigNotFoundError(sampleKey);
    }
}

bool ToyTupleReader::TupleConfigInMap(TString sampleKey) const {
    bool tupleConfigFound = (m_sampleToTupleConfigMap.find(sampleKey) != m_sampleToTupleConfigMap.end());
    return tupleConfigFound;
}

void ToyTupleReader::TupleConfigNotFoundError(TString sampleKey) const {
    MessageSvc::Line();
    MessageSvc::Error("ToyTupleReader", "Cannot find sample: " + sampleKey + " among the saved ToyTupleConfigs", "logic_error");
}

void ToyTupleReader::InitialiseToyTupleComponentReaders() {
    for (auto & yieldConfig : m_yieldConfigs) { m_componentReaders.emplace_back(yieldConfig); }
    for (auto & _keyTupleConfigPair : m_sampleToTupleConfigMap) { _keyTupleConfigPair.second.DeletePdf(); }
}

void ToyTupleReader::SetIndex(int index) { m_toyFileHandler.SetIndex(index); }

void ToyTupleReader::SetObservable(RooArgSet * _observables) { m_observables = _observables; }

RooDataSet * ToyTupleReader::NextToyData(uint _index) {
    DeleteOldData(m_dataset);
    OpenFile();
    GetTupleHeaders(_index);
    SetupComponentReaders();
    TFile _tFile("tmp.root", to_string(OpenMode::RECREATE));   // PATCH TO AVOID Error in <TBranch::TBranch::WriteBasketImpl>: basket's WriteBuffer failed WHEN CLONING
    ExtractComponentsFromFile(_index);
    CreateDataset();
    ClearTupleTrees();
    CloseFile();
    _tFile.Close();
    return m_dataset;
}

void ToyTupleReader::DeleteOldData(RooAbsData * data) {
    if (data != nullptr) { delete data; }
}

void ToyTupleReader::GetTupleHeaders(uint _index) {
    m_tupleHeaderHandler.SetToyTupleFile(m_file);
    if (SettingDef::Toy::mergeConfig) {
        MessageSvc::Info("ToyTupleReader", (TString) "GetTupleHeaders", to_string(_index), m_key);
        m_componentHeaders = m_tupleHeaderHandler.GetHeaders(m_yieldConfigs, _index, m_key);
    }
    else
        m_componentHeaders = m_tupleHeaderHandler.GetHeaders(m_yieldConfigs);
}

void ToyTupleReader::OpenFile() {
    m_toyFileHandler.SetTupleOpenMode(OpenMode::READ);
    m_file = m_toyFileHandler.GetNextToyTupleFile();
}

void ToyTupleReader::SetupComponentReaders() {
    for (int i = 0; i < m_componentReaders.size(); i++) {
        ToyTupleComponentHeader & componentHeader = m_componentHeaders[i];
        ToyTupleComponentReader & componentReader = m_componentReaders[i];
        componentReader.SetSourceFile(m_file);
        componentReader.SetHeader(componentHeader);
    }
}

void ToyTupleReader::ExtractComponentsFromFile(uint _index) {
    for (auto & componentReader : m_componentReaders) {
        TTree * componentTree = nullptr;
        if (SettingDef::Toy::mergeConfig)
            componentTree = componentReader.GetComponentTreeFromFile(_index, m_key);
        else
            componentTree = componentReader.GetComponentTreeFromFile();
        m_tupleComponentTrees.push_back(componentTree);
    }
}

void ToyTupleReader::CreateDataset() {
    TList * treeList      = FillTListWithTupleTrees();
    TTree * combinedTuple = TTree::MergeTrees(treeList);
    ConvertTreeToDataset(combinedTuple);
    delete combinedTuple;
    delete treeList;
}

TList * ToyTupleReader::FillTListWithTupleTrees() const {
    TList * list = new TList();
    for (auto & componentTree : m_tupleComponentTrees) { list->Add(componentTree); }
    return list;
}

void ToyTupleReader::ConvertTreeToDataset(TTree * combinedTuple) {
    m_dataset = new RooDataSet(m_datasetName.Data(), m_datasetName.Data(), combinedTuple, *m_observables);
    m_dataset->convertToVectorStore();
}

void ToyTupleReader::ClearTupleTrees() { m_tupleComponentTrees.clear(); }

void ToyTupleReader::CloseFile() { m_toyFileHandler.CloseTupleFile(); }

RooDataHist * ToyTupleReader::BinCurrentToy() {
    DeleteOldData(m_datahist);
    CreateBinnedFromDataset();
    return m_datahist;
}

void ToyTupleReader::CreateBinnedFromDataset() {
    m_datahist = new RooDataHist(m_datahistName.Data(), m_datahistName.Data(), *m_observables, *m_dataset);
}

ToyTupleReader & ToyTupleReader::operator=(const ToyTupleReader & _other) {
    m_toyFileHandler         = _other.m_toyFileHandler;
    m_tupleConfigLoader      = _other.m_tupleConfigLoader;
    m_tupleHeaderHandler     = _other.m_tupleHeaderHandler;
    m_componentReaders       = _other.m_componentReaders;
    m_componentHeaders       = _other.m_componentHeaders;
    m_sampleToTupleConfigMap = _other.m_sampleToTupleConfigMap;
    m_yieldConfigs           = _other.m_yieldConfigs;
    m_datasetName            = _other.m_datasetName;
    m_datahistName           = _other.m_datahistName;
    return *this;
}

ToyTupleReader & ToyTupleReader::operator=(ToyTupleReader && _other) {
    m_toyFileHandler         = move(_other.m_toyFileHandler);
    m_tupleConfigLoader      = move(_other.m_tupleConfigLoader);
    m_tupleHeaderHandler     = move(_other.m_tupleHeaderHandler);
    m_componentReaders       = move(_other.m_componentReaders);
    m_componentHeaders       = move(_other.m_componentHeaders);
    m_sampleToTupleConfigMap = move(_other.m_sampleToTupleConfigMap);
    m_yieldConfigs           = move(_other.m_yieldConfigs);
    m_datasetName            = _other.m_datasetName;
    m_datahistName           = _other.m_datahistName;
    return *this;
}

ToyTupleReader::~ToyTupleReader() {
    MessageSvc::Warning("ToyTupleReader", (TString) "Delete", m_key);
    DeleteOldData(m_dataset);
    DeleteOldData(m_datahist);
}

#endif