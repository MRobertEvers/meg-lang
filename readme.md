# Sushi Lang

Sushi is my cat.

In this branch, we are sidestepping a bit. The previous implementation had a pretty tight coupling between the AST and codegen. This branch, we explore some refactoring. We can traverse the tree with `IAstVisitor` to 'pretty' print as a proof of concept.

For example, the following input:
```
fn func(): i8 { return 9*8; } fn main(): i8 { return 12*1*3+4; }
```

produces this output here.

```
fn func()
{
return 9*8;
}
fn main()
{
return 12*1*3+4;
}
```

# Building

Build uses cmake. 

```
mkdir build
cd build
cmake ..
```

Once cmake has generated the build make files.

```
make
```

The output is `sushi`.