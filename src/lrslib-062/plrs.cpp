#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <queue>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/atomic.hpp>
#include "lrslib.h"
#include "plrs.hpp"
#include <sys/time.h>




using namespace std;

//Synynchrochronization Variables
boost::mutex                            consume_mutex;
boost::condition_variable               consume;
boost::atomic<plrs_output*>             output_list;
boost::atomic<bool> 	producers_finished(false);
boost::atomic<bool>	initializing(true);
boost::mutex		cobasis_list_mutex;

//List of starting cobasis
queue<string>		cobasis_list;

//Total counts
long RAYS = 0;
long VERTICES = 0;
long BASIS = 0;
long FACETS = 0;
long INTVERTICES = 0;

lrs_mp Tnum, Tden, tN, tD, Vnum, Vden;

int INITDEPTH = 4;
int MAXTHREADS = 12;
int ESTIMATES = 0;
int SUBTREESIZE = 1000;
int SETUP = FALSE;   // generate threads but do not run them
int cobasislistsize = 0;
string INPUTFILE;
ofstream OUTSTREAM;
int PLRS_DEBUG=0;
int PLRS_DEBUG_PHASE1=0;

plrs_output * reverseList(plrs_output* head){
	plrs_output * last = head, * new_head = NULL;
	while(last) {
      		plrs_output * tmp = last;
      		last = last->next;
      		tmp->next = new_head;
      		new_head = tmp;
    	}
	return new_head;
}


void processCobasis(string cobasis){

	stringstream ss(cobasis);
	string height;
	//split string after h= and retreive height value
	getline(ss, height, '=');
	getline(ss, height, ' ');

	//Check if the cobasis is a leaf node
//	if(atoi(height.c_str()) == INITDEPTH){
	if (ESTIMATES || (atoi(height.c_str()) == INITDEPTH) ) { /* FIXME this is wrong */
		//Remove the following characters
		char chars[] = "#VRBh=facetsFvertices/rays";
                unsigned hull = FALSE;
                if (cobasis.compare(0,2,"F#")==0)
                        hull = TRUE;
                    
		for(unsigned int i = 0; i < sizeof(chars); ++i){
			cobasis.erase(remove(cobasis.begin(), cobasis.end(), chars[i]), cobasis.end());
		}
		//Split the string after facets (do not need det, indet etc. for restart)
		ss.str(cobasis);
		getline(ss, cobasis, 'I');
/* 2013.2.14 set depth to zero: hull=F between 3rd and 4th spaces in cobasis;  hull=T 2nd and 3rd*/
                unsigned found = cobasis.find(" ");
                found = cobasis.find(" ",found+1);
                if (hull == FALSE)
                   found = cobasis.find(" ",found+1);
                unsigned found1 = cobasis.find(" ",found+1);
                cobasis.replace(found+1,found1-found-1,"0");


		//Save in cobasis list as a starting point for a thread
		cobasis_list.push(cobasis);
	}
}


void copyFile(string infile, string outfile){
	ifstream input_file (infile.c_str());
	ofstream output_file (outfile.c_str());
	string line;

	if(output_file.is_open()){
		if(input_file.is_open()){
			while(input_file.good()){
				getline(input_file, line);
				output_file<<line<<endl;
			}
			input_file.close();
			output_file.close();
		}else{
			printf("Error reading input file!\n");
			exit(1);
		}
	}else{
		printf("Error creating temporary file!\n");
		exit(1);
	}
}


void doWork(int thread_number, string starting_cobasis){
	//Create a temporary .ine file 
	char * thread_file = new char[256];
	sprintf(thread_file, "%s_%d.ine", INPUTFILE.c_str(),thread_number);
	copyFile(INPUTFILE, thread_file);
	ofstream out_file (thread_file, ios::app);
/* 2013.2.14 mindepth set to zero */
	out_file<<"mindepth "<< 0 <<endl;
	out_file<<"restart "<<starting_cobasis<<endl;
	if (PLRS_DEBUG)
		out_file<<"printcobasis 1"<<endl;
	out_file.close();

	char * argv[] = { thread_file };
	lrs_main(1, argv);
	//No longer need temporary .ine file so delete it
	if(remove( thread_file ) != 0 ) printf("Error deleting thread file!\n");
	delete[] thread_file;
}



void startThread(int thread_number){

	//keep pulling starting cobasis and do work
	while(true){
		//Get cobasis list lock
		boost::unique_lock<boost::mutex> lock(cobasis_list_mutex);
		//check if starting cobasis left
		if(!cobasis_list.size())
			break;

		//There is a cobasis left store and pop from list
		string starting_cobasis = cobasis_list.front();
		cobasis_list.pop();
		//Release cobasis list lock
		lock.unlock();

		//Begin searching tree with starting cobasis
		doWork(thread_number, starting_cobasis);

	}

	
}

void findInitCobasis(){
	char * argv[] = {"init_temp.ine"};
	lrs_main(1, argv);
	//No longer need temporary ine file so delete it
	if(remove("init_temp.ine") != 0) printf("Error deleting init file!\n");
}


void processOutput(){
	// this will atomically pop everything that has been posted so far.
	// consume list is a linked list in 'reverse post order'
	plrs_output* consume_list = output_list.exchange(0,boost::memory_order_acquire);

	//Reverse list since order is important when initializing
	if(initializing){
		consume_list = reverseList(consume_list);		
	}
		

	//proccess the list of output accordingly
	while(consume_list){

		if(consume_list->type == "vertex"){
			if (OUTSTREAM == NULL)
			  printf("%s\n",consume_list->data.c_str()); 
			else  OUTSTREAM <<consume_list->data<<endl;

		}else if(consume_list->type == "ray"){
			if (OUTSTREAM == NULL)
			  printf("%s\n",consume_list->data.c_str());
			else  OUTSTREAM <<consume_list->data<<endl;

		}else if(consume_list->type =="cobasis"){
			if(initializing){
				//Initializing so process cobasis - if leaf node store in starting cobasis list
				//Note that we will not be piping initial cobasis to output
				processCobasis(consume_list->data);
			}else{
				if (OUTSTREAM == NULL)
				  printf("%s\n",consume_list->data.c_str()); 
				else  OUTSTREAM <<consume_list->data<<endl;
			}
		}else if(consume_list->type =="V cobasis"){
			if(!initializing){
				if (OUTSTREAM == NULL)
				  printf("%s\n",consume_list->data.c_str());
				else  OUTSTREAM <<consume_list->data<<endl;
			}


		}else if(consume_list->type == "facet count"){
			FACETS += atoi(consume_list->data.c_str());

		}else if(consume_list->type == "ray count"){
			RAYS += atoi(consume_list->data.c_str());

		}else if(consume_list->type == "basis count"){
			BASIS += atoi(consume_list->data.c_str());

		}else if(consume_list->type == "vertex count"){
			VERTICES += atoi(consume_list->data.c_str());

		}else if(consume_list->type == "integer vertex count"){
			INTVERTICES += atoi(consume_list->data.c_str());
			
		}else if(consume_list->type == "volume"){
                        const char * c = consume_list->data.c_str();
 			plrs_readrat(Tnum,Tden,c);
                        copy(tN,Vnum);  copy(tD,Vden);
                        linrat(tN,tD,1L,Tnum,Tden,1L,Vnum,Vden);
                       // 	cout << "volume " << prat("",Tnum,Tden) << endl;
                       // 	cout << "tvolume " << prat("",Vnum,Vden) << endl;
			
			
		}else if(consume_list->type == "options warning"){
			//Only pipe warnings if initializing otherwise they are displayed multiple times
			if(initializing){
				if (OUTSTREAM == NULL)
				  printf("%s\n", consume_list->data.c_str());
				else OUTSTREAM <<consume_list->data<<endl;
			}
		}else if(consume_list->type == "header"){
			//Only pipe headers if initializing otherwise they are displayed multiple times
			if(initializing){
				if (OUTSTREAM == NULL)
				  printf("%s\n", consume_list->data.c_str());
				else  OUTSTREAM <<consume_list->data<<endl;
			}
		}else if(consume_list->type == "debug"){
			//Print debug output if it's produced
			if (OUTSTREAM == NULL)
			 printf("%s\n", consume_list->data.c_str());
			else  OUTSTREAM << consume_list->data<<endl;
			}

		//Free memory of current consume_list node
		plrs_output* temp = consume_list->next;
		delete consume_list;
		consume_list = temp;			
	}
}

void consumeOutput() {
	while(!producers_finished) {
		processOutput();
		boost::unique_lock<boost::mutex> lock(consume_mutex);                
		// check one last time while holding the lock before blocking.
		if(!output_list && !producers_finished) consume.wait(lock);
	}
	//Producer thread(s) have finished searching so manage output_list one last time
	processOutput();
}

void notifyProducerFinished(){
	//Get consumer lock
	boost::unique_lock<boost::mutex> lock(consume_mutex);  
	producers_finished = true;
	//notify consumer thread in case it is waiting for producer
	consume.notify_one();
}


void initializeStartingCobasis(){

	printf("*Max depth of %d to initialize starting cobasis list\n",
		INITDEPTH);
        if(OUTSTREAM != NULL)
	    OUTSTREAM <<"*Max depth of "<<INITDEPTH<<" to initialize starting cobasis list"<<endl;
	
	//Copy contents of ine file to temporary file	
	copyFile(INPUTFILE, "init_temp.ine");
	ofstream init_temp_file ("init_temp.ine", ios::app);
	init_temp_file<<"maxdepth "<<INITDEPTH<<endl;
	if (ESTIMATES)
        {
                init_temp_file<<"estimates "<<ESTIMATES<<endl;
		printf("*Estimates %d\n",ESTIMATES);
                if(OUTSTREAM != NULL)
	              OUTSTREAM <<"*Estimates "<<ESTIMATES<<endl;
                if (SUBTREESIZE<1)
                       SUBTREESIZE=1000;
	        printf("*Subtreesize %d\n",SUBTREESIZE);
                init_temp_file<<"subtreesize "<<SUBTREESIZE<<endl;
                if(OUTSTREAM != NULL)
	               OUTSTREAM <<"*Subtreesize "<<SUBTREESIZE<<endl;
        }
	if (!ESTIMATES || PLRS_DEBUG)
		init_temp_file<<"printcobasis 1"<<endl;
	init_temp_file.close();
	findInitCobasis(); /* 2015.7.14, lrs_main can exit() here if bad input*/
	boost::thread consumer_init_thread(consumeOutput);
	//boost::thread producer_init_thread(findInitCobasis);

	//Wait for producer thread to find all cobasis at depth <= 1
	//producer_init_thread.join();
	//Notify init consumer thread that init producer is finished
	notifyProducerFinished();
	//wait for init consumer thread to finish building starting cobasis list
	consumer_init_thread.join();
	//finished initialization starting cobasis array
	initializing = false;
	producers_finished = false;
	printf("*Finished initializing cobasis list with %lu starting cobases\n", cobasis_list.size());
        cobasislistsize = cobasis_list.size();
	
}


int main(int argc, char* argv[]){

//      allocate mp for volume calculation

        lrs_alloc_mp(tN); lrs_alloc_mp(tD); lrs_alloc_mp(Vnum); lrs_alloc_mp(Vden);
        lrs_alloc_mp(Tnum); lrs_alloc_mp(Tden);

        itomp(ZERO,Vnum); itomp(ONE,Vden);

	struct timeval start, end;
	gettimeofday(&start, NULL);

	//Print version
	printf("*plrs:%s%s(%s)",TITLE,VERSION,ARITH);

	string outputfile;
        int firstfile=1;
	int phase1time=0;

	for (int i = 1; i < argc; i++) {
			string arg(argv[i]);
			if (arg == "-in" && i + 1 <= argc) {
				INPUTFILE = string(argv[i + 1]);
                                i++;
			} else if (arg == "-out" && i + 1 <= argc) {
				outputfile = string(argv[i + 1]);
                                i++;
			} else if (arg == "-mt"  && i + 1 <= argc) {
				MAXTHREADS = atoi(argv[i + 1]);
                                i++;
			} else if (arg == "-id"  && i + 1 <= argc){
				INITDEPTH = atoi(argv[i+1]);
                                i++;
			} else if (arg == "-deb"){
				PLRS_DEBUG=1;
			} else if (arg == "-deb1"){
				PLRS_DEBUG_PHASE1=1;
			} else if (arg == "-est" && i+1 <=argc){
				ESTIMATES = atoi(argv[i+1]);
				i++;
			} else if (arg == "-st" && i+1 <= argc){
                                SUBTREESIZE = atoi(argv[i+1]);
                                i++;
			} else if (arg == "-set" ){
				SETUP = TRUE;  // produce threads but do not run them!
                                //i++;
			} else {
                                if (firstfile==1){
                                  INPUTFILE=string(argv[i]);
                                  firstfile++;
                                } else if (firstfile==2) {
                                  outputfile=string(argv[i]);
                                  firstfile++;
                                } else {
                                   printf("Invalid arguments, please try again.\n");
			           printf("%s\n",USAGE);
				   exit(0);
				}
                              }

        }

        printf("%d processes\n",MAXTHREADS);
        printf("%s\n",AUTHOR);

	if(INPUTFILE.empty()){
		printf("No input file.\n");
		printf("%s\n",USAGE);
		exit(0);
	}

	printf("*Input taken from %s\n",INPUTFILE.c_str());

	if(!outputfile.empty()){
		OUTSTREAM.open(outputfile.c_str());
		if(!OUTSTREAM.is_open()){
			printf("Error opening output file!\n");
			exit(0);
		}
		printf("*Output written to: %s\n",outputfile.c_str());
	}
	
        if(OUTSTREAM != NULL)
            {
	     OUTSTREAM <<"*plrs:"<<TITLE<<VERSION<<"("<<ARITH<<")"<<MAXTHREADS<<" processes"<<endl<<AUTHOR<<endl;
	     OUTSTREAM <<"*Input taken from "<<INPUTFILE<<endl;
            }
	if (PLRS_DEBUG_PHASE1)
		PLRS_DEBUG=1;
	initializeStartingCobasis();
	if (PLRS_DEBUG_PHASE1)
		PLRS_DEBUG=0;

        if(SETUP == TRUE)
      { // make thread files but do not run any
	//keep pulling starting cobasis and output file
	while(cobasis_list.size()>0){

	string starting_cobasis = cobasis_list.front();
	cobasis_list.pop();

	//Create  .ine file 
	char * thread_file = new char[256];
        int threadnumber = cobasis_list.size();
	sprintf(thread_file, "%s_%d.ine", INPUTFILE.c_str(),threadnumber);
	copyFile(INPUTFILE, thread_file);
	ofstream out_file (thread_file, ios::app);
	out_file<<"mindepth "<< 0 <<endl;
	out_file<<"restart "<<starting_cobasis<<endl;
	out_file.close();

	}

        printf("*Setup terminates\n");
	
      } else {
	//Create one consumer thread to manage output 
	boost::thread consumer_thread(consumeOutput);
	//Create producer threads
	boost::thread_group producer_threads;


	    printf("*Starting %d producer thread(s) and 1 consumer thread\n",MAXTHREADS);
		gettimeofday(&end, NULL);
                phase1time = end.tv_sec - start.tv_sec;
		printf("*Phase 1 time: %d seconds\n",phase1time);
	    for(int i = 0; i < MAXTHREADS; i++){
		producer_threads.create_thread(boost::bind(startThread,i));
	    }
	
	    //wait for producer threads to finish searching subtrees
	    producer_threads.join_all();
	    //Notify consumer thread producers are finished
	    notifyProducerFinished();
	    //wait for consumer thread to finish piping output
	    consumer_thread.join();
       }   

	if (OUTSTREAM == NULL)
		printf("end\n");
	else
		OUTSTREAM <<"end"<<endl;;
        if(OUTSTREAM != NULL)
           {
	    OUTSTREAM<<"*Finished initializing cobasis list with "<<cobasislistsize<<" starting cobases"<<endl;
	    OUTSTREAM <<"*Starting "<<MAXTHREADS<<" producer thread(s) and 1 consumer thread"<<endl;
           }
	if(FACETS > 0){
                printf("%s\n", prat("*Volume=",Vnum,Vden).c_str());
		printf("*Totals: facets=%ld bases=%ld\n",FACETS,BASIS);
                if (OUTSTREAM != NULL) {
                    OUTSTREAM <<prat("*Volume=",Vnum,Vden) << endl ;
                    OUTSTREAM <<"*Totals: facets="<<FACETS<<" bases="<<BASIS<<endl;
                }
	}else{
		printf("*Totals: vertices=%ld rays=%ld bases=%ld integer-vertices=%ld\n",VERTICES,RAYS,BASIS,INTVERTICES);
                if (OUTSTREAM != NULL)
                    OUTSTREAM<<"*Totals: vertices="<<VERTICES<<" rays="<<RAYS<<" bases="<<BASIS<< " integer-vertices="<<INTVERTICES<<endl;
	}

       if(OUTSTREAM != NULL)
            OUTSTREAM<< "*Phase 1 time: "<< phase1time <<
                        " seconds"<<endl;
  	gettimeofday(&end, NULL); 
	printf("*Elapsed time: %ld seconds\n", end.tv_sec  - start.tv_sec);
        if (OUTSTREAM != NULL)
           {
	         OUTSTREAM <<"*Elapsed time: "<<end.tv_sec  - start.tv_sec<<" seconds."<<endl;
           }

        lrs_clear_mp(tN); lrs_clear_mp(tD); lrs_clear_mp(Vnum); lrs_clear_mp(Vden);
        lrs_clear_mp(Tnum); lrs_clear_mp(Tden);

	return 0;
}

void post_output(const char *type, const char *data){
	plrs_output *o = new plrs_output;
	o->type = type;
	o->data = data;
        if (PLRS_DEBUG)
                cout <<o->data<<endl;
        //atomically post the output to the list.
        plrs_output* stale_head = output_list.load(boost::memory_order_relaxed);
        do {
                o->next = stale_head;
        }while( !output_list.compare_exchange_weak( stale_head, o, boost::memory_order_release ) );

        // Because only one thread can post the 'first output', only that thread will attempt
        // to aquire the lock and therefore there should be no contention on this lock except
        // when *this thread is about to block on a wait condition.  
        if( !stale_head ) {
                boost::unique_lock<boost::mutex> lock(consume_mutex);
                consume.notify_one();
        }
}
