#ifndef TOYTUPLEGENERATOR_HPP
#define TOYTUPLEGENERATOR_HPP

#include "ToyFileHandler.hpp"
#include "ToyTupleComponentGenerator.hpp"
#include "ToyTupleComponentHeader.hpp"
#include "ToyTupleConfig.hpp"
#include "ToyTupleHeaderHandler.hpp"
#include "ToyYieldConfig.hpp"
#include "RooWorkspace.h"

/**
 * \class ToyTupleGenerator
 * \brief Generates a single toy nTuple.
 * \brief Owns multiple ToyTupleComponentGenerator.
 * \brief Does not generate the individual components (those are done by ToyTupleComponentGenerator) but this class is responsible for passing the file objects and retrieving ToyTupleConfigs.
 */
class ToyTupleGenerator {

  public:
    /**
     * \brief Default constructor.
     */
    ToyTupleGenerator();
    /**
     * \brief Constructs an instance of ToyTupleGenerator and configures it with the ConfigHolder and vector of ToyYieldConfig passed.
     * \brief Configures the ToyTupleGenerator using the argument passed.
     * \param configHolder Configure the file paths to ToyTupleConfig files and toy nTuples.
     * \param yieldConfigs Configure which components are generated and the mean events of those components.
     */
    ToyTupleGenerator(const ConfigHolder & _configHolder, const vector< ToyYieldConfig > & _yieldConfigs);
    /**
     * \brief Copy constructor.
     */
    ToyTupleGenerator(const ToyTupleGenerator & _other);
    /**
     * \brief Move constructor.
     */
    ToyTupleGenerator(const ToyTupleGenerator && _other);
    /**
     * \brief Destructor.
     */
    ~ToyTupleGenerator(){};

    void Init();
    /**
     * \brief Generates a single toy nTuple.
     * \brief Also increments the internal toy index so successive calls to ToyTupleGenerator::Generate() will generate non-overlapping toy nTuples.
     */
    void Generate();
    /**
     * \brief Equal operator.
     */
    ToyTupleGenerator & operator=(const ToyTupleGenerator & _other);
    /**
     * \brief Move equal operator.
     */
    ToyTupleGenerator & operator=(const ToyTupleGenerator && _other);

    void SetToyIndex(uint _index) {m_index = _index; return;};

  private:
    void                   Print() const;
    void                   ReadToyTupleConfigs();
    void                   EvaluateGeneratorMean();
    const ToyTupleConfig & FindMatchingTupleConfig(const ToyYieldConfig & yieldConfig) const;
    bool                   TupleConfigInMap(TString sampleKey) const;
    void                   InitialiseToyComponentGenerators();
    void                   DeleteOldHeaders();
    void                   OverwriteOpen();
    void                   WriteComponentTreesToCurrentTuple();
    void                   UpdateToyTupleHeader();
    void                   WriteToyTupleHeader();
    void                   CloseFile();

  private:
    ConfigHolder m_configHolder;

    ToyFileHandler        m_toyFileHandler;
    ToyTupleHeaderHandler m_tupleHeaderHandler;

    map< TString, ToyTupleConfig >       m_sampleToTupleConfigMap;
    vector< ToyYieldConfig >             m_yieldConfigs;
    vector< ToyTupleComponentGenerator > m_componentGenerators;   // Has same ordering as ToyYieldConfig
    vector< ToyTupleComponentHeader >    m_tupleComponentHeaders;

    TFile * m_file       = nullptr;   // Since this object owns the file, it should also own the trees in the file
    TTree * m_headerTree = nullptr;

    uint m_index = 0;

    vector< TTree * > m_componentTrees;
    RooWorkspace * m_workspace; // The RooWorkspace which owns all the PDFs.
};

#endif
