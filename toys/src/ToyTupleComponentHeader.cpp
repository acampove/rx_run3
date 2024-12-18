#ifndef TOYTUPLECOMPONENTHEADER_CPP
#define TOYTUPLECOMPONENTHEADER_CPP

#include "ToyTupleComponentHeader.hpp"

#include "Math/ProbFunc.h"

ToyTupleComponentHeader::ToyTupleComponentHeader() {
    m_sample          = new TString("");
    m_headerExistFlag = false;
}

ToyTupleComponentHeader::ToyTupleComponentHeader(const ToyTupleComponentHeader & other)
    : ToyTupleComponentHeader() {
    *m_sample         = *other.m_sample;
    m_generatorMean   = other.m_generatorMean;
    m_generatedEvents = other.m_generatedEvents;
    m_poissonCDF      = other.m_poissonCDF;
    m_headerExistFlag = other.m_headerExistFlag;
}

ToyTupleComponentHeader::ToyTupleComponentHeader(const ToyTupleComponentHeader && other)
    : ToyTupleComponentHeader() {
    *m_sample         = *other.m_sample;
    m_generatorMean   = move(other.m_generatorMean);
    m_generatedEvents = move(other.m_generatedEvents);
    m_poissonCDF      = move(other.m_poissonCDF);
    m_headerExistFlag = move(other.m_headerExistFlag);
}

ToyTupleComponentHeader::ToyTupleComponentHeader(TString sample) {
    *m_sample         = sample;
    m_headerExistFlag = false;
}

ToyTupleComponentHeader::ToyTupleComponentHeader(TString sample, int generatorMean, int generatedEvents)
    : ToyTupleComponentHeader() {
    *m_sample         = sample;
    m_generatorMean   = generatorMean;
    m_generatedEvents = generatedEvents;
    m_headerExistFlag = true;
    EvaluatePoissonCDF();
}

void ToyTupleComponentHeader::EvaluatePoissonCDF() {
    m_poissonCDF = ROOT::Math::poisson_cdf(m_generatedEvents, m_generatorMean) - 1e-8;
    // Need to subtract a small number so that inverse Poisson CDF is not offset by 1
}

void ToyTupleComponentHeader::AttachToTree(TTree * headerTree) {
    headerTree->SetBranchAddress(m_sampleBranchName, &m_sample);
    headerTree->SetBranchAddress(m_generatorMeanBranchName, &m_generatorMean);
    headerTree->SetBranchAddress(m_generatedEventsBranchName, &m_generatedEvents);
    headerTree->SetBranchAddress(m_poissonCDFBranchName, &m_poissonCDF);
}

void ToyTupleComponentHeader::CreateBranches(TTree * headerTree) {
    headerTree->Branch(m_sampleBranchName, m_sample);
    headerTree->Branch(m_generatorMeanBranchName, &m_generatorMean);
    headerTree->Branch(m_generatedEventsBranchName, &m_generatedEvents);
    headerTree->Branch(m_poissonCDFBranchName, &m_poissonCDF);
}

ToyTupleComponentHeader & ToyTupleComponentHeader::operator=(const ToyTupleComponentHeader & other) {
    *m_sample         = *other.m_sample;
    m_generatorMean   = other.m_generatorMean;
    m_generatedEvents = other.m_generatedEvents;
    m_poissonCDF      = other.m_poissonCDF;
    m_headerExistFlag = other.m_headerExistFlag;
    return *this;
}

ToyTupleComponentHeader & ToyTupleComponentHeader::operator=(const ToyTupleComponentHeader && other) {
    *m_sample         = *other.m_sample;
    m_generatorMean   = move(other.m_generatorMean);
    m_generatedEvents = move(other.m_generatedEvents);
    m_poissonCDF      = move(other.m_poissonCDF);
    m_headerExistFlag = move(other.m_headerExistFlag);
    return *this;
}

#endif