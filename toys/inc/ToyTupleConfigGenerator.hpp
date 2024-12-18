#ifndef TOYTUPLECONFIGGENERATOR_HPP
#define TOYTUPLECONFIGGENERATOR_HPP

#include "FitComponent.hpp"
#include "FitGenerator.hpp"
#include "FitHolder.hpp"
#include "FitManager.hpp"

#include "ToyTupleConfig.hpp"

/**
 * \class ToyTupleConfigGenerator
 * \brief Contructs ToyTupleConfig and stores them to suitable .root files.
 * \brief ToyTupleConfig can be constructed from:
 * \li FitManager
 * \li FitHolder
 * \li FitComponent and a nominal yield argument
 * \li ConfigHolder, RooRealVar, RooAbsPdf and nominal yield argument
 */
class ToyTupleConfigGenerator {

  public:
    /**
     * \brief Default constructor.
     */
    ToyTupleConfigGenerator();
    /**
     * \brief Copy constructor.
     */
    ToyTupleConfigGenerator(const ToyTupleConfigGenerator & _other);
    /**
     * \brief Move constructor.
     */
    ToyTupleConfigGenerator(ToyTupleConfigGenerator && _other);
    /**
     * \brief Destructor.
     */
    ~ToyTupleConfigGenerator(){};

    /**
     * \brief Constructs a ToyTupleConfig from the _nominalYield, PDF from FitComponent and Sample from the ConfigHolder in FitComponent.
     * \brief The ROOT file path is configured from the ConfigHolder of the FitComponent passed.
     */
    void UpdateTupleConfig(const FitComponent & _fitComponent, double _nominalYield);
    /**
     * \brief Constructs multiple instances of ToyTupleConfig, one per FitComponent found within the FitHolder argument.
     * \brief The ROOT file path is configured from the ConfigHolder (EventType) of the FitHolder passed.
     */
    void UpdateTupleConfig(const FitHolder & _fitHolder);
    /**
     * \brief Constructs multiple instances of ToyTupleConfig, one per FitComponent found.
     * \brief This function recursively calls ToyTupleConfigGenerator::UpdateTupleConfig(FitHolder & _fitHolder) for each FitHolder owned by the FitManager argument passed.
     */
    void UpdateTupleConfig(const FitManager & _fitManager);
    /**
     * \brief Constructs multiple instances of ToyTupleConfig, one per FitComponent found.
     * \brief This function recursively calls ToyTupleConfigGenerator::UpdateTupleConfig(FitManager & _fitManager) for each FitManager owned by the FitGenerator argument passed.
     */
    void UpdateTupleConfig(const FitGenerator & _fitGenerator);
    /**
     * \brief Constructs and saves a ToyTupleConfig using the ConfigHolder, RooRealVar, RooAbsPdf and nominal yield argument passed.
     * \brief The ROOT file path is configured from the ConfigHolder (EventType) passed.
     * \brief Has a different API so that it can be easily used in python scripts (rather than compiling another C++ executable).
     */
    void UpdateTupleConfigWithPDF(const ConfigHolder & _configHolder, const TString & _componentKey, const RooRealVar & _observable, RooAbsPdf & _pdf, double _nominalYield);

    /**
     * \brief Constructs a ToyTupleConfig from the _nominalYield, PDF from FitComponent and Sample from the ConfigHolder in FitComponent.
     * \brief The ROOT file path is configured from the ConfigHolder of the FitComponent passed.
     */
    void OverwriteTupleConfig(const FitComponent & _fitComponent, double _nominalYield);
    /**
     * \brief Constructs multiple instances of ToyTupleConfig, one per FitComponent found within the FitHolder argument.
     * \brief The ROOT file path is configured from the ConfigHolder (EventType) of the FitHolder passed.
     */
    void OverwriteTupleConfig(const FitHolder & _fitHolder);
    /**
     * \brief Constructs multiple instances of ToyTupleConfig, one per FitComponent found.
     * \brief This function recursively calls ToyTupleConfigGenerator::OverwriteTupleConfig(FitHolder & _fitHolder) for each FitHolder owned by the FitManager argument passed.
     */
    void OverwriteTupleConfig(const FitManager & _fitManager);
    /**
     * \brief Constructs and saves a ToyTupleConfig using the ConfigHolder, RooRealVar, RooAbsPdf and nominal yield argument passed.
     * \brief The ROOT file path is configured from the ConfigHolder (EventType) passed.
     */
    void OverwriteTupleConfig(const FitGenerator & _fitGenerator);

    void OverwriteTupleConfig(const ConfigHolder & _configHolder, const TString & _componentKey, const RooRealVar & _observable, RooAbsPdf & _pdf, double _nominalYield);

  private:
    void AddComponent(const FitComponent & _fitComponent, const TString _componentKey, double _nominalYield);
    void AddConfig(const TString & _componentKey, const RooRealVar & _observable, RooAbsPdf & _pdf, double _nominalYield, const TString & _description);
    void ExtractConfigHolder(const FitComponent & _fitComponent);
    void ExtractComponents(const FitHolder & _fitHolder);
    void ExtractSignal(const FitHolder & _fitHolder);
    void ExtractBackgrounds(const FitHolder & _fitHolder);
    void ExtractComponent(const FitComponentAndYield & _componentAndYield, const TString _componentKey);
    void ThrowIfObservableNotInPdf(const RooRealVar & observable, const RooAbsPdf & pdf) const;
    void UpdateComponents(TString _key = "");
    void OverwriteComponents(TString _key = "");
    void DeleteCurrentComponents();

  private:
    ConfigHolder m_configHolder;

    vector< ToyTupleConfig > m_configs;
};

#endif