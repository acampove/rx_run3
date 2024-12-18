#ifndef FITRESULTLOGGER_CPP
#define FITRESULTLOGGER_CPP

#include "FitResultLogger.hpp"
#include "TFile.h"
#include "TString.h"
#include "TTree.h"

#include "IOSvc.hpp"
#include "MessageSvc.hpp"
#include "RooAbsPdf.h"
#include "RooAbsReal.h"
#include "RooFitResult.h"
#include "RooFormulaVar.h"
#include "RooRealVar.h"
#include <stdexcept>
#include <vector>

#include "TFile.h"
#include "TTree.h"

#include "IOSvc.hpp"
#include "MessageSvc.hpp"

const TString FitResultLogger::parListBranchName = "name";
const TString FitResultLogger::parListTreeName   = "ParameterNames";

FitResultLogger::FitResultLogger() {
    m_numberOfVariables   = 0;
    m_numberOfFormulaVars = 0;
}

void FitResultLogger::AddVariable(RooRealVar & variable) {
    CheckLogNotStarted();
    if (VariableNotDuplicated(variable)) { AppendVariable(variable); }
}

void FitResultLogger::CheckLogNotStarted() const {
    if (!m_values.empty()) { MessageSvc::Error("FitResultLogger", "Logging has started but a new variable is added.", "logic_error"); }
}

bool FitResultLogger::VariableNotDuplicated(const RooRealVar & variable) const {
    for (int i = 0; i < m_listOfVariables.size(); i++) {
        bool duplicatedAddress = (m_listOfVariables[i] == &variable);
        bool duplicatedName    = (TString(m_listOfVariables[i]->GetName()) == TString(variable.GetName()));
        if (duplicatedAddress || duplicatedName) { return false; }
    }
    return true;
}

void FitResultLogger::AppendVariable(RooRealVar & variable) {
    m_listOfVariables.push_back(&variable);
    m_numberOfVariables++;
}

void FitResultLogger::AddFormulaVar(RooFormulaVar & formulaVar) {
    CheckLogNotStarted();
    if (FormulaVarNotDuplicated(formulaVar)) { AppendFormulaVar(formulaVar); }
}

bool FitResultLogger::FormulaVarNotDuplicated(const RooFormulaVar & formulaVar) const {
    for (int i = 0; i < m_listOfFormulaVars.size(); i++) {
        bool duplicatedAddress = (m_listOfFormulaVars[i] == &formulaVar);
        bool duplicatedName    = (TString(m_listOfFormulaVars[i]->GetName()) == TString(formulaVar.GetName()));
        if (duplicatedAddress || duplicatedName) { return false; }
    }
    return true;
}

void FitResultLogger::AppendFormulaVar(RooFormulaVar & formulaVar) {
    m_listOfFormulaVars.push_back(&formulaVar);
    m_numberOfFormulaVars++;
}

void FitResultLogger::AddVariablesFromModel(const RooAbsPdf & model) {
    RooArgSet &  allVars = *(model.getVariables());
    TIterator *  iter    = allVars.createIterator();
    RooRealVar * var;
    while ((var = (RooRealVar *) iter->Next())) { AddVariable(*var); }
    delete iter;
}

void FitResultLogger::LogFit(const RooFitResult & fitResult, const Double_t & offsetLL) {
    CheckResultConsistency(fitResult);
    LogAllVariables();
    LogAllFormulaVars(fitResult);
    LogFitStatus(fitResult);
    LogMatrices(fitResult);
    LogOffsetLL(offsetLL);
}

void FitResultLogger::CheckResultConsistency(const RooFitResult & fitResult) {
    const RooArgList & listOfFittedPars = fitResult.floatParsFinal();
    if (m_listOfParNames.size() == 0) {
        RecordParNames(listOfFittedPars);
    } else {
        CheckParCount(listOfFittedPars);
        CheckParNames(listOfFittedPars);
    }
}

void FitResultLogger::RecordParNames(const RooArgList & listOfFittedPars) {
    TString parName;
    for (int i = 0; i < listOfFittedPars.getSize(); i++) {
        parName = TString(listOfFittedPars.at(i)->GetName());
        m_listOfParNames.push_back(parName);
    }
}

void FitResultLogger::CheckParCount(const RooArgList & listOfFittedPars) const {
    int thisFitResultParCount     = listOfFittedPars.getSize();
    int previousFitResultParCount = m_listOfParNames.size();
    if (thisFitResultParCount != previousFitResultParCount) { MessageSvc::Error("FitResultLogger", "This RooFitResult contains different parameters compared to the last RooFitResult", "logic_error"); }
}

void FitResultLogger::CheckParNames(const RooArgList & listOfFittedPars) const {
    TString thisFitParName;
    TString lastFitParName;
    for (int i = 0; i < m_listOfParNames.size(); i++) {
        thisFitParName = TString(listOfFittedPars.at(i)->GetName());
        lastFitParName = TString(m_listOfParNames[i]);
        if (thisFitParName != lastFitParName) { MessageSvc::Error("FitResultLogger", "This RooFitResult contains parameters with names that do not match the previous RooFitResult.", "logic_error"); }
    }
}

void FitResultLogger::LogAllVariables() {
    RooRealVar * variablePointer;
    double       value;
    double       error;
    for (int i = 0; i < m_numberOfVariables; i++) {
        variablePointer = m_listOfVariables[i];
        value           = variablePointer->getValV();
        error           = variablePointer->getError();
        m_values.push_back(value);
        m_errors.push_back(error);
    }
}

void FitResultLogger::LogAllFormulaVars(const RooFitResult & fitResult) {
    RooFormulaVar * formulaVarPointer;
    double          value;
    double          error;
    for (int i = 0; i < m_numberOfFormulaVars; i++) {
        formulaVarPointer = m_listOfFormulaVars[i];
        value             = formulaVarPointer->getValV();
        error             = formulaVarPointer->getPropagatedError(fitResult);
        m_values.push_back(value);
        m_errors.push_back(error);
    }
}

void FitResultLogger::LogFitStatus(const RooFitResult & fitResult) {
    int covarianceQuality = fitResult.covQual();
    int fitStatus         = fitResult.status();
    int minimizeStatus    = -999;
    int migradStatus      = -999;
    int minosStatus       = -999;
    int hesseStatus       = -999;
    for (uint i = 0; i < fitResult.numStatusHistory(); ++i) {
        if ((TString) fitResult.statusLabelHistory(i) == "MINIMIZE") minimizeStatus = fitResult.statusCodeHistory(i);
        if ((TString) fitResult.statusLabelHistory(i) == "MIGRAD") migradStatus = fitResult.statusCodeHistory(i);
        if ((TString) fitResult.statusLabelHistory(i) == "MINOS") minosStatus = fitResult.statusCodeHistory(i);
        if ((TString) fitResult.statusLabelHistory(i) == "HESSE") hesseStatus = fitResult.statusCodeHistory(i);
    }
    double edm           = fitResult.edm();
    double minNll        = fitResult.minNll();
    int    numInvalidNLL = fitResult.numInvalidNLL();
    unsigned int numFits = fitResult.numStatusHistory();

    m_covarianceQualities.push_back(covarianceQuality);
    m_fitStatuses.push_back(fitStatus);
    m_minimizeStatus.push_back(minimizeStatus);
    m_migradStatus.push_back(migradStatus);
    m_minosStatus.push_back(minosStatus);
    m_hesseStatus.push_back(hesseStatus);
    m_edm.push_back(edm);
    m_minNll.push_back(minNll);
    m_numInvalidNLL.push_back(numInvalidNLL);
    m_numFits.push_back(numFits);
}

void FitResultLogger::LogOffsetLL( const Double_t & offsetLL){
    m_offsetsNLL.push_back(offsetLL);
}
void FitResultLogger::LogMatrices(const RooFitResult & fitResult) {
    TMatrixDSym correlationMatrix = fitResult.correlationMatrix();
    TMatrixDSym covarianceMatrix  = fitResult.covarianceMatrix();
    m_correlationMatrices.push_back(correlationMatrix);
    m_covarianceMatrices.push_back(covarianceMatrix);
}

void FitResultLogger::SaveResults(TString fileName, TString treeName) {
    CheckLogStarted();
    InstantiateOutputs(fileName, treeName);
    AllocateContainers();
    AttachBranchesToTree();
    FillTree();
    FillParNameTree();
    WriteAndClose();
    PrintSuccessfulOutput(fileName, treeName);
};

void FitResultLogger::CheckLogStarted() const {
    if (m_covarianceQualities.empty()) { MessageSvc::Error("FitResultLogger", "Tried to save output but nothing was logged.", "logic_error"); }
}

void FitResultLogger::InstantiateOutputs(const TString & fileName, const TString & treeName) {
    m_outFile     = IOSvc::OpenFile(fileName.Data(), OpenMode::RECREATE);
    m_outTree     = new TTree(treeName, "");
    m_parNameTree = new TTree(parListTreeName, "");
}

void FitResultLogger::AllocateContainers() {
    m_singleFitResult.values = (double *) malloc((m_numberOfVariables + m_numberOfFormulaVars + 2) * sizeof(double));
    m_singleFitResult.errors = (double *) malloc((m_numberOfVariables + m_numberOfFormulaVars + 2) * sizeof(double));
    m_singleFitResult.correlationMatrix.ResizeTo(m_correlationMatrices[0]);
    m_singleFitResult.covarianceMatrix.ResizeTo(m_covarianceMatrices[0]);
}

void FitResultLogger::AttachBranchesToTree() {
    for (int i = 0; i < m_numberOfVariables; i++) { AddBranch(*(m_listOfVariables[i]), i); }
    for (int i = 0; i < m_numberOfFormulaVars; i++) { AddBranch(*(m_listOfFormulaVars[i]), i + m_numberOfVariables); }
    m_outTree->Branch("covarianceQuality", &m_singleFitResult.covarianceQuality);
    m_outTree->Branch("fitStatus", &m_singleFitResult.fitStatus);
    m_outTree->Branch("minimizeStatus", &m_singleFitResult.minimizeStatus);
    m_outTree->Branch("migradStatus", &m_singleFitResult.migradStatus);
    m_outTree->Branch("minosStatus", &m_singleFitResult.minosStatus);
    m_outTree->Branch("hesseStatus", &m_singleFitResult.hesseStatus);
    m_outTree->Branch("edm", &m_singleFitResult.edm);
    m_outTree->Branch("minNll", &m_singleFitResult.minNll);
    m_outTree->Branch("numInvalidNLL", &m_singleFitResult.numInvalidNLL);
    m_outTree->Branch("correlationMatrix", &m_singleFitResult.correlationMatrix);
    m_outTree->Branch("covarianceMatrix", &m_singleFitResult.covarianceMatrix);
    m_outTree->Branch("offsetLL", &m_singleFitResult.offsetNLL);
    m_outTree->Branch("numFits", &m_singleFitResult.numFits);

}

void FitResultLogger::AddBranch(const RooAbsReal & realValueProxy, int proxyIndex) {
    TString valueName = RemoveMathFormula(TString(realValueProxy.GetName()));
    TString errorName = valueName + "_error";
    m_outTree->Branch(valueName.Data(), m_singleFitResult.values + proxyIndex);
    m_outTree->Branch(errorName.Data(), m_singleFitResult.errors + proxyIndex);
}

TString FitResultLogger::RemoveMathFormula(TString initialName) const {
    TString newString = initialName.ReplaceAll("-", "_");
    return newString;
}

void FitResultLogger::FillTree() {
    int numberOfFits = CalculateNumberOfFits();
    for (int index = 0; index < numberOfFits; index++) { FillEvent(index); }
}

int FitResultLogger::CalculateNumberOfFits() const {
    int numberOfValues = m_values.size();
    return numberOfValues / (m_numberOfVariables + m_numberOfFormulaVars);
}

void FitResultLogger::FillEvent(int index) {
    int offset = index * (m_numberOfVariables + m_numberOfFormulaVars);
    for (int i = 0; i < (m_numberOfVariables + m_numberOfFormulaVars); i++) {
        m_singleFitResult.values[i] = m_values[offset + i];
        m_singleFitResult.errors[i] = m_errors[offset + i];
    }
    m_singleFitResult.covarianceQuality = m_covarianceQualities[index];
    m_singleFitResult.fitStatus         = m_fitStatuses[index];
    m_singleFitResult.minimizeStatus    = m_minimizeStatus[index];
    m_singleFitResult.migradStatus      = m_migradStatus[index];
    m_singleFitResult.minosStatus       = m_minosStatus[index];
    m_singleFitResult.hesseStatus       = m_hesseStatus[index];
    m_singleFitResult.edm               = m_edm[index];
    m_singleFitResult.minNll            = m_minNll[index];
    m_singleFitResult.numInvalidNLL     = m_numInvalidNLL[index];
    m_singleFitResult.correlationMatrix = m_correlationMatrices[index];
    m_singleFitResult.covarianceMatrix  = m_covarianceMatrices[index];
    m_singleFitResult.offsetNLL         = m_offsetsNLL[index];
    m_singleFitResult.numFits           = m_numFits[index];
    m_outTree->Fill();
}

void FitResultLogger::FillParNameTree() {
    TString parName;
    m_parNameTree->Branch(parListBranchName, &parName);
    for (int i = 0; i < m_listOfParNames.size(); i++) {
        parName = RemoveMathFormula(m_listOfParNames[i]);
        m_parNameTree->Fill();
    } 
}

void FitResultLogger::WriteAndClose() {
    m_outTree->Write();
    m_parNameTree->Write();
    m_outFile->Close();
    free(m_singleFitResult.values);
    free(m_singleFitResult.errors);
}

void FitResultLogger::PrintSuccessfulOutput(TString fileName, TString treeName) const {
    MessageSvc::Line();
    MessageSvc::Info("FitResultLogger");
    MessageSvc::Info("File", fileName);
    MessageSvc::Info("Tuple", treeName);
    MessageSvc::Line();
}

#endif