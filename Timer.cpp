// Project:      CSS432 UDP Socket Class
// Professor:    Munehiro Fukuda
// Organization: CSS, University of Washington, Bothell
// Date:         March 5, 2004
// Updated:      2016, Prof. Dimpsey

#include "Timer.h"


Timer::Timer( ) 
{
    startTime.tv_sec = 0;
    startTime.tv_usec = 0;
    endTime.tv_sec = 0;
    endTime.tv_usec = 0;
}

void Timer::Start( ) 
{
    gettimeofday(&startTime, NULL);
}

// Get the diff between the start and the current time 
long Timer::End() 
{
    gettimeofday(&endTime, NULL);
    long interval =
        (endTime.tv_sec - startTime.tv_sec) * 1000000 +
        (endTime.tv_usec - startTime.tv_usec);
    return interval;
}

