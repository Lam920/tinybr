#include "utils.h"

void incomplete_command(void)
{
	fprintf(stderr, "Command line is not complete. Try option \"help\"\n");
	exit(-1);
}

int matches(const char *prefix, const char *string)
{
	if (!*prefix)
		return 1;
	while (*string && *prefix == *string) {
		prefix++;
		string++;
	}

	return *prefix;
}