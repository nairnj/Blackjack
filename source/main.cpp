/*********************************************************************
    main.cpp
    Blackjack coding main entry file
	Created by John Nairn on July 28, 2008.
	
    Copyright (c) 2008 John A. Nairn, All rights reserved.
	see http://www.geditcom.com/Blackjack for documentation
*********************************************************************/

#include "Hand.hpp"
#include "Deck.hpp"
#include "Dealer.hpp"

// error codes
enum { noErr=0, FileAccessErr, BadOptionErr, MemoryErr };

// Griffin table
enum { noGriffin,fullDeckGriffin,upcardRemovedGriffin };

// global defaults
bool hitsSoft17=false;
int ddFlag=DDAny;
bool ddAfterSplit=true;
bool resplitting=false;
char *outfile=NULL;
int ndecks=1;
int upstart=ACE;
int upend=TEN;
int cacheCards=0;
bool verbose=false;
bool standCalcs=false;
bool hitCalcs=false;
bool doubleCalcs=false;
bool comboCalcs=false;
bool approxSplitCalcs=false;
bool exactSplitCalcs=false;
bool exactRecursiveSplitCalcs=false;
int residCalcs=noGriffin;
int maxSplitHands=2;
int maxRecursiveSplitHands=2;

// prototypes
void produceTable(ostream &,Dealer &);
void Usage(void);
char *NextArgument(int,char * const [],int,char);

// main entry point
int main (int argc, char * const argv[])
{
	unsigned int parmInd,optInd;
	char *parm;
	
	// read options parameters
	for(parmInd=1; parmInd<argc; parmInd++)
	{	// for each one, look at each character
		for(optInd=0; optInd<strlen(argv[parmInd]); optInd++)
		{	char opt=argv[parmInd][optInd];
		
			// optional '-' ignored
			if(opt=='-' && optInd==0)
				continue;
				
			// get help
			else if(opt=='?')
			{	Usage();
				return noErr;
			}
			
			// output file name
			else if(opt=='o')
			{	parm=NextArgument(++parmInd,argv,argc,opt);
				if(parm==NULL) return BadOptionErr;
				if(outfile!=NULL) delete [] outfile;
				outfile=new char[strlen(parm)+1];
				strcpy(outfile,parm);
				break;
			}
			
			else if(opt=='c')
			{	parm=NextArgument(++parmInd,argv,argc,opt);
				if(parm==NULL) return BadOptionErr;
				sscanf(parm,"%d",&cacheCards);
				if(cacheCards<0 || cacheCards>MAX_CACHE_SIZE)
				{   cerr << "Dealer cache size is out of range (0-" << MAX_CACHE_SIZE << ")" << endl;
					return BadOptionErr;
				}
				break;
			}
			
			else if(opt=='v')
				verbose=true;
			
			else if(opt=='S')
				standCalcs=true;
			
			else if(opt=='H')
				hitCalcs=true;
			
			else if(opt=='D')
				doubleCalcs=true;
			
			else if(opt=='A')
				approxSplitCalcs=true;
			
			else if(opt=='C')
				comboCalcs=true;
			
			else if(opt=='G')
				residCalcs=fullDeckGriffin;
			
			else if(opt=='g')
				residCalcs=upcardRemovedGriffin;
			
			else if(opt=='E' || opt=='R')
			{	if(opt=='E')
					exactSplitCalcs=true;
				else
					exactRecursiveSplitCalcs=true;
				optInd++;
				if(optInd>=strlen(argv[parmInd]))
				{	cerr << "Maximum number of hands must follow exact splitting option immediately" << endl;
					return BadOptionErr;
				}
				char numopt=argv[parmInd][optInd];
				int handParm = (int)numopt-(int)'0';
				if(handParm<2 || handParm>9)
				{	cerr << "Maximum number of exact split hands must be 2 to 9" << endl;
					return BadOptionErr;
				}
				if(opt=='E')
					maxSplitHands = handParm;
				else
					maxRecursiveSplitHands = handParm;
			}
			
			else if(opt=='i' || opt=='f' || opt=='d')
			{	optInd++;
				if(optInd>=strlen(argv[parmInd]))
				{	cerr << "Numerical option (dealer up card or desks) must follow option immediately" << endl;
					return BadOptionErr;
				}
				char numopt=argv[parmInd][optInd];
				int num = (numopt=='T' || numopt=='t') ? TEN : (int)numopt-(int)'0';
				if(num<ACE || num>TEN || (num>8 && opt=='d'))
				{	cerr << "Invalid dealer up card (1-9 or T) or number of decks (1-8) setting" << endl;
					return BadOptionErr;
				}
				if(opt=='i')
					upstart=num;
				else if(opt=='f')
					upend=num;
				else if(opt=='d')
					ndecks=num;
			}
			
			else if(opt=='h')
				hitsSoft17=true;
			
			else if(opt=='s')
				hitsSoft17=false;
				
			else if(opt=='l')
				ddFlag=DDAny;
				
			else if(opt=='r')
				ddFlag=DD10OR11;
				
			else if(opt=='n')
				ddAfterSplit=false;
				
			else if(opt=='m')
				resplitting=true;
				
			else
			{   cerr << "Unknown Blackjack option '" << argv[parmInd][optInd] << "' was used" << endl;
				return BadOptionErr;
			}
		}
	}
	
	// need something
	if(!standCalcs && !hitCalcs && !doubleCalcs && !approxSplitCalcs && !exactSplitCalcs && 
				!exactRecursiveSplitCalcs && !comboCalcs && residCalcs==noGriffin)
	{	cerr << "No calculation options were selected" << endl;
		return BadOptionErr;
	}
	
	// output file
	ofstream fout;
	if(outfile!=NULL)
	{	fout.open(outfile);
		if(!fout)
		{	cerr << "Output file '" << outfile << "' could not be created" << endl;
			return FileAccessErr;
		}
	}
	else
		verbose=false;
	
	// get the stream
	ostream os((outfile!=NULL) ? fout.rdbuf() : cout.rdbuf());
	
	// create the dealer
	Dealer theDealer(hitsSoft17,cacheCards);
	cout << "Dealer cache size = " << theDealer.getCacheSize() << " in " << 
				theDealer.getCacheBytes() << " bytes" << endl;
	
	// get the table
	clock_t startTicks=clock();
	produceTable(os,theDealer);
	double clockTime=(double)(clock()-startTicks)/(double)CLOCKS_PER_SEC;
	cout << "Calculation time " << clockTime << " secs" << endl;

    return noErr;
}

// table of standing, hitting, and doubling down expected values
void produceTable(ostream &os,Dealer &dealer)
{
	int c1,c2,upcard;
	Hand hand;
	
	// intialize
	Deck theDeck(ndecks);
	
	int step = (upstart<=upend) ? 1 : -1 ;
	for(upcard=upstart; upcard!=upend+step; upcard+=step)
	{	dealer.setUpcard(upcard,theDeck);
	
		if(verbose)
			cout << "Dealer Up Card " << upcard << endl;
		os << "\nDealer Up Card " << upcard << endl;
		os << "Number of Decks: " << theDeck.getDecks() << endl;
		if(dealer.getHitsSoft17())
			os << "Dealer hits soft 17" << endl;
		else
			os << "Dealer stands soft 17" << endl;
		if(upcard==ACE || upcard==TEN)
			os << "Results conditioned on dealer not having blackjack" << endl;
	
		// expected values for standing
		if(standCalcs)
		{	if(verbose)
				cout << "... standing" << endl;
			os << "\nSTANDING" << endl;
			os << "hand\t1\t2\t3\t4\t5\t6\t7\t8\t9\t10" << endl;
			for(c1=1; c1<=10; c1++)
			{	os << c1 ;
				for(c2=1;c2<=c1;c2++)
				{	// standing expected values
					hand.reset(c1,c2,theDeck);
					if(hand.isNatural())
						os << "\t1.5";
					else
						os << "\t" << hand.standExval(theDeck,dealer) ;
					theDeck.restore(c1,c2);
				}
				os << endl;
			}
		}
		
		// expected value for hitting and then following basic strategy
		if(hitCalcs)
		{	if(verbose)
				cout << "... hitting" << endl;
			os << "\nHITTING" << endl;
			os << "hand\t1\t2\t3\t4\t5\t6\t7\t8\t9\t10" << endl;
			
			for(c1=1; c1<=10; c1++)
			{	os << c1  ;
				for(c2=1;c2<=c1;c2++)
				{	// hitting expected values
					hand.reset(c1,c2,theDeck);
					os << "\t" << hand.hitExval(theDeck,dealer) ;
					theDeck.restore(c1,c2);
				}
				os << endl;
			}
		}
		
		// expected value for hitting and then following basic strategy
		if(doubleCalcs)
		{	if(verbose)
				cout << "... doubling down" << endl;
			os << "\nDOUBLING DOWN" << endl;
			os << "hand\t1\t2\t3\t4\t5\t6\t7\t8\t9\t10" << endl;
			for(c1=ACE; c1<=TEN; c1++)
			{	os << c1 ;
				for(c2=1;c2<=c1;c2++)
				{	// standing expected values
					hand.reset(c1,c2,theDeck);
					os << "\t" << hand.doubleExval(theDeck,dealer) ;
					theDeck.restore(c1,c2);
				}
				os << endl;
			}
		}
		
		/* Combo calcs - max expected value among stand, hit, DD, split according to rules setting
			Use (l)as vegas or (r)eno to set DD any two or just 10 & 11
			Default is DD after split (on hands allowed in previous setting), Use (n)o for no DD after split
			Default is no replitting, use (m)ultiple hands for replitting to 4 hands allowed
			Set cache size
		*/
		if(comboCalcs)
		{	if(verbose)
				cout << "... maximum of hit, stand, double" << endl;
			os << "\nOPTIMAL STRATEGY: (H)it, (S)tand, (D)ouble, or S(P)lit (DD ";
			if(ddFlag==DDAny)
				os << "any two cards, " ;
			else
				os << "10&11 only, " ;
			if(ddAfterSplit)
			{	os << "DD after split, ";
				dealer.setDDAfterSplit(ddFlag);
			}
			else
			{	os << "no DD after split, ";
				dealer.setDDAfterSplit(DDNone);
			}
			if(resplitting)
				os << "resplitting allowed" ;
			else
				os << "no resplitting" ;
			os << ")" << endl;
			os << "hand\t1\t2\t3\t4\t5\t6\t7\t8\t9\t10" << endl;
			for(c1=1; c1<=10; c1++)
			{	os << c1 ;
				for(c2=1;c2<=c1;c2++)
				{	float standVal=1.5,hitVal,ddVal,splitVal;
					
					hand.reset(c1,c2,theDeck);
					if(!hand.isNatural())
						standVal=hand.standExval(theDeck,dealer);
					hitVal=hand.hitExval(theDeck,dealer);
					if(ddFlag==DD10OR11 && (hand.isSoft() || (hand.getTotal()!=10 && hand.getTotal()!=11)))
						ddVal=-5.;
					else
						ddVal=hand.doubleExval(theDeck,dealer);
					
					if(c1==c2)
					{	hand.unhit(c1);
						splitVal=hand.approxSplitPlay(theDeck,dealer,resplitting && c1!=1);
						hand.hit(c1);
					}
					else
						splitVal=-5.;
						
					// get the maximum
					char strategy='S';
					if(hitVal>standVal)
					{	standVal=hitVal;
						strategy='H';
					}
					if(ddVal>standVal)
					{	standVal=ddVal;
						strategy='D';
					}
					if(splitVal>standVal)
					{	standVal=splitVal;
						strategy='P';
					}
					os << "\t" << standVal << " " << strategy;
						
					theDeck.restore(c1,c2);
				}
				os << endl;
			}
		}
		
		// approximate splitting
		if(approxSplitCalcs)
		{	if(verbose)
				cout << "... approximate splitting" << endl;
			os << "\nAPPROXIMATE SPLITTING" << endl;
			os << "RS\tNo\tNo\tNo\tYes\tYes\tYes" << endl;
			os << "DD\tNo\tAny\t10&11\tNo\tAny\t10&11" << endl;
			for(c1=ACE; c1<=TEN; c1++)
			{	os << c1 << "," << c1 ;
				
				// hand with one card, but remove second one too
				hand.reset(c1,theDeck);
				theDeck.remove(c1);
				
				// calculations ResplitNONE+ResplitALLOWED,DDNone+DDAny+DD10OR11
				float results[6];
				hand.splitCalcs(theDeck,dealer,ResplitNONE+ResplitALLOWED,DDNone+DDAny+DD10OR11,results);
				for(int i=0;i<6;i++) os << "\t" << results[i];
				
				theDeck.restore(c1,c1);
				
				os << endl;
			}
		}
		
		// exact splitting
		if(exactRecursiveSplitCalcs)
		{	if(verbose)
				cout << "... exact splitting" << endl;
			os << "\nEXACT SPLITTING (Recursive Method)" << endl;
			os << "MH\t" << maxRecursiveSplitHands << "\t" << maxRecursiveSplitHands << "\t" << maxRecursiveSplitHands << endl;
			os << "DD\tNo\tAny\t10&11" << endl;
			for(c1=ACE; c1<=TEN; c1++)
			{	os << c1 << "," << c1 ;
				
				hand.reset(c1,c1,theDeck);
				
				// calculations
				float results[3];
				hand.exactSplitCalcs(theDeck,dealer,maxRecursiveSplitHands,DDNone+DDAny+DD10OR11,results);
				for(int i=0;i<3;i++) os << "\t" << results[i];
				
				theDeck.restore(c1,c1);
				
				os << endl;
			}
		}
		
		// new exact splitting
		if(exactSplitCalcs)
		{	if(verbose)
				cout << "... exact splitting" << endl;
			os << "\nEXACT SPLITTING (Unique Hands Method)" << endl;
			os << "MH\t" << maxSplitHands << "\t" << maxSplitHands << "\t" << maxSplitHands << endl;
			os << "DD\tNo\tAny\t10&11" << endl;
			if(!theDeck.initHandHashTable(dealer))
			{	os << "Out of memory creating hand hash table for exact splitting calculations" << endl;
				return;
			}
			for(c1=ACE; c1<=TEN; c1++)
			{	os << c1 << "," << c1 ;
				
				hand.reset(c1,c1,theDeck);
				
				// calculations
				float results[3];
				hand.handExactSplitCalcs(theDeck,dealer,maxSplitHands,DDNone+DDAny+DD10OR11,results);
				for(int i=0;i<3;i++) os << "\t" << results[i];
				
				theDeck.restore(c1,c1);
				
				os << endl;
			}
		}
		
		/* residuals for counting
			Does Griffin tables, where dealer up card and player cards are NOT removed. Also hard hitting
			compares taking one card to standing.
			The hard hitting results are identical. Griffin states doubling and splitting are approximate, based
			on adjusted infinite deck results. Here doubling is exact.
			Griffin not clear on soft hitting. Here exact and compares playing to completion to standing
			Splitting is not clear. because cards need to be removed to correctly find P(2), P(3), and P(4).
			Here removed player cards (but not dealer up card) and used approximate splitting method. Results
			differ from Griffin, but may either be do to removal strategy or to approxiamte splitting methods
			Uses Combo table settings to control splitting section rules
			
			Expected Value = mean + (52*d-r-1)/(52*d-r-R) Sum(i=1,R) E(c_i)/d
				r = # cards removed in table calcs = 0 for G and 1 for g or 2 for G and 3 for g when splitting
				R = # cards removed in addition to the base cards
				d = # of decks
				E(c_i) is effect on mean on removing card c_i in the table normalized to 1 deck
		*/
		if(residCalcs!=noGriffin)
		{	if(verbose)
				cout << "... card removal effects" << endl;
			
			float minExval = -.25;
			if(residCalcs==fullDeckGriffin)
				cout << "Split pair (for splitting only) removed" << endl;
			else
				cout << "Dealer up card and split pair (for splitting only) removed" << endl;
			os << "\nhand\tmean\tA\t2\t3\t4\t5\t6\t7\t8\t9\tT" << endl;
			float ndecks=theDeck.getDecks();
			
			// restore upcard to reproduce Giffin tables
			if(residCalcs==fullDeckGriffin)
				dealer.makeUnremovable(theDeck);
			
			os << "HITTING HARD HANDS" << endl;
			for(c1=17;c1>=12;c1--)
			{	// mean
				hand.reset(10,c1-10);			// fill hand, but do not remove from theDeck
				float mean = 0.5*hand.doubleExval(theDeck,dealer)-hand.standExval(theDeck,dealer);
				os << c1 << "\t" << 100.*mean ;
				
				for(c2=ACE;c2<=TEN;c2++)
				{	// remove one card
					theDeck.remove(c2);
					float exval = 0.5*hand.doubleExval(theDeck,dealer)-hand.standExval(theDeck,dealer);
					os << "\t" << 100.*(exval-mean)*ndecks ;
					theDeck.restore(c2);
				}
				
				os << endl;
			}
			
			os << "HITTING SOFT HANDS" << endl;
			for(c1=19;c1>=17;c1--)
			{	// mean
				hand.reset(1,c1-11);
				float mean = hand.hitExval(theDeck,dealer)-hand.standExval(theDeck,dealer);
				if(mean < minExval) continue;
				os << c1 << "\t" << 100.*mean ;
				
				for(c2=ACE;c2<=TEN;c2++)
				{	// remove one card
					theDeck.remove(c2);
					float exval = hand.hitExval(theDeck,dealer)-hand.standExval(theDeck,dealer);
					os << "\t" << 100.*(exval-mean)*ndecks ;
					theDeck.restore(c2);
				}
				
				os << endl;
			}

			os << "HARD DOUBLING DOWN" << endl;
			for(c1=11;c1>=7;c1--)
			{	// mean
				hand.reset(5,c1-5);
				float mean = hand.doubleExval(theDeck,dealer)-hand.hitExval(theDeck,dealer);
				if(mean < minExval) continue;
				os << c1 << "\t" << 100.*mean ;
				
				for(c2=ACE;c2<=TEN;c2++)
				{	// remove one card
					theDeck.remove(c2);
					float exval = hand.doubleExval(theDeck,dealer)-hand.hitExval(theDeck,dealer);
					os << "\t" << 100.*(exval-mean)*ndecks ;
					theDeck.restore(c2);
				}
				
				os << endl;
			}
			
			os << "SOFT DOUBLING DOWN" << endl;
			for(c1=20;c1>=13;c1--)
			{	// mean
				hand.reset(1,c1-11);
				bool hit = hand.twoCardHit(theDeck,dealer);
				float alt = hit ? hand.hitExval(theDeck,dealer) : hand.standExval(theDeck,dealer) ;
				float mean = hand.doubleExval(theDeck,dealer)-alt;
				if(mean < minExval) continue;
				os << "A," << c1-11 << "\t" << 100.*mean ;
				
				for(c2=ACE;c2<=TEN;c2++)
				{	// remove one card
					theDeck.remove(c2);
					alt = hit ? hand.hitExval(theDeck,dealer) : hand.standExval(theDeck,dealer) ;
					float exval = hand.doubleExval(theDeck,dealer)-alt;
					os << "\t" << 100.*(exval-mean)*ndecks ;
					theDeck.restore(c2);
				}
				
				os << endl;
			}

			os << "SPLITTING (";
			if(ddAfterSplit)
			{	os << "DD after split ";
				dealer.setDDAfterSplit(ddFlag);
				if(ddFlag==DDAny)
					os << "any two cards, " ;
				else
					os << "10and 11 only, ";
			}
			else
			{	os << "no DD after split, ";
				dealer.setDDAfterSplit(DDNone);
			}
			if(resplitting)
				os << "resplitting allowed, except aces)" << endl;
			else
				os << "no resplitting)" << endl;
			for(c1=TEN;c1>=ACE;c1--)
			{	bool handResplit = c1==ACE ? false : resplitting;
			
				// mean
				hand.reset(c1,c1,theDeck);			// remove from deck too
				bool hit = hand.twoCardHit(theDeck,dealer);
				float alt = hit ? hand.hitExval(theDeck,dealer) : hand.standExval(theDeck,dealer) ;
				hand.unhit(c1);
				float mean = hand.approxSplitPlay(theDeck,dealer,handResplit)-alt;
				if(-fabs(mean) < minExval) continue;
				os << c1 << "," << c1 << "\t" << 100.*mean ;
				
				for(c2=ACE;c2<=TEN;c2++)
				{	// remove one card
					theDeck.remove(c2);
					hand.hit(c1);
					alt = hit ? hand.hitExval(theDeck,dealer) : hand.standExval(theDeck,dealer) ;
					hand.unhit(c1);
					float exval = hand.approxSplitPlay(theDeck,dealer,handResplit)-alt;
					os << "\t" << 100.*(exval-mean)*ndecks ;
					theDeck.restore(c2);
				}
				
				os << endl;
				theDeck.restore(c1,c1);
			}
			
			// remove to balance subsequent restore
			if(residCalcs==fullDeckGriffin)
				theDeck.remove(upcard);
		}

		// return upcard to the deck
		theDeck.restore(upcard);
	}
}

// print help and user message
void Usage(void)
{
	cout << "\nBlackjack (version 1.0)" << endl;
	cout << "\nUsage:\n"
			"   Blackjack [-options]\n\n"
            "This program calculates expected values for the game of blackjack.\n"
            "The table of results can include various expected values and can be\n"
            "written to standard output or diverted to a file.\n\n"
            "Calculation Options (can be grouped and '-' is optional):\n"
            "    -S                 Expected values for standing all hands\n"
            "    -H                 Expected values for hitting all hands and following\n"
			"                          basic strategy to completion\n"
            "    -D                 Expected values for doubling down all hands\n"
			"    -C                 Combo table show maximum of S, H, and D tables\n"
            "    -A                 Approximate pair splitting expected values\n"
            "    -E2                Exact pair splitting expected values to given number of hands\n"
			"                          (immediately after, 2-9)\n"
            "    -R2                Exact pair splitting expected values to given number of hands\n"
			"                          (immediately after, 2-9, uses recursive algorithm)\n"
			"    -G                 Griffin effect of card removal using a full decks\n"
			"    -g                 Griffin effect of card removal with dealer upcard removed\n"
            "\nSetting Options (can be grouped and '-' is optional):\n"
            "    -o fname           Write table to tab-delimited text file\n"
            "    -d2                Number of decks (immediately after, 1-8)\n"
            "    -h                 Dealer hits soft 17\n"
            "    -s                 Dealer stands soft 17\n"
            "    -i4                Initial dealer up card (immediately after, 1-9 or T)\n"
            "    -fT                Final dealer up card (immediately after, 1-9 or T)\n"
			"    -c num             Dealer cache size (0 to 23)\n"
			"    -l                 Double down any two cards (Las Vegas rules, Combo/Griffin tables only)\n"
			"    -r                 Double down 10 & 11 only (Reno rules, Combo/Griffin tables only)\n"
			"    -n                 No double down after splitting (Combo/Griffin tables only)\n"
 			"    -m                 Replitting allowed (Combo/Griffin tables only)\n"
            "    -v                 Verbose to list progress to std out\n"
			"                          (only applies when -o is active)\n"
            "    -?                 Print this help information\n"
			"\nDefaults:\n"
			"   No expected values, std out, 1 deck, stand soft 17, dealer up card range\n"
			"       1 to 10, dealer cache size 0, and not verbose\n"
			"\nExample: complete tables using approximate splitting methods\n"
			"   Blackjack SHDAo bjtable.txt\n"
		 << endl;
}

// grab next argument or quit with error is not found
char *NextArgument(int parmInd,char * const argv[],int argc,char option)
{
	// error if not there
	if(parmInd>=argc)
	{   cerr << "Blackjack option '" << option << "' is missing its required argument" << endl;
		return NULL;
	}
	return argv[parmInd];
}



