#ifndef PARSERSVC_CPP
#define PARSERSVC_CPP

#include "ParserSvc.hpp"

#include "HelperSvc.hpp"

#include "SettingDef.hpp"

#include <fmt_ostream.h>

#include "EventType.hpp"
#include "FitConfiguration.hpp"
#include "EfficiencyForFitHandler.hpp"

ParserSvc::ParserSvc(TString _option) {
    if (!_option.Contains("quiet")) MessageSvc::Banner();
    //
    m_option = _option;
    CLI::App m_parserCLI11("");
    //
    m_return = 0;
    //
    if (!_option.Contains("quiet")) LHCbStyle();
    //
    if (HasEOS())
        SettingDef::IO::useEOS = true;
    else
        SettingDef::IO::useEOS = false;
    //
    if (SettingDef::IO::exe.Contains(".py") && SettingDef::IO::exe.Contains("submit")) {
        SettingDef::trowLogicError  = true;
        SettingDef::Tuple::addTuple = false;
    } else {
        if( SettingDef::Tuple::datasetCache){ 
            SettingDef::trowLogicError  = true;
            SettingDef::Tuple::addTuple = false;
        }else{
            SettingDef::trowLogicError  = false;
            SettingDef::Tuple::addTuple = true;
        }
    }

    if ((TString) getenv("LB2LEMUROOT") != "") {
        cout << MAGENTA;
        MessageSvc::Line();
        MessageSvc::Print("Setting up for", (TString) getenv("LB2LEMUROOT"));
        MessageSvc::Line();
        cout << RESET;
        SettingDef::IO::gangaDir  = (TString) getenv("LB2LEMUROOT") + "/tuples";
        SettingDef::IO::configDir = (TString) getenv("LB2LLLSYS") + "/yaml";
        SettingDef::IO::dataDir   = (TString) getenv("LB2LLLSYS") + "/data";
        SettingDef::IO::eosHome   = (TString) getenv("EOSHOME") + "/Lb2Lemu";
    }
}

void ParserSvc::Init(int argc, char ** argv) {
    SettingDef::IO::exe = ((TString) argv[0]).ReplaceAll(".out", "");
    if (SettingDef::IO::exe.Contains(".py") && SettingDef::IO::exe.Contains("submit")) {
        SettingDef::trowLogicError  = true;
        SettingDef::Tuple::addTuple = false;
    } else {
        SettingDef::trowLogicError  = false;
        SettingDef::Tuple::addTuple = true;  
        if( SettingDef::Tuple::datasetCache ){
            SettingDef::Tuple::addTuple = false;  
            SettingDef::trowLogicError  = true;
        }
    }
    m_fileYAML.clear();
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            if ((TString) argv[i] == "--help") m_return = 1;

            if ((TString) argv[i] == "--debug") m_debug = true;

            if ((TString) argv[i] == "--yaml") {
                if ((TString) argv[i + 1] != "")
                    m_fileYAML.push_back((TString) argv[i + 1]);
                else
                    m_fileYAML.push_back(SettingDef::IO::configDir + "/config.yaml");
            }
        }
    }
    if (m_fileYAML.size() != 0) SettingDef::IO::yaml = ((TString) m_fileYAML[0]).Remove(0, m_fileYAML[0].Last('/') + 1).ReplaceAll("config-", "").ReplaceAll("config", "").ReplaceAll(".yaml", "");
    if (m_fileYAML.size() == 0) {
        InitCLI11();
    } else {
        InitYAML();
    }
    return;
}

void ParserSvc::Init(TString _fileYAML, TString _fileYAML2OL) {
    m_fileYAML.clear();
    m_fileYAML.push_back(_fileYAML);
    if (_fileYAML2OL != "") m_fileYAML.push_back(_fileYAML2OL);
    SettingDef::IO::yaml = ((TString) m_fileYAML[0]).Remove(0, m_fileYAML[0].Last('/') + 1).ReplaceAll("config-", "").ReplaceAll("config", "").ReplaceAll(".yaml", "");
    if (_fileYAML2OL != "") { SettingDef::IO::yaml = ((TString) m_fileYAML[1]).Remove(0, m_fileYAML[1].Last('/') + 1).ReplaceAll("config-", "").ReplaceAll("config", "").ReplaceAll(".yaml", ""); }
    if (m_fileYAML[0] != "") InitYAML();
    return;
}

void ParserSvc::InitCLI11() {
    cout << WHITE;
    MessageSvc::Line();
    MessageSvc::Print("Setting CLI11 parser");
    MessageSvc::Line();
    cout << RESET;

    // SettingDef
    m_parserCLI11.add_option("--debug", SettingDef::debug, "Run in debug mode [CO,CH,TH,WH,ET,FC,FH,FM,FP]");

    // SettingDef::Config
    m_parserCLI11.add_option("--prj", SettingDef::Config::project, "Project type [RKst,RK,RPhi,RL]")->group("Config");
    m_parserCLI11.add_option("--ana", SettingDef::Config::ana, "Analysis type [EE,MM]")->group("Config");
    m_parserCLI11.add_option("--smp", SettingDef::Config::sample, "Sample [check AllowedConf for more info]")->group("Config");
    m_parserCLI11.add_option("--q2", SettingDef::Config::q2bin, "Q2 bin [low,central,high,jps,psi]")->group("Config");
    m_parserCLI11.add_option("--year", SettingDef::Config::year, "Year [11,12,R1,15,16,R2p1,17,18,R2p2]")->group("Config");
    m_parserCLI11.add_option("--mag", SettingDef::Config::polarity, "Magnet polarity [MD,MU]")->group("Config");
    m_parserCLI11.add_option("--l0", SettingDef::Config::trigger, "L0 category [L0I,L0L]")->group("Config");
    m_parserCLI11.add_option("--brem", SettingDef::Config::brem, "Brem category [G0,G1,G2]")->group("Config");
    m_parserCLI11.add_option("--trk", SettingDef::Config::track, "Track category [LL,DD]")->group("Config");

    // SettingDef::Cut
    m_parserCLI11.add_option("--cut", SettingDef::Cut::option, "Cut options [noPID,noL0,noTRG,noMVA] ('-' separated)")->group("Cut");

    // SettingDef::Weight
    m_parserCLI11.add_option("--w", SettingDef::Weight::option, "Weights options [PID,L0,...] ('-' separated)")->group("Weight");

    // SettingDef::Tuple
    m_parserCLI11.add_option("--tup", SettingDef::Tuple::option, "Tuple options [gng,pro,cre,spl]")->group("Tuple");
    m_parserCLI11.add_option("--gng", SettingDef::Tuple::gngVer, "Tuple version - Ganga")->group("Tuple");
    m_parserCLI11.add_option("--pro", SettingDef::Tuple::proVer, "Tuple version - Process")->group("Tuple");
    m_parserCLI11.add_option("--cre", SettingDef::Tuple::creVer, "Tuple version - Create")->group("Tuple");
    m_parserCLI11.add_option("--spl", SettingDef::Tuple::splVer, "Tuple version - SPlot")->group("Tuple");
    m_parserCLI11.add_option("--frac", SettingDef::Tuple::frac, "Run on reduced number of events")->group("Tuple");

    // SettingDef::Fit
    m_parserCLI11.add_option("--fit", SettingDef::Fit::option, "Fit options [] ('-' separated)")->group("Fit");

    // SettingDef::Efficiency
    m_parserCLI11.add_option("--eff", SettingDef::Efficiency::option, "Efficiency options [] ('-' separated)")->group("Efficiency");

    return;
}

void ParserSvc::InitYAML() {
    cout << WHITE;
    MessageSvc::Line();
    MessageSvc::Print("Setting YAML parser with " + to_string(m_fileYAML.size()) + " yaml(s)");
    MessageSvc::Line();
    cout << RESET;
    for (size_t i = 0; i < m_fileYAML.size(); i++) {
        if (!IOSvc::ExistFile(m_fileYAML[i])) MessageSvc::Error("InitYAML", (TString) "Invalid", m_fileYAML[i], "file", "EXIT_FAILURE");

        if (i == 0) {
            cout << WHITE;
            MessageSvc::Line();
            MessageSvc::Print("Parsing " + m_fileYAML[i]);
            MessageSvc::Line();
            cout << RESET;
        } else {
            cout << MAGENTA;
            MessageSvc::Line();
            MessageSvc::Print("Parsing " + m_fileYAML[i] + " w/o loading year, polarity, trigger, triggerConf, extraCut if \"\"");
            MessageSvc::Line();
            cout << WHITE;
        }

        m_parserYAML = YAML::LoadFile(m_fileYAML[i].Data());
        if (m_parserYAML.IsNull()) MessageSvc::Error("InitYAML", (TString) "Invalid", m_fileYAML[i], "parser", "EXIT_FAILURE");
        if (m_debug) {
            cout << YELLOW;
            MessageSvc::Line();
            MessageSvc::Debug("InitYAML", (TString) "Nodes =", to_string(m_parserYAML.size()));
            for (YAML::iterator _it = m_parserYAML.begin(); _it != m_parserYAML.end(); ++_it) { MessageSvc::Debug("InitYAML", (TString) "SubNodes =", to_string(_it->second.size())); }
            cout << YELLOW;
            cout << m_parserYAML << endl;
            MessageSvc::Line();
            cout << RESET;
        }

        ParseSettingYAML(m_parserYAML["Setting"]);
        ParseConfigYAML(m_parserYAML["Config"]);
        ParseCutYAML(m_parserYAML["Cut"]);
        ParseWeightYAML(m_parserYAML["Weight"]);
        ParseTupleYAML(m_parserYAML["Tuple"]);

        ParseEventsYAML(m_parserYAML["Events"]);

        ParseEfficiencyYAML(m_parserYAML["Efficiency"]);

        // Fast way to bypass the parsing and Initialization of FitConfigurations ( too slow ) in parsing yaml files in python scripts (hltcorrections only uses this trick) .
        if (!SettingDef::IO::exe.Contains("noFITParse")) {
            ParseFitYAML(m_parserYAML["Fit"]);
            ParseToyYAML(m_parserYAML["Toy"]);
        }
    }
    return;
}

void ParserSvc::ParseSettingYAML(YAML::Node _nodeYAML) {
    if (m_debug) {
        MessageSvc::Debug("ParseSettingYAML", (TString) "Nodes =", to_string(_nodeYAML.size()));
        cout << YELLOW;
        cout << _nodeYAML << endl;
        MessageSvc::Line();
        cout << RESET;
    }
    if (_nodeYAML) {
        if (_nodeYAML["name"]) {
            if (SettingDef::name == "")
                SettingDef::name = _nodeYAML["name"].as< TString >();
            else
                SettingDef::name = SettingDef::name + SettingDef::separator + _nodeYAML["name"].as< TString >();
        }

        if (_nodeYAML["debug"]) SettingDef::debug = _nodeYAML["debug"].as< TString >();
        if (SettingDef::debug.Contains("KRN")) SettingDef::debug.ReplaceAll("KRN", "CO-CH-TH-TR-WH-ET");
        if (SettingDef::debug.Contains("TUP")) SettingDef::debug.ReplaceAll("TUP", "TH-TR");
        if (SettingDef::debug.Contains("FIT")) SettingDef::debug.ReplaceAll("FIT", "FC-FH-FM-FP-FT");
        if (SettingDef::debug.Contains("PS")) SetDebug(true);

        if (_nodeYAML["outDir"]) {
            SettingDef::IO::outDir = _nodeYAML["outDir"].as< TString >();
            if ((SettingDef::IO::outDir != "FitDir") && !IOSvc::ExistDir(SettingDef::IO::outDir)) IOSvc::MakeDir(SettingDef::IO::outDir);
        }
        if (_nodeYAML["eos"])     SettingDef::IO::useEOS     = _nodeYAML["eos"].as< bool >();
        if (_nodeYAML["eosHome"]) SettingDef::IO::useEOSHome = _nodeYAML["eosHome"].as< bool >();
    }
    return;
}

void ParserSvc::ParseConfigYAML(YAML::Node _nodeYAML) {
    if (m_debug) {
        MessageSvc::Debug("ParseConfigYAML", (TString) "Nodes =", to_string(_nodeYAML.size()));
        cout << YELLOW;
        cout << _nodeYAML << endl;
        MessageSvc::Line();
        cout << RESET;
    }
    if (_nodeYAML) {
        if (_nodeYAML["project"]) SettingDef::Config::project = _nodeYAML["project"].as< TString >();
        if (_nodeYAML["ana"]) SettingDef::Config::ana = _nodeYAML["ana"].as< TString >();
        if (_nodeYAML["sample"]) SettingDef::Config::sample = _nodeYAML["sample"].as< TString >();
        if (_nodeYAML["q2bin"]) SettingDef::Config::q2bin = _nodeYAML["q2bin"].as< TString >();
        if (_nodeYAML["year"]) SettingDef::Config::year = _nodeYAML["year"].as< TString >() != "" ? _nodeYAML["year"].as< TString >() : SettingDef::Config::year;
        if (_nodeYAML["polarity"]) SettingDef::Config::polarity = _nodeYAML["polarity"].as< TString >() != "" ? _nodeYAML["polarity"].as< TString >() : SettingDef::Config::polarity;
        if (_nodeYAML["trigger"]) SettingDef::Config::trigger = _nodeYAML["trigger"].as< TString >() != "" ? _nodeYAML["trigger"].as< TString >() : SettingDef::Config::trigger;
        if (_nodeYAML["triggerConf"]) SettingDef::Config::triggerConf = _nodeYAML["triggerConf"].as< TString >() != "" ? _nodeYAML["triggerConf"].as< TString >() : SettingDef::Config::triggerConf;
        if (_nodeYAML["brem"]) SettingDef::Config::brem = _nodeYAML["brem"].as< TString >();
        if (_nodeYAML["track"]) SettingDef::Config::track = _nodeYAML["track"].as< TString >();
    }
    return;
}

void ParserSvc::ParseCutYAML(YAML::Node _nodeYAML) {
    if (m_debug) {
        MessageSvc::Debug("ParseCutYAML", (TString) "Nodes =", to_string(_nodeYAML.size()));
        cout << YELLOW;
        cout << _nodeYAML << endl;
        MessageSvc::Line();
        cout << RESET;
    }
    if (_nodeYAML) {
        if (_nodeYAML["option"])      SettingDef::Cut::option      = _nodeYAML["option"].as< TString >();
        if (_nodeYAML["extraCut"])    SettingDef::Cut::extraCut    = _nodeYAML["extraCut"].as< TString >() != "" ? _nodeYAML["extraCut"].as< TString >() : SettingDef::Cut::extraCut;
        if (_nodeYAML["extraEEOnly"]) SettingDef::Cut::extraEEOnly = _nodeYAML["extraEEOnly"].as< bool >();
        if (_nodeYAML["extraMMOnly"]) SettingDef::Cut::extraMMOnly = _nodeYAML["extraMMOnly"].as< bool >();
        if( SettingDef::Cut::extraMMOnly && SettingDef::Cut::extraEEOnly ){
            MessageSvc::Error("extraEEOnly,extraMMOnly are mutually exclusive, fix the yaml (false,true) , (true,false), (false,false)", "", "EXIT_FAILURE");
        }
        if (_nodeYAML["tightLowQ2"])  SettingDef::Cut::tightLowQ2  = _nodeYAML["tightLowQ2"].as< bool >();
        if (_nodeYAML["force"])       SettingDef::Cut::force       = _nodeYAML["force"].as< bool >();
        if (_nodeYAML["mvaVer"]) {
            SettingDef::Cut::mvaVer   = _nodeYAML["mvaVer"].as< TString >();
            SettingDef::Cut::extraCut = UpdateMVACut(TCut(SettingDef::Cut::extraCut));
        }
    }
    return;
}

void ParserSvc::ParseWeightYAML(YAML::Node _nodeYAML) {
    if (m_debug) {
        MessageSvc::Debug("ParseWeightYAML", (TString) "Nodes =", to_string(_nodeYAML.size()));
        cout << YELLOW;
        cout << _nodeYAML << endl;
        MessageSvc::Line();
        cout << RESET;
    }
    if (_nodeYAML) {
        if (_nodeYAML["option"]) SettingDef::Weight::option = _nodeYAML["option"].as< TString >();
        if (_nodeYAML["config"]) SettingDef::Weight::config = _nodeYAML["config"].as< TString >();
        if (_nodeYAML["trkVer"]) SettingDef::Weight::trkVer = _nodeYAML["trkVer"].as< TString >();
        if (_nodeYAML["pidVer"]) SettingDef::Weight::pidVer = _nodeYAML["pidVer"].as< TString >();
        if (_nodeYAML["l0Ver"]) SettingDef::Weight::l0Ver = _nodeYAML["l0Ver"].as< TString >();
        if (_nodeYAML["hltVer"]) SettingDef::Weight::hltVer = _nodeYAML["hltVer"].as< TString >();
        if (_nodeYAML["mcVer"]) SettingDef::Weight::mcVer = _nodeYAML["mcVer"].as< TString >();
        if (_nodeYAML["iBS"]) SettingDef::Weight::iBS = _nodeYAML["iBS"].as< int >();
        if (_nodeYAML["useBS"]) SettingDef::Weight::useBS = _nodeYAML["useBS"].as< bool >();
        if (_nodeYAML["useStatusL0Formula"]) SettingDef::Weight::useStatusL0Formula = _nodeYAML["useStatusL0Formula"].as<bool>();
        if (_nodeYAML["priorChain"]) SettingDef::Weight::priorChain = _nodeYAML["priorChain"].as<bool>();
        if (_nodeYAML["usePIDPTElectron"]) SettingDef::Weight::usePIDPTElectron = _nodeYAML["usePIDPTElectron"].as<bool>();
        if (_nodeYAML["useMCRatioPID"]) SettingDef::Weight::useMCRatioPID = _nodeYAML["useMCRatioPID"].as<bool>();
        if (_nodeYAML["q2SmearFileTag"]) SettingDef::Weight::q2SmearFileTag = _nodeYAML["q2SmearFileTag"].as<TString>();
        if (_nodeYAML["q2SmearDiffVar"]) SettingDef::Weight::q2SmearDiffVar = _nodeYAML["q2SmearDiffVar"].as<TString>();
        if (_nodeYAML["TrkFromRKst"]) SettingDef::Weight::TrkFromRKst = _nodeYAML["TrkFromRKst"].as<bool>();
        if (_nodeYAML["L0I_EToE"]) SettingDef::Weight::L0I_EToE =  _nodeYAML["L0I_EToE"].as<bool>();
    }
    return;
}

void ParserSvc::ParseTupleYAML(YAML::Node _nodeYAML) {
    if (m_debug) {
        MessageSvc::Debug("ParseTupleYAML", (TString) "Nodes =", to_string(_nodeYAML.size()));
        cout << YELLOW;
        cout << _nodeYAML << endl;
        MessageSvc::Line();
        cout << RESET;
    }
    if (_nodeYAML) {
        if (_nodeYAML["option"])       SettingDef::Tuple::option = _nodeYAML["option"].as< TString >();
        if (_nodeYAML["gngVer"])       SettingDef::Tuple::gngVer = _nodeYAML["gngVer"].as< TString >();
        if (_nodeYAML["proVer"])       SettingDef::Tuple::proVer = _nodeYAML["proVer"].as< TString >();
        if (_nodeYAML["creVer"])       SettingDef::Tuple::creVer = _nodeYAML["creVer"].as< TString >();
        if (_nodeYAML["splVer"])       SettingDef::Tuple::splVer = _nodeYAML["splVer"].as< TString >();
        if (_nodeYAML["outVer"])       SettingDef::Tuple::outVer = _nodeYAML["outVer"].as< TString >();
        if (_nodeYAML["fileName"])     SettingDef::Tuple::fileName = _nodeYAML["fileName"].as< TString >();
        if (_nodeYAML["tupleName"])    SettingDef::Tuple::tupleName = _nodeYAML["tupleName"].as< TString >();
        if (_nodeYAML["dataFrame"])    SettingDef::Tuple::dataFrame = _nodeYAML["dataFrame"].as< bool >();
        if (_nodeYAML["branches"])     SettingDef::Tuple::branches = _nodeYAML["branches"].as< bool >();
        if (_nodeYAML["branchList"])   SettingDef::Tuple::branchList = _nodeYAML["branchList"].as< vector< TString > >();
        if (_nodeYAML["aliases"])      SettingDef::Tuple::aliases = _nodeYAML["aliases"].as< bool >();
        if (_nodeYAML["frac"])         SettingDef::Tuple::frac = _nodeYAML["frac"].as< double >();
        if (_nodeYAML["chainexctrg"])  SettingDef::Tuple::chainexctrg = _nodeYAML["chainexctrg"].as< bool >();
        if (_nodeYAML["datasetCache"]) SettingDef::Tuple::datasetCache = _nodeYAML["datasetCache"].as< bool >();
        if (_nodeYAML["addTuple"])     SettingDef::Tuple::addTuple = _nodeYAML["addTuple"].as< bool >();
        if (_nodeYAML["noInit"])       SettingDef::Tuple::noInit = _nodeYAML["noInit"].as<bool>();
	if (_nodeYAML["useURLS"])      SettingDef::Tuple::useURLS = _nodeYAML["useURLS"].as<bool>();

        // All Vars in one GO!
        if (_nodeYAML["IsoBins"]) {
            SettingDef::Tuple::isoBins   = {};
            vector< TString > ids_filled = {};
            for (auto specsBin = _nodeYAML["IsoBins"].begin(); specsBin != _nodeYAML["IsoBins"].end(); ++specsBin) {
                vector< tuple< TString, int , double, double, TString, TString> > _binsVar; //information on the 2 Dimensions for histogram making
                const YAML::Node & _isoNode    = *specsBin;
                TString            var_ID      = _isoNode["ID"].as< TString >();
                vector< int >      vars_bins   = _isoNode["Bins"].as< vector< int > >();
                vector< TString >  vars_naming = _isoNode["Vars"].as< vector< TString > >();
                TString _classType = "TH1D";
                if( vars_bins.size() ==2){
                    _classType = _isoNode["classType"] ? _isoNode["classType"].as<TString>() : "TH2Poly";
                }
                double min_x = _isoNode["minX"] ? _isoNode["minX"].as<double>() :  std::numeric_limits<double>::min();
                double max_x = _isoNode["maxX"] ? _isoNode["maxX"].as<double>() :  std::numeric_limits<double>::max();
                double min_y = _isoNode["minY"] ? _isoNode["minY"].as<double>() :  std::numeric_limits<double>::min();
                double max_y = _isoNode["maxY"] ? _isoNode["maxY"].as<double>() :  std::numeric_limits<double>::max();
                TString _titleX = _isoNode["titleX"] ?  _isoNode["titleX"].as< TString>() : var_ID;
                TString _titleY = _isoNode["titleY"] ?  _isoNode["titleY"].as< TString>() : "Counts";
                if (vars_bins.size() != vars_naming.size()) { 
                    for( auto & v : vars_bins)   cout <<" varSize   "<< v << endl;
                    for( auto & v : vars_naming) cout <<" varNaming "<< v << endl;
                    MessageSvc::Error("Invalid sizes of Var, Bins ", var_ID, "EXIT_FAILURE");                     
                }
                for (int i = 0; i < vars_bins.size(); ++i) { 
                    if( i == 0 ){ 
                        _binsVar.push_back(make_tuple(vars_naming[i], vars_bins[i] , min_x, max_x , _titleX , _classType)); 
                    }else if( i ==1){
                        _binsVar.push_back(make_tuple(vars_naming[i], vars_bins[i] , min_y, max_y , _titleY, _classType)); 
                    }
                }
                if (!(std::find(ids_filled.begin(), ids_filled.end(), var_ID) == ids_filled.end())) {
                    MessageSvc::Error("Duplicate IDS for IsoBins", var_ID, "EXIT_FAILURE");
                } else {
                    SettingDef::Tuple::isoBins.push_back(make_pair(var_ID, _binsVar));
                    ids_filled.push_back(var_ID);
                }
            }
        }
    }
    return;
}

void ParserSvc::ParseEventsYAML(YAML::Node _nodeYAML) {
    if (m_debug) {
        MessageSvc::Debug("ParseEventsYAML", (TString) "Nodes =", to_string(_nodeYAML.size()));
        cout << YELLOW;
        cout << _nodeYAML << endl;
        MessageSvc::Line();
        cout << RESET;
    }
    if (_nodeYAML && (_nodeYAML.size() > 0)) {
        MessageSvc::Line();
        MessageSvc::Info(Color::Cyan, "InitYAML", (TString) "EventType(s) to parse", to_string(_nodeYAML.size()));

        SettingDef::Events::cutOptionCL    = _nodeYAML["cutOptionCL"] ? _nodeYAML["cutOptionCL"].as< TString >() : SettingDef::Cut::option;
        SettingDef::Events::cutOptionSigMC = _nodeYAML["cutOptionSigMC"] ? _nodeYAML["cutOptionSigMC"].as< TString >() : SettingDef::Cut::option;
        SettingDef::Events::cutOptionBkgMC = _nodeYAML["cutOptionBkgMC"] ? _nodeYAML["cutOptionBkgMC"].as< TString >() : SettingDef::Cut::option;

        SettingDef::Events::weightOption = _nodeYAML["weightOption"] ? _nodeYAML["weightOption"].as< TString >() : SettingDef::Weight::option;

        SettingDef::Events::tupleOption = _nodeYAML["tupleOption"] ? _nodeYAML["tupleOption"].as< TString >() : SettingDef::Tuple::option;

        MessageSvc::Info("cutOptionCL", SettingDef::Events::cutOptionCL);
        MessageSvc::Info("cutOptionSigMC", SettingDef::Events::cutOptionSigMC);
        MessageSvc::Info("cutOptionBkgMC", SettingDef::Events::cutOptionBkgMC);
        MessageSvc::Info("weightOption", SettingDef::Events::weightOption);
        MessageSvc::Info("tupleOption", SettingDef::Events::tupleOption);
        MessageSvc::Line();

        int _count = 0;
        for (YAML::iterator _it = _nodeYAML.begin(); _it != _nodeYAML.end(); ++_it) {
            if (_it->first.as< TString >() == "event") {
                YAML::Node _eventYAML = _it->second;
                try {
                    MessageSvc::Info("InitYAML", (TString) "Parsing EventType", to_string(_count));
                    _count++;
                    EventType _et = GetEventTypeYAML(_eventYAML);
                    if (_et.IsInitialized()) SettingDef::Events::types.emplace_back(_et);
                } catch (const exception & e) {
                    cout << e.what() << endl;
                    SettingDef::Events::fails.push_back(_eventYAML["sample"].as< TString >());
                    continue;
                }
            }
        }

        int  _size  = SettingDef::Events::types.size();
        auto _erase = [&](const EventType & _sample) {
            if ((_sample.GetQ2bin() != hash_q2bin(SettingDef::Config::q2bin)) && (hash_q2bin(SettingDef::Config::q2bin) != Q2Bin::All)) MessageSvc::Warning("InitYAML", (TString) "Remove", _sample.GetSample(), to_string(_sample.GetTrigger()), to_string(_sample.GetBrem()), to_string(_sample.GetQ2bin()));
            return (_sample.GetQ2bin() != hash_q2bin(SettingDef::Config::q2bin)) && (hash_q2bin(SettingDef::Config::q2bin) != Q2Bin::All);
        };
        SettingDef::Events::types.erase(remove_if(SettingDef::Events::types.begin(), SettingDef::Events::types.end(), _erase), SettingDef::Events::types.end());

        sort(SettingDef::Events::types.begin(), SettingDef::Events::types.end());

        MessageSvc::Info("InitYAML", (TString) "EventType(s) to parse", to_string(_nodeYAML.size()));
        MessageSvc::Info("InitYAML", (TString) "EventType(s) parsed", to_string(SettingDef::Events::types.size()));
        if (SettingDef::Events::fails.size() != 0) MessageSvc::Warning("InitYAML", (TString) "EventType(s) failed", to_string(SettingDef::Events::fails.size()));
        MessageSvc::Line();
    }
    return;
}

void ParserSvc::ParseEfficiencyYAML(YAML::Node _nodeYAML) {
    if (m_debug) {
        MessageSvc::Debug("ParseEfficiencyYAML", (TString) "Nodes =", to_string(_nodeYAML.size()));
        cout << YELLOW;
        cout << _nodeYAML << endl;
        MessageSvc::Line();
        cout << RESET;
    }
    auto CheckEfficiencyOption = [](const TString & _myOption, const vector< TString > & _allowedOptions) {
        auto _options = TokenizeString(_myOption, "-");
        if (_options.size() == 0) return true;
        for (auto & _opt : _options) {
            if (!CheckVectorContains(_allowedOptions, _opt)) { MessageSvc::Error("ParseEfficiencyYAML", (TString) "Option not supported", _myOption, "EXIT_FAILURE"); }
        }
        return true;
    };
    if (_nodeYAML) {
        if (_nodeYAML["option"]) SettingDef::Efficiency::option = _nodeYAML["option"].as< TString >();
        CheckEfficiencyOption(SettingDef::Efficiency::option, SettingDef::AllowedConf::EfficiencyOptions);

        //Used for effiicency submission/file generation
        if (_nodeYAML["ver"]) SettingDef::Efficiency::ver = _nodeYAML["ver"].as< TString >();        
        //TODO : remove this 
        /*
         if (_nodeYAML["weightOptionSig"]) SettingDef::Efficiency::weightOptionSig = _nodeYAML["weightOptionSig"].as< TString >();
         if (_nodeYAML["weightOptionBkg"]) SettingDef::Efficiency::weightOptionBkg = _nodeYAML["weightOptionBkg"].as< TString >();
        */
        // if (_nodeYAML["ver"]) ObsoleteFlag("Efficiency::ver");
        if (_nodeYAML["weightOptionSig"]) ObsoleteFlag("Efficiency::weightOptionSig");
        if (_nodeYAML["weightOptionBkg"]) ObsoleteFlag("Efficiency::weightOptionBkg");


        if (_nodeYAML["flatnessVer"]) SettingDef::Efficiency::flatnessVer = _nodeYAML["flatnessVer"].as< TString >();
        
        if (_nodeYAML["fitconfiguration"]){            
            //Must pass in also the Fit yaml to figure out if the "dRatio,sRatio options are in to fill up or not the SigConstr slots"
            SettingDef::Efficiency::fitconfiguration = EfficiencyForFitHandler( _nodeYAML["fitconfiguration"] );
        }
        if (_nodeYAML["scaleEfficiency"]) SettingDef::Efficiency::scaleEfficiency = _nodeYAML["scaleEfficiency"].as< double >();
        if (_nodeYAML["scaleSystematics"]) SettingDef::Efficiency::scaleSystematics = _nodeYAML["scaleSystematics"].as< double >();

    }
    return;
}

void ParserSvc::ParseFitYAML(YAML::Node _nodeYAML) {
    if (m_debug) {
        MessageSvc::Debug("ParseFitYAML", (TString) "Nodes =", to_string(_nodeYAML.size()));
        cout << YELLOW;
        cout << _nodeYAML << endl;
        MessageSvc::Line();
        cout << RESET;
    }
    if (_nodeYAML) {
        if (_nodeYAML["option"]) SettingDef::Fit::option = _nodeYAML["option"].as< TString >();
        if (_nodeYAML["ver"]) {
            if (SettingDef::Fit::ver == "")
                SettingDef::Fit::ver = _nodeYAML["ver"].as< TString >();
            else
                SettingDef::Fit::ver = SettingDef::Fit::ver + SettingDef::separator + _nodeYAML["ver"].as< TString >();
        }
        if (_nodeYAML["scan1DParameter"]) SettingDef::Fit::scan1DParameter = _nodeYAML["scan1DParameter"].as< TString >();
        if (_nodeYAML["startLLScanFromMin"]) SettingDef::Fit::startLLScanFromMin = _nodeYAML["startLLScanFromMin"].as< bool >();
        if (_nodeYAML["nScanPointsProfile"]) SettingDef::Fit::nScanPointsProfile = _nodeYAML["nScanPointsProfile"].as< int >();
        if (_nodeYAML["scanProfileManual"]) SettingDef::Fit::scanProfileManual   = _nodeYAML["scanProfileManual"].as< bool >();
        if (_nodeYAML["minValScan"]) SettingDef::Fit::minValScan = _nodeYAML["minValScan"].as< double >();
        if (_nodeYAML["maxValScan"]) SettingDef::Fit::maxValScan = _nodeYAML["maxValScan"].as< double >();
        if (_nodeYAML["binned"]) SettingDef::Fit::doBinned = _nodeYAML["binned"].as< bool >();
        if (_nodeYAML["nBins"]) SettingDef::Fit::nBins = _nodeYAML["nBins"].as< int >();
        if (_nodeYAML["splitL0Categories"]) SettingDef::Fit::splitL0Categories = _nodeYAML["splitL0Categories"].as< bool >();
        if (_nodeYAML["splitRunPeriods"])   SettingDef::Fit::splitRunPeriods   = _nodeYAML["splitRunPeriods"].as< bool >();
        if (_nodeYAML["splitTrackCategories"]) SettingDef::Fit::splitTrackCategories = _nodeYAML["splitTrackCategories"].as< bool >();
        if (_nodeYAML["plotSumCategories"]) SettingDef::Fit::plotSumCategories = _nodeYAML["plotSumCategories"].as< bool >();
        if (_nodeYAML["blindYield"]) SettingDef::Fit::blindYield = _nodeYAML["blindYield"].as< bool >();
        if (_nodeYAML["blindEfficiency"]) SettingDef::Fit::blindEfficiency = _nodeYAML["blindEfficiency"].as< bool >();
        if (_nodeYAML["blindRatio"]) SettingDef::Fit::blindRatio = _nodeYAML["blindRatio"].as< bool >();
        if (_nodeYAML["reduceRooKeysPDF"]) SettingDef::Fit::reduceRooKeysPDF = _nodeYAML["reduceRooKeysPDF"].as< bool >();
        if (_nodeYAML["useDatasetCache"]) SettingDef::Fit::useDatasetCache = _nodeYAML["useDatasetCache"].as< bool >();
        if (_nodeYAML["redoDatasetCache"]) SettingDef::Fit::redoDatasetCache = _nodeYAML["redoDatasetCache"].as< bool >();
        if (_nodeYAML["useRatioComb"])               SettingDef::Fit::useRatioComb = _nodeYAML["useRatioComb"].as< bool >();
        if (_nodeYAML["useNumericalExpTurnOn"])      SettingDef::Fit::useNumericalExpTurnOn = _nodeYAML["useNumericalExpTurnOn"].as< bool >();
        if (_nodeYAML["nCPUDataFit"])                SettingDef::Fit::nCPUDataFit = _nodeYAML["nCPUDataFit"].as< int >();
        if (_nodeYAML["CorrelateConstraintsNoNorm"]) SettingDef::Fit::CorrelateConstraintsNoNorm = _nodeYAML["CorrelateConstraintsNoNorm"].as< bool >();

        if (_nodeYAML["LocalCaches"]) SettingDef::Fit::LocalCaches = _nodeYAML["LocalCaches"].as<bool>();
        if (_nodeYAML["loadFitComponentCaches"]) SettingDef::Fit::loadFitComponentCaches = _nodeYAML["loadFitComponentCaches"].as< bool >();
        if (_nodeYAML["saveFitComponentCaches"]) SettingDef::Fit::saveFitComponentCaches = _nodeYAML["saveFitComponentCaches"].as< bool >();
        if (_nodeYAML["redoFitComponentCaches"]) SettingDef::Fit::redoFitComponentCaches = _nodeYAML["redoFitComponentCaches"].as< bool >();
        if (_nodeYAML["rareOnly"]) SettingDef::Fit::rareOnly = _nodeYAML["rareOnly"].as< bool >();
        if (_nodeYAML["LPTMCandidates"]) SettingDef::Fit::LPTMCandidates = _nodeYAML["LPTMCandidates"].as< bool >();
        if (_nodeYAML["useBremFracCache"]) SettingDef::Fit::useBremFracCache = _nodeYAML["useBremFracCache"].as< bool >();
        if (_nodeYAML["redoBremFracCache"]) SettingDef::Fit::redoBremFracCache = _nodeYAML["redoBremFracCache"].as< bool >();
        if (_nodeYAML["useRecursiveFractions"])  SettingDef::Fit::useRecursiveFractions = _nodeYAML["useRecursiveFractions"].as< bool >();
        if (_nodeYAML["useRooRealSumPDF"])  SettingDef::Fit::useRooRealSumPDF = _nodeYAML["useRooRealSumPDF"].as< bool >();
        if (_nodeYAML["useMinuit2"]) SettingDef::Fit::useMinuit2 = _nodeYAML["useMinuit2"].as< bool >();
        if (_nodeYAML["RatioParsMinos"]) SettingDef::Fit::RatioParsMinos = _nodeYAML["RatioParsMinos"].as< bool >();
        if (_nodeYAML["initialParamFile"]){
            MessageSvc::Warning("Trying to parse ParamFile as list, if it fails, please convert to a list in your yaml, parsing it as a simple string has been DEPRECATED");
            //if as<vector<TString>> fails it means there is a BadFile error form YamlCPP, the text above should be enough to fix it.
            SettingDef::Fit::initialParamFile.clear();
            for( auto & el : _nodeYAML["initialParamFile"].as< vector<TString> >() ){
                TString _element = el;
                if(_element.Contains("$ANASYS")  || el.Contains("$WSPACESYS") ||  el.Contains("$REPOSYS")){
                    _element = _element.ReplaceAll( "$ANASYS", TString(getenv("ANASYS")));
                    _element = _element.ReplaceAll( "$WSPACESYS", TString(getenv("WSPACESYS")));
                    _element = _element.ReplaceAll( "$REPOSYS", TString(getenv("REPOSYS")));
                }
                SettingDef::Fit::initialParamFile.push_back( _element);
            }
        }
        if (_nodeYAML["dumpParamFile"]){ 
            SettingDef::Fit::dumpParamFile = _nodeYAML["dumpParamFile"].as< TString >();            
        }else{
            if( !SettingDef::Fit::dumpParamFile.Contains(SettingDef::IO::outDir)){
                SettingDef::Fit::dumpParamFile = SettingDef::IO::outDir +"/"+SettingDef::Fit::dumpParamFile ;
            }
        }
        if (_nodeYAML["rJPsiFit"]) SettingDef::Fit::rJPsiFit = _nodeYAML["rJPsiFit"].as< bool >();
        if (_nodeYAML["RPsiFit"])  SettingDef::Fit::RPsiFit  = _nodeYAML["RPsiFit"].as< bool >();
        if (_nodeYAML["RXFit"])    SettingDef::Fit::RXFit    = _nodeYAML["RXFit"].as< bool >();
        //we do this because "configuration managers don't contains this"
        if (_nodeYAML["rJPsiFit"] ||_nodeYAML["RPsiFit"] || _nodeYAML["RXFit"] ){
            int nTrue = int(SettingDef::Fit::rJPsiFit) + int(SettingDef::Fit::RPsiFit) + int(SettingDef::Fit::RXFit);
            if( ! (nTrue ==0 || nTrue == 1)) MessageSvc::Error("Cannot configure rJPsiFit, RPsiFit, RXFit to true at the same time, pick one or nothing","","EXIT_FAILURE");
            if( !_nodeYAML["RatioSystFile"]) MessageSvc::Error("You configured those special flags for injection of systematics in the fit, please provide yamsl for the covariance!","","EXIT_FAILURE");
            SettingDef::Fit::RatioSystFile.clear();
            for( auto & el : _nodeYAML["RatioSystFile"].as< vector<TString> >() ){
                TString _element = el;
                if(_element.Contains("$ANASYS")  || el.Contains("$WSPACESYS") ||  el.Contains("$REPOSYS")){
                    _element = _element.ReplaceAll( "$ANASYS", TString(getenv("ANASYS")));
                    _element = _element.ReplaceAll( "$WSPACESYS", TString(getenv("WSPACESYS")));
                    _element = _element.ReplaceAll( "$REPOSYS", TString(getenv("REPOSYS")));
                }
                SettingDef::Fit::RatioSystFile.push_back( _element);
            }
        }

        if (_nodeYAML["splot2"]) SettingDef::Fit::useSPlot2 = _nodeYAML["splot2"].as< bool >();
        
        if (_nodeYAML["IndexBootTemplateMisID"]) SettingDef::Fit::IndexBootTemplateMisID = _nodeYAML["IndexBootTemplateMisID"].as<int>();

        YAML::Node _configYAML = _nodeYAML["configuration"];
        if (_configYAML && (_configYAML.size() > 0) && (_configYAML.size() <= 3)) SettingDef::Fit::configurations = GetConfigurationsYAML(_configYAML, "Fit");

        YAML::Node _yamlYAML = _nodeYAML["yaml"];
        if (_yamlYAML) SettingDef::Fit::yamls = GetYamlsYAML(_yamlYAML, "Fit");
    }

    return;
}

void ParserSvc::ParseToyYAML(YAML::Node _nodeYAML) {
    if (m_debug) {
        MessageSvc::Debug("ParseToyYAML", (TString) "Nodes =", to_string(_nodeYAML.size()));
        cout << YELLOW;
        cout << _nodeYAML << endl;
        MessageSvc::Line();
        cout << RESET;
    }
    if (_nodeYAML) {
        if (_nodeYAML["option"]) SettingDef::Toy::option = _nodeYAML["option"].as< TString >();
        if (_nodeYAML["tupleVer"]) SettingDef::Toy::tupleVer = _nodeYAML["tupleVer"].as< TString >();
        if (_nodeYAML["studyVer"]) SettingDef::Toy::studyVer = _nodeYAML["studyVer"].as< TString >();
        if (_nodeYAML["nJobs"]) SettingDef::Toy::nJobs = _nodeYAML["nJobs"].as< int >();
        if (_nodeYAML["jobIndex"]) SettingDef::Toy::jobIndex = _nodeYAML["jobIndex"].as< int >();
        if (_nodeYAML["nToysPerJob"]) SettingDef::Toy::nToysPerJob = _nodeYAML["nToysPerJob"].as< int >();
        if (_nodeYAML["constraintOverwriteFile"]){
            SettingDef::Toy::constraintOverwriteFile.clear();
            for( auto & el : _nodeYAML["constraintOverwriteFile"].as< vector<TString> >() ){
                TString _element = el;
                if(_element.Contains("$ANASYS")  || el.Contains("$WSPACESYS") ||  el.Contains("$REPOSYS")){
                _element = _element.ReplaceAll( "$ANASYS", TString(getenv("ANASYS")));
                _element = _element.ReplaceAll( "$WSPACESYS", TString(getenv("WSPACESYS")));
                _element = _element.ReplaceAll( "$REPOSYS", TString(getenv("REPOSYS")));
                }
                SettingDef::Toy::constraintOverwriteFile.push_back( _element);
            }	
        }
        if (_nodeYAML["frozenOverwrite"]) SettingDef::Toy::frozenOverwrite = _nodeYAML["frozenOverwrite"].as<bool>();
        if (_nodeYAML["mergeConfig"]) SettingDef::Toy::mergeConfig = _nodeYAML["mergeConfig"].as< bool >();
        if (_nodeYAML["Silent"]) SettingDef::Toy::Silent = _nodeYAML["Silent"].as< bool >();
        if (_nodeYAML["CopyLocally"]) SettingDef::Toy::CopyLocally = _nodeYAML["CopyLocally"].as< bool >();

        if (_nodeYAML["ReadFractionToysComponents"]) SettingDef::Toy::ReadFractionToysComponents = _nodeYAML["ReadFractionToysComponents"].as<TString>();
        if(  SettingDef::Toy::ReadFractionToysComponents != ""){
            MessageSvc::Warning("Reading the cut removal fraction from yaml",ExpandEnvironment(SettingDef::Toy::ReadFractionToysComponents));
            auto subParser = YAML::LoadFile( ExpandEnvironment(ExpandEnvironment(SettingDef::Toy::ReadFractionToysComponents)).Data());
            if (subParser.IsNull()) MessageSvc::Error("InitYAML", (TString)"Invalid", "EXIT_FAILURE");
            MessageSvc::Warning("Looping");
            for( YAML::iterator _it1 = subParser.begin(); _it1!= subParser.end(); ++_it1){
                TString _Key_ = _it1->first.as<TString>();
                SettingDef::Toy::ReductionFactor[_Key_];
                YAML::Node _subYAML = _it1->second;
                MessageSvc::Info("Read", _Key_);
                for( YAML::iterator _it2 = _subYAML.begin(); _it2!= _subYAML.end(); ++_it2){
                    TString _Component_ = _it2->first.as<TString>();
                    double Fraction     = _it2->second.as<double>();
                    MessageSvc::Info("SubComp", _Component_);
                    std::cout<< "-> Frac (add/remove) " << Fraction << std::endl;
                    if( Fraction<0) MessageSvc::Error("Invalid Negative Fraction passed, fix yaml","","EXIT_FAILURE");
                    SettingDef::Toy::ReductionFactor[_Key_][_Component_] = Fraction;
                }
            }
            for( auto & el : SettingDef::Toy::ReductionFactor ){
                std::cout<< "---- "<< el.first << " -----" <<std::endl;
                for( auto & el1 : el.second ){
                    std::cout<< "+)"<<el1.first << " : " << el1.second<<std::endl;
                }
            }
        }
        if (_nodeYAML["configurationOverrideFile"]) SettingDef::Toy::configurationOverrideFile = _nodeYAML["configurationOverrideFile"].as<TString>();
        YAML::Node _configYAML = _nodeYAML["configuration"];
        if (_configYAML && (_configYAML.size() > 0) && (_configYAML.size() <= 2)) SettingDef::Toy::configurations = GetConfigurationsYAML(_configYAML, "Toy");

        YAML::Node _yamlYAML = _nodeYAML["yaml"];
        if (_yamlYAML) SettingDef::Toy::yamls = GetYamlsYAML(_yamlYAML, "Toy");
    }
    if (SettingDef::Toy::tupleVer == "") SettingDef::Toy::tupleVer = SettingDef::Fit::ver;
    if (SettingDef::Toy::studyVer == "") SettingDef::Toy::studyVer = SettingDef::Fit::ver;
    return;
}

EventType ParserSvc::LoadFromYAML(TString _fileYAML) {
    MessageSvc::Info(Color::Cyan, "LoadFromYAML", (TString) "from", _fileYAML);
    m_fileYAML.clear();
    m_fileYAML.push_back(_fileYAML);
    SettingDef::IO::yaml = ((TString) m_fileYAML[0]).Remove(0, m_fileYAML[0].Last('/') + 1).ReplaceAll("config-", "").ReplaceAll("config", "").ReplaceAll(".yaml", "");
    if (m_fileYAML[0] != "") InitYAML();

    EventType _et(hash_project(SettingDef::Config::project), hash_analysis(SettingDef::Config::ana), SettingDef::Config::sample, hash_q2bin(SettingDef::Config::q2bin), hash_year(SettingDef::Config::year), hash_polarity(SettingDef::Config::polarity), hash_trigger(SettingDef::Config::trigger), hash_brem(SettingDef::Config::brem), hash_track(SettingDef::Config::track), SettingDef::Cut::option, SettingDef::Weight::option, SettingDef::Tuple::option);
    return _et;
}

EventType ParserSvc::GetEventTypeYAML(YAML::Node _nodeYAML) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "GetEventTypeYAML", (TString) "Parsing", to_string(_nodeYAML.size()), "key(s)");

    // Cache values that can be overwritten in the single GetEventType node. Restore defaults once EventType is created
    TString _gngVer = SettingDef::Tuple::gngVer;
    TString _proVer = SettingDef::Tuple::proVer;
    TString _splVer = SettingDef::Tuple::splVer;

    TString _weightConfig = SettingDef::Weight::config;
    TString _triggerConf  = SettingDef::Config::triggerConf;

    TString _prj      = _nodeYAML["prj"] ? _nodeYAML["prj"].as< TString >() : SettingDef::Config::project;
    TString _sample   = _nodeYAML["sample"].as< TString >();
    TString _ana      = _nodeYAML["ana"] ? _nodeYAML["ana"].as< TString >() : SettingDef::Config::ana;
    TString _q2bin    = _nodeYAML["q2bin"] ? _nodeYAML["q2bin"].as< TString >() : SettingDef::Config::q2bin;
    TString _year     = _nodeYAML["year"] ? _nodeYAML["year"].as< TString >() : SettingDef::Config::year;
    TString _polarity = _nodeYAML["polarity"] ? _nodeYAML["polarity"].as< TString >() : SettingDef::Config::polarity;
    TString _trigger  = _nodeYAML["trigger"] ? _nodeYAML["trigger"].as< TString >() : SettingDef::Config::trigger;

    if (_nodeYAML["triggerConf"]) SettingDef::Config::triggerConf = _nodeYAML["triggerConf"].as< TString >();

    TString _brem  = _nodeYAML["brem"] ? _nodeYAML["brem"].as< TString >() : SettingDef::Config::brem;
    TString _track = _nodeYAML["track"] ? _nodeYAML["track"].as< TString >() : SettingDef::Config::track;

    ConfigHolder _ch = ConfigHolder(
            hash_project(_prj), 
            hash_analysis(_ana), 
            _sample, 
            hash_q2bin(_q2bin), 
            hash_year(_year), 
            hash_polarity(_polarity), 
            hash_trigger(_trigger), 
            hash_triggerconf(SettingDef::Config::triggerConf), 
            hash_brem(_brem), 
            hash_track(_track));

    TString _cutOption;
    if (_nodeYAML["cutOption"])
        _cutOption = _nodeYAML["cutOption"].as< TString >();
    else {
        if (!_ch.IsMC())
            _cutOption = SettingDef::Events::cutOptionCL;
        else {
            if (_ch.IsSignalMC())
                _cutOption = SettingDef::Events::cutOptionSigMC;
            else
                _cutOption = SettingDef::Events::cutOptionBkgMC;
        }
    }

    TString _weightOption = _nodeYAML["weightOption"] ? _nodeYAML["weightOption"].as< TString >() : SettingDef::Events::weightOption;
    if (_nodeYAML["weightConf"]) SettingDef::Weight::config = _nodeYAML["weightConf"].as< TString >();

    TString _tupleOption = _nodeYAML["tupleOption"] ? _nodeYAML["tupleOption"].as< TString >() : SettingDef::Events::tupleOption;

    // Overload default values for gngVer, proVer, splVer, if defined in the node.
    if (_nodeYAML["gngVer"]) SettingDef::Tuple::gngVer = _nodeYAML["gngVer"].as< TString >();
    if (_nodeYAML["proVer"]) SettingDef::Tuple::proVer = _nodeYAML["proVer"].as< TString >();
    if (_nodeYAML["splVer"]) SettingDef::Tuple::splVer = _nodeYAML["splVer"].as< TString >();

    EventType _et(_prj, _ana, _sample, _q2bin, _year, _polarity, _trigger, _brem, _cutOption, _weightOption, _tupleOption, false);

    _et.Init(true, false);
    //_et.Print();

    // Reset SettingDef which are configured locally
    SettingDef::Config::triggerConf = _triggerConf;
    SettingDef::Weight::config      = _weightConfig;
    SettingDef::Tuple::gngVer       = _gngVer;
    SettingDef::Tuple::proVer       = _proVer;
    SettingDef::Tuple::splVer       = _splVer;
    return _et;
}

vector< FitConfiguration > ParserSvc::GetConfigurationsYAML(YAML::Node _nodeYAML, TString _type) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "GetConfigurationsYAML", (TString) _type, "Configuration(s) to parse", to_string(_nodeYAML.size()));

    vector< FitConfiguration > _configurations;
    for (YAML::iterator _it = _nodeYAML.begin(); _it != _nodeYAML.end(); ++_it) {
        Analysis _ana = hash_analysis(_it->first.as< TString >());
        if ((SettingDef::Config::ana != "") && (SettingDef::Config::ana != to_string(_ana))) {
            MessageSvc::Warning("GetConfigurationsYAML", (TString) _type, "Configuration parsing for", to_string(_ana), "SKIPPED");
            continue;
        }

        YAML::Node _configYAML = _it->second;

        try {
            MessageSvc::Line();
            MessageSvc::Info("GetConfigurationsYAML", (TString) _type, "Configuration parsing", to_string(_configYAML.size()), "key(s) for", to_string(_ana));

            TString _varName = _configYAML["varName"] ? _configYAML["varName"].as< TString >() : "";

            bool _massConstrained = _configYAML["massConstrained"] ? _configYAML["massConstrained"].as< bool >() : false;

            bool   _binnedMC;
            int    _nBinsMC;
            double _minMC, _maxMC;
            if (_configYAML["binAndRangeMC"]) {                                                      //< when the node is found
                _binnedMC = (bool) (int) _configYAML["binAndRangeMC"].as< vector< double > >()[0];   //<ugly double -> int -> bool conversion
                _nBinsMC  = _configYAML["binAndRangeMC"].as< vector< double > >()[1];
                _minMC    = _configYAML["binAndRangeMC"].as< vector< double > >()[2];
                _maxMC    = _configYAML["binAndRangeMC"].as< vector< double > >()[3];
            } else {
                MessageSvc::Info("BinAndRangeMC", (TString) "Using Defaults from SettingDef::Fit for MC fit, Define your owns [0/1 = Unbinned/Binned Fit, nBins, min, max] for MC");
                _binnedMC = SettingDef::Fit::doBinned;
                _nBinsMC  = SettingDef::Fit::nBins;
                _minMC    = -1;   //< dummy values, will never be used, no scheme MC will be ever generated
                _maxMC    = -1;   //< dummy values, will never be used, no scheme MC will be ever generated
            }

            /*TODO :  New feature, custom named ranges to use for each "sample" bookkeped */
            /*
            map<TString, tuple< bool, int, double, double> > _namedRanges;
            if(_configYAML["binAndRanges"]){
                for(YAML::iterator _it = _configYAML["binAndRanges"].begin(); _it != _configYAML["binAndRanges"].end(); ++_it){
                    TString _nameRange = _it->second["name"].as<TString>();
                    bool    _binned = _it->second["binned"].as<bool>();
                    int     _nBins  = _it->second["nbins"].as<int>();
                    double  _min    = _it->second["min"].as<double>();
                    double  _max    = _it->second["max"].as<double>();
                    _namedRanges[ _nameRange] = make_tuple(_binned, _nBins, _min, _max);
                }
                for(auto & _range : _namedRanges){
                   TString _toPrint = TString(fmt::format("Name {0} | nBins {1} | min {2} | max{3} | binnedFit {4}", _range.first,
                                                                                                                         get<1>(_range.second),
                                                                                                                         get<2>(_range.second),
                                                                                                                         get<3>(_range.second),
                                                                                                                         get<0>(_range.second)));
                   MessageSvc::Info("ParsedRanges ", _toPrint, "");
                }
                abort();
            }
            */

            bool   _binnedCL;
            int    _nBinsCL;
            double _minCL, _maxCL;
            if (_configYAML["binAndRangeCL"]) {                                                      //< when the node is found
                _binnedCL = (bool) (int) _configYAML["binAndRangeCL"].as< vector< double > >()[0];   //<ugly double -> int -> bool conversion
                _nBinsCL  = _configYAML["binAndRangeCL"].as< vector< double > >()[1];
                _minCL    = _configYAML["binAndRangeCL"].as< vector< double > >()[2];
                _maxCL    = _configYAML["binAndRangeCL"].as< vector< double > >()[3];
            } else {
                MessageSvc::Info("BinAndRangeCL", (TString) "Using Defaults from SettingDef::Fit for CL fit, Define your owns [0/1 = Unbinned/Binned Fit, nBins, min, max] for CL");
                _binnedCL = SettingDef::Fit::doBinned;
                _nBinsCL  = SettingDef::Fit::nBins;
                _minCL    = -1;
                _maxCL    = -1;
            }

            vector< tuple< TString, double, double > > _extraRanges;
            if (_configYAML["extraRange"]) {
                vector< string > _ranges = _configYAML["extraRange"].as< vector< string > >();
                for (auto _range : _ranges) {
                    TString _rawrange = (TString) _range;
                    _rawrange.ReplaceAll("", "");
                    auto * _strCollection = ((TString) _rawrange).Tokenize("|");
                    if (_strCollection->GetEntries() != 3) { MessageSvc::Error("ExtraRange", (TString) "Wrong format, use [name, min, max]", "EXIT_FAILURE"); }
                    TString _name = TString(((TObjString *) (*_strCollection).At(0))->String());
                    TString _min  = TString(((TObjString *) (*_strCollection).At(1))->String());
                    TString _max  = TString(((TObjString *) (*_strCollection).At(2))->String());

                    _extraRanges.push_back(make_tuple(TString(((TObjString *) (*_strCollection).At(0))->String()), _min.Atof(), _max.Atof()));
                }
            }

            vector< TString > _composition = _configYAML["composition"].as< vector< TString > >();

            map< Brem, vector< TString > > _compositions;

            bool _splitBrem = _configYAML["splitBrem"] ? _configYAML["splitBrem"].as< bool >() : false;
            if (_splitBrem) {
                for (const auto & _brem : SettingDef::AllowedConf::BremCategories) {
                    if (_brem == "") continue;
                    MessageSvc::Info("GetConfigurationsYAML", (TString) _type, "SplitBrem", _brem);
                    vector< TString > _tmp;
                    for (const auto & _comp : _composition) {
                        if (((TString) _comp).Contains(_brem))
                            _tmp.push_back(((TString) _comp).ReplaceAll("-" + to_string(Brem::G0), "").ReplaceAll("-" + to_string(Brem::G1), "").ReplaceAll("-" + to_string(Brem::G2), "").Data());
                        else if (!((TString) _comp).BeginsWith("Signal"))
                            _tmp.push_back(_comp);
                    }
                    _compositions[hash_brem(_brem)] = _tmp;
                }
            } else {
                _compositions[Brem::All] = _composition;
            }

            TString _splitYears = "comb";
            if(  SettingDef::Fit::splitRunPeriods && SettingDef::Config::year == "") _splitYears = "runs";
            if(  SettingDef::Fit::splitRunPeriods && SettingDef::Config::year == "R1") _splitYears = ""; //should return  11, 12 splitted
            if(  SettingDef::Fit::splitRunPeriods && SettingDef::Config::year == "R2") _splitYears = ""; //should return  15,16,17,18 splitted
            if(  SettingDef::Fit::splitRunPeriods && SettingDef::Config::year == "R2p1") _splitYears = ""; //should return  15,16 splitted
            if(  SettingDef::Fit::splitRunPeriods && SettingDef::Config::year == "R2p2") _splitYears = ""; //should return  17,18 splitted
            if( !SettingDef::Fit::splitRunPeriods) _splitYears = "noSplit";
            
            vector< TString > _years    = GetYears(SettingDef::Config::year, _splitYears);

            vector< TString > _triggers = GetTriggers(SettingDef::Config::trigger, SettingDef::Fit::splitL0Categories);

            if ((SettingDef::Config::project != to_string(Prj::RL)) && (SettingDef::Config::project != to_string(Prj::RKS)) && (SettingDef::Efficiency::option != "T&P")) SettingDef::Fit::splitTrackCategories = false;

            vector< TString > _tracks = GetTracks(SettingDef::Config::track, SettingDef::Config::project, SettingDef::Fit::splitTrackCategories);

            MessageSvc::Info("Fitting and chaining configurations for years : ");
            for( auto & el : _years){
                MessageSvc::Info("Year", el);
            }
            MessageSvc::Info("Fitting and chaining configurations for triggers : ");
            for( auto & el : _triggers){
                MessageSvc::Info("Trigger", el);
            }        
            MessageSvc::Info("Fitting and chaining configurations for Tracks : ");
            for( auto & el : _tracks){
                MessageSvc::Info("Track", el);
            }
            //load the labels if known and configured
            auto _LOAD_LABELS_FROM_NODE_ =[]( YAML::Node & _labelNode){
                map< Sample, TString> _labels;
                for (YAML::iterator _itLabel = _labelNode.begin(); _itLabel != _labelNode.end(); ++_itLabel) {
                    TString _ID    = _itLabel->first.as<TString>() ; 
                    TString _Label = _itLabel->second.as<TString>();                    
                    std::cout<<YELLOW << " Read ID = "<< _ID << " --> Label "<< _Label<< RESET<< std::endl;                    
                    Sample _sample = hash_sample(_ID);
                    if( _labels.find(_sample) == _labels.end()){
                        _labels[_sample] = _Label;
                    }else{
                        MessageSvc::Error("Please fix labels settings, replicas present");
                    }
                }
                return _labels;
            };
            TString _originalCutOption = SettingDef::Cut::option;
            for (const auto & _year : _years) {
                if( hash_year(_year) == Year::Run2p1 && SettingDef::Fit::splitRunPeriods){
                    SettingDef::Cut::option += "-noHLT2"; //HACK, TODO : fix R2p1 in 15 for HLT cuts string present in tuples.
                }
                for (const auto & _trigger : _triggers) {
                    for (const auto & _comp : _compositions) {
                        for (const auto & _track : _tracks) {
                            if (_varName == "") {
                                if (_type == "Fit") _configurations.push_back(FitConfiguration(hash_project(SettingDef::Config::project), 
                                                                                                _ana, 
                                                                                                hash_q2bin(SettingDef::Config::q2bin), 
                                                                                                hash_year(_year), 
                                                                                                hash_polarity(SettingDef::Config::polarity), 
                                                                                                hash_trigger(_trigger), 
                                                                                                _comp.first, 
                                                                                                hash_track(_track), 
                                                                                                _massConstrained, 
                                                                                                make_tuple(_binnedMC, _nBinsMC, _minMC, _maxMC), 
                                                                                                make_tuple(_binnedCL, _nBinsCL, _minCL, _maxCL), 
                                                                                                _comp.second));
                                if (_type == "Toy") _configurations.push_back(FitConfiguration(hash_project(SettingDef::Config::project), 
                                                                                _ana, 
                                                                                hash_q2bin(SettingDef::Config::q2bin), 
                                                                                hash_year(_year), 
                                                                                hash_polarity(SettingDef::Config::polarity), 
                                                                                hash_trigger(_trigger), 
                                                                                _comp.first, 
                                                                                hash_track(_track), 
                                                                                _comp.second));
                            } else {
                                _configurations.push_back(FitConfiguration(hash_project(SettingDef::Config::project), 
                                                                            _ana, 
                                                                            hash_q2bin(SettingDef::Config::q2bin), 
                                                                            hash_year(_year), 
                                                                            hash_polarity(SettingDef::Config::polarity), 
                                                                            hash_trigger(_trigger), 
                                                                            _comp.first, hash_track(_track), 
                                                                            _varName, 
                                                                            make_tuple(_binnedMC, _nBinsMC, _minMC, _maxMC), 
                                                                            make_tuple(_binnedCL, _nBinsCL, _minCL, _maxCL), 
                                                                            _comp.second));
                            }
                            if (_extraRanges.size() != 0) _configurations.back().AddExtraRanges(_extraRanges);
                            
                            if(_configYAML["labels"]){
                                YAML::Node _labels = _configYAML["labels"];
                                _configurations.back().SetLabels( _LOAD_LABELS_FROM_NODE_( _labels));
                            }
                            if( _configYAML["colors"]){
                                YAML::Node _colors = _configYAML["colors"];
                                _configurations.back().SetColors( _LOAD_LABELS_FROM_NODE_( _colors));                                
                            }
                        }
                    }
                }
                if( hash_year(_year) == Year::Run2p1 && SettingDef::Fit::splitRunPeriods){
                    SettingDef::Cut::option = _originalCutOption; //HACK, TODO : fix R2p1 in 15 for HLT cuts string present in tuples.
                }                
            }
        } catch (const exception & e) {
            cout << e.what() << endl;
            MessageSvc::Error("GetConfigurationsYAML", (TString) _type, "Configuration parsing", to_string(_ana), "EXIT_FAILURE");
        }
    }

    MessageSvc::Line();
    MessageSvc::Info("GetConfigurationsYAML", (TString) _type, "Configuration(s) parsed", to_string(_configurations.size()));
    MessageSvc::Line();
    return _configurations;
}

vector< FitConfiguration > ParserSvc::GetConfigurationsYAML(map< pair< Prj, Q2Bin >, pair< TString, TString > > _yamls, TString _type) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "GetConfigurationsYAML", (TString) _type, "YAML(s) to parse", to_string(_yamls.size()));

    vector< FitConfiguration > _configurationsTmp = SettingDef::Fit::configurations;

    vector< FitConfiguration > _configurations;
    for (auto _yaml : _yamls) {
        TString _project = to_string(_yaml.first.first);
        TString _q2bin   = to_string(_yaml.first.second);
        if ((SettingDef::Config::project != "") && (SettingDef::Config::project != _project)) {
            MessageSvc::Warning("GetConfigurationsYAML", (TString) _type, "Configuration parsing for", _project, "SKIPPED");
            continue;
        }
        if ((SettingDef::Config::q2bin != "") && (SettingDef::Config::q2bin != _q2bin)) {
            MessageSvc::Warning("GetConfigurationsYAML", (TString) _type, "Configuration parsing for", _q2bin, "SKIPPED");
            continue;
        }
        Init(_yaml.second.first);
        _configurations.insert(_configurations.end(), SettingDef::Fit::configurations.begin(), SettingDef::Fit::configurations.end());

        //        _configurations.push_back(SettingDef::Fit::configurations)
    }
    SettingDef::Fit::configurations = _configurationsTmp;

    MessageSvc::Line();
    MessageSvc::Info("GetConfigurationsYAML", (TString) _type, "Configuration(s) parsed", to_string(_configurations.size()));
    MessageSvc::Line();
    return _configurations;
}

map< pair< Prj, Q2Bin >, pair< TString, TString > > ParserSvc::GetYamlsYAML(YAML::Node _nodeYAML, TString _type) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "GetYamlsYAML", (TString) _type, "YAML(s) to parse", to_string(_nodeYAML.size()));

    map< pair< Prj, Q2Bin >, pair< TString, TString > > _yamls;
    for (YAML::iterator _it1 = _nodeYAML.begin(); _it1 != _nodeYAML.end(); ++_it1) {
        Prj        _project = hash_project((TString) _it1->first.as< TString >());
        YAML::Node _subYAML = _it1->second;
        MessageSvc::Info("GetYamlsYAML", (TString) to_string(_project), "YAML(s) to parse", to_string(_subYAML.size()));
        for (YAML::iterator _it2 = _subYAML.begin(); _it2 != _subYAML.end(); ++_it2) {
            Q2Bin             _q2bin = hash_q2bin((TString) _it2->first.as< TString >());
            vector< TString > _parse = _it2->second.as< vector< TString > >();
            if (_parse.size() != 2) MessageSvc::Error("GetYamlsYAML", (TString) _type, "Invalid number of options to parse", to_string(_parse.size()), "EXIT_FAILURE");
            TString _yaml = _parse[0];
            if (_yaml.Contains("$ANASYS") || _yaml.Contains("$WSPACESYS") || _yaml.Contains("$REPOSYS")) {
                MessageSvc::Warning("Using ANASYS for YAML parsing");
                _yaml.ReplaceAll("$ANASYS", getenv("ANASYS"));
                _yaml.ReplaceAll("$WSPACESYS", getenv("WSPACESYS"));
                _yaml.ReplaceAll("$REPOSYS", getenv("REPOSYS"));
            }
            TString _option = _parse[1];
            MessageSvc::Info("GetYamlsYAML", to_string(_q2bin), _yaml, _option);
            std::cout<<RED<< "Adding yaml["<< to_string(_project)<<","<< to_string(_q2bin) << "]"<<RESET<<std::endl;
            _yamls[make_pair(_project, _q2bin)] = make_pair(_yaml, _option);
        }
    }

    MessageSvc::Line();
    MessageSvc::Info("GetYamlsYAML", (TString) _type, "YAML(s) parsed", to_string(_yamls.size()));
    MessageSvc::Line();
    return _yamls;
}

int ParserSvc::Run(int argc, char ** argv) {
    int _flag;
    if (m_fileYAML.size() == 0)
        _flag = RunCLI11(argc, argv);
    else
        _flag = RunYAML();
    if (SettingDef::debug.Contains("KRN")) SettingDef::debug = "CO-CH-TH-TR-WH-ET";
    if (SettingDef::debug.Contains("TUP")) SettingDef::debug = "TH-TR";
    if (SettingDef::debug.Contains("FIT")) SettingDef::debug = "FC-FH-FM-FP-FT";
    return _flag;
}

int ParserSvc::RunCLI11(int argc, char ** argv) {
    try {
        m_parserCLI11.parse(argc, argv);
    } catch (CLI::ParseError & e) {
        int _flag = m_parserCLI11.exit(e);
        if (m_return) return m_return;
        return _flag;
    }
    PrintSettings();
    return m_return;
}

int ParserSvc::RunYAML() {
    try {
    } catch (YAML::ParserException & e) {
        cerr << e.what() << endl;
        m_return = 1;
    }
    PrintSettings();
    return m_return;
}

TString ParserSvc::GetArgumentYAML(TString _node, TString _key) {
    if (m_parserYAML) {
        // const char* m_node = _node.Data();
        // const char* m_key  = _key.Data();
        YAML::Node _nodeYAML = m_parserYAML[_node];
        return _nodeYAML[_key].as< TString >();
    }
    return TString("");
}

map< TString, pair< TupleHolder, vector< tuple< ConfigHolder, CutHolder, WeightHolder > > > > ParserSvc::GetListOfSamples(TString _yamlFile, Q2Bin _q2Bin, Analysis _ana, bool _Forefficiencies) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "GetListOfSamples", _yamlFile, to_string(_ana), to_string(_q2Bin));
    MessageSvc::Line();
    if (!IOSvc::ExistFile(_yamlFile)) MessageSvc::Error("GetListOfSamples", (TString) "Invalid", _yamlFile, "file", "EXIT_FAILURE");

    map< TString, pair< TupleHolder, vector< tuple< ConfigHolder, CutHolder, WeightHolder > > > > ret_map;
    // object to return is map[SampleName, [ Tuple(to process (only 1)), [list of Cuts to apply to it in the form of Cut+Weight+Config] ];

    // Fully load this Yaml File
    auto parserYaml = YAML::LoadFile(_yamlFile.Data());

    // Inspect only the "Create" node
    YAML::Node _nodeYaml = parserYaml["Create"];
    // Search for global CutOption for [Data, SignalMC [IsSignalMC() ? ], BackgroundMC [ !IsSignalMC()]]
    // Those cutOptions are used for each sample if not "specifically" overloaded inline in the parser
    if (!_nodeYaml) MessageSvc::Error("GetListOfSamples", (TString) "Asked wrong CreateNode");
    if (!_nodeYaml["cutOptionCL"]) MessageSvc::Error("GetListOfSamples", (TString) "Please add cutOptionCL Node", "EXIT_FAILURE");
    if (!_nodeYaml["cutOptionSigMC"]) MessageSvc::Error("GetListOfSamples", (TString) "Please add cutOptionSigMC Node", "EXIT_FAILURE");
    if (!_nodeYaml["cutOptionBkgMC"]) MessageSvc::Error("GetListOfSamples", (TString) "Please add cutOptionBkgMC Node", "EXIT_FAILURE");
    SettingDef::Events::cutOptionCL    = _nodeYaml["cutOptionCL"].as< TString >();
    SettingDef::Events::cutOptionSigMC = _nodeYaml["cutOptionSigMC"].as< TString >();
    SettingDef::Events::cutOptionBkgMC = _nodeYaml["cutOptionBkgMC"].as< TString >();
    // TO BE CHECKED HOW TO USE THIS FOR EFFICIENCIES..... (we may just re-use the same yaml file for fit + add extra cutOption and a specific Weight Option ....) [not used anyway at the moment and we trigger exteernally at global level
    // SettingDef::Events::weightOption   = _nodeYaml["weightOption"]  ? _nodeYaml["weightOption"].as<TString>()   : SettingDef::Weight::option;

    // Figure out if you are requiring the "noQ2" slice
    TString _searchQ2 = to_string(_q2Bin);
    if (_searchQ2 == "") {   // for noQ2
        _searchQ2 = "noQ2";
        // If no Q2, append the CutOption noQ2 to it [do it for all!]
        MessageSvc::Warning("GetListOfSamples", (TString) "Selected q2 Bin = ALL");
        // ADD BY CONSTRUCTION the noQ2 option for the CutOptions....
        SettingDef::Events::cutOptionCL += "-noQ2";
        SettingDef::Events::cutOptionSigMC += "-noQ2";
        SettingDef::Events::cutOptionBkgMC += "-noQ2";
    }

    // Find in this YAML file the sub-node of ANA
    TString _searchANA = to_string(_ana);
    
    // Go to find the q2-node in the yaml ["Create"]["Q2"]["MM/EE"] = - { } list of dictionaries of Samples...
    auto _myq2node = _nodeYaml[_searchQ2.Data()];
    if (!_myq2node) MessageSvc::Error("GetListOfSamples", (TString) "Asked for wrong node q2 [not existing in YAML FILE]", _searchQ2, "EXIT_FAILURE");

    auto _myAnaNode = _myq2node[_searchANA];
    if (!_myAnaNode) MessageSvc::Error("GetListOfSamples", (TString) "Asked for wrong node q2-ANA [not existing in YAML FILE]", _searchQ2 + "-" + _searchANA, "EXIT_FAILURE");

    // Cache the trigger configuration default (unfortunately not in constructors...(must overload it at run time to get it right !!!!!)))
    TString original_config_trg = SettingDef::Config::triggerConf;

    for (YAML::iterator _itSample = _myAnaNode.begin(); _itSample != _myAnaNode.end(); ++_itSample) {
        // It's a list of sample process all "sample" fields in a given q2region-ANA slot of the yaml file
        // for (YAML::iterator it = sensors.begin(); it != sensors.end(); ++it) {
        const YAML::Node & _sampleYaml = *_itSample;
        // get the node of the sample : it's a dictionary of { "sample": XXX, "triggerAndConfs" : [], "brems" : [], {Optional} cutOption : "XXX" [full Overload of the CutOptionCL,SigMC etc..}], extraCut : "XXX" [append ExtraCutSpecific for it]} #this tells the trigger XXX-CONF splits to apply + the split by Brem ["", "0G", "1G", "2G"]?
        TString _sampleName = _sampleYaml["sample"].as< TString >();

        //---- check whether one has already booked this ....
        bool makenew_tupleHolder = true;
        if (ret_map.find(_sampleName) != ret_map.end()) {
            makenew_tupleHolder = false;
            MessageSvc::Warning("GetListOfSamples", _sampleName, "already in map, not reloading the TupleHolder for it");
        }
        //---- Get list of triggers-conf to split over
        vector< string > _triggers = _sampleYaml["triggerAndConfs"].as< vector< string > >();
        //---- Get list of Brem categories to split over
        vector< string > _brems              = _sampleYaml["brems"].as< vector< string > >();
        TString          _dedicatedCutOption    = "ERROR";
        TString          _dedicatedExtraCut     = "";
        //allows you in the Create field to cocktailup different pro[XXX] or cre[XXX] versions , bypassing global flag
        TString          _dedicatedTupleOption  = SettingDef::Tuple::option;

        // For each sample you can use a different cutOption
        if (_sampleYaml["cutOption"]  ){ _dedicatedCutOption   = _sampleYaml["cutOption"].as< TString >(); }
        if (_sampleYaml["tupleOption"]){ _dedicatedTupleOption = _sampleYaml["tupleOption"].as< TString >(); }

        if (_sampleYaml["extraCut"]) { _dedicatedExtraCut = TCut(_sampleYaml["extraCut"].as< TString >()); }

        // If this is parsed for Efficiencies and Data is present in the config file, it skips it.
        if (_Forefficiencies && _sampleName == "LPT") continue;
        // SKIP for efficiencies the real data samples used as input.
        MessageSvc::Info("SampleParsed", _sampleName);
        MessageSvc::Info("TriggerParsed (n) ", to_string(_triggers.size()));
        MessageSvc::Info("BremSplitParsed (n)", to_string(_brems.size()));

        if (_dedicatedCutOption != "ERROR") { MessageSvc::Warning("GetListOfSamples", (TString) "Using Specific CutOption you declared in this Sample [for all splits]", _dedicatedCutOption); }

        // Collect for this sample a vector< ConfigHolder [for checking], CutHolder [ a selection ], WeightHolder [ a weight ] >
        // BaseConfigHolder do not hold the split in TriggerCategory and it doesn't know about TriggerConf...This is used to make a new tuple
        vector< tuple< ConfigHolder, CutHolder, WeightHolder > > _populateIt;
        // Exclusive ALL for the Base ConfigHolder.
        auto _trconforiginal            = SettingDef::Config::triggerConf;
        SettingDef::Config::triggerConf = "exclusive";
        ConfigHolder _baseConfigHolder(
                hash_project(SettingDef::Config::project), 
                hash_analysis(_searchANA), 
                _sampleName, 
                _q2Bin, 
                hash_year(SettingDef::Config::year), 
                hash_polarity(SettingDef::Config::polarity),
                Trigger::All,   // IMPORTANT, DO NOT CHANGE
                hash_triggerconf(SettingDef::Config::triggerConf), 
                Brem::All,
                Track::All);     // IMPORTANT, DO NOT CHANGE

        SettingDef::Config::triggerConf = _trconforiginal;
        // 1 tuple holder to be splitted by trigger/brem categories
        TupleHolder * _tholder = nullptr;
        if (makenew_tupleHolder) {
            try {
                // Some samples do not exists.... we do a try catch for this...
                _tholder = new TupleHolder(_baseConfigHolder, _dedicatedTupleOption);
                _tholder->Init();
            } catch (const exception & e) {
                MessageSvc::Warning("GetListOfSamples", (TString) "FAILED SAMPLE CREATION Not existing....");
                cout << e.what() << endl;
                TString failed_Report = TString("FAILED SAMPLE CREATION Not existing.... Prj : ") + SettingDef::Config::project + "- sample " + _sampleName + ": ANA " + _searchANA + " : q2bin " + to_string(_q2Bin) + " Year-Pol " + SettingDef::Config::year + "-" + SettingDef::Config::polarity;
                SettingDef::Events::fails.push_back(failed_Report);
                continue;
            }
        }
        // Go nested in the splits...
        for (auto & trigger : _triggers) {
            for (auto & brem : _brems) {
                // triggers: ["L0I-exclusive", "L0L-exclusive", "L0L-inclusive"], tokenize the "-", left = The category,, right = "logic".
                // the trigger-triggerConf  / brem are the same strings you can find in EnumeratorSvc.
                TString trg      = ((TString) trigger).ReplaceAll("-", " - ");
                TString trg_cat  = TString(((TObjString *) (*trg.Tokenize("-")).At(0))->String());
                TString trg_conf = TString(((TObjString *) (*trg.Tokenize("-")).At(1))->String());
                trg_cat.ReplaceAll(" ", "");
                trg_conf.ReplaceAll(" ", "");
                TString bremcat = brem;
                // overload system settings to "stream it inside EventType"

                SettingDef::Config::triggerConf = trg_conf;
                ConfigHolder config(
                        hash_project(SettingDef::Config::project), 
                        hash_analysis(_searchANA), 
                        _sampleName, 
                        _q2Bin, 
                        hash_year(SettingDef::Config::year), 
                        hash_polarity(SettingDef::Config::polarity), 
                        hash_trigger(trg_cat), 
                        hash_triggerconf(SettingDef::Config::triggerConf), 
                        hash_brem(bremcat));

                config.Init();
                if (_Forefficiencies) {
                    // FOR EFFICIENCY PARSING !
                    if (!config.IsMC()) {
                        MessageSvc::Warning("GetListOfSamples", (TString) "SKIPPING DATA FOR EFFICIENCY CALCULATION");
                        continue;
                    }
                    // This sample is flagged to be used for Efficiencies.... If it's a signal ("splittable...") all good.
                    if (config.IsMC()) {
                        if (hash_brem(bremcat) != Brem::All) {
                            MessageSvc::Warning("GetListOfSamples", (TString) "Skipping bookkeping for efficiency  due to Brem split, only ALL brem allowed");
                            continue;
                            // skip this slicing...keep only the ones haivng Brem::All, and Trigger::XXX (not ALL)
                            // efficiencies calculated for Brem::All and for each "trigger conf
                        }
                        if (hash_trigger(trg_cat) == Trigger::All) {
                            MessageSvc::Warning("GetListOfSamples", (TString) "Skipping bookkeping for efficiency  due to merged Trigger flag, only Splitted trigger allowed");
                            continue;
                            // skip the slicing where both trigger are merged...
                        }
                    } else if (config.IsMC()) {
                        // thus it's a background sample
                        if (!(hash_trigger(trg_cat) == Trigger::All && hash_brem(bremcat) == Brem::All)) {
                            MessageSvc::Warning("GetListOfSamples", (TString) "Skipping bookkeping for efficiency for Background Sample, only allowed merged trigger and merged brem categories");
                            continue;
                        }
                    }
                }
                // change cutOptionMC / LPT here (must be overloaded here in some way)
                TString _cutOption_toUse    = "ERROR";
                TString _weightOption_toUse = SettingDef::Weight::option;   // use global WeightOption
                // now let's tag stuff
                if (!config.IsMC()) {
                    _cutOption_toUse = SettingDef::Events::cutOptionCL;   // use CutOptionCL
                } else {
                    if (config.IsSignalMC()) {
                        _cutOption_toUse = SettingDef::Events::cutOptionSigMC;
                    } else {
                        _cutOption_toUse = SettingDef::Events::cutOptionBkgMC;
                    }
                }
                if (_dedicatedCutOption != "ERROR") {
                    // If was overloaded with a dedicated One, use IT!
                    MessageSvc::Warning("GetListOfSamples", (TString) "OVERLOADING CUTOPTION with dedicatd one");
                    _cutOption_toUse = _dedicatedCutOption;   //
                }
                auto original_extraCut = SettingDef::Cut::extraCut;
                if (_dedicatedExtraCut != SettingDef::Cut::extraCut) { SettingDef::Cut::extraCut = _dedicatedExtraCut; }
                config.PrintInline();
                MessageSvc::Info("CutOption", _cutOption_toUse);
                MessageSvc::Info("ExtraCut", SettingDef::Cut::extraCut);
                MessageSvc::Info("WeightOption", _weightOption_toUse);

                // Create the CutHolder, WeightHolder and reset the ExtraCut if was overloaded.....
                CutHolder ch(config, _cutOption_toUse);
                ch.Init();
                WeightHolder wh(config, _weightOption_toUse);
                wh.Init();
                // Reset it...
                SettingDef::Cut::extraCut = original_extraCut;
                // for this "sample" this slice of <configholder, cutholder, weightholder> will be used.
                _populateIt.push_back(make_tuple(config, ch, wh));
            }   // end brems loop
        }       // end trigger loop
        if (!makenew_tupleHolder) {
            // If this tuple was already preseent, we just chain to already existing container
            // for this sample TupleHolder already filled, we just have to chain a set of new stuff....
            ret_map[_sampleName].second.insert(ret_map[_sampleName].second.end(), _populateIt.begin(), _populateIt.end());
        } else {
            ret_map[_sampleName] = make_pair(*_tholder, _populateIt);
        }
    }   // end sample loop
    // Reset the "run-time modified triggerConf" !
    SettingDef::Config::triggerConf = original_config_trg;

    sort(SettingDef::Events::fails.begin(), SettingDef::Events::fails.end());
    MessageSvc::Warning("GetListOfSamples", (TString) "Failed to load", to_string(SettingDef::Events::fails.size()), "samples");
    for (auto & _fail : SettingDef::Events::fails) { MessageSvc::Warning("GetListOfSamples", _fail); }

    return ret_map;
}


void ParserSvc::ObsoleteFlag( TString _flag) const {
    MessageSvc::Warning("Obsolete configuration" , _flag );
    MessageSvc::Warning("Get in touch with RX team to cross check new way of dealing this flag");
    MessageSvc::Error("Stopping here", (TString)"code behaviour will not be what you meant to do","EXIT_FAILURE");
    return ;
}

#endif
