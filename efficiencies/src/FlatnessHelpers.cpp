#ifndef FLATNESS_HELPERS_CPP
#define FLATNESS_HELPERS_CPP

#include "FlatnessHelpers.hpp"
#include "ConstDef.hpp"
#include "EfficiencySvc.hpp"
#include "MessageSvc.hpp"

double FlatnessHelpers::ProducedDecays(const Prj & _project, const Year & _year, const Polarity & _polarity, const Analysis & _ana) {
    auto   lumi             = LoadLuminosity(_project, _year, _polarity);
    double Luminosity       = lumi.first;
    double Luminosity_ERROR = lumi.second;
    // https://lhcb.web.cern.ch/lhcb/speakersbureau/html/bb_ProductionAngles.html
    const double sigma_pp_BpmX_7TeV  = 251.8E-06;               // barn
    const double sigma_pp_BpmX_8TeV  = 291.6E-06;               // barn
    const double sigma_pp_BpmX_13TeV = 527.3E-06 * 13. / 14.;   // barn (scaled to 13 TeV)
    double       lumi_barn           = Luminosity * 1E12;       // from /pb = /barn
    double       branching_ratio     = 0.;
    if (_project == Prj::RK && _ana == Analysis::EE) { branching_ratio = PDG::BF::Bu2KJPs * PDG::BF::JPs2EE; }
    if (_project == Prj::RK && _ana == Analysis::MM) { branching_ratio = PDG::BF::Bu2KJPs * PDG::BF::JPs2MM; }
    if (_project == Prj::RKst && _ana == Analysis::EE) { branching_ratio = PDG::BF::Bd2KstJPs * PDG::BF::JPs2EE * PDG::BF::Kst2KPi; }
    if (_project == Prj::RKst && _ana == Analysis::MM) { branching_ratio = PDG::BF::Bd2KstJPs * PDG::BF::JPs2MM * PDG::BF::Kst2KPi; }
    double cross_sectionProductionB = 0.;

    cout << to_string(_project) << " - " << to_string(_year) << " - " << to_string(_ana) << endl;
    if (_year == Year::Y2011) {
        cross_sectionProductionB = sigma_pp_BpmX_7TeV;
    } else if (_year == Year::Y2012) {
        cross_sectionProductionB = sigma_pp_BpmX_8TeV;
    } else if (_year == Year::Y2015 || _year == Year::Y2016 || _year == Year::Run2p1 || _year == Year::Y2015 || _year == Year::Y2016 || _year == Year::Run2p2) {
        cross_sectionProductionB = sigma_pp_BpmX_13TeV;
    } else {
        MessageSvc::Error("Invalid input year", "", "EXIT_FAILURE");
    }
    // esure luminosity is in ??

    cout << to_string(_project) << " - " << to_string(_year) << " - " << to_string(_polarity) << " - " << to_string(_ana) << endl;

    cout << "sigma( pp -> bb ) barn =  " << cross_sectionProductionB << endl;
    cout << "factors           =  2* 0.4" << endl;
    cout << "lumi_barn         =  " << lumi_barn << endl;
    cout << "BranchingRatio    =  " << branching_ratio << endl;
    double expected = branching_ratio * cross_sectionProductionB * 2 * 0.4 * lumi_barn;
    cout << "sigma(pp->bb) * factors * lumi_barn * BranchingRatio (4pi) = " << expected << endl;
    // cout<< expected << endl;
    cout << to_string(_project) << " " << to_string(_year) << " " << to_string(_ana) << " : " << Luminosity << "  " << expected << endl;
    return expected;
};

vector< pair< Year, Polarity > > FlatnessHelpers::GetYearsAndPolarities(const Year & _year) {
    std::vector< pair< Year, Polarity > > _return;
    if (_year == Year::Run1) {
        _return = {{Year::Y2011, Polarity::MU}, {Year::Y2011, Polarity::MD}, {Year::Y2012, Polarity::MU}, {Year::Y2012, Polarity::MD}};
    } else if (_year == Year::Run2p1) {
        _return = {{Year::Y2015, Polarity::MU}, {Year::Y2015, Polarity::MD}, {Year::Y2016, Polarity::MU}, {Year::Y2016, Polarity::MD}};
    } else if (_year == Year::Run2p2) {
        _return = {{Year::Y2017, Polarity::MU}, {Year::Y2017, Polarity::MD}, {Year::Y2018, Polarity::MU}, {Year::Y2018, Polarity::MD}};
    } else {
        MessageSvc::Error("GetYearsAndPolarities Invalid input Year", to_string(_year), "EXIT_FAILURE");
    }
    return _return;
}

#endif   // !FLATNESS_HELPERS_CPP