# Design V3

I've been needing a more sane way to turn the ast into codegen. Right now I have IR, but that is just another tree structure, which is deeply nested and very confusing to follow.

I've thought for a while that I needed a way to flatten the structure of codegen. So my thought is to create a "VM" of sorts in which the Ast produces.

The idea is that the instructions for this machine can manipulate the state of the machine. Basically, making an interpreted language. This will help with const evaluation as well as flatten the generation.

