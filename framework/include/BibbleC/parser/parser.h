// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_PARSER_PARSER_H
#define BIBBLEC_PARSER_PARSER_H

#include "BibbleC/diagnostic/diagnostics.h"

#include "BibbleC/lexer/token.h"

#include "BibbleC/parser/ast/expression/binary_expression.h"
#include "BibbleC/parser/ast/expression/boolean_literal.h"
#include "BibbleC/parser/ast/expression/call_expression.h"
#include "BibbleC/parser/ast/expression/cast_expression.h"
#include "BibbleC/parser/ast/expression/integer_literal.h"
#include "BibbleC/parser/ast/expression/member_access.h"
#include "BibbleC/parser/ast/expression/new_expression.h"
#include "BibbleC/parser/ast/expression/unary_expression.h"
#include "BibbleC/parser/ast/expression/variable_expression.h"

#include "BibbleC/parser/ast/global/class_declaration.h"
#include "BibbleC/parser/ast/global/function.h"
#include "BibbleC/parser/ast/global/global_variable.h"

#include "BibbleC/parser/ast/statement/break_statement.h"
#include "BibbleC/parser/ast/statement/compound_statement.h"
#include "BibbleC/parser/ast/statement/continue_statement.h"
#include "BibbleC/parser/ast/statement/for_statement.h"
#include "BibbleC/parser/ast/statement/if_statement.h"
#include "BibbleC/parser/ast/statement/return_statement.h"
#include "BibbleC/parser/ast/statement/variable_declaration.h"
#include "BibbleC/parser/ast/statement/while_statement.h"

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

        Type* parseType(bool parseArray = true);

        ASTNodePtr parseGlobal();
        ASTNodePtr parseExpression(int precedence = 1);
        ASTNodePtr parsePrimary();
        ASTNodePtr parseParenthesizedExpression();

        ClassDeclarationPtr parseClassDeclaration();
        FunctionPtr parseFunction(lexer::SourceLocation sourceStart, Type* returnType, Type* implType);
        GlobalVariablePtr parseGlobalVariable(lexer::SourceLocation sourceStart, Type* type, bool constant);

        BreakStatementPtr parseBreakStatement();
        CompoundStatementPtr parseCompoundStatement();
        ContinueStatementPtr parseContinueStatement();
        ForStatementPtr parseForStatement();
        IfStatementPtr parseIfStatement();
        ReturnStatementPtr parseReturnStatement();
        VariableDeclarationPtr parseVariableDeclaration(lexer::SourceLocation sourceStart, Type* type);
        WhileStatementPtr parseWhileStatement();

        IntegerLiteralPtr parseIntegerLiteral();
        IntegerLiteralPtr parseCharacterLiteral();
        BooleanLiteralPtr parseBooleanLiteral();
        VariableExpressionPtr parseVariableExpression();
        CallExpressionPtr parseCallExpression(ASTNodePtr callee);
        BinaryExpressionPtr parseIndexExpression(ASTNodePtr left, SourcePair source, lexer::Token operatorToken);
        NewExpressionPtr parseNewExpression();
    };
}

#endif //BIBBLEC_PARSER_PARSER_H
