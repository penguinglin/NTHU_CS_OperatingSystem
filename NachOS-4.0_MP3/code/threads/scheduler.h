// scheduler.h 
//	Data structures for the thread dispatcher and scheduler.
//	Primarily, the list of threads that are ready to run.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "copyright.h"
#include "list.h"
#include "thread.h"

// The following class defines the scheduler/dispatcher abstraction -- 
// the data structures and operations needed to keep track of which 
// thread is running, and which threads are ready but not running.

class Scheduler {
  public:
    Scheduler();		// Initialize list of ready threads 
    ~Scheduler();		// De-allocate ready list

    void ReadyToRun(Thread* thread);	
    				// Thread can be dispatched.
    Thread* FindNextToRun();	// Dequeue first thread on the ready 
				// list, if any, and return thread.
    void Run(Thread* nextThread, bool finishing);
    				// Cause nextThread to start running
    void CheckToBeDestroyed();// Check if thread that had been
    				// running needs to be deleted
    void Print();		// Print contents of ready list
    
    // SelfTest for scheduler is implemented in class Thread

    void UpdatePriority(); // MP3: update thread priority
    //void checkReadyList(); // MP3: adjust the belonging readyList of
                           // the thread whose priority has been updated
    bool L1_Empty() { 
      return readyList_L1->IsEmpty(); 
    }
    
    bool L2_Empty() { 
      return readyList_L2->IsEmpty(); 
    }

    bool L3_Empty() { 
      return readyList_L3->IsEmpty(); 
    }

    int L1_Front_Remain() {
      if(readyList_L1->IsEmpty() == 0) return readyList_L1->Front()->get_Burst_Time() - readyList_L1->Front()->get_Use_Time();
      else return -1;
    }

  private:
    SortedList<Thread *> *readyList_L1, *readyList_L2;
    List<Thread *> *readyList_L3;  // MP3: queues of threads that are ready to run,
				// but not running
    Thread *toBeDestroyed;	// finishing thread to be destroyed
    				// by the next thread that runs
};

#endif // SCHEDULER_H
