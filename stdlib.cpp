#include "stdlib.hpp"

#define used __attribute__((used))

used void *memset(void *s, int c, size_t n)
{
  void *dest = s;
  asm ("rep stosb" : "+D" (dest), "+c" (n) : "a" (c) : "memory");
  return s;
}

used void *memcpy(void * __restrict__ d, const void * __restrict__ s, size_t n)
{
  void *dest = d;
  asm ("rep movsb" : "+D" (dest), "+S" (s), "+c" (n) :: "memory");
  return d;
}
