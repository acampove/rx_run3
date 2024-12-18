#ifndef TOYGENERATOR_HPP
#define TOYGENERATOR_HPP

#include "FitConfiguration.hpp"

#include "ToyTupleGenerator.hpp"

/**
 * \class ToyGenerator
 * \brief Constructs and owns many ToyTupleGenerator.
 */
class ToyGenerator {

  public:
    /**
     * \brief Default constructor.
     */
    ToyGenerator(){};
    /**
     * \brief Copy constructor.
     */
    ToyGenerator(const ToyGenerator & _other);
    /**
     * \brief Move constructor.
     */
    ToyGenerator(const ToyGenerator && _other);
    /**
     * \brief Destructor.
     */
    ~ToyGenerator(){};

    /**
     * \brief Construct one ToyTupleGenerator for each FitConfiguration
     */
    void GetTupleGenerators();
    /**
     * \brief Construct one ToyTupleGenerator for each FitConfiguration
     */
    void GetTupleGenerators(vector< FitConfiguration > _configurations);
    /**
     * \brief Generate one toy nTuple per ToyTupleGenerator.
     * \brief The ToyTupleGenerator must be constructed by GetTupleGenerators().
     */
    void Generate(uint _index = 0);
    /**
     * \brief Sets the seed used for generation.
     */
    void SetSeed(unsigned long _seed);

  private:
    map< ConfigHolder, map<TString, double> > ParseConfigurationOverride( TString _overrideFiles );
    vector< ToyTupleGenerator > m_tupleGenerators;
};

#endif