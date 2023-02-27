# Design V3

I've been needing a more sane way to turn the ast into codegen. Right now I have IR, but that is just another tree structure, which is deeply nested and very confusing to follow.

I've thought for a while that I needed a way to flatten the structure of codegen. So my thought is to create a "VM" of sorts in which the Ast produces.

The idea is that the instructions for this machine can manipulate the state of the machine. Basically, making an interpreted language. This will help with const evaluation as well as flatten the generation.

## Notes

Zig binops are not 3 address. There are tree binops. I.e. The lhs is a binop and the rhs is a binop. 
 I don't want that. I want something more primitive where the lhs and the rhs can only be :VarRef or :Val
The problem now is that the IR needs temporary values.


:Val i32 %1 = :BinOp i32 :ConstInt 5, :ConstInt 6
:Val i32 %2 = :BinOp i32 :ConstInt 5, %1
:Val i32 %3 = :Call i32 printf_ret_char_count(i8* :StringLiteral "%d\n", %2)

I could then look for temporaries that are unused and not emit them or show warning.

Unlike LLVM this IR would understand higher constructs like %Point. (LLVM has to pass by SRet and all that.)

:Val %Point %my_point = :Alloca %Point
:Val %Point %1 = :Call %Point create_point(:ConstInt 5, :ConstInt 6)
:Store %1 %my_point


Function keeps a count of temporaries and increments number for name.


Some instructions "yield" other instructions. Usually the yielded instructions are used to refer to some result of the instruction.





## Reaction

So I tried linear IR for a bit. I'm not sure it's better. Here's how I think about it. I want the IR to be a full blown VM. 
The problem is that during semantic analysis, I need to be able to look up variables the same way the VM is going to look up variables... but this would mean that I need to emulate the scope rules of the VM.

I'm not sure. I think a tree-IR is actually easier, but now I have a better understanding.