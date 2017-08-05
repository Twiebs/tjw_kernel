
#define pow(base, exponent) __builtin_pow(base, exponent)
#define strcmp(a, b) __builtin_strcmp(a, b)
#define strncmp(a, b, num) __builtin_strncmp(a, b, num)
#define ldexp(x, exp) __builtin_ldexp(x, exp)

#define min(a,b) (a < b ? a : b)
#define max(a,b) (a > b ? a : b)