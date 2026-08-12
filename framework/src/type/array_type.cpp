// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/type/array_type.h"

#include <algorithm>

namespace bibblec {
    ArrayType::ArrayType(Type* elementType)
        : Type(std::string(elementType->getName()) + "[]")
        , mElementType(elementType) {}

    Type* ArrayType::getElementType() const {
        return mElementType;
    }

    int ArrayType::getSize() const {
        return 8;
    }

    bibblir::Type* ArrayType::getBibblirType() const {
        return bibblir::Type::GetArrayType(mElementType->getBibblirType());
    }

    Type::CastLevel ArrayType::castTo(Type* destType) const {
        return CastLevel::Disallowed;
    }

    std::string ArrayType::getSymbolID(Type* thisType) const {
        return "[" + mName;
    }

    bool ArrayType::isArrayType() const {
        return true;
    }

    static std::vector<std::unique_ptr<ArrayType>> arrayTypes;

    ArrayType* ArrayType::Get(Type* elementType) {
        auto it = std::ranges::find_if(arrayTypes, [elementType](auto& arrayType) {
            return arrayType->getElementType() == elementType;
        });
        if (it != arrayTypes.end()) return it->get();

        arrayTypes.push_back(std::make_unique<ArrayType>(elementType));
        return arrayTypes.back().get();
    }

    void ArrayType::Reset() {
        arrayTypes.clear();
    }
}
