#ifndef FITHOLDER_HPP
#define FITHOLDER_HPP

#include "FitterSvc.hpp"
#include "HelperSvc.hpp"
#include "MessageSvc.hpp"

#include "FitComponent.hpp"
#include "FitConfiguration.hpp"

#include <algorithm>
#include <iostream>
#include <map>
#include <vector>

#include "TMatrixDSym.h"
#include "TObject.h"
#include "TString.h"

#include "RooAbsPdf.h"
#include "RooAbsReal.h"
#include "RooAddPdf.h"
#include "RooArgList.h"
#include "RooRealVar.h"

class FitterTool;

/**
 * \class FitComponentAndYield
 * \brief FitComponent and Yield
 */
class FitComponentAndYield : public TObject {
  public:
    FitComponentAndYield() = default;
    FitComponentAndYield(const FitComponentAndYield & _other) {
        fitComponent = _other.fitComponent;
        yield        = _other.yield;
    };
    FitComponent fitComponent;      // The FitComponent object  [save and read back]
    RooAbsReal * yield = nullptr;   // The yield assigned to the fit component [save and read back]
    ClassDef(FitComponentAndYield, 1);
};

/**
 * \class FitHolder
 * \brief FitHolder is a container object with extra setting and private members representing a single Fit to data for a category .
          1 FitHolder holds 1 model (m_model) which is created as a sum of PDFs from the map of FitComponentAndYield.
          map< TString, FitComponentAndYield > holds by KEY the components of the current fit
          For example FitHolder = fit to data for JPsMM in L0L category for 2011 datasample.
          m_bkgComponents are the components marked as background shapes
          m_bkgComponents = <(Lb Pdf, yield Lb)>
                            <(Comb Pdf, yield Comb)>
                            <(Bs, yields Bs)>

          m_data is the FitComponent for the signal shape. FitComponent also host the dataset associated to the datasample to fit.

          m_model is the full model of the sum of signal shape + background shape(s)

          m_fitter is used only to fit the instance of a single FitHolder. Useful if you want to isolate one category to study and do the fit only for that

          m_binned
 */
class FitHolder : public TObject {

  public:
    FitComponentAndYield m_sigComponent;   // Signal Component fit

    Str2ComponentMap m_bkgComponents;   // the background components

    FitComponent m_data;   // Data Fit Component for the fit

    RooAbsPdf * m_model = nullptr; // Full model build summing the fit components using the yields
    //RooRealSumPdf * m_model = nullptr;

    FitterTool * m_fitter = nullptr;   //! Do not strem to disk [this is the fitter tool instance used for a stand-alone fit to the FitHolder]

  private:
    TString m_name;

    bool m_debug = false;

    TString m_option;

    FitConfiguration m_configuration;

    TString m_sigComponentKey = "";   // The signal component Key, used to easy access from FitManager

    shared_ptr< FitParameterPool > m_parameterPool;       //! Global object not saved to disk.
    FitParameterSnapshot           m_parameterSnapshot;   // A snapshot of parameters used by this FitManager. Reloads the parameters to FitParameterPool.

    bool m_isInitialized = false;

    bool m_isLoaded = false;

    bool m_isReduced = false;

  public:
    /**
     * \brief Default constructor
     */
    FitHolder() = default;

    /**
     * \brief Constructor with TString and FitConfiguration
     */
    FitHolder(TString _name, FitConfiguration _configuration, TString _option);

    /**
     * \brief Constructor with TString and RooRealVar
     */
    FitHolder(TString _name, RooRealVar * _var, TString _option);

    /**
     * \brief Copy constructor
     */
    FitHolder(const FitHolder & _fitHolder);

    /**
     * \brief Constructor with LoadFromDisk
     */
    FitHolder(TString _holderName, TString _option, TString _name, TString _dir);

    /**
     * \brief Equality checkers
     */
    bool operator==(const FitHolder & _fitHolder) const { return (Name() == _fitHolder.Name() && Option() == _fitHolder.Option()); };

    bool operator!=(const FitHolder & _fitHolder) const { return !((*this) == _fitHolder); };

    /**
     * \brief Operator access to the FitComponents of the FitHolder
     * @param  _keyComponent [description]
     */
    FitComponent & operator[](const TString & _keyComponent);

    /**
     * \brief Fit name
     */
    const TString Name() const { return m_name; };

    void SetName(TString _name) {
        m_name = _name;
        return;
    };

    /**
     * \brief Fit configuration
     */
    FitConfiguration Configuration() const { return m_configuration; };

    /**
     * \brief Fit option
     */
    const TString Option() const { return m_option; };

    /**
     * \brief Retrieve the FitterTool object used to fit
     */
    FitterTool * Fitter() const { return m_fitter; };
    FitterTool * Fitter() { return m_fitter; };

    /**
     * \brief Retrieve the Data of the fit
     */
    FitComponent   Data() const { return m_data; };
    FitComponent & Data() { return m_data; };

    /**
     * \brief Get the DataSet of the fit
     */
    RooDataSet * GetDataSet() const { return m_data.DataSet(); };
    RooDataSet * GetDataSet() { return m_data.DataSet(); };

    /**
     * \brief Get the Model of the fit
     */
    RooAbsPdf * GetModel() const { return m_model; };
    RooAbsPdf * GetModel() { return m_model; };

    /**
     * \brief Get the Signal PDF of the fit
     */
    RooAbsPdf * GetSig() const { return (RooAbsPdf *) m_sigComponent.fitComponent.PDF(); };
    RooAbsPdf * GetSig() { return (RooAbsPdf *) m_sigComponent.fitComponent.PDF(); };

    /**
     * \brief Check wheter a component is present or not
     */
    bool HasComponent(const TString _component) const noexcept {
        if (m_bkgComponents.find(_component) != m_bkgComponents.end()) return true;
        if (_component == m_sigComponentKey) return true;
        return false;
    };

    /**
     * \brief Print parameter map
     */
    void PrintModelParameters() const noexcept;

    /**
     * \brief Set model parameters constant
     */
    void SetConstantModelAllPars();

    /**
     * \brief Set model parameters constant except
     * @param  _names [description]
     */
    void SetConstantModelExceptPars(vector< string > _names);

    /**
     * \brief Get signal FitComponent (allow modifications inline of private member)
     */
    FitComponentAndYield & SignalComponent() { return m_sigComponent; };

    /**
     * \brief Get signal FitComponent (not allow modifications)
     */
    FitComponentAndYield SignalComponent() const { return m_sigComponent; };

    /**
     * \brief signal FitComponent
     */
    TString SignalComponentKey() const { return m_sigComponentKey; };

    /**
     * \brief Create signal FitComponent and Yield
     * @param  _eventType       EventType used to uniquely identify the MC sample to fit
     * @param  _key    ComponentKey assigned to the generated FitComponent
     * @param  _pdf    ComponentPDF parsed to generate the PDF shape
     * @param  _option ComponentOption
     *                          "RooKeysPDF" : Will make given sample the Pdf as RooKeysPDF
     *                          "RooAddPdf"  : Used only for copying around FitCompoents when merged
     *                          "StringToPDF": Use the String2Pdf mechanism to generate PDFs))
     */
    void CreateSignal(const EventType & _eventType, TString _key, TString _pdf, TString _option);

    /**
     * \brief Create signal FitComponent and Yield
     * @param  _eventType       EventType used to uniquely identify the MC sample to fit
     * @param  _key    ComponentKey assigned to the generated FitComponent
     * @param  _pdf             RooAddPdf already known and it's streamed to save the SignalComponent of this fit Holder
     * @param  _option ComponentOption
     *                          "RooKeysPDF" : Will make given sample the Pdf as RooKeysPDF
     *                          "RooAddPdf"  : Used only for copying around FitCompoents when merged
     *                          "StringToPDF": Use the String2Pdf mechanism to generate PDFs))
     */
    void CreateSignal(const EventType & _eventType, TString _key, RooAddPdf * _pdf, TString _option);

    void CreateSignal(const FitComponent & _component);

    /**
     * \brief Set signal Yield
     * @param  _var Pre-existing RooAbsReal, RooRealVar *yield = new...
     */
    void SetSignalYield(RooAbsReal * _var);

    /**
     * \brief Get signal Yield (apply in-line modification fitholder.SignalYield() = new RooRealVar)
     */
    RooAbsReal * SignalYield() { return m_sigComponent.yield; };
    RooAbsReal * SignalYield() const { return m_sigComponent.yield; };

    /**
     * \brief Print signal FitComponent
     */
    void PrintSignalComponent() const noexcept;

    /**
     * \brief Print background FitComponent
     */
    void PrintBackgroundComponents() const noexcept;

    /**
     * \brief Print data FitComponent
     */
    void PrintDataComponent() const noexcept;

    /**
     * \brief Print FitComponent
     */
    void PrintComponents() const noexcept;

    /**
     * \brief Get background FitComponent
     */
    Str2ComponentMap BackgroundComponents() const { return m_bkgComponents; };

    /**
     * \brief Get background FitComponent (no modifications allowed)
     */
    Str2ComponentMap & BackgroundComponents() { return m_bkgComponents; };

    /**
     * \brief Get background FitComponent and allow modifications inline of private member
     */
    FitComponentAndYield & BackgroundComponent(TString _key) { return m_bkgComponents[_key]; };

    /**
     * \brief Create background FitComponent and Yield
     * @param  _eventType       EventType used to uniquely identify the MC sample to fit
     * @param  _key    ComponentKey assigned to the generated FitComponent
     * @param  _pdf    ComponentPDF parsed to generate the PDF shape
     * @param  _option ComponentOption
     *                          "RooKeysPDF" : Will make given sample the Pdf as RooKeysPDF
     *                          "RooAddPdf"  : Used only for copying around FitCompoents when merged
     *                          "StringToPDF": Use the String2Pdf mechanism to generate PDFs))
     */
    void CreateBackground(const EventType & _eventType, TString _key, TString _pdf, TString _option);

    /**
     * \brief Create background FitComponent and Yield
     * @param  _eventType       EventType used to uniquely identify the MC sample to fit
     * @param  _key    ComponentKey assigned to the generated FitComponent
     * @param  _pdf             RooAddPdf already known and it's streamed to save the SignalComponent of this fit Holder
     * @param  _option ComponentOption
     *                          "RooKeysPDF" : Will make given sample the Pdf as RooKeysPDF
     *                          "RooAddPdf"  : Used only for copying around FitCompoents when merged
     *                          "StringToPDF": Use the String2Pdf mechanism to generate PDFs))
     */
    void CreateBackground(const EventType & _eventType, TString _key, RooAddPdf * _pdf, TString _option);

    /**
     * \brief Create background FitComponent (from signal FitComponent) and Yield
     * @param  _eventType       EventType used to uniquely identify the MC sample to fit
     * @param  _key    ComponentKey assigned to the generated FitComponent
     *
     * Generate according to the Signal shape already defined, a new background PDF stored with a different componentKey. Used to dynamically generate the Bs shape as a plain copy of the B0 one. All PDF modifications to define the mass shift for the Bs is done afterwards. This method also generate a backgroun yield
     */
    void CreateBackgroundFromSignal(const EventType & _eventType, TString _key);

    void CreateBackground(const FitComponent & _component);

    /**
     * \brief Set background Yield passing the key of the component
     * @param  _key [description]
     * @param  _var          [description]
     */
    void SetBackgroundYield(TString _key, RooAbsReal * _var);

    /**
     * \brief Get background Yield
     * @param  _key [description]
     */
    RooAbsReal * BackgroundYield(TString _key);

    void FixBackgroundYield(TString _key, double _value);

    /**
     * \brief Check if signal FitComponent
     */
    bool IsSigKey() const noexcept;

    /**
     * \brief Check if in background FitComponent map
     * @param  _key [description]
     */
    bool IsInBkgComponentMap(TString _key) const noexcept;

    /**
     * \brief Create data FitComponent
     * @param  _eventType       EventType used to uniquely identify the MC sample to fit
     * @param  _key    ComponentKey assigned to the generated FitComponent
     * @param  _option ComponentOption
     *
     * Generate according to the EventType and component key the m_data FitComponent which is a FitComponent holding the fit to Data. This method also generate the dataset to fit for (i.e. the dataset for the fit to data)
     */
    void CreateData(const EventType & _eventType, TString _key, TString _option);

    void CreateData(const FitComponent & _component);


    /**
     * \brief Create/Build fit model  (assumes all pieces exists)
     */
    void BuildModel();

    /**
     * \brief Create fit model
     * @param  _pdfs   [description]
     * @param  _yields [description]
     */
    void CreateModel(const RooArgList & _pdfs, const RooArgList & _yields);

    /**
     * \brief Signal name
     * @param  _key [description]
     */
    TString SignalName() const noexcept { return m_sigComponent.fitComponent.Name(); };

    /**
     * \brief Background name
     * @param  _key [description]
     */
    TString BackgroundName(TString _key) const noexcept;

    /**
     * \brief Pointer to the signal PDF
     * @param  _key [description]
     */
    RooAbsPdf * SignalPDF() { return m_sigComponent.fitComponent.PDF(); };

    /**
     * \brief Pointer to the background PDF
     * @param  _key [description]
     */
    RooAbsPdf * BackgroundPDF(TString _key);

    /**
     * \brief Print Yields of the model composition
     */
    void PrintYields() const noexcept;

    /**
     * \brief      Initialize Signal and Background Yields
     * @param[in]  _signalYields      initialize signal yields ranges?
     * @param[in]  _backgroundYields  initialize background yields ranges ?
     */
    void InitRanges(bool _signalYields = true, bool _backgroundYields = true);

    /**
     * \brief Init
     */
    void Init();

    bool IsInitialized() const { return m_isInitialized; };

    bool IsLoaded() const { return m_isLoaded; };

    bool IsReduced() const { return m_isReduced; };

    void SetStatus(bool _isLoaded, bool _isReduced);

    void RefreshParameterPool();

    void CheckDataSize(Long64_t _entries = 0, bool _error = false);

    /**
     * \brief Close
     */
    void Close();

    /**
     * \brief Create FitterTool
     */
    void CreateFitter();

    /**
     * \brief Fit
     */
    void Fit();

    /**
     * \brief Get yield in specified range of the full fit model
     * @param  _sample [description]
     * @param  _min    [description]
     * @param  _max    [description]
     */
    map< TString, pair< double, double > > GetIntegrals(double min, double max);
    pair< double, double >                 GetIntegral(TString _key, double _min, double _max);
    pair< double, double >                 GetIntegralSignal(double _min, double _max);
    pair< double, double >                 GetIntegralBackgrounds(double _min, double _max);

    /**
     * \brief Do SPlot
     */
    void DoSPlot();

    /**
     * \brief Print keys and values of each fit component for the background and signal map of FitComponents
     */
    void PrintKeys() const noexcept;

    /**
     * \brief KeyMap
     */
    vector< TString > KeyMap() const noexcept {
        vector< TString > _keys;
        if (m_sigComponent.fitComponent.PDF() != nullptr) { _keys.push_back(m_sigComponentKey); }
        for (auto & _component : m_bkgComponents) { _keys.push_back(_component.first); }
        return _keys;
    };

    /**
     * \brief Save FitHolder to disk
     */
    void SaveToDisk(TString _name = "", bool _verbose = false);

    /**
     * \brief Load FitHolder from disk
     */
    void LoadFromDisk(TString _name = "", TString _dir = "");

    void ReduceComponents(TCut _cut);

    /**
     * \brief Save FitHolder to log
     */
    void SaveToLog(TString _name = "");

    bool ContainsBackground(const Sample & _sample) const noexcept {
        for (const auto & _fc : m_bkgComponents) {
            if (hash_sample(_fc.first) == _sample) return true;
        }
        return false;
    };

  private:
    /**
     * \brief Check allowed arguments
     */
    bool Check();

    /**
     * \brief Activate debug
     * @param  _debug [description]
     */
    void SetDebug(bool _debug) { m_debug = _debug; };

    /**
     * \brief Create Yield
     * @param  _key  [description]
     * @param  _componentType [description]
     */
    RooAbsReal * CreateYield(TString _key, TString _componentType = "");

    ClassDef(FitHolder, 1);
};

ostream & operator<<(ostream & os, const FitHolder & _fitHolder);

#endif
