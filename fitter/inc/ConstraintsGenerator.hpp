#ifndef CONSTRAINTSGENERATOR_HPP
#define CONSTRAINTSGENERATOR_HPP

#include <vector>

#include "TString.h"

#include "RooAddition.h"
#include "RooArgList.h"
#include "RooArgSet.h"
#include "RooConstVar.h"
#include "RooConstraintSum.h"
#include "RooGaussian.h"
#include "RooMultiVarGaussianSuppressWarning.h"
#include "RooMultiVarGaussianNoNorm.h"
#include "RooRealVar.h"

#include "FitterSvc.hpp"

using namespace std;

/** \class ConstraintsGenerator
 * \brief A class that takes care of creating constraints
 * Creates the required RooFit objects to constraint variables.
 */
class ConstraintsGenerator {

  public:
    /**
     * \brief Constructor
     */
    ConstraintsGenerator();

    /**
     * \brief Destructor to clean up all objects owned by ConstraintGenerator.
     */
    ~ConstraintsGenerator();

    /**
     * \brief Creates a gaussian constraint on variable.
     * \param variable - The RooRealVar to constraint.
     * \return Reference to the gaussian PDF that constraints the variable.
     * \note The gaussian constraint will have mean equivalent to the value of the variable and a sigma equivalent to its error.
     */
    RooGaussian & GenerateConstraint(RooRealVar & variable);

    /**
     * \brief Creates a correlated gaussian constraint from a list of variables and their covariance matrix.
     * \param listOfVariables - A list of RooRealVar to constaint.
     * \param covarianceMatrix - The covariance matrix of the list of variable to constraint.
     * \return Reference to the multi variable gaussian PDF that constraints the variables.
     * \note Like GenerateConstraint(RooRealVar & variable), the means are the values of RooRealVar inside listOfVariables.
     * \note The order of RooRealVar inside listOfVariables must match their ordering in the covarianceMatrix.
     *       I.e. RooRealVar at index 3 inside listOfVariables must have it's corresponding covariance at row/column 3 inside the covarianceMatrix.
     * \note listOfVariables size must match covarianceMatrix's number of rows.
     */
    RooMultiVarGaussianSuppressWarning & GenerateCorrelatedConstraints(RooArgList & listOfVariables, TMatrixDSym covarianceMatrix);
    /**
     * \brief Creates a correlated gaussian constraint from a list of variables and their covariance matrix, the fitter will not use the overall determinant as likelihood offset
     * \param listOfVariables - A list of RooRealVar to constaint.
     * \param covarianceMatrix - The covariance matrix of the list of variable to constraint.
     * \return Reference to the multi variable gaussian PDF that constraints the variables.
     * \note Like GenerateConstraint(RooRealVar & variable), the means are the values of RooRealVar inside listOfVariables.
     * \note The order of RooRealVar inside listOfVariables must match their ordering in the covarianceMatrix.
     *       I.e. RooRealVar at index 3 inside listOfVariables must have it's corresponding covariance at row/column 3 inside the covarianceMatrix.
     * \note listOfVariables size must match covarianceMatrix's number of rows.
     */    
    RooMultiVarGaussianNoNorm & GenerateCorrelatedConstraintsNoNorm(RooArgList & listOfVariables, TMatrixDSym covarianceMatrix);

    /**
     * \brief Used to change the constraint on variable to a new mean value.
     * \param variable - The RooRealVar whose mean to change.
     * \param newValue - A double which will be the new mean value the variable will constraint to.
     * \note This is used mainly when running multiple toys.
     */
    void SetMeanValue(RooRealVar & variable, double newValue);

    /**
     * \brief Generates a RooArgSet which can be used with RooFit's ExternalConstraints().
     * \return Returns a reference to a RooArgSet containing all the constraint PDFs.
     */
    RooArgSet & GetAllConstraints();

    /**
     * \brief Returns a variable to variable linker to smear the Gaussian means.
     * \return Returns a map linking a variable to the mean it is constrained to.
     */
    map< RooRealVar *, RooRealVar * > GetVariableToMeanMap();

    /**
     * \brief Returns a vector of correlated variables, their constraint means and the covariance.
     * \return Returns a vector of variables, means and their covariance in the correct order.
     */
    vector< CorrelatedConstraintsInfo > GetCorrelatedConstraintsInfos();

    /**
     * \brief Adds a constraint likelihood term to a likelihood object.
     * \param NLL - The likelihood object to add to.
     * \returns A RooAddition which adds the constraint likelihood and original likelihood together.
     */
    RooAddition AddConstraintsToNLL(RooAbsReal & NLL);

    /**
     * \brief Generates a likelihood term from the constraints inside ConstraintGenerator's memory.
     * \param name - TString name of the likelihood term object.
     * \returns A reference to the likelihood object created with all the constraining PDFs.
     */
    RooConstraintSum & GenerateConstraintNLL(TString name);

    /**
     * \brief Used to determine if any constraints has been added.
     * \returns A boolean True if this object contains any constraints. False otherwise.
     */
    bool HasConstraints();

  private:
    RooRealVar &                         CreateMean(RooRealVar & variable);
    RooConstVar &                        CreateSigma(RooRealVar & variable);
    RooArgList &                         CreateListOfMeans(RooArgList & listOfVariables);
    RooGaussian &                        CreateGaussianConstraint(RooRealVar & variable, RooAbsReal & mean, RooConstVar & sigma);
    RooMultiVarGaussianSuppressWarning  & CreateMultiVarGaussianConstraint(      RooArgList & listOfVariables, RooArgList & listOfMeans, TMatrixDSym & covarianceMatrix);
    RooMultiVarGaussianNoNorm           & CreateMultiVarGaussianConstraintNoNorm(RooArgList & listOfVariables, RooArgList & listOfMeans, TMatrixDSym & covarianceMatrix);

    TString                              CombineNamesInList(TString header, RooArgList & listOfVariables);
    void                                 LogCorrelatedConstraintsInfo(const RooArgList& listOfVariables, const RooArgList& listOfMeans, const TMatrixDSym& covarianceMatrix);
    void                                 DeleteConstraintSum();
    void                                 SumAllConstraints(TString name);
    template < typename T > void         ClearCollector(vector< T > & collector);

  private:
    // These private collectors are here to prevent memory leak
    // And also for this class to own the objects
    vector< RooAbsReal * >              m_variableCollector;
    vector< RooAbsPdf * >               m_constraintCollector;
    vector< RooArgList * >              m_argListCollector;
    map< RooRealVar *, RooRealVar * >   m_variableToMeanMap;
    vector< CorrelatedConstraintsInfo > m_correlatedConstraintsInfo;
    RooConstraintSum *                  m_allConstraints = nullptr;
    RooArgSet                           m_listOfConstrainedVariables;
    RooArgSet                           m_listOfConstraints;
};

#endif