/*********************************************************************
    Dealer.cpp
    Blackjack
	Created by John Nairn on July 28, 2008.
	
    Copyright (c) 2008 John A. Nairn, All rights reserved.
*********************************************************************/

#include "stdafx.h"
#include "Hand.hpp"
#include "Deck.hpp"

#pragma mark Dealer: Constructors and Destructors

// Constructors
Dealer::Dealer()
{	hitsSoft17 = false;
	ddAfterSplit = DDNone;
	hand = new Hand();
	Tarray = NULL;
	initCacheTable(0);
}

// Specify the number of decks
Dealer::Dealer(bool hits,int cacheSize)
{	hitsSoft17=hits;
	ddAfterSplit=DDNone;
	hand = new Hand();
	Tarray=NULL;
	initCacheTable(cacheSize);
}

// allocate memory for the dealer cache
void Dealer::initCacheTable(int size)
{
	int i,j;
	
	// free previous array
	if(Tarray!=NULL)
	{	free(Tarray);
		free(Tj);
		Tarray=NULL;
	}
	
	// size limits
	if(size>MAX_CACHE_SIZE)
		size=MAX_CACHE_SIZE;
	else if(size==0)
		cacheSize=0;
	
	// allocate memory
	size_t tarraySize;
	while(true)
	{	tarraySize = size>MAX_HAND_SIZE ? size : MAX_HAND_SIZE;
	
		// full Tarray data
		size_t blen = 11*tarraySize*(size_t)sizeof(unsigned long);
		Tarray = (unsigned long *)malloc(blen);
		if(Tarray==NULL)
		{	size--;
			continue;
		}
		
		// pointers into the Tarray
		blen = tarraySize*(size_t)sizeof(long *);
		Tj = (unsigned long **)malloc(blen);
		if(Tj==NULL)
		{	free(Tarray);
			Tarray = NULL;
			size--;
			continue;
		}
		unsigned long *Tjptr = Tarray;
		for(i=0; i<tarraySize; i++)
		{	Tj[i] = Tjptr;
			Tjptr += 11;
		}
		
		// dealer probabilities
		size_t cacheBytes = (size_t)GetTj(size,11)*(size_t)sizeof(DealerProbs);
		hold = (DealerProbs *)malloc(cacheBytes);
		if(hold==NULL)
		{	free(Tarray);
			Tarray=NULL;
			free(Tj);
			size--;
			continue;
		}
		
		// memory allocation successful
		break;
	}
		
	// fill the Tj[i][j] = T_(i+1)(j) array
	for(i=0; i<tarraySize; i++)
	{	for(j=0; j<11; j++)
			Tj[i][j] = GetTj(i+1,j);
	}
	
	cacheSize=size;
}

// Evaluate Tj function which is equal to N+j-1 things
// taken j at a time
unsigned long Dealer::GetTj(int j,int x)
{
	if(x==0) return 0;
	double tjcalc=1.;
	for(int i=1; i<=j; i++)
		tjcalc = tjcalc*(x-1+i)/i;
	return((long)(tjcalc+0.5));
}

#pragma mark Dealer: Methods

// given dealer upcard and current deck (with the card already removed),
// calculate player expected values for hand <=16,17,18,19,20,21, expected value of bust is -1
// the dealer does not have blackjack
void Dealer::getPlayerExVals(Deck &deck,DealerProbs *probs)
{
	unsigned long address=0;
	bool hasAddress=false;
	int i;
	
	// see if already done
	if(deck.getRemovals(cacheSize,removed,getRemovableUpcard()))
	{	address=0;
		for(i=0; i<cacheSize; i++)
			address+=Tj[cacheSize-i-1][removed[i]];
		
		if(hold[address].p[0]<4.)
		{	*probs = hold[address];
			return;
		}
		hasAddress=true;
	}
	
	// calculate the probabilities
	for(i=Prob17; i<=ProbBust; i++) probs->p[i]=0.;
	totalWeight = 1.;
	HitDealer(deck,probs);
	
	// make conditional on no dealer blackjack
	if(int naturalCard=hand->getNaturalCard())
	{	float probNatural;
		if(deck.removeAndGetWeight(naturalCard,&probNatural))
		{	probs->p[Prob21]-=probNatural;
			float norm=0.;
			for(i=Prob17; i<=ProbBust; i++) norm+=probs->p[i];
			for(i=Prob17; i<=ProbBust; i++) probs->p[i]/=norm;
			deck.restore(naturalCard);
		}
	}
	
	// convert to player expectations
	DealerProbs dlr=*probs;
	probs->p[ExVal21] = dlr.p[ProbBust]+dlr.p[Prob20]+dlr.p[Prob19]+dlr.p[Prob18]+dlr.p[Prob17];
	probs->p[ExVal20] = probs->p[ExVal21]-dlr.p[Prob21]-dlr.p[Prob20];
	probs->p[ExVal19] = probs->p[ExVal20]-dlr.p[Prob20]-dlr.p[Prob19];
	probs->p[ExVal18] = probs->p[ExVal19]-dlr.p[Prob19]-dlr.p[Prob18];
	probs->p[ExVal17] = probs->p[ExVal18]-dlr.p[Prob18]-dlr.p[Prob17];
	probs->p[ExVal16] = probs->p[ExVal17]-dlr.p[Prob17];
	
	// store the results
	if(hasAddress) hold[address]=*probs;
}

// Recursive routine to hit dealer until done and accumulate the probablitiles
void Dealer::HitDealer(Deck &deck,DealerProbs *probs)
{
	float wt;
	
	for(int i=ACE; i<=TEN; i++)
	{	if(!deck.removeAndGetWeight(i,&wt)) continue;
	
		// increase weight and adjust totals
		float oldWeight=totalWeight;
		totalWeight*=wt;
		hand->hit(i);
		int sum=hand->getTotal();

		// hit again or add to probabilities
		if(sum<17 || (hand->isSoft17() && hitsSoft17))
			HitDealer(deck,probs);
		else
		{	int index = sum>21 ? ProbBust : sum-17;
			probs->p[index]+=totalWeight;
		}
		
		// restore card and weight
		hand->unhit(i,deck);
		totalWeight=oldWeight;
	}
}

// given dealer upcard and current deck (with the card already removed) and knowlegde that player
// has kout hole cards known not to be splitCards,  calculate player expected values for
// hand <=16,17,18,19,20,21, expected value of bust is -1
// the dealer does not have blackjack
// Because of conditional weights, cannot use the dealer cache
void Dealer::getSplitPlayerExVals(Deck &deck,DealerProbs *probs,int splitCard,int kout)
{	int i;

	// calculate the probabilities with custom hit method
	for(i=Prob17; i<=ProbBust; i++) probs->p[i]=0.;
	totalWeight = 1.;
	splitHitDealer(deck,probs,splitCard,kout);
	
	// make conditional on no dealer blackjack
	if(int naturalCard=hand->getNaturalCard())
	{	float probNatural;
		if(deck.removeAndGetWeight(naturalCard,&probNatural))
		{	probNatural=conditionalSplitWt(probNatural,naturalCard,deck,splitCard,kout);
			probs->p[Prob21]-=probNatural;
			float norm=0.;
			for(i=Prob17; i<=ProbBust; i++) norm+=probs->p[i];
			for(i=Prob17; i<=ProbBust; i++) probs->p[i]/=norm;
			deck.restore(naturalCard);
		}
	}
	
	// convert to player expectations
	DealerProbs dlr=*probs;
	probs->p[ExVal21] = dlr.p[ProbBust]+dlr.p[Prob20]+dlr.p[Prob19]+dlr.p[Prob18]+dlr.p[Prob17];
	probs->p[ExVal20] = probs->p[ExVal21]-dlr.p[Prob21]-dlr.p[Prob20];
	probs->p[ExVal19] = probs->p[ExVal20]-dlr.p[Prob20]-dlr.p[Prob19];
	probs->p[ExVal18] = probs->p[ExVal19]-dlr.p[Prob19]-dlr.p[Prob18];
	probs->p[ExVal17] = probs->p[ExVal18]-dlr.p[Prob18]-dlr.p[Prob17];
	probs->p[ExVal16] = probs->p[ExVal17]-dlr.p[Prob17];
}

// Recursive routine to hit dealer until done and accumulate the probablitiles
// with knownledege that player has kout (!=0) hole cards known not to be splitCard
void Dealer::splitHitDealer(Deck &deck,DealerProbs *probs,int splitCard,int kout)
{
	float wt;
	
	for(int i=ACE; i<=TEN; i++)
	{	if(!deck.removeAndGetWeight(i,&wt)) continue;
		wt=conditionalSplitWt(wt,i,deck,splitCard,kout);
	
		// increase weight and adjust totals
		float oldWeight=totalWeight;
		totalWeight*=wt;
		hand->hit(i);
		int sum=hand->getTotal();

		// hit again or add to probabilities
		if(sum<17 || (hand->isSoft17() && hitsSoft17))
			splitHitDealer(deck,probs,splitCard,kout);
		else
		{	int index = sum>21 ? ProbBust : sum-17;
			probs->p[index]+=totalWeight;
		}
		
		// restore card and weight
		hand->unhit(i,deck);
		totalWeight=oldWeight;
	}
}

// Convert dealer weight for card i to conditional weight given kout (!=0) cards of type splitCard known to be in player's hand
float Dealer::conditionalSplitWt(float wt,int i,Deck &deck,int splitCard,int kout)
{	float ntot=(float)deck.getTotalCards()+1.f;			// number before card i was removed
	wt*=ntot/(ntot-kout);							// input wt = n(i)/ntot, now wt = n(i)/(ntot-k)
	if(i!=splitCard)
	{	float nj=(float)deck.getNumber(splitCard);
		wt*=(ntot-nj-(float)kout)/(ntot-nj);
	}
	return wt;
}

#pragma mark Dealer: Accessors

// change up card and remove from deck - caller is responsible for restoring to deck
void Dealer::setUpcard(int card,Deck &deck)
{	hand->reset(card,deck);
	removable=true;
	clearCache();
}
int Dealer::getUpcard(void) { return hand->getFirstCard(); }

// for Griffin tables, convert to nonremovable upcard
void Dealer::makeUnremovable(Deck &deck)
{	removable=false;
	deck.restore(getUpcard());
}
int Dealer::getRemovableUpcard(void) { return removable ? hand->getFirstCard() : 0 ; }

// clear dealer cache (whenever upcard changes)
void Dealer::clearCache(void)
{	if(cacheSize==0) return;
	long length=GetTj(cacheSize,11);
	for(long i=0; i<length; i++) hold[i].p[0]=5.;
}
int Dealer::getCacheSize(void) { return cacheSize; }
size_t Dealer::getCacheBytes(void) { return (size_t)GetTj(cacheSize,11)*(size_t)sizeof(DealerProbs); }

// hitting of soft 17
bool Dealer::getHitsSoft17(void) { return hitsSoft17; }
void Dealer::setHitsSoft17(bool hits) { hitsSoft17=hits; }

// return DD after split option
int Dealer::getDDAfterSplit(void) { return ddAfterSplit; }
void Dealer::setDDAfterSplit(int ddOption) { ddAfterSplit=ddOption; }

