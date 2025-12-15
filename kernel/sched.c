/*
 * implementing the scheduler
 */

#include "sched.h"
#include "spike_interface/spike_utils.h"

process* ready_queue_head = NULL;
process* blocked_queue_head = NULL;

//
// insert a process, proc, into the END of ready queue.
//
void insert_to_ready_queue( process* proc ) {
  sprint( "going to insert process %d to ready queue.\n", proc->pid );
  // if the queue is empty in the beginning
  if( ready_queue_head == NULL ){
    proc->status = READY;
    proc->queue_next = NULL;
    ready_queue_head = proc;
    return;
  }

  // ready queue is not empty
  process *p;
  // browse the ready queue to see if proc is already in-queue
  for( p=ready_queue_head; p->queue_next!=NULL; p=p->queue_next )
    if( p == proc ) return;  //already in queue

  // p points to the last element of the ready queue
  if( p==proc ) return;
  p->queue_next = proc;
  proc->status = READY;
  proc->queue_next = NULL;

  return;
}

//
// choose a proc from the ready queue, and put it to run.
// note: schedule() does not take care of previous current process. If the current
// process is still runnable, you should place it into the ready queue (by calling
// ready_queue_insert), and then call schedule().
//
extern process procs[NPROC];
void schedule() {
  if ( !ready_queue_head ){
    // by default, if there are no ready process, and all processes are in the status of
    // FREE and ZOMBIE, we should shutdown the emulated RISC-V machine.
    int should_shutdown = 1;

    for( int i=0; i<NPROC; i++ )
      if( (procs[i].status != FREE) && (procs[i].status != ZOMBIE) ){
        should_shutdown = 0;
        sprint( "ready queue empty, but process %d is not in free/zombie state:%d\n", 
          i, procs[i].status );
      }

    if( should_shutdown ){
      sprint( "no more ready processes, system shutdown now.\n" );
      shutdown( 0 );
    }else{
      panic( "Not handled: we should let system wait for unfinished processes.\n" );
    }
  }

  current = ready_queue_head;
  assert( current->status == READY );
  ready_queue_head = ready_queue_head->queue_next;

  current->status = RUNNING;
  sprint( "going to schedule process %d to run.\n", current->pid );
  switch_to( current );
}


void insert_to_blocked_queue(process *proc) 
{
  if( proc == NULL ) return;

  if( blocked_queue_head == NULL ){
    proc->status = BLOCKED;
    proc->queue_next = NULL;
    blocked_queue_head = proc;
    return;
  }

  process *p = blocked_queue_head;
  process *prev = NULL;

  // traverse the whole queue: if found, do nothing; otherwise prev ends at tail
  for( ; p != NULL; prev = p, p = p->queue_next ){
    if( p == proc ) return;
  }

  // append to tail
  prev->queue_next = proc;
  proc->status = BLOCKED;
  proc->queue_next = NULL;
  return;
}


void remove_block_and_insert(process* proc)
{
  process *cur, *prev;
  process *target = proc->parent;

  if (blocked_queue_head == NULL || target == NULL)
    return;

  prev = NULL;
  cur = blocked_queue_head;

  while (cur != NULL) {
    if (cur == target) {
      if (prev == NULL)
        blocked_queue_head = cur->queue_next;
      else
        prev->queue_next = cur->queue_next;

      cur->status = READY;
      insert_to_ready_queue(cur);
      return;
    }
    prev = cur;
    cur = cur->queue_next;
  }
}

