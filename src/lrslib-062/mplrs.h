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

#include "lrslib.h"

#include <mpi.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>


#define USAGE "Usage is: \n mpirun -np <number of processes> mplrs <infile> <outfile> \n or \n mpirun -np <number of processes> mplrs <infile> <outfile> -id <initial depth> -maxc <maxcobases> -maxd <depth> -lmin <int> -lmax <int> -scale <int> -maxbuf <int> -countonly -hist <file> -temp <prefix> -freq <file> -stop <stopfile> -checkp <checkpoint file> -restart <checkpoint file> -time <seconds> -stopafter <int>"

/* Default values for options. */
#define DEF_LMIN 3	/* default -lmin  */
#define DEF_LMAX -1	/* default -lmax  */
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

/* singly linked list */
typedef struct slist {
        void *data;
        slist *next;
} slist;

typedef struct outlist {
	char *type;
	char *data;
	outlist *next;
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

	msgbuf *next;
} msgbuf;

/* A structure containing the state of this process.
 * Each process has one.
 */
typedef struct mplrsv {
	/* MPI communication buffers */
	msgbuf *outgoing;
	slist *cobasis_list;

	int caughtsig; /* flag for catching a signal */
	/* counts */
	unsigned long rays;
	unsigned long vertices;
	unsigned long bases;
	unsigned long facets;
	unsigned long intvertices;
	lrs_mp Tnum, Tden, tN, tD, Vnum, Vden;

	struct timeval start, end;

	/* MPI info */
	int rank; 			/* my rank */
	int size;			/* number of MPI processes */
	int my_tag;			/* to distinguish MPI sends */
	char host[MPI_MAX_PROCESSOR_NAME]; /* name of this host */

	/* output_list */
	outlist *output_list;

	/* for convenience */
	char *tfn_prefix;
	char *tfn;
	FILE *tfile;
	int initializing;		/* in phase 1? */
	int countonly; /* countonly */
	int outnum; /* number of output lines buffered */
	int maxbuf; /* maximum number of output lines to buffer before flush */

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
	MPI_Request *sigcheck;		/* MPI_Requests for reporting signals*/

	int checkpointing;		/* are we checkpointing now? */

	/* user options */
	unsigned int lmin;		/* option -lmin */
	unsigned int lmax;		/* option -lmax */
	unsigned int scalec;		/* option -scale*/
	unsigned int initdepth;		/* option -id   */
	unsigned int maxdepth;		/* option -maxd */
	unsigned int maxcobases;	/* option -maxc */
	unsigned int time_limit;	/* option -time */
	unsigned long maxncob;		/* option -stopafter */

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
	unsigned int checkpoint;	/* do we want to checkpoint now? */

	/* other */
	int waiting_initial;		/* waiting for initial producer,
					 * hold output until after 'begin'
					 */
} consumerv;

/* MASTER and CONSUMER and INITIAL must be different */
#define MASTER 0   /* the MPI process that becomes master */
#define CONSUMER 1 /* the MPI process that becomes consumer */
#define INITIAL 2

#define CHECKFLAG -3	/* must be distinct negative values */
#define RESTARTFLAG -4

/* define DEBUG to get many mplrs debug messages */
#ifdef DEBUG
#define mprintf(a) printf a
#else
#define mprintf(a)
#endif
/* define DEBUG2 to get even more */
#ifdef DEBUG2
#define mprintf2(a) printf a
#else
#define mprintf2(a)
#endif
/* define DEBUG3 to get lots */
#ifdef DEBUG3
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
void master_checksigs(void);
void master_restart(void);
void master_checkpoint(void);
void master_checkpointfile(void);
void master_checkpointconsumer(void);
void print_histogram(timeval *, timeval *);

int mplrs_worker(void);
void clean_outgoing_buffers(void); /* shared with master */
void do_work(const int *, const char *);
void process_output(void);
void send_output(int, char *);
void process_cobasis(const char *);
inline slist *addlist(slist *, void *);
void return_unfinished_cobases(void);
char *append_out(char *, int *, const char *);
int mplrs_worker_finished(void);

int mplrs_consumer(void);
void consumer_start_incoming(void);
msgbuf *consumer_queue_incoming(int *, int);
void consumer_proc_messages(void);
int consumer_checkpoint(void);
inline int outgoing_msgbuf_completed(msgbuf *);
inline void free_msgbuf(msgbuf *);
outlist *reverse_list(outlist*);
void send_master_stats(void);
void recv_master_stats(void);
void send_counting_stats(int);
void recv_counting_stats(int);
void initial_print(void);
inline void phase1_print(void);
void final_print(void);
inline char *dupstr(const char *str);

#endif /* MPLRSH */
