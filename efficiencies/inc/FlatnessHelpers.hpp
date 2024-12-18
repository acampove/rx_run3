#ifndef FLATNESS_HELPERS_HPP
#define FLATNESS_HELPERS_HPP

#include "EnumeratorSvc.hpp"

// A collection of functions for the flatness

class FlatnessHelpers {
  public:
    /**
     * @brief Return the value Luminosity * production cross section *  Luminosity in a given year/polarity [4PI acceptance]
     * @param _project
     * @param _year
     * @param _polarity
     * @param _ana
     * @return double
     */
    static double ProducedDecays(const Prj & _project, const Year & _year, const Polarity & _polarity, const Analysis & _ana);

    /**
     * @brief Get the Years And Polarities for a given year, a vector holding pairs basically
     *
     * @param _year  ( merged/unmerged years to span over )
     * @return vector< pair< Year, Polarity > >
     */
    static vector< pair< Year, Polarity > > GetYearsAndPolarities(const Year & _year);
};

/**
 * @brief Number to use for calculations
 *  Each number has a value, error and a name.
 * Implementatio of Products , Divisions, Sum, Difference propagating errors simply ( with correlation terms as well )
 */
struct Number {
    double  value;
    double  error;
    TString name;
    Number() = default;
    /**
     * @brief Construct a new Number object
     *
     * @param x [value]
     * @param err  [error]
     * @param _name   [name]
     */
    Number(double _x, double _err, TString _name)
        : value(_x)
        , error(_err)
        , name(_name){};
    Number(RooRealVar * _var, TString _name)
        : name(_name) {
        if (_var == nullptr) { MessageSvc::Error("var is null pointer ", name, "EXIT_FAILURE"); }
        value = _var->getVal();
        error = _var->getError();
    };
    /**
     * @brief Make a RooRealVar out of it
     *
     * @return RooRealVar* (new!!!) free it when needed
     */
    RooRealVar * makevar() {
        auto * v = new RooRealVar(name, name, value);
        v->setError(error);
        return v;
    }

    /**
     * @brief Get The Name of the Number
     */
    TString Name() const { return name; }
    /**
     * @brief Relative Error (err/value)
     */
    double RelError() const { return error / value; }
    /**
     * @brief Relative Error ^{2} (Error/Value)^{2}
     */
    double RelError2() const { return RelError() * RelError(); }
    /**
     * @brief Raw Error
     */
    double Err() const { return error; }
    /**
     * @brief Error^{2}
     */
    double Err2() const { return error * error; }
    /**
     * @brief Value
     */
    double Val() const { return value; }
    /**
     * @brief Value2
     */
    double Val2() const { return value * value; }
    /**
     * @brief print the name and the error    (DEPRECATED ? )
     */
    void print() const { cout << " Value is " << value << " +/- " << error << endl; }
    void Print() const { std::cout << Name() << "  :   " << Val() << " +- " << Err() << " ( " << RelError() * 100 << " % ) " << std::endl; }
    /**
     * @brief ADD 2 Numbers and add correlation term
     *
     * @param a
     * @param b
     * @param _corr
     * @return Number
     */
    static Number Plus(const Number & a, const Number & b, double _corr = 0.) {
        TString _name = TString("(") + a.Name() + " + " + b.Name() + TString(")");
        double  value = a.Val() + b.Val();
        double  err   = std::sqrt(a.Err2() + b.Err2() + 2 * _corr);
        return Number(value, err, _name);
    }
    static Number Minus(const Number & a, const Number & b, double _corr = 0.) {
        TString _name = TString("(") + a.Name() + " - " + b.Name() + TString(")");
        double  value = a.Val() - b.Val();
        double  err   = std::sqrt(a.Err2() + b.Err2() - 2 * _corr);
        return Number(value, err, _name);
    }
    static Number Product(const Number & a, double scale) {
        TString _name = TString("( ") + to_string(scale) + " * " + a.Name() + TString(" )");
        return Number(a.Val() * scale, a.Err() * scale, _name);
    }
    static Number Product(const Number & a, const Number & b, double corr = 0.) {
        double  value = a.Val() * b.Val();
        double  err   = value * sqrt(a.RelError2() + b.RelError2());
        TString _name = TString("(") + a.name + " * " + b.name + ")";
        return Number(value, err, _name);
    }
    static Number Divide(const Number & a, const Number & b, double corr = 0.) {
        TString _name = TString("(") + a.name + " / " + b.name + ")";
        double  value = a.Val() / b.Val();
        double  err   = std::sqrt(a.Err2() / b.Val2() + a.Val2() / (b.Val2() * b.Val2()) * b.Err2() - 2 * (a.Val() / b.Val2()) * corr / b.Val());
        return Number(value, err, _name);
    }
};

#endif