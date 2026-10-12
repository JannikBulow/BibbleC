// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/type/string_type.h"

namespace bibblec {
    StringType::StringType()
        : Type("string") {}

    int StringType::getSize() const {
        return 8;
    }

    bibblir::Type* StringType::getBibblirType() const {
        return bibblir::Type::GetStringType();
    }

    Type::CastLevel StringType::castTo(Type* destType) const {
        return CastLevel::Disallowed;
    }

    std::string StringType::getSymbolID(Type* thisType) const {
        return "t";
    }

    bool StringType::isStringType() const {
        return true;
    }
}
