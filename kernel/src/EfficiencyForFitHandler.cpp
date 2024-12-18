#ifndef EFFICIENCYFORFITHANDLER_CPP
#define EFFICIENCYFORFITHANDLER_CPP
#include "EfficiencyForFitHandler.hpp"
#include "EnumeratorSvc.hpp"
#include "yamlcpp.h"
#include "IOSvc.hpp"
// ClassImp(InfoEff)

TString InfoEff::wOption()const{return m_wOption;}
TString InfoEff::wConfig()const{return m_wConfig;}
TString InfoEff::effVersion()const{return m_effVersion;}
TString InfoEff::effVariable()const{return m_effVariable;}
TString InfoEff::TreeName()const{ return this->wOption().ReplaceAll("-","_"); };
InfoEff::InfoEff( const YAML::Node & inNode ){
    if( ! inNode["wOpt"]) MessageSvc::Error("Must specify wOpt in eff-node", "", "EXIT_FAILURE");
    if( ! inNode["wConf"]) MessageSvc::Error("Must specify wConf in eff-node", "", "EXIT_FAILURE");
    if( ! inNode["eVer"]) MessageSvc::Error("Must specify eVer in eff-node", "", "EXIT_FAILURE");
    if( ! inNode["eVar"]) MessageSvc::Error("Must specify weVarOpt in eff-node", "", "EXIT_FAILURE");
    if( ! inNode["eVar"]) MessageSvc::Error("Must specify weVarOpt in eff-node", "", "EXIT_FAILURE");
    m_wOption = inNode["wOpt"].as<TString>();
    m_wConfig = inNode["wConf"].as<TString>(); 
    m_effVersion = inNode["eVer"].as<TString>();
    m_effVariable = inNode["eVar"].as<TString>(); 
}
InfoEff::InfoEff( const InfoEff & other){
    m_wOption =  other.wOption();
    m_wConfig = other.wConfig();
    m_effVersion = other.effVersion();
    m_effVariable = other.effVariable();
}
InfoEff::InfoEff( const TString & _wOpt, const TString & _wConf, const TString & _eVer, const TString & _eVar){
        m_wOption     = _wOpt;
        m_wConfig     = _wConf; 
        m_effVersion  = _eVer;
        m_effVariable = _eVar;
}
void InfoEff::EmitToYaml(  YAML::Emitter  & _emitter)const{
    _emitter << YAML::Key << "eVer" << effVersion() ;
    _emitter << YAML::Key << "wOpt" << wOption() ;
    _emitter << YAML::Key << "wConf" << wConfig() ;
    _emitter << YAML::Key << "eVar" << effVariable() ;
    return;
}
void InfoEff::Print()const{
    std::cout<<BLUE<<"(wOpt, wConf, eVer, eVar) : " << GREEN<< wOption() << " , "<< wConfig() <<" , "<< effVersion() << " , "<< effVariable() << RESET << std::endl;
    return;
}

void InfoEff::UpdateWeightOption( TString _weightOption){
    MessageSvc::Debug("UpdateWeightOption", m_wOption , _weightOption);
    this->m_wOption = _weightOption;
    return;
}
void InfoEff::UpdateWeightConfig( TString _weightConfig){
    MessageSvc::Debug("UpdateWeightConfig", m_wConfig , _weightConfig);
    this->m_wConfig = _weightConfig;
    return;
}



// ClassImp(EfficiencyForFitHandler)

EfficiencyForFitHandler::EfficiencyForFitHandler(const YAML::Node & _EffForFitConfiguration){ 
    //const YAML::Node & _EffForFitConfiguration){
    if( _EffForFitConfiguration.Type() != YAML::NodeType::Map ){
        MessageSvc::Error("Efficiency::fitconfiguration node must be a map with key = Project ");
    }
    for (YAML::const_iterator _it1 = _EffForFitConfiguration.begin(); _it1 != _EffForFitConfiguration.end(); ++_it1) {
        Prj        _project = hash_project((TString) _it1->first.as< TString >());
        YAML::Node _subQ2YAML = _it1->second;
        for (YAML::const_iterator _it2 = _subQ2YAML.begin(); _it2 != _subQ2YAML.end(); ++_it2) {
            Q2Bin             _q2bin = hash_q2bin((TString) _it2->first.as< TString >());
            MessageSvc::Debug("Adding EffConfiguration for Fit", to_string(_project)+" - "+to_string( _q2bin));
            YAML::Node _subNODE = _it2->second;
            // MessageSvc::Debug("Adding EffConfiguration for Fit (extract BkgConstr)", to_string(_project)+" - "+to_string( _q2bin));
            const auto _BKGCONF  =  _subNODE["BkgConstr"];
            // MessageSvc::Debug("Adding EffConfiguration for Fit (extract SigConstr)", to_string(_project)+" - "+to_string( _q2bin));
            auto _bkgConf = InfoEff( _BKGCONF);
            MessageSvc::Info("Eps loaded for the fit for background ( for rRatios )");
            std::cout<<"bkg-constr"<<std::endl;
            _bkgConf.Print();
            m_bkg_constraints[ make_pair(_project, _q2bin)] = _bkgConf;
            if(  _subNODE["SigConstr"] ){               
                const auto _SIGCONF  =  _subNODE["SigConstr"];
                auto _sigConf = InfoEff( _SIGCONF);
                MessageSvc::Info("Eps loaded for the fit for signal ( for rRatios ), must have BS calculated");
                _sigConf.Print();  
                m_sig_efficiencies[ make_pair(_project, _q2bin)] = _sigConf;                    
            }else{
                MessageSvc::Warning("SigConstr for Ratio not specified, forced to be the same as BkgConf");
                m_sig_efficiencies[ make_pair(_project, _q2bin)] = _bkgConf;
            }
            if( _subNODE["covTuple"]){
                MessageSvc::Warning("External covariance tuple will be used for errors on efficiencies");
                m_covTuple = _subNODE["covTuple"].as<TString>();
                if( !IOSvc::ExistFile(m_covTuple)){
                    MessageSvc::Error("Please provide a valid file for the covariance ntuple", "","EXIT_FAILURE");
                }
            }else{
                MessageSvc::Warning("Computation on the fly expected");
                m_covTuple = "";
            }
        }
    }
}
void EfficiencyForFitHandler::Print() const {
    for( auto & el : m_bkg_constraints  ){
        auto prj = to_string(el.first.first);
        auto q2b = to_string(el.first.second);
        MessageSvc::Info("EfficiencyForFitHandler (BKG)", prj+"-"+q2b );
        m_bkg_constraints.at( el.first).Print();
        MessageSvc::Info("EfficiencyForFitHandler (SIG)", prj+"-"+q2b );
        m_sig_efficiencies.at( el.first).Print();
        MessageSvc::Info("EfficiencyForFitHandler (covTuple)", prj+"-"+q2b, m_covTuple );
    }
}

TString EfficiencyForFitHandler::CovTuple() const{ 
    return m_covTuple;
}

void EfficiencyForFitHandler::UpdateSigEfficiencyWeightConf( TString _weightConfig){
    for( auto & el : m_sig_efficiencies){
        m_sig_efficiencies[el.first].UpdateWeightConfig(_weightConfig );
    }
    return; 
}
void EfficiencyForFitHandler::UpdateSigEfficiencyWeightOption( TString _weightOption){
    for( auto & el : m_sig_efficiencies){
        m_sig_efficiencies[el.first].UpdateWeightOption(_weightOption );
    }
    return; 
}

bool EfficiencyForFitHandler::HasEpsSignalSlot( Prj _prj , Q2Bin _q2bin){
    return m_sig_efficiencies.find( make_pair( _prj, _q2bin)) != m_sig_efficiencies.end();
}

TString EfficiencyForFitHandler::slot( const Prj & _prj, const Q2Bin & _q2bin){
    return to_string(_prj)+"-"+to_string(_q2bin);
}
InfoEff EfficiencyForFitHandler::GetEfficiencyInfo( const Prj & _prj, const Q2Bin & _q2bin, const TString & type){
    auto pp = make_pair( _prj, _q2bin);            
    if( type == "BKGOVERSIGNAL" && m_bkg_constraints.find(pp) == m_bkg_constraints.end()){
        MessageSvc::Error( "GetEfficiencyInfo from Pool Invalid Slot(fix Yamls to have the slot filled)", slot(_prj, _q2bin) ,"EXIT_FAILURE");
    }
    if( type == "RRATIO" && m_sig_efficiencies.find(pp) == m_sig_efficiencies.end()){
        MessageSvc::Error( "GetEfficiencyInfo from Pool Invalid Slot(fix Yamls to have the slot filled)", slot(_prj, _q2bin) ,"EXIT_FAILURE");
    }
    if(type == "BKGOVERSIGNAL"){
        return m_bkg_constraints.at( pp);
    }
    if( type == "RRATIO"){
        return m_sig_efficiencies.at(pp);
    }
    MessageSvc::Error("GetEfficiencyInfo", "Invalid type asked (not an RRATIO,BKGOVERSIGNAL)","EXIT_FAILURE");
    return InfoEff();
}
void EfficiencyForFitHandler::EmitToYaml(  YAML::Emitter  & _emitter) const {
    auto reshuffle = []( const map< pair< Prj, Q2Bin> , InfoEff> & _input){
        map< Prj, map<Q2Bin, InfoEff> > _output;
        for( auto & el : _input){
            _output[el.first.first][el.first.second] = el.second;
        }
        return _output;
    };
    auto _emit_bkg_constr = reshuffle(m_bkg_constraints);
    auto _emit_sig_constr = reshuffle(m_sig_efficiencies);

    // _emitter << YAML::BeginMap;                
    _emitter << YAML::Key << "fitconfiguration"; 
    _emitter << YAML::BeginMap;
    for( const auto & _prj : _emit_bkg_constr){
        _emitter << YAML::Key << to_string(_prj.first) ; 
        _emitter << YAML::BeginMap;
        for( const auto & _q2bin : _prj.second){
            _emitter << YAML::Key << to_string(_q2bin.first) ; 
            _emitter << YAML::BeginMap;
            _emitter << YAML::Key << "BkgConstr" ; 
                _emitter << YAML::BeginMap;
                _q2bin.second.EmitToYaml( _emitter);
                _emitter << YAML::EndMap;                            
                _emitter << YAML::Key << "SigConstr" ; 
               _emitter << YAML::BeginMap;
                _emit_sig_constr.at(_prj.first).at( _q2bin.first).EmitToYaml(_emitter);
            _emitter << YAML::EndMap;     
            _emitter << YAML::Key << "covTuple" << CovTuple(); 
            _emitter << YAML::EndMap;                                            
        }
        _emitter << YAML::EndMap;
    }
    _emitter << YAML::EndMap;        
} 

EfficiencyForFitHandler GetEffContainer( const YAML::Node & _node){ 
    return EfficiencyForFitHandler( _node);
}

EfficiencyForFitHandler GetEffContainer( const TString & yaml_ConfigFile){
    MessageSvc::Info("GetEffContainer", yaml_ConfigFile);
    auto parserYaml = YAML::LoadFile(yaml_ConfigFile.Data());
    bool _nodefound = false;
    if(parserYaml["Efficiency"]){
        if( parserYaml["Efficiency"]["fitconfiguration"]){
            MessageSvc::Info("Node Efficiency::fitconfiguration found");
            _nodefound = true;
        }else{
            MessageSvc::Info("Node Efficiency::fitconfiguration not found");
        }        
    }else{
        MessageSvc::Info("Node Efficiency not found");
    }
    if( _nodefound){
        auto Container = GetEffContainer(parserYaml["Efficiency"]["fitconfiguration"]);
        Container.Print();
        return Container;
    }
    return EfficiencyForFitHandler();
}

#endif // !EFFICIENCYFORFITHANDLER_HPP
