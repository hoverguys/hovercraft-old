# Hovercraft #
---

## Building ##

### Installing Dependencies ###

#### OS X ####

The easiest way to install dependencies with OS X is using [Homebrew](http://brew.sh).

Install DevkitPPC: Instructions [here](http://wiibrew.org/wiki/Devkitppc_setup_(Mac_OS_X)).

Install Boost: `brew install boost`

Install ASSIMP: `brew install assimp`

### Compiling on POSIX systems (Linux, OS X) ###

From the project's root directory:

```
cd tools/obj2bin_src
make
cd ../..
make
```