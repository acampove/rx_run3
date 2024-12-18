#ifndef ZIPFOR_H
#define ZIPFOR_H

#include <boost/range/iterator_range.hpp>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tuple/tuple.hpp>
#include <cstdlib>
#include <iterator>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>

template < class... Conts > auto zip_range(Conts &... conts) -> decltype(boost::make_iterator_range(boost::make_zip_iterator(boost::make_tuple(conts.begin()...)), boost::make_zip_iterator(boost::make_tuple(conts.end()...)))) 
{ 
    auto it_1 = boost::make_zip_iterator(boost::make_tuple(conts.begin()...));
    auto it_2 = boost::make_zip_iterator(boost::make_tuple(conts.end()...));

    return {it_1, it_2}; 
}

#endif
