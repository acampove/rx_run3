#include "EfficiencySvc.hpp"
#include "EventType.hpp"
#include "ParserSvc.hpp"

/**
 * Test EventType
 */
int main(int argc, char ** argv) {

    ParserSvc parser("");
    parser.Init(argc, argv);
    if (parser.Run(argc, argv) != 0) return 1;

    EventType et0 = EventType();
    et0.Init();
    cout << et0 << endl;

    et0.GetTupleEntries();

    if (SettingDef::Tuple::branchList.size() != 0) { et0.ScanTuple(); }

    return 0;

    CutHolder c0 = et0.GetCutHolder();
    cout << c0;
    CutHolder c1 = c0.Clone("");
    cout << c1;

    WeightHolder w0 = et0.GetWeightHolder();
    cout << w0;
    WeightHolder w1 = w0.Clone("{B}_fit", "PID-L0");
    cout << w1;

    return 0;

    SettingDef::Tuple::dataFrame = false;

    auto tStart = chrono::high_resolution_clock::now();
    et0.GetTupleEntries();
    auto tEnd = chrono::high_resolution_clock::now();
    MessageSvc::Line();
    MessageSvc::Warning((TString) SettingDef::IO::exe, "Took", to_string(chrono::duration_cast< chrono::seconds >(tEnd - tStart).count()), "seconds");
    MessageSvc::Line();

    SettingDef::Tuple::dataFrame = true;

    tStart = chrono::high_resolution_clock::now();
    et0.GetTupleEntries();
    tEnd = chrono::high_resolution_clock::now();
    MessageSvc::Line();
    MessageSvc::Warning((TString) SettingDef::IO::exe, "Took", to_string(chrono::duration_cast< chrono::seconds >(tEnd - tStart).count()), "seconds");
    MessageSvc::Line();

    LoadEfficiencyForFit(et0);

    return 0;

    auto et1 = EventType(Prj::RKst, Analysis::MM, to_string(Sample::Empty), Q2Bin::All, Year::All, Polarity::All, Trigger::All, Brem::All, Track::All, SettingDef::Cut::option, SettingDef::Weight::option, SettingDef::Tuple::option, false);
    et1.Init(true, false);
    cout << et1 << endl;
    et1.SaveToDisk("test");

    auto et2 = et1;
    et2.LoadFromDisk("test");
    et2.Init(true, false);
    cout << et2 << endl;

    MessageSvc::Line();
    if (et1 != et2)
        MessageSvc::Error("LoadFromDisk != SaveToDisk");
    else
        MessageSvc::Info("LoadFromDisk == SaveToDisk");
    MessageSvc::Line();

    auto et3 = et1;
    et3.LoadFromNamed("test");
    et3.Init(true, false);
    cout << et3 << endl;

    MessageSvc::Line();
    if (et1 != et3)
        MessageSvc::Error("LoadFromNamed != SaveToDisk");
    else
        MessageSvc::Info("LoadFromNamed == SaveToDisk");
    MessageSvc::Line();

    TString yaml = et1.SaveToYAML("test");
    auto    et4  = parser.LoadFromYAML(yaml);
    et4.Init(true, false);
    cout << et4 << endl;

    MessageSvc::Line();
    if (et1 != et4)
        MessageSvc::Error("LoadFromYAML != SaveToYAML");
    else
        MessageSvc::Info("LoadFromYAML == SaveToYAML");
    MessageSvc::Line();

    return 0;

    et1.SaveToYAML("test-noEvents", "noEvents");
    et1.SaveToYAML("test-noEfficiency", "noEfficiency");
    et1.SaveToYAML("test-noFit", "noFit");
    et1.SaveToYAML("test-noToy", "noToy");

    for (const auto & project : SettingDef::AllowedConf::Projects) {
        if (project == to_string(Prj::All)) continue;
        if (project == to_string(Prj::RK)) continue;
        if (project == to_string(Prj::RPhi)) continue;

        for (const auto & ana : SettingDef::AllowedConf::Analyses) {
            if (ana == to_string(Analysis::All)) continue;

            for (const auto & q2bin : SettingDef::AllowedConf::Q2Bins) {
                if (q2bin == to_string(Q2Bin::All)) continue;

                for (const auto & year : GetYears(SettingDef::Config::year)) {
                    if (year == to_string(Year::All)) continue;

                    for (const auto & trigger : SettingDef::AllowedConf::L0Categories) {
                        if (trigger == to_string(Trigger::All)) continue;

                        for (const auto & triggerConf : SettingDef::AllowedConf::L0Configurations) {
                            if ((triggerConf != to_string(TriggerConf::Exclusive)) && (trigger == to_string(Trigger::L0I))) continue;
                            // if ((triggerConf != to_string(TriggerConf::Exclusive)) && (trigger == to_string(Trigger::All))) continue;

                            SettingDef::Config::triggerConf = triggerConf;

                            TString copt = "no";
                            // for (const auto & copt : SettingDef::AllowedConf::CutOptions) {

                            TString wopt = "no";
                            // for (const auto & wopt : SettingDef::AllowedConf::WeightOptions) {

                            auto et1 = EventType(hash_project(project), hash_analysis(ana), to_string(Sample::Empty), hash_q2bin(q2bin), hash_year(year), Polarity::All, hash_trigger(trigger), Brem::All, Track::All, copt, wopt, SettingDef::Tuple::option, false);
                            et1.Init(true, false);
                            cout << et1 << endl;

                            TString yaml = et1.SaveToYAML("test");
                            auto    et5  = parser.LoadFromYAML(yaml);
                            et5.Init(true, false);
                            cout << et5 << endl;

                            MessageSvc::Line();
                            if (et1 != et5)
                                MessageSvc::Error("LoadFromYAML != SaveToYAML");
                            else
                                MessageSvc::Info("LoadFromYAML == SaveToYAML");
                            MessageSvc::Line();
                            //}
                            //}
                        }
                    }
                }
            }
        }
    }

    return 0;
}
