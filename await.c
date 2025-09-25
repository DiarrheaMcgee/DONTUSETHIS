#include <stdio.h>
#include <unistd.h>

#include "async.h"

int main(void)
{
	asyncInit(0);

	FILE *rand = fopen("/dev/urandom", "r");
	if (rand == NULL) {
		perror("fopen");
		return 1;
	}

	FILE *null = fopen("/dev/null", "w");
	if (null == NULL) {
		perror("fopen");
		return 1;
	}

	char buf[8192] = {0};

	Async *state;
	ASYNC2(state, {
		for (int i = 0; i < 1000; i++) {
			for (int j = 0; j < 100; j++) {
				size_t n = fread(buf, 1, sizeof(buf), rand);
				if (n != sizeof(buf)) {
					perror("fread");
					break;
				}
				n = fwrite(buf, 1, n, null);
				if (n != sizeof(buf)) {
					perror("fwrite");
					break;
				}
			}
			putc('.', stdout);
			fflush(stdout);
		}
		putc('\n', stdout);
	});

	puts("started reading");
	asyncAwait(state);
	puts("everythings done");

	asyncDeinit();
	return 0;
}
