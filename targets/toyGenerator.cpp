#include "ToyGenerator.hpp"
#include "ParserSvc.hpp"

/**
 * toyTupleGenerator
 * toyTupleGenerator.out --yaml config-###.yaml
 */
int main(int argc, char ** argv) {

    auto tStart = chrono::high_resolution_clock::now();

    ParserSvc parser("");
    parser.Init(argc, argv);
    if (parser.Run(argc, argv) != 0) return 1;
    
    if( SettingDef::Toy::CopyLocally){
      SettingDef::Toy::CopyLocally = false; 
    }
    ToyGenerator generator = ToyGenerator();
    generator.GetTupleGenerators();
    generator.SetSeed(SettingDef::Toy::jobIndex + 823453);

    MessageSvc::Info("Generating", to_string(SettingDef::Toy::nToysPerJob), "toy(s) for job", to_string(SettingDef::Toy::jobIndex), "of", to_string(SettingDef::Toy::nJobs), "...");
    for (int i = 0; i < SettingDef::Toy::nToysPerJob; i++) {
        MessageSvc::Line();
        MessageSvc::Info("Generating", "Toy", to_string(i), "...");
        generator.Generate(i);
    }

    auto tEnd = chrono::high_resolution_clock::now();
    MessageSvc::Line();
    MessageSvc::Warning((TString) SettingDef::IO::exe, "Took", to_string(chrono::duration_cast< chrono::seconds >(tEnd - tStart).count()), "seconds");
    MessageSvc::Line();

    return 0;
}
