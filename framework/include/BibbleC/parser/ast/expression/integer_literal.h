// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_PARSER_AST_EXPRESSION_INTEGER_LITERAL_H
#define BIBBLEC_PARSER_AST_EXPRESSION_INTEGER_LITERAL_H

#include "BibbleC/parser/ast/node.h"

#include <cstdint>

namespace bibblec::parser {
    class BIBBLEC_EXPORT IntegerLiteral : public ASTNode {
    public:
        IntegerLiteral(scope::Scope* scope, uintmax_t value, Type* type, SourcePair source);

        uintmax_t getValue() const;

        ASTNodePtr cloneExternal(scope::Scope* in) override;

        bibblir::Value* codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) override;

        void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;
        bool triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) override;

    private:
        uintmax_t mValue;
    };

    using IntegerLiteralPtr = std::unique_ptr<IntegerLiteral>;
}

#endif //BIBBLEC_PARSER_AST_EXPRESSION_INTEGER_LITERAL_H
