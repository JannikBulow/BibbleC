// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/type/error_type.h"
#include "BibbleC/type/function_type.h"
#include "BibbleC/type/integer_type.h"
#include "BibbleC/type/type.h"

#include "BibbleC/util/better_strings.h"

#include <memory>
#include <unordered_map>

namespace bibblec {
    static std::unordered_map<std::string, std::unique_ptr<Type>, StringHash, StringEqual> types;
    static std::unordered_map<std::string, Type*, StringHash, StringEqual> aliases;

    void Type::Init() {
        types["byte"] = std::make_unique<IntegerType>(IntegerType::Byte, true);
        types["short"] = std::make_unique<IntegerType>(IntegerType::Short, true);
        types["int"] = std::make_unique<IntegerType>(IntegerType::Int, true);
        types["long"] = std::make_unique<IntegerType>(IntegerType::Long, true);
        types["ubyte"] = std::make_unique<IntegerType>(IntegerType::Byte, false);
        types["ushort"] = std::make_unique<IntegerType>(IntegerType::Short, false);
        types["uint"] = std::make_unique<IntegerType>(IntegerType::Int, false);
        types["ulong"] = std::make_unique<IntegerType>(IntegerType::Long, false);

        types["error-type"] = std::make_unique<ErrorType>();

        AddAlias("char", types["byte"].get());
    }

    void Type::AddAlias(std::string name, Type* type) {
        aliases[std::move(name)] = type;
    }

    bool Type::Exists(std::string_view name) {
        return Get(name) != nullptr;
    }

    Type* Type::Get(std::string_view name) {
        auto typeIt = types.find(name);
        if (typeIt != types.end()) return typeIt->second.get();

        auto aliasIt = aliases.find(name);
        if (aliasIt != aliases.end()) return aliasIt->second;

        return nullptr;
    }

    void Type::Reset() {
        types.clear();
        aliases.clear();

        FunctionType::Reset();
    }
}
