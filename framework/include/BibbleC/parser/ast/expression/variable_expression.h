// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_PARSER_AST_EXPRESSION_VARIABLE_EXPRESSION_H
#define BIBBLEC_PARSER_AST_EXPRESSION_VARIABLE_EXPRESSION_H

#include "BibbleC/parser/ast/node.h"

namespace bibblec::parser {
    class VariableExpression : public ASTNode {
    public:
        VariableExpression(scope::Scope* scope, std::string name, SourcePair source);

        ASTNodePtr cloneExternal(scope::Scope* in) override;

        bibblir::Value* codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) override;

        void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;

    private:
        std::string mName;
    };

    using VariableExpressionPtr = std::unique_ptr<VariableExpression>;
}

#endif //BIBBLEC_PARSER_AST_EXPRESSION_VARIABLE_EXPRESSION_H
