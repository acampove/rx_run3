#ifndef FITTERTOOL_HPP
#define FITTERTOOL_HPP

#include "EnumeratorSvc.hpp"

#include "FitterSvc.hpp"
#include "HelperSvc.hpp"
#include "HistogramSvc.hpp"
#include "MessageSvc.hpp"

#include "FitComponent.hpp"
#include "FitHolder.hpp"
#include "FitManager.hpp"

#include "ConstraintsGenerator.hpp"

#include "RooAbsData.h"
#include "RooAddition.h"
#include "RooCmdConfig.h"
#include "RooDataHist.h"
#include "RooDataSet.h"
#include "RooHist.h"
#include "RooMinimizer.h"
#include "RooNLLVar.h"
#include "RooPlotable.h"
#include "RooStats/SPlot.h"

#include "Math/CholeskyDecomp.h"
#include "TLegend.h"
using namespace RooStats;



namespace Plotting{
  /* Graphics settings for plots */
  void ConfigurePadPlot( TPad * _plotPad);
  void ConfigurePullPlot( TPad * _resPad);
  /* Graphics settings for legend */
  TLegend * GetLegend( int nLabels);
  /* Graphics settings for RooPlot with PDFs shapes */
  void CustomizeRooPlot( RooPlot * _frame);
  /* Graphics settings for RooPlot with Pulls */
  void CustomizeFramePull( RooPlot *_framePull);  
  /* Function used to "order" things in the plot */
  int bkgsortidx( const TString &_name);  

  /* Function that takes a frame, root file name, open mode for it and just dump down all the objects contained in the frame */
  void SaveAllObjectFromFrame( RooPlot *_frame, TString _outFile, OpenMode _oMode = OpenMode::RECREATE);
};


struct InitialParamOption {
    double Value;
    double Error;
    bool   Constant;
    bool   NoRange;
    bool   gConst;
    bool   RangeSpecified;
    double LowerLimit;
    double UpperLimit;
};

class FitterTool {

  private:
    TString m_name;   //---ID name of the fit you do with this FitterTool, useful for toy fit results for instance and change names around
    //---- we squash all the EE modes and MM modes in one go
    map< TString, QSquareFit > m_fitManagers;   // <- map holding the fitmanagers and the wrappers to use for EE / MM variables for the fit

    RooCmdArg m_optList;   // <- optList filled as you do for createNLL

    RooFitResult * m_fitResults = nullptr;

    Int_t m_status = -1;

    bool m_isFitTo       = kFALSE;   // < - Internal flag to identify if it has to use MCFit range or DataFit one
    bool m_isInitialized = kFALSE;   // < - internal flag of the class

    bool m_reFit = kFALSE;

    // Likelihood creation flags.
    Bool_t m_extended         = kTRUE;    // <- Extended fit (put always to true)
    Bool_t m_offsetLikelihood = kTRUE;    // <- add offset to likelihood
    Int_t  m_nCPU             = 1;        // <- num cpu to createNLL
    Int_t  m_strategyCPU      = 2;        // <- cpu strategy
    Bool_t m_sumW2Error       = kFALSE;   // < - unused
    Bool_t m_verbose          = kFALSE;   // <- run in Verbose mode
    // Typically are passed to the fitTo, we use them in the RooMinimizer setup and usage
    TString m_minType          = "Minuit";   //< allowed Minuit2
    TString m_minAlg           = "migrad";
    Int_t   m_strategyMINUIT   = 2;        // <- minuit strategy
    Bool_t  m_logFile          = kFALSE;   // <- create a Log file of the fit. This is bugged and causes memory leaks and hundreds of unclosed files, do not use for toys.
    Bool_t  m_initialHesse     = kFALSE;   // < - run Hesse before minuit to have better error
    Bool_t  m_hesse            = kTRUE;    // < - run Hesse
    Int_t   m_maxFunctionCalls = 10000000; // < - FOR VERY COMPLEX FITS WE MUST INCREASE THIS BY QUITE A LOT.
    Bool_t  m_minos            = kFALSE;   // < - Use Minos at the end
    Bool_t  m_optimizeConst    = kFALSE;   // < - const optimization
    Bool_t  m_save             = kTRUE;
    Bool_t  m_timer            = kTRUE;   // < - measure the time taken by the fit
    /*

    */
    Int_t   m_printLevel       =  3;      // <- print level (verbose!!)
    Int_t   m_printEvalErrors  = -1;      // <- print level
    Bool_t  m_warnings         = kTRUE;   // < - warnings printing

    Bool_t  m_excludeRange = kFALSE;
    TString m_excludedRange = "";

    Double_t m_offset = 0; 
    // Strategy MINUIT
    // SET STRategy. In the current release, this parameter can take on three
    // integer values (0, 1, 2), and the default value is 1. Value 0 indicates to Minuit that it should economize
    // function calls; it is intended for cases where there are many variable parameters and/or the function takes
    // a long time to calculate and/or the user is not interested in very precise values for parameter errors. On
    // the other hand, the value 2 indicates that Minuit is allowed to waste function calls in order to be sure that
    // all values are precise; it is intended for cases where the function is evaluated in a very short time and/or
    // where the parameter errors must be calculated reliably

    // Strategy RooMinimizer
    // Type         Algorithm
    // OldMinuit    migrad, simplex, minimize (=migrad+simplex), migradimproved (=migrad+improve)
    // Minuit       migrad, simplex, minimize (=migrad+simplex), migradimproved (=migrad+improve)
    // Minuit2      migrad, simplex, minimize, scan
    // GSLMultiMin  conjugatefr, conjugatepr, bfgs, bfgs2, steepestdescent
    // GSLSimAn     -

    // Strategy CPU
    // Strategy 0 = RooFit::BulkPartition (Default) --> Divide events in N equal chunks
    // Strategy 1 = RooFit::Interleave --> Process event i%N in process N. Recommended for binned data with
    //              a substantial number of zero-bins, which will be distributed across processes more equitably in this strategy
    // Strategy 2 = RooFit::SimComponents --> Process each component likelihood of a RooSimultaneous fully in a single process
    //              and distribute components over processes. This approach can be benificial if normalization calculation time
    //              dominates the total computation time of a component (since the normalization calculation must be performed
    //              in each process in strategies 0 and 1. However beware that if the RooSimultaneous components do not share many
    //              parameters this strategy is inefficient: as most minuit-induced likelihood calculations involve changing
    //              a single parameter, only 1 of the N processes will be active most of the time if RooSimultaneous components
    //              do not share many parameters
    // Strategy 3 = RooFit::Hybrid --> Follow strategy 0 for all RooSimultaneous components, except those with less than
    //              30 dataset entries, for which strategy 2 is followed.

    //----- deal with constraints
    ConstraintsGenerator m_constraints;
    bool                 m_constraintParameters = false;
    bool                 m_constraintEfficiencies = false;
    /// List of colors to plot background components
    // const vector< Color_t > m_colors = {kRed, kBlue, kViolet, kCyan, kGreen, kOrange, kGray+2, kYellow, kMagenta};
    const vector< Color_t > m_colors = {kRed, kBlue, kGreen, kMagenta, kOrange, kAzure + 10, kYellow, kGray + 2, kPink, kGreen+4, kSpring, kViolet, kTeal};

    bool m_saveAllExt = false;
    bool m_profile1DRatios = false ;
    bool m_profile2DRatios = false ;

  public:

    /**
     * \brief Default constructor
     */
    FitterTool() = default;

    /**
     * \brief Constructor with TString
     */
    FitterTool(TString _name);
    /**
    * \brief Constructor with a simple FitterInfoContainer pre-filled, very special use case for simple fits in python 
    Example : 
    varMap = r.Str2VarMapObj()
    pdf = r.StringToPdf( string_to_fit["RK_low"], "rk_low_run1", bmass, varMap)
    pdf.Print("v")
    container           = r.FitterInfoContainer()
    container.var       = bmass 
    container.binned = false
    container.dataset   = ConvertHistogramToRooDataSet( histoRun1, bmass, wvar, "RK_low_Run1")
    container.ismc      = r.kTrue
    container.fullmodel = pdf 
    fitter = r.FitterTool( "FitCompRKlow")
    fitter.FitTo()

    */
    FitterTool(TString _name, FitterInfoContainer * _container);
    /**
     * \brief Constructor with FitComponent
     */
    FitterTool(TString _name, FitComponent * _fitComponent);

    /**
     * \brief Constructor with FitHolder
     */
    FitterTool(TString _name, FitHolder * _fitHolder);

    /**
     * \brief Constructor with FitManager
     */
    FitterTool(TString _name, FitManager * _fitManager);

    /**
     * \brief Fit name
     */
    const TString GetName() const { return m_name; };

    /**
      * \brief GetLLOffset initial values 
    */
    const Double_t LLOffset() const { return m_offset; }
    /**
     * \brief Fit results
     */
    const RooFitResult * Results() const { return m_fitResults; };
    RooFitResult *       Results() { return m_fitResults; };

    /**
     * \brief Add FitComponent
     * @param  _fitComponent [description]
     */
    void AddFitComponent(FitComponent * _fitComponent);

    /**
     * \brief Add FitHolder
     * @param  _fitHolder [description]
     */
    void AddFitHolder(FitHolder * _fitHolder);

    /**
     * \brief Add FitManager
     * @param  _fitManager [description]
     */
    void AddFitManager(FitManager * _fitManager);

    /**
     *  \brief Retrieves references to the FitManagers used by FitterTool.
     */
    map< TString, QSquareFit > & GetFitManagers() { return m_fitManagers; };

    /**
     *  \brief Sets the flag on whether to constraint
     */
    void SetConstraintFlag(bool _shouldConstraint) {         m_constraintParameters = _shouldConstraint; }
    void SetConstraintFlagEffs( bool _shouldConstraintEffs){ m_constraintEfficiencies = _shouldConstraintEffs; }

    /**
     * \brief Add GaussConstraint
     * @param  _par   [description]
     * @param  _mean  [description]
     * @param  _sigma [description]
     */
    void AddGaussConstraint(RooRealVar * _par);

    /**
     * \brief Add MultiVar GaussConstraint
     * @param  _listOfVariables [description]
     * @param  _covMatrix       [description]
     */
    void AddMultiVarGaussConstraint(RooArgList & listOfVariables, TMatrixDSym covMatrix);

    /**
     * \brief Initialize the fitter loading the FitHolders from the fit manager, generate internally the map of FitInfo for EE and MM mode
     */
    void Init();

    /**
     * \brief Sets the initial parameter values to those defined in initialParamFile
     */
    void SetInitialValuesAndErrors();

    /**
     * \brief Close FitComponent
     */
    void Close();

    /**
     *  \brief Gets the linkage map of constrained variables to the means.
     */
    map< RooRealVar *, RooRealVar * > GetConstraintToMeanMap() { return m_constraints.GetVariableToMeanMap(); };

    /**
     *  \brief Gets the linkage map of constrained variables to the means.
     */
    vector< CorrelatedConstraintsInfo > GetCorrelatedConstraintsInfos() { return m_constraints.GetCorrelatedConstraintsInfos(); };

    /**
     * \brief Initialize Holder
     * @param  _holder [description]
     * @param  _name   [description]
     * @param  _var    [description]
     */
    FitterInfoContainer InitHolder(const FitHolder & _holder, TString _name);

    /**
     * \brief Initialize the Opt List
     */
    void InitOptList();

    /**
     * \brief Create the likelihood for the EE/MM mode, sum the Likelihoods of EE /MM mode, if simultaneous merge the likelihoods;
     */
    void Fit(bool _saveResults = true);

    /**
     * \brief Fit to using model->fitTo(data); used for single model with 1 data set only. No categories involved.
     */
    void FitTo(bool _saveResults = true);

    /**
     * \brief Compute SPlot
     */
    void DoSPlot();

    /**
     * \brief ConfigureMinimizer
     * @param  _minimizer [description]
     * @param  _logFile   [description]
     */
    void ConfigureMinimizer(RooMinimizer & _minimizer, TString _logFile);

    /**
     * \brief FittingProcedure lunch steps of fitter after being configured
     * @param  _minimizer [description]
     * @param  _nll       [description]
     */
    void FittingProcedure(RooMinimizer & _minimizer, RooAbsReal * _nll);

    /**
     * \brief Custom plotting routine, dedicated to RJPs
     */
    void PlotResults();

    /**
     * \brief Draws the reduced correlation matrix given a list of parameters.
     */
    void MakeCorrelationMatrix(const RooArgList & _parameterList, TString _name = "");

    /**
     * \brief Custom plotting routine, dedicated to RJPs combining the categories in EE/MM for each fit manager
     */
    void PlotSumCategories();

    /**
     * \brief PlotProfileLikelihood
     * @param  _nllSimultaneous  [Likelihood function minimized by main code]
     * @param  _varForLikelihood [description]
     * @param  _min              [description]
     * @param  _max              [description]
     */
    void PlotProfileLikelihood(const TString &  _varForLikelihood, double _min, double _max);
    /**
     * \brief PlotContour
     * @param  _var1 [description]
     * @param  _var2 [description]
     */
    void PlotProfileLikelihoodROOT(const TString &  _varForLikelihoodName , double _minRange, double _maxRange);

    void PlotContour(RooMinimizer & _minimizer , RooRealVar & _var1, RooRealVar & _var2);

    /**
     * \brief PrintFitter
     */
    void PrintFitter();

    /**
     * \brief PrintResults
     */
    void PrintResults(ostream & _stream = cout);

    /**
     * \brief SaveResults
     * @param  _name [description]
     */
    void SaveResults(TString _name = "");

    void SaveDOTs();

    void ReFit(Bool_t _flag) { m_reFit = _flag; };

    void Extended(Bool_t _flag) { m_extended = _flag; };
    void Hesse(Bool_t _flag) { m_hesse = _flag; };
    void InitialHesse(Bool_t _flag) { m_initialHesse = _flag; };
    void LogFile(Bool_t _flag) { m_logFile = _flag; };
    void MinimizerType(TString _minType, TString _minAlg) {
        m_minType = _minType;
        m_minAlg  = _minAlg;
    };
    void Minos(Bool_t _flag) { m_minos = _flag; };
    void MaxFunctionCalls(Int_t _maxFunctionCalls) { m_maxFunctionCalls = _maxFunctionCalls; };
    void NCPU(Int_t _ncpu) { m_nCPU = _ncpu; };
    void Offset(Bool_t _flag) { m_offsetLikelihood = _flag; };
    void OptimizeConst(Bool_t _flag) { m_optimizeConst = _flag; };
    void PrintLevel(Bool_t _flag) { m_printLevel = _flag; };
    void PrintEvalErrors(Bool_t _flag) { m_printEvalErrors = _flag; };
    void Save(Bool_t _flag) { m_save = _flag; };
    void StrategyCPU(Bool_t _flag) { m_strategyCPU = _flag; };
    void StrategyMINUIT(Int_t _flag) { m_strategyMINUIT = _flag; };
    void SumW2Error(Bool_t _flag) { m_sumW2Error = _flag; };
    void Timer(Bool_t _flag) { m_timer = _flag; };
    void Verbose(Bool_t _flag) { m_verbose = _flag; };
    void Warnings(Bool_t _flag) { m_warnings = _flag; };
    void ExcludeRange(Bool_t _flag) { m_excludeRange = _flag; };
    void SetExcludedRange(const TString _rangeString) {m_excludedRange = _rangeString; };

    void FullDebug();   // <- use this to correctly check internal dependencies of the fit

    /**
     * \brief Prints how the paramters are shared between components.
     * \brief A one-to-many linking from parameter to components.
     * \brief Parameters must be shared between at least 2 components to be printed.
     */
    void PrintParameterComponentRelation();

    /**
     * \brief Save everything to workspace.
    */
    void SaveToWorkspace(TString _name);

    void SetProfile1D( bool _flag){ m_profile1DRatios = _flag;}
    void SetProfile2D( bool _flag){ m_profile2DRatios = _flag;}

  private:
    bool m_debug = false;
    map< TString, InitialParamOption > m_initialParams;
    /**
     * \brief Activate debug
     * @param  _debug [description]
     */
    void SetDebug(bool _debug) { m_debug = _debug; };

    /**
     * \brief Print OptList
     */
    void PrintOptList();

    inline void AddConstraints(const RooArgSet & _nLL);

    void PlotCorrelationMatrix(TH2 * _hCorr, TString _corrName);

    /**git
     * \brief Load the ArgSet to produce the Addition of likelihoods
     * @param  _nLL_set [description]
     */
    inline void CreateNLL(RooArgSet & _nLL);

    map< TString, vector< TString > > m_parameterToComponentsMap;
    map< TString, RooRealVar * >      m_independentParameterMap;

    void                               GetParameterMaps();
    vector< RooAbsReal * >             GetParametersInComponent(const FitComponentAndYield & _componentAndYield);
    void                               InitStepSize();
    void                               LoadInitialParameters();
    map< TString, InitialParamOption > ParseInitialParams();
    void                               DumpInitialParameters();

    inline void DeleteNLL();

    void DumpYieldsValues();
    void AddParameterComponentRelation(const TString & _componentName, const vector< RooAbsReal * > _parameters);
    ClassDef(FitterTool, 0);
};



namespace MergedPlot { 
  // Supporting functions to deal with QSquareFit underlying structure to merge the EE/MM datasets from the sub-categories  
  // Usage example (see FitterTool::MakePlotsSumCategories for the actual application of this)
  // Useful method to merge fit-plot results when dealing with splitting categories fits (Years,Polarity,Trigger for example)
  // Let's say you have a QSquareFit object with 1 FitManager with 3 MM categoris [Trigger] and 3 EE [Trigger] categories.
  // MM categories has Bs + Lb + Comb + Signal as pdf thus:
  // manager.second = QSquareFit object
  // Create a Combined Muon mode dataset from the 3 Trigger categories
  // RooDataSet * combdataMM = MergeDataSet<Analysis::MM>(manager.second);
  // Map housing the List of PDfs indexed by TYPE (Bs,Comb,Lb,Signal) and the List of Coefficients associatd to them.*/
  // map< TString, pair< RooArgList, RooArgList  > > pdfs_MM = GetPDFsCoefsCategories2<Analysis::MM>(manager.second);
  // RooArgList housing all the pdfs for the Sum of the 3 Category Full PDF*/
  // RooArgList all_pdfsMM = AllPDFsFromDecomposition(pdfs_MM);
  // RooArgList housing all the coefficients for the Sum of the 3 Category Full PDF*/
  // RooArgList all_coefsMM = AllCoefsFromDecomposition(pdfs_MM);
  // Build the Simultaneous PDF with the correct MM yields and MM shapes*/
  // RooAddPdf * merged_pdfMM = new RooAddPdf("merged_pdfMM", "merged_pdfMM", all_pdfsMM, all_coefsMM);
  // Now you can : pdfs_MM[signal].first = RooArgList to pass to the Components() command when plotting plotting the sum of a given species of background, signal
  // Given a FitManager casted to QSquareFit object-struct, merge the MM/EE dataset(S) from the various sub-categories
  template < Analysis ana > RooDataSet * MergeDataSet(const QSquareFit & _fitManagers);

  // Supporting functions to deal with QSquareFit underlying structure to merge the EE or MM analysis sub-structures of shapes from the categories
  // This returs a vector["TYPE = Signal,Bs,Lb,PartReco, etc.."]  = pair< ListPDFs, ListYields > ; with sorting following bkgsortidx .
  // Where ListPDFs = TYPE_LOI, TYP_L0L, TYPE...
  // and ListYidls  = n_TYPE_L0I, n_TYPE_L0L
  // template < Analysis ana > map< TString, pair< RooArgList, RooArgList > > GetPDFsCoefsCategories(const QSquareFit & _fitManagers) {
  template < Analysis ana > 
  map<TString, pair<RooArgList,RooArgList> > GetPDFsCoefsCategories(const QSquareFit & _fitManagers);

  template < Analysis ana > 
  map<TString, Int_t>  GetColorsCategories(const QSquareFit & _fitManagers);


  // Given the  map["TYPE = Signal,Bs,Lb,PartReco, etc.."]  = pair< ListPDFs, ListYields >, it return a list of all pdfs
  // RooArgList AllPDFsFromDecomposition(const map< TString, pair< RooArgList, RooArgList > > & _map);
  RooArgList AllPDFsFromDecomposition(const map<TString, pair< RooArgList, RooArgList >> & _map);

  /**
  * \brief Collect all coefficients PDFs for the full model generation as if the fit was not done splitted by category
  * @param  const map< TString, pair< RooArgList, RooArgList  > > & _map [TString = KeyOfShape], pair< Pdf_shapeTYPEList, Coef_shapeTYPEList >
  *
  */
  // Given the  map["TYPE = Signal,Bs,Lb,PartReco, etc.."]  = pair< ListPDFs, ListYields >, it return a list of all coefficients
  RooArgList AllCoefsFromDecomposition(const map<TString, pair< RooArgList, RooArgList >> & _map);

  /**
  * \brief Create a canvas with the summed PDFs in it splitted by categories
  * @param  Pass the QSquareFit object, the combined dataset, the full model generated summing all categories, the corresponing _mapOFPDFs, th observable of interest, a list of colors for the plot
  */
  template < Analysis ana > TCanvas * PlotAndResidualPad(const QSquareFit & _fitManagers, const RooDataSet * _dataset, const RooAddPdf * _model, const map<TString, pair< RooArgList, RooArgList > > & _mapPDFS, const RooRealVar * _observable, TString _managername, const map<TString,Int_t > & colors);

  const vector<Int_t> DefaultColors{ (Int_t)kRed, (Int_t)kBlue, (Int_t)kGreen, (Int_t)kMagenta, (Int_t)kOrange, (Int_t)kAzure + 10, (Int_t)kYellow, (Int_t)kGray + 2, (Int_t)kPink, (Int_t)kGreen+4, (Int_t)kSpring, (Int_t)kViolet, (Int_t)kTeal};
};
#endif
