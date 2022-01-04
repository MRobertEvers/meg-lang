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

This function recursively checks each block in the if/else chain if they diverge. It's recursive because each block may, in turn, have further if else statements which must be checked for divergence. Rust stores the current checking divergence in a member var. This is confusing, don't do this. For each statement in a block, it stores whether we have already diverged, and if so, warns on following statements.

Each statement is typechecked in order in `check_block_with_expected`.

```rust
            for (pos, s) in blk.stmts.iter().enumerate() {
                self.check_stmt(s, blk.stmts.len() - 1 == pos);
            }
```

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


### Rust Parsing

Rust considers all Expressions as Statements, but not the other way around.

During parsing a block, it iterates over each statement in a block (which may be an expression).

```rust
    crate fn parse_block_tail(
        &mut self,
        lo: Span,
        s: BlockCheckMode,
        recover: AttemptLocalParseRecovery,
    ) -> PResult<'a, P<Block>> {
        let mut stmts = vec![];
        while !self.eat(&token::CloseDelim(token::Brace)) {
            if self.token == token::Eof {
                break;
            }
            let stmt = match self.parse_full_stmt(recover) {
                Err(mut err) if recover.yes() => {
                    self.maybe_annotate_with_ascription(&mut err, false);
                    err.emit();
                    self.recover_stmt_(SemiColonMode::Ignore, BlockMode::Ignore);
                    Some(self.mk_stmt_err(self.token.span))
                }
                Ok(stmt) => stmt,
                Err(err) => return Err(err),
            };
            if let Some(stmt) = stmt {
                stmts.push(stmt);
            } else {
                // Found only `;` or `}`.
                continue;
            };
        }
        Ok(self.mk_block(stmts, s, lo.to(self.prev_token.span)))
    }
```

## Call Stack When Debugging Compiler

I ran the rust compiler in debug mode on the following file

```rust
fn square(num: i32) -> i32 {
    5 * {
        let x = 4; 4*x
          //^----> call stack taken from when we were parsing here.
    }
}

pub fn main() {
    square(4);
}
```

```
parse_assoc_expr_with (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/expr.rs:161)
<rustc_parse::parser::Parser>::parse_assoc_expr (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/expr.rs:152)
<rustc_parse::parser::Parser>::parse_expr_res::{closure#0} (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/expr.rs:140)
<rustc_parse::parser::Parser>::with_res::<core::result::Result<rustc_ast::ptr::P<rustc_ast::ast::Expr>, rustc_errors::diagnostic_builder::DiagnosticBuilder>, <rustc_parse::parser::Parser>::parse_expr_res::{closure#0}> (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/mod.rs:1254)
<rustc_parse::parser::Parser>::parse_expr_res (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/expr.rs:140)
<rustc_parse::parser::Parser>::parse_expr (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/expr.rs:99)
<rustc_parse::parser::Parser>::parse_initializer (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/stmt.rs:395)
parse_local (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/stmt.rs:268)
<rustc_parse::parser::Parser>::parse_local_mk::{closure#0} (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/stmt.rs:216)
<rustc_parse::parser::Parser>::collect_tokens_trailing_token::<rustc_ast::ast::Stmt, <rustc_parse::parser::Parser>::parse_local_mk::{closure#0}> (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/attr_wrapper.rs:229)
<rustc_parse::parser::Parser>::parse_local_mk (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/stmt.rs:214)
parse_stmt_without_recovery (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/stmt.rs:63)
parse_full_stmt (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/stmt.rs:527)
parse_block_tail (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/stmt.rs:499)
parse_block_common (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/stmt.rs:481)
parse_block_expr (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/expr.rs:1880)
<rustc_parse::parser::Parser>::parse_bottom_expr (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/expr.rs:1245)
<rustc_parse::parser::Parser>::parse_dot_or_call_expr::{closure#0} (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/expr.rs:895)
{closure#0}<rustc_parse::parser::expr::{impl#2}::parse_dot_or_call_expr::{closure#0}> (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/expr.rs:2904)
<rustc_parse::parser::Parser>::collect_tokens_trailing_token::<rustc_ast::ptr::P<rustc_ast::ast::Expr>, <rustc_parse::parser::Parser>::collect_tokens_for_expr<<rustc_parse::parser::Parser>::parse_dot_or_call_expr::{closure#0}>::{closure#0}> (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/attr_wrapper.rs:219)
<rustc_parse::parser::Parser>::collect_tokens_for_expr::<<rustc_parse::parser::Parser>::parse_dot_or_call_expr::{closure#0}> (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/expr.rs:2903)
<rustc_parse::parser::Parser>::parse_dot_or_call_expr (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/expr.rs:894)
parse_prefix_expr (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/expr.rs:582)
parse_assoc_expr_with (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/expr.rs:171)
<rustc_parse::parser::Parser>::parse_assoc_expr_with::{closure#0} (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/expr.rs:294)
<rustc_parse::parser::Parser>::with_res::<core::result::Result<rustc_ast::ptr::P<rustc_ast::ast::Expr>, rustc_errors::diagnostic_builder::DiagnosticBuilder>, <rustc_parse::parser::Parser>::parse_assoc_expr_with::{closure#0}> (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/mod.rs:1254)
parse_assoc_expr_with (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/expr.rs:293)
<rustc_parse::parser::Parser>::parse_assoc_expr (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/expr.rs:152)
<rustc_parse::parser::Parser>::parse_expr_res::{closure#0} (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/expr.rs:140)
<rustc_parse::parser::Parser>::with_res::<core::result::Result<rustc_ast::ptr::P<rustc_ast::ast::Expr>, rustc_errors::diagnostic_builder::DiagnosticBuilder>, <rustc_parse::parser::Parser>::parse_expr_res::{closure#0}> (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/mod.rs:1254)
<rustc_parse::parser::Parser>::parse_expr_res (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/expr.rs:140)
parse_stmt_without_recovery (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/stmt.rs:104)
parse_full_stmt (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/stmt.rs:527)
parse_block_tail (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/stmt.rs:499)
parse_block_common (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/stmt.rs:481)
<rustc_parse::parser::Parser>::parse_inner_attrs_and_block (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/stmt.rs:462)
parse_fn_body (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/item.rs:1840)
parse_fn (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/item.rs:1813)
<rustc_parse::parser::Parser>::parse_item_kind (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/item.rs:226)
<rustc_parse::parser::Parser>::parse_item_common_ (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/item.rs:139)
<rustc_parse::parser::Parser>::parse_item_common::{closure#0} (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/item.rs:119)
<rustc_parse::parser::Parser>::collect_tokens_trailing_token::<core::option::Option<rustc_ast::ast::Item>, <rustc_parse::parser::Parser>::parse_item_common::{closure#0}> (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/attr_wrapper.rs:229)
parse_item_common (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/item.rs:117)
<rustc_parse::parser::Parser>::parse_item_ (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/item.rs:92)
<rustc_parse::parser::Parser>::parse_item (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/item.rs:83)
parse_mod (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/item.rs:59)
<rustc_parse::parser::Parser>::parse_crate_mod (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/parser/item.rs:29)
parse_crate_from_file (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_parse/src/lib.rs:61)
rustc_interface::passes::parse::{closure#0} (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_interface/src/passes.rs:57)
<rustc_data_structures::profiling::VerboseTimingGuard>::run::<core::result::Result<rustc_ast::ast::Crate, rustc_errors::diagnostic_builder::DiagnosticBuilder>, rustc_interface::passes::parse::{closure#0}> (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_data_structures/src/profiling.rs:644)
time<core::result::Result<rustc_ast::ast::Crate, rustc_errors::diagnostic_builder::DiagnosticBuilder>, rustc_interface::passes::parse::{closure#0}> (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_session/src/utils.rs:16)
parse (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_interface/src/passes.rs:56)
<rustc_interface::queries::Queries>::parse::{closure#0} (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_interface/src/queries.rs:125)
<rustc_interface::queries::Query<rustc_ast::ast::Crate>>::compute::<<rustc_interface::queries::Queries>::parse::{closure#0}> (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_interface/src/queries.rs:38)
parse (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_interface/src/queries.rs:124)
rustc_driver::run_compiler::{closure#1}::{closure#2} (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_driver/src/lib.rs:316)
enter<rustc_driver::run_compiler::{closure#1}::{closure#2}, core::result::Result<core::option::Option<rustc_interface::queries::Linker>, rustc_errors::ErrorReported>> (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_interface/src/queries.rs:393)
rustc_driver::run_compiler::{closure#1} (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_driver/src/lib.rs:314)
rustc_interface::interface::create_compiler_and_run::<core::result::Result<(), rustc_errors::ErrorReported>, rustc_driver::run_compiler::{closure#1}>::{closure#1} (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_interface/src/interface.rs:220)
with_source_map<core::result::Result<(), rustc_errors::ErrorReported>, rustc_interface::interface::create_compiler_and_run::{closure#1}> (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_span/src/lib.rs:1010)
rustc_interface::interface::create_compiler_and_run::<core::result::Result<(), rustc_errors::ErrorReported>, rustc_driver::run_compiler::{closure#1}> (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_interface/src/interface.rs:214)
rustc_interface::interface::run_compiler::<core::result::Result<(), rustc_errors::ErrorReported>, rustc_driver::run_compiler::{closure#1}>::{closure#0} (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_interface/src/interface.rs:236)
rustc_interface::util::setup_callbacks_and_run_in_thread_pool_with_globals::<rustc_interface::interface::run_compiler<core::result::Result<(), rustc_errors::ErrorReported>, rustc_driver::run_compiler::{closure#1}>::{closure#0}, core::result::Result<(), rustc_errors::ErrorReported>>::{closure#0}::{closure#0} (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_interface/src/util.rs:148)
<scoped_tls::ScopedKey<rustc_span::SessionGlobals>>::set::<rustc_interface::util::setup_callbacks_and_run_in_thread_pool_with_globals<rustc_interface::interface::run_compiler<core::result::Result<(), rustc_errors::ErrorReported>, rustc_driver::run_compiler::{closure#1}>::{closure#0}, core::result::Result<(), rustc_errors::ErrorReported>>::{closure#0}::{closure#0}, core::result::Result<(), rustc_errors::ErrorReported>> (/Users/matthewevers/.cargo/registry/src/github.com-1ecc6299db9ec823/scoped-tls-1.0.0/src/lib.rs:137)
rustc_span::create_session_globals_then::<core::result::Result<(), rustc_errors::ErrorReported>, rustc_interface::util::setup_callbacks_and_run_in_thread_pool_with_globals<rustc_interface::interface::run_compiler<core::result::Result<(), rustc_errors::ErrorReported>, rustc_driver::run_compiler::{closure#1}>::{closure#0}, core::result::Result<(), rustc_errors::ErrorReported>>::{closure#0}::{closure#0}> (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_span/src/lib.rs:108)
rustc_interface::util::setup_callbacks_and_run_in_thread_pool_with_globals::<rustc_interface::interface::run_compiler<core::result::Result<(), rustc_errors::ErrorReported>, rustc_driver::run_compiler::{closure#1}>::{closure#0}, core::result::Result<(), rustc_errors::ErrorReported>>::{closure#0} (/Users/matthewevers/Documents/git_repos/rust/compiler/rustc_interface/src/util.rs:146)
__rust_begin_short_backtrace<rustc_interface::util::setup_callbacks_and_run_in_thread_pool_with_globals::{closure#0}, core::result::Result<(), rustc_errors::ErrorReported>> (/Users/matthewevers/Documents/git_repos/rust/library/std/src/sys_common/backtrace.rs:123)
<std::thread::Builder>::spawn_unchecked::<rustc_interface::util::setup_callbacks_and_run_in_thread_pool_with_globals<rustc_interface::interface::run_compiler<core::result::Result<(), rustc_errors::ErrorReported>, rustc_driver::run_compiler::{closure#1}>::{closure#0}, core::result::Result<(), rustc_errors::ErrorReported>>::{closure#0}, core::result::Result<(), rustc_errors::ErrorReported>>::{closure#1}::{closure#0} (/Users/matthewevers/Documents/git_repos/rust/library/std/src/thread/mod.rs:477)
<core::panic::unwind_safe::AssertUnwindSafe<<std::thread::Builder>::spawn_unchecked<rustc_interface::util::setup_callbacks_and_run_in_thread_pool_with_globals<rustc_interface::interface::run_compiler<core::result::Result<(), rustc_errors::ErrorReported>, rustc_driver::run_compiler::{closure#1}>::{closure#0}, core::result::Result<(), rustc_errors::ErrorReported>>::{closure#0}, core::result::Result<(), rustc_errors::ErrorReported>>::{closure#1}::{closure#0}> as core::ops::function::FnOnce<()>>::call_once (/Users/matthewevers/Documents/git_repos/rust/library/core/src/panic/unwind_safe.rs:271)
std::panicking::try::do_call::<core::panic::unwind_safe::AssertUnwindSafe<<std::thread::Builder>::spawn_unchecked<rustc_interface::util::setup_callbacks_and_run_in_thread_pool_with_globals<rustc_interface::interface::run_compiler<core::result::Result<(), rustc_errors::ErrorReported>, rustc_driver::run_compiler::{closure#1}>::{closure#0}, core::result::Result<(), rustc_errors::ErrorReported>>::{closure#0}, core::result::Result<(), rustc_errors::ErrorReported>>::{closure#1}::{closure#0}>, core::result::Result<(), rustc_errors::ErrorReported>> (/Users/matthewevers/Documents/git_repos/rust/library/std/src/panicking.rs:406)
std::panicking::try::<core::result::Result<(), rustc_errors::ErrorReported>, core::panic::unwind_safe::AssertUnwindSafe<<std::thread::Builder>::spawn_unchecked<rustc_interface::util::setup_callbacks_and_run_in_thread_pool_with_globals<rustc_interface::interface::run_compiler<core::result::Result<(), rustc_errors::ErrorReported>, rustc_driver::run_compiler::{closure#1}>::{closure#0}, core::result::Result<(), rustc_errors::ErrorReported>>::{closure#0}, core::result::Result<(), rustc_errors::ErrorReported>>::{closure#1}::{closure#0}>> (/Users/matthewevers/Documents/git_repos/rust/library/std/src/panicking.rs:370)
std::panic::catch_unwind::<core::panic::unwind_safe::AssertUnwindSafe<<std::thread::Builder>::spawn_unchecked<rustc_interface::util::setup_callbacks_and_run_in_thread_pool_with_globals<rustc_interface::interface::run_compiler<core::result::Result<(), rustc_errors::ErrorReported>, rustc_driver::run_compiler::{closure#1}>::{closure#0}, core::result::Result<(), rustc_errors::ErrorReported>>::{closure#0}, core::result::Result<(), rustc_errors::ErrorReported>>::{closure#1}::{closure#0}>, core::result::Result<(), rustc_errors::ErrorReported>> (/Users/matthewevers/Documents/git_repos/rust/library/std/src/panic.rs:133)
<std::thread::Builder>::spawn_unchecked::<rustc_interface::util::setup_callbacks_and_run_in_thread_pool_with_globals<rustc_interface::interface::run_compiler<core::result::Result<(), rustc_errors::ErrorReported>, rustc_driver::run_compiler::{closure#1}>::{closure#0}, core::result::Result<(), rustc_errors::ErrorReported>>::{closure#0}, core::result::Result<(), rustc_errors::ErrorReported>>::{closure#1} (/Users/matthewevers/Documents/git_repos/rust/library/std/src/thread/mod.rs:476)
call_once<std::thread::{impl#0}::spawn_unchecked::{closure#1}, ()> (/Users/matthewevers/Documents/git_repos/rust/library/core/src/ops/function.rs:227)
_$LT$alloc..boxed..Box$LT$F$C$A$GT$$u20$as$u20$core..ops..function..FnOnce$LT$Args$GT$$GT$::call_once (/Users/matthewevers/Documents/git_repos/rust/library/alloc/src/boxed.rs:1811)
_$LT$alloc..boxed..Box$LT$F$C$A$GT$$u20$as$u20$core..ops..function..FnOnce$LT$Args$GT$$GT$::call_once (/Users/matthewevers/Documents/git_repos/rust/library/alloc/src/boxed.rs:1811)
thread_start (/Users/matthewevers/Documents/git_repos/rust/library/std/src/sys/unix/thread.rs:108)
_pthread_start (@_pthread_start:40)
```

## Rust Module Codegen

Rust gathers all the module 'items' and codegens them iteratively, compiler/rustc_codegen_llvm/src/base.rs

```rust
            // ... and now that we have everything pre-defined, fill out those definitions.
            for &(mono_item, _) in &mono_items {
                mono_item.define::<Builder<'_, '_, '_>>(&cx);
            }

```