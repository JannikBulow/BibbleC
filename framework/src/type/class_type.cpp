// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/type/class_type.h"

#include <format>

namespace bibblec {
    ClassType::ClassType(std::string moduleName, std::string name)
        : Type(std::format("class {}::{}", moduleName, name))
        , mModuleName(std::move(moduleName))
        , mClassName(std::move(name)) {}

    std::string_view ClassType::getModuleName() const {
        return mModuleName;
    }

    std::string_view ClassType::getClassName() const {
        return mClassName;
    }

    int ClassType::getSize() const {
        return 8;
    }

    bibblir::Type* ClassType::getBibblirType() const {
        return bibblir::Type::GetClassType(mModuleName, mClassName);
    }

    Type::CastLevel ClassType::castTo(Type* destType) const {
        return CastLevel::Disallowed;
    }

    std::string ClassType::getSymbolID(Type* thisType) const {
        return std::format("C{}/{};", mModuleName, mClassName);
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

    ClassType* ClassType::Get(const std::string& moduleName, const std::string& name) {
        auto it = std::ranges::find_if(classTypes, [&name, &moduleName](const auto& type) { return type->getClassName() == name && type->getModuleName() == moduleName; });
        if (it != classTypes.end()) return it->get();
        return nullptr;
    }

    ClassType* ClassType::Create(std::string moduleName, std::string name) {
        ClassType* found = Get(moduleName, name);
        if (found) return found;

        classTypes.push_back(std::make_unique<ClassType>(std::move(moduleName), std::move(name)));
        return classTypes.back().get();
    }

    void ClassType::Reset() {
        classTypes.clear();
    }
}
