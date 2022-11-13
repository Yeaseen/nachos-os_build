// preemptive.cc 
//	Extension to make kernel threads be periodically preempted
//      It only works on Linux x86 environments
//
// Copyright (c) 2007 Universidad de Las Palmas de Gran Canaria
//
// Permission to use, copy, modify, and distribute this software and its
// documentation for any purpose, without fee, and without written agreement is
// hereby granted, provided that the above copyright notice appear in all 
// copies of this software.

#ifndef PREEMPTIVE_H
#define PREEMPTIVE_H

class PreemptiveScheduler
{
  public:
    PreemptiveScheduler() {}
    ~PreemptiveScheduler() {}
    
    // Set up time slicing between kernel threads.
    //   'timeSliceLength' is the time slice duration,
    //   measured in native x86 machine instructions
    
    void SetUp ( unsigned long timeSliceLength );
};

#endif

