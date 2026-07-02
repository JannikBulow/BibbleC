// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_PARSER_AST_NODE_H
#define BIBBLEC_PARSER_AST_NODE_H

#include "BibbleC/debug/source_pair.h"

#include "BibbleC/diagnostic/diagnostics.h"

#include "BibbleC/scope/scope.h"

#include "BibbleC/type/type.h"

#include "BibbleC/api.h"

#include <BibblIR/ir/builder.h>

#include <BibblIR/module.h>

#include <memory>
#include <vector>

namespace bibblec::parser {
    class BIBBLEC_EXPORT ASTNode {
    protected:
        using ASTNodePtr = std::unique_ptr<ASTNode>;

    public:
        ASTNode(scope::Scope* scope, SourcePair source, Type* type = nullptr)
            : mScope(scope)
            , mSource(std::move(source))
            , mType(type) {}

        virtual ~ASTNode() = default;

        Type* getType() const { return mType; }
        const SourcePair& getSource() const { return mSource; }

        virtual std::vector<ASTNode*> getChildren() { return {}; }

        virtual ASTNodePtr cloneExternal(scope::Scope* in) { return nullptr; }

        virtual bibblir::Value* codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) = 0;
        virtual void setEmittedValue(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) {};

        virtual void typeCheck(diagnostic::Diagnostics& diag, bool& exit) = 0;
        virtual bool triviallyImplicitCast(diagnostic::Diagnostics& diag, Type* destType) { return false; }
        bool canImplicitCast(diagnostic::Diagnostics& diag, Type* destType);

        // can't be a member because one path does return std::move(node)
        static ASTNodePtr CastTo(ASTNodePtr& node, Type* destType);

    protected:
        scope::Scope* mScope;
        SourcePair mSource;
        Type* mType;
    };

    using ASTNodePtr = std::unique_ptr<ASTNode>;
}

#endif //BIBBLEC_PARSER_AST_NODE_H
