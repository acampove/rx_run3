#ifndef SETTINGDEF_CPP
#define SETTINGDEF_CPP

#include "SettingDef.hpp"

#include "WeightDefRX.hpp"

#include "EventType.hpp"
#include "FitConfiguration.hpp"
#include "EfficiencyForFitHandler.hpp"
namespace SettingDef {

    TString name = "";

    TString debug = "";

    bool verbose = false;

    bool trowLogicError = false;

    bool useMultiThread = true;

    TString separator = "-";

    TString blindString = "TheBlindingString";

}   // namespace SettingDef

//============== IO defaults ==================
TString SettingDef::IO::exe        = "";
TString SettingDef::IO::yaml       = "";
TString SettingDef::IO::repoDir    = (TString) getenv("REPOSYS");
TString SettingDef::IO::anaDir     = (TString) getenv("ANASYS");
TString SettingDef::IO::gangaDir   = (TString) getenv("REPOSYS") + "/tuples";
TString SettingDef::IO::ioDir      = (TString) getenv("IOSYS");
TString SettingDef::IO::configDir  = (TString) getenv("ANASYS") + "/yaml";
TString SettingDef::IO::dataDir    = (TString) getenv("ANASYS") + "/data";
TString SettingDef::IO::outDir     = "./";
bool    SettingDef::IO::useEOS     = false;
bool    SettingDef::IO::useEOSHome = false;
TString SettingDef::IO::eosDir     = (TString) getenv("EOSSYS");
TString SettingDef::IO::eosHome    = (TString) getenv("EOSHOME") + "/ewp-rkstz";

//============== Config defaults ==================
TString SettingDef::Config::project     = to_string(Prj::All);
TString SettingDef::Config::ana         = to_string(Analysis::All);
TString SettingDef::Config::sample      = "";
TString SettingDef::Config::q2bin       = to_string(Q2Bin::All);
TString SettingDef::Config::year        = to_string(Year::All);
TString SettingDef::Config::polarity    = to_string(Polarity::All);
TString SettingDef::Config::trigger     = to_string(Trigger::All);
TString SettingDef::Config::triggerConf = to_string(TriggerConf::Inclusive);
TString SettingDef::Config::brem        = to_string(Brem::All);
TString SettingDef::Config::track       = to_string(Track::All);

//============== Cut defaults ==================
TString SettingDef::Cut::option      = "";      //Change cuts on request with stirng-options, check implementations in CutHolderR(K,Kst,Phi,L....)
TString SettingDef::Cut::extraCut    = "";      //Manually pass a string cut to add on top of all 
bool    SettingDef::Cut::force       = false;   //Bypass some logics, be aware of its usage for some work-around to enable cutting even if forbidden by logic
bool    SettingDef::Cut::extraEEOnly = false;   //Extra cut (if specified) applied only on EE samples
bool    SettingDef::Cut::extraMMOnly = false;   //Extra cut (if specified) applied only on EE samples
bool    SettingDef::Cut::tightLowQ2  = false;   //Flag to enable a tighter q2 cut on Muon low RK and RKst samples to compare to published analysis 
TString SettingDef::Cut::mvaVer      = "";      // Version of the MVA classifiers

//============== Weight configuration defaults and allowed flags ==================
TString SettingDef::Weight::option = "";
TString SettingDef::Weight::config = "{B}_fit";
TString SettingDef::Weight::trkVer = "";   // Version of the TRK maps
TString SettingDef::Weight::pidVer = "";   // Version of the PID maps
TString SettingDef::Weight::l0Ver  = "";   // Version of the L0 maps
TString SettingDef::Weight::hltVer = "";   // Version of the HLT maps
TString SettingDef::Weight::mcVer  = "";   // Version of the MC maps
TString SettingDef::Weight::q2SmearFileTag = ""; //Tag of smearing factors =  see data/q2smearing/vgngVer/q2smearing_PRJ<THETAG>.yaml
TString SettingDef::Weight::q2SmearDiffVar = ""; //Tag of smearing factors =  see data/q2smearing/vgngVer/q2smearing_PRJ<THETAG>.yaml
int     SettingDef::Weight::iBS    = -1;   // Define set of bootstrapping weights to use (-1 = no bootstrapping)
bool    SettingDef::Weight::useBS  = false;// Enable Bootstrapping 


bool    SettingDef::Weight::useStatusL0Formula         = false; // By default do not use the L0 formula where weights are multiplied by status of the particle
bool    SettingDef::Weight::priorChain                 = false; // L0 and HLT weights for prior BDT chain

bool    SettingDef::Weight::usePIDPTElectron           = false; // PID maps for electron->electron with PT,ETA scheme 
bool    SettingDef::Weight::useMCRatioPID              = false; // PID maps electrons with data/sim weight ratios

bool    SettingDef::Weight::TrkFromRKst                = false; // Tracking maps from Bp or B0 mode
bool    SettingDef::Weight::L0I_EToE                   = false; // When grabbing maps on the fly use the L0I_EToE setup and use B_PT as input variable 

//============== Tuple defaults ==================
TString           SettingDef::Tuple::option       = "";
TString           SettingDef::Tuple::gngVer       = "";   // Tuple version - Ganga
TString           SettingDef::Tuple::proVer       = "";   // Tuple version - Process
TString           SettingDef::Tuple::creVer       = "";   // Tuple version - Create
TString           SettingDef::Tuple::splVer       = "";   // Tuple version - SPlot
TString           SettingDef::Tuple::outVer       = "";   // Tuple version - Output of iterative processing
TString           SettingDef::Tuple::fileName     = "";
TString           SettingDef::Tuple::tupleName    = "DT";
TString           SettingDef::Tuple::DT           = "DecayTuple";      // Decay Tuple
TString           SettingDef::Tuple::MCT          = "MCDecayTuple";    // MCDecay Tuple
TString           SettingDef::Tuple::ET           = "EventTuple";      // Event Tuple
TString           SettingDef::Tuple::LT           = "LumiTuple";       // Lumi Tuple
TString           SettingDef::Tuple::SPT          = "SPlotTuple";      // SPlot Tuple
TString           SettingDef::Tuple::RST          = "DecayTree";       // RapidSim Tuple
TString           SettingDef::Tuple::TT           = "ToyTuple";        // Toy Tuple
TString           SettingDef::Tuple::TST          = "ToyStudyTuple";   // ToyStudy Tuple
bool              SettingDef::Tuple::addTuple     = true;
bool              SettingDef::Tuple::dataFrame    = false;
bool              SettingDef::Tuple::chainexctrg  = false;
bool              SettingDef::Tuple::branches     = false;   // SetBranchStatus from cards
vector< TString > SettingDef::Tuple::branchList   = {};
bool              SettingDef::Tuple::aliases      = true;   // SetAlias from cards
double            SettingDef::Tuple::frac         = -1;
bool              SettingDef::Tuple::datasetCache = false;
bool              SettingDef::Tuple::noInit       = false; // Avoid TupleHolder::Init() to load TChains ( Used for BS-fit-loop to fasten the procedure when reloading from Disk the EventTypes private members )
bool              SettingDef::Tuple::useURLS      = false; 
vector< tuple< TString, vector< tuple< TString, int , double, double, TString , TString> > > > SettingDef::Tuple::isoBins = {};

void SettingDef::Tuple::ConfigureNoTupleLoading(){
    SettingDef::Tuple::noInit = true;
    SettingDef::Tuple::addTuple = false;    
}
void SettingDef::Tuple::ConfigureTupleLoading(){
    SettingDef::Tuple::noInit = false;
    SettingDef::Tuple::addTuple = true;    
}
//============== Events defaults ==================
TString             SettingDef::Events::cutOptionCL    = "";
TString             SettingDef::Events::cutOptionSigMC = "";
TString             SettingDef::Events::cutOptionBkgMC = "";
TString             SettingDef::Events::weightOption   = "";
TString             SettingDef::Events::tupleOption    = "";
vector< EventType > SettingDef::Events::types          = {};
vector< TString >   SettingDef::Events::fails          = {};

//============== Efficiency defaults ==================
TString SettingDef::Efficiency::option          = "";
TString SettingDef::Efficiency::ver             = "";// Efficiency version
TString SettingDef::Efficiency::flatnessVer     = "";// Flatness version [ useful to bookkep flatness sudies iterations for a gngVer]
double  SettingDef::Efficiency::minUncertainty   = 0.01;
double  SettingDef::Efficiency::scaleEfficiency  = 1.0;
double  SettingDef::Efficiency::scaleSystematics = 1.0;

EfficiencyForFitHandler SettingDef::Efficiency::fitconfiguration = EfficiencyForFitHandler();

//============== Fit defaults ==================
TString SettingDef::Fit::option               = "";
TString SettingDef::Fit::ver                  = "";   // Fit version
TString SettingDef::Fit::scan1DParameter      = "";   // Parameter to do 1D LL scans
bool    SettingDef::Fit::startLLScanFromMin   = true;
int     SettingDef::Fit::nScanPointsProfile   = 20;
double  SettingDef::Fit::minValScan           = -1.0;
double  SettingDef::Fit::maxValScan           = -1.0;
bool    SettingDef::Fit::scanProfileManual    = false;

bool    SettingDef::Fit::doBinned             = true;
int     SettingDef::Fit::nBins                = 100;
TString SettingDef::Fit::varSchemeMC          = "FitMC";
TString SettingDef::Fit::varSchemeCL          = "FitCL";
double  SettingDef::Fit::stepSize             = 0.01;   // Fraction of value (sort of obsolete, only MC fit)
double  SettingDef::Fit::stepSizePar          = 0.0001; // Fraction of range (sort of obsolete, only MC fit)
double  SettingDef::Fit::stepSizeRange        = 0.01;   // Fraction of range (sort of obsolete, only MC fit)
double  SettingDef::Fit::stepSizeYield        = 0.1;    // Fraction of Poisson uncertainty[WARNING, SEE FITTERTOOL.cpp logic] how step sizes are configured, not a unique rule to do it.... TO IMPROVE initial errors settings on parameters
bool    SettingDef::Fit::splitL0Categories    = true; //if Trg  == All, fits are run with L0I, L0L separated datasets
bool    SettingDef::Fit::splitRunPeriods      = false;//if Year == All, fits are run with R1,R2p1,R2p2 separated datasets
bool    SettingDef::Fit::splitTrackCategories = true; //
bool    SettingDef::Fit::plotSumCategories    = false;// < ENABLE IT IF ALL VAR ARE IN SAME RANGE FOR EE and MM in the FITHOLDER !
bool    SettingDef::Fit::blindYield           = false;
bool    SettingDef::Fit::blindEfficiency      = true;
bool    SettingDef::Fit::blindRatio           = true;
bool    SettingDef::Fit::reduceRooKeysPDF     = false;
bool    SettingDef::Fit::useDatasetCache      = false; //Load datasets previously made instead of ntuples
bool    SettingDef::Fit::redoDatasetCache     = false; //Recreate DataSetCaches, run once fits with this, and load after to get fast fits
bool    SettingDef::Fit::loadFitComponentCaches = false;
bool    SettingDef::Fit::saveFitComponentCaches = false;
bool    SettingDef::Fit::redoFitComponentCaches = false;
bool    SettingDef::Fit::rareOnly               = false;
bool    SettingDef::Fit::useRatioComb           = false; 
bool    SettingDef::Fit::useNumericalExpTurnOn  = false; 
bool    SettingDef::Fit::LocalCaches          = false; 
int     SettingDef::Fit::nCPUDataFit          = 1;
bool    SettingDef::Fit::LPTMCandidates       = true;  //Remove multiple candidates , given the full selection being applied, DO NOT RELY ON isSingle cuts.
bool    SettingDef::Fit::useBremFracCache     = false; //Compute Brem-Fractions and store/load from caches on eos
bool    SettingDef::Fit::redoBremFracCache    = false; //Compute Brem-Fractions and store/load from caches on eos

bool    SettingDef::Fit::useRecursiveFractions= true;//SO FAR ALWAYS LIKE THIS
bool    SettingDef::Fit::useRooRealSumPDF     = false;//SO FAR ALWAYS LIKE THIS
bool    SettingDef::Fit::useMinuit2           = false;//Fit with Minuit2 
bool    SettingDef::Fit::RatioParsMinos       = false;//once migrad, hesse run, run MINOS over the RX double ratio parameters(only). Used for asymmetric errors in toys


bool    SettingDef::Fit::CorrelateConstraintsNoNorm = true; //For multivariate gaussian constraint, do we want to use the constant factor for the offset removed from the likelihood?

vector<TString> SettingDef::Fit::initialParamFile     = {}; //Matching by name the parameters, can be used for both MC and MORE IMPORTANT SETTINGS as the Cross feed constraints. Also to stabilize the fits
TString SettingDef::Fit::dumpParamFile        = "params.yaml";

vector< FitConfiguration >                          SettingDef::Fit::configurations = {};
map< pair< Prj, Q2Bin >, pair< TString, TString > > SettingDef::Fit::yamls;

bool SettingDef::Fit::useSPlot2  = false;   // If fitting for sPLot, with simultaneous-EE/MM fit, do constraint the previously fitted yields for background shapes.

/*
    One of them to true, and a non-empy list of yamls will inject 
    in the fitter the r-ratio systematic from the covariances we created
*/
bool SettingDef::Fit::rJPsiFit = false ;
bool SettingDef::Fit::RPsiFit  = false  ;
bool SettingDef::Fit::RXFit    = false    ;
vector<TString> SettingDef::Fit::RatioSystFile = {};
bool SettingDef::Fit::rJPsiFitWithSystematics(){
    return (SettingDef::Fit::rJPsiFit && SettingDef::Fit::RatioSystFile.size() !=0);
}
bool SettingDef::Fit::RPsiFitWithSystematics(){
    return (SettingDef::Fit::RPsiFit && SettingDef::Fit::RatioSystFile.size() !=0);
}
bool SettingDef::Fit::RXFitWithSystematics(){
    return (SettingDef::Fit::RXFit && SettingDef::Fit::RatioSystFile.size() !=0);
}

/*
    Special flag to use KDE-shapes from Bootstrapped index
*/
int SettingDef::Fit::IndexBootTemplateMisID = -1;

//============== Toy defaults ===============
TString SettingDef::Toy::option                  = "";
TString SettingDef::Toy::tupleVer                = "";
TString SettingDef::Toy::studyVer                = "";
int     SettingDef::Toy::nJobs                   = 1;
int     SettingDef::Toy::jobIndex                = 0;
int     SettingDef::Toy::nToysPerJob             = 100;
vector<TString> SettingDef::Toy::constraintOverwriteFile = {};
bool    SettingDef::Toy::frozenOverwrite         = false;
bool    SettingDef::Toy::mergeConfig             = true;
bool    SettingDef::Toy::Silent                  = false;
bool    SettingDef::Toy::CopyLocally             = false;

TString SettingDef::Toy::ReadFractionToysComponents = "";
TString SettingDef::Toy::configurationOverrideFile  = "";

map<TString, map< TString, double> >  SettingDef::Toy::ReductionFactor; 

vector< FitConfiguration >                          SettingDef::Toy::configurations = {};
map< pair< Prj, Q2Bin >, pair< TString, TString > > SettingDef::Toy::yamls;

map< TString, map< TString, TString > > SettingDef::Toy::TupleConfigGeneratorInputs;

//============== AllowedConfigurations defaults ==================

/**
 * \brief Projects
 * @param "RKst" = B0 -> K*0 ll
 * @param "RK"   = B0 -> K+ ll
 * @param "RPhi" = Bs -> phi ll
 * @param "RL"   = Lb -> L ll
 * @param "RKS"  = B0 -> KS ll
 */
const vector< TString > SettingDef::AllowedConf::Projects = {to_string(Prj::All), to_string(Prj::RKst), to_string(Prj::RK), to_string(Prj::RPhi), to_string(Prj::RL), to_string(Prj::RKS)};

/**
 * \brief Analyses
 * @param "MM" = muons
 * @param "EE" = electrons
 * @param "ME" = lfv (only data)
 */
const vector< TString > SettingDef::AllowedConf::Analyses = {to_string(Analysis::All), to_string(Analysis::MM), to_string(Analysis::EE), to_string(Analysis::ME)};

/**
 * \brief Samples
 * @param "LPT"    = Data
 * @param "B","Lb" = MC
 */
const map< Prj, vector< TString > > SettingDef::AllowedConf::Samples = {
    // RUN ./scripts/testTupleLists.sh TO TEST IF ALL SAMPLES CAN BE LOADED
    // ADD MISSING SAMPLES
    // RUN testKernel.out TO SORT MC SAMPLES AND PASTE THE OUTPUT BELOW
    {Prj::RKst,
     {to_string(Sample::Empty),
      // Data
      to_string(Sample::Data), to_string(Sample::Data) + "_SBU", to_string(Sample::Data) + "_SBL", to_string(Sample::Data) + "SS", to_string(Sample::Data) + "SSHH",
      // MC Bd
      "Bd2D0XNuEE", "Bd2D0XNuMM", "Bd2DNuKstNuEE", "Bd2DNuKstNuMM", "Bd2DNuKstPiEE", "Bd2DPiEE", "Bd2DPiMM", "Bd2DstNuD0PiKPiEE", "Bd2DstNuDPiKPiEE", "Bd2DstNuDPiKPiMM", "Bd2KPiEE", "Bd2KPiEEvNoKst", "Bd2KPiJPsEE", "Bd2KPiJPsMM", "Bd2KPiMM", "Bd2KPiMMvNoKst", "Bd2KstEE", "Bd2KstEEvNOFLT", "Bd2KstEEvPS", "Bd2KstEEvPSQ2", "Bd2KstEtaEEG", "Bd2KstEtaEEGvQ2", "Bd2KstGEE", "Bd2KstGEEv08a", "Bd2KstGEEv08d", "Bd2KstGEEv08f", "Bd2KstGEEv08h", "Bd2KstJPsEE", "Bd2KstJPsEEvNOFLT", "Bd2KstJPsMM", "Bd2KstJPsMMvNOFLT", "Bd2KstMM", 
      "Bd2KstMMvNOFLT", "Bd2KstPi0EEG", "Bd2KstPsiEE", "Bd2KPiPsiEE", "Bd2KstPsiEEvNOFLT", "Bd2KstPsiMM", "Bd2KPiPsiMM", "Bd2KstPsiMMvNOFLT", "Bd2XPsiEE",
      "Bd2KstPsiPiPiJPsEE", "Bd2KstSwapJPsEE", "Bd2KstSwapJPsMM", "Bd2KstSwapPsiEE", "Bd2KstSwapPsiMM", "Bd2KstJPsEESS", 
      // MC Bs
      "Bs2KPiJPsEE", "Bs2KPiJPsMM", "Bs2KstJPsEE", "Bs2KstJPsMM", "Bs2KstPsiEE", "Bs2KstPsiMM", "Bs2PhiEE", "Bs2PhiJPsEE", "Bs2PhiPsiEE", "Bs2PhiMM", "Bs2PhiJPsMM", "Bs2PhiPsiMM", "Bs2XJPsEE", "Bs2XJPsMM", "Bs2KsKstJPsEE",
      // MC Bu
      "Bu2DKNuNuEE", "Bu2K1EE", "Bu2K1MM", "Bu2K2EE", "Bu2K2MM", "Bu2KEE", "Bu2KJPsEE", "Bu2KJPsMM", "Bu2KMM", "Bu2KPiPiEE", "Bu2KPiPiJPsEE", "Bu2KPiPiJPsMM", "Bu2KPiPiMM", "Bu2KPiPiPsiEE", "Bu2KPiPiPsiMM", "Bu2KPsiEE", "Bu2KPsiMM", 
      "Bu2XJPsEE",
      "Bu2XJPsMM",
      "Bd2XJPsEE",
      "Bd2XJPsMM",
      "Bu2KPsiPiPiJPsEE",
      // MC Lb
      "Lb2XJPsEE", "Lb2XJPsMM", "Lb2pKEE", "Lb2pKJPsEE", "Lb2pKJPsMM", "Lb2pKMM", "Lb2pKPsiEE", "Lb2pKPsiMM", "Lb2LcPiEE","Lb2LcKEE","Lb2pKPiPiEE","Lb2pKKKEE",
      // Photon conversions
      "Bd2KstEtaGEE",
      "Bu2D0PiEE","Bu2K1GammaEE","Bd2KstPiPiEE","Bu2D0RhoEE","Bu2D0KPiRhoEE"
      }},
    {Prj::RK,
     {to_string(Sample::Empty),
      // Data
      to_string(Sample::Data), to_string(Sample::Data) + "_SBU", to_string(Sample::Data) + "_SBL", to_string(Sample::Data) + "SS",
      // MC Bd
      "Bd2K0EE", "Bd2K2EE", "Bd2KPiEE", "Bd2KPiEEvNoKst", "Bd2KPiMM", "Bd2KPiMMvNoKst", "Bd2KstEE", "Bd2KstJPsEE", "Bd2KstJPsMM", "Bd2KstMM", "Bd2KstPsiEE", "Bd2KPiPsiEE", "Bd2KstPsiMM", "Bd2KPiPsiMM", "Bd2XJPsEE", "Bd2XJPsMM", "Bd2XPsiEE",
      // MC Bs
      "Bs2XJPsEE", "Bs2XJPsMM", "Bu2KEtaPrimeGEE",
      // MC Bu
      "Bu2DKENuENuEE", "Bu2DKENuPiEE", "Bu2DKMuNuMuNuMM", "Bu2DKMuNuPiMM", "Bu2DKNuNuEE", "Bu2DKNuNuMM", "Bu2DKPiENuEE", "Bu2DKPiMuNuMM", "Bu2KEE", "Bu2KEEvL0", "Bu2KEEvMS", "Bu2KJPsEE", "Bu2KJPsEEv09c", "Bu2KJPsEESS", "Bu2KJPsMM", "Bu2KJPsMMv09b", "Bu2KJPsMMv09d", "Bu2KMM", "Bu2KMMvB0", "Bu2KMMvL0", "Bu2KPiPiOSEE", "Bu2KPiPiOSMM", "Bu2KPiPiSSEE", "Bu2KPiPiSSMM", "Bu2KPsiEE", "Bu2KPsiMM", "Bu2KPsiPiPiJPsEE", "Bu2KstEE", "Bu2KstEEvPS", "Bu2PiJPsEE", "Bu2PiJPsMM", "Bu2PiPsiEE", "Bu2PiPsiMM", "Bu2XJPsEE", "Bu2XJPsMM","Bu2KstMM",
      // MC Lb
      "Lb2XJPsEE", "Lb2XJPsMM", "Lb2pKEE", "Lb2pKJPsEE", "Lb2pKJPsMM", "Lb2pKMM", "Lb2pKPsiEE", "Lb2pKPsiMM", "Lb2LcPiEE","Lb2LcKEE","Lb2pKPiPiEE","Lb2pKKKEE",
      // MC KEta for low q2 background 
      "Bu2KEtaPrimeGEE", "Bd2KstEtaGEE", "Bu2KPiEE",
      // Photon conversions
      "Bd2KPiPi0EE", "Bd2KPiGammaEE", "Bd2KstPi0EE", "Bd2KstGammaHighEE", "Bd2KstPi0GammaEE","Bu2D0PiEE","Bu2K1GammaEE","Bd2Kst1410GammaEE","Bd2DPiEE","Bu2KPiPiOSEE","Bd2KstPiPiEE","Bu2D0RhoEE","Bu2D0KPiRhoEE"
      }},
    {Prj::RPhi,
     {to_string(Sample::Empty),
      // Data
      to_string(Sample::Data), to_string(Sample::Data) + "_SBU", to_string(Sample::Data) + "_SBL", to_string(Sample::Data) + "SS", to_string(Sample::Data) + "SSHH",
      // MC Bd
      "Bd2KstEE", "Bd2KstJPsEE", "Bd2KstJPsMM", "Bd2KstMM", "Bd2PhiJPsEE", "Bd2PhiJPsMM",
      // MC Bs
      "Bs2PhiEE", "Bs2PhiJPsEE", "Bs2PhiJPsMM", "Bs2PhiMM", "Bs2PhiPsiEE", "Bs2PhiPsiMM",
      // MC Lb
      "Lb2pKEE", "Lb2pKJPsEE", "Lb2pKJPsMM", "Lb2pKMM", "Lb2pKPsiEE", "Lb2pKPsiMM"}},
    {Prj::RL,
     {to_string(Sample::Empty),
      // Data
      to_string(Sample::Data), to_string(Sample::Data) + "-SBU", to_string(Sample::Data) + "-SBL",
      // MC Bd
      "Bd2KSEE", "Bd2KSJPsEE", "Bd2KSJPsMM", "Bd2KSMM", "Bd2KSPsiEE", "Bd2KSPsiMM",
      // MC Lb
      "Lb2LEE", "Lb2LJPsEE", "Lb2LJPsMM", "Lb2LMM", "Lb2LPsiEE", "Lb2LPsiMM", "Lb2LME", "Lb2LJPsME", "Lb2LPsiME"}},
    {Prj::RKS,
     {to_string(Sample::Empty),
      // Data
      to_string(Sample::Data), to_string(Sample::Data) + "-SBU", to_string(Sample::Data) + "-SBL",
      // MC Bd
      "Bd2KSJPsMM",
      // MC Bs
      "Bs2KSJPsMM",
      // MC Lb
      "Lb2LJPsMM"}}};

/**
 * \brief SampleToTex conversion (underlying particles defined in StyleSvc::Tex)
 */
const map< TString, TString > SettingDef::AllowedConf::TexSamples = {
    {to_string(Sample::Empty), "Empty"},
    {to_string(Sample::Data), "Data"},
    {to_string(Sample::Data) + "SS", "SS data"},

    {"Bd2KstMM", Tex::B0 + Tex::rightarrow + Tex::Kstar0 + Tex::MM},
    {"Bd2KstEE", Tex::B0 + Tex::rightarrow + Tex::Kstar0 + Tex::EE},
    {"Bd2KstEEvPS", Tex::B0 + Tex::rightarrow + Tex::Kstar0 + Tex::EE + "Phase space"},
    {"Bd2KstJPsMM", Tex::B0 + Tex::rightarrow + Tex::Kstar0 + Tex::JPsi + "(" + Tex::MM + ")"},
    {"Bd2KstJPsEE", Tex::B0 + Tex::rightarrow + Tex::Kstar0 + Tex::JPsi + "(" + Tex::EE + ")"},
    {"Bd2KstPsiMM", Tex::B0 + Tex::rightarrow + Tex::Kstar0 + Tex::Psi + "(" + Tex::MM + ")"},
    {"Bd2KstPsiEE", Tex::B0 + Tex::rightarrow + Tex::Kstar0 + Tex::Psi + "(" + Tex::EE + ")"},

    {"Bs2KstJPsMM", Tex::Bs + Tex::rightarrow + Tex::Kstar0 + Tex::JPsi + "(" + Tex::MM + ")"},
    {"Bs2KstJPsEE", Tex::Bs + Tex::rightarrow + Tex::Kstar0 + Tex::JPsi + "(" + Tex::EE + ")"},
    {"Bs2KstPsiMM", Tex::Bs + Tex::rightarrow + Tex::Kstar0 + Tex::JPsi + "(" + Tex::MM + ")"},
    {"Bs2KstPsiEE", Tex::Bs + Tex::rightarrow + Tex::Kstar0 + Tex::JPsi + "(" + Tex::EE + ")"},

    {"Lb2pKJPsMM", Tex::Lambdab + Tex::rightarrow + Tex::Proton + Tex::Kaon + Tex::JPsi + "(" + Tex::MM + ")"},
    {"Lb2pKJPsEE", Tex::Lambdab + Tex::rightarrow + Tex::Proton + Tex::Kaon + Tex::JPsi + "(" + Tex::EE + ")"},
    {"Lb2pKPsiMM", Tex::Lambdab + Tex::rightarrow + Tex::Proton + Tex::Kaon + Tex::Psi + "(" + Tex::MM + ")"},
    {"Lb2pKPsiEE", Tex::Lambdab + Tex::rightarrow + Tex::Proton + Tex::Kaon + Tex::Psi + "(" + Tex::EE + ")"},

    {"Bd2KstGEE", Tex::B0 + Tex::rightarrow + Tex::Kstar0 + "\\gaMMa" + "(" + Tex::EE + ")"},
    {"Bd2KstGEEv08a", Tex::B0 + Tex::rightarrow + Tex::Kstar0 + "\\gaMMa" + "(" + Tex::EE + ")"},
    {"Bd2KstGEEv08d", Tex::B0 + Tex::rightarrow + Tex::Kstar0 + "\\gaMMa" + "(" + Tex::EE + ")"},
    {"Bd2KstGEEv08f", Tex::B0 + Tex::rightarrow + Tex::Kstar0 + "\\gaMMa" + "(" + Tex::EE + ")"},
    {"Bd2KstGEEv08h", Tex::B0 + Tex::rightarrow + Tex::Kstar0 + "\\gaMMa" + "(" + Tex::EE + ")"},

    {"Bu2KMM", Tex::Bp + Tex::rightarrow + Tex::KPlus + Tex::MM},
    {"Bu2KEE", Tex::Bp + Tex::rightarrow + Tex::KPlus + Tex::EE},
    {"Bu2KJPsMM", Tex::Bp + Tex::rightarrow + Tex::KPlus + Tex::JPsi + "(" + Tex::MM + ")"},
    {"Bu2KJPsMMv09b", Tex::Bp + Tex::rightarrow + Tex::KPlus + Tex::JPsi + "(" + Tex::MM + ")"},
    {"Bu2KJPsEE", Tex::Bp + Tex::rightarrow + Tex::KPlus + Tex::JPsi + "(" + Tex::EE + ")"},
    {"Bu2KPsiMM", Tex::Bp + Tex::rightarrow + Tex::KPlus + Tex::Psi + "(" + Tex::MM + ")"},
    {"Bu2KPsiEE", Tex::Bp + Tex::rightarrow + Tex::KPlus + Tex::Psi + "(" + Tex::EE + ")"},

    {"Bs2PhiMM", Tex::Bs + Tex::rightarrow + Tex::Phi + Tex::MM},
    {"Bs2PhiEE", Tex::Bs + Tex::rightarrow + Tex::Phi + Tex::EE},
    {"Bs2PhiJPsMM", Tex::Bs + Tex::rightarrow + Tex::Phi + Tex::JPsi + "(" + Tex::MM + ")"},
    {"Bs2PhiJPsEE", Tex::Bs + Tex::rightarrow + Tex::Phi + Tex::JPsi + "(" + Tex::EE + ")"},
    {"Bs2PhiPsiMM", Tex::Bs + Tex::rightarrow + Tex::Phi + Tex::Psi + "(" + Tex::MM + ")"},
    {"Bs2PhiPsiEE", Tex::Bs + Tex::rightarrow + Tex::Phi + Tex::Psi + "(" + Tex::EE + ")"},

    {"Bu2KPiPiEE", Tex::Bp + Tex::rightarrow + Tex::KPlus + Tex::PiPlus + Tex::PiMinus + Tex::EE},
    {"Bu2K1EE", Tex::Bp + Tex::rightarrow + Tex::K1 + Tex::EE},
    {"Bu2K2EE", Tex::Bp + Tex::rightarrow + Tex::K2 + Tex::EE},
    {"Bu2KstEE", Tex::Bp + Tex::rightarrow + Tex::KstarP + Tex::EE},

    {"Bd2KstEtaEEG", Tex::B0 + Tex::rightarrow + Tex::Kstar0 + Tex::Eta + "(" + Tex::EE + Tex::Photon + ")"},
    {"Bd2KstPi0EEG", Tex::B0 + Tex::rightarrow + Tex::Kstar0 + Tex::Pi0 + "(" + Tex::EE + Tex::Photon + ")"},

    {"Bd2DNuKstNuEE", Tex::B0 + Tex::rightarrow + Tex::Dminus + "(" + Tex::Kstar0 + "#it{e}^{#minus}" + Tex::anue + ")" + "e^{+}" + Tex::nue},

    {"Bs2KstJPsMM", Tex::Bs + Tex::rightarrow + Tex::Kstar0 + Tex::JPsi + "(" + Tex::MM + ")"},
    {"Bs2KstJPsEE", Tex::Bs + Tex::rightarrow + Tex::Kstar0 + Tex::JPsi + "(" + Tex::EE + ")"},
    {"Bs2KstPsiMM", Tex::Bs + Tex::rightarrow + Tex::Kstar0 + Tex::JPsi + "(" + Tex::MM + ")"},
    {"Bs2KstPsiEE", Tex::Bs + Tex::rightarrow + Tex::Kstar0 + Tex::JPsi + "(" + Tex::EE + ")"},

    {"Lb2pKJPsMM", Tex::Lambdab + Tex::rightarrow + Tex::Proton + Tex::Kaon + Tex::JPsi + "(" + Tex::MM + ")"},
    {"Lb2pKJPsEE", Tex::Lambdab + Tex::rightarrow + Tex::Proton + Tex::Kaon + Tex::JPsi + "(" + Tex::EE + ")"},
    {"Lb2pKPsiMM", Tex::Lambdab + Tex::rightarrow + Tex::Proton + Tex::Kaon + Tex::Psi + "(" + Tex::MM + ")"},
    {"Lb2pKPsiEE", Tex::Lambdab + Tex::rightarrow + Tex::Proton + Tex::Kaon + Tex::Psi + "(" + Tex::EE + ")"},
};

/**
 * \brief Q2 bins
 * @param "Low"     = 0.1-1.1
 * @param "Central" = 1.1-6.0
 * @param "High"    = MM: JPs_M>15, EE: JPs_TRACK_M>14
 * @param "JPsi"    =
 * @param "Psi"     =
 * @param "Gamma"   =
 */
const vector< TString > SettingDef::AllowedConf::Q2Bins = {to_string(Q2Bin::All), to_string(Q2Bin::Low), to_string(Q2Bin::Central), to_string(Q2Bin::High), to_string(Q2Bin::JPsi), to_string(Q2Bin::Psi)};

/**
 * \brief Years
 * @param "11"   = 2011
 * @param "11"   = 2012
 * @param "R1"   = 2011+2012
 * @param "15"   = 2015
 * @param "16"   = 2016
 * @param "R2p1" = 2015+2016
 * @param "17"   = 2017
 * @param "18"   = 2018
 * @param "R2p2" = 2017+2018
 */
const vector< TString > SettingDef::AllowedConf::Years = {to_string(Year::All), to_string(Year::Y2011), to_string(Year::Y2012), to_string(Year::Run1), to_string(Year::Y2015), to_string(Year::Y2016), to_string(Year::Run2p1), to_string(Year::Y2017), to_string(Year::Y2018), to_string(Year::Run2p2)};

/**
 * \brief Magnet polarities
 * @param "MD" = magnet down
 * @param "MU" = magned up
 */
const vector< TString > SettingDef::AllowedConf::Polarities = {to_string(Polarity::All), to_string(Polarity::MD), to_string(Polarity::MU)};

/**
 * \brief Trigger categories
 * @param "L0I" = L0 TIS
 * @param "L0L" = L0 Muon/Electron
 * @param "L0H" = L0 Hadron
 */
const vector< TString > SettingDef::AllowedConf::L0Categories = {to_string(Trigger::All), to_string(Trigger::L0I), to_string(Trigger::L0L), to_string(Trigger::L0H)};

/**
 * \brief Trigger configurations
 * @param "Inclusive"    = Trigger::L0I -> L0I, Trigger::L0L -> L0L
 * @param "Exclusive"    = Trigger::L0I -> L0I, Trigger::L0L -> L0L&&!L0I
 * @param "Exclusive2"   = Trigger::L0L -> L0L, Trigger::L0I -> L0I&&!L0L
 */
const vector< TString > SettingDef::AllowedConf::L0Configurations = {to_string(TriggerConf::Inclusive), to_string(TriggerConf::Exclusive), to_string(TriggerConf::Exclusive2)};

/**
 * \brief Brem categories
 * @param "0G" = 0 recovered brem-clusters
 * @param "1G" = 1 recovered brem-clusters
 * @param "2G" = 2 recovered brem-clusters
 */
const vector< TString > SettingDef::AllowedConf::BremCategories = {to_string(Brem::All), to_string(Brem::G0), to_string(Brem::G1), to_string(Brem::G2)};

/**
 * \brief Track categories
 * https://twiki.cern.ch/twiki/bin/view/LHCb/TupleToolTrackInfo
 * - 0 = Unknown
 * - 1 = Velo track
 * - 2 = 2D Velo track
 * - 3 = Long (or forward) track
 * - 4 = Upstream track
 * - 5 = Downstream track
 * - 6 = seed track
 * - 7 = muon track
 * - 8 = calorimeter cosmic track
 * - 9 = TT track
 * @param "LL" = Long-Long (hadrons)
 * @param "DD" = Down-Down (hadrons)
 * @param "TAG" = Tag&Probe method
 * @param "PRB" = Tag&Probe method
 */
const vector< TString > SettingDef::AllowedConf::TrackCategories = {to_string(Track::All), to_string(Track::LL), to_string(Track::DD), to_string(Track::TAG), to_string(Track::PRB)};

/**
 * \brief Cut options ('-' separated)
 * @param "no"           = no Cut
 * @param "noEXTRA"      = no Extra Cut
 * @param "noPID"        = no PID Cut
 * @param "FORCE"        = FORCE bypassing PID cut logic [ NEVER USE IT REALLY, FOR DEBUGGING ONLY] , EventType only related.
 * @param "ePID"         = use electron cut only
 * @param "PIDMeerkat"   = PID Cut based on resampled branches
 * @param "noTRG"        = no TRG Cut
 * @param "noL0"         = no L0 Cut
 * @param "noHLT"        = no HLT Cut
 * @param "noHLT1"       = no HLT1 Cut
 * @param "noHLT2"       = no HLT2 Cut
 * @param "noKSTMASS"    = no Kst mass Cut
 * @param "TrackMuonOR"   = Select muon mode in HLT1 using also HLTTrackMuon lines together to TrackMVA(Run2) and AllTrackL0(Run1)
 * @param "TrackMuonALONE"= Select muon mode in HLT1 using only HLTTrackMuon lines
 * @param "noTRACK"      = no TRACK Cut
 * @param "TCKADC"       = TCK ADC Cut
 * @param "L0TIS"        = TIS category grabber for L0 TISTOSSING
 * @param "L0TISL1"      = TIS category grabber for L0 TISTOSSING (L1 for Muons/Electrons L0L maps)
 * @param "L0TISL2"      = TIS category grabber for L0 TISTOSSING (L2 for Muons/Electrons L0L maps)
 * @param "L0TISTOS"     = TIS&TOS category grabber for L0 TISTOSSING
 * @param "L0TISLComb"   = TIS for combined TISTOSSING of di-lepton system
 * @param "L0TISTOSLComb"= TOS&TOS for comibined TISTOSSING of di-lepton system
 * @param "L0TISTOSLComb"= TOS&TOS for comibined TISTOSSING of di-lepton system
 * @param "TISTOSBREM0"  = Make TISTOSSING for L0 for E_hasBrem0
 * @param "TISTOSBREM1"  = Make TISTOSSING for L0 for E_hasBrem1
 * @param "NominalTAG"   = Nominal tag to use for L0 TISTOSSING
 * @param "HadronTAG"    = Hadron tag to use for L0 TISTOSSING
 * @param "LeptonTAG"    = Lepton tag to use for L0 TISTOSSING
 * @param "TISTOSHLT1"   = TISTOSSING only HLT1 decision
 * @param "TAGHLTORALL"  = Pick as tag for TISTOSSING HLT the OR of all available lines
 * @param "TAGHLTPHYS"   = Pick as tag for TISTOSSING HLT the HLT1 PHYS TIS , HLT2 PHYS TIS 
 * @param "HLT1TCK"      = HLT1TCK cut for BDT1 training
 * @param "noTCKCat"     = no TCKCat Cut to align Hlt1TrackMVA in MC
 * @param "noPS"         = no PreSelection Cut
 * @param "noMinLPET"    = no min lepton PT/ET Cut
 * @param "noMinHPT"     = no min hadron PT Cut
 * @param "noBKG"        = no Background Cut
 * @param "BKGwoPID"     = Background w/o PID Cut
 * @param "noPKGOverRecoedCentral" = Remove the overreconstructed KEE in central q2 of Electron mode RKst
 * @param "BKGCOS"       = cosThetaL Cut
 * @param "noSL"         = no semileptonic Cut
 * @param "HOP"          = add HOP mass cut (rare mode ONLY)
 * @param "HOPJPs"       = add HOP mass cut to JPSi mode as well as in central q2 range
 * @param "HOP2D"        = add 2D HOP mass cut (rare mode ONLY)
 * @param "noMVA"        = no MVA Cut
 * @param "addPR"        = add PartReco Cut (JPsEE only)
 * @oaram "noPRMVA"      = Given that you use the MVA cut , the PR MVA is not applied in the selection, only combinatorial MVA used
 * @param "noQ2"         = no Q2 Cut
 * @param "noBREM"       = no BREM Cut
 * @param "isSingle"     = remove multiple candidates
 * @param "isSingleRND"  = add the isSingle_RND cut forced
 * @param "noIsSingle"   = keep multiple candidates
 * @param "tm"           = truth match signal or background
 * @param "tmTight"      = truth match signal tight or background
 * @param "tmNoGhost"    = truth match signal no-ghost or background
 * @param "tmSig"        = truth match signal
 * @param "tmSigTight"   = truth match signal tight
 * @param "tmSigNoGhost" = truth match signal no-ghost
 * @param "tmSwap"       = truth match signal swap [CAT==30]
 * @param "tmIncludeSwap"= truth match signal swap (on top of others) [used for misID background studies]
 * @param "tmIncludeGhost"= truth match including ghosts on top ( agnostic to tmXXX)
 * @param "tmBkg"        = truth match background
 * @param "tmCustom"            = truth match by "Sample" name navigating TrueIDs / MotherIDs / GMotherIDs
 * @param "tmCustomSwap"        = truth match by "Sample" name navigating TrueIDs / MotherIDs / GMotherIDs with Swaps mis-ID for Hadrons (special for each sample)
 * @param "tmCustomWithSwap"    = truth match by "Sample" name navigating TrueIDs / MotherIDs / GMotherIDs with correct ID OR swaps misID
 * @param "tmCustomVeto2XSwap"  = For the 2XJPs samples , apply selection vetoing the MisID cases or Decay  in Flight 
 * @param "tmCustomVetoPsi2JPsX"= Add veto on 2XJPs samples for the Psi-> J/Psi X decays.
 * @param "VetoGhost"         = Remove from candidates the BKGCAT60 category
 * @param "PRH"          = PartReco Hadronic truth matching added 
 * @param "PRL"          = PartReco Leptonic truth matching added
 * @param "massB"        = B mass Cut
 * @param "massBDTF"     = DTF B mass Cut applied on sample
 * @param "massBT"       = B mass Cut Tight
 * @param "massBL"       = B mass Cut Loose
 * @param "SBU"          = upper side-band
 * @param "SBL"          = lower side-band
 * @param "NORM"         = normalization cut w/o BKGCAT60 , DEPRECATED
 * @param "NORM60"       = normalization cut w/  BKGCAT60 , DEPRECATED
 * @param "OneGhost"        = Require there is at most 1 Ghost track in the reconstructed candidate
 * @param "OneGhostNMatches"= on top of BKGCAT selection for signal, it also requires that SignalLikeness >= (n-finalstates -1 ) and at most 1 Ghost (TRUEID=0) in the decay tree
 * @param "MatchCat60Mothers" = on top of BKGCAT selection for signal, requires that n-1 final states match the DecChain properly ( valid only for IsSignalMC() samples )
 * @param "CustomSignalLikeness"= on top of BKGCAT selection for signal, it also requires that SignalLikeness >= (n-finalstates -1 ) and at most 1 Ghost (TRUEID=0) in the decay tree. Done without SignalLikeness branch available.
 * @param "HLT1AllTrackL0AlignedTOS" = switch to use new Alignment branch for HLT1 decision in MC 2012
 * @param "noMuIsMuon" =  drop IsMuon among pre-selection cuts on muon mode [ to use when doing wIsMuon with weights ] 
 * @param "noTopoMuHLT2" = drop HLT2TopoMuon selection lines.
 * @param "q2SmearBp" = cut on JPS_M from smearing Bp JPS mass
 * @param "q2SmearB0" = cut on JPS_M from smearing B0 JPS mass
 * @param "q2SmearMCDTBp" = cut on JPS_M from smearing Bp JPs mass fits using as TRUE_M of JPs for the formula the MCDT_POSTFSR true mass ( i.e the L1+L2 true kinematics )
 * @param "q2SmearMCDTB0" = cut on JPS_M from smearing Bp JPs mass fits using as TRUE_M of JPs for the formula the MCDT_POSTFSR true mass ( i.e the L1+L2 true kinematics )
 * @param "MVAR1"   = cut for MVA in Run1   independent of year [used for cocktail maker with mixed years around]
 * @param "MVAR2p1" = cut for MVA in Run2p1 independent of year [used for cocktail maker with mixed years around]
 * @param "MVAR2p2" = cut for MVA in Run2p2 independent of year [used for cocktail maker with mixed years around]
 * @param "Brem0" = cut to construct brem0 cut with custom inner logic ( see CutHolderRKst )
 * @param "Brem1" = cut to construct brem1 cut with custom inner logic ( see CutHolderRKst )
 * @param "Brem2" = cut to construct brem2 cut with custom inner logic ( see CutHolderRKst )
 * @param "Brem01" = cut to construct brem0Or1 cut with custom inner logic ( see CutHolderRKst ) 
 * @param "Brem12" = cut to construct brem1Or2 cut with custom inner logic ( see CutHolderRKst )  * 
 * @param "MAXP"    = cut for all final states on Max(P) < 200E3 = boundaries of PID maps (( see actual implementation in GetPreSelectionCut in CutHolder RK,RKst) )
 * @param "combBveto"       = cut on JPs constr. B mass to remove peak on same sign data 
 * @param "XFeedKstIN"      = cut on Part-Recoed Kst masses (TRUE-ported from MCDT) for the Kst-narrow mass region [RK only]
 * @param "XFeedKstOUT"     = cut on Part-Recoed Kst masses (TRUE-ported from MCDT) for the Kst-wide mass region  [RK only]
 * @param "MVACentral"      = cut on J/Psi mode as in the MVACentral q2 ( EE only )
 * @param "MuonCentralMVA"  = cut on J/Psi mode as in the MuonCentralMVA q2 (MM only) )
 * @param "vetoPsi"         = cut on J/Psi mode vetoing the Psi->J/Psi cascade with a B_DTF_PSI_M cut.
 * @param "vetoJPs"         = cut on Psi2S mode vetoing the J/Psi resonance with a B_DTF_JPs_M cut.
 * @param "novetoPsi"         = cut on J/Psi mode vetoing the Psi->J/Psi cascade with a B_DTF_PSI_M cut.
 * @param "novetoJPs"         = cut on Psi2S mode vetoing the J/Psi resonance with a B_DTF_JPs_M cut.
 * @param "q2PsiWide"       = use wide q2 window for electron Psi2S.
 * @param "SSLep"           = cut on SSID for LPTSS samples ( same sign Leptonic ( ll K+ pi-(K-) ))
 * @param "SSHad"           = cut on SSID for LPTSS samples ( same sign Hadronic ( ll K+ pi-(K-) ))
 * @param "keepSSBKGCuts"   = For LPTSS samples try to keep as many BKG veto cuts as possible
 * @param "cutECALDistance" = add the cut for electrons on the ECAL distance
 * @param "alignR2p1HLT2"   = align HLT2 sleections of 15 and 16. Use carefully! this deviates from baseline selection (<1% effect)
 * @param "SPECIAL_RKLOW"   = For internal checks , remove HOP and PRMVA from low RK electron mode! 
 * @param "SPECIAL2_RKLOW"   = For internal checks , remove PRMVA from low RK electron mode! 
 * @param "SPECIAL3_RKLOW"   = For internal checks , remove all MVAs from low RK electron mode!
 * @param "SPECIAL4_RKLOW"   = For internal checks , soften HOP to 4700 and PRMVA to 0.20
 * @param "SPECIAL5_RKLOW"    = For internal checks , soften HOP to 4700 and remove PRMVA
 * @param q2All[MIN,MAX]"     = Customize q2 range definition ( JPs_M ) both EE/MM applied when q2 = All configured
 * @param q2Low[MIN,MAX]"     = Customize q2 range definition ( JPs_M ) both EE/MM applied when q2 = Low configured
 * @param q2Central[MIN,MAX]" = Customize q2 range definition ( JPs_M ) both EE/MM applied when q2 = Central configured
 * @param q2JPsi[MIN,MAX]"    = Customize q2 range definition ( JPs_M ) both EE/MM applied when q2 = JPsi configured
 * @param q2Psi[MIN,MAX]"     = Customize q2 range definition ( JPs_M ) both EE/MM applied when q2 = Psi configured
 * @param q2High[MIN,MAX]"    = Customize q2 range definition ( JPs_M ) both EE/MM applied when q2 = High configured 
 * @param q2Gamma[MIN,MAX]"   = Customize q2 range definition ( JPs_M ) both EE/MM applied when q2 = Gamma configured  ( only RKst , or Vectorial final states [TODO RPhi ? ])
 * @param SLCTL               = For RKst Central Q2 Electrons cut on top with Cos(ThetaL) < 0.6
 * @param SLCTLMKE            = For RKst Central Q2 Electrons cut on top with m(K+e-) > 1885 as done in RK
 * @param noOpeningAngles     = Avoid cutting on Opening angles final states ( useful for studying the gamma-q2)
 * @param  PIDELECTRON3       = PIDe>3 only
 * @param  PIDELECTRON2       = PIDe>2 only
 * @param  PIDELECTRON3_PROB4 = PIDe>3 & ProbNNe>0.4
 * ************ PID SCANS CHECK POST-UNBLINDING 
 * @param  PIDELECTRON2_PROB_0p2  = PIDe>2 & ProbNNe>0.2
 * @param  PIDELECTRON2_PROB_0p25 = PIDe>2 & ProbNNe>0.25
 * @param  PIDELECTRON2_PROB_0p3  = PIDe>2 & ProbNNe>0.3
 * @param  PIDELECTRON2_PROB_0p35 = PIDe>2 & ProbNNe>0.35
 * @param  PIDELECTRON2_PROB_0p4  = PIDe>2 & ProbNNe>0.4
 * @param  PIDELECTRON2_PROB_0p45 = PIDe>2 & ProbNNe>0.45
 * @param  PIDELECTRON2_PROB_0p5  = PIDe>2 & ProbNNe>0.50
 * @param  PIDELECTRON2_PROB_0p55 = PIDe>2 & ProbNNe>0.55
 * @param  PIDELECTRON2_PROB_0p6  = PIDe>2 & ProbNNe>0.60


 * NEW FEATURES 2024 
 * @param yaml[fileyaml:bitfield] = full dependency at run time of the cut-setup to use! Bypass everything else (needed in Run3)
 */

const vector< TString > SettingDef::AllowedConf::CutOptions = {
    // EXCLUDE COMMON CUTS
    "no", "noEXTRA", "noPID", "ePID", "noProbHH", "noProbEE", "noProbMM", "BKGwoPID", "noSPD", "noTRG", "noL0", "noHLT", "noHLT1", "noHLT2", "noTCKCat", "noPS", "noMinLPET", "noMinHPT", "noBKG", "noSL", "noMVA", "noPRMVA", "addPR", "noQ2", "noBREM", "noTRACK", "noKSTMASS",
    // EXCLUDE M_IsMuon in Prior PID 
    "noMuIsMuon", "noTopoMuHLT2",
    // INCLUDE COMMON CUTS
    "NORM", "NORM60", "PIDMeerkat", "BKGCOS", 
    "MVALOOSE", "HOP", "HOP2D", "HOPJPs",  
    "isSingle", "noIsSingle", "isSingleRND", "MVACentral",
    "MAXP", "combBveto", "SPlotnTracks", "vetoPsi", "vetoJPs", "q2PsiWide", "alignR2p1HLT2",
    // TRUTH MATCHING 
    "tm", "tmTight", "tmNoGhost", "tmSig", "tmSigTight", "tmSigNoGhost", 
    "tmSwap", "tmBkg", "tmCustom", "tmCustomSwap", "tmCustomWithSwap", "tmIncludeSwap", "tmIncludeGhost",
    "tmCustomVeto2XSwap", "VetoGhost", "tmCustomVetoPsi2JPsX","tmCustomVetoBu2Psi2JPsX",
    // SPECIAL 2XJPs on RKst 
    "rhoC", //Pi from Rho+
    "rhoZ", //Pi from Rho-0
    "norho",//Pi not from any Rho
    "B_X_K", 
    "B_K", 
    "B_X_JPs", 
    "B_X_Y_JPs",
    // PART RECO Hadronic,Leptonic, Both Truth matching
    "PRH", "PRL", "PRB",     
    // B Mass cuts
    "massB", "massBDTF", "massBT", "massBL", "SBU", "SBL",
    //TCKOption for priorchain
    "HLT1TCK", "TCKADC",
    //TISTOSSING L0 
    "L0TIS", "L0TISL1", "L0TISL2", "L0TISTOS", "L0TISTOSL1", "L0TISTOSL2", "L0TISLComb", "L0TISTOSLComb", "NominalTAG", "HadronTAG", "LeptonTAG",  "TISTOSBREM0", "TISTOSBREM1",
    //TISTOSSING HLT
    "TAGHLTORALL","TAGHLTPHYS" , "TISTOSHLT1",        
    //HLT muon mode cuts switches 
    "TrackMuonOR","TrackMuonALONE",
    // RPhi CUTS
    "noPhi", "noGhost", "noFid", "loosePhi", "loosePID",
    // RL RKS CUTS
    "BRMM",
    // Selection and efficiency studies
    "OneGhostNMatches" , "MatchCat60Mothers", 
    "HLT1AllTrackL0AlignedTOS" , "OneGhost",
    // SignalLikeness workaround
    "CustomSignalLikeness",
    // Q2 Smearing options for cuts on Q2 values
    "q2SmearBp", "q2SmearB0"  , "q2SmearMCDTBp", "q2SmearMCDTB0",
    // Brem customizer for fits in brem categories 
    "Brem0","Brem1","Brem2","Brem12", "Brem01",
    // Force MVA cuts from run periods
    "MVAR1", "MVAR2p1", "MVAR2p2",
    // MaxP < 200E3 on all particle species
    "XFeedKstIN",
    "XFeedKstOUT", 
    // Special for SS data in RKst/RPhi 
    "SSLep", "SSHad", "keepSSBKGCuts",
    "cutECALDistance", 
    "SPECIAL_RKLOW",
    "SPECIAL2_RKLOW",
    "SPECIAL3_RKLOW",
    "SPECIAL4_RKLOW",
    "SPECIAL5_RKLOW",
    //Same effect of Cut::force flag in EventType for PID setup 
    "FORCE", 
    "noPKGOverRecoedCentral",
    "Bs2DsXMassCentralEE", 
    "MuonCentralMVA",
    "q2All[",
    "q2Gamma[",
    "q2Low[",
    "q2Central[",
    "q2JPsi[",
    "q2Psi[",
    "q2High[",    
    //Special RKst central options for SL extra cuts on top
    "SLCTL",
    "SLCTLMKE", 
    //In review, align to RK-PIDe cuts (cut only on PIDe>3, no ProbNNe cut)
    "PIDELECTRON3", "PIDELECTRON2", 
    "PIDELECTRON3_PROB4",
    "noOpeningAngles",  // to study gamma q2 and very low 
    //PIDe > 2 && ProbNN > 0.x cut scan setups
    "PIDELECTRON2_PROB_0p2",
    "PIDELECTRON2_PROB_0p25",
    "PIDELECTRON2_PROB_0p3",
    "PIDELECTRON2_PROB_0p35",
    "PIDELECTRON2_PROB_0p4",
    "PIDELECTRON2_PROB_0p45",
    "PIDELECTRON2_PROB_0p5",
    "PIDELECTRON2_PROB_0p55",
    "PIDELECTRON2_PROB_0p6",
    "PIDELECTRON5_PROB_0p2",
    "PIDELECTRON5_PROB_0p25",
    "PIDELECTRON5_PROB_0p3",
    "PIDELECTRON5_PROB_0p35",
    "PIDELECTRON5_PROB_0p4",
    "PIDELECTRON5_PROB_0p45",
    "PIDELECTRON5_PROB_0p5",
    "PIDELECTRON5_PROB_0p55",
    "PIDELECTRON5_PROB_0p6",
    "PIDELECTRON7_PROB_0p2",
    "PIDELECTRON7_PROB_0p25",
    "PIDELECTRON7_PROB_0p3",
    "PIDELECTRON7_PROB_0p35",
    "PIDELECTRON7_PROB_0p4",
    "PIDELECTRON7_PROB_0p45",
    "PIDELECTRON7_PROB_0p5",
    "PIDELECTRON7_PROB_0p55",
    "PIDELECTRON7_PROB_0p6",
    "yaml["
};

/**
 * \brief Weight options ('-' separated)
 * @param "no"      = no Weight
 * @param "MCT"     = bypass checks to construct MCDT weights which only use BKIN-MULT
 * @param "NORM"    = bypass checks to construct MCDT weights to normalise w/o BKGCAT60
 * @param "NORM60"  = bypass checks to construct MCDT weights to normalise w/  BKGCAT60
 * @param "Meerkat" = bypass checks to construct weights when using PIDMeerkat
 * @param "TRK"     = Tracking
 * @param "PID"     = PID
 * @param "BKIN"    = Kinematics
 * @param "MULT"    = Multiplicity
 * @param "RECO"    = Reconstruction
 * @param "L0"      = L0 trigger 
 * @param "BREM"    = L0L(e) with BREM splitted up. [L0 By Brem studied, no effects]
 * @param "COMB"    = used with L0-COMB trigger  will give the same results on L0I , but on L0L we use the "dilepton" TISTOS ratios directly
 * @param "DIST"    = used with L0-DIST trigger  will give the same results on L0I , but on L0L we bin in ECAL distance between electrons instead of ECAL region
 * @param "HLT"     = HLT trigger
 * @param "nTracks" = HLT trigger with nTracks as input variable [use HLT-nTracks]
 * @param "BETA"    = HLT trigger with B_ETA as input variable   [use HLT-BETA   ]
 * @param "LB"      = m(pK) for Lb
 * @param "LB_KIN"   = Lb kinematic weights (wkin on Lb2pKJPs tuples ONLY for RKst )
 * @param "PTRECO"  = m(Kpi(pi))
 * @param "fromLOI" = pick BDT1 training from L0I 
 * @param "SP"         = SPlot
 * @param "ISMUON"     = IsMuon as weight instead of Cut
 * @param "SMEAR"      = PlaceHolder for TupleProcess
 * @param "SMEARBP"    = PlaceHolder Nickname (SmearBp , use with CutOption for Q2SmearBp)
 * @param "SMEARB0"    = PlaceHolder Nickname (SmearB0 , use with CutOption for Q2SmearB0)
 * @param "SMEARBPN"    = PlaceHolder Nickname (SmearBp , use with CutOption for Q2SmearBp), normalized, requires the Q2SmearNormalizer
 * @param "SMEARB0N"    = PlaceHolder Nickname (SmearB0 , use with CutOption for Q2SmearB0), normalized, requires the Q2SmearNormalizer
 * @param "PORTBRANCH" = Ship KST branches from MCDT to DT for Kst_TRUEM
 * @param "0G,1G,2G"   = Placeholders for efficiencyCreateFast to split Brem computation of efficiencies...
 * @param "XJPSWeight" = 2XJps weights to attach , see XJPsReweighter.cpp file
 * @param "LUMI"       = Lumi scaling of samples-shapes
 * @param "MODEL"      = DecayModel reweighting
 * @param "RW1D"       = BKIN-MULT reweighting with 1D histograms instead of BDT (similar to muon BR ana)
*/
const vector< TString > SettingDef::AllowedConf::WeightOptions = {"no", "noBS","MCT", "NORM", "NORM60", "Meerkat", "COMB", "DIST", "BREM", "0G","1G","2G", "RW1D" , WeightDefRX::ID::TRK, WeightDefRX::ID::PID, WeightDefRX::ID::L0, WeightDefRX::ID::HLT, WeightDefRX::ID::BKIN, WeightDefRX::ID::MULT, WeightDefRX::ID::RECO, WeightDefRX::ID::BDT, WeightDefRX::ID::LB, WeightDefRX::ID::LB_KIN, WeightDefRX::ID::PTRECO, WeightDefRX::ID::SP, WeightDefRX::ID::BS, "fromLOI" , WeightDefRX::ID::ISMUON, "nTracks", "BETA" , "SMEAR","SMEARBP", "SMEARB0", "PORTBRANCH", "XJPSWeight", "LUMI",
"SMEARBPN",
"SMEARB0N", 
"MODEL"};

/**
 * \brief Weight configuration to select specific maps ('_' separated)
 * @param "{B}"      = project dependent B
 * @param "nointerp" = binned (PID, L0)
 * @param "interp"   = interpolated (PID, L0)
 * @param "kde"      = KDE (PID) with interpolated (L0)
 * @param "meerkat"  = resampled PID response branches with Meerkat
 * @param "fit"      = fitted (L0) with interpolated (PID)
 */
const vector< TString > SettingDef::AllowedConf::WeightConfig = {"{B}", "Bp", "B0", "Bs", "Lb", "nointerp", "interp", "kde", "meerkat", "fit", "MML0Lincl", "MML0Lexcl"};

/**
 * \brief Tuples options ('-' separated)
 * @param "gng" = Ganga
 * @param "pro" = Process
 * @param "cre" = Create
 * @param "spl" = SPlot
 * @param "rap" = RapidSim
 */
const vector< TString > SettingDef::AllowedConf::TupleOptions = {"gng", "pro", "cre", "spl", "rap", "tmp", "chainexctrg"};

/**
 * \brief Efficiency options ('-' separated)
 * @param "gen"         = generator
 * @param "flt"         = filtering
 * @param "ISO"         = build all efficiencies in all Bins defined in the IsoBin.csv files using the globbing of yaml/fitter directory
 * @param "T&P"         = special field for fitting HLT TISTOS method
 * @param "SCAN"        = used in EfficiencyCreateFast.cpp to scan through All Efficiencies ( maybe to remove, OBSOLETE since not used)
 * @param "noSPD"       = do not cut when making efficiencies on MCDecayTuple on NSPD hits
 * @param "OnTheFly"      = Append Weight columns on the fly using HelperProcessing module  when running efficiency jobs
 * @param "OnTheFlyFitter"= Append Weight columns on the fly using HelperProcessing module  when running fits using Weighed MC shapes ( interplay to managers content ! ). See HelperSvc.cpp
 * @param "BremFrac"    = Compute efficiencies ALSO splitting by brem fractions the counting (for signal!)
 * @param "trueQ2"      = When computing efficiencies with efficiencyCreateFast.cpp , cut MCDecayTuple with the TRUE_JPS mass depending on q2 ( applied for signal only, see logic in efficiencyCreateFast.cpp executable)
 * @param "trueQ2PostFSR"      = When computing efficiencies with efficiencyCreateFast.cpp , cut MCDecayTuple with the TRUE_JPS mass from L1 + L2 
 * @param "HistoQ2"     = When computing efficiencies with efficiencyCreateFast.cpp , cut MCDecayTuple with the TRUE_JPS mass from L1 + L2 
 * @param "HistoBPT"    = Computing efficiencies with efficiencyCreateFast.cpp in bins of B PT
 * ==================== Efficiency Options when running fits configuring efficiency inputs ==================== 
 * Check the effect of those flags in EfficiencySvc.cpp 
 * @param "noCov"       = suppress all correlations in covariance matrix when fitting
 * @param "noRunCov"    = suppress all correlations among run periods in covariance matrix when fitting 
 * @param "noRunCov"    = suppress all correlations among Run periods in covariance matrix when fitting 
 * @param "noTrgCov"    = suppress all correlations among Trg periods in covariance matrix when fitting 
 * @param "noAnaCov"    = suppress all correlations among Analysis (EE-MM) periods in covariance matrix when fitting 
 * @param "noQ2Cov"     = suppress all correlations among Q2Bins periods in covariance matrix when fitting 
 * @param "noPrjCov"    = suppress all correlations among Projects in covariance matrix when fitting  
 * @param "forceEpsErrorsBkg" =  By-pass the minUncertainty threshold settings forcing relative errors on efficiencies on samples as described in EffiiencySvc::LoadEfficiencyForFit . To enable if the fit requires eps(Bkg-constraints) to be gaussian constrained and use a more "realistic" error
 */
const vector< TString > SettingDef::AllowedConf::EfficiencyOptions = {"", "gen", "flt", "ISO", "T&P", "SCAN", "noSPD","trueQ2", "BremFrac",
                                                                      "forceEpsErrorsBkg", "OnTheFly", "OnTheFlyFitter",  
                                                                      "noCov",     //no correlations among BS efficiencies AT ALL 
                                                                      "noRunCov",  //Suppress correlations among run periods
                                                                      "noTrgCov",  //Suppress correlations among triggers categories
                                                                      "noAnaCov",  //Suppress correlations among electrons and muons 
                                                                      "noQ2Cov",   //Suppress correlations among q2 bins 
                                                                      "noPrjCov",  //Suppress correlations among RK-RKst
                                                                      "noCovLeakage", //Remove the Leakage from the 2D covariances in the fitter
                                                                      "HistoQ2",      //Special flag to trigger in efficiencyCreateFast the plots making for eps(q2) dependencies
                                                                      "HistoBPT",     //Special flag to trigger in efficiencyCreateFast the plots making for eps(BPT) dependencies
                                                                      "trueQ2PostFSR"}; //For rare modes cut at denominator with the TRUE q2 post-Finalstate radiation (L1+L2)_TRUEM, instead of (B-Hadrons)_TRUEM
/**
 * \brief Fit components
 */
const vector< TString > SettingDef::AllowedConf::FitComponents = {"", "CB", "CBAndGauss", "Ipatia", "Ipatia2", "Exp"};

/**
 * \brief Fit options ('-' separated)
 * @param "forceSavebeforemod"     = force SaveToDisk beforemod for the fit.
 * @param "modshape"               = modify signal and background (Bs) PDF shape (mass shift and sigma scale)
 * @param "modshapejps"            = modify signal and background (Bs) PDF shape (mass shift and sigma scale) using JPs
 * @param "modshapejpsmm"          = modify signal and background (Bs) PDF shape (mass shift and sigma scale) using JPsMM
 * @param "modshapejpsee"          = modify signal and background (Bs) PDF shape (mass shift and sigma scale) using JPsEE
 * @param "modscalejpsmm"          = use JPs MM sigma scale for JPs EE
 * @param "modscaleLR"             = Apply a Sigma Scaling on Left/Right independently ( used for q2 correction TESTS)
 * @param "modscaletagprobe"       = share sigma scale on TAG , PRB for HLT corrections 
 * @param "modshifttagprobe"       = share mass shift on TAG, PRB for HLT corrections
 * @param "modshapetail"           = modify signal and background (Bs) PDF shape (tail scale)
 * @param "modshiftrx"             = share mass shift between RK and RK*.
 * @param "modscalerx"             = share width scale between RK and RK*.
 * @param "modshiftbrem"           = use a different mass shift for each electron brem category
 * @param "modscalebrem"           = use a different width scale for each electron brem category
 * @param "modbremgaussfrac"       = use to boost/shrink RHS gaussian component of 0G,1G,2G shapes when fitting signal shapes. TODO: implement the same on Brem-Merged fits.
 * @param "modshiftbremdifference" = parameterise mass shift for individual brem as a difference between them
 * @param "modshapefracbrem"       = modify signal and background (Bs) PDF shape (Finalize frac shift)E 
 * @param "modyieldsig"            = link signal EE yield to MM yield (e.g. r_JPs, R_Psi, R_Kst) in each L0 category and Run separately
 * @param "modyieldsigsingle"      = same as "modyieldsig" with a single Ratio EE/MM (r_)
 * @param "modyieldsigdouble"      = same as "modyieldsig" with a double Ratio EE/MM X/JPs (R_)
 * @param "modyieldsignotrg"       = same as "modyieldsig" but common among L0 categories
 * @param "modyieldsignoyr"        = same as "modyieldsig" but common among Runs
 * @param "modyieldbkg"            = link background yields (e.g. Lb, Bs between EE and MM, leakage between Central and JPs) in each L0 category separately w/o efficiencies
 * @param "modyieldbkgeff"         = link background yields (e.g. Lb, Bs between EE and MM, leakage between Central and JPs) in each L0 category separately w/ efficiencies
 * @param "modyieldbkgbs"          = link background yields (e.g. expected Bs yield from PDG)
 * @param "modyieldbkgbs2phi"      = link background yields (e.g. expected Bs2Phi yield from PDG)
 * @param "modyieldbkgpsi2jpsx"    = link background yields (e.g. expected Bx2HPsi_PiPiJPs_EE yield from PDG)
 * @param "modyieldbkgPRpsi2jpsx"  = link background yields (e.g. expected Bx2HPsi_PiPiJPs_EE yield from PDG in the part-reco noDTF fits!)
 * @param "modyieldbkgPRpsi2jpspipi"= ink background yields (e.g. expected Bu2Psi_PiPiJPs_EE in RKST only yield from PDG in the part-reco shapes (tandem to veto flags (TruthMatchingSvc for 2XJps samples!) fits)
 * @param "modyieldbkghadswap"     = link background yields (e.g. expected K <-> misID from Control mode yield)
 * @param "modyieldbkglb"          = link background yields (e.g. expected Lb yield from PDG)
 * @param "modyieldbkgmid"         = link background yields (e.g. expected MisID yield from PDG)
 * @param "modyieldbkgdslc"        = link background yields (e.g. expected Double Semileptonic background from PDG and efficiency to signal (J/Psi mode))
 * @param "modyieldbkgleak"        = Leakage in Psi(2S) mode fits from the JPsi->ee decay with efficiencies and Branching Ratios [to see what to do with the smearing!]
 * @param "modyieldbkgprleak"      = Leakage in PartReco J/Psi mode fits from the Psi->ee decay with efficiencies and Branching Ratios [to see what to do with the smearing!]
 * TODO : @param "modyieldbkgleakage"     = link background yields (e.g. expected MisID yield from PDG)
 * @param "fixshapebkg"            = If your fit let float some background shape parameter, and you want to constrain all of them, add this option (Comb background not floated)
 * @param "crossfeed"              = constrain PartReco in Bp from B0
 * @param "posyield"               = positively defined yields
 * @param "gconst"                 = gauss constrain constants fits [ fs/fd, Branching ratios]
 * @param "gconstCombSS"           = gauss constraint parameters from SS data gconstCombSS[a,b,c] <-- select parameters to gauss constraints
 * @param "gconstComb"             = gauss constraint parameters from SS data gconstComb[a,b,c] <-- select parameters to gauss constraints
 * @param "gconsteffsBkg"          = gauss constrain 1D efficiencies used for Bkg/Sig ratios [ Combine this with noCov in Efficiency::option ]
 * @param "gconsteffsFull"         = gauss constrain signal efficiencies for the r-Ratio fit [ covariances enabled automatically, shut the calculation with noCov in Efficiency::option ]
 * @param "chainexctrgsig"         = chain exclusive trigger categories
 * @param "chainexctrgbkg"         = chain exclusive trigger categories
 * @param "reduce"                 = add extra variables to RooDataSet to allow ReduceComponents
 * @param "splot"                  = perform sPlot
 * @param "toyconf"                = create ToyConfiguration
 * @param "dry"                    = dry run (only build components, no fit to data)
 * @param "drysig"                 = dry run (only build signal components, no fit to data)
 * @param "profileRatios"          = At the end of hte fit run the Likelihood profile of the r-Ratios parameters in the fit 
 * @param "saveWS"                 = At the end of hte fit run Save all to workspace (model + datasets)
 * @param "profileRatios2D"        = At the end of hte fit run the Likelihood profile of the r-Ratios parameters in the fit (2D on R-Ratios)
 * @param "initialHesse"           = Run Hesse Before Migrad to get better errors, ( only for FitGenerator::Fit() )
 * @param "reloadSaved"            = (NOT WORKING YET) FOR DEBUGGING, run fits with ```-forceSaveBeforeMod``` and then rerun it locally with reloadSaved (TODO, implement)
 * @param "SnpFitVarOnly"          = make a local snapshots of only fitting variables when creating datasets
 //SET OF VETO OPTIONS ON MODSHAPE FOR SHIFT/SCALE 
 * @param "noMSHIFTEE[PAR1,PAR2,...]"   = avoid to mass shift   things composing PDF EE shape, example -noMSHIFTEE[m,mg,mcb] for signal PDF EE will NOT M-SHIFT the mass, gaussian mass and mass of crystal ball... 
 * @param "noSSCALEEE[PAR1,PAR2,...]"   = avoid to sigma scale  things composing PDF EE shape, example -noSSCALEEE[s,sg,scb] for signal PDF EE will NOT s-SCALE the sigma, sigma gaussian of the crystal ball...
 * @param "freeParsEE[PAR1,PAR2]"       = (OBSOLETE) Remove constant of shape in fit for the parameter for PDF EE in Brem-splitted fits  (example -freeParsEE[fg] --> flexibly float the extra gaussian shape )
 * @param "freeParsEEBound[PAR1,PAR2]"  = (OBSOLETE) Remove constant of shape in fit for the parameter for PDF EE in Brem-splitted fits  (example -freeParsEE[fg] --> flexibly float the extra gaussian shape )
 * @param "free2XPR"                    = Free the relative fractions of part-reco components ( Bs, Bu, Bd ) 
 * @param "cutRangeCL"                  = An option used on 2XJPs samples inside the fitter to evaluate on the fly relative efficienceis accounting for the mass fir range 
 * @param "ShiftMassPar"                = When making RooKeyPdfs we shift the mass parameter according to the overall mass shift in data, barely used, see FitComponent.cpp for RooKeyPdfs making
 * @param "noLLOffset"                  = Disable offsetted Likelihood in FitGenerator DataFit
 * 
 */
const vector< TString > SettingDef::AllowedConf::FitOptions = {"",
                                                               //
                                                               "modshape", "modshapejps", "modshapejpsmm", "modshapejpsee", "modshiftrx", "modscalerx", "modtailrx", "modshapegauss", "modshapetail", "modshapefracgauss", "modshapefracbrem", "modscalejpsmm", "acon", "modbremgaussfrac", 
                                                               //
                                                               "modshiftbrem", "modscalebrem", "modshapefracbrem", "modshiftbremdifference", "modbremfrac", "modbremgaussfrac",
                                                               //
                                                               "acon",
                                                               //
                                                               "fixshape", "fixshapebkg", "cutRangeCL", "ShiftMassPar",
                                                               //
                                                               "modyieldsig", "modyieldsigsingle", "modyieldsigdouble", "modyieldsignotrg", "modyieldsignoyr", 
                                                               "modyieldbkg", 
                                                               "modyieldbkgbs",  //RK + RKst
                                                               "modyieldbkgbs2phi", //RKst 
                                                               "modyieldbkghadswap", //RKst 
                                                               "modyieldbkgpsi2jpsx", //RK + RKst 
                                                               "modyieldbkgPRpsi2jpsx", //RK + RKst 
                                                               "modyieldbkgPRpsi2jpspipi", 
                                                               "modyieldbkglb", "modyieldbkgmid", "modyieldbkgleak", 
                                                               "modyieldbkgprleak", "modyieldbkgprsi2jpsx",
                                                               "modyieldbkgeff", "modyieldbkgeffmm", "crossfeed",
                                                               "modyieldbkgdslc", "modyieldbkgketaprime",
                                                               //
                                                               "posyield", "gconst", "gconsteffsBkg", "gconsteffsFull", "gconstCombSS",  "gconstComb",
                                                               "chainexctrgsig", "chainexctrgbkg",
                                                               //
                                                               "reduce", "splot", "toyconf", "dry", "drysig",
                                                               // HLT correction specific
                                                               "modscaletagprobe"  ,"modshifttagprobe", 
                                                               // Q2 Smearing correction specific  (assumes you use as PDF the bifurcated DSCB shape)
                                                               "modscaleLR", 
                                                               // Bootstrapping workaround (actually saveToDisk)
                                                               "forceSavebeforemod","reloadSaved" ,
                                                               // Likelihood profiles
                                                               "profileRatios", "profileRatios2D", 
                                                               //Save final Workspace from FitGenerator
                                                               "saveWS",
                                                               // Force Snapshots local to keep only the fitting Var
                                                               "initialHesse", 
                                                               "SnpFitVarOnly", 
                                                               "noMSHIFTEE", "noSSCALEEE", 
                                                               "freeParsEE", "freeParsEEBound", "noCache", 
                                                               "noLLOffset"
                                                               };                                                               
bool IsBATCH(TString _batch) noexcept { return ((TString) getenv("BATCH")).Contains(_batch); }

bool HasEOS() noexcept { return (((TString) getenv("EOS")) == "1"); }

void TrowLogicError(bool _flag) noexcept {
    SettingDef::trowLogicError = _flag;
    return;
}

void PrintSettings() noexcept {
    cout << YELLOW;
    MessageSvc::Line();
    MessageSvc::Print("SettingDef");
    MessageSvc::Line();
    MessageSvc::Print("name", SettingDef::name);
    MessageSvc::Print("debug", SettingDef::debug);
    MessageSvc::Print("verbose", SettingDef::verbose ? "true" : "false");
    MessageSvc::Print("trowLogicError", SettingDef::trowLogicError ? "true" : "false");
    MessageSvc::Print("useMultiThread", SettingDef::useMultiThread ? "true" : "false");
    MessageSvc::Print("separator", SettingDef::separator);
    MessageSvc::Print("blindString", SettingDef::blindString);
    MessageSvc::Line();
    cout << RESET;

    PrintIOSettings();
    PrintConfigSettings();
    PrintCutSettings();
    PrintWeightSettings();
    PrintTupleSettings();
    PrintEventsSettings();
    PrintEfficiencySettings();
    PrintFitSettings();
    PrintToySettings();
    PrintROOTSettings();
}

void PrintIOSettings() noexcept {
    cout << YELLOW;
    MessageSvc::Line();
    MessageSvc::Print("SettingDef::IO");
    MessageSvc::Line();
    MessageSvc::Print("exe", SettingDef::IO::exe);
    MessageSvc::Print("yaml", SettingDef::IO::yaml);
    MessageSvc::Print("gangaDir", SettingDef::IO::gangaDir);
    MessageSvc::Print("ioDir", SettingDef::IO::ioDir);
    MessageSvc::Print("dataDir", SettingDef::IO::dataDir);
    MessageSvc::Print("outDir", SettingDef::IO::outDir);
    MessageSvc::Print("useEOS", SettingDef::IO::useEOS ? "true" : "false");
    MessageSvc::Print("useEOSHome", SettingDef::IO::useEOSHome ? "true" : "false");
    MessageSvc::Print("eosDir", SettingDef::IO::eosDir);
    MessageSvc::Print("eosHome", SettingDef::IO::eosHome);
    MessageSvc::Line();
    cout << RESET;
}

void PrintConfigSettings() noexcept {
    cout << YELLOW;
    MessageSvc::Line();
    MessageSvc::Print("SettingDef::Config");
    MessageSvc::Line();
    MessageSvc::Print("project", SettingDef::Config::project);
    MessageSvc::Print("ana", SettingDef::Config::ana);
    MessageSvc::Print("sample", SettingDef::Config::sample);
    MessageSvc::Print("q2bin", SettingDef::Config::q2bin);
    MessageSvc::Print("year", SettingDef::Config::year);
    MessageSvc::Print("polarity", SettingDef::Config::polarity);
    MessageSvc::Print("trigger", SettingDef::Config::trigger);
    MessageSvc::Print("triggerConf", SettingDef::Config::triggerConf);
    MessageSvc::Print("brem", SettingDef::Config::brem);
    MessageSvc::Print("track", SettingDef::Config::track);
    MessageSvc::Line();
    cout << RESET;
}

void PrintCutSettings() noexcept {
    cout << YELLOW;
    MessageSvc::Line();
    MessageSvc::Print("SettingDef::Cut");
    MessageSvc::Line();
    MessageSvc::Print("option", SettingDef::Cut::option);
    MessageSvc::Print("extraCut", SettingDef::Cut::extraCut);
    MessageSvc::Print("extraEEOnly", SettingDef::Cut::extraEEOnly ? "true" : "false");
    MessageSvc::Print("extraMMOnly", SettingDef::Cut::extraMMOnly ? "true" : "false");
    MessageSvc::Print("tightLowQ2", SettingDef::Cut::tightLowQ2 ? "true" : "false");
    MessageSvc::Print("force", SettingDef::Cut::force ? "true" : "false");
    MessageSvc::Line();
    cout << RESET;
}

void PrintWeightSettings() noexcept {
    cout << YELLOW;
    MessageSvc::Line();
    MessageSvc::Print("SettingDef::Weight");
    MessageSvc::Line();
    MessageSvc::Print("option", SettingDef::Weight::option);
    MessageSvc::Print("config", SettingDef::Weight::config);
    MessageSvc::Print("do Bootstrapping", SettingDef::Weight::useBS ? "true" : "false" );
    MessageSvc::Print("priorChain", SettingDef::Weight::priorChain ? "true" : "false" );
    MessageSvc::Print("pidVer", SettingDef::Weight::pidVer );
    MessageSvc::Print("usePIDPTElectron", SettingDef::Weight::usePIDPTElectron ? "true": "false");
    MessageSvc::Print("useMCRatioPID", SettingDef::Weight::useMCRatioPID ? "true": "false");
    MessageSvc::Print("trkVer", SettingDef::Weight::trkVer );
    MessageSvc::Print("l0Ver", SettingDef::Weight::l0Ver );
    MessageSvc::Print("useStatusL0Formula", SettingDef::Weight::useStatusL0Formula ? "true": "false" );
    MessageSvc::Print("hltVer", SettingDef::Weight::hltVer );
    MessageSvc::Print("mcVer", SettingDef::Weight::mcVer );
    MessageSvc::Print("q2SmearFileTag", SettingDef::Weight::q2SmearFileTag );  
    MessageSvc::Print("q2SmearDiffVar", SettingDef::Weight::q2SmearDiffVar );  
    MessageSvc::Print("PID maps for electron->electron with PT,ETA scheme ", SettingDef::Weight::usePIDPTElectron  ? "true" : "false");
    MessageSvc::Print("PID maps for electrons with data/sim weight ratios", SettingDef::Weight::useMCRatioPID  ? "true" : "false");
    MessageSvc::Print("Tracking maps from B0 mode", SettingDef::Weight::TrkFromRKst ? "true" : "false");      
    MessageSvc::Print("L0I(e)->L0I(e) logic on attaching", SettingDef::Weight::L0I_EToE ? "true" : "false");
    MessageSvc::Line();
    cout << RESET;
}

void PrintTupleSettings() noexcept {
    cout << YELLOW;
    MessageSvc::Line();
    MessageSvc::Print("SettingDef::Tuple");
    MessageSvc::Line();
    MessageSvc::Print("option", SettingDef::Tuple::option);
    MessageSvc::Print("gngVer", SettingDef::Tuple::gngVer);
    MessageSvc::Print("proVer", SettingDef::Tuple::proVer);
    MessageSvc::Print("creVer", SettingDef::Tuple::creVer);
    MessageSvc::Print("splVer", SettingDef::Tuple::splVer);
    MessageSvc::Print("outVer", SettingDef::Tuple::outVer);
    MessageSvc::Print("fileName", SettingDef::Tuple::fileName);
    MessageSvc::Print("tupleName", SettingDef::Tuple::tupleName);
    MessageSvc::Print("addTuple", SettingDef::Tuple::addTuple ? "true" : "false");
    MessageSvc::Print("dataFrame", SettingDef::Tuple::dataFrame ? "true" : "false");
    MessageSvc::Print("branches", SettingDef::Tuple::branches ? "true" : "false");
    MessageSvc::Print("aliases", SettingDef::Tuple::aliases ? "true" : "false");
    MessageSvc::Print("frac", to_string(SettingDef::Tuple::frac));
    MessageSvc::Print("datasetCache", SettingDef::Tuple::datasetCache ? "true" : "false");
    MessageSvc::Line();
    cout << RESET;
}

void PrintEventsSettings() noexcept {
    cout << YELLOW;
    MessageSvc::Line();
    MessageSvc::Print("SettingDef::Events");
    MessageSvc::Line();
    MessageSvc::Print("cutOptionCL", SettingDef::Events::cutOptionCL);
    MessageSvc::Print("cutOptionSigMC", SettingDef::Events::cutOptionSigMC);
    MessageSvc::Print("cutOptionBkgMC", SettingDef::Events::cutOptionBkgMC);
    MessageSvc::Print("weightOption", SettingDef::Events::weightOption);
    MessageSvc::Print("tupleOption", SettingDef::Events::tupleOption);
    MessageSvc::Print("types", to_string(SettingDef::Events::types.size()));
    MessageSvc::Line();
    cout << RESET;
}

void PrintEfficiencySettings() noexcept {
    cout << YELLOW;
    MessageSvc::Line();
    MessageSvc::Print("SettingDef::Efficiency");
    MessageSvc::Line();
    MessageSvc::Print("option", SettingDef::Efficiency::option);
    MessageSvc::Print("ver", SettingDef::Efficiency::ver);
    MessageSvc::Print("flatnessVer", SettingDef::Efficiency::flatnessVer);
    MessageSvc::Print("minUncertainty", to_string(SettingDef::Efficiency::minUncertainty));
    MessageSvc::Print("scaleEfficiency", to_string(SettingDef::Efficiency::scaleEfficiency));
    MessageSvc::Print("scaleSystematics", to_string(SettingDef::Efficiency::scaleSystematics));

    MessageSvc::Print("fitconfiguration");
    SettingDef::Efficiency::fitconfiguration.Print();   
    MessageSvc::Line();
    cout << RESET;
}

void PrintFitSettings() noexcept {
    cout << YELLOW;
    MessageSvc::Line();
    MessageSvc::Print("SettingDef::Fit");
    MessageSvc::Line();
    MessageSvc::Print("option", SettingDef::Fit::option);
    MessageSvc::Print("ver", SettingDef::Fit::ver);
    MessageSvc::Print("doBinned", SettingDef::Fit::doBinned ? "true" : "false");
    MessageSvc::Print("nBins", to_string(SettingDef::Fit::nBins));
    MessageSvc::Print("varSchemeMC", SettingDef::Fit::varSchemeMC);
    MessageSvc::Print("varSchemeCL", SettingDef::Fit::varSchemeCL);
    MessageSvc::Print("stepSize", to_string(SettingDef::Fit::stepSize));
    MessageSvc::Print("stepSizePar", to_string(SettingDef::Fit::stepSizePar));
    MessageSvc::Print("stepSizeYield", to_string(SettingDef::Fit::stepSizeYield));
    MessageSvc::Print("splitL0Categories", SettingDef::Fit::splitL0Categories ? "true" : "false");
    MessageSvc::Print("splitTrackCategories", SettingDef::Fit::splitTrackCategories ? "true" : "false");
    MessageSvc::Print("plotSumCategories", SettingDef::Fit::plotSumCategories ? "true" : "false");
    MessageSvc::Print("blindYield", SettingDef::Fit::blindYield ? "true" : "false");
    MessageSvc::Print("blindEfficiency", SettingDef::Fit::blindEfficiency ? "true" : "false");
    MessageSvc::Print("blindRatio", SettingDef::Fit::blindRatio ? "true" : "false");
    MessageSvc::Print("reduceRooKeysPDF", SettingDef::Fit::reduceRooKeysPDF ? "true" : "false");
    MessageSvc::Print("useDatasetCache", SettingDef::Fit::useDatasetCache ? "true" : "false");
    MessageSvc::Print("redoDatasetCache", SettingDef::Fit::redoDatasetCache ? "true" : "false");
    MessageSvc::Print("useBremFracCache", SettingDef::Fit::useBremFracCache ? "true" : "false");
    MessageSvc::Print("redoBremFracCache", SettingDef::Fit::redoBremFracCache ? "true" : "false");

    MessageSvc::Print("useRecursiveFractions", SettingDef::Fit::useRecursiveFractions ? "true" : "false");
    MessageSvc::Print("useRooRealSumPDF", SettingDef::Fit::useRooRealSumPDF ? "true" : "false");
    MessageSvc::Print("useMinuit2", SettingDef::Fit::useMinuit2 ? "true" : "false");

    // MessageSvc::Print("initialParamFile", SettingDef::Fit::initialParamFile);
    MessageSvc::Print("initialParamFile"); 
    for( auto & el :  SettingDef::Fit::initialParamFile){
        std::cout<< el << std::endl;
    }
    MessageSvc::Print("dumpParamFile",          SettingDef::Fit::dumpParamFile);
    MessageSvc::Print("configurations",         to_string(SettingDef::Fit::configurations.size()));
    MessageSvc::Print("yamls",                  to_string(SettingDef::Fit::yamls.size()));
    MessageSvc::Print("useSPlot2",              SettingDef::Fit::useSPlot2 ? "true" : "false");
    MessageSvc::Print("useRatioComb",           SettingDef::Fit::useRatioComb ? "true" : "false");
    MessageSvc::Print("useNumericalExpTurnOn",  SettingDef::Fit::useNumericalExpTurnOn ? "true" : "false");
    MessageSvc::Print("loadFitComponentCaches", SettingDef::Fit::loadFitComponentCaches ? "true" : "false");
    MessageSvc::Print("saveFitComponentCaches", SettingDef::Fit::saveFitComponentCaches ? "true" : "false");
    MessageSvc::Print("redoFitComponentCaches", SettingDef::Fit::redoFitComponentCaches ? "true" : "false");
    MessageSvc::Print("rareOnly",               SettingDef::Fit::rareOnly ? "true" : "false");
    MessageSvc::Print("nCPUDataFit",            to_string(SettingDef::Fit::nCPUDataFit));
    MessageSvc::Print("LPTMCandidates",             SettingDef::Fit::LPTMCandidates ? "true" : "false"); 
    MessageSvc::Print("CorrelateConstraintsNoNorm", SettingDef::Fit::CorrelateConstraintsNoNorm ? "true" : "false"); 

    MessageSvc::Print("scan1DParameter",    SettingDef::Fit::scan1DParameter);
    MessageSvc::Print("startLLScanFromMin", SettingDef::Fit::startLLScanFromMin ? "true" : "false"); 
    MessageSvc::Print("nScanPointsProfile", to_string(SettingDef::Fit::nScanPointsProfile)); 

    MessageSvc::Line();
    cout << RESET;
}

void PrintToySettings() noexcept {
    cout << YELLOW;
    MessageSvc::Line();
    MessageSvc::Print("SettingDef::Toy");
    MessageSvc::Line();
    MessageSvc::Print("option", SettingDef::Toy::option);
    MessageSvc::Print("tupleVer", SettingDef::Toy::tupleVer);
    MessageSvc::Print("studyVer", SettingDef::Toy::studyVer);
    MessageSvc::Print("nJobs", to_string(SettingDef::Toy::nJobs));
    MessageSvc::Print("jobIndex", to_string(SettingDef::Toy::jobIndex));
    MessageSvc::Print("nToysPerJob", to_string(SettingDef::Toy::nToysPerJob));
    MessageSvc::Print("constraintOverwriteFile"); 
    for( auto & el :  SettingDef::Toy::constraintOverwriteFile){
        std::cout<< el << std::endl;
    }
    MessageSvc::Print("frozenOverwrite", to_string(SettingDef::Toy::frozenOverwrite)); 
    MessageSvc::Print("mergeConfig", to_string(SettingDef::Toy::mergeConfig));
    MessageSvc::Print("configurations", to_string(SettingDef::Toy::configurations.size()));
    MessageSvc::Print("yamls", to_string(SettingDef::Toy::yamls.size()));
    MessageSvc::Print("Silent"     , to_string(SettingDef::Toy::Silent));
    MessageSvc::Print("CopyLocally", to_string(SettingDef::Toy::CopyLocally));
    MessageSvc::Print("ReadFractionToysComponents", SettingDef::Toy::ReadFractionToysComponents);
    MessageSvc::Print("configurationOverrideFile", SettingDef::Toy::configurationOverrideFile);

    MessageSvc::Line();
    cout << RESET;
}

void PrintROOTSettings() noexcept {
    cout << YELLOW;
    MessageSvc::Line();
    MessageSvc::Print("ROOT");
    MessageSvc::Line();
    if (ROOT::GetThreadPoolSize() == 1) cout << MAGENTA;
    MessageSvc::Print("ImplicitMTPoolSize", to_string(ROOT::GetThreadPoolSize()));
    cout << YELLOW;
    MessageSvc::Line();
    cout << RESET;
}

TString gRX::Project()     noexcept { return SettingDef::Config::project; }
TString gRX::Analysis()    noexcept { return SettingDef::Config::ana; }
TString gRX::Sample()      noexcept { return SettingDef::Config::sample; }
TString gRX::Q2Bin()       noexcept { return SettingDef::Config::q2bin; }
TString gRX::Year()        noexcept { return SettingDef::Config::year; }
TString gRX::Polarity()    noexcept { return SettingDef::Config::polarity; }
TString gRX::Trigger()     noexcept { return SettingDef::Config::trigger; }
TString gRX::TriggerConf() noexcept { return SettingDef::Config::triggerConf; }
TString gRX::Brem()        noexcept { return SettingDef::Config::brem; }
TString gRX::Track()       noexcept { return SettingDef::Config::track; }

#endif
