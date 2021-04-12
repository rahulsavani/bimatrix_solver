/*********************************************************/
/* nashdemo is a simple template for lrsnashlib.c        */
/*                                                       */
/* It builds two 3x4 matrices A B and computes           */
/* their equilibria                                      */
/*********************************************************/
/* 
Compile:
gcc -O3 -o nashdemo nashdemo.c lrsnashlib.c lrslib.c lrsgmp.c -lgmp -DGMP

Usage:
nashdemo
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lrsdriver.h"
#include "lrslib.h"
#include "lrsnashlib.h"


int main()
{
  long s,t;
  game Game;                             // Storage for one game
  game *g = &Game;
        gInfo GI= {.name="Game"};        // Input file name could go here if there is one   
        g->aux = &GI;


  if ( !lrs_init ("\n*nashdemo:"))       // Done once but essential for lrslib usage !
    return 1;


  g->nstrats[ROW]=3;               // row player
  g->nstrats[COL]=4;               // col player

  setFwidth(g,4);                  // field length for printing games

  for(s=0;s<3;s++)                 // Game 1: load payoff matrices with some integers
     for(t=0;t<4;t++)
     {
  	g->payoff[s][t][ROW].num=s+t;
  	g->payoff[s][t][COL].num=s*t;
 	g->payoff[s][t][ROW].den=1;
  	g->payoff[s][t][COL].den=1;
     }
        printGame(g);
        lrs_solve_nash(g);

  for(s=0;s<3;s++)                 // Game 2: load payoff matrices with some rationals
     for(t=0;t<4;t++)
     {
        g->payoff[s][t][ROW].num=s+t;
        g->payoff[s][t][COL].num=1;
        g->payoff[s][t][ROW].den=2;
        g->payoff[s][t][COL].den=3;
     }

        printGame(g);
        lrs_solve_nash(g);


	return 0;
}


