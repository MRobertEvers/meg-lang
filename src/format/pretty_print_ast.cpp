#include "pretty_print_ast.h"

#include "FormatParser.h"
#include "NodeFormatter.h"
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

		if( span->is_raw() )
		{
			std::cout << span->get_raw();
		}
		else
		{
			auto iter = span->children.rbegin();
			for( ; iter != span->children.rend(); ++iter )
			{
				stack.push_back(&*iter);
			}
		}
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

		if( span->is_raw() )
		{
			current_line.push_back(span);
			if( span->is_terminator() )
			{
				lines.push_back(current_line);
				current_line.clear();
			}
		}
		else
		{
			if( (span->is_terminator()) )
			{
				auto iter = span->children.rbegin();
				for( ; iter != span->children.rend(); ++iter )
				{
					stack.push_back(&*iter);
				}
			}
			else
			{
				current_line.push_back(span);
			}
		}
	}

	if( current_line.size() != 0 )
	{
		lines.push_back(current_line);
	}

	return lines;
}

void
pretty_print_ast(Vec<Token> const& source, ast::IAstNode const* node)
{
	int max_line_len = 5;

	auto span = FormatParser::get_span(&source, node);

	Vec<Vec<NodeSpan*>> next_lines = break_into_lines(&span);

	NodeFormatter fmt{&source};

	Vec<Vec<NodeSpan*>> current_lines{};
	bool did_break = false;
	do
	{
		did_break = false;
		if( current_lines.size() == 0 )
		{
			current_lines = next_lines;
			next_lines.clear();
		}

		for( auto& line : current_lines )
		{
			int line_len = 0;

			int max_prio = 0;
			for( auto& span : line )
			{
				line_len += span->get_length();
				if( span->priority > max_prio )
				{
					max_prio = span->priority;
				}
			}

			if( line_len <= max_line_len || max_prio == 0 )
			{
				next_lines.push_back(line);
				continue;
			}

			Vec<NodeSpan*> line_partial_front;
			int i;
			for( i = 0; i < line.size(); ++i )
			{
				auto& span = line[i];
				if( span->priority >= max_prio )
				{
					did_break = true;

					// print_span(span);
					// std::cout << std::endl;
					auto broken_node_lines = fmt.break_node(span->get_opaque());
					// print(broken_node_lines);
					// std::cout << std::endl;

					// TODO: this leaks
					if( broken_node_lines.size() > 0 )
					{
						auto iter = broken_node_lines[0].begin();

						Vec<NodeSpan*> follow;
						for( ; iter != broken_node_lines[0].end(); ++iter )
						{
							line_partial_front.push_back(new NodeSpan{*iter});
						}
					}

					next_lines.push_back(line_partial_front);

					if( broken_node_lines.size() > 1 )
					{
						auto iter = broken_node_lines.begin();
						iter++;
						for( ; iter != broken_node_lines.end(); ++iter )
						{
							Vec<NodeSpan*> ll;
							for( auto& sp : *iter )
							{
								ll.push_back(new NodeSpan{sp});
							}
							next_lines.push_back(ll);
						}
					}

					i++;
					break;
				}
				else
				{
					line_partial_front.push_back(span);
				}
			}

			for( ; i < line.size(); ++i )
			{
				auto& span = line[i];
				next_lines.back().push_back(span);
			}
		}
		current_lines.clear();
	} while( did_break );

	print(next_lines);
}