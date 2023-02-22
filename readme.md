# Sushi Lang

Sushi is my cat.

In the branch, I look at a more structured approach to semantic analysis, i.e. Type checking.


## Installation

Requires llvm libraries; follow the llvm website to set up llvm libs on your system.

Uses cmake. This doc uses `make` but you can use whatever build system cmake supports.



## Building

This build is similar to a previous example. We use clang to call into the code generated by LLVM.

Build uses cmake. 

```
mkdir build
cd build
cmake ..
# Use this for debug
cmake .. -DCMAKE_BUILD_TYPE=Debug
```

Once cmake has generated the build make files.

```
make
```

The output is `sushi`.

The `./sushi` executable takes a single argument, the file to compile. It produces an `output.o` file, which can be linked normally against C linkage. 

For example you can compile a compilable executable using gcc or clang. `gcc ./output.o`


You can use clang to bootstrap calling into our object code.

For example, given the following input for sushi,

```
struct my_nested {
    n: i32;
}

struct my_struct {
    a: i32;
    b: my_nested;
}

fn func(s: my_struct*): i32 {
    let a = s.a;
    return s.b.n + a;
}
```

The `output.o` file will contain a `func` symbol which can be called into using the following cpp code.

```cpp
#include <iostream>

struct my_nested
{
	int n;
};

struct my_struct
{
	int a;
	struct my_nested b;
};

extern "C" int func(my_struct const* s);

int
main()
{
	struct my_struct s;
	s.a = 1;
	s.b.n = 2;
	std::cout << func(&s) << std::endl;

	return 0;
}
```

In order to link and call the code

```
clang++ test.cpp output.o -o test

./test

>>> 3
```

## Tests

Tests are run using Nodejs and jest. This is temporary until I get more familiar with testing in cpp and sushi is able to generate a main function.

In the `test` folder run `npm install`. Then you can run tests using `npm run test`.

In order to run tests, you have to manually compile the sushi binary in `build`. See build instructions.

## Debugging With VSCode

You can debug your sushi binary with lldb. Be sure to change the `args` field to the file you want to debug.

```
{
    // Use IntelliSense to learn about possible attributes.
    // Hover to view descriptions of existing attributes.
    // For more information, visit: https://go.microsoft.com/fwlink/?linkid=830387
    "version": "0.2.0",
    "configurations": [
        {
            "type": "lldb",
            "request": "launch",
            "name": "Launch",
            "program": "${workspaceFolder}/build/sushi",
            "cwd": "${workspaceFolder}",
            "args": ["main.sushi"],
        },
        
    ]
}
```

## TODO

1. Move type checking out of parser
2. Pretty print from AST
3. Code generation only on HIR
4. Add debug information
5. Add syntax highlighting in vscode
6. Compiler explorer addon

## Notes

LLVM Obj dump 

```
llvm-objdump -D ./a.out
```

## Discussionm

Additionally, I spent some time thinking about how to handle type names (as opposed to names of variables and functions).

I did a quick check on clang and it seems like they track type names and value names. They have a `TypeDecl` and a `ValueDecl` class that inherit from `NamedDecl`. I determined that they are stored in the same structure when exporing scope names.

E.g.

```cpp
struct Wow
{
	int x;
};

int
main()
{

	auto v = Wow;

	return 1;
}
```

The above code generates the following error
```
no.cpp:24:11: error: 'Wow' does not refer to a value
        auto v = Wow;
```


I looked at passing structs by value, but it appears that llvm doesn't have native support for that. I looked at how clang passes structs by value. Basically, it instead just passes an array of integers that is big enough to fit the struct and then performs some llvm type casting.

The following c code produces the llvm ir below. You can use clang to emit llvm ir, `clang -S -emit-llvm test_c.c`.

```c
struct my_struct
{
	int a;
	int b;
	int c;
};

int
func(struct my_struct s)
{
	return s.b;
}

int
call_func()
{
	struct my_struct s;
	s.a = 0;
	s.b = 3;
	s.c = 6;
	return func(s);
}
```

```llvm-ir
; ModuleID = 'test_c.c'
source_filename = "test_c.c"
target datalayout = "e-m:o-i64:64-i128:128-n32:64-S128"
target triple = "arm64-apple-macosx12.0.0"

%struct.my_struct = type { i32, i32, i32 }

; Function Attrs: noinline nounwind optnone ssp uwtable
define i32 @func([2 x i64] %0) #0 {
  %2 = alloca %struct.my_struct, align 4
  %3 = alloca [2 x i64], align 8
  store [2 x i64] %0, [2 x i64]* %3, align 8
  %4 = bitcast %struct.my_struct* %2 to i8*
  %5 = bitcast [2 x i64]* %3 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %4, i8* align 8 %5, i64 12, i1 false)
  %6 = getelementptr inbounds %struct.my_struct, %struct.my_struct* %2, i32 0, i32 1
  %7 = load i32, i32* %6, align 4
  ret i32 %7
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #1

; Function Attrs: noinline nounwind optnone ssp uwtable
define i32 @call_func() #0 {
  %1 = alloca %struct.my_struct, align 4
  %2 = alloca [2 x i64], align 8
  %3 = getelementptr inbounds %struct.my_struct, %struct.my_struct* %1, i32 0, i32 0
  store i32 0, i32* %3, align 4
  %4 = getelementptr inbounds %struct.my_struct, %struct.my_struct* %1, i32 0, i32 1
  store i32 3, i32* %4, align 4
  %5 = getelementptr inbounds %struct.my_struct, %struct.my_struct* %1, i32 0, i32 2
  store i32 6, i32* %5, align 4
  %6 = bitcast [2 x i64]* %2 to i8*
  %7 = bitcast %struct.my_struct* %1 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %6, i8* align 4 %7, i64 12, i1 false)
  %8 = load [2 x i64], [2 x i64]* %2, align 8
  %9 = call i32 @func([2 x i64] %8)
  ret i32 %9
}

attributes #0 = { noinline nounwind optnone ssp uwtable "disable-tail-calls"="false" "frame-pointer"="non-leaf" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "probe-stack"="__chkstk_darwin" "stack-protector-buffer-size"="8" "target-cpu"="apple-m1" "target-features"="+aes,+crc,+crypto,+dotprod,+fp-armv8,+fp16fml,+fullfp16,+lse,+neon,+ras,+rcpc,+rdm,+sha2,+sha3,+sm4,+v8.5a,+zcm,+zcz" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }

!llvm.module.flags = !{!0, !1, !2, !3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = !{i32 2, !"SDK Version", [2 x i32] [i32 12, i32 0]}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 1, !"branch-target-enforcement", i32 0}
!3 = !{i32 1, !"sign-return-address", i32 0}
!4 = !{i32 1, !"sign-return-address-all", i32 0}
!5 = !{i32 1, !"sign-return-address-with-bkey", i32 0}
!6 = !{i32 7, !"PIC Level", i32 2}
!7 = !{!"Apple clang version 13.0.0 (clang-1300.0.29.3)"}
```

## Notes - Calling with structs (pointers)

Passing structs by pointers is straight forward, llvm ir that clang produces looks like this;

```c
struct my_struct
{
	int a;
	int b;
	int c;
};

int
func(struct my_struct* s)
{
	return s->b;
}

int
call_func()
{
	struct my_struct s;
	s.a = 0;
	s.b = 3;
	s.c = 6;
	return func(&s);
}
```

```llvm-ir
; Function Attrs: noinline nounwind optnone ssp uwtable
define i32 @func(%struct.my_struct* %0) #0 {
  %2 = alloca %struct.my_struct*, align 8
  store %struct.my_struct* %0, %struct.my_struct** %2, align 8
  %3 = load %struct.my_struct*, %struct.my_struct** %2, align 8
  %4 = getelementptr inbounds %struct.my_struct, %struct.my_struct* %3, i32 0, i32 1
  %5 = load i32, i32* %4, align 4
  ret i32 %5
}

; Function Attrs: noinline nounwind optnone ssp uwtable
define i32 @call_func() #0 {
  %1 = alloca %struct.my_struct, align 4
  %2 = getelementptr inbounds %struct.my_struct, %struct.my_struct* %1, i32 0, i32 0
  store i32 0, i32* %2, align 4
  %3 = getelementptr inbounds %struct.my_struct, %struct.my_struct* %1, i32 0, i32 1
  store i32 3, i32* %3, align 4
  %4 = getelementptr inbounds %struct.my_struct, %struct.my_struct* %1, i32 0, i32 2
  store i32 6, i32* %4, align 4
  %5 = call i32 @func(%struct.my_struct* %1)
  ret i32 %5
}
```

The llvm ir that is produced by the current sushi lang program is

```
struct my_struct {
    a: i32;
    b: i32;
}

fn func(s: my_struct*): i32 {
    let x: i32 = s.a; 
    return s.b; 
}
```

```
%my_struct = type { i32, i32 }

define i32 @func(%my_struct* %s) {
entry:
  %s1 = alloca %my_struct*, align 8
  store %my_struct* %s, %my_struct** %s1, align 8
  %x = alloca i32, align 4
  %Deref = load %my_struct*, %my_struct** %s1, align 8
  %my_struct.a = getelementptr inbounds %my_struct, %my_struct* %Deref, i32 0, i32 0
  %0 = load i32, i32* %my_struct.a, align 4
  store i32 %0, i32* %x, align 4
  %Deref2 = load %my_struct*, %my_struct** %s1, align 8
  %my_struct.b = getelementptr inbounds %my_struct, %my_struct* %Deref2, i32 0, i32 1
  %1 = load i32, i32* %my_struct.b, align 4
  ret i32 %1
}
```

# Notes - Pass by Reference

Calling by reference is identical to call by pointer

```
struct my_struct
{
	int a;
	int b;
	int c;
};

int
func(struct my_struct& s)
{
	return s.b;
}

int
call_func()
{
	struct my_struct s;
	s.a = 0;
	s.b = 3;
	s.c = 6;
	return func(s);
}
```

```
; Function Attrs: noinline nounwind optnone ssp uwtable
define i32 @_Z4funcR9my_struct(%struct.my_struct* nonnull align 4 dereferenceable(12) %0) #0 {
  %2 = alloca %struct.my_struct*, align 8
  store %struct.my_struct* %0, %struct.my_struct** %2, align 8
  %3 = load %struct.my_struct*, %struct.my_struct** %2, align 8
  %4 = getelementptr inbounds %struct.my_struct, %struct.my_struct* %3, i32 0, i32 1
  %5 = load i32, i32* %4, align 4
  ret i32 %5
}

; Function Attrs: noinline nounwind optnone ssp uwtable
define i32 @_Z9call_funcv() #0 {
  %1 = alloca %struct.my_struct, align 4
  %2 = getelementptr inbounds %struct.my_struct, %struct.my_struct* %1, i32 0, i32 0
  store i32 0, i32* %2, align 4
  %3 = getelementptr inbounds %struct.my_struct, %struct.my_struct* %1, i32 0, i32 1
  store i32 3, i32* %3, align 4
  %4 = getelementptr inbounds %struct.my_struct, %struct.my_struct* %1, i32 0, i32 2
  store i32 6, i32* %4, align 4
  %5 = call i32 @_Z4funcR9my_struct(%struct.my_struct* nonnull align 4 dereferenceable(12) %1)
  ret i32 %5
}
```

## Notes - GEP

GEP takes a PointerToTy as it's Ptr value.

I.e.

```
llvm::Type* PointerToTy = ...; // Must be a pointer type

```


## Notes - Deref on Struct Nested

```c
struct Nested
{
	int n;
};

struct T
{
	int a;
	struct Nested b;
};

int
my_func(struct T* my_t)
{
	int a = my_t->a;
	return my_t->b.n + a;
}
```

```
; Function Attrs: noinline nounwind optnone ssp uwtable
define i32 @my_func(%struct.T* %0) #0 {
  %2 = alloca %struct.T*, align 8
  %3 = alloca i32, align 4
  store %struct.T* %0, %struct.T** %2, align 8
  %4 = load %struct.T*, %struct.T** %2, align 8
  %5 = getelementptr inbounds %struct.T, %struct.T* %4, i32 0, i32 0
  %6 = load i32, i32* %5, align 4
  store i32 %6, i32* %3, align 4
  %7 = load %struct.T*, %struct.T** %2, align 8
  %8 = getelementptr inbounds %struct.T, %struct.T* %7, i32 0, i32 1
  %9 = getelementptr inbounds %struct.Nested, %struct.Nested* %8, i32 0, i32 0
  %10 = load i32, i32* %9, align 4
  %11 = load i32, i32* %3, align 4
  %12 = add nsw i32 %10, %11
  ret i32 %12
}
```

## Exploration 7 



The last several branches were an exploration of how to appropriately generate llvm-ir. What I'm realizing is that it might be best to specify some language features a bit better so I can organize the code and data in a way that makes more sense.

Postfix expressions. Identifiers can have several postfix operations performed on them.

```
postfix_expr := simple_expr postfix_expr_tail

postfix_expr_tail := "[" expr "]"
                    | "(" func-args ")"
                    | "." identifier
                    | "->" identifier
                    | "++"
                    | "--"
```

In this branch, only '.' and function call is implemented. 

Additionally handling parenthized expressions was added.

```
expr         := postfix_expr
              | postfix_expr binary_operator postfix_expr

postfix_expr := simple_expr
              | simple_expr postfix_expr_tail

simple_expr  := "(" expr ")"
              | identifier
              | literal
```

Example working code.

```
struct my_nested {
    n: i32;
}

struct my_struct {
    a: i32;
    b: my_nested;
}

fn func(s: my_struct*): i32 {
    let a = s.a;
    return (s.b.n + (a) * 5);
}
```

I additionally took a look at how to handle LValues. For example, we don't want `5=3` to make it to codegen.

`chibbic` captures errors like this at codegen. `clang` tracks LValueness during compilation (although I didn't dig too deep).

Generally, something is an lvalue if all operations on it return lvalues. 

```
let x = 5;
x+6 = 4; // x+6 is not an lvalue.
```

It depends on the operation defined between the two operands. Let `&` denote an LValue and `&&` denote an LRValue, that is an LRValue can be either an L value or an RValue, but only an LValue can be assigned. This is ad-hoc pseudo code which I am making up on the spot.

```
define '+' (l&& int, r&& int): int&&
define '++' (l&& int): l& if l is l& else l&&
```

LValueness is a property of the expression, not the type. In addition to types, each expression also has a LValueness or RValueness. Update: Microsoft C++ help articles agree with my assessment.

> Every C++ expression has a type, and belongs to a value category. The value categories are the basis for rules that compilers > must follow when creating, copying, and moving temporary objects during expression evaluation.
> 
> The C++17 standard defines expression value categories as follows:
> 
> A glvalue is an expression whose evaluation determines the identity of an object, bit-field, or function.
> A prvalue is an expression whose evaluation initializes an object or a bit-field, or computes the value of the operand of an > operator, as specified by the context in which it appears.
> An xvalue is a glvalue that denotes an object or bit-field whose resources can be reused (usually because it is near the end of > its lifetime). Example: Certain kinds of expressions involving rvalue references (8.3.2) yield xvalues, such as a call to a > function whose return type is an rvalue reference or a cast to an rvalue reference type.
> An lvalue is a glvalue that is not an xvalue.
> An rvalue is a prvalue or an xvalue.




For now lets ignore this.

In this branch, function call expressions and while loops are added.

# Exploration 9

In this branch I did some more exploration of how pretty printing works. I spent some time thinking about it and then I took a look at prettier (the js pretty printer). One problem I was having was how to handle comments, and generally things that don't appear in the AST. 

I looked at Prettier (the JS formatter) and found out that they use a published algorithm.

Prettier linked this paper, `https://homepages.inf.ed.ac.uk/wadler/papers/prettier/prettier.pdf`, which describes the implementation of a pretty printing algorithm and how the formatting ast can be constructed.

In general, prettier is fairly well documented, https://github.com/prettier/prettier/blob/main/commands.md.

The pretty printer in this branch is implemented in a similar manner.