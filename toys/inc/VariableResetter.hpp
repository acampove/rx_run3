#ifndef VARIABLERESETTER_HPP
#define VARIABLERESETTER_HPP

#include "RooAbsPdf.h"
#include "RooRealVar.h"

#include <vector>

using namespace std;

/** \class VariableResetter
 * \brief A class to reset RooRealVar to their initial values. This will reset PDF shapes of toy fits.
 */
class VariableResetter {

  public:
    /**
     * \brief Default constructor.
     */
    VariableResetter(){};
    /**
     * \brief The destructor.
     */
    ~VariableResetter(){};

    /**
     * \brief Adds a RooRealVar to reset when calling ResetAllVariables().
     * \note The initial value is recorded when AddVariable(RooRealVar& variable) is called.
     * \param variable The RooRealVar to reset.
     */
    void AddVariable(RooRealVar & variable);
    /**
     * \brief Adds all RooRealVar nodes extracted from the instance of a derived RooAbsPdf class to the list of RooRealVar to reset when calling ResetAllVariables().
     * \note The initial values are recorded when AddVariablesFromModel(RooAbsPdf& model) is called.
     * \param model Instance of a class derived from RooAbsPdf to extract RooRealVar from.
     */
    void AddVariablesFromModel(RooAbsPdf & model);
    /**
     * \brief Resets all RooRealVar to their initial values when passed to AddVariable(RooRealVar& variable) or AddVariablesFromModel(RooAbsPdf& model)
     */
    void ResetAllVariables();

  private:
    bool notDuplicated(RooRealVar & variable);
    void appendVariable(RooRealVar & variable);

  private:
    vector< RooRealVar * > m_listOfVariables;
    vector< double >       m_initialValues;
    vector< double >       m_initialErrors;
};

#endif