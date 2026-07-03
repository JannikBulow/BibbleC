// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/type/error_type.h"

namespace bibblec {
    ErrorType::ErrorType()
        : Type("error-type") {}

    int ErrorType::getSize() const {
        return 0;
    }

    bibblir::Type* ErrorType::getBibblirType() const {
        return bibblir::Type::GetVoidType();
    }

    Type::CastLevel ErrorType::castTo(Type* destType) const {
        return CastLevel::Disallowed;
    }

    std::string ErrorType::getSymbolID(Type* thisType) const {
        return "stray error type";
    }

    bool ErrorType::isErrorType() const {
        return true;
    }
}
