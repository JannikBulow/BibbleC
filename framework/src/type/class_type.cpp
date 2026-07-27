// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/type/class_type.h"

namespace bibblec {
    ClassType::ClassType(std::string name)
        : Type("class " + name)
        , mName(std::move(name)) {}

    std::string_view ClassType::getName() const {
        return mName;
    }

    int ClassType::getSize() const {
        return 8;
    }

    bibblir::Type* ClassType::getBibblirType() const {
        return bibblir::Type::GetClassType();
    }

    Type::CastLevel ClassType::castTo(Type* destType) const {
        return CastLevel::Disallowed;
    }

    std::string ClassType::getSymbolID(Type* thisType) const {
        return "C" + mName + ";";
    }

    bool ClassType::isClassType() const {
        return true;
    }

    static std::vector<std::unique_ptr<ClassType>> classTypes;

    std::vector<ClassType*> ClassType::GetAllClassTypes() {
        std::vector<ClassType*> result;
        result.reserve(classTypes.size());
        for (const auto& type : classTypes) {
            result.emplace_back(type.get());
        }
        return result;
    }

    ClassType* ClassType::Create(std::string name) {
        auto it = std::ranges::find_if(classTypes, [&name](const auto& type) { return type->getName() == name; });
        if (it != classTypes.end()) return it->get();

        classTypes.push_back(std::make_unique<ClassType>(std::move(name)));
        return classTypes.back().get();
    }

    void ClassType::Reset() {
        classTypes.clear();
    }
}
