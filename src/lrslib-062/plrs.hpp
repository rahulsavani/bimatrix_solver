/* 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA  02110-1335, USA.

	Ver 1.0  parallel version (plrs)

	Author: Gary Roumanis

	I have adapted the synchronous lrslib library (v.4.3.) to take advantage
	of multiple processors/cores. My goal was to keep the program portable
	and stable. Thus, I took advantage of the free BOOST (http://www.boost.org/)
	library. Moreover, to limit the introduction of bugs I have made a concious
	effort to keep the underlying lrslib code untouched. 

	Initial lrs Author: David Avis avis@cs.mcgill.ca

*/

#ifndef PLRS_HPP_INCLUDED
#define PLRS_HPP_INCLUDED

#include <string>
#include <sstream>

#define USAGE "Usage is plrs <infile> <outfile> -mt <max threads> -id <initial depth>"

struct plrs_output {
        std::string type;
        std::string data;
        plrs_output* next;
};

plrs_output * reverseList(plrs_output* head);
void processCobasis(string cobasis);
void findInitCobasis();
void copyFile(string infile, string outfile);
void doWork(int thread_number, string starting_cobasis);
void startThread(int thread_number);
void processOutput();
void consumeOutput();
void notifyProducerFinished();
void initializeStartingCobasis();

#endif
