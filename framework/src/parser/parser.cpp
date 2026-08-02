// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/parser/parser.h"

#include "BibbleC/type/class_type.h"

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
                std::format("expected '{}{}{}', got '{}{}{}'",
                    fmt::bold, temp.getName(), fmt::reset,
                    fmt::bold, current().getText(), fmt::reset)
            );
            std::exit(1);
        }
    }

    int Parser::getBinaryOperatorPrecedence(lexer::TokenType tokenType) {
        switch (tokenType) {
            case lexer::TokenType::LeftParen:
            case lexer::TokenType::Dot:
                return 100;

            case lexer::TokenType::Star:
            case lexer::TokenType::Slash:
            case lexer::TokenType::Percent:
                return 75;
            case lexer::TokenType::Plus:
            case lexer::TokenType::Minus:
                return 70;

            case lexer::TokenType::Equal:
            case lexer::TokenType::PlusEqual:
            case lexer::TokenType::MinusEqual:
            case lexer::TokenType::StarEqual:
            case lexer::TokenType::SlashEqual:
            case lexer::TokenType::PercentEqual:
                return 20;

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
        auto recoverPosition = mPosition;

        Type* type;
        if (current().getTokenType() == lexer::TokenType::Identifier) {
            std::string name(consume().getText());

            if (Type* classType = Type::Get(name)) {
                type = classType;
            } else if (ClassType* classType = ClassType::Get(std::string(mActiveScope->getModuleName()), name)) {
                type = classType;
            } else {
                type = nullptr;
            }
        } else {
            if (current().getTokenType() == lexer::TokenType::Type) {
                type = Type::Get(consume().getText());
            } else {
                type = nullptr;
            }
        }

        if (!type) mPosition = recoverPosition;
        return type;
    }

    ASTNodePtr Parser::parseGlobal() {
        lexer::SourceLocation sourceStart = current().getStartLocation();

        if (Type* type = parseType()) {
            if (peek(1).getTokenType() == lexer::TokenType::LeftParen) {
                return parseFunction(sourceStart, type, nullptr);
            } else {
                mDiag.reportCompilerError(
                    current().getStartLocation(),
                    current().getEndLocation(),
                    "expected parsable function (this error message is temporary and will be removed once proper global parsing is done)"
                );
                std::exit(1);
            }
        }

        switch (current().getTokenType()) {
            case lexer::TokenType::ClassKeyword:
                return parseClassDeclaration();

            case lexer::TokenType::EndOfFile:
                consume();
                return nullptr;

            default:
                mDiag.reportCompilerError(
                    current().getStartLocation(),
                    current().getEndLocation(),
                    std::format("expected global expression, got '{}{}{}'", fmt::bold, current().getText(), fmt::reset)
                );
                std::exit(1);
        }
    }

    ASTNodePtr Parser::parseExpression(int precedence) {
        SourcePair source;
        ASTNodePtr left;

        source.start = current().getStartLocation();

        int unaryOperatorPrecedence = getUnaryOperatorPrecedence(current().getTokenType());
        if (unaryOperatorPrecedence >= precedence) {
            lexer::Token operatorToken = consume();
            ASTNodePtr operand = parseExpression(unaryOperatorPrecedence);
            source.end = peek(-1).getEndLocation();
            left = std::make_unique<UnaryExpression>(mActiveScope, std::move(operand), std::move(operatorToken), source);
        } else {
            left = parsePrimary();
        }

        while (true) {
            int binaryOperatorPrecedence = getBinaryOperatorPrecedence(current().getTokenType());
            if (binaryOperatorPrecedence < precedence) break;

            lexer::Token operatorToken = consume();

            if (operatorToken.getTokenType() == lexer::TokenType::LeftParen) {
                left = parseCallExpression(std::move(left));
            } else if (operatorToken.getTokenType() == lexer::TokenType::Dot) {
                expectToken(lexer::TokenType::Identifier);
                std::string id(consume().getText());
                source.end = peek(-1).getEndLocation();
                left = std::make_unique<MemberAccess>(mActiveScope, std::move(left), std::move(id), source);
            } else {
                ASTNodePtr right = parseExpression(binaryOperatorPrecedence);
                source.end = peek(-1).getEndLocation();
                left = std::make_unique<BinaryExpression>(mActiveScope, std::move(left), std::move(operatorToken), std::move(right), source);
            }
        }

        return left;
    }

    ASTNodePtr Parser::parsePrimary() {
        lexer::SourceLocation sourceStart = current().getStartLocation();

        if (Type* type = parseType()) {
            return parseVariableDeclaration(sourceStart, type);
        }

        switch (current().getTokenType()) {
            case lexer::TokenType::LeftBrace:
                return parseCompoundStatement();

            case lexer::TokenType::IfKeyword:
                return parseIfStatement();

            case lexer::TokenType::ReturnKeyword:
                return parseReturnStatement();


            case lexer::TokenType::IntegerLiteral:
                return parseIntegerLiteral();

            case lexer::TokenType::CharacterLiteral:
                return parseCharacterLiteral();

            case lexer::TokenType::TrueKeyword:
            case lexer::TokenType::FalseKeyword:
                return parseBooleanLiteral();

            case lexer::TokenType::Identifier:
                return parseVariableExpression();

            case lexer::TokenType::LeftParen:
                return parseParenthesizedExpression();

            case lexer::TokenType::NewKeyword:
                return parseNewExpression();

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

        if (current().getTokenType() == lexer::TokenType::Type) {
            Type* destType = parseType();

            expectToken(lexer::TokenType::RightParen);
            consume();

            ASTNodePtr expression = parseExpression(85);

            source.end = peek(-1).getEndLocation();
            return std::make_unique<CastExpression>(mActiveScope, std::move(expression), destType, source);
        }

        auto expression = parseExpression();
        expectToken(lexer::TokenType::RightParen);
        consume();

        return expression;
    }

    ClassDeclarationPtr Parser::parseClassDeclaration() {
        SourcePair source;
        source.start = consume().getStartLocation();

        expectToken(lexer::TokenType::Identifier);
        std::string className(consume().getText());

        ClassType* classType = ClassType::Create(std::string(mActiveScope->getModuleName()), className);

        expectToken(lexer::TokenType::LeftBrace);
        consume();

        std::vector<ClassField> fields;
        std::vector<ClassMethod> methods;
        while (current().getTokenType() != lexer::TokenType::RightBrace) {
            lexer::SourceLocation memberStart = current().getStartLocation();

            expectToken(lexer::TokenType::Type);
            Type* type = parseType();

            expectToken(lexer::TokenType::Identifier);
            if (peek(1).getTokenType() == lexer::TokenType::LeftParen) {
                FunctionPtr function = parseFunction(memberStart, type, classType);
                methods.emplace_back(std::move(function));
                continue;
            }

            std::string name(consume().getText());

            expectToken(lexer::TokenType::Semicolon);
            consume();

            fields.emplace_back(type, std::move(name));
        }
        source.end = consume().getEndLocation();

        return std::make_unique<ClassDeclaration>(mActiveScope, std::move(className), std::move(fields), std::move(methods), source);
    }

    FunctionPtr Parser::parseFunction(lexer::SourceLocation sourceStart, Type* returnType, Type* implType) {
        SourcePair source;
        source.start = sourceStart;
        source.end = current().getEndLocation();

        expectToken(lexer::TokenType::Identifier);
        std::string name(consume().getText());

        if (implType && name == "init") name = ".init";

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

        scope::ScopePtr scope = std::make_unique<scope::Scope>(std::nullopt, mActiveScope);
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
            implType,
            std::move(name),
            functionType,
            std::move(arguments),
            std::move(scope),
            std::move(body),
            source,
            blockEnd
        );
    }

    CompoundStatementPtr Parser::parseCompoundStatement() {
        SourcePair source;
        source.start = consume().getStartLocation();

        scope::ScopePtr scope = std::make_unique<scope::Scope>(std::nullopt, mActiveScope);
        mActiveScope = scope.get();

        std::vector<ASTNodePtr> body;
        while (current().getTokenType() != lexer::TokenType::RightBrace) {
            body.push_back(parseExpression());
            expectToken(lexer::TokenType::Semicolon);
            consume();
        }
        source.end = consume().getEndLocation();

        mTokens.insert(mTokens.begin() + mPosition, lexer::Token(";", lexer::TokenType::Semicolon, source.end, source.end));

        mActiveScope = scope->getParent();

        return std::make_unique<CompoundStatement>(std::move(body), std::move(scope), source);
    }

    IfStatementPtr Parser::parseIfStatement() {
        SourcePair source;
        source.start = consume().getStartLocation();

        expectToken(lexer::TokenType::LeftParen);
        consume();

        auto condition = parseExpression();

        expectToken(lexer::TokenType::RightParen);
        consume();

        source.end = peek(-1).getEndLocation();

        auto body = parseExpression();

        ASTNodePtr elseBody = nullptr;
        if (peek(1).getTokenType() == lexer::TokenType::ElseKeyword) {
            expectToken(lexer::TokenType::Semicolon);
            consume();

            consume(); // else
            elseBody = parseExpression();
        }

        return std::make_unique<IfStatement>(std::move(condition), std::move(body), std::move(elseBody), mActiveScope, source);
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

    VariableDeclarationPtr Parser::parseVariableDeclaration(lexer::SourceLocation sourceStart, Type* type) {
        SourcePair source;
        source.start = sourceStart;

        expectToken(lexer::TokenType::Identifier);
        std::string name(consume().getText());

        ASTNodePtr initialValue = nullptr;
        if (current().getTokenType() == lexer::TokenType::Equal) {
            consume();
            initialValue = parseExpression();
        }

        source.end = peek(-1).getEndLocation();

        return std::make_unique<VariableDeclaration>(mActiveScope, std::move(name), type, std::move(initialValue), source);
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

    BooleanLiteralPtr Parser::parseBooleanLiteral() {
        SourcePair source(current().getStartLocation(), current().getEndLocation());
        bool value = consume().getTokenType() == lexer::TokenType::TrueKeyword;
        return std::make_unique<BooleanLiteral>(mActiveScope, value, source);
    }

    VariableExpressionPtr Parser::parseVariableExpression() {
        SourcePair source(current().getStartLocation(), current().getEndLocation());
        std::string text(consume().getText());
        return std::make_unique<VariableExpression>(mActiveScope, std::move(text), source);
    }

    CallExpressionPtr Parser::parseCallExpression(ASTNodePtr callee) {
        SourcePair source;
        source.start = callee->getSource().start;

        std::vector<ASTNodePtr> parameters;
        while (current().getTokenType() != lexer::TokenType::RightParen) {
            parameters.push_back(parseExpression());

            if (current().getTokenType() != lexer::TokenType::RightParen) {
                expectToken(lexer::TokenType::Comma);
                consume();
            }
        }
        source.end = consume().getEndLocation();

        return std::make_unique<CallExpression>(mActiveScope, std::move(callee), std::move(parameters), source);
    }

    NewExpressionPtr Parser::parseNewExpression() {
        SourcePair source;
        source.start = consume().getStartLocation();

        Type* allocatedType = parseType();

        expectToken(lexer::TokenType::LeftParen);
        consume();

        std::vector<ASTNodePtr> parameters;
        while (current().getTokenType() != lexer::TokenType::RightParen) {
            parameters.push_back(parseExpression());

            if (current().getTokenType() != lexer::TokenType::RightParen) {
                expectToken(lexer::TokenType::Comma);
                consume();
            }
        }
        source.end = consume().getEndLocation();

        return std::make_unique<NewExpression>(mActiveScope, allocatedType, std::move(parameters), source);
    }
}
