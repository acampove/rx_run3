#ifndef FITPARAMETERPOOL_HPP
#define FITPARAMETERPOOL_HPP

#include "FitterSvc.hpp"

#include "EventType.hpp"
#include "FitConfiguration.hpp"
#include "FitParameterConfig.hpp"

#include <iostream>
#include <map>

#include "TString.h"

#include "RooAbsReal.h"
#include "RooRealVar.h"

class ParameterWrapper {
  public:
    ParameterWrapper();
    ParameterWrapper(RooAbsReal * _parameter);
    void               SetParameter(RooAbsReal * _parameter);
    void               Blind();
    void               SmearParameter(double newValue);
    bool               IsBlinded() const;
    RooAbsReal *       GetBlindedParameter();
    RooAbsReal *       GetUnblindedParameter();
    RooAbsReal *       GetBaseParameter();
    const RooAbsReal * GetBaseParameter() const;
    void               DeleteParameter();
    double             TransformCovarianceIfBlinded(double _covariance) const;
    double             TransformVarianceIfBlinded(double _variance) const;

  private:
    void ThrowIfNotRooRealVar(RooAbsReal * _parameter);

  private:
    bool         m_isBlinded          = false;     // Blinding boolean
    RooAbsReal * m_blindedParameter   = nullptr;   // The parameter to return
    RooAbsReal * m_unblindedParameter = nullptr;   // The unblinded parameter
};

class CorrelatedEfficienciesHolder {
  public:
    CorrelatedEfficienciesHolder();
    CorrelatedEfficienciesHolder(const vector< RooRealVar * > & _correlatedEfficiencies, const vector< vector< double > > & _covarianceMatrix);
    void                       SetCorrelatedEfficiencies(const vector< RooRealVar * > & _correlatedEfficiencies);
    void                       SetCovarianceMatrix(const vector< vector< double > > & _covarianceMatrix);
    vector< RooRealVar * >     GetCorrelatedEfficiencies() const;
    vector< vector< double > > GetCovarianceMatrix() const;

  private:
    void ThrowIfCovarianceMatrixWrong(const vector< vector< double > > & _covarianceMatrix) const;
    void ThrowIfRowAndColumnMisMatch(const vector< vector< double > > & _covarianceMatrix) const;
    void ThrowIfOffDiagonalZero(const vector< vector< double > > & _covarianceMatrix) const;
    bool IsCorrelatedEfficienciesEmpty() const;
    bool IsCovarianceMatrixEmpty() const;
    void WarnEmptyContainer(TString _containerName) const;
    void ThrowIfListAndMatrixSizeMismatch() const;

  private:
    vector< RooRealVar * >     m_correlatedEfficiencies;
    vector< vector< double > > m_covarianceMatrix;
};

/**
 * \brief      FitParameterPool class which manages all the yield, efficiency and ratio parameters used by the fitter
 * \brief      Also manages some shape parameters, but mostly the ones shared between PDFs.
 * \brief      This is initialised as a static, global instance so it manages all instance of fit parameters. This makes memory management cleaner.
 * \brief      The plan is to move all shape parameter here in the future. This will help reduce memory leaks.
 * @note       Breaks saving to disk for now.
 */

class FitParameterPool {
  public:
    /**
     * \brief      Constructs the object.
     */
    FitParameterPool();

    /**
     * \brief      Deletes all parameters bookkept by FitParamterPool from memory.
     */
    void ClearParameters();

    /**
     * \brief      Configures the appropriate ratio and efficiencies given some FitConfiguration and the option used on them.
     * @param[in]  _configurations  The vector of FitConfiguration.
     * @param[in]  _option          The string option
     */
    void ConfigureParameters(vector< FitConfiguration > _configurations, TString _option);

    /**
     * \brief      Gets the efficiency bookkept by FitParameterConfig passed.
     * @param[in]  _effKey  The FitParameterConfig to identify an efficiency.
     * @return     The efficiency unique to the FitParameterConfig passed.
     */
    RooAbsReal * GetEfficiency(const FitParameterConfig & _effKey, bool _forRatio=false);

    /**
     * \brief      Gets the fixed efficiency bookkept by FitParameterConfig passed.
     * @param[in]  _effKey  The FitParameterConfig to identify a fixed efficiency.
     * @return     The fixed efficiency unique to the FitParameterConfig passed.
     */
    RooAbsReal * GetFixedEfficiency(const FitParameterConfig & _effKey);

    /**
     * \brief      Gets the ratio bookkept by FitParameterConfig passed.
     * @param[in]  _ratioKey  The FitParameterConfig to identify a ratio parameter.
     * @return     The ratio parameter unique to the FitParameterConfig passed.
     */
    RooAbsReal * GetRatioParameter(const FitParameterConfig & _ratioKey, const RatioType & _ratioType);

    /**
     * \brief      Gets the all the single ratios bookkept by FitParameterPool
     * @return     The single ratios in FitParameterPool
     */
    vector <RooRealVar *> GetSingleRatios() const;

    /**
     * \brief      Gets the all the double ratios bookkept by FitParameterPool
     * @return     The double ratios in FitParameterPool
     */
    vector <RooRealVar *> GetDoubleRatios() const;

    /**
     * \brief      Gets the yield bookkept by FitParameterConfig passed.
     * @param[in]  _yieldKey  The FitParameterConfig to identify a yield parameter.
     * @return     The yield parameter unique to the FitParameterConfig passed.
     */
    RooAbsReal * GetYield(const FitParameterConfig & _yieldKey);

    /**
     * \brief      Gets the shape parameter from it's name string.
     * @param[in]  _shapeParameterName  The shape parameter name string.
     * @return     The shape parameter.
     */
    RooAbsReal * GetShapeParameter(TString _shapeParameterName);

    /**
     * \brief      Adds an efficiency. Internally calls EfficiencySvc to retrieve it.
     * @param[in]  _fitParameterConfig  The FitParameterConfig referring to a specific efficiency value.
     */
    void AddEfficiency(const FitParameterConfig & _fitParameterConfig);

    /**
     * \brief      Adds a yield parameter.
     * @param[in]  _yieldKey  The yield FitParameterConfig.
     * @param      _variable  The yield parameter pointer.
     */
    void AddYieldParameter(const FitParameterConfig & _yieldKey, RooAbsReal * _yieldParameter);

    /**
     * \brief      Adds a ratio parameter.
     * @param[in]  _ratioKey        The ratio FitParameterConfig
     * @param      _ratioParameter  The ratio parameter pointer.
     */
    void AddRatioParameter(const FitParameterConfig & _ratioKey, const RatioType & _ratioType, RooAbsReal * _ratioParameter);

    /**
     * \brief      Adds a shape parameter.
     * @param      _variable  The shape parameter.
     */
    void AddShapeParameter(RooAbsReal * _shapeParameter);

    /**
     * \brief      Adds a parameter to the constrained parameter list.
     * @param      _variable  The constrained parameter.
     */
    void AddConstrainedParameter(RooRealVar * _constrainedParameter);

    /**
     * \brief      Replaces the yield parameter bookkept by the FitParameterConfig passed with another.
     * \brief      Deletes the old parameter from memory.
     * @param[in]  _yieldKey      The yield FitParameterConfig.
     * @param      _newParameter  The new yield parameter.
     */
    void ReplaceYieldParameter(const FitParameterConfig & _yieldKey, RooAbsReal * _newParameter);

    /**
     * \brief      Replaces the shape parameter bookkept with the string passed with another.
     * \brief      Deletes the old parameter from memory.
     * @param[in]  _shapeParameterName  The shape parameter name.
     * @param      <unnamed>            The new shape parameter.
     */
    void ReplaceShapeParameter(TString _shapeParameterName, RooAbsReal * _newParameter);

    /**
     * \brief      Adds a background yield from the ratio of signal yields in EE and MM.
     * \brief      Uses the formula: n_bkgEE = n_sigEE * n_bkgMM / n_sigMM
     * \brief      Converts the elctron FitParameterConfig to their corresponding muonic ones.
     * @param[in]  _keyBkgEE  The electron backgrounf FitParameterConfig.
     * @param[in]  _keySigEE  The electron signal FitParameterConfig.
     */
    void AddBackgroundYieldSignalRatio(const FitParameterConfig & _yieldKeyBkgEE, const FitParameterConfig & _yieldKeySigEE);

    void AddBackgroundYieldSignalRatio(const FitParameterConfig & _yieldKeyBkg, const FitParameterConfig & _yieldKeySig, const double _scale);

    /**
     * \brief OBSOLETE ( we use eps-ratios and constraint enters as constant on efficiencies values)
     * TODO : Cleanup , REMOVE
    void AddBackgroundYieldSignalConstrainedRatio(const FitParameterConfig & _yieldKeyBkg, const FitParameterConfig & _yieldKeySig, const double _ratio, const double _error, const TString & _ratioName, const double _scale = 1.);
    */

    /**
     * \brief      Adds a background yield from the ratio of efficiencies in EE and MM.
     * \brief      Uses the formula: n_bkgEE = n_bkgMM * eff_bkgEE / eff_bkgMM
     * \brief      Converts the elctron FitParameterConfig to their corresponding muonic ones.
     * @param[in]  _keyBkgEE  The electron background FitParameterConfig.
     */
    void AddBackgroundYieldEfficiencyRatio(const FitParameterConfig & _yieldKeyBkgEE);
    
    /**
     * \brief      Adds yield as a function of a single ratio and the yield of the muon mode.
     * \brief      Uses the formula: n_sigEE = n_sigMM * single_ratio_sig * eff_sigEE / eff_sigMM
     * \brief      Converts the elctron FitParameterConfig to the corresponding muonic one to retrieve the muon yield and efficiency.
     * @param[in]  _keySigEE  The electron FitParameterConfig.
     * @param[in]  _option    The option string.
     */

    void AddBackgroundYieldSignalEfficiencyRatio(const FitParameterConfig & _keyBkg, const FitParameterConfig & _keySignal, double _scale = 1.);
    /**
     * \brief      Adds yield as a function of a yield multiplied by an efficiency ratio.
     * \brief      Uses the formula: n_bkg = n_sig * eff_bkg / eff_sig
     * @param[in]  _keyBkg       The bkg FitParameterConfig
     * @param[in]  _keySignal    The signal FitParameterConfig.
     */

    void AddSingleRatioYield(const FitParameterConfig & _keySigEE, TString _option);

    /**
     * \brief      Adds yield as a function of a double ratio and the yield of the muon mode.
     * \brief      Uses the formula: n_sigEE = n_sigMM * double_ratio_sig * JPs_single_ratio * (eff_sigEE / eff_sigMM)
     * \brief      Converts the elctron FitParameterConfig to the corresponding muonic one to retrieve the muon yield and efficiency.
     * \brief      Retrives the JPs single ratio to build the double ratio.
     * @param[in]  _keySigEE  The electron FitParameterConfig.
     * @param[in]  _option    The option string.
     */
    void AddDoubleRatioYield(const FitParameterConfig & _keySigEE, TString _option);

    /**
     * \brief      Returns a vector of all efficiencies bookkept by FitParameterPool, regardless of blinding.
     * @return     All efficiencies.
     */
    vector< RooRealVar * > GetEfficiencies();

    /**
     * \brief      Returns a vector of all efficiencies bookkept by FitParameterPool.
     * @return     All efficiencies.
     */
    vector< RooAbsReal * > GetAllIndependentSignalYields();

    vector< RooAbsReal * > GetAllIndependentBackgroundYields();

    vector< RooAbsReal * > GetAllIndependentYields();

    vector< RooRealVar * > GetConstrainedParameters();

    /**
     * \brief      Gets number efficiencies bookkept by FitParameterPool.
     * @return     Int number of efficiencies.
     */
    int GetEfficienciesCount() const;

    /**
     * \brief      Gets the number of yield parameters bookkept by FitParameterPool.
     * @return     Int number of yields.
     */
    int GetYieldsCount() const;

    /**
     * \brief      Gets the number of shape parameters bookkept by FitParameterPool.
     * @return     Int number of shape parameters.
     */
    int GetShapeParameterCount() const;

    /**
     * \brief      Prints all the parameters bookkept by FitParameterPool.
     */
    void PrintParameters() const;

    /**
     * \brief      Gets the maps linking floating efficiencies to their constant efficiency.
     * @return     A map linking fixed efficiencies to their floating equivalent.
     */
    map< RooRealVar *, RooRealVar * > GetFloatingToFixedEfficiencyMap() const;

    void AddYieldFormula(const FitParameterConfig & _yieldKey, RooAbsReal * _floatingYield, vector< RooAbsReal * > _numerators, vector< RooAbsReal * > _denominators, double _scale = 1.);

    /**
     * \brief      Initialises the covariances and efficiencies needed by the likelihood minimization.
     */
    void FillConstrainedEfficiencyContainers(const RooArgList & _listOfLikelihoods);

    /**
     * \brief      Returns all the constrained parameters needed by the likelihood minimization.
     * @return     A vector of parameters that should be constrained in the likelihood.
     */
    vector< RooRealVar * > GetConstrainedParametersInLikelihood(const RooArgList & _listOfLikelihoods) const;

    /**
     * \brief      Returns all the uncorrelated efficiencies needed by the likelihood minimization.
     * @return     A vector of uncorrelated efficiencies RooRealVar that should be constrained in the likelihood.
     */
    vector< RooRealVar * > GetUncorrelatedEfficiencies() { return m_uncorrelatedEfficiencies; }

    /**
     * \brief      Returns all the correlated efficiencies needed by the likelihood minimization.
     * @return     A vector of struct. In the struct is a vector of correlated efficiency RooRealVar and covariance matrix constrained in the likelihood.
     */
    vector< CorrelatedEfficienciesHolder > GetCorrelatedEfficiencies() { return m_correlatedEfficiencies; }
    /**
     * \brief      Gets the ratio bookkept by FitParameterConfig passed.
     * @param[in]  _ratioKey  The FitParameterConfig to identify a ratio parameter.
     * @return     The ratio parameter unique to the FitParameterConfig passed.
     */
    vector <RooRealVar *> GetSingleRatios() {
        vector <RooRealVar *> _singleRatios;
        for (auto & _keyRatioPair : m_ratioMap){
            if (_keyRatioPair.first.second == RatioType::SingleRatio){
                auto * _singleRatio = (RooRealVar*)_keyRatioPair.second.GetBaseParameter();
                _singleRatios.push_back(_singleRatio);
            }
        }
        return _singleRatios;
    };    
    void InitEffRatioSystematicCovariance();
    pair< RooArgList, TMatrixDSym > GetSystematicEffRatioListAndCovarianceMatrix();
    /**
     * \brief      The destructor. Deletes all parameters from memory.
     */
    ~FitParameterPool();

    void                                   AddYieldFormula(const FitParameterConfig & _yieldKey, RooAbsReal * _var1, RooAbsReal * _var2);
    void                                   RemoveYieldParameter(const FitParameterConfig & _yieldKey);
    void                                   RemoveShapeParameter(TString _name);

  private:
    template < typename T > void           DeleteParametersInMap(map< T, ParameterWrapper > & _parameterMap);
    void                                   ModShiftBremDifference(TString _bremKey);
    void                                   ThrowIfEfficiencyDoesNotExist(const FitParameterConfig & _effKey, TString _callerName) const;
    bool                                   EfficiencyExists(const FitParameterConfig & _effKey) const;
    bool                                   ShouldBlind(const FitParameterConfig & _key) const;
    void                                   ThrowIfYieldDoesNotExist(const FitParameterConfig & _yieldKey, TString _callerName) const;
    void                                   ThrowIfShapeDoesNotExist(const TString & _shapeParameterName, TString _callerName) const;
  public : 
    bool                                   ShapeParameterExists(const TString & _shapeParameterName) const;
  private : 
    bool                                   YieldExists(const FitParameterConfig & _yieldKey) const;
    template < typename T > void           PrintParametersInMap(const map< T, ParameterWrapper > & _parameterMap) const;
    void                                   AddYieldFormula(const FitParameterConfig & _yieldKey, RooAbsReal * _var1, RooAbsReal * _var2, const double _scale);
    void                                   AddYieldFormula(const FitParameterConfig & _yieldKey, RooAbsReal * _var1, RooAbsReal * _var2, RooAbsReal * _var3);
    void                                   AddYieldFormula(const FitParameterConfig & _yieldKey, RooAbsReal * _var1, RooAbsReal * _var2, RooAbsReal * _var3, RooAbsReal * _var4);
    void                                   AddYieldFormula(const FitParameterConfig & _yieldKey, RooAbsReal * _var1, RooAbsReal * _var2, RooAbsReal * _var3, RooAbsReal * _var4, RooAbsReal * _var5);
    TString                                GetFormulaString(int _nNumerators, int _nDenominators, double _scale);
    RooArgList                             GetArgList(RooAbsReal * _floatingYield, vector< RooAbsReal * > _numerators, vector< RooAbsReal * > _denominators);
    void                                   AddDoubleRatioYieldFormula(const FitParameterConfig & _yieldKey, RooAbsReal * _var1, RooAbsReal * _var2, RooAbsReal * _var3);
    bool                                   RemoveEfficiency(const FitParameterConfig & _efficiencyKey);
    bool                                   RemoveRatio(const FitParameterConfig & _ratioKey, const RatioType & _ratioType);
    void                                   ThrowIfYieldExists(const FitParameterConfig & _yieldKey, TString _callerName) const;
    void                                   AddFixedEfficiencyRatio(const FitParameterConfig & _numeratorKey, const FitParameterConfig & _denominatorKey, double _scale = 1);
    void                                   AddFixedEfficiency(const FitParameterConfig & _effKey);
    void                                   ThrowIfRatioDoesNotExist(const FitParameterConfig & _ratioKey, const RatioType & _ratioType, TString _callerName) const;
    bool                                   RatioExists(const FitParameterConfig & _ratioKey, const RatioType & _ratioType) const;
    void                                   AddHadronisationRatio(const FitParameterConfig & _configKey, const RatioType & _ratioType, double _value, double _error, const TString _name, bool _constrain);
    void                                   AddBranchingRatio(const FitParameterConfig & _configKey, double _numerator, double _denominator, double _numeratorError, double _denominatorError, const TString _name, bool _constrain);
    vector< FitParameterConfig >           GetEfficienciesInLikelihood(const RooArgList & _listOfLikelihoods) const;
    vector< RooRealVar * >                 GetUniqueVariablesInLikelihood(const RooArgList & _listOfLikelihoods) const;
    vector< FitParameterConfig >           GetMatchedEfficiencies(const vector< RooRealVar * > & _variablesInLikelihood) const;
    vector< vector< bool > >               GetEfficiencyAdjacencyMatrix(const vector< FitParameterConfig > & _efficiencyKeys) const;
    void                                   PrintAdjacencyMatrix(const vector< vector< bool > > & _adjacencyMatrix, const vector< FitParameterConfig > & _efficiencyKeys);
    vector< RooRealVar * >                 ExtractUncorrelatedEfficiencies(const vector< vector< bool > > & _adjacencyMatrix, const vector< FitParameterConfig > & _efficiencyKeys);
    vector< CorrelatedEfficienciesHolder > ExtractCorrelatedEfficiencies(const vector< vector< bool > > & _adjacencyMatrix, const vector< FitParameterConfig > & _efficiencyKeys);
    vector< vector< FitParameterConfig > > SubdivideCorrelatedEfficiencies(const vector< vector< bool > > & _adjacencyMatrix, const vector< FitParameterConfig > & _efficiencyKeys) const;
    vector< RooRealVar * >                 GetEfficiencyList(const vector< FitParameterConfig > & _efficiencyKeys);
    vector< vector< double > >             GetCovarianceMatrix(const vector< FitParameterConfig > & _correlatedEfficiencyKeys) const;
    vector< vector< double > >             InitialiseCovarianceMatrix(int _nEfficiency) const;
    double                                 GetVarianceForMatrix(const FitParameterConfig & _efficiencyKey) const;
    double                                 GetCovarianceForMatrix(const FitParameterConfig & _effKeyA, const FitParameterConfig & _effKeyB) const;
    vector< RooRealVar * >                 GetMatchedConstrainedParameters(vector< RooRealVar * > _variablesInLikelihood) const;  

  private:
    // Private methods used by FitParameterSnapshot
    // These methods should only be used when configuring a snapshot hence the friend class usage.
    friend class FitParameterSnapshot;
    //    friend void FitParameterSnapshot::ReloadParameters();
    map< FitParameterConfig, ParameterWrapper >                    GetEfficiencyMap() const;
    map< FitParameterConfig, ParameterWrapper >                    GetFixedEfficiencyMap() const;
    map< pair< FitParameterConfig, RatioType >, ParameterWrapper > GetRatioMap() const;
    map< FitParameterConfig, ParameterWrapper >                    GetYieldMap() const;
    map< TString, ParameterWrapper >                               GetShapeParameterMap() const;
    void                                                           LoadEfficiencyMap(const map< FitParameterConfig, ParameterWrapper > _efficiencyMap);
    void                                                           LoadFixedEfficiencyMap(const map< FitParameterConfig, ParameterWrapper > _efficiencyMap);
    void                                                           LoadRatioMap(const map< pair< FitParameterConfig, RatioType >, ParameterWrapper > _ratioMap);
    void                                                           LoadYieldMap(const map< FitParameterConfig, ParameterWrapper > _yieldMap);
    void                                                           LoadShapeParameterMap(const map< TString, ParameterWrapper > _shapeParameterMap);
    void                                                           LoadFloatingToFixedEfficiencyMap(const map< RooRealVar *, RooRealVar * > _floatingToFixedEfficiencyMap);
    void                                                           LoadConstrainedParameters( const vector< RooRealVar* > & _constrainedParameters);
  private:
    map< RooRealVar *, RooRealVar * >                              m_floatingToFixedEfficiencyMap;
    map< FitParameterConfig, ParameterWrapper >                    m_efficiencyMap;
    map< FitParameterConfig, ParameterWrapper >                    m_fixedEfficiencyMap;
    map< pair< FitParameterConfig, RatioType >, ParameterWrapper > m_ratioMap;
    map< FitParameterConfig, ParameterWrapper >                    m_yieldMap;
    map< TString, ParameterWrapper >                               m_shapeParameterMap;
    vector< RooRealVar * >                                         m_constrainedParameters;
    vector< RooRealVar * >                                         m_uncorrelatedEfficiencies;
    vector< CorrelatedEfficienciesHolder >                         m_correlatedEfficiencies;
    map< pair< FitParameterConfig, RatioType > , map< pair< FitParameterConfig, RatioType > , double >  > m_systematicsRatios;
};

/**
 * \brief      Printout operator.
 *
 * @param      os                 The operating system.
 * @param[in]  _fitParameterPool  The instance of FitParameterPool.
 *
 */
ostream & operator<<(ostream & os, const FitParameterPool & _fitParameterPool);

// The namespace to initialise FitParameterPool as a static global instance.
namespace RXFitter {
    weak_ptr< FitParameterPool > & GetWeakPointer();
    shared_ptr< FitParameterPool > GetParameterPool();
}   // namespace RXFitter

/**
 * \brief      Class to stream the fit parameters to disk and re-load them to a static instance of FitParameterPool.
 */
class FitParameterSnapshot : public TObject {
  public:
    /**
     * \brief      Default constructor.
     */
    FitParameterSnapshot();

    /**
     * \brief      Configures the efficiency, ratio, yield and shape maps to be saved by the FitParameterSnapshot.
     */
    void ConfigureSnapshotMap();

    /**
     * \brief      Loads the parameters saved by the FitParameterSnapshot to FitParameterPool.
     */
    void ReloadParameters();

  private:
    shared_ptr< FitParameterPool >                                 m_parameterPool;                  //! Global objects are not saved
    map< RooRealVar *, RooRealVar * >                              m_floatingToFixedEfficiencyMap;   // FloatingToFixedEfficiencyLinker
    map< FitParameterConfig, ParameterWrapper >                    m_fixedEfficiencyMap;             // Fixed Efficiencies
    map< FitParameterConfig, ParameterWrapper >                    m_efficiencyMap;                  // Efficiencies
    map< pair< FitParameterConfig, RatioType >, ParameterWrapper > m_ratioMap;                       // Ratio Parameters
    map< FitParameterConfig, ParameterWrapper >                    m_yieldMap;                       // Yields
    map< TString, ParameterWrapper >                               m_shapeParameterMap;              // Shape Parameters
    vector< RooRealVar * >                                         m_constrainedParameters;          // Constrained Parameters
    ClassDef(FitParameterSnapshot, 1);
};

#endif
