#ifndef TOYTUPLECOMPONENTGENERATOR_HPP
#define TOYTUPLECOMPONENTGENERATOR_HPP

#include "ToyTupleComponentHeader.hpp"
#include "ToyTupleConfig.hpp"
#include "ToyYieldConfig.hpp"

#include "TRandom3.h"

/** \class ToyTupleComponentGenerator
 * \brief Generates a the TTree for a single component (e.g. Signal or combinatorial background) for toy nTuples.
 * \brief It needs to be configured by passing a ToyTupleConfig and ToyYieldConfig.
 */
class ToyTupleComponentGenerator {
  public:
    /**
     * \brief Default constructor.
     */
    ToyTupleComponentGenerator();
    /**
     * \brief Copy constructor.
     * \note Only the ToyTupleConfig and ToyYieldConfig are copied.
     * Other private members are changed on every iteration of toy generation so their information is not propagated.
     */
    ToyTupleComponentGenerator(const ToyTupleComponentGenerator & other);
    /**
     * \brief Move constructor.
     * \note Only the ToyTupleConfig and ToyYieldConfig are moved.
     * Other private members are changed on every iteration of toy generation so their information is not propagated.
     */
    ToyTupleComponentGenerator(ToyTupleComponentGenerator && other);
    /**
     * \brief Constructs a ToyTupleComponentGenerator and configures it with a ToyTupleConfig and ToyYieldConfig.
     */
    ToyTupleComponentGenerator(const ToyTupleConfig & tupleConfig, const ToyYieldConfig & yieldConfig);
    /**
     * \brief Configures the ToyTupleConfig used for nominal yield information, PDF shape and observables to generate over.
     */
    void SetTupleConfig(const ToyTupleConfig & tupleConfig);
    /**
     * \brief Configures the ToyYieldConfig used for the mean number of events when generating.
     */
    void SetYieldConfig(const ToyYieldConfig & yieldConfig);
    /**
     * \brief Generates a TTree according to the ToyTupleConfig and ToyYieldConfig configured to the target file.
     * \param targetFile The target file to write the TTree.
     */
    void WriteComponentTreeToFile(TFile * targetFile, uint _index = 0, TString _key = "");
    /**
     * \brief Get the header containing information on the last generated TTree.
     * \brief This is attached to the file by ToyTupleHeaderHandler.
     */
    ToyTupleComponentHeader GetLastGeneratedHeader() const;
    /**
     * \brief Equal operator.
     * \note Only the ToyTupleConfig and ToyYieldConfig data are copied.
     * Other private members are changed on every iteration of toy generation so their information is not propagated.
     */
    ToyTupleComponentGenerator & operator=(const ToyTupleComponentGenerator & other);
    /**
     * \brief std::move equal operator.
     * \note Only the ToyTupleConfig and ToyYieldConfig data are moved.
     * Other private members are changed on every iteration of toy generation so their information is not propagated.
     */
    ToyTupleComponentGenerator & operator=(ToyTupleComponentGenerator && other);
    /**
     * \brief Static function which changes the two random number generator seeds.
     * \brief They are the static random number generators:
     * \li RooRandom::randomGenerator()
     * \li ToyTupleComponentGenerator::m_randomNumberGenerator
     */
    static void SetSeed(unsigned long seed);
    /**
     * \brief Destructor.
     */
    ~ToyTupleComponentGenerator();

  public:
    /**
     * \brief The random number generator used to Poisson smear the yields.
     */
    static TRandom3 m_randomNumberGenerator;

  private:
    void        CheckSampleMatches() const;
    bool        BothConfigSet() const;
    void        ThrowIfSamplesDoNotMatch() const;
    void        SetTargetFile(TFile * targetFile);
    void        GenerateDataTree(uint _index, TString _key);
    int         SmearGeneratorMean();
    void        GenerateTupleTree(int nEvents);
    void        WriteTreeToFile(uint _index, TString _key);
    void        UpdateGeneratedEvents(int totalEvents);
    static void PrintNewSeedMessage(unsigned long seed);

  private:
    ToyTupleConfig          m_tupleConfig;
    ToyYieldConfig          m_yieldConfig;
    ToyTupleComponentHeader m_tupleComponentHeader;

    TTree * m_componentTree = nullptr;
    TFile * m_targetFile    = nullptr;
    int     m_lastGeneratedEvents;
};

#endif