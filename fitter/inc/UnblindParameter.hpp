#ifndef UNBLINDPARAMETER_HPP
#define UNBLINDPARAMETER_HPP

#include "TRandom3.h"

#include "RooAbsReal.h"
#include "RooArgSet.h"
#include "RooRealProxy.h"
#include "RooRealVar.h"

class UnblindParameter : public RooAbsReal {
  public:
    UnblindParameter();
    UnblindParameter(TString _name, TString _title, RooRealVar & _blindedParameter);
    UnblindParameter(const UnblindParameter & _unblindedParameter, const char * name = 0);
    void           printValue(std::ostream & stream) const;
    Double_t       getHiddenVal(const RooArgSet * nset = 0) const;
    static void    SetSeed(unsigned long _seed);
    static void    SetSeed(TString _blindingString);
    virtual double TransformCovariance(double _covariance) const = 0;
    virtual double TransformVariance(double _variance) const     = 0;

  public:
    static TRandom3 m_randomNumberGenerator;

  protected:
    void     TransformBlindedParameter(RooRealVar & _blindedParameter);
    Double_t evaluate() const;
    Double_t getValV(const RooArgSet * nset = 0) const;

  protected:
    RooRealProxy m_value;

  private:
    virtual double BlindingValueTransform(double _unblindedValue) const = 0;
    virtual double BlindingErrorTransform(double _unblindedError) const = 0;
    virtual double UnblindingValueTransform(double _blindedValue) const = 0;
    virtual double UnblindingErrorTransform(double _blindedError) const = 0;

  private:
    ClassDef(UnblindParameter, 1);
};

class UnblindOffsetScale : public UnblindParameter {
  public:
    UnblindOffsetScale();
    UnblindOffsetScale(TString _name, TString _title, double _scaleLimit, RooRealVar & _blindedParameter);
    UnblindOffsetScale(const UnblindOffsetScale & _unblindedParameter, const char * name = 0);
    TObject * clone(const char * newname) const;
    double    TransformCovariance(double _covariance) const;
    double    TransformVariance(double _variance) const;

  private:
    void    CheckScaleLimit(double _scaleLimit);
    void    GetTransformationParameters(RooRealVar & _blindedParameter, double _scaleLimit);
    bool    TransformationFoundOnDisk() const;
    TString GetTransformFilePath() const;
    void    LoadTransformation();
    void    ReadTransformationBinary(const TString & _filePath);
    void    GenerateTransformation(RooRealVar & _blindedParameter, double _scaleLimit);
    void    SaveTransformation() const;
    TString GetSavePath() const;
    void    MakeSaveDirectory(const TString & _filePath) const;
    void    WriteTransformationBinary(const TString & _filePath) const;
    double  BlindingValueTransform(double _unblindedValue) const;
    double  BlindingErrorTransform(double _unblindedError) const;
    double  UnblindingValueTransform(double _blindedValue) const;
    double  UnblindingErrorTransform(double _blindedError) const;

  private:
    double m_scale  = 1.;
    ClassDef(UnblindOffsetScale, 1);
};

#endif
