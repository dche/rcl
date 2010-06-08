// A sheer wrapper of the OpenCL API.
//
// More rubyish syntax is defined in ruby file. See ../opencl.rb
// 
// Copyright (c) 2010, Diego Che

#include <ruby.h>

#include <OpenCL/opencl.h>

/*
 * Modules.
 *
 * OpenCL::Capi
 */
static VALUE rb_mOpenCL;
static VALUE rb_mCapi;

/*
 * OpenCL::Capi::constants definition.
 */

#define RCL_DEF_CONSTANT(constant) \
    do \
        rb_define_const(rb_mCapi, #constant, INT2FIX(constant)); \
    while (0)

static void
define_opencl_constants(void)
{
    RCL_DEF_CONSTANT(CL_SUCCESS);
    
    // cl_platform_info 
    RCL_DEF_CONSTANT(CL_PLATFORM_PROFILE);
    RCL_DEF_CONSTANT(CL_PLATFORM_VERSION);
    RCL_DEF_CONSTANT(CL_PLATFORM_NAME);
    RCL_DEF_CONSTANT(CL_PLATFORM_VENDOR);
    RCL_DEF_CONSTANT(CL_PLATFORM_EXTENSIONS);
    
    // cl_device_type
    RCL_DEF_CONSTANT(CL_DEVICE_TYPE_CPU);
    RCL_DEF_CONSTANT(CL_DEVICE_TYPE_GPU);
    RCL_DEF_CONSTANT(CL_DEVICE_TYPE_ACCELERATOR);
    RCL_DEF_CONSTANT(CL_DEVICE_TYPE_DEFAULT);
    RCL_DEF_CONSTANT(CL_DEVICE_TYPE_ALL);
    
    // cl_device_info
    RCL_DEF_CONSTANT(CL_DEVICE_TYPE);
    RCL_DEF_CONSTANT(CL_DEVICE_PLATFORM);
    RCL_DEF_CONSTANT(CL_DEVICE_VENDOR_ID);
    // TODO: ... more ...
    RCL_DEF_CONSTANT(CL_DEVICE_NAME);
    RCL_DEF_CONSTANT(CL_DEVICE_VENDOR);
    RCL_DEF_CONSTANT(CL_DRIVER_VERSION);
    RCL_DEF_CONSTANT(CL_DEVICE_PROFILE);
    RCL_DEF_CONSTANT(CL_DEVICE_VERSION);
    RCL_DEF_CONSTANT(CL_DEVICE_EXTENSIONS);
    
    // cl_context_properties
    RCL_DEF_CONSTANT(CL_CONTEXT_PLATFORM);
    
    // cl_context_info
    RCL_DEF_CONSTANT(CL_CONTEXT_REFERENCE_COUNT);
    RCL_DEF_CONSTANT(CL_CONTEXT_DEVICES);
    RCL_DEF_CONSTANT(CL_CONTEXT_PROPERTIES);
    
    // error codes.
    RCL_DEF_CONSTANT(CL_INVALID_VALUE);
    RCL_DEF_CONSTANT(CL_INVALID_PLATFORM);
    RCL_DEF_CONSTANT(CL_INVALID_CONTEXT);
}

/*
 * OpenCL error code and error message.
 */
 
static VALUE rb_eOpenCL;
static VALUE rcl_errors;    // { errcode => message }

#define RCL_DEF_CL_ERROR(errcode, errstr) \
    do \
        rb_hash_aset(rcl_errors, INT2FIX(errcode), rb_str_new2(errstr)); \
    while (0)

static void
define_opencl_errors(void)
{
    rcl_errors = rb_hash_new();
    
    RCL_DEF_CL_ERROR(CL_INVALID_VALUE, "A parameter is not an expected value.");
    RCL_DEF_CL_ERROR(CL_INVALID_PLATFORM, "A parameter is not a valid platform.");
    RCL_DEF_CL_ERROR(CL_INVALID_CONTEXT, "A parameter is not a valid context.");
    
    rb_obj_freeze(rcl_errors);
}

static VALUE
rcl_error_init(VALUE self, VALUE errcode)
{
    if (FIXNUM_P(errcode) && !NIL_P(rb_hash_aref(rcl_errors, errcode))) {
        rb_iv_set(self, "cl_errcode", errcode);
    } else {
        rb_raise(rb_eArgError, "Invalid CL error code.");
    }
    return self;
}

static void
check_cl_error(cl_int errcode, int warn)
{   
    if (errcode == CL_SUCCESS) return;
    
    VALUE code = INT2FIX(errcode);
    VALUE str = rb_hash_aref(rcl_errors, code);
    VALUE errobj = Qnil;
    
    if (!warn) {
        VALUE args[] = { code };
        errobj = rb_class_new_instance(1, args, rb_eOpenCL);
    }
    
    if (NIL_P(str)) {
        char *fmt = "Unexpected error occured: [%d].";
        if (warn) {
            rb_warn(fmt, errcode);
        } else {
            rb_raise(errobj, fmt, errcode);            
        }
    } else {
        char *msg = RSTRING_PTR(str);
        if (warn) {
            rb_warn(msg);
        } else {
            rb_raise(errobj, msg);            
        }
    }
}

#define Check_And_Raise(code)   check_cl_error(code, 0)
#define Check_And_Warn(code)    check_cl_error(code, 1)

static void
define_class_clerror(void)
{
    rb_eOpenCL = rb_define_class_under(rb_mCapi, "CLError", rb_eRuntimeError);
    rb_define_method(rb_eOpenCL, "initialize", rcl_error_init, 1);
    rb_define_attr(rb_eOpenCL, "cl_errcode", 1, 0);    
}

/*
 * class Platform
 */

static VALUE rb_cPlatform;

/*
 * :nodoc:
 */
static VALUE
rcl_platform_alloc(VALUE self)
{
    rb_raise(rb_eRuntimeError, "Retrieve platform by Capi#platforms instead.");
    return Qnil;
}

/*
 * call-seq:
 *      OpenCL.platforms  -> [Platform, ..] or []
 *
 * Wraps the +clGetPlatformIDs()+.
 */
static VALUE
rcl_platforms(VALUE self)
{
    cl_uint num_p;
    cl_platform_id p_ids[16];   // CHECK: literal constant.
    int i;
    
    VALUE list = rb_ary_new();
    cl_int res = clGetPlatformIDs(16, p_ids, &num_p);   // CHECK: ditto.
    if (CL_SUCCESS == res) {
        for(i = 0; i < num_p; i++) {
            VALUE o = Data_Wrap_Struct(rb_cPlatform, 0, 0, p_ids[i]);
            rb_ary_push(list, o);
        }
    } else {
        Check_And_Warn(res);
    }
    return list;
}

/*
 * call-seq: 
 *      Platform#info(CL_PLATFORM_VENDOR) -> "NVIDIA, Inc."
 *
 * Wrapps the +clGetPlatformInfo()+.
 */

static VALUE
rcl_platform_info(VALUE self, VALUE param)
{
    if (!FIXNUM_P(param)) {
        rb_raise(rb_eArgError, "Invalid platform info type.");
    }
    
    cl_platform_info info = FIX2INT(param);
    cl_int res;
    char param_value[2048];     // CHECK: literal constant, and correct size.
    size_t param_value_size;
    
    cl_platform_id platform = DATA_PTR(self);
    res = clGetPlatformInfo(platform, 
                            info, 2048, param_value, &param_value_size);
                            // CHECK: ditto.
    
    Check_And_Raise(res);
    return rb_str_new(param_value, param_value_size);
}

static void
define_class_platform(void)
{
    rb_cPlatform = rb_define_class_under(rb_mCapi, "Platform", rb_cObject);
    rb_define_module_function(rb_mCapi, "platforms", rcl_platforms, 0);
    rb_define_alloc_func(rb_cPlatform, rcl_platform_alloc);
    rb_define_method(rb_cPlatform, "info", rcl_platform_info, 1);    
}

/*
 * class Device
 */
 
static VALUE rb_cDevice;

/*
 * :nodoc:
 */
static VALUE
rcl_device_alloc(VALUE self)
{
    rb_raise(rb_eRuntimeError, "Retrieve device by Capi#devices instead.");
    return Qnil;
}

/*
 * call-seq:
 *      Capi.devices(CL_DEVICE_TYPE_GPU, platform)  -> [Device ..]
 *
 * Wrapps +clGetDeviceIDs()+
 */
static VALUE
rcl_devices(VALUE self, VALUE type, VALUE platform)
{
    if (!FIXNUM_P(type)) {
        rb_raise(rb_eArgError, "Invalid device type.");
    }
    if (!NIL_P(platform) && TYPE(platform) != T_DATA) { // CHECK: more strict?
        rb_raise(rb_eArgError, "Invalid platform.");
    }
    
    cl_device_id d_ids[256];    // CHECK: literal constant and correct value.
    cl_uint num_id;
    cl_int res;
    
    cl_platform_id pid = NIL_P(platform) ? NULL : DATA_PTR(platform);
    cl_device_type dt = FIX2INT(type);
    res = clGetDeviceIDs(pid, dt, 256, d_ids, &num_id); // CHECK: ditto.
    
    VALUE devs = rb_ary_new();
    if (res != CL_SUCCESS) {
        Check_And_Warn(res);
    } else {
        int i;
        for (i = 0; i < num_id; i++) {
            VALUE o = Data_Wrap_Struct(rb_cDevice, 0, 0, d_ids[i]);
            rb_ary_push(devs, o);
        }
    }    
    return devs;
}

/*
 * call-seq:
 *      Device#info(CL_DEVICE_VENDOR) -> "NVIDIA, Inc."
 *
 * Wrapps +clGetDeviceInfo()+
 */
static VALUE
rcl_device_info(VALUE self, VALUE param)
{
    if (!FIXNUM_P(param)) {
        rb_raise(rb_eArgError, "Invalid device info type.");
    }
    
    cl_device_info info = FIX2INT(param);
    cl_int res;
    char param_value[2048];     // CHECK: literal constant.
    size_t param_value_size;
    
    cl_device_id device = DATA_PTR(self);
    res = clGetDeviceInfo(device, info, 2048, (void *)param_value, &param_value_size);
                          
    if (CL_SUCCESS != res) {
        Check_And_Raise(res);
    }
    
    VALUE ret = Qnil;
    switch (info) {
        // cl_device_fp_config
        case CL_DEVICE_DOUBLE_FP_CONFIG:
        // case CL_DEVICE_HALF_FP_CONFIG:   // CHECK: undefined in SL 10.6.3.
        case CL_DEVICE_SINGLE_FP_CONFIG:
            break;
        // cl_device_mem_cache_type
        case CL_DEVICE_GLOBAL_MEM_CACHE_TYPE:
            break;
        // cl_device_local_mem_type
        case CL_DEVICE_LOCAL_MEM_TYPE:
            break;
        // cl_command_queue_properties
        case CL_DEVICE_QUEUE_PROPERTIES:
            break;
        // cl_device_type
        // case CL_DEVICE_TYPE:
        // // cl_bool
        // case :
        // case :
        // case :
        // case :
        // case :
        // case :
        //     break;
        // // cl_uint
        // case :
        // case :
        // case :
        // case :
        // case :
        // case :
        //     break;
        // // cl_ulong
        // case :
        // case :
        // case :
        // case :
        // case :
        // case :
        //     break;
        // // size_t and size_t[]
        // case :
        // case :
        // case :
        // case :
        // case :
        // case :
        // case :
        // case :
        //     break;
        // cl_platform_id
        case CL_DEVICE_PLATFORM:
            break;
        default:
            ret = rb_str_new(param_value, param_value_size);
    }
    return ret;
}

/*
 * call-seq:
 *      Device#available?  -> true | false
 */
static VALUE
rcl_device_is_available(VALUE self)
{
    rb_notimplement();
    return Qtrue;
}

static void
define_class_device(void)
{
    rb_cDevice = rb_define_class_under(rb_mCapi, "Device", rb_cObject);
    rb_define_module_function(rb_mCapi, "devices", rcl_devices, 2);
    rb_define_alloc_func(rb_cDevice, rcl_device_alloc);
    rb_define_method(rb_cDevice, "info", rcl_device_info, 1);
    rb_define_method(rb_cDevice, "available?", rcl_device_is_available, 0);
    rb_define_attr(rb_cDevice, "type", 1, 0);    
}

/*
 * class Context
 */
static VALUE rb_cContext;

// WHEN DO YOU NEED A POINTER WRAPPER? WHEN YOU NEED THE ALLOC->INIT SEMANTICS
typedef struct _rcl_context_t
{
    cl_context  c;
} rcl_context;

static void
rcl_context_free(void *ptr)
{
    clReleaseContext(((rcl_context *)ptr)->c);
    free(ptr);
}

static VALUE
rcl_context_alloc(VALUE klass)
{
    rcl_context *p = ALLOC_N(rcl_context, 1);
    return Data_Wrap_Struct(klass, 0, rcl_context_free, p);
}

static void
rcl_pfn_notify(const char *errinfo, const void *private_info, size_t cb, void *user_data)
{
    // TODO: do something.
}

static void
set_context_properties(cl_context_properties *props, VALUE arr, uint len)
{
    
}

static void
set_context_device_list(cl_device_id *devs, VALUE arr, uint len)
{
    
}

/*
 * call-seq:
 *      Context.new(platform, CL_DEVICE_TYPE_GPU)
 *      Context.new(platform, devices) do |info|
 *        ...
 *      end
 *
 * Wrappes +clCreateContext()+ and +clCreateContextFromType()+
 *
 */
static VALUE
rcl_context_init(int argc, VALUE *argv, VALUE self)
{
    if (argc != 2) {
        rb_raise(rb_eArgError, "Wrong number of arguments: (%d of 2).", argc);
    }
    
    cl_context_properties *props = NULL;
    if (TYPE(argv[0]) == T_ARRAY) {
        size_t ar_len = RARRAY_LEN(argv[0]);
        if ((ar_len  % 2) != 0) {
            rb_raise(rb_eArgError, "Invalid context properties list.");
        }
        
        props = ALLOCA_N(cl_context_properties, ar_len + 1);
        set_context_properties(props, argv[0], ar_len);
        props[ar_len] = 0;
    }   
    
    VALUE devs = Qnil;
    cl_device_type dev_type = CL_DEVICE_TYPE_DEFAULT;
    
    if (TYPE(argv[1]) == T_ARRAY) {
        devs = argv[1];
    } else if(FIXNUM_P(argv[1])) {
        dev_type = FIX2INT(argv[1]);
    } else {
        rb_raise(rb_eArgError, "Invalid argument. Expected device type or device array.");
    }
    
    cl_int res;
    cl_context context;
    
    if (!NIL_P(devs)) {
        cl_uint num_dev = RARRAY_LEN(devs);
        cl_device_id *dev_ids = ALLOCA_N(cl_device_id, num_dev);
        set_context_device_list(dev_ids, devs, num_dev);
        
        context = clCreateContext(props, num_dev, dev_ids, rcl_pfn_notify, NULL, &res);
    } else {
        context = clCreateContextFromType(props, dev_type, rcl_pfn_notify, NULL, &res);
    }
    
    Check_And_Raise(res);
    
    rcl_context *pc;
    
    Data_Get_Struct(self, rcl_context, pc);
    pc->c = context;
    
    return self;
}

/*
 * call-seq:
 *      cxt = context.dup
 */

static VALUE
rcl_context_init_copy(VALUE copy, VALUE orig)
{
    if (copy == orig) return copy;
    
    if (rb_class_of(orig) != rb_cContext) {
        rb_raise(rb_eTypeError, "Wrong argument type.");
    }
    
    rcl_context *copy_p;
    rcl_context *orig_p;
    
    Data_Get_Struct(copy, rcl_context, copy_p);
    Data_Get_Struct(orig, rcl_context, orig_p);
    
    cl_int res = clRetainContext(orig_p->c);
    Check_And_Raise(res);
    
    copy_p->c = orig_p->c;
    
    return copy;
}

static VALUE
build_device_array(cl_device_id *devs, size_t cb)
{
    return rb_ary_new();
}

/*
 * call-seq:
 *      Context#info(CL_CONTEXT_REFERENCE_COUNT) -> 2
 *
 * Wrapps +clGetContextInfo()+.
 */

static VALUE
rcl_context_info(VALUE self, VALUE param)
{
    if (!FIXNUM_P(param)) {
        rb_raise(rb_eArgError, "Invalid context info.");
    }    
    cl_context_info iname = FIX2INT(param);
    
    rcl_context *pc;
    Data_Get_Struct(self, rcl_context, pc);
    
    cl_uint info[2048];
    size_t info_size;
     
    cl_int res = clGetContextInfo(pc->c, iname, 2048, (void *)info, &info_size);
    
    Check_And_Raise(res);
    
    switch (iname) {
        case CL_CONTEXT_REFERENCE_COUNT:
            return UINT2FIX(info[0]);
        case CL_CONTEXT_DEVICES:
            return build_device_array((cl_device_id *)info, info_size);
        case CL_CONTEXT_PROPERTIES:
            rb_notimplement();  // TODO: implement.
            break;
        default:
            break;
    }
    return Qnil;
}

static void
define_class_context(void)
{
    rb_cContext = rb_define_class_under(rb_mCapi, "Context", rb_cObject);
    rb_define_alloc_func(rb_cContext, rcl_context_alloc);
    rb_define_method(rb_cContext, "initialize", rcl_context_init, -1);
    rb_define_method(rb_cContext, "initialize_copy", rcl_context_init_copy, 1);
    rb_define_method(rb_cContext, "info", rcl_context_info, 1);
}

/*
 * class CommandQueue
 */
 
static VALUE rb_cCommandQueue;

static void
define_class_command_queue(void)
{
    rb_cCommandQueue = rb_define_class_under(rb_mCapi, "CommandQueue", rb_cObject);
    
}

/*
 * class Program
 */

static VALUE rb_cProgram;

static void
define_class_program(void)
{
    rb_cProgram = rb_define_class_under(rb_mCapi, "Program", rb_cObject);
}

/*
 * Entry point
 */

void 
Init_capi()
{
    rb_mOpenCL = rb_define_module("OpenCL");
    rb_mCapi = rb_define_module_under(rb_mOpenCL, "Capi");
    
    define_opencl_constants();    
    define_opencl_errors();
    
    define_class_clerror();
    
    define_class_platform();    
    define_class_device();
    define_class_context();
    define_class_command_queue();
    define_class_program();
    
}
