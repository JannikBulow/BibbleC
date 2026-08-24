// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_PARSER_AST_GLOBAL_IMPORT_STATEMENT_H
#define BIBBLEC_PARSER_AST_GLOBAL_IMPORT_STATEMENT_H

#include "BibbleC/parser/ast/node.h"

namespace bibblec::parser {
    class BIBBLEC_EXPORT ImportStatement : public ASTNode {
    public:
        ImportStatement(scope::Scope* scope, std::vector<std::string> module, SourcePair source);

        const std::vector<std::string>& getModule() const;

        bibblir::Value* codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) override;

        void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;

    private:
        std::vector<std::string> mModule;
    };

    using ImportStatementPtr = std::unique_ptr<ImportStatement>;
}

#endif //BIBBLEC_PARSER_AST_GLOBAL_IMPORT_STATEMENT_H
