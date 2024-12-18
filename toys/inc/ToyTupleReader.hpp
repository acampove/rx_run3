#ifndef TOYTUPLEREADER_HPP
#define TOYTUPLEREADER_HPP

#include "ToyFileHandler.hpp"
#include "ToyTupleComponentReader.hpp"
#include "ToyTupleConfig.hpp"
#include "ToyTupleConfigLoader.hpp"
#include "ToyTupleHeaderHandler.hpp"
#include "ToyYieldConfig.hpp"

#include "RooDataHist.h"
#include "RooDataSet.h"
#include "TString.h"

/**
 * \class ToyTupleReader
 * \brief Reads the toy nTuple for a single category.
 * \brief Owns many ToyTupleComponentReader.
 * \brief The toy nTuple is read by the ToyTupleComponentReader owned. Each component of the data is read by a single instance of ToyTupleComponentReader.
 * \brief ToyTupleReader passes the TFile object and configures them. The final RooDataSet returned is constructed by adding the components read by individual ToyTupleComponentReader.
 */
class ToyTupleReader {
  public:
    /**
     * \brief Default constructor.
     */
    ToyTupleReader();
    /**
     * \brief Copy constructor.
     */
    ToyTupleReader(const ToyTupleReader & _other);
    /**
     * \brief Move constructor.
     */
    ToyTupleReader(ToyTupleReader && _other);
    /**
     * \brief Constructs an instance of ToyTupleReader and configures it with the ConfigHolder and vector of ToyYieldConfig passed.
     */
    ToyTupleReader(const ConfigHolder & configHolder, const vector< ToyYieldConfig > & yieldConfigs);
    /**
     * \brief Destructor.
     */
    ~ToyTupleReader();

    /**
     * \brief Configures the ToyTupleReader.
     * \param configHolder configures the file path to search for ToyTupleConfig and toy nTuples.
     * \param yieldConfigs configures which component to generate and how many of each.
     */
    void Init();
    /**
     * \brief Gets the next toy data.
     * \brief Internally increments the toy index so successive calls to this function will return different data assuming data stored with different indices are unique.
     */
    RooDataSet * NextToyData(uint _index);
    /**
     * \brief Bins the RooDataSet object constructed in the last call to NextToyData()
     */
    RooDataHist * BinCurrentToy();
    /**
     * \brief Changes the current index used in the file path for reading toy nTuple.
     */
    void SetIndex(int index);
    /**
     * \brief      Sets the observable.
     */
    void SetObservable(RooArgSet * _observables);
    /**
     * \brief Equal operator.
     */
    ToyTupleReader & operator=(const ToyTupleReader & _other);
    /**
     * \brief Move equal operator.
     */
    ToyTupleReader & operator=(ToyTupleReader && _other);

  private:
    void                   SetDataObjectsName();
    void                   Print() const;
    void                   ReadToyTupleConfigs();
    void                   EvaluateGeneratorMean();
    const ToyTupleConfig & FindMatchingTupleConfig(const ToyYieldConfig & ToyYieldConfig) const;
    bool                   TupleConfigInMap(TString sampleKey) const;
    void                   TupleConfigNotFoundError(TString yieldConfigSample) const;
    void                   InitialiseToyTupleComponentReaders();
    void                   DeleteOldData(RooAbsData * data);
    void                   GetTupleHeaders(uint _index);
    void                   OpenFile();
    void                   SetupComponentReaders();
    void                   ExtractComponentsFromFile(uint _index);
    void                   CreateDataset();
    TList *                FillTListWithTupleTrees() const;
    void                   ConvertTreeToDataset(TTree * combinedTuple);
    void                   ClearTupleTrees();
    void                   CloseFile();
    void                   CreateBinnedFromDataset();
    void                   DeleteObservable();

  private:
    ConfigHolder m_configHolder;

    ToyFileHandler        m_toyFileHandler;
    ToyTupleConfigLoader  m_tupleConfigLoader;
    ToyTupleHeaderHandler m_tupleHeaderHandler;

    map< TString, ToyTupleConfig >    m_sampleToTupleConfigMap;
    vector< ToyYieldConfig >          m_yieldConfigs;
    vector< ToyTupleComponentReader > m_componentReaders;
    vector< ToyTupleComponentHeader > m_componentHeaders;
    vector< TTree * >                 m_tupleComponentTrees;

    TFile * m_file = nullptr;

    TString m_key = "";

    TString m_datasetName;
    TString m_datahistName;
    RooDataSet  * m_dataset   = nullptr;
    RooDataHist * m_datahist  = nullptr;
    RooArgSet * m_observables = nullptr;
};

#endif
