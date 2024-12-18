#ifndef LATABLE_CPP
#define LATABLE_CPP

#include "LaTable.hpp"

#include "MessageSvc.hpp"

void LaTable::Create(const string _name, const string _caption, const uint _nColumns) {
    m_nColumns   = _nColumns;
    m_rowEntries = 0;
    m_file.open(_name, ios::out);
    MessageSvc::Line();
    if (m_file.fail()) {
        MessageSvc::Warning("LaTable", (TString) "Could not open file:  ", (TString) _name);
    } else {
        MessageSvc::Info("LaTable", (TString) "Create file: ", (TString) _name);
    }
    string _columnString(m_nColumns, 'c');

    m_file << "\\documentclass[a4paper,11pt,twoside,BCOR=15mm,DIV=12]{scrbook}\n";
    m_file << "\\usepackage{float}\n";
    m_file << "\\usepackage{booktabs}\n";
    m_file << "\\usepackage{caption}\n";
    m_file << "\n\n";

    m_file << "\\begin{document}\n";
    m_file << "\n\n";
    m_file << "\\begin{table}[htbp]\n";
    m_file << "\\begin{center}\n";
    m_file << "\\caption{" << _caption << "}\n";
    m_file << "\\begin{tabular}{" << _columnString << "}\n";

    return;
}

void LaTable::AddRow(const vector< string > & _entries) {
    if (_entries.size() != m_nColumns) { MessageSvc::Warning("LaTable", (TString) "Warning: Number of entries does not match table columns"); }
    uint _num = _entries.size();
    if (_num > m_nColumns) _num = m_nColumns;
    for (uint i = 0; i < _num; ++i) { AddEntry(_entries.at(i)); }
    FinalizeRow();
    return;
}

void LaTable::AddRow(const vector< double > & _entries) {
    if (_entries.size() != m_nColumns) { MessageSvc::Warning("LaTable", (TString) "Warning: Number of entries does not match table columns"); }
    uint _num = _entries.size();
    if (_num > m_nColumns) _num = m_nColumns;
    for (uint i = 0; i < _num; ++i) { AddEntry(_entries.at(i)); }
    FinalizeRow();
    return;
}

void LaTable::AddRow(const vector< double > & _values, const vector< double > & _errors) {
    if (_values.size() != m_nColumns) { MessageSvc::Warning("LaTable", (TString) "Warning: Number of entries does not match table columns"); }
    uint _num = _values.size();
    if (_num > m_nColumns) _num = m_nColumns;
    for (uint i = 0; i < _num; ++i) { AddEntry(_values.at(i), _errors.at(i)); }
    FinalizeRow();
    return;
}

void LaTable::AddEntry(const string _entry) {
    if (m_rowEntries == m_nColumns) FinalizeRow();
    if (m_rowEntries > 0) m_file << "\t& ";
    m_file << _entry;
    ++m_rowEntries;
    return;
}

void LaTable::AddEntry(const double _entry) {
    if (m_rowEntries == m_nColumns) FinalizeRow();
    if (m_rowEntries > 0) m_file << "\t& ";
    m_file << _entry;
    ++m_rowEntries;
    return;
}

void LaTable::AddEntry(const double _value, const double _error) {
    if (m_rowEntries == m_nColumns) FinalizeRow();
    if (m_rowEntries > 0) m_file << "\t& ";
    m_file << "$" << _value << "\\pm" << _error << "$";
    ++m_rowEntries;
    return;
}

void LaTable::FinalizeRow() {
    if (m_rowEntries == 0) return;
    for (uint i = m_rowEntries; i < m_nColumns; ++i) m_file << "\t& ";
    m_file << "\t\\tabularnewline\n";
    m_rowEntries = 0;
    return;
}

void LaTable::TopRule() {
    FinalizeRow();
    m_file << "\\toprule\n";
    return;
}

void LaTable::MidRule() {
    FinalizeRow();
    m_file << "\\midrule\n";
    return;
}

void LaTable::BottomRule() {
    FinalizeRow();
    m_file << "\\bottomrule\n";
    return;
}

void LaTable::Close() {
    FinalizeRow();
    m_file << "\\end{tabular}\n";
    m_file << "\\end{center}\n";
    m_file << "\\end{table}\n";
    m_file << "\n\n";
    m_file << "\\end{document}\n";

    MessageSvc::Info("LaTable", (TString) "Close file");
    MessageSvc::Line();

    m_file.close();

    return;
}

#endif