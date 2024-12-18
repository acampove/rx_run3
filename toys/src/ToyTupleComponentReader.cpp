#ifndef TOYTUPLECOMPONENTREADER_CPP
#define TOYTUPLECOMPONENTREADER_CPP

#include "ToyTupleComponentReader.hpp"

#include "MessageSvc.hpp"
#include "poissoninv.hpp"

#include "TFile.h"
#include "TString.h"
#include "TTree.h"
#include "SettingDef.hpp"


ToyTupleComponentReader::ToyTupleComponentReader() { m_componentTreeName = ""; }

ToyTupleComponentReader::ToyTupleComponentReader(const ToyTupleComponentReader & other) {
    m_yieldConfig       = other.m_yieldConfig;
    m_componentTreeName = other.m_componentTreeName;
}

ToyTupleComponentReader::ToyTupleComponentReader(ToyTupleComponentReader && other) {
    m_yieldConfig       = move(other.m_yieldConfig);
    m_componentTreeName = other.m_componentTreeName;
}

ToyTupleComponentReader::ToyTupleComponentReader(const ToyYieldConfig & yieldConfig) { SetYieldConfig(yieldConfig); }

void ToyTupleComponentReader::SetYieldConfig(const ToyYieldConfig & yieldConfig) {
    m_yieldConfig       = yieldConfig;
    m_componentTreeName = yieldConfig.GetComponentName();
}

void ToyTupleComponentReader::SetSourceFile(TFile * sourceFile) { m_sourceFile = sourceFile; }

void ToyTupleComponentReader::SetHeader(const ToyTupleComponentHeader & componentHeader) {
    m_componentHeader = componentHeader;
    ThrowIfHeaderAndConfigIncompatible();
}

void ToyTupleComponentReader::ThrowIfHeaderAndConfigIncompatible() const {
    TString headerSample = m_componentHeader.GetSample();
    TString yieldSample  = m_yieldConfig.GetSample();
    if (headerSample != yieldSample) {
        MessageSvc::Line();
        MessageSvc::Info("ToyTupleComponentReader");
        MessageSvc::Info("ToyTupleComponentHeader Sample: " + headerSample);
        MessageSvc::Info("ToyYieldConfig Sample: " + yieldSample);
        MessageSvc::Error("ToyTupleComponentReader", "The label of the header and tuple config does not match.", "logic_error");
    }
}

TTree * ToyTupleComponentReader::GetComponentTreeFromFile(uint _index, TString _key) {
    CheckTupleAndHeaderMean();
    int nEventsToClone = GetNumberOfEventsToClone();
    ReadComponentTreeFromFile(_index, _key);
    CloneComponentTreeFromFile(nEventsToClone, _key);
    return m_clonedTree;
}

void ToyTupleComponentReader::CheckTupleAndHeaderMean() {
    int requestedMean = m_yieldConfig.GetMeanEvents();
    int generatorMean = m_componentHeader.GetGeneratorMean();
    if (requestedMean > generatorMean) {
        RequestedMeanMoreThanGeneratorMeanError();
    } else if (requestedMean == generatorMean) {
        m_readStrategy = ToyTupleComponentReader::ReadStrategy::ReadAll;
    } else if (requestedMean < generatorMean) {
        m_readStrategy = ToyTupleComponentReader::ReadStrategy::ReadReduced;
    }
}

void ToyTupleComponentReader::RequestedMeanMoreThanGeneratorMeanError() const {
    TString sample        = m_yieldConfig.GetSample();
    int     requestedMean = m_yieldConfig.GetMeanEvents();
    int     generatorMean = m_componentHeader.GetGeneratorMean();
    MessageSvc::Line();
    MessageSvc::Info("ToyTupleComponentReader");
    MessageSvc::Info("Sample: ", sample);
    MessageSvc::Info("Requested Mean: ", TString::Itoa(requestedMean, 10));
    MessageSvc::Info("Generator Mean: ", TString::Itoa(generatorMean, 10));
    MessageSvc::Error("ToyTupleComponentReader", "The requested mean exceeds the generator mean found in the header of the toy tuple.", "logic_error");
}

int ToyTupleComponentReader::GetNumberOfEventsToClone() const {
    int nEventsToClone;
    switch (m_readStrategy) {
        case ToyTupleComponentReader::ReadStrategy::ReadAll: nEventsToClone = m_componentHeader.GetGeneratedEvents(); break;
        case ToyTupleComponentReader::ReadStrategy::ReadReduced: nEventsToClone = CalculateReducedMeanInversePoissonCDF(); break;
    }
    return nEventsToClone;
}

double ToyTupleComponentReader::CalculateReducedMeanInversePoissonCDF() const {
    int    reducedPoissonMean = m_yieldConfig.GetMeanEvents();
    double poissonCDF         = m_componentHeader.GetPoissonCDF();
    double inverseCDF         = PoissonInverse::poissinv(poissonCDF, reducedPoissonMean);
    return inverseCDF;
}

void ToyTupleComponentReader::ReadComponentTreeFromFile(uint _index, TString _key) {
    TString _treePath = "";
    if (_key != "") _treePath = (TString) to_string(_index) + "/" + _key + "/" + m_componentTreeName.Data();
    else            _treePath = m_componentTreeName.Data();
    MessageSvc::Info("ToyTupleComponentReader", (TString) "ReadComponentTreeFromFile", _treePath);
    m_componentTree = (TTree *) m_sourceFile->Get(_treePath);
    if (m_componentTree == nullptr) { MessageSvc::Error("ToyTupleComponentReader", "Component tree not found in file.", _treePath); }
}

void ToyTupleComponentReader::CloneComponentTreeFromFile(int nEventsToClone, TString _key ) { 
    if(SettingDef::Toy::ReadFractionToysComponents == "" ||  _key.Contains("-MM-")){
        std::cout<< "CloneComponentTreeFromFile " << _key << " : " << m_componentTreeName << std::endl;
        std::cout<< "CloneComponentTreeFromFile Baseline Entries:  " << m_componentTree->GetEntries() << std::endl;
        m_clonedTree = m_componentTree->CloneTree(nEventsToClone); 
        std::cout<< "CloneComponentTreeFromFile Kept     Entries:  " << m_clonedTree->GetEntries() << std::endl;
    }else{
        std::cout<< "CloneComponentTreeFromFile " << _key << " : " << m_componentTreeName << std::endl;
        double  FractionKeep = SettingDef::Toy::ReductionFactor.at(_key).at(m_componentTreeName);
        if (FractionKeep == 1.0){
            std::cout<< "CloneComponentTreeFromFile Baseline Entries:  " << m_componentTree->GetEntries() << std::endl;
            m_clonedTree = m_componentTree->CloneTree(nEventsToClone); 
            std::cout<< "CloneComponentTreeFromFile Kept     Entries:  " << m_clonedTree->GetEntries() << std::endl;
        }else{
            if( FractionKeep > 0 ){
                double  nEntries       =   double(m_componentTree->GetEntries());
                double  nEntriesKeep   =   double(m_randomNumberGenerator.Poisson( std::round(FractionKeep * nEntries) ));
                int     nMultipleFill  = int(std::floor( nEntriesKeep/nEntries));
                m_clonedTree = m_componentTree->CloneTree(0);
                std::cout<< "CloneComponentTreeFromFile Original Entries:  " << nEntries     << std::endl;
                std::cout<< "CloneComponentTreeFromFile Kept     Entries:  " << nEntriesKeep << std::endl;
                std::cout<< "CloneComponentTreeFromFile Ratio    Entries:  " << nEntriesKeep/nEntries << std::endl;
                std::cout<< "CloneComponentTreeFromFile MultipleFills   :  " << nMultipleFill << std::endl;
                for (size_t i = 0; i < m_componentTree->GetEntries(); i++){                
                    for( int J = 0; J < nMultipleFill; ++J){
                        m_componentTree->GetEntry(i);
                        m_clonedTree->Fill();
                    }
                    if (ToyTupleComponentReader::m_randomNumberGenerator.Uniform(0., 1.) <   (nEntriesKeep - nMultipleFill*nEntries)/nEntries ){
                        m_componentTree->GetEntry(i);
                        m_clonedTree->Fill();
                    }
                }
                std::cout<< "CloneComponentTreeFromFile Kept Final Entries:  " << m_clonedTree->GetEntries() << std::endl;
                std::cout<< "CloneComponentTreeFromFile Kept Ratio Entries:  " << float(m_clonedTree->GetEntries())/float(nEntries) << std::endl;
            }else if (FractionKeep ==0){
                //zero out the component/empty fully
                m_clonedTree = m_componentTree->CloneTree(0);
            }else{
                MessageSvc::Error("Cannot support negative fractions");
            }
        }
    }
}

ToyTupleComponentReader & ToyTupleComponentReader::operator=(const ToyTupleComponentReader & other) {
    m_yieldConfig       = other.m_yieldConfig;
    m_componentTreeName = other.m_componentTreeName;
    return *this;
}

ToyTupleComponentReader & ToyTupleComponentReader::operator=(ToyTupleComponentReader && other) {
    m_yieldConfig       = move(other.m_yieldConfig);
    m_componentTreeName = other.m_componentTreeName;
    return *this;
}


TRandom3 ToyTupleComponentReader::m_randomNumberGenerator = TRandom3();
void ToyTupleComponentReader::SetSeed(unsigned long seed) {
    ToyTupleComponentReader::PrintNewSeedMessage(seed);
    ToyTupleComponentReader::m_randomNumberGenerator.SetSeed(seed);
}

void ToyTupleComponentReader::PrintNewSeedMessage(unsigned long seed) {
    MessageSvc::Line();
    MessageSvc::Info((TString) "ToyTupleComponentReader", (TString) "Changing ToyTupleComponentReader seed");
    MessageSvc::Info("New Seed", TString::LLtoa(seed, 10));
    MessageSvc::Line();
    return;
}

ToyTupleComponentReader::~ToyTupleComponentReader() {}

#endif