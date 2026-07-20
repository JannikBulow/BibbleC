// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_PARSER_AST_EXPRESSION_BOOLEAN_LITERAL_H
#define BIBBLEC_PARSER_AST_EXPRESSION_BOOLEAN_LITERAL_H

#include "BibbleC/parser/ast/node.h"

namespace bibblec::parser {
    class BIBBLEC_EXPORT BooleanLiteral : public ASTNode {
    public:
        BooleanLiteral(scope::Scope* scope, bool value, SourcePair source);

        bool getValue() const;

        bibblir::Value* codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) override;

        void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;
        bool triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) override;

    private:
        bool mValue;
    };

    using BooleanLiteralPtr = std::unique_ptr<BooleanLiteral>;
}

#endif //BIBBLEC_PARSER_AST_EXPRESSION_BOOLEAN_LITERAL_H
