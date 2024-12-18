#include "ParserSvc.hpp"
#include "SettingDef.hpp"

/**
 * Test Kernel
 */
int main(int argc, char ** argv) {

    MessageSvc::Line();
    MessageSvc::Info("Allowed Projects");
    for (const auto & it : SettingDef::AllowedConf::Projects) { MessageSvc::Info(it, (TString) "hash_project", to_string(static_cast< int >(hash_project(it))), "to_string", to_string(hash_project(it))); }

    MessageSvc::Line();
    MessageSvc::Info("Allowed Analyses");
    for (const auto & it : SettingDef::AllowedConf::Analyses) { MessageSvc::Info(it, (TString) "hash_analysis", to_string(static_cast< int >(hash_analysis(it))), "to_string", to_string(hash_analysis(it))); }

    for (const auto & prj : SettingDef::AllowedConf::Projects) {
        if (prj == to_string(Prj::All)) continue;
        MessageSvc::Line();
        MessageSvc::Info("Allowed Samples" + prj);
        for (const auto & it : SettingDef::AllowedConf::Samples.at(hash_project(prj))) { MessageSvc::Info(it); }

        MessageSvc::Line();
        vector< TString > samples = SettingDef::AllowedConf::Samples.at(hash_project(prj));
        RemoveVectorDuplicates(samples);
        bool isBd = false;
        bool isBs = false;
        bool isBu = false;
        bool isLb = false;
        int  nBd  = 0;
        int  nBs  = 0;
        int  nBu  = 0;
        int  nLb  = 0;
        for (const auto & it : samples) {
            if (it == to_string(Sample::Empty)) continue;
            if (it.Contains(to_string(Sample::Data))) continue;
            if (it.BeginsWith("K")) continue;
            if (it.Contains("SB")) continue;

            if (it.BeginsWith("Bd")) {
                if (!isBd) {
                    isBd = true;
                    cout << endl << "// MC Bd" << endl;
                }
                nBd++;
            }
            if (it.BeginsWith("Bs")) {
                if (!isBs) {
                    isBs = true;
                    cout << endl << "// MC Bs" << endl;
                }
                nBs++;
            }
            if (it.BeginsWith("Bu")) {
                if (!isBu) {
                    isBu = true;
                    cout << endl << "// MC Bu" << endl;
                }
                nBu++;
            }
            if (it.BeginsWith("Lb")) {
                if (!isLb) {
                    isLb = true;
                    cout << endl << "// MC Lb" << endl;
                }
                nLb++;
            }
            cout << "\"" << it << "\", ";
        }
        cout << endl << endl;
        MessageSvc::Info("nBd = " + to_string(nBd));
        MessageSvc::Info("nBs = " + to_string(nBs));
        MessageSvc::Info("nBu = " + to_string(nBu));
        MessageSvc::Info("nLb = " + to_string(nLb));
    }

    MessageSvc::Line();
    MessageSvc::Info("Allowed Q2Bins");
    for (const auto & it : SettingDef::AllowedConf::Q2Bins) { MessageSvc::Info(it, (TString) "hash_q2bin", to_string(static_cast< int >(hash_q2bin(it))), "to_string", to_string(hash_q2bin(it))); }

    MessageSvc::Line();
    MessageSvc::Info("Allowed Years");
    for (const auto & it : SettingDef::AllowedConf::Years) { MessageSvc::Info(it, (TString) "hash_year", to_string(static_cast< int >(hash_year(it))), "to_string", to_string(hash_year(it))); }

    MessageSvc::Line();
    MessageSvc::Info("Allowed Polarities");
    for (const auto & it : SettingDef::AllowedConf::Polarities) { MessageSvc::Info(it, (TString) "hash_polarity", to_string(static_cast< int >(hash_polarity(it))), "to_string", to_string(hash_polarity(it))); }

    MessageSvc::Line();
    MessageSvc::Info("Allowed L0Categories");
    for (const auto & it : SettingDef::AllowedConf::L0Categories) { MessageSvc::Info(it, (TString) "hash_trigger", to_string(static_cast< int >(hash_trigger(it))), "to_string", to_string(hash_trigger(it))); }

    MessageSvc::Line();
    MessageSvc::Info("Allowed L0Configurations");
    for (const auto & it : SettingDef::AllowedConf::L0Configurations) { MessageSvc::Info(it, (TString) "hash_triggerconf", to_string(static_cast< int >(hash_triggerconf(it))), "to_string", to_string(hash_triggerconf(it))); }

    MessageSvc::Line();
    MessageSvc::Info("Allowed BremCategories");
    for (const auto & it : SettingDef::AllowedConf::BremCategories) { MessageSvc::Info(it, (TString) "hash_brem", to_string(static_cast< int >(hash_brem(it))), "to_string", to_string(hash_brem(it))); }

    MessageSvc::Line();

    return 0;
}
