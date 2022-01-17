#pragma once

#include "ast/IAstNode.h"
#include "common/Vec.h"
#include "lexer/token.h"

/**
 * @brief
 *
 * Pretty prints the ast. Uses the same algorithm that Prettier uses (the js formatting library)
 *
 * https://homepages.inf.ed.ac.uk/wadler/papers/prettier/prettier.pdf
 *
 * The idea is to create a "Formatting AST" out of the AST, then traverse the
 * and print the result.
 *
 * The complexity comes in how the AST is traversed. Some nodes require look ahead
 * and back-tracking (i.e. a group node), to calculate if it can fit on a line.
 *
 * @param source
 * @param node
 */
void pretty_print_ast(Vec<Token> const& source, ast::IAstNode const* node);