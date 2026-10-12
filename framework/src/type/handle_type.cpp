// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/type/handle_type.h"

namespace bibblec {
    HandleType::HandleType()
        : Type("handle") {}

    int HandleType::getSize() const {
        return 8;
    }

    bibblir::Type* HandleType::getBibblirType() const {
        return bibblir::Type::GetHandleType();
    }

    Type::CastLevel HandleType::castTo(Type* destType) const {
        return CastLevel::Disallowed;
    }

    std::string HandleType::getSymbolID(Type* thisType) const {
        return "h";
    }

    bool HandleType::isHandleType() const {
        return true;
    }
}
