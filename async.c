#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include "async.h"

#define ASYNC_MAX_JOB_COUNT 32
#define ASYNC_MAX_STACK_COUNT 4
#define ASYNC_THREAD_COUNT 2

static u8 *stack_base;
static pthread_mutex_t lock;
static bool async_initialized;
static AsyncState states[ASYNC_THREAD_COUNT];
static size_t thread_index;

static void *threadloop(void *_state)
{
	AsyncState *state = _state;
	if (setjmp2(&state->loop) != 0) {
		pthread_mutex_lock(&lock);
		free(state->stack);
		state->has_job = false;
		pthread_mutex_unlock(&lock);
	}

	while (true) {
		if (state->should_exit) {
			pthread_exit(NULL);
		}

		if (state->has_job) {
			longjmp2(&state->job);
		}
		else {
			sleepu(2);
		}
	}

	return NULL;
}

static pthread_t MAIN_THREAD;

void asyncInitInternal(void *_stack_base)
{
	stack_base = _stack_base;
	MAIN_THREAD = pthread_self();
	pthread_mutex_init(&lock, NULL);
	async_initialized = true;

	for (int i = 0; i < ASYNC_THREAD_COUNT; i++) {
		pthread_create(&states[i].id, NULL, threadloop, states + i);
	}
}

bool asyncIsMainThread(void)
{
	return (pthread_self() == MAIN_THREAD);
}

void asyncDeinit(void)
{
	for (int i = 0; i < ASYNC_THREAD_COUNT; i++) {
		states[i].should_exit = true;
	}

	for (int i = 0; i < ASYNC_THREAD_COUNT; i++) {
		void *ret;
		pthread_join(states[i].id, &ret);
	}

	pthread_mutex_destroy(&lock);
	async_initialized = false;
}

AsyncState *asyncCreateJob(void)
{
	pthread_mutex_lock(&lock);
	
	ThreadState job;

	while (states[thread_index].has_job) {
		thread_index = (thread_index + 1) % ASYNC_THREAD_COUNT;
		sleepu(1);
	}

	AsyncState *state = &states[thread_index];

	if (setjmp2(&job) == 0) {
		void *rsp = job.rsp;
		void *rbp = job.rbp;
		ptrdiff_t rbp_offset = (ptrdiff_t)((ptrdiff_t)rbp - (ptrdiff_t)rsp);
		size_t stack_size = (size_t)stack_base - (size_t)rsp;

		u8 *stack = malloc(stack_size + ASYNC_DEFAULT_STACK_SIZE);

		void *new_rsp = stack + ASYNC_DEFAULT_STACK_SIZE - stack_size;
		void *new_rbp = (void *)((ptrdiff_t)new_rsp + rbp_offset);
		memcpy(new_rsp, rsp, stack_size);

		job.rsp = new_rsp;
		job.rbp = new_rbp;

		state->job = job;
		state->stack = stack;
		state->has_job = true;

		pthread_mutex_unlock(&lock);

		return NULL;
	}
	return state;
}

