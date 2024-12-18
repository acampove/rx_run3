#ifndef VEC_EXTENDS
#define VEC_EXTENDS

#include "SettingDef.hpp"

#include "TString.h"
#include "TObjString.h"

#include <vector>

/**
 * \brief Check if element is inside a vector
 * @param  _vec   [description]
 * @param  _value [description]
 */
template < typename T > bool CheckVectorContains(vector< T > & _vec, T _value) { return find(_vec.begin(), _vec.end(), _value) != _vec.end(); };

template < typename T > bool CheckVectorContains(const vector< T > & _vec, T _value) { return find(_vec.begin(), _vec.end(), _value) != _vec.end(); };

template < typename T > void RemoveVectorDuplicates(T & _vec) {
    sort(_vec.begin(), _vec.end());
    _vec.erase(unique(_vec.begin(), _vec.end()), _vec.end());
    return;
};

template < typename T, typename S > vector< T > GetVectorFirst(vector< pair< T, S > > & _pair) {
    vector< T > _vec;
    transform(_pair.begin(), _pair.end(), back_inserter(_vec), mem_fn(&pair< T, S >::first));
    return _vec;
};

template < typename T, typename S > vector< T > GetVectorSecond(vector< pair< T, S > > & _pair) {
    vector< T > _vec;
    transform(_pair.begin(), _pair.end(), back_inserter(_vec), mem_fn(&pair< T, S >::second));
    return _vec;
};

/**
 * GetValue (assumes "-finder[value]")
 * @param  _string [description]
 * @param  _finder [description]
 * @param  _value  [description]
 */
template < typename T > void GetValue(TString _string, const TString & _finder, T & _value) {
    auto * _strCollection = _string.Tokenize(SettingDef::separator);
    for (auto _opt : *_strCollection) {
        TString _optstr(((TObjString *) _opt)->String());
        if (_optstr.Contains(_finder)) {
            // remove the parenthesis for the _value
            _optstr.ReplaceAll("[", "").ReplaceAll("]", "");
            // remove the _finder _string
            _optstr.ReplaceAll(_finder, "");
            // we are left with a number
            _value = static_cast< T >(_optstr.Atof());
            break;
        }
    }
    return;
};

/**
 * Easy concatenate of vector : C = A + B . Merge the 2 to a new one
 */
template < typename T > vector< T > operator+(const vector< T > & A, const vector< T > & B) {
    vector< T > AB;
    AB.reserve(A.size() + B.size());
    AB.insert(AB.end(), A.begin(), A.end());
    AB.insert(AB.end(), B.begin(), B.end());
    return AB;
}

/**
 * Enable C+=A. C now has C entries + A entries
 */
template < typename T > vector< T > & operator+=(vector< T > & A, const vector< T > & B) {
    A.reserve(A.size() + B.size);
    A.insert(A.end(), B.begin(), B.end());
    return A;
}


template< typename T> vector< T > LineSpace( const T & start_in, const T & end_in,  int num_steps){
    /* A dummy linespace vector creator with start/end and nSteps configuratble */
    std::vector<double> linspaced;
    double start(start_in);
    double end(end_in);
    double num(num_steps);
    if (num == 0) { return linspaced; }
    if (num == 1) {
        linspaced.push_back(start);
        return linspaced;
    }
    double delta = (end - start) / (num - 1);
    for(int i=0; i < num-1; ++i){
        linspaced.push_back(start + delta * i);
    }
    linspaced.push_back(end); // I want to ensure that start and end
                                // are exactly the same as the input
    return linspaced;
};

#endif
