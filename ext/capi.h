#ifndef RCL_CAPI_H__
#define RCL_CAPI_H__

#include "ruby_ext.h"

#if defined (__APPLE__) || defined(MACOSX)
    #include <OpenCL/opencl.h>
    // Mirror types to GL types, until OpenCL on Mac OSX is 1.1.
    typedef unsigned int cl_GLuint;
    typedef int          cl_GLint;
    typedef unsigned int cl_GLenum;
#else
    #include <CL/opencl.h>
#endif

// NOTE: this macro needs local defined class variables
//       using a special pattern for name, /rcl_c\w+/
#define EXPECT_RCL_TYPE(o, klass) \
    do { \
        Check_Type(o, T_DATA); \
        if (rb_class_of(o) != rcl_c##klass) { \
            rb_raise(rb_eTypeError, "expected %s is an instance of %s.", #o, #klass); \
        } \
    } while (0)

#define EXTRACT_BOOLEAN(ro, var) \
    cl_bool var; \
    do { \
        EXPECT_BOOLEAN(ro); \
        var = ro == Qtrue ? CL_TRUE : CL_FALSE; \
    } while (0)

#endif
