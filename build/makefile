# -------------------------------------------------------------------------
# To compile Blackjack use the following commands
#
# cd to directory with this 'makefile'
# give the command 'make'
# Other options are
#    make clean - to remove all compiled objects
#    make mostlyclear - to remove .o files, but leave executable output
#    make install - to copy compiled code to desired installation destination
#
# If you need other options, you can edit the makefile or override them in the make command
# The following are the most important variables
#
# 1. $(CC) is the gcc compiler, $(RM) is the rm command, $(CP) is copy for install
# 2. $(CFLAGS) is options for c++ compiler
# 4. $(LFLAGS) is options to linking step
# 3. $(output) is path to save executable
# 4. $(ioutput) is path to install executable
#
# Copyright (c) 2008 John A. Nairn, All rights reserved.
# see http://www.geditcom.com/Blackjack for documentation

# 1. Define commands and compiler to use
RM = rm
CP = cp
SYSTEM = mac-clabg
CC = c++
SYSTEM = default
LFLAGS =

# Custom SYSTEM settings
ifeq ($(SYSTEM),mac-clang)
    CC = /opt/local/bin/clang-mp-7.0
	LFLAGS = -lc++
endif

# 2. Compiler options -c=compile only, -O3=optimize most -O0=not optimized -g=debugging info
CFLAGS = -c -O3 -std=c++11

# 3. Define executable destination
output = ../calculations/Blackjack

# 4. Define executable 'make install' location
ioutput = ~/bin/Blackjack

# path to files
src = ../source
prefix = $(src)/prefix.hpp
main = $(src)/main
Deck = $(src)/Deck
Dealer = $(src)/Dealer
Hand = $(src)/Hand
PlayHand = $(src)/PlayHand

# -------------------------------------------------------------------------
# all compiled objects
objects = main.o Deck.o Dealer.o Hand.o PlayHand.o

# -------------------------------------------------------------------------
# Link all objects
Blackjack : $(objects)
	$(CC) $(LFLAGS) -o $(output) $(objects)

# -------------------------------------------------------------------------
# All source code files
main.o : $(main).cpp $(Deck).hpp $(Hand).hpp $(Dealer).hpp $(prefix)
	$(CC) $(CFLAGS) -include $(prefix) $(main).cpp
	
Deck.o : $(Deck).hpp $(Dealer).hpp $(prefix)
	$(CC) $(CFLAGS) -include $(prefix) $(Deck).cpp
	
Dealer.o : $(Dealer).hpp $(Deck).hpp $(Hand).hpp $(PlayHand).hpp $(prefix)
	$(CC) $(CFLAGS) -include $(prefix) $(Dealer).cpp
	
Hand.o : $(Hand).hpp $(Deck).hpp $(Dealer).hpp $(PlayHand).hpp $(prefix)
	$(CC) $(CFLAGS) -include $(prefix) $(Hand).cpp

PlayHand.o : $(PlayHand).hpp $(Hand).hpp $(Deck).hpp $(Dealer).hpp $(prefix)
	$(CC) $(CFLAGS) -include $(prefix) $(PlayHand).cpp

# -------------------------------------------------------------------------
# To make executable
all : Blackjack
        
# -------------------------------------------------------------------------
# To clean compiled objects and executable      
clean : 
	$(RM) $(objects) $(output)

# -------------------------------------------------------------------------
# To clean compiled objects only      
mostlyclean :
	$(RM) $(objects)

# -------------------------------------------------------------------------
# To install        
install : 
	$(CP) $(output) $(ioutput)
