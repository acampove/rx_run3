#ifndef TOYTUPLEHEADERHANDLER_HPP
#define TOYTUPLEHEADERHANDLER_HPP

// TODO : using map to cleanup tuple searches

#include "ToyTupleComponentHeader.hpp"
#include "ToyYieldConfig.hpp"

#include "TFile.h"
#include "TTree.h"
#include <vector>

/**
 * \class ToyTupleHeaderHandler
 * \brief Handles the ToyTupleComponentHeader loading and saving from toy nTuples.
 */
class ToyTupleHeaderHandler {
  public:
    /**
     * \brief Default constructor.
     */
    ToyTupleHeaderHandler();
    /**
     * \brief Copy constructor.
     */
    ToyTupleHeaderHandler(const ToyTupleHeaderHandler & other);
    /**
     * \brief Destructor.
     */
    ~ToyTupleHeaderHandler(){};

    /**
     * \brief Sets this instance of ToyTupleHeaderHandler to read from and write to the TFile passed.
     */
    void SetToyTupleFile(TFile * toyTupleFile);
    /**
     * \brief Gets the header relevant.
     * \brief The vector of ToyYieldConfig will tell the ToyTupleHeaderHandler which ToyTupleComponentHeader we are interested in.
     * \brief Returns an entry vector if no header is found.
     */
    std::vector< ToyTupleComponentHeader > GetHeaders(const std::vector< ToyYieldConfig > & ToyYieldConfigs, uint _index = 0, TString _key = "");
    /**
     * \brief Writes the vector of header passed in the argument the toy nTuple file last passed by call to SetToyTupleFile().
     * \brief Overwrites everything previously stored in the header.
     * \brief This is because ROOT does not allow removing/modifying entries of TTree once they are written.
     */
    void WriteHeaders(const std::vector< ToyTupleComponentHeader > & finalHeaders, uint _index = 0, TString _key = "");

  private:
    void                    ThrowIfFileNotGiven() const;
    void                    DeleteOldHeaders();
    std::vector< TString >  GetRequestedSampleKeys(const std::vector< ToyYieldConfig > & yieldConfigs) const;
    void                    ExtractHeadersFromFile(uint _index, TString _key);
    void                    GetHeaderTreeFromFile(uint _index, TString _key);
    bool                    HeaderTreeExists() const;
    bool                    HeaderTreeHasEntry() const;
    void                    ExtractHeadersFromHeaderTree();
    void                    GetRequestedComponentHeaders(const std::vector< TString > & requestedSampleKeys);
    ToyTupleComponentHeader RetrieveInitialHeader(const TString & sampleKey) const;
    bool                    InitialHeadersContains(const TString & sampleKey) const;
    ToyTupleComponentHeader GetInitialComponentHeader(const TString & sampleKey) const;
    ToyTupleComponentHeader CreateEmptyHeader(const TString & sampleKey) const;
    void                    GetNonRequestedHeaders(const std::vector< TString > & requestedSampleKeys);
    bool                    HeaderNotRequested(const TString & headerKey, const std::vector< TString > & requestedSampleKeys) const;
    void                    FillUpdatedHeaders(const std::vector< ToyTupleComponentHeader > & finalHeaders);
    void                    SortUpdatedHeadersBySample();
    void                    DeleteHeaderTreeFromFile();
    void                    CreateNewHeaderTree(uint _index, TString _key);
    void                    FillHeaderTree();
    void                    WriteUpdatedHeader();

  private:
    std::map< TString, ToyTupleComponentHeader > m_initialComponentHeadersMap;
    std::vector< ToyTupleComponentHeader >       m_requestedComponentHeaders;
    std::vector< ToyTupleComponentHeader >       m_nonRequestedComponentHeaders;
    std::vector< ToyTupleComponentHeader >       m_updatedComponentHeaders;

    TTree *      m_headerTree              = nullptr;
    TFile *      m_toyTupleFile            = nullptr;
    const char * m_headerTreeName          = "ToyTupleHeader";
    const char * m_headerTreeNameWithCycle = "ToyTupleHeader;*";
};

#endif