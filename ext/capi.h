#ifndef RCL_CAPI_H__
#define RCL_CAPI_H__

#include <ruby.h>
#include <assert.h>

#include <OpenCL/opencl.h>

#define TRACE(...)     fprintf(stderr, __VA_ARGS__)

// NOTE: this macro needs local defined class variables 
//       take specific pattern of name, /rcl_c\w+/
#define Expect_RCL_Type(o, klass) \
    do { \
        Check_Type(o, T_DATA); \
        if (rb_class_of(o) != rcl_c##klass) { \
            rb_raise(rb_eTypeError, "Expected %s is an instance of %s.", #o, #klass); \
        } \
    } while (0)

#define Expect_Boolean(ro, var) \
    cl_bool var; \
    do { \
        if (ro != Qtrue && ro != Qfalse) \
            rb_raise(rb_eTypeError, \
                    "Expected %s is true or false.", #ro); \
            var = ro == Qtrue ? CL_TRUE : CL_FALSE; \
    } while (0)

#define Expect_Fixnum(ro) \
    do { \
        if (!FIXNUM_P(ro)) \
            rb_raise(rb_eTypeError, \
                     "Expected %s is a Fixnum.", #ro); \
    } while (0)
    
#define Expect_Array(ro) \
    do { \
        if (TYPE(ro) != T_ARRAY) \
            rb_raise(rb_eTypeError, \
                    "Expected %s is an Array.", #ro); \
    } while (0)
    
#define Expect_NonEmpty_Array(ro) \
    do { \
        Expect_Array(ro); \
        if (RARRAY_LEN(ro) == 0) \
            rb_raise(rb_eArgError, \
                    "Expected %s is not empty.", #ro); \
    } while (0)

#define Extract_Size(sizet, var) \
    size_t var; \
    do { \
        Expect_Fixnum(sizet); \
        var = FIX2UINT(sizet); \
    } while (0)

#endif  // RCL_CAPI_H__
