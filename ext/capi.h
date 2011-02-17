#ifndef RCL_CAPI_H__
#define RCL_CAPI_H__

#include "ruby_ext.h"
#include <OpenCL/opencl.h>

// NOTE: this macro needs local defined class variables
//       using a special pattern for name, /rcl_c\w+/
#define Expect_RCL_Type(o, klass) \
    do { \
        Check_Type(o, T_DATA); \
        if (rb_class_of(o) != rcl_c##klass) { \
            rb_raise(rb_eTypeError, "expected %s is an instance of %s.", #o, #klass); \
        } \
    } while (0)

#define Extract_Boolean(ro, var) \
    cl_bool var; \
    do { \
        Expect_Boolean(ro); \
        var = ro == Qtrue ? CL_TRUE : CL_FALSE; \
    } while (0)

#endif
