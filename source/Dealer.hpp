/*********************************************************************
    Dealer.hpp
    Blackjack
	Created by John Nairn on July 28, 2008.
	
    Copyright (c) 2008 John A. Nairn, All rights reserved.
	see http://www.geditcom.com/Blackjack for documentation
*********************************************************************/

#ifndef _DEALER_

#define _DEALER_

// Dealer probablity block
typedef struct {
	float p[6];
} DealerProbs;

// maximum cache size using size_t (unsigned long) for malloc()
// This requires 3,147,075,584 bytes and fails on Mac, but maybe 
// not all systems. Size of 25 overflows the size_t variable
#define MAX_CACHE_SIZE 24

// for enumerating hands and insuring Tj[][] array large enough
// Largest possible hand following basic strategy are:
// 1 deck: 3,2,2,2,2,1,1,1,1,any
// inf deck: 1,1,1,1,1,1,1,1,4,1,1,1,1,any
// The 4 is when hitting soft 18 vs Ace and make the hand a hard 12
// Since first card is not counted when enumerating hands, the max is 1 less or 13
#define MAX_HAND_SIZE 13

class Hand;
class Deck;

enum { Prob17=0,Prob18,Prob19,Prob20,Prob21,ProbBust,DealerProbLength };
enum { ExVal16=0,ExVal17,ExVal18,ExVal19,ExVal20,ExVal21 };

#define ResplitNONE 1
#define ResplitALLOWED 2

#define DDNone 1
#define DDAny 2
#define DD10OR11 4

class Dealer
{
    public:	
		unsigned long **Tj;
		
        // constructors and destructors
        Dealer();
		Dealer(bool,int);
		void initCacheTable(int);
		
		// methods
		void getPlayerExVals(Deck &,DealerProbs *);
		unsigned long GetTj(int,int);
		void getSplitPlayerExVals(Deck &,DealerProbs *,int,int);
		void splitHitDealer(Deck &,DealerProbs *,int,int);
		float conditionalSplitWt(float,int,Deck &,int,int);
		
		// accessors
		void setUpcard(int,Deck &);
		int getUpcard(void);
		void makeUnremovable(Deck &);
		int getRemovableUpcard(void);
		void clearCache(void);
		int getCacheSize(void);
		size_t getCacheBytes(void);
		bool getHitsSoft17(void);
		void setHitsSoft17(bool hits);
		int getDDAfterSplit(void);
		void setDDAfterSplit(int);
				
	private:
		bool hitsSoft17;
		Hand *hand;
		float totalWeight;
		int ddAfterSplit;
		bool removable;
		
		int cacheSize;
		int removed[100];
		unsigned long *Tarray;
		DealerProbs *hold;
		
		void HitDealer(Deck &,DealerProbs *);

};

#endif
