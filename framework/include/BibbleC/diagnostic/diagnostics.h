// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_DIAGNOSTIC_DIAGNOSTICS_H
#define BIBBLEC_DIAGNOSTIC_DIAGNOSTICS_H

#include "BibbleC/debug/source_pair.h"

#include "BibbleC/lexer/SourceLocation.h"

#include "BibbleC/util/better_strings.h"

#include "BibbleC/api.h"

#include <array>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace bibblec {
    namespace fmt {
        constexpr std::string_view bold     = "\x1b[1m";
        constexpr std::string_view red      = "\x1b[31m";
        constexpr std::string_view yellow   = "\x1b[93m";
        constexpr std::string_view reset    = "\x1b[0m";
    }

    namespace diagnostic {
        constexpr std::array KNOWN_WARNINGS = {
            "implicit"
        };

        class BIBBLEC_EXPORT Diagnostics {
        public:
            Diagnostics(std::ostream& errorStream, std::ostream& warningStream);

            void addText(std::string path, std::string_view text);
            void setWarning(std::string_view warning, bool enable);
            void disableAllWarnings();

            [[noreturn]] void fatalError(std::string_view message);

            void reportCompilerError(lexer::SourceLocation start, lexer::SourceLocation end, std::string_view message);
            void reportCompilerError(const SourcePair& source, std::string_view message) { reportCompilerError(source.start, source.end, message); }
            void reportCompilerWarning(lexer::SourceLocation start, lexer::SourceLocation end, std::string_view warning, std::string_view message);
            void reportCompilerWarning(const SourcePair& source, std::string_view warning, std::string_view message) { reportCompilerWarning(source.start, source.end, warning, message); }

        private:
            std::ostream& mErrorStream;
            std::ostream& mWarningStream;

            std::unordered_map<std::string, std::string_view, StringHash, StringEqual> mTexts;
            std::vector<std::string_view> mWarnings;

            unsigned int getLinePosition(std::string_view text, unsigned int line);
        };
    }
}

#endif //BIBBLEC_DIAGNOSTIC_DIAGNOSTICS_H
