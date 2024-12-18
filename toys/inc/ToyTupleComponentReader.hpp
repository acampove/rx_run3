#ifndef TOYTUPLECOMPONENTREADER_HPP
#define TOYTUPLECOMPONENTREADER_HPP

#include "ToyTupleComponentHeader.hpp"
#include "ToyYieldConfig.hpp"
#include "TRandom3.h"

/**
 * \class ToyTupleComponentReader
 * \brief Reads a single component from toy nTuples.
 * \brief The component \c sample and \c mean \c yield is configured via ToyYieldConfig.
 */
class ToyTupleComponentReader {
  public:
    /**
     * \brief Default constructor.
     */
    ToyTupleComponentReader();
    /**
     * \brief Copy constructor. Only the \c ToyYieldConfig and \c m_componentTreeName is copied.
     * \brief Other private members can change over the toy loop.
     */
    ToyTupleComponentReader(const ToyTupleComponentReader & other);
    /**
     * \brief Move constructor. Only the internal \c ToyYieldConfig and \c m_componentTreeName is moved.
     * \brief Other private members can change over the toy loop.
     */
    ToyTupleComponentReader(ToyTupleComponentReader && other);
    /**
     * \brief Constructs a ToyTupleComponentReader and configures it's ToyYielcConfig.
     */
    ToyTupleComponentReader(const ToyYieldConfig & yieldConfig);
    /**
     * \brief Configures the internal ToyYieldConfig.
     */
    void SetYieldConfig(const ToyYieldConfig & yieldConfig);
    /**
     * \brief Sets the ToyTupleComponentReader to read from a source file.
     */
    void SetSourceFile(TFile * sourceFile);
    /**
     * \brief Configures the header. This is header changes on every iteration of toy loop.
     */
    void SetHeader(const ToyTupleComponentHeader & componentHeader);
    /**
     * \brief Equal operator. Only the \c ToyYieldConfig and \c m_componentTreeName information is copied.
     * \brief Other private members can change over the toy loop.
     */
    ToyTupleComponentReader & operator=(const ToyTupleComponentReader & other);
    /**
     * \brief Move equal operator. Only the \c ToyYieldConfig and \c m_componentTreeName information is moved.
     * \brief Other private members can change over the toy loop.
     */
    ToyTupleComponentReader & operator=(ToyTupleComponentReader && other);
    /**
     * \brief Gets a component tree from the target file.
     * \brief The mean number of events to read is configured by the ToyYieldConfig.
     */
    TTree * GetComponentTreeFromFile(uint _index = 0, TString _key = "");
    /**
     * \brief Destructor.
     */
    ~ToyTupleComponentReader();

    /**
     * \brief Static function which changes the random number generator seeds.
     * \li ToyTupleComponentGenerator::m_randomNumberGenerator
     */
    static void SetSeed(unsigned long seed);
    static TRandom3 m_randomNumberGenerator;
    static void PrintNewSeedMessage(unsigned long seed);    
  private:
    void   ThrowIfHeaderAndConfigIncompatible() const;
    void   CheckTupleAndHeaderMean();
    void   RequestedMeanMoreThanGeneratorMeanError() const;
    int    GetNumberOfEventsToClone() const;
    double CalculateReducedMeanInversePoissonCDF() const;
    void   ReadComponentTreeFromFile(uint _index, TString _key);
    void   CloneComponentTreeFromFile(int nEventsToClone , TString _key );

  private:
    enum class ReadStrategy { ReadAll, ReadReduced };
    ReadStrategy            m_readStrategy;
    ToyYieldConfig          m_yieldConfig;
    TString                 m_componentTreeName;
    TFile *                 m_sourceFile = nullptr;
    ToyTupleComponentHeader m_componentHeader;
    TTree *                 m_componentTree = nullptr;
    TTree *                 m_clonedTree    = nullptr;
};

#endif