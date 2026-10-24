/* kernel_check.h:  CHECK() for the kernel_* test programs. */
#ifndef KERNEL_CHECK_H
#define KERNEL_CHECK_H
#include <stdio.h>
static int check_failures = 0, check_count = 0;
#define CHECK(cond) do { check_count++; if (!(cond)) { check_failures++; \
   printf("%s:%d: FAILED: %s\n", __FILE__, __LINE__, #cond); } } while (0)
#define CHECK_STR(a, b) do { check_count++; if (strcmp((a), (b)) != 0) { check_failures++; \
   printf("%s:%d: FAILED: \"%s\" != \"%s\"\n", __FILE__, __LINE__, (a), (b)); } } while (0)
#define CHECK_INT(a, b) do { long long va_ = (long long) (a), vb_ = (long long) (b); check_count++; \
   if (va_ != vb_) { check_failures++; \
   printf("%s:%d: FAILED: %s == %lld, expected %lld\n", __FILE__, __LINE__, #a, va_, vb_); } } while (0)
static int CheckReport(const char *name)
{
   printf("%s: %d checks, %d failed\n", name, check_count, check_failures);
   return check_failures != 0;
}
#endif
