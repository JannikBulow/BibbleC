// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_PARSER_AST_GLOBAL_GLOBAL_VARIABLE_H
#define BIBBLEC_PARSER_AST_GLOBAL_GLOBAL_VARIABLE_H

#include "BibbleC/parser/ast/node.h"

namespace bibblec::parser {
    class BIBBLEC_EXPORT GlobalVariable : public ASTNode {
    public:
        GlobalVariable(scope::Scope* scope, std::string name, Type* type, ASTNodePtr initialValue, bool constant, SourcePair source);

        std::vector<ASTNode*> getChildren() override;

        ASTNodePtr cloneExternal(scope::Scope* in) override;

        bibblir::Value* codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) override;

        void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;

    private:
        std::string mName;
        ASTNodePtr mInitialValue;

        scope::Symbol* mSymbol;
    };

    using GlobalVariablePtr = std::unique_ptr<GlobalVariable>;
}

#endif //BIBBLEC_PARSER_AST_GLOBAL_GLOBAL_VARIABLE_H
