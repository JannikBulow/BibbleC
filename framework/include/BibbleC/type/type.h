// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_TYPE_TYPE_H
#define BIBBLEC_TYPE_TYPE_H

#include "BibbleC/api.h"

#include <BibblIR/type/type.h>

#include <string>

namespace bibblec {
    class BIBBLEC_EXPORT Type {
    public:
        enum class CastLevel {
            Implicit,
            ImplicitWarning,
            Explicit,
            Disallowed
        };

        explicit Type(std::string name) : mName(std::move(name)) {}
        virtual ~Type() = default;

        std::string_view getName() const { return mName; }
        virtual int getSize() const = 0;

        virtual bibblir::Type* getBibblirType() const = 0;

        virtual CastLevel castTo(Type* destType) const = 0;
        virtual std::string getImplicitCastWarning(Type* destType) const { return ""; }
        virtual std::string getSymbolID(Type* thisType) const = 0;

        virtual bool isVoidType() const { return false; }
        virtual bool isIntegerType() const { return false; }
        virtual bool isBooleanType() const { return false; }
        virtual bool isFunctionType() const { return false; }

        static void Init();
        static bool Exists(std::string_view name);
        static Type* Get(std::string_view name);
        static void Reset();

    protected:
        std::string mName;
    };
}



#endif //BIBBLEC_TYPE_TYPE_H
