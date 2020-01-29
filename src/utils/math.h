
#define pow(base, exponent) __builtin_pow(base, exponent)
#define strcmp(a, b) __builtin_strcmp(a, b)
#define strncmp(a, b, num) __builtin_strncmp(a, b, num)
#define ldexp(x, exp) __builtin_ldexp(x, exp)

static inline uint64_t min_uint64(uint64_t a, uint64_t b) 
{
    uint64_t result = a < b ? a : b;
    return result;
}

static inline int64_t max_int64(int64_t a, int64_t b) 
{
    const int64_t result = a > b ? a : b;
    return result;
}

#define min(a,b) (a < b ? a : b)
#define max(a,b) (a > b ? a : b)