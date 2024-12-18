#ifndef FITGENERATOR_HPP
#define FITGENERATOR_HPP

#include "FitterSvc.hpp"
#include "HelperSvc.hpp"
#include "MessageSvc.hpp"

#include "FitConfiguration.hpp"
#include "FitManager.hpp"
#include "FitParameterPool.hpp"

#include <algorithm>
#include <iostream>
#include <time.h>
#include <vector>

#include "TObject.h"
#include "TString.h"

class FitterTool;

/**
 * \class FitGenerator
 * \brief Fit info
 */
class FitGenerator : public TObject {

  public:
    // The underlying fitter tool for a single FitGenerator fit
    FitterTool * m_fitter = nullptr;   //!

  private:
    bool m_debug = false;

    TString m_name;     // Name of the FitGenerator
    TString m_option;   // Option for the FitGenerator

    vector< FitConfiguration > m_configurations = {};

    Str2ManagerMap m_managers;

    shared_ptr< FitParameterPool > m_parameterPool;       //! Global objects are not saved
    FitParameterSnapshot           m_parameterSnapshot;   //

    bool m_saveToDisk = false;

    bool m_isInitialized = false;

    bool m_isLoaded = false;

    bool m_isReduced = false;

  public:
    /**
     * \brief Default constructor
     */
    FitGenerator() = default;

    /**
     * \brief Constructor with TString
     */
    FitGenerator(TString _name, TString _option);

    /**
     * \brief Copy constructor
     */
    FitGenerator(const FitGenerator & _fitGenerator);

    /**
     * \brief Constructor with LoadFromDisk
     */
    FitGenerator(TString _generatorName, TString _option, TString _name, TString _dir);

    /**
     * \brief Equality checkers
     */
    bool operator==(const FitGenerator & _fitGenerator) const { return (Name() == _fitGenerator.Name() && Option() == _fitGenerator.Option()); };

    bool operator!=(const FitGenerator & _fitGenerator) const { return !((*this) == _fitGenerator); };

    /**
     * \brief  Fit name
     */
    const TString Name() const noexcept { return m_name; };

    void SetName(TString _name) {
        m_name = _name;
        return;
    };

    /**
     * \brief  Fit option
     */
    const TString Option() const { return m_option; };

    vector< FitConfiguration > Configurations() const { return m_configurations; };

    /**
     * \brief Retrieve the FitManagers
     */
    Str2ManagerMap Managers() const { return m_managers; };

    FitHolder Holder();

    /**
     * \brief Retrieve the FitterTool object used for the fit
     */
    FitterTool * Fitter() const { return m_fitter; };
    FitterTool * Fitter() { return m_fitter; };

    void Parse();

    /**
     * \brief Add FitManager
     * @param _fitManager [description]
     */
    void AddFitManager(const FitManager & _fitManager);

    /**
     * \brief Init
     */
    void Init(TCut _cut = TCut(NOCUT), int _BSIDX = -1);

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
     * \brief Fit
     */
    void Fit();

    /*
     * \brief Logs the previous fit result into an nTuple
     */
    void LogFitResult();

    /**
     * \brief Prepare FitManager
     */
    void Prepare();

    /**
     * \brief Create FitManager Data
     */
    void CreateData();

    /**
     * \brief Modify FitManager Shapes
     */
    void ModifyShapes();

    /**
     * \brief Modify FitManager Yields
     */
    void ModifyYields();

    /**
     * \brief Create FitterTool
     */
    void CreateFitter();

    /**
     * \brief Save FitGenerator to disk
     */
    void SaveToDisk(TString _name = "", bool _force = false);

    /**
     * \brief Load FitGenerator from disk
     */
    void LoadFromDisk(TString _name = "", TString _dir = "");

    void ReduceComponents(TCut _cut);

    /**
     * \brief Save FitGenerator to log
     */
    void SaveToLog(TString _name = "") const noexcept;

    /**
     * \brief Save FitGenerator to YAML
     */
    void SaveToYAML(TString _name = "", TString _option = "") const noexcept;

  private:
    /**
     * \brief Activate debug
     * @param  _debug [description]
     */
    void SetDebug(bool _debug) { m_debug = _debug; };

    bool IsInManagerMap(TString _name) const noexcept;

    ClassDef(FitGenerator, 1);
};

ostream & operator<<(ostream & os, const FitGenerator & _fitGenerator);

#endif
