// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_PARSER_AST_STATEMENT_BREAK_STATEMENT_H
#define BIBBLEC_PARSER_AST_STATEMENT_BREAK_STATEMENT_H

#include "BibbleC/parser/ast/node.h"

namespace bibblec::parser {
    class BIBBLEC_EXPORT BreakStatement : public ASTNode {
    public:
        BreakStatement(std::string label, scope::Scope* scope, SourcePair source);

        bibblir::Value* codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) override;

        void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;

    private:
        std::string mLabel;
    };

    using BreakStatementPtr = std::unique_ptr<BreakStatement>;
}

#endif //BIBBLEC_PARSER_AST_STATEMENT_BREAK_STATEMENT_H
