#How to build bzip2 for the board

##clone this repo
1. git clone https://github.com/Sisyph/embRando.git
##build gold for your build system
1. cd gold
2. mkdir build
3. cd build
4. sudo apt-get install texinfo
5. ../configure --enable-gold --enable-plugins --disable-werror
6. make all-gold -j

##llvm
1. cd llvm

##
1. git clone
2.
