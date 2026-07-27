// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_PARSER_AST_GLOBAL_CLASS_DECLARATION_H
#define BIBBLEC_PARSER_AST_GLOBAL_CLASS_DECLARATION_H

#include "BibbleC/parser/ast/global/function.h"

#include "BibbleC/parser/ast/node.h"

namespace bibblec::parser {
    struct ClassField {
        Type* type;
        std::string name;

        ClassField(Type* type, std::string name) : type(type), name(std::move(name)) {}
    };

    struct ClassMethod {
        FunctionPtr impl;
        bool isVirtual = false; // will soon be used

        ClassMethod(FunctionPtr impl) : impl(std::move(impl)) {}
    };

    class BIBBLEC_EXPORT ClassDeclaration : public ASTNode {
    public:
        ClassDeclaration(scope::Scope* scope, std::string name, std::vector<ClassField> fields, std::vector<ClassMethod> methods, SourcePair source);

        std::vector<ASTNode*> getChildren() override;

        ASTNodePtr cloneExternal(scope::Scope* in) override;

        bibblir::Value* codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) override;
        void setEmittedValue(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) override;

        void typeCheck(diagnostic::Diagnostics& diag, bool& exit) override;

    private:
        std::string mName;
        std::vector<ClassField> mFields;
        std::vector<ClassMethod> mMethods;

        scope::Symbol* mSymbol;
    };

    using ClassDeclarationPtr = std::unique_ptr<ClassDeclaration>;
}

#endif //BIBBLEC_PARSER_AST_GLOBAL_CLASS_DECLARATION_H
