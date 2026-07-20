// Copyright 2026 Jannik Laugmand Bülow

#ifndef BIBBLEC_DEBUG_SOURCE_PAIR_H
#define BIBBLEC_DEBUG_SOURCE_PAIR_H

#include "BibbleC/lexer/source_location.h"

namespace bibblec {
    struct SourcePair {
        lexer::SourceLocation start;
        lexer::SourceLocation end;
    };
}

#endif //BIBBLEC_DEBUG_SOURCE_PAIR_H
