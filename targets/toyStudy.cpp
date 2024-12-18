#include "ToyStudy.hpp"
#include "ParserSvc.hpp"

/**
 * toyStudy
 * toyStudy.out --yaml config-###.yaml
 */
int main(int argc, char ** argv) {

    auto tStart = chrono::high_resolution_clock::now();

    ParserSvc parser("");
    parser.Init(argc, argv);
    if (parser.Run(argc, argv) != 0) return 1;

    if(SettingDef::Toy::Silent){
        MessageSvc::Warning("Suppressing all messagesvc text printout in execution");
        MessageSvc::SILENCED = true;
    }

    ToyStudy study = ToyStudy();
    study.SetupFitter();
    study.SetupReader();
    study.CheckKeys();
    study.SynchroniseObservable();

    MessageSvc::Info("Fitting", to_string(SettingDef::Toy::nToysPerJob), "toy(s) for job", to_string(SettingDef::Toy::jobIndex), "of", to_string(SettingDef::Toy::nJobs), "...");
    // int startIndex = SettingDef::Toy::jobIndex * SettingDef::Toy::nToysPerJob;
    // int endIndex   = (SettingDef::Toy::jobIndex + 1) * (SettingDef::Toy::nToysPerJob);
    int startIndex = 0;
    int endIndex   = SettingDef::Toy::nToysPerJob;
    for (int i = startIndex; i < endIndex; i++) { study.Fit(i); }

    study.SaveResults("");

    auto tEnd = chrono::high_resolution_clock::now();
    MessageSvc::Line();
    MessageSvc::Warning((TString) SettingDef::IO::exe, "Took", to_string(chrono::duration_cast< chrono::seconds >(tEnd - tStart).count()), "seconds");
    MessageSvc::Line();

    if(SettingDef::Toy::CopyLocally){
        std::cout<< "Cleaning up local files CopyLocally*.root"<<std::endl;
        TString _command = "rm -rf CopyLocally*.root";
        IOSvc::runCommand(_command);
    }
    return 0;
}
