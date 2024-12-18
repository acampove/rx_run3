#ifndef WEIGHTHOLDERRX_CPP
#define WEIGHTHOLDERRX_CPP

#include "WeightHolderRX.hpp"
#include "CutHolder.hpp"

#include "CutDefRX.hpp"
#include "SettingDef.hpp"
#include "WeightDefRX.hpp"

#include "HistogramSvc.hpp"

#include "core.h"
#include "fmt_ostream.h"
#include "vec_extends.h"

#include "TString.h"
#include "yamlcpp.h"
ClassImp(WeightHolderRX)

    WeightHolderRX::WeightHolderRX()
    : m_configHolder() {
    if (SettingDef::debug.Contains("WH")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("ConfigHolder", (TString) "Default");
    m_weightOption = SettingDef::Weight::option;
    m_weightConfig = SettingDef::Weight::config;
    if( m_weightOption.BeginsWith("[") || m_weightOption.EndsWith("]")){
        MessageSvc::Warning("WeightHolderRX, forcing weightConfig from Weight Option using [XXX] begin/end with");
        m_weightConfig = StripStringBetween(m_weightOption, "[","]");
        //Weak check that the weightConfig contains at least one among the allowed ones in SettingDef
        bool valid = false ;
        for( auto & wConfBitsAllow : SettingDef::AllowedConf::WeightConfig){
            if( m_weightConfig.Contains(wConfBitsAllow)){ valid = true; break;}
        }
        if( ! valid){
            MessageSvc::Error("WeightHolderRX with custom wconfig not allowe", m_weightConfig, "EXIT_FAILURE");
        }else{
            MessageSvc::Warning("WeightHolderRX bypassing global settings via weightOption for weightConfig", m_weightConfig);
        }
        //clear the [XXX] from WeightOption
        m_weightOption =  m_weightOption.ReplaceAll(TString::Format("[%s]", m_weightConfig.Data()), "");
    }

}

WeightHolderRX::WeightHolderRX(const ConfigHolder & _configHolder, TString _weightOption)
    : m_configHolder(_configHolder) {
    if (SettingDef::debug.Contains("WH")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("ConfigHolder", (TString) "ConfigHolder");
    m_weightOption = _weightOption;
    m_weightConfig = SettingDef::Weight::config;
    if( m_weightOption.BeginsWith("[") || m_weightOption.EndsWith("]") ) {
        MessageSvc::Warning("WeightHolderRX, forcing weightConfig from Weight Option using [XXX] begin/end with");
        m_weightConfig = StripStringBetween(m_weightOption, "[","]");
        //Weak check that the weightConfig contains at least one among the allowed ones in SettingDef
        bool valid = false ;
        for( auto & wConfBitsAllow : SettingDef::AllowedConf::WeightConfig){
            if( m_weightConfig.Contains(wConfBitsAllow)){ valid = true; break;}
        }
        if( ! valid){
            MessageSvc::Error("WeightHolderRX with custom wconfig not allowe", m_weightConfig, "EXIT_FAILURE");
        }else{
            MessageSvc::Warning("WeightHolderRX bypassing global settings via weightOption for weightConfig", m_weightConfig);
        }
        //clear the [XXX] from WeightOption
        m_weightOption =  m_weightOption.ReplaceAll(TString::Format("[%s]", m_weightConfig.Data()), "");
    }
}

WeightHolderRX::WeightHolderRX(const WeightHolderRX & _weightHolder)
    : m_configHolder(_weightHolder.GetConfigHolder()) {
    if (SettingDef::debug.Contains("WH")) SetDebug(true);
    if (m_debug) MessageSvc::Debug("WeightHolderRX", (TString) "WeightHolderRX");
    m_weightOption = _weightHolder.Option();
    m_weightConfig = _weightHolder.Config();
    m_weight       = _weightHolder.Weight();
    m_weights      = _weightHolder.Weights();
    // the [XXX] logic is not needed here as we have a copy constructor, we just import everything including the wConfig.
}

ostream & operator<<(ostream & os, const WeightHolderRX & _weightHolder) {
    os << WHITE;
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "WeightHolderRX");
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "weightOption", _weightHolder.Option());
    MessageSvc::Print((ostream &) os, "weightConfig", _weightHolder.Config());
    if (IsWeight(_weightHolder.Weight())) MessageSvc::Print((ostream &) os, "weight", _weightHolder.Weight());
    MessageSvc::Print((ostream &) os, "weights", to_string(_weightHolder.Weights().size()));
    // MessageSvc::Line(os);
    os << RESET;
    return os;
}

void WeightHolderRX::Init() {
    MessageSvc::Info(Color::Cyan, "WeightHolderRX", (TString) "Initialize ...");
    if (!m_weightOption.Contains("MCT") && !m_weightOption.Contains("NORM")) {
      if (! (SettingDef::IO::exe.Contains("upleProcess") || !SettingDef::IO::exe.Contains("submitHLT") )){
            if (m_weightOption.Contains(WeightDefRX::ID::BS) && ((SettingDef::Weight::iBS < 0) || (SettingDef::Weight::iBS > WeightDefRX::nBS))) MessageSvc::Error("WeightHolderRX", (TString) "Incompatible Weight options", m_weightOption, "EXIT_FAILURE");
            // if (m_weightOption.Contains(WeightDefRX::ID::PID) && !m_weightOption.Contains(WeightDefRX::ID::TRK) && !m_weightConfig.Contains("meerkat")) MessageSvc::Error("WeightHolderRX", (TString) "Incompatible Weight options", m_weightOption, "EXIT_FAILURE");
            if (m_weightOption.Contains(WeightDefRX::ID::L0) && !m_weightOption.Contains(WeightDefRX::ID::PID) && !m_weightConfig.Contains("meerkat")) MessageSvc::Error("WeightHolderRX", (TString) "Incompatible Weight options", m_weightOption, "EXIT_FAILURE");
            if (m_weightOption.Contains(WeightDefRX::ID::HLT) && !m_weightOption.Contains(WeightDefRX::ID::L0)) MessageSvc::Error("WeightHolderRX", (TString) "Incompatible Weight options", m_weightOption, "EXIT_FAILURE");
            // if (m_weightOption.Contains(WeightDefRX::ID::BKIN) && (!m_weightOption.Contains(WeightDefRX::ID::L0) || !m_weightOption.Contains(WeightDefRX::ID::HLT))) MessageSvc::Error("WeightHolderRX", (TString) "Incompatible Weight options", m_weightOption, "EXIT_FAILURE");
            if (m_weightOption.Contains(WeightDefRX::ID::MULT) && !m_weightOption.Contains(WeightDefRX::ID::BKIN)) MessageSvc::Error("WeightHolderRX", (TString) "Incompatible Weight options", m_weightOption, "EXIT_FAILURE");
            if (!m_weightOption.Contains(WeightDefRX::ID::BDT)) {
                if ((m_weightOption.Contains(WeightDefRX::ID::RECO) && !m_weightOption.Contains(WeightDefRX::ID::MULT)) && !m_weightOption.Contains(WeightDefRX::ID::PTRECO)) MessageSvc::Error("WeightHolderRX", (TString) "Incompatible Weight options", m_weightOption, "EXIT_FAILURE");
            }
        }
    }
    if (m_weightOption.Contains("TRK") && m_configHolder.GetAna() == Analysis::MM) {
        MessageSvc::Warning("Removing -TRK from WeightOption on Muons, once maps become available, remove this line in WeightHolderRX.cpp");
        m_weightOption = m_weightOption.ReplaceAll("-TRK", "").ReplaceAll("TRK-","").ReplaceAll("TRK", "");
    }
    if (m_configHolder.IsMC()){
        CreateWeightMC();
    }else
        CreateWeightCL();
    return;
}

void WeightHolderRX::CreateWeightMC() {
    if (m_debug) MessageSvc::Debug("CreateWeightMC", m_weightOption);

    m_weight = TString(NOWEIGHT);


    //The Pentaquark weight : use -LB
    if (m_weightOption.Contains(WeightDefRX::ID::LB)){ 
        if( m_configHolder.GetSample().Contains("Lb2pKJPs") ||  m_configHolder.GetSample().Contains("Lb2pKPsi")){
            m_weights[WeightDefRX::ID::LB]   = TString(WeightDefRX::LB);
        }
    }

    if (m_weightOption.Contains("SMEARBPN") || m_weightOption.Contains("SMEARB0N") ){ 
        if(m_weightOption.Contains("SMEARBPN")){
            m_weights["Q2"] = "wJPs_M_smear_Bp_fromMCDT_wMC";
        }
        if(m_weightOption.Contains("SMEARB0N")){
            m_weights["Q2"] = "wJPs_M_smear_B0_fromMCDT_wMC";
        }     
    }

    //The LBKinematic weight : use -LB_KIN
    if (m_weightOption.Contains(WeightDefRX::ID::LB_KIN)){ 
        if( m_configHolder.GetSample().Contains("Lb2pKJPs") ){ 
            // ||  m_configHolder.GetSample().Contains("Lb2pKPsi")){ Note: no wRPk.kin weights in
            m_weights[WeightDefRX::ID::LB_KIN] = TString(WeightDefRX::LB_KIN);
        }
    }

    if( m_weightOption.Contains("XJPSWeight")){
        m_weights["XJPSWeight"] = "wDecFile";
        m_weight = "wDecFile";
        return; 
    }

    if (m_weightOption.Contains(WeightDefRX::ID::PTRECO)) {
        m_weights[WeightDefRX::ID::PTRECO] = TString(WeightDefRX::PTRECO);
        //m_weight                           = m_weights[WeightDefRX::ID::PTRECO];
        //m_weights[WeightDefRX::ID::FULL]   = m_weight;
        //return;
    }

    Prj         prj  = m_configHolder.GetProject();
    Analysis    ana  = m_configHolder.GetAna();
    Trigger     trg  = m_configHolder.GetTrigger();
    TriggerConf trgc = m_configHolder.GetTriggerConf();

    if (m_weightOption.Contains(WeightDefRX::ID::BS) && m_configHolder.HasBS() && SettingDef::Weight::useBS && SettingDef::Weight::iBS>=0 && SettingDef::Weight::iBS< WeightDefRX::nBS ) { 
        m_weights[WeightDefRX::ID::BS] = TString(WeightDefRX::BS) + "[" + to_string(SettingDef::Weight::iBS) + "]"; 
    }

    if (m_weightOption.Contains(WeightDefRX::ID::PID)) {
        m_weights[WeightDefRX::ID::PID] = TString(WeightDefRX::PID);
        if( m_weightOption.Contains(WeightDefRX::ID::BS)){
            if( !m_weights[WeightDefRX::ID::PID].Contains("PIDCalib_BS") ){
                m_weights[WeightDefRX::ID::PID].ReplaceAll("PIDCalib", Form("PIDCalib_BS[%i]", SettingDef::Weight::iBS));
            }
        }
    }
    if( ana == Analysis::MM && m_weightOption.Contains(WeightDefRX::ID::ISMUON)){
        m_weights[WeightDefRX::ID::ISMUON] = TString(WeightDefRX::ISMUON); 
    }
    if (m_weightOption.Contains(WeightDefRX::ID::TRK)) { 
        m_weights[WeightDefRX::ID::TRK] = TString(WeightDefRX::TRK); 
        if( m_weightOption.Contains(WeightDefRX::ID::BS)){
            if( !m_weights[WeightDefRX::ID::TRK].Contains("TRKCalib_BS") ){
                m_weights[WeightDefRX::ID::TRK].ReplaceAll("TRKCalib", Form("TRKCalib_BS[%i]", SettingDef::Weight::iBS));
            }
        }
    }

    map< tuple< TriggerConf, Trigger, Analysis >, TString > _l0Weight =  WeightDefRX::wL0 ;
    if( m_weightOption.Contains("COMB") && ana == Analysis::EE){
        MessageSvc::Warning("Using dilepton weight");
        _l0Weight = WeightDefRX::wL0comb ;
    }
    if (m_weightOption.Contains(WeightDefRX::ID::L0)) {
        auto _weightTmp = TString(_l0Weight.at(make_tuple(trgc, trg, ana)));
        if( m_weightOption.Contains("DIST") && ana == Analysis::EE)
            _weightTmp.ReplaceAll("_wL0L_", "_wL0L_dist_");
        m_weights[WeightDefRX::ID::L0] = _weightTmp; 
        if( ana == Analysis::EE && trg == Trigger::L0L && m_weightOption.Contains("BREM")){
            auto _weight = m_weights[WeightDefRX::ID::L0];
            m_weights[WeightDefRX::ID::L0] = _weight.ReplaceAll("E1_wL0L_incl_{B}_eff", "E1_wL0L_incl_{B}_brem_eff" )
                                                    .ReplaceAll("E2_wL0L_incl_{B}_eff", "E2_wL0L_incl_{B}_brem_eff" );
        }
        if( m_weightOption.Contains(WeightDefRX::ID::BS)){
            if( !m_weights[WeightDefRX::ID::L0].Contains("effCL_BS") ){
                m_weights[WeightDefRX::ID::L0].ReplaceAll("effCL", Form("effCL_BS[%i]", SettingDef::Weight::iBS));
            }
            if( !m_weights[WeightDefRX::ID::L0].Contains("effMC_BS")){
                m_weights[WeightDefRX::ID::L0].ReplaceAll("effMC", Form("effMC_BS[%i]",  SettingDef::Weight::iBS));
            }
        }
    }

    map< pair< TriggerConf, Trigger >, TString > _hltWeight = WeightDefRX::wHLT;
    if (m_weightOption.Contains(WeightDefRX::ID::HLT)) { 
        m_weights[WeightDefRX::ID::HLT] = TString(_hltWeight.at(make_pair(trgc, trg))); 
        if( m_weightOption.Contains(WeightDefRX::ID::BS)){
            if( !m_weights[WeightDefRX::ID::HLT].Contains("effCL_BS") ){
                m_weights[WeightDefRX::ID::HLT].ReplaceAll("effCL", Form("effCL_BS[%i]", SettingDef::Weight::iBS));
            }
            if( !m_weights[WeightDefRX::ID::HLT].Contains("effMC_BS")){
                m_weights[WeightDefRX::ID::HLT].ReplaceAll("effMC", Form("effMC_BS[%i]",  SettingDef::Weight::iBS));
            }
        }
    }

    if (m_weightOption.Contains(WeightDefRX::ID::BDT)) {
      if( m_weightOption.Contains("fromLOI")){
	MessageSvc::Warning("WeightHolder from LOI BDT");
	m_weights[WeightDefRX::ID::BDT] = TString(WeightDefRX::BDT_L0I);       
	if( m_weightOption.Contains(WeightDefRX::ID::BS)){
	  if( !m_weights[WeightDefRX::ID::BDT].Contains("BDT_BS_{OPT}_{B}_MM_L0I") ){
	    m_weights[WeightDefRX::ID::BDT].ReplaceAll("BDT_{OPT}_{B}_MM_L0I", Form("BDT_BS_{OPT}_{B}_MM_L0I[%i]", SettingDef::Weight::iBS));
	  }
	}
      }else{
	//Nominal
	m_weights[WeightDefRX::ID::BDT] = TString(WeightDefRX::BDT);
	if( m_weightOption.Contains(WeightDefRX::ID::BS)){
	  if( !m_weights[WeightDefRX::ID::BDT].Contains("BDT_BS_{OPT}_{B}_MM_L0L") ){
		  m_weights[WeightDefRX::ID::BDT].ReplaceAll("BDT_{OPT}_{B}_MM_L0L", Form("BDT_BS_{OPT}_{B}_MM_L0L[%i]", SettingDef::Weight::iBS));
	  }
	}
      }
      if (!m_weightOption.Contains("MCT") && m_weightOption.Contains(WeightDefRX::ID::RECO)){
    	m_weights[WeightDefRX::ID::BDT2] = TString(WeightDefRX::BDT2);
      }
    } else if (m_weightOption.Contains("RW1D")) {
      m_weights["RW1D"] = TString(WeightDefRX::RW1D); 
    } else {
      if (m_weightOption.Contains(WeightDefRX::ID::BKIN)) { 
	m_weights[WeightDefRX::ID::BKIN] = TString(WeightDefRX::BKIN); 
      }
      if (m_weightOption.Contains(WeightDefRX::ID::MULT)) { 
	m_weights[WeightDefRX::ID::MULT] = TString(WeightDefRX::MULT); 
      }
      if (!m_weightOption.Contains("MCT")) {
	if ((m_weightOption.Contains(WeightDefRX::ID::RECO) && !m_weightOption.Contains(WeightDefRX::ID::PTRECO))) { m_weights[WeightDefRX::ID::RECO] = TString(WeightDefRX::RECO); }
      }
    }
    
    //LUMI WEIGHTS SUPPORTED ONLY FOR Run-Periods Weight Holders, and Merged Polarities, 
    //TODO : if really needed to lumi-weight MD, 11+12 samples , so 1/2 of the stat.
    if( m_weightOption.Contains("LUMI")){        
        //rough lumi division among years is hardcoded here 
        auto _PATCHED_GETMCDTENTRIES_ = [&](const Prj & _project, const Year & _year, const Polarity & _polarity, const TString & _sample, bool _force, TString _type, bool _debug = false){    
            //Copy-paste hack lambda function from EfficiencySvc.cpp file, due to incompatible #include and dependendencies we hack this around...
            TString _file = fmt::format("{0}/mct/v{1}/{2}/{3}.yaml", SettingDef::IO::gangaDir, GetBaseVer(SettingDef::Tuple::gngVer), to_string(_project), _sample);
            MessageSvc::Line();
            MessageSvc::Info(Color::Cyan, "_PATCHED_GETMCDTENTRIES_", (TString) fmt::format("{0} - {1} - {2} -- {3}", to_string(_project), to_string(_year), to_string(_polarity), _file));
            YAML::Node _parser = YAML::LoadFile(_file.Data());
            if (_parser.IsNull()) MessageSvc::Error("_PATCHED_GETMCDTENTRIES_", (TString) "Invalid", _file, "parser", "EXIT_FAILURE");
            if (_debug) {
                cout << YELLOW;
                MessageSvc::Line();
                MessageSvc::Debug("_PATCHED_GETMCDTENTRIES_", (TString) "Nodes =", to_string(_parser.size()));
                for (YAML::iterator _it = _parser.begin(); _it != _parser.end(); ++_it) { MessageSvc::Debug("GetMCTEntries", (TString) "SubNodes =", to_string(_it->second.size())); }
                cout << YELLOW;
                cout << _parser << endl;
                MessageSvc::Line();
                cout << RESET;
            }        
            auto _years      = GetYears(to_string(_year));
            auto _polarities = GetPolarities(to_string(_polarity));

            map< pair< Year, Polarity >, pair< double, double > > _collected;

            double _entries = 0.;
            for (auto & _yy : _years) {
                for (auto & _pp : _polarities) {
                    if (_debug) MessageSvc::Debug("GetMCTEntries", (TString) "Parsing year =", _yy, ", polarity = ", _pp);
                    YAML::Node _yearNode = _parser[_yy.Data()][_pp.Data()];
                    if (!_yearNode) continue;
                    const int _nTotal = _yearNode[_type].as< int >();
                    if (_debug) cout << YELLOW << "GetMCTEntries Year = " << _yy << " Polarity = " << _pp << "   Value = " << _nTotal << RESET << endl;
                    if (_nTotal == -1) {
                        if (_debug) MessageSvc::Debug("GetMCTEntries", (TString) "SKIPPING");
                        continue;
                    }
                    _collected[make_pair(hash_year(_yy), hash_polarity(_pp))] = make_pair(0., 0.);
                    _entries += _nTotal;
                }
            }
            if ((_collected.size() != _years.size() * _polarities.size()) && !_force) { 
                MessageSvc::Error("GetMCTEntries", (TString) "Issue in collecting MCT entries", "EXIT_FAILURE"); 
            }
            MessageSvc::Info("GetMCTEntries", (TString) fmt::format("MCT entries = {0}", _entries));
            MessageSvc::Line();
            return _entries;
        };

        map< pair< Year, Polarity > , double  > _lumis{ 
            { { Year::Y2011, Polarity::MD } , 563.045},
            { { Year::Y2011, Polarity::MU } , 425.045},
            { { Year::Y2012, Polarity::MD } , 992.388},
            { { Year::Y2012, Polarity::MU } , 1000.577},
            { { Year::Y2015, Polarity::MD } , 162.629},
            { { Year::Y2015, Polarity::MU } , 122.689},
            { { Year::Y2016, Polarity::MD } , 849.626},
            { { Year::Y2016, Polarity::MU } , 795.372},
            { { Year::Y2017, Polarity::MD } , 849.626},
            { { Year::Y2017, Polarity::MU } , 795.372},
            { { Year::Y2018, Polarity::MD } , 1095.0},
            { { Year::Y2018, Polarity::MU } , 1095.0},
        };
        if(m_configHolder.GetPolarity() != Polarity::All){ 
            MessageSvc::Warning("LUMI weight for polarity MD or MU for run merged not implemented, implement it!!!");
        }else{
            switch(  m_configHolder.GetYear() ){
                case Year::Run1:{
                    double _mcdtsum = (double)_PATCHED_GETMCDTENTRIES_( m_configHolder.GetProject(),m_configHolder.GetYear(), Polarity::All, m_configHolder.GetSample(), false,  "ngng_evt");
                    //const Prj & _project, const Year & _year, const Polarity & _polarity, const TString & _sample, bool _force, TString _type, bool _debug
                    map< pair< Year, Polarity > , double  > _mcdtsingleFactor{
                        { { Year::Y2011, Polarity::MD } , _mcdtsum/(double)_PATCHED_GETMCDTENTRIES_( m_configHolder.GetProject(), Year::Y2011,Polarity::MD, m_configHolder.GetSample(), false,  "ngng_evt")   },
                        { { Year::Y2011, Polarity::MU } , _mcdtsum/(double)_PATCHED_GETMCDTENTRIES_( m_configHolder.GetProject(), Year::Y2011,Polarity::MU, m_configHolder.GetSample(), false,  "ngng_evt")   },
                        { { Year::Y2012, Polarity::MD } , _mcdtsum/(double)_PATCHED_GETMCDTENTRIES_( m_configHolder.GetProject(), Year::Y2012,Polarity::MD, m_configHolder.GetSample(), false,  "ngng_evt")   },
                        { { Year::Y2012, Polarity::MU } , _mcdtsum/(double)_PATCHED_GETMCDTENTRIES_( m_configHolder.GetProject(), Year::Y2012,Polarity::MU, m_configHolder.GetSample(), false,  "ngng_evt")   }
                    };
                    for( auto & el : _mcdtsingleFactor){
                        MessageSvc::Info( fmt::format( "MCDT entries (lumi scaling) {0} - {1} - {2} : {3}", m_configHolder.GetSample().Data(), to_string( el.first.first), to_string(el.first.second), to_string( el.second) ));
                    }
                    //assumes merging 11+12 and both polarities 
                    //int _polarity = polarity == Polarity::MD ? -1 : +1; ( tuple process ! )
                    TString _lumiWeight = fmt::format("({0}*(Year*Polarity==-11)+{1}*(Year*Polarity==11)+{2}*(Year*Polarity==-12)+{3}*(Year*Polarity==12))/{4}", 
                        (double)_lumis[{Year::Y2011,Polarity::MD}]*0.001 * _mcdtsingleFactor[{Year::Y2011,Polarity::MD}],
                        (double)_lumis[{Year::Y2011,Polarity::MU}]*0.001 * _mcdtsingleFactor[{Year::Y2011,Polarity::MU}],
                        (double)_lumis[{Year::Y2012,Polarity::MD}]*0.001 * _mcdtsingleFactor[{Year::Y2012,Polarity::MD}],
                        (double)_lumis[{Year::Y2012,Polarity::MU}]*0.001 * _mcdtsingleFactor[{Year::Y2012,Polarity::MU}],
                        (double)(_lumis[{Year::Y2011,Polarity::MD}] + _lumis[{Year::Y2011,Polarity::MU}] + _lumis[{Year::Y2012,Polarity::MD}] + _lumis[{Year::Y2012,Polarity::MU}])*0.001
                    );
                    m_weights["LUMI"] = _lumiWeight;
                    break;
                }
                case Year::Run2p1:{
                    //assumes merging 15+16 and both polarities 
                    double _mcdtsum = (double)_PATCHED_GETMCDTENTRIES_( m_configHolder.GetProject(),m_configHolder.GetYear(), Polarity::All, m_configHolder.GetSample(), false,  "ngng_evt");                
                    map< pair< Year, Polarity > , double  > _mcdtsingleFactor{
                        { { Year::Y2015, Polarity::MD } , _mcdtsum/(double)_PATCHED_GETMCDTENTRIES_( m_configHolder.GetProject(), Year::Y2015,Polarity::MD, m_configHolder.GetSample(), false,  "ngng_evt")   },
                        { { Year::Y2015, Polarity::MU } , _mcdtsum/(double)_PATCHED_GETMCDTENTRIES_( m_configHolder.GetProject(), Year::Y2015,Polarity::MU, m_configHolder.GetSample(), false,  "ngng_evt")   },
                        { { Year::Y2016, Polarity::MD } , _mcdtsum/(double)_PATCHED_GETMCDTENTRIES_( m_configHolder.GetProject(), Year::Y2016,Polarity::MD, m_configHolder.GetSample(), false,  "ngng_evt")   },
                        { { Year::Y2016, Polarity::MU } , _mcdtsum/(double)_PATCHED_GETMCDTENTRIES_( m_configHolder.GetProject(), Year::Y2016,Polarity::MU, m_configHolder.GetSample(), false,  "ngng_evt")   }
                    };
                    //int _polarity = polarity == Polarity::MD ? -1 : +1; ( tuple process ! )
                    TString _lumiWeight = fmt::format("({0}*(Year*Polarity==-15)+{1}*(Year*Polarity==15)+{2}*(Year*Polarity==-16)+{3}*(Year*Polarity==16))/{4}", 
                        (double)_lumis[ {Year::Y2015,Polarity::MD}]*0.001 * _mcdtsingleFactor[{Year::Y2015,Polarity::MD}],
                        (double)_lumis[ {Year::Y2015,Polarity::MU}]*0.001 * _mcdtsingleFactor[{Year::Y2015,Polarity::MU}],
                        (double)_lumis[ {Year::Y2016,Polarity::MD}]*0.001 * _mcdtsingleFactor[{Year::Y2016,Polarity::MD}],
                        (double)_lumis[ {Year::Y2016,Polarity::MU}]*0.001 * _mcdtsingleFactor[{Year::Y2016,Polarity::MU}],
                        (double)(_lumis[ {Year::Y2015,Polarity::MD}] + _lumis[ {Year::Y2015,Polarity::MU}] + _lumis[ {Year::Y2016,Polarity::MD}] + _lumis[ {Year::Y2016,Polarity::MU}])*0.001 
                    );
                    m_weights["LUMI"] = _lumiWeight;
                    break; 
                }
                case Year::Run2p2: {
                    double _mcdtsum = (double)_PATCHED_GETMCDTENTRIES_( m_configHolder.GetProject(),m_configHolder.GetYear(), Polarity::All, m_configHolder.GetSample(), false,  "ngng_evt");      
                    map< pair< Year, Polarity > , double  > _mcdtsingleFactor{
                        { { Year::Y2017, Polarity::MD } , _mcdtsum/(double)_PATCHED_GETMCDTENTRIES_( m_configHolder.GetProject(), Year::Y2017,Polarity::MD, m_configHolder.GetSample(), false,  "ngng_evt")   },
                        { { Year::Y2017, Polarity::MU } , _mcdtsum/(double)_PATCHED_GETMCDTENTRIES_( m_configHolder.GetProject(), Year::Y2017,Polarity::MU, m_configHolder.GetSample(), false,  "ngng_evt")   },
                        { { Year::Y2018, Polarity::MD } , _mcdtsum/(double)_PATCHED_GETMCDTENTRIES_( m_configHolder.GetProject(), Year::Y2018,Polarity::MD, m_configHolder.GetSample(), false,  "ngng_evt")   },
                        { { Year::Y2018, Polarity::MU } , _mcdtsum/(double)_PATCHED_GETMCDTENTRIES_( m_configHolder.GetProject(), Year::Y2018,Polarity::MU, m_configHolder.GetSample(), false,  "ngng_evt")   }
                    };
                    TString _lumiWeight = fmt::format("({0}*(Year*Polarity==-17)+{1}*(Year*Polarity==17)+{2}*(Year*Polarity==-18)+{3}*(Year*Polarity==18))/{4}", 
                        (double)_lumis[ {Year::Y2017,Polarity::MD}]*0.001 * _mcdtsingleFactor[ {Year::Y2017, Polarity::MD}], 
                        (double)_lumis[ {Year::Y2017,Polarity::MU}]*0.001 * _mcdtsingleFactor[ {Year::Y2017, Polarity::MU}], 
                        (double)_lumis[ {Year::Y2018,Polarity::MD}]*0.001 * _mcdtsingleFactor[ {Year::Y2018, Polarity::MD}], 
                        (double)_lumis[ {Year::Y2018,Polarity::MU}]*0.001 * _mcdtsingleFactor[ {Year::Y2018, Polarity::MU}], 
                        (double)(_lumis[ {Year::Y2017,Polarity::MD}] + _lumis[ {Year::Y2017,Polarity::MU}] + _lumis[ {Year::Y2018,Polarity::MD}] + _lumis[ {Year::Y2018,Polarity::MU}])*0.001
                    );
                    m_weights["LUMI"] = _lumiWeight;
                    break;
                }
                default :  MessageSvc::Warning("LUMI-Weight cannot be enabled for this Year configuration!"); break;
            };
        }
    }

    if (trg == Trigger::All) {
        if (m_debug) MessageSvc::Debug("Trigger::All", m_weightOption);
        // BUILDS A || (B && !A)
        if (m_weightOption.Contains(WeightDefRX::ID::L0)) {
            MessageSvc::Error("CreateWeightMC", (TString) "Trigger::All WeightDefRX::ID::L0 not implemented");
        }
    }
    if( m_weightOption.Contains("MODEL") && m_configHolder.IsSignalMC() ){
        m_weights["MODEL"] = "decay_model_w";
    }
    for (auto _weight : m_weights) {
        if (_weight.second == "") MessageSvc::Error("CreateWeightMC", (TString) "Invalid", _weight.first, "weight", "EXIT_FAILURE");
    }

    for (auto _weight : m_weights) { m_weights[_weight.first] = UpdateNamesWeightMC(_weight.first, _weight.second); }
    for (auto _weight : m_weights) { m_weights[_weight.first] = TString(ReplaceProject(TCut(_weight.second), m_configHolder.GetProject())); }
    for (auto _weight : m_weights) { m_weights[_weight.first] = CleanWeight(_weight.second); }

    if (IsWeightInMap(WeightDefRX::ID::BS, m_weights)) m_weight += " * " + m_weights[WeightDefRX::ID::BS];
    if (IsWeightInMap(WeightDefRX::ID::PID, m_weights)) m_weight += " * " + m_weights[WeightDefRX::ID::PID];
    if (IsWeightInMap(WeightDefRX::ID::ISMUON, m_weights)) m_weight += " * " + m_weights[WeightDefRX::ID::ISMUON];    
    if (IsWeightInMap(WeightDefRX::ID::TRK, m_weights)) m_weight += " * " + m_weights[WeightDefRX::ID::TRK];
    if (trg == Trigger::All) {
        if (IsWeightInMap(WeightDefRX::ID::TRGALL, m_weights)) m_weight += " * " + m_weights[WeightDefRX::ID::TRGALL];
    } else {
        if (IsWeightInMap(WeightDefRX::ID::L0, m_weights)) m_weight += " * " + m_weights[WeightDefRX::ID::L0];
    }
    if (IsWeightInMap(WeightDefRX::ID::HLT, m_weights)) m_weight    += " * " + m_weights[WeightDefRX::ID::HLT];
    if (IsWeightInMap(WeightDefRX::ID::BDT, m_weights)) m_weight    += " * " + m_weights[WeightDefRX::ID::BDT];
    if (IsWeightInMap(WeightDefRX::ID::BDT2, m_weights)) m_weight   += " * " + m_weights[WeightDefRX::ID::BDT2];
    if (IsWeightInMap(WeightDefRX::ID::RW1D, m_weights)) m_weight    += " * " + m_weights[WeightDefRX::ID::RW1D];
    
    if (IsWeightInMap(WeightDefRX::ID::LB   , m_weights)) m_weight += " * " + m_weights[WeightDefRX::ID::LB];
    if (IsWeightInMap(WeightDefRX::ID::LB_KIN, m_weights)) m_weight += " * " + m_weights[WeightDefRX::ID::LB_KIN];

    if (IsWeightInMap(WeightDefRX::ID::PTRECO, m_weights)) m_weight += " * " + m_weights[WeightDefRX::ID::PTRECO];
    if (IsWeightInMap("XJPsWeight",            m_weights)) m_weight += " * " + m_weights["XJPSWeight"];
    if (IsWeightInMap("LUMI",            m_weights))       m_weight += " * " + m_weights["LUMI"];

    if (IsWeightInMap("Q2",            m_weights))       m_weight += " * " + m_weights["Q2"];
    if (IsWeightInMap("MODEL", m_weights))               m_weight += " * " + m_weights["MODEL"];
    m_weight = CleanWeight(m_weight);
    if (IsWeight(m_weight)) m_weight = "(" + m_weight + ")";

    m_weights[WeightDefRX::ID::FULL] = m_weight;

    if (m_debug) MessageSvc::Debug("CreateWeightMC", m_weight);
    return;
}

TString WeightHolderRX::UpdateNamesWeightMC(TString _name, TString _weight) {
    if (m_debug) MessageSvc::Debug("UpdateNamesWeightMC", _name, _weight);

    Prj      prj = m_configHolder.GetProject();
    Analysis ana = m_configHolder.GetAna();
    Trigger  trg = m_configHolder.GetTrigger();
    if (trg == Trigger::L0H ) trg = Trigger::L0L;

    if (m_weightOption.Contains(WeightDefRX::ID::TRK) && _name.Contains(WeightDefRX::ID::TRK)) {
        if (m_debug) MessageSvc::Debug("Weight " + _name);

        if (m_weightConfig.Contains("kde"))
            // _weight.ReplaceAll("w", "wi"); 
            _weight.ReplaceAll("w", "w"); //use no interpolation for TRK weights (since Marteen update on 3D maps (Phi,Eta,PT based))
        else if (m_weightConfig.Contains("fit") || m_weightConfig.Contains("meerkat"))
            // _weight.ReplaceAll("w", "wi");
            _weight.ReplaceAll("w", "w"); //use no interpolation for TRK weights (since Marteen update on 3D maps (Phi,Eta,PT based))
        else if (m_weightConfig.Contains("nointerp"))
            _weight.ReplaceAll("wi", "w");
        else if (m_weightConfig.Contains("interp"))
            _weight.ReplaceAll("w", "wi");
        else
            MessageSvc::Error("UpdateNamesWeightMC " + _name, (TString) "Invalid weightConfig", m_weightConfig, "EXIT_FAILURE");
    }

    if (m_weightOption.Contains(WeightDefRX::ID::PID) && _name.Contains(WeightDefRX::ID::PID)) {
        if (m_debug) MessageSvc::Debug("Weight " + _name);

        if (SettingDef::Weight::useMCRatioPID && ana == Analysis::EE) //Use weight data simulation ratios for electrons
            _weight.ReplaceAll("PIDCalib","PIDCalib_Weight");

        if (m_weightConfig.Contains("kde"))
            //_weight.ReplaceAll("w", "wk");
            _weight.ReplaceAll("PIDCalib", "PIDCalibKDE");
        else if (m_weightConfig.Contains("fit") || m_weightConfig.Contains("kde")) {
            _weight.ReplaceAll("w", "wi");
            // _weight.ReplaceAll("w", "w");
        } else if (m_weightConfig.Contains("nointerp"))
            _weight.ReplaceAll("wi", "w");
        else if (m_weightConfig.Contains("interp"))
            _weight.ReplaceAll("w", "wi");
        else
            MessageSvc::Error("UpdateNamesWeightMC " + _name, (TString) "Invalid weightConfig", m_weightConfig, "EXIT_FAILURE");
    }
    if(m_weightOption.Contains(WeightDefRX::ID::ISMUON) && _name.Contains(WeightDefRX::ID::ISMUON)) {
        if (m_debug) MessageSvc::Debug("Weight " + _name);
        if (m_weightConfig.Contains("kde"))
            _weight.ReplaceAll("PIDCalib", "PIDCalibKDE");
        else if (m_weightConfig.Contains("fit") || m_weightConfig.Contains("kde") || m_weightConfig.Contains("meerkat")) {
            _weight.ReplaceAll("w", "wi");
        } else if (m_weightConfig.Contains("nointerp"))
            _weight.ReplaceAll("wi", "w");
        else if (m_weightConfig.Contains("interp"))
            _weight.ReplaceAll("w", "wi");
        else
            MessageSvc::Error("UpdateNamesWeightMC " + _name, (TString) "Invalid weightConfig", m_weightConfig, "EXIT_FAILURE");
    }
    if ((m_weightOption.Contains(WeightDefRX::ID::BDT) || m_weightOption.Contains(WeightDefRX::ID::BDT2)) && (_name.Contains(WeightDefRX::ID::BDT) || _name.Contains(WeightDefRX::ID::BDT2) || _name.Contains(WeightDefRX::ID::TRGALL))) {
        if (m_debug) MessageSvc::Debug("Weight " + _name);
        if (_name.Contains(WeightDefRX::ID::BDT2) && ana == Analysis::EE) {
            _weight.ReplaceAll("BDT2", "BDT3");
            _weight.ReplaceAll("{L0}", to_string(trg));
        }
        if (_name.Contains(WeightDefRX::ID::BDT2) && ana == Analysis::MM) _weight.ReplaceAll("{L0}", "L0L");
        if (m_weightOption.Contains(WeightDefRX::ID::BKIN) && m_weightOption.Contains(WeightDefRX::ID::MULT))
            _weight.ReplaceAll("{OPT}", "BKIN_MULT");
        else if (m_weightOption.Contains(WeightDefRX::ID::BKIN))
            _weight.ReplaceAll("{OPT}", "BKIN");
        else
            MessageSvc::Error("UpdateNamesWeightMC " + _name, (TString) "Invalid weightOption", m_weightOption, "EXIT_FAILURE");
        if (m_weightConfig.Contains("fit") || m_weightConfig.Contains("kde"))
            _weight.ReplaceAll("w", "wf");   // these branches are based on fitted L0 weights
        else if (m_weightConfig.Contains("nointerp"))
            _weight.ReplaceAll("wi", "w");   // these branches are based on no interpolated binned L0 weights
        else if (m_weightConfig.Contains("interp"))
            _weight.ReplaceAll("w", "wi");   // these branches are based on interpolated binned L0 weights
        else if (m_weightConfig.Contains("meerkat"))
            _weight.ReplaceAll("w", "wf");
        else
            MessageSvc::Error("UpdateNamesWeightMC " + _name, (TString) "Invalid weightConfig", m_weightConfig, "EXIT_FAILURE");

        if (m_weightConfig.Contains("Bp"))
            _weight.ReplaceAll("{B}", "Bp");
        else if (m_weightConfig.Contains("B0"))
            _weight.ReplaceAll("{B}", "B0");
        else if (m_weightConfig.Contains("Bs"))
            _weight.ReplaceAll("{B}", "Bs");
        else
            MessageSvc::Error("UpdateNamesWeightMC " + _name, (TString) "Invalid weightConfig", m_weightConfig, "EXIT_FAILURE");

        _weight.ReplaceAll("{LL}", to_string(m_configHolder.GetAna()));

        _weight.ReplaceAll("{L0}", m_configHolder.GetTriggerAndConf("short"));

    } else if (m_weightOption.Contains(WeightDefRX::ID::RW1D) && _name.Contains(WeightDefRX::ID::RW1D)) {
        
        if (m_debug) MessageSvc::Debug("Weight " + _name);
        
        if (m_weightConfig.Contains("Bp"))
            _weight.ReplaceAll("{B}", "Bp");
        else if (m_weightConfig.Contains("B0"))
            _weight.ReplaceAll("{B}", "B0");
        else if (m_weightConfig.Contains("Bs"))
            _weight.ReplaceAll("{B}", "Bs");
        else
            MessageSvc::Error("UpdateNamesWeightMC " + _name, (TString) "Invalid weightConfig", m_weightConfig, "EXIT_FAILURE");

    } else if ((m_weightOption.Contains(WeightDefRX::ID::BKIN) || m_weightOption.Contains(WeightDefRX::ID::MULT) || m_weightOption.Contains(WeightDefRX::ID::RECO)) && (_name.Contains(WeightDefRX::ID::BKIN) || _name.Contains(WeightDefRX::ID::MULT) || _name.Contains(WeightDefRX::ID::RECO) || _name.Contains(WeightDefRX::ID::TRGALL))) {

        if (m_debug) MessageSvc::Debug("Weight " + _name);

        if (m_weightConfig.Contains("MML0Lincl")) {
            _weight.ReplaceAll("{LL}_{L0}", "MM_L0L_incl");
            _weight.ReplaceAll("_{pre}", "_MML0Lincl");
        } else if (m_weightConfig.Contains("MML0Lexcl")) {
            _weight.ReplaceAll("{LL}_{L0}", "MM_L0L_excl");
            _weight.ReplaceAll("_{pre}", "_MML0Lexcl");
        } else
            _weight.ReplaceAll("_{pre}", "");

        if (m_weightConfig.Contains("fit") || m_weightConfig.Contains("kde"))
            _weight.ReplaceAll("w", "wf");   // these branches are based on fitted L0 weights
        else if (m_weightConfig.Contains("nointerp"))
            _weight.ReplaceAll("wi", "w");   // these branches are based on no interpolated binned L0 weights
        else if (m_weightConfig.Contains("interp"))
            _weight.ReplaceAll("w", "wi");   // these branches are based on interpolated binned L0 weights
        else if (m_weightConfig.Contains("meerkat"))
            _weight.ReplaceAll("w", "wf");
        else
            MessageSvc::Error("UpdateNamesWeightMC " + _name, (TString) "Invalid weightConfig", m_weightConfig, "EXIT_FAILURE");

        if (m_weightConfig.Contains("Bp"))
            _weight.ReplaceAll("{B}", "Bp");
        else if (m_weightConfig.Contains("B0"))
            _weight.ReplaceAll("{B}", "B0");
        else if (m_weightConfig.Contains("Bs"))
            _weight.ReplaceAll("{B}", "Bs");
        else
            MessageSvc::Error("UpdateNamesWeightMC " + _name, (TString) "Invalid weightConfig", m_weightConfig, "EXIT_FAILURE");

        _weight.ReplaceAll("{LL}", to_string(m_configHolder.GetAna()));
        // if ((m_configHolder.GetAna() == Analysis::EE) && m_weightConfig.Contains("EE")) _weight.ReplaceAll("MM", "EE");

        _weight.ReplaceAll("{L0}", m_configHolder.GetTriggerAndConf("short"));

        // if (m_weightConfig.Contains("L0I"))
        //    _weight.ReplaceAll("{L0}", "L0I");
        // else if (m_weightConfig.Contains("L0L"))
        //    _weight.ReplaceAll("{L0}", "L0L");
        // else
        //    MessageSvc::Error("UpdateNamesWeightMC " + _name, (TString) "Invalid weightConfig", m_weightConfig, "EXIT_FAILURE");

        // if (m_weightConfig.Contains("inclusive"))
        //    _weight.ReplaceAll("", "");
        // else if (m_weightConfig.Contains("exclusive2"))
        //    _weight.ReplaceAll("L0I", "L0I_exclusive");
        // else if (m_weightConfig.Contains("exclusive"))
        //    _weight.ReplaceAll("L0L", "L0L_exclusive");
        // else
        //    MessageSvc::Error("UpdateNamesWeightMC " + _name, (TString) "Invalid weightConfig", m_weightConfig, "EXIT_FAILURE");
    }

    if (m_weightOption.Contains(WeightDefRX::ID::L0) && (_name.Contains(WeightDefRX::ID::L0) || _name.Contains(WeightDefRX::ID::TRGALL))) {
        if (m_debug) MessageSvc::Debug("Weight " + _name);

        if (m_weightConfig.Contains("fit") || m_weightConfig.Contains("kde"))
            _weight.ReplaceAll("w", "wf");
        else if (m_weightConfig.Contains("nointerp"))
            _weight.ReplaceAll("wi", "w");
        else if (m_weightConfig.Contains("interp"))
            _weight.ReplaceAll("w", "wi");
        else if (m_weightConfig.Contains("meerkat"))
            _weight.ReplaceAll("w", "wf");
        else
            MessageSvc::Error("UpdateNamesWeightMC " + _name, (TString) "Invalid weightConfig", m_weightConfig, "EXIT_FAILURE");

        if (m_weightConfig.Contains("Bp"))
            _weight.ReplaceAll("{B}", "Bp");
        else if (m_weightConfig.Contains("B0"))
            _weight.ReplaceAll("{B}", "B0");
        else if (m_weightConfig.Contains("Bs"))
            _weight.ReplaceAll("{B}", "Bs");
        else
            MessageSvc::Error("UpdateNamesWeightMC " + _name, (TString) "Invalid weightConfig", m_weightConfig, "EXIT_FAILURE");

        vector< pair< TString, TString > > _heads = GetParticleBranchNames(m_configHolder.GetProject(), m_configHolder.GetAna(), m_configHolder.GetQ2bin(), "onlyhead");
        _weight.ReplaceAll("{B}", _heads[0].first).ReplaceAll("{HEAD}", _heads[0].first);

        //Important bit 
        if(SettingDef::Weight::useStatusL0Formula == false){
            if (m_configHolder.GetYear() == Year::Run1) {
                _weight.ReplaceAll("E1_L0ElectronDecision_TOS",Form("((E1_L0Calo_ECAL_realET > %f && Year == 11) || (E1_L0Calo_ECAL_realET > %f && Year == 12))", CutDefRX::Quality::L0Calo_ECAL_realET_Thresholds.at( Year::Y2011), CutDefRX::Quality::L0Calo_ECAL_realET_Thresholds.at( Year::Y2012)));
                _weight.ReplaceAll("E2_L0ElectronDecision_TOS",Form("((E2_L0Calo_ECAL_realET > %f && Year == 11) || (E2_L0Calo_ECAL_realET > %f && Year == 12))", CutDefRX::Quality::L0Calo_ECAL_realET_Thresholds.at( Year::Y2011), CutDefRX::Quality::L0Calo_ECAL_realET_Thresholds.at( Year::Y2012)));
            }
            else if (m_configHolder.GetYear() == Year::Run2p1) {
                _weight.ReplaceAll("E1_L0ElectronDecision_TOS",Form("((E1_L0Calo_ECAL_realET > %f && Year == 15) || (E1_L0Calo_ECAL_realET > %f && Year == 16))", CutDefRX::Quality::L0Calo_ECAL_realET_Thresholds.at( Year::Y2015), CutDefRX::Quality::L0Calo_ECAL_realET_Thresholds.at( Year::Y2016)));
                _weight.ReplaceAll("E2_L0ElectronDecision_TOS",Form("((E2_L0Calo_ECAL_realET > %f && Year == 15) || (E2_L0Calo_ECAL_realET > %f && Year == 16))", CutDefRX::Quality::L0Calo_ECAL_realET_Thresholds.at( Year::Y2015), CutDefRX::Quality::L0Calo_ECAL_realET_Thresholds.at( Year::Y2016)));
            }
            else if (m_configHolder.GetYear() == Year::Run2p2) {
                _weight.ReplaceAll("E1_L0ElectronDecision_TOS",Form("((E1_L0Calo_ECAL_realET > %f && Year == 17) || (E1_L0Calo_ECAL_realET > %f && Year == 18))", CutDefRX::Quality::L0Calo_ECAL_realET_Thresholds.at( Year::Y2017), CutDefRX::Quality::L0Calo_ECAL_realET_Thresholds.at( Year::Y2018)));
                _weight.ReplaceAll("E2_L0ElectronDecision_TOS",Form("((E2_L0Calo_ECAL_realET > %f && Year == 17) || (E2_L0Calo_ECAL_realET > %f && Year == 18))", CutDefRX::Quality::L0Calo_ECAL_realET_Thresholds.at( Year::Y2017), CutDefRX::Quality::L0Calo_ECAL_realET_Thresholds.at( Year::Y2018)));
            } else {
                _weight.ReplaceAll("E1_L0ElectronDecision_TOS",Form("(E1_L0Calo_ECAL_realET > %f)", CutDefRX::Quality::L0Calo_ECAL_realET_Thresholds.at( m_configHolder.GetYear())));
                _weight.ReplaceAll("E2_L0ElectronDecision_TOS",Form("(E2_L0Calo_ECAL_realET > %f)", CutDefRX::Quality::L0Calo_ECAL_realET_Thresholds.at( m_configHolder.GetYear())));                
            }
            _weight.ReplaceAll("M1_L0MuonDecision_TOS","(1>0)");
            _weight.ReplaceAll("M2_L0MuonDecision_TOS","(1>0)");            
        }else{
            _weight.ReplaceAll("E1_L0ElectronDecision_TOS",Form("(E1_L0ElectronDecision_TOS && E1_L0Calo_ECAL_realET > %f)", CutDefRX::Quality::L0Calo_ECAL_realET_Thresholds.at( m_configHolder.GetYear())));
            _weight.ReplaceAll("E2_L0ElectronDecision_TOS",Form("(E2_L0ElectronDecision_TOS && E2_L0Calo_ECAL_realET > %f)", CutDefRX::Quality::L0Calo_ECAL_realET_Thresholds.at( m_configHolder.GetYear())));
        }
        //The final L0 weight will be as baseline ( useStatusL0Formula == false by default )
        //P(data) = ( 1- ( 1- w1*( P1_realET))*(1- w2*( P2_realET )))
        //For useStatusL0Formula =True : 
        //P(data) = ( 1- ( 1- w1*( P1_TOS && realET))*(1- w2*(P2_TOS && realET )))
    }

    if (m_weightOption.Contains(WeightDefRX::ID::HLT) && (_name.Contains(WeightDefRX::ID::HLT) || _name.Contains(WeightDefRX::ID::TRGALL))) {
        if (m_debug) MessageSvc::Debug("Weight " + _name);

        if (m_weightConfig.Contains("fit") || m_weightConfig.Contains("kde"))
            _weight.ReplaceAll("w", "wi");   // Bp_fit chained to w (interp)
        else if (m_weightConfig.Contains("nointerp"))
            _weight.ReplaceAll("wi", "w");
        else if (m_weightConfig.Contains("interp"))
            _weight.ReplaceAll("w", "wi");
        else if (m_weightConfig.Contains("meerkat"))
            _weight.ReplaceAll("w", "wi");   // meerkat chaining with interp
        else
            MessageSvc::Error("UpdateNamesWeightMC " + _name, (TString) "Invalid weightConfig", m_weightConfig, "EXIT_FAILURE");

        if (trg != Trigger::All) {
            switch (trg) {
                case Trigger::L0I: _weight.ReplaceAll("_{L0}_", "_L0I_"); break;
                case Trigger::L0L: _weight.ReplaceAll("_{L0}_", "_L0L_"); break;
                case Trigger::L0H: _weight.ReplaceAll("_{L0}_", "_L0L_"); break;
                default: MessageSvc::Error("UpdateNamesWeightMC " + _name, (TString) "Invalid trg", to_string(trg), "EXIT_FAILURE"); break;
            }
        }

        if (m_weightConfig.Contains("Bp"))
            _weight.ReplaceAll("{B}", "Bp");
        else if (m_weightConfig.Contains("B0"))
            _weight.ReplaceAll("{B}", "B0");
        else if (m_weightConfig.Contains("Bs"))
            _weight.ReplaceAll("{B}", "Bs");
        else
            MessageSvc::Error("UpdateNamesWeightMC " + _name, (TString) "Invalid weightConfig", m_weightConfig, "EXIT_FAILURE");
        if( m_weightOption.Contains( WeightDefRX::ID::HLT +"-nTracks")){
            _weight.ReplaceAll("HLT_","HLTnTracks_");
        }
        if( m_weightOption.Contains( WeightDefRX::ID::HLT +"-BETA")){
            _weight.ReplaceAll("HLT_","HLTBETA_");
        }
    }



    if (m_debug) MessageSvc::Debug("UpdateNamesWeightMC", _weight);
    return _weight;
}

void WeightHolderRX::CreateWeightCL() {
    if (m_debug) MessageSvc::Debug("CreateWeightCL", m_weightOption);

    m_weight = TString(NOWEIGHT);

    if (m_weightOption.Contains(WeightDefRX::ID::SP)) {
        Q2Bin q2bin = m_configHolder.GetQ2bin();

        TString _weight = "nsig_";
        switch (q2bin) {
            case Q2Bin::JPsi: _weight += "JPs"; break;
            default: MessageSvc::Error("CreateWeightCL", (TString) "Invalid q2bin", to_string(q2bin), "EXIT_FAILURE"); break;
        }
        _weight += SettingDef::separator;
        _weight += m_configHolder.GetKey();
        _weight += "_sw";
        _weight.ReplaceAll("-", "M");   // WEIRD FEATURE OF GetClonedTree
        _weight = CleanWeight(_weight);

        m_weights[WeightDefRX::ID::SP] = _weight;
    }

    if (m_weightOption.Contains(WeightDefRX::ID::BS) && m_configHolder.HasBS() && SettingDef::Weight::useBS && SettingDef::Weight::iBS>=0 && SettingDef::Weight::iBS< WeightDefRX::nBS){ 
        m_weights[WeightDefRX::ID::BS] = TString(WeightDefRX::BS) + "[" + to_string(SettingDef::Weight::iBS) + "]"; 
    }
    
    if (IsWeightInMap(WeightDefRX::ID::BS, m_weights)) m_weight += " * " + m_weights[WeightDefRX::ID::BS];
    if (IsWeightInMap(WeightDefRX::ID::SP, m_weights)) m_weight += " * " + m_weights[WeightDefRX::ID::SP];

    m_weight = CleanWeight(m_weight);
    m_weight = "(" + m_weight + ")";

    m_weights[WeightDefRX::ID::FULL] = m_weight;

    if (m_debug) MessageSvc::Debug("CreateWeightCL", m_weight);
    return;
}

#endif
