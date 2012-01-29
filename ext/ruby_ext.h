#ifndef RUBY_EXT_H__
#define RUBY_EXT_H__

#include <ruby.h>
#include <assert.h>
#include <strings.h>
#include <alloca.h>

#define TRACE(...) \
    do { \
        fprintf(stderr, __VA_ARGS__); \
        fprintf(stderr, "\n"); \
    } while(0)

#define EXPECT_INTEGER(ro) \
    do { \
        if (!FIXNUM_P(ro) && TYPE(ro) != T_BIGNUM) { \
            rb_raise(rb_eTypeError, "expected %s is an Integer.", #ro); \
        } \
    } while (0)

#define EXPECT_FLOAT(ro) \
    do { \
        if (TYPE(ro) != T_FLOAT && !FIXNUM_P(ro) && TYPE(ro) != T_BIGNUM) { \
            rb_raise(rb_eTypeError, "expected %s is of type Float.", #ro); \
        } \
    } while (0)

#define EXPECT_BOOLEAN(ro) \
    do { \
        if (ro != Qtrue && ro != Qfalse) { \
            rb_raise(rb_eTypeError, \
                    "expected %s is true or false.", #ro); \
        } \
    } while (0)

#define EXPECT_FIXNUM(ro) \
    do { \
        if (!FIXNUM_P(ro)) \
            rb_raise(rb_eTypeError, \
                     "expected %s is a Fixnum.", #ro); \
    } while (0)

#define EXPECT_ARRAY(ro) \
    do { \
        if (TYPE(ro) != T_ARRAY) \
            rb_raise(rb_eTypeError, \
                    "expected %s is an Array.", #ro); \
    } while (0)

#define EXPECT_NON_EMPTY_ARRAY(ro) \
    do { \
        EXPECT_ARRAY(ro); \
        if (RARRAY_LEN(ro) == 0) \
            rb_raise(rb_eArgError, \
                    "expected %s is not empty.", #ro); \
    } while (0)

#define EXTRACT_SIZE(sizet, var) \
    size_t var; \
    do { \
        if (NIL_P(sizet)) { \
            var = 0; \
        } else { \
            EXPECT_FIXNUM(sizet); \
            var = FIX2INT(sizet); \
        } \
    } while (0)

#ifdef HAVE_MACRUBY
#include "macruby_internal.h"
#else
#define GC_WB(dst, newval) \
    do { \
        *(void **)dst = newval; \
    } while (0)
#endif

#endif  // RUBY_EXT_H__
