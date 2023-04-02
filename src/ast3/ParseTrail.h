#pragma once

#include "Span.h"
#include "lex3/Cursor.h"

#include <set>

class ParserMetaInformation
{
public:
	std::set<int> comments;
};

/**
 * @brief Tracks source code information during parsing.
 *
 * Create this class when a node starts parsing, then
 * call 'mark' to denote end of parsing. This will attempt
 * to capture any meta-information about the source that is
 * not normally represented in the AST. E.g. Comments.
 *
 */
class ParseTrail
{
	Cursor& cursor;
	ParserMetaInformation& meta;
	int start = 0;
	int size = 0;

public:
	ParseTrail(Cursor& cursor, ParserMetaInformation& meta)
		: cursor(cursor)
		, meta(meta)
		, start(0){};

	Span mark();
};