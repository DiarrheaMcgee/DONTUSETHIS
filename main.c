#include <stdio.h>

#include "async.h"

static i64 x = 0;

int main(void)
{
	asyncInit();

	ASYNC({
		for (int i = 0; i < 1000000; i++) {
			x += 1;
		}
	});

	ASYNC({
		sleepu(5);
		printf("%ld\n", x);
	});

	for (int i = 0; i < 1000000; i++) {
		x -= 1;
	}

	asyncDeinit();
	return 0;
}
