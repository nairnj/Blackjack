/*********************************************************************
    Hand.hpp
    Blackjack
	Created by John Nairn on July 28, 2008.
	
    Copyright (c) 2008 John A. Nairn, All rights reserved.
	see http://www.geditcom.com/Blackjack for documentation
*********************************************************************/

#ifndef _HAND_

#define _HAND_

#include "Dealer.hpp"
#include "PlayHand.hpp"

class Deck;

class Hand
{
    public:	
        // constructors and destructors
        Hand();
		Hand(int);
		Hand(int,int);
		
		// Methods
		void reset(void);
		void reset(int);
		void reset(int,int);
		void reset(int,Deck &);
		void reset(int,int,Deck &);
		void hit(int);
		void unhit(int);
		void unhit(int,Deck &);
		
		// expected values
		float standExval(Deck &,Dealer &);
		float hitExval(Deck &,Dealer &);
		float doubleExval(Deck &,Dealer &);
		float splitStandExval(Deck &,Dealer &,int);
		
		// spllitting
		void splitCalcs(Deck &,Dealer &,int,int,float *);
		float approxSplitPlay(Deck &,Dealer &,bool);
		float approxSplitExval(Deck &,Dealer &,bool,int);
		void exactSplitCalcs(Deck &,Dealer &,int,int,float *);
		float exactSplitExval(Deck &,Dealer &,Hand **,int &,int);
		void handExactSplitCalcs(Deck &,Dealer &,int,int,float *);
		float handExactSplitExval(Deck &,Dealer &,Hand **,int &,int,handset &);

		// basic stragegy
		bool basicSplitHit(Deck &,Dealer &);
		bool basicHit(Deck &,Dealer &);
		bool twoCardException(Deck &,Dealer &,bool &);
		bool basicDD(Deck &,Dealer &);
		bool twoCardHit(Deck &,Dealer &);
				
		// Accessors
		int getTotal(void);
		int getPlayerIndex(void);
		bool isSoft(void);
		bool isSoft17(void);
		float getDoubled(void);
		void setDoubled(float);
		float getBetPerHand(void);
		int getNaturalCard(void);
		bool isNatural(void);
		int getFirstCard(void);
		bool handIs(int,int);
		void setOrder(int);
		void display(void);
		char cardChar(int);
		
	private:
		int total;
		int aces;
		int cards;
		int firstCard;
		float doubled;
		int order;

		// enumerate hands
		void collectHands(Deck &,Dealer &,handset &);
		void enumerateHands(Deck &,Dealer &,char *,handset &);
		void clearHands(handset &);
		
};

#endif
