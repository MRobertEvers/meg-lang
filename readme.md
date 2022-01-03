# Meg Lang

In this branch, we are officially compiling input source code.

Module expects single top level `fn` definition. Functions can have a single return statement.

```
fn func(): i8 
{ 
    return 9*8; 
}
```

# Notes on Code Generation

LLVM has the following mental model. A module contains functions and variables among other things. Functions contains blocks.

For generating an AST for code gen.

```
statements do not return a value from codegen;
expressions do return a value from codegen;

a block contains a list of statements
e.g. a return statement, for loop, if statement. 
(Rust does this in their AST too)
```


# Resources

The Rust Parser entry point is in `compiler/rustc_parse/src/parser/item.rs` with function `parse_mod`.

`parse_item_kind` function appears close to the top of the parse and appears to parse all top level structures of a module.
