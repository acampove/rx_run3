#ifndef TOYYIELDCONFIG_CPP
#define TOYYIELDCONFIG_CPP

#include "ToyYieldConfig.hpp"
#include "vec_extends.h"

#include "EnumeratorSvc.hpp"
#include "HelperSvc.hpp"
#include "MessageSvc.hpp"
#include <math.h>

ToyYieldConfig::ToyYieldConfig() { ResetYieldVariables(); }

ToyYieldConfig::ToyYieldConfig(TString _sample, double _scalingFactor, int _meanEvents) {
    m_sample        = _sample;
    m_scalingFactor = _scalingFactor;    
    m_meanEvents    = _meanEvents;    
    /*    
    HACK for small yields, negative yield generator will just not work! ( will have to throw an error )
    if( m_meanEvents <0 ){
      MessageSvc::Error("Please fix the fit you used to generate toy Tuples (negative yield found)",_sample);
    }
    if( m_meanEvents < SettingDef::Toy::YieldGeneratorTrheshold){    
      MessageSvc::Warning("Found VERY low yield value to generate, will generate slightly more");
      m_meanEvents = SettingDef::Toy::YieldGeneratorTrheshold;
    }
    */
    CheckToyTupleYieldVariables();
}

ToyYieldConfig::ToyYieldConfig(TString _composition) {
    auto *  _strCollection = ((TString) _composition).ReplaceAll(" ", "").Tokenize("|");
    TString _sampleID      = TString(((TObjString *) (*_strCollection).At(0))->String());
    TString _pdfType       = TString(((TObjString *) (*_strCollection).At(1))->String());
    TString _pdfOption     = TString(((TObjString *) (*_strCollection).At(2))->String());
    if (hash_pdftype(_pdfType) == PdfType::ToyPDF) {
        double _scale = m_noScalingFactorFlag;
        GetValue< double >(_pdfOption, TString("scale"), _scale);
        double _yield = m_noMeanEventsFlag;
        GetValue< double >(_pdfOption, TString("yield"), _yield);

        MessageSvc::Info("ToyYieldConfig", _composition);
        MessageSvc::Info(_sampleID, _pdfType, _pdfOption);
        MessageSvc::Info("Scale", to_string(_scale));
        MessageSvc::Info("Yield", to_string(_yield));

        m_sample        = _sampleID;
        m_scalingFactor = _scale;
        m_meanEvents    = _yield;

        CheckToyTupleYieldVariables();
        Print();
    } else {
        MessageSvc::Error("GetTupleGenerators", (TString) "PdfType", _pdfType, "not supported", "EXIT_FAILURE");
    }
}

ToyYieldConfig::ToyYieldConfig(TString _composition, std::map< TString, double> _override ) {
    auto *  _strCollection = ((TString) _composition).ReplaceAll(" ", "").Tokenize("|");
    TString _sampleID      = TString(((TObjString *) (*_strCollection).At(0))->String());
    TString _pdfType       = TString(((TObjString *) (*_strCollection).At(1))->String());
    TString _pdfOption     = TString(((TObjString *) (*_strCollection).At(2))->String());
    if (hash_pdftype(_pdfType) == PdfType::ToyPDF) {
        if (_override.find(_sampleID) != _override.end()){
            m_sample = _sampleID;
            m_scalingFactor = _override[_sampleID];
            m_meanEvents = 0.;
        }
        else{
            double _scale = m_noScalingFactorFlag;
            GetValue< double >(_pdfOption, TString("scale"), _scale);
            double _yield = m_noMeanEventsFlag;
            GetValue< double >(_pdfOption, TString("yield"), _yield);

            MessageSvc::Info("ToyYieldConfig", _composition);
            MessageSvc::Info(_sampleID, _pdfType, _pdfOption);
            MessageSvc::Info("Scale", to_string(_scale));
            MessageSvc::Info("Yield", to_string(_yield));

            m_sample        = _sampleID;
            m_scalingFactor = _scale;
            m_meanEvents    = _yield;
        }
        CheckToyTupleYieldVariables();
        Print();
    } else {
        MessageSvc::Error("GetTupleGenerators", (TString) "PdfType", _pdfType, "not supported", "EXIT_FAILURE");
    }
}

ToyYieldConfig::ToyYieldConfig(const ToyYieldConfig & _other) {
    m_sample              = _other.m_sample;
    m_scalingFactor       = _other.m_scalingFactor;
    m_meanEvents          = _other.m_meanEvents;
    m_yieldVariableStatus = _other.m_yieldVariableStatus;
}

ToyYieldConfig::ToyYieldConfig(ToyYieldConfig && _other) {
    m_sample              = _other.m_sample;
    m_scalingFactor       = move(_other.m_scalingFactor);
    m_meanEvents          = move(_other.m_meanEvents);
    m_yieldVariableStatus = move(_other.m_yieldVariableStatus);
}

void ToyYieldConfig::ResetYieldVariables() {
    m_scalingFactor       = m_noScalingFactorFlag;
    m_meanEvents          = m_noMeanEventsFlag;
    m_yieldVariableStatus = ToyYieldConfig::YieldVariablesStatus::NoneSet;
}

void ToyYieldConfig::CheckToyTupleYieldVariables() {
    EvaluateSetFlags();
    ThrowIfScaleAndMeanNotSet();
    WarnIfBothScaleAndMeanGiven();
    NotifyIfOnlyOneGiven();
}

void ToyYieldConfig::EvaluateSetFlags() {
    bool scalingFactorSet = (m_scalingFactor != m_noScalingFactorFlag);
    bool meanEventsSet    = (m_meanEvents != m_noMeanEventsFlag);
    if (!(scalingFactorSet || meanEventsSet)) {
        m_yieldVariableStatus = ToyYieldConfig::YieldVariablesStatus::NoneSet;
    } else if ((!scalingFactorSet) && meanEventsSet) {
        m_yieldVariableStatus = ToyYieldConfig::YieldVariablesStatus::MeanSet;
    } else if (scalingFactorSet && (!meanEventsSet)) {
        m_yieldVariableStatus = ToyYieldConfig::YieldVariablesStatus::ScaleSet;
    } else if (scalingFactorSet && meanEventsSet) {
        m_yieldVariableStatus = ToyYieldConfig::YieldVariablesStatus::BothSet;
    }
}

void ToyYieldConfig::ThrowIfScaleAndMeanNotSet() const {
    if (m_yieldVariableStatus == ToyYieldConfig::YieldVariablesStatus::NoneSet) { MessageSvc::Error("ToyYieldConfig", (TString) "No \'ScalingFactor\' or \'MeanEvents\' node found", "logic_error"); }
    return;
}

void ToyYieldConfig::WarnIfBothScaleAndMeanGiven() const {
    if (m_yieldVariableStatus == ToyYieldConfig::YieldVariablesStatus::BothSet) {
        MessageSvc::Warning("ToyYieldConfig", (TString) "Both -ScalingFactor- and -MeanEvents- are given");
        MessageSvc::Warning("ToyYieldConfig", (TString) "The larger of (ScalingFactor*NominalYield vs MeanEvents) will be used");
    }
    return;
}

void ToyYieldConfig::NotifyIfOnlyOneGiven() const {
    switch (m_yieldVariableStatus) {
        case ToyYieldConfig::YieldVariablesStatus::MeanSet: MessageSvc::Info("ToyYieldConfig", (TString) "Using -meanEvents- for this toy tuple configuration"); break;
        case ToyYieldConfig::YieldVariablesStatus::ScaleSet: MessageSvc::Info("ToyYieldConfig", (TString) "Using -scalingFactor- for this toy tuple configuration"); break;
        case ToyYieldConfig::YieldVariablesStatus::NoneSet: break;
        case ToyYieldConfig::YieldVariablesStatus::BothSet: break;
    }
}

bool ToyYieldConfig::IsSet() const { return (m_scalingFactor != m_noScalingFactorFlag || m_meanEvents != m_noMeanEventsFlag); }

void ToyYieldConfig::EvaluateGeneratorMean(double nominalYield) {
    switch (m_yieldVariableStatus) {
        case ToyYieldConfig::YieldVariablesStatus::NoneSet: break;
        case ToyYieldConfig::YieldVariablesStatus::ScaleSet: ScaledMean(nominalYield); break;
        case ToyYieldConfig::YieldVariablesStatus::BothSet: UseLargerMean(nominalYield); break;
        case ToyYieldConfig::YieldVariablesStatus::MeanSet: break;
    }
}

void ToyYieldConfig::ScaledMean(double nominalYield) { 
  m_meanEvents = (int) round(nominalYield * m_scalingFactor); 
}

void ToyYieldConfig::UseLargerMean(double nominalYield) {
    int ScaledMean = (int) round(nominalYield * m_scalingFactor);
    GeneratorMeanMessage(ScaledMean);
    m_meanEvents = fmax(ScaledMean, m_meanEvents);
}

void ToyYieldConfig::GeneratorMeanMessage(int ScaledMean) const {
    MessageSvc::Info("ToyYieldConfig");
    MessageSvc::Info("Sample", m_sample);
    MessageSvc::Warning("ToyYieldConfig", (TString) "Evaluating Toy Component Mean");
    if (ScaledMean > m_meanEvents) {
        MessageSvc::Warning("ToyYieldConfig", (TString) "Will use a mean scaled from nominal yield as generator mean");
    } else {
        MessageSvc::Warning("ToyYieldConfig", (TString) "Will use the given generator mean");
    }
    return;
}

void ToyYieldConfig::Print() const {
    MessageSvc::Info("ToyYieldConfig");
    MessageSvc::Info("Sample", m_sample);
    switch (m_yieldVariableStatus) {
        case ToyYieldConfig::YieldVariablesStatus::NoneSet: break;
        case ToyYieldConfig::YieldVariablesStatus::ScaleSet:
            MessageSvc::Info("ScalingFactor", to_string(m_scalingFactor));
            MessageSvc::Info("MeanEvents", to_string(m_meanEvents));
            break;
        case ToyYieldConfig::YieldVariablesStatus::MeanSet: MessageSvc::Info("MeanEvents", to_string(m_meanEvents)); break;
        case ToyYieldConfig::YieldVariablesStatus::BothSet:
            MessageSvc::Info("ScalingFactor", to_string(m_scalingFactor));
            MessageSvc::Info("MeanEvents", to_string(m_meanEvents));
            break;
    }
    return;
}

ToyYieldConfig & ToyYieldConfig::operator=(const ToyYieldConfig & _other) {
    m_sample              = _other.m_sample;
    m_scalingFactor       = _other.m_scalingFactor;
    m_meanEvents          = _other.m_meanEvents;
    m_yieldVariableStatus = _other.m_yieldVariableStatus;
    return *this;
}

ToyYieldConfig & ToyYieldConfig::operator=(ToyYieldConfig && _other) {
    m_sample              = _other.m_sample;
    m_scalingFactor       = move(_other.m_scalingFactor);
    m_meanEvents          = move(_other.m_meanEvents);
    m_yieldVariableStatus = move(_other.m_yieldVariableStatus);
    return *this;
}

#endif
