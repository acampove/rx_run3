#ifndef YAMLCPP_H
#define YAMLCPP_H

#include "EnumeratorSvc.hpp"
#include "HelperSvc.hpp"

#include "vec_extends.h"
#include "yaml-cpp/yaml.h"

#include "TString.h"

#include "RooRealVar.h"

/**
 * \namespace YAML
 * \brief YAML custom types
 */
namespace YAML {

    template <> struct convert< TString > {
        static Node encode(const TString & st) {
            Node node;
            node.push_back(st);
            return node;
        }

        static bool decode(const Node & node, TString & st) {
            if (node.IsSequence()) return false;
            st = TString(node.as< string >());
            return true;
        }
    };

    template <> struct convert< vector< TString > > {
        static Node encode(const vector< TString > & st) {
            Node node;
            for (auto & el : st) { node.push_back(el); }
            return node;
        }
        static bool decode(const Node & node, vector< TString > & vst) {
            if (!node.IsSequence()) return false;
            for (size_t i = 0; i < node.size(); i++) { vst.push_back(node[i].as< TString >()); }
            return true;
        }
    };

    template <> struct convert< RooRealVar > {
        static Node encode(const RooRealVar & var) {
            Node    node;
            TString name  = var.GetName();
            double  val   = var.getVal();
            double  error = var.getError();
            double  min   = var.hasMin() ? var.getMin() : 0;
            double  max   = var.hasMax() ? var.getMax() : 0;
            node.push_back(name);
            node.push_back(val);
            node.push_back(error);
            node.push_back(min);
            node.push_back(max);
            return node;
        }

        static bool decode(const Node & node, RooRealVar & var) {
            if (!node.IsSequence() || (node.size() != 5)) return false;
            TString name  = node[0].as< TString >();
            double  val   = node[1].as< double >();
            double  error = node[2].as< double >();
            double  min   = node[3].as< double >();
            double  max   = node[4].as< double >();

            var.SetName(name);
            var.SetTitle(name);
            var.setVal(val);
            var.setError(error);

            if ((min != 0) && (max != 0)) var.setRange(min, max);
            return true;
        }
    };

};   // namespace YAML

#endif
