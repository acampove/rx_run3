#include "EventType.hpp"
#include "ParserSvc.hpp"
#include <fmt_ostream.h>

/**
 * Retrieve Luminosity
 * tupleLumi.out
 */
int main(int argc, char ** argv) {

    ParserSvc parser("");
    parser.Init(argc, argv);
    if (parser.Run(argc, argv) != 0) return 1;

    SettingDef::trowLogicError = true;

    vector< TString > projects = SettingDef::AllowedConf::Projects;
    if (SettingDef::Config::project != "") projects = {SettingDef::Config::project};

    SettingDef::Config::ana = to_string(Analysis::MM);

    SettingDef::Config::sample = to_string(Sample::Data);

    vector< TString > years = GetYears(SettingDef::Config::year);
    if (SettingDef::Config::year != "") years = {SettingDef::Config::year};

    vector< TString > polarities = GetPolarities(SettingDef::Config::polarity);
    if (SettingDef::Config::polarity != "") polarities = {SettingDef::Config::polarity};

    SettingDef::Tuple::option = "gng";
    if (SettingDef::Tuple::gngVer == "") SettingDef::Tuple::gngVer = "10";

    SettingDef::Tuple::tupleName = "LT";
    SettingDef::Tuple::branches  = false;
    SettingDef::Tuple::aliases   = false;

    for (const auto & project : projects) {
        if (project == to_string(Prj::All)) continue;
        SettingDef::Config::project = project;

        YAML::Emitter out;
        out << YAML::BeginMap;
        for (const auto & year : years) {
            if (year == to_string(Year::All)) continue;
            SettingDef::Config::year = year;

            out << YAML::Key << year;
            out << YAML::BeginMap;

            for (const auto & polarity : polarities) {
                if (polarity == to_string(Polarity::All)) continue;
                SettingDef::Config::polarity = polarity;

                try {
                    auto et = EventType();
                    et.Init();
                    auto lumi = et.GetTupleHolder().GetLuminosity();
                    et.Close();

                    out << YAML::Key << polarity;
                    out << YAML::BeginMap;
                    out << YAML::Key << "value" << YAML::Value << round(lumi.first * 1000) / 1000;
                    out << YAML::Key << "err" << YAML::Value << round(lumi.second * 1000) / 1000;
                    out << YAML::EndMap;
                } catch (const exception & e) {
                    cout << e.what() << endl;
                    continue;
                }
            }
            out << YAML::EndMap;
        }
        out << YAML::EndMap;

        TString  _name = fmt::format("LumiInfo_{0}.yaml", SettingDef::Config::project);
        ofstream _file(_name);
        if (!_file.is_open()) MessageSvc::Error("Unable to open file", _name, "EXIT_FAILURE");
        _file << out.c_str() << endl;
        _file.close();
    }

    return 0;
}
