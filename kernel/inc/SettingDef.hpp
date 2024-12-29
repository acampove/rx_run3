#pragma once

#include "EnumeratorSvc.hpp"
#include "MessageSvc.hpp"
#include "StyleSvc.hpp"
#include <vector>
#include "TString.h"

using namespace std;

class EventType;
class FitConfiguration;
class EfficiencyForFitHandler;
/**
 * \namespace SettingDef
 * \brief Setting service
 */
namespace SettingDef {

    extern TString name;

    extern TString debug;

    extern bool verbose;

    extern bool trowLogicError;

    extern bool useMultiThread;

    extern TString separator;

    extern TString blindString;

    /**
     * \class IO
     * \brief IO settings
     */
    class IO {

      public:
        static TString exe;
        static TString yaml;
        static TString repoDir;
        static TString anaDir;

        static TString gangaDir;
        static TString ioDir;

        static TString configDir;
        static TString dataDir;

        static TString outDir;

        static bool useEOS;
        static bool useEOSHome;

        static TString eosDir;
        static TString eosHome;
    };

    /**
     * \class Config
     * \brief ConfigHolder settings
     */
    class Config {

      public:
        static TString project;
        static TString ana;
        static TString sample;

        static TString q2bin;

        static TString year;
        static TString polarity;

        static TString trigger;
        static TString triggerConf;

        static TString brem;
        static TString track;
    };

    /**
     * \class Cut
     * \brief CutHolder settings
     */
    class Cut {

      public:
        static TString option;
        static TString extraCut;
        static bool extraEEOnly;
        static bool extraMMOnly;
        static bool tightLowQ2;
        static bool force;
        static TString mvaVer;
    };

    /**
     * \class Weight
     * \brief WeightHolder settings
     */
    class Weight {

      public:
        static TString option;
        static TString config;

        static TString trkVer;
        static TString pidVer;
        static TString l0Ver;
        static TString hltVer;
        static TString mcVer;
        static TString q2SmearFileTag;
        static TString q2SmearDiffVar;
        static int     iBS;
        static bool    useBS;
        static bool    useStatusL0Formula;
        static bool    priorChain;
        static bool    usePIDPTElectron; 
        static bool    useMCRatioPID;
        static bool    TrkFromRKst;
        static bool    L0I_EToE;

    };

    /**
     * \class Tuple
     * \brief TupleHolder settings
     */
    class Tuple {

      public:
        static TString option;

        static TString gngVer;
        static TString proVer;
        static TString creVer;
        static TString splVer;
        static TString outVer;

        static TString fileName;
        static TString tupleName;

        static TString DT;
        static TString MCT;
        static TString ET;
        static TString LT;
        static TString SPT;
        static TString RST;
        static TString TT;
        static TString TST;

        static bool addTuple;
        static bool dataFrame;
        static bool chainexctrg;

        static bool              branches;
        static vector< TString > branchList;
        static bool              aliases;
        static bool              useURLS; 
        static double frac;

        static bool datasetCache;


        static vector< tuple< TString, vector< tuple< TString, int , double, double, TString ,TString > > > > isoBins;

        static bool noInit; //avoid TupleHolder::Init() to load TChains ( used for BS-fit-loop to fasten the procedure when reloading from Disk )

        static void ConfigureTupleLoading();   //configure the flags on noInit, addTuple, datasetCache in a coherent manner to load tuples
        static void ConfigureNoTupleLoading(); //configure the flags on noInit, addTuple, datasetCache in a coherent manner to not load tuples
    };

    /**
     * \class Events
     * \brief EventTypes settings
     */
    class Events {

      public:
        static TString cutOptionCL;
        static TString cutOptionSigMC;
        static TString cutOptionBkgMC;
        static TString weightOption;
        static TString tupleOption;

        static vector< EventType > types;
        static vector< TString >   fails;
    };

    /**
     * \class Efficiency
     * \brief EfficiencyType settings
     */
    class Efficiency {

      public:
        static TString option;

        static TString ver;

        static TString flatnessVer;

        static double minUncertainty;

        static EfficiencyForFitHandler fitconfiguration;

        static double scaleEfficiency;

        static double scaleSystematics;

    };

    /**
     * \class Fit
     * \brief FitComponent and Fit settings
     */
    class Fit {

      public:
        static TString option;

        static TString ver;

        static TString scan1DParameter;
        static bool    startLLScanFromMin;
        static int     nScanPointsProfile;
        static double  minValScan;
        static double  maxValScan;
        static bool    scanProfileManual;
      
        static bool doBinned;

        static int nBins;

        static TString varSchemeMC;
        static TString varSchemeCL;

        static double stepSize;
        static double stepSizePar;
        static double stepSizeYield;
        static double stepSizeRange;

        static bool splitL0Categories;
        static bool splitRunPeriods;

        static bool splitTrackCategories;

        static bool plotSumCategories;

        static bool blindYield;
        static bool blindEfficiency;
        static bool blindRatio;

        static bool reduceRooKeysPDF;

        static bool useDatasetCache;
        static bool redoDatasetCache;
        static bool useBremFracCache;
        static bool redoBremFracCache;

        static bool useRecursiveFractions; //RooAddPdf around is always done with Recursive Fractions for components and fits. 
        static bool useRooRealSumPDF; //RooAddPdf around is always done with Recursive Fractions for components and fits. 

        static TString dumpParamFile;
        static vector<TString> initialParamFile;

        static bool LocalCaches;


        static bool saveFitComponentCaches;
        static bool loadFitComponentCaches;
        static bool redoFitComponentCaches;
        
        static bool rareOnly;

        static bool useRatioComb;
        static bool useNumericalExpTurnOn;
        static bool LPTMCandidates; 
        static bool useMinuit2;
        static bool RatioParsMinos;
        static int  nCPUDataFit; 
        static bool CorrelateConstraintsNoNorm;
        
        static vector< FitConfiguration > configurations;
        static map< pair< Prj, Q2Bin >, pair< TString, TString > > yamls;        
        static bool useSPlot2;
        // 3 types of fits , mutually exclusive each other 
        // Fit for rJPsi 
        // Fit for RPsi
        // Fit for RX 
        // R-Ratio Systematics yamls covariance loaded from yaml generated from 
        // python/Systematic/ scripts.
        static bool rJPsiFit; 
        static bool RPsiFit; 
        static bool RXFit; 
        static bool MuonBranchingRatioFit; 

        static vector<TString> RatioSystFile; 
        static bool rJPsiFitWithSystematics();
        static bool RPsiFitWithSystematics();
        static bool RXFitWithSystematics();

        static int IndexBootTemplateMisID;
    };

    /**
     * \class Toy
     * \brief Toy settings
     */
    class Toy {

      public:
        static TString option;

        static TString tupleVer;
        static TString studyVer;

        static int nJobs;
        static int jobIndex;
        static int nToysPerJob;

        static bool mergeConfig; // merge output toy tuples to reduce number of files and keep things clean

        static bool Silent; 
        static bool CopyLocally; 
        static vector<TString> constraintOverwriteFile;
        static bool frozenOverwrite;
        static vector< FitConfiguration >                          configurations;
        static map< pair< Prj, Q2Bin >, pair< TString, TString > > yamls;

        static map< TString, map< TString, TString > > TupleConfigGeneratorInputs;

        static TString ReadFractionToysComponents;
        static TString configurationOverrideFile;
        
        static map<TString, map< TString, double> >  ReductionFactor;
    };

    typedef map<Prj, vector<TString>> SampleList;
    /**
     * \class AllowedConf
     * \brief Allowed settings
     */
    class AllowedConf 
    {
        public:
            /**
             * @brief Function used to initialize configuration from YAML files
             * @param conf_dir Directory containing the following configuration files:\n
             *    sample_run12.yaml \n
             *    sample_run3.yaml  \n
             */
            static void Initialize(const std::string &conf_dir);

            static const vector< TString >             Projects;
            static const vector< TString >             Analyses;

            static SampleList Samples_run12;
            static SampleList Samples_run3;
            static SampleList Samples;

            static const map< TString, TString >       TexSamples;   // Map for SampleToTex conversion
            static const vector< TString >             Q2Bins;
            static const vector< TString >             Years;
            static const vector< TString >             Polarities;
            static const vector< TString >             L0Categories;
            static const vector< TString >             L0Configurations;
            static const vector< TString >             BremCategories;
            static const vector< TString >             TrackCategories;

            static const vector< TString > CutOptions;
            static const vector< TString > WeightOptions;
            static const vector< TString > WeightConfig;
            static const vector< TString > TupleOptions;
            static const vector< TString > EfficiencyOptions;
            static const vector< TString > FitComponents;
            static const vector< TString > FitOptions;
        private:
            /**
             * @brief Will take path to yaml config and use it to fill the list of samples
             * @param conf_path Path to YAML file
             * @param samples empty container of samples, i.e. "map<Prj, <vector<TString>>>"
             */
            static void _ReadSamplesFromYaml(const std::string &conf_path, SampleList &samples);
            static SampleList& _MergeSamples(const SampleList &s1, const SampleList &s2);
    };

};   // namespace SettingDef

bool IsBATCH(TString _batch) noexcept;
bool HasEOS() noexcept;

void TrowLogicError(bool _flag) noexcept;
void PrintSettings() noexcept;
void PrintIOSettings() noexcept;
void PrintConfigSettings() noexcept;
void PrintCutSettings() noexcept;
void PrintWeightSettings() noexcept;
void PrintTupleSettings() noexcept;
void PrintEventsSettings() noexcept;
void PrintEfficiencySettings() noexcept;
void PrintFitSettings() noexcept;
void PrintToySettings() noexcept;
void PrintROOTSettings() noexcept;

/**
 * \namespace gRX
 * \brief gRX namespace (similar to gROOT) to retrieve global configurations
 */
namespace gRX {
    TString Project() noexcept;
    TString Analysis() noexcept;
    TString Sample() noexcept;
    TString Q2Bin() noexcept;
    TString Year() noexcept;
    TString Polarity() noexcept;
    TString Trigger() noexcept;
    TString TriggerConf() noexcept;
    TString Brem() noexcept;
    TString Track() noexcept;
};   // namespace gRX

