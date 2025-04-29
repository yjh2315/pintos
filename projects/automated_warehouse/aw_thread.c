#include "projects/automated_warehouse/aw_thread.h"

//
// You need to read carefully threads/synch.h and threads/synch.c
// 
// In the code, a fucntion named "sema_down" implements blocking thread and 
// makes list of blocking thread
// 
// And a function named "sema_up" implements unblocing thread using blocking list
//
// You must implement blocking list using "blocking_threads" in this code.
// Then you can also implement unblocking thread.
//


struct list blocked_threads;

/**
 * A function unblocking all blocked threads in "blocked_threads" 
 * It must be called by robot threads
 */
void block_thread(){
    enum intr_level old_level;
    old_level = intr_disable ();
    list_push_back(&blocked_threads, &thread_current()->elem);      //list back에 저장.
    thread_block ();
    intr_set_level (old_level);
}

/**
 * A function unblocking all blocked threads in "blocked_threads" 
 * It must be called by central control thread
 */
void unblock_threads(){
    enum intr_level old_level;
    old_level = intr_disable ();
    while(!list_empty(&blocked_threads)){       //list가 빌 때까지 반복해서 unblock
        thread_unblock(list_entry(list_pop_front(&blocked_threads), struct thread, elem));
    }
    intr_set_level(old_level);
}