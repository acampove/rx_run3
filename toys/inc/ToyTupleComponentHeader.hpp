#ifndef TOYTUPLECOMPONENTHEADER_HPP
#define TOYTUPLECOMPONENTHEADER_HPP

#include "TString.h"
#include "TTree.h"

/**
 * \class ToyTupleComponentHeader
 * \brief A data object which stores information needed to produce the header TTree for toy nTuples.
 * \brief The information stored are:
 * \li \c Sample name of the component
 * \li The \c Generator \c Mean used.
 * \li The \c Number \c of \c Generated \c Events poisson sampled from the generator mean.
 * \li The \c Poisson \c CDF used when reading a subset of the generated events.
 */

using namespace std;

class ToyTupleComponentHeader {
  public:
    /**
     * \brief Default constructor.
     */
    ToyTupleComponentHeader();
    /**
     * \brief Copy constructor. Compiler generated trivial copy constructor not applicable due to TString pointer.
     */
    ToyTupleComponentHeader(const ToyTupleComponentHeader & other);
    /**
     * \brief Move constructor. Compiler generated trivial move constructor not applicable due to TString pointer.
     */
    ToyTupleComponentHeader(const ToyTupleComponentHeader && other);
    /**
     * \brief Constructs a ToyTupleComponentHeader and sets its \c sample.
     */
    ToyTupleComponentHeader(TString sample);
    /**
     * \brief Constructs a ToyTupleComponentHeader and sets all it's internal member.
     * \brief The Poisson CDF is automatically calculated using \c generatorMean and \c generatedEvents.
     */
    ToyTupleComponentHeader(TString sample, int generatorMean, int generatedEvents);
    /**
     * \brief Returns true if all four data members are set.
     */
    bool HeaderExists() const { return m_headerExistFlag; }
    /**
     * \brief Returns the sample TString.
     */
    TString GetSample() const { return *m_sample; }
    /**
     * \brief Returns the stored \c Generator \c Mean.
     */
    int GetGeneratorMean() const { return m_generatorMean; }
    /**
     * \brief Returns the stored \c Generated \c Events.
     */
    int GetGeneratedEvents() const { return m_generatedEvents; }
    /**
     * \brief Returns the stored \c Poisson \c CDF.
     */
    double GetPoissonCDF() const { return m_poissonCDF; }
    /**
     * \brief Attach this instance of ToyTupleComponentHeader to a TTree for reading.
     */
    void AttachToTree(TTree * headerTree);
    /**
     * \brief Creates branches from this instance of ToyTupleComponentHeader data member to a TTree for writing.
     */
    void CreateBranches(TTree * headerTree);
    /**
     * \brief Equal operator. Compiler generated trivial equal operator not applicable due to TString pointer.
     */
    ToyTupleComponentHeader & operator=(const ToyTupleComponentHeader & other);
    /**
     * \brief Move equal operator. Compiler generated trivial move equal operator not applicable due to TString pointer.
     */
    ToyTupleComponentHeader & operator=(const ToyTupleComponentHeader && other);
    /**
     * \brief Destructor.
     */
    ~ToyTupleComponentHeader() { delete m_sample; }

  private:
    void EvaluatePoissonCDF();

  private:
    TString * m_sample = nullptr;
    int       m_generatorMean;
    int       m_generatedEvents;
    double    m_poissonCDF;
    bool      m_headerExistFlag;

    const char * m_sampleBranchName          = "sample";
    const char * m_generatorMeanBranchName   = "generatorMean";
    const char * m_generatedEventsBranchName = "generatedEvents";
    const char * m_poissonCDFBranchName      = "poissonCDF";
};

#endif