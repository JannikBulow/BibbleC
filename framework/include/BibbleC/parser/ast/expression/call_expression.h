// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_PARSER_AST_EXPRESSION_CALL_EXPRESSION_H
#define BIBBLEC_PARSER_AST_EXPRESSION_CALL_EXPRESSION_H

#include "BibbleC/parser/ast/node.h"

namespace bibblec::parser {
    class BIBBLEC_EXPORT CallExpression : public ASTNode {
    public:
        CallExpression(scope::Scope* scope, ASTNodePtr callee, std::vector<ASTNodePtr> parameters, SourcePair source);

        ASTNode* getCallee() const;

        scope::Symbol* getBestViableFunction(diagnostic::Diagnostics& diag);

        std::vector<ASTNode*> getChildren() override;

        bibblir::Value* codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) override;

        void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;

    private:
        ASTNodePtr mCallee;
        std::vector<ASTNodePtr> mParameters;

        bool mIsMemberFunction;
        bool mIsVirtualMethod;
        scope::Symbol* mBestViableFunction;
    };

    using CallExpressionPtr = std::unique_ptr<CallExpression>;
}

#endif //BIBBLEC_PARSER_AST_EXPRESSION_CALL_EXPRESSION_H
