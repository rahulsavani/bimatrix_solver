#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/******************************************************************
December 30,2019
Program to make a cross reference list between H and V reps
Usage (same for ext file):
Add  printcobasis and incidence options to cube.ine

% lrs cube.ine cube.ext
% xref cube.ext

Edit the output file  cube.ext.x so that the second line contains two integers

rows maxindex
 
where rows >= # output lines in cube.ext.x 
      maxindex >= # input lines in cube.ine 

or just use 0 0 and the program will tell you which values to use

% hvref cube.ext.x

gives V to H and H to V cross reference tables
**********************************************************************/
int main(int argc, char *argv[]) {
  int i=0, j=0;
  int first=1;
  int Hrep=1;
  int rows=0, maxindex=0;
  int inrows=0, outrows=0;
  int rowindex;
  char s[10000];
  int x;

  int *nrow;
  int **table;

  FILE *lrs_ifp;

  if(argc > 1)
    {
     if((lrs_ifp = fopen (argv[1], "r")) == NULL)
      {
         printf ("\nBad input file name\n");
         return 1;
      }
    }
    else
      lrs_ifp=stdin;

  x=fscanf(lrs_ifp,"%s", s);
  while(first && x != EOF )
    {
      if(strcmp(s,"H-representation") == 0 || strcmp(s,"V-representation") == 0)
       {
         if(strcmp(s,"H-representation") == 0)
            printf("\nH-representation");
         else
          {
            printf("\nV-representation");
            Hrep=0;
          }
         first=0;
         fscanf(lrs_ifp,"%d %d",&rows,&maxindex);
         printf("\n%d %d",rows,maxindex);

         nrow=(int *)malloc((maxindex+2)*sizeof(int));
         table = (int **)malloc((rows+2) * (maxindex+2) * sizeof(int));
         for (i=0;i<=maxindex+1;i++)
             table[i] = (int*)malloc((rows+2) * sizeof(int));
         for(i=0;i<=maxindex;i++)
          {
           nrow[i]=0;
           for(j=0;j<=rows;j++)
             table[i][j]=0;
          }
       }
      if(first)
        x=fscanf(lrs_ifp,"%s", s);
     }
  while(x != EOF)
    {
      first=1;
      x=fscanf(lrs_ifp,"%s", s);
      while(x != EOF && strcmp(s,"#") !=0)
          {
           i=atoi(s);
           if(first==1)
             {
              printf("\n%d:",i);
              rowindex=i;
              inrows++;
             }
           else 
             {
              if(i>outrows)
               outrows=i;
              if(i <= maxindex && nrow[i] < rows)
                {
                  nrow[i]++;
                  table[i][ nrow[i]]=rowindex;
                }
              printf(" %d",i);
             }
           x=fscanf(lrs_ifp,"%s", s);
           first=0;
          }
    }

  if(inrows <= rows && outrows <= maxindex)
    {
      if(Hrep)
         printf("\n\nV-representation");
      else
         printf("\n\nH-representation");
      printf("\n%d %d",outrows,inrows);

      for(i=1;i<=outrows;i++)
          {
            printf("\n%d:",i);
            for(j=1;j<=nrow[i];j++)
                printf(" %d",table[i][j]);
          }
    }
  else
       printf("\n\nInput parameters too small, rerun with:");

  printf("\ninput rows=%d maxindex=%d",inrows,outrows);
  printf("\n");
  free(nrow);
  for (i=0;i<=maxindex+1;i++)
       free(table[i]); 
  free(table);
  return 0;
}

