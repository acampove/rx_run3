/*****************************************************************************
 * Project: RooFit                                                           *
 * Package: RooFitModels                                                     *
 * @(#)root/roofit:$Id$
 * Authors:                                                                  *
 *   WV, Wouter Verkerke, UC Santa Barbara, verkerke@slac.stanford.edu       *
 *   DK, David Kirkby,    UC Irvine,         dkirkby@uci.edu                 *
 *                                                                           *
 * Copyright (c) 2000-2005, Regents of the University of California          *
 *                          and Stanford University. All rights reserved.    *
 *                                                                           *
 * Redistribution and use in source and binary forms,                        *
 * with or without modification, are permitted according to the terms        *
 * listed in LICENSE (http://roofit.sourceforge.net/license.txt)             *
 *****************************************************************************/
 
/** \class RooExpTurnOn
    \ingroup Renato Quagliani
**/
 
#include "RooFit.h"
#include "Riostream.h"
#include "Riostream.h"
#include <math.h>
#include "RooExpTurnOn.h"
#include "RooAbsReal.h"
#include "RooRealVar.h"
#include "RooRandom.h"
#include "RooMath.h"
#include "TMath.h"
#include "Math/SpecFunc.h"
#include "gsl/gsl_sf_bessel.h"
using namespace std;
 
ClassImp(RooExpTurnOn);
 
////////////////////////////////////////////////////////////////////////////////
 
RooExpTurnOn::RooExpTurnOn(const char *name, const char *title,
          RooAbsReal& _x, RooAbsReal& _mO,
          RooAbsReal& _sE, RooAbsReal & _bComb) :
  RooAbsPdf(name,title),
  x("x","Observable",this,_x),
  mO("mO","mO",this,_mO),
  sE("sE","sE",this,_sE),
  bComb("b","b",this,_bComb){

}
 
 
RooExpTurnOn::RooExpTurnOn(const RooExpTurnOn& other, const char* name) :
  RooAbsPdf(other,name), x("x",this,other.x), 
                         mO("mO",this,other.mO),
                         sE("sE",this,other.sE),
                         bComb("b",this,other.bComb){
}

double RooExpTurnOn::evaluate() const{
  if(fabs(bComb) < 1e-12){
    return 1./(1.+ TMath::Exp( sE * (mO - x)));
  }
  const double  num =     TMath::Exp( bComb * x);
  const double  den = 1.+ TMath::Exp( sE * (mO - x));
  return num/den;
}

////////////////////////////////////////////////////////////////////////////////
 
Int_t RooExpTurnOn::getAnalyticalIntegral(RooArgSet& allVars, RooArgSet& analVars, const char* /*rangeName*/) const
{
  if (matchArgs(allVars,analVars,x)) return 1 ;
  return 0 ;
}
double RooExpTurnOn::hyperg_z_GT1(double a, double b, double c, double z) const { 
  //calculates 2F1 for z < -1 
  double coef1,coef2;
  coef1=ROOT::Math::tgamma(c)*ROOT::Math::tgamma(b-a)*pow(1.-z,-a)/(ROOT::Math::tgamma(b)*ROOT::Math::tgamma(c-a)); 
  coef2=ROOT::Math::tgamma(c)*ROOT::Math::tgamma(a-b)*pow(1.-z,-b)/(ROOT::Math::tgamma(a)*ROOT::Math::tgamma(c-b)); 
  
  double termone = ROOT::Math::hyperg(a,c-b,a-b+1.,1./(1.-z));
  if( std::isnan( termone)){
    std::cout<<GetName()<<"::coef1          " <<coef1<<std::endl;
    std::cout<<GetName()<<"::Nan TermOne sE " << sE    << std::endl;
    std::cout<<GetName()<<"::Nan TermOne  x " <<  x    << std::endl;
    std::cout<<GetName()<<"::Nan TermOne  b " << bComb << std::endl;
    std::cout<<GetName()<<"::Nan TermOne mO " << mO    << std::endl;
    std::cout<<GetName()<<"::hyperg_z_GT1   ("<< a <<" , "<< c-b << " , "<< a-b+1.<<" , "<<1./(1.-z) <<" , " << " ) "<< std::endl;
  }
  double termtwo = ROOT::Math::hyperg(b,c-a,b-a+1.,1./(1.-z));
  if( std::isnan( termtwo)){
    std::cout<<GetName()<<"::coef2          "<<coef2<<std::endl;
    std::cout<<GetName()<<"::Nan TermOne sE " << sE    << std::endl;
    std::cout<<GetName()<<"::Nan TermOne  x " <<  x    << std::endl;
    std::cout<<GetName()<<"::Nan TermOne  b " << bComb << std::endl;
    std::cout<<GetName()<<"::Nan TermOne mO " << mO    << std::endl;
    std::cout<<GetName()<<", [termtwo]hyperg_z_GT1 ("<< b <<" , "<< c-a << " , "<< b-a+1.<<" , "<<1./(1.-z) <<" , " << " ) "<< std::endl;

  }  
  return coef1*ROOT::Math::hyperg(a,c-b,a-b+1.,1./(1.-z))+coef2*ROOT::Math::hyperg(b,c-a,b-a+1.,1./(1.-z)); 
}
double RooExpTurnOn::hyperg_z(double a, double b, double c, double z)const {
  //domain errors around? Check
  double ret = ROOT::Math::hyperg(a,b,c,z);
  if( std::isnan( ret)){
    std::cout<<"Nan TermOne sE " << sE    << std::endl;
    std::cout<<"Nan TermOne  x " <<  x    << std::endl;
    std::cout<<"Nan TermOne  b " << bComb << std::endl;
    std::cout<<"Nan TermOne mO " << mO    << std::endl;
    std::cout<<GetName()<<", hyperg_z( "<< a << " , "<< b <<" , "<< c << " , " << z << " )"<<std::endl;
  }
  return ROOT::Math::hyperg(a,b,c,z);
}
////////////////////////////////////////////////////////////////////////////////
double RooExpTurnOn::getA()const {
  return 1.;
}
double RooExpTurnOn::getB()const {
  return bComb/sE;
}
double RooExpTurnOn::getC()const {
  return (bComb + sE)/sE;
}
double RooExpTurnOn::getZ(double mass )const {
  return TMath::Exp( (mass- mO)*sE );
}
double RooExpTurnOn::IntegralVal( double xVal) const {
  double Integrand = TMath::Exp( xVal * bComb)/bComb;
  double A = getA();
  double B = getB();
  double C = getC();
  double Z = getZ(xVal);
  double Factor = 0.;
  if( fabs(Z) <1.){    
    double X = ROOT::Math::hyperg( A, B, C, -Z);
    if( std::isnan(X)){
      std::cout<<GetName()<<"::IntegralVal      Z " << Z     << std::endl;
      std::cout<<GetName()<<"::Nan IntegralVal sE " << sE    << std::endl;
      std::cout<<GetName()<<"::Nan IntegralVal  x " << x     << std::endl;
      std::cout<<GetName()<<"::Nan IntegralVal  b " << bComb << std::endl;
      std::cout<<GetName()<<"::Nan IntegralVal mO " << mO    << std::endl;
      std::cout<<GetName()<<"::hyperg( "<<A << " , "<< B << " , "<< C << " , "<< Z << " )"<< std::endl;
    }
    Factor = -1. + ROOT::Math::hyperg( A, B, C, -Z);
  }else if( -Z < -1. ){
    Factor = -1. + hyperg_z_GT1( A, B, C, -Z);   
  }else if( -Z > 1){
    abort();   
  }else{
    std::cout<<GetName()<<"::INVALID DOMAIN For Z    " << Z << std::endl;
    std::cout<<GetName()<<"::INVALID DOMAIN For A    " << A << std::endl;
    std::cout<<GetName()<<"::INVALID DOMAIN For B    " << B << std::endl;
    std::cout<<GetName()<<"::INVALID DOMAIN For C    " << C << std::endl;
    std::cout<<GetName()<<"::INVALID DOMAIN For xVal " << xVal << std::endl;
  }
  return Integrand * Factor;
}

double RooExpTurnOn::analyticalIntegral(Int_t code, const char* rangeName) const{
  if( code == 1){
    double max = x.max(rangeName);
    double min = x.min(rangeName);
    if( fabs(bComb) <1e-12  ){    
      double oneOverSe = 1./sE;
      double LogTerm =  TMath::Log( (TMath::Exp( max * sE) + TMath::Exp( mO*sE) )/( TMath::Exp(min*sE + TMath::Exp(mO*sE))));
      return oneOverSe * LogTerm;
    }
    return -IntegralVal( max) + IntegralVal( min);
  }
  assert(0);
  return 0;
}
