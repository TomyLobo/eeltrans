# How to build?
Just run "make" in the "AVSTrans_cpp" subdirectory.

In theory, you should be able to compile and run this on any POSIX-compliant system with GCC installed, including Cygwin/Babun on Windows.
With some minor effort, it should run on Visual Studio, too.

# How to run?
Syntax:

    ./avstrans-cli < eeltrans-code.txt > avs-code.txt

# Linux binary
I also made a travis build for it, so there's a Linux release binary, too:
https://github.com/TomyLobo/eeltrans/releases

As Binary releases for Linux go, this might not work on all Linux versions.
I tested it on my WSL (the windows bash thing) which is xenial.
The Travis worker ran on trusty, so that should work too.
