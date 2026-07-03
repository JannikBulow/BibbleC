// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/type/function_type.h"

#include <BibblIR/type/function_type.h>

#include <format>
#include <memory>

namespace bibblec {
    FunctionType::FunctionType(Type* returnType, std::vector<Type*> argumentTypes)
        : Type(std::format("{}(", returnType->getName()))
        , mReturnType(returnType)
        , mArgumentTypes(std::move(argumentTypes)) {
        if (!mArgumentTypes.empty()) {
            for (size_t i = 0; i < mArgumentTypes.size() - 1; i++) {
                mName += std::format("{}, ", mArgumentTypes[i]->getName());
            }
            mName += mArgumentTypes.back()->getName();
        }
        mName += ')';
    }

    Type* FunctionType::getReturnType() const {
        return mReturnType;
    }

    const std::vector<Type*>& FunctionType::getArgumentTypes() const {
        return mArgumentTypes;
    }

    int FunctionType::getSize() const {
        return 0;
    }

    bibblir::Type* FunctionType::getBibblirType() const {
        std::vector<bibblir::Type*> argumentTypes;
        argumentTypes.reserve(mArgumentTypes.size());
        for (Type* argumentType : mArgumentTypes) {
            argumentTypes.push_back(argumentType->getBibblirType());
        }
        return bibblir::FunctionType::Create(mReturnType->getBibblirType(), argumentTypes);
    }

    Type::CastLevel FunctionType::castTo(Type* destType) const {
        return CastLevel::Disallowed;
    }

    std::string FunctionType::getSymbolID(Type* thisType) const {
        return "%unimplemented%";
    }

    bool FunctionType::isFunctionType() const {
        return true;
    }

    static std::vector<std::unique_ptr<FunctionType>> functionTypes;

    FunctionType* FunctionType::Create(Type* returnType, const std::vector<Type*>& argumentTypes) {
        auto it = std::ranges::find_if(functionTypes, [returnType, &argumentTypes](std::unique_ptr<FunctionType>& type) {
            return type->getReturnType() == returnType && type->getArgumentTypes() == argumentTypes;
        });

        if (it != functionTypes.end()) {
            return it->get();
        }

        functionTypes.push_back(std::make_unique<FunctionType>(returnType, argumentTypes));
        return functionTypes.back().get();
    }

    void FunctionType::Reset() {
        functionTypes.clear();
    }
}
