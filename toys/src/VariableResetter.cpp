#ifndef VARIABLERESETTER_CPP
#define VARIABLERESETTER_CPP

#include "VariableResetter.hpp"

#include "TIterator.h"

void VariableResetter::AddVariable(RooRealVar & variable) {
    if (notDuplicated(variable)) { appendVariable(variable); }
}

bool VariableResetter::notDuplicated(RooRealVar & variable) {
    RooRealVar * address = &variable;
    for (int i = 0; i < m_listOfVariables.size(); i++) {
        if (m_listOfVariables[i] == address) { return false; }
    }
    return true;
}

void VariableResetter::appendVariable(RooRealVar & variable) {
    m_listOfVariables.push_back(&variable);
    m_initialValues.push_back(variable.getValV());
    m_initialErrors.push_back(variable.getError());
}

void VariableResetter::AddVariablesFromModel(RooAbsPdf & model) {
    RooArgSet &  allVars = *(model.getVariables());
    TIterator *  iter    = allVars.createIterator();
    RooAbsReal * var;
    while ((var = (RooAbsReal *) iter->Next())) {
        if (not(var->isDerived()) && not(var->isConstant()) && not((TString) var->GetName() == "dummyBlindState")) {   // Derivative of blinded variables are excluded
            AddVariable(*((RooRealVar *) var));
        }
    }
    delete iter;
}

void VariableResetter::ResetAllVariables() {
    RooRealVar * var;
    double       initialValue;
    double       initialError;
    for (int i = 0; i < m_listOfVariables.size(); i++) {
        var          = m_listOfVariables[i];
        initialValue = m_initialValues[i];
        initialError = m_initialErrors[i];
        var->setVal(initialValue);
        var->setError(initialError);
    }
}

#endif