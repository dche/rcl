/* A sheer wrapper of the OpenCL API.
 *
 * More rubyish syntax is defined in ruby file. See ../opencl.rb
 *
 * TODO: add equality method (eql?) for all CL objects.
 * TODO: check all Array that might get an empty value.
 * TODO: check all arguments that could be +nil+.
 * TODO: check all value conversions.
 * TODO: Big endien support.
 */

#include "capi.h"

// CL objects
VALUE rcl_cPlatform;
VALUE rcl_cDevice;
VALUE rcl_cContext;
VALUE rcl_cImageFormat;
VALUE rcl_cCommandQueue;
VALUE rcl_cSampler;
VALUE rcl_cEvent;
VALUE rcl_cMemory;
VALUE rcl_cProgram;
VALUE rcl_cKernel;

extern void rcl_define_opencl_constants(VALUE);
extern void rcl_define_class_clerror(VALUE);
extern void rcl_define_class_platform(VALUE);
extern void rcl_define_class_device(VALUE);
extern void rcl_define_class_context(VALUE);
extern void rcl_define_class_image_format(VALUE);
extern void rcl_define_class_command_queue(VALUE);
extern void rcl_define_class_sampler(VALUE);
extern void rcl_define_class_event(VALUE);
extern void rcl_define_class_memory(VALUE);
extern void rcl_define_class_program(VALUE);
extern void rcl_define_class_kernel(VALUE);

extern void rcl_define_class_pointer(VALUE);

/*
 * Entry point
 */

void
Init_capi()
{
    VALUE rcl_mOpenCL = rb_define_module("OpenCL");
    VALUE rcl_mCapi = rb_define_module_under(rcl_mOpenCL, "Capi");

    rcl_define_opencl_constants(rcl_mOpenCL);
    rcl_define_class_clerror(rcl_mOpenCL);
    rcl_define_class_pointer(rcl_mOpenCL);

    rcl_define_class_platform(rcl_mCapi);
    rcl_define_class_device(rcl_mCapi);
    rcl_define_class_context(rcl_mCapi);
    rcl_define_class_image_format(rcl_mCapi);
    rcl_define_class_command_queue(rcl_mCapi);
    rcl_define_class_sampler(rcl_mCapi);
    rcl_define_class_event(rcl_mCapi);
    rcl_define_class_memory(rcl_mCapi);
    rcl_define_class_program(rcl_mCapi);
    rcl_define_class_kernel(rcl_mCapi);
}
