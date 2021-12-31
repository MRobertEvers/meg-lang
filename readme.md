# Meg Lang

# Exploration

Create make files with `cmake` (expects LLVM is available in usr/lib)

```
mkdir build
cd build
cmake ..
```

Now you can build the src with `make` which will produce a `MegLang` file.

Run `MegLang`, this will produce an `output.o` which is some test LLVM output. The `output.o` file contains a function definition for `extern "C" double floater()`, which can be called from a C function.

Compile `test.cpp` to call the LLVM generated code from C.

```
clang++ test.cpp output.o -o test
```

Then run the `test` file and see the output.

```
Value: 10.4
```


Putting it all together, it looks like

```
mkdir build
cd build
cmake ..

make

./MegLang

clang++ test.cpp output.o -o test

./test

>>> Value: 10.4
```