#ifndef VARIABLESMEARER_HPP
#define VARIABLESMEARER_HPP

#include "RooRealVar.h"
#include "TRandom3.h"

using namespace std;

/**
 * \enum smearDistribution
 */
enum class smearDistribution { Gauss, Poisson };

/** \class VariableSmearer
 * \brief A class to returned smeared \e double values of a RooRealVar.
 */
class VariableSmearer {

  public:
    /**
     * \brief The default constructor.
     */
    VariableSmearer(){};
    /**
     * \brief Initialize the smearing using the value and error of a RooRealVar by a smearDistribution.
     * \param variable Reference to a RooRealVar for the value and error of smearing.
     * \param distribution The smearing distribution to apply.
     */
    VariableSmearer(RooRealVar * _variableToSmear, double _meanValue, double _error, smearDistribution distribution = smearDistribution::Gauss);
    /**
     * \brief A static instance of TRandom3 which generates the smeared values returned by SmearedValue().
     */
    ~VariableSmearer(){};

    /**
     * \brief Smears the appropriate variable.
     */
    void SmearVariable();

    /**
     * \brief Returns a smeared double.
     */
    double SmearedValue();
    /**
     * \brief Sets the of the seed in the random number generator instance.
     * \param seed The seed parsed forms of the first 32 bits of the seed, the other 32 bits comes from the variable name hash.
     */
    void ResetSeed(uint32_t _first32Bits);

  private:
    double gaussianSmear();
    double poissonSmear();

  private:
    RooRealVar *      m_variableToSmear;
    smearDistribution m_distribution;
    double            m_meanValue = 0;
    double            m_error     = 0;
    TRandom3          m_randomNumberGenerator;
};

#endif
