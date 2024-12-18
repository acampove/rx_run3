#ifndef CORRELATEDVARIABLESSMEARER_CPP
#define CORRELATEDVARIABLESSMEARER_CPP

#include "TDecompChol.h"

#include "CorrelatedVariablesSmearer.hpp"
#include "MessageSvc.hpp"
#include "MurmurHash3.h"

#include <iostream>

CorrelatedVariablesSmearer::CorrelatedVariablesSmearer(vector <RooRealVar *> _variablesToSmear, vector <double> _meanValues, TMatrixDSym& _covarianceMatrix) : 
    m_choleskyDecomposition(_covarianceMatrix.GetNrows(), _covarianceMatrix.GetNcols()), // Construct the decomposed, lower triangular matrix to have appropriate size
    m_meanValues(_meanValues.size())
{
    if (_variablesToSmear.size() != _meanValues.size()){
        MessageSvc::Error("CorrelatedVariablesSmearer", (TString)"_variablesToSmear and _meanValues sizes do not match!");
    }
    MessageSvc::Info("CorrelatedVariablesSmearer", (TString)"Correlated Variables");
    cout << "Correlated Smearing" << endl;
    for (int i = 0; i < _variablesToSmear.size(); i++){
        MessageSvc::Info("CorrelatedVariablesSmearer", TString(_variablesToSmear[i]->GetName()), to_string(_meanValues[i]));
    }
    MessageSvc::Info("CorrelatedVariablesSmearer", (TString)"Covariance Matrix");
    _covarianceMatrix.Print();
    MessageSvc::Line();

    m_variablesToSmear = _variablesToSmear;
    m_nVariables = _variablesToSmear.size();

    // Init the mean TVectorD
    for (int i = 0; i < m_nVariables; i++){
        m_meanValues(i) = _meanValues[i];
    }

    // Get the cholesky decomposition of the covariance matrix1
    TDecompChol cholesky(_covarianceMatrix);
    cholesky.Decompose();
    auto decomp = cholesky.GetU();
    m_choleskyDecomposition = decomp.Transpose(decomp); // We need to transpose the upper triangular matrix into lower triangular matrix
}

void CorrelatedVariablesSmearer::SmearVariables() {
    TVectorD _correlatedVariables = GenerateCorrelatedMeans();
    SetMeanValues(_correlatedVariables);
}

TVectorD CorrelatedVariablesSmearer::GenerateCorrelatedMeans() {
    // x' = U * x + a
    // x : uncorrelated random variables with mean 0, width 1
    // U : lower triangular matrix from cholesky decomposition of covariance
    // a : mean of target distribution
    TVectorD _correlatedVariables = GenerateUncorrelatedGaussians();
    _correlatedVariables *= m_choleskyDecomposition;
    _correlatedVariables += m_meanValues;

    return _correlatedVariables;
}

TVectorD CorrelatedVariablesSmearer::GenerateUncorrelatedGaussians() {
    TVectorD _uncorrelatedGaussians(m_nVariables);
    for (int i = 0; i < m_nVariables; i++){
        _uncorrelatedGaussians(i) = m_randomNumberGenerator.Gaus(0, 1); // Use the same RNG as VariableSmearer
    }
    return _uncorrelatedGaussians;
}

void CorrelatedVariablesSmearer::SetMeanValues(const TVectorD& _correlatedVariables){
    for (int i = 0; i < m_nVariables; i++){
        m_variablesToSmear[i]->setVal(_correlatedVariables(i));
    }
    for (int i = 0; i < m_nVariables; i++){
        TString _name(m_variablesToSmear[i]->GetName());
        MessageSvc::Info("Post-Smear", _name, to_string(_correlatedVariables(i)));
    }    
}

// First 32 bits are set externally (typically job ID)
void CorrelatedVariablesSmearer::ResetSeed(uint32_t _first32Bits){
    vector <TString> _names;
    vector <uint32_t> _hashes;

    // Get the 32 bit hashes of all the smeared variables
    for (auto * variable : m_variablesToSmear){
        TString _name(variable->GetName());
        uint32_t _nameHash;
        MurmurHash3_x86_32(_name.Data(), _name.Length(), _first32Bits, &_nameHash);
        _hashes.push_back(_nameHash);
    }

    // Use the XOR operation on all the individual variable hashes
    // XOR operation is both associative and commutative (i.e. the last 32 bits are order invariant)
    uint32_t _last32Bits = _hashes[0];
    for (size_t i = 1; i < _hashes.size(); i++){
        _last32Bits = _last32Bits ^ _hashes[i];
    }

    unsigned long _generatorSeed = (unsigned long) _first32Bits << 32 | _last32Bits;
    m_randomNumberGenerator.SetSeed(_generatorSeed);
}

#endif
