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

#define Expect_Integer(ro) \
    do { \
        if (!FIXNUM_P(ro) && TYPE(ro) != T_BIGNUM) { \
            rb_raise(rb_eTypeError, "expected %s is an Integer.", #ro); \
        } \
    } while (0)

#define Expect_Float(ro) \
    do { \
        if (TYPE(ro) != T_FLOAT && !FIXNUM_P(ro) && TYPE(ro) != T_BIGNUM) { \
            rb_raise(rb_eTypeError, "expected %s is of type Float.", #ro); \
        } \
    } while (0)

#define Expect_Boolean(ro) \
    do { \
        if (ro != Qtrue && ro != Qfalse) { \
            rb_raise(rb_eTypeError, \
                    "expected %s is true or false.", #ro); \
        } \
    } while (0)

#define Expect_Fixnum(ro) \
    do { \
        if (!FIXNUM_P(ro)) \
            rb_raise(rb_eTypeError, \
                     "expected %s is a Fixnum.", #ro); \
    } while (0)

#define Expect_Array(ro) \
    do { \
        if (TYPE(ro) != T_ARRAY) \
            rb_raise(rb_eTypeError, \
                    "expected %s is an Array.", #ro); \
    } while (0)

#define Expect_NonEmpty_Array(ro) \
    do { \
        Expect_Array(ro); \
        if (RARRAY_LEN(ro) == 0) \
            rb_raise(rb_eArgError, \
                    "expected %s is not empty.", #ro); \
    } while (0)

#define Extract_Size(sizet, var) \
    size_t var; \
    do { \
        if (NIL_P(sizet)) { \
            var = 0; \
        } else { \
            Expect_Fixnum(sizet); \
            var = FIX2UINT(sizet); \
        } \
    } while (0)

#endif  // RCL_CAPI_H__
