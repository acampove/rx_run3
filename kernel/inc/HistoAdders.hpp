#ifndef HISTOADDERS_HPP
#define HISTOADDERS_HPP
#include <TH1.h>
#include <TH1D.h>
#include <THn.h>
#include "MessageSvc.hpp"
#include <fmt_ostream.h>
#include "yamlcpp.h"
#include "ConstDef.hpp"
#include "WeightHolder.hpp"
#include "HistogramSvc.hpp"

const bool _DEBUGMESSAGE = false;
/**
* \class TH1DHistoAdder
* \brief Functor class to attach weights from TH1D for 1D corrections ( used for L0/HLT corrections )
*/
class TH1DHistoAdder {
public:
    TH1DHistoAdder( TH1D & h , 
                    TString _finalBranchName, 
                    bool _interpolate, 
                    TString _infoMapSource ) : m_histo(h) , m_finalBranchName(_finalBranchName), m_interpolate(_interpolate) , m_infoMapSource(_infoMapSource) {}
    TH1DHistoAdder( const TH1DHistoAdder&& other) : m_histo(other.m_histo), m_finalBranchName(other.m_finalBranchName), m_interpolate(other.m_interpolate) , m_infoMapSource(other.m_infoMapSource){}
    TH1DHistoAdder( const TH1DHistoAdder& other)  : m_histo(other.m_histo), m_finalBranchName(other.m_finalBranchName), m_interpolate(other.m_interpolate) , m_infoMapSource(other.m_infoMapSource){}
    double operator()(double _variable) { 
      double _val = 1.0;
      double _var = _variable;
      int _bin = m_histo.FindFixBin(_var);
      if (m_histo.IsBinUnderflow(_bin) || m_histo.IsBinOverflow(_bin)) {
          if (_var <= m_histo.GetXaxis()->GetXmin()) _var = m_histo.GetXaxis()->GetXmin() + m_histo.GetXaxis()->GetBinWidth(1) / 100.;
          if (_var >= m_histo.GetXaxis()->GetXmax()) _var = m_histo.GetXaxis()->GetXmax() - m_histo.GetXaxis()->GetBinWidth(m_histo.GetNbinsX()) / 100.;
          _bin = m_histo.FindFixBin(_var);
      }
      if( !m_interpolate){
          _val = m_histo.GetBinContent(_bin);
      }else{
          _val = m_histo.Interpolate( _var);
      }
      return _val;
    }
    const char* branchName()const{
        return m_finalBranchName.Data();
    }
    const TString sourceHisto() const{
        return m_infoMapSource;
    }
private:
    TH1D    m_histo;
    TString m_finalBranchName;
    TString m_infoMapSource;
    bool m_interpolate;
};

class BSTH1DHistoAdder {
public:
    BSTH1DHistoAdder( vector<TH1D> & h , 
                    TString _finalBranchName, 
                    bool _interpolate, 
                    TString _infoMapSource ) : m_histo(h) , m_finalBranchName(_finalBranchName), m_interpolate(_interpolate) , m_infoMapSource(_infoMapSource) {}
    BSTH1DHistoAdder( vector<TH1D> && h , 
                    TString _finalBranchName, 
                    bool _interpolate, 
                    TString _infoMapSource , 
                    TString _option ) : m_histo(h) , m_finalBranchName(_finalBranchName), m_interpolate(_interpolate) , m_infoMapSource(_infoMapSource) {}                       
    BSTH1DHistoAdder( const BSTH1DHistoAdder&& other) : m_histo(other.m_histo), m_finalBranchName(other.m_finalBranchName), m_interpolate(other.m_interpolate) , m_infoMapSource(other.m_infoMapSource){}
    BSTH1DHistoAdder( const BSTH1DHistoAdder& other)  : m_histo(other.m_histo), m_finalBranchName(other.m_finalBranchName), m_interpolate(other.m_interpolate) , m_infoMapSource(other.m_infoMapSource){}
    ROOT::VecOps::RVec< double > operator()(double _variable) { 
        ROOT::VecOps::RVec< double > _values = {};
        for( int i = 0; i < m_histo.size(); ++i){        
            double _val = 1.0;
            double _var = _variable;
            int _bin = m_histo[i].FindFixBin(_var);
            if (m_histo[i].IsBinUnderflow(_bin) || m_histo[i].IsBinOverflow(_bin)) {
                if (_var <= m_histo[i].GetXaxis()->GetXmin()) _var = m_histo[i].GetXaxis()->GetXmin() + m_histo[i].GetXaxis()->GetBinWidth(1) / 100.;
                if (_var >= m_histo[i].GetXaxis()->GetXmax()) _var = m_histo[i].GetXaxis()->GetXmax() - m_histo[i].GetXaxis()->GetBinWidth(m_histo[i].GetNbinsX()) / 100.;
                _bin = m_histo[i].FindFixBin(_var);
            }
            if( !m_interpolate){
                _val = m_histo[i].GetBinContent(_bin);
            }else{
                _val = m_histo[i].Interpolate( _var);
            }
            _values.push_back( _val);
        }
        return _values;
    }
    const char* branchName()const{
        return m_finalBranchName.Data();
    }
    const TString sourceHisto() const{
        return m_infoMapSource;
    }
    const int nHistos() const{ 
        return m_histo.size();
    }    
private:
    vector<TH1D>  m_histo;
    TString m_finalBranchName;
    TString m_infoMapSource;
    bool m_interpolate;
};


class TH2DHistAdder {
public:
    TH2DHistAdder( TH2D & h , 
                    TString _finalBranchName, 
                    bool _interpolate, 
                    TString _infoMapSource , 
                    TString _option ) : m_histo(h) , m_finalBranchName(_finalBranchName), m_interpolate(_interpolate) , m_infoMapSource(_infoMapSource), m_option(_option) {}
    TH2DHistAdder( const TH2DHistAdder&& other) : m_histo(other.m_histo), m_finalBranchName(other.m_finalBranchName), m_interpolate(other.m_interpolate) , m_infoMapSource(other.m_infoMapSource), m_option(other.m_option){}
    TH2DHistAdder( const TH2DHistAdder& other)  : m_histo(other.m_histo), m_finalBranchName(other.m_finalBranchName), m_interpolate(other.m_interpolate) , m_infoMapSource(other.m_infoMapSource), m_option(other.m_option){}
    double operator()(double _variableX, double _variableY) { 
        double _val = 1.0;
        double _varX = _variableX;
        double _varY = _variableY;
        int _bin = m_histo.FindFixBin(_varX, _varY);
        if (m_histo.IsBinUnderflow(_bin) || m_histo.IsBinOverflow(_bin)) {
            if (_varX <= m_histo.GetXaxis()->GetXmin()) _varX = m_histo.GetXaxis()->GetXmin() + m_histo.GetXaxis()->GetBinWidth(1) / 100.;
            if (_varX >= m_histo.GetXaxis()->GetXmax()) _varX = m_histo.GetXaxis()->GetXmax() - m_histo.GetXaxis()->GetBinWidth(m_histo.GetNbinsX()) / 100.;
            if (_varY <= m_histo.GetYaxis()->GetXmin()) _varY = m_histo.GetYaxis()->GetXmin() + m_histo.GetYaxis()->GetBinWidth(1) / 100.;
            if (_varY >= m_histo.GetYaxis()->GetXmax()) _varY = m_histo.GetYaxis()->GetXmax() - m_histo.GetYaxis()->GetBinWidth(m_histo.GetNbinsY()) / 100.;
            _bin = m_histo.FindFixBin(_varX, _varY);
        }
        if (m_interpolate) {
            _val = m_histo.Interpolate(_varX, _varY);
            if (_val == 0) {
                _val = m_histo.GetBinContent(_bin);
                // MessageSvc::Warning("GetHistogramVal", (TString) "Cannot interpolate bin " + to_string(_bin) + " with (X,Y) = (" + to_string(_varX) + "," + to_string(_varY) + "), assigning bin content", to_string(_val));
            }
        } else _val = m_histo.GetBinContent(_bin);
        if (m_option.Contains("effr") && ((_val < 0) || (_val > 1))){
            MessageSvc::Warning("GetHistogramVal", m_histo.GetName(), "bin " + to_string(_bin) + " with (X,Y) = (" + to_string(_varX) + "," + to_string(_varY) + ") and value =", to_string(_val));
            if( _val <0) _val = 0.;
            if( _val >1) _val = 1.;
        }
        if (m_option.Contains("ratior") && (_val < 0)){
            MessageSvc::Warning("GetHistogramVal", m_histo.GetName(), "bin " + to_string(_bin) + " with (X,Y) = (" + to_string(_varX) + "," + to_string(_varY) + ") and value =", to_string(_val));        
            _val = 0;
        }
        return _val;
    }
    const char* branchName()const{
        return m_finalBranchName.Data();
    }
    const TString sourceHisto() const{
        return m_infoMapSource;
    }
private:
    TH2D    m_histo;
    TString m_finalBranchName;
    TString m_infoMapSource;
    bool m_interpolate;
    TString m_option ;
};

//
class TH2DHistAdderL0EBremSplit {
public:
    TH2DHistAdderL0EBremSplit( TH2D & h0 ,  TH2D & h1, 
                    TString _finalBranchName, 
                    bool _interpolate, 
                    TString _infoMapSource , 
                    TString _option ) : m_histo_0(h0) , m_histo_1(h1), m_finalBranchName(_finalBranchName), m_interpolate(_interpolate) , m_infoMapSource(_infoMapSource), m_option(_option) {}
    TH2DHistAdderL0EBremSplit( const TH2DHistAdderL0EBremSplit&& other) : m_histo_0(other.m_histo_0), m_histo_1(other.m_histo_1), m_finalBranchName(other.m_finalBranchName), m_interpolate(other.m_interpolate) , m_infoMapSource(other.m_infoMapSource),  m_option(other.m_option){}
    TH2DHistAdderL0EBremSplit( const TH2DHistAdderL0EBremSplit& other)  : m_histo_0(other.m_histo_0), m_histo_1(other.m_histo_1), m_finalBranchName(other.m_finalBranchName), m_interpolate(other.m_interpolate) , m_infoMapSource(other.m_infoMapSource),  m_option(other.m_option){}
    double operator()(double _variableX, double _variableY, bool _hasBremAdded ) { 
        double _val = 1.0;
        double _varX = _variableX;
        double _varY = _variableY;
        if( _hasBremAdded==false ){
            /* The electron over which we attach is in Brem0 */
            int _bin = m_histo_0.FindFixBin(_varX, _varY);
            if (m_histo_0.IsBinUnderflow(_bin) || m_histo_0.IsBinOverflow(_bin)) {
                if (_varX <= m_histo_0.GetXaxis()->GetXmin()) _varX = m_histo_0.GetXaxis()->GetXmin() + m_histo_0.GetXaxis()->GetBinWidth(1) / 100.;
                if (_varX >= m_histo_0.GetXaxis()->GetXmax()) _varX = m_histo_0.GetXaxis()->GetXmax() - m_histo_0.GetXaxis()->GetBinWidth(m_histo_0.GetNbinsX()) / 100.;
                if (_varY <= m_histo_0.GetYaxis()->GetXmin()) _varY = m_histo_0.GetYaxis()->GetXmin() + m_histo_0.GetYaxis()->GetBinWidth(1) / 100.;
                if (_varY >= m_histo_0.GetYaxis()->GetXmax()) _varY = m_histo_0.GetYaxis()->GetXmax() - m_histo_0.GetYaxis()->GetBinWidth(m_histo_0.GetNbinsY()) / 100.;
                _bin = m_histo_0.FindFixBin(_varX, _varY);
            }
            if (m_interpolate) {
                _val = m_histo_0.Interpolate(_varX, _varY);
                if (_val == 0){
                    _val = m_histo_0.GetBinContent(_bin);
                    // MessageSvc::Warning("GetHistogramVal", (TString) "Cannot interpolate bin " + to_string(_bin) + " with (X,Y) = (" + to_string(_varX) + "," + to_string(_varY) + "), assigning bin content", to_string(_val));
                }
            } else _val = m_histo_0.GetBinContent(_bin);
            if (m_option.Contains("effr") && ((_val < 0) || (_val > 1))){
                MessageSvc::Warning("GetHistogramVal (0G)", m_histo_0.GetName(), "bin " + to_string(_bin) + " with (X,Y) = (" + to_string(_varX) + "," + to_string(_varY) + ") and value =", to_string(_val));
                if( _val < 0) _val = 0.;
                if( _val >1 ) _val = 1.;
            }
            if (m_option.Contains("ratior") && (_val < 0)){
                MessageSvc::Warning("GetHistogramVal (0G)", m_histo_0.GetName(), "bin " + to_string(_bin) + " with (X,Y) = (" + to_string(_varX) + "," + to_string(_varY) + ") and value =", to_string(_val));        
                if( _val <0) _val = 0.;
            }
        }else{ 
            /* The electron over which we attach is in Brem1 */
            int _bin = m_histo_1.FindFixBin(_varX, _varY);
            if (m_histo_1.IsBinUnderflow(_bin) || m_histo_1.IsBinOverflow(_bin)) {
                if (_varX <= m_histo_1.GetXaxis()->GetXmin()) _varX = m_histo_1.GetXaxis()->GetXmin() + m_histo_1.GetXaxis()->GetBinWidth(1) / 100.;
                if (_varX >= m_histo_1.GetXaxis()->GetXmax()) _varX = m_histo_1.GetXaxis()->GetXmax() - m_histo_1.GetXaxis()->GetBinWidth(m_histo_1.GetNbinsX()) / 100.;
                if (_varY <= m_histo_1.GetYaxis()->GetXmin()) _varY = m_histo_1.GetYaxis()->GetXmin() + m_histo_1.GetYaxis()->GetBinWidth(1) / 100.;
                if (_varY >= m_histo_1.GetYaxis()->GetXmax()) _varY = m_histo_1.GetYaxis()->GetXmax() - m_histo_1.GetYaxis()->GetBinWidth(m_histo_1.GetNbinsY()) / 100.;
                _bin = m_histo_1.FindFixBin(_varX, _varY);
            }
            if (m_interpolate) {
                _val = m_histo_1.Interpolate(_varX, _varY);
                if (_val == 0) {
                    _val = m_histo_1.GetBinContent(_bin);
                    // MessageSvc::Warning("GetHistogramVal", (TString) "Cannot interpolate bin " + to_string(_bin) + " with (X,Y) = (" + to_string(_varX) + "," + to_string(_varY) + "), assigning bin content", to_string(_val));
                }
            } else _val = m_histo_1.GetBinContent(_bin);
            if (m_option.Contains("effr") && ((_val < 0) || (_val > 1))){
                MessageSvc::Warning("GetHistogramVal (1G)", m_histo_1.GetName(), "bin " + to_string(_bin) + " with (X,Y) = (" + to_string(_varX) + "," + to_string(_varY) + ") and value =", to_string(_val));
                if(_val <0) _val =0.;
                if(_val >0) _val =1.;
            }
            if (m_option.Contains("ratior") && (_val < 0)){
                MessageSvc::Warning("GetHistogramVal (1G)", m_histo_1.GetName(), "bin " + to_string(_bin) + " with (X,Y) = (" + to_string(_varX) + "," + to_string(_varY) + ") and value =", to_string(_val));                    
                if( _val <0) _val =0.;
            }
        }
        return _val;
    }
    const char* branchName()const{
        return m_finalBranchName.Data();
    }
    const TString sourceHisto() const{
        return m_infoMapSource;
    }
private:
    TH2D    m_histo_0;
    TH2D    m_histo_1; 
    TString m_finalBranchName;
    TString m_infoMapSource;
    bool m_interpolate;
    TString m_option ;
};


/**
* \class BSTH2DHistAdder
* \brief Functor class to attach weights from vector<TH2D> for bootstrapped 2D corrections ( used for L0 corrections )
*/
class BSTH2DHistAdder {
public:
    BSTH2DHistAdder( vector<TH2D> & h , 
                    TString _finalBranchName, 
                    bool _interpolate, 
                    TString _infoMapSource , 
                    TString _option ) : m_histo(h) , m_finalBranchName(_finalBranchName), m_interpolate(_interpolate) , m_infoMapSource(_infoMapSource) {}
    BSTH2DHistAdder( vector<TH2D> && h , 
                    TString _finalBranchName, 
                    bool _interpolate, 
                    TString _infoMapSource , 
                    TString _option ) : m_histo(h) , m_finalBranchName(_finalBranchName), m_interpolate(_interpolate) , m_infoMapSource(_infoMapSource) {}                    
    BSTH2DHistAdder( const BSTH2DHistAdder&& other) : m_histo(other.m_histo), m_finalBranchName(other.m_finalBranchName), m_interpolate(other.m_interpolate) , m_infoMapSource(other.m_infoMapSource){}
    BSTH2DHistAdder( const BSTH2DHistAdder& other)  : m_histo(other.m_histo), m_finalBranchName(other.m_finalBranchName), m_interpolate(other.m_interpolate) , m_infoMapSource(other.m_infoMapSource){}
    ROOT::VecOps::RVec< double > operator()(double _variableX, double _variableY) { 
        ROOT::VecOps::RVec< double > _values = {};
        for( int i = 0; i < m_histo.size(); ++i){
            double _val = 1.0;
            double _varX = _variableX;
            double _varY = _variableY;
            int _bin = m_histo[i].FindFixBin(_varX, _varY);
            if (m_histo[i].IsBinUnderflow(_bin) || m_histo[i].IsBinOverflow(_bin)) {
                if (_varX <= m_histo[i].GetXaxis()->GetXmin()) _varX = m_histo[i].GetXaxis()->GetXmin() + m_histo[i].GetXaxis()->GetBinWidth(1) / 100.;
                if (_varX >= m_histo[i].GetXaxis()->GetXmax()) _varX = m_histo[i].GetXaxis()->GetXmax() - m_histo[i].GetXaxis()->GetBinWidth(m_histo[i].GetNbinsX()) / 100.;
                if (_varY <= m_histo[i].GetYaxis()->GetXmin()) _varY = m_histo[i].GetYaxis()->GetXmin() + m_histo[i].GetYaxis()->GetBinWidth(1) / 100.;
                if (_varY >= m_histo[i].GetYaxis()->GetXmax()) _varY = m_histo[i].GetYaxis()->GetXmax() - m_histo[i].GetYaxis()->GetBinWidth(m_histo[i].GetNbinsY()) / 100.;
                _bin = m_histo[i].FindFixBin(_varX, _varY);
            }
            if (m_interpolate) {
                _val = m_histo[i].Interpolate(_varX, _varY);
                if (_val == 0) {
                    _val = m_histo[i].GetBinContent(_bin);
                    // MessageSvc::Warning("GetHistogramVal", (TString) "Cannot interpolate bin " + to_string(_bin) + " with (X,Y) = (" + to_string(_varX) + "," + to_string(_varY) + "), assigning bin content", to_string(_val));
                }
            } else _val = m_histo[i].GetBinContent(_bin);
            if (m_option.Contains("effr") && ((_val < 0) || (_val > 1))) MessageSvc::Warning("GetHistogramVal", m_histo[i].GetName(), "bin " + to_string(_bin) + " with (X,Y) = (" + to_string(_varX) + "," + to_string(_varY) + ") and value =", to_string(_val));
            if (m_option.Contains("ratior") && (_val < 0)) MessageSvc::Warning("GetHistogramVal", m_histo[i].GetName(), "bin " + to_string(_bin) + " with (X,Y) = (" + to_string(_varX) + "," + to_string(_varY) + ") and value =", to_string(_val));        
            _values.push_back( _val);
        }
        return _values;    
    }
    const char* branchName()const{
        return m_finalBranchName.Data();
    }
    const TString sourceHisto() const{
        return m_infoMapSource;
    }
    const int nHistos() const{ 
        return m_histo.size();
    }
private:
    vector<TH2D>  m_histo;
    TString m_finalBranchName;
    TString m_infoMapSource;
    bool m_interpolate;
    TString m_option ;
};


/**
* \class TH3FHistoAdder
* \brief Functor class to attach weights from TH3F object ( used for TRK corrections )
*/
class TH3FHistoAdder{
    public:
    TH3FHistoAdder( TH3F & h , 
                    TString _finalBranchName, 
                    bool _interpolate, 
                    TString _infoMapSource ) : m_histo(h) , m_finalBranchName(_finalBranchName), m_interpolate(_interpolate) , m_infoMapSource(_infoMapSource) {}
    TH3FHistoAdder( const TH3FHistoAdder&& other) : m_histo(other.m_histo), m_finalBranchName(other.m_finalBranchName), m_interpolate(other.m_interpolate) , m_infoMapSource(other.m_infoMapSource){}
    TH3FHistoAdder( const TH3FHistoAdder& other)  : m_histo(other.m_histo), m_finalBranchName(other.m_finalBranchName), m_interpolate(other.m_interpolate) , m_infoMapSource(other.m_infoMapSource){}
    double operator()(double _varX, double _varY, double _varZ) { 
      double _val = 1.0;
      double _err = 0;

      double _x = _varX;
      double _y = _varY;
      double _z = _varZ;
      int _bin = m_histo.FindFixBin(_x, _y, _z);      
      auto _xAxis = m_histo.GetXaxis(); auto _xmin = _xAxis->GetXmin(); auto _xmax = _xAxis->GetXmax(); auto _nbins_X = m_histo.GetNbinsX();
      auto _yAxis = m_histo.GetYaxis(); auto _ymin = _yAxis->GetXmin(); auto _ymax = _yAxis->GetXmax(); auto _nbins_Y = m_histo.GetNbinsY();
      auto _zAxis = m_histo.GetZaxis(); auto _zmin = _zAxis->GetXmin(); auto _zmax = _zAxis->GetXmax(); auto _nbins_Z = m_histo.GetNbinsZ();
      if (m_histo.IsBinUnderflow(_bin) || m_histo.IsBinOverflow(_bin)) {
          if( _x <= _xmin)  _x = _xmin + _xAxis->GetBinWidth(1)/100.;
          if( _y <= _ymin)  _y = _ymin + _yAxis->GetBinWidth(1)/100.;
          if( _z <= _zmin)  _z = _zmin + _zAxis->GetBinWidth(1)/100.;

          if( _x >= _xmax)  _x = _xmax - _xAxis->GetBinWidth( _nbins_X)/100.;
          if( _y >= _ymax)  _y = _ymax - _yAxis->GetBinWidth( _nbins_Y)/100.;
          if( _z >= _zmax)  _z = _zmax - _zAxis->GetBinWidth( _nbins_Z)/100.;
          _bin = m_histo.FindFixBin(_x, _y, _z);
      }
      if( !m_interpolate){
          _val = m_histo.GetBinContent(_bin);
          _err = m_histo.GetBinError(_bin);
      }else{
          _val = m_histo.Interpolate(_x,_y,_z);
          _err = 0.;
      }
      //TRACKING MAPS !!!
      return _val;
    }
    const char* branchName()const{
        return m_finalBranchName.Data();
    }
    const TString sourceHisto() const{
        return m_infoMapSource;
    }
private:
    TH3F    m_histo;
    TString m_finalBranchName;
    TString m_infoMapSource;
    bool m_interpolate;
};

class BSTH3FHistoAdder{
    public:
    BSTH3FHistoAdder( vector<TH3F> & h , 
                    TString _finalBranchName, 
                    bool _interpolate, 
                    TString _infoMapSource ) : m_histo(h) , m_finalBranchName(_finalBranchName), m_interpolate(_interpolate) , m_infoMapSource(_infoMapSource) {}
    BSTH3FHistoAdder( const BSTH3FHistoAdder&& other) : m_histo(other.m_histo), m_finalBranchName(other.m_finalBranchName), m_interpolate(other.m_interpolate) , m_infoMapSource(other.m_infoMapSource){}
    BSTH3FHistoAdder( const BSTH3FHistoAdder& other)  : m_histo(other.m_histo), m_finalBranchName(other.m_finalBranchName), m_interpolate(other.m_interpolate) , m_infoMapSource(other.m_infoMapSource){}
    ROOT::VecOps::RVec< double > operator()(double _varX, double _varY, double _varZ) { 
        ROOT::VecOps::RVec< double > _values = {};        
        for ( int idx = 0; idx < m_histo.size(); ++idx ){
            double _val = 1.0;
            double _x = _varX;
            double _y = _varY;
            double _z = _varZ;
            int _bin = m_histo[idx].FindFixBin(_x, _y, _z);      
            auto _xAxis = m_histo[idx].GetXaxis(); auto _xmin = _xAxis->GetXmin(); auto _xmax = _xAxis->GetXmax(); auto _nbins_X = m_histo[idx].GetNbinsX();
            auto _yAxis = m_histo[idx].GetYaxis(); auto _ymin = _yAxis->GetXmin(); auto _ymax = _yAxis->GetXmax(); auto _nbins_Y = m_histo[idx].GetNbinsY();
            auto _zAxis = m_histo[idx].GetZaxis(); auto _zmin = _zAxis->GetXmin(); auto _zmax = _zAxis->GetXmax(); auto _nbins_Z = m_histo[idx].GetNbinsZ();
            if (m_histo[idx].IsBinUnderflow(_bin) || m_histo[idx].IsBinOverflow(_bin)) {
                if( _x <= _xmin)  _x = _xmin + _xAxis->GetBinWidth(1)/100.;
                if( _y <= _ymin)  _y = _ymin + _yAxis->GetBinWidth(1)/100.;
                if( _z <= _zmin)  _z = _zmin + _zAxis->GetBinWidth(1)/100.;

                if( _x >= _xmax)  _x = _xmax - _xAxis->GetBinWidth( _nbins_X)/100.;
                if( _y >= _ymax)  _y = _ymax - _yAxis->GetBinWidth( _nbins_Y)/100.;
                if( _z >= _zmax)  _z = _zmax - _zAxis->GetBinWidth( _nbins_Z)/100.;
                _bin = m_histo[idx].FindFixBin(_x, _y, _z);
            }
            if( !m_interpolate){
                _val = m_histo[idx].GetBinContent(_bin);
            }else{
                _val = m_histo[idx].Interpolate(_x, _y, _z);
            }
            _values.push_back(_val);
        }
        return _values;
    }
    const char* branchName()const{
        return m_finalBranchName.Data();
    }
    const TString sourceHisto() const{
        return m_infoMapSource;
    }
private:
    vector<TH3F>    m_histo;
    TString m_finalBranchName;
    TString m_infoMapSource;
    bool m_interpolate;
};


class Q2SmearNormalizer{
    public:    
    Q2SmearNormalizer( const Prj & _prj , const Year & _year , TString _finalBranchName) : m_project(_prj)  , m_finalBranchName(_finalBranchName) , m_year(_year) { Init(); };
    Q2SmearNormalizer( const Q2SmearNormalizer&& other) : m_project(other.m_project), m_year(other.m_year), m_finalBranchName(other.m_finalBranchName){ Init();}
    Q2SmearNormalizer( const Q2SmearNormalizer& other)  : m_project(other.m_project), m_year(other.m_year), m_finalBranchName(other.m_finalBranchName){ Init();}
    bool HasLRSigmaScale()const{ return m_hasLRSigmaScale;}
    double operator()( double jps_mass_reco, double jps_mass_true, int bkgcat , double e1_brems, double e2_brems , int year  , double jpsi_reconstructedMASS, double jpsi_TRUE_MASS)const{
        if(jps_mass_true< -0.1){
            return 1.; //this is an invalid case where the porting from MCDT has not work, those events must be always rejected when cutting on q2 smeared values!
        }
        //Use the year passed at construction
        Year useYear = from_int(year);
        
        int bremCat = (int)e1_brems + (int)e2_brems;
        bremCat = bremCat >=2 ? 2 : bremCat;
        if( bremCat <0 || bremCat >2) MessageSvc::Error("Q2SmearNormalizer Invalid","","EXIT_FAILURE");        
        double _trueMass = jps_mass_true;
        double _recoMass = jps_mass_reco;
        if(bkgcat==60){
            //BKGCAT60 ==> trueM == recoM;
            _trueMass = _recoMass;
        }

        if( bremCat <0 || bremCat >2) MessageSvc::Error("Q2SmearNormalizer Invalid","","EXIT_FAILURE");               
        if(!m_hasLRSigmaScale){
            return 1.;   //weight is 1 so we don't normalize it        
        }else{         
            // if( _recoMass - _trueMass - (m_massMCFit.at(useYear).at(bremCat) - PDG::Mass::JPs ) > 0 ){
            if( jpsi_reconstructedMASS - jpsi_TRUE_MASS - (m_massMCFit.at(useYear).at(bremCat) - PDG::Mass::JPs ) > 0 ){
                //if above 0 , the integral is proportional to 
                //double xscaleL = sqrt(2.) 
               return TMath::Power( m_sigmaScaleR.at(useYear).at(bremCat)/m_sigmaScaleL.at(useYear).at(bremCat), 2);
               //return m_sigmaScaleR.at(useYear).at(bremCat)/m_sigmaScaleL.at(useYear).at(bremCat);               
            }else{
                // if ( m(Reco) - m(True) - ( muMC - PDG::JPs)) <0 use sigmaScale Left
                return 1.;
            }
        }
        return 1.;
    }
    void Init(){
        LoadSigmaScalePars();
        LoadMassMCPars();
        LoadMassShiftPars();
        return;
    }
    void Print(){
        MessageSvc::Info("Q2Smearing Normalization parameters loaded for ", TString(branchName()));
        for( auto year : m_loadYears){
            cout<< BLUE << "#) Setups for year " << to_string(year) << RESET<< endl;
            for( int i = 0 ; i < 3; ++i){
                cout<< CYAN << "          "<< i << "G" <<  endl;
                if(m_hasLRSigmaScale){
                    cout<< CYAN << "s_scaleL  | "<< m_sigmaScaleL.at(year).at(i)<< endl;
                    cout<< CYAN << "s_scaleR  | "<< m_sigmaScaleR.at(year).at(i)<< endl;
                }else{
                    cout<< CYAN << "s_scale   | "<< m_sigmaScale.at(year).at(i)<< endl;
                }
                cout<< CYAN << "m_shift  | "<< m_massShift.at(year).at(i)<< endl;
                cout<< CYAN << "m_MCFit  | "<< m_massMCFit.at(year).at(i)<< RESET<<endl;
            }
        }
        return;
    }
    void LoadSigmaScalePars(){
        TString _file = "";
        if(SettingDef::Weight::q2SmearFileTag == ""){
            _file = TString::Format("%s/data/q2smearing/v%s/q2smearing_%s.yaml", getenv("ANASYS"), SettingDef::Tuple::gngVer.Data(), to_string(m_project).Data());
        }else{
            _file = TString::Format("%s/data/q2smearing/v%s/q2smearing_%s_%s.yaml", getenv("ANASYS"), SettingDef::Tuple::gngVer.Data(), SettingDef::Weight::q2SmearFileTag.Data(), to_string(m_project).Data());
        }
        MessageSvc::Info("Q2SmearNormalization File parsed = ", _file);
        YAML::Node _parser = YAML::LoadFile(_file.Data());
        if (_parser.IsNull()) MessageSvc::Error("LoadSigmaScalePars", (TString) "Invalid", _file, "parser", "EXIT_FAILURE");        
        for( auto _year : m_loadYears){
            YAML::Node _yearNode = _parser[to_string(_year)];
            if( _yearNode["sScaleL"] &&  _yearNode["sScaleR"]){
                m_sigmaScaleL[_year] = _yearNode["sScaleL"].as<vector<double> > ();            
                m_sigmaScaleR[_year] = _yearNode["sScaleR"].as<vector<double> > ();         
                m_sigmaScale[_year] =  {};   
                m_hasLRSigmaScale = true; 
            }else if( _yearNode["sScale"]){
                //Old yamls, only 1 sigma scale value
                m_sigmaScale[_year] = _yearNode["sScale"].as<vector<double> > ();            
                m_sigmaScaleL[_year] =  {};     
                m_sigmaScaleR[_year] =  {};     
                m_hasLRSigmaScale = false;         
            }else{
                MessageSvc::Error("CANNOT LOAD INPUT FILE For S-Scales",  _file);
            }
        }
        for( auto year : m_loadYears){
            if( m_hasLRSigmaScale){ 
                if( m_sigmaScaleL[year].size() != 3) MessageSvc::Error("Invalid init LoadSigmaScalePars (L)","","EXIT_FAILURE");
                if( m_sigmaScaleR[year].size() != 3) MessageSvc::Error("Invalid init LoadSigmaScalePars (R)","","EXIT_FAILURE");
            }else{
                if( m_sigmaScale[year].size() != 3) MessageSvc::Error("Invalid init LoadSigmaScalePars","","EXIT_FAILURE");
            }
        }
        return;
    }
    void LoadMassMCPars(){
        TString _file = "";
        if(SettingDef::Weight::q2SmearFileTag == ""){
            _file = TString::Format("%s/data/q2smearing/v%s/q2smearing_%s.yaml", getenv("ANASYS"), SettingDef::Tuple::gngVer.Data(), to_string(m_project).Data());
        }else{
            _file = TString::Format("%s/data/q2smearing/v%s/q2smearing_%s_%s.yaml", getenv("ANASYS"), SettingDef::Tuple::gngVer.Data(), SettingDef::Weight::q2SmearFileTag.Data(), to_string(m_project).Data());
        }
        YAML::Node _parser = YAML::LoadFile(_file.Data());
        if (_parser.IsNull()) MessageSvc::Error("LoadMassMCPars", (TString) "Invalid", _file, "parser", "EXIT_FAILURE");   
        for( auto & year : m_loadYears){
            YAML::Node _yearNode = _parser[to_string(year)];
            m_massMCFit[year] = _yearNode["mMC"].as<vector<double> > ();
        }         
        for( auto year : m_loadYears){
            if( m_massMCFit[year].size() != 3) MessageSvc::Error("Invalid init LoadMassMCPars","","EXIT_FAILURE");
        }
        return;
    }
    void LoadMassShiftPars(){
        TString _file = "";
        if(SettingDef::Weight::q2SmearFileTag == ""){
            _file = TString::Format("%s/data/q2smearing/v%s/q2smearing_%s.yaml", getenv("ANASYS"), SettingDef::Tuple::gngVer.Data(), to_string(m_project).Data());
        }else{
            _file = TString::Format("%s/data/q2smearing/v%s/q2smearing_%s_%s.yaml", getenv("ANASYS"), SettingDef::Tuple::gngVer.Data(), SettingDef::Weight::q2SmearFileTag.Data(), to_string(m_project).Data());
        }
        YAML::Node _parser = YAML::LoadFile(_file.Data());
        if (_parser.IsNull()) MessageSvc::Error("LoadMassShiftPars", (TString) "Invalid", _file, "parser", "EXIT_FAILURE");        
        for( auto &  year : m_loadYears){
            YAML::Node _yearNode = _parser[to_string(year)];
            m_massShift[year] = _yearNode["mShift"].as<vector<double> >();
        }
        for( auto year : m_loadYears){
            if( m_massShift[year].size() != 3) MessageSvc::Error("Invalid init LoadMassShiftPars",to_string(year),"EXIT_FAILURE");
        }
        return;    
    }
    const char* branchName()const{
        return m_finalBranchName.Data();
    }
    private : 
    Prj m_project ; 
    Year m_year;
    vector< Year> m_loadYears{  Year::Y2011 , Year::Y2012 , Year::Y2015, Year::Y2016 , Year::Y2017, Year::Y2018 };
    TString m_finalBranchName;
    map< Year, std::vector<double>> m_sigmaScale; //for yamls having the sigmaScale  unique
    map< Year, std::vector<double>> m_sigmaScaleL;//for yamls having the sigmaScaleL unique
    map< Year, std::vector<double>> m_sigmaScaleR;//for yamls having the sigmaScaleR unique

    map< Year, std::vector<double>> m_massMCFit; 
    map< Year, std::vector<double>> m_massShift; 
    bool m_hasLRSigmaScale = false; 
};

/**
* \class Q2SmearCorrection
* \brief Functor class to attach Q2 smearing to EE ntuples using Yaml files saving parameters to use
*/
class Q2SmearCorrection{
    public:    
    Q2SmearCorrection( const Prj & _prj , const Year & _year , TString _finalBranchName) : m_project(_prj)  , m_finalBranchName(_finalBranchName) , m_year(_year) { Init(); };
    Q2SmearCorrection( const Q2SmearCorrection&& other) : m_project(other.m_project), m_year(other.m_year), m_finalBranchName(other.m_finalBranchName){ Init();}
    Q2SmearCorrection( const Q2SmearCorrection& other)  : m_project(other.m_project), m_year(other.m_year), m_finalBranchName(other.m_finalBranchName){ Init();}
    bool HasLRSigmaScale()const{ return m_hasLRSigmaScale;}
    double operator()( double jps_mass_reco, double jps_mass_true, int bkgcat , double e1_brems, double e2_brems , int year  , double jpsi_reconstructedMASS, double jpsi_TRUE_MASS)const{
        if(jps_mass_true< -0.1){
            return -1.; //this is an invalid case where the porting from MCDT has not work, those events must be always rejected when cutting on q2 smeared values!
        }
        //Use the year passed at construction
        Year useYear = from_int(year);
        int bremCat = (int)e1_brems + (int)e2_brems;
        bremCat = bremCat >=2 ? 2 : bremCat;
        if( bremCat <0 || bremCat >2) MessageSvc::Error("Q2SmearCorrection Invalid","","EXIT_FAILURE");        
        double _trueMass = jps_mass_true;
        double _recoMass = jps_mass_reco;
        if(bkgcat==60){
            //BKGCAT60 ==> trueM == recoM;
            _trueMass = _recoMass;
        }
        // now :  left and right logic to use 
        // s_scale :  sigma Data / sigma MC 
        // There is an extra sigma Factor on top to account for
        // recoMass - ( trueMass - ( FSR shift) ) - massMCFited is with a sigma the distance from the mean has to
        if( !m_hasLRSigmaScale){
            double  _correctedJPsMass = _trueMass 
                                + m_sigmaScale.at(useYear).at(bremCat) * ( _recoMass - _trueMass)
                                +  m_massShift.at(useYear).at(bremCat) + 
                                ( 1. - m_sigmaScale.at(useYear).at(bremCat)) * (m_massMCFit.at(useYear).at(bremCat) - PDG::Mass::JPs);     
            return _correctedJPsMass;
        }else{   
            //The Left-Right setup  [ not used ]
            if( jpsi_reconstructedMASS - jpsi_TRUE_MASS - (m_massMCFit.at(useYear).at(bremCat) - PDG::Mass::JPs ) > 0 ){
                //if( _recoMass - _trueMass - (m_massMCFit.at(useYear).at(bremCat) - PDG::Mass::JPs ) > 0 ){
                double  _correctedJPsMass = _trueMass 
                                    + m_sigmaScaleR.at(useYear).at(bremCat) * ( _recoMass - _trueMass) 
                                    +  m_massShift.at(useYear).at(bremCat) + 
                                    ( 1. - m_sigmaScaleR.at(useYear).at(bremCat)) * (m_massMCFit.at(useYear).at(bremCat) - PDG::Mass::JPs);     
                std::cout<< "SMEARING RIGHT"<< std::endl;
                std::cout<< "Mass(Original) : "<< _recoMass << std::endl;
                std::cout<< "Mass(Final) : "<<    _correctedJPsMass << std::endl;
                return _correctedJPsMass;            
            }else{
                // if ( m(Reco) - m(True) - ( muMC - PDG::JPs)) <0 use sigmaScale Left
                double  _correctedJPsMass = _trueMass 
                                    + m_sigmaScaleL.at(useYear).at(bremCat) * ( _recoMass - _trueMass) 
                                    +  m_massShift.at(useYear).at(bremCat) + 
                                    ( 1. - m_sigmaScaleL.at(useYear).at(bremCat)) * (m_massMCFit.at(useYear).at(bremCat) - PDG::Mass::JPs);     
                std::cout<< "SMEARING LEFT"<< std::endl;
                std::cout<< "Mass(Original) : "<< _recoMass << std::endl;
                std::cout<< "Mass(Final) : "<<    _correctedJPsMass << std::endl;
                return _correctedJPsMass;            
            }
        }
        return _recoMass;
    }
    void Init(){
        LoadSigmaScalePars();
        LoadMassMCPars();
        LoadMassShiftPars();
        return;
    }
    void Print(){
        MessageSvc::Info("Q2Smearing correction parameters loaded for ", TString(branchName()));
        for( auto year : m_loadYears){
            cout<< BLUE << "#) Setups for year " << to_string(year) << RESET<< endl;
            for( int i = 0 ; i < 3; ++i){
                cout<< CYAN << "          "<< i << "G" <<  endl;
                if(m_hasLRSigmaScale){
                    cout<< CYAN << "s_scaleL  | "<< m_sigmaScaleL.at(year).at(i)<< endl;
                    cout<< CYAN << "s_scaleR  | "<< m_sigmaScaleR.at(year).at(i)<< endl;
                }else{
                    cout<< CYAN << "s_scale   | "<< m_sigmaScale.at(year).at(i)<< endl;
                }
                cout<< CYAN << "m_shift  | "<< m_massShift.at(year).at(i)<< endl;
                cout<< CYAN << "m_MCFit  | "<< m_massMCFit.at(year).at(i)<< RESET<<endl;
            }
        }
        return;
    }
    void LoadSigmaScalePars(){
        TString _file = "";
        if(SettingDef::Weight::q2SmearFileTag == ""){
            _file = TString::Format("%s/data/q2smearing/v%s/q2smearing_%s.yaml", getenv("ANASYS"), SettingDef::Tuple::gngVer.Data(), to_string(m_project).Data());
        }else{
            _file = TString::Format("%s/data/q2smearing/v%s/q2smearing_%s_%s.yaml", getenv("ANASYS"), SettingDef::Tuple::gngVer.Data(), SettingDef::Weight::q2SmearFileTag.Data(), to_string(m_project).Data());
        }
        MessageSvc::Info("Q2Smear File parsed = ", _file);
        YAML::Node _parser = YAML::LoadFile(_file.Data());
        if (_parser.IsNull()) MessageSvc::Error("LoadSigmaScalePars", (TString) "Invalid", _file, "parser", "EXIT_FAILURE");        
        for( auto _year : m_loadYears){
            YAML::Node _yearNode = _parser[to_string(_year)];
            if( _yearNode["sScaleL"] &&  _yearNode["sScaleR"]){
                m_sigmaScaleL[_year] = _yearNode["sScaleL"].as<vector<double> > ();            
                m_sigmaScaleR[_year] = _yearNode["sScaleR"].as<vector<double> > ();         
                m_sigmaScale[_year] =  {};   
                m_hasLRSigmaScale = true; 
            }else if( _yearNode["sScale"]){
                //Old yamls, only 1 sigma scale value
                m_sigmaScale[_year] = _yearNode["sScale"].as<vector<double> > ();            
                m_sigmaScaleL[_year] =  {};     
                m_sigmaScaleR[_year] =  {};     
                m_hasLRSigmaScale = false;         
            }else{
                MessageSvc::Error("CANNOT LOAD INPUT FILE For S-Scales",  _file);
            }
        }
        for( auto year : m_loadYears){
            if( m_hasLRSigmaScale){ 
                if( m_sigmaScaleL[year].size() != 3) MessageSvc::Error("Invalid init LoadSigmaScalePars (L)","","EXIT_FAILURE");
                if( m_sigmaScaleR[year].size() != 3) MessageSvc::Error("Invalid init LoadSigmaScalePars (R)","","EXIT_FAILURE");
            }else{
                if( m_sigmaScale[year].size() != 3) MessageSvc::Error("Invalid init LoadSigmaScalePars","","EXIT_FAILURE");
            }
        }
        return;
    }
    void LoadMassMCPars(){
        TString _file = "";
        if(SettingDef::Weight::q2SmearFileTag == ""){
            _file = TString::Format("%s/data/q2smearing/v%s/q2smearing_%s.yaml", getenv("ANASYS"), SettingDef::Tuple::gngVer.Data(), to_string(m_project).Data());
        }else{
            _file = TString::Format("%s/data/q2smearing/v%s/q2smearing_%s_%s.yaml", getenv("ANASYS"), SettingDef::Tuple::gngVer.Data(), SettingDef::Weight::q2SmearFileTag.Data(), to_string(m_project).Data());
        }
        YAML::Node _parser = YAML::LoadFile(_file.Data());
        if (_parser.IsNull()) MessageSvc::Error("LoadMassMCPars", (TString) "Invalid", _file, "parser", "EXIT_FAILURE");   
        for( auto & year : m_loadYears){
            YAML::Node _yearNode = _parser[to_string(year)];
            m_massMCFit[year] = _yearNode["mMC"].as<vector<double> > ();
        }         
        for( auto year : m_loadYears){
            if( m_massMCFit[year].size() != 3) MessageSvc::Error("Invalid init LoadMassMCPars","","EXIT_FAILURE");
        }
        return;
    }
    void LoadMassShiftPars(){
        TString _file = "";
        if(SettingDef::Weight::q2SmearFileTag == ""){
            _file = TString::Format("%s/data/q2smearing/v%s/q2smearing_%s.yaml", getenv("ANASYS"), SettingDef::Tuple::gngVer.Data(), to_string(m_project).Data());
        }else{
            _file = TString::Format("%s/data/q2smearing/v%s/q2smearing_%s_%s.yaml", getenv("ANASYS"), SettingDef::Tuple::gngVer.Data(), SettingDef::Weight::q2SmearFileTag.Data(), to_string(m_project).Data());
        }
        YAML::Node _parser = YAML::LoadFile(_file.Data());
        if (_parser.IsNull()) MessageSvc::Error("LoadMassShiftPars", (TString) "Invalid", _file, "parser", "EXIT_FAILURE");        
        for( auto &  year : m_loadYears){
            YAML::Node _yearNode = _parser[to_string(year)];
            m_massShift[year] = _yearNode["mShift"].as<vector<double> >();
        }
        for( auto year : m_loadYears){
            if( m_massShift[year].size() != 3) MessageSvc::Error("Invalid init LoadMassShiftPars",to_string(year),"EXIT_FAILURE");
        }
        return;    
    }
    const char* branchName()const{
        return m_finalBranchName.Data();
    }
    private : 
    Prj m_project ; 
    Year m_year;
    vector< Year> m_loadYears{  Year::Y2011 , Year::Y2012 , Year::Y2015, Year::Y2016 , Year::Y2017, Year::Y2018 };
    TString m_finalBranchName;
    map< Year, std::vector<double>> m_sigmaScale; //for yamls having the sigmaScale  unique
    map< Year, std::vector<double>> m_sigmaScaleL;//for yamls having the sigmaScaleL unique
    map< Year, std::vector<double>> m_sigmaScaleR;//for yamls having the sigmaScaleR unique

    map< Year, std::vector<double>> m_massMCFit; 
    map< Year, std::vector<double>> m_massShift; 
    bool m_hasLRSigmaScale = false; 
};


//=====================================================================
/**
* \class PIDHistoAdder
* \brief Functor class to attach PID weights 
*
* call as PIDHistoAdder _myPIDBranch( _weightHolder, _branchName, option );
* 
* option is used to define the type of PID maps to be loaded
* all options that are understood by WeightHolder.cpp can be used
*  
* options:
*     - KDE:                   KDE PID maps (always no interp), electron F&C with interp
*     - KDE_ALTKERNEL:         KDE PID maps, alternative KDE kernels are used, electron F&C with interp
*     - KDE_ALTNTRACKS:        KDE PID maps, alternative nTracks Binning for KDE and electron F&C maps, electrons with interp
*     - KDE_NoOpt:             KDE PID maps, no Optimization of electron binning scheme
*     - "":                    default manual binning PID maps, electron F&C no interp
*     - interp:                default manual binning PID maps, electron F&C both with interp
*     - your-option-goes-here: feel like you miss something? contact @schmitse and add it yourself!
* 
*/
//=====================================================================
class PIDHistoAdder {
public:
    //=====================================================================
    // default constructor with WHolder, branchName, and option
    //=====================================================================    
    PIDHistoAdder(WeightHolder &_weightHolder,
          TString _finalBranchName,
          TString _option ) : m_weightHolder(_weightHolder), m_finalBranchName(_finalBranchName), m_option(_option), m_isInit(false) {
        if (!m_isInit) m_isInit = Init();
    }
    //=====================================================================
    // copy constructor that uses different branchName
    //=====================================================================    
    PIDHistoAdder(PIDHistoAdder &other,TString _finalBranchName) : 
        m_weightHolder(other.m_weightHolder), 
        m_Maps(other.m_Maps), 
        m_finalBranchName(_finalBranchName), 
        m_option(other.m_option), m_isInit(other.m_isInit) {      
            if(_DEBUGMESSAGE){      
                MessageSvc::Debug("PIDHistoAdder", "Copy from other PIDHistoAdder with option: " + m_option);
                MessageSvc::Debug("PIDHistoAdder", "Other BranchName: " + other.m_finalBranchName);
                MessageSvc::Debug("PIDHistoAdder", "This  BranchName: " + m_finalBranchName);
            }
        if (!m_isInit) m_isInit = Init();
    }
    //=====================================================================
    // move constructor
    //=====================================================================    
    PIDHistoAdder( PIDHistoAdder&& other) : 
        m_weightHolder(other.m_weightHolder), 
        m_Maps(other.m_Maps), 
        m_finalBranchName(other.m_finalBranchName), 
        m_option(other.m_option), 
        m_isInit(other.m_isInit) {
        if(_DEBUGMESSAGE){
            MessageSvc::Debug("PIDHistoAdder", "Move from other PIDHistoAdder with option: " + m_option);
            MessageSvc::Debug("PIDHistoAdder", "Other BranchName: " + other.m_finalBranchName);
            MessageSvc::Debug("PIDHistoAdder", "This  BranchName: " + m_finalBranchName);
        }
        if (!m_isInit) m_isInit = Init();
    }
    //=====================================================================
    // copy constructor
    //=====================================================================    
    PIDHistoAdder( const PIDHistoAdder& other)  : m_weightHolder(other.m_weightHolder), m_Maps(other.m_Maps), m_finalBranchName(other.m_finalBranchName), m_option(other.m_option), m_isInit(other.m_isInit) {
        if(_DEBUGMESSAGE){
            MessageSvc::Debug( "PIDHistoAdder", "Copy from other PIDHistoAdder with option: " + m_option);
        }
        if (!m_isInit) m_isInit = Init();
    }

    bool Init(){
        //=====================================================================
        // Init HistoAdder
        // Build m_Maps, map that stores PID maps of form TH1D, vector<TH2D>
        // m_Maps maps (PDG TRUEID, PDG ID, BREM) to the corresponding PID map
        //=====================================================================

        MessageSvc::Info(Color::Green, "PIDHistoAdder", "Init with Option: " + m_option);	
        auto _config = m_weightHolder.GetConfigHolder();
        auto _ana     = _config.GetAna();
        auto _project = _config.GetProject();
        
        //=====================================================================	
        // had Names for Maps
        //=====================================================================
        map<Prj, TString> _mapHad1NameM = {{Prj::RPhi, "K"}, {Prj::RKst, "K"},  {Prj::RK, "K"}};
        map<Prj, TString> _mapHad2NameM = {{Prj::RPhi, "K"}, {Prj::RKst, "Pi"}, {Prj::RK, ""}};    
        //=====================================================================
        // TRUEIDs for Maps
        //=====================================================================
        map<Prj, int>   _mapHad1ID      = {{Prj::RPhi, PDG::ID::K}, {Prj::RKst, PDG::ID::K},  {Prj::RK, PDG::ID::K}};
        map<Prj, int>   _mapHad2ID      = {{Prj::RPhi, PDG::ID::K}, {Prj::RKst, PDG::ID::Pi}, {Prj::RK, 0}};
        
        TString _had1NameM = _mapHad1NameM.at(_project);
        TString _had2NameM = _mapHad2NameM.at(_project);
        TString _lepName   = _ana == Analysis::MM ? "M" : "E";
        int     _had1IDM   = _mapHad1ID.at(_project);
        int     _had2IDM   = _mapHad2ID.at(_project);
        int     _lepIDM    = _ana == Analysis::MM ?  PDG::ID::M : PDG::ID::E;
        
        //=====================================================================
        // Build m_Maps
        // tuple< TRUEID, ID, nBrem > 
        //=====================================================================
        if (_ana == Analysis::MM) {
            m_Maps = {
                {make_tuple( PDG::ID::K,  _had1IDM, 0 ), m_weightHolder.GetWeightMapsPID(    "PID_K_MID_"+ _had1NameM+         "-effr-"+m_option)}, // Kaon -> Had1 Eff 
                {make_tuple( PDG::ID::Pi, _had1IDM, 0 ), m_weightHolder.GetWeightMapsPID(    "PID_Pi_MID_"+_had1NameM+         "-effr-"+m_option)}, // Pion -> Had1 Eff
                {make_tuple( PDG::ID::M,  _had1IDM, 0 ), m_weightHolder.GetWeightMapsPID(    "PID_M_MID_"+ _had1NameM+         "-effr-"+m_option)}, // Muon -> Had1 Eff
                {make_tuple( PDG::ID::E,  _had1IDM, 0 ), m_weightHolder.GetWeightMapsPID_fac("PID_E_MID_"+ _had1NameM+   "_brem0-effr-"+m_option)}, // Elec -> Had1 Eff (brem0 because TRUEID had)
                {make_tuple( PDG::ID::P,  _had1IDM, 0 ), m_weightHolder.GetWeightMapsPID(    "PID_P_MID_"+ _had1NameM+         "-effr-"+m_option)}, // Prot -> Had1 Eff
            
                {make_tuple( PDG::ID::K,  _lepIDM,  0 ),  m_weightHolder.GetWeightMapsPID(    "PID_K_MID_"+ _lepName+          "-effr-"+m_option)},  // Kaon -> Lept Eff
                {make_tuple( PDG::ID::Pi, _lepIDM,  0 ),  m_weightHolder.GetWeightMapsPID(    "PID_Pi_MID_"+_lepName+          "-effr-"+m_option)},  // Pion -> Lept Eff
                {make_tuple( PDG::ID::M,  _lepIDM,  0 ),  m_weightHolder.GetWeightMapsPID(    "PID_" +      _lepName+       "_ID-effr-"+m_option)},  // Muon -> Muon Eff
                {make_tuple( PDG::ID::P,  _lepIDM,  0 ),  m_weightHolder.GetWeightMapsPID(    "PID_P_MID_"+ _lepName+          "-effr-"+m_option)}   // Prot -> Muon Eff
            }; 
            if (_config.GetNBodies() > 3) {
                m_Maps.insert({
                    {make_tuple( PDG::ID::K,  _had2IDM, 0 ), m_weightHolder.GetWeightMapsPID(    "PID_K_MID_"+ _had2NameM+         "-effr-"+m_option)}, // Kaon -> Had2 Eff
                        {make_tuple( PDG::ID::Pi, _had2IDM, 0 ), m_weightHolder.GetWeightMapsPID(    "PID_Pi_MID_"+_had2NameM+         "-effr-"+m_option)}, // Pion -> Had2 Eff
                        {make_tuple( PDG::ID::M,  _had2IDM, 0 ), m_weightHolder.GetWeightMapsPID(    "PID_M_MID_"+ _had2NameM+         "-effr-"+m_option)}, // Muon -> Had2 Eff
                        {make_tuple( PDG::ID::E,  _had2IDM, 0 ), m_weightHolder.GetWeightMapsPID_fac("PID_E_MID_"+ _had2NameM+   "_brem0-effr-"+m_option)}, // Elec -> Had2 Eff (brem0 because TRUEID had)
                        {make_tuple( PDG::ID::P,  _had2IDM, 0 ), m_weightHolder.GetWeightMapsPID(    "PID_P_MID_"+ _had2NameM+         "-effr-"+m_option)}  // Prot -> Had2 Eff
                    }
                );
            }
        }
        else if (_ana == Analysis::EE) {
            m_Maps = {
                {make_tuple( PDG::ID::K,  _had1IDM, 0 ), m_weightHolder.GetWeightMapsPID(    "PID_K_MID_"+ _had1NameM+         "-effr-"+m_option)}, // Kaon -> Had1 Eff
                {make_tuple( PDG::ID::Pi, _had1IDM, 0 ), m_weightHolder.GetWeightMapsPID(    "PID_Pi_MID_"+_had1NameM+         "-effr-"+m_option)}, // Pion -> Had1 Eff
                {make_tuple( PDG::ID::M,  _had1IDM, 0 ), m_weightHolder.GetWeightMapsPID(    "PID_M_MID_"+ _had1NameM+         "-effr-"+m_option)}, // Muon -> Had1 Eff
                {make_tuple( PDG::ID::E,  _had1IDM, 0 ), m_weightHolder.GetWeightMapsPID_fac("PID_E_MID_"+ _had1NameM+   "_brem0-effr-"+m_option)}, // Elec -> Had1 Eff (brem0 because TRUEID had)
                {make_tuple( PDG::ID::P,  _had1IDM, 0 ), m_weightHolder.GetWeightMapsPID(    "PID_P_MID_"+ _had1NameM+         "-effr-"+m_option)}, // Prot -> Had1 Eff
                
                {make_tuple( PDG::ID::K,  _lepIDM,  0 ),  m_weightHolder.GetWeightMapsPID(    "PID_K_MID_"+ _lepName+          "-effr-"+m_option)}, // Kaon -> Lept brem0 Eff
                {make_tuple( PDG::ID::K,  _lepIDM,  1 ),  m_weightHolder.GetWeightMapsPID(    "PID_K_MID_"+ _lepName+          "-effr-"+m_option)}, // Kaon -> Lept brem1 Eff (brem1 and brem0 use the same maps because we dont differ)
                {make_tuple( PDG::ID::Pi, _lepIDM,  0 ),  m_weightHolder.GetWeightMapsPID(    "PID_Pi_MID_"+_lepName+          "-effr-"+m_option)}, // Pion -> Lept brem0 Eff
                {make_tuple( PDG::ID::Pi, _lepIDM,  1 ),  m_weightHolder.GetWeightMapsPID(    "PID_Pi_MID_"+_lepName+          "-effr-"+m_option)}, // Pion -> Lept brem1 Eff
                {make_tuple( PDG::ID::E,  _lepIDM,  0 ),  m_weightHolder.GetWeightMapsPID_fac("PID_"+       _lepName+ "_ID_brem0-effr-"+m_option)}, // Elec -> Lept brem0 Eff
                {make_tuple( PDG::ID::E,  _lepIDM,  1 ),  m_weightHolder.GetWeightMapsPID_fac("PID_"+       _lepName+ "_ID_brem1-effr-"+m_option)}, // Elec -> Lept brem1 Eff
                {make_tuple( PDG::ID::P,  _lepIDM,  0 ),  m_weightHolder.GetWeightMapsPID(    "PID_P_MID_"+ _lepName+          "-effr-"+m_option)}, // Prot -> Elec brem0 Eff
                {make_tuple( PDG::ID::P,  _lepIDM,  1 ),  m_weightHolder.GetWeightMapsPID(    "PID_P_MID_"+ _lepName+          "-effr-"+m_option)}  // Prot -> Elec brem1 Eff
            };
            if (_config.GetNBodies() > 3) {
                m_Maps.insert({
                    {make_tuple( PDG::ID::K,  _had2IDM, 0 ), m_weightHolder.GetWeightMapsPID(    "PID_K_MID_"+ _had2NameM+         "-effr-"+m_option)},  // Kaon -> Had2 Eff
                    {make_tuple( PDG::ID::Pi, _had2IDM, 0 ), m_weightHolder.GetWeightMapsPID(    "PID_Pi_MID_"+_had2NameM+         "-effr-"+m_option)},  // Pion -> Had2 Eff
                    {make_tuple( PDG::ID::M,  _had2IDM, 0 ), m_weightHolder.GetWeightMapsPID(    "PID_M_MID_"+ _had2NameM+         "-effr-"+m_option)},  // Muon -> Had2 Eff
                    {make_tuple( PDG::ID::E,  _had2IDM, 0 ), m_weightHolder.GetWeightMapsPID_fac("PID_E_MID_"+ _had2NameM+   "_brem0-effr-"+m_option)},  // Elec -> Had2 Eff (brem0 because TRUEID had)
                    {make_tuple( PDG::ID::P,  _had2IDM, 0 ), m_weightHolder.GetWeightMapsPID(    "PID_P_MID_"+ _had2NameM+         "-effr-"+m_option)}   // Prot -> Had2 Eff
                });	    
            }
        }
        else if (_ana == Analysis::ME){
            MessageSvc::Error("PIDHistoAdder", "Analysis ME not implemented yet", "EXIT_FAILURE");
            m_Maps = {};
            return false;
        }
        else {
            MessageSvc::Error("PIDHistoAdder", "Cannot parse ana", "EXIT_FAILURE");
            m_Maps = {};
            return false;
        }    
        // for(std::map<tuple<int,int,int>,pair<TH1D*,vector<TH2D*>>>::iterator it = m_Maps.begin(); it != m_Maps.end(); ++it) {
        //     cout << "Key: ( " << get<0>(it->first) << ", " << get<1>(it->first) << ", " << get<2>(it->first) << " ) " << endl;
        //     cout << "Value: " << it->second.first->GetName() << " and vector " << endl;
        // }
        MessageSvc::Info(Color::Green, "PIDHistoAdder", "Successfully Initialised PIDHistoAdder with option: " + m_option);
        MessageSvc::Info(Color::Green, "PIDHistoAdder", "BranchName: " + m_finalBranchName);
        return true;
    }
    
    double operator() (double _variableP, double _variableEta, int _variableTracks, int _variableTrueID, int _variableID, int _variableBrem) {
        if (!m_isInit) {
            MessageSvc::Error("PIDHistoAdder", "Not Initialised", "EXIT_FAILURE");
            return -1;
        }
        double _val = 0.; // init with 0.
        if (!(m_Maps.find(make_tuple(_variableTrueID, _variableID, _variableBrem)) != m_Maps.end())) {
            //=====================================================================
            // if (TRUEID, ID, BREM) isnt found this map is a nullptr!
            // in this case i set the eff to 0.! (E->Mu and Mu->E)  
            // (some of 3222 (xi0), 3312(xi pm) are in there as well, they are ghost though in most cases)
            //=====================================================================
            if (!(std::find(m_failedInds.begin(), m_failedInds.end(), make_tuple(_variableTrueID, _variableID, _variableBrem)) != m_failedInds.end())) {m_failedInds.push_back(make_tuple(_variableTrueID, _variableID, _variableBrem));}
            return _val;
        }
        //=====================================================================
        // the map pair is at the position (TRUEID, ID, BREM)
        //=====================================================================
        const pair <TH1D*, vector<TH2D*>> _mapToUse = m_Maps.at(make_tuple(_variableTrueID, _variableID, _variableBrem));
        //=====================================================================
        // first element of pair at the (TRUEID, ID, BREM) spot is the nTracks Map
        // use it to find the index of the map to extract the value from
        //=====================================================================
        TH1D * m_nTracksHist = _mapToUse.first;
        int _nTIndex        = _nTIndex = m_nTracksHist->FindFixBin(_variableTracks);
        if (m_nTracksHist->IsBinUnderflow(_nTIndex) || m_nTracksHist->IsBinOverflow(_nTIndex)) {
            // MessageSvc::Warning("PIDHistoAdder", "FindFixBin for nTracks value: "+ to_string(_variableTracks), " is out of range with index: " + to_string(_nTIndex));
            if (_variableTracks <= m_nTracksHist->GetXaxis()->GetXmin()) _variableTracks = m_nTracksHist->GetXaxis()->GetXmin() + 1;
            if (_variableTracks >= m_nTracksHist->GetXaxis()->GetXmax()) _variableTracks = m_nTracksHist->GetXaxis()->GetXmax() - 1;
            _nTIndex = m_nTracksHist->FindFixBin(_variableTracks);
        }
        int _bin      = -1;
        TH2D * m_histo = _mapToUse.second.at(_nTIndex-1); // convert to vector.at() numbering from 0 to size-1    
        if (m_histo != nullptr) {
            _bin = m_histo->FindFixBin(_variableP, _variableEta);
            if (m_histo->IsBinUnderflow(_bin) || m_histo->IsBinOverflow(_bin)) {
                if (_variableP <= m_histo->GetXaxis()->GetXmin()) _variableP     = m_histo->GetXaxis()->GetXmin() + m_histo->GetXaxis()->GetBinWidth(1) / 100.;
                if (_variableP >= m_histo->GetXaxis()->GetXmax()) _variableP     = m_histo->GetXaxis()->GetXmax() - m_histo->GetXaxis()->GetBinWidth(m_histo->GetNbinsX()) / 100.;
                if (_variableEta <= m_histo->GetYaxis()->GetXmin()) _variableEta = m_histo->GetYaxis()->GetXmin() + m_histo->GetYaxis()->GetBinWidth(1) / 100.;
                if (_variableEta >= m_histo->GetYaxis()->GetXmax()) _variableEta = m_histo->GetYaxis()->GetXmax() - m_histo->GetYaxis()->GetBinWidth(m_histo->GetNbinsY()) / 100.;
                _bin = m_histo->FindFixBin(_variableP, _variableEta);
            }
            //=====================================================================
            // if interp in option set interp to true
            //=====================================================================
            bool _interpolate = m_option.Contains("interp") ? true : false;
            //=====================================================================
            // for KDE use interp for electron F&C maps
            //=====================================================================
            if (m_option.Contains("KDE")) {_interpolate = (_variableTrueID == 11) ? true : false;}
            if( _interpolate ) {
                _val = m_histo->Interpolate( _variableP, _variableEta);
            } else {
                _val = m_histo->GetBinContent(_bin);
            }
        }
        return _val;
    }

    const map<tuple<int, int, int>, pair<TH1D*, vector<TH2D*>>> getMaps(){
        //=====================================================================
        // return mMaps maps
        //=====================================================================
        return m_Maps;
    }
    const vector< tuple< int, int, int> > getFailedInds(){
        //=====================================================================
        // return failed Indices
        //=====================================================================
        return m_failedInds;
    }
    const char* branchName() const{
        //=====================================================================
        // return branchName
        //=====================================================================
        return m_finalBranchName.Data();
    }

private:
    //=====================================================================
    // private variables
    //=====================================================================
    WeightHolder                                                      m_weightHolder;
    vector < tuple <int, int, int> >                                  m_failedInds;
    map < tuple < int, int, int >, pair < TH1D*, vector < TH2D* > > > m_Maps;
    TString                                                           m_finalBranchName;
    TString                                                           m_option;
    bool                                                              m_isInit;
};

//=====================================================================
/**
* \class PIDHistoAdderBSKDE
* \brief Functor class to attach BS PID weights 
*
* call as PIDHistoAdderBS _myPIDBranchBS( _weightHolder, _branchName, option );
* 
* option is used to define the type of PID maps to be loaded
* all options that are understood by WeightHolder.cpp can be used
*  
* options:
*     - KDE:                   bootstrapped KDE maps (always no interp), bootstrapped electron F&C maps with interp
*     - "":                    needs to be implemented by @alseuthe, once the maps are produced 
*                              the class functions should be able to parse it already ONLY ADJUST WHolder, if needed!
*     - your-option-goes-here: feel like you miss something? contact @schmitse and add it yourself!
*/
//=====================================================================
class PIDHistoAdderBSKDE {
public:
    //=====================================================================
    // default constructor with WHolder, branchName, option, and number of BS
    //=====================================================================    
    PIDHistoAdderBSKDE(  WeightHolder &_weightHolder,
                      TString _finalBranchName,
                      TString _option,
              int _nBS = 100 ) : m_weightHolder(_weightHolder), m_finalBranchName(_finalBranchName), m_option(_option), m_nBS(_nBS), m_isInit(false) {
    if (!m_isInit) m_isInit = Init();
    }
    //=====================================================================
    // copy constructor that changes branchName used
    //=====================================================================    
    PIDHistoAdderBSKDE(PIDHistoAdderBSKDE &other,
            TString _finalBranchName) : m_weightHolder(other.m_weightHolder), m_Maps(other.m_Maps), m_finalBranchName(_finalBranchName), m_option(other.m_option), m_nBS(other.m_nBS), m_isInit(other.m_isInit) {
        MessageSvc::Info(Color::Cyan, "PIDHistoAdder", "Copy from other PIDHistoAdder with option: " + m_option);
        MessageSvc::Info(Color::Cyan, "PIDHistoAdder", "Other BranchName: " + other.m_finalBranchName);
        MessageSvc::Info(Color::Cyan, "PIDHistoAdder", "This  BranchName: " + m_finalBranchName);
    if (!m_isInit) m_isInit = Init();
    }
    //=====================================================================
    // move constructor
    //=====================================================================    
    PIDHistoAdderBSKDE( const PIDHistoAdderBSKDE&& other) : m_weightHolder(other.m_weightHolder), m_Maps(other.m_Maps), m_finalBranchName(other.m_finalBranchName), m_option(other.m_option), m_nBS(other.m_nBS), m_isInit(other.m_isInit) {
        if(_DEBUGMESSAGE){
            MessageSvc::Debug( "PIDHistoAdder", "Move from other PIDHistoAdder with option: " + m_option);
            MessageSvc::Debug( "PIDHistoAdder", "Other BranchName: " + other.m_finalBranchName);
            MessageSvc::Debug( "PIDHistoAdder", "This  BranchName: " + m_finalBranchName);
        }
    if (!m_isInit) m_isInit = Init();
    }
    //=====================================================================
    // copy constructor
    //=====================================================================    
    PIDHistoAdderBSKDE( const PIDHistoAdderBSKDE& other)  : m_weightHolder(other.m_weightHolder), m_Maps(other.m_Maps), m_finalBranchName(other.m_finalBranchName), m_option(other.m_option), m_nBS(other.m_nBS), m_isInit(other.m_isInit) {
        MessageSvc::Info(Color::Cyan, "PIDHistoAdder", "Copy from other PIDHistoAdder with option: " + m_option);
    if (!m_isInit) m_isInit = Init();
    }

    bool Init(){
        //=====================================================================
        // Init HistoAdderBS
        // Build m_Maps, map that stores PID maps of form TH1D, vector<TH2D>
        // m_Maps maps (PDG TRUEID, PDG ID, BREM) to the corresponding PID map
        //=====================================================================
        
        MessageSvc::Info(Color::Cyan, "PIDHistoAdderBS", "Init with Option: " + m_option);	
        auto _config = m_weightHolder.GetConfigHolder();
        
        auto _ana     = _config.GetAna();
        auto _project = _config.GetProject();
        
        //=====================================================================	
        // had Names for Maps
        //=====================================================================
        map<Prj, TString> _mapHad1NameM = {{Prj::RPhi, "K"}, {Prj::RKst, "K"},  {Prj::RK, "K"}};
        map<Prj, TString> _mapHad2NameM = {{Prj::RPhi, "K"}, {Prj::RKst, "Pi"}, {Prj::RK, ""}};    
        //=====================================================================
        // TRUEIDs for Maps
        //=====================================================================
        map<Prj, int>   _mapHad1ID      = {{Prj::RPhi, PDG::ID::K}, {Prj::RKst, PDG::ID::K},  {Prj::RK, PDG::ID::K}};
        map<Prj, int>   _mapHad2ID      = {{Prj::RPhi, PDG::ID::K}, {Prj::RKst, PDG::ID::Pi}, {Prj::RK, 0}};
        
        TString _had1NameM = _mapHad1NameM.at(_project);
        TString _had2NameM = _mapHad2NameM.at(_project);
        TString _lepName   = _ana == Analysis::MM ? "M" : "E";
        int     _had1IDM   = _mapHad1ID.at(_project);
        int     _had2IDM   = _mapHad2ID.at(_project);
        int     _lepIDM    = _ana == Analysis::MM ?  PDG::ID::M : PDG::ID::E;
        
        //=====================================================================
        // build m_Maps
        //=====================================================================
        if (_ana == Analysis::MM) {
            m_Maps = {
                {make_tuple( PDG::ID::K,  _had1IDM, 0 ), m_weightHolder.GetWeightMapsBSKDE(  "PID", "PID_K_MID_"+ _had1NameM+         "-effr-" + m_option, WeightDefRX::nBS)}, // Kaon -> Had1 Eff
                {make_tuple( PDG::ID::Pi, _had1IDM, 0 ), m_weightHolder.GetWeightMapsBSKDE(  "PID", "PID_Pi_MID_"+_had1NameM+         "-effr-" + m_option, WeightDefRX::nBS)}, // Pion -> Had1 Eff
                {make_tuple( PDG::ID::M,  _had1IDM, 0 ), m_weightHolder.GetWeightMapsBSKDE(  "PID", "PID_M_MID_"+ _had1NameM+         "-effr-" + m_option, WeightDefRX::nBS)}, // Muon -> Had1 Eff
                {make_tuple( PDG::ID::E,  _had1IDM, 0 ), m_weightHolder.GetWeightMapsBS(     "PID", "PID_E_MID_"+ _had1NameM+   "_brem0-effr-" + m_option, WeightDefRX::nBS)}, // Elec -> Had1 Eff (brem0 because TRUEID had)
                {make_tuple( PDG::ID::P,  _had1IDM, 0 ), m_weightHolder.GetWeightMapsBSKDE(  "PID", "PID_P_MID_"+ _had1NameM+         "-effr-" + m_option, WeightDefRX::nBS)}, // Prot -> Had1 Eff
                
                {make_tuple( PDG::ID::K,  _lepIDM,  0 ),  m_weightHolder.GetWeightMapsBSKDE(  "PID", "PID_K_MID_"+ _lepName+          "-effr-" + m_option, WeightDefRX::nBS)},  // Kaon -> Lept Eff
                {make_tuple( PDG::ID::Pi, _lepIDM,  0 ),  m_weightHolder.GetWeightMapsBSKDE(  "PID", "PID_Pi_MID_"+_lepName+          "-effr-" + m_option, WeightDefRX::nBS)},  // Pion -> Lept Eff
                {make_tuple( PDG::ID::M,  _lepIDM,  0 ),  m_weightHolder.GetWeightMapsBSKDE(  "PID", "PID_" +      _lepName+       "_ID-effr-" + m_option, WeightDefRX::nBS)},  // Muon -> Muon Eff
                {make_tuple( PDG::ID::P,  _lepIDM,  0 ),  m_weightHolder.GetWeightMapsBSKDE(  "PID", "PID_P_MID_"+ _lepName+          "-effr-" + m_option, WeightDefRX::nBS)}   // Prot -> Muon Eff
            }; 
            if (_config.GetNBodies() > 3) {
                m_Maps.insert({
                    {make_tuple( PDG::ID::K,  _had2IDM, 0 ), m_weightHolder.GetWeightMapsBSKDE(  "PID", "PID_K_MID_"+ _had2NameM+         "-effr-" + m_option, WeightDefRX::nBS)}, // Kaon -> Had2 Eff
                    {make_tuple( PDG::ID::Pi, _had2IDM, 0 ), m_weightHolder.GetWeightMapsBSKDE(  "PID", "PID_Pi_MID_"+_had2NameM+         "-effr-" + m_option, WeightDefRX::nBS)}, // Pion -> Had2 Eff
                    {make_tuple( PDG::ID::M,  _had2IDM, 0 ), m_weightHolder.GetWeightMapsBSKDE(  "PID", "PID_M_MID_"+ _had2NameM+         "-effr-" + m_option, WeightDefRX::nBS)}, // Muon -> Had2 Eff
                    {make_tuple( PDG::ID::E,  _had2IDM, 0 ), m_weightHolder.GetWeightMapsBS(     "PID", "PID_E_MID_"+ _had2NameM+   "_brem0-effr-" + m_option, WeightDefRX::nBS)}, // Elec -> Had2 Eff (brem0 because TRUEID had)
                    {make_tuple( PDG::ID::P,  _had2IDM, 0 ), m_weightHolder.GetWeightMapsBSKDE(  "PID", "PID_P_MID_"+ _had2NameM+         "-effr-" + m_option, WeightDefRX::nBS)}  // Prot -> Had2 Eff
                    }
                );
            }
        }
        else if (_ana == Analysis::EE) {
            m_Maps = {
            {make_tuple( PDG::ID::K,  _had1IDM, 0 ), m_weightHolder.GetWeightMapsBSKDE(  "PID", "PID_K_MID_"+ _had1NameM+         "-effr-" + m_option, WeightDefRX::nBS)}, // Kaon -> Had1 Eff
            {make_tuple( PDG::ID::Pi, _had1IDM, 0 ), m_weightHolder.GetWeightMapsBSKDE(  "PID", "PID_Pi_MID_"+_had1NameM+         "-effr-" + m_option, WeightDefRX::nBS)}, // Pion -> Had1 Eff
            {make_tuple( PDG::ID::M,  _had1IDM, 0 ), m_weightHolder.GetWeightMapsBSKDE(  "PID", "PID_M_MID_"+ _had1NameM+         "-effr-" + m_option, WeightDefRX::nBS)}, // Muon -> Had1 Eff
            {make_tuple( PDG::ID::E,  _had1IDM, 0 ), m_weightHolder.GetWeightMapsBS(     "PID", "PID_E_MID_"+ _had1NameM+   "_brem0-effr-" + m_option, WeightDefRX::nBS)}, // Elec -> Had1 Eff (brem0 because TRUEID had)
            {make_tuple( PDG::ID::P,  _had1IDM, 0 ), m_weightHolder.GetWeightMapsBSKDE(  "PID", "PID_P_MID_"+ _had1NameM+         "-effr-" + m_option, WeightDefRX::nBS)}, // Prot -> Had1 Eff
            
            {make_tuple( PDG::ID::K,  _lepIDM,  0 ),  m_weightHolder.GetWeightMapsBSKDE(  "PID", "PID_K_MID_"+ _lepName+          "-effr-" + m_option, WeightDefRX::nBS)}, // Kaon -> Lept brem0 Eff
            {make_tuple( PDG::ID::K,  _lepIDM,  1 ),  m_weightHolder.GetWeightMapsBSKDE(  "PID", "PID_K_MID_"+ _lepName+          "-effr-" + m_option, WeightDefRX::nBS)}, // Kaon -> Lept brem1 Eff (brem1 and brem0 use the same maps because we dont differ)
            {make_tuple( PDG::ID::Pi, _lepIDM,  0 ),  m_weightHolder.GetWeightMapsBSKDE(  "PID", "PID_Pi_MID_"+_lepName+          "-effr-" + m_option, WeightDefRX::nBS)}, // Pion -> Lept brem0 Eff
            {make_tuple( PDG::ID::Pi, _lepIDM,  1 ),  m_weightHolder.GetWeightMapsBSKDE(  "PID", "PID_Pi_MID_"+_lepName+          "-effr-" + m_option, WeightDefRX::nBS)}, // Pion -> Lept brem1 Eff
            {make_tuple( PDG::ID::E,  _lepIDM,  0 ),  m_weightHolder.GetWeightMapsBS(     "PID", "PID_"+       _lepName+ "_ID_brem0-effr-" + m_option, WeightDefRX::nBS)}, // Elec -> Lept brem0 Eff
            {make_tuple( PDG::ID::E,  _lepIDM,  1 ),  m_weightHolder.GetWeightMapsBS(     "PID", "PID_"+       _lepName+ "_ID_brem1-effr-" + m_option, WeightDefRX::nBS)}, // Elec -> Lept brem1 Eff
            {make_tuple( PDG::ID::P,  _lepIDM,  0 ),  m_weightHolder.GetWeightMapsBSKDE(  "PID", "PID_P_MID_"+ _lepName+          "-effr-" + m_option, WeightDefRX::nBS)}, // Prot -> Elec brem0 Eff
            {make_tuple( PDG::ID::P,  _lepIDM,  1 ),  m_weightHolder.GetWeightMapsBSKDE(  "PID", "PID_P_MID_"+ _lepName+          "-effr-" + m_option, WeightDefRX::nBS)}  // Prot -> Elec brem1 Eff
            };
            if (_config.GetNBodies() > 3) {
            m_Maps.insert({	    
                {make_tuple( PDG::ID::K,  _had2IDM, 0 ), m_weightHolder.GetWeightMapsBSKDE(  "PID", "PID_K_MID_"+ _had2NameM+         "-effr-" + m_option, WeightDefRX::nBS)},  // Kaon -> Had2 Eff
                {make_tuple( PDG::ID::Pi, _had2IDM, 0 ), m_weightHolder.GetWeightMapsBSKDE(  "PID", "PID_Pi_MID_"+_had2NameM+         "-effr-" + m_option, WeightDefRX::nBS)},  // Pion -> Had2 Eff
                {make_tuple( PDG::ID::M,  _had2IDM, 0 ), m_weightHolder.GetWeightMapsBSKDE(  "PID", "PID_M_MID_"+ _had2NameM+         "-effr-" + m_option, WeightDefRX::nBS)},  // Muon -> Had2 Eff
                {make_tuple( PDG::ID::E,  _had2IDM, 0 ), m_weightHolder.GetWeightMapsBS(     "PID", "PID_E_MID_"+ _had2NameM+   "_brem0-effr-" + m_option, WeightDefRX::nBS)},  // Elec -> Had2 Eff (brem0 because TRUEID had)
                {make_tuple( PDG::ID::P,  _had2IDM, 0 ), m_weightHolder.GetWeightMapsBSKDE(  "PID", "PID_P_MID_"+ _had2NameM+         "-effr-" + m_option, WeightDefRX::nBS)}   // Prot -> Had2 Eff
                }
                );    
            }
        }
        else if (_ana == Analysis::ME){
            MessageSvc::Error("PIDHistoAdderBS", "Analysis ME not implemented yet", "EXIT_FAILURE");
            return false;
        }
        else {
            MessageSvc::Error("PIDHistoAdderBS", "Cannot parse ana", "EXIT_FAILURE");
            return false;
        }    
        
        MessageSvc::Info(Color::Green, "PIDHistoAdderBS", "Initialised PIDHistoAdder with option: " + m_option);
        return true;
    }
    
    ROOT::VecOps::RVec< double > operator() (double _variableP, double _variableEta, int _variableTracks, int _variableTrueID, int _variableID, int _variableBrem) {
        ROOT::VecOps::RVec< double> _values(m_nBS);// init with zeroes

        if (!m_isInit) {
            MessageSvc::Error("PIDHistoAdder", "Not Initialised", "EXIT_FAILURE");
            for (int i = 0; i < m_nBS; i++) {_values.at(i) = -1.; }
            return _values;
        }

        if (!(m_Maps.find(make_tuple(_variableTrueID, _variableID, _variableBrem)) != m_Maps.end())) {
            //=====================================================================
            // if (TRUEID, ID, BREM) isnt found this map is a nullptr!
            // in this case i set the eff to 0.! (E->Mu and Mu->E) 
            // (some of 3222 (xi0), 3312(xi pm) are in there as well, they are ghost though in most cases)
            //=====================================================================
            if (std::find(m_failedInds.begin(), m_failedInds.end(), make_tuple(_variableTrueID, _variableID, _variableBrem)) != m_failedInds.end()) {m_failedInds.push_back(make_tuple(_variableTrueID, _variableID, _variableBrem));}
            return _values;
        }
        //=====================================================================
        // if interp in option set interp to true
        //=====================================================================
        bool _interpolate = m_option.Contains("interp") ? true : false;
        //=====================================================================
        // for KDE use interp for electron F&C maps
        //=====================================================================
        if (m_option.Contains("KDE")) {_interpolate = (_variableTrueID == 11) ? true : false;}
        
        //=====================================================================
        // the map pair is at the position (TRUEID, ID, BREM)
        //=====================================================================
        const vector< pair < TH1D*, vector< TH2D* > > > _mapToUseVector = m_Maps.at(make_tuple(_variableTrueID, _variableID, _variableBrem));

        if (_mapToUseVector.size() != m_nBS) {
            MessageSvc::Error("PIDHistoAdderBS", "_MapToUseVector size != " + to_string(m_nBS), "EXIT_FAILURE");
            return _values;
        }

        for (int i = 0; i<m_nBS; i++) {
            //=====================================================================
            // first element of pair at the (TRUEID, ID, BREM) spot is the nTracks Map
            // use it to find the index of the map to extract the value from
            //=====================================================================
            pair< TH1D*, vector< TH2D* > > _mapToUse = _mapToUseVector.at(i);
            TH1D * m_nTracksHist = _mapToUse.first;
            int _nTIndex        = _nTIndex = m_nTracksHist->FindFixBin(_variableTracks);
            if (m_nTracksHist->IsBinUnderflow(_nTIndex) || m_nTracksHist->IsBinOverflow(_nTIndex)) {
                if (_variableTracks <= m_nTracksHist->GetXaxis()->GetXmin()) _variableTracks = m_nTracksHist->GetXaxis()->GetXmin() + 1;
                if (_variableTracks >= m_nTracksHist->GetXaxis()->GetXmax()) _variableTracks = m_nTracksHist->GetXaxis()->GetXmax() - 1;
                _nTIndex = m_nTracksHist->FindFixBin(_variableTracks);
            }
            int _bin      = -1;
            TH2D * m_histo = _mapToUse.second.at(_nTIndex-1); // convert to vector.at() numbering from 0 to size-1
            if (m_histo != nullptr) {
                _bin = m_histo->FindFixBin(_variableP, _variableEta);
                if (m_histo->IsBinUnderflow(_bin) || m_histo->IsBinOverflow(_bin)) {
                    if (_variableP <= m_histo->GetXaxis()->GetXmin()) _variableP     = m_histo->GetXaxis()->GetXmin() + m_histo->GetXaxis()->GetBinWidth(1) / 100.;
                    if (_variableP >= m_histo->GetXaxis()->GetXmax()) _variableP     = m_histo->GetXaxis()->GetXmax() - m_histo->GetXaxis()->GetBinWidth(m_histo->GetNbinsX()) / 100.;
                    if (_variableEta <= m_histo->GetYaxis()->GetXmin()) _variableEta = m_histo->GetYaxis()->GetXmin() + m_histo->GetYaxis()->GetBinWidth(1) / 100.;
                    if (_variableEta >= m_histo->GetYaxis()->GetXmax()) _variableEta = m_histo->GetYaxis()->GetXmax() - m_histo->GetYaxis()->GetBinWidth(m_histo->GetNbinsY()) / 100.;
                    _bin = m_histo->FindFixBin(_variableP, _variableEta);
                }
                if( _interpolate ) {
                    _values.at(i) = m_histo->Interpolate( _variableP, _variableEta);
                } else {
                    _values.at(i) = m_histo->GetBinContent(_bin);
                }
            }
        }
        return _values;
    }

    const map<tuple<int, int, int>, vector< pair < TH1D*, vector< TH2D* > > > > getMaps(){
    //=====================================================================
    // return mMaps maps
    //=====================================================================
    return m_Maps;
    }
    const vector< tuple< int, int, int> > getFailedInds(){
    //=====================================================================
    // return failed Indices
    //=====================================================================
    return m_failedInds;
    }
    const char* branchName() const{
    //=====================================================================
    // return branchName
    //=====================================================================
        return m_finalBranchName.Data();
    }

private:
    //=====================================================================
    // privates members
    //=====================================================================
    WeightHolder                                                            m_weightHolder;
    vector< tuple< int, int, int > >                                        m_failedInds;
    bool                                                                    m_isInit;
    map< tuple< int, int, int >, vector< pair< TH1D*, vector< TH2D* > > > > m_Maps;
    TString                                                                 m_finalBranchName;
    TString                                                                 m_option;
    int                                                                     m_nBS;
};




class PIDHistoAdderBS_SMEAR{
public:
    //=====================================================================
    // default constructor with WHolder, branchName, option, and number of BS
    //=====================================================================    
    PIDHistoAdderBS_SMEAR(  WeightHolder &_weightHolder,
                      TString _finalBranchName,
                      TString _option,
              int _nBS = 100 ) : m_weightHolder(_weightHolder), m_finalBranchName(_finalBranchName), m_option(_option), m_nBS(_nBS), m_isInit(false) {
    if (!m_isInit) m_isInit = Init();
    }
    //=====================================================================
    // copy constructor that changes branchName used
    //=====================================================================    
    PIDHistoAdderBS_SMEAR(PIDHistoAdderBS_SMEAR &other,
            TString _finalBranchName) : m_weightHolder(other.m_weightHolder), m_Maps(other.m_Maps), m_finalBranchName(_finalBranchName), m_option(other.m_option), m_nBS(other.m_nBS), m_isInit(other.m_isInit) {
            if(_DEBUGMESSAGE){
                MessageSvc::Debug("PIDHistoAdderBS_SMEAR", "Copy from other PIDHistoAdder with option: " + m_option);
                MessageSvc::Debug("PIDHistoAdderBS_SMEAR", "Other BranchName: " + other.m_finalBranchName);
                MessageSvc::Debug("PIDHistoAdderBS_SMEAR", "This  BranchName: " + m_finalBranchName);
            }
        if (!m_isInit) m_isInit = Init();
    }
    //=====================================================================
    // move constructor
    //=====================================================================    
    PIDHistoAdderBS_SMEAR( const PIDHistoAdderBS_SMEAR&& other) : m_weightHolder(other.m_weightHolder), m_Maps(other.m_Maps), m_finalBranchName(other.m_finalBranchName), m_option(other.m_option), m_nBS(other.m_nBS), m_isInit(other.m_isInit) {
        if(_DEBUGMESSAGE){
            MessageSvc::Debug("PIDHistoAdderBS_SMEAR", "Move from other PIDHistoAdder with option: " + m_option);
            MessageSvc::Debug("PIDHistoAdderBS_SMEAR", "Other BranchName: " + other.m_finalBranchName);
            MessageSvc::Debug("PIDHistoAdderBS_SMEAR", "This  BranchName: " + m_finalBranchName);
        }
        if (!m_isInit) m_isInit = Init();
    }
    //=====================================================================
    // copy constructor
    //=====================================================================    
    PIDHistoAdderBS_SMEAR( const PIDHistoAdderBS_SMEAR& other)  : m_weightHolder(other.m_weightHolder), m_Maps(other.m_Maps), m_finalBranchName(other.m_finalBranchName), m_option(other.m_option), m_nBS(other.m_nBS), m_isInit(other.m_isInit) {
        if(_DEBUGMESSAGE){
            MessageSvc::Debug("PIDHistoAdderBS_SMEAR", "Copy from other PIDHistoAdder with option: " + m_option);
        }
        if (!m_isInit) m_isInit = Init();
    }

    //==============================================================================================================================
    // Given an input 3D map (the pair<1D,vector<2D>>), generate a vector<100> elements with gaussian smeared TH2D maps
    //============================================================================================================================== 
    vector< pair< TH1D*, vector< TH2D* > > >  GaussSmear( TString _seedString, const  pair< TH1D*, vector< TH2D* > > & myBaseMap , TString _optionSmear="eff"){
        int seedBase = TMath::Abs( (int)_seedString.Hash());
        vector< pair< TH1D*, vector< TH2D* > > > _smearedHistos; 
        _smearedHistos.resize(m_nBS);
        for( int i =0; i <m_nBS ; ++i){
            //increase seed by 1 unity for each randomization we have             
            TH1D * _nTracksSlice = dynamic_cast<TH1D*>( CopyHist(myBaseMap.first, true) ); 
            if(_nTracksSlice == nullptr) MessageSvc::Error("Cannot Cast to TH1D , fix types", "","EXIT_FAILURE");
            vector< TH2D *> _randomized; 
            for( int ntracksSlot = 0; ntracksSlot < myBaseMap.second.size() ; ++ntracksSlot){
                seedBase = seedBase +i + ntracksSlot ; 
                TH2D * _randomizedH   = dynamic_cast<TH2D*>( RandomizeAllEntries( myBaseMap.second.at(ntracksSlot) , seedBase , _optionSmear ) );
                if(_randomizedH == nullptr) MessageSvc::Error("Cannot Cast to TH2D , fix types", "","EXIT_FAILURE");
                _randomized.push_back(_randomizedH );
            }
            _smearedHistos[i]= make_pair( _nTracksSlice,_randomized );
        }
        return _smearedHistos;
    };
    bool Init(){
        //=====================================================================
        // Init HistoAdder
        // Build m_Maps, map that stores PID maps of form TH1D, vector<TH2D>
        // m_Maps maps (PDG TRUEID, PDG ID, BREM) to the corresponding PID map
        //=====================================================================
        
        MessageSvc::Info(Color::Green, "PIDHistoAdderBS_SMEAR", "Init with Option: " + m_option);	
        auto _config = m_weightHolder.GetConfigHolder();
        auto _ana     = _config.GetAna();
        auto _project = _config.GetProject();
        
        //=====================================================================	
        // had Names for Maps
        //=====================================================================
        map<Prj, TString> _mapHad1NameM = {{Prj::RPhi, "K"}, {Prj::RKst, "K"},  {Prj::RK, "K"}};
        map<Prj, TString> _mapHad2NameM = {{Prj::RPhi, "K"}, {Prj::RKst, "Pi"}, {Prj::RK, ""}};    
        //=====================================================================
        // TRUEIDs for Maps
        //=====================================================================
        map<Prj, int>   _mapHad1ID      = {{Prj::RPhi, PDG::ID::K}, {Prj::RKst, PDG::ID::K},  {Prj::RK, PDG::ID::K}};
        map<Prj, int>   _mapHad2ID      = {{Prj::RPhi, PDG::ID::K}, {Prj::RKst, PDG::ID::Pi}, {Prj::RK, 0}};
        
        TString _had1NameM = _mapHad1NameM.at(_project);
        TString _had2NameM = _mapHad2NameM.at(_project);
        TString _lepName   = _ana == Analysis::MM ? "M" : "E";
        int     _had1IDM   = _mapHad1ID.at(_project);
        int     _had2IDM   = _mapHad2ID.at(_project);
        int     _lepIDM    = _ana == Analysis::MM ?  PDG::ID::M : PDG::ID::E;
        
        //=====================================================================
        // Build m_Maps
        // tuple< TRUEID, ID, nBrem > 
        //=====================================================================
        if (_ana == Analysis::MM) {
            m_Maps = {
                {make_tuple( PDG::ID::K,  _had1IDM, 0 ), GaussSmear( "H1_K" , m_weightHolder.GetWeightMapsPID(   "PID_K_MID_"+ _had1NameM+         "-effr-"+m_option))}, // Kaon -> Had1 Eff 
                {make_tuple( PDG::ID::Pi, _had1IDM, 0 ), GaussSmear( "H1_Pi", m_weightHolder.GetWeightMapsPID(   "PID_Pi_MID_"+_had1NameM+         "-effr-"+m_option))}, // Pion -> Had1 Eff
                {make_tuple( PDG::ID::M,  _had1IDM, 0 ), GaussSmear( "H1_M" ,m_weightHolder.GetWeightMapsPID(    "PID_M_MID_"+ _had1NameM+         "-effr-"+m_option))}, // Muon -> Had1 Eff
                {make_tuple( PDG::ID::E,  _had1IDM, 0 ), GaussSmear( "H1_E" ,m_weightHolder.GetWeightMapsPID_fac("PID_E_MID_"+ _had1NameM+   "_brem0-effr-"+m_option))}, // Elec -> Had1 Eff (brem0 because TRUEID had)
                {make_tuple( PDG::ID::P,  _had1IDM, 0 ), GaussSmear( "H1_P" ,m_weightHolder.GetWeightMapsPID(    "PID_P_MID_"+ _had1NameM+         "-effr-"+m_option))}, // Prot -> Had1 Eff
            
                {make_tuple( PDG::ID::K,  _lepIDM,  0 ),  GaussSmear( "M_K" , m_weightHolder.GetWeightMapsPID(    "PID_K_MID_"+ _lepName+          "-effr-"+m_option))},  // Kaon -> Lept Eff
                {make_tuple( PDG::ID::Pi, _lepIDM,  0 ),  GaussSmear( "M_Pi", m_weightHolder.GetWeightMapsPID(    "PID_Pi_MID_"+_lepName+          "-effr-"+m_option))},  // Pion -> Lept Eff
                {make_tuple( PDG::ID::M,  _lepIDM,  0 ),  GaussSmear( "M_M" , m_weightHolder.GetWeightMapsPID(    "PID_" +      _lepName+       "_ID-effr-"+m_option))},  // Muon -> Muon Eff
                {make_tuple( PDG::ID::P,  _lepIDM,  0 ),  GaussSmear( "M_P" , m_weightHolder.GetWeightMapsPID(    "PID_P_MID_"+ _lepName+          "-effr-"+m_option))}   // Prot -> Muon Eff
            }; 
            if (_config.GetNBodies() > 3) {
                m_Maps.insert({
                        {make_tuple( PDG::ID::K,  _had2IDM, 0 ), GaussSmear( "H2_K"  , m_weightHolder.GetWeightMapsPID(    "PID_K_MID_"+ _had2NameM+         "-effr-"+m_option))}, // Kaon -> Had2 Eff
                        {make_tuple( PDG::ID::Pi, _had2IDM, 0 ), GaussSmear( "H2_Pi" , m_weightHolder.GetWeightMapsPID(    "PID_Pi_MID_"+_had2NameM+         "-effr-"+m_option))}, // Pion -> Had2 Eff
                        {make_tuple( PDG::ID::M,  _had2IDM, 0 ), GaussSmear( "H2_M"  , m_weightHolder.GetWeightMapsPID(    "PID_M_MID_"+ _had2NameM+         "-effr-"+m_option))}, // Muon -> Had2 Eff
                        {make_tuple( PDG::ID::E,  _had2IDM, 0 ), GaussSmear( "H2_E"  , m_weightHolder.GetWeightMapsPID_fac("PID_E_MID_"+ _had2NameM+   "_brem0-effr-"+m_option))}, // Elec -> Had2 Eff (brem0 because TRUEID had)
                        {make_tuple( PDG::ID::P,  _had2IDM, 0 ), GaussSmear( "H2_P"  , m_weightHolder.GetWeightMapsPID(    "PID_P_MID_"+ _had2NameM+         "-effr-"+m_option))}  // Prot -> Had2 Eff
                    }
                );
            }
        }
        else if (_ana == Analysis::EE) {
            m_Maps = {
                {make_tuple( PDG::ID::K,  _had1IDM, 0 ), GaussSmear( "H1_K"  , m_weightHolder.GetWeightMapsPID(    "PID_K_MID_"+ _had1NameM+         "-effr-"+m_option))}, // Kaon -> Had1 Eff
                {make_tuple( PDG::ID::Pi, _had1IDM, 0 ), GaussSmear( "H1_Pi" , m_weightHolder.GetWeightMapsPID(    "PID_Pi_MID_"+_had1NameM+         "-effr-"+m_option))}, // Pion -> Had1 Eff
                {make_tuple( PDG::ID::M,  _had1IDM, 0 ), GaussSmear( "H1_M"  ,m_weightHolder.GetWeightMapsPID(    "PID_M_MID_"+ _had1NameM+         "-effr-"+m_option))}, // Muon -> Had1 Eff
                {make_tuple( PDG::ID::E,  _had1IDM, 0 ), GaussSmear( "H1_E"  ,m_weightHolder.GetWeightMapsPID_fac("PID_E_MID_"+ _had1NameM+   "_brem0-effr-"+m_option))}, // Elec -> Had1 Eff (brem0 because TRUEID had)
                {make_tuple( PDG::ID::P,  _had1IDM, 0 ), GaussSmear( "H1_P"  ,m_weightHolder.GetWeightMapsPID(    "PID_P_MID_"+ _had1NameM+         "-effr-"+m_option))}, // Prot -> Had1 Eff
                
                {make_tuple( PDG::ID::K,  _lepIDM,  0 ),  GaussSmear( "E_K"  ,m_weightHolder.GetWeightMapsPID(    "PID_K_MID_"+ _lepName+          "-effr-"+m_option))}, // Kaon -> Lept brem0 Eff
                {make_tuple( PDG::ID::K,  _lepIDM,  1 ),  GaussSmear( "E_K"  ,m_weightHolder.GetWeightMapsPID(    "PID_K_MID_"+ _lepName+          "-effr-"+m_option))}, // Kaon -> Lept brem1 Eff (brem1 and brem0 use the same maps because we dont differ)
                {make_tuple( PDG::ID::Pi, _lepIDM,  0 ),  GaussSmear( "E_Pi"  ,m_weightHolder.GetWeightMapsPID(   "PID_Pi_MID_"+_lepName+          "-effr-"+m_option))}, // Pion -> Lept brem0 Eff
                {make_tuple( PDG::ID::Pi, _lepIDM,  1 ),  GaussSmear( "E_Pi"  ,m_weightHolder.GetWeightMapsPID(   "PID_Pi_MID_"+_lepName+          "-effr-"+m_option))}, // Pion -> Lept brem1 Eff
                // {make_tuple( PDG::ID::E,  _lepIDM,  0 ),  GaussSmear( "E_E_0"  ,m_weightHolder.GetWeightMapsPID_fac("PID_"+       _lepName+ "_ID_brem0-effr-"+m_option))}, // Elec -> Lept brem0 Eff (SEE LATER)
                // {make_tuple( PDG::ID::E,  _lepIDM,  1 ),  GaussSmear( "E_E_1"  ,m_weightHolder.GetWeightMapsPID_fac("PID_"+       _lepName+ "_ID_brem1-effr-"+m_option))}, // Elec -> Lept brem1 Eff (SEE LATER)
                {make_tuple( PDG::ID::P,  _lepIDM,  0 ),  GaussSmear( "E_P"  ,m_weightHolder.GetWeightMapsPID(    "PID_P_MID_"+ _lepName+          "-effr-"+m_option))}, // Prot -> Elec brem0 Eff
                {make_tuple( PDG::ID::P,  _lepIDM,  1 ),  GaussSmear( "E_P"  ,m_weightHolder.GetWeightMapsPID(    "PID_P_MID_"+ _lepName+          "-effr-"+m_option))}  // Prot -> Elec brem1 Eff
            };
            if (_config.GetNBodies() > 3) {
                m_Maps.insert({
                    {make_tuple( PDG::ID::K,  _had2IDM, 0 ), GaussSmear( "M_K" , m_weightHolder.GetWeightMapsPID(    "PID_K_MID_"+ _had2NameM+         "-effr-"+m_option))},  // Kaon -> Had2 Eff
                    {make_tuple( PDG::ID::Pi, _had2IDM, 0 ), GaussSmear( "M_Pi", m_weightHolder.GetWeightMapsPID(    "PID_Pi_MID_"+_had2NameM+         "-effr-"+m_option))},  // Pion -> Had2 Eff
                    {make_tuple( PDG::ID::M,  _had2IDM, 0 ), GaussSmear( "M_M", m_weightHolder.GetWeightMapsPID(    "PID_M_MID_"+ _had2NameM+         "-effr-"+m_option))},  // Muon -> Had2 Eff
                    {make_tuple( PDG::ID::E,  _had2IDM, 0 ), GaussSmear( "M_E", m_weightHolder.GetWeightMapsPID_fac("PID_E_MID_"+ _had2NameM+   "_brem0-effr-"+m_option))},  // Elec -> Had2 Eff (brem0 because TRUEID had)
                    {make_tuple( PDG::ID::P,  _had2IDM, 0 ), GaussSmear( "M_P", m_weightHolder.GetWeightMapsPID(    "PID_P_MID_"+ _had2NameM+         "-effr-"+m_option))}   // Prot -> Had2 Eff
                });	    
            }

            if(SettingDef::Weight::usePIDPTElectron){
                //NOTE: altough we insert, it doesn't update!
                m_Maps.insert({
                    {make_tuple( PDG::ID::E,  _lepIDM,  0 ),  m_weightHolder.GetWeightMapsBS(     "PID", "PID_"+       _lepName+ "_ID_brem0-effr-" + m_option, WeightDefRX::nBS)}, // Elec -> Lept brem0 Eff (Load 100 PT, ETA BS maps instead of Gaussian Smearing)
                    {make_tuple( PDG::ID::E,  _lepIDM,  1 ),  m_weightHolder.GetWeightMapsBS(     "PID", "PID_"+       _lepName+ "_ID_brem1-effr-" + m_option, WeightDefRX::nBS)}, // Elec -> Lept brem0 Eff // Elec -> Lept brem1 Eff (Load 100 PT, ETA BS maps instead of Gaussian Smearing)
                });
            }
            else {
                if( m_option.Contains("WeightMapPID")){
                    //NOTE: altough we insert, it doesn't update!
                    //Eps(e->e) is a data/MC ratio efficiency, when we gauss smear , we need to check >0 only
                    m_Maps.insert({
                        {make_tuple( PDG::ID::E,  _lepIDM,  0 ),  GaussSmear( "E_E_0"  ,m_weightHolder.GetWeightMapsPID_fac("PID_"+       _lepName+ "_ID_brem0-effr-"+m_option))}, // Elec -> Lept brem0 Eff
                        {make_tuple( PDG::ID::E,  _lepIDM,  1 ),  GaussSmear( "E_E_1"  ,m_weightHolder.GetWeightMapsPID_fac("PID_"+       _lepName+ "_ID_brem1-effr-"+m_option))}, // Elec -> Lept brem1 Eff
                    });
                }else{
                    //NOTE: altough we insert, it doesn't update!
                    //Eps(e->e) is a direct efficiency , when we gauss smear , we need to check [0,1] bound only.
                    m_Maps.insert({
                        {make_tuple( PDG::ID::E,  _lepIDM,  0 ),  GaussSmear( "E_E_0"  ,m_weightHolder.GetWeightMapsPID_fac("PID_"+       _lepName+ "_ID_brem0-effr-"+m_option))}, // Elec -> Lept brem0 Eff
                        {make_tuple( PDG::ID::E,  _lepIDM,  1 ),  GaussSmear( "E_E_1"  ,m_weightHolder.GetWeightMapsPID_fac("PID_"+       _lepName+ "_ID_brem1-effr-"+m_option))}, // Elec -> Lept brem1 Eff
                    });                    
                }
            }
        }
        else if (_ana == Analysis::ME){
            MessageSvc::Error("PIDHistoAdderBS_SMEAR", "Analysis ME not implemented yet", "EXIT_FAILURE");
            m_Maps = {};
            return false;
        }
        else {
            MessageSvc::Error("PIDHistoAdderBS_SMEAR", "Cannot parse ana", "EXIT_FAILURE");
            m_Maps = {};
            return false;
        }    
        // for(std::map<tuple<int,int,int>,pair<TH1D*,vector<TH2D*>>>::iterator it = m_Maps.begin(); it != m_Maps.end(); ++it) {
        //     cout << "Key: ( " << get<0>(it->first) << ", " << get<1>(it->first) << ", " << get<2>(it->first) << " ) " << endl;
        //     cout << "Value: " << it->second.first << " and vector " << endl;
        // }
        MessageSvc::Info(Color::Green, "PIDHistoAdderBS_SMEAR", "Successfully Initialised PIDHistoAdder with option: " + m_option);
        MessageSvc::Info(Color::Green, "PIDHistoAdderBS_SMEAR", "BranchName: " + m_finalBranchName);
        return true;
    }
    
    ROOT::VecOps::RVec< double > operator() (double _variableP, double _variableEta, int _variableTracks, int _variableTrueID, int _variableID, int _variableBrem) {
        //The core of it 
        ROOT::VecOps::RVec< double> _values(m_nBS);// init with zeroes
        if (!m_isInit) {
            MessageSvc::Error("PIDHistoAdderBS_SMEAR", "Not Initialised", "EXIT_FAILURE");
            for (int i = 0; i < m_nBS; i++) {_values.at(i) = -1.; }
            return _values;
        }    
        if (!(m_Maps.find(make_tuple(_variableTrueID, _variableID, _variableBrem)) != m_Maps.end())) {
            //=====================================================================
            // if (TRUEID, ID, BREM) isnt found this map is a nullptr!
            // in this case i set the eff to 0.! (E->Mu and Mu->E) 
            // (some of 3222 (xi0), 3312(xi pm) are in there as well, they are ghost though in most cases)
            //=====================================================================
            if (std::find(m_failedInds.begin(), m_failedInds.end(), make_tuple(_variableTrueID, _variableID, _variableBrem)) != m_failedInds.end()) {m_failedInds.push_back(make_tuple(_variableTrueID, _variableID, _variableBrem));}
            return _values;
        }
        //=====================================================================
        // if interp in option set interp to true
        //=====================================================================
        bool _interpolate = m_option.Contains("interp") ? true : false;
        //=====================================================================
        // for KDE use interp for electron F&C maps
        //=====================================================================
        if (m_option.Contains("KDE")) {_interpolate = (_variableTrueID == 11) ? true : false;}
        
        //=====================================================================
        // the map pair is at the position (TRUEID, ID, BREM)
        //=====================================================================
        const vector< pair < TH1D*, vector< TH2D* > > > _mapToUseVector = m_Maps.at(make_tuple(_variableTrueID, _variableID, _variableBrem));

        if (_mapToUseVector.size() != m_nBS) {
            MessageSvc::Error("PIDHistoAdderBS_SMEAR", "_MapToUseVector size != " + to_string(m_nBS), "EXIT_FAILURE");
            return _values;
        }

        for (int i = 0; i<m_nBS; i++) {
            //=====================================================================
            // first element of pair at the (TRUEID, ID, BREM) spot is the nTracks Map
            // use it to find the index of the map to extract the value from
            //=====================================================================
            pair< TH1D*, vector< TH2D* > > _mapToUse = _mapToUseVector.at(i);
            TH1D * m_nTracksHist = _mapToUse.first;
            int _nTIndex        = _nTIndex = m_nTracksHist->FindFixBin(_variableTracks);
            if (m_nTracksHist->IsBinUnderflow(_nTIndex) || m_nTracksHist->IsBinOverflow(_nTIndex)) {
                if (_variableTracks <= m_nTracksHist->GetXaxis()->GetXmin()) _variableTracks = m_nTracksHist->GetXaxis()->GetXmin() + 1;
                if (_variableTracks >= m_nTracksHist->GetXaxis()->GetXmax()) _variableTracks = m_nTracksHist->GetXaxis()->GetXmax() - 1;
                _nTIndex = m_nTracksHist->FindFixBin(_variableTracks);
            }

            int _bin      = -1;
            TH2D * m_histo = _mapToUse.second.at(_nTIndex-1); // convert to vector.at() numbering from 0 to size-1
        
            if (m_histo != nullptr) {
                _bin = m_histo->FindFixBin(_variableP, _variableEta);
                if (m_histo->IsBinUnderflow(_bin) || m_histo->IsBinOverflow(_bin)) {
                    /*
                        TODO (maybe) : at edges we have to linearly extrapolate or use a fitting function from the 1D dependency For example we can have for electron maps something like: 
                        if(Eta> X)
                        eps(Eta-1D) =  X + slope ( eta - X); where slope is pre-computed. 
                        if(P> X )
                        eps(P-1D) =  X + slope ( eta - X); where slope is pre-computed.   
                        This would avoid interpolation issues for large jumps and big bin sizes.                
                    */
                    if (_variableP <= m_histo->GetXaxis()->GetXmin()) _variableP     = m_histo->GetXaxis()->GetXmin() + m_histo->GetXaxis()->GetBinWidth(1) / 100.;
                    if (_variableP >= m_histo->GetXaxis()->GetXmax()) _variableP     = m_histo->GetXaxis()->GetXmax() - m_histo->GetXaxis()->GetBinWidth(m_histo->GetNbinsX()) / 100.;
                    if (_variableEta <= m_histo->GetYaxis()->GetXmin()) _variableEta = m_histo->GetYaxis()->GetXmin() + m_histo->GetYaxis()->GetBinWidth(1) / 100.;
                    if (_variableEta >= m_histo->GetYaxis()->GetXmax()) _variableEta = m_histo->GetYaxis()->GetXmax() - m_histo->GetYaxis()->GetBinWidth(m_histo->GetNbinsY()) / 100.;
                    _bin = m_histo->FindFixBin(_variableP, _variableEta);
                }
                if( _interpolate ) {
                    _values.at(i) = m_histo->Interpolate( _variableP, _variableEta);
                } else {
                    _values.at(i) = m_histo->GetBinContent(_bin);
                }
            }
        }
        return _values;
    }

    const map<tuple<int, int, int>, vector< pair < TH1D*, vector< TH2D* > > > > getMaps(){
        //=====================================================================
        // return mMaps maps
        //=====================================================================
        return m_Maps;
    }
    const vector< tuple< int, int, int> > getFailedInds(){
        //=====================================================================
        // return failed Indices
        //=====================================================================
        return m_failedInds;
    }
    const char* branchName() const{
        //=====================================================================
        // return branchName
        //=====================================================================
        return m_finalBranchName.Data();
    }

    private:
        //=====================================================================
        // privates members
        //=====================================================================
        WeightHolder                                                            m_weightHolder;
        vector< tuple< int, int, int > >                                        m_failedInds;
        bool                                                                    m_isInit;
        map< tuple< int, int, int >, vector< pair< TH1D*, vector< TH2D* > > > > m_Maps;
        TString                                                                 m_finalBranchName;
        TString                                                                 m_option;
        int                                                                     m_nBS;
};


class DecModelWeightsAdder{
    public : 
    DecModelWeightsAdder() = default; 
    DecModelWeightsAdder( const DecModelWeightsAdder&& other) : m_hists(other.m_hists), m_ana(other.m_ana), m_isInit(other.m_isInit){
        if(!m_isInit) m_isInit=  Init();
    }
    DecModelWeightsAdder( const DecModelWeightsAdder& other)  : m_hists(other.m_hists), m_ana(other.m_ana), m_isInit(other.m_isInit){
        if(!m_isInit) m_isInit=  Init();    
    }
    DecModelWeightsAdder( Analysis & _ana ){
        m_ana = _ana; 
        m_isInit = Init(); 
    }
    bool Init(){
        TString path = "/afs/cern.ch/user/e/elsmith/work_home/public/RX_weights/";
        TString wfilename_1 = path+"RKst_low_final_weights_"+to_string(m_ana)+".root";
        TString wfilename_2 = path+"RKst_middle_final_weights_"+to_string(m_ana)+".root";
        TString wfilename_3 = path+"RKst_upper_final_weights_"+to_string(m_ana)+".root";

        auto weights_file_1= IOSvc::OpenFile(wfilename_1 , OpenMode::READ);
        auto weights_file_2= IOSvc::OpenFile(wfilename_2 , OpenMode::READ);
        auto weights_file_3= IOSvc::OpenFile(wfilename_3 , OpenMode::READ);

        m_hists = { 
            { 1 , weights_file_1->Get<THn>(TString::Format("weights_%s", to_string(m_ana).Data())) },
            { 2 , weights_file_2->Get<THn>(TString::Format("weights_%s", to_string(m_ana).Data())) },
            { 3 , weights_file_3->Get<THn>(TString::Format("weights_%s", to_string(m_ana).Data())) },
        };     MessageSvc::Warning("Scaling histograms");
        if(m_ana == Analysis::EE){
            m_hists[1]->Scale( 0.19880942/0.3040990);
            m_hists[2]->Scale( 0.035392303/0.039658531);
            m_hists[3]->Scale( 0.76579828/0.65624241);
        }else{	
            m_hists[1]->Scale( 0.7464806600635691) ;
            m_hists[2]->Scale( 0.7779076344519419) ;
            m_hists[3]->Scale( 1.0192005293948623) ;
        }
        return true;
    };
    double operator()(const double & q2,const double & ctl,const double & ctk,const double & phi){ 
        double vals[4] = {q2,ctl,ctk,phi};
        int _index = -1;
        if(q2<0.1){
            _index =1;
        }
        if(q2>=0.1 && q2<0.4){
            _index = 2;
        }
        if(q2>=0.4 && q2<18.5){
            _index = 3;
        }
        if( _index == -1 && q2 > 18.5){ return 0.; } //return -1000.;
        if( _index == -1 ){
            // std::cout<<"q2 not assigned, it is"<<q2<<std::endl;      
            // return -5000; //not assigned q2 
            return 0.;
        }
        double ValReturn = m_hists[_index]->GetBinContent(m_hists[_index]->GetBin(vals));
        if(ValReturn <0) MessageSvc::Error("Negative weight, invalid", "","EXIT_FAILURE");
        return ValReturn;
    };
    private:
        bool m_isInit = false; 
        std::map<int,THn* >  m_hists;
        Analysis m_ana;
};


/**
* \class TH1DHistoAdder
* \brief Functor class to attach weights from TH1D for 1D corrections ( used for L0/HLT corrections )
*/
class TH1FHistoAdder {
public:
    TH1FHistoAdder( TH1F & h , 
                    TString _finalBranchName, 
                    bool _interpolate, 
                    TString _infoMapSource ) : m_histo(h) , m_finalBranchName(_finalBranchName), m_interpolate(_interpolate) , m_infoMapSource(_infoMapSource) {}
    TH1FHistoAdder( const TH1FHistoAdder&& other) : m_histo(other.m_histo), m_finalBranchName(other.m_finalBranchName), m_interpolate(other.m_interpolate) , m_infoMapSource(other.m_infoMapSource){}
    TH1FHistoAdder( const TH1FHistoAdder& other)  : m_histo(other.m_histo), m_finalBranchName(other.m_finalBranchName), m_interpolate(other.m_interpolate) , m_infoMapSource(other.m_infoMapSource){}
    double operator()(double _variable){ 
      double _val = 1.0;
      double _var = _variable;
      int _bin = m_histo.FindFixBin(_var);
      if (m_histo.IsBinUnderflow(_bin) || m_histo.IsBinOverflow(_bin)) {
          if (_var <= m_histo.GetXaxis()->GetXmin()) _var = m_histo.GetXaxis()->GetXmin() + m_histo.GetXaxis()->GetBinWidth(1) / 100.;
          if (_var >= m_histo.GetXaxis()->GetXmax()) _var = m_histo.GetXaxis()->GetXmax() - m_histo.GetXaxis()->GetBinWidth(m_histo.GetNbinsX()) / 100.;
          _bin = m_histo.FindFixBin(_var);
      }
      if( !m_interpolate){
          _val = m_histo.GetBinContent(_bin);
      }else{
          _val = m_histo.Interpolate( _var);
      }
      return _val;
    }
    const char* branchName()const{
        return m_finalBranchName.Data();
    }
    const TString sourceHisto() const{
        return m_infoMapSource;
    }
private:
    TH1F    m_histo;
    TString m_finalBranchName;
    TString m_infoMapSource;
    bool m_interpolate;
};




/**
* \class Q2SmearCorrection
* \brief Functor class to attach Q2 smearing to EE ntuples using Yaml files saving parameters to use
*/
class Q2SmearDifferential{
    //Only supports 1-s-scale , 1 mass shift 1 mass ean value
    public:
    Q2SmearDifferential( const Prj & _prj , const Year & _year , TString _finalBranchName ) : m_project(_prj)  , m_finalBranchName(_finalBranchName) { Init(); };
    Q2SmearDifferential( const Q2SmearDifferential&& other) : m_project(other.m_project),  m_finalBranchName(other.m_finalBranchName){ Init();}
    Q2SmearDifferential( const Q2SmearDifferential& other)  : m_project(other.m_project),  m_finalBranchName(other.m_finalBranchName){ Init();}   
    double operator()(   double jps_mass_reco, double jps_mass_true, int bkgcat , double e1_brems, double e2_brems , int year  , bool isL0I  , double inputVariable )const{
        if(jps_mass_true< -0.1){
            return -1.; //this is an invalid case where the porting from MCDT has not work, those events must be always rejected when cutting on q2 smeared values!
        }
        Year _year = Year::All;
        if( year < 13 && year > 10) _year = Year::Run1;
        else if( year < 17 && year > 10 ) _year = Year::Run2p1;
        else if( year < 19 && year > 10) _year = Year::Run2p2;
        if(_year == Year::All) MessageSvc::Error("Invalid read of year in event loop, break, and crash","","EXIT_FAILURE");
        Trigger _trigger = isL0I ? Trigger::L0I : Trigger::L0L ; 

        int bremCat = (int)e1_brems + (int)e2_brems;
        bremCat = bremCat >=2 ? 2 : bremCat;
        if( bremCat <0 || bremCat >2) MessageSvc::Error("Q2SmearCorrection Invalid","","EXIT_FAILURE");        
        double _trueMass = jps_mass_true;
        double _recoMass = jps_mass_reco;
        if(bkgcat==60){
            //BKGCAT60 ==> trueM == recoM;
            _trueMass = _recoMass;
        }
        // now :  left and right logic to use 
        // s_scale :  sigma Data / sigma MC 
        // There is an extra sigma Factor on top to account for
        // recoMass - ( trueMass - ( FSR shift) ) - massMCFited is with a sigma the distance from the mean has to


        auto ID =  make_pair( _year, _trigger);

        double _var = inputVariable;
      
        int _binIndex = m_sigmaScale.at( ID).at(bremCat).FindFixBin(_var);
        if (m_sigmaScale.at( ID).at(bremCat).IsBinUnderflow(_binIndex) || m_sigmaScale.at( ID).at(bremCat).IsBinOverflow(_binIndex)) {
            if (_var <= m_sigmaScale.at( ID).at(bremCat).GetXaxis()->GetXmin()){ 
                _var = m_sigmaScale.at( ID).at(bremCat).GetXaxis()->GetXmin() + m_sigmaScale.at( ID).at(bremCat).GetXaxis()->GetBinWidth(1) / 100.;
            }
            if (_var >= m_sigmaScale.at( ID).at(bremCat).GetXaxis()->GetXmax()){
                auto nXBins = m_sigmaScale.at( ID).at(bremCat).GetNbinsX();
                _var = m_sigmaScale.at( ID).at(bremCat).GetXaxis()->GetXmax() - m_sigmaScale.at( ID).at(bremCat).GetXaxis()->GetBinWidth(nXBins) / 100.;
            }
        }
        _binIndex = m_sigmaScale.at( ID).at(bremCat).FindFixBin(_var);        


        double sScale     = m_sigmaScale.at( ID).at(bremCat).GetBinContent( _binIndex);
        double mShift     = m_massShift.at( ID).at(bremCat).GetBinContent( _binIndex);
        double massMCFit  = m_massMCFit.at( ID).at(bremCat).GetBinContent( _binIndex);

        // double sScale     = m_sigmaScale.at( ID)[bremCat].Interpolate( _var);
        // double mShift     = m_massShift.at( ID)[bremCat].Interpolate( _var);
        // double massMCFit  = m_massMCFit.at( ID)[bremCat].Interpolate( _var);

        //have to figure out if we want to interpolate SMEAR       
        // if( !m_interpolate){
        // _val = m_histo.GetBinContent(_bin);
        // }else{
        // _val = m_histo.Interpolate( _var);
        // }
        

        double  _correctedJPsMass = _trueMass 
                                +  sScale  * ( _recoMass - _trueMass)
                                +  mShift  + 
                                ( 1. - sScale) * (massMCFit - PDG::Mass::JPs);     
        return _correctedJPsMass;               
    }
    void Init(){
        LoadSmearingParameters();
        return;
    }
    // void Print(){
    //     MessageSvc::Info("Q2SmearingDifferenttial correction parameters loaded for ", TString(branchName()));
    //     for( auto year : m_loadYears){
    //         cout<< BLUE << "#) Setups for year " << to_string(year) << RESET<< endl;
    //         for( int i = 0 ; i < 3; ++i){
    //             cout<< CYAN << "          "<< i << "G" <<  endl;
    //             if(m_hasLRSigmaScale){
    //                 cout<< CYAN << "s_scaleL  | "<< m_sigmaScaleL.at(year).at(i)<< endl;
    //                 cout<< CYAN << "s_scaleR  | "<< m_sigmaScaleR.at(year).at(i)<< endl;
    //             }else{
    //                 cout<< CYAN << "s_scale   | "<< m_sigmaScale.at(year).at(i)<< endl;
    //             }
    //             cout<< CYAN << "m_shift  | "<< m_massShift.at(year).at(i)<< endl;
    //             cout<< CYAN << "m_MCFit  | "<< m_massMCFit.at(year).at(i)<< RESET<<endl;
    //         }
    //     }
    //     return;
    // }
    void LoadSmearingParameters(){

        MessageSvc::Info("LoadSmearingParameters" , to_string(m_project));
        for( auto _runPeriod : m_loadYears){
            for( auto & _trigger : m_loadTriggers){
                auto _ID = make_pair( _runPeriod, _trigger);
                m_sigmaScale[_ID].resize(3);
                m_massMCFit[_ID].resize(3);
                m_massShift[_ID].resize(3);
                int idx = 0; 
                vector<TString> brems{"Brem0","Brem1","Brem2"};
                for( auto & brem : brems){
                    TString _eosPath  = TString::Format("/eos/lhcb/wg/RD/RKstar/weights/v%s/q2smearDifferential/%s/", SettingDef::Tuple::gngVer.Data(), SettingDef::Weight::q2SmearDiffVar.Data()) ; 
                    TString _fileLoad = TString::Format("%s_%s_%s_%s_%s.root", SettingDef::Weight::q2SmearDiffVar.Data() , to_string(m_project).Data(), to_string( _runPeriod ).Data(), to_string(_trigger ).Data(), brem.Data() );
                    TString _loadFile = _eosPath+_fileLoad; 
                    TFile *f = IOSvc::OpenFile(_loadFile, OpenMode::READ);                    
                    TH1D * shift  = f->Get<TH1D>("mShift");
                    TH1D * scale  = f->Get<TH1D>("sScale");
                    TH1D * massMC = f->Get<TH1D>("mMCFit");
                    shift->SetDirectory(0);
                    scale->SetDirectory(0);
                    massMC->SetDirectory(0);
                    MessageSvc::Info("Loading histograms from", _fileLoad);
                    m_sigmaScale[_ID][idx] = (*scale);
                    m_massShift[_ID][idx]  = (*shift);
                    m_massMCFit[_ID][idx]  = (*massMC);
                    // m_sigmaScale[_ID][idx]
                    // m_massShift[_ID][idx]
                    // m_massMCFit[_ID][idx]
                    IOSvc::CloseFile(f);
                    idx++;
                }
            }
        }
        return;
    }
    const char* branchName()const{
        return m_finalBranchName.Data();
    }
    private : 
    Prj m_project ; 
    vector< Year>       m_loadYears{ Year::Run1, Year::Run2p1, Year::Run2p2 };
    vector< Trigger> m_loadTriggers{ Trigger::L0I , Trigger::L0L };
    TString m_finalBranchName;
    map< pair< Year , Trigger> , std::vector<TH1D> > m_sigmaScale; //for yamls having the sigmaScale  unique
    map< pair< Year , Trigger> , std::vector<TH1D> > m_massMCFit; 
    map< pair< Year , Trigger> , std::vector<TH1D> > m_massShift; 
};

#endif // !HISTOADDERS_HPP
