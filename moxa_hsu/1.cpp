#include <stdio.h>

#define GENERIC_MAX(x, y) ((x) > (y) ? (x) : (y))

#define ENSURE_int(i)   _Generic((i), int:   (i))
#define ENSURE_float(f) _Generic((f), float: (f))


#define MAX(type, x, y) \
  (type)GENERIC_MAX(ENSURE_##type(x), ENSURE_##type(y))

int main (void)
{
  int    ia = 1,    ib = 2;
  float  fa = 3.0f, fb = 4.0f;
  double da = 5.0,  db = 6.0;

  printf("%d\n", MAX(int,   ia, ib)); // ok
  printf("%f\n", MAX(float, fa, fb)); // ok

//printf("%d\n", MAX(int,   ia, fa));  compiler error, one of the types is wrong
//printf("%f\n", MAX(float, fa, ib));  compiler error, one of the types is wrong
//printf("%f\n", MAX(double, fa, fb)); compiler error, the specified type is wrong
//printf("%f\n", MAX(float, da, db));  compiler error, one of the types is wrong

//printf("%d\n", MAX(unsigned int, ia, ib)); // wont get away with this either
//printf("%d\n", MAX(int32_t, ia, ib)); // wont get away with this either
  return 0;
}