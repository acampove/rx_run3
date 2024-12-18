#ifndef FITRESULTLOGGER_HPP
#define FITRESULTLOGGER_HPP

#include "TMatrixDSym.h"

// Forward declaration of classes
class TFile;
class TTree;
class RooAbsReal;
class RooRealVar;
class RooFormulaVar;
class RooAbsPdf;
class RooFitResult;
class RooArgList;

using namespace std;
/**
 * \brief A struct which helps factorise data handling when dumping results into an nTuple.
 */
struct singleFitResult_t {
    double *     values;
    double *     errors;
    int          covarianceQuality;
    int          fitStatus;
    int          minimizeStatus;
    int          migradStatus;
    int          minosStatus;
    int          hesseStatus;
    double       edm;
    double       minNll;
    int          numInvalidNLL;
    Double_t     offsetNLL;
    unsigned int numFits;
   TMatrixDSym correlationMatrix;
   TMatrixDSym covarianceMatrix;
};

// TODO : Modify logger to output results as it runs, logger2, oh yeah!

/**
 *  \class FitResultLogger
 *  \brief Logs the results produced by toy fit loops and saves the result into a nTuple.
 */
class FitResultLogger {
  public:
    /**
     * \brief Default constructor.
     */
    FitResultLogger();
    /**
     * Default destructor.
     */
    ~FitResultLogger(){};

    /**
     * \brief Adds a single RooRealVar to log.
     */
    void AddVariable(RooRealVar & variable);
    /**
     * \brief Adds a single RooFormulaVar to log.
     */
    void AddFormulaVar(RooFormulaVar & formulaVar);
    /**
     * \brief Finds all the RooRealVar within a RooAbsPdf and adds them to the logger.
     * \param model The RooAbsPdf from where variables are retrieved.
     */
    void AddVariablesFromModel(const RooAbsPdf & model);
    /**
     * \brief Logs a toy fit result.
     * \brief The values of each RooRealVar and RooFormulaVar added to this logger is saved.
     * \brief Also saves the fitStatus, correlationQuality, correlation matrix and covariance matrix of the fit.
     * \param fitResult A RooFitResult object produced by RooFit after convergence. The convergence quality (matrices and status code) are requested from it.
     */
    void LogFit(const RooFitResult & fitResult, const Double_t & offsetLL);
    /**
     * \brief Dumps all logged variables and fit convergence quality objects into an nTuple.
     */
    void SaveResults(TString fileName, TString treeName);

    static const TString parListBranchName;
    static const TString parListTreeName;

  private:
    void    CheckLogNotStarted() const;
    bool    VariableNotDuplicated(const RooRealVar & variable) const;
    void    AppendVariable(RooRealVar & variable);
    bool    FormulaVarNotDuplicated(const RooFormulaVar & formulaVar) const;
    void    AppendFormulaVar(RooFormulaVar & formulaVar);
    void    CheckResultConsistency(const RooFitResult & fitResult);
    void    RecordParNames(const RooArgList & listOfParNames);
    void    CheckParCount(const RooArgList & listOfFittedPars) const;
    void    CheckParNames(const RooArgList & listOfFittedPars) const;
    void    LogAllVariables();
    void    LogAllFormulaVars(const RooFitResult & fitResult);
    void    LogFitStatus(const RooFitResult & fitResult);
    void    LogMatrices(const RooFitResult & toyFitResult);
    void    LogOffsetLL(const Double_t & _offsetLLUsed);
    void    InstantiateOutputs(const TString & fileName, const TString & treeName);
    void    CheckLogStarted() const;
    void    AllocateContainers();
    void    AttachBranchesToTree();
    void    AddBranch(const RooAbsReal & realValueProxy, int proxyIndex);
    TString RemoveMathFormula(TString initialString) const;
    void    FillTree();
    int     CalculateNumberOfFits() const;
    void    FillEvent(int index);
    void    FillParNameTree();
    void    WriteAndClose();
    void    PrintSuccessfulOutput(TString fileName, TString treeName) const;

  private:
    std::vector< RooRealVar * >    m_listOfVariables;
    std::vector< RooFormulaVar * > m_listOfFormulaVars;
    std::vector< TString >         m_listOfParNames;
    std::vector< double >          m_values;
    std::vector< double >          m_errors;
    std::vector< int >             m_covarianceQualities;
    std::vector< int >             m_fitStatuses;
    std::vector< int >             m_minimizeStatus;
    std::vector< int >             m_migradStatus;
    std::vector< int >             m_minosStatus;
    std::vector< int >             m_hesseStatus;
    std::vector< double >          m_edm;
    std::vector< double >          m_minNll;
    std::vector< int >             m_numInvalidNLL;
    std::vector< Double_t >        m_offsetsNLL;
    std::vector< unsigned int >    m_numFits;

    std::vector< TMatrixDSym >     m_correlationMatrices;
    std::vector< TMatrixDSym >     m_covarianceMatrices;

    TFile *           m_outFile     = nullptr;
    TTree *           m_outTree     = nullptr;
    TTree *           m_parNameTree = nullptr;
    singleFitResult_t m_singleFitResult;
    int               m_numberOfVariables;
    int               m_numberOfFormulaVars;
};

#endif