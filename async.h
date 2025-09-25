#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <assert.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;
typedef int64_t  i64;
typedef int32_t  i32;
typedef int16_t  i16;
typedef int8_t   i8;
typedef double   f64;
typedef float    f32;

#define ASYNC_DEFAULT_STACK_SIZE (1024 * 1024 * 4)

typedef struct ThreadState {
	u8     *rsp;
	u8     *rbp;
	size_t  rbx;
	size_t  rsi;
	size_t  r12;
	size_t  r13;
	size_t  r14;
	size_t  r15;
	size_t  rdi;
	void   *rip;
} ThreadState;

extern size_t setjmp2(ThreadState *state);
extern void longjmp2(ThreadState *state);

static inline void sleepu(const u64 us)
{
	struct timespec ts = {0};
	ts.tv_sec = us / (1000 * 1000);
	ts.tv_nsec = (us % 1000) * 1000;
	nanosleep(&ts, &ts);
}

typedef struct Async {
	pthread_t id;
	ThreadState loop;
	ThreadState job;
	u8 *stack;
	bool has_job;
	bool should_exit;
	bool initialized;
} Async;

typedef struct AsyncReturn {
	Async *state;
	bool is_async;
} AsyncReturn;

extern void asyncInitInternal(void *_stack_base, const u8 threads);
static inline void asyncInit(const u8 threads) // used to avoid stack offsets from call
{
	void *stack_base = NULL;
	__asm__ __volatile__("mov %%rsp, %0" : "=r"(stack_base));
	asyncInitInternal(stack_base, threads);
}

extern void asyncDeinit(void);
extern AsyncReturn asyncCreateJob(void);
extern bool asyncIsMainThread(void);
extern void asyncCancel(Async *state);
extern void asyncReset(Async *state);

#define ASYNC(block) \
do { \
	assert(asyncIsMainThread()); \
	AsyncReturn ret = asyncCreateJob(); \
	if (ret.is_async) { \
		do { \
			block \
		} while (0); \
		longjmp2(&ret.state->loop); \
	} \
} while (0)

#define ASYNC2(state, block) \
do { \
	assert(asyncIsMainThread()); \
	AsyncReturn ret = asyncCreateJob(); \
	if (ret.is_async) { \
		do { \
			block \
		} while (0); \
		longjmp2(&ret.state->loop); \
	} \
	(state) = ret.state; \
} while (0)

static inline void asyncAwait(Async *state)
{
	if (state == NULL) return;
	while (state->has_job) sleepu(1);
}

