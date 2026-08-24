// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/parser/ast/global/class_declaration.h"

#include "BibbleC/type/class_type.h"

#include <BibblIR/ir/class.h>

namespace bibblec::parser {
    ClassMethod::ClassMethod(FunctionPtr impl, Kind kind, Dispatch dispatch)
        : type(static_cast<FunctionType*>(impl->getType()))
        , name(impl->getName())
        , impl(std::move(impl))
        , kind(kind)
        , dispatch(dispatch) {
        switch (kind) {
            case Normal:
                break;
            case Constructor:
                this->dispatch = NonVirtual;
                this->impl->mName = ".init";
                this->impl->mSymbol->name = ".init";
                break;
            case Finalizer:
                this->dispatch = Virtual;
                this->impl->mName = ".finalize";
                this->impl->mSymbol->name = ".finalize";
                break;
        }
    }

    bool ClassMethod::isVirtual() const {
        return dispatch == Virtual;
    }

    ClassDeclaration::ClassDeclaration(scope::Scope* scope, std::string name, std::string externalModuleName, std::vector<ClassField> fields, std::vector<ClassMethod> methods, SourcePair source)
        : ASTNode(scope, source)
        , mName(std::move(name))
        , mExternalModuleName(std::move(externalModuleName))
        , mFields(std::move(fields))
        , mMethods(std::move(methods))
        , mSymbol(nullptr) {
        mScope->addSymbol(std::make_unique<scope::Symbol>(mName, nullptr));
        mSymbol = mScope->getLatestSymbol();

        if (Type* type = Type::Get(mName)) {
            mType = type;
        } else {
            mType = ClassType::Create(std::string(mScope->getModuleName()), mName);
        }

        // yeah this is prob fine
        ClassType* classType = static_cast<ClassType*>(mType);

        std::vector<ClassType::Member> classTypeMembers;
        classTypeMembers.reserve(mFields.size() + mMethods.size());
        for (auto& field : mFields) {
            classTypeMembers.emplace_back(field.type, field.name, false);
        }
        for (auto& method : mMethods) {
            if (method.isVirtual()) classTypeMembers.emplace_back(method.type, method.name, true, method.impl ? method.impl->mSymbol : nullptr);
        }
        classType->setFields(classTypeMembers);
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
            methods.emplace_back(FunctionPtr(static_cast<Function*>(method.impl->cloneExternal(in).release())), method.kind, method.dispatch);
        }

        return std::make_unique<ClassDeclaration>(in, mName, mExternalModuleName.empty() ? std::string(mScope->getModuleName()) : mExternalModuleName, mFields, std::move(methods), mSource);
    }

    bibblir::Value* ClassDeclaration::codegen(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) {
        bibblir::Class* clas = bibblir::Class::Create(module, mName);
        static_cast<ClassType*>(mType)->setBibblirClass(clas);

        for (auto& field : mFields) {
            clas->addField(field.type->getBibblirType(), field.name);
        }

        for (auto& method : mMethods) {
            bibblir::Value* impl = nullptr;
            if (method.impl) {
                impl = method.impl->codegen(builder, module, diag);
            }

            if (method.dispatch == ClassMethod::Virtual) {
                clas->addMethod(static_cast<bibblir::FunctionType*>(method.impl->getType()->getBibblirType()), std::string(method.impl->getName()), impl);
            }
        }

        return clas;
    }

    void ClassDeclaration::setEmittedValue(bibblir::IRBuilder& builder, bibblir::Module& module, diagnostic::Diagnostics& diag) {
        for (auto& method : mMethods) {
            if (method.impl) method.impl->setEmittedValue(builder, module, diag);
        }
    }

    void ClassDeclaration::typeCheck(diagnostic::Diagnostics& diag, bool& exit) {
        for (auto& method : mMethods) {
            method.impl->typeCheck(diag, exit);
        }
    }
}
