/*********************************************************************
    prefix.hpp
    Blackjack coding prefix file
    Created by John Nairn on July 28, 2008.
	
    Copyright (c) 2008 John A. Nairn, All rights reserved.
	see http://www.geditcom.com/Blackjack for documentation
*********************************************************************/

// C includes
#ifdef WINDOWS_EXE
#include <stdio.h>
#endif
#include <cstdio>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <math.h>

// For c++89 compliance
using namespace std;

// math utilities
#define fmin(a,b) (((a)>(b))?(b):(a))
#define fmax(a,b) (((a)>(b))?(a):(b))

