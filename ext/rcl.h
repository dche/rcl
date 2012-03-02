
#ifndef RCL_H__
#define RCL_H__

#include "ruby_ext.h"

#if defined (__APPLE__) || defined(MACOSX)
    #include <OpenCL/opencl.h>
#else
    #include <CL/opencl.h>
#endif

#if defined __GNUC__ && __GNUC__ >= 4
#pragma GCC visibility push(default)

void Init_capi();

#pragma GCC visibility pop
#endif

#endif
