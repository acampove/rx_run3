#ifndef TOYTUPLECOMPONENTGENERATOR_CPP
#define TOYTUPLECOMPONENTGENERATOR_CPP

#include "ToyTupleComponentGenerator.hpp"

#include "MessageSvc.hpp"

#include "RooDataSet.h"
#include "RooRandom.h"
#include "TFile.h"
#include "TList.h"
#include "TTree.h"
#include "RooArgSet.h"
#include "RooHistPdf.h"
TRandom3 ToyTupleComponentGenerator::m_randomNumberGenerator = TRandom3();

ToyTupleComponentGenerator::ToyTupleComponentGenerator() {}

ToyTupleComponentGenerator::ToyTupleComponentGenerator(const ToyTupleComponentGenerator & other) {
    m_tupleConfig = other.m_tupleConfig;
    m_yieldConfig = other.m_yieldConfig;
}

ToyTupleComponentGenerator::ToyTupleComponentGenerator(ToyTupleComponentGenerator && other) {
    m_tupleConfig = move(other.m_tupleConfig);
    m_yieldConfig = move(other.m_yieldConfig);
}

ToyTupleComponentGenerator::ToyTupleComponentGenerator(const ToyTupleConfig & tupleConfig, const ToyYieldConfig & yieldConfig) {
    SetTupleConfig(tupleConfig);
    SetYieldConfig(yieldConfig);
}

void ToyTupleComponentGenerator::SetTupleConfig(const ToyTupleConfig & tupleConfig) {
    m_tupleConfig = tupleConfig;
    CheckSampleMatches();
}

void ToyTupleComponentGenerator::SetYieldConfig(const ToyYieldConfig & yieldConfig) {
    m_yieldConfig = yieldConfig;
    CheckSampleMatches();
}

void ToyTupleComponentGenerator::CheckSampleMatches() const {
    if (BothConfigSet()) { ThrowIfSamplesDoNotMatch(); }
}

bool ToyTupleComponentGenerator::BothConfigSet() const {
    bool yieldConfigSet = m_yieldConfig.IsSet();
    bool tupleConfigSet = m_tupleConfig.IsSet();
    return (yieldConfigSet && tupleConfigSet);
}

void ToyTupleComponentGenerator::ThrowIfSamplesDoNotMatch() const {
    TString tupleSample = m_tupleConfig.GetSample();
    TString yieldSample = m_yieldConfig.GetSample();
    if (tupleSample != yieldSample) {
        MessageSvc::Line();
        MessageSvc::Info("ToyTupleConfig sample: ", tupleSample);
        MessageSvc::Info("ToyYieldConfig sample: ", yieldSample);
        MessageSvc::Error("ToyTupleComponentGenerator", "The samples do not match!", "logic_error");
    }
}

void ToyTupleComponentGenerator::WriteComponentTreeToFile(TFile * targetFile, uint _index, TString _key) {
    SetTargetFile(targetFile);
    GenerateDataTree(_index, _key);
}

void ToyTupleComponentGenerator::SetTargetFile(TFile * targetFile) { m_targetFile = targetFile; }

void ToyTupleComponentGenerator::GenerateDataTree(uint _index, TString _key) {
    int totalEvents = SmearGeneratorMean();
    GenerateTupleTree(totalEvents);
    WriteTreeToFile(_index, _key);
    UpdateGeneratedEvents(totalEvents);
}

int ToyTupleComponentGenerator::SmearGeneratorMean() { return ToyTupleComponentGenerator::m_randomNumberGenerator.Poisson((double) m_yieldConfig.GetMeanEvents()); }

void ToyTupleComponentGenerator::GenerateTupleTree(int nEvents) {
    RooAbsPdf *  generatorPdf = m_tupleConfig.GetPdf();
    RooArgSet * _observables  = m_tupleConfig.GetObservables();
    // if( true){
    // TODO: hack around the fgBrem fractions, maybe fix at generation time ? , set forced to  0 ? 
    //     TIterator *  it     = _observables->createIterator();
    //     RooRealVar * arg;
    //     while ((arg = (RooRealVar *) it->Next())) {
    //         TString  ParamName  = arg->GetName();
    //         if (ParamName.Contains("fgBrem")){
    //             std::cout<<RED << "FOUND! "<< ParamName << std::endl;
    //             // ParamName.Print();
    //         }
    //     }
    // }
    if( generatorPdf->ClassName() == "RooHistPdf"){
        std::cout<< "Generator with RooHistPdf"<<std::endl;
        RooDataSet * dataset      = generatorPdf->generate(*_observables, RooFit::AutoBinned(0), RooFit::NumEvents(nEvents));
        m_componentTree           = dataset->GetClonedTree();
        delete dataset;
    }else{
        RooDataSet * dataset      = generatorPdf->generate(*_observables, nEvents);
        m_componentTree           = dataset->GetClonedTree();
        delete dataset;
    }
}

void ToyTupleComponentGenerator::WriteTreeToFile(uint _index, TString _key) {
    TString treeName = m_yieldConfig.GetComponentName();
    m_componentTree->SetName(treeName.Data());
    m_targetFile->cd();
    if (_key != "") {
        TString _treeDir = (TString) to_string(_index) + "/" + _key;
        if (m_targetFile->GetDirectory(_treeDir) == 0) m_targetFile->mkdir(_treeDir);
        m_targetFile->cd(_treeDir);
    }
    m_componentTree->Write(0, TObject::kOverwrite);
}

void ToyTupleComponentGenerator::UpdateGeneratedEvents(int generatedEvents) { m_lastGeneratedEvents = generatedEvents; }

ToyTupleComponentHeader ToyTupleComponentGenerator::GetLastGeneratedHeader() const {
    TString                 sample          = m_yieldConfig.GetSample();
    int                     generatorMean   = m_yieldConfig.GetMeanEvents();
    int                     generatedEvents = m_lastGeneratedEvents;
    ToyTupleComponentHeader header(sample, generatorMean, generatedEvents);
    return move(header);
}

ToyTupleComponentGenerator & ToyTupleComponentGenerator::operator=(const ToyTupleComponentGenerator & other) {
    m_tupleConfig = other.m_tupleConfig;
    m_yieldConfig = other.m_yieldConfig;
    return *this;
}

ToyTupleComponentGenerator & ToyTupleComponentGenerator::operator=(ToyTupleComponentGenerator && other) {
    m_tupleConfig = move(other.m_tupleConfig);
    m_yieldConfig = move(other.m_yieldConfig);
    return *this;
}

void ToyTupleComponentGenerator::SetSeed(unsigned long seed) {
    ToyTupleComponentGenerator::PrintNewSeedMessage(seed);
    ToyTupleComponentGenerator::m_randomNumberGenerator.SetSeed(seed);
    RooRandom::randomGenerator()->SetSeed(seed);
}

void ToyTupleComponentGenerator::PrintNewSeedMessage(unsigned long seed) {
    MessageSvc::Line();
    MessageSvc::Info((TString) "ToyTupleComponentGenerator", (TString) "Changing RooFit and ToyTupleComponentGenerator seed");
    MessageSvc::Info("New Seed", TString::LLtoa(seed, 10));
    MessageSvc::Line();
}

ToyTupleComponentGenerator::~ToyTupleComponentGenerator() {}

#endif
