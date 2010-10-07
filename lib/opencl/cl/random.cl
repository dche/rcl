
/*
 A PRNG used in each the kernel.

 Implemented a simplified CMWC.

 Borrowed from here: http://inst.cs.berkeley.edu/~ianh/proj1.html.

 I have not verified if the author's smplification to CMWC is valid yet.

 TODO: 1. Find another PRNG with small memory trace, or
       2. Run diehard test on current one.
 */

#define RCL_MAX_RANDOM      4294967295  // 2 ^ 32 - 1
#define RCL_MAX_RANDOM_F    4294967295.0f

typedef struct {
    ulong a, b, c
} rcl_random_state;

ulong rcl_random(rcl_random_state *st)
{
    unsigned long prev = st->b;
    st->b = st->a * 1103515245 + 19811105;
    st->a = (~prev ^ (st->b >> 3)) - st->c++;
    return st->b;
}

float rcl_randomf(rcl_random_state *st)
{
    return (rcl_random(st) & RCL_MAX_RANDOM) / RCL_MAX_RANDOM_F;
}

void rcl_srandom(rcl_random_state *st, ulong seed)
{
    st->a = seed + get_global_id(0);
    st->b = 0;
    st->c = 362436;
}
