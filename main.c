#include <stdio.h>
#include <unistd.h>

#include "async.h"

static i64 x = 0;

int main(void)
{
	asyncInit(3);

	ASYNC({
		for (int i = 0; i < 1000000; i++) {
			x += 1;
		}
	});

	Async *state;
	ASYNC2(state, {
		sleepu(500 * 1000);
		printf("%ld\n", x);
	});

	for (int i = 0; i < 1000000; i++) {
		x -= 1;
	}

	//asyncAwait(state);
	asyncCancel(state);

	puts("everythings done");

	asyncDeinit();
	return 0;
}
