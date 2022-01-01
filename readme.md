# Meg Lang

In this branch, we are officially compiling input source code.

Module expects single top level `fn` definition. Functions can have a single return statement.

```
fn func(): i8 { 
    return 9*8; 
}
```


# Resources

The Rust Parser entry point is in `compiler/rustc_parse/src/parser/item.rs` with function `parse_mod`.

`parse_item_kind` function appears close to the top of the parse and appears to parse all top level structures of a module.
