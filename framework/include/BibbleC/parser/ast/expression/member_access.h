// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_PARSER_AST_EXPRESSION_MEMBER_ACCESS_H
#define BIBBLEC_PARSER_AST_EXPRESSION_MEMBER_ACCESS_H

#include "BibbleC/parser/ast/node.h"

#include "BibbleC/type/class_type.h"

namespace bibblec::parser {
    class BIBBLEC_EXPORT MemberAccess : public ASTNode {
        friend class CallExpression;
    public:
        MemberAccess(scope::Scope* scope, ASTNodePtr object, std::string id, SourcePair source);

        std::vector<ASTNode*> getChildren() override;

        bibblir::Value* codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) override;

        void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;

    private:
        ASTNodePtr mObject;
        std::string mId;

        ClassType* mClassType;
    };

    using MemberAccessPtr = std::unique_ptr<MemberAccess>;
}

#endif //BIBBLEC_PARSER_AST_EXPRESSION_MEMBER_ACCESS_H
