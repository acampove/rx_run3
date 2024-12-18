#ifndef TOYTUPLECONFIG_HPP
#define TOYTUPLECONFIG_HPP

#include "RooAbsPdf.h"
#include "RooWorkspace.h"
#include "TString.h"
#include "TTimeStamp.h"

#include "TFile.h"
#include "TTree.h"

/**
 * \class ToyTupleConfig
 * \brief A data type object to configure a single component for toy generation and reading.
 * \brief The data stored internally are:
 * \li \c Sample to bookkeep the component.
 * \li \c PDF A pointer to the PDF for a single component.
 * \li \c Nominal \c yield of this component.
 * \li \c Observable \c Key to retrieve the pointer to the observable from the PDF.
 * \li \c BkgType Background type. (Not used currently)
 * \li \c Description of this component. (Not really used currently)
 * \li \c TimeStamp when this component is produced. (For bookkeeping reasons maybe?)
 */
class ToyTupleConfig {

  public:
    /**
     * \brief Default constructor.
     */
    ToyTupleConfig();
    /**
     * \brief Constructor that sets all the internal data type.
     */
    ToyTupleConfig(const TString & pdfKey, const TString & observableKey, const TString & sample, double nominalYield, const TTimeStamp & timestamp, RooAbsPdf & pdf);

    /**
     * \brief Copy constructor. Compiler generated trivial copy constructor not applicable due to TString pointer data members.
     */
    ToyTupleConfig(const ToyTupleConfig & other);
    /**
     * \brief Move constructor. Compiler generated trivial move constructor not applicable due to TString pointer data members.
     */
    ToyTupleConfig(ToyTupleConfig && other);
    /**
     * \brief Destructor.
     */
    ~ToyTupleConfig();

    void Init();

    /**
     * \brief Attach this instance of ToyTupleConfig to a TTree. Used to read ToyTupleConfig from a suitable nTuple.
     */
    void AttachToTree(TTree * tree);
    /**
     * \brief Create branches from this instance of ToyTupleConfig to a TTree. Used to write ToyTupleConfig to an nTuple.
     */
    void CreateBranches(TTree * tree);
    /**
     * \brief Returns the \c Sample stored.
     */
    TString GetSample() const { return *m_sample; };
    /**
     * \brief Returns the \c nominal \c yield stored.
     */
    double GetNominalYield() const { return m_nominalYield; };
    /**
     * \brief Returns the \c PDF \c pointer stored.
     */
    TString GetObservableKey() const { return *m_observableKey; };
    /**
     * \brief Returns a \c RooArgSet which contains the \c observable.
     */
    RooArgSet * GetObservables();
    /**
     * \brief Returns the \c PDF \c key to get the PDF from a ROOT file.
     */
    TString GetPdfKey() const { return *m_pdfKey; };
    /**
     * \brief Returns true if the internal data members are set correctly.
     */
    RooAbsPdf * GetPdf() const { return m_pdf; };
    /**
     * \brief Returns the \c key \c of the \c observable stored.
     */
    bool IsSet() const;
    /**
     * \brief Prints the stored information of data members.
     */
    void Print() const;
    /**
     * \brief Loads a PDF from a ROOT file and sets the internal PDF pointer to it.
     */
    void LoadPdfFromWorkspace(RooWorkspace * workspace, TString _key = "");
    /**
     * \brief Deletes the PDF pointed to.
     */
    void DeletePdf();
    /**
     * \brief Equal constructor. Compiler generated trivial equal operator not applicable due to TString pointer data members.
     */
    ToyTupleConfig & operator=(const ToyTupleConfig & other);
    /**
     * \brief Move equal operator. Compiler generated trivial move equal operator not applicable due to TString pointer data members.
     */
    ToyTupleConfig & operator=(ToyTupleConfig && other);

  private:
    void GetObservableValues();
    void DeleteObservables();

    // TStrings are pointers so that TTree handling is easier
    TString *    m_sample          = nullptr;
    double       m_nominalYield    = 0.;
    TString *    m_pdfKey          = nullptr;
    TString *    m_observableKey   = nullptr;
    RooAbsPdf *  m_pdf             = nullptr;
    TTimeStamp * m_timestamp       = nullptr;
    TString *    m_observableTitle = nullptr;
    double       m_observableMin   = 0.;
    double       m_observableMax   = 0.;

    RooArgSet * m_observables = nullptr;

    int m_setBranchAddressReturnCode[8];
    const int m_setBranchAddressOkCode = 0;
};

#endif
