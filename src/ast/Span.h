

#pragma once

namespace ast
{

/**
 * @brief Used to keep track of the appears of a node in the source code for pretty printing.
 *
 */
struct Span
{
	int start = 0;
	int size = 0;
	Span(){};
	Span(int start, int size)
		: start(start)
		, size(size){};
	// char const* start = nullptr;
	// unsigned int size = 0;
	// unsigned int num_trailing_newlines = 0;

	// /**
	//  * @brief Construct a new Span object
	//  * I'm using duck typing here because I don't want to include files from 'lexer' in 'ast'
	//  *
	//  * TODO: Better way to do this?
	//  *
	//  * @tparam TDuck
	//  * @param duck
	//  */
	// template<typename TDuck>
	// Span(TDuck& duck)
	// 	: start(duck.start)
	// 	, size(duck.size)
	// 	, num_trailing_newlines(duck.num_trailing_newlines)
};
} // namespace ast