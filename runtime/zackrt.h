#ifndef ZACKRT_H
#define ZACKRT_H

/* ZackLang C runtime library (decision D46).
 *
 * This header is the contract between the code the compiler generates and the
 * runtime. Names are prefixed with `zack_` and use the plain C calling
 * convention.
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_MSC_VER)
#define ZACK_NORETURN __declspec(noreturn)
#elif defined(__GNUC__) || defined(__clang__)
#define ZACK_NORETURN __attribute__((noreturn))
#else
#define ZACK_NORETURN
#endif

/* Writes `value` in decimal followed by a line feed to standard output. */
void zack_print_i32(int32_t value);

/* Prints a panic message with its source location to standard error and ends
 * the program with exit code 101 (decision D31). */
ZACK_NORETURN void zack_panic(const char* message, const char* file, int32_t line);

#ifdef __cplusplus
}
#endif

#endif /* ZACKRT_H */
