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

The function block codegen is `compiler/rustc_codegen_ssa/src/mir/block.rs`; it can show how to gen a function return block.

```
impl<'a, 'tcx, Bx: BuilderMethods<'a, 'tcx>> FunctionCx<'a, 'tcx, Bx> {
```

Rust also checks if an Expr returns here, compiler/rustc_ast/src/ast.rs
```rust
impl Expr {
    /// Returns `true` if this expression would be valid somewhere that expects a value;
    /// for example, an `if` condition.
    pub fn returns(&self) -> bool {
        if let ExprKind::Block(ref block, _) = self.kind {
            match block.stmts.last().map(|last_stmt| &last_stmt.kind) {
                // Implicit return
                Some(StmtKind::Expr(_)) => true,
                // Last statement is an explicit return?
                Some(StmtKind::Semi(expr)) => matches!(expr.kind, ExprKind::Ret(_)),
                // This is a block that doesn't end in either an implicit or explicit return.
                _ => false,
            }
        } else {
            // This is not a block, it is a value.
            true
        }
    }
    ```