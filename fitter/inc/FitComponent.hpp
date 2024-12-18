#ifndef FITCOMPONENT_HPP
#define FITCOMPONENT_HPP

#include "EfficiencySvc.hpp"
#include "FitterSvc.hpp"
#include "HelperSvc.hpp"
#include "MessageSvc.hpp"

#include "EventType.hpp"
#include "FitParameterPool.hpp"

#include <iostream>
#include <vector>

#include "TCanvas.h"
#include "TF1.h"
#include "TFile.h"
#include "TLegend.h"
#include "TObject.h"
#include "TString.h"

#include "RooAbsData.h"
#include "RooAbsPdf.h"
#include "RooAbsReal.h"
#include "RooDataHist.h"
#include "RooDataSet.h"
#include "RooFFTConvPdf.h"
#include "RooFitResult.h"
#include "RooFormulaVar.h"
#include "RooHistPdf.h"
#include "RooKeysPdf.h"
#include "RooNDKeysPdf2.h"
#include "RooNumConvPdf.h"
#include "RooPlot.h"
#include "RooRealVar.h"
#include "RooWorkspace.h"

#include "Math/IntegratorOptions.h"

/**
 * \brief Stream to disk FitComponent data
 */
// #define STREAMDATA (to use for Reduction of ntuples when optimizing MVA? )

class FitterTool;

/**
 * \class FitComponent
 * \brief Fit info
 */
class FitComponent : public TObject {

  private:
    /// EventType used to generarote a sample for a MC fit or a RooKeysPDF, for Combinatorial etc, no eventtype will be used
    EventType m_eventType;   // The event Type of the FitComponent

    /// Name assigned to the FitComponent
    TString m_name = "";
    /// Type of the PDF to be generated
    PdfType m_type = PdfType::Empty;
    /// Option to generate the fit components, parse to the string2pdf
    TString m_option = "";
    /// Path to the dataset cache
    TString m_datasetCache = "";
    /// Usually the B mass in the tuple
    RooRealVar * m_var = nullptr;   // Observable linked to pdf

    /// PDF associated to the FitComponent
    shared_ptr < RooAbsPdf > m_pdf;   //! PDF of this FitComponent

    /// Shape parameter map holding the pdf RooRealVars [ m_pdf and this map must always be alligned]
    Str2VarMap m_shapeParameters;

    shared_ptr< FitParameterPool > m_parameterPool;       //! Global object not saved to disk.
    FitParameterSnapshot           m_parameterSnapshot;   // A snapshot of parameters used by this FitManager. Reloads the parameters to FitParameterPool.

    /// The dataset of the fit component
    // Using shared_ptr now, STREAMDATA is disabled
// #ifdef STREAMDATA
//     RooDataSet *  m_data = nullptr;   // Stream to disk
//     RooDataHist * m_hist = nullptr;   // Stream to disk
// #else
    shared_ptr < RooDataSet > m_data;  //! Stream to disk
    shared_ptr < RooDataHist > m_hist; //! Stream to disk
// #endif

    /// Binned fit
    bool m_binned = true;

    /// flags on when the current pdf has been modified and all parameters have been set to const
    bool m_isModified = false;

    bool m_isLoaded = false;

    bool m_isReduced = false;

    double m_ratioReduced = 1;

    bool m_isAdded = false;

    FitterTool * m_fitter = nullptr;   //!  Do not stream to disk

    bool m_saveAllExt = false;

  public:
    /**
     * \brief Default constructor
     */
    FitComponent() = default;

    /**
     * \brief Constructor with EventType and TString
     */
    FitComponent(const EventType & _eventType, TString _name, TString _pdf, RooRealVar * _var, TString _option);

    /**
     * \brief Constructor with EventType and PdfType
     */
    FitComponent(const EventType & _eventType, TString _name, PdfType _pdf, RooRealVar * _var, TString _option);

    /**
     * \brief Constructor with EventType and RooAbsPdf
     */
    FitComponent(const EventType & _eventType, TString _name, RooAbsPdf * _pdf, RooRealVar * _var, TString _option);

    /**
     * \brief Constructor with EventType and RooAbsData and RooAbsPdf
     */
    FitComponent(const EventType & _eventType, TString _name, RooAbsData * _data, TString _pdf, RooRealVar * _var, TString _option);

    /**
     * \brief Constructor with EventType and RooAbsData
     */
    FitComponent(const EventType & _eventType, TString _name, RooAbsData * _data, PdfType _pdf, RooRealVar * _var, TString _option);

    /**
     * \brief Constructor with EventType and RooAbsData and RooAbsPdf
     */
    FitComponent(const EventType & _eventType, TString _name, RooAbsData * _data, RooAbsPdf * _pdf, RooRealVar * _var, TString _option);

    /**
     * \brief Copy constructor
     */
    FitComponent(const FitComponent & _fitComponent);

    /**
     * \brief Constructor with LoadFromDisk
     */
    FitComponent(TString _componentName, TString _option, TString _name, TString _dir);

    /**
     * \brief Equality checkers
     */
    bool operator==(const FitComponent & _fitComponent) const { return (GetEventType() == _fitComponent.GetEventType() && Name() == _fitComponent.Name() && Option() == _fitComponent.Option() && Type() == _fitComponent.Type()); };

    bool operator!=(const FitComponent & _fitComponent) const { return !((*this) == _fitComponent); };

    /**
     * \brief Operator access to the parameters of the FitComponent
     * @param  _keyPar [description]
     */
    RooAbsReal * operator[](const TString & _keyPar);

    /**
     * \brief Get FitComponent EventType
     */
    const EventType & GetEventType() const { return m_eventType; };
    EventType &       GetEventType() { return m_eventType; };

    /**
     * \brief Set FitComponent EventType (for FitHolder::CreateBackgroundFromSignal)
     */
    void SetEventType(const EventType & _eventType) {
        m_eventType = _eventType;
        return;
    };

    /**
     * \brief Get FitComponent name
     */
    TString Name() const noexcept { return m_name; };

    /**
     * \brief Set FitComponent name
     * @param  _name [description]
     */
    void SetName(TString _name) { m_name = _name; };

    /**
     * \brief Get PdfType
     */
    const PdfType Type() const noexcept { return m_type; };

    /**
     * \brief Get FitComponent variable
     */
    RooRealVar * Var() const noexcept { return m_var; };

    /**
     * \brief Set FitComponent variable
     * @param _var [description]
     */
    void SetVar(RooRealVar * _var) {
        m_var = _var;
        return;
    };

    /**
     * \brief Get option used to generate FitComponent
     */
    const TString Option() const noexcept { return m_option; };

    /**
     * \brief Set option used to generate FitComponent
     */
    void SetOption(TString _option) { m_option = _option; };

    /**
     * \brief Get PDF
     */
    RooAbsPdf * PDF() const { return m_pdf.get(); };

    /**
     * \brief Get DataSet
     */
    RooDataSet * DataSet() const { return m_data.get(); };

    /**
     * \brief Get DataHist
     */
    RooDataHist * DataHist() const { return m_hist.get(); };

    Long64_t DataSize(TString _option = "") const;

    void SetData(RooAbsData * _data);

    const TString DataSetCache() const { return m_datasetCache; };

    /**
     * \brief Init FitComponent
     */
    void Init();

    /**
     * \brief Close FitComponent
     */
    void Close(TString _option = "");

    /**
     * \brief Set binned Fit
     * @param  _flag [description]
     */
    void SetBinnedFit(bool _flag);

    bool IsBinned() const noexcept { return m_binned; };

    /**
     * \brief Create PDF
     */
    void CreatePDF();

    /**
     * \brief Create Data
     */
    void CreateData();

    /**
     * \brief Reduce dataset
     * @param _cut [description]
     */
    void ReduceComponent(TCut _cut);

    /**
     * \brief Modify PDF acting on the Str2VarMap ShapeParameter
     * @param  _varToModify  [description]
     * @param  _modifyingVar [description]
     * @param  _modifyOption [description]
     */
    void ModifyPDF(TString _varToModify, RooRealVar * _modifyingVar, TString _modifyOption);

    bool IsModified() const noexcept { return m_isModified; };

    bool IsLoaded() const { return m_isLoaded; };

    bool IsReduced() const { return m_isReduced; };

    double RatioReduced() const { return m_ratioReduced; };

    bool IsAdded() const { return m_isAdded; };

    void SetStatus(bool _isLoaded, bool _isReduced);

    void SetAdded(bool _isAdded);

    void RefreshParameterPool();

    /**
     * \brief Print parameter map
     */
    void PrintParameters() const noexcept;

    /**
     * \brief Set parameters constant
     */
    void SetConstantAllPars();

    /**
     * \brief Set parameters constant except
     * @param  _names [description]
     */
    void SetConstantExceptPars(vector< string > _names);

    /**
     * \brief Set parameters constant except
     * @param  _names [description]
     */
    void SetConstantExceptParsChangeRange(vector< string > _names, double min, double max);

    /**
     * \brief PrintKeys
     */
    void PrintKeys() const noexcept;

    /**
     * \brief Get parameter map, underlying pointers are the same but a map is a new object.
     */
    Str2VarMap ShapeParameters() const { return m_shapeParameters; };

    /**
     * \brief Save FitComponent to disk
     */
    void SaveToDisk(TString _name = "");

    /**
     * \brief Load FitComponent from disk
     */
    void LoadFromDisk(TString _name = "", TString _dir = "");

    /**
     * \brief Save FitComponent PDF cache for StringToFit only.. avoid refitting MC shapes all the time. 
     * TODO : RooKeysPDF saving and reloading...
    */
    bool SaveFitComponentCache();

    /**
     * \brief Load FitComponent PDF cache for StringToFit only. I.e ShapeParameters are loaded from Disk. StringToPDF is used to re-make PDF! 
     * TODO : RooKeysPDF saving and reloading...
    */
    bool LoadFitComponentCache();

    /**
     * \brief Load FitComponent PDF cache for RooKeysPdf only.
    */
    bool LoadFitComponentCacheRooKeysPdf();

    /**
     * \brief Method to load RooKeysPDF from RooWorkspace. Used by LoadFitComponentCacheRooKeysPdf and PdfType::Template
    */
    bool LoadRooKeysPDF(TString _fullpath, TString _workspaceKey);

    /**
     * \brief Save FitComponent to log
     */
    void SaveToLog(TString _name = "") const noexcept;

    /**
     * \brief Convolute PDF
     * e.g. "PartReco | RooKeysPDF | -conv[MEAN,SIGMA,SCALE]"
     * @param _mean  [description]
     * @param _sigma [description]
     * @param _order The order used in the interpolation polynomial
     */
    void ConvPDF(RooRealVar * _mean = nullptr, RooRealVar * _sigma = nullptr, double _order = 2);

  private:
    /**
     * \brief Check allowed arguments
     */
    bool Check() const noexcept;

    bool m_debug = false;
    /**
     * \brief Activate debug
     * @param  _debug [description]
     */
    void SetDebug(bool _debug) { m_debug = _debug; };

    /**
     * \brief Create StringToPDF Shape
     */
    void CreateStringToPDF();

    /**
     * \brief Create StringToPDF Shape and Fit MC
     */
    void CreateStringToFit();

    /**
     * \brief Create RooHistPDF Shape
     */
    void CreateRooHistPDF();

    /**
     * \brief Create RooKeysPDF Shape
     */
    void CreateRooKeysPDF();

    /**
     * \brief Create PDF from a template RooKeyPdf or fit RooDataSet from template Histogram 
     */
    void CreatePdfFromTemplate();
    /**
     * \brief Add PDFs from yaml
     * e.g. "PartReco | RooKeysPDF | -add[Bd2XJPsMM * 1 + Bu2XJPsMM * 2 + Bs2XJPsMM * 0.5] | -tmCustom"
     */
    void AddPDFs(TCut _cut = TCut(NOCUT));

    void AddDatasets();

    void ReduceData(Long64_t _maxEvt, bool _random = false);

    void ShiftDataset(double _shift);

    pair< double, double > GetRange();
    
    pair< double, double > GetExcludedRange();

    void CreateDatasetCachePath();
    void SaveDatasetCache();
    void LoadDatasetCache();
    void DeleteDatasetCache();

    tuple<TString,TString, TString > FitComponentCacheInfos();

    ClassDef(FitComponent, 1);
};

ostream & operator<<(ostream & os, const FitComponent & _fitComponent);

#endif
