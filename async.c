#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include "async.h"

#define ASYNC_DEFAULT_THREAD_COUNT 2
#define ASYNC_MAX_THREAD_COUNT 8
static u8 thread_count = ASYNC_DEFAULT_THREAD_COUNT;

static u8 *stack_base;
static pthread_mutex_t lock;
static bool async_initialized;
static Async states[ASYNC_MAX_THREAD_COUNT];
static size_t thread_index;
static pthread_t MAIN_THREAD;

static void *threadloop(void *_state)
{
	Async *state = _state;
	state->initialized = true;
	if (setjmp2(&state->loop) != 0) {
		pthread_mutex_lock(&lock);
		if (state->stack != NULL)
			free(state->stack);
		state->stack = NULL;
		state->has_job = false;
		pthread_mutex_unlock(&lock);
	}

	while (true) {
		if (state->should_exit) {
			state->initialized = false;
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

void asyncInitInternal(void *_stack_base, const u8 threads)
{
	stack_base = _stack_base;
	MAIN_THREAD = pthread_self();
	pthread_mutex_init(&lock, NULL);
	async_initialized = true;

	if (threads != 0) {
		if (threads > ASYNC_MAX_THREAD_COUNT)
			thread_count = ASYNC_MAX_THREAD_COUNT;
		else
			thread_count = threads;
	}

	for (int i = 0; i < thread_count; i++) {
		pthread_create(&states[i].id, NULL, threadloop, states + i);
	}
}

void asyncCancel(Async *state)
{
	if (!async_initialized || state == NULL) return;

	state->should_exit = true;
	sleepu(10);

	if (!state->initialized) {
		pthread_join(state->id, NULL);
	}
	else {
		if (pthread_cancel(state->id) < 0)
			pthread_kill(state->id, SIGKILL);
		pthread_join(state->id, NULL);
	}

	pthread_mutex_lock(&lock);

	if (state->has_job && state->stack != NULL)
		free(state->stack);

	state->id = 0;
	state->has_job = false;
	state->initialized = false;
	state->stack = NULL;

	pthread_mutex_unlock(&lock);
}

void asyncReset(Async *state)
{
	asyncCancel(state);
	pthread_create(&state->id, NULL, threadloop, state);
}

bool asyncIsMainThread(void)
{
	return (pthread_self() == MAIN_THREAD);
}

void asyncDeinit(void)
{
	for (int i = 0; i < thread_count; i++) {
		states[i].should_exit = true;
	}

	for (int i = 0; i < thread_count; i++) {
		if (states[i].id != 0)
			pthread_join(states[i].id, NULL);
	}

	pthread_mutex_destroy(&lock);
	async_initialized = false;
}

AsyncReturn asyncCreateJob(void)
{
	pthread_mutex_lock(&lock);

	ThreadState job;

	while (states[thread_index].has_job) {
		thread_index = (thread_index + 1) % thread_count;
		sleepu(1);
	}

	Async *state = &states[thread_index];

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

		AsyncReturn ret = {0};
		ret.state = state;
		ret.is_async = false;
		return ret;
	}

	AsyncReturn ret = {0};
	ret.state = state;
	ret.is_async = true;
	return ret;
}

