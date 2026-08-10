// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_PARSER_AST_STATEMENT_CONTINUE_STATEMENT_H
#define BIBBLEC_PARSER_AST_STATEMENT_CONTINUE_STATEMENT_H

#include "BibbleC/parser/ast/node.h"

namespace bibblec::parser {
    class BIBBLEC_EXPORT ContinueStatement : public ASTNode {
    public:
        ContinueStatement(std::string label, scope::Scope* scope, SourcePair source);

        bibblir::Value* codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) override;

        void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;

    private:
        std::string mLabel;
    };

    using ContinueStatementPtr = std::unique_ptr<ContinueStatement>;
}

#endif //BIBBLEC_PARSER_AST_STATEMENT_CONTINUE_STATEMENT_H
