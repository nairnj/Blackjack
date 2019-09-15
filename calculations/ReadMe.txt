When the Blackjack executable is compiled using the make options, the compiled binary is saved in this folder. You can open this folder in command line too to run calculations and save them to files in this folder.

For example, the following commands would generature complete analysis foa single deck game:

Blackjack -SHDA -c 18
Blackjack -SHDA -c 18 -h -i1f6
Blackjack -E2 -c 23
Blackjack -E3 -c 23
Blackjack -E4 -c 23
Blackjack -E2 -c 23 -h -i1f6
Blackjack -E3 -c 23 -h -i1f6
Blackjack -E4 -c 23 -h -i1f6

Warning: the -En options will take a long time to run.
