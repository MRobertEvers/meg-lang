#pragma once

// clang-format off
#define CHAR_WHITESPACE_CASES \
    ' ':  \
    case '\n': \
    case '\r': \
    case '\t'

#define CHAR_IDENTIFIER_START_CASES \
	'a'...'z': \
	case 'A'...'Z': \
	case '_': \
	case '@'

#define CHAR_DIGIT_CASES \
         '0'...'9'

#define CHAR_IDENTIFIER_CASES \
    CHAR_IDENTIFIER_START_CASES: \
    case CHAR_DIGIT_CASES

// clang-format on