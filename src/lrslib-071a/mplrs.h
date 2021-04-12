/* mplrs.h: header for mplrs.c
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

Author: Charles Jordan skip@ist.hokudai.ac.jp
Based on plrs by Gary Roumanis
Initial lrs Author: David Avis avis@cs.mcgill.ca
*/

#ifndef MPLRSH
#define MPLRSH 1

#ifdef MA
#define GMP
#endif

#include "lrsdriver.h"
#include "lrslib.h"

#include <mpi.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <stdio.h>

extern FILE *lrs_ofp; /* hack to get redund final print in output file */

#define USAGE "Usage is: \n mpirun -np <number of processes> mplrs <infile> <outfile> \n or \n mpirun -np <number of processes> mplrs <infile> <outfile> -id <initial depth> -maxc <maxcobases> -maxd <depth> -lmin <int> -lmax <int> -scale <int> -maxbuf <int> -countonly -hist <file> -temp <prefix> -freq <file> -stop <stopfile> -checkp <checkpoint file> -restart <checkpoint file> -time <seconds> -stopafter <int> -redund"

/* Default values for options. */
#define DEF_LMIN 3	/* default -lmin  */
#define DEF_LMAX 0	/* default -lmax. but note orig_lmax behavior!  */
#define DEF_ID   2	/* default -id    */
#define DEF_MAXD 0	/* default -maxd  */
#define DEF_MAXC 50	/* default -maxc  */
#define DEF_MAXNCOB 0   /* default -stopafter (disabled) */
#define DEF_MAXBUF  500 /* default -maxbuf */

#define DEF_TEMP   "/tmp/" /* default prefix for temporary files
			 * use /a/b to get files /a/bfilename,
			 * use /a/b/ to get files /a/b/filename
			 */
#define DEF_INPUT  NULL	/* default input filename (or NULL)       */
#define DEF_OUTPUT NULL	/* default output filename (or NULL)	  */
#define DEF_HIST   NULL	/* default histogram filename (or NULL)   */
#define DEF_RESTART NULL/* default restart filename (or NULL)	  */
#define DEF_FREQ   NULL /* default sub-problem size filename (NULL) */
#define DEF_CHECKP NULL	/* default checkpoint filename (or NULL)  */
#define DEF_STOP   NULL	/* default stop-signal filename (or NULL) */

#define DEF_SCALEC 100	/* default multiplicative scaling factor for maxc,
			 * used when L is too large (controlled by lmax) */

#if defined(MA) || (defined(LRSLONG) && !defined(B128))
#define mplrs_init_lrs_main lrs1_main
#elif defined(LRSLONG) /* B128 */
#define mplrs_init_lrs_main lrs2_main
#elif defined(GMP)
#define mplrs_init_lrs_main lrsgmp_main
#elif defined(FLINT)
#define mplrs_init_lrs_main lrsv2_main
#elif defined(MP)
#define mplrs_init_lrs_main lrsv2_main
#endif

/* singly linked list */
typedef struct slist {
        void *data;
        struct slist *next;
} slist;

typedef struct outlist {
	char *type;
	char *data;
	struct outlist *next;
} outlist;

/* A linked-list of buffers for MPI communications.
 * req[0...count-1] correspond to buf[0...count-1]
 *
 * When req[i] completes, should free buf[i].
 * When all reqs complete, should free buf, req, tags,sizes,types.
 */
typedef struct msgbuf {
	MPI_Request *req;
	void **buf;
	int count;
	int target;
	int data;  /* optional, use yourself if needed for something */
	int queue;  /* if 1, send items 1...count after 0 has completed */
	/* queue pointers must be NULL or something free()able */
	int *tags;  /* tags to use on items 1...count if queued */
	int *sizes; /* sizes of sends if queued */
	MPI_Datatype *types; /* types of sends if queued */

	struct msgbuf *next;
} msgbuf;

/* A structure containing the state of this process.
 * Each process has one.
 */
typedef struct mplrsv {
	/* MPI communication buffers */
	msgbuf *outgoing;
	slist *cobasis_list;
	long (*lrs_main)(int, char **, lrs_dic **, lrs_dat **, long, long, char *, lrs_restart_dat *);
	lrs_dic *P;
	lrs_dat *Q;
	lrs_restart_dat *R;

	int caughtsig; /* flag for catching a signal */
	unsigned int abortinit; /* lrs_main stage 0 (setup) failed? */
	unsigned int overflow; /* 0: lrslong 1:lrslong2 2:lrsgmp */
	/* counts */
	unsigned long long rays;
	unsigned long long vertices;
	unsigned long long bases;
	unsigned long long facets;
	unsigned long long linearities;
	unsigned long long intvertices;
	unsigned long long deepest;
        unsigned long long nredundcol;
	lrs_mp Tnum, Tden, tN, tD, Vnum, Vden;

	struct timeval start, end;

	/* MPI info */
	int rank; 			/* my rank */
	int size;			/* number of MPI processes */
	int my_tag;			/* to distinguish MPI sends */
	char host[MPI_MAX_PROCESSOR_NAME]; /* name of this host */

	/* output_list */
	outlist *output_list;
	outlist *ol_tail;

	char *finalwarn;	/* for process_output "finalwarn" */
	int finalwarn_len;	/* length allocated for finalwarn */
	char *curwarn;		/* to discard "finalwarn" messages on */
	int curwarn_len;	/* overflow, preventing duplicates    */
	/* for convenience */
	const char *tfn_prefix;
	char *tfn;
	FILE *tfile;
	int initializing;		/* in phase 1? */
	int countonly; /* countonly */
	int outnum; /* number of output lines buffered */
	int maxbuf; /* maximum number of output lines to buffer before flush */
	int outputblock; /* temporarily prevent a maxbuf-based output flush */
	int redund; /* bool: is this a redund run? */

	char *input_filename;		/* input filename */
	char *input;			/* buffer for contents of input file */
} mplrsv;

/* A structure for variables only the master needs */
typedef struct masterv {
	slist *cobasis_list;		/* list of work to do (L) */
	unsigned long tot_L;		/* total size of L (total # jobs) */
	unsigned long size_L;		/* current size of L (for histograms
					 * and scaling)
					 */
	unsigned long num_empty;	/* number of times L became empty */
	unsigned int num_producers; 	/* number of producers running */
	unsigned int *act_producers;    /* whether each producer owes us
					 * remaining bases message.
					 * Needed only for histograms.
					 */
	unsigned int live_workers;      /* number that haven't exited */
	/* MPI communication buffers */
	int *workin; 			/* incoming messages from producers
					   desiring work */
	MPI_Request *mworkers;		/* MPI_Requests for these messages */
	msgbuf *incoming;		/* incoming cobases from producers */
	float *sigbuf; 			/*incoming signal/termination requests*/
	MPI_Request *sigcheck;		/* MPI_Requests for reporting these*/

	int checkpointing;		/* are we checkpointing now? */
	int cleanstop;			/* was a cleanstop requested? */
	int messages;			/* do we want to set R->messages? */

	/* user options */
	unsigned int lmin;		/* option -lmin */
	unsigned int lmax;		/* option -lmax */
	int orig_lmax;			/*user changed lmax? if not,lmax=lmin*/
	unsigned int scalec;		/* option -scale*/
	unsigned int initdepth;		/* option -id   */
	unsigned int maxdepth;		/* option -maxd */
	unsigned int maxcobases;	/* option -maxc */
	unsigned int time_limit;	/* option -time */
	unsigned long maxncob;		/* option -stopafter */
	int lponly;			/* bool for -lponly option */
	int redund;			/* bool for -redund option */
	int max_redundworker;		/* max id for a worker
					 * used if m>np-2
					 */
	/* files */
	char *hist_filename;		/*histogram filename (or NULL)*/
	FILE *hist;
	int doing_histogram;		/* are we doing a histogram? */
	char *freq_filename;		/*are we outputting sub-problem sizes?*/
	FILE *freq;
	char *restart_filename;		/* restart from a checkpoint */
	FILE *restart;	
	char *checkp_filename;		/* filename to save checkpoint*/
	FILE *checkp;
	char *stop_filename;		/* option -stop */
	FILE *stop;
	FILE *input;
} masterv;

/* A structure for variables only the consumer needs */
typedef struct consumerv {
	/* MPI communication buffers */
	  /* for headers */
	MPI_Request *prodreq;    	/* consumer keeps an open request
					 * for each producer and master
					 */
	int *prodibf;			/* and two ints as a receive buffer */
	  /* for content */
	msgbuf *incoming; 		/* incoming MPI communication buffers */

	/* output */
	char *output_filename;		/* output filename (or NULL) */
	FILE *output;			/* output file (NULL for stdout) */

	/* status */
	unsigned int num_producers;	/* number of producers still going */

	/* other */
	unsigned int oflow_flag;	/* 0: no overflow message yet */
	int *overflow;			/* number of overflowed workers*/
	int waiting_initial;		/* waiting for initial producer,
					 * hold output until after 'begin'
					 */
	int final_print;		/* do the final print? (bool) */
	long *redineq;     /* bool vector for redund, which rows redundant */
	int final_redundcheck;		/* are we in the final redund check? */
} consumerv;

/* MASTER and CONSUMER and INITIAL must be different */
#define MASTER 0   /* the MPI process that becomes master */
#define CONSUMER 1 /* the MPI process that becomes consumer */
#define INITIAL 2

#define CHECKFLAG -3	/* must be distinct negative values */
#define RESTARTFLAG -4
#define STOPFLAG -5

/* define MDEBUG to get many mplrs debug messages */
#ifdef MDEBUG
#define mprintf(a) printf a
#else
#define mprintf(a)
#endif
/* define MDEBUG2 to get even more */
#ifdef MDEBUG2
#define mprintf2(a) printf a
#else
#define mprintf2(a)
#endif
/* define MDEBUG3 to get lots */
#ifdef MDEBUG3
#define mprintf3(a) printf a
#else
#define mprintf3(a)
#endif

/* function prototypes */
void mplrs_init(int, char **);
void mplrs_caughtsig(int);
void master_sendfile(void);
void mplrs_initstrucs();
void mplrs_commandline(int, char **);
void mplrs_initfiles(void);
void bad_args(void);
int mplrs_fallback(void);

int mplrs_master(void);
void send_work(int, int);
void recv_producer_lists(void);
void process_returned_cobases(msgbuf *);
void setparams(int *);
void check_stop(void);
void master_stop_consumer(int);
void master_checksigs(void);
void master_restart(void);
void master_checkpoint(void);
void master_checkpointfile(void);
void master_checkpointconsumer(void);
void print_histogram(struct timeval *, struct timeval *);

int mplrs_worker(void);
void mplrs_worker_init(void);
void clean_outgoing_buffers(void); /* shared with master */
void do_work(const int *, char *);
void worker_report_overflow(void);
void process_output(void);
void process_curwarn(void);
void send_output(int, char *);
void process_cobasis(const char *);
slist *addlist(slist *, void *);
void return_unfinished_cobases(void);
char *append_out(char *, int *, const char *);
int mplrs_worker_finished(void);

int mplrs_consumer(void);
void consumer_start_incoming(void);
msgbuf *consumer_queue_incoming(int *, int);
void consumer_proc_messages(void);
int consumer_checkpoint(void);
int outgoing_msgbuf_completed(msgbuf *);
void free_msgbuf(msgbuf *);
outlist *reverse_list(outlist*);
void send_master_stats(void);
void recv_master_stats(void);
void send_counting_stats(int);
void recv_counting_stats(int);
void initial_print(void);
void phase1_print(void);
void consumer_setredineq(void);
void final_print(void);
char *dupstr(const char *str);

int okay_to_flush(void);
void post_output(const char *, const char *);
void open_outputblock(void);
void close_outputblock(void);
void mplrs_cleanstop(int);
void mplrs_emergencystop(const char *);
void overflow_cleanup(void);
void set_restart(const int *, char *);
#endif /* MPLRSH */
