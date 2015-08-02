# Hovercraft #

## Building ##

### Installing Dependencies ###

Install DevkitPPC: Instructions [here](http://devkitpro.org/wiki/Getting_Started/devkitPPC).

#### Linux ####

For the purposes of this guide, we will assume you are using a debian-based distro with `apt-get`.

Install Boost Program Options: `sudo apt-get install libboost-program-options-dev`

Install ASSIMP: `sudo apt-get install libassimp-dev`

#### OS X ####

The easiest way to install dependencies with OS X is using [Homebrew](http://brew.sh). We will assume this is what you are using.

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
