#ifndef UNBLINDPARAMETER_CPP
#define UNBLINDPARAMETER_CPP

#include "UnblindParameter.hpp"

#include "ConfigHolder.hpp"
#include "IOSvc.hpp"
#include "MessageSvc.hpp"
#include "SettingDef.hpp"

#include <fstream>
#include <iostream>
#include <vector>

TRandom3 UnblindParameter::m_randomNumberGenerator = TRandom3();

UnblindParameter::UnblindParameter()
    : RooAbsReal()
    , m_value() {}

UnblindParameter::UnblindParameter(TString _name, TString _title, RooRealVar & _blindedParameter)
    : RooAbsReal(_name.Data(), _title.Data())
    , m_value("value", "Blinded value", this, _blindedParameter) {}

UnblindParameter::UnblindParameter(const UnblindParameter & _unblindedParameter, const char * _name)
    : RooAbsReal(_unblindedParameter, _name)
    , m_value("asym", this, _unblindedParameter.m_value) {}

void UnblindParameter::printValue(std::ostream & stream) const { stream << "(hidden)" << endl; }

Double_t UnblindParameter::getHiddenVal(const RooArgSet * nset) const { return RooAbsReal::getVal(nset); }

void UnblindParameter::SetSeed(unsigned long _seed) { m_randomNumberGenerator.SetSeed(_seed); }

// TODO implement string-based blinding seed
void UnblindParameter::SetSeed(TString _blindingString) {
    unsigned long _seed = _blindingString.Hash();
    m_randomNumberGenerator.SetSeed(_seed);
}

Double_t UnblindParameter::evaluate() const {
    double _blindedValue   = m_value;
    double _unblindedValue = UnblindingValueTransform(_blindedValue);
    return _unblindedValue;
}

Double_t UnblindParameter::getValV(const RooArgSet * nset) const { return RooAbsReal::getValV(nset); }

void UnblindParameter::TransformBlindedParameter(RooRealVar & _blindedParameter) {
    double _unblindedValue = _blindedParameter.getVal();
    double _unblindedError = _blindedParameter.getError();
    double _unblindedMin   = _blindedParameter.getMin();
    double _unblindedMax   = _blindedParameter.getMax();
    double _blindedValue   = BlindingValueTransform(_unblindedValue);
    double _blindedError   = BlindingErrorTransform(_unblindedError);
    // double _blindedError   = (_unblindedError/_unblindedValue )* _blindedValue;  MAYBE?
    double _blindedMin     = BlindingValueTransform(_unblindedMin);
    double _blindedMax     = BlindingValueTransform(_unblindedMax);
    _blindedParameter.setRange(_blindedMin, _blindedMax);
    _blindedParameter.setVal(_blindedValue);
    _blindedParameter.setError(_blindedError);
}

UnblindOffsetScale::UnblindOffsetScale()
    : UnblindParameter() {}

UnblindOffsetScale::UnblindOffsetScale(TString _name, TString _title, double _scaleLimit, RooRealVar & _blindedParameter)
    : UnblindParameter(_name, _title, _blindedParameter) {
    CheckScaleLimit(_scaleLimit);
    GetTransformationParameters(_blindedParameter, _scaleLimit);
    TransformBlindedParameter(_blindedParameter);
}

UnblindOffsetScale::UnblindOffsetScale(const UnblindOffsetScale & _unblindedParameter, const char * _name)
    : UnblindParameter(_unblindedParameter, _name) {
    m_scale  = _unblindedParameter.m_scale;
}

void UnblindOffsetScale::CheckScaleLimit(double _scaleLimit) {
    if (not(_scaleLimit > 1.)){ MessageSvc::Error("UnblindOffsetScale", (TString) "scaleLimit passed is not real positive", "EXIT_FAILURE"); }
    if (_scaleLimit < 0.) { 
        MessageSvc::Error("UnblindOffsetScale", (TString) "scaleLimit passed is negative ! ", "EXIT_FAILURE"); 
    }
}

void UnblindOffsetScale::GetTransformationParameters(RooRealVar & _blindedParameter, double _scaleLimit) {
    if (TransformationFoundOnDisk()) {
        LoadTransformation();
    } else {
        GenerateTransformation(_blindedParameter, _scaleLimit);
        SaveTransformation();
    }
}

bool UnblindOffsetScale::TransformationFoundOnDisk() const {
    auto _filePath = GetTransformFilePath();
    return IOSvc::ExistFile(_filePath);
}

TString UnblindOffsetScale::GetTransformFilePath() const {
    ConfigHolder _config;   // This doesn't matter, we just need a dummy ConfigHolder
    TString      _fitDir   = IOSvc::GetFitDir("", _config);
    TString      _fileName = TString(GetName());
    TString      _filePath = _fitDir + "/UnblindOffsetScaleBinaries/" + _fileName;
    return _filePath;
}

void UnblindOffsetScale::LoadTransformation() {
    TString _filePath = GetTransformFilePath();
    ReadTransformationBinary(_filePath);
}

void UnblindOffsetScale::ReadTransformationBinary(const TString & _filePath) {
    std::vector< double > buffer(1);
    double *              bufferPointer = &buffer[0];
    fstream               _transformFile;
    _transformFile.open(_filePath.Data(), std::ios::in | std::ios::binary);
    _transformFile.read((char *) bufferPointer, 1 * sizeof(double));
    m_scale = buffer[0];
    _transformFile.close();
    MessageSvc::Info("UnblindOffsetScale", (TString) "Loaded transformation parameters from file", _filePath);
}

void UnblindOffsetScale::GenerateTransformation(RooRealVar & _blindedParameter, double _scaleLimit) {
    double _parameterValue = _blindedParameter.getVal();    
    TString _namePar = _blindedParameter.GetName();
    MessageSvc::Debug("GenerateTransformation for parameter", _namePar);
    if( _namePar.Contains("eff")){
        //Maybe also the RLL ????? !!!!!
        TString _blindString = SettingDef::blindString;
        //================================================================================================
        //We are blinding an efficiency here . Efficiency Ratios matters ! 
        //================================================================================================
        // The bliding depends on the global configured SettingDef::blindString
        // Our bliding is done so that the 'relative' errors are always unblinded, a scale transformation ensures sigma/value to be unblinded (sensitivity)
        // This poses issues when performing ratios of blinded parameters and we must ensure that the scale factor of blinding is DIFFERENT for various species of efficiencies 
        // THerefore the Seed to generate will be the same to preserve some ratios properties on constraints, but different for what final results will show up

        // - Blind scale so that EE/MM ratio is also blinded. eff(EE)/eff(MM) = alpha/beta 
        // - If we would not do a different seed , eff(EE)/eff(MM) even if blinded will be the unblinded value
        _blindString +=  _namePar.Contains("-EE-") ? "-EE" : "";
        _blindString +=  _namePar.Contains("-MM-") ? "-MM" : "";
        _blindString +=  _namePar.Contains("-ME-") ? "-ME" : "";

        // - Blind scale so that central/low efficiency ratio is also blinded. effA(EE)/effB(EE) = alpha/beta  if Aq2 != Bq2
        _blindString +=  _namePar.Contains("-low-") ?     "-low" : "";
        _blindString +=  _namePar.Contains("-central-") ? "-central" : "";
        _blindString +=  _namePar.Contains("-high-") ?    "-high" : "";
        // - Blind scale so that RK/RKst efficiency ratio is also blinded. effA(EE)/effB(EE) = alpha/beta  if Aprj!= Bprj
        if( _namePar.Contains("Bd2Kst")){ 
            //The cross - feed efficiency is special in RK , as it needs to have a blinding scale exactly the same as in RKst , so that ratio is properly defined without scaling factor being crap
            //We will generate the same blinding scale of RKst even if it's RK 
            _blindString +=  _namePar.Contains("-RK-") ?    "-RKst" : "";        
        }else{
            _blindString +=  _namePar.Contains("-RK-") ?    "-RK" : "";
            _blindString +=  _namePar.Contains("-RKst-") ?  "-RKst" : "";
            _blindString +=  _namePar.Contains("-RPhi-") ?  "-RPhi" : "";
        }
        // - Blind scale so that Run period comparisons is also not possible via ratios
        _blindString +=  _namePar.Contains("-R1-")   ? "-R1" :   "";
        _blindString +=  _namePar.Contains("-R2p1-") ? "-R2p1" : "";
        _blindString +=  _namePar.Contains("-R2p2-") ? "-R2p2" : "";                
        MessageSvc::Warning("UnblindOffsetScale (Use Local Random generator)", TString(_blindedParameter.GetName()));
        MessageSvc::Debug("BlindString for parameter ", TString(_blindedParameter.GetName()), TString(_blindString));
        TRandom3 _myLocalRnd;
        unsigned long _seed = _blindString.Hash();
        _myLocalRnd.SetSeed(_seed);
        m_scale = _myLocalRnd.Uniform(1.0,_scaleLimit); //that scale limit was 1.0, -1 ! that's crap.
	//m_scale = 1.; Debugging hack , force eff-blinding to 1
    }else{    
        MessageSvc::Warning("UnblindOffsetScale (Use general Random generator)", TString(_blindedParameter.GetName()));
        m_scale = m_randomNumberGenerator.Uniform(1,_scaleLimit);
    }
}

void UnblindOffsetScale::SaveTransformation() const {
    auto _filePath = GetSavePath();
    MakeSaveDirectory(_filePath);
    WriteTransformationBinary(_filePath);
}

TString UnblindOffsetScale::GetSavePath() const {
    if (not(HasEOS())) {
        return GetTransformFilePath();
    } else {
        TString _fileName = TString(GetName());
        TString _filePath = "UnblindOffsetScaleBinaries/" + _fileName;
        return _filePath;
    }
}

void UnblindOffsetScale::MakeSaveDirectory(const TString & _filePath) const {
    auto _lastBackslashPosition = _filePath.Last('/');
    auto _folderPath            = _filePath(0, _lastBackslashPosition);
    IOSvc::MakeDir(_folderPath);
}

void UnblindOffsetScale::WriteTransformationBinary(const TString & _filePath) const {
    fstream _outFile;
    _outFile.open(_filePath.Data(), std::ios::out | std::ios::app | std::ios::binary);
    _outFile.write((char *) &m_scale, sizeof(double));
    _outFile.close();
    MessageSvc::Info("UnblindOffsetScale", (TString) "Saved transformation parameters to", _filePath);
}

TObject * UnblindOffsetScale::clone(const char * _newName) const { return new UnblindOffsetScale(*this, _newName); }

double UnblindOffsetScale::TransformCovariance(double _covariance) const { return m_scale * _covariance; };

double UnblindOffsetScale::TransformVariance(double _variance) const { return m_scale * m_scale * _variance; }; 
//variance here is the unblinded one.

double UnblindOffsetScale::BlindingValueTransform(double _unblindedValue) const {
    //  V[Blind] = [V[Unblinded] - Offset]    * Scale
    //Err[Blind] = (Err[Unblind]/V[Unblind] ) * V[Blind]
    double _blindedValue = _unblindedValue * m_scale;
    return _blindedValue;
}
double UnblindOffsetScale::UnblindingValueTransform(double _blindedValue) const {
    //V[Unblind] = V[Blind]/Scale + Offset]
    //E[Unblind] = E[Blind]/V[Blind] * V[Unblind]
    double _unblindedValue = _blindedValue / m_scale;
    return _unblindedValue;
}

double UnblindOffsetScale::BlindingErrorTransform(double _unblindedError) const {
    double _blindedError = _unblindedError * m_scale;
    return _blindedError;
}



double UnblindOffsetScale::UnblindingErrorTransform(double _blindedError) const {
    std::cout<<"UnblidingErrorTransform..... Probably is wrong, we blind errors such that we preserve relative uncertaintites, with scale on central value it's tricky" << std::endl;    
    double _unblindedError = _blindedError / m_scale;
    return _unblindedError;
}

#endif
