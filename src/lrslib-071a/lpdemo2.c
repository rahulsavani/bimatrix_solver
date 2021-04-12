/*
lpdemo2.c         Contributed by Terje Lensberg     October 28, 2015
- Contains a C struct to represent LP problems and code to solve such LP representations with lrs
- Illustrates the interface to lrs and its use of rational numbers
- This version does only maximization
- Compile: 
  gcc -O3 -o lpdemo2 lpdemo2.c lrslib.c lrsgmp.c -lgmp -DGMP
- Usage: ./lpdemo2
*/

#include <stdio.h>
#include <string.h>
#include "lrsdriver.h"
#include "lrslib.h"

#define LE -1L        // A constraint type used here. GE and EQ are defined in lrslib.h
#define ROW 0
#define COL 1
#define MAXROW 10     // maximum number of rows
#define MAXCOL 10     // maximum number of columns

typedef struct {
	long num;
	long den;
} ratnum;             // Rational number: "num/den"

typedef struct {
	ratnum a[MAXCOL];   // Coefficients
  long ctype;         // Constraint type (GE, LE, EQ)
	ratnum rhs;         // 'b'
} lprow;              // A row in the LP problem

typedef struct {      // C struct to represent an LP problem
  long dim[2];        // Number of rows and cols
	lprow row[MAXROW];  // The rows
} lpp;

/*  An LP problem:
    max z = (3/4)*x1 +       x2 -       x3
    s.t.        2*x1 + (2/3)*x2 - (2/3)*x3 <= 1
                  x1 +     2*x2 + (1/3)*x3 <= 3/2
                  x1 +       x2 +       x3  = 1   
                  x1                       >= 0
                             x2            >= 0
                                        x3 >= 0
    Solution: x1 = 11/32, x2 = 9/16, x3 = 3/32, z = 93/128
*/

lpp LP = // The LP problem as a C struct with coefficients as rational numbers {num,den}
{ {7, 3},                                   // Dimensions
  { // -------- a --------- ctype  rhs      // lprow contents
    { {{3,4}, {1,1}, {-1,1}}, GE, {0,1} },  // Objective function (goes in row 0 with constraint type GE) 
    { {{2,1}, {2,3}, {-2,3}}, LE, {1,1} },  // Inequality constraints
    { {{1,1}, {2,1}, { 1,3}}, LE, {3,2} }, 
    { {{1,1}, {1,1}, { 1,1}}, EQ, {1,1} },  // Equality constraint
    { {{1,1}, {0,1}, { 0,1}}, GE, {0,1} },  // Lower bounds
    { {{0,1}, {1,1}, { 0,1}}, GE, {0,1} },
    { {{0,1}, {0,1}, { 1,1}}, GE, {0,1} }
  }
};

//----------------------------------------------------------------------------------------//
// Build an lrs representation from the C struct:
// - Replace LE constraints with GE by multiplying LE rows by -1
// - Multiply the RHS column by -1 and move it in front of matrix a
// - For each row, call lrs_set_row() with the associated (and possibly modified) constraint type
void buildLP (lrs_dic *P, lrs_dat *Q, lpp *lp) 
{
	long num[MAXCOL];
	long den[MAXCOL];
	long i, j, sgn;
	long m = lp->dim[ROW];
	long n = lp->dim[COL];
	lprow *row;
  
	for (i=0; i<m; i++) {
		row = lp->row + i;
		sgn = row->ctype == LE ? -1L : 1L; // sgn = 1 for EQ too
		if(sgn < 0L)   // ctype == LE. Multiply row by -1 and change ctype to GE
			row->ctype = GE;
    // RHS
		num[0] = -sgn*row->rhs.num; // Opposite sign in column 0
		den[0] = row->rhs.den;
		// Coef. matrix
		for(j=0; j<n; j++) { 
			num[j+1] = sgn*row->a[j].num;  // j+1: RHS goes in column 0
		  den[j+1] = row->a[j].den;
		}
		// Specify constraint type and set row
		lrs_set_row(P,Q,i,num,den,row->ctype);
/*
		{ // Uncomment this to print the lrs input representation of the LP problem
      char out[40];
			printf("\n");
			for(j=0;j<n+1;j++) {
				if(den[j] == 1)
					sprintf(out, "%ld", num[j]);
				else
					sprintf(out, "%ld/%ld", num[j], den[j]);
				printf(" %7s", out);
			}	
			printf("    ctype=%s", row->ctype == GE ? "GE" : "EQ");
		}	// end print
*/
	}
	printf("\n");
}

//----------------------------------------------------------------------------------------//
// This is a slightly modified version of main() in lpdemo.c
int lp_solve (lpp *lp) 
{
  lrs_dic *P;	/* structure for holding current dictionary and indices  */
  lrs_dat *Q;	/* structure for holding static problem data             */
  lrs_mp_vector output;	/* one line of output:ray,vertex,facet,linearity */
  long col;	/* output column index for dictionary            */

// allocate and init structure for static problem data
  Q = lrs_alloc_dat ("LRS globals");
  if (Q == NULL)
     return 1;
  strcpy(Q->fname,"lpdemo2");
  Q->m = lp->dim[ROW]-1;   // Rows, excluding the objective function  
	Q->n = 1+lp->dim[COL];   // Columns, including RHS which goes in column 0
  Q->lponly = TRUE;        // we do not want all vertices generated!
	Q->maximize = TRUE;

  output = lrs_alloc_mp_vector (Q->n);

  P = lrs_alloc_dic (Q);   // allocate and initialize lrs_dic
  if (P == NULL)
      return 1;

  // Build the LP representation in the format required by lrs 
  buildLP(P,Q,lp);

// Solve the LP
	if (!lrs_solve_lp(P,Q))
    return 1;

// Print output
  prat ("\nObjective value = ", P->objnum, P->objden);

  for (col = 0; col < Q->n; col++)
    if (lrs_getsolution (P, Q, output, col))
      lrs_printoutput (Q, output);

/* free space : do not change order of next lines! */
  lrs_clear_mp_vector (output, Q->n);
  lrs_free_dic (P,Q);         /* deallocate lrs_dic */
  lrs_free_dat (Q);           /* deallocate lrs_dat */

  return 0;
} 


//----------------------------------------------------------------------------------------//
int main(void) 
{
	lpp *lp = &LP;

/* Global initialization - done once */
  if ( !lrs_init ("\n*lpdemo2:"))
    return 1;

	lp_solve(lp);

  lrs_close ("lp:");
  printf("\n");
}

