#ifndef __THREADS_H__
#define __THREADS_H__

#include <lpc_types.h>

void thread_init(uint32_t quantaUs);
void thread_go(); // THIS WILL NEVER RETURN
int thread_create(void (*run)(void*), void* userdata);
void thread_kill(int thread_id);
void thread_self_term();

#endif
