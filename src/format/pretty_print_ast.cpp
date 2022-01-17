#include "pretty_print_ast.h"

#include "FormatParser.h"
#include "NodeSpan.h"
#include "common/String.h"
#include "common/Vec.h"

#include <iostream>

static void
print_span(NodeSpan* span)
{
	Vec<NodeSpan*> stack;
	stack.push_back(span);

	while( stack.size() != 0 )
	{
		auto span = stack.back();
		stack.pop_back();

		// if( span->is_raw() )
		// {
		// 	std::cout << span->get_raw();
		// }
		// else
		// {
		// 	auto iter = span->children.rbegin();
		// 	for( ; iter != span->children.rend(); ++iter )
		// 	{
		// 		stack.push_back(&*iter);
		// 	}
		// }
	}

	std::cout << std::flush;
}

static void
print(Vec<Vec<NodeSpan>>& lines)
{
	for( auto& line : lines )
	{
		for( auto& span : line )
		{
			print_span(&span);
		}

		std::cout << std::flush;
	}
}
static void
print(Vec<Vec<NodeSpan*>>& lines)
{
	for( auto& line : lines )
	{
		for( auto& span : line )
		{
			print_span(span);
		}

		std::cout << std::flush;
	}
}

Vec<Vec<NodeSpan*>>
break_into_lines(NodeSpan* root)
{
	Vec<Vec<NodeSpan*>> lines;

	// Break any mandatory line breaks;
	Vec<NodeSpan*> current_line;
	Vec<NodeSpan*> stack;
	stack.push_back(root);

	while( stack.size() != 0 )
	{
		auto span = stack.back();
		stack.pop_back();

		// if( span->is_raw() )
		// {
		// 	current_line.push_back(span);
		// 	// if( span->is_terminator() )
		// 	// {
		// 	// 	lines.push_back(current_line);
		// 	// 	current_line.clear();
		// 	// }
		// }
		// else
		// {
		// 	// if( (span->is_terminator()) )
		// 	// {
		// 	// 	auto iter = span->children.rbegin();
		// 	// 	for( ; iter != span->children.rend(); ++iter )
		// 	// 	{
		// 	// 		stack.push_back(&*iter);
		// 	// 	}
		// 	// }
		// 	// else
		// 	// {
		// 	// 	current_line.push_back(span);
		// 	// }
		// }
	}

	if( current_line.size() != 0 )
	{
		lines.push_back(current_line);
	}

	return lines;
}

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

void
pretty_print_ast(Vec<Token> const& source, ast::IAstNode const* node)
{
	int max_line_len = 5;

	auto root_doc = FormatParser::get_span(&source, node);

	Vec<AstCallIteration> stack;
	Vec<String> out;

	// Line suffixes are defered and printed at the nearest line break.
	Vec<AstCallIteration> line_suffixes;
	int current_line_len = 0;

	stack.emplace_back(0, PrintMode::line, &root_doc);

	while( !stack.empty() )
	{
		auto iter = stack.back();
		stack.pop_back();

		auto ind = iter.indentation;
		auto mode = iter.mode;
		auto doc = iter.doc;

		switch( doc->type )
		{
		case NodeSpan::SpanType::text:
		{
			out.push_back(doc->content);
			current_line_len += doc->content.length();
		}
		break;
		case NodeSpan::SpanType::document:
		{
			auto group_mode = mode;
			// if( !doc->is_break() ) // Check if line fits
			// {
			// 	group_mode = PrintMode::line;
			// }
			// else
			// {
			// 	group_mode = PrintMode::break_line;
			// }

			auto child_node_iter = doc->children.rbegin();
			for( ; child_node_iter != doc->children.rend(); ++child_node_iter )
			{
				stack.emplace_back(ind, group_mode, &*child_node_iter);
			}
		}
		break;
		case NodeSpan::SpanType::group:
		{
			auto group_mode = PrintMode::line;
			if( !doc->is_break() ) // Check if line fits
			{
				group_mode = PrintMode::line;
			}
			else
			{
				group_mode = PrintMode::break_line;
			}

			auto child_node_iter = doc->children.rbegin();
			for( ; child_node_iter != doc->children.rend(); ++child_node_iter )
			{
				stack.emplace_back(ind, group_mode, &*child_node_iter);
			}
		}
		break;

		case NodeSpan::SpanType::hard_line:
			if( mode == PrintMode::line )
			{
				// hard_line is nothing in line mode.

				mode = PrintMode::break_line;

				// Fallthrough
			}
			else
			{
				// Fallthrough!
			}
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
				out.push_back(" ");
				current_line_len += 1;
			}
			else
			{
				// If there are line breaks, insert the line_suffixes
				// before breaking the line.
				if( line_suffixes.size() != 0 )
				{
					stack.emplace_back(ind, mode, doc);
					auto l_iter = line_suffixes.rbegin();
					for( ; l_iter != line_suffixes.rend(); ++l_iter )
					{
						stack.emplace_back(*l_iter);
					}
					line_suffixes.clear();
					break;
				}
				else
				{
					out.push_back("\n");
					out.push_back(String(ind, ' '));
					current_line_len = ind;
				}
			}
		}
		break;

		case NodeSpan::SpanType::indent:
		{
			auto child_node_iter = doc->children.rbegin();
			for( ; child_node_iter != doc->children.rend(); ++child_node_iter )
			{
				stack.emplace_back(ind + 4, mode, &*child_node_iter);
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

	for( auto& s : out )
	{
		std::cout << s;
	}

	std::cout << std::endl;
	// // This span is the owning span.
	// auto span = FormatParser::get_span(&source, node);

	// Vec<Vec<NodeSpan*>> next_lines = break_into_lines(&span);

	// // NodeFormatter fmt{&source};

	// // This contains a list of spans created in the process
	// // of breaking up lines. TODO: Clean this whole thing up
	// // so we don't have to do this.
	// Vec<NodeSpan> gc_spans;
	// Vec<Vec<NodeSpan*>> current_lines{};
	// bool did_break = false;
	// do
	// {
	// 	did_break = false;
	// 	if( current_lines.size() == 0 )
	// 	{
	// 		current_lines = next_lines;
	// 		next_lines.clear();
	// 	}

	// 	for( auto& line : current_lines )
	// 	{
	// 		int line_len = 0;

	// 		int max_prio = 0;
	// 		for( auto& span : line )
	// 		{
	// 			line_len += span->get_length();
	// 			// if( span->priority > max_prio )
	// 			// {
	// 			// 	max_prio = span->priority;
	// 			// }
	// 		}

	// 		if( line_len <= max_line_len || max_prio == 0 )
	// 		{
	// 			next_lines.push_back(line);
	// 			continue;
	// 		}

	// 		Vec<NodeSpan*> line_partial_front;
	// 		int i;
	// 		for( i = 0; i < line.size(); ++i )
	// 		{
	// 			auto& span = line[i];
	// 			// if( span->priority >= max_prio )
	// 			// {
	// 			// 	did_break = true;

	// 			// 	// print_span(span);
	// 			// 	// std::cout << std::endl;
	// 			// 	// auto broken_node_lines = fmt.break_node(span->get_opaque());
	// 			// 	// // print(broken_node_lines);
	// 			// 	// // std::cout << std::endl;

	// 			// 	// // TODO: this leaks
	// 			// 	// if( broken_node_lines.size() > 0 )
	// 			// 	// {
	// 			// 	// 	auto iter = broken_node_lines[0].begin();

	// 			// 	// 	Vec<NodeSpan*> follow;
	// 			// 	// 	for( ; iter != broken_node_lines[0].end(); ++iter )
	// 			// 	// 	{
	// 			// 	// 		gc_spans.emplace_back(*iter);
	// 			// 	// 		line_partial_front.push_back(&gc_spans.back());
	// 			// 	// 	}
	// 			// 	// }

	// 			// 	// next_lines.push_back(line_partial_front);

	// 			// 	// if( broken_node_lines.size() > 1 )
	// 			// 	// {
	// 			// 	// 	auto iter = broken_node_lines.begin();
	// 			// 	// 	iter++;
	// 			// 	// 	for( ; iter != broken_node_lines.end(); ++iter )
	// 			// 	// 	{
	// 			// 	// 		Vec<NodeSpan*> ll;
	// 			// 	// 		for( auto& sp : *iter )
	// 			// 	// 		{
	// 			// 	// 			gc_spans.emplace_back(sp);
	// 			// 	// 			ll.push_back(&gc_spans.back());
	// 			// 	// 		}
	// 			// 	// 		next_lines.push_back(ll);
	// 			// 	// 	}
	// 			// 	// }

	// 			// 	i++;
	// 			// 	break;
	// 			// }
	// 			// else
	// 			// {
	// 			// 	line_partial_front.push_back(span);
	// 			// }
	// 		}

	// 		for( ; i < line.size(); ++i )
	// 		{
	// 			auto& span = line[i];
	// 			next_lines.back().push_back(span);
	// 		}
	// 	}
	// 	current_lines.clear();
	// } while( did_break );

	// print(next_lines);
}