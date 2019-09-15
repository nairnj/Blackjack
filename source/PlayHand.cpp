/*********************************************************************
    PlayHand.cpp
    Blackjack
	Created by John Nairn on 8/7/08.
	
    Copyright (c) 2008 John A. Nairn, All rights reserved.
	see http://www.geditcom.com/Blackjack for documentation
*********************************************************************/

#include "Hand.hpp"
#include "Deck.hpp"

#pragma mark PlayHand: Constructors and Destructors

PlayHand::PlayHand()
{	cards[0]=0;
	repeat=0;
	bet=0.;
	splitBet=0.;
	splitCard=0;
}

PlayHand::PlayHand(char *cds,int firstCard,float betsize)
{	cards[0]=cds[0];
	for(int i=1;i<=cds[0];i++)	cards[i]=cds[i];
	repeat = 1;
	splitCard=firstCard;
	bet = betsize;
	if(cds[1]==splitCard)
	{	splits = 1;
		splitBet = betsize;
	}
	else
	{	splits = 0;
		splitBet = 0.;
	}
}

#pragma mark PlayHand: Methods

// add 1 to repeat counter
void PlayHand::incrementRepeat(bool splitable,float betsize)
{	repeat++;
	bet += betsize;
	if(splitable)
	{	splits++;
		splitBet += betsize;
	}
}

// print hand to standard output
void PlayHand::display(void)
{	cout << splitCard << " ";
	for(int j=1;j<=cards[0];j++) cout << (int)cards[j] << " ";
	bool isSoft;
	int total=getTotal(&isSoft);
	cout << "(" ;
	if(isSoft)
		cout << "soft ";
	else
		cout << "hard ";
	cout << total ;
	cout << ", cards=" << (int)cards[0];
	cout << ", repeats=" << repeat;
	cout << ", splits=" << splits;
	cout << ", bet=" << bet;
	cout << ", split_bet=" << splitBet;
	cout << ")" << endl;
}

// remove all cards and get its weight, return false if any now missing
bool PlayHand::removeAndGetWeight(Deck &deck,Dealer &dealer,float *wt)
{
	float cardwt,totalwt=1.;
	
	// get weights for each card
	for(int i=1;i<=cards[0];i++)
	{	if(!deck.removeAndGetWeight((int)cards[i],&cardwt,dealer))
		{	// weight is zero, restore removed cards
			for(int j=1;j<i;j++)
				deck.restore((int)cards[j]);
			return false;
		}
		totalwt*=cardwt;
	}
	
	// acceptable hand
	*wt = totalwt*(float)repeat;
	return true;
}

// remove and get weight for non-split portion of this hand
// Code duplicates much of removeAndGetWeight() to save one floating point calculation
bool PlayHand::removeAndGetNonsplitWeight(Deck &deck,Dealer &dealer,float *wt)
{	if(splits==repeat) return false;		// all hands are splits
	
	// get weights for each card
	float cardwt,totalwt=1.;
	for(int i=1;i<=cards[0];i++)
	{	if(!deck.removeAndGetWeight((int)cards[i],&cardwt,dealer))
		{	// weight is zero, restore removed cards
			for(int j=1;j<i;j++)
				deck.restore((int)cards[j]);
			return false;
		}
		totalwt*=cardwt;
	}
	
	// acceptable hand
	*wt = totalwt*(float)(repeat-splits);
	return true;
}

// hit this hand with the actual hand
void PlayHand::fillHand(Hand *hand)
{	for(int i=1;i<=cards[0];i++)
		hand->hit((int)cards[i]);
	hand->setDoubled(bet/(float)repeat);		// bet per hand
}

// hit this hand with actual hand and get bet size from non-split fraction
// Warning: never call if repeat==splits
void PlayHand::fillNonsplitHand(Hand *hand)
{	for(int i=1;i<=cards[0];i++)
		hand->hit((int)cards[i]);
	hand->setDoubled((bet-splitBet)/(float)(repeat-splits));		// nonsplit bet per hand
}

// remove cards in this hand and restore to deck (leave initial card intact)
void PlayHand::removeHand(Hand *hand,Deck &deck)
{	for(int i=1;i<=cards[0];i++)
		hand->unhit((int)cards[i],deck);
}

#pragma mark PlayHand: Accessors

int PlayHand::getNumberOfCards(void) { return (int)cards[0]; }
bool PlayHand::isSplitable(void) { return splits>0; }
int PlayHand::getTotal(bool *isSoft)
{	int total=splitCard;
	int aces = total==1 ? 1 : 0 ;
	for(int i=1;i<=cards[0];i++)
	{	total+=(int)cards[i];
		if(cards[1]==1) aces++;
	}
	if(total<12 && aces>0)
	{	total+=10;
		*isSoft=true;
	}
	else
		*isSoft=false;
	return total;
}

