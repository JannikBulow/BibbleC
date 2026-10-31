// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_PARSER_AST_GLOBAL_ATTRIBUTE_H
#define BIBBLEC_PARSER_AST_GLOBAL_ATTRIBUTE_H

#include "BibbleC/parser/ast/node.h"

namespace bibblec::parser {
    struct Attribute {
        std::string name;
        std::vector<SharedASTNodePtr> arguments;
    };
}

#endif //BIBBLEC_PARSER_AST_GLOBAL_ATTRIBUTE_H
