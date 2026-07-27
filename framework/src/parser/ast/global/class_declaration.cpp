// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/parser/ast/global/class_declaration.h"

#include "BibbleC/type/class_type.h"

#include <BibblIR/ir/class.h>

namespace bibblec::parser {
    ClassDeclaration::ClassDeclaration(scope::Scope* scope, std::string name, std::vector<ClassField> fields, std::vector<ClassMethod> methods, SourcePair source)
        : ASTNode(scope, source)
        , mName(std::move(name))
        , mFields(std::move(fields))
        , mMethods(std::move(methods))
        , mSymbol(nullptr) {
        mScope->addSymbol(std::make_unique<scope::Symbol>(mName, nullptr));
        mSymbol = mScope->getLatestSymbol();

        std::vector<ClassType::Field> classTypeFields;
        classTypeFields.reserve(mFields.size());
        for (auto& field : mFields) {
            classTypeFields.emplace_back(field.name, field.type);
        }

        if (Type* type = Type::Get(mName)) {
            mType = type;
        } else {
            mType = ClassType::Create(mName, std::move(classTypeFields));
        }
    }

    std::vector<ASTNode*> ClassDeclaration::getChildren() {
        std::vector<ASTNode*> children;
        for (auto& method : mMethods) {
            children.push_back(method.impl.get());
        }
        return children;
    }

    ASTNodePtr ClassDeclaration::cloneExternal(scope::Scope* in) {
        std::vector<ClassMethod> methods;
        for (auto& method : mMethods) {
            methods.emplace_back(FunctionPtr(static_cast<Function*>(method.impl->cloneExternal(in).release())));
        }

        return std::make_unique<ClassDeclaration>(in, mName, mFields, std::move(methods), mSource);
    }

    bibblir::Value* ClassDeclaration::codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) {
        bibblir::Class* clas = bibblir::Class::Create(module, mName);

        for (auto& field : mFields) {
            clas->addField(field.type->getBibblirType(), field.name);
        }

        for (auto& method : mMethods) {
            clas->addMethod(static_cast<bibblir::FunctionType*>(method.impl->getType()->getBibblirType()), std::string(method.impl->getName()), method.impl->codegen(builder, module, diag));
        }

        return clas;
    }

    void ClassDeclaration::setEmittedValue(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) {
        for (auto& method : mMethods) {
            method.impl->setEmittedValue(builder, module, diag);
        }
    }

    void ClassDeclaration::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
        for (auto& method : mMethods) {
            method.impl->typeCheck(diag, exit);
        }
    }
}
