#include "SettingDef.hpp"
#include "EfficiencyForFitHandler.hpp"
#include "FitGenerator.hpp"
#include "EfficiencySvc.hpp"
#include "FitterTool.hpp"
#include "ParserSvc.hpp"
#include "itertools.hpp"
#include "ConstDef.hpp"
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds

/**
 * fitGenerator
 * createCovarianceTuple.out --yaml yaml/fitter/NODT2/NOMINAL/config-Fit###.yaml
 */
int main(int argc, char ** argv) {
    auto tStart = chrono::high_resolution_clock::now();
    //================================
    // Parsing Yaml file
    //================================
    ParserSvc parser("");
    parser.Init(argc, argv);
    if (parser.Run(argc, argv) != 0) return 1;

    ROOT::EnableThreadSafety();
    ROOT::DisableImplicitMT(); 
    // if (yaml == "") yaml = "./config_RX_Yields_SplitByRun_SplityByTrigger_Toys_16JuneChecks.yaml";
    // ConfigureFromYAML(yaml);    
    struct MappedElements{ 
        TString FileName;
        TString TreeName;
        TString BranchName;
        pair<double,double> Lumi;
        pair<double,double> Gen;
        pair<double,double> Flt;
        double LumiNormTot =1.;
        MappedElements() = default;        
        MappedElements( const TString & fileName, 
                        const TString & treeName, 
                        const TString & branchName, 
                        const pair<double, double> & lumiValWeight, 
                        const pair<double, double> & epsGen , 
                        const pair<double, double> & epsFiltering ){
            
            FileName = fileName;
            TreeName = treeName;
            BranchName = branchName;
            Lumi = lumiValWeight;
            Gen  = epsGen;
            Flt  = epsFiltering;
            LumiNormTot =1.;
        };
        TString GetExpression( TString aliasID) const{
            TString _lumiWeight = TString::Format( "(%f/%f)", lumi().first, LumiNormTot);
            TString _BranchName = TString::Format("%s.%s", aliasID.Data(), branchName().Data());
            TString _epsGeoFlt  = TString::Format("(%f * %f)", gen().first, flt().first);
    	    //Expression for chaining Vector columns is epsGeo * epsFlt * EfficiencyBranchVector
            TString _expression = TString::Format("%s * %s * %s", _lumiWeight.Data(), _epsGeoFlt.Data(), _BranchName.Data() );
            return _expression;
        };
        void SetNormLumiVal( double totLumi){
            LumiNormTot = totLumi;
        };        
        TString fileName()const{        return FileName;}
        TString treeName()const{        return TreeName;}
        TString branchName()const{      return BranchName;}
        pair<double,double> lumi()const{return Lumi;}
        pair<double,double> gen()const{ return Gen;}
        pair<double,double> flt()const{ return Flt;}
        void Print()const{
            std::cout<<GREEN<<"*) InputFile  : "<< fileName()<<"::"<<treeName()<<RESET<<endl;
            std::cout<<GREEN<<"*) BranchName : "<<branchName()<<RESET<<std::endl;
            std::cout<<YELLOW<<"*) eps(gen)  : "<<100*gen().first    <<" +/- "<< 100*gen().second << " %" RESET<<std::endl;
            std::cout<<YELLOW<<"*) eps(flt)  : "<<100*flt().first    <<" +/- "<< 100*flt().second << " %" RESET<<std::endl;
            std::cout<<YELLOW<<"*) Lumi(w)   : "<<lumi().first <<" +/- "<< lumi().second << " 1/pb ; normed with "<< LumiNormTot <<  RESET<<std::endl;
        };
    };
    auto FillMap = []( const ConfigHolder & _config, const InfoEff & info ){
        map< TString , MappedElements > _TUPLEADD;
        Prj         _prj      = _config.GetProject();
        Analysis    _ana      = _config.GetAna();
        Q2Bin       _q2bin    = _config.GetQ2bin();
        TString     _sample   = _config.GetSample();
        Polarity    _polarity = _config.GetPolarity();
        Year        _year     = _config.GetYear();
        Trigger     _trg      = _config.GetTrigger();
        TriggerConf _trgConf  = _config.GetTriggerConf();
        Track       _trk      = _config.GetTrack();        
        auto wOpt  = info.wOption();
        auto wConf = info.wConfig();
        auto eVer  = info.effVersion();   
        TString _gather= TString::Format("effnorm_%s_%s_w%s", to_string(_trg).Data() , to_string(_trgConf).Data(), wConf.Data());
        vector< TString > _Polarities = GetPolarities( to_string(_polarity));
        vector< TString > _Years      = GetYears( to_string(_year));
        vector< pair< Year, Polarity > > _yearsAndPolarities;
        for (const auto && [yy, pp] : iter::product(_Years, _Polarities)) {
            _yearsAndPolarities.push_back(make_pair<Year,Polarity>(hash_year(yy), hash_polarity(pp)));
        }
        if (_Years.size() != 1)      MessageSvc::Warning("GetCovariance::FillMap", (TString) "Combine", to_string(_Years.size()), "years");
        if (_Polarities.size() != 1) MessageSvc::Warning("GetCovariance::FillMap", (TString) "Combine", to_string(_Polarities.size()), "polarities");
        //TODO : SET AND RESET AT THE END! 
        auto _BASEEVER = SettingDef::Efficiency::ver;
        SettingDef::Efficiency::ver = eVer;
        double _lumiTotWeight = 0.;
        vector< pair< TString, TString> > _remappedAliasTupleBranch; 
        for (auto & _yearAndPolarity : _yearsAndPolarities) {
            auto _yy      = _yearAndPolarity.first;
            auto _pol     = _yearAndPolarity.second;
            auto _dirName = IOSvc::GetTupleDir("eff", to_string(_prj), to_string(_ana), to_string(_q2bin), to_string(_yy), to_string(_trg));
            // auto _effTuple= _dirName+ "/" + to_string(_yy) + to_string(_pol)+"/EffTuple.root";       
            //!!!!!!!!!! NEW NAMING SCHEME FOR LEAKAGE NEEDED ! !!!!!!! 
            auto _effTuple= _dirName+ "/" + to_string(_yy) + to_string(_pol)+"/"+_sample+"_EffTuple.root";
            if( !IOSvc::ExistFile(_effTuple)){
                MessageSvc::Error("GetCovariance (file not exist)", _effTuple, "EXIT_FAILURE");
            }
            TString ID = TString::Format( "%s_%s_%s_%s_%s_%s",_sample.Data(), 
                                                              to_string(_prj).Data(), 
                                                              to_string(_q2bin).Data(), 
                                                              to_string(_ana).Data(), 
                                                              to_string(_yy).Data(), 
                                                              to_string(_pol).Data());
            TString _tupleName =  wOpt;
            _tupleName = _tupleName.ReplaceAll("-","_");     
            if( _TUPLEADD.find( ID) != _TUPLEADD.end() ){ 
                MessageSvc::Error("Adding tuple already in with ID", ID,"EXIT_FAILURE");
            }else{
                MessageSvc::Info("Adding ", ID, "to map");
            }  
            auto _lumi = LoadLuminosity( _prj,  _yy, _pol, false, true);
            _lumi.first *=PDG::Const::SQS.at(_yy);
            _lumi.second*=PDG::Const::SQS.at(_yy);
            _lumiTotWeight+= _lumi.first;
            _TUPLEADD[ID] = MappedElements( 
                _effTuple,
                _tupleName,
                _gather,
                _lumi, /*pair< double, double > LoadLuminosity(const Prj & _project, const Year & _year, const Polarity & _polarity, bool _debug, bool _silent)*/
                GetGeneratorEfficiency(_prj ,  _yy, _pol, _sample, _q2bin), 
                GetFilteringEfficiency(_prj ,  _yy, _pol, _sample)
            );
        }
        for( auto & el : _TUPLEADD){
            el.second.SetNormLumiVal( _lumiTotWeight);
        }
        SettingDef::Efficiency::ver = _BASEEVER;
        return _TUPLEADD;
    };
    /*
        Jpsi_Muon_L0I  (R1)       
        eps_sel = vector<100>  
        eps_flt = double 
        eps_gen = double 
        lumi(11-MD)
        eps_ vector * eps_flt * eps_gen * lumi / sum(Lumi)
    */
    auto join = []( const vector<TString> vectorString , TString _delimiter){
        TString el = vectorString.at(0);
        for( int i =1; i < vectorString.size(); ++i){
            el = TString::Format("%s %s %s", el.Data(), _delimiter.Data(), vectorString.at(i).Data());
        }
        return el;
    };

    //The samples which ends up in the covariance tuples!
    auto CheckValid = [](const ConfigHolder & cH ){
        std::map< tuple< Prj, Q2Bin, Analysis> , vector<TString> >  _AllowedInCovariance{ 
            { { Prj::RK, Q2Bin::JPsi,      Analysis::EE }, { "Bu2KJPsEE"} },
            { { Prj::RK, Q2Bin::Psi,       Analysis::EE }, { "Bu2KPsiEE"} },
            { { Prj::RK, Q2Bin::Low,       Analysis::EE }, { "Bu2KEE"} },
            { { Prj::RK, Q2Bin::Central,   Analysis::EE }, { "Bu2KEE"  , "Bu2KJPsEE"} },//Leakage allowed for covariance! 
            { { Prj::RKst, Q2Bin::JPsi,    Analysis::EE }, { "Bd2KstJPsEE"} },
            { { Prj::RKst, Q2Bin::Psi,     Analysis::EE }, { "Bd2KstPsiEE"} },
            { { Prj::RKst, Q2Bin::Low,     Analysis::EE }, { "Bd2KstEE"} },
            { { Prj::RKst, Q2Bin::Central, Analysis::EE }, { "Bd2KstEE", "Bd2KstJPsEE"} },//Leakage allowed for covariance! 
            { { Prj::RK, Q2Bin::JPsi,      Analysis::MM }, { "Bu2KJPsMM"} },
            { { Prj::RK, Q2Bin::Psi,       Analysis::MM }, { "Bu2KPsiMM"} },
            { { Prj::RK, Q2Bin::Low,       Analysis::MM }, { "Bu2KMM"} },
            { { Prj::RK, Q2Bin::Central,   Analysis::MM }, { "Bu2KMM"} },
            { { Prj::RKst, Q2Bin::JPsi,    Analysis::MM }, { "Bd2KstJPsMM"} },
            { { Prj::RKst, Q2Bin::Psi,     Analysis::MM }, { "Bd2KstPsiMM"} },
            { { Prj::RKst, Q2Bin::Low,     Analysis::MM }, { "Bd2KstMM"} },
            { { Prj::RKst, Q2Bin::Central, Analysis::MM }, { "Bd2KstMM"} }              
            /* 
            TODO : enable it for RPhi 
            { { Prj::RPhi, Q2Bin::JPsi,      Analysis::EE }, { "Bs2PhiJPsEE"} },
            { { Prj::RPhi, Q2Bin::Psi,       Analysis::EE }, { "Bs2PhiPsiEE"} },
            { { Prj::RPhi, Q2Bin::Low,       Analysis::EE }, { "Bs2PhiEE"} },
            { { Prj::RPhi, Q2Bin::Central,   Analysis::EE }, { "Bs2PhiEE"} },
            { { Prj::RPhi, Q2Bin::JPsi,      Analysis::MM }, { "Bs2PhiJPsMM"} },
            { { Prj::RPhi, Q2Bin::Psi,       Analysis::MM }, { "Bs2PhiPsiMM"} },
            { { Prj::RPhi, Q2Bin::Low,       Analysis::MM }, { "Bs2PhiMM"} },
            { { Prj::RPhi, Q2Bin::Central,   Analysis::MM }, { "Bs2PhiMM"} }, 
            */    
        };
        auto this_slot = make_tuple<Prj, Q2Bin, Analysis>( cH.GetProject(), cH.GetQ2bin(), cH.GetAna());
        if( _AllowedInCovariance.find( this_slot) == _AllowedInCovariance.end()){
            cH.PrintInline();
            MessageSvc::Error("Prj,Q2Bin,Ana not allowed in covariance matrix calculation (only signal)", "","EXIT_FAILURE");
        }
        if( ! CheckVectorContains( _AllowedInCovariance.at(this_slot), cH.GetSample())){
            cH.PrintInline();
            MessageSvc::Error("Sample for Covariance matrix not allowed", (TString)"Efficiency must be computed also for this sample (not at the moment)","EXIT_FAILURE");            
        }
        return true;
    };        
    //1 Efficiency is 1 ConfigHolder 
    //This is the logic to use so we need a vector of ConfigHolders 
    map< Prj, TString> _texPrj{
        { Prj::RK , "R_{K}"}, 
        { Prj::RKst, "R_{Kst}"}
    };
    map< Q2Bin, TString> _texQ2Bin{
        { Q2Bin::Psi    , "q^{2}_{#psi(2S)}"}, 
        { Q2Bin::JPsi    , "q^{2}_{J/#psi}"}, 
        { Q2Bin::Low    , "q^{2}_{low}"}, 
        { Q2Bin::Central, "q^{2}_{central}"}
    };
    map< Trigger, TString> _texL0{
        { Trigger::L0I    , "L0I"}, 
        { Trigger::L0L    , "L0L"}, 
    };
    map< Analysis, TString> _texAna{
        { Analysis::EE  , "e"}, 
        { Analysis::MM    , "#mu"}, 
    };
    //Plug in all those fucking guys for the full covariance matrix for Run1 
    
    
    vector< std::tuple< Prj , Q2Bin, Analysis , Trigger, TriggerConf, TString , TString, Year  > >  HoldersEfficiencies{ 
        { Prj::RK,   Q2Bin::JPsi, Analysis::EE, Trigger::L0I, TriggerConf::Exclusive, "Bu2KJPsEE", "sig", Year::Run1 },
        { Prj::RK,   Q2Bin::JPsi, Analysis::EE, Trigger::L0L, TriggerConf::Exclusive, "Bu2KJPsEE", "sig", Year::Run1 },
        { Prj::RK,   Q2Bin::JPsi, Analysis::MM, Trigger::L0I, TriggerConf::Exclusive, "Bu2KJPsMM", "sig", Year::Run1 },
        { Prj::RK,   Q2Bin::JPsi, Analysis::MM, Trigger::L0L, TriggerConf::Exclusive, "Bu2KJPsMM", "sig", Year::Run1 },
        { Prj::RK,   Q2Bin::JPsi, Analysis::EE, Trigger::L0I, TriggerConf::Exclusive, "Bu2KJPsEE", "sig", Year::Run2p1 },
        { Prj::RK,   Q2Bin::JPsi, Analysis::EE, Trigger::L0L, TriggerConf::Exclusive, "Bu2KJPsEE", "sig", Year::Run2p1 },
        { Prj::RK,   Q2Bin::JPsi, Analysis::MM, Trigger::L0I, TriggerConf::Exclusive, "Bu2KJPsMM", "sig", Year::Run2p1 },
        { Prj::RK,   Q2Bin::JPsi, Analysis::MM, Trigger::L0L, TriggerConf::Exclusive, "Bu2KJPsMM", "sig", Year::Run2p1 },
        { Prj::RK,   Q2Bin::JPsi, Analysis::EE, Trigger::L0I, TriggerConf::Exclusive, "Bu2KJPsEE", "sig", Year::Run2p2 },
        { Prj::RK,   Q2Bin::JPsi, Analysis::EE, Trigger::L0L, TriggerConf::Exclusive, "Bu2KJPsEE", "sig", Year::Run2p2 },
        { Prj::RK,   Q2Bin::JPsi, Analysis::MM, Trigger::L0I, TriggerConf::Exclusive, "Bu2KJPsMM", "sig", Year::Run2p2 },
        { Prj::RK,   Q2Bin::JPsi, Analysis::MM, Trigger::L0L, TriggerConf::Exclusive, "Bu2KJPsMM", "sig", Year::Run2p2 },
        
        { Prj::RKst, Q2Bin::JPsi, Analysis::EE, Trigger::L0I, TriggerConf::Exclusive, "Bd2KstJPsEE", "sig", Year::Run1 },
        { Prj::RKst, Q2Bin::JPsi, Analysis::EE, Trigger::L0L, TriggerConf::Exclusive, "Bd2KstJPsEE", "sig", Year::Run1 },
        { Prj::RKst, Q2Bin::JPsi, Analysis::MM, Trigger::L0I, TriggerConf::Exclusive, "Bd2KstJPsMM", "sig", Year::Run1 },
        { Prj::RKst, Q2Bin::JPsi, Analysis::MM, Trigger::L0L, TriggerConf::Exclusive, "Bd2KstJPsMM", "sig", Year::Run1 },
        { Prj::RKst, Q2Bin::JPsi, Analysis::EE, Trigger::L0I, TriggerConf::Exclusive, "Bd2KstJPsEE", "sig", Year::Run2p1 },
        { Prj::RKst, Q2Bin::JPsi, Analysis::EE, Trigger::L0L, TriggerConf::Exclusive, "Bd2KstJPsEE", "sig", Year::Run2p1 },
        { Prj::RKst, Q2Bin::JPsi, Analysis::MM, Trigger::L0I, TriggerConf::Exclusive, "Bd2KstJPsMM", "sig", Year::Run2p1 },
        { Prj::RKst, Q2Bin::JPsi, Analysis::MM, Trigger::L0L, TriggerConf::Exclusive, "Bd2KstJPsMM", "sig", Year::Run2p1 },
        { Prj::RKst, Q2Bin::JPsi, Analysis::EE, Trigger::L0I, TriggerConf::Exclusive, "Bd2KstJPsEE", "sig", Year::Run2p2 },
        { Prj::RKst, Q2Bin::JPsi, Analysis::EE, Trigger::L0L, TriggerConf::Exclusive, "Bd2KstJPsEE", "sig", Year::Run2p2 },
        { Prj::RKst, Q2Bin::JPsi, Analysis::MM, Trigger::L0I, TriggerConf::Exclusive, "Bd2KstJPsMM", "sig", Year::Run2p2 },
        { Prj::RKst, Q2Bin::JPsi, Analysis::MM, Trigger::L0L, TriggerConf::Exclusive, "Bd2KstJPsMM", "sig", Year::Run2p2 },
        
        { Prj::RK,   Q2Bin::Central, Analysis::EE, Trigger::L0I, TriggerConf::Exclusive, "Bu2KEE", "sig", Year::Run1 },
        { Prj::RK,   Q2Bin::Central, Analysis::EE, Trigger::L0L, TriggerConf::Exclusive, "Bu2KEE", "sig", Year::Run1 },
        { Prj::RK,   Q2Bin::Central, Analysis::MM, Trigger::L0I, TriggerConf::Exclusive, "Bu2KMM", "sig", Year::Run1 },
        { Prj::RK,   Q2Bin::Central, Analysis::MM, Trigger::L0L, TriggerConf::Exclusive, "Bu2KMM", "sig", Year::Run1 },
        { Prj::RK,   Q2Bin::Central, Analysis::EE, Trigger::L0I, TriggerConf::Exclusive, "Bu2KEE", "sig", Year::Run2p1 },
        { Prj::RK,   Q2Bin::Central, Analysis::EE, Trigger::L0L, TriggerConf::Exclusive, "Bu2KEE", "sig", Year::Run2p1 },
        { Prj::RK,   Q2Bin::Central, Analysis::MM, Trigger::L0I, TriggerConf::Exclusive, "Bu2KMM", "sig", Year::Run2p1 },
        { Prj::RK,   Q2Bin::Central, Analysis::MM, Trigger::L0L, TriggerConf::Exclusive, "Bu2KMM", "sig", Year::Run2p1 },
        { Prj::RK,   Q2Bin::Central, Analysis::EE, Trigger::L0I, TriggerConf::Exclusive, "Bu2KEE", "sig", Year::Run2p2 },
        { Prj::RK,   Q2Bin::Central, Analysis::EE, Trigger::L0L, TriggerConf::Exclusive, "Bu2KEE", "sig", Year::Run2p2 },
        { Prj::RK,   Q2Bin::Central, Analysis::MM, Trigger::L0I, TriggerConf::Exclusive, "Bu2KMM", "sig", Year::Run2p2 },
        { Prj::RK,   Q2Bin::Central, Analysis::MM, Trigger::L0L, TriggerConf::Exclusive, "Bu2KMM", "sig", Year::Run2p2 },
        
        { Prj::RKst, Q2Bin::Central, Analysis::EE, Trigger::L0I, TriggerConf::Exclusive, "Bd2KstEE", "sig", Year::Run1 },
        { Prj::RKst, Q2Bin::Central, Analysis::EE, Trigger::L0L, TriggerConf::Exclusive, "Bd2KstEE", "sig", Year::Run1 },
        { Prj::RKst, Q2Bin::Central, Analysis::MM, Trigger::L0I, TriggerConf::Exclusive, "Bd2KstMM", "sig", Year::Run1 },
        { Prj::RKst, Q2Bin::Central, Analysis::MM, Trigger::L0L, TriggerConf::Exclusive, "Bd2KstMM", "sig", Year::Run1 },
        { Prj::RKst, Q2Bin::Central, Analysis::EE, Trigger::L0I, TriggerConf::Exclusive, "Bd2KstEE", "sig", Year::Run2p1 },
        { Prj::RKst, Q2Bin::Central, Analysis::EE, Trigger::L0L, TriggerConf::Exclusive, "Bd2KstEE", "sig", Year::Run2p1 },
        { Prj::RKst, Q2Bin::Central, Analysis::MM, Trigger::L0I, TriggerConf::Exclusive, "Bd2KstMM", "sig", Year::Run2p1 },
        { Prj::RKst, Q2Bin::Central, Analysis::MM, Trigger::L0L, TriggerConf::Exclusive, "Bd2KstMM", "sig", Year::Run2p1 },
        { Prj::RKst, Q2Bin::Central, Analysis::EE, Trigger::L0I, TriggerConf::Exclusive, "Bd2KstEE", "sig", Year::Run2p2 },
        { Prj::RKst, Q2Bin::Central, Analysis::EE, Trigger::L0L, TriggerConf::Exclusive, "Bd2KstEE", "sig", Year::Run2p2 },
        { Prj::RKst, Q2Bin::Central, Analysis::MM, Trigger::L0I, TriggerConf::Exclusive, "Bd2KstMM", "sig", Year::Run2p2 },
        { Prj::RKst, Q2Bin::Central, Analysis::MM, Trigger::L0L, TriggerConf::Exclusive, "Bd2KstMM", "sig", Year::Run2p2 },

        { Prj::RK,   Q2Bin::Low, Analysis::EE, Trigger::L0I, TriggerConf::Exclusive, "Bu2KEE", "sig", Year::Run1 },
        { Prj::RK,   Q2Bin::Low, Analysis::EE, Trigger::L0L, TriggerConf::Exclusive, "Bu2KEE", "sig", Year::Run1 },
        { Prj::RK,   Q2Bin::Low, Analysis::MM, Trigger::L0I, TriggerConf::Exclusive, "Bu2KMM", "sig", Year::Run1 },
        { Prj::RK,   Q2Bin::Low, Analysis::MM, Trigger::L0L, TriggerConf::Exclusive, "Bu2KMM", "sig", Year::Run1 },
        { Prj::RK,   Q2Bin::Low, Analysis::EE, Trigger::L0I, TriggerConf::Exclusive, "Bu2KEE", "sig", Year::Run2p1 },
        { Prj::RK,   Q2Bin::Low, Analysis::EE, Trigger::L0L, TriggerConf::Exclusive, "Bu2KEE", "sig", Year::Run2p1 },
        { Prj::RK,   Q2Bin::Low, Analysis::MM, Trigger::L0I, TriggerConf::Exclusive, "Bu2KMM", "sig", Year::Run2p1 },
        { Prj::RK,   Q2Bin::Low, Analysis::MM, Trigger::L0L, TriggerConf::Exclusive, "Bu2KMM", "sig", Year::Run2p1 },
        { Prj::RK,   Q2Bin::Low, Analysis::EE, Trigger::L0I, TriggerConf::Exclusive, "Bu2KEE", "sig", Year::Run2p2 },
        { Prj::RK,   Q2Bin::Low, Analysis::EE, Trigger::L0L, TriggerConf::Exclusive, "Bu2KEE", "sig", Year::Run2p2 },
        { Prj::RK,   Q2Bin::Low, Analysis::MM, Trigger::L0I, TriggerConf::Exclusive, "Bu2KMM", "sig", Year::Run2p2 },
        { Prj::RK,   Q2Bin::Low, Analysis::MM, Trigger::L0L, TriggerConf::Exclusive, "Bu2KMM", "sig", Year::Run2p2 },
        
        { Prj::RKst, Q2Bin::Low, Analysis::EE, Trigger::L0I, TriggerConf::Exclusive, "Bd2KstEE", "sig", Year::Run1 },
        { Prj::RKst, Q2Bin::Low, Analysis::EE, Trigger::L0L, TriggerConf::Exclusive, "Bd2KstEE", "sig", Year::Run1 },
        { Prj::RKst, Q2Bin::Low, Analysis::MM, Trigger::L0I, TriggerConf::Exclusive, "Bd2KstMM", "sig", Year::Run1 },
        { Prj::RKst, Q2Bin::Low, Analysis::MM, Trigger::L0L, TriggerConf::Exclusive, "Bd2KstMM", "sig", Year::Run1 },
        { Prj::RKst, Q2Bin::Low, Analysis::EE, Trigger::L0I, TriggerConf::Exclusive, "Bd2KstEE", "sig", Year::Run2p1 },
        { Prj::RKst, Q2Bin::Low, Analysis::EE, Trigger::L0L, TriggerConf::Exclusive, "Bd2KstEE", "sig", Year::Run2p1 },
        { Prj::RKst, Q2Bin::Low, Analysis::MM, Trigger::L0I, TriggerConf::Exclusive, "Bd2KstMM", "sig", Year::Run2p1 },
        { Prj::RKst, Q2Bin::Low, Analysis::MM, Trigger::L0L, TriggerConf::Exclusive, "Bd2KstMM", "sig", Year::Run2p1 },
        { Prj::RKst, Q2Bin::Low, Analysis::EE, Trigger::L0I, TriggerConf::Exclusive, "Bd2KstEE", "sig", Year::Run2p2 },
        { Prj::RKst, Q2Bin::Low, Analysis::EE, Trigger::L0L, TriggerConf::Exclusive, "Bd2KstEE", "sig", Year::Run2p2 },
        { Prj::RKst, Q2Bin::Low, Analysis::MM, Trigger::L0I, TriggerConf::Exclusive, "Bd2KstMM", "sig", Year::Run2p2 },
        { Prj::RKst, Q2Bin::Low, Analysis::MM, Trigger::L0L, TriggerConf::Exclusive, "Bd2KstMM", "sig", Year::Run2p2 },

        { Prj::RK,   Q2Bin::Psi, Analysis::EE, Trigger::L0I, TriggerConf::Exclusive, "Bu2KPsiEE", "sig", Year::Run1 },
        { Prj::RK,   Q2Bin::Psi, Analysis::EE, Trigger::L0L, TriggerConf::Exclusive, "Bu2KPsiEE", "sig", Year::Run1 },
        { Prj::RK,   Q2Bin::Psi, Analysis::MM, Trigger::L0I, TriggerConf::Exclusive, "Bu2KPsiMM", "sig", Year::Run1 },
        { Prj::RK,   Q2Bin::Psi, Analysis::MM, Trigger::L0L, TriggerConf::Exclusive, "Bu2KPsiMM", "sig", Year::Run1 },
        { Prj::RK,   Q2Bin::Psi, Analysis::EE, Trigger::L0I, TriggerConf::Exclusive, "Bu2KPsiEE", "sig", Year::Run2p1 },
        { Prj::RK,   Q2Bin::Psi, Analysis::EE, Trigger::L0L, TriggerConf::Exclusive, "Bu2KPsiEE", "sig", Year::Run2p1 },
        { Prj::RK,   Q2Bin::Psi, Analysis::MM, Trigger::L0I, TriggerConf::Exclusive, "Bu2KPsiMM", "sig", Year::Run2p1 },
        { Prj::RK,   Q2Bin::Psi, Analysis::MM, Trigger::L0L, TriggerConf::Exclusive, "Bu2KPsiMM", "sig", Year::Run2p1 },
        { Prj::RK,   Q2Bin::Psi, Analysis::EE, Trigger::L0I, TriggerConf::Exclusive, "Bu2KPsiEE", "sig", Year::Run2p2 },
        { Prj::RK,   Q2Bin::Psi, Analysis::EE, Trigger::L0L, TriggerConf::Exclusive, "Bu2KPsiEE", "sig", Year::Run2p2 },
        { Prj::RK,   Q2Bin::Psi, Analysis::MM, Trigger::L0I, TriggerConf::Exclusive, "Bu2KPsiMM", "sig", Year::Run2p2 },
        { Prj::RK,   Q2Bin::Psi, Analysis::MM, Trigger::L0L, TriggerConf::Exclusive, "Bu2KPsiMM", "sig", Year::Run2p2 },
        
        { Prj::RKst, Q2Bin::Psi, Analysis::EE, Trigger::L0I, TriggerConf::Exclusive, "Bd2KstPsiEE", "sig", Year::Run1 },
        { Prj::RKst, Q2Bin::Psi, Analysis::EE, Trigger::L0L, TriggerConf::Exclusive, "Bd2KstPsiEE", "sig", Year::Run1 },
        { Prj::RKst, Q2Bin::Psi, Analysis::MM, Trigger::L0I, TriggerConf::Exclusive, "Bd2KstPsiMM", "sig", Year::Run1 },
        { Prj::RKst, Q2Bin::Psi, Analysis::MM, Trigger::L0L, TriggerConf::Exclusive, "Bd2KstPsiMM", "sig", Year::Run1 },
        { Prj::RKst, Q2Bin::Psi, Analysis::EE, Trigger::L0I, TriggerConf::Exclusive, "Bd2KstPsiEE", "sig", Year::Run2p1 },
        { Prj::RKst, Q2Bin::Psi, Analysis::EE, Trigger::L0L, TriggerConf::Exclusive, "Bd2KstPsiEE", "sig", Year::Run2p1 },
        { Prj::RKst, Q2Bin::Psi, Analysis::MM, Trigger::L0I, TriggerConf::Exclusive, "Bd2KstPsiMM", "sig", Year::Run2p1 },
        { Prj::RKst, Q2Bin::Psi, Analysis::MM, Trigger::L0L, TriggerConf::Exclusive, "Bd2KstPsiMM", "sig", Year::Run2p1 },
        { Prj::RKst, Q2Bin::Psi, Analysis::EE, Trigger::L0I, TriggerConf::Exclusive, "Bd2KstPsiEE", "sig", Year::Run2p2 },
        { Prj::RKst, Q2Bin::Psi, Analysis::EE, Trigger::L0L, TriggerConf::Exclusive, "Bd2KstPsiEE", "sig", Year::Run2p2 },
        { Prj::RKst, Q2Bin::Psi, Analysis::MM, Trigger::L0I, TriggerConf::Exclusive, "Bd2KstPsiMM", "sig", Year::Run2p2 },
        { Prj::RKst, Q2Bin::Psi, Analysis::MM, Trigger::L0L, TriggerConf::Exclusive, "Bd2KstPsiMM", "sig", Year::Run2p2 },

        { Prj::RK,   Q2Bin::Central, Analysis::EE, Trigger::L0I, TriggerConf::Exclusive, "Bu2KJPsEE",   "leakage", Year::Run1 },
        { Prj::RK,   Q2Bin::Central, Analysis::EE, Trigger::L0L, TriggerConf::Exclusive, "Bu2KJPsEE",   "leakage", Year::Run1 },
        { Prj::RKst, Q2Bin::Central, Analysis::EE, Trigger::L0I, TriggerConf::Exclusive, "Bd2KstJPsEE", "leakage", Year::Run1 },
        { Prj::RKst, Q2Bin::Central, Analysis::EE, Trigger::L0L, TriggerConf::Exclusive, "Bd2KstJPsEE", "leakage", Year::Run1 },
        { Prj::RK,   Q2Bin::Central, Analysis::EE, Trigger::L0I, TriggerConf::Exclusive, "Bu2KJPsEE",   "leakage", Year::Run2p1 },
        { Prj::RK,   Q2Bin::Central, Analysis::EE, Trigger::L0L, TriggerConf::Exclusive, "Bu2KJPsEE",   "leakage", Year::Run2p1 },
        { Prj::RKst, Q2Bin::Central, Analysis::EE, Trigger::L0I, TriggerConf::Exclusive, "Bd2KstJPsEE", "leakage", Year::Run2p1 },
        { Prj::RKst, Q2Bin::Central, Analysis::EE, Trigger::L0L, TriggerConf::Exclusive, "Bd2KstJPsEE", "leakage", Year::Run2p1 },
        { Prj::RK,   Q2Bin::Central, Analysis::EE, Trigger::L0I, TriggerConf::Exclusive, "Bu2KJPsEE",   "leakage", Year::Run2p2 },
        { Prj::RK,   Q2Bin::Central, Analysis::EE, Trigger::L0L, TriggerConf::Exclusive, "Bu2KJPsEE",   "leakage", Year::Run2p2 },
        { Prj::RKst, Q2Bin::Central, Analysis::EE, Trigger::L0I, TriggerConf::Exclusive, "Bd2KstJPsEE", "leakage", Year::Run2p2 },
        { Prj::RKst, Q2Bin::Central, Analysis::EE, Trigger::L0L, TriggerConf::Exclusive, "Bd2KstJPsEE", "leakage", Year::Run2p2 },
    };
    
    // Correlation is rho(xy) = Cov(x,y)/ (sigma(X)* sigma(Y))

    vector< TString > _rowsLabels = {} ;
    auto GetConfigHolder =[&](  const std::tuple< Prj , Q2Bin, Analysis , Trigger, TriggerConf, TString , TString, Year  > & el){
        auto _prj      = std::get<0>( el);
        auto _q2bin    = std::get<1>( el);
        auto _ana      = std::get<2>( el);
        auto _trg      = std::get<3>( el);
        auto _trgConf  = std::get<4>( el);
        auto _sample   = std::get<5>( el);
        auto _year     = std::get<7>( el);        
        ConfigHolder myConf(_prj, _ana  , _sample, _q2bin, _year , Polarity::All, _trg, _trgConf, Brem::All, Track::All   );
        return myConf;        
    };
    auto GetLabel = [&]( const std::tuple< Prj , Q2Bin, Analysis , Trigger, TriggerConf, TString , TString, Year  > & el){
        auto _prj      = std::get<0>( el);
        auto _q2bin    = std::get<1>( el);
        auto _ana      = std::get<2>( el);
        auto _trg      = std::get<3>( el);
        auto _trgConf  = std::get<4>( el);
        auto _sample   = std::get<5>( el);
        TString _decModeLabel = std::get<6>( el);
        auto _year     = std::get<7>( el);
        TString _label = TString::Format("%s (%s,%s,%s,%s) #varepsilon_{%s}", to_string(_year).Data() , _texPrj.at(_prj).Data() , _texAna.at( _ana).Data()  , _texQ2Bin.at(_q2bin).Data(), _texL0.at( _trg).Data() , _decModeLabel.Data());
        return _label; 
    };

    MessageSvc::Info("Creating a Covariance Matrix made of ", TString::Format( "%zu x %zu elements", HoldersEfficiencies.size(), HoldersEfficiencies.size()));
    ROOT::DisableImplicitMT(); 
    ROOT::RDataFrame df(100);
    df.Define("bsIDX", "rdfentry_").Snapshot("CovTuple", "CovarianceTuple_tmp.root");
    TFile f("CovarianceTuple_tmp.root", "READ");
    TTree *EfficiencyTree  = f.Get<TTree>("CovTuple") ;
    vector< TString>  _expressionEfficiency = {} ;
    for( int i =0; i < HoldersEfficiencies.size() ; ++i){
        auto _configA = GetConfigHolder( HoldersEfficiencies[i]);
        if( !SettingDef::Efficiency::fitconfiguration.HasEpsSignalSlot( _configA.GetProject(), _configA.GetQ2bin())){
            MessageSvc::Warning("Skipping ConfigHolder slot, missing in the input yaml setups", (TString)to_string(_configA.GetProject()) + " - " + to_string(_configA.GetQ2bin()));
            _expressionEfficiency.push_back("????NOT VALID?????");
            continue;
        }
        auto infoA = SettingDef::Efficiency::fitconfiguration.GetEfficiencyInfo( _configA.GetProject() ,_configA.GetQ2bin(), "RRATIO");

        CheckValid(_configA);
        ROOT::DisableImplicitMT(); 
        TString _IDConfig = _configA.GetKey("addtrgconf-addsample");

        //add all friends from this tuple.
        auto mapA = FillMap(_configA, infoA);
        vector<TString> _expressionAveragingA; 
        for( const auto & el : mapA){
            MessageSvc::Line();
            std::cout<<RED<<"- ID :" << el.first<< std::endl;
            el.second.Print();
            MessageSvc::Line();    
            TString friend_aliasing= TString::Format("%s = %s", el.first.Data(), el.second.treeName().Data());
            MessageSvc::Warning("AddFriend", friend_aliasing, el.second.fileName());
            EfficiencyTree->AddFriend( friend_aliasing, el.second.fileName());
            _expressionAveragingA.push_back( el.second.GetExpression( el.first));
        }
        TString _exprA = join( _expressionAveragingA , " + ");
        _expressionEfficiency.push_back( _exprA);
    }

    ROOT::RDataFrame dfFinal( *EfficiencyTree);
    ROOT::RDF::RNode lastNode( dfFinal);
    vector<std::string> _colsKeep = { "bsIDX"} ;
    for( int i =0; i < HoldersEfficiencies.size() ; ++i){
        auto _configA = GetConfigHolder( HoldersEfficiencies[i]);
        if( !SettingDef::Efficiency::fitconfiguration.HasEpsSignalSlot( _configA.GetProject(), _configA.GetQ2bin())){
            MessageSvc::Line();            
            MessageSvc::Warning("Skipping ConfigHolder slot, missing in the input yaml setups", (TString)to_string(_configA.GetProject()) + " - " + to_string(_configA.GetQ2bin()));
            MessageSvc::Line();
            continue;
        } 
        TString _IDConfig = _configA.GetKey("addtrgconf-addsample").ReplaceAll("-", "_");

        TString _myDefine = TString::Format("eff_%s", _IDConfig.Data());
        TString _expressionAssociated = _expressionEfficiency[i];
        std::cout<< "Define( " << _myDefine.Data() << " , " << _expressionAssociated << " )"<< std::endl;
        lastNode = lastNode.Define( _myDefine.Data(), _expressionAssociated.Data());        
        std::cout<< "---------------------------------------------------------------------" << std::endl;
        _colsKeep.push_back( _myDefine.Data());
    }
    lastNode.Snapshot("CovTuple", "tupleCov_RK_wB0_RKst_wBp.root", _colsKeep);
    return 0;
}
