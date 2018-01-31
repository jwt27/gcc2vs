# gcc2vs
A simple program to convert error messages from gcc to a Visual Studio compatible format.

## Installing
From an MSYS2 shell, type:

    git clone https://github.com/jwt27/gcc2vs.git
    cd gcc2vs/
    make install
    
## Using
Set up a makefile project in Visual Studio, which calls `make` from within an MSYS2 shell. In your makefile, pipe all output from `gcc` through `gcc2vs`:
  
      %.o: %.cpp
          gcc -o $@ $^ 2>&1 | gcc2vs
          
You can also pipe the entire `make` command through `gcc2vs`. In that case you must invoke `gcc` with absolute paths to your source files.

