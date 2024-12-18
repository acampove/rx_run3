#ifndef FITMANAGER_HPP
#define FITMANAGER_HPP

#include "EventType.hpp"
#include "FitterSvc.hpp"
#include "HelperSvc.hpp"
#include "MessageSvc.hpp"

#include "FitConfiguration.hpp"
#include "FitHolder.hpp"
#include "FitParameterPool.hpp"

#include <algorithm>
#include <iostream>
#include <time.h>
#include <vector>

#include "TObject.h"
#include "TString.h"

#include "RooAbsReal.h"
#include "RooRealVar.h"

class FitterTool;

/**
 * \class FitManager
 * \brief Fit info
 */
class FitManager : public TObject {

  public:
    // The underlying fitter tool for a single FitManager fit
    FitterTool * m_fitter = nullptr;   //!

  private:
    bool m_debug = false;

    vector< FitConfiguration > m_configurationsMM = {};   //<holder for the FitConfigurations used to build MM-FitHolders
    vector< FitConfiguration > m_configurationsEE = {};   //<holder for the FitConfigurations used to build EE-FitHolders

    TString m_name;     // Name of the FitManager
    TString m_option;   // Option for the FitManager

    Str2HolderMap m_holdersMM;   // The Holder for the Muon fit categories
    Str2HolderMap m_holdersEE;   // The Holder for the Electron fit categories

    Str2HolderBremMap m_holdersEEBrem;   // The Holder for Electrons Brem categories which gets merged for the full fit

    shared_ptr< FitParameterPool > m_parameterPool;       //! Global object not saved to disk.
    FitParameterSnapshot           m_parameterSnapshot;   // A snapshot of parameters used by this FitManager. Reloads the parameters to FitParameterPool.

    bool m_isInitialized = false;

    bool m_isLoaded = false;

    bool m_isReduced = false;

  public:
    /**
     * \brief Default constructor
     */
    FitManager() = default;

    /**
     * \brief Constructor with TString and FitConfiguration
     */
    FitManager(TString _name, vector< FitConfiguration > _configurations, TString _option);

    /**
     * \brief Constructor with TString
     */
    FitManager(TString _name, TString _option);

    /**
     * \brief Copy constructor
     */
    FitManager(const FitManager & _fitManager);

    /**
     * \brief Constructor with LoadFromDisk
     */
    FitManager(TString _managerName, TString _option, TString _name, TString _dir);

    /**
     * \brief Equality checkers
     */
    bool operator==(const FitManager & _fitManager) const { return (Name() == _fitManager.Name() && Option() == _fitManager.Option() && ConfigurationsMM() == _fitManager.ConfigurationsMM() && ConfigurationsEE() == _fitManager.ConfigurationsEE()); };

    bool operator!=(const FitManager & _fitManager) const { return !((*this) == _fitManager); };

    /**
     * \brief Operator access to the FitHolder of the FitManager
     * @param  _keyHolder [description]
     */
    FitHolder & operator[](const TString & _keyHolder);

    /**
     * \brief  Fit name
     */
    const TString Name() const noexcept { return m_name; };

    /**
     * \brief  Fit option
     */
    const TString Option() const { return m_option; };

    vector< FitConfiguration > ConfigurationsMM() const noexcept { return m_configurationsMM; };
    vector< FitConfiguration > ConfigurationsEE() const noexcept { return m_configurationsEE; };
    vector< FitConfiguration > Configurations() const noexcept;

    /**
     * \brief  MM FitHolder map
     */
    Str2HolderMap HoldersMM() const { return m_holdersMM; };

    /**
     * \brief  EE FitHolder map
     */
    Str2HolderMap HoldersEE() const { return m_holdersEE; };

    /**
     * \brief  EE brem FitHolder map
     */
    Str2HolderBremMap HoldersEEBrem() const { return m_holdersEEBrem; };

    /**
     * \brief Holders map [merge MM and EE maps]
     */
    Str2HolderMap Holders() const;

    /**
     * \brief Retrieve the FitterTool object used for the fit
     */
    FitterTool * Fitter() const { return m_fitter; };
    FitterTool * Fitter() { return m_fitter; };

    /**
     * \brief Print MM FitHolder map
     */
    void PrintHoldersMM() const noexcept;

    /**
     * \brief Print EE FitHolder map
     */
    void PrintHoldersEE() const noexcept;

    /**
     * \brief Init
     */
    void Init();

    bool IsInitialized() const { return m_isInitialized; };

    bool IsLoaded() const { return m_isLoaded; };

    bool IsReduced() const { return m_isReduced; };

    void SetStatus(bool _isLoaded, bool _isReduced);

    void RefreshParameterPool();

    /**
     * \brief Close
     */
    void Close();

    /**
     * \brief Create FitterTools
     */
    void CreateFitter();

    /**
     * \brief Fit
     */
    void Fit();

    /**
     * \brief Do SPlot
     */
    void DoSPlot();

    /**
     * \brief Add FitHolderMM
     * @param  _name      [description]
     * @param  _holder [description]
     */
    void AddFitHolderMM(TString _name, FitHolder * _holder);

    /**
     * \brief Add FitHolderEE
     * @param  _name      [description]
     * @param  _holder [description]
     */
    void AddFitHolderEE(TString _name, FitHolder * _holder);

    /**
     * \brief Add FitHolder
     * @param  _name      [description]
     * @param  _holder [description]
     */
    void AddFitHolder(TString _name, FitHolder * _holder);

    /**
     * \brief Prepare FitHolders
     */
    void Prepare();

    /**
     * \brief Finalize FitHolders
     */
    void Finalize();

    /**
     * \brief Init yield ranges for FitHolders
     */
    void InitRanges();

    /**
     * \brief Create Data for FitHolders
     */
    void CreateData();

    /**
     * \brief Print FitConfiguration
     */
    void PrintConfigurations() const noexcept;

    /**
     * \brief Print Keys name for the [] access
     */
    void PrintKeys() const noexcept;

    /**
     * \brief Print Details on the Actual PDF contained in the FitComponents
     */
    void PrintPDFs(TString _option = "");

    /**
     * \brief Save FitManager to disk
     */
    void SaveToDisk(TString _name = "", bool _verbose = false);

    /**
     * \brief Load FitManager from disk
     */
    void LoadFromDisk(TString _name = "", TString _dir = "");

    void ReduceComponents(TCut _cut);

    /**
     * \brief Save FitManager to log
     */
    void SaveToLog(TString _name = "") const noexcept;

    /**
     * \brief Save FitManager to YAML
     */
    void SaveToYAML(TString _name = "", TString _option = "") const noexcept;

  private:
    /**
     * \brief Check allowed arguments
     */
    bool Check() const noexcept;

    /**
     * \brief Activate debug
     * @param  _debug [description]
     */
    void SetDebug(bool _debug) { m_debug = _debug; };

    /**
     * \brief Check if in MM FitHolder map
     * @param  _name [description]
     */
    bool IsInMMHolderMap(TString _name) const noexcept;

    /**
     * \brief Check if in EE FitHolder map
     * @param  _name [description]
     */
    bool IsInEEHolderMap(TString _name) const noexcept;

    /**
     * \brief Add FitConfiguration
     * @param _configuration [description]
     */
    void AddFitConfiguration(const FitConfiguration & _configuration);

    /**
     * \brief Prepare FitHolders
     * @param  _holder        [description]
     * @param  _configuration [description]
     */
    void Prepare(FitHolder & _holder, const FitConfiguration & _configuration);

    /**
     * \brief Prepare Brem FitHolders
     * @param  _holder        [description]
     * @param  _configuration [description]
     */
    void PrepareBrem(FitHolder & _holder, const FitConfiguration & _configuration);

    /**
     * \brief Add Background, templated with the Background Type enumerator class
     * @param  _holder        [description]
     * @param  _configuration [description]
     */
    void AddBackgrounds(FitHolder & _holder, const FitConfiguration & _configuration);

    double GetBremFraction(EventType & _eventType);

    double ComputeBremFraction(EventType & _eventType);

    ClassDef(FitManager, 1);
};

ostream & operator<<(ostream & os, const FitManager & _fitManager);

#endif
