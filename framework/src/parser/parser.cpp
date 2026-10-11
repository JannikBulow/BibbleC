// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/parser/parser.h"

#include "BibbleC/type/array_type.h"
#include "BibbleC/type/class_type.h"

namespace bibblec::parser {
    static std::string LastSegment(std::string_view dottedName) {
        size_t dot = dottedName.rfind('.');
        return std::string(dottedName.substr(dot == std::string_view::npos ? 0 : dot + 1));
    }

    Parser::Parser(std::vector<lexer::Token>& tokens, diagnostic::Diagnostics& diag, scope::Scope* globalScope, std::string importedModuleName)
        : mTokens(tokens)
        , mPosition(0)
        , mDiag(diag)
        , mImportedModuleName(std::move(importedModuleName))
        , mActiveScope(globalScope) {
        collectModuleAliases();
    }

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

    void Parser::collectModuleAliases() {
        std::string ownModule(mActiveScope->getModuleName());
        mModuleAliases.emplace(LastSegment(ownModule), std::move(ownModule));

        for (size_t i = 0; i < mTokens.size(); ++i) {
            if (mTokens[i].getTokenType() != lexer::TokenType::ImportKeyword) continue;

            std::string module;
            std::string alias;
            size_t end = i + 1;
            while (end < mTokens.size() && mTokens[end].getTokenType() == lexer::TokenType::Identifier) {
                alias = std::string(mTokens[end].getText());
                if (!module.empty()) module += '.';
                module += alias;
                end++;
                if (end < mTokens.size() && mTokens[end].getTokenType() == lexer::TokenType::Dot) end++;
                else break;
            }
            if (module.empty()) continue;

            auto [it, inserted] = mModuleAliases.emplace(alias, module);
            if (!inserted && it->second != module) {
                mDiag.reportCompilerError(mTokens[i].getStartLocation(), mTokens[end - 1].getEndLocation(),
                    std::format("module name '{}{}{}' is ambiguous: it could mean '{}{}{}' or '{}{}{}'",
                        fmt::bold, alias, fmt::reset,
                        fmt::bold, it->second, fmt::reset,
                        fmt::bold, module, fmt::reset));
                std::exit(1);
            }
        }
    }

    std::string Parser::resolveModuleAlias(const lexer::Token& aliasToken) {
        auto it = mModuleAliases.find(std::string(aliasToken.getText()));
        if (it == mModuleAliases.end()) {
            mDiag.reportCompilerError(aliasToken.getStartLocation(), aliasToken.getEndLocation(),
                std::format("unknown module '{}{}{}' (did you remember to import?)",
                    fmt::bold, aliasToken.getText(), aliasToken.getName()));
            std::exit(1);
        }
        return it->second;
    }

    bool Parser::isQualifiedTypeAhead() const {
        auto typeAt = [this](size_t p) {
            return p < mTokens.size() ? mTokens[p].getTokenType() : lexer::TokenType::EndOfFile;
        };

        size_t position = mPosition + 2;
        if (typeAt(position) != lexer::TokenType::Identifier) return false;
        position++;
        while (typeAt(position) == lexer::TokenType::LeftBracket && typeAt(position + 1) == lexer::TokenType::RightBracket) {
            position += 2;
        }
        return typeAt(position) == lexer::TokenType::Identifier;
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
            case lexer::TokenType::LeftBracket:
            case lexer::TokenType::Dot:
                return 100;

            case lexer::TokenType::Star:
            case lexer::TokenType::Slash:
            case lexer::TokenType::Percent:
                return 75;
            case lexer::TokenType::Plus:
            case lexer::TokenType::Minus:
                return 70;

            case lexer::TokenType::LessThan:
            case lexer::TokenType::GreaterThan:
            case lexer::TokenType::LessEqual:
            case lexer::TokenType::GreaterEqual:
                return 55;

            case lexer::TokenType::DoubleEqual:
            case lexer::TokenType::BangEqual:
                return 50;

            case lexer::TokenType::Ampersand:
                return 45;
            case lexer::TokenType::Pipe:
                return 40;
            case lexer::TokenType::Caret:
                return 35;

            case lexer::TokenType::DoubleAmpersand:
                return 30;
            case lexer::TokenType::DoublePipe:
                return 25;

            case lexer::TokenType::Equal:
            case lexer::TokenType::PlusEqual:
            case lexer::TokenType::MinusEqual:
            case lexer::TokenType::StarEqual:
            case lexer::TokenType::SlashEqual:
            case lexer::TokenType::PercentEqual:
            case lexer::TokenType::AmpersandEqual:
            case lexer::TokenType::PipeEqual:
            case lexer::TokenType::CaretEqual:
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

    Type* Parser::parseType(bool parseArray) {
        auto recoverPosition = mPosition;

        Type* type;
        if (current().getTokenType() == lexer::TokenType::Identifier) {
            std::string moduleName;
            std::string name;

            bool parsePending = false;

            if (peek(1).getTokenType() == lexer::TokenType::DoubleColon) {
                moduleName = resolveModuleAlias(consume());
                consume();
                expectToken(lexer::TokenType::Identifier);
                name = consume().getText();
                parsePending = true;
            } else {
                name = consume().getText();
                moduleName = mActiveScope->getModuleName();
            }

            if (Type* classType = Type::Get(name)) {
                type = classType;
            } else if (ClassType* classType = ClassType::Get(moduleName, name)) {
                type = classType;
            } else if (parsePending) {
                type = ClassType::Create(std::move(moduleName), std::move(name));
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

        if (parseArray) {
            while (current().getTokenType() == lexer::TokenType::LeftBracket) {
                consume();
                if (current().getTokenType() == lexer::TokenType::RightBracket) {
                    consume();
                    type = ArrayType::Get(type);
                } else {
                    type = nullptr;
                }
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
                return parseGlobalVariable(sourceStart, type, false);
            }
        }

        switch (current().getTokenType()) {
            case lexer::TokenType::ClassKeyword:
                return parseClassDeclaration();

            case lexer::TokenType::ConstKeyword:
                consume();
                return parseGlobalVariable(sourceStart, parseType(), true);

            case lexer::TokenType::ImportKeyword:
                return parseImportStatement();

            case lexer::TokenType::ModuleKeyword:
                consume();
                while (current().getTokenType() != lexer::TokenType::Semicolon)
                {
                    expectToken(lexer::TokenType::Identifier);
                    consume();

                    if (current().getTokenType() != lexer::TokenType::Semicolon)
                    {
                        expectToken(lexer::TokenType::Dot);
                        consume();
                    }
                }
                consume();
                return nullptr;

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
            } else if (operatorToken.getTokenType() == lexer::TokenType::LeftBracket) {
                left = parseIndexExpression(std::move(left), source, operatorToken);
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

        bool qualified = current().getTokenType() == lexer::TokenType::Identifier && peek(1).getTokenType() == lexer::TokenType::DoubleColon;

        if (!qualified || isQualifiedTypeAhead()) {
            if (Type* type = parseType()) {
                return parseVariableDeclaration(sourceStart, type);
            }
        }

        switch (current().getTokenType()) {
            case lexer::TokenType::BreakKeyword:
                return parseBreakStatement();

            case lexer::TokenType::LeftBrace:
                return parseCompoundStatement();

            case lexer::TokenType::ContinueKeyword:
                return parseContinueStatement();

            case lexer::TokenType::ForKeyword:
                return parseForStatement();

            case lexer::TokenType::IfKeyword:
                return parseIfStatement();

            case lexer::TokenType::ReturnKeyword:
                return parseReturnStatement();

            case lexer::TokenType::WhileKeyword:
                return parseWhileStatement();


            case lexer::TokenType::IntegerLiteral:
                return parseIntegerLiteral();

            case lexer::TokenType::CharacterLiteral:
                return parseCharacterLiteral();

            case lexer::TokenType::TrueKeyword:
            case lexer::TokenType::FalseKeyword:
                return parseBooleanLiteral();

            case lexer::TokenType::Identifier:
                if (qualified) return parseQualifiedVariableExpression();
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

            Type* type;
            std::string name;
            ClassMethod::Kind methodKind;
            ClassMethod::Dispatch methodDispatch;

            if (current().getTokenType() == lexer::TokenType::Identifier && current().getText() == className) { // constructor
                type = Type::Get("void");
                name = ".init";
                methodKind = ClassMethod::Constructor;
                methodDispatch = ClassMethod::NonVirtual;
                consume();
            } else if (current().getTokenType() == lexer::TokenType::Tilde) { // finalizer
                consume();
                expectToken(lexer::TokenType::Identifier);
                if (current().getText() != className) {
                    mDiag.reportCompilerError(current().getStartLocation(), current().getEndLocation(), "invalid finalizer declaration");
                    std::exit(1);
                }
                consume();

                type = Type::Get("void");
                name = ".finalize";
                methodKind = ClassMethod::Finalizer;
                methodDispatch = ClassMethod::NonVirtual;
            } else {
                type = parseType();
                expectToken(lexer::TokenType::Identifier);
                name = consume().getText();
                methodKind = ClassMethod::Normal;
                methodDispatch = ClassMethod::Virtual; // TODO: figure out the virtual situation
            }

            if (current().getTokenType() != lexer::TokenType::LeftParen) {
                expectToken(lexer::TokenType::Semicolon);
                consume();

                fields.emplace_back(type, std::move(name));
                continue;
            }

            // method parsing from this point on

            SourcePair memberSource;
            memberSource.start = memberStart;

            consume(); // (    already guaranteed to be a left paren because of the field block above

            std::vector<FunctionArgument> arguments;
            std::vector<Type*> argumentTypes;
            while (current().getTokenType() != lexer::TokenType::RightParen) {
                Type* argumentType = parseType();

                expectToken(lexer::TokenType::Identifier);
                std::string argumentName(consume().getText());

                arguments.emplace_back(argumentType, std::move(argumentName));
                argumentTypes.push_back(argumentType);

                if (current().getTokenType() != lexer::TokenType::RightParen) {
                    expectToken(lexer::TokenType::Comma);
                    consume();
                }
            }
            memberSource.end = consume().getEndLocation();

            FunctionType* functionType = FunctionType::Create(type, std::move(argumentTypes));
            std::vector<ASTNodePtr> body;

            // if current is =, create return

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

            if (!mImportedModuleName.empty()) body.clear();

            FunctionPtr impl = std::make_unique<Function>(
                std::vector<lexer::Token>(),
                classType,
                std::move(name),
                mImportedModuleName,
                functionType,
                std::move(arguments),
                std::move(scope),
                std::move(body),
                memberSource,
                blockEnd
            );

            methods.emplace_back(std::move(impl), methodKind, methodDispatch);
        }
        source.end = consume().getEndLocation();

        return std::make_unique<ClassDeclaration>(mActiveScope, std::move(className), mImportedModuleName, std::move(fields), std::move(methods), source);
    }

    FunctionPtr Parser::parseFunction(lexer::SourceLocation sourceStart, Type* returnType, Type* implType) {
        SourcePair source;
        source.start = sourceStart;

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
        source.end = consume().getEndLocation();

        FunctionType* functionType = FunctionType::Create(returnType, std::move(argumentTypes));
        std::vector<ASTNodePtr> body;

        scope::ScopePtr scope = std::make_unique<scope::Scope>(std::nullopt, mActiveScope);
        mActiveScope = scope.get();

        SourcePair blockEnd;

        // if current is equal, create return with parseExpression

        if (current().getTokenType() == lexer::TokenType::Semicolon) {
            blockEnd = {current().getStartLocation(), current().getEndLocation()};
            consume();
        } else {
            expectToken(lexer::TokenType::LeftBrace);
            consume();

            while (current().getTokenType() != lexer::TokenType::RightBrace) {
                body.push_back(parseExpression());
                expectToken(lexer::TokenType::Semicolon);
                consume();
            }

            blockEnd = {current().getStartLocation(), current().getEndLocation()};
            consume();
        }

        mActiveScope = scope->getParent();

        if (!mImportedModuleName.empty()) body.clear();

        return std::make_unique<Function>(
            std::vector<lexer::Token>(),
            implType,
            std::move(name),
            mImportedModuleName,
            functionType,
            std::move(arguments),
            std::move(scope),
            std::move(body),
            source,
            blockEnd
        );
    }

    GlobalVariablePtr Parser::parseGlobalVariable(lexer::SourceLocation sourceStart, Type* type, bool constant) {
        SourcePair source;
        source.start = sourceStart;

        expectToken(lexer::TokenType::Identifier);
        std::string name(consume().getText());

        ASTNodePtr initialValue = nullptr;
        if (current().getTokenType() == lexer::TokenType::Equal) {
            consume();
            initialValue = parseExpression();
        }

        expectToken(lexer::TokenType::Semicolon);
        consume();

        source.end = peek(-1).getEndLocation();

        return std::make_unique<GlobalVariable>(mActiveScope, std::move(name), mImportedModuleName, type, std::move(initialValue), constant, source);
    }

    ImportStatementPtr Parser::parseImportStatement() {
        SourcePair source;
        source.start = consume().getStartLocation();

        std::vector<std::string> module;
        while (current().getTokenType() != lexer::TokenType::Semicolon) {
            expectToken(lexer::TokenType::Identifier);
            module.emplace_back(consume().getText());

            if (current().getTokenType() != lexer::TokenType::Semicolon) {
                expectToken(lexer::TokenType::Dot);
                consume();
            }
        }
        source.end = consume().getEndLocation();

        return std::make_unique<ImportStatement>(mActiveScope, std::move(module), source);
    }

    BreakStatementPtr Parser::parseBreakStatement() {
        SourcePair source(current().getStartLocation(), current().getEndLocation());
        consume();

        std::string label;
        if (current().getTokenType() == lexer::TokenType::Identifier) {
            label = consume().getText();
            source.end = peek(-1).getEndLocation();
        }

        return std::make_unique<BreakStatement>(std::move(label), mActiveScope, source);
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

    ContinueStatementPtr Parser::parseContinueStatement() {
        SourcePair source(current().getStartLocation(), current().getEndLocation());
        consume();

        std::string label;
        if (current().getTokenType() == lexer::TokenType::Identifier) {
            label = consume().getText();
            source.end = peek(-1).getEndLocation();
        }

        return std::make_unique<ContinueStatement>(std::move(label), mActiveScope, source);
    }

    ForStatementPtr Parser::parseForStatement() {
        SourcePair source;
        source.start = consume().getStartLocation();

        expectToken(lexer::TokenType::LeftParen);
        consume();

        auto init = parseExpression();
        expectToken(lexer::TokenType::Semicolon);
        consume();

        auto condition = parseExpression();
        expectToken(lexer::TokenType::Semicolon);
        consume();

        auto it = parseExpression();

        expectToken(lexer::TokenType::RightParen);
        source.end = consume().getEndLocation();

        auto body = parseExpression();

        return std::make_unique<ForStatement>(std::move(init), std::move(condition), std::move(it), std::move(body), mActiveScope, source);
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

    WhileStatementPtr Parser::parseWhileStatement() {
        SourcePair source;
        source.start = consume().getStartLocation();

        expectToken(lexer::TokenType::LeftParen);
        consume();

        auto condition = parseExpression();

        expectToken(lexer::TokenType::RightParen);
        consume();

        source.end = peek(-1).getEndLocation();

        auto body = parseExpression();

        return std::make_unique<WhileStatement>(std::move(condition), std::move(body), mActiveScope, source);
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

    VariableExpressionPtr Parser::parseQualifiedVariableExpression() {
        SourcePair source;
        source.start = current().getStartLocation();
        std::string module = resolveModuleAlias(consume());
        consume();
        expectToken(lexer::TokenType::Identifier);
        std::string name(consume().getText());
        source.end = peek(-1).getEndLocation();
        return std::make_unique<VariableExpression>(mActiveScope, std::move(name), source, std::move(module));
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

    BinaryExpressionPtr Parser::parseIndexExpression(ASTNodePtr left, SourcePair source, lexer::Token operatorToken) {
        ASTNodePtr index = parseExpression();
        expectToken(lexer::TokenType::RightBracket);
        source.end = consume().getEndLocation();
        return std::make_unique<BinaryExpression>(mActiveScope, std::move(left), std::move(operatorToken), std::move(index), source);
    }

    NewExpressionPtr Parser::parseNewExpression() {
        SourcePair source;
        source.start = consume().getStartLocation();

        Type* allocatedType = parseType(false);

        std::vector<ASTNodePtr> parameters;

        if (current().getTokenType() == lexer::TokenType::LeftBracket) {
            allocatedType = ArrayType::Get(allocatedType);

            consume();
            parameters.push_back(parseExpression());
            expectToken(lexer::TokenType::RightBracket);
            source.end = consume().getEndLocation();
        } else {
            expectToken(lexer::TokenType::LeftParen);
            consume();

            while (current().getTokenType() != lexer::TokenType::RightParen) {
                parameters.push_back(parseExpression());

                if (current().getTokenType() != lexer::TokenType::RightParen) {
                    expectToken(lexer::TokenType::Comma);
                    consume();
                }
            }
            source.end = consume().getEndLocation();
        }

        return std::make_unique<NewExpression>(mActiveScope, allocatedType, std::move(parameters), source);
    }
}
