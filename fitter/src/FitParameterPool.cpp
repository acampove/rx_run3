#ifndef FITPARAMETERPOOL_CPP
#define FITPARAMETERPOOL_CPP

#include "FitParameterPool.hpp"

#include "ConstDef.hpp"
#include "EfficiencySvc.hpp"
#include "MessageSvc.hpp"
#include "UnblindParameter.hpp"

#include "format.h"

FitParameterPool::FitParameterPool() { UnblindParameter::SetSeed(SettingDef::blindString); }


void FitParameterPool::InitEffRatioSystematicCovariance(){
    MessageSvc::Info(Color::Cyan, "FitParameterPool",  (TString) "InitEffRatioSystematicCovariance, set all elements to 0");
    //Assumes m_ratioMap has been already filled, ( order of calls matters, see FitGenerator Parser etc..)
    int nRows = 0;
    for( auto & ratioEl_Row : m_ratioMap ){
        if( ratioEl_Row.first.second != RatioType::EfficiencyRatioSystematic) continue;
        nRows ++;
        for( auto & ratioEl_Col : m_ratioMap){
            if( ratioEl_Col.first.second != RatioType::EfficiencyRatioSystematic) continue;
            m_systematicsRatios[ratioEl_Row.first][ratioEl_Col.first] = 0.f;
        }
    }
    
    MessageSvc::Info(Color::Cyan, "FitParameterPool",  TString::Format("Recognized a matrix of %i x %i", nRows,nRows));
    auto _find_position_index_label =[]( TString _myLabel , vector<TString> _availableLabels ){
        int _indexFound = -1;
        for( int i = 0 ; i < _availableLabels.size(); ++i){
            if( _myLabel == _availableLabels[i] && _indexFound < 0){ _indexFound = i; continue;}
            if( _myLabel == _availableLabels[i] && _indexFound > 0){ MessageSvc::Error("Found label more than once, yaml is probably bugged", "","EXIT_FAILURE"); }
        }
        if( _indexFound < 0) MessageSvc::Error("Didn't found the looked for label, will abort, there is a bug to fix...","","EXIT_FAILURE");
        return _indexFound;
    };
    if(m_systematicsRatios.size() !=0 ){
        TMatrixDSym matrixSum(m_systematicsRatios.size());
        MessageSvc::Warning("Loading external yamls systematic, labels match mapping");
        vector< TString> _LABELS_FITTER_WANTS = {};
        for( auto & _column : m_systematicsRatios) _LABELS_FITTER_WANTS.push_back( _column.first.first.GetKey("eff_syst"));
        for( auto & _SYST_YAML_FILENAME : SettingDef::Fit::RatioSystFile){
            MessageSvc::Warning("Loading file for systematic", _SYST_YAML_FILENAME );
            auto _PARSERYAML_ = YAML::LoadFile(_SYST_YAML_FILENAME.Data());
            std::vector<TString> _YAML_LABELS_ = _PARSERYAML_["labels"].as<vector<TString>>();
            for( auto & _LABEL_FIT : _LABELS_FITTER_WANTS){
                if( ! CheckVectorContains( _YAML_LABELS_, _LABEL_FIT  )){
                    MessageSvc::Error("Yaml fit loaded doesn't contains the label needed by the fit!", _LABEL_FIT);
                }
            }
            TMatrixDSym matrix(m_systematicsRatios.size());
            int idx_Row = 0;
            for( auto & _row : m_systematicsRatios){
                TString _label_row = _row.first.first.GetKey("eff_syst");
                std::cout<< idx_Row << " ) "<< _label_row << std::endl;
                vector<double> _values_covariance = _PARSERYAML_["cov_matrix"][_label_row.Data()].as<vector<double>>();
                int idx_Col = 0;
                for( auto & _column : m_systematicsRatios){
                    TString _label_column = _column.first.first.GetKey("eff_syst");
                    int _indexRow = _find_position_index_label( _label_row,    _YAML_LABELS_);
                    int _indexCol = _find_position_index_label( _label_column, _YAML_LABELS_);
                    double value = _values_covariance.at(_indexCol) * pow(SettingDef::Efficiency::scaleSystematics,2);
                    matrix( idx_Row, idx_Col) =  value;
                    m_systematicsRatios[_row.first][_column.first]+= value;
                    idx_Col++;
                }
                idx_Row++;
            }
            matrix.Print();
            if( !matrix.IsSymmetric()) MessageSvc::Error("matrix source is NOT symmetric!", "","EXIT_FAILURE");
            matrixSum+= matrix;
        }
        MessageSvc::Warning("Combined Covariance matrix (sum of elements)");
        matrixSum.Print();
        if( !matrixSum.IsSymmetric()) MessageSvc::Error("matrix sum is NOT symmetric!", "","EXIT_FAILURE");        
    }
    return;
}
pair< RooArgList, TMatrixDSym > FitParameterPool::GetSystematicEffRatioListAndCovarianceMatrix(){
    RooArgList _listOfSystematicsEfficiencyRatios;
    TMatrixDSym _covariance( m_systematicsRatios.size());
    int idxRow = 0;
    for( auto & row : m_systematicsRatios ){
        int idxCol = 0;
        _listOfSystematicsEfficiencyRatios.add( *GetRatioParameter( row.first.first,  RatioType::EfficiencyRatioSystematic) );
        for( auto & col : row.second){
            _covariance(idxRow,idxCol) = m_systematicsRatios.at(row.first).at(col.first);
            idxCol++;
        }
        idxRow++;
    }
    MessageSvc::Warning("GetSystematicEffRatioListAndCovarianceMatrix");
    _listOfSystematicsEfficiencyRatios.Print();
    _covariance.Print();
    return make_pair(_listOfSystematicsEfficiencyRatios, _covariance);
}
void FitParameterPool::ClearParameters() {
    MessageSvc::Info(Color::Cyan, "FitParameterPool", (TString) "ClearParameters");
    DeleteParametersInMap(m_efficiencyMap);
    DeleteParametersInMap(m_yieldMap);
    DeleteParametersInMap(m_shapeParameterMap);
    DeleteParametersInMap(m_fixedEfficiencyMap);
    DeleteParametersInMap(m_ratioMap);
    m_ratioMap.clear();
    m_efficiencyMap.clear();
    m_yieldMap.clear();
    m_shapeParameterMap.clear();
    m_fixedEfficiencyMap.clear();
    //ClearParameters Must remove everything , including the vector<RooRealVar*> of constrained parameters.
    m_constrainedParameters.clear();
}

template < typename T > void FitParameterPool::DeleteParametersInMap(map< T, ParameterWrapper > & _parameterMap) {
    for (auto & _keyParameterPair : _parameterMap) {
        auto _parameterWrapper = _keyParameterPair.second;
        _parameterWrapper.DeleteParameter();
    }
}
void FitParameterPool::ConfigureParameters(vector< FitConfiguration > _configurations, TString _option) {
    MessageSvc::Info(Color::Cyan, "FitParameterPool", (TString) "ConfigureParameters", _option, to_string(_configurations.size()));
    vector< FitParameterConfig > _neededEfficiencies;
    //TODO : IMPLEMENT LOGIC HERE OF FORRATIO() and READAPT THE GETEFFICIENCY() CALLEE...
    //TODO : IMPLEMENT LOGIC HERE OF COVARIANCE MATRIX CONSTRUCTION, ELSE FITTERTOOL WILL CONSTRUCT IT AT THE END....
    
    for (auto & _configuration : _configurations) {
        TString _configKey = _configuration.GetKey();

        if( SettingDef::Fit::useRatioComb && 
           ( !( _configuration.GetYear()    == Year::Run2p2 && 
                _configuration.GetTrigger() == Trigger::L0I ) && 
            ( _configuration.GetQ2bin() == Q2Bin::Low || _configuration.GetQ2bin() == Q2Bin::Central )  
           )
            && _configuration.HasSample(Sample::Comb)
        ){
            TString _key = _configKey;
            AddShapeParameter(new RooRealVar("b_RatioComb_" + _key, "b_{" + _key + "}^{scale}", 1, -100, 100));
        }

        if((!(_option.Contains("modshapejpsmm") && (_configuration.GetAna() == Analysis::MM)) && !(_option.Contains("modshapejpsee") && (_configuration.GetAna() == Analysis::EE))) || _configuration.GetQ2bin() == Q2Bin::JPsi){
           if (!_option.Contains("modshifttagprobe") ||
                (_option.Contains("modshifttagprobe") && _configuration.GetTrack() == Track::TAG)) {
                TString _keyOption = "";
                if (_option.Contains("modshifttagprobe")) _keyOption += "-notrack";

                if (_option.Contains("modshiftrx") && _configuration.GetProject() == Prj::RKst) {
                    _keyOption += "-noprj";
                    TString _key = _configuration.GetKey(_keyOption);
                    AddShapeParameter(new RooRealVar("m_shift_" + _key, "m_{" + _key + "}^{shift}", 0, -50, 50));
                } else {
                    TString _key = _configuration.GetKey(_keyOption);
                    AddShapeParameter(new RooRealVar("m_shift_" + _key, "m_{" + _key + "}^{shift}", 0, -50, 50));
                }
            }
            if (!_option.Contains("modscaletagprobe") || 
                (_option.Contains("modscaletagprobe") && _configuration.GetTrack() == Track::TAG)) {
                TString _keyOption = "";
                if (_option.Contains("modscaletagprobe")) _keyOption += "-notrack";
                if (_option.Contains("modscalerx") && _configuration.GetProject() == Prj::RKst) {
                    auto _projectLessKey = _configuration.GetKey("noprj");
                    AddShapeParameter(new RooRealVar("s_scale_" + _projectLessKey, "#sigma_{" + _projectLessKey + "}^{scale}", 1.1, 0.5, 2));
                } else {
                    // Before  (were not using modscaletagprobe on MM alone)
                    // AddShapeParameter(new RooRealVar("s_scale_" + _configKey, "#sigma_{" + _configKey + "}^{scale}", 1.1, 0.5, 2));                    
                    TString _key = _configuration.GetKey(_keyOption);
                    AddShapeParameter(new RooRealVar("s_scale_" + _key, "#sigma_{" + _key + "}^{scale}", 1.1, 0.5, 2));                    
                    if( _option.Contains("modscaleLR")){
                        AddShapeParameter(new RooRealVar("s_scaleL_" + _key, "#sigma(L)_{" + _key + "}^{scale}", 1.1, 0.5, 2));                    
                        AddShapeParameter(new RooRealVar("s_scaleR_" + _key, "#sigma(R)_{" + _key + "}^{scale}", 1.1, 0.5, 2));                    
                    }
                }
            }

            if (_option.Contains("modtailrx") && _configuration.GetProject() == Prj::RKst) {
                auto _projectLessKey = _configuration.GetKey("noprj");
                if (_option.Contains("modshapetail")) {
                    // AddShapeParameter(new RooRealVar("f_shift_" + _projectLessKey, "f_{" + _projectLessKey + "}^{shift}", 0, -1, 1));
                    AddShapeParameter(new RooRealVar("a_scale_" + _projectLessKey, "#alpha_{" + _projectLessKey + "}^{scale}", 1, 0.5, 2));
                    AddShapeParameter(new RooRealVar("n_scale_" + _projectLessKey, "n_{" + _projectLessKey + "}^{scale}", 1, 0.5, 2));
                }
            } else {
                if (_option.Contains("modshapetail")) {
                    // AddShapeParameter(new RooRealVar("f_shift_" + _configKey, "f_{" + _configKey + "}^{shift}", 0, -1, 1));
                    AddShapeParameter(new RooRealVar("a_scale_" + _configKey, "#alpha_{" + _configKey + "}^{scale}", 1, 0.5, 2));
                    AddShapeParameter(new RooRealVar("n_scale_" + _configKey, "n_{" + _configKey + "}^{scale}", 1, 0.5, 2));
                }
            }

            if (_option.Contains("modshapefrac")) {
                AddShapeParameter(new RooRealVar("fcb_scale_" + _configKey, "fcb_{" + _configKey + "}^{scale}", 1, 0.5, 2));
                AddShapeParameter(new RooRealVar("fg_scale_" + _configKey, "fg_{" + _configKey + "}^{scale}", 1, 0.5, 2));
            }

            AddShapeParameter(new RooRealVar("m_conv_" + _configKey, "m_{" + _configKey + "}^{convolution}", 0, -50, 50));
            AddShapeParameter(new RooRealVar("s_conv_" + _configKey, "#sigma_{" + _configKey + "}^{convolution}", 5, 0, 100));
        }

        if (_configuration.GetAna() == Analysis::EE && _option.Contains("modshiftbrem")) {
            auto _bremKey = _configuration.GetKeyWithBrem(Brem::G0);
            if (_option.Contains("modshiftrx") && _configuration.GetProject() == Prj::RKst) {
                auto _projectLessKey = _bremKey.ReplaceAll(to_string(_configuration.GetProject()) + SettingDef::separator, "");
                AddShapeParameter(new RooRealVar("m_shift_" + _projectLessKey, "m_{" + _projectLessKey + "}^{shift}", 0, -50, 50));
            } else {
                AddShapeParameter(new RooRealVar("m_shift_" + _bremKey, "m_{" + _bremKey + "}^{shift}", 0, -50, 50));
            }
            vector< Brem > _brems = {Brem::G1, Brem::G2};
            for (auto _brem : _brems) {
                _bremKey = _configuration.GetKeyWithBrem(_brem);
                if (_option.Contains("modshiftbremdifference")) {
                    if (_option.Contains("modshiftrx") && _configuration.GetProject() == Prj::RKst) {
                        auto _projectLessKey = _bremKey.ReplaceAll(to_string(_configuration.GetProject()) + SettingDef::separator, "");
                        ModShiftBremDifference(_bremKey);
                    } else {
                        ModShiftBremDifference(_bremKey);
                    }
                } else {
                    if (_option.Contains("modshiftrx") && _configuration.GetProject() == Prj::RKst) {
                        auto _projectLessKey = _bremKey.ReplaceAll(to_string(_configuration.GetProject()) + SettingDef::separator, "");
                        RemoveShapeParameter("m_shift_" + _projectLessKey);
                        AddShapeParameter(new RooRealVar("m_shift_" + _projectLessKey, "m_{" + _projectLessKey + "}^{shift}", 0, -50, 50));
                    } else {
                        AddShapeParameter(new RooRealVar("m_shift_" + _bremKey, "m_{" + _bremKey + "}^{shift}", 0, -50, 50));
                    }
                }
            }
        }

        if (_configuration.GetAna() == Analysis::EE && _option.Contains("modscalebrem")) {
            vector< Brem > _brems = {Brem::G0, Brem::G1, Brem::G2};
            for (auto _brem : _brems) {
                auto _bremKey = _configuration.GetKeyWithBrem(_brem);
                if (_option.Contains("modscalerx") && _configuration.GetProject() == Prj::RKst) {
                    auto _projectLessKey = _bremKey.ReplaceAll(to_string(_configuration.GetProject()) + SettingDef::separator, "");
                    AddShapeParameter(new RooRealVar("s_scale_" + _projectLessKey, "#sigma_{" + _projectLessKey + "}^{scale}", 1.1, 0.5, 2));
                } else {
                    AddShapeParameter(new RooRealVar("s_scale_" + _bremKey, "#sigma_{" + _bremKey + "}^{scale}", 1.1, 0.5, 2));
                }
            }

            if (_option.Contains("modbremfrac") && (_configuration.GetAna() == Analysis::EE)) {
                AddShapeParameter(new RooRealVar("g_scale_" + _configKey + "-0G", "g_{" + _configKey + ",0G}^{scale}", 1, 0.5, 2.0));
                AddShapeParameter(new RooRealVar("g_scale_" + _configKey + "-1G", "g_{" + _configKey + ",1G}^{scale}", 1, 0.5, 2.0));
            }
        }

        if (_configuration.GetAna() == Analysis::EE && _option.Contains("modbremgaussfrac")) {
            // NOTE fracGauss in Brem1/2 (RHS - ONLY, depends on StringToFit setup, be careful!!!) 
            // Scale factor applied to accomodate RHS bumps in signal EE shapes (only when Brem-splitted fits) 
            // NB : we must be sure that the fg refers to the gaussian we place on the RHS of signal, this is dependent on the fit setup.
            double minValScalingBrems =   0.;
            // double maxValScalibgBrems = SettingDef::Fit::useRooRealSumPDF ?  10.0 :  10.0;
            double maxValScalibgBrems =   10.0;
            //3 probably is better!
            if( _configuration.HasBrem()){
                //IF the fitter split by brem
                // vector< Brem > _brems = {Brem::G1, Brem::G2};
                vector< Brem > _brems = {Brem::G1, Brem::G2};
                for (auto _brem : _brems){
                    auto _bremKey = _configuration.GetKeyWithBrem(_brem);                    
                    AddShapeParameter(new RooRealVar("fgBrem_scale_" + _bremKey, "fRHS_{" + _bremKey + "}^{scale}", 1.0, minValScalingBrems,maxValScalibgBrems));                    
                }
            }else{
                //IF the fitter doesn't split by brem
                AddShapeParameter(new RooRealVar("fgBrem_scale_" + _configKey, "fRHS_{" + _configKey + "}^{scale}", 1.0, minValScalingBrems,maxValScalibgBrems));
            }
        }
        if (_configuration.HasSample(Sample::Bs)){
            AddShapeParameter(new RooRealVar("m_offset", "m_{" + _configKey + "}^{offset}", 0, -1000, 1000)); 
        }

        if (_option.Contains("modyieldsig")) {
            if (_configuration.GetAna() == Analysis::EE) {
                // Only need to do this for one ana, since it's a ratio between EE and MM.
                auto _signalConfig   = FitParameterConfig::GetSignalConfig(_configuration);
                auto _MMSignalConfig = _signalConfig.ReplaceConfig(Analysis::MM);
                auto _keyRatio       = _signalConfig.ReplaceConfig(Analysis::All);
                //------ (deal with systematics in eff-ratio to gauss constraint around 1)
                auto  _keyRatioNOSHARE     = FitParameterConfig::GetRatioConfigSyst(_configuration);;
                auto  _ratioTypeStringSyst = FitParameterConfig::GetLeadingR(_option +"-eratioSyst", _keyRatioNOSHARE.GetQ2bin());
                //------ (deal with systematics in eff-ratio to gauss constraint around 1)
                if (_option.Contains("modyieldsignotrg")) _keyRatio = _keyRatio.ReplaceConfig(Trigger::All);
                if (_option.Contains("modyieldsignoyr"))  _keyRatio = _keyRatio.ReplaceConfig(Year::All);
                auto  _ratioTypeString     = FitParameterConfig::GetLeadingR(_option, _keyRatio.GetQ2bin());
                double _initialVal = 1.;
                double _upperBound = 5.;
                if( (_keyRatio.GetQ2bin() == Q2Bin::Low || _keyRatio.GetQ2bin() == Q2Bin::Central || _keyRatio.GetQ2bin() == Q2Bin::High) && SettingDef::Fit::blindRatio ){
                    /*
                        If it's a rare mode and blindRatio is enabled, The initialValue of the R-Ratio parameter (still to be blinded with a single scale parameter is
                        randomly shooted between [0,50], and the upperLimit of the parameter is also randomized with 50 * alpha with alpha in [1,2]
                        This way it becomes impossible to infer the blinding scale applied afterwards and see in the log file the initial parameter to infer the blinding scale value applied
                        All this is required since blinding is done with a simple random scale, thus if you would know in advance the initial Parameter, and the final fit print 
                        the initialValue, you can compute the blinding factors applied. 
                        Such trick of moving around initialValues + not printing them + moving randomly the upperBound, makes impossible to compute by hand the blinding factors.
                        However the Seeds configured by the RandomGenerator are unique for the parameters of interest, since they are from the hashed Fit Name parameter
                    */
                    MessageSvc::Warning("ParameterPool::ConfigureParameters, modyieldsig", _keyRatio.GetRatioName(_ratioTypeString) , "Blinding initialValues [0,50] Uniform from Hashed string");
                    TString _strForSeed = _keyRatio.GetRatioName(_ratioTypeString);
                    UnblindParameter::m_randomNumberGenerator.SetSeed( _strForSeed.Hash() );
                    //TODO: maybe tweak this!
                    // _initialVal = UnblindParameter::m_randomNumberGenerator.Uniform(0,_upperBound);
                    // _initialVal   = UnblindParameter::m_randomNumberGenerator.Uniform(0.4,3);
                    _initialVal   = UnblindParameter::m_randomNumberGenerator.Uniform(0.5,1.5);
                    _upperBound   = _upperBound *  UnblindParameter::m_randomNumberGenerator.Uniform(1,2);
                }else{
                    MessageSvc::Warning("ParameterPool::ConfigureParameters, modyieldsig", _keyRatio.GetRatioName(_ratioTypeString) , "InitialValue is 1.");
                }
                if( _initialVal < 0 || _initialVal  > _upperBound){
                    MessageSvc::Error("Cannot configure the R-Ratio, bounds-central values are wrong, FIX IT");
                }
                auto       _ratioVar       = new RooRealVar(_keyRatio.GetRatioName(_ratioTypeString), _keyRatio.GetRatioLabel(_option), _initialVal , 0, _upperBound);
                RatioType _ratioType       = (_keyRatio.GetQ2bin() == Q2Bin::JPsi) ? RatioType::SingleRatio : RatioType::DoubleRatio;
                _ratioVar->setError( 0.2 * _initialVal);
                _neededEfficiencies.push_back(_signalConfig);
                _neededEfficiencies.push_back(_MMSignalConfig);
                AddRatioParameter(_keyRatio, _ratioType, _ratioVar);
                //---- inject systematics parameters in the parameter pool
                if( _ratioType == RatioType::SingleRatio && SettingDef::Fit::rJPsiFitWithSystematics() && _keyRatioNOSHARE.GetQ2bin() == Q2Bin::JPsi ){
                    auto  _eRatioSyst    = new RooRealVar(_keyRatioNOSHARE.GetRatioName(_ratioTypeStringSyst), _keyRatioNOSHARE.GetRatioLabel(_ratioTypeStringSyst), 1*SettingDef::Efficiency::scaleSystematics , 0.7 *SettingDef::Efficiency::scaleSystematics, 1.3 * SettingDef::Efficiency::scaleSystematics );
                    AddRatioParameter( _keyRatioNOSHARE, RatioType::EfficiencyRatioSystematic,_eRatioSyst);
                }else if( _ratioType == RatioType::DoubleRatio && SettingDef::Fit::RPsiFitWithSystematics() && _keyRatioNOSHARE.GetQ2bin() == Q2Bin::Psi ){
                    auto  _eRatioSyst    = new RooRealVar(_keyRatioNOSHARE.GetRatioName(_ratioTypeStringSyst), _keyRatioNOSHARE.GetRatioLabel(_ratioTypeStringSyst), 1*SettingDef::Efficiency::scaleSystematics , 0.7 *SettingDef::Efficiency::scaleSystematics, 1.3 * SettingDef::Efficiency::scaleSystematics );
                    AddRatioParameter( _keyRatioNOSHARE, RatioType::EfficiencyRatioSystematic,_eRatioSyst);
                }else if( _ratioType == RatioType::DoubleRatio && SettingDef::Fit::RXFitWithSystematics() && ( _keyRatioNOSHARE.GetQ2bin() == Q2Bin::Low || _keyRatioNOSHARE.GetQ2bin() == Q2Bin::Central )){
                    auto  _eRatioSyst    = new RooRealVar(_keyRatioNOSHARE.GetRatioName(_ratioTypeStringSyst), _keyRatioNOSHARE.GetRatioLabel(_ratioTypeStringSyst), 1*SettingDef::Efficiency::scaleSystematics , 0.7 *SettingDef::Efficiency::scaleSystematics, 1.3 * SettingDef::Efficiency::scaleSystematics );
                    AddRatioParameter( _keyRatioNOSHARE, RatioType::EfficiencyRatioSystematic,_eRatioSyst);
                }
            }
        }

        if (_option.Contains("modyieldbkg")) {
            if (_configuration.HasSample(Sample::Lb)) {
                if (_option.Contains("modyieldbkglb") || _option.Contains("modyieldbkgeff")) {
                    auto _bkgConfig = FitParameterConfig::GetBackgroundConfig(_configuration, Sample::Lb);
                    _neededEfficiencies.push_back(_bkgConfig);
                    if (_option.Contains("modyieldbkgeff") && (_configuration.GetAna() == Analysis::EE)) {
                        auto _keyBkgMM = _bkgConfig.ReplaceConfig(Analysis::MM);
                        _neededEfficiencies.push_back(_keyBkgMM);
                    }
                }
            }
            if (_configuration.HasSample(Sample::Bs2Phi)) {
                if (_option.Contains("modyieldbkgbs2phi") || _option.Contains("modyieldbkgeff")) {
                    auto _bkgConfig = FitParameterConfig::GetBackgroundConfig(_configuration, Sample::Bs2Phi);
                    _neededEfficiencies.push_back(_bkgConfig);
                    if (_option.Contains("modyieldbkgeff") && (_configuration.GetAna() == Analysis::EE)) {
                        auto _keyBkgMM = _bkgConfig.ReplaceConfig(Analysis::MM);
                        _neededEfficiencies.push_back(_keyBkgMM);
                    }
                }
            }
            if (_configuration.HasSample(Sample::HadSwap)) {
                if (_option.Contains("modyieldbkghadswap") || _option.Contains("modyieldbkgeff")) {
                    auto _bkgConfig = FitParameterConfig::GetBackgroundConfig(_configuration, Sample::HadSwap);
                    _neededEfficiencies.push_back(_bkgConfig);
                    if (_option.Contains("modyieldbkgeff") && (_configuration.GetAna() == Analysis::EE)) {
                        auto _keyBkgMM = _bkgConfig.ReplaceConfig(Analysis::MM);
                        _neededEfficiencies.push_back(_keyBkgMM);
                    }
                }
            }
            if (_configuration.HasSample(Sample::DSLC)) {
                // if (_option.Contains("modyieldbkgdslc") || _option.Contains("modyieldbkgeff")) {
                if (_option.Contains("modyieldbkgdslc")){ //|| _option.Contains("modyieldbkgeff")) {
                    auto _bkgConfig = FitParameterConfig::GetBackgroundConfig(_configuration, Sample::DSLC);
                    _neededEfficiencies.push_back(_bkgConfig);
                    if (_option.Contains("modyieldbkgeff") && (_configuration.GetAna() == Analysis::EE)) {
                        auto _keyBkgMM = _bkgConfig.ReplaceConfig(Analysis::MM);
                        _neededEfficiencies.push_back(_keyBkgMM);
                    }
                }
            }
            
            if (_configuration.HasSample(Sample::MisID)) {
                if (_option.Contains("modyieldbkgmid") || _option.Contains("modyieldbkgeff")) {
                    auto _bkgConfig = FitParameterConfig::GetBackgroundConfig(_configuration, Sample::MisID);
                    _neededEfficiencies.push_back(_bkgConfig);
                    if (_option.Contains("modyieldbkgeff") && (_configuration.GetAna() == Analysis::EE)) {
                        auto _keyBkgMM = _bkgConfig.ReplaceConfig(Analysis::MM);
                        _neededEfficiencies.push_back(_keyBkgMM);
                    }
                }
            }
            if (_configuration.HasSample(Sample::Leakage)) {
                if( _option.Contains("modyieldbkgprleak")){
                    //Part Reco Leakage Psi for J/Psi/ PSI(TODO) q2!
                    auto _bkgConfig    = FitParameterConfig::GetBackgroundConfig(_configuration, Sample::Leakage);
                    MessageSvc::Info("ADDING LEAKAGE PART RECO EFFICIENCY NEEDED!");
                    _bkgConfig.SetForRatio(false);
                    _neededEfficiencies.push_back(_bkgConfig);
                }
                else if (_option.Contains("modyieldbkgeff")) {
                    //Part Reco Leakage for central q2 !! 
                    auto _bkgConfig    = FitParameterConfig::GetBackgroundConfig(_configuration, Sample::Leakage);
                    auto _leakConfig   = FitParameterConfig::GetSignalConfig(_configuration);                
                    if (_bkgConfig.GetQ2bin() == Q2Bin::Central) _leakConfig = _bkgConfig.ReplaceConfig(Q2Bin::JPsi).ReplaceConfig(GetSignalSample(Q2Bin::JPsi));
                    if (_bkgConfig.GetQ2bin() == Q2Bin::Psi)     _leakConfig = _bkgConfig.ReplaceConfig(Q2Bin::JPsi).ReplaceConfig(GetSignalSample(Q2Bin::JPsi));
                    if (_bkgConfig.GetQ2bin() == Q2Bin::High)    _leakConfig = _bkgConfig.ReplaceConfig(Q2Bin::Psi).ReplaceConfig(GetSignalSample(Q2Bin::Psi));                
                    /*
                        NEEDED EFFICIENCY FOR LEAKAGE MUST BE THE ONE WITH FULL CORRECTIONS IN.
                    */
                    _bkgConfig.SetForRatio(true);
                    _leakConfig.SetForRatio(true);
                    _neededEfficiencies.push_back(_bkgConfig);
                    _neededEfficiencies.push_back(_leakConfig);
                }
            }
            if (_configuration.HasSample(Sample::Psi2JPsX)) {
                if (_option.Contains("modyieldbkgpsi2jpsx") || _option.Contains("modyieldbkgPRpsi2jpsx")){
                    auto _bkgConfig = FitParameterConfig::GetBackgroundConfig(_configuration, Sample::Psi2JPsX);
                    _neededEfficiencies.push_back(_bkgConfig);
                }
            }
            if (_configuration.HasSample(Sample::Psi2JPsPiPi)) {
                //TODO : add protection to have this for RKst only in J/Psi q2 
                if (_option.Contains("modyieldbkgPRpsi2jpspipi") ) {
                    auto _bkgConfig = FitParameterConfig::GetBackgroundConfig(_configuration, Sample::Psi2JPsPiPi);
                    _neededEfficiencies.push_back(_bkgConfig);
                }
            }
            if (_configuration.HasSample(Sample::KEtaPrime)){
                if (_option.Contains("modyieldbkgketaprime")){
                    auto _bkgConfig       = FitParameterConfig::GetBackgroundConfig(_configuration, Sample::KEtaPrime);
                    auto _jpsiReference   = _bkgConfig.ReplaceConfig("Bu2KJPs" + to_string(_bkgConfig.GetAna())) //change Name sample
                                                      .ReplaceConfig(Q2Bin::JPsi).ReplaceConfig(GetSignalSample(Q2Bin::JPsi));
                    MessageSvc::Info("KEtaPrime adding needed efficiencies");
                    _bkgConfig.Print();
                    _jpsiReference.Print();
                    _bkgConfig.SetForRatio(true);
                    _jpsiReference.SetForRatio(true);
                    _neededEfficiencies.push_back(_bkgConfig);
                    _neededEfficiencies.push_back(_jpsiReference);
                }
            }
            // TODO : test 
            // 1) TRUEB - EEGAMMA  from MCDT ported 
            // 2) Compute efficiencies with smeared q2 
            // 3) Inject, test fit! 
            // IMPLEMENTATION SUGGESTED FROM VAVA, MH, CRISTOPH : 
            // LOAD EFFICIENCIES FOR KETAPRIME COMPUTED WITH PAS/TOT(MDCDT) having MCDT cutted in TRUEQ2 = ( B- K - GAMMA ) to define TRUE_Q2 
            // FLOAT FULLY B.R. RATIO OF B+ => K EtaPrime /  B+ => K J/Psi 
            // FIX EFFICIENCY RATIO epsFull[FIX]( B+=> K EtaPrime=> ( eeGamma) for EE in Q2Low ) / epsFull( B+ => K+ J/Psi ) [ ONE PARAMETER RATIO FIXED , not numerator/denominators]
            // ALLOW EVENTUALLY THE B.R> to come from some External Measurements ( ConstDef.hpp ==> current value is for "full q2" , we want in the q2 of interest, not mesured, some extrapolation to do later)
            // if (_configuration.HasSample(Sample::KEtaPrime) && 
            //     _configuration.GetAna() == Analysis::EE)  && 
            //     _option.Contains( "modyieldbkgketaprime")) 
            //{
            //     auto _bkgConfig  = FitParameterConfig::GetBackgroundConfig(_configuration, Sample::KEtaPrime);
            //     auto _jPsiSignal = _bkgConfig.ReplaceConfig("Bu2KJPsEE") //change Name sample
            //                                                .ReplaceConfig(Q2Bin::JPsi) //change Project to RKst
            //                                                .ReplaceConfig(GetSignalSample(Q2Bin::JPsi)); //Change      
            //     MessageSvc::Info("KEtaPrime Constraint to signal J/Psi adding requested efficiencies");
            //     _bkgConfig.Print();
            //     _sigRKstConf.Print();                
            //     _neededEfficiencies.push_back(_bkgConfig);
            //     _neededEfficiencies.push_back(_jPsiSignal);
            // }
        }  
        if (_option.Contains("gconstCombSS")) {
            if (_configuration.HasSample(Sample::Comb) && _configuration.GetAna() == Analysis::EE) {
                TString _keyComb = TString::Format("%s-%s-%s", to_string(_configuration.GetProject()).Data(), to_string(_configuration.GetAna()).Data(), to_string(_configuration.GetQ2bin()).Data());
                _keyComb.ReplaceAll(to_string(_configuration.GetTrigger()), "");
                _keyComb.ReplaceAll(to_string(_configuration.GetYear()), "");
                _keyComb = CleanString(_keyComb);
                TString _mParName = TString::Format("m_Comb-%s", _keyComb.Data());
                if (not(ShapeParameterExists(_mParName))) {
                    auto _expTurnOnMVar = new RooRealVar(_mParName, _mParName, 0);
                    AddShapeParameter(_expTurnOnMVar);
                }
                TString _sEParName = TString::Format("sE_Comb-%s", _keyComb.Data());
                if (not(ShapeParameterExists(_sEParName))) {
                    auto _expTurnOnsEVar = new RooRealVar(_sEParName, _sEParName, 0);
                    AddShapeParameter(_expTurnOnsEVar);
                }
            }
        }
        if (_option.Contains("crossfeed")) {
            if (_configuration.HasSample(Sample::Bd2Kst)) {
                auto _bkgConfig   = FitParameterConfig::GetBackgroundConfig(_configuration, Sample::Bd2Kst);
                auto _sigRKstConf = _bkgConfig.ReplaceConfig("Bd2Kst" + to_string(_bkgConfig.GetAna())) //change Name sample
                                              .ReplaceConfig(Prj::RKst) //change Project to RKst
                                              .ReplaceConfig(GetSignalSample(_bkgConfig.GetQ2bin())); //Change                 
                //TODO : use the fully corrected Bd2Kst efficeincy!
                MessageSvc::Info("Bd2Kst cross feed add BkgConfig, and SignalConfig for needed efficiencies");
                _bkgConfig.SetForRatio(true);
                _sigRKstConf.SetForRatio(true);
                _bkgConfig.Print();
                _sigRKstConf.Print();
                _neededEfficiencies.push_back(_bkgConfig);
                _neededEfficiencies.push_back(_sigRKstConf);
            }
            if (_configuration.HasSample(Sample::Bu2Kst)) {
                auto _bkgConfigBu2Kst = FitParameterConfig::GetBackgroundConfig(_configuration, Sample::Bu2Kst);
                TString _ratioTypeString = "fBu2Kst_over_Bd2Kst";
                TString _ratioLabel = TString::Format("fBu2Kst_{%s}", _bkgConfigBu2Kst.GetKey().Data());          
                auto       _ratioVar       = new RooRealVar(_bkgConfigBu2Kst.GetRatioName(_ratioTypeString),_ratioLabel, 1. , 0., 5.);
                RatioType _ratioType       = RatioType::YieldRatio;  
                AddRatioParameter(_bkgConfigBu2Kst, _ratioType, _ratioVar);            
            }
            if (_configuration.HasSample(Sample::BdBu)) {
                auto _bkgConfigBdBu2Kst = FitParameterConfig::GetBackgroundConfig(_configuration, Sample::BdBu);
                TString _ratioTypeString = "fBdBu_over_Bd2Kst";
                TString _ratioLabel = TString::Format("fBdBu_{%s}", _bkgConfigBdBu2Kst.GetKey().Data());          
                auto       _ratioVar       = new RooRealVar(_bkgConfigBdBu2Kst.GetRatioName(_ratioTypeString),_ratioLabel, 1. , 0., 5.);
                RatioType _ratioType       = RatioType::YieldRatio;  
                AddRatioParameter(_bkgConfigBdBu2Kst, _ratioType, _ratioVar);            
            }
        }
    }

    if (_option.Contains("modyieldbkg")) {
        bool _gaussConstrain = _option.Contains("gconst");

        FitParameterConfig _emptyConfig = FitParameterConfig(Prj::All, Analysis::All, to_string(Sample::Empty), Q2Bin::All, Year::All, Polarity::All, Trigger::All, Brem::All, Track::All, Sample::Empty);

        if (_option.Contains("modyieldbkgbs")) {
            AddHadronisationRatio(_emptyConfig.ReplaceConfig(Year::Run1), RatioType::FsOverFd, PDG::Const::fsOverfd7, PDG::Const::fsOverfd7_err, "fs_Over_fd-R1", _gaussConstrain);
            AddHadronisationRatio(_emptyConfig.ReplaceConfig(Year::Run2), RatioType::FsOverFd, PDG::Const::fsOverfd13, PDG::Const::fsOverfd13_err, "fs_Over_fd-R2", _gaussConstrain);

            AddBranchingRatio(_emptyConfig.ReplaceConfig(Prj::RKst).ReplaceConfig(Q2Bin::JPsi).ReplaceConfig(Sample::Bs), PDG::BF::Bs2KstJPs,     PDG::BF::Bd2KstJPs, PDG::BF::Bs2KstJPs_err, PDG::BF::Bd2KstJPs_err, "bfRatio_Bs2KstJPs-Bd2KstJPs", _gaussConstrain);
            //NB : the 1.5 is for the s-wave + p-wave contribution in CKM suppressed Bs -> Kst J/Psi , see https://arxiv.org/pdf/1509.00400.pdf 
            // 2./3. for the K+ pi- combination ( instead of K0 pi0).
            AddBranchingRatio(_emptyConfig.ReplaceConfig(Prj::RK).ReplaceConfig(Q2Bin::JPsi).ReplaceConfig(Sample::Bs),  1.75 * PDG::BF::Bs2KstJPs*PDG::BF::Kst2KPi, PDG::BF::Bu2KJPs, PDG::BF::Bs2KstJPs_err, PDG::BF::Bu2KJPs_err, "bfRatio_Bs2KstJPs-Bu2KJPs", _gaussConstrain);

            AddBranchingRatio(_emptyConfig.ReplaceConfig(Prj::RKst).ReplaceConfig(Q2Bin::Psi).ReplaceConfig(Sample::Bs), PDG::BF::Bs2KstPsi      , PDG::BF::Bd2KstPsi, PDG::BF::Bs2KstPsi_err, PDG::BF::Bd2KstPsi_err, "bfRatio_Bs2KstPsi-Bd2KstPsi", _gaussConstrain);
            //NB : the 1.5 is for the s-wave + p-wave contribution in CKM suppressed Bs -> Kst J/Psi see https://arxiv.org/pdf/1509.00400.pdf 
            // 2./3. for the K+ pi- combination ( instead of K0 pi0).
            AddBranchingRatio(_emptyConfig.ReplaceConfig(Prj::RK).ReplaceConfig(Q2Bin::Psi).ReplaceConfig(Sample::Bs),   1.75 * PDG::BF::Bs2KstPsi*PDG::BF::Kst2KPi, PDG::BF::Bu2KPsi  , PDG::BF::Bs2KstJPs_err, PDG::BF::Bu2KPsi_err, "bfRatio_Bs2KstPsi-Bu2KPsi", _gaussConstrain);

        }

        if (_option.Contains("modyieldbkgleak")) {
            AddBranchingRatio(_emptyConfig.ReplaceConfig(Prj::RKst).ReplaceConfig(Q2Bin::Psi).ReplaceConfig(Sample::Leakage), PDG::BF::Bd2KstPsi * PDG::BF::Psi2EE, PDG::BF::Bd2KstJPs * PDG::BF::JPs2EE, PDG::BF::Bd2KstPsi_err * PDG::BF::Psi2EE, PDG::BF::Bd2KstJPs_err * PDG::BF::JPs2EE, "bfRatio_Bd2KstPsi-Bd2KstJPs", _gaussConstrain);
            AddBranchingRatio(_emptyConfig.ReplaceConfig(Prj::RK).ReplaceConfig(Q2Bin::Psi).ReplaceConfig(Sample::Leakage), PDG::BF::Bu2KPsi * PDG::BF::Psi2EE, PDG::BF::Bu2KJPs * PDG::BF::JPs2EE, PDG::BF::Bu2KPsi_err * PDG::BF::Psi2EE, PDG::BF::Bu2KJPs_err * PDG::BF::JPs2EE, "bfRatio_Bu2KPsi-Bu2KJPs", _gaussConstrain);
        }

        if (_option.Contains("modyieldbkgbs2phi")) {
            AddHadronisationRatio(_emptyConfig.ReplaceConfig(Year::Run1), RatioType::FsOverFd, PDG::Const::fsOverfd7, PDG::Const::fsOverfd7_err, "fs_Over_fd-R1", _gaussConstrain);
            AddHadronisationRatio(_emptyConfig.ReplaceConfig(Year::Run2), RatioType::FsOverFd, PDG::Const::fsOverfd13, PDG::Const::fsOverfd13_err, "fs_Over_fd-R2", _gaussConstrain);

            AddBranchingRatio(_emptyConfig.ReplaceConfig(Prj::RKst).ReplaceConfig(Q2Bin::JPsi).ReplaceConfig(Sample::Bs2Phi), PDG::BF::Bs2PhiJPs * PDG::BF::Phi2KK, PDG::BF::Bd2KstJPs * PDG::BF::Kst2KPi, PDG::BF::Bs2PhiJPs_err * PDG::BF::Phi2KK, PDG::BF::Bd2KstJPs_err * PDG::BF::Kst2KPi, "bfRatio_Bs2PhiJPs-Bd2KstJPs", _gaussConstrain);
            AddBranchingRatio(_emptyConfig.ReplaceConfig(Prj::RKst).ReplaceConfig(Q2Bin::Psi).ReplaceConfig(Sample::Bs2Phi), PDG::BF::Bs2PhiPsi * PDG::BF::Phi2KK, PDG::BF::Bd2KstPsi * PDG::BF::Kst2KPi, PDG::BF::Bs2PhiPsi_err * PDG::BF::Phi2KK, PDG::BF::Bd2KstPsi_err * PDG::BF::Kst2KPi,  "bfRatio_Bs2PhiPsi-Bd2KstPsi",  _gaussConstrain);
        }

        if (_option.Contains("modyieldbkgpsi2jpsx")) {
            // Psi2S -> JPs X -> ee X versus Psi2S -> ee
            AddBranchingRatio(_emptyConfig.ReplaceConfig(Prj::RK).ReplaceConfig(Q2Bin::Psi).ReplaceConfig(Sample::Psi2JPsX),   PDG::BF::Psi2JPsX * PDG::BF::JPs2EE, 
                                                                                                                               PDG::BF::Psi2EE ,  
                                                                                                                               PDG::BF::Psi2JPsX_err * PDG::BF::JPs2EE, 
                                                                                                                               PDG::BF::Psi2EE_err,  "bfRatio_KPsi2S2JpsiX_KPsi2See", 
                                                                                                                               _gaussConstrain);
            AddBranchingRatio(_emptyConfig.ReplaceConfig(Prj::RKst).ReplaceConfig(Q2Bin::Psi).ReplaceConfig(Sample::Psi2JPsX), PDG::BF::Psi2JPsX * PDG::BF::JPs2EE, 
                                                                                                                               PDG::BF::Psi2EE ,  
                                                                                                                               PDG::BF::Psi2JPsX_err * PDG::BF::JPs2EE, 
                                                                                                                               PDG::BF::Psi2EE_err,  "bfRatio_KstPsi2S2JpsiX_KstPsi2See", _gaussConstrain);
        }
        
        if (_option.Contains("modyieldbkgPRpsi2jpsx")) {
            //Psi --> J/Psi anything b.r. used , it includes the chic intermediate decays in dec file, must be vetoed, linked to what TruthMatchingSvc does.
            AddBranchingRatio(_emptyConfig.ReplaceConfig(Prj::RK).ReplaceConfig(Q2Bin::JPsi).ReplaceConfig(Sample::Psi2JPsX),  PDG::BF::Bu2KPsi * PDG::BF::Psi2JPsX , 
                                                                                                                               PDG::BF::Bu2KJPs ,  
                                                                                                                               PDG::BF::Psi2JPsX * PDG::BF::Bu2KPsi * TMath::Sqrt( TMath::Sq(PDG::BF::Psi2JPsX_err/PDG::BF::Psi2JPsX) + TMath::Sq(PDG::BF::Bu2KPsi_err/PDG::BF::Bu2KPsi) ), 
                                                                                                                               PDG::BF::Bu2KJPs_err, 
                                                                                                                               "bfRatio_Psi2JPsX_Over_KJPs", _gaussConstrain);
            AddBranchingRatio(_emptyConfig.ReplaceConfig(Prj::RKst).ReplaceConfig(Q2Bin::JPsi).ReplaceConfig(Sample::Psi2JPsX),PDG::BF::Bd2KstPsi * PDG::BF::Psi2JPsX , 
                                                                                                                               PDG::BF::Bd2KstJPs ,
                                                                                                                               PDG::BF::Psi2JPsX * PDG::BF::Bd2KstPsi * TMath::Sqrt( TMath::Sq(PDG::BF::Psi2JPsX_err/PDG::BF::Psi2JPsX) + TMath::Sq(PDG::BF::Bd2KstPsi_err/PDG::BF::Bd2KstPsi) ), 
                                                                                                                               PDG::BF::Bd2KstJPs_err, 
                                                                                                                               "bfRatio_Psi2JPsX_Over_KstJPs", _gaussConstrain);
        }
        if (_option.Contains("modyieldbkgPRpsi2jpspipi")){
            // B+ => (Psi -> J/Psi pipi)K 
            //----------------------------
            // B0 => J/Psi K*0             
            AddBranchingRatio(_emptyConfig.ReplaceConfig(Prj::RKst).ReplaceConfig(Q2Bin::JPsi).ReplaceConfig(Sample::Psi2JPsPiPi),PDG::BF::Bu2KPsi * PDG::BF::Psi2JPsPiPi ,  //B+=> Psi [ -> J/Psi pi pi] K 
                                                                                                                                  PDG::BF::Bd2KstJPs * PDG::BF::Kst2KPi ,
                                                                                                                                  PDG::BF::Psi2JPsPiPi * PDG::BF::Bu2KPsi  * TMath::Sqrt( TMath::Sq(PDG::BF::Psi2JPsPiPi_err/PDG::BF::Psi2JPsPiPi) + TMath::Sq(PDG::BF::Bu2KPsi_err/PDG::BF::Bu2KPsi) ), 
                                                                                                                                  PDG::BF::Bd2KstJPs_err * PDG::BF::Kst2KPi, 
                                                                                                                                 "bfRatio_Bu2Psi2JPsPiPi_Over_Bd2KstJPs", _gaussConstrain);                                                                                                                                                         
        }
        if (_option.Contains("modyieldbkgprleak")) {
            // Psi2S -> JPs X -> ee X versus Psi2S -> ee
            //void FitParameterPool::AddBranchingRatio(const FitParameterConfig & _configKey, double _numerator, double _denominator, double _numeratorError, double _denominatorError, const TString _name, bool _constrain)
            //TODO : TO UPDATE FOR PSI + J/PSI sim Fit with DTF - noDTF ? Gather RPsi and rJPsi (nODTF) ? 
            AddBranchingRatio(_emptyConfig.ReplaceConfig(Prj::RK).ReplaceConfig(Q2Bin::JPsi).ReplaceConfig(Sample::Leakage),    PDG::BF::Bu2KPsi      * PDG::BF::Psi2EE, 
                                                                                                                                PDG::BF::Bu2KJPs      * PDG::BF::JPs2EE ,  
                                                                                                                                PDG::BF::Bu2KPsi      * PDG::BF::Psi2EE* TMath::Sqrt( TMath::Sq(PDG::BF::Bu2KPsi_err/PDG::BF::Bu2KPsi) + TMath::Sq(PDG::BF::Psi2EE_err/PDG::BF::Psi2EE) ), 
                                                                                                                                PDG::BF::Bu2KJPs      * PDG::BF::JPs2EE* TMath::Sqrt( TMath::Sq(PDG::BF::Bu2KJPs_err/PDG::BF::Bu2KJPs) + TMath::Sq(PDG::BF::JPs2EE_err/PDG::BF::JPs2EE) ), 
                                                                                                                                "bfRatio_KPsiEE_Over_KJPsEE",     
                                                                                                                                _gaussConstrain);
            AddBranchingRatio(_emptyConfig.ReplaceConfig(Prj::RKst).ReplaceConfig(Q2Bin::JPsi).ReplaceConfig(Sample::Leakage),  PDG::BF::Bd2KstPsi      * PDG::BF::Psi2EE, 
                                                                                                                                PDG::BF::Bd2KstJPs      * PDG::BF::JPs2EE ,  
                                                                                                                                PDG::BF::Bd2KstPsi      * PDG::BF::Psi2EE * TMath::Sqrt( TMath::Sq(PDG::BF::Bd2KstPsi_err/PDG::BF::Bd2KstPsi) + TMath::Sq(PDG::BF::Psi2EE_err/PDG::BF::Psi2EE) ), 
                                                                                                                                PDG::BF::Bd2KstJPs      * PDG::BF::JPs2EE * TMath::Sqrt( TMath::Sq(PDG::BF::Bd2KstJPs_err/PDG::BF::Bd2KstJPs) + TMath::Sq(PDG::BF::JPs2EE_err/PDG::BF::JPs2EE) ), 
                                                                                                                                "bfRatio_KstPsiEE_Over_KstJPsEE",     
                                                                                                                                _gaussConstrain);                                                                                                                                
        }

        if (_option.Contains("modyieldbkglb")) {
            AddHadronisationRatio(_emptyConfig, RatioType::FLbOverFd, PDG::Const::fLbOverfd, PDG::Const::fLbOverfd_err, "fLb_Over_fd", _gaussConstrain);

            AddBranchingRatio(_emptyConfig.ReplaceConfig(Prj::RK).ReplaceConfig(Q2Bin::JPsi).ReplaceConfig(Sample::Lb),  PDG::BF::Lb2pKJPs, PDG::BF::Bu2KJPs, PDG::BF::Lb2pKJPs_err, PDG::BF::Bu2KJPs_err , "bfRatio_Lb2pKJPs-Bu2KJPs", _gaussConstrain);
            AddBranchingRatio(_emptyConfig.ReplaceConfig(Prj::RK).ReplaceConfig(Q2Bin::Psi).ReplaceConfig(Sample::Lb),   PDG::BF::Lb2pKPsi, PDG::BF::Bu2KPsi, PDG::BF::Lb2pKPsi_err, PDG::BF::Bu2KPsi_err , "bfRatio_Lb2pKPsi-Bu2KPsi", _gaussConstrain);
            AddBranchingRatio(_emptyConfig.ReplaceConfig(Prj::RKst).ReplaceConfig(Q2Bin::JPsi).ReplaceConfig(Sample::Lb),PDG::BF::Lb2pKJPs, PDG::BF::Bd2KstJPs * PDG::BF::Kst2KPi, PDG::BF::Lb2pKJPs_err, PDG::BF::Bd2KstJPs_err * PDG::BF::Kst2KPi, "bfRatio_Lb2pKJPs-Bd2KstJPs", _gaussConstrain);
            AddBranchingRatio(_emptyConfig.ReplaceConfig(Prj::RKst).ReplaceConfig(Q2Bin::Psi).ReplaceConfig(Sample::Lb), PDG::BF::Lb2pKPsi, PDG::BF::Bd2KstPsi * PDG::BF::Kst2KPi, PDG::BF::Lb2pKPsi_err,  PDG::BF::Bd2KstPsi_err * PDG::BF::Kst2KPi, "bfRatio_Lb2pKPsi-Bd2KstPsi", _gaussConstrain);
        }

        if (_option.Contains("modyieldbkgdslc")) {
            //On RKst Bd2DNuKstNu
            //Kst -> K pi piece on signal removed as shared in the ratio and being 
            //PDG::BF::Bd2DNuKstNu from the D- => ( K*0 enu) without the specification of K*0->Kpi
            AddBranchingRatio(_emptyConfig.ReplaceConfig(Prj::RKst).ReplaceConfig(Q2Bin::JPsi).ReplaceConfig(Sample::DSLC), PDG::BF::Bd2DNuKstNu, PDG::BF::Bd2KstJPs * PDG::BF::JPs2EE, PDG::BF::Bd2DNuKstNu_err, PDG::BF::Bd2KstJPs_err * PDG::BF::JPs2EE, "bfRatio_Bd2DNuKstNu-Bd2KstJPs", _gaussConstrain);
            AddBranchingRatio(_emptyConfig.ReplaceConfig(Prj::RKst).ReplaceConfig(Q2Bin::Psi).ReplaceConfig(Sample::DSLC), PDG::BF::Bd2DNuKstNu, PDG::BF::Bd2KstPsi * PDG::BF::Psi2EE, PDG::BF::Bd2DNuKstNu_err, PDG::BF::Bd2KstPsi_err * PDG::BF::Psi2EE, "bfRatio_Bd2DNuKstNu-Bd2KstPsi", _gaussConstrain);
            //On RK Bu2DKNuNu
            AddBranchingRatio(_emptyConfig.ReplaceConfig(Prj::RK).ReplaceConfig(Q2Bin::JPsi).ReplaceConfig(Sample::DSLC), PDG::BF::Bu2DKNuNu, PDG::BF::Bu2KJPs * PDG::BF::JPs2EE, PDG::BF::Bu2DKNuNu_err, PDG::BF::Bu2KJPs_err * PDG::BF::JPs2EE, "bfRatio_Bu2DKNuNu-Bu2KJPs", _gaussConstrain);
            AddBranchingRatio(_emptyConfig.ReplaceConfig(Prj::RK).ReplaceConfig(Q2Bin::Psi).ReplaceConfig(Sample::DSLC), PDG::BF::Bu2DKNuNu, PDG::BF::Bu2KPsi * PDG::BF::Psi2EE, PDG::BF::Bu2DKNuNu_err, PDG::BF::Bu2KPsi_err * PDG::BF::Psi2EE, "bfRatio_Bu2DKNuNu-Bu2KPsi", _gaussConstrain);
        }

        if (_option.Contains("modyieldbkgmid")) {
            AddBranchingRatio(_emptyConfig.ReplaceConfig(Prj::RK).ReplaceConfig(Q2Bin::JPsi).ReplaceConfig(Sample::MisID), PDG::BF::Bu2PiJPs, PDG::BF::Bu2KJPs, PDG::BF::Bu2PiJPs_err, PDG::BF::Bu2KJPs_err, "bfRatio_Bu2PiJPs-Bu2KJPs", _gaussConstrain);
            AddBranchingRatio(_emptyConfig.ReplaceConfig(Prj::RK).ReplaceConfig(Q2Bin::Psi).ReplaceConfig(Sample::MisID), PDG::BF::Bu2PiPsi, PDG::BF::Bu2KPsi, PDG::BF::Bu2PiPsi_err, PDG::BF::Bu2KPsi_err, "bfRatio_Bu2PiPsi-Bu2KPsi", _gaussConstrain);
        }
        if (_option.Contains("modyieldbkgketaprime")) {
            auto _kEtaPrimeConfig = _emptyConfig.ReplaceConfig(Prj::RK).ReplaceConfig(Q2Bin::Low).ReplaceConfig(Sample::KEtaPrime);
            //
            AddBranchingRatio(_emptyConfig.ReplaceConfig(Prj::RK).ReplaceConfig(Q2Bin::Low).ReplaceConfig(Sample::KEtaPrime),                                         
                                                    PDG::BF::Bu2KEtaPrime * PDG::BF::EtaPrime_EEGamma * PDG::BF::EtaPrime_EEGamma_InLowQ2, 
                                            PDG::BF::Bu2KJPs * PDG::BF::JPs2EE, 
                                                    PDG::BF::Bu2KEtaPrime * PDG::BF::EtaPrime_EEGamma * PDG::BF::EtaPrime_EEGamma_InLowQ2 *
                                                    TMath::Sqrt(TMath::Sq( PDG::BF::Bu2KEtaPrime_err/PDG::BF::Bu2KEtaPrime) + 
                                                                TMath::Sq( PDG::BF::EtaPrime_EEGamma_err/PDG::BF::EtaPrime_EEGamma) +
                                                                TMath::Sq( PDG::BF::EtaPrime_EEGamma_InLowQ2_err/PDG::BF::EtaPrime_EEGamma_InLowQ2)),
                                            PDG::BF::Bu2KJPs * PDG::BF::JPs2EE * 
                                                    TMath::Sqrt(TMath::Sq( PDG::BF::Bu2KJPs_err/ PDG::BF::Bu2KJPs) +
                                                                TMath::Sq( PDG::BF::JPs2EE_err/PDG::BF::JPs2EE)),
                                          "bfRatio_Bu2KEtaPrime-Bu2KJPsEE", 
                                          _gaussConstrain); //ALWAYS GAUSS CONSTRAINT if gconst and keep it blinded
            //No need of doing this !                                           
            // m_ratioMap[{_kEtaPrimeConfig, RatioType::BranchingFraction}].Blind();
        }
    }

    for (auto & _par : m_shapeParameterMap) {
        if (not(_par.second.GetBaseParameter()->isDerived())) { 
            ((RooRealVar *) _par.second.GetBaseParameter())->setError(SettingDef::Fit::stepSizePar * TMath::Abs(((RooRealVar *) _par.second.GetBaseParameter())->getMax() - ((RooRealVar *) _par.second.GetBaseParameter())->getMin())); 
        }
    }

    MessageSvc::Info(Color::Cyan, "FitParameterPool", (TString)"ConfigureParameters, add needed efficiencies SIZE", to_string(_neededEfficiencies.size()));
    for (auto & _effKey : _neededEfficiencies) { 
        AddEfficiency(_effKey); 
    }

    PrintParameters();  

    /* DEBUGGING, stop here*/
    // MessageSvc::Error("STOPPING HERE", "","EXIT_FAILURE");
    return;
}

void FitParameterPool::ModShiftBremDifference(TString _bremKey) {
    MessageSvc::Info(Color::Cyan, "FitParameterPool", (TString) "ModShiftBremDifference", _bremKey);
    auto _0BremKey = _bremKey;
    _0BremKey.ReplaceAll("1G", "0G").ReplaceAll("2G", "0G");
    RooRealVar * _shiftDifference      = new RooRealVar("m_shift_difference" + _bremKey, "m_{" + _bremKey + "}^{difference}", 0, -50, 50);
    RooRealVar * _shiftDifferenceGauss = new RooRealVar("mg_shift_difference" + _bremKey, "m_{" + _bremKey + "}^{difference}", 0, -50, 50);
    auto *       _0BremShift           = GetShapeParameter("m_shift_" + _0BremKey);
    auto *       _0BremShiftGauss      = GetShapeParameter("mg_shift_" + _0BremKey);
    AddShapeParameter(_shiftDifference);
    AddShapeParameter(_shiftDifferenceGauss);
    AddShapeParameter(new RooFormulaVar("m_shift_" + _bremKey, "@0 - @1", RooArgList(*_0BremShift, *_shiftDifference)));
    AddShapeParameter(new RooFormulaVar("mg_shift_" + _bremKey, "@0 - @1", RooArgList(*_0BremShiftGauss, *_shiftDifferenceGauss)));
}

RooAbsReal * FitParameterPool::GetEfficiency(const FitParameterConfig & _baseEffKey, bool _forRatio) {
    FitParameterConfig _effKey(_baseEffKey);
    if( _forRatio){
        _effKey.SetForRatio(true);
    }
    AddEfficiency(_effKey);
    PrintParametersInMap(m_efficiencyMap);
    return m_efficiencyMap[_effKey].GetUnblindedParameter();
}

void FitParameterPool::ThrowIfEfficiencyDoesNotExist(const FitParameterConfig & _effKey, TString _callerName) const {
    if (not(EfficiencyExists(_effKey))){ 
        MessageSvc::Error("FitParameterPool", _callerName, _effKey.GetKey(), (TString) "Does not exist", "EXIT_FAILURE"); 
    }else{
        MessageSvc::Debug("FitParameterPool", _callerName, _effKey.GetKey(), (TString) "exist"); 
    }
}

bool FitParameterPool::EfficiencyExists(const FitParameterConfig & _effKey) const {
    bool _efficiencyExists = m_efficiencyMap.find(_effKey) != m_efficiencyMap.end();
    return _efficiencyExists;
}

void FitParameterPool::AddEfficiency(const FitParameterConfig & _effKey) {
    MessageSvc::Info(Color::Cyan, "FitParameterPool::AddEfficiency", TString::Format("Key %s , forRatio %s", _effKey.GetKey().Data(),_effKey.ForRatio() ? "True": "False") );
    TString _MODE = _effKey.ForRatio()? "RRATIO" :  "BKGOVERSIGNAL";                       
    bool _printIt = true; 
    if (not(EfficiencyExists(_effKey))){
        auto _cHolder = _effKey.GetConfigHolder();
        auto _efficiency    = LoadEfficiencyForFit(_cHolder, _MODE );     
        TString _effHeadString = "eff_" + to_string(_effKey.GetComponentSample()) + "-";
        TString _effName       = TString(_efficiency->GetName()).ReplaceAll("eff_", _effHeadString);
        _efficiency->SetName(_effName);
        m_efficiencyMap[_effKey] = ParameterWrapper(_efficiency);
        MessageSvc::Debug("AddEfficiency", _effName); 
        std::cout<< _effKey << std::endl;
        if( SettingDef::Fit::option.Contains("gconsteffsBkg") &&  (_effKey.ForRatio() == false || _effKey.IsLeakageComponent() || _effKey.IsKEtaPrimeComponent() || _effKey.IsCrossFeedComponent() )){
            //DECISION IF FLOAT OR CONSTANT
            MessageSvc::Debug("AddEfficiency (float)"); 
            _efficiency->setConstant(0);
            if( _effKey.ForRatio()== false && _effKey.IsSignalComponent()){
                MessageSvc::Warning("Efficiency signal for background constraints is set to constant");
                _efficiency->setConstant(1);
                if( _effKey.GetQ2bin() != Q2Bin::JPsi && _effKey.GetQ2bin() != Q2Bin::Psi){
                    //Rare mode efficiency is used also at denominator for cross-feed constraint!
                    _efficiency->setConstant(0);
                }
            }
            if( _effKey.GetComponentSample() == Sample::HadSwap){ 
                //had swaps make them constant in the constraint! 
                MessageSvc::Warning("Efficiency for component HadSwap constraints is set to constant");
                _efficiency->setConstant(1);
            }
            if( _effKey.GetComponentSample() == Sample::Lb){ 
                //had swaps make them constant in the constraint! 
                MessageSvc::Warning("Efficiency for component Lb constraints is set to constant");
                _efficiency->setConstant(1);
            }
            if( _effKey.GetComponentSample() == Sample::Bs2Phi){ 
                MessageSvc::Warning("Efficiency for component Bs2Phi constraints is set to constant");
                _efficiency->setConstant(1);
            }            
            if( _effKey.GetComponentSample() == Sample::Bs && _effKey.GetQ2bin() == Q2Bin::JPsi && _effKey.GetProject() == Prj::RK){
                MessageSvc::Warning("Efficiency for component Bs(J/Psi,RK) constraints is set to constant");
                _efficiency->setConstant(1);
            }        
            if( _effKey.GetComponentSample() == Sample::KEtaPrime && _effKey.GetQ2bin() == Q2Bin::Low && _effKey.GetProject() == Prj::RK){
                MessageSvc::Warning("Efficiency for component KEtaPrime(Low,RK) constraints is set to floating");
                _efficiency->setConstant(0);
            }
            if( _effKey.GetComponentSample() == Sample::Bd2Kst && ( _effKey.GetQ2bin() == Q2Bin::Low  || _effKey.GetQ2bin() == Q2Bin::Central) && _effKey.GetProject() == Prj::RK){
                MessageSvc::Warning("Efficiency for component Kst in RK constraints is set to floating");
                _efficiency->setConstant(0);
            }           
	    //if( _effKey.GetComponentSample() == Sample::MisID && (_effKey.GetQ2bin() == Q2Bin::JPs && _effKey.GetProject() == Prj::RK)){ 
	       // MessageSvc::Warning("Efficiency for component MisIID in RK J/Psi q2 is set to constant");
	       //_efficiency->setConstant(1);
	    //}
        }else{
            MessageSvc::Warning("Efficiency for component is set to constant");
            _efficiency->setConstant(1);
        }
        
        if(_effKey.IsSignalComponent() && _effKey.ForRatio() ){
            if (SettingDef::Fit::option.Contains("gconsteffsFull") ){
                MessageSvc::Warning("Efficiency for SignalComponent, forRatioTrue is set to float");
                _efficiency->setConstant(0);
            }else{
                MessageSvc::Warning("Efficiency for SignalComponent, forRatioTrue is set to constant");
                _efficiency->setConstant(1);
            }
        }
        //!!!!!!!!!!!!!!!!! BLIND EFFICIENCIES ON SIGNAL MODE ONLY HERE !!!!!!!!!!!!!!!
        if (SettingDef::Fit::blindEfficiency) {             
            if (ShouldBlind(_effKey)){
                _printIt = false; 
                m_efficiencyMap[_effKey].Blind(); 
            }
        }
        // AddConstrainedParameter(_efficiency);
        // Efficiencies are treated differently due to their correlation
        if( _printIt){
            MessageSvc::Info("AddEfficiency", _efficiency);
        }else{
            MessageSvc::Info("AddEfficiency (blinded)", TString(_efficiency->GetName() ) );
        }
    }else{
        MessageSvc::Info(Color::Cyan, "FitParameterPool", TString::Format("AddEfficiency (%s) with key (%s) already in map, SKIP", _MODE.Data(), _effKey.GetKey().Data() ));
    }
    return;
}

// Blind rare mode efficiencies for signal
bool FitParameterPool::ShouldBlind(const FitParameterConfig & _key) const {
    auto _q2bin   = _key.GetQ2bin();
    bool _isRare  = (_q2bin == Q2Bin::Low  || _q2bin == Q2Bin::Central || _q2bin == Q2Bin::High);
    bool _isSignal= _key.IsSignalComponent() || _key.IsCrossFeedComponent() || _key.IsKEtaPrimeComponent();
    if (_isRare && _isSignal){
        return true;
    } else {
        return false;
    }
}

RooAbsReal * FitParameterPool::GetYield(const FitParameterConfig & _yieldKey) {
    if (YieldExists(_yieldKey)) {
        cout<<RED<< "Yield exists, GetUnblindedParameter() "<< RESET<<endl;
        return m_yieldMap[_yieldKey].GetUnblindedParameter();
    } else if (_yieldKey.GetAna() == Analysis::EE) {   // Get a combined brem yield
        auto _key0G = _yieldKey.ReplaceConfig(Brem::G0);
        auto _key1G = _yieldKey.ReplaceConfig(Brem::G1);
        auto _key2G = _yieldKey.ReplaceConfig(Brem::G2);
        ThrowIfYieldDoesNotExist(_key0G, "GetYield");
        ThrowIfYieldDoesNotExist(_key1G, "GetYield");
        ThrowIfYieldDoesNotExist(_key2G, "GetYield");
        auto            _yield0G   = m_yieldMap[_key0G].GetUnblindedParameter();
        auto            _yield1G   = m_yieldMap[_key1G].GetUnblindedParameter();
        auto            _yield2G   = m_yieldMap[_key2G].GetUnblindedParameter();
        auto            _yieldName = _yieldKey.GetYieldString();
        RooArgSet       _args(*_yield0G, *_yield1G, *_yield2G);
        RooFormulaVar * _yieldCombined = new RooFormulaVar(_yieldName, "@0 + @1 + @2", _args);
        AddYieldParameter(_yieldKey, _yieldCombined);
        return m_yieldMap[_yieldKey].GetUnblindedParameter();
    } else {        
        cout<<"ASKED"<<endl;
        cout<<_yieldKey<<endl;
        cout<<"In the Map"<<endl;
        bool _found = m_yieldMap.find(_yieldKey) != m_yieldMap.end() ; 
        cout<< "YieldExist ? "<< _found << endl;
        for( auto & el : m_yieldMap){
            auto _var = el.second.GetBaseParameter();
            cout<< "*)" << endl;
            cout<< el.first << endl;
            cout<< _var->GetName() << endl;
            bool equal = _yieldKey == el.first; 
            cout<< "Equalit to asked ? "<< equal << endl;
            break;
        }        
        // PrintParametersInMap(m_yieldMap);
        ThrowIfYieldDoesNotExist(_yieldKey, "GetYield");   // Throw an error if the yield is not EE or does not exists
        return m_yieldMap[_yieldKey].GetUnblindedParameter();
    }
}

void FitParameterPool::ThrowIfYieldDoesNotExist(const FitParameterConfig & _yieldKey, TString _callerName) const {
    if (not(YieldExists(_yieldKey))) { MessageSvc::Error("FitParameterPool", _callerName, _yieldKey.GetKey(), (TString) "Does not exist", "EXIT_FAILURE"); }
}

bool FitParameterPool::YieldExists(const FitParameterConfig & _yieldKey) const {
    bool _yieldExist = m_yieldMap.find(_yieldKey) != m_yieldMap.end();
    return _yieldExist;
}

RooAbsReal * FitParameterPool::GetShapeParameter(TString _shapeParameterName) {
    ThrowIfShapeDoesNotExist(_shapeParameterName, "GetShapeParameter");
    return m_shapeParameterMap[_shapeParameterName].GetUnblindedParameter();
}

void FitParameterPool::ThrowIfShapeDoesNotExist(const TString & _shapeParameterName, TString _callerName) const {
    if (not(ShapeParameterExists(_shapeParameterName))) { 
        MessageSvc::Error("FitParameterPool", _callerName, _shapeParameterName, (TString) "Does not exist", "EXIT_FAILURE"); 
    }
    return; 
}

bool FitParameterPool::ShapeParameterExists(const TString & _shapeParameterName) const {
    bool _shapeParameterExists = m_shapeParameterMap.find(_shapeParameterName) != m_shapeParameterMap.end();
    return _shapeParameterExists;
}

void FitParameterPool::AddShapeParameter(RooAbsReal * _parameter) {
    MessageSvc::Info(Color::Cyan, "FitParameterPool", (TString) "AddShapeParameter with name", _parameter->GetName());
    TString _parameterName = TString(_parameter->GetName());
    if (not(ShapeParameterExists(_parameterName))) {
        // std::cout<<"parameter wrapper construction"<<std::endl;
        m_shapeParameterMap[_parameterName] = ParameterWrapper(_parameter);
        // std::cout<<"parameter wrapper printing"<<std::endl;
        MessageSvc::Info("AddShapeParameter", _parameter);
    } else {
        MessageSvc::Warning("FitParameterPool", "AddShapeParameter", TString(_parameter->GetName()), "Already exists!");
    }
    return;
}

void FitParameterPool::AddConstrainedParameter(RooRealVar * _parameter) {
    for( auto & v : m_constrainedParameters ){
        if( _parameter->GetName() == v->GetName() || _parameter->GetTitle() == v->GetTitle()){
            MessageSvc::Warning("FitParameterPool", (TString) "Adding a constraint with a pointer having a matching name to already existing parameter in the pool");
        }
    }
    //Is VectorOfPointer properly checked like this?
    if (not(CheckVectorContains(m_constrainedParameters, _parameter))) {
        MessageSvc::Info(TString::Format("AddConstrainedParameter [pointer][%p]", _parameter), _parameter);
        MessageSvc::Info(TString::Format("AddConstrainedParameter [pointer][%p]", _parameter), TString( Form("%i",m_constrainedParameters.size())));
        m_constrainedParameters.push_back(_parameter);
    } else {
        MessageSvc::Warning("FitParameterPool", (TString) "Double adding a constraint, pointer-matching. Skipping this add");
    }
    return;
}

void FitParameterPool::AddYieldParameter(const FitParameterConfig & _yieldKey, RooAbsReal * _parameter) {
    MessageSvc::Info(Color::Cyan, "FitParameterPool", (TString) "AddYieldParameter with key", _yieldKey.GetKey(), "and name", _parameter->GetName());
    if (not(YieldExists(_yieldKey))) {
        m_yieldMap[_yieldKey] = ParameterWrapper(_parameter);
    } else {
        MessageSvc::Warning("FitParameterPool", "AddYieldParameter", TString(_parameter->GetName()), "Already exists!");
    }
    if (SettingDef::Fit::blindYield) {
        if (ShouldBlind(_yieldKey)) m_yieldMap[_yieldKey].Blind();
    }
    MessageSvc::Info("AddYieldParameter", _parameter);
    return;
}

void FitParameterPool::ReplaceYieldParameter(const FitParameterConfig & _yieldKey, RooAbsReal * _newParameter) {
    ThrowIfYieldDoesNotExist(_yieldKey, "ReplaceYieldParameter");
    RemoveYieldParameter(_yieldKey);
    m_yieldMap[_yieldKey] = ParameterWrapper(_newParameter);
}

void FitParameterPool::ReplaceShapeParameter(TString _name, RooAbsReal * _newParameter) {
    ThrowIfShapeDoesNotExist(_name, "ReplaceShapeParameter");
    RemoveShapeParameter(_name);
    m_shapeParameterMap[_name] = ParameterWrapper(_newParameter);
}

bool FitParameterPool::RemoveEfficiency(const FitParameterConfig & _efficiencyKey) {
    ThrowIfEfficiencyDoesNotExist(_efficiencyKey, "RemoveEfficiency");
    MessageSvc::Warning("RemvoeEfficiency",  _efficiencyKey.GetKey());
    m_efficiencyMap[_efficiencyKey].DeleteParameter();
    m_efficiencyMap.erase(_efficiencyKey);
}

bool FitParameterPool::RemoveRatio(const FitParameterConfig & _ratioKey, const RatioType & _ratioType) {
    ThrowIfRatioDoesNotExist(_ratioKey, _ratioType, "RemoveRatio");
    m_ratioMap[{_ratioKey, _ratioType}].DeleteParameter();
    m_ratioMap.erase({_ratioKey, _ratioType});
}

void FitParameterPool::RemoveYieldParameter(const FitParameterConfig & _yieldKey) {
    ThrowIfYieldDoesNotExist(_yieldKey, "RemoveYieldParameter");
    m_yieldMap[_yieldKey].DeleteParameter();
    m_yieldMap.erase(_yieldKey);
}

void FitParameterPool::RemoveShapeParameter(TString _name) {
    ThrowIfShapeDoesNotExist(_name, "ReplaceShapeParameter");
    m_shapeParameterMap[_name].DeleteParameter();
    m_shapeParameterMap.erase(_name);
}

void FitParameterPool::PrintParameters() const {
    if (m_efficiencyMap.size() != 0) {
        MessageSvc::Line();
        MessageSvc::Info(Color::Cyan, "FitParameterPool", (TString) "PrintParameterPool", "Efficiencies");
        PrintParametersInMap(m_efficiencyMap);
    }else{
        MessageSvc::Info(Color::Cyan, "FitParameterPool", (TString) "PrintParameterPool", "Efficiencies Empty");
    }
    if (m_yieldMap.size() != 0) {
        MessageSvc::Line();
        MessageSvc::Info(Color::Cyan, "FitParameterPool", (TString) "PrintParameterPool", "Yields");
        PrintParametersInMap(m_yieldMap);
    }else{
        MessageSvc::Info(Color::Cyan, "FitParameterPool", (TString) "PrintParameterPool", "Yields Empty");
    }
    if (m_shapeParameterMap.size() != 0) {
        MessageSvc::Line();
        MessageSvc::Info(Color::Cyan, "FitParameterPool", (TString) "PrintParameterPool", "ShapeParameters");
        PrintParametersInMap(m_shapeParameterMap);
    }else{
        MessageSvc::Info(Color::Cyan, "FitParameterPool", (TString) "PrintParameterPool", "ShapeParameters Empty");
    }
    if (m_ratioMap.size() != 0) {
        MessageSvc::Line();
        MessageSvc::Info(Color::Cyan, "FitParameterPool", (TString) "PrintParameterPool", "Ratios");
        PrintParametersInMap(m_ratioMap);
    }else{
        MessageSvc::Info(Color::Cyan, "FitParameterPool", (TString) "PrintParameterPool", "Ratios Empty");
    }
    if( m_constrainedParameters.size() != 0){
        MessageSvc::Line();
        MessageSvc::Info(Color::Cyan, "FitParameterPool", (TString) "PrintParameterPool", "ConstrainedParameters");
        for(auto & var : m_constrainedParameters){
            var->Print();
        }
    }else{
        MessageSvc::Info(Color::Cyan, "FitParameterPool", (TString) "PrintParameterPool", "ConstrainedParameters Empty");
    }
    return;
}

template < typename T > void FitParameterPool::PrintParametersInMap(const map< T, ParameterWrapper > & _parameterMap) const {
    MessageSvc::Line();
    cout<<"PrintParameters in Map (size) = "<< _parameterMap.size() << endl;
    for (const auto & _keyParameterPair : _parameterMap) {
        const auto    _variable = _keyParameterPair.second.GetBaseParameter();
        if( _variable != nullptr){
            TString _name     = TString(_variable->GetName());
            if( _name.Contains("blind" )){
                MessageSvc::Info(_name, (TString)"(hidden initialValue forced)");
                // MessageSvc::Info(_name, _variable); //TODO remove when unblinding , print actual values
            }else if(_name.Contains("shift") || _name.Contains("scale")){
                MessageSvc::Info(_name, (RooRealVar *) _variable);
            }else{
                MessageSvc::Info(_name, _variable);
            }
        }else{        
            cout<<"Problem **** "<< " Variable is nullptr"<< endl;
        }
    }
    MessageSvc::Line();
}

int FitParameterPool::GetEfficienciesCount() const { return m_efficiencyMap.size(); }

int FitParameterPool::GetYieldsCount() const { return m_yieldMap.size(); }

int FitParameterPool::GetShapeParameterCount() const { return m_shapeParameterMap.size(); }

FitParameterPool::~FitParameterPool() { ClearParameters(); }

ostream & operator<<(ostream & os, const FitParameterPool & _fitParameterPool) {
    os << WHITE;
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "FitParameterPool");
    MessageSvc::Line(os);
    MessageSvc::Print((ostream &) os, "efficiencies", to_string(_fitParameterPool.GetEfficienciesCount()));
    MessageSvc::Print((ostream &) os, "yield parameters", to_string(_fitParameterPool.GetYieldsCount()));
    MessageSvc::Print((ostream &) os, "shape parameters", to_string(_fitParameterPool.GetShapeParameterCount()));
    MessageSvc::Line(os);
    os << RESET;
    return os;
}



void FitParameterPool::AddBackgroundYieldSignalRatio(const FitParameterConfig & _yieldKeyBkgEE, const FitParameterConfig & _yieldKeySigEE) {
    /*
        NB: no extra scale term here ! 
        Y(BKG-EE) = Y(BKG-MM)/Y(SIG-MM) * Y(SIG-EE)
    */
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitParameterPool", (TString) "AddBackgroundYieldSignalRatio with keys", _yieldKeyBkgEE.GetKey(), _yieldKeySigEE.GetKey());
    FitParameterConfig _yieldKeyBkgMM = _yieldKeyBkgEE.ReplaceConfig(Analysis::MM);
    FitParameterConfig _yieldKeySigMM = _yieldKeySigEE.ReplaceConfig(Analysis::MM);
    RooAbsReal * _yieldSigEE = GetYield(_yieldKeySigEE);
    RooAbsReal * _yieldBkgMM = GetYield(_yieldKeyBkgMM);
    RooAbsReal * _yieldSigMM = GetYield(_yieldKeySigMM);
    vector<RooAbsReal*> _numerators  =  {_yieldSigEE};
    vector<RooAbsReal*> _denominators = {_yieldSigMM};
    RemoveYieldParameter(_yieldKeyBkgEE);
    //Y(BkgEE) = Y(BkgMM) * YSig(EE)/YSig(MM)
    AddYieldFormula(_yieldKeyBkgEE, _yieldBkgMM, _numerators, _denominators);
    return;
}




void FitParameterPool::AddBackgroundYieldSignalRatio(const FitParameterConfig & _yieldKeyBkg, const FitParameterConfig & _yieldKeySig, const double _scale) {
    /*
        TODO : Seems to be NEVER used, if so we should drop
        Do Y(BKG) = Y(SIG) * scale(Factor-Constant); 
    */
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitParameterPool", (TString) "AddBackgroundYieldSignalRatio with keys", _yieldKeyBkg.GetKey(), _yieldKeySig.GetKey(), to_string(_scale));
    RooAbsReal * _yieldBkg = GetYield(_yieldKeyBkg);
    RooAbsReal * _yieldSig = GetYield(_yieldKeySig);
    RooRealVar * _scaleSig = new RooRealVar((TString) _yieldBkg->GetName() + "_" + _yieldSig->GetName() + "_scale", (TString) _yieldBkg->GetName() + "_" + _yieldSig->GetName() + "_scale", _scale);
    _scaleSig->setConstant(1);
    MessageSvc::Info("AddBackgroundYieldSignalRatio", _scale);
    RemoveYieldParameter(_yieldKeyBkg);
    AddYieldFormula(_yieldKeyBkg, _yieldSig, _scaleSig);
    return;
}
/* 
    TODO : Cleanup , REMOVE (should be useless so far)
void FitParameterPool::AddBackgroundYieldSignalConstrainedRatio(const FitParameterConfig & _yieldKeyBkg, const FitParameterConfig & _yieldKeySig, const double _ratio, const double _error, const TString & _ratioName, const double _scale) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitParameterPool", (TString) "AddBackgroundYieldSignalRatio with keys", _yieldKeyBkg.GetKey(), _yieldKeySig.GetKey(), to_string(_ratio) + " +/- " + to_string(_error));
    RooAbsReal * _yieldBkg = GetYield(_yieldKeyBkg);
    RooAbsReal * _yieldSig = GetYield(_yieldKeySig);
    RooRealVar * _ratioBkg = new RooRealVar(_ratioName, _ratioName, _ratio);
    _ratioBkg->setError(_error);
    _ratioBkg->setConstant(0);
    RatioType _ratioType = RatioType::YieldRatio;
    MessageSvc::Info("AddBackgroundYieldSignalRatio", _ratioBkg);
    RemoveYieldParameter(_yieldKeyBkg);
    AddRatioParameter(_yieldKeyBkg, _ratioType, _ratioBkg);
    AddConstrainedParameter(_ratioBkg);
    //Not sure
    AddYieldFormula(_yieldKeyBkg, _yieldSig, _ratioBkg, _scale);
    return;
}
*/

void FitParameterPool::AddBackgroundYieldEfficiencyRatio(const FitParameterConfig & _keyBkgEE) {
    /*
        Do Y(EE-BKG-X) = Y(MM-BKG-X) * eps( EE-BKG-X )/eps(MM-BKG-X)
    */
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitParameterPool", (TString) "AddBackgroundYieldEfficiencyRatio with key", _keyBkgEE.GetKey());
    FitParameterConfig _keyBkgMM  = _keyBkgEE.ReplaceConfig(Analysis::MM);
    RatioType          _ratioType = RatioType::EfficiencyRatio;

    RemoveYieldParameter(_keyBkgEE);
    RooAbsReal * _yieldBkgMM = GetYield(_keyBkgMM);
    vector<RooAbsReal*> _numerators   = {GetEfficiency( _keyBkgEE)};
    vector<RooAbsReal*> _denominators = {GetEfficiency( _keyBkgMM)};

    AddYieldFormula(_keyBkgEE, _yieldBkgMM, _numerators, _denominators);
    return;
}

void FitParameterPool::AddBackgroundYieldSignalEfficiencyRatio(const FitParameterConfig & _keyBkg, const FitParameterConfig & _keySignal, double _scale) {
    /*
        Do Y(BKG-ANA-X) = Y(SIG-ANA-X) * eps( BKG-ANA-X )/eps(SIG-ANA-X)
        In case of leakage the efficiencies used are the forRatio ones!
    */    
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitParameterPool", (TString) "AddBackgroundYieldSignalEfficiencyRatio with keys", _keyBkg.GetKey(), _keySignal.GetKey());
    auto      _yieldSignal = GetYield(_keySignal);
    auto      _yieldBkg    = GetYield(_keyBkg);
    bool     _forRatioBkg   = false; 
    bool     _forRatioSig   = false; 
    //If dealing with Leakage we want to keep the "fully-corrected stuff"
    if( _keyBkg.GetComponentSample() == Sample::Leakage && _keySignal.IsSignalComponent()){
        MessageSvc::Warning("LEAKAGE YIELD, USING FOR RATIO SETUP");        
        _forRatioBkg   = true; //
        _forRatioSig   = true; //
    }
    if( _keyBkg.GetComponentSample() == Sample::Bd2Kst){
        MessageSvc::Warning("Cross-Feed YIELD, USING FOR RATIO SETUP RKst, BKG SETUP Bd2Kst");
        //This is the cross-feed background, we want to have 
        //eff( full correction - RK ) [ forRatio = False, but Version and option with all correction steps with the Bp_setup ]
        //eff( signal- RKst ) the for Ratio = True,     with Version and option having the Bp_setup ] , to align at the YAML LEVEL!
        _forRatioBkg   = true; //RK-low/central BkgConfig has to be aligned to RKst low/central ones
        // _forRatioSig   = false;
        _forRatioSig   = true ;
        //on the cross feed we need a scale factor of 1/0.93% for the non factorization of efficiency ratios at generator level, almost the same in Run1 and Run2 (for low/central)
        _scale = 1./0.93;
    }
    if( _keyBkg.GetComponentSample() == Sample::KEtaPrime ){
        MessageSvc::Warning("KETAPRIME YIELD, USING FOR RATIO SETUP");        
        _forRatioBkg   = true; //RK-low/central BkgConfig has to be aligned to RKst low/central ones
        _forRatioSig   = true; 
        MessageSvc::Error("AddBackgroundYieldSignalEfficiencyRatio for KEtaPrime cannot be called, Branching ratio term needed", "","EXIT_FAILURE");
    }
    auto _epsBkg = GetEfficiency(    _keyBkg, _forRatioBkg );
    auto _epsSig = GetEfficiency( _keySignal, _forRatioSig );
    RemoveYieldParameter(_keyBkg);
    vector< RooAbsReal*> _numerators  = { _epsBkg};
    vector< RooAbsReal*> _denominators= { _epsSig};
    AddYieldFormula(_keyBkg, _yieldSignal, _numerators, _denominators, _scale);
    return;
}

void FitParameterPool::AddRatioParameter(const FitParameterConfig & _ratioKey, const RatioType & _ratioType, RooAbsReal * _ratioParameter) {
    MessageSvc::Info(Color::Cyan, "FitParameterPool", (TString) "AddRatioParameter with key", _ratioKey.GetKey(), "and name", _ratioParameter->GetName());
    MessageSvc::Info(Color::Cyan, "FitParameterPool", (TString) "AddRatioParameter with key", _ratioKey.GetKey(), "and type", to_string(_ratioType) );
    if (not(RatioExists(_ratioKey, _ratioType))) {        
        m_ratioMap[{_ratioKey, _ratioType}] = ParameterWrapper(_ratioParameter);
        if (SettingDef::Fit::blindRatio && !_ratioKey.IsCrossFeedComponent() && !_ratioKey.IsKEtaPrimeComponent() ) { 
            //!!!!! Cross-feed parameters fBdBu_Over... Are always UNBLINDED ! we computed them
            //Same story for KEtaPrime, we don't really want to blind it(?)
            //It leaks downn here the Bu2KEtaPrime Branching Ratio Ratio 
            if (ShouldBlind(_ratioKey)){
                m_ratioMap[{_ratioKey, _ratioType}].Blind();
                MessageSvc::Info("AddRatioParameter [Blind()]", (TString)_ratioParameter->GetName(), (TString)_ratioParameter->GetTitle());
            }else{
                MessageSvc::Info("AddRatioParameter", _ratioParameter);
            }
        }else{
            MessageSvc::Info("AddRatioParameter", _ratioParameter);
        }
    } else {
        MessageSvc::Warning("FitParameterPool", (TString) "AddRatioParameter", _ratioParameter->GetName(), (TString) "Already Exists, skip adding");
    }
    return;
}

RooAbsReal * FitParameterPool::GetRatioParameter(const FitParameterConfig & _ratioKey, const RatioType & _ratioType) {
    if(_ratioKey.IsSignalComponent() && _ratioType == RatioType::EfficiencyRatio){
        MessageSvc::Warning("GetRatioParameter for eps(signal) full correction ratio");
        auto CC = _ratioKey;
        CC.SetForRatio(true);
        ThrowIfRatioDoesNotExist(CC, _ratioType, "GetRatioParameter");
        return m_ratioMap[{CC, _ratioType}].GetUnblindedParameter();
    }
    ThrowIfRatioDoesNotExist(_ratioKey, _ratioType, "GetRatioParameter");
    return m_ratioMap[{_ratioKey, _ratioType}].GetUnblindedParameter();
}

vector <RooRealVar *> FitParameterPool::GetSingleRatios() const {
    vector <RooRealVar *> _singleRatios;
    for (auto & _keyRatioPair : m_ratioMap){
        bool _isSingleRatio = _keyRatioPair.first.second == RatioType::SingleRatio;
        if (_isSingleRatio){
            auto * _ratio = (RooRealVar*)_keyRatioPair.second.GetBaseParameter();
            _singleRatios.push_back(_ratio);
        }
    }
    return _singleRatios;
}

vector <RooRealVar *> FitParameterPool::GetDoubleRatios() const {
    vector <RooRealVar *> _doubleRatios;
    for (auto & _keyRatioPair : m_ratioMap){
        bool _isDoubleRatio = _keyRatioPair.first.second == RatioType::DoubleRatio;
        if (_isDoubleRatio){
            auto * _ratio = (RooRealVar*)_keyRatioPair.second.GetBaseParameter();
            _doubleRatios.push_back(_ratio);
        }
    }
    return _doubleRatios;
}

void FitParameterPool::ThrowIfRatioDoesNotExist(const FitParameterConfig & _ratioKey, const RatioType & _ratioType, TString _callerName) const {
    if (not(RatioExists(_ratioKey, _ratioType))) { MessageSvc::Error("FitParameterPool", _callerName, _ratioKey.GetKey(), (TString) "Ratio parameter not found!", "EXIT_FAILURE"); }
}

bool FitParameterPool::RatioExists(const FitParameterConfig & _ratioKey, const RatioType & _ratioType) const {
    bool _ratioExists = m_ratioMap.find({_ratioKey, _ratioType}) != m_ratioMap.end();
    return _ratioExists;
}

void FitParameterPool::AddSingleRatioYield(const FitParameterConfig & _keySigEE, TString _option) {
    FitParameterConfig _keySigMM = _keySigEE.ReplaceConfig(Analysis::MM);
    FitParameterConfig _keyRatio = _keySigEE.ReplaceConfig(Analysis::All);
    if (_option.Contains("modyieldsignotrg")) _keyRatio = _keyRatio.ReplaceConfig(Trigger::All);
    if (_option.Contains("modyieldsignoyr")) _keyRatio  = _keyRatio.ReplaceConfig(Year::All);
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitParameterPool", (TString) "AddSingleRatioYield with key", _keyRatio.GetKey());
    bool _forRatio = true;
    auto * _effEE       = GetEfficiency(_keySigEE, _forRatio);
    auto * _effMM       = GetEfficiency(_keySigMM, _forRatio);
    auto * _yieldMM     = (RooRealVar*)GetYield(_keySigMM);
    auto _singleRatio = GetRatioParameter(_keyRatio, RatioType::SingleRatio);
    MessageSvc::Warning("RemoveYieldParameter");
    RemoveYieldParameter(_keySigEE);
    vector < RooAbsReal * > _numerators = {_effEE};
    vector < RooAbsReal * > _denominators = {_effMM, _singleRatio};
    //NOTE : should be Y(EE) = eps(EE) / (epsMM * rRatio)
    if( SettingDef::Fit::rJPsiFit){
        MessageSvc::Warning("Plugging the single ratio systematics ( THIS MUST HAPPEN ONLY IF FITTING RJPSI!!! ) ");
        FitParameterConfig _keyRatioSyst = _keySigEE.ReplaceConfig( Analysis::All).ReplaceConfig("").ReplaceConfig( Sample::Empty);
        auto _syst_term = GetRatioParameter( _keyRatioSyst, RatioType::EfficiencyRatioSystematic);
        _numerators.push_back( _syst_term);    
    }
    if( SettingDef::Efficiency::scaleSystematics != 1.0){
        AddYieldFormula(_keySigEE, _yieldMM, _numerators, _denominators, 1./ SettingDef::Efficiency::scaleSystematics);
    }else{
        AddYieldFormula(_keySigEE, _yieldMM, _numerators, _denominators);
    }
    return;
}

void FitParameterPool::AddDoubleRatioYield(const FitParameterConfig & _keySigEE, TString _option) {
    FitParameterConfig _keySigMM = _keySigEE.ReplaceConfig(Analysis::MM);
    FitParameterConfig _keyRatio = _keySigEE.ReplaceConfig(Analysis::All);
    if (_option.Contains("modyieldsignotrg")) _keyRatio = _keyRatio.ReplaceConfig(Trigger::All);
    if (_option.Contains("modyieldsignoyr")) _keyRatio = _keyRatio.ReplaceConfig(Year::All);
    FitParameterConfig _keyRatioJPs = _keyRatio.ReplaceConfig(Q2Bin::JPsi).ReplaceConfig(Sample::JPsi);
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitParameterPool", (TString) "AddDoubleRatioYield with key", _keyRatio.GetKey());
    // AddFixedEfficiencyRatio(_keySigEE, _keySigMM); NO NEED OF THIS!
    bool _forRatio = true;
    auto * _effEE       = GetEfficiency(_keySigEE, _forRatio);
    auto * _effMM       = GetEfficiency(_keySigMM, _forRatio);
    auto * _yieldMM     = (RooRealVar*)GetYield(_keySigMM);
    auto * _ratioDouble = GetRatioParameter(_keyRatio, RatioType::DoubleRatio);
    auto * _ratioJPs    = GetRatioParameter(_keyRatioJPs, RatioType::SingleRatio);
    MessageSvc::Warning("RemoveYieldParameter");
    RemoveYieldParameter(_keySigEE);
    vector < RooAbsReal * > _numerators   = {_effEE};
    vector < RooAbsReal * > _denominators = {_effMM, _ratioDouble, _ratioJPs};
    MessageSvc::Warning("AddYieldFormula");
    if( SettingDef::Fit::RPsiFit || SettingDef::Fit::RXFit ){
        if(SettingDef::Fit::RPsiFit) MessageSvc::Warning("Plugging the double ratio systematics ( THIS MUST HAPPEN ONLY IF FITTING RPsi!!! ) ");
        if(SettingDef::Fit::RXFit)   MessageSvc::Warning("Plugging the double ratio systematics ( THIS MUST HAPPEN ONLY IF FITTING RX  !!! ) ");
        FitParameterConfig _keyRatioSyst = _keySigEE.ReplaceConfig( Analysis::All).ReplaceConfig("").ReplaceConfig( Sample::Empty);
        auto _syst_term = GetRatioParameter( _keyRatioSyst, RatioType::EfficiencyRatioSystematic);
        _numerators.push_back( _syst_term);
    }
    if( SettingDef::Efficiency::scaleSystematics != 1.0){
        AddYieldFormula(_keySigEE, _yieldMM, _numerators, _denominators, 1./SettingDef::Efficiency::scaleSystematics);
    }else{
        AddYieldFormula(_keySigEE, _yieldMM, _numerators, _denominators);
    }    
    return;
}

void FitParameterPool::AddYieldFormula(const FitParameterConfig & _yieldKey, RooAbsReal * _var1, RooAbsReal * _var2) {
    auto _nameVar1  = TString(_var1->GetName());
    auto _nameVar2  = TString(_var2->GetName());
    auto _yieldName = _yieldKey.GetYieldString();
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitParameterPool", (TString) "AddYieldFormula with key " + _yieldKey.GetKey() + " and name", _yieldName, "= @0*@1", _nameVar1, _nameVar2);
    ThrowIfYieldExists(_yieldKey, "AddYieldFormula");
    RooArgSet       _args(*_var1, *_var2);
    RooFormulaVar * _par = new RooFormulaVar(_yieldName, "@0*@1", _args);
    AddYieldParameter(_yieldKey, _par);
    return;
}

void FitParameterPool::AddYieldFormula(const FitParameterConfig & _yieldKey, RooAbsReal * _var1, RooAbsReal * _var2, const double _scale) {
    auto _nameVar1  = TString(_var1->GetName());
    auto _nameVar2  = TString(_var2->GetName());
    auto _yieldName = _yieldKey.GetYieldString();
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitParameterPool", (TString) "AddYieldFormula with key " + _yieldKey.GetKey() + " and name", _yieldName, "= @0*@1", _nameVar1, _nameVar2);
    ThrowIfYieldExists(_yieldKey, "AddYieldFormula");
    RooArgSet       _args(*_var1, *_var2);
    RooFormulaVar * _par = new RooFormulaVar(_yieldName, (TString) fmt::format("@0*@1*{0}", to_string(_scale)), _args);
    AddYieldParameter(_yieldKey, _par);
    return;
}

void FitParameterPool::AddYieldFormula(const FitParameterConfig & _yieldKey, RooAbsReal * _var1, RooAbsReal * _var2, RooAbsReal * _var3) {
    auto _nameVar1  = TString(_var1->GetName());
    auto _nameVar2  = TString(_var2->GetName());
    auto _nameVar3  = TString(_var3->GetName());
    auto _yieldName = _yieldKey.GetYieldString();
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitParameterPool", (TString) "AddYieldFormula with key " + _yieldKey.GetKey() + " and name", _yieldName, "= @0/@1*@2", _nameVar1, _nameVar2, _nameVar3);
    ThrowIfYieldExists(_yieldKey, "AddYieldFormula");
    RooArgSet       _args(*_var1, *_var2, *_var3);
    RooFormulaVar * _par = new RooFormulaVar(_yieldName, "@0/@1*@2", _args);
    AddYieldParameter(_yieldKey, _par);
    return;
}

void FitParameterPool::AddYieldFormula(const FitParameterConfig & _yieldKey, RooAbsReal * _var1, RooAbsReal * _var2, RooAbsReal * _var3, RooAbsReal * _var4) {
    auto _nameVar1  = TString(_var1->GetName());
    auto _nameVar2  = TString(_var2->GetName());
    auto _nameVar3  = TString(_var3->GetName());
    auto _nameVar4  = TString(_var4->GetName());
    auto _yieldName = _yieldKey.GetYieldString();
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitParameterPool", (TString) "AddYieldFormula with key " + _yieldKey.GetKey() + " and name", _yieldName, "= @0/@1*@2/@3", _nameVar1, _nameVar2, _nameVar3, _nameVar4);
    ThrowIfYieldExists(_yieldKey, "AddYieldFormula");
    RooArgSet       _args(*_var1, *_var2, *_var3, *_var4);
    RooFormulaVar * _par = new RooFormulaVar(_yieldName, "@0/@1*@2/@3", _args);
    AddYieldParameter(_yieldKey, _par);
    return;
}

void FitParameterPool::AddYieldFormula(const FitParameterConfig & _yieldKey, RooAbsReal * _var1, RooAbsReal * _var2, RooAbsReal * _var3, RooAbsReal * _var4, RooAbsReal * _var5) {
    auto _nameVar1  = TString(_var1->GetName());
    auto _nameVar2  = TString(_var2->GetName());
    auto _nameVar3  = TString(_var3->GetName());
    auto _nameVar4  = TString(_var4->GetName());
    auto _nameVar5  = TString(_var5->GetName());
    auto _yieldName = _yieldKey.GetYieldString();
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitParameterPool", (TString) "AddYieldFormula with key " + _yieldKey.GetKey() + " and name", _yieldName, "= @0/@1*@2/@3/@4", _nameVar1, _nameVar2, _nameVar3, _nameVar4, _nameVar5);
    ThrowIfYieldExists(_yieldKey, "AddYieldFormula");
    RooArgSet       _args(*_var1, *_var2, *_var3, *_var4, *_var5);
    RooFormulaVar * _par = new RooFormulaVar(_yieldName, "@0/@1*@2/@3/@4", _args);
    AddYieldParameter(_yieldKey, _par);
    MessageSvc::Info("AddYieldFormula", _par);
    return;
}

void FitParameterPool::AddYieldFormula(const FitParameterConfig & _yieldKey, RooAbsReal * _floatingYield, vector< RooAbsReal * > _numerators, vector< RooAbsReal * > _denominators, double _scale) {
    if (YieldExists(_yieldKey)) { RemoveYieldParameter(_yieldKey); }
    auto _yieldName = _yieldKey.GetYieldString();
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitParameterPool", (TString) "AddYieldFormula with key " + _yieldKey.GetKey());
    ThrowIfYieldExists(_yieldKey, "AddYieldFormula");
    auto            _formulaString = GetFormulaString(_numerators.size(), _denominators.size(), _scale);
    auto            _argList       = GetArgList(_floatingYield, _numerators, _denominators);
    RooFormulaVar * _par           = new RooFormulaVar(_yieldName, _formulaString, _argList);
    AddYieldParameter(_yieldKey, _par);
    MessageSvc::Info("AddYieldFormula", _par);
    return;
}

TString FitParameterPool::GetFormulaString(int _nNumerators, int _nDenominators, double _scale) {
    TString _formulaString  = (_scale == 1.) ? "@0" : fmt::format("{0}*@0", to_string(_scale));
    int     _parameterIndex = 1;
    for (int i = 0; i < _nNumerators; i++) {
        _formulaString = _formulaString + fmt::format("*@{0}", to_string(_parameterIndex));
        _parameterIndex++;
    }
    for (int i = 0; i < _nDenominators; i++) {
        _formulaString = _formulaString + fmt::format("/@{0}", to_string(_parameterIndex));
        _parameterIndex++;
    }
    return _formulaString;
}

RooArgList FitParameterPool::GetArgList(RooAbsReal * _floatingYield, vector< RooAbsReal * > _numerators, vector< RooAbsReal * > _denominators) {
    RooArgList _argList;
    _argList.add(*_floatingYield);
    for (auto _parameter : _numerators) { _argList.add(*_parameter); }
    for (auto _parameter : _denominators) { _argList.add(*_parameter); }
    return _argList;
}

void FitParameterPool::AddDoubleRatioYieldFormula(const FitParameterConfig & _yieldKey, RooAbsReal * _var1, RooAbsReal * _var2, RooAbsReal * _var3) {
    auto _nameVar1  = TString(_var1->GetName());
    auto _nameVar2  = TString(_var2->GetName());
    auto _nameVar3  = TString(_var3->GetName());
    auto _yieldName = _yieldKey.GetYieldString();
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitParameterPool", (TString) "AddDoubleRatioYieldFormula with key " + _yieldKey.GetKey() + " and name", _yieldName, "= @0*@1*@2", _nameVar1, _nameVar2, _nameVar3);
    ThrowIfYieldExists(_yieldKey, "AddDoubleRatioYieldFormula");
    RooArgSet       _args(*_var1, *_var2, *_var3);
    RooFormulaVar * _par = new RooFormulaVar(_yieldName, "@0*@1*@2", _args);
    AddYieldParameter(_yieldKey, _par);
    return;
}

void FitParameterPool::ThrowIfYieldExists(const FitParameterConfig & _yieldKey, TString _callerName) const {
    if (YieldExists(_yieldKey)) { MessageSvc::Error("FitParameterPool", _callerName, _yieldKey.GetYieldString(), (TString) "Already Added", "EXIT_FAILURE"); }
}

void FitParameterPool::AddFixedEfficiencyRatio(const FitParameterConfig & _numeratorKey, const FitParameterConfig & _denominatorKey, double _scale) {
    MessageSvc::Line();
    MessageSvc::Warning("FitParameterPool", (TString) "AddFixedEffiicencyRatio (OBSOLETE, Use Denominators,Numerators approach)");
    MessageSvc::Info(Color::Cyan, "FitParameterPool", (TString) "AddFixedEfficiencyRatio with keys", _numeratorKey.GetKey(), _denominatorKey.GetKey(), "and scale", to_string(_scale));
    bool _fullRatio = _numeratorKey.IsSignalComponent() && _denominatorKey.IsSignalComponent() ;     
    if(_fullRatio){
        MessageSvc::Info(Color::Cyan, "FitParameterPool", (TString) "AddFixedEfficiencyRatio with keys", _numeratorKey.GetKey(), _denominatorKey.GetKey(), "FullRatio active");    
    }
    // cout<<RED<<"BEFORE"<< RESET<< endl;
    // PrintParametersInMap(m_efficiencyMap);
    auto         _numeratorVar   = GetEfficiency(_numeratorKey,   _fullRatio);
    // PrintParametersInMap(m_efficiencyMap);
    auto         _denominatorVar = GetEfficiency(_denominatorKey, _fullRatio);
    // PrintParametersInMap(m_efficiencyMap);
    // if( _numeratorVar == nullptr ){
    //     MessageSvc::Error("Numerator Var is nullptr","","EXIT_FAILURE");
    // }else{
    //     cout<< "Numerator Var " << _numeratorVar->GetName() << endl;
    // }
    // if( _denominatorVar == nullptr ){
    //     cout<< "VAR denominator "<< endl;
    //     cout<< "KEY FitConfiguration" << _denominatorKey.GetKey()<< endl;
    //     MessageSvc::Error("Denominator Var is nullptr","","EXIT_FAILURE");
    // }
    double       _ratioValue     = _numeratorVar->getVal() / _denominatorVar->getVal();
    auto         _sample         = _numeratorKey.GetComponentSample();
    auto         _q2Bin          = _numeratorKey.GetQ2bin();
    auto         _project        = _numeratorKey.GetProject();

    TString name  = "effRatio_" + _numeratorKey.GetKey();
    TString title = "effRatio_" + _numeratorKey.GetKey();
    if( _fullRatio){
        name += "_FULL";
        title+= "_FULL";     
        auto CC  = _numeratorKey; 
        auto CCB = _denominatorKey;
        CC.SetForRatio(true);
        CCB.SetForRatio(true);
        RooArgList _epsRatioList( *GetEfficiency(CC), *GetEfficiency(CCB));
        RatioType    _ratioType      = RatioType::EfficiencyRatio;
        RooFormulaVar * _ratioEffVar = new RooFormulaVar( name, title, _epsRatioList);
        AddRatioParameter(CC, _ratioType, _ratioEffVar);
    }else{
        RooRealVar * _ratioVar       = new RooRealVar(name, title, _scale * _ratioValue);
        RatioType    _ratioType      = RatioType::EfficiencyRatio;
        _ratioVar->setConstant(1);
        AddRatioParameter(_numeratorKey, _ratioType, _ratioVar);
    }
}

void FitParameterPool::AddFixedEfficiency(const FitParameterConfig & _effKey) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, "FitParameterPool", (TString) "AddFixedEfficiency with key", _effKey.GetKey());
    auto    _floatingEfficiency   = GetEfficiency(_effKey);
    TString _newName              = "const_" + (TString) _floatingEfficiency->GetName();
    double  _value                = _floatingEfficiency->getVal();
    auto    _constantEfficiency   = new RooRealVar(_newName, _newName, _value);
    m_fixedEfficiencyMap[_effKey] = ParameterWrapper(_constantEfficiency);
    if (ShouldBlind(_effKey)) { m_fixedEfficiencyMap[_effKey].Blind(); }
}

RooAbsReal * FitParameterPool::GetFixedEfficiency(const FitParameterConfig & _effKey) { return m_fixedEfficiencyMap[_effKey].GetUnblindedParameter(); }

vector< RooRealVar * > FitParameterPool::GetEfficiencies() {
    vector< RooRealVar * > _efficiencies;
    for (auto & _keyEfficiencyPair : m_efficiencyMap) {
        auto _efficiency = (RooRealVar *) _keyEfficiencyPair.second.GetBaseParameter();
        _efficiencies.push_back(_efficiency);
    }
    return _efficiencies;
}

vector< RooAbsReal * > FitParameterPool::GetAllIndependentSignalYields() {
    vector< RooAbsReal * > _yields;
    for (auto & _keyYieldPair : m_yieldMap) {
        auto & _yieldKey = _keyYieldPair.first;
        auto   _yieldPar = _keyYieldPair.second.GetUnblindedParameter();
        if (_yieldKey.GetComponentSample() == GetSignalSample(_yieldKey.GetQ2bin())) {   // is signal yield
            if (not(_yieldPar->isDerived())) {                                           // Does not depend on other par, not a formula
                _yields.push_back(_yieldPar);
            }
        }
    }
    return _yields;
}

vector< RooAbsReal * > FitParameterPool::GetAllIndependentBackgroundYields() {
    vector< RooAbsReal * > _yields;
    for (auto & _keyYieldPair : m_yieldMap) {
        auto & _yieldKey = _keyYieldPair.first;
        auto   _yieldPar = _keyYieldPair.second.GetUnblindedParameter();
        if (_yieldKey.GetComponentSample() != GetSignalSample(_yieldKey.GetQ2bin())) {   // is signal yield
            if (not(_yieldPar->isDerived())) {                                           // Does not depend on other par, not a formula
                _yields.push_back(_yieldPar);
            }
        }
    }
    return _yields;
}

vector< RooAbsReal * > FitParameterPool::GetAllIndependentYields() {
    vector< RooAbsReal * > _sigYields = GetAllIndependentSignalYields();
    vector< RooAbsReal * > _bkgYields = GetAllIndependentBackgroundYields();
    vector< RooAbsReal * > _yields;
    for (auto _yield : _sigYields) _yields.push_back(_yield);
    for (auto _yield : _bkgYields) _yields.push_back(_yield);
    return _yields;
}

void FitParameterPool::AddHadronisationRatio(const FitParameterConfig & _configKey, const RatioType & _ratioType, double _value, double _error, const TString _name, bool _constrain) {    
    bool _isConstant         = not(_constrain);
    double min = 0.; 
    double max = 1.;
    if( _value < 0){ MessageSvc::Error("Invalid Hadronisation ratio , negative value not allowed, fix the code","","EXIT_FAILURE");}
    if( _value < min || _value > max){
        MessageSvc::Warning("Value exceed max, forcing max to 10* value , special case for fd/fs or fd/fLb");
        max = _value * 10.; 
    }
    auto _hadronisationRatio = new RooRealVar(_name, _name, _value, min, max);
    _hadronisationRatio->setError(_error);
    _hadronisationRatio->setConstant(_isConstant);
    auto _ratioConfig = GetConfigForRatio(_ratioType, _configKey);
    AddRatioParameter(_ratioConfig, _ratioType, _hadronisationRatio);
    if (_constrain) AddConstrainedParameter(_hadronisationRatio);
    return;
}

void FitParameterPool::AddBranchingRatio(const FitParameterConfig & _configKey, double _numerator, double _denominator, double _numeratorError, double _denominatorError, const TString _name, bool _constrain) {
    bool         _isConstant     = not(_constrain);
    if( _configKey.IsKEtaPrimeComponent()){
        _isConstant = false;
    }
    double       _value          = _numerator / _denominator;
    double       _error          = _value * TMath::Sqrt( TMath::Sq( _numeratorError/_numerator ) + TMath::Sq( _denominatorError/_denominator));
    if(false || _configKey.IsKEtaPrimeComponent()){
        MessageSvc::Debug("AddBranchingRatio", _name);
        std::cout<<RED<< "numerator   = "<< _numerator   << " +/- " << _numeratorError   <<  RESET<< std::endl;
        std::cout<<RED<< "denominator = "<< _denominator << " +/- " << _denominatorError <<  RESET<< std::endl;
        std::cout<<RED<< "value       = "<< _value  << " +/- " << _error << RESET<< std::endl;
        std::cout<<RED<< "Constant?   = "<< _isConstant  <<  RESET<< std::endl;
        std::cout<<RED<< "GConstr ?   = "<< _constrain  <<  RESET<< std::endl;
    }
    double min = 0; 
    double max = _value * 10.;
    RooRealVar * _branchingRatio = new RooRealVar(_name, _name, _value, min, max);
    if( _value < 0){ MessageSvc::Error("Invalid Branching ratio , negative value not allowed, fix the code","","EXIT_FAILURE");}
    _branchingRatio->setError(_error);    
    if( _constrain){
        min = _value - 10 * _error < 0. ? 0. : _value - 10 * _error;
        max = _value + 10 * _error;
    }else{
        min = 0; 
        max = 10 * _value;
    }
    _branchingRatio->setMin( min);
    _branchingRatio->setMax( max);
    _branchingRatio->setRange( min, max);

    _branchingRatio->setConstant(_isConstant);
    auto _ratioConfig = GetConfigForRatio(RatioType::BranchingFraction, _configKey);
    AddRatioParameter(_ratioConfig, RatioType::BranchingFraction, _branchingRatio);
    if (_constrain) AddConstrainedParameter(_branchingRatio);
    return;
}

void FitParameterPool::FillConstrainedEfficiencyContainers(const RooArgList & _listOfLikelihoods) {
    MessageSvc::Info("FitParameterPool", (TString) "FillConstrainedEfficiencyContainers");
    m_uncorrelatedEfficiencies.clear();
    m_correlatedEfficiencies.clear();
    vector< FitParameterConfig > _efficiencyKeysLinkedToLikelihood = GetEfficienciesInLikelihood(_listOfLikelihoods);
    vector< vector< bool > >     _adjacencyMatrix                  = GetEfficiencyAdjacencyMatrix(_efficiencyKeysLinkedToLikelihood);
    PrintAdjacencyMatrix(_adjacencyMatrix, _efficiencyKeysLinkedToLikelihood);
    m_uncorrelatedEfficiencies = ExtractUncorrelatedEfficiencies(_adjacencyMatrix, _efficiencyKeysLinkedToLikelihood);
    m_correlatedEfficiencies   = ExtractCorrelatedEfficiencies(_adjacencyMatrix, _efficiencyKeysLinkedToLikelihood);
}

vector< FitParameterConfig > FitParameterPool::GetEfficienciesInLikelihood(const RooArgList & _listOfLikelihoods) const {
    vector< RooRealVar * > _variablesInLikelihood = GetUniqueVariablesInLikelihood(_listOfLikelihoods);
    sort(_variablesInLikelihood.begin(), _variablesInLikelihood.end());
    // We will do a binary search so the variables need to be sorted
    vector< FitParameterConfig > _efficienciesInLikelihood = GetMatchedEfficiencies(_variablesInLikelihood);
    return move(_efficienciesInLikelihood);
}

vector< RooRealVar * > FitParameterPool::GetUniqueVariablesInLikelihood(const RooArgList & _listOfLikelihoods) const {
    vector< RooRealVar * > _uniqueVariables;
    for (const auto * _likelihood : _listOfLikelihoods) {
        for (auto * _variable : *(_likelihood->getVariables())) {
            bool _isUnique = not(CheckVectorContains(_uniqueVariables, (RooRealVar *) _variable));
            if (_isUnique) { _uniqueVariables.push_back((RooRealVar *) _variable); }
        }
    }
    return move(_uniqueVariables);
}

vector< FitParameterConfig > FitParameterPool::GetMatchedEfficiencies(const vector< RooRealVar * > & _variablesInLikelihood) const {
    vector< FitParameterConfig > _matchedEfficiencyKeys;
    for (const auto & _keyEfficiencyPair : m_efficiencyMap) {
        const auto &       _efficiencyKey     = _keyEfficiencyPair.first;
        const RooRealVar * _efficiencyPointer = (RooRealVar *) _keyEfficiencyPair.second.GetBaseParameter();
        bool               _isMatched         = binary_search(_variablesInLikelihood.begin(), _variablesInLikelihood.end(), _efficiencyPointer);
        bool               _isFloat           = not(_efficiencyPointer->isConstant());
        if (_isMatched && _isFloat) { _matchedEfficiencyKeys.push_back(_efficiencyKey); }
    }
    return move(_matchedEfficiencyKeys);
}

vector< vector< bool > > FitParameterPool::GetEfficiencyAdjacencyMatrix(const vector< FitParameterConfig > & _efficiencyKeys) const {
    int                      _nEfficiency = _efficiencyKeys.size();
    vector< vector< bool > > _adjacencyMatrix(_nEfficiency);

    for (auto & _column : _adjacencyMatrix) { _column.resize(_nEfficiency); }
        for (int i = 0; i < _nEfficiency; i++) {
        // Efficiencies which has not the ratio Flag, set all its adjacency elements to false
        _adjacencyMatrix[i][i]=false;                
        for (int j = i+1; j < _nEfficiency; j++) {        
            _adjacencyMatrix[i][j] = _efficiencyKeys[i].ForRatio() && _efficiencyKeys[j].ForRatio();
            _adjacencyMatrix[j][i] = _efficiencyKeys[i].ForRatio() && _efficiencyKeys[j].ForRatio();                        
        }
    }

    for (int i = 0; i < _nEfficiency; i++) {
        // If the efficiency is not for the ratio, set all its adjacency elements to false, so that a 1D g-constraint is done. 
        bool _forRatioTag                =  _efficiencyKeys[i].ForRatio();
        bool _isLeakage                  =  _efficiencyKeys[i].IsLeakageComponent();
        bool _isKEtaPrimeGEE             =  _efficiencyKeys[i].IsKEtaPrimeComponent();
        bool _isCrossFeedComponent       =  _efficiencyKeys[i].IsCrossFeedComponent();

        if (  !_forRatioTag  ){
            //non R - ratio connected efficiencies will land in a 1-D constraint (uncorrelated efficiencies list later on)            
            for (int j = 0; j < _nEfficiency; j++) {
                _adjacencyMatrix[i][j] = false;
                _adjacencyMatrix[j][i] = false;
            }
        }
        if( _isLeakage && SettingDef::Efficiency::option.Contains("noCovLeakage")){
            //Remove leakage from full-covariance matrix of efficiency , do 1D 
            for (int j = 0; j < _nEfficiency; j++) {
                _adjacencyMatrix[i][j] = false;
                _adjacencyMatrix[j][i] = false;
            }
        }
        if( _isCrossFeedComponent){
            //Remove CrossFeed (Bd2Kst) from full-covariance matrix of efficiency, do 1D 
            for (int j = 0; j < _nEfficiency; j++) {
                _adjacencyMatrix[i][j] = false;
                _adjacencyMatrix[j][i] = false;
            }
        }
        if( _isKEtaPrimeGEE){
            //Remove KEtaPrime (KEtaPrime-RK) from full-covariance matrix of efficiency, do 1D 
            for (int j = 0; j < _nEfficiency; j++) {
                _adjacencyMatrix[i][j] = false;
                _adjacencyMatrix[j][i] = false;
            }
        }
    }

    return move(_adjacencyMatrix);
}

void FitParameterPool::PrintAdjacencyMatrix(const vector< vector< bool > > & _adjacencyMatrix, const vector< FitParameterConfig > & _efficiencyKeys) {
    MessageSvc::Line();
    MessageSvc::Info(Color::Cyan, (TString) "Printing Adjacency Matrix for parameters : ");
    int _nEfficiency = _efficiencyKeys.size();
    for (const auto & _effKey : _efficiencyKeys) {
        TString _name = m_efficiencyMap[_effKey].GetUnblindedParameter()->GetName();
        MessageSvc::Info(_name);
    }
    for (int i = 0; i < _nEfficiency; i++) {
        for (int j = 0; j < _nEfficiency; j++) { cout << _adjacencyMatrix[i][j] << " "; }
        cout << endl;
    }
    MessageSvc::Line();
}

vector< RooRealVar * > FitParameterPool::ExtractUncorrelatedEfficiencies(const vector< vector< bool > > & _adjacencyMatrix, const vector< FitParameterConfig > & _efficiencyKeys) {
    int                    _nEfficiency = _efficiencyKeys.size();
    vector< RooRealVar * > _uncorrelatedEfficiencies;
    for (int i = 0; i < _nEfficiency; i++) {
        const auto & _row = _adjacencyMatrix[i];
        // variable is uncorrelated if all
        int  _sumOffDiagonal = accumulate(_row.begin(), _row.end(), (int) 0);
        bool _isUncorrelated = (_sumOffDiagonal == 0);
        if (_isUncorrelated) {
            const auto & _efficiencyKey      = _efficiencyKeys[i];
            RooRealVar * _efficiencyVariable = (RooRealVar *) (m_efficiencyMap[_efficiencyKey].GetBaseParameter());
            // RooRealVar * _efficiencyVariable = (RooRealVar *) (m_efficiencyMap[_efficiencyKey].GetUnblindedParameter()); maybe?
            _uncorrelatedEfficiencies.push_back(_efficiencyVariable);
        }
    }
    return move(_uncorrelatedEfficiencies);
}

vector< CorrelatedEfficienciesHolder > FitParameterPool::ExtractCorrelatedEfficiencies(const vector< vector< bool > > & _adjacencyMatrix, const vector< FitParameterConfig > & _efficiencyKeys) {
    // We split the correlated efficiencies into independent subsets
    vector< vector< FitParameterConfig > > _correlatedEfficienciesList = SubdivideCorrelatedEfficiencies(_adjacencyMatrix, _efficiencyKeys);
    vector< CorrelatedEfficienciesHolder > _correlatedEfficiencies;
    for (const auto & _correlatedEfficiencyKeys : _correlatedEfficienciesList) {
        vector< RooRealVar * >     _correlatedEfficiencyList = GetEfficiencyList(_correlatedEfficiencyKeys);
        vector< vector< double > > _covarianceMatrix         = GetCovarianceMatrix(_correlatedEfficiencyKeys);
        _correlatedEfficiencies.emplace_back(_correlatedEfficiencyList, _covarianceMatrix);
    }
    return move(_correlatedEfficiencies);
}

vector< vector< FitParameterConfig > > FitParameterPool::SubdivideCorrelatedEfficiencies(const vector< vector< bool > > & _adjacencyMatrix, const vector< FitParameterConfig > & _efficiencyKeys) const {
    int _nEfficiency = _efficiencyKeys.size();
    // Subdivied correlated efficiencies into independent chunks
    vector< vector< FitParameterConfig > > _subdividedEfficiencies;
    vector< bool >                         _isUsed(_nEfficiency, false);
    for (int i = 0; i < _nEfficiency; i++) {
        if (_isUsed[i]) continue;
        vector< FitParameterConfig > _correlatedEfficiencyList;
        const auto &                 _row            = _adjacencyMatrix[i];
        int                          _sumOffDiagonal = accumulate(_row.begin(), _row.end(), (int) 0);
        bool                         _isUncorrelated = (_sumOffDiagonal == 0);
        if (_isUncorrelated) continue;
        _correlatedEfficiencyList.push_back(_efficiencyKeys[i]);
        _isUsed[i] = true;
        for (int j = i + 1; j < _nEfficiency; j++) {
            bool _isCorrelated = _row[j];
            if (_isCorrelated) {
                _correlatedEfficiencyList.push_back(_efficiencyKeys[j]);
                _isUsed[j] = true;
            }
        }
        _subdividedEfficiencies.push_back(_correlatedEfficiencyList);
    }
    return move(_subdividedEfficiencies);
}

vector< RooRealVar * > FitParameterPool::GetEfficiencyList(const vector< FitParameterConfig > & _efficiencyKeys) {
    vector< RooRealVar * > _efficiencyList;    
    for (const auto & _key : _efficiencyKeys) {     
        RooRealVar * _efficiency = (RooRealVar *) m_efficiencyMap[_key].GetBaseParameter();
        // RooRealVar * _efficiency = (RooRealVar *) m_efficiencyMap[_key].GetUnblindParameter(); maybe?
        if(!_efficiency->isConstant()){
            _efficiencyList.push_back(_efficiency);
        }else{
            MessageSvc::Warning("GetEfficiencyList, Skipping because forced const", _efficiency);
        }
        // if( _key.IsSignalComponent()){
        //     auto CC = _key; 
        //     CC.SetForRatio(true);
        //     RooRealVar * _efficiencyFULL = (RooRealVar *) m_efficiencyMap[CC].GetBaseParameter();
        //     if(!_efficiencyFULL->isConstant()){
        //         _efficiencyList.push_back(_efficiencyFULL);
        //     }else{
        //         MessageSvc::Warning("GetEfficiencyList (FULL), Skipping because forced const", _efficiency);
        //     }
        // }
    }
    return move(_efficiencyList);
}

vector< vector< double > > FitParameterPool::GetCovarianceMatrix(const vector< FitParameterConfig > & _correlatedEfficiencyKeys) const {
    int                        _nCorrelatedEfficiencies = _correlatedEfficiencyKeys.size();
    vector< vector< double > > _covarianceMatrix        = InitialiseCovarianceMatrix(_nCorrelatedEfficiencies);
    for (int i = 0; i < _nCorrelatedEfficiencies; i++) {
        _covarianceMatrix[i][i] = GetVarianceForMatrix(_correlatedEfficiencyKeys[i]);
        for (int j = i + 1; j < _nCorrelatedEfficiencies; j++) {
            double _covariance      = GetCovarianceForMatrix(_correlatedEfficiencyKeys[i], _correlatedEfficiencyKeys[j]);
            _covarianceMatrix[i][j] = _covariance;
            _covarianceMatrix[j][i] = _covariance;
        }
    }
    return move(_covarianceMatrix);
}

vector< vector< double > > FitParameterPool::InitialiseCovarianceMatrix(int _nEfficiency) const {
    vector< vector< double > > _covarianceMatrix(_nEfficiency);
    for (auto & _row : _covarianceMatrix) { _row.resize(_nEfficiency); }
    for( int i= 0; i < _covarianceMatrix.size(); ++i){
        for( int j = 0; j < _covarianceMatrix[i].size(); ++j){
            _covarianceMatrix[i][j] = 0;
        }
    }
    return move(_covarianceMatrix);
}

double FitParameterPool::GetVarianceForMatrix(const FitParameterConfig & _efficiencyKey) const {
    ConfigHolder _config            = _efficiencyKey.GetConfigHolder();
    const auto & _efficiencyWrapper = m_efficiencyMap.at(_efficiencyKey);   // Need to us at() instead of operator[] for const qualifier
    double       _rawVariance       = GetVariance(_config);
    double       _varianceInMatrix  = _efficiencyWrapper.TransformVarianceIfBlinded(_rawVariance);
    // double       _error             = ((RooRealVar *) _efficiencyWrapper.GetBaseParameter())->getError();
    // _varianceInMatrix               = _error * _error;
    return _varianceInMatrix;
}

double FitParameterPool::GetCovarianceForMatrix(const FitParameterConfig & _effKeyA, const FitParameterConfig & _effKeyB) const {
    ConfigHolder _configA            = _effKeyA.GetConfigHolder();
    ConfigHolder _configB            = _effKeyB.GetConfigHolder();
    const auto & _effWrapperA        = m_efficiencyMap.at(_effKeyA);
    const auto & _effWrapperB        = m_efficiencyMap.at(_effKeyB);
    double       _rawCovariance      = GetCovariance(_configA, _configB);    
    //will return a scale * scale * rawCovariance 
    double       _covarianceInMatrix = _effWrapperB.TransformCovarianceIfBlinded(_effWrapperA.TransformCovarianceIfBlinded(_rawCovariance));
    // double       _errorA             = ((RooRealVar *) _effWrapperA.GetBaseParameter())->getError();
    // double       _errorB             = ((RooRealVar *) _effWrapperB.GetBaseParameter())->getError();
    // _covarianceInMatrix              = 0 * _errorA * _errorB;
    return _covarianceInMatrix;
}

vector< RooRealVar * > FitParameterPool::GetConstrainedParametersInLikelihood(const RooArgList & _listOfLikelihoods) const {
    vector< RooRealVar * > _variablesInLikelihood = GetUniqueVariablesInLikelihood(_listOfLikelihoods);
    sort(_variablesInLikelihood.begin(), _variablesInLikelihood.end());
    vector< RooRealVar * > _constrainedParametersInLikelihood = GetMatchedConstrainedParameters(_variablesInLikelihood);
    return move(_constrainedParametersInLikelihood);
}

vector< RooRealVar * > FitParameterPool::GetMatchedConstrainedParameters(vector< RooRealVar * > _variablesInLikelihood) const {
    vector< RooRealVar * > _constrainedParametersInLikelihood;
    for (auto * _parameter : m_constrainedParameters) {
        bool _isInLikelihood = binary_search(_variablesInLikelihood.begin(), _variablesInLikelihood.end(), _parameter);
        if (_isInLikelihood) _constrainedParametersInLikelihood.push_back(_parameter);
    }
    return move(_constrainedParametersInLikelihood);
}

vector< RooRealVar * > FitParameterPool::GetConstrainedParameters() { return m_constrainedParameters; }

map< RooRealVar *, RooRealVar * > FitParameterPool::GetFloatingToFixedEfficiencyMap() const { return m_floatingToFixedEfficiencyMap; }

map< FitParameterConfig, ParameterWrapper > FitParameterPool::GetFixedEfficiencyMap() const { return m_fixedEfficiencyMap; }

map< FitParameterConfig, ParameterWrapper > FitParameterPool::GetEfficiencyMap() const { return m_efficiencyMap; }

map< pair< FitParameterConfig, RatioType >, ParameterWrapper > FitParameterPool::GetRatioMap() const { return m_ratioMap; }

map< FitParameterConfig, ParameterWrapper > FitParameterPool::GetYieldMap() const { return m_yieldMap; }

map< TString, ParameterWrapper > FitParameterPool::GetShapeParameterMap() const { return m_shapeParameterMap; }

void FitParameterPool::LoadFloatingToFixedEfficiencyMap(const map< RooRealVar *, RooRealVar * > _floatingToFixedEfficiencyMap) { m_floatingToFixedEfficiencyMap = _floatingToFixedEfficiencyMap; }

void FitParameterPool::LoadFixedEfficiencyMap(const map< FitParameterConfig, ParameterWrapper > _efficiencyMap) { m_fixedEfficiencyMap = _efficiencyMap; }

void FitParameterPool::LoadEfficiencyMap(const map< FitParameterConfig, ParameterWrapper > _efficiencyMap) { m_efficiencyMap = _efficiencyMap; }

void FitParameterPool::LoadRatioMap(const map< pair< FitParameterConfig, RatioType >, ParameterWrapper > _ratioMap) { m_ratioMap = _ratioMap; }

void FitParameterPool::LoadYieldMap(const map< FitParameterConfig, ParameterWrapper > _yieldMap) { m_yieldMap = _yieldMap; }

void FitParameterPool::LoadShapeParameterMap(const map< TString, ParameterWrapper > _shapeParameterMap) { m_shapeParameterMap = _shapeParameterMap; }

void FitParameterPool::LoadConstrainedParameters(const vector<RooRealVar*> & _constrainedParameters ){ 
//void FitParameterPool::LoadConstrainedParameters(const vector< RooRealVar* > _constrainedParameters) { m_constrainedParameters = _constrainedParameters; }
    for( auto & _constrainedVar : _constrainedParameters){ 
        AddConstrainedParameter(_constrainedVar); 
    }
}

namespace RXFitter {
    // Returns the weak_ptr reflecting a shared_ptr to the only instance of FitParameterPool.
    // This will allow to check if the shared_ptr is still alive, solving the dangling pointer problem.
    weak_ptr< FitParameterPool > & GetWeakPointer() {
        static weak_ptr< FitParameterPool > _weakPool;
        return _weakPool;
    }

    // Factory function returning a shared pointer to the only instance of the FitParameterPool.
    shared_ptr< FitParameterPool > GetParameterPool() {
        if (GetWeakPointer().expired()) {
            shared_ptr< FitParameterPool > shared = make_shared< FitParameterPool >();
            GetWeakPointer()                      = shared;
            return GetWeakPointer().lock();
        }
        return GetWeakPointer().lock();
    }
};   // namespace RXFitter

ParameterWrapper::ParameterWrapper() {}

ParameterWrapper::ParameterWrapper(RooAbsReal * _parameter)
    : ParameterWrapper() {
    SetParameter(_parameter);
}

void ParameterWrapper::SetParameter(RooAbsReal * _parameter) { m_unblindedParameter = _parameter; } //m_unblindedParameter is the Raw Efficiency for example

void ParameterWrapper::ThrowIfNotRooRealVar(RooAbsReal * _parameter) {
    if (!_parameter) { MessageSvc::Error("ParameterWrapper", (TString) "passed a null pointer", "EXIT_FAILURE"); }
    if (not(_parameter->InheritsFrom(RooRealVar::Class()))) { MessageSvc::Error("ParameterWrapper", _parameter->GetName(), (TString) "is not of type RooRealVar", "EXIT_FAILURE"); }
}

void ParameterWrapper::Blind() {
    ThrowIfNotRooRealVar(m_unblindedParameter);   // Only blinds if base parameter is of type RooRealVar
    m_isBlinded          = true;
    m_blindedParameter   = m_unblindedParameter;
    m_unblindedParameter = BlindParameter((RooRealVar *) m_blindedParameter);
}

bool ParameterWrapper::IsBlinded() const { return m_isBlinded; }

RooAbsReal * ParameterWrapper::GetBlindedParameter() { return m_blindedParameter; }

RooAbsReal * ParameterWrapper::GetUnblindedParameter() { return m_unblindedParameter; }

RooAbsReal * ParameterWrapper::GetBaseParameter() {
    if (IsBlinded()) {
        return m_blindedParameter;
    } else {
        return m_unblindedParameter;
    }
}

const RooAbsReal * ParameterWrapper::GetBaseParameter() const {
    if (IsBlinded()) {
        return m_blindedParameter;
    } else {
        return m_unblindedParameter;
    }
}

void ParameterWrapper::DeleteParameter() {
    delete m_unblindedParameter;
    m_unblindedParameter = nullptr;
    if (IsBlinded()) {
        delete m_blindedParameter;
        m_blindedParameter = nullptr;
    }
}

double ParameterWrapper::TransformCovarianceIfBlinded(double _covariance) const {
    if (IsBlinded()) {
        return ((UnblindParameter *) m_unblindedParameter)->TransformCovariance(_covariance);
    } else {
        return _covariance;
    }
}

double ParameterWrapper::TransformVarianceIfBlinded(double _variance) const {
    if (IsBlinded()) {
        return ((UnblindParameter *) m_unblindedParameter)->TransformVariance(_variance);
    } else {
        return _variance;
    }
}

CorrelatedEfficienciesHolder::CorrelatedEfficienciesHolder() {}

CorrelatedEfficienciesHolder::CorrelatedEfficienciesHolder(const vector< RooRealVar * > & _correlatedEfficiencies, const vector< vector< double > > & _covarianceMatrix) {
    SetCorrelatedEfficiencies(_correlatedEfficiencies);
    SetCovarianceMatrix(_covarianceMatrix);
}

void CorrelatedEfficienciesHolder::SetCorrelatedEfficiencies(const vector< RooRealVar * > & _correlatedEfficiencies) { m_correlatedEfficiencies = _correlatedEfficiencies; }

void CorrelatedEfficienciesHolder::SetCovarianceMatrix(const vector< vector< double > > & _covarianceMatrix) {
    ThrowIfCovarianceMatrixWrong(_covarianceMatrix);
    m_covarianceMatrix = _covarianceMatrix;
}

void CorrelatedEfficienciesHolder::ThrowIfCovarianceMatrixWrong(const vector< vector< double > > & _covarianceMatrix) const {
    ThrowIfRowAndColumnMisMatch(_covarianceMatrix);
    ThrowIfOffDiagonalZero(_covarianceMatrix);
}

void CorrelatedEfficienciesHolder::ThrowIfRowAndColumnMisMatch(const vector< vector< double > > & _covarianceMatrix) const {
    auto _nRow = _covarianceMatrix.size();
    for (const auto & _column : _covarianceMatrix) {
        if (_column.size() != _nRow) { MessageSvc::Error("CorrelatedEfficienciesHolder", (TString) "Trying to use a covariance matrix where the row and column sizes are mismatched", "EXIT_FAILURE"); }
    }
}

void CorrelatedEfficienciesHolder::ThrowIfOffDiagonalZero(const vector< vector< double > > & _covarianceMatrix) const {
    auto _matrixSize = _covarianceMatrix.size();
    for (int i = 0; i < _matrixSize; i++) {
        double       _diagonalElement = _covarianceMatrix[i][i];
        const auto & _column          = _covarianceMatrix[i];
        bool         _offDiagonalZero = accumulate(_column.begin(), _column.end(), 0.) == _diagonalElement;
        // if (_offDiagonalZero) {
        //     MessageSvc::Error("CorrelatedEfficienciesHolder", (TString)"covariance matrix sum of off diagonal terms is zero", "EXIT_FAILURE");
        // }
    }
}

vector< RooRealVar * > CorrelatedEfficienciesHolder::GetCorrelatedEfficiencies() const {
    if (IsCorrelatedEfficienciesEmpty()) { WarnEmptyContainer("correlated efficiency list"); }
    ThrowIfListAndMatrixSizeMismatch();
    return m_correlatedEfficiencies;
}

vector< vector< double > > CorrelatedEfficienciesHolder::GetCovarianceMatrix() const {
    if (IsCovarianceMatrixEmpty()) { WarnEmptyContainer("covariance matrix"); }
    ThrowIfListAndMatrixSizeMismatch();
    return m_covarianceMatrix;
}

bool CorrelatedEfficienciesHolder::IsCorrelatedEfficienciesEmpty() const {
    bool _isEmpty = m_correlatedEfficiencies.size() == 0;
    return _isEmpty;
}

bool CorrelatedEfficienciesHolder::IsCovarianceMatrixEmpty() const {
    bool _isEmpty = m_covarianceMatrix.size() == 0;
    return _isEmpty;
}

void CorrelatedEfficienciesHolder::WarnEmptyContainer(TString _containerName) const { MessageSvc::Warning("CorrelatedEfficienciesHolder", _containerName, "is empty"); }

void CorrelatedEfficienciesHolder::ThrowIfListAndMatrixSizeMismatch() const {
    auto _nCorrelatedEfficiencies = m_correlatedEfficiencies.size();
    auto _covarianceMatrixSize    = m_covarianceMatrix.size();
    if (_nCorrelatedEfficiencies != _covarianceMatrixSize) {
        cout<< " n Correlated Efficiencies Size = "<< _nCorrelatedEfficiencies << endl;
        for( auto * cc : m_correlatedEfficiencies){
            std::cout<< "CorrEff) "<< cc->GetName() << std::endl;
        }
        cout<< " Covariance Matrix Size         = "<< _covarianceMatrixSize   << endl;

        MessageSvc::Error("CorrelatedEfficienciesHolder", (TString) "Trying to get correlated efficiencies or covariance matrix but their sizes do not match", "EXIT_FAILURE"); 
    }
}

FitParameterSnapshot::FitParameterSnapshot() { m_parameterPool = RXFitter::GetParameterPool(); }

void FitParameterSnapshot::ConfigureSnapshotMap() {
    MessageSvc::Line();
    MessageSvc::Info("FitParameterSnapshot", (TString) "ConfigureSnapshotMap");
    m_parameterPool->PrintParameters();
    m_floatingToFixedEfficiencyMap = m_parameterPool->GetFloatingToFixedEfficiencyMap();
    m_fixedEfficiencyMap           = m_parameterPool->GetFixedEfficiencyMap();
    m_efficiencyMap                = m_parameterPool->GetEfficiencyMap();
    m_ratioMap                     = m_parameterPool->GetRatioMap();
    m_yieldMap                     = m_parameterPool->GetYieldMap();
    m_shapeParameterMap            = m_parameterPool->GetShapeParameterMap();
    m_constrainedParameters        = m_parameterPool->GetConstrainedParameters();
}

void FitParameterSnapshot::ReloadParameters() {
    MessageSvc::Line();
    MessageSvc::Info("FitParameterSnapshot", (TString) "ReloadParameters");
    m_parameterPool->ClearParameters();
    m_parameterPool->LoadFloatingToFixedEfficiencyMap(m_floatingToFixedEfficiencyMap);
    m_parameterPool->LoadFixedEfficiencyMap(m_fixedEfficiencyMap);
    m_parameterPool->LoadEfficiencyMap(m_efficiencyMap);
    m_parameterPool->LoadRatioMap(m_ratioMap);
    m_parameterPool->LoadYieldMap(m_yieldMap);
    m_parameterPool->LoadShapeParameterMap(m_shapeParameterMap);
    m_parameterPool->LoadConstrainedParameters(m_constrainedParameters);
    m_parameterPool->PrintParameters();
}

#endif
