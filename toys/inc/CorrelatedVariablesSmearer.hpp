#ifndef CORRELATEDVARIABLESSMEARER_HPP
#define CORRELATEDVARIABLESSMEARER_HPP

#include "RooRealVar.h"
#include "TRandom3.h"
#include "TVectorD.h"
#include "TMatrixD.h"
#include "TMatrixDSym.h"

#include <vector>

using namespace std;

/** \class CorrelatedVariablesSmearer
 * \brief A class to returned smeared \e double values of a RooRealVar.
 */
class CorrelatedVariablesSmearer {

  public:
    /**
     * \brief The default constructor.
     */
    CorrelatedVariablesSmearer(){};
    /**
     * \brief Initialize the smearing using the value and error of a RooRealVar by a smearDistribution.
     * \param variable Reference to a RooRealVar for the value and error of smearing.
     * \param distribution The smearing distribution to apply.
     */
    CorrelatedVariablesSmearer(vector <RooRealVar *> _variablesToSmear, vector <double> _meanValues, TMatrixDSym& _covarianceMatrix);

    /**
     * \brief Smears the appropriate variable.
     */
    void SmearVariables();

    /**
     * \brief Sets the of the seed in the random number generator instance.
     * \param seed The seed parsed forms of the first 32 bits of the seed, the other 32 bits comes from the variable name hash.
     */
    void ResetSeed(uint32_t first32Bits);

  private:
    TVectorD GenerateCorrelatedMeans();
    TVectorD GenerateUncorrelatedGaussians();
    void SetMeanValues(const TVectorD& _correlatedVariables);

  private:
    vector <RooRealVar *> m_variablesToSmear;
    TVectorD m_meanValues;
    TMatrixD m_choleskyDecomposition;
    TRandom3 m_randomNumberGenerator;
    int m_nVariables;
};

#endif
