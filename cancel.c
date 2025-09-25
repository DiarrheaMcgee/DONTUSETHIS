#include <stdio.h>

#include "async.h"

int main(void)
{
	asyncInit(0);

	Async *state;
	ASYNC2(state, {
		sleepu(1000 * 1000);
	});

	puts("started waiting");
	asyncCancel(state);
	puts("finished");

	asyncDeinit();
	return 0;
}
