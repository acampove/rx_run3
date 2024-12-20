#ifndef YAMLCUTREADER_CPP
#define YAMLCUTREADER_CPP


#include "YamlCutReader.hpp"

ClassImp(YamlCutReader)

YamlCutReader::YamlCutReader( const std::string & input_file_yaml){
    m_file_read= input_file_yaml;
    Init();
}

bool YamlCutReader::Init(){
    try {
        // Load the YAML file
        YAML::Node config = YAML::LoadFile(m_file_read);
        
        // Parse "Categories"
        if (config["Categories"]) {
            for (const auto& category : config["Categories"]) {
                std::string key = category.first.as<std::string>();
                std::string value = category.second.as<std::string>();
                m_cut_categories[key] = value;
            }
        } else {
            std::cerr << "No 'categories' key found in the YAML file." << std::endl;
            return false;
        }

        // Parse "Selections"
        if (config["Selections"]) {
            for (const auto& selection : config["Selections"]) {
                std::string key = selection.first.as<std::string>();
                std::vector<std::string> values;
                for (const auto& item : selection.second) {
                    values.push_back(item.as<std::string>());
                }
                m_cut_definitions[key] = values;
            }
        } else {
            std::cerr << "No 'selections' key found in the YAML file." << std::endl;
            return false;
        }

        return true; // Successful parsing

    } catch (const YAML::Exception& e) {
        std::cerr << "Error parsing YAML file: " << e.what() << std::endl;
        return false;
    }
} 


void YamlCutReader::PrintData() const {
    MessageSvc::Info("YamlCutReader::PrintData()",(TString)"Cut Categories");
    for (const auto& [key, value] : m_cut_categories) {
        MessageSvc::Info(Color::Green,  TString(key), TString(value));
    }
    MessageSvc::Info("YamlCutReader::PrintData()", (TString)"Selections");
    for (const auto& [key, values] : m_cut_definitions) {
        MessageSvc::Debug("Category", (TString)key);
        for (const auto& val : values) {
            MessageSvc::Debug("-cut =", (TString)val);
            if( !m_cut_categories.contains(val)){
                MessageSvc::Warning("this category is not defined, please add it!");
            }
        }
    }
}



// Return the expanded list of TCut objects for a given selection_id
const std::vector<TCut> YamlCutReader::GetSelections(const std::string& selection_id) {
    std::vector<TCut> expanded_cuts;
    try {
        const auto& category_keys = m_cut_definitions.at(selection_id); // Get list of category keys for this selection
        for (const auto& key : category_keys) {
            auto it = m_cut_categories.find(key);
            if (it != m_cut_categories.end()) {
                expanded_cuts.push_back(TCut(it->second.c_str())); // Create a TCut from the category value
            } else {
                 MessageSvc::Error("Warning: Category key '", (TString)key , "' not found in cut categories.","EXIT_FAILURE");
            }
        }
    } catch (const std::out_of_range& e) {
        MessageSvc::Error("Selection ID '", (TString)selection_id , "' not found in cut definitions.","EXIT_FAILURE");
    }
    return expanded_cuts;
}

const TCut YamlCutReader::GetSelection( const  std::string& selection_id ){
    auto cuts = GetSelections( selection_id);
    TCut _finalCut("");
    for ( auto & cut : cuts){
        _finalCut = _finalCut && cut;
    }
    return _finalCut;
}
#endif
