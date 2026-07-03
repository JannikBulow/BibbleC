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

    ASTNodePtr Parser::parseGlobal() {
        switch (current().getTokenType()) {
            case lexer::TokenType::Identifier:
                if (peek(1).getTokenType() == lexer::TokenType::LeftParen) {
                    return parseFunction(current().getStartLocation(), Type::Get("error-type"));
                } else {
                    mDiag.reportCompilerError(
                        current().getStartLocation(),
                        current().getEndLocation(),
                        "expected parsable function (this error message is temporary and will be removed once proper global parsing is done)"
                    );
                    std::exit(1);
                }

            case lexer::TokenType::Type: {
                lexer::SourceLocation sourceStart = current().getStartLocation();
                Type* type = parseType();
                if (peek(1).getTokenType() == lexer::TokenType::LeftParen) {
                    return parseFunction(sourceStart, type);
                } else {
                    mDiag.reportCompilerError(
                        current().getStartLocation(),
                        current().getEndLocation(),
                        "expected parsable function (this error message is temporary and will be removed once proper global parsing is done)"
                    );
                    std::exit(1);
                }
            }

            case lexer::TokenType::EndOfFile:
                consume();
                return nullptr;
        }
    }

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
            case lexer::TokenType::ReturnKeyword:
                return parseReturnStatement();


            case lexer::TokenType::IntegerLiteral:
                return parseIntegerLiteral();

            case lexer::TokenType::CharacterLiteral:
                return parseCharacterLiteral();

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

    FunctionPtr Parser::parseFunction(lexer::SourceLocation sourceStart, Type* returnType) {
        SourcePair source;
        source.start = sourceStart;
        source.end = current().getEndLocation();

        expectToken(lexer::TokenType::Identifier);
        std::string name(consume().getText());

        expectToken(lexer::TokenType::LeftParen);
        consume();

        std::vector<FunctionArgument> arguments;
        std::vector<Type*> argumentTypes;
        while (current().getTokenType() != lexer::TokenType::RightParen) {
            Type* type = parseType();

            expectToken(lexer::TokenType::Identifier);
            std::string argumentName(consume().getText());

            arguments.emplace_back(type, std::move(argumentName));
            argumentTypes.push_back(type);

            if (current().getTokenType() != lexer::TokenType::RightParen) {
                expectToken(lexer::TokenType::Comma);
                consume();
            }
        }
        consume(); // )

        FunctionType* functionType = FunctionType::Create(returnType, argumentTypes);
        std::vector<ASTNodePtr> body;

        // if current is equal, create return with parseExpression

        expectToken(lexer::TokenType::LeftBrace);
        consume();

        scope::ScopePtr scope = std::make_unique<scope::Scope>(mActiveScope);
        mActiveScope = scope.get();

        while (current().getTokenType() != lexer::TokenType::RightBrace) {
            body.push_back(parseExpression());
            expectToken(lexer::TokenType::Semicolon);
            consume();
        }

        SourcePair blockEnd(current().getStartLocation(), current().getEndLocation());
        consume();

        mActiveScope = scope->getParent();

        return std::make_unique<Function>(
            std::vector<lexer::Token>(),
            std::move(name),
            functionType,
            std::move(arguments),
            std::move(scope),
            std::move(body),
            source,
            blockEnd
        );
    }

    ReturnStatementPtr Parser::parseReturnStatement() {
        SourcePair source;
        source.start = consume().getStartLocation();

        if (current().getTokenType() == lexer::TokenType::Semicolon) {
            source.end = peek(-1).getEndLocation();
            return std::make_unique<ReturnStatement>(mActiveScope, nullptr, source);
        }

        ASTNodePtr returnValue = parseExpression();

        source.end = peek(-1).getEndLocation();

        return std::make_unique<ReturnStatement>(mActiveScope, std::move(returnValue), source);
    }

    IntegerLiteralPtr Parser::parseIntegerLiteral() {
        SourcePair source(current().getStartLocation(), current().getEndLocation());
        std::string text(consume().getText());
        uintmax_t value = std::stoull(text, nullptr, 0);
        return std::make_unique<IntegerLiteral>(mActiveScope, value, Type::Get("int"), source);
    }

    IntegerLiteralPtr Parser::parseCharacterLiteral() {
        SourcePair source(current().getStartLocation(), current().getEndLocation());
        std::string text(consume().getText());
        char value = text[0];
        return std::make_unique<IntegerLiteral>(mActiveScope, value, Type::Get("char"), source);
    }
}
