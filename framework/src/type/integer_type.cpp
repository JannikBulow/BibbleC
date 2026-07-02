// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/diagnostic/diagnostics.h"

#include "BibbleC/type/integer_type.h"

#include <format>

namespace bibblec {
    static std::string GetIntegerTypeName(IntegerType::Size size, bool isSigned) {
        using namespace std::string_literals;

        const char* typeName = nullptr;
        switch (size) {
            case IntegerType::Byte:
                typeName = "byte";
                break;
            case IntegerType::Short:
                typeName = "short";
                break;
            case IntegerType::Int:
                typeName = "int";
                break;
            case IntegerType::Long:
                typeName = "long";
                break;
        }

        if (typeName == nullptr) [[unlikely]] {
            return "unreachable integer type name";
        }

        return isSigned ? typeName : "u"s + typeName;
    }

    IntegerType::IntegerType(Size size, bool isSigned)
        : Type(GetIntegerTypeName(size, isSigned))
        , mSize(size)
        , mSigned(isSigned) {}

    int IntegerType::getSize() const {
        return mSize;
    }

    bibblir::Type* IntegerType::getBibblirType() const {
        return bibblir::Type::GetIntegerType(getSize());
    }

    Type::CastLevel IntegerType::castTo(Type* destType) const {
        if (destType->isIntegerType()) { //TODO: check signed/unsigned in both types here
            if (destType->getSize() < getSize()) {
                return CastLevel::ImplicitWarning;
            }
            return CastLevel::Implicit;
        } else if (destType->isBooleanType()) {
            return CastLevel::ImplicitWarning;
        }
        return CastLevel::Disallowed;
    }

    std::string IntegerType::getImplicitCastWarning(Type* destType) const {
        return std::format("potential loss of data casting '{}{}{}' to '{}{}{}'",
            fmt::bold, mName, fmt::reset,
            fmt::bold, destType->getName(), fmt::reset);
    }

    std::string IntegerType::getSymbolID(Type* thisType) const {
        if (mSigned) {
            switch (mSize) {
                case Byte: return "b";
                case Short: return "s";
                case Int: return "i";
                case Long: return "l";
            }
        } else {
            switch (mSize) {
                case Byte: return "B";
                case Short: return "S";
                case Int: return "I";
                case Long: return "L";
            }
        }
        return "";
    }

    bool IntegerType::isIntegerType() const {
        return true;
    }

    bool IntegerType::isSigned() const {
        return mSigned;
    }
}
