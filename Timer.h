// Project:      CSS432 UDP Socket Class
// Professor:    Munehiro Fukuda
// Organization: CSS, University of Washington, Bothell
// Date:         March 5, 2004
// Updated:      2016, Prof. Dimpsey

#ifndef _TIMER_H_
#define _TIMER_H_

#include <sys/time.h>
#include <iostream>
using namespace std;

class Timer 
{
    public:
        Timer();       
        void Start();          
        long End();               		// endTime - startTime

    private:
        struct timeval startTime;  
        struct timeval endTime;    
};

#endif
