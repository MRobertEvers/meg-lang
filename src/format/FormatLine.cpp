#include "FormatLine.h"

#include "ast/ast.h"

#include <iostream>

void
LineSpan::append_span(LineSpan span)
{
	if( span.priority > priority )
	{
		priority = span.priority;
	}

	if( span.is_terminator() )
	{
		contains_terminal = true;
	}

	children.push_back(span);
}

static void
print_span(LineSpan* span)
{
	Vec<LineSpan*> stack;
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
print(Vec<Vec<LineSpan>>& lines)
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
print(Vec<Vec<LineSpan*>>& lines)
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

Vec<Vec<LineSpan*>>
break_into_lines(LineSpan* root)
{
	Vec<Vec<LineSpan*>> lines;

	// Break any mandatory line breaks;
	Vec<LineSpan*> current_line;
	Vec<LineSpan*> stack;
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

Vec<LineSpan>
GatherIntoLines::gather_into_lines(ast::IAstNode const* node)
{
	int max_line_len = 5;

	node->visit(this);

	Vec<Vec<LineSpan*>> next_lines = break_into_lines(&current_line);
	print(next_lines);

	BreakNode breaker;

	Vec<Vec<LineSpan*>> current_lines{};
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

			Vec<LineSpan*> line_partial_front;
			int i;
			for( i = 0; i < line.size(); ++i )
			{
				auto& span = line[i];
				if( span->priority >= max_prio )
				{
					did_break = true;

					// print_span(span);
					// std::cout << std::endl;
					auto broken_node_lines = breaker.break_node(span->get_opaque());
					// print(broken_node_lines);
					// std::cout << std::endl;

					// TODO: this leaks
					if( broken_node_lines.size() > 0 )
					{
						auto iter = broken_node_lines[0].begin();

						Vec<LineSpan*> follow;
						for( ; iter != broken_node_lines[0].end(); ++iter )
						{
							line_partial_front.push_back(new LineSpan{*iter});
						}
					}

					next_lines.push_back(line_partial_front);

					if( broken_node_lines.size() > 1 )
					{
						auto iter = broken_node_lines.begin();
						iter++;
						for( ; iter != broken_node_lines.end(); ++iter )
						{
							Vec<LineSpan*> ll;
							for( auto& sp : *iter )
							{
								ll.push_back(new LineSpan{sp});
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

	std::cout << "HELLO" << std::endl;
	print(next_lines);

	return {};
}

void
GatherIntoLines::visit(ast::Module const* node)
{
	OpaqueScope scope{*this, node};

	for( auto const& s : node->statements )
	{
		s->visit(this);
	}
}

void
GatherIntoLines::visit(ast::Function const* node)
{
	OpaqueScope scope{*this, node};

	node->Proto->visit(this);

	node->Body->visit(this);
}

void
GatherIntoLines::visit(ast::Block const* node)
{
	OpaqueScope scope{*this, node};

	append_span(LineSpan{0, "{\n"});

	{
		IndentScope s{*this};
		for( auto& stmt : node->statements )
		{
			stmt->visit(this);
		}
	}

	append_span(LineSpan{0, "}\n"});
}

void
GatherIntoLines::visit(ast::BinaryOperation const* node)
{
	OpaqueScope scope{*this, node};
	node->LHS->visit(this);

	append_span(LineSpan{0, " "});
	append_span(LineSpan{0, String{node->Op} + " "});
	node->RHS->visit(this);
}

void
GatherIntoLines::visit(ast::Number const* node)
{
	OpaqueScope scope{*this, node};
	append_span(LineSpan{0, std::to_string(node->Val)});
}

void
GatherIntoLines::visit(ast::Return const* node)
{
	OpaqueScope scope{*this, node};
	append_span(LineSpan{0, "return "});

	node->ReturnExpr->visit(this);
	append_span(LineSpan{0, ";\n"});
}

void
GatherIntoLines::visit(ast::Prototype const* node)
{
	OpaqueScope scope{*this, node, 3};

	append_span(LineSpan{0, "fn "});
	node->Name->visit(this);
	append_span(LineSpan{0, "("});

	auto& args = node->get_parameters()->Parameters;

	for( int i = 0; i < args.size(); i++ )
	{
		auto& arg = args[i];

		arg->Name->visit(this);

		append_span(LineSpan{0, ": "});

		arg->Type->visit(this);

		if( i != args.size() - 1 )
		{
			append_span(LineSpan{0, ", "});
		}
	}

	append_span(LineSpan{0, "): "});

	node->ReturnType->visit(this);
}

void
GatherIntoLines::visit(ast::ValueIdentifier const* node)
{
	OpaqueScope scope{*this, node};
	append_span(LineSpan{0, node->get_name()});
}

void
GatherIntoLines::visit(ast::TypeIdentifier const* node)
{
	OpaqueScope scope{*this, node};
	append_span(LineSpan{0, node->get_name()});
}

void
GatherIntoLines::visit(ast::Let const* node)
{
	OpaqueScope scope{*this, node, 2};
	append_span(LineSpan{0, "let "});

	node->Name->visit(this);
	if( !node->Type->is_empty() )
	{
		append_span(LineSpan{0, ": "});
		node->Type->visit(this);
	}

	append_span(LineSpan{0, " = "});

	node->RHS->visit(this);

	append_span(LineSpan{0, ";\n"});
}

void
GatherIntoLines::visit(ast::Struct const* node)
{
	OpaqueScope scope{*this, node};
	append_span(LineSpan{0, "struct "});

	node->TypeName->visit(this);
	append_span(LineSpan{0, " {\n"});

	{
		IndentScope s{*this};
		for( auto& m : node->MemberVariables )
		{
			m->Name->visit(this);

			append_span(LineSpan{0, ": "});

			m->Type->visit(this);

			append_span(LineSpan{0, ";\n"});
		}
	}

	append_span(LineSpan{0, "}\n"});
}

void
GatherIntoLines::visit(ast::MemberReference const* node)
{
	OpaqueScope scope{*this, node, 0};
	node->base->visit(this);
	append_span(LineSpan{0, "."});
	node->name->visit(this);
}

void
GatherIntoLines::visit(ast::TypeDeclarator const* node)
{
	OpaqueScope scope{*this, node};
	auto base = node->get_base();
	if( base )
	{
		base->visit(this);
	}
	append_span(LineSpan{0, node->get_name()});
}

void
GatherIntoLines::visit(ast::If const* node)
{
	OpaqueScope scope{*this, node};
	append_span(LineSpan{0, "if "});

	node->condition->visit(this);
	append_span(LineSpan{0, " "});
	node->then_block->visit(this);

	if( !node->else_block.is_null() )
	{
		append_span(LineSpan{0, "else "});

		node->else_block->visit(this);
	}
}

void
GatherIntoLines::visit(ast::Assign const* node)
{
	OpaqueScope scope{*this, node};
	node->lhs->visit(this);
	append_span(LineSpan{0, " = "});

	node->rhs->visit(this);
	append_span(LineSpan{0, ";\n"});
}

void
GatherIntoLines::visit(ast::While const* node)
{
	OpaqueScope scope{*this, node};
	append_span(LineSpan{0, "while "});

	node->condition->visit(this);
	append_span(LineSpan{0, " "});
	node->loop_block->visit(this);
}

void
GatherIntoLines::visit(ast::Call const* node)
{
	OpaqueScope scope{*this, node, 3};
	node->call_target->visit(this);
	append_span(LineSpan{0, "("});

	auto& args = node->args.args;
	for( int i = 0; i < args.size(); i++ )
	{
		auto& arg = args[i];
		arg->visit(this);

		if( i != args.size() - 1 )
		{
			append_span(LineSpan{0, ","});
		}
	}

	append_span(LineSpan{0, ")"});
}

void
GatherIntoLines::visit(ast::Statement const* node)
{
	OpaqueScope scope{*this, node};
	node->stmt->visit(this);
}

void
GatherIntoLines::visit(ast::Expression const* node)
{
	OpaqueScope scope{*this, node};
	node->base->visit(this);
}

void
GatherIntoLines::append_span(LineSpan span)
{
	if( need_indent )
	{
		current_line.append_span(LineSpan{0, String(current_indentation * 4, ' ')});
		need_indent = false;
	}

	current_line.append_span(span);

	if( span.is_raw() && span.is_terminator() )
	{
		need_indent = true;
	}
}

LineSpan
GatherIntoLines::get_span(ast::IAstNode const* node, int indentation)
{
	GatherIntoLines g{indentation};
	node->visit(&g);

	return g.current_line.children[0];
}

Vec<Vec<LineSpan>>
BreakNode::break_node(ast::IAstNode const* node)
{
	result.clear();
	node->visit(this);

	if( result.size() == 0 )
	{
		Vec<LineSpan> dummy;
		auto span = GatherIntoLines::get_span(node);
		span.priority = 0;
		dummy.push_back(span);
		result.push_back(dummy);
	}

	return result;
}

void
BreakNode::visit(ast::Module const* node)
{}

void
BreakNode::visit(ast::Function const* node)
{}

void
BreakNode::visit(ast::Block const* node)
{}

void
BreakNode::visit(ast::BinaryOperation const* node)
{}

void
BreakNode::visit(ast::Number const* node)
{}

void
BreakNode::visit(ast::Return const* node)
{}

void
BreakNode::visit(ast::Prototype const* node)
{
	Vec<LineSpan> line;

	line.push_back(LineSpan{0, "fn "});
	line.push_back(GatherIntoLines::get_span(node->Name.get()));
	line.push_back(LineSpan{0, "(\n"});

	result.push_back(line);
	line.clear();

	auto& args = node->get_parameters()->Parameters;

	for( int i = 0; i < args.size(); i++ )
	{
		auto& arg = args[i];
		line.push_back(GatherIntoLines::get_span(arg->Name.get(), 1));

		line.push_back(LineSpan{0, ": "});

		line.push_back(GatherIntoLines::get_span(arg->Type.get(), 0));

		if( i != args.size() - 1 )
		{
			line.push_back(LineSpan{0, ",\n"});
		}
		else
		{
			line.push_back(LineSpan{0, "\n"});
		}
		result.push_back(line);
		line.clear();
	}

	line.push_back(LineSpan{0, "): "});

	line.push_back(GatherIntoLines::get_span(node->ReturnType.get()));

	line.push_back(LineSpan{0, " "});
	result.push_back(line);
}

void
BreakNode::visit(ast::ValueIdentifier const* node)
{}

void
BreakNode::visit(ast::TypeIdentifier const* node)
{}

void
BreakNode::visit(ast::Let const* node)
{}

void
BreakNode::visit(ast::Struct const* node)
{}

void
BreakNode::visit(ast::MemberReference const* node)
{}

void
BreakNode::visit(ast::TypeDeclarator const* node)
{}

void
BreakNode::visit(ast::If const* node)
{}

void
BreakNode::visit(ast::Assign const* node)
{}

void
BreakNode::visit(ast::While const* node)
{}

void
BreakNode::visit(ast::Call const* node)
{}

void
BreakNode::visit(ast::Statement const* node)
{}

void
BreakNode::visit(ast::Expression const* node)
{}