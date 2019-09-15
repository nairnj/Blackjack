To compile Blackjack use the following commands

1. cd to directory with this 'makefile'
2 give the command 'make'

Other 'make' options are:

  make clean - to remove all compiled objects
  make mostlyclear - to remove .o files, but leave executable output
  make install - to copy compiled code to desired installation destination

The following variables let you alter the make process without editing the makefile

1. CC - the C++ compiler (default is c++)
2. CFLAGS - options for c++ compiler (default is '-c -O3 -std=c++11')
3. LFLAGS - options to linking step (default is empty)
4. output - path to save executable (default is ../calculations/Blackjack)
5. ioutput - path to install executable (dafault is ~/bin/Blackjack)
6. SYSTEM - customize the make file for specific systems

Current SYSTEM options are

1. default - no customizations
2. mac-clang - compile using clamg-mp

For example, to make in insteall on your desktop, use

  make output=~/Desktop/Blackjack
