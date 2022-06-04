#include "pretty_print_ast.h"

#include "FormatParser.h"
#include "NodeSpan.h"
#include "common/String.h"
#include "common/Vec.h"
#include "reverse.h"
#include "utils.h"

#include <iostream>

enum class PrintMode
{
	line,
	break_line
};

struct AstCallIteration
{
	int indentation = 0;
	PrintMode mode;
	NodeSpan* doc;

	AstCallIteration(int ind, PrintMode mode, NodeSpan* doc)
		: indentation(ind)
		, mode(mode)
		, doc(doc){};
};

static int calc_len(NodeSpan& root_doc);

template<typename Callback>
static void
traverse_format_ast(NodeSpan& root_doc, Callback out)
{
	int max_line_len = 80;

	Vec<AstCallIteration> stack;

	// Line suffixes are defered and printed at the nearest line break.
	Vec<AstCallIteration> line_suffixes;
	int current_line_len = 0;

	// this value is used to support the 'min_spacing' node.
	int distance_to_non_empty = -1;

	// this is used to support the 'min_newline' node.
	int num_consecutive_newlines = 0;

	auto emit_string =
		[&current_line_len, &distance_to_non_empty, &num_consecutive_newlines, &out](String s) {
			if( s.back() == '\n' )
			{
				current_line_len = 0;
				distance_to_non_empty = -1;
				num_consecutive_newlines += 1;
			}
			else
			{
				auto ending_whitespace = count_end_whitespace(s);
				if( ending_whitespace != s.length() && distance_to_non_empty == -1 )
					distance_to_non_empty = 0;

				if( ending_whitespace != s.length() )
					num_consecutive_newlines = 0;

				distance_to_non_empty = ending_whitespace;

				current_line_len += s.length();
			}

			out(s);
		};

	stack.emplace_back(0, PrintMode::line, &root_doc);

	while( !stack.empty() )
	{
		auto iter = stack.back();
		stack.pop_back();

		auto ind = iter.indentation;
		auto mode = iter.mode;
		auto doc = iter.doc;

		auto emit_line_break = [&emit_string, &line_suffixes, &stack, &ind, &mode, &doc]() {
			// If there are line breaks, insert the line_suffixes
			// before breaking the line.
			if( line_suffixes.size() != 0 )
			{
				stack.emplace_back(ind, mode, doc);

				for( auto& suffix : reverse(line_suffixes) )
				{
					stack.emplace_back(suffix);
				}

				line_suffixes.clear();
			}
			else
			{
				emit_string("\n");
				emit_string(String(ind, ' '));
			}
		};

		switch( doc->type )
		{
		case NodeSpan::SpanType::text:
		{
			emit_string(doc->content);
		}
		break;
		case NodeSpan::SpanType::document:
		{
			for( auto& node : reverse(doc->children) )
			{
				stack.emplace_back(ind, mode, &node);
			}
		}
		break;
		case NodeSpan::SpanType::min_padding:
		{
			if( distance_to_non_empty != -1 && distance_to_non_empty < doc->min_spacing )
			{
				emit_string(String(doc->min_spacing - distance_to_non_empty, ' '));
			}
		}
		break;
		case NodeSpan::SpanType::min_newline:
		{
			if( num_consecutive_newlines < doc->min_spacing )
			{
				auto need = doc->min_spacing - num_consecutive_newlines;
				for( int i = 0; i < need; ++i )
				{
					emit_line_break();
				}
			}
		}
		break;
		case NodeSpan::SpanType::group:
		{
			auto group_mode = PrintMode::line;
			// If the group wants to be printed on a single line, then allow it
			// to do so if it fits.
			if( !doc->is_break() )
			{
				auto size = current_line_len;
				for( auto& node : doc->children )
				{
					size += calc_len(node);
				}

				if( size > max_line_len )
				{
					group_mode = PrintMode::break_line;
				}
				else
				{
					group_mode = PrintMode::line;
				}
			}
			else
			{
				group_mode = PrintMode::break_line;
			}

			for( auto& node : reverse(doc->children) )
			{
				stack.emplace_back(ind, group_mode, &node);
			}
		}
		break;

		case NodeSpan::SpanType::hard_line:
			// hard_line is always break.
			mode = PrintMode::break_line;
			// Fallthrough
		case NodeSpan::SpanType::soft_line:
			if( mode == PrintMode::line )
			{
				// soft_line is nothing in line mode.
				break;
			}
			else
			{
				// Fallthrough!
			}

		// The 'line' span is either a space if we are printing on
		// one line, or a line break if we are in break mode.
		case NodeSpan::SpanType::line:
		{
			if( mode == PrintMode::line )
			{
				emit_string(" ");
			}
			else
			{
				emit_line_break();
			}
		}
		break;

		case NodeSpan::SpanType::indent:
		{
			for( auto& node : reverse(doc->children) )
			{
				stack.emplace_back(ind + 4, mode, &node);
			}
		}
		break;
		case NodeSpan::SpanType::line_suffix:
		{
			for( auto& elem : doc->children )
			{
				line_suffixes.emplace_back(ind, mode, &elem);
			}
		}
		break;
		default:
			std::cout << "Unsupported SpanType" << std::endl;
			break;
		}
	}
}

// Performs look ahead to calculate the length of a group node.
// And returns the length of the first line.
static int
calc_len(NodeSpan& root_doc)
{
	int len = 0;
	bool done = false;

	// TODO: Way to stop traversal?
	traverse_format_ast(root_doc, [&len, &done](String s) {
		if( done )
			return;
		auto break_line = s.find("\n");
		if( break_line == String::npos )
			len += s.length();
		else
		{
			len += break_line;
			done = true;
		}
	});

	return len;
}

static int
pretty_print_ast_stdout(NodeSpan& span)
{
	Vec<String> out;
	traverse_format_ast(span, [&out](String s) { out.emplace_back(s); });

	for( auto& s : out )
	{
		std::cout << s;
	}
	std::cout << std::flush;
	return 0;
}

void
pretty_print_ast(Vec<Token> const& source, ast::IAstNode const* node)
{
	auto root_doc = FormatParser::get_span(&source, node);

	pretty_print_ast_stdout(root_doc);
}