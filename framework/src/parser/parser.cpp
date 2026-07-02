// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/parser/parser.h"

#include "../../../cmake-build-debug/_deps/vlex-src/include/diagnostic/Diagnostic.h"

namespace bibblec::parser {
    Parser::Parser(std::vector<lexer::Token>& tokens, diagnostic::Diagnostics& diag, scope::Scope* globalScope)
        : mTokens(tokens)
        , mPosition(0)
        , mDiag(diag)
        , mActiveScope(globalScope) {}

    std::vector<ASTNodePtr> Parser::parse() {
        std::vector<ASTNodePtr> ast;

        while (mPosition < mTokens.size()) {
            auto node = parseGlobal();
            if (node) {
                ast.push_back(std::move(node));
            }
        }

        return ast;
    }

    lexer::Token Parser::current() const {
        return mTokens[mPosition];
    }

    lexer::Token Parser::consume() {
        return mTokens[mPosition++];
    }

    lexer::Token Parser::peek(int offset) const {
        return mTokens[mPosition + offset];
    }

    void Parser::expectToken(lexer::TokenType type) {
        if (current().getTokenType() != type) {
            lexer::Token temp("", type, lexer::SourceLocation(), lexer::SourceLocation());
            mDiag.reportCompilerError(
                current().getStartLocation(),
                current().getEndLocation(),
                std::format("Expected '{}{}{}', got '{}{}{}'",
                    fmt::bold, temp.getName(), fmt::reset,
                    fmt::bold, current().getText(), fmt::reset)
            );
            std::exit(1);
        }
    }

    int Parser::getBinaryOperatorPrecedence(lexer::TokenType tokenType) {
        switch (tokenType) {
            case lexer::TokenType::LeftParen:
                return 100;

            case lexer::TokenType::Star:
            case lexer::TokenType::Slash:
            case lexer::TokenType::Percent:
                return 75;
            case lexer::TokenType::Plus:
            case lexer::TokenType::Minus:
                return 70;

            default:
                return 0;
        }
    }

    int Parser::getUnaryOperatorPrecedence(lexer::TokenType tokenType) {
        switch (tokenType) {
            case lexer::TokenType::Minus:
                return 100;
            default:
                return 0;
        }
    }

    Type* Parser::parseType() {
        expectToken(lexer::TokenType::Type);
        return Type::Get(consume().getText());
    }

    ASTNodePtr Parser::parseGlobal() {}

    ASTNodePtr Parser::parseExpression(int precedence) {
        SourcePair source;
        ASTNodePtr left;

        source.start = current().getStartLocation();

        int unaryOperatorPrecedence = getBinaryOperatorPrecedence(current().getTokenType());
        if (unaryOperatorPrecedence >= precedence) {
            lexer::Token operatorToken = consume();
            auto operand = parseExpression(unaryOperatorPrecedence);
            source.end = peek(-1).getEndLocation();
            //TODO: left = unary expr
        } else {
            left = parsePrimary();
        }

        while (true) {
            int binaryOperatorPrecedence = getBinaryOperatorPrecedence(current().getTokenType());
            if (binaryOperatorPrecedence < precedence) break;

            lexer::Token operatorToken = consume();
            ASTNodePtr right = parseExpression(binaryOperatorPrecedence);
            source.end = peek(-1).getEndLocation();
            left = std::make_unique<BinaryExpression>(mActiveScope, std::move(left), std::move(operatorToken), std::move(right), std::move(source));
        }

        return left;
    }

    ASTNodePtr Parser::parsePrimary() {
        switch (current().getTokenType()) {
            case lexer::TokenType::LeftParen:
                return parseParenthesizedExpression();

            default:
                mDiag.reportCompilerError(
                    current().getStartLocation(),
                    current().getEndLocation(),
                    std::format("Expected an expression. Got '{}{}{}'", fmt::bold, current().getText(), fmt::reset));
                std::exit(1);
        }
    }

    ASTNodePtr Parser::parseParenthesizedExpression() {
        SourcePair source;
        source.start = consume().getStartLocation();

        //TODO: type cast

        auto expression = parseExpression();
        expectToken(lexer::TokenType::RightParen);
        consume();

        return expression;
    }
}
