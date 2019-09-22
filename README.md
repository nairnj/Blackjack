<h1>Blackjack</h1>

<p>This project contains C++ software for many calculations of blackjack expected values. Calculations for standing,
splitting, and doubling down are fast and exact.</p>

<p>Exact calculations of splitting and resplitting are very computationally intensive. Prior to this software, no exact
calculations were available. This software makes the calculations possible because of a new algorithm
for splitting. This project includes a paper that describes algorithm development. The algorithm
sped up simple recursize blackjack programming by an estimated five orders of magntiude. As a result, calculations
that previously would take 11,000 years (on a single 3GHz processor) were reduced to 45 days. In other words,
exact splitting calculations are now possible, but still take a long time.</p>

<p>Besides expected value calculations, two software options can calculate tables used in counting theories that were
derived by Peter A. Griffin in the "Theory of Blackjack." In brief, these calculations find the <i>change</i> in expected values caused by removing one specific card from the deck.</p>

<p>See <code>Blackjack.html</code> file in the <code>documentation</code> folder for details on compiling and running the calculations.</p>
