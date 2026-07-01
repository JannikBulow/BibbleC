// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_UTIL_BETTER_STRINGS_H
#define BIBBLEC_UTIL_BETTER_STRINGS_H

#include <string>

namespace bibblec {
    struct StringHash {
        using is_transparent = void;

        size_t operator()(std::string_view text) const noexcept {
            return std::hash<std::string_view>{}(text);
        }

        size_t operator()(const std::string& text) const noexcept {
            return std::hash<std::string>{}(text);
        }
    };

    struct StringEqual {
        using is_transparent = void;

        bool operator()(std::string_view a, std::string_view b) const noexcept {
            return a == b;
        }

        bool operator()(const std::string& a, const std::string& b) const noexcept {
            return a == b;
        }

        bool operator()(const std::string& a, std::string_view b) const noexcept {
            return a == b;
        }

        bool operator()(std::string_view a, const std::string& b) const noexcept {
            return a == b;
        }
    };
}

#endif //BIBBLEC_UTIL_BETTER_STRINGS_H
