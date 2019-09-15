/*********************************************************************
    PlayHand.hpp
    Blackjack
	Created by John Nairn on 8/7/08.
	
    Copyright (c) 2008 John A. Nairn, All rights reserved.
	see http://www.geditcom.com/Blackjack for documentation
*********************************************************************/

#ifndef _PLAYHAND_

#define _PLAYHAND_

#include "Dealer.hpp"

enum { NO_SPLITS=0,SOME_SPLITS,ALL_SPLITS,SPLIT_ONLY };

class Deck;
class Dealer;
class Hand;

class PlayHand
{
    public:
	
        // constructors and destructors
        PlayHand();
        PlayHand(char *,int,float);
		
		// methods
		void incrementRepeat(bool,float);
		void display(void);
		bool removeAndGetWeight(Deck &,Dealer &,float *);
		bool removeAndGetNonsplitWeight(Deck &deck,Dealer &dealer,float *wt);
		void fillHand(Hand *);
		void fillNonsplitHand(Hand *);
		void removeHand(Hand *,Deck &);
		
		// accessors
		int getNumberOfCards(void);
		bool isSplitable(void);
		int getTotal(bool *);
						
	private:
		char cards[MAX_HAND_SIZE+1];		// 0 is # of cards, cards then follow
		int repeat;
		int splits;
		float bet,splitBet;
		int splitCard;
};

typedef vector<PlayHand *> handset;

#endif


