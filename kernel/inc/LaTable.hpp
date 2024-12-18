#ifndef LATABLE_HPP
#define LATABLE_HPP

#include <fstream>
#include <iostream>
#include <vector>

using namespace std;

/** LaTable is a tool to automatically produce LaTax tables
 * usage example:
 * @code
 * LaTable _table;
 * const uint _nColumns = 3;
 * _table.Create("test_table.tex", "my caption", _nColumns);
 * vector<string> _header = {"my", "new", "table"};
 * _table.AddRow(_header);
 *
 * _table.TopRule();
 *
 * vector<double> _entries = {1., 2., 3.};
 * _table.AddRow(_entries);
 * _entries = {1., 2.};
 * _table.AddRow(_entries);
 * _entries = {1., 2., 3., 4.};
 * _table.AddRow(_entries);
 *
 * _table.MidRule();
 *
 * vector<double> _values = {5., 5., 5.};
 * vector<double> _errors = {0.1, 0.2, 0.3};
 * _table.AddRow(_values, _errors);
 *
 * _table.MidRule();
 * _table.AddEntry(7.);
 * _table.AddEntry("seven");
 * _table.AddEntry(7., 0.7);
 * _table.AddEntry(7.);
 *
 * _table.BottomRule();
 * _table.Close();
 * @endcode
 *
 * This will create a file called test_table.tex
 * @code
 * \documentclass[a4paper,11pt,twoside,BCOR=15mm,DIV=12]{scrbook}
 * \usepackage{float}
 * \usepackage{booktabs}
 * \usepackage{caption}
 *
 *
 * \begin{document}
 *
 *
 * \begin{table}[htbp]
 * \begin{center}
 * \caption{my caption}
 * \begin{tabular}{ccc}
 * my  & new   & table \tabularnewline
 * \toprule
 * 1   & 2 & 3 \tabularnewline
 * 1   & 2 &   \tabularnewline
 * 1   & 2 & 3 \tabularnewline
 * \midrule
 * $5\pm0.1$   & $5\pm0.2$ & $5\pm0.3$ \tabularnewline
 * \midrule
 * 7   & seven & $7\pm0.7$ \tabularnewline
 * 7   &   &   \tabularnewline
 * \bottomrule
 * \end{tabular}
 * \end{center}
 * \end{table}
 *
 *
 * \end{document}
 * @endcode
 */
class LaTable {
  public:
    void Create(const string _name, const string _caption, const uint _nColumns);
    void AddRow(const vector< string > & _entries);
    void AddRow(const vector< double > & _entries);
    void AddRow(const vector< double > & _values, const vector< double > & _errors);
    void AddEntry(const string _entry);
    void AddEntry(const double _entry);
    void AddEntry(const double _value, const double _error);
    void TopRule();
    void MidRule();
    void BottomRule();
    void Close();

  private:
    void    FinalizeRow();
    uint    m_nColumns;
    uint    m_rowEntries;
    fstream m_file;
};

#endif
