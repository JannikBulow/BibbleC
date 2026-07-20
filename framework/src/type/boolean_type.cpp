// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/type/boolean_type.h"

namespace bibblec {
    BooleanType::BooleanType()
        : Type("bool") {}

    int BooleanType::getSize() const {
        return 1;
    }

    bibblir::Type* BooleanType::getBibblirType() const {
        return bibblir::Type::GetBooleanType();
    }

    Type::CastLevel BooleanType::castTo(Type* destType) const {
        if (destType->isIntegerType()) {
            return CastLevel::Implicit;
        }
        return CastLevel::Disallowed;
    }

    std::string BooleanType::getSymbolID(Type* thisType) const {
        return "z";
    }

    bool BooleanType::isBooleanType() const {
        return true;
    }
}
