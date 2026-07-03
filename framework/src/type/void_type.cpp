// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/type/void_type.h"

namespace bibblec {
    VoidType::VoidType()
        : Type("void") {}

    int VoidType::getSize() const {
        return 0;
    }

    bibblir::Type* VoidType::getBibblirType() const {
        return bibblir::Type::GetVoidType();
    }

    Type::CastLevel VoidType::castTo(Type* destType) const {
        return CastLevel::Disallowed;
    }

    std::string VoidType::getSymbolID(Type* thisType) const {
        return "V";
    }

    bool VoidType::isVoidType() const {
        return true;
    }
}
