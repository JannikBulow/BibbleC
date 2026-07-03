// Copyright 2026 Jannik Laugmand Bülow

#include "BibbleC/diagnostic/diagnostics.h"

#include <algorithm>
#include <format>

namespace bibblec::diagnostic {
    Diagnostics::Diagnostics(std::ostream& errorStream, std::ostream& warningStream)
        : mErrorStream(errorStream)
        , mWarningStream(warningStream) {
        mWarnings.reserve(KNOWN_WARNINGS.size());
        std::ranges::copy(KNOWN_WARNINGS, std::back_inserter(mWarnings));
    }

    void Diagnostics::addText(std::string path, std::string_view text) {
        mTexts[std::move(path)] = text;
    }

    void Diagnostics::setWarning(std::string_view warning, bool enable) {
        auto warningIt = std::ranges::find(KNOWN_WARNINGS, warning);
        if (warningIt == KNOWN_WARNINGS.end()) {
            fatalError(std::format("unknown warning: -W{}{}", enable ? "" : "no-", warning));
        }

        auto it = std::ranges::find(mWarnings, warning);
        if (enable && it == mWarnings.end()) mWarnings.emplace_back(warning);
        if (!enable && it != mWarnings.end()) mWarnings.erase(it);
    }

    void Diagnostics::disableAllWarnings() {
        mWarnings.clear();
    }

    void Diagnostics::fatalError(std::string_view message) {
        mErrorStream << std::format("{}bibble: {}fatal error: {}{}", fmt::bold, fmt::red, fmt::reset, message) << std::endl;
        std::exit(1);
    }

    void Diagnostics::reportCompilerError(lexer::SourceLocation start, lexer::SourceLocation end, std::string_view message) {
        std::string_view text = mTexts.find(start.file)->second;

        unsigned int lineStart = getLinePosition(text, start.line - 1);
        unsigned int lineEnd = getLinePosition(text, end.line) - 1;

        end.position += 1;

        std::string before = std::string(text.substr(lineStart, start.position - lineStart));
        std::string_view error = text.substr(start.position, end.position - start.position);
        std::string_view after = text.substr(end.position, lineEnd - end.position);
        std::string spacesBefore = std::string(std::to_string(start.line).length(), ' ');
        std::string spacesAfter = std::string(before.length(), ' ');

        std::ranges::for_each(before, [](char& c) { if (c == '\t') c = ' '; });

        mErrorStream << std::format("{}{}:{}:{} {}error: {}{}\n", fmt::bold, start.file, start.line, start.col, fmt::red, fmt::reset, message);
        mErrorStream << std::format("    {} | {}{}{}{}{}{}\n", start.line, before, fmt::bold, fmt::red, error, fmt::reset, after);
        mErrorStream << std::format("    {} | {}{}{}^{}{}\n", spacesBefore, spacesAfter, fmt::bold, fmt::red, std::string(error.length() - 1, '~'), fmt::reset);
    }

    void Diagnostics::reportCompilerWarning(lexer::SourceLocation start, lexer::SourceLocation end, std::string_view warning, std::string_view message) {
        auto it = std::find(mWarnings.begin(), mWarnings.end(), warning);
        if (it == mWarnings.end()) return;

        auto text = mTexts[std::string(start.file)];

        unsigned int lineStart = getLinePosition(text, start.line - 1);
        unsigned int lineEnd = getLinePosition(text, end.line) - 1;

        end.position += 1;

        std::string_view before = text.substr(lineStart, start.position - lineStart);
        std::string_view error = text.substr(start.position, end.position - start.position);
        std::string_view after = text.substr(end.position, lineEnd - end.position);
        std::string spacesBefore = std::string(std::to_string(start.line).length(), ' ');
        std::string spacesAfter = std::string(before.length(), ' ');

        mWarningStream << std::format("{}{}:{}:{} {}warning: {}{}", fmt::bold, start.file, start.line, start.col, fmt::yellow, fmt::reset, message);
        mWarningStream << std::format(" [{}{}-W{}{}]\n", fmt::bold, fmt::yellow, warning, fmt::reset);
        mWarningStream << std::format("    {} | {}{}{}{}{}{}\n", start.line, before, fmt::bold, fmt::yellow, error, fmt::reset, after);
        mWarningStream << std::format("    {} | {}{}{}^{}{}\n", spacesBefore, spacesAfter, fmt::bold, fmt::yellow, std::string(error.length()-1, '~'), fmt::reset);
    }

    unsigned int Diagnostics::getLinePosition(std::string_view text, unsigned int lineNumber) {
        unsigned int line = 0;
        for (unsigned int i = 0; i < lineNumber; i++) {
            while (text[line] != '\n') {
                line++;
            }
            line++;
        }
        return line;
    }
}
