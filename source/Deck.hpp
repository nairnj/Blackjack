/*********************************************************************
    Deck.hpp
    Blackjack
	Created by John Nairn on July 28, 2008.
	
    Copyright (c) 2008 John A. Nairn, All rights reserved.
	see http://www.geditcom.com/Blackjack for documentation
*********************************************************************/

#ifndef _DECK_

#define _DECK_

#define ACE 1
#define TEN 10

class Dealer;

class Deck
{
    public:
	
        // constructors and destructors
        Deck();
		Deck(int);
    
		// methods
		bool remove(int);
		bool remove(int,int);
		bool removeAndGetWeight(int,float *);
		bool removeAndGetWeight(int,float *,Dealer &);
		float probNotSplitCard(int,Dealer &);
		void probSplit234(int,Dealer &,float *);
		void restore(int);
		void restore(int,int);
		bool getRemovals(int,int *,int);
		bool initHandHashTable(Dealer &);
		void clearHandHashTable(Dealer &);
		int getHandAddress(Dealer &,int,int);
		
		// accessors
		void setDecks(int);
		int getDecks(void);
		int getNumber(int);
		int getTotalCards(void);
		
	private:
		int ndecks;			// number of decks
		int ncards;			// number of cards left
		int totalCards;		// total number of cards
		int totalOne;		// total number of one type of crad in full deck
		int totalTens;		// number of tens in full deck
		int nc[11];			// card counts ([1] thru [10])
		
		int *handHashTable;

};

#endif
