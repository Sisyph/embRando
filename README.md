#How to build bzip2 for the board

##clone this repo
1. git clone https://github.com/Sisyph/embRando.git

##build gold for your build system
1. cd gold
2. mkdir build
3. cd build
4. sudo apt-get install texinfo
5. ../configure --enable-gold --enable-plugins --disable-werror
..* may have to pass CC and CCX
6. make all-gold -j

##musl
1. idk fix this, expecially targets, and

##llvm
1. cd llvm
2. mkdir build
3. cd build

##
1.
2.
