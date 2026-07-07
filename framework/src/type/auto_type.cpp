// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/type/auto_type.h"

namespace bibblec {
    AutoType::AutoType()
        : Type("auto") {}

    int AutoType::getSize() const {
        return 0;
    }

    bibblir::Type* AutoType::getBibblirType() const {
        return bibblir::Type::GetVoidType();
    }

    Type::CastLevel AutoType::castTo(Type* destType) const {
        return CastLevel::Disallowed;
    }

    std::string AutoType::getSymbolID(Type* thisType) const {
        return "stray unresolved auto type";
    }

    bool AutoType::isAutoType() const {
        return true;
    }
}
