// A sheer wrapper of the OpenCL API.
//
// More rubyish syntax is defined in ruby file. See ../opencl.rb
// 
// Copyright (c) 2010, Diego Che

#include <ruby.h>
#include <assert.h>

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
    
    // cl_command_queue_properties
    RCL_DEF_CONSTANT(CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);
    RCL_DEF_CONSTANT(CL_QUEUE_PROFILING_ENABLE);
    
    // cl_command_queue_info
    RCL_DEF_CONSTANT(CL_QUEUE_CONTEXT);
    RCL_DEF_CONSTANT(CL_QUEUE_DEVICE);
    RCL_DEF_CONSTANT(CL_QUEUE_REFERENCE_COUNT);
    RCL_DEF_CONSTANT(CL_QUEUE_PROPERTIES);
    
    // error codes.
    RCL_DEF_CONSTANT(CL_INVALID_VALUE);
    RCL_DEF_CONSTANT(CL_INVALID_PLATFORM);
    RCL_DEF_CONSTANT(CL_INVALID_CONTEXT);
}


// CHECK: add equality check for all classes?
// TODO: add macros for converting between VALUE and cl_xxx pointers.

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
    // VALUE errobj = Qnil;
    // 
    // if (!warn) {
    //     VALUE args[] = { code };
    //     errobj = rb_class_new_instance(1, args, rb_eOpenCL);
    // }
    // TODO: find a way to raise an Exception object?
    if (NIL_P(str)) {
        char *fmt = "Unexpected error occured: [%d].";
        if (warn) {
            rb_warn(fmt, errcode);
        } else {
            rb_raise(rb_eOpenCL, fmt, errcode);            
        }
    } else {
        char *msg = RSTRING_PTR(str);
        if (warn) {
            rb_warn(msg);
        } else {
            rb_raise(rb_eOpenCL, msg);            
        }
    }
}

#define Check_And_Raise(code)   (check_cl_error(code, 0))
#define Check_And_Warn(code)    (check_cl_error(code, 1))
// NOTE: this macro needs local defined class variables 
//       take specific pattern of name, /rb_c\w+/
#define Check_RCL_Type(o, klass) \
    do { \
        Check_Type(o, T_DATA); \
        if (rb_class_of(o) != rb_c##klass) { \
            rb_raise(rb_eTypeError, "Expected %s is instance of %s.", #o, #klass); \
        } \
    } while (0)

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

// cl ptr -> ruby object
#define RPLATFORM(ptr)      (Data_Wrap_Struct(rb_cPlatform, 0, 0, (ptr)))
// ruby object -> cl ptr
#define PLATFORM_PTR(ro)    (DATA_PTR(ro))

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
            VALUE o = RPLATFORM(p_ids[i]);
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
    
    cl_platform_id platform = PLATFORM_PTR(self);
    res = clGetPlatformInfo(platform, 
                            info, 2048, (void *)param_value, &param_value_size);
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

#define RDEVICE(ptr)      Data_Wrap_Struct(rb_cDevice, 0, 0, (ptr))
#define DEVICE_PTR(ro)    DATA_PTR(ro)

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
    if (!NIL_P(platform)) Check_RCL_Type(platform, Platform);
    
    cl_device_id d_ids[256];    // CHECK: literal constant and correct value.
    cl_uint num_id;
    cl_int res;
    
    cl_platform_id pid = NIL_P(platform) ? NULL : PLATFORM_PTR(platform);
    cl_device_type dt = FIX2INT(type);
    res = clGetDeviceIDs(pid, dt, 256, d_ids, &num_id); // CHECK: ditto.
    
    VALUE devs = rb_ary_new();
    if (res != CL_SUCCESS) {
        Check_And_Warn(res);
    } else {
        int i;
        for (i = 0; i < num_id; i++) {
            VALUE o = RDEVICE(d_ids[i]);
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
    
    cl_device_id device = DEVICE_PTR(self);
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

#define RCONTEXT(ptr, lhs) \
    do { \
        rcl_context_t *p = ALLOC_N(rcl_context_t, 1); \
        p->c = (ptr); \
        lhs = Data_Wrap_Struct(rb_cContext, 0, rcl_context_free, p); \
    } while (0)

#define CONTEXT_PTR(ro, lhs) \
    do { \
        rcl_context_t *p; \
        Data_Get_Struct(ro, rcl_context_t, p); \
        lhs = p->c; \
    } while (0)

// WHEN DO YOU NEED A POINTER WRAPPER? WHEN YOU NEED THE ALLOC->INIT SEMANTICS
typedef struct {
    cl_context  c;
} rcl_context_t;

static void
rcl_context_free(void *ptr)
{
    clReleaseContext(((rcl_context_t *)ptr)->c);
    free(ptr);
}

static VALUE
rcl_context_alloc(VALUE klass)
{
    VALUE ret;
    RCONTEXT(NULL, ret);
    
    assert(CLASS_OF(ret) == klass);
    
    return ret;
}

static void
rcl_pfn_notify(const char *errinfo, const void *private_info, size_t cb, void *user_data)
{
    // TODO: do something.
}

static void
set_context_properties(cl_context_properties *props, VALUE arr, uint len)
{
    int i;
    for (i = 0; i < len; i += 2) {
        VALUE pn = rb_ary_entry(arr, i);
        VALUE ptr = rb_ary_entry(arr, i + 1);
        
        assert(!(NIL_P(pn) || NIL_P(ptr)));
        
        props[i] = NUM2LONG(pn);    // CHECK: should all use NUM2LONG, LONG2NUM?
                                    // Depends on the size of CL constants.
        switch (props[i]) {
            case CL_CONTEXT_PLATFORM:
                props[i+1] = (cl_context_properties)DATA_PTR(ptr);
                break;
            default:
                rb_raise(rb_eArgError, "Invalid context property.");
        }
    }
}

static void
set_context_device_list(cl_device_id *devs, VALUE arr, uint len)
{
    int i;
    for (i = 0; i < len; i++) {
        VALUE dev = rb_ary_entry(arr, i);
        Check_RCL_Type(dev, Device);
        
        devs[i] = DEVICE_PTR(dev);
    }
}

static VALUE
build_device_array(cl_device_id *devs, size_t cb)
{
    VALUE ret = rb_ary_new();
    size_t num_dev = cb / sizeof(cl_device_id);
    
    int i;
    for (i = 0; i < num_dev; i++) {
        VALUE dev = RDEVICE(devs[i]);
        rb_ary_push(ret, dev);
    }
    
    return ret;
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
rcl_context_init(VALUE self, VALUE parg, VALUE darg)
{
    cl_context_properties *props = NULL;
    if (TYPE(parg) == T_ARRAY) {
        size_t ar_len = RARRAY_LEN(parg);
        if ((ar_len  % 2) != 0) {
            rb_raise(rb_eArgError, "Invalid context properties list.");
        }
        
        props = ALLOCA_N(cl_context_properties, ar_len + 1);
        set_context_properties(props, parg, ar_len);
        props[ar_len] = 0;
    }   
    
    VALUE devs = Qnil;
    cl_device_type dev_type = CL_DEVICE_TYPE_DEFAULT;
    
    if (TYPE(darg) == T_ARRAY && RARRAY_LEN(darg) > 0) {
        devs = darg;
    } else if(FIXNUM_P(darg)) {
        dev_type = FIX2INT(darg);
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
    
    rcl_context_t *pc;
    
    Data_Get_Struct(self, rcl_context_t, pc);
    pc->c = context;
    
    return self;
}

/*
 * call-seq:
 *      cxt = context.dup
 *
 * Use +clRetainContext()+.
 */

static VALUE
rcl_context_init_copy(VALUE copy, VALUE orig)
{
    if (copy == orig) return copy;
    Check_RCL_Type(orig, Context);
    
    rcl_context_t *copy_p;
    rcl_context_t *orig_p;
    
    Data_Get_Struct(copy, rcl_context_t, copy_p);
    Data_Get_Struct(orig, rcl_context_t, orig_p);
    
    if (copy_p->c == orig_p->c) return copy;
    
    cl_int res;
    if (copy_p->c != NULL) {
        res = clReleaseContext(copy_p->c);
        Check_And_Raise(res);
    }
    res = clRetainContext(orig_p->c);
    Check_And_Raise(res);
    
    copy_p->c = orig_p->c;
    
    return copy;
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
    
    cl_context cxt;
    CONTEXT_PTR(self, cxt);
    
    char info[512];
    size_t info_size;
     
    cl_int res = clGetContextInfo(cxt, iname, 512, (void *)info, &info_size);
    
    Check_And_Raise(res);
    
    switch (iname) {
        case CL_CONTEXT_REFERENCE_COUNT:
            return INT2NUM(*(int *)info);
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
    rb_define_method(rb_cContext, "initialize", rcl_context_init, 2);
    rb_define_method(rb_cContext, "initialize_copy", rcl_context_init_copy, 1);
    rb_define_method(rb_cContext, "info", rcl_context_info, 1);
}

/*
 * class CommandQueue
 */
 
static VALUE rb_cCommandQueue;

#define COMMAND_QUEUE_PTR(ro, lhs) \
    do { \
        rcl_command_queue_t *p; \
        Data_Get_Struct(ro, rcl_command_queue_t, p); \
        lhs = p->cq; \
    } while (0)

typedef struct {
    cl_command_queue cq;
} rcl_command_queue_t;

static void
rcl_command_queue_free(void *ptr)
{
    clReleaseCommandQueue(((rcl_command_queue_t *)ptr)->cq);
    free(ptr);
}

static VALUE
rcl_command_queue_alloc(VALUE klass)
{
    rcl_command_queue_t *p = ALLOC_N(rcl_command_queue_t, 1);
    p->cq = NULL;
    
    return Data_Wrap_Struct(klass, 0, rcl_command_queue_free, p);
}

/*
 * call-seq:
 *      CommandQueue.new(context, device, 0)
 *
 * Wrapps +clCreateCommandQueue()+. 
 *
 * Note that command queue properties must be +0+, 
 * +CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE+, +CL_QUEUE_PROFILING_ENABLE+, 
 * or the bit-or of the last two.
 */

static VALUE
rcl_command_queue_init(VALUE self, VALUE context, VALUE device, VALUE props)
{
    Check_RCL_Type(context, Context);
    Check_RCL_Type(device, Device);
    
    cl_context cxt;
    CONTEXT_PTR(context, cxt);
    
    cl_device_id did = DEVICE_PTR(device);
    cl_uint properties = NUM2UINT(props);
    
    cl_int res;
    cl_command_queue q = clCreateCommandQueue(cxt, did, properties, &res);
    
    Check_And_Raise(res);
    
    rcl_command_queue_t *pcq;
    Data_Get_Struct(self, rcl_command_queue_t, pcq);
    
    pcq->cq = q;
    
    return self;
}

static VALUE
rcl_command_queue_init_copy(VALUE copy, VALUE orig)
{
    if (copy == orig) return copy;
    
    Check_RCL_Type(orig, CommandQueue);
    
    rcl_command_queue_t *copy_p;
    rcl_command_queue_t *orig_p;    
    Data_Get_Struct(copy, rcl_command_queue_t, copy_p);
    Data_Get_Struct(orig, rcl_command_queue_t, orig_p);
    
    if (copy_p->cq == orig_p->cq) return copy;
    
    cl_int res;
    if (copy_p->cq != NULL) {
        res = clReleaseCommandQueue(copy_p->cq);
        Check_And_Raise(res);
    }
    res = clRetainCommandQueue(orig_p->cq);
    Check_And_Raise(res);
    
    copy_p->cq = orig_p->cq;
    
    return copy;
}

/*
 * call-seq:
 *   CommandQueue#info(CL_COMMAND_QUEUE_)
 */

static VALUE
rcl_command_queue_info(VALUE self, VALUE param)
{
    if (!FIXNUM_P(param)) {
        rb_raise(rb_eArgError, "Invalid command queue info type.");
    }
    
    cl_command_queue_info info = FIX2UINT(param);
    intptr_t param_value;   // CHECK: can handle current info types only.
    size_t sz_ret;
    
    cl_command_queue cq;
    COMMAND_QUEUE_PTR(self, cq);
    
    cl_int res = clGetCommandQueueInfo(cq, info, sizeof(intptr_t), (void *)&param_value, &sz_ret);
    Check_And_Raise(res);
    
    VALUE ret = Qnil;
    switch (info) {
    case CL_QUEUE_CONTEXT:
        RCONTEXT((cl_context)param_value, ret);
        break;
    case CL_QUEUE_DEVICE:
        return RDEVICE((cl_device_id)param_value);
    case CL_QUEUE_REFERENCE_COUNT:
        return INT2NUM(param_value);
    case CL_QUEUE_PROPERTIES:
        return UINT2NUM(param_value);
    default:
        break;
    }
    return ret;
}

/*
 * call-seq:
 *      CommandQueue#set_property(CL_QUEUE_PROFILING_ENABLE, true)
 *
 * Wrapps +clSetCommandQueueProperty()+.
 *
 * Returns the receiver.
 */

static VALUE
rcl_command_queue_set_property(VALUE self, VALUE props, VALUE yesno)
{
    cl_command_queue_properties pval = NUM2UINT(props);
    
    if (yesno != Qtrue && yesno != Qfalse) {
        rb_raise(rb_eArgError, "Argument 2 is not true or false.");
    }
    
    cl_command_queue q;
    COMMAND_QUEUE_PTR(self, q);
    
    cl_int res = clSetCommandQueueProperty(q, pval, yesno, NULL);
    Check_And_Raise(res);
    
    return self;
}

static void
define_class_command_queue(void)
{
    rb_cCommandQueue = rb_define_class_under(rb_mCapi, "CommandQueue", rb_cObject);
    rb_define_alloc_func(rb_cCommandQueue, rcl_command_queue_alloc);
    rb_define_method(rb_cCommandQueue, "initialize", rcl_command_queue_init, 3);
    rb_define_method(rb_cCommandQueue, "initialize_copy", rcl_command_queue_init_copy, 1);
    rb_define_method(rb_cCommandQueue, "info", rcl_command_queue_info, 1);
    rb_define_method(rb_cCommandQueue, "set_property", rcl_command_queue_set_property, 2);
}

/*
 * class Sampler
 */

static VALUE rb_cSampler;

typedef struct {
    cl_sampler  s;
} rcl_sampler_t;

static void
rcl_sampler_free(void *ptr)
{
    
}

static VALUE
rcl_sampler_alloc(VALUE klass)
{
    return Qnil;
}

static VALUE
rcl_sampler_init(VALUE self, VALUE context, VALUE normalized_coords, 
                 VALUE addressing_mode, VALUE filter_mode)
{
    return self;
}

static VALUE
rcl_sampler_init_copy(VALUE copy, VALUE orig)
{
    return copy;
}

static VALUE
rcl_sampler_info(VALUE self, VALUE param)
{
    return self;
}

static void
define_class_sampler(void)
{
    rb_cSampler = rb_define_class_under(rb_mCapi, "Sampler", rb_cObject);
    rb_define_alloc_func(rb_cSampler, rcl_sampler_alloc);
    rb_define_method(rb_cSampler, "initialize", rcl_sampler_init, 4);
    rb_define_method(rb_cSampler, "initialize_copy", rcl_sampler_init_copy, 1);
    rb_define_method(rb_cSampler, "info", rcl_sampler_info, 1);
        
}

/*
 * class Event
 */
 
static VALUE rb_cEvent;

typedef struct {
    cl_event e;
} rcl_event_t;

static void
rcl_event_free(void *ptr)
{
    clReleaseEvent(((rcl_event_t *)ptr)->e);
    free(ptr);
}

static VALUE
rcl_event_alloc(VALUE klass)
{
    rb_raise(rb_eRuntimeError, "Can't instantiate Event.");
    return Qnil;
}

static VALUE
rcl_event_init_copy(VALUE copy, VALUE orig)
{
    return copy;
}

static VALUE
rcl_event_info(VALUE self, VALUE param)
{
    return Qnil;
    
}

static VALUE
rcl_wait_for_events(VALUE events)
{
    return Qtrue;
}

static void
define_class_event(void)
{
    rb_cEvent = rb_define_class_under(rb_mCapi, "Event", rb_cObject);
    rb_define_alloc_func(rb_cEvent, rcl_event_alloc);
    rb_define_method(rb_cEvent, "initialize_copy", rcl_event_init_copy, 1);
    rb_define_method(rb_cEvent, "info", rcl_event_info, 1);
    rb_define_module_function(rb_mCapi, "wait_for_events", rcl_wait_for_events, 1);
}

/*
 * class Memory
 */

static VALUE rb_cMemory;

typedef struct _rcl_memory_t
{
    cl_mem  mem;
} rcl_memory;

static void
define_class_memory(void)
{
    rb_cMemory = rb_define_class_under(rb_mCapi, "Memory", rb_cObject);
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
 * class Kernel
 */

static VALUE rb_cKernel;

typedef struct {
    cl_kernel k;
} rcl_kernel_t;

static void
define_class_kernel(void)
{
    rb_cKernel = rb_define_class_under(rb_mCapi, "Kernel", rb_cObject);
}

/*
 * Execution
 */
 
/*
 * Misc
 */
 
static VALUE
rcl_flush(VALUE self)
{
    return self;
}

static VALUE
rcl_finish(VALUE self)
{
    return self;
}
 
static VALUE
rcl_supported_image_format(VALUE self)
{
    return Qnil;
}

static void
define_capi_methods(void)
{
    
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
    define_class_sampler();
    define_class_event();
    define_class_memory();
    define_class_program();
    define_class_kernel();
    
    define_capi_methods();    
}
