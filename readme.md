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

Rust warnings on unreachable code. compiler/rustc_typeck/src/check/fn_ctxt/_impl.rs

```rust

impl<'a, 'tcx> FnCtxt<'a, 'tcx> {
    /// Produces warning on the given node, if the current point in the
    /// function is unreachable, and there hasn't been another warning.
    pub(in super::super) fn warn_if_unreachable(&self, id: hir::HirId, span: Span, kind: &str) {
        // FIXME: Combine these two 'if' expressions into one once
        // let chains are implemented
        if let Diverges::Always { span: orig_span, custom_note } = self.diverges.get() {
            // If span arose from a desugaring of `if` or `while`, then it is the condition itself,
            // which diverges, that we are about to lint on. This gives suboptimal diagnostics.
            // Instead, stop here so that the `if`- or `while`-expression's block is linted instead.
            if !span.is_desugaring(DesugaringKind::CondTemporary)
                && !span.is_desugaring(DesugaringKind::Async)
                && !orig_span.is_desugaring(DesugaringKind::Await)
            {
                self.diverges.set(Diverges::WarnedAlways);

                debug!("warn_if_unreachable: id={:?} span={:?} kind={}", id, span, kind);

                self.tcx().struct_span_lint_hir(lint::builtin::UNREACHABLE_CODE, id, span, |lint| {
                    let msg = format!("unreachable {}", kind);
                    lint.build(&msg)
                        .span_label(span, &msg)
                        .span_label(
                            orig_span,
                            custom_note
                                .unwrap_or("any code following this expression is unreachable"),
                        )
                        .emit();
                })
            }
        }
    }
```

Diverging code is how rust detects when a function returns early. It's their way of detecting dead code.
We can look for `diverges.set` to see where rust detects diverging code. 

We see that rust looks at the return type of each statement `!` and if one of them has a `never` return type, then diverge always.

```rust
        // Any expression that produces a value of type `!` must have diverged
        if ty.is_never() {
            self.diverges.set(self.diverges.get() | Diverges::always(expr.span));
        }
```

It appears that this function is used to get the 'return type' of an expression (even if it is only a compiler type)
compiler/rustc_typeck/src/check/expr.rs
```rust
    fn check_expr_kind(
        &self,
        expr: &'tcx hir::Expr<'tcx>,
        expected: Expectation<'tcx>,
    ) -> Ty<'tcx> {
```

Notice that rust does dead code checks in the `typecheck` pass. 

```rust
    // A generic function for checking the 'then' and 'else' clauses in an 'if'
    // or 'if-else' expression.
    fn check_then_else(
        &self,
        cond_expr: &'tcx hir::Expr<'tcx>,
        then_expr: &'tcx hir::Expr<'tcx>,
        opt_else_expr: Option<&'tcx hir::Expr<'tcx>>,
        sp: Span,
        orig_expected: Expectation<'tcx>,
    ) -> Ty<'tcx> {
        let cond_ty = self.check_expr_has_type_or_error(cond_expr, self.tcx.types.bool, |_| {});

        self.warn_if_unreachable(
            cond_expr.hir_id,
            then_expr.span,
            "block in `if` or `while` expression",
        );

        let cond_diverges = self.diverges.get();
        self.diverges.set(Diverges::Maybe);

        let expected = orig_expected.adjust_for_branches(self);
        let then_ty = self.check_expr_with_expectation(then_expr, expected);
        let then_diverges = self.diverges.get();
        self.diverges.set(Diverges::Maybe);

        // We've already taken the expected type's preferences
        // into account when typing the `then` branch. To figure
        // out the initial shot at a LUB, we thus only consider
        // `expected` if it represents a *hard* constraint
        // (`only_has_type`); otherwise, we just go with a
        // fresh type variable.
        let coerce_to_ty = expected.coercion_target_type(self, sp);
        let mut coerce: DynamicCoerceMany<'_> = CoerceMany::new(coerce_to_ty);

        coerce.coerce(self, &self.misc(sp), then_expr, then_ty);

        if let Some(else_expr) = opt_else_expr {
            let else_ty = if sp.desugaring_kind() == Some(DesugaringKind::LetElse) {
                // todo introduce `check_expr_with_expectation(.., Expectation::LetElse)`
                //   for errors that point to the offending expression rather than the entire block.
                //   We could use `check_expr_eq_type(.., tcx.types.never)`, but then there is no
                //   way to detect that the expected type originated from let-else and provide
                //   a customized error.
                let else_ty = self.check_expr(else_expr);
                let cause = self.cause(else_expr.span, ObligationCauseCode::LetElse);

                if let Some(mut err) =
                    self.demand_eqtype_with_origin(&cause, self.tcx.types.never, else_ty)
                {
                    err.emit();
                    self.tcx.ty_error()
                } else {
                    else_ty
                }
            } else {
                self.check_expr_with_expectation(else_expr, expected)
            };
            let else_diverges = self.diverges.get();

            let opt_suggest_box_span =
                self.opt_suggest_box_span(else_expr.span, else_ty, orig_expected);
            let if_cause =
                self.if_cause(sp, then_expr, else_expr, then_ty, else_ty, opt_suggest_box_span);

            coerce.coerce(self, &if_cause, else_expr, else_ty);

            // We won't diverge unless both branches do (or the condition does).
            self.diverges.set(cond_diverges | then_diverges & else_diverges);
        } else {
            self.if_fallback_coercion(sp, then_expr, &mut coerce);

            // If the condition is false we can't diverge.
            self.diverges.set(cond_diverges);
        }

        let result_ty = coerce.complete(self);
        if cond_ty.references_error() { self.tcx.ty_error() } else { result_ty }
    }
```

This function recursively checks each block in the if/else chain if they diverge. It's recursive because each block may, in turn, have further if else statements which must be checked for divergence. Rust stores the current checking divergence in a member var. This is confusing, don't do this.

For example, when checking if a return statement is valid, the type of the statement is `never`. As seen above, if return type is `never`, then the block diverges.


The `return` expression has return type never, but in type check, the expression following return is checked to have to expected return type.

```rust
// check_return_expr
    pub(super) fn check_return_expr(
        &self,
        return_expr: &'tcx hir::Expr<'tcx>,
        explicit_return: bool,
    ) {
        let ret_coercion = self.ret_coercion.as_ref().unwrap_or_else(|| {
            span_bug!(return_expr.span, "check_return_expr called outside fn body")
        });

        let ret_ty = ret_coercion.borrow().expected_ty();

        // HERE!
        let return_expr_ty = self.check_expr_with_hint(return_expr, ret_ty);


...

    pub(super) fn check_expr_with_hint(
        &self,
        expr: &'tcx hir::Expr<'tcx>,
        expected: Ty<'tcx>,
    ) -> Ty<'tcx> {
        self.check_expr_with_expectation(expr, ExpectHasType(expected))
    }
```

Inside the `check_return_expr` function, it calls coerce on the type with the expected type and `ObligationCauseCode::ReturnValue`. If the type can't be coerced, then it emits a error. `coerce_inner` is the function that does this check. `compiler/rustc_typeck/src/check/coercion.rs`.