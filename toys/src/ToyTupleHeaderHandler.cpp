#ifndef TOYTUPLEHEADERHANDLER_CPP
#define TOYTUPLEHEADERHANDLER_CPP

#include "ToyTupleHeaderHandler.hpp"
#include "MessageSvc.hpp"
#include <algorithm>

ToyTupleHeaderHandler::ToyTupleHeaderHandler() { m_toyTupleFile = nullptr; }

ToyTupleHeaderHandler::ToyTupleHeaderHandler(const ToyTupleHeaderHandler & other) {
    m_initialComponentHeadersMap   = other.m_initialComponentHeadersMap;
    m_requestedComponentHeaders    = other.m_requestedComponentHeaders;
    m_nonRequestedComponentHeaders = other.m_nonRequestedComponentHeaders;
    m_updatedComponentHeaders      = other.m_updatedComponentHeaders;
    m_headerTree                   = other.m_headerTree;
    m_toyTupleFile                 = other.m_toyTupleFile;
}

void ToyTupleHeaderHandler::SetToyTupleFile(TFile * toyTupleFile) { m_toyTupleFile = toyTupleFile; }

std::vector< ToyTupleComponentHeader > ToyTupleHeaderHandler::GetHeaders(const std::vector< ToyYieldConfig > & yieldConfigs, uint _index, TString _key) {
    ThrowIfFileNotGiven();
    DeleteOldHeaders();
    std::vector< TString > requestedSampleKeys = GetRequestedSampleKeys(yieldConfigs);
    ExtractHeadersFromFile(_index, _key);
    GetRequestedComponentHeaders(requestedSampleKeys);
    GetNonRequestedHeaders(requestedSampleKeys);
    return m_requestedComponentHeaders;
}

void ToyTupleHeaderHandler::ThrowIfFileNotGiven() const {
    if (m_toyTupleFile == nullptr || (!(m_toyTupleFile->IsOpen()))) { MessageSvc::Error("ToyTupleHeaderHandler", "Trying to retrieve header tree but an open file was not passed."); }
}

void ToyTupleHeaderHandler::DeleteOldHeaders() {
    m_initialComponentHeadersMap.clear();
    m_requestedComponentHeaders.clear();
    m_nonRequestedComponentHeaders.clear();
    m_updatedComponentHeaders.clear();
}

std::vector< TString > ToyTupleHeaderHandler::GetRequestedSampleKeys(const std::vector< ToyYieldConfig > & yieldConfigs) const {
    std::vector< TString > requestedSampleKeys;
    for (auto & yieldConfig : yieldConfigs) { requestedSampleKeys.push_back(yieldConfig.GetSample()); }
    return requestedSampleKeys;
}

void ToyTupleHeaderHandler::ExtractHeadersFromFile(uint _index, TString _key) {
    GetHeaderTreeFromFile(_index, _key);
    if (HeaderTreeExists() && HeaderTreeHasEntry()) { ExtractHeadersFromHeaderTree(); }
}

void ToyTupleHeaderHandler::GetHeaderTreeFromFile(uint _index, TString _key) {
    TString _treePath = "";
    if (_key != "")
        _treePath = (TString) to_string(_index) + "/" + _key + "/" + m_headerTreeName;
    else
        _treePath = m_headerTreeName;
    MessageSvc::Info("ToyTupleHeaderHandler", (TString) "GetHeaderTreeFromFile", _treePath);
    m_headerTree = (TTree *) m_toyTupleFile->Get(_treePath);
    if (m_headerTree == nullptr) { MessageSvc::Error("ToyTupleHeaderHandler", "Header tree not found in file.", _treePath); }
}

bool ToyTupleHeaderHandler::HeaderTreeExists() const { return (m_headerTree != nullptr); }

bool ToyTupleHeaderHandler::HeaderTreeHasEntry() const { return (m_headerTree->GetEntries() != 0); }

void ToyTupleHeaderHandler::ExtractHeadersFromHeaderTree() {
    MessageSvc::Info("ToyTupleHeaderHandler", (TString) "ExtractHeadersFromHeaderTree");
    MessageSvc::Line();
    ToyTupleComponentHeader componentHeader;
    componentHeader.AttachToTree(m_headerTree);
    for (int i = 0; i < m_headerTree->GetEntries(); i++) {
        m_headerTree->GetEntry(i);
        TString sampleKey                       = componentHeader.GetSample();
        m_initialComponentHeadersMap[sampleKey] = componentHeader;
    }
}

void ToyTupleHeaderHandler::GetRequestedComponentHeaders(const std::vector< TString > & requestedSampleKeys) {
    for (auto & sampleKey : requestedSampleKeys) {
        ToyTupleComponentHeader header = RetrieveInitialHeader(sampleKey);
        m_requestedComponentHeaders.push_back(header);
    }
}

ToyTupleComponentHeader ToyTupleHeaderHandler::RetrieveInitialHeader(const TString & sampleKey) const {
    ToyTupleComponentHeader header;
    if (InitialHeadersContains(sampleKey)) {
        header = GetInitialComponentHeader(sampleKey);
    } else {
        header = CreateEmptyHeader(sampleKey);
    }
    return std::move(header);
}

bool ToyTupleHeaderHandler::InitialHeadersContains(const TString & sampleKey) const {
    bool sampleFound = (m_initialComponentHeadersMap.find(sampleKey) != m_initialComponentHeadersMap.end());
    return sampleFound;
}

ToyTupleComponentHeader ToyTupleHeaderHandler::GetInitialComponentHeader(const TString & sampleKey) const {
    ToyTupleComponentHeader header = m_initialComponentHeadersMap.at(sampleKey);
    return std::move(header);
}

ToyTupleComponentHeader ToyTupleHeaderHandler::CreateEmptyHeader(const TString & sampleKey) const {
    ToyTupleComponentHeader header(sampleKey);
    return std::move(header);
}

void ToyTupleHeaderHandler::GetNonRequestedHeaders(const std::vector< TString > & requestedSampleKeys) {
    for (auto & headerKeyPair : m_initialComponentHeadersMap) {
        TString                   headerKey = headerKeyPair.first;
        ToyTupleComponentHeader & header    = headerKeyPair.second;
        if (HeaderNotRequested(headerKey, requestedSampleKeys)) { m_nonRequestedComponentHeaders.push_back(header); }
    }
}

bool ToyTupleHeaderHandler::HeaderNotRequested(const TString & headerKey, const std::vector< TString > & requestedSampleKeys) const {
    auto & keys         = requestedSampleKeys;
    bool   notRequested = (std::find(keys.begin(), keys.end(), headerKey) == keys.end());
    return notRequested;
}

void ToyTupleHeaderHandler::WriteHeaders(const std::vector< ToyTupleComponentHeader > & finalHeaders, uint _index, TString _key) {
    FillUpdatedHeaders(finalHeaders);
    SortUpdatedHeadersBySample();
    DeleteHeaderTreeFromFile();
    CreateNewHeaderTree(_index, _key);
    FillHeaderTree();
    WriteUpdatedHeader();
}

void ToyTupleHeaderHandler::FillUpdatedHeaders(const std::vector< ToyTupleComponentHeader > & finalHeaders) {
    m_updatedComponentHeaders = m_nonRequestedComponentHeaders;
    m_updatedComponentHeaders.insert(m_updatedComponentHeaders.end(), finalHeaders.begin(), finalHeaders.end());
}

void ToyTupleHeaderHandler::SortUpdatedHeadersBySample() {
    std::sort(m_updatedComponentHeaders.begin(), m_updatedComponentHeaders.end(), [](const ToyTupleComponentHeader & a, const ToyTupleComponentHeader & b) { return (a.GetSample() < b.GetSample()); });
}

void ToyTupleHeaderHandler::DeleteHeaderTreeFromFile() { m_toyTupleFile->Delete(m_headerTreeNameWithCycle); }

void ToyTupleHeaderHandler::CreateNewHeaderTree(uint _index, TString _key) {
    m_toyTupleFile->cd();
    if (_key != "") {
        TString _treeDir = (TString) to_string(_index) + "/" + _key;
        if (m_toyTupleFile->GetDirectory(_treeDir) == 0) m_toyTupleFile->mkdir(_treeDir);
        m_toyTupleFile->cd(_treeDir);
    }
    m_headerTree = new TTree(m_headerTreeName, "");
}

void ToyTupleHeaderHandler::FillHeaderTree() {
    ToyTupleComponentHeader headerToWrite;
    headerToWrite.CreateBranches(m_headerTree);
    for (auto & header : m_updatedComponentHeaders) {
        headerToWrite = header;
        m_headerTree->Fill();
    }
}

void ToyTupleHeaderHandler::WriteUpdatedHeader() { m_headerTree->Write(); }

#endif