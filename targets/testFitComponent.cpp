#include "EventType.hpp"
#include "FitComponent.hpp"
#include "FitConfiguration.hpp"
#include "ParserSvc.hpp"

/**
 * Test FitComponent
 */
int main(int argc, char ** argv) {

    ParserSvc parser("");
    parser.Init(argc, argv);
    if (parser.Run(argc, argv) != 0) return 1;

    vector< FitConfiguration > configurations = SettingDef::Fit::configurations;
    if (configurations.size() != 1) MessageSvc::Error(SettingDef::IO::exe, (TString) "Only supports 1 FitConfiguration", "EXIT_FAILURE");
    FitConfiguration conf = configurations.front();

    vector< Sample > samples = conf.BackgroundSamples();

    for (auto sample : samples) {
        EventType et     = get< 0 >(conf.GetBackground(sample));
        TString   option = get< 2 >(conf.GetBackground(sample));
        cout << et << endl;

        et.Init();

        PdfType      type = conf.GetTypeFromSample(sample);
        FitComponent fc   = FitComponent(et, to_string(sample) + SettingDef::separator + et.GetKey(), type, conf.Var(), SettingDef::Fit::option + option);

        // if (et.IsMC()) {
        configurations.front().UseBinAndRange(SettingDef::Fit::varSchemeMC);
        fc.SetBinnedFit(configurations.front().IsBinnedMC());
        fc.CreatePDF();
        if (fc.Option().Contains("conv")) {
            fc.ConvPDF();
            MessageSvc::Info("CreatePDF", fc.PDF());
            MessageSvc::Line();
        }
        //} else {
        //    configurations.front().UseBinAndRange(SettingDef::Fit::varSchemeCL);
        //    fc.SetBinnedFit(configurations.front().IsBinnedCL());
        //    fc.CreateData();
        //}

        fc.SaveToDisk("test");

        fc.Close();
    }

    // fc.SaveToDisk();
    // auto fc2 = FitComponent(et.GetKey(), "", "", "./");

    return 0;
}
