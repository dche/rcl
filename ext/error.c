
#include "capi.h"

static VALUE rb_eOpenCL;

static VALUE
rcl_error_init(VALUE self, VALUE code)
{
    if (TYPE(code) != T_STRING) {
        rb_raise(rb_eArgError, "Expected a String.");
    }
    // decode error code from string.
    VALUE errcode = rb_funcall(code, rb_intern("to_i"), 0);
    rb_iv_set(self, "@code", errcode);

    VALUE errors = rb_cvar_get(rb_eOpenCL, rb_intern("@@messages"));
    VALUE msg = rb_hash_lookup(errors, errcode);
    if (NIL_P(msg)) {
        msg = rb_str_new2("Unrecognized CL error.");
    }
    // NOTE: MacRuby doesn't export this function. Ignore the compiler warning.
    rb_call_super(1, &msg);

    return self;
}

void
check_cl_error(cl_int errcode, int warn)
{
    if (errcode == CL_SUCCESS) return;

    const char *fmt = "%s";

    if (warn) {
        const char *msg = "unknown OpenCL error.";

        VALUE errors = rb_cvar_get(rb_eOpenCL, rb_intern("@@messages"));
        VALUE err_msg = rb_hash_lookup(errors, INT2FIX(errcode));
        if (!NIL_P(err_msg)) {
            msg = RSTRING_PTR(err_msg);
        }
        rb_warn(fmt, msg);
    } else {
        // NOTE: because rb_raise can't raise a Exception object, we have to
        //       encode error code to a string.
        // SEE: cl_error.rb::new
        fmt = "%d";
        rb_raise(rb_eOpenCL, fmt, errcode);
    }
}

void
rcl_define_class_clerror(VALUE mod)
{
    // NOTE: MacRuby's GC does not allow a static Hash object. But that's
    //       OK. Better practice is to remove all static variables (for the
    //       sake of multiple VMs in same process).
    // SEE: pointer.c - define_cl_types()
    VALUE rcl_errors = rb_hash_new();

#define RCL_DEF_CL_ERROR(errcode, errstr) \
    do { \
        rb_define_const(mod, #errcode, INT2FIX(errcode)); \
        rb_hash_aset(rcl_errors, INT2FIX(errcode), rb_str_new2(errstr)); \
    } while (0)

    RCL_DEF_CL_ERROR(CL_BUILD_PROGRAM_FAILURE, "Failure to build the program executable.");
    RCL_DEF_CL_ERROR(CL_COMPILER_NOT_AVAILABLE, "Compiler is not available.");
    RCL_DEF_CL_ERROR(CL_DEVICE_NOT_AVAILABLE, "Specified device is not currently available.");
    RCL_DEF_CL_ERROR(CL_DEVICE_NOT_FOUND, "No OpenCL devices that match the specified devices were found.");
    RCL_DEF_CL_ERROR(CL_IMAGE_FORMAT_MISMATCH, "Specified source and destination images are not valid image objects.");
    RCL_DEF_CL_ERROR(CL_IMAGE_FORMAT_NOT_SUPPORTED, "Specified image format is not supported.");
    RCL_DEF_CL_ERROR(CL_INVALID_ARG_INDEX, "Invalid argument index.");
    RCL_DEF_CL_ERROR(CL_INVALID_ARG_SIZE, "Invalid argument size.");
    RCL_DEF_CL_ERROR(CL_INVALID_ARG_VALUE, "Invalid argument value.");
    RCL_DEF_CL_ERROR(CL_INVALID_BINARY, "Invalid binary.");
    RCL_DEF_CL_ERROR(CL_INVALID_BUFFER_SIZE, "Invalid buffer size.");
    RCL_DEF_CL_ERROR(CL_INVALID_BUILD_OPTIONS, "Invalid build options.");
    RCL_DEF_CL_ERROR(CL_INVALID_COMMAND_QUEUE, "Invalid command queue.");
    RCL_DEF_CL_ERROR(CL_INVALID_CONTEXT, "Invalid context.");
    RCL_DEF_CL_ERROR(CL_INVALID_DEVICE, "Invalid device.");
    RCL_DEF_CL_ERROR(CL_INVALID_DEVICE_TYPE, "Invalid device type.");
    RCL_DEF_CL_ERROR(CL_INVALID_EVENT, "Invalie event object.");
    RCL_DEF_CL_ERROR(CL_INVALID_EVENT_WAIT_LIST, "Invalid event wait list.");
    RCL_DEF_CL_ERROR(CL_INVALID_GL_OBJECT, "Invalie GL object.");
    RCL_DEF_CL_ERROR(CL_INVALID_GLOBAL_OFFSET, "Argument global_work_offset shall not be nil.");
    RCL_DEF_CL_ERROR(CL_INVALID_HOST_PTR, "Invalid host pointer.");
    RCL_DEF_CL_ERROR(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR, "Invalid image format descriptor.");
    RCL_DEF_CL_ERROR(CL_INVALID_IMAGE_SIZE, "Invalid image size.");
    RCL_DEF_CL_ERROR(CL_INVALID_KERNEL_NAME, "Specified kernel name is not found in program.");
    RCL_DEF_CL_ERROR(CL_INVALID_KERNEL, "Specified kernel is not a valid kernel object.");
    RCL_DEF_CL_ERROR(CL_INVALID_KERNEL_ARGS, "Invalid kernel arguments.");
    RCL_DEF_CL_ERROR(CL_INVALID_KERNEL_DEFINITION, "Invalid kernel definition.");
    RCL_DEF_CL_ERROR(CL_INVALID_MEM_OBJECT, "Invalid memory, image, or buffer object.");
    RCL_DEF_CL_ERROR(CL_INVALID_OPERATION, "Invalid operation.");
    RCL_DEF_CL_ERROR(CL_INVALID_PLATFORM, "A parameter is not a valid platform.");
    RCL_DEF_CL_ERROR(CL_INVALID_PROGRAM, "Specified program is not a valid program object.");
    RCL_DEF_CL_ERROR(CL_INVALID_PROGRAM_EXECUTABLE, "Invalid program executable.");
    RCL_DEF_CL_ERROR(CL_INVALID_QUEUE_PROPERTIES, "Invalid queue properties.");
    RCL_DEF_CL_ERROR(CL_INVALID_SAMPLER, "Invalid sampler.");
    RCL_DEF_CL_ERROR(CL_INVALID_VALUE, "Invalid value.");
    RCL_DEF_CL_ERROR(CL_INVALID_WORK_DIMENSION, "Invalid work dimension value.");
    RCL_DEF_CL_ERROR(CL_INVALID_WORK_GROUP_SIZE, "Invalid work group size.");
    RCL_DEF_CL_ERROR(CL_INVALID_WORK_ITEM_SIZE, "Invalid work item size.");
    RCL_DEF_CL_ERROR(CL_MAP_FAILURE, "Failed to map the requested region into the host address space.");
    RCL_DEF_CL_ERROR(CL_MEM_OBJECT_ALLOCATION_FAILURE, "Failed to allocate memory for data store associated with image or buffer objects specified as arguments to kernel.");
    RCL_DEF_CL_ERROR(CL_MEM_COPY_OVERLAP, "Source and destination images are the same image (or the source and destination buffers are the same buffer), and the source and destination regions overlap.");
    RCL_DEF_CL_ERROR(CL_OUT_OF_HOST_MEMORY, "Failed to allocate resources required by the OpenCL implementation on the host.");
    RCL_DEF_CL_ERROR(CL_OUT_OF_RESOURCES, "Failed to queue the execution instance of given kernel on the command-queue because of insufficient resources needed to execute the kernel.");
    RCL_DEF_CL_ERROR(CL_PROFILING_INFO_NOT_AVAILABLE, "CL_QUEUE_PROFILING_ENABLE flag is not set for the command-queue and the profiling information is currently not available.");
#ifdef CL_VERSION_1_1
    RCL_DEF_CL_ERROR(CL_MISALIGNED_SUB_BUFFER_OFFSET, "The subbuffer's origin and/or offest values is not aligned to the CL_DEVICE_MEM_BASE_ADDR_ALIGN value of the device.");
    RCL_DEF_CL_ERROR(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST, "The execution status of any of the events in event_wait_list is a negative integer value.");
#endif

    rb_eOpenCL = rb_define_class_under(mod, "CLError", rb_eRuntimeError);
    rb_define_method(rb_eOpenCL, "initialize", rcl_error_init, 1);
    rb_define_class_variable(rb_eOpenCL, "@@messages", rcl_errors);
}
