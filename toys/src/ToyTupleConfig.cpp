#ifndef TOYTUPLECONFIG_CPP
#define TOYTUPLECONFIG_CPP

#include "ToyTupleConfig.hpp"

#include "MessageSvc.hpp"

ToyTupleConfig::ToyTupleConfig() { Init(); }

ToyTupleConfig::ToyTupleConfig(const TString & pdfKey, const TString & observableKey, const TString & sample, double nominalYield, const TTimeStamp & timestamp, RooAbsPdf & pdf) {
    Init();
    *m_sample          = sample;
    m_nominalYield     = nominalYield;
    *m_pdfKey          = pdfKey;
    *m_observableKey   = observableKey;
    m_pdf              = &pdf;
    *m_timestamp       = timestamp;
    GetObservableValues();
}

ToyTupleConfig::ToyTupleConfig(const ToyTupleConfig & other) {
    Init();
    *m_sample          = *other.m_sample;
    m_nominalYield     = other.m_nominalYield;
    *m_pdfKey          = *other.m_pdfKey;
    *m_observableKey   = *other.m_observableKey;
    m_pdf              = other.m_pdf;
    *m_timestamp       = *other.m_timestamp;
    *m_observableTitle = *other.m_observableTitle;
    m_observableMin    = other.m_observableMin;
    m_observableMax    = other.m_observableMax;
}

ToyTupleConfig::ToyTupleConfig(ToyTupleConfig && other) {
    Init();
    *m_sample          = *other.m_sample;
    m_nominalYield     = move(other.m_nominalYield);
    *m_pdfKey          = *other.m_pdfKey;
    *m_observableKey   = *other.m_observableKey;
    m_pdf              = move(other.m_pdf);
    *m_timestamp       = move(*other.m_timestamp);
    *m_observableTitle = *other.m_observableTitle;
    m_observableMin    = move(other.m_observableMin);
    m_observableMax    = move(other.m_observableMax);
}

void ToyTupleConfig::Init() {
    m_sample          = new TString("");
    m_pdfKey          = new TString("");
    m_observableKey   = new TString("");
    m_timestamp       = new TTimeStamp();
    m_observableTitle = new TString("");
}

void ToyTupleConfig::AttachToTree(TTree * tree) {
    if (tree == nullptr) MessageSvc::Error("ToyTupleConfig", "AttachToTree TTree is a nullptr", "EXIT_FAILURE");
    m_setBranchAddressReturnCode[0] = tree->SetBranchAddress("sample", &m_sample);
    m_setBranchAddressReturnCode[1] = tree->SetBranchAddress("nominalYield", &m_nominalYield);
    m_setBranchAddressReturnCode[2] = tree->SetBranchAddress("pdfKey", &m_pdfKey);
    m_setBranchAddressReturnCode[3] = tree->SetBranchAddress("observableKey", &m_observableKey);
    m_setBranchAddressReturnCode[4] = tree->SetBranchAddress("timestamp", &m_timestamp);
    m_setBranchAddressReturnCode[5] = tree->SetBranchAddress("observableTitle", &m_observableTitle);
    m_setBranchAddressReturnCode[6] = tree->SetBranchAddress("observableMin", &m_observableMin);
    m_setBranchAddressReturnCode[7] = tree->SetBranchAddress("observableMax", &m_observableMax);
    for (auto & returnCode : m_setBranchAddressReturnCode) {
        if (returnCode != m_setBranchAddressOkCode) MessageSvc::Error("ToyTupleConfig", "An error code was return calling TTree::SetBranchAddress", to_string(returnCode), "EXIT_FAILURE");
    }
}

void ToyTupleConfig::CreateBranches(TTree * tree) {
    if (tree == nullptr) MessageSvc::Error("ToyTupleConfig", "CreateBranches TTree is a nullptr", "EXIT_FAILURE");
    tree->Branch("sample", m_sample);
    tree->Branch("nominalYield", &m_nominalYield);
    tree->Branch("pdfKey", m_pdfKey);
    tree->Branch("observableKey", m_observableKey);
    tree->Branch("timestamp", m_timestamp);
    tree->Branch("observableTitle", m_observableTitle);
    tree->Branch("observableMin", &m_observableMin);
    tree->Branch("observableMax", &m_observableMax);
}

void ToyTupleConfig::GetObservableValues() {
    RooArgSet * allVariables = m_pdf->getVariables();
    const RooRealVar& observable = (RooRealVar&)(*allVariables)[m_observableKey->Data()];
    *m_observableTitle = observable.GetTitle();
    m_observableMin = observable.getMin();
    m_observableMax = observable.getMax();
}

RooArgSet * ToyTupleConfig::GetObservables() {
    if (m_observables == nullptr){
        auto * _var = new RooRealVar(m_observableKey->Data(), m_observableTitle->Data(), 
                                     m_observableMin, m_observableMax);
        m_observables = new RooArgSet();
        m_observables->addOwned(*_var);
    }
    return m_observables;
}

bool ToyTupleConfig::IsSet() const { return !(*m_sample == ""); }

void ToyTupleConfig::LoadPdfFromWorkspace(RooWorkspace * workspace, TString _key) {
    m_pdf = workspace->pdf(m_pdfKey->Data());
    if (m_pdf == nullptr) MessageSvc::Error("ToyTupleConfig", "PDF Key", *m_pdfKey, "not found", "EXIT_FAILURE");
}

void ToyTupleConfig::DeletePdf() {
    if(m_pdf != nullptr){
        delete m_pdf;
        m_pdf = nullptr;
    }
}

void ToyTupleConfig::Print() const {
    TString _rangeString = "[" + to_string(m_observableMin) + "," + to_string(m_observableMax) + "]";
    MessageSvc::Line();
    MessageSvc::Info("ToyTupleConfig");
    MessageSvc::Info("Sample", *m_sample);
    MessageSvc::Info("Nominal Yield", to_string(m_nominalYield));
    MessageSvc::Info("Pdf Key", *m_pdfKey);
    MessageSvc::Info("Observable", *m_observableKey, *m_observableTitle, _rangeString);
    MessageSvc::Info("Time Stamp", TString(m_timestamp->AsString()));
}

ToyTupleConfig & ToyTupleConfig::operator=(const ToyTupleConfig & other) {
    *m_sample          = *other.m_sample;
    m_nominalYield     = other.m_nominalYield;
    *m_pdfKey          = *other.m_pdfKey;
    *m_observableKey   = *other.m_observableKey;
    m_pdf              = other.m_pdf;
    *m_timestamp       = *other.m_timestamp;
    *m_observableTitle = *other.m_observableTitle;
    m_observableMin    = other.m_observableMin;
    m_observableMax    = other.m_observableMax;
    return *this;
}

ToyTupleConfig & ToyTupleConfig::operator=(ToyTupleConfig && other) {
    *m_sample          = *other.m_sample;
    m_nominalYield     = move(other.m_nominalYield);
    *m_pdfKey          = *other.m_pdfKey;
    *m_observableKey   = *other.m_observableKey;
    m_pdf              = move(other.m_pdf);
    *m_timestamp       = move(*other.m_timestamp);
    *m_observableTitle = *other.m_observableTitle;
    m_observableMin    = move(other.m_observableMin);
    m_observableMax    = move(other.m_observableMax);
    return *this;
}

void ToyTupleConfig::DeleteObservables() {
    if (m_observables != nullptr) {
        delete m_observables;
        m_observables = nullptr;
    }
}

ToyTupleConfig::~ToyTupleConfig() {
    delete m_sample;
    delete m_pdfKey;
    delete m_observableKey;
    delete m_timestamp;
    DeleteObservables();
}

#endif
