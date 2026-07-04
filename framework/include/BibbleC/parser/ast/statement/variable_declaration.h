// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_PARSER_AST_STATEMENT_VARIABLE_DECLARATION_H
#define BIBBLEC_PARSER_AST_STATEMENT_VARIABLE_DECLARATION_H

#include "BibbleC/parser/ast/node.h"

namespace bibblec::parser {
    class VariableDeclaration : public ASTNode {
    public:
        VariableDeclaration(scope::Scope* scope, std::string name, Type* type, ASTNodePtr initialValue, SourcePair source);

        std::vector<ASTNode*> getChildren() override;

        bibblir::Value* codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) override;

        void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;

    private:
        std::string mName;
        ASTNodePtr mInitialValue;

        scope::Symbol* mSymbol;
    };

    using VariableDeclarationPtr = std::unique_ptr<VariableDeclaration>;
}

#endif //BIBBLEC_PARSER_AST_STATEMENT_VARIABLE_DECLARATION_H
