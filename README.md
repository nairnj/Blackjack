# Blackjack

This project contains c++ software for many calculations of blackjack expected values. Calculations for standing,
splitting, and doubling down are fast an exact.

Exact calculations of splitting and resplitting a very computationally intensive. Prior to this software, no exact
calculations were available. This software makes the calculations possible because of a new algorithm
for splitting. This project includes a paper that describes algorithm development. The algorithm
sped up simple recursize blackjack programming by an estimated five orders of magntiude. As a result, a calculations
that previously would take 11,000 years (on a single 3Hz processor) were reduced to 45 days. In other words,
exact splitting calculations are now possible, but still take a long time.

Besides expected values, two software options can calculate tables used in counting theories that were
derived by Peter A. Griffin in the "Theory of Blackjack."
