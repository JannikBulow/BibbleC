// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_PARSER_AST_GLOBAL_ATTRIBUTE_H
#define BIBBLEC_PARSER_AST_GLOBAL_ATTRIBUTE_H

#include "BibbleC/parser/ast/node.h"

namespace bibblec::parser {
    struct Attribute {
        std::string name;
        std::vector<SharedASTNodePtr> arguments;
    };

    constexpr Attribute* FindAttribute(std::vector<Attribute>& attributes, std::string_view name) {
        for (auto& attribute : attributes) {
            if (attribute.name == name) return &attribute;
        }
        return nullptr;
    }
}

#endif //BIBBLEC_PARSER_AST_GLOBAL_ATTRIBUTE_H
