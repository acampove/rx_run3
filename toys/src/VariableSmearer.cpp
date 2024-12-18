#ifndef VARIABLESMEARER_CPP
#define VARIABLESMEARER_CPP

#include "VariableSmearer.hpp"
#include "MurmurHash3.h"

#include <iostream>

#include "MessageSvc.hpp"

VariableSmearer::VariableSmearer(RooRealVar * _variableToSmear, double _meanValue, double _error, smearDistribution distribution) // Sometimes we smear the Gaussian constrained mean
{
    cout << "Smearing Variable : " << _variableToSmear->GetName() << endl;
    cout << "Mean : "  << _meanValue << endl;
    cout << "Error : " << _error << endl;
    cout << " === " << endl;

    m_meanValue       = _meanValue;
    m_error           = _error;
    m_distribution    = distribution;
    m_variableToSmear = _variableToSmear;
}

void VariableSmearer::SmearVariable() {
    double _smearedValue = SmearedValue();
    m_variableToSmear->setVal(_smearedValue);
    MessageSvc::Info("Post-Smear", (TString) m_variableToSmear->GetName(), to_string(_smearedValue));
}

double VariableSmearer::SmearedValue() {
    double newValue;
    switch (m_distribution) {
        case smearDistribution::Gauss: newValue = gaussianSmear(); break;
        case smearDistribution::Poisson: newValue = poissonSmear(); break;
    }
    return newValue;
}

double VariableSmearer::gaussianSmear() {
    double newValue = m_randomNumberGenerator.Gaus(m_meanValue, m_error);
    return newValue;
}

double VariableSmearer::poissonSmear() {
    double newValue = m_randomNumberGenerator.Poisson(m_meanValue);
    return newValue;
}

void VariableSmearer::ResetSeed(uint32_t _first32Bits) {
    TString _name(m_variableToSmear->GetName());
    uint32_t _nameHash;
    MurmurHash3_x86_32(_name.Data(), _name.Length(), _first32Bits, &_nameHash);
    unsigned long _generatorSeed = (unsigned long) _first32Bits << 32 | _nameHash;
    m_randomNumberGenerator.SetSeed(_generatorSeed);
}

#endif
