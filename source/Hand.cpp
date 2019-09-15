/*********************************************************************
    Hand.cpp
    Blackjack
	Created by John Nairn on July 28, 2008.
	
    Copyright (c) 2008 John A. Nairn, All rights reserved.
	see http://www.geditcom.com/Blackjack for documentation
*********************************************************************/

#include "Hand.hpp"
#include "Deck.hpp"

#pragma mark Hand: Constructors and Destructors

// Constructors (0, 1, or 2 cards)
Hand::Hand() { reset(); }
Hand::Hand(int card) { reset(card); }
Hand::Hand(int card1,int card2) { reset(card1,card2); }

#pragma mark Hand: Methods

// reset hand to new settings
void Hand::reset(void) { total=aces=cards=firstCard=0; }
void Hand::reset(int card)
{	total = firstCard = card;
	aces = 0;
	if(card==1) aces++;
	cards = 1;
	doubled = 1.;
}
void Hand::reset(int card1,int card2)
{	total = card1+card2;
	firstCard = card1;
	aces=0;
	if(card1==1) aces++;
	if(card2==1) aces++;
	cards = 2;
	doubled = 1.;
}

// reset the hand and remove from deck (assumes they can be removed)
// caller is responsible for restoring to deck when done
void Hand::reset(int card,Deck &deck)
{	reset(card);
	deck.remove(card);
}
void Hand::reset(int card1,int card2,Deck &deck)
{	reset(card1,card2);
	deck.remove(card1,card2);
}

// Specify the number of decks and return updated total
void Hand::hit(int card)
{	total+=card;
	if(card==1) aces++;
	cards++;
}

// Remove card from the hand
void Hand::unhit(int card)
{	total-=card;
	if(card==1) aces--;
	cards--;
}

// Remove card from the hand and restore it to the deck
void Hand::unhit(int card,Deck &deck)
{	unhit(card);
	deck.restore(card);
}

#pragma mark Hand: Expected Value Methods

// calculate the expected value of stand this hand against supplied dealer and deck
float Hand::standExval(Deck &deck,Dealer &dealer)
{	// loss if busted
	int index=getPlayerIndex();
	if(index>ExVal21) return -1.;
	
	// get expected values from dealer probablities
	DealerProbs probs;
	dealer.getPlayerExVals(deck,&probs);
	return probs.p[index];
}

// calculate the expected value of stand this hand against supplied dealer and deck
// kout is number of cards held by player known not to be = firstCard
float Hand::splitStandExval(Deck &deck,Dealer &dealer,int kout)
{	// loss if busted
	int index=getPlayerIndex();
	if(index>ExVal21) return -1.;
	
	// normal dealer probabilites if none out or if all are out
	DealerProbs probs;
	if(kout==0 || deck.getNumber(firstCard)==0)
		dealer.getPlayerExVals(deck,&probs);
	else
		dealer.getSplitPlayerExVals(deck,&probs,firstCard,kout);
	return probs.p[index];
}

// calculate the expected value to hit this hand and then finish using basic strategy
float Hand::hitExval(Deck &deck,Dealer &dealer)
{
	float exval=0,wt;
	
	for(int i=ACE; i<=TEN; i++)
	{	if(!deck.removeAndGetWeight(i,&wt,dealer)) continue;
	
		// add card
		hit(i);

		// hit again or add to probabilities
		if(basicHit(deck,dealer))
			exval+=wt*hitExval(deck,dealer);
		else
			exval+=wt*standExval(deck,dealer);
		
		// remove from hand and restore to deck
		unhit(i,deck);
	}
	
	return exval;
}

// calculate the expected value to hit this hand and then finish using
float Hand::doubleExval(Deck &deck,Dealer &dealer)
{
	float exval=0,wt;
	
	for(int i=ACE; i<=TEN; i++)
	{	if(!deck.removeAndGetWeight(i,&wt,dealer)) continue;
	
		// add card
		hit(i);

		// add expected value
		exval+=wt*standExval(deck,dealer);
		
		// remove from hand and restore to deck
		unhit(i,deck);
	}
	
	// double for expected value
	return 2.*exval;
}

#pragma mark Hand::Splitting Calculations

// Entry to handle bulk splitting calculations
// replitFlags can be ResplitNONE+ResplitALLOWED
// ddFlags can be DDNone+DDAny+DD10OR11
// results is array of length 6
void Hand::splitCalcs(Deck &deck,Dealer &dealer,int replitFlags,int ddFlags,float *results)
{
	int i;
	
	for(i=0;i<6;i++) results[i]=0.;
	
	// replit options
	for(i=1;i<=2;i++)
	{	// skip if this replitting option not selected
		if(!(i&replitFlags)) continue;
		bool resplit = (i==ResplitALLOWED) ;
		int base = resplit ? 3 : 0 ;
		
		// do DD not allowed
		if(ddFlags&DDNone)
		{	dealer.setDDAfterSplit(DDNone);
			results[base+0] = approxSplitPlay(deck,dealer,resplit);
		}
		
		// do DD any allowed
		if(ddFlags&DDAny)
		{	if(ddFlags&DDNone && (firstCard==TEN || firstCard==ACE))
				results[base+1] = results[base+0];
			else
			{	dealer.setDDAfterSplit(DDAny);
				results[base+1] = approxSplitPlay(deck,dealer,resplit);
			}
		}
		
		// do DD 10 or 11 only allowed
		if(ddFlags&DD10OR11)
		{	if(ddFlags&DDAny && (dealer.getUpcard()==1 || dealer.getUpcard()>=7 || firstCard>=9 || firstCard==ACE))
				results[base+2] = results[base+1];
			else
			{	dealer.setDDAfterSplit(DD10OR11);
				results[base+2] = approxSplitPlay(deck,dealer,resplit);
			}
		}
	}
}

// General method for approximate splitting calculations
float Hand::approxSplitPlay(Deck &deck,Dealer &dealer,bool resplit)
{
	float explay;
	if(resplit)
	{	// expected value with two split cards removed and conditioned
		// on second card of hand not being a split card
		float ex2 = approxSplitExval(deck,dealer,resplit,1);
	
		// remove another and repeat expected value
		float ex3=0.,ex4=0.;
		if(deck.remove(firstCard))
		{	ex3 = approxSplitExval(deck,dealer,resplit,2);
			
			// last hand with no resplit
			if(deck.remove(firstCard))
			{	ex4 = approxSplitExval(deck,dealer,false,0);
				//ex4 = ex3+(ex3-ex2);
				deck.restore(firstCard);
			}
			else
				ex4=0.;
			
			deck.restore(firstCard);
			
			float prob[10];
			deck.probSplit234(firstCard,dealer,prob);
			explay = 2.*prob[0]*ex2 + 3.*prob[8]*ex3 + (ex2+2.*ex3)*prob[9];
			if(deck.getNumber(firstCard)>1)
			{	explay += prob[3]*4.*ex4 + prob[4]*(ex3+3.*ex4) + prob[5]*2.*(ex3+ex4)
							 + prob[6]*(ex2+3.*ex4) + prob[7]*(ex2+ex3+2.*ex4);
			}
			// Griffin analysis
			//explay = 2.*prob[0]*ex2 + 3.*prob[1]*ex3 + 4.*prob[2]*ex4;
		}
		else
			explay = 2.*ex2;
	}
	else
		explay=2.*approxSplitExval(deck,dealer,resplit,0);
	
	return explay;
}

// calculate the expected value to hit this hand and then finish using
// kout is known number of player hole cards not equal to split card
float Hand::approxSplitExval(Deck &deck,Dealer &dealer,bool resplit,int kout)
{
	float exval=0,wt,pns=1.;
	
	// probability not a split card
	if(resplit && cards==1)
		pns = deck.probNotSplitCard(firstCard,dealer);
	
	for(int i=ACE; i<=TEN; i++)
	{	// If resplitting, skip another split card or calculate prob not split card
		if(resplit && cards==1 && i==firstCard) continue;
		
		// get weight and adjust if resplitting
		if(!deck.removeAndGetWeight(i,&wt,dealer)) continue;
		if(resplit && cards==1) wt /= pns;
	
		// add card
		hit(i);

		// hit again or add to probabilities
		if(basicSplitHit(deck,dealer))
			exval += wt*approxSplitExval(deck,dealer,resplit,kout);
		else
			exval += doubled*wt*splitStandExval(deck,dealer,kout);
		
		// remove from hand and restore to deck
		unhit(i,deck);
	}
	
	return exval;
}

// Entry to handle bulk exact splitting calculations
// maxSplitHands=2 for no resplitting or no of hands enumerated
// ddFlags can be DDNone+DDAny+DD10OR11
// results is array of length 3
void Hand::exactSplitCalcs(Deck &deck,Dealer &dealer,int maxSplitHands,int ddFlags,float *results)
{
	Hand *hands[maxSplitHands];
	int i;
	
	for(i=0;i<3;i++) results[i]=0.;
	
	// zero is this base hand
	hands[0]=this;
	hands[0]->setOrder(0);
	unhit(firstCard);
	
	// create first new hand
	hands[1]=new Hand(firstCard);
	hands[1]->setOrder(1);
	int numSplitHands=2;
	
	// do DD not allowed
	if(ddFlags&DDNone)
	{	dealer.setDDAfterSplit(DDNone);
		results[0] = exactSplitExval(deck,dealer,hands,numSplitHands,maxSplitHands);
	}
	
	// do DD any allowed
	if(ddFlags&DDAny)
	{	if(ddFlags&DDNone && (firstCard==TEN || firstCard==ACE))
			results[1] = results[0];
		else
		{	dealer.setDDAfterSplit(DDAny);
			results[1] = exactSplitExval(deck,dealer,hands,numSplitHands,maxSplitHands);
		}
	}
	
	// do DD 10 or 11 only allowed
	if(ddFlags&DD10OR11)
	{	if(ddFlags&DDNone && (firstCard==TEN || firstCard==ACE))
			results[2] = results[0];
		else if(ddFlags&DDAny && (dealer.getUpcard()==1 || dealer.getUpcard()>=7 || firstCard>=9 || firstCard==ACE))
			results[2] = results[1];
		else
		{	dealer.setDDAfterSplit(DD10OR11);
			results[2] = exactSplitExval(deck,dealer,hands,numSplitHands,maxSplitHands);
		}
	}

	// delete the second hand
	delete hands[1];
}

// hit a hand that has been split
float Hand::exactSplitExval(Deck &deck,Dealer &dealer,Hand **hands,int &numSplitHands,int maxSplitHands)
{
	float exval=0.,wt;
	bool newHand;
	
	for(int i=TEN; i>=ACE; i--)
	{	if(!deck.removeAndGetWeight(i,&wt,dealer)) continue;
	
		// add new hand or new card
		if(i==firstCard && cards==1 && numSplitHands<maxSplitHands)
		{	hands[numSplitHands]=new Hand(firstCard);
			hands[numSplitHands]->setOrder(numSplitHands);
			numSplitHands++;
			newHand=true;
		}
		else
		{	hit(i);
			newHand=false;
		}

		// hit again, move on to next hand, or add to probabilities
		if(basicSplitHit(deck,dealer))
			exval += wt*exactSplitExval(deck,dealer,hands,numSplitHands,maxSplitHands);
		else if(this!=hands[numSplitHands-1])
			exval+=wt*hands[order+1]->exactSplitExval(deck,dealer,hands,numSplitHands,maxSplitHands);
		else
		{	// if just finished last hand then add up expected values for all hands
			DealerProbs probs;
			dealer.getPlayerExVals(deck,&probs);
			float totalVal=0.;
			for(int j=0; j<numSplitHands; j++)
			{	int index = hands[j]->getPlayerIndex();
				if(index>ExVal21)
					totalVal -= hands[j]->getDoubled();
				else
					totalVal += hands[j]->getDoubled()*probs.p[index];
			}
			exval+=wt*totalVal;
		}
		
		// remove from hand and restore to deck or remove new hand and restore to deck
		if(newHand)
		{	numSplitHands--;
			delete hands[numSplitHands];
			deck.restore(i);
		}
		else
			unhit(i,deck);
	}
	
	return exval;
}

#pragma mark Hand::Splitting by Possible Hands

// Entry to handle bulk exact splitting calculations
// maxSplitHands=2 for no resplitting or no of hands enumerated
// ddFlags can be DDNone+DDAny+DD10OR11
// results is array of length 3
void Hand::handExactSplitCalcs(Deck &deck,Dealer &dealer,int maxSplitHands,int ddFlags,float *results)
{
	handset handList;
	Hand *hands[maxSplitHands];
	
	for(int i=0;i<3;i++) results[i]=0.;
	
	// zero is this base hand
	hands[0]=this;
	hands[0]->setOrder(0);
	unhit(firstCard);
	
	// create first new hand
	hands[1]=new Hand(firstCard);
	hands[1]->setOrder(1);
	int numSplitHands=2;
	
	// do DD not allowed
	if(ddFlags&DDNone)
	{	dealer.setDDAfterSplit(DDNone);
		collectHands(deck,dealer,handList);			// find list of possible hands
		results[0] = handExactSplitExval(deck,dealer,hands,numSplitHands,maxSplitHands,handList);
		clearHands(handList);
	}
	
	// do DD any allowed
	if(ddFlags&DDAny)
	{	if(ddFlags&DDNone && (firstCard==TEN || firstCard==ACE))
			results[1] = results[0];
		else
		{	dealer.setDDAfterSplit(DDAny);
			collectHands(deck,dealer,handList);			// find list of possible hands
			results[1] = handExactSplitExval(deck,dealer,hands,numSplitHands,maxSplitHands,handList);
			clearHands(handList);
		}
	}
	
	// do DD 10 or 11 only allowed
	if(ddFlags&DD10OR11)
	{	if(ddFlags&DDNone && (firstCard==TEN || firstCard==ACE))
			results[2] = results[0];
		else if(ddFlags&DDAny && (dealer.getUpcard()==1 || dealer.getUpcard()>=7 || firstCard>=9 || firstCard==ACE))
			results[2] = results[1];
		else
		{	dealer.setDDAfterSplit(DD10OR11);
			collectHands(deck,dealer,handList);			// find list of possible hands
			results[2] = handExactSplitExval(deck,dealer,hands,numSplitHands,maxSplitHands,handList);
			clearHands(handList);
		}
	}

	// delete the second hand
	delete hands[1];
}

// hit a hand that has been split
float Hand::handExactSplitExval(Deck &deck,Dealer &dealer,Hand **hands,int &numSplitHands,int maxSplitHands,handset &handList)
{
	float exval=0.,wt;
	
	// check for resplitting
	if(numSplitHands<maxSplitHands)
	{	if(deck.removeAndGetWeight(firstCard,&wt,dealer))
		{	hands[numSplitHands]=new Hand(firstCard);
			hands[numSplitHands]->setOrder(numSplitHands);
			numSplitHands++;
			exval += wt*handExactSplitExval(deck,dealer,hands,numSplitHands,maxSplitHands,handList);
			numSplitHands--;
			delete hands[numSplitHands];
			deck.restore(firstCard);
		}
	}
	
	for(int i=0; i<handList.size(); i++)
	{	if(numSplitHands<maxSplitHands && handList[i]->isSplitable())
		{	// only use the non-split hands
			if(!handList[i]->removeAndGetNonsplitWeight(deck,dealer,&wt)) continue;
			handList[i]->fillNonsplitHand(this);
		}
		else
		{	if(!handList[i]->removeAndGetWeight(deck,dealer,&wt)) continue;
			handList[i]->fillHand(this);
		}
		
		if(this==hands[numSplitHands-1])
		{	// if just finished last hand then add up expected values for all hands
			DealerProbs probs;
			dealer.getPlayerExVals(deck,&probs);
			float totalVal=0.;
			for(int j=0; j<numSplitHands; j++)
			{	int index = hands[j]->getPlayerIndex();
				if(index>ExVal21)
					totalVal -= 1.;			// doubled hands are never a bust
				else
					totalVal += hands[j]->getBetPerHand()*probs.p[index];
			}
			exval+=wt*totalVal;
		}
		else
		{	// if not last hand, move on to the next hand in the list
			exval += wt*hands[order+1]->handExactSplitExval(deck,dealer,hands,numSplitHands,maxSplitHands,handList);
		}
		
		// remove all cards from hand (except the first) and restore to the deck
		handList[i]->removeHand(this,deck);
	}
	
	return exval;
}

#pragma mark Hand::Collect Possible Hands

// collect all possible hands for current split situation
void Hand::collectHands(Deck &deck,Dealer &dealer,handset &handList)
{	char cds[20];
	deck.clearHandHashTable(dealer);
	cds[0]=0;
	enumerateHands(deck,dealer,cds,handList);
}

// clear objects in a handset
void Hand::clearHands(handset &handList)
{	for(int j=0;j<handList.size();j++) delete handList[j];
	handList.clear();
}

// calculate the expected value to hit this hand and then finish using
void Hand::enumerateHands(Deck &deck,Dealer &dealer,char *cds,handset &handList)
{
	for(int i=TEN; i>=ACE; i--)
	{	// get weight and adjust if resplitting
		if(!deck.remove(i)) continue;
	
		// add card
		hit(i);
		cds[0]++;
		cds[cds[0]]=(char)i;

		// hit again or add to probabilities
		if(basicSplitHit(deck,dealer))
			enumerateHands(deck,dealer,cds,handList);
		else
		{	int index=deck.getHandAddress(dealer,firstCard,(int)handList.size());
			if(index==handList.size())
				handList.push_back(new PlayHand(cds,firstCard,doubled));
			else
				handList[index]->incrementRepeat(cds[1]==firstCard,doubled);
		}
		
		// remove from hand and restore to deck
		cds[0]--;
		unhit(i,deck);
	}
}

#pragma mark Hand::Basic Strategy

// special cases when hitting a split hand
bool Hand::basicSplitHit(Deck &deck,Dealer &dealer)
{
	if(cards==1)
	{	doubled=1.;
		return true;
	}
		
	else if(cards==2)
	{	// split aces receive one card
		if(firstCard==ACE) return false;
		
		// check on double down, if yes return from for a hit,  but set double to no future hits
		if(basicDD(deck,dealer))
		{	doubled=2.;
			return true;
		}
		else
			doubled=1.;
			
		// two card exceptions
		bool exception;
		bool hit = twoCardException(deck,dealer,exception);
		if(exception) return hit;
	}
	
	// if doubled, then no more hits
	if(doubled>1.5) return false;
	
	// all the rest use basicHit
	return basicHit(deck,dealer);
}

// check if basic strategy says to hit this hand
// if had has less then 3 cards, this assumes already checked for 2-card exceptions
bool Hand::basicHit(Deck &deck,Dealer &dealer)
{
	int ndecks = deck.getDecks();
	int upcard = dealer.getUpcard();
	
	if(isSoft())
	{	switch(upcard)
		{	case ACE:
				if(ndecks==1 && !dealer.getHitsSoft17() && total==8)
					return false;
			case 9:
			case TEN:
				if(total>=9) return false;
				return true;
			
			default:
				if(total>=8) return false;
				return true;
		}
	}
	else
	{	// default hard hitting
		switch(upcard)
		{	case 2:
			case 3:
				if(total>=13) return false;
				return true;
			
			case 4:
			case 5:
			case 6:
				if(total>=12) return false;
				return true;
			
			case TEN:
				// stand 3 or more card 16s with one deck
				if(total==16 && ndecks==1 && cards>=3) return false;
			default:
				if(total>=17) return false;
				return true;
		}
	}
}

// check if this had is a two card exception to basic stragegy
// if yes, set exception and return true or false for hit or stand
// this assumes hand has 3 or more cards, if not, call two card exceptions first
bool Hand::twoCardException(Deck &deck,Dealer &dealer,bool &exception)
{	// only meant for 2 card hands
	if(cards!=2)
	{	exception=false;
		return false;
	}
	
	// get variables
	int ndecks = deck.getDecks();
	int upcard = dealer.getUpcard();
	bool hits17 = dealer.getHitsSoft17();
	exception = true;
	
	// check soft and hard hands
	if(!isSoft())
	{	switch(ndecks)
		{	case 1:
				// Agrees with Griffin, Theory of Blackjack, page 20 (but only for !hits17)
				if(upcard==2)
				{	if(handIs(10,3) && !hits17) return true;
				}
				else if(upcard==3)
				{	if(handIs(8,4)) return false;
					if(handIs(7,5)) return false;
					if(handIs(6,6)) return false;	// not in Griffin because it is split instead
				}
				else if(upcard==4)
				{	if(handIs(TEN,2)) return true;
				}
				else if(upcard==6)
				{	if(handIs(10,2) && !hits17) return true;
				}
				else if(upcard==TEN)
				{	if(handIs(7,7)) return false;
				}
				break;
			
			case 2:
				if(upcard==3 && hits17)
				{	if(handIs(8,4)) return false;
					if(handIs(7,5)) return false;
					if(handIs(6,6)) return false;
				}
				else if(upcard==4)
				{	if(handIs(10,2)) return true;
				}
				break;
			
			default:
				if(upcard==4 && !hits17)
				{	if(handIs(10,2)) return true;
				}
				break;
		}
	}
	
	// not an exception
	exception=false;
	return false;
}

// check if basic strategy says to double down after split only
bool Hand::basicDD(Deck &deck,Dealer &dealer)
{
	int ddOption=dealer.getDDAfterSplit();
	if(ddOption==DDNone) return false;
	
	int ndecks = deck.getDecks();
	int upcard = dealer.getUpcard();
	bool hits17 = dealer.getHitsSoft17();
	
	if(isSoft())
	{	// no if 10/11 only
		if(ddOption==DD10OR11) return false;
	
		// exceptions
		if(ndecks==2)
		{	if(upcard==2)
			{	if(total==7) return false;
				if(total==8 && hits17) return true;
			}
			else if(upcard==4)
			{	if(total==3) return false;
				if(total==4 && !hits17) return false;
			}
			else if(upcard==6)
			{	if(total==9 && !hits17) return false;
			}
		}
		
		else if(ndecks>2)
		{	if(upcard==2)
			{	if(total==7) return false;
				if(total==8 && hits17) return true;
			}
			else if(upcard==4)
			{	if(total==3) return false;
				if(total==4) return false;
			}
			else if(upcard==6)
			{	if(total==9 && !hits17) return false;
			}
		}
		
		// basic strategy
		switch(upcard)
		{	case 2:
				if(total==7) return true;
				return false;
			
			case 3:
				if(total==7 || total==8) return true;
				return false;
			
			case 4:
				if(total>=9 || total==2) return false;
				return true;
				
			case 5:
				if(total>=9) return false;
				return true;
			
			case 6:
				if(total>=10) return false;
				return true;
				
			default:
				return false;
		}
	}
	else
	{	if(total>11) return false;
		if(total<10 && ddOption==DD10OR11) return false;
		
		// exceptions
		if(ndecks==1)
		{	// Agrees with Griffin, Theory of Blackjack, page 20 (but only for !hits17)
			// No exception on 5 when hits17 is new here and it is very close (.13002 DD vs .129947 Hit)
			if(handIs(6,2))
				if(upcard==6 || (upcard==5 && !hits17)) return false;
		}
		else if(ndecks==2)
		{	if(total==8 && (upcard==5 || upcard==6)) return false;
			if(upcard==ACE && !hits17)
			{	if(handIs(9,2)) return false;
				if(handIs(8,3)) return false;
			}
		}
		else if(ndecks>2)
		{	if(total==9 && upcard==2) return false;
			if(total==8 && (upcard==5 || upcard==6)) return false;
			if(upcard==ACE && total==11 && !hits17) return false;
		}
		
		switch(upcard)
		{	case 2:
			case 3:
			case 4:
				if(total<9) return false;
				return true;
			
			case 5:
			case 6:
				if(total<8) return false;
				return true;
			
			case 7:
			case 8:
			case 9:
				if(total<10) return false;
				return true;
			
			// T and A
			default:
				if(total<11) return false;
				return true;
		}
	}
}

// check two card hand to see if it should be hit
bool Hand::twoCardHit(Deck &deck,Dealer &dealer)
{	bool exception;
	bool hit = twoCardException(deck,dealer,exception);
	if(exception) return hit;
	return basicHit(deck,dealer);
}

#pragma mark Hand: Accessors

// total for hand, accounting for if it is soft
int Hand::getTotal(void) { return isSoft() ? total+10 : total ; }

// index into dealerplayer expected values arrays array (>ExVal21 means busted)
int Hand::getPlayerIndex(void)
{	int sum=getTotal();
	return sum<16 ? ExVal16 : sum-16;
}

// is it a soft hand
bool Hand::isSoft(void) { return (total<12 && aces>0) ; }

// check if hand is soft 17 (for dealer use)
bool Hand::isSoft17(void) { return total==7 && aces>0 ; }

// has split hand been doubled
float Hand::getDoubled(void) { return doubled; }
void Hand::setDoubled(float bet) { doubled=bet; }
float Hand::getBetPerHand(void) { return doubled; }

// return card required to make this hand a natural (or zero if none)
int Hand::getNaturalCard(void)
{	if(cards>1) return 0;
	if(total==ACE || total==TEN) return 11-total;
	return 0;
}

// check if 2-card 21 or natural
bool Hand::isNatural(void) { return getTotal()==21 && cards==2; }

// return first card
int Hand::getFirstCard(void) { return firstCard; }

// check two card hand (only valid for 2-card hands)
bool Hand::handIs(int c1,int c2)
{	return total==c1+c2 && (firstCard==c1 || firstCard==c2);
}

// set order of hands in exact splitting
void Hand::setOrder(int num) { order=num; }

// display hand info
void Hand::display(void)
{	cout << "(";
	if(cards==1)
		cout << cardChar(firstCard);
	else if(cards==2)
		cout << cardChar(firstCard) << "," << cardChar(total-firstCard);
	else
		cout << "cards=" << cards << ", 1st=" << firstCard;
	if(isSoft())
		cout << " soft ";
	else
		cout << " hard ";
	cout << getTotal();
	if(doubled>1.1) cout << ", dbled";
	cout << ")";
}

// display card character
char Hand::cardChar(int num)
{	if(num==1)
		return 'A';
	else if(num==10)
		return 'T';
	else
		return '0'+(char)num;
}

