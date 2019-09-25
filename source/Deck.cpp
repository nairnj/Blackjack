/*********************************************************************
    Deck.cpp
    Blackjack
	Created by John Nairn on July 28, 2008.
	
    Copyright (c) 2008 John A. Nairn, All rights reserved.
*********************************************************************/

#include "stdafx.h"
#include "Deck.hpp"
#include "Dealer.hpp"

#pragma mark Deck: Constructors and Destructors

// Constructors
Deck::Deck()
{	setDecks(1);
	handHashTable=NULL;
}

// Specify the number of decks
Deck::Deck(int nd)
{	setDecks(nd);
	handHashTable=NULL;
}

#pragma mark Deck: Methods

// remove card(s) (if possible) and return true or false if removed
bool Deck::remove(int card)
{	if(nc[card]==0) return false;
	nc[card]--;
	ncards--;
	return true;
}
bool Deck::remove(int card1,int card2)
{	if(!remove(card1)) return false;
	if(!remove(card2))
	{	restore(card1);
		return false;
	}
	return true;
}

// remove card (if possible), calculate weight, and return true or false if removed
bool Deck::removeAndGetWeight(int card,float *weight)
{	if(nc[card]==0) return false;
	*weight=(float)nc[card]/(float)ncards;
	nc[card]--;
	ncards--;
	return true;
}

// remove card (if possible), calculate conditional weight assuming dealer
// does not have blackjack. Return true or false if removed 
bool Deck::removeAndGetWeight(int card,float *weight,Dealer &dealer)
{	if(nc[card]==0) return false;
	int upcard = dealer.getUpcard();
	if(upcard==ACE || upcard==TEN)
	{	*weight=(float)nc[card]/(float)(ncards-1);
		if(card!=11-upcard)
			*weight*=(float)(ncards-nc[11-upcard]-1)/(float)(ncards-nc[11-upcard]);
	}
	else
		*weight=(float)nc[card]/(float)ncards;
	nc[card]--;
	ncards--;
	return true;
}

// return conditional probablity that the next card is not splitCard
float Deck::probNotSplitCard(int splitCard,Dealer &dealer)
{
	float probSplit;
	int upcard = dealer.getUpcard();
	
	// get probablity next card is a split card
	if(upcard==ACE || upcard==TEN)
	{	// conditional probability of split given dealer has an ace or a ten
		probSplit = (float)nc[splitCard]/(float)(ncards-1);
		if(splitCard!=11-upcard)
			probSplit *= (float)(ncards-nc[11-upcard]-1)/(float)(ncards-nc[11-upcard]);
	}
	else
		probSplit = (float)nc[splitCard]/(float)ncards;
	
	// one minus is probablity not a split card
	return 1.f-probSplit;
}

// Get probability splitting 2, 3, 4 hands
// assumes at least 1 split card in the deck
void Deck::probSplit234(int splitCard,Dealer &dealer,float *prob)
{
	float pns,pnm1s,pnm3ss,pnm2ss,pnm1ss;
	int upcard = dealer.getUpcard();
	
	if(upcard==ACE || upcard==TEN)
	{	pns = (float)nc[splitCard]/(float)(ncards-1);
		pnm1s = (float)nc[splitCard]/(float)(ncards-2);
		pnm1ss = (float)(nc[splitCard]-1)/(float)(ncards-2);
		pnm2ss = (float)(nc[splitCard]-1)/(float)(ncards-3);
		pnm3ss = (float)(nc[splitCard]-1)/(float)(ncards-4);
		if(splitCard != (11-upcard))
		{	float nmnj = (float)(ncards-nc[11-upcard]);
			pns *= (nmnj-1.f)/nmnj;
			pnm1s *= (nmnj-2.f)/(nmnj-1.f);
			pnm1ss *= (nmnj-2.f)/(nmnj-1.f);
			pnm2ss *= (nmnj-3.f)/(nmnj-2.f);
			pnm3ss *= (nmnj-4.f)/(nmnj-3.f);
		}
	}
	else
	{	pns = (float)nc[splitCard]/(float)ncards;
		pnm1s = (float)nc[splitCard]/(float)(ncards-1);
		pnm1ss = (float)(nc[splitCard]-1)/(float)(ncards-1);
		pnm2ss = (float)(nc[splitCard]-1)/(float)(ncards-2);
		pnm3ss = (float)(nc[splitCard]-1)/(float)(ncards-3);
	}
	
	// Let n=nonsplit, s=split, x=any
	prob[0] = (1.f-pns)*(1.f-pnm1s);						// nnxx (4) = P(2)
	float probns = (1.f-pns)*pnm1s;
	float probsn = pns*(1.f-pnm1ss);
	prob[8] = probsn*(1.f-pnm2ss)*(1.f-pnm3ss);			// snnn (1) = P(3/2)
	prob[9] = probns*(1.f-pnm2ss)*(1.f-pnm3ss);			// nsnn (2) = P(3/1)
	prob[1] = prob[8]+prob[9];							// nsnn + snnn (2) = P(3)
	prob[2] = 1.f-prob[0]-prob[1];						// the rest (10) = P(4)
	
	// partition prob[2] into specific orders
	prob[3] = pns*pnm1ss;							// ssxx (4) = P(4/5)
	prob[4] = probsn*pnm2ss;						// snsx (2) = P(4/4)
	prob[5] = probsn*(1.f-pnm2ss)*pnm3ss;			// snns (1) = P(4/3)
	prob[6] = probns*pnm2ss;						// nssx (2) = P(4/2)
	prob[7] = probns*(1.f-pnm2ss)*pnm3ss;			// nsns (1) = P(4/1)
	
	//cout << " P[4]: " << prob[2] << " summed P[4]: " << (prob[3]+prob[4]+prob[5]+prob[6]+prob[7]) << endl;
	
}

// restore card(s) - assumes they can be restored
void Deck::restore(int card)
{	nc[card]++;
	ncards++;
}
void Deck::restore(int card1,int card2)
{	restore(card1);
	restore(card2);
}

// compile non-decreasing list of removed cards
bool Deck::getRemovals(int maxSave,int *removed,int upcard)
{
	// are too many gone
	if(totalCards-ncards-1>maxSave) return false;
	
	// Temporarily reinstate dealer up card 
	if(upcard>0) nc[upcard]++;
	
	// Decribe removed cards in nondecreasing sequence where
	//	card of rank i (ace=1) is called i and non-existent
	//	card is called 0
	int num=0;					// Number removed cards found
	
	// Special case for 10's to save small amount of time
	int cardStop=totalTens-nc[TEN];
	while(num<cardStop)
		removed[num++]=TEN;
		
	// Rest of the ranks
	for(int i=9;i>=ACE;i--)
	{	cardStop=num+totalOne-nc[i];
		while(num<cardStop)
			removed[num++]=i;
	}
	
	// Fill remaining card_ids with 0
	while(num<maxSave)
		removed[num++]=0;
		
	// Remove dealer upcard again
	if(upcard>0) nc[upcard]--;
	
	return true;
}

#pragma mark Deck::Hand Hash Table

// initialize hand has table, return false if fails
bool Deck::initHandHashTable(Dealer &dealer)
{	if(handHashTable!=NULL) return true;
	size_t handBytes = (size_t)dealer.GetTj(MAX_HAND_SIZE,11)*(size_t)sizeof(int);
	handHashTable = (int *)malloc(handBytes);
	if(handHashTable==NULL) return false;
	return true;
}

// clear the hand hash table
void Deck::clearHandHashTable(Dealer &dealer)
{	long length=dealer.GetTj(MAX_HAND_SIZE,11);
	for(long i=0; i<length; i++) handHashTable[i]=-1;
}

// check current hand in the hand hashtable
int Deck::getHandAddress(Dealer &dealer,int splitCard,int endIndex)
{
	// temporarily restore the two split cards
	nc[splitCard]++;
	nc[splitCard]++;
	
	// warning - assumes deck will always get removals and will always get valid address
	int removed[MAX_HAND_SIZE];
	getRemovals(MAX_HAND_SIZE,removed,dealer.getUpcard());
	long address=0;
	for(int i=0; i<MAX_HAND_SIZE; i++)
		address+=dealer.Tj[MAX_HAND_SIZE-i-1][removed[i]];
	
	// retrieve this hand's index in playable hands list
	int handIndex=handHashTable[address];
	
	// if first time, store its index
	if(handIndex<0)
	{	handHashTable[address]=endIndex;
		handIndex=endIndex;
	}
	
	// remove split cards again
	nc[splitCard]--;
	nc[splitCard]--;
	
	return handIndex;
}

#pragma mark Deck: Accessors

// reset decks to all cards present
void Deck::setDecks(int nd)
{
	ndecks=nd;
	totalOne=4*nd;
	for(int i=ACE;i<TEN;i++) nc[i]=totalOne;
	totalTens=nc[TEN]=16*nd;
	totalCards=ncards=52*nd;
}

// return number of decks
int Deck::getDecks(void) { return ndecks; }

// number of certain denomination
int Deck::getNumber(int value) { return nc[value]; }

// total number of cards left
int Deck::getTotalCards(void) { return ncards; }

