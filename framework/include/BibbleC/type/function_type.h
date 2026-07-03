// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_TYPE_FUNCTION_TYPE_H
#define BIBBLEC_TYPE_FUNCTION_TYPE_H

#include "BibbleC/type/type.h"

#include <vector>

namespace bibblec {
    class BIBBLEC_EXPORT FunctionType : public Type {
    public:
        FunctionType(Type* returnType, std::vector<Type*> argumentTypes);

        Type* getReturnType() const;
        const std::vector<Type*>& getArgumentTypes() const;

        int getSize() const override;

        bibblir::Type* getBibblirType() const override;

        CastLevel castTo(Type* destType) const override;
        std::string getSymbolID(Type* thisType) const override;

        bool isFunctionType() const override;

        static FunctionType* Create(Type* returnType, const std::vector<Type*>& argumentTypes);
        static void Reset();

    private:
        Type* mReturnType;
        std::vector<Type*> mArgumentTypes;
    };
}

#endif //BIBBLEC_TYPE_FUNCTION_TYPE_H
