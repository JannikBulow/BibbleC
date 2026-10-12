// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_PARSER_AST_EXPRESSION_STRING_LITERAL_H
#define BIBBLEC_PARSER_AST_EXPRESSION_STRING_LITERAL_H

#include "BibbleC/parser/ast/node.h"

#include <string>

namespace bibblec::parser {
    class BIBBLEC_EXPORT StringLiteral : public ASTNode {
    public:
        StringLiteral(scope::Scope* scope, std::string value, SourcePair source);

        std::string_view getValue() const;

        ASTNodePtr cloneExternal(scope::Scope* in) override;

        bibblir::Value* codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) override;

        void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;

    private:
        std::string mValue;
    };

    using StringLiteralPtr = std::unique_ptr<StringLiteral>;
}

#endif //BIBBLEC_PARSER_AST_EXPRESSION_STRING_LITERAL_H
