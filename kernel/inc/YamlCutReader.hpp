#ifndef YAMLCUTREADER_HPP
#define YAMLCUTREADER_HPP
#include "MessageSvc.hpp"
#include "ParserSvc.hpp"
#include "IOSvc.hpp"
#include <yaml-cpp/yaml.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <yaml-cpp/yaml.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <TCut.h>

using cut_class = std::string;  // Define cut_class externally

class YamlCutReader {
    /*
    Helper Class to read a yaml file structured as follows : 

    Categories : 
        mydefinition_cat : "B_DTF_JPsi_M > 5100 && ..."
        mydefinition_cat2 : "XXXX > 3045"
    Selections : 
        default_selection : 
            - mydefinition_cat
            - mydefinition_cat2

    --> read a map< category, cut> 
    --> read a map< selectiondef, vector<cut> > where vector is filled based on categories upstream
    This class handles the reading of this form for a selection and get back the string and TCut combination or return vector<TCut>

    */
    public:
        YamlCutReader() = default;
        YamlCutReader(const std::string& input_file_yaml);

        // Open and read the file, fill the cut_categories and cut_definitions
        bool Init();

        // Return the expanded list of TCut objects for a given selection_id
        const std::vector<TCut> GetSelections(const std::string& selection_id);  
        const TCut GetSelection(const std::string& selection_id);  

        // Print categories and selections (for debugging purposes)
        void PrintData() const; 

        // Getters for categories and selections
        const std::map<cut_class, std::string>& GetCategories() const { return m_cut_categories; }
        const std::map<cut_class, std::vector<std::string>>& GetSelectionsDefinitions() const { return m_cut_definitions; }

    private: 
        std::string m_file_read; 
        std::map<cut_class, std::string>  m_cut_categories; 
        std::map<cut_class, std::vector<std::string>>  m_cut_definitions;

    ClassDef(YamlCutReader,0);

};
#endif 