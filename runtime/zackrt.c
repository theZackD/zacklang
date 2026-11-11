#include "zackrt.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#define ZACK_PANIC_EXIT_CODE 101

void zack_print_i32(int32_t value) {
    printf("%" PRId32 "\n", value);
}

void zack_panic(const char* message, const char* file, int32_t line) {
    /* Flush first so standard output written before the panic is not lost. */
    fflush(stdout);
    fprintf(stderr, "panic: %s\n  --> %s:%" PRId32 "\n", message, file, line);
    fflush(stderr);
    exit(ZACK_PANIC_EXIT_CODE);
}
