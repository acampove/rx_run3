//-----------------------------------------------------------------------------
// Implementation file for target : efficiencyCreate
//
// 2018-11-15 : Renato Quagliani, Simone Bifani
//-----------------------------------------------------------------------------
#include "EfficiencyCalculator.hpp"
#include "EnumeratorSvc.hpp"
#include "EventType.hpp"
#include "HelperSvc.hpp"
#include "IOSvc.hpp"
#include "ParserSvc.hpp"

int main(int argc, char ** argv) {
    auto tStart = chrono::high_resolution_clock::now();
    // Call our Parser for YAML file
    ParserSvc parser("");
    parser.Init(argc, argv);

    if (parser.Run(argc, argv) != 1) { return 0; }

    //--- define the set of input variables over which make the division.......
    Prj                                prj        = hash_project(SettingDef::Config::project);
    Analysis                           ana        = hash_analysis(SettingDef::Config::ana);
    Q2Bin                              q2bin      = hash_q2bin(SettingDef::Config::q2bin);
    vector< pair< TString, TString > > _particles = GetParticleBranchNames(prj, ana, q2bin);

    for (auto & el : _particles) { std::cout << "particle : " << el.first << "  | " << el.second << std::endl; }

    function< RooRealVar(TString _particle, TString _property, double min, double max, TString unit, int nBins) > _makeRooRealVar;
    // Lamda function, generate RooRealVar for a particle, propertu, in min, max with a unit and with a given Nb of bins
    _makeRooRealVar = [](TString _particle, TString _property, double min, double max, TString unit, int nBins) {
        RooRealVar var = RooRealVar(_particle + "_" + _property, _particle + "_" + _property, min, max, unit);
        var.setBins(nBins);
        return var;
    };

    // Generate the set of Variables Bins/Ranges hardcoded
    vector< RooRealVar > _variables;
    for (auto & _part : _particles) {
        _variables.push_back(_makeRooRealVar(_part.first, "PT", 0, 45e3, "MeV/c^{2}", 100));
        _variables.push_back(_makeRooRealVar(_part.first, "ETA", 0, 45e3, "MeV/c^{2}", 7));
    }
    for (const auto & var : _variables) {
        std::cout << "==== Created Variable : " << std::endl;
        var.Print();
    }
    abort();

    // Set of divisions to make :
    // 1- Numerator : Preselection Cuts all weights enabled / Truth matched MC
    // 2-

    //======================  Define the set of years/efficiencies and polarities over which we want to have the efficiencies dumped
    // for( const auto _comb : Combinations){
    // Overload default switchers and build a "default" event Type using all the other fields specified by the yaml config file in Config only field.

    //--- HACK HERE FOR DEFINING ONE AND FOR ALL THE EFFICIENCY STRATEGY
    // Denominator builder, default EventType from yaml file parsed overloading the year,pol,trigger
    EventType et = EventType();
    if (!et.IsSignalMC()) { MessageSvc::Error("SignalMC only supported", "", "EXIT_FAILURE"); }
    MessageSvc::Line();
    MessageSvc::Line();
    MessageSvc::Warning("=============== STARTING EFFICIENCY CALCULATION ===============================");
    cout << et << std::endl;
    MessageSvc::Line();
    MessageSvc::Line();
    ZippedEventType _zip = et.Zip();
    // This evaluation is correct only when the final tuple we filter/cut is fully linked to the ancestor SKIMMED one! If we start to lose some event/file we have serious issues in this computation
    // Get the analogous MCDecayTree of this EventType with no cuts in it from a SKIM processed step.
    _zip.tupleName     = "MCT";
    _zip.cutOption     = "no";
    _zip.tupleOption   = "pro";
    _zip.weightOption  = "no";    //< we actually have to plug here "MC1" when available on MC1 tuple step and this will depend if the numerator EventType has MC1 weights for efficiencies
    TString   _ver     = "PID";   //< will need to be updated for denominator def to "pro-MVA" , last Step
    EventType et_denom = EventType(_zip);
    // Reset to original one to make default EventType building working fine.
    SettingDef::Tuple::proVer = _ver;
    // abort();
    std::cout << RED << "================= NEW EFFICIENCY CALCULATION ==================" << RESET << std::endl;
    EfficiencyCalculator eh = EfficiencyCalculator();
    eh.SetRatio(et, et_denom);

    //--- get out the GeneratorLevelEff and Filtering Efficiencies. Those are saved on disk always !
    eh.ComputeGeneratorEfficiency();
    eh.ComputeFilteringEfficiency();

    /*
      //Tell the calculator which are the variables in the Pas and TOT to be used to divide things
      for( auto & _var : _variables){
        eh.SetVariables(_var.first, _var.second, 0);
      }

      // --- Compute the actual efficnecy.
      eh.ComputeEfficiency();

      // --- Save to Disk all efficiencies calculated here.
      // eh.SaveToDisk();
      eh.DumpPlots();

      MessageSvc::Line();
      MessageSvc::Line();
      MessageSvc::Warning("=============== DONE EFFICIENCY CALCULATION ===============================");
      MessageSvc::Line();
      MessageSvc::Line();

      auto tEnd = chrono::high_resolution_clock::now();
      MessageSvc::Line();
      MessageSvc::Warning((TString) SettingDef::IO::exe, "Took", to_string(chrono::duration_cast<chrono::seconds>(tEnd - tStart).count()), "seconds");
      MessageSvc::Line();
      */

    return 0;
}
