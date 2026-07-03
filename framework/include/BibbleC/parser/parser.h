// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_PARSER_PARSER_H
#define BIBBLEC_PARSER_PARSER_H

#include "BibbleC/diagnostic/diagnostics.h"

#include "BibbleC/lexer/Token.h"

#include "BibbleC/parser/ast/expression/binary_expression.h"
#include "BibbleC/parser/ast/expression/integer_literal.h"

#include "BibbleC/parser/ast/statement/return_statement.h"

#include "BibbleC/parser/ast/node.h"

#include "BibbleC/scope/scope.h"

#include "BibbleC/api.h"

#include <vector>

namespace bibblec::parser {
    class BIBBLEC_EXPORT Parser {
    public:
        Parser(std::vector<lexer::Token>& tokens, diagnostic::Diagnostics& diag, scope::Scope* globalScope);

        std::vector<ASTNodePtr> parse();

    private:
        std::vector<lexer::Token>& mTokens;
        size_t mPosition;

        diagnostic::Diagnostics& mDiag;

        scope::Scope* mActiveScope;

        lexer::Token current() const;
        lexer::Token consume();
        lexer::Token peek(int offset) const;

        void expectToken(lexer::TokenType type);

        int getBinaryOperatorPrecedence(lexer::TokenType tokenType);
        int getUnaryOperatorPrecedence(lexer::TokenType tokenType);

        Type* parseType();

        ASTNodePtr parseGlobal();
        ASTNodePtr parseExpression(int precedence = 1);
        ASTNodePtr parsePrimary();
        ASTNodePtr parseParenthesizedExpression();

        ReturnStatementPtr parseReturnStatement();

        IntegerLiteralPtr parseIntegerLiteral();
        IntegerLiteralPtr parseCharacterLiteral();
    };
}

#endif //BIBBLEC_PARSER_PARSER_H
