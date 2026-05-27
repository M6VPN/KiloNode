/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/compat_process_helper.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

int
main(int argc, char *argv[])
{
	char buf[128];
	size_t i;

	if (argc > 1 && strcmp(argv[1], "slow") == 0) {
		for (;;)
			pause();
	}
	if (argc > 1 && strcmp(argv[1], "large") == 0) {
		for (i = 0; i < 5000; i++)
			(void)putchar('A');
		return 0;
	}

	(void)fputs("helper stdout\n", stdout);
	(void)fputs("helper stderr\n", stderr);
	if (fgets(buf, sizeof(buf), stdin) != NULL)
		(void)printf("input=%s", buf);

	return 0;
}
