#ifndef UTILS_H
#define UTILS_H
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void incomplete_command(void) __attribute__((noreturn));

#define NEXT_ARG() do { argv++; if (--argc <= 0) incomplete_command(); } while(0)

int matches(const char *prefix, const char *string);

#endif // UTILS_H