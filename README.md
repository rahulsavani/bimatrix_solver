Bimatrix solver
---------------

This code enumerates **all** equilibria of a bimatrix game. The equilibria 
of a game is a collection of convex polytopal equilibrium components. 
The enumeration happens in two parts:

1. First lrs (http://cgm.cs.mcgill.ca/~avis/C/lrs.html) is used to enumerate all
   **extreme** equilibria

2. An implementation by Bernhard von Stengel of the Bron-Kerbosch algorithm
   (https://en.wikipedia.org/wiki/Bron%E2%80%93Kerbosch_algorithm) for finding
   maximal cliques is used on the extreme equilibria in order to find all
   equilibrium components.

For a non-degenerate game, every equilibrium component is a singleton and 
the components are exactly the extreme equilbiria.

An online web-based interface to this solver can be found at:

http://cgi.csc.liv.ac.uk/~rahul/bimatrix_solver/

Citation
--------

If this code is used in your research, please cite the following paper, where
the underlying algorithms were first presented in full:

D. Avis, G. Rosenberg, R. Savani , and B. von Stengel (2010), Enumeration of Nash Equilibria for Two-Player Games. Economic Theory 42, 9-37. Online solver available at http://banach.lse.ac.uk.

Installation
------------

The script solve_game.py uses two binaries that you should compile yourself.
Sources for those are provided in this repository.

1. create a binary lrsnash using the sources in src/lrslib-062. Newer versions 
   may be available from http://cgm.cs.mcgill.ca/~avis/C/lrs.html; this version
   is kept here as it includes some minor edits to allow compilation with clang
   on Mac OSX. One compiled and working, move the binary lrsnash to bin.

2. complile src/coclique3.c and rename and move the created binary to
   bin/clique.

Running the script
------------------

<pre>
python solve_game.py -i examples/input/75_eq.txt
</pre>

Example output
--------------

<pre>
3 x 3 payoff matrix A:

  -1   1   1
  -1  -1   1
  -1  -1  -1

3 x 3 payoff matrix B:

  1  -1  -1
  1   1  -1
  1   1   1

EE = Extreme Equilibrium, EP = Expected Payoff

Decimal Output

  EE  1  P1:  (1)  0.000000  0.000000  1.000000  EP=  -1.0  P2:  (1)  1.000000  0.000000  0.000000  EP=  1.0
  EE  2  P1:  (2)  0.000000  1.000000  0.000000  EP=  -1.0  P2:  (1)  1.000000  0.000000  0.000000  EP=  1.0
  EE  3  P1:  (3)  1.000000  0.000000  0.000000  EP=  -1.0  P2:  (1)  1.000000  0.000000  0.000000  EP=  1.0

Rational Output

  EE  1  P1:  (1)  0  0  1  EP=  -1  P2:  (1)  1  0  0  EP=  1
  EE  2  P1:  (2)  0  1  0  EP=  -1  P2:  (1)  1  0  0  EP=  1
  EE  3  P1:  (3)  1  0  0  EP=  -1  P2:  (1)  1  0  0  EP=  1

Connected component 1:
{1, 2, 3}  x  {1}
</pre>

Compiling lrsnash on a mac
--------------------------

Mac uses clang instead of gcc as standard. The following changes have been made to the version of lrslib-062 in this repo to get it to work on Mac:

1. In the makefile 

<pre>
RANLIB ?= /bin/true
</pre>

was replaced with

<pre>
RANLIB ?= /usr/bin/true
</pre>

2. The method

<pre>
int lrs_solve_nash(game * g)
</pre>

from lrsnashlib.c

specifies a return type but has a bunch of return statements of the form

<pre>
else
   return;
</pre>

which were converted to

<pre>
else
  return 0;
</pre>

in response to the errors show below (gcc ignores this error and proceeds in the obvious way, but not by clang).

<pre>
lrsnashlib.c:55:5: error: non-void function 'lrs_solve_nash' should return a value [-Wreturn-type]
    return;
    ^
lrsnashlib.c:67:5: error: non-void function 'lrs_solve_nash' should return a value [-Wreturn-type]
    return;
    ^
lrsnashlib.c:77:5: error: non-void function 'lrs_solve_nash' should return a value [-Wreturn-type]
    return;
    ^
lrsnashlib.c:89:5: error: non-void function 'lrs_solve_nash' should return a value [-Wreturn-type]
    return;
    ^
4 errors generated.
make: *** [lrsnash] Error 1
rm 2nash-GMP.o lrs-GMP.o
</pre>
