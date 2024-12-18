#include "FitGenerator.hpp"
#include "FitterTool.hpp"
#include "ParserSvc.hpp"
#include "ToyTupleConfigGenerator.hpp"

#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds

/**
 * fitGenerator
 * fitGenerator.out --yaml yaml/fitter/config-Fit###.yaml
 */
int main(int argc, char ** argv) {
    auto tStart = chrono::high_resolution_clock::now();
    //================================
    // Parsing Yaml file
    //================================
    ParserSvc parser("");
    parser.Init(argc, argv);
    if (parser.Run(argc, argv) != 0) return 1;

    int startLoop = SettingDef::Weight::iBS;
    SettingDef::Weight::iBS = -1;

    //================================
    // Configure fit names
    //================================
    TString name;
    if (SettingDef::name != "")
        name = SettingDef::name;
    else {
        name = parser.YAML();
        name.Remove(0, name.Last('/') + 1);
        name.ReplaceAll("config-", "");
        name.ReplaceAll(".yaml", "");
    }

    //================================
    // Configure directory nominal result storing
    //================================
    auto co = ConfigHolder();
    if (SettingDef::IO::outDir == "FitDir") {
        SettingDef::IO::outDir = IOSvc::GetFitDir(SettingDef::Fit::option, co);
        IOSvc::MakeDir(SettingDef::IO::outDir, OpenMode::WARNING);
    }

    //================================
    // Are we running the fit with Bootstrapping enabled? If yes, force useBS to false and make a nominal fit 
    // The option -forceSavebeforemod is added such that the raw-MC fits results are saved
    // If we do this, all yields associated to PDF components are not initialized
    // If we do this, all modifiers to shapes and yields according to cosntraints are not reloaded
    // Thus, when dealing with the "reloaded" fit from disk, we have to "re-init" the parameter pool , "re-init" the yields bounds before modifying them, and "re-inject" constraints.
    // And when fitting "Construct on the fly the Model in each FitHolder" (see fitter tool)
    // All the operations are driven by SettingDef::Weight::useBS , which when set to false does the baseline work.
    // Debugging this requires that we read carefulll the createNLL output(s) that all pdfs and yields are in the right place. 
    //================================    
    auto fit_bootstrapped = SettingDef::Weight::useBS;
    if(  fit_bootstrapped) SettingDef::Weight::useBS = false;    
    MessageSvc::Line();
    MessageSvc::Warning("FitGenerator", name, SettingDef::Fit::option);
    MessageSvc::Line();
    MessageSvc::Warning("FitGenerator", (TString) "Output =", SettingDef::IO::outDir);
    if (SettingDef::Fit::option.Contains("toyconf")) {
        TString td = IOSvc::GetTupleDir("toy", co) + "/Configuration/ToyConfiguration.root";
        MessageSvc::Line();
        MessageSvc::Warning("FitGenerator", (TString) "TupleToy =", td);
        if (IOSvc::ExistFile(td)) MessageSvc::Error("FitGenerator", "Already existing TupleToy", "EXIT_FAILURE");
    }
    MessageSvc::Line();
    FitGenerator fg(name, SettingDef::Fit::option);
    fg.SaveToYAML();
    fg.Init();
    if (!SettingDef::Fit::option.Contains("dry")) {
        fg.Fit();
        fg.LogFitResult();     
        if (SettingDef::Fit::option.Contains("toyconf")) {
            ToyTupleConfigGenerator tg = ToyTupleConfigGenerator();
            tg.OverwriteTupleConfig(fg);
        }
    }
    fg.Close();
    //================================
    // Are we running the fit with Bootstrapping enabled? If yes, force useBS to true and loop over BS indexes
    //================================  
    if( fit_bootstrapped ){
        auto _PARPOOL_ = RXFitter::GetParameterPool();
        SettingDef::Weight::useBS       = true;  
        SettingDef::Fit::LPTMCandidates = false;
        MessageSvc::Info((TString) SettingDef::IO::exe, (TString)Form("Start BS loops (1-%i)", WeightDefRX::nBS) , "");
        auto _originalWeightOpt = SettingDef::Weight::option;
        int nLoops = WeightDefRX::nBS; //do them ALL...
        if(startLoop < 0 ){
            startLoop = 0;
        }else{
            startLoop = startLoop;
        }
        auto dumpParamFile = SettingDef::Fit::dumpParamFile;
	    MessageSvc::Warning("Starting bsFit ",to_string(startLoop));
        DisableMultiThreads();
        for( int bsIDx = startLoop; bsIDx < nLoops ; ++bsIDx){       
            _PARPOOL_->ClearParameters();  
            _PARPOOL_->PrintParameters();   
            SettingDef::IO::outDir = Form("./bs%i", bsIDx);
            IOSvc::MakeDir(Form("./bs%i", bsIDx), OpenMode::WARNING ); //change to ERROR
            auto tStartBS = chrono::high_resolution_clock::now();
            SettingDef::Weight::option = _originalWeightOpt +"-"+WeightDefRX::ID::BS;
            SettingDef::Weight::iBS = bsIDx;
            FitGenerator fgBS(name, SettingDef::Fit::option);
            //tweak hack around something nasty going on with ParserSvc logic.
            SettingDef::Fit::dumpParamFile = SettingDef::IO::outDir +"/"+dumpParamFile;
            //fgBS.SaveToYAML();
            MessageSvc::Warning( (TString)Form("FitGeneratorBS %i/%i", bsIDx, nLoops), name, (TString) "Init");
            fgBS.Init();
            MessageSvc::Warning( (TString)Form("FitGeneratorBS %i/%i", bsIDx, nLoops), name, (TString) "Fit");
            fgBS.Fit();
            MessageSvc::Warning( (TString)Form("FitGeneratorBS %i/%i", bsIDx, nLoops), name, (TString) "LogFitResults");
            fgBS.LogFitResult();
            MessageSvc::Warning( (TString)Form("FitGeneratorBS %i/%i", bsIDx, nLoops), name, (TString) "Close");
            fgBS.Close();          
            auto tEndBS = chrono::high_resolution_clock::now();
            MessageSvc::Warning((TString)Form("*** FitGeneratorBS %i/%i", bsIDx, nLoops), "Took", to_string(chrono::duration_cast< chrono::seconds >(tEndBS - tStartBS).count()), "seconds");
            MessageSvc::Line();
        }
        // ============================================================
        // Merge sorted the Bootstrap results in a single ntuple.
        // ============================================================        
        MessageSvc::Info("Merging results bootstrapped fits");
        TString hadd_string = "hadd -f FitResultBS.root";
        TString _allBSfits = "";
        for( int bsIDx = 0; bsIDx < nLoops; ++bsIDx){ 
            TString _outFload = Form("./bs%i/FitResultBS.root", bsIDx );
            _allBSfits = _allBSfits+" "+_outFload;
        }
        hadd_string = hadd_string+_allBSfits;        
        IOSvc::runCommand( hadd_string );
        MessageSvc::Info("Merging results bootstrapped fits");
    }
    auto tEnd = chrono::high_resolution_clock::now();
    MessageSvc::Line();
    MessageSvc::Warning((TString) SettingDef::IO::exe, "Took", to_string(chrono::duration_cast< chrono::seconds >(tEnd - tStart).count()), "seconds");
    MessageSvc::Line();

    return 0;
}
