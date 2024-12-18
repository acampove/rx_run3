#ifndef PDGROUNDING_HPP
#define PDGROUNDING_HPP

#include <boost/math/special_functions/fpclassify.hpp>
#include <boost/math/special_functions/round.hpp>
#include <boost/math/special_functions/powm1.hpp>
#include <boost/lexical_cast.hpp>

namespace pdgrounding {
    /*!
    \brief Class for PDG rounding
    */
    double round_at_position(const double in_decimal, int decimal_point = 0) {
      namespace bmath = boost::math;
      double power = bmath::powm1(double(10.), decimal_point) + double(1.0);
      return bmath::round(in_decimal * power) / power;
    }

    std::string double_to_string(const double value, const int precision = 2) {
      std::stringstream to_string;
      to_string.setf(std::ios::fixed);
      if (precision <= 0 ) {
        to_string.unsetf(std::ios::showpoint);
        to_string.precision(0);
        long double rounded(round_at_position(value, precision));
        to_string << rounded;
        return to_string.str();
      }
      else {
        to_string.setf(std::ios::showpoint);
        to_string.precision(precision);
      }
      to_string << value;
      return to_string.str();
    }

    std::string GetPDGRounded(const double value, const double error) {
        std::stringstream to_string;
        to_string.setf(std::ios::scientific | std::ios::showpoint);
        to_string.precision(2);
        to_string << error;
        std::string a_string = to_string.str();
        std::string exponential = a_string;
        a_string.erase(a_string.find_first_of("Ee"), std::string::npos);
        double three_digit_error = boost::lexical_cast<double>(a_string);
          std::string number = exponential.substr(exponential.find_last_of("Ee") + 1, std::string::npos);
          int error_exponent = boost::lexical_cast<int>(number);
        int shift_precision = 1;
        if (three_digit_error > 0.9999 && three_digit_error < 3.55) shift_precision = 1;
        else if (three_digit_error >= 3.55 && three_digit_error < 9.50) shift_precision = 0;
        else if (three_digit_error >= 9.50) {
          shift_precision = 1;
          three_digit_error = 1.000;
          error_exponent += 1;//rounding up
        }
        //error
        std:string string_error = "";
        double output = round_at_position(three_digit_error, shift_precision) * pow(10., error_exponent);
        if (error_exponent > 0) string_error = double_to_string(output, 0);
        else string_error =  double_to_string(output, std::abs(error_exponent) + shift_precision);

        //value
        std::string string_value = "";
        double _value   = round_at_position(value, -error_exponent + shift_precision);
        if( error_exponent > 0) string_value = double_to_string(_value, 0);
        else                     string_value = double_to_string(_value, std::abs(error_exponent) + shift_precision);

        cout << value << " +/- " << error << " is rounded to: " << string_value << " +/- " << string_error << endl;
        return string_value + "\\pm" + string_error;
    }

}   // namespace pdgrounding

#endif /* end of include guard: PDGROUNDING_HPP */
