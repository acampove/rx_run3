#ifndef CONSTRAINTSGENERATOR_CPP
#define CONSTRAINTSGENERATOR_CPP

#include "ConstraintsGenerator.hpp"

#include "FitterSvc.hpp"

ConstraintsGenerator::ConstraintsGenerator() {
    m_listOfConstraints          = RooArgList("allConstraints");
    m_listOfConstrainedVariables = RooArgSet("constrainingVariables");
}

RooGaussian & ConstraintsGenerator::GenerateConstraint(RooRealVar & variable) {
    RooRealVar &  mean          = CreateMean(variable);
    RooConstVar & sigma         = CreateSigma(variable);
    TString _nameVar = variable.GetName();
    MessageSvc::Warning("Setting range of 1D Gauss Constrained Parameter to +/- 10 sigma");
    //if( _nameVar.Contains("nbkg") && mean.getVal() >0){
    //variable.setMin(0);
    //}else{
    variable.setMin(mean.getVal() - 10 * sigma.getVal());
    //}
    variable.setMax(mean.getVal() + 10 * sigma.getVal());
    RooGaussian & pdfConstraint = CreateGaussianConstraint(variable, mean, sigma);
    m_variableToMeanMap[&variable] = &mean;
    return pdfConstraint;
}

RooRealVar & ConstraintsGenerator::CreateMean(RooRealVar & variable) {
    m_listOfConstrainedVariables.add(variable);
    TString      meanName          = TString(variable.GetName()) + "_mean";
    RooRealVar * mean              = new RooRealVar(meanName.Data(), meanName.Data(), variable.getValV());
    m_variableCollector.push_back(mean);
    return *mean;
}

RooConstVar & ConstraintsGenerator::CreateSigma(RooRealVar & variable) {
    TString       sigmaName = TString(variable.GetName()) + "_sigma";
    RooConstVar * sigma     = new RooConstVar(sigmaName.Data(), sigmaName.Data(), variable.getError());
    m_variableCollector.push_back(sigma);
    return *sigma;
}

RooGaussian & ConstraintsGenerator::CreateGaussianConstraint(RooRealVar & variable, RooAbsReal & mean, RooConstVar & sigma) {
    TString constraintName = TString(variable.GetName()) + "_pdfconstraint";
    RooGaussian * pdfConstraint  = new RooGaussian(constraintName.Data(), constraintName.Data(), variable, mean, sigma);
    //https://root-forum.cern.ch/t/convolution-of-gauss-pdf-and-step-function-pdf-using-roofit/21016
    auto _relUnctGaussConstraint = abs(sigma.getVal()/mean.getVal()) * 100 ;
    if( sigma.getVal() < 0) MessageSvc::Warning("GenerateConstraint, negative error, please check");
    MessageSvc::Info( TString::Format("CreateGaussianConstraint %s s/m", constraintName.Data())  , TString::Format( "( %.2f ) per-cent", _relUnctGaussConstraint ));    
    m_constraintCollector.push_back(pdfConstraint);
    m_listOfConstraints.add(*pdfConstraint);
    return *pdfConstraint;
}

RooMultiVarGaussianSuppressWarning & ConstraintsGenerator::GenerateCorrelatedConstraints(RooArgList & listOfVariables, TMatrixDSym covarianceMatrix) {
    assert(listOfVariables.getSize() == covarianceMatrix.GetNrows());
    RooArgList &                         listOfMeans   = CreateListOfMeans(listOfVariables);
    RooMultiVarGaussianSuppressWarning & pdfConstraint = CreateMultiVarGaussianConstraint(listOfVariables, listOfMeans, covarianceMatrix);
    LogCorrelatedConstraintsInfo(listOfVariables, listOfMeans, covarianceMatrix);
    return pdfConstraint;
}

RooMultiVarGaussianNoNorm & ConstraintsGenerator::GenerateCorrelatedConstraintsNoNorm(RooArgList & listOfVariables, TMatrixDSym covarianceMatrix) {
    assert(listOfVariables.getSize() == covarianceMatrix.GetNrows());
    RooArgList &                         listOfMeans   = CreateListOfMeans(listOfVariables);
    RooMultiVarGaussianNoNorm & pdfConstraint = CreateMultiVarGaussianConstraintNoNorm(listOfVariables, listOfMeans, covarianceMatrix);
    LogCorrelatedConstraintsInfo(listOfVariables, listOfMeans, covarianceMatrix);
    return pdfConstraint;
}

RooArgList & ConstraintsGenerator::CreateListOfMeans(RooArgList & listOfVariables) {
    TString      listName    = "list" + to_string(m_argListCollector.size());
    RooArgList * listOfMeans = new RooArgList(listName);
    for (int i = 0; i < listOfVariables.getSize(); i++) {
        RooRealVar * variable = (RooRealVar *) listOfVariables.at(i);
        RooRealVar & mean     = CreateMean(*variable);
        listOfMeans->add(mean);
    }
    m_argListCollector.push_back(listOfMeans);
    return *(listOfMeans);
}

RooMultiVarGaussianSuppressWarning & ConstraintsGenerator::CreateMultiVarGaussianConstraint(RooArgList & listOfVariables, RooArgList & listOfMeans, TMatrixDSym & covarianceMatrix) {
    TString                              constraintName = CombineNamesInList("correlated_gaussian_constraint", listOfVariables);
    RooMultiVarGaussianSuppressWarning * pdfConstraint  = new RooMultiVarGaussianSuppressWarning(constraintName.Data(), constraintName.Data(), listOfVariables, listOfMeans, covarianceMatrix);
    m_constraintCollector.push_back(pdfConstraint);
    m_listOfConstraints.add(*pdfConstraint);
    return *pdfConstraint;
}
RooMultiVarGaussianNoNorm & ConstraintsGenerator::CreateMultiVarGaussianConstraintNoNorm(RooArgList & listOfVariables, RooArgList & listOfMeans, TMatrixDSym & covarianceMatrix) {
    TString                              constraintName = CombineNamesInList("correlated_gaussian_constraint", listOfVariables);
    RooMultiVarGaussianNoNorm * pdfConstraint  = new RooMultiVarGaussianNoNorm(constraintName.Data(), constraintName.Data(), listOfVariables, listOfMeans, covarianceMatrix);
    m_constraintCollector.push_back(pdfConstraint);
    m_listOfConstraints.add(*pdfConstraint);
    return *pdfConstraint;
}

TString ConstraintsGenerator::CombineNamesInList(TString header, RooArgList & listOfVariables) {
    TString varName;
    TString returnedName = header;
    for (int i = 0; i < listOfVariables.getSize(); i++) {
        varName = TString(listOfVariables.at(i)->GetName());
        returnedName += "_" + varName;
    }
    return returnedName;
}

void ConstraintsGenerator::LogCorrelatedConstraintsInfo(const RooArgList& listOfVariables, const RooArgList& listOfMeans, const TMatrixDSym& covarianceMatrix){
    vector <RooRealVar*> vectorOfVariables;
    vector <RooRealVar*> vectorOfMeans;
    for (auto iter = listOfVariables.begin(); iter != listOfVariables.end(); iter++){
        auto * variable = static_cast<RooRealVar*>(*iter);
        vectorOfVariables.push_back(variable);
    }
    for (auto iter = listOfMeans.begin(); iter != listOfMeans.end(); iter++){
        auto * mean = static_cast<RooRealVar*>(*iter);
        vectorOfMeans.push_back(mean);
    }
    m_correlatedConstraintsInfo.emplace_back(vectorOfVariables, vectorOfMeans, covarianceMatrix);
}

RooArgSet & ConstraintsGenerator::GetAllConstraints() { return m_listOfConstraints; }

map< RooRealVar *, RooRealVar * > ConstraintsGenerator::GetVariableToMeanMap() { return m_variableToMeanMap; }

vector< CorrelatedConstraintsInfo > ConstraintsGenerator::GetCorrelatedConstraintsInfos() { return m_correlatedConstraintsInfo; }

RooAddition ConstraintsGenerator::AddConstraintsToNLL(RooAbsReal & NLL) {
    TString constraintName = "constraintsFor_" + TString(NLL.GetName());
    TString sumedNllName   = TString(NLL.GetName()) + "_withConstraints";
    GenerateConstraintNLL(constraintName);
    RooAddition sumedNLL = RooAddition(sumedNllName.Data(), "nllWithCons", RooArgSet(NLL, *m_allConstraints));
    return move(sumedNLL);
}

RooConstraintSum & ConstraintsGenerator::GenerateConstraintNLL(TString name) {
    DeleteConstraintSum();
    SumAllConstraints(name);
    return *m_allConstraints;
}

void ConstraintsGenerator::DeleteConstraintSum() {
    if (m_allConstraints != nullptr) { 
        delete m_allConstraints; 
        m_allConstraints = nullptr;
    }
}

void ConstraintsGenerator::SumAllConstraints(TString name) { m_allConstraints = new RooConstraintSum(name.Data(), "constraintSum", m_listOfConstraints, m_listOfConstrainedVariables); }

bool ConstraintsGenerator::HasConstraints() { return (m_listOfConstraints.getSize() != 0); }

ConstraintsGenerator::~ConstraintsGenerator() {
    ClearCollector(m_variableCollector);
    ClearCollector(m_constraintCollector);
    ClearCollector(m_argListCollector);
    DeleteConstraintSum();
}

template < typename T > void ConstraintsGenerator::ClearCollector(vector< T > & collector) {
    for (int i = 0; i < collector.size(); i++) { delete collector[i]; }
    collector.clear();
}

#endif
