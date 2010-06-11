// A sheer wrapper of the OpenCL API.
//
// More rubyish syntax is defined in ruby file. See ../opencl.rb
// 
// Copyright (c) 2010, Diego Che

#include <ruby.h>
#include <assert.h>

#include <OpenCL/opencl.h>

#define TRACE(...)     fprintf(stderr, __VA_ARGS__)

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
     
    // cl_addressing_mode	
    RCL_DEF_CONSTANT(CL_ADDRESS_NONE);
    RCL_DEF_CONSTANT(CL_ADDRESS_CLAMP_TO_EDGE);
    RCL_DEF_CONSTANT(CL_ADDRESS_CLAMP);
    RCL_DEF_CONSTANT(CL_ADDRESS_REPEAT);

    // cl_bool	
    RCL_DEF_CONSTANT(CL_FALSE);
    RCL_DEF_CONSTANT(CL_TRUE);

    // cl_build_status	
    RCL_DEF_CONSTANT(CL_BUILD_ERROR);
    RCL_DEF_CONSTANT(CL_BUILD_IN_PROGRESS);
    RCL_DEF_CONSTANT(CL_BUILD_NONE);
    RCL_DEF_CONSTANT(CL_BUILD_SUCCESS);

    // cl_channel_order	
    RCL_DEF_CONSTANT(CL_A);
    RCL_DEF_CONSTANT(CL_R);
    RCL_DEF_CONSTANT(CL_RG);
    RCL_DEF_CONSTANT(CL_RA);
    RCL_DEF_CONSTANT(CL_RGB);
    RCL_DEF_CONSTANT(CL_RGBA);
    RCL_DEF_CONSTANT(CL_BGRA);
    RCL_DEF_CONSTANT(CL_ARGB);
    RCL_DEF_CONSTANT(CL_INTENSITY);
    RCL_DEF_CONSTANT(CL_LUMINANCE);

    // cl_channel_type	
    RCL_DEF_CONSTANT(CL_FLOAT);
    RCL_DEF_CONSTANT(CL_HALF_FLOAT);
    RCL_DEF_CONSTANT(CL_SIGNED_INT16);
    RCL_DEF_CONSTANT(CL_SIGNED_INT32);
    RCL_DEF_CONSTANT(CL_SIGNED_INT8);
    RCL_DEF_CONSTANT(CL_SNORM_INT8);
    RCL_DEF_CONSTANT(CL_SNORM_INT16);
    RCL_DEF_CONSTANT(CL_UNORM_INT_101010);
    RCL_DEF_CONSTANT(CL_UNORM_INT16);
    RCL_DEF_CONSTANT(CL_UNORM_INT8);
    RCL_DEF_CONSTANT(CL_UNORM_SHORT_555);
    RCL_DEF_CONSTANT(CL_UNORM_SHORT_565);
    RCL_DEF_CONSTANT(CL_UNSIGNED_INT16);
    RCL_DEF_CONSTANT(CL_UNSIGNED_INT32);
    RCL_DEF_CONSTANT(CL_UNSIGNED_INT8);
    
    // cl_command_queue_info
    RCL_DEF_CONSTANT(CL_QUEUE_CONTEXT);
    RCL_DEF_CONSTANT(CL_QUEUE_DEVICE);
    RCL_DEF_CONSTANT(CL_QUEUE_REFERENCE_COUNT);
    RCL_DEF_CONSTANT(CL_QUEUE_PROPERTIES);
    
    // cl_command_queue_properties
    RCL_DEF_CONSTANT(CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);
    RCL_DEF_CONSTANT(CL_QUEUE_PROFILING_ENABLE);
    
    // cl_command_type	
    RCL_DEF_CONSTANT(CL_COMMAND_ACQUIRE_GL_OBJECTS);
    RCL_DEF_CONSTANT(CL_COMMAND_COPY_BUFFER);
    RCL_DEF_CONSTANT(CL_COMMAND_COPY_BUFFER_TO_IMAGE);
    RCL_DEF_CONSTANT(CL_COMMAND_COPY_IMAGE);
    RCL_DEF_CONSTANT(CL_COMMAND_COPY_IMAGE_TO_BUFFER);
    RCL_DEF_CONSTANT(CL_COMMAND_MAP_BUFFER);
    RCL_DEF_CONSTANT(CL_COMMAND_MAP_IMAGE);
    RCL_DEF_CONSTANT(CL_COMMAND_MARKER);
    RCL_DEF_CONSTANT(CL_COMMAND_NATIVE_KERNEL);
    RCL_DEF_CONSTANT(CL_COMMAND_NDRANGE_KERNEL);
    RCL_DEF_CONSTANT(CL_COMMAND_READ_BUFFER);
    RCL_DEF_CONSTANT(CL_COMMAND_READ_IMAGE);
    RCL_DEF_CONSTANT(CL_COMMAND_RELEASE_GL_OBJECTS);
    RCL_DEF_CONSTANT(CL_COMMAND_TASK);
    RCL_DEF_CONSTANT(CL_COMMAND_UNMAP_MEM_OBJECT);
    RCL_DEF_CONSTANT(CL_COMMAND_WRITE_BUFFER);
    RCL_DEF_CONSTANT(CL_COMMAND_WRITE_IMAGE);
    
    // cl_context_info
    RCL_DEF_CONSTANT(CL_CONTEXT_REFERENCE_COUNT);
    RCL_DEF_CONSTANT(CL_CONTEXT_DEVICES);
    RCL_DEF_CONSTANT(CL_CONTEXT_PROPERTIES);
    
    // cl_context_properties
    RCL_DEF_CONSTANT(CL_CONTEXT_PLATFORM);
    
    // cl_device_exec_capabilities	
    RCL_DEF_CONSTANT(CL_EXEC_KERNEL); 
    RCL_DEF_CONSTANT(CL_EXEC_NATIVE_KERNEL);

    // cl_device_fp_config	
    RCL_DEF_CONSTANT(CL_FP_DENORM);
    RCL_DEF_CONSTANT(CL_FP_FMA);
    RCL_DEF_CONSTANT(CL_FP_INF_NAN);
    RCL_DEF_CONSTANT(CL_FP_ROUND_TO_INF);
    RCL_DEF_CONSTANT(CL_FP_ROUND_TO_NEAREST);
    RCL_DEF_CONSTANT(CL_FP_ROUND_TO_ZERO);

    // cl_device_info
    RCL_DEF_CONSTANT(CL_DEVICE_ADDRESS_BITS);                       
    RCL_DEF_CONSTANT(CL_DEVICE_AVAILABLE);                          
    RCL_DEF_CONSTANT(CL_DEVICE_COMPILER_AVAILABLE);                 
    RCL_DEF_CONSTANT(CL_DEVICE_ENDIAN_LITTLE);                    
    RCL_DEF_CONSTANT(CL_DEVICE_ERROR_CORRECTION_SUPPORT);        
    RCL_DEF_CONSTANT(CL_DEVICE_EXECUTION_CAPABILITIES);             
    RCL_DEF_CONSTANT(CL_DEVICE_EXTENSIONS);
    RCL_DEF_CONSTANT(CL_DEVICE_GLOBAL_MEM_CACHE_SIZE);              
    RCL_DEF_CONSTANT(CL_DEVICE_GLOBAL_MEM_CACHE_TYPE);              
    RCL_DEF_CONSTANT(CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE);          
    RCL_DEF_CONSTANT(CL_DEVICE_GLOBAL_MEM_SIZE);                    
    RCL_DEF_CONSTANT(CL_DEVICE_IMAGE_SUPPORT);                      
    RCL_DEF_CONSTANT(CL_DEVICE_IMAGE2D_MAX_HEIGHT);                
    RCL_DEF_CONSTANT(CL_DEVICE_IMAGE2D_MAX_WIDTH);                  
    RCL_DEF_CONSTANT(CL_DEVICE_IMAGE3D_MAX_DEPTH);                  
    RCL_DEF_CONSTANT(CL_DEVICE_IMAGE3D_MAX_HEIGHT);                 
    RCL_DEF_CONSTANT(CL_DEVICE_IMAGE3D_MAX_WIDTH);                  
    RCL_DEF_CONSTANT(CL_DEVICE_LOCAL_MEM_SIZE);                     
    RCL_DEF_CONSTANT(CL_DEVICE_LOCAL_MEM_TYPE);                     
    RCL_DEF_CONSTANT(CL_DEVICE_MAX_CLOCK_FREQUENCY);                
    RCL_DEF_CONSTANT(CL_DEVICE_MAX_COMPUTE_UNITS);                  
    RCL_DEF_CONSTANT(CL_DEVICE_MAX_CONSTANT_ARGS);                  
    RCL_DEF_CONSTANT(CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE);           
    RCL_DEF_CONSTANT(CL_DEVICE_MAX_MEM_ALLOC_SIZE);                 
    RCL_DEF_CONSTANT(CL_DEVICE_MAX_PARAMETER_SIZE);                 
    RCL_DEF_CONSTANT(CL_DEVICE_MAX_READ_IMAGE_ARGS);                
    RCL_DEF_CONSTANT(CL_DEVICE_MAX_SAMPLERS);                       
    RCL_DEF_CONSTANT(CL_DEVICE_MAX_WORK_GROUP_SIZE);                
    RCL_DEF_CONSTANT(CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS);           
    RCL_DEF_CONSTANT(CL_DEVICE_MAX_WORK_ITEM_SIZES);                
    RCL_DEF_CONSTANT(CL_DEVICE_MAX_WRITE_IMAGE_ARGS);               
    RCL_DEF_CONSTANT(CL_DEVICE_MEM_BASE_ADDR_ALIGN);                
    RCL_DEF_CONSTANT(CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE);           
    RCL_DEF_CONSTANT(CL_DEVICE_NAME);                               
    RCL_DEF_CONSTANT(CL_DEVICE_PLATFORM);
    RCL_DEF_CONSTANT(CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR);        
    RCL_DEF_CONSTANT(CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE);      
    RCL_DEF_CONSTANT(CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT);       
    RCL_DEF_CONSTANT(CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT);         
    RCL_DEF_CONSTANT(CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG);        
    RCL_DEF_CONSTANT(CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT);       
    RCL_DEF_CONSTANT(CL_DEVICE_PROFILE);                           
    RCL_DEF_CONSTANT(CL_DEVICE_PROFILING_TIMER_RESOLUTION);         
    RCL_DEF_CONSTANT(CL_DEVICE_QUEUE_PROPERTIES);                   
    RCL_DEF_CONSTANT(CL_DEVICE_SINGLE_FP_CONFIG);                   
    RCL_DEF_CONSTANT(CL_DEVICE_TYPE);                              
    RCL_DEF_CONSTANT(CL_DEVICE_VENDOR_ID);                          
    RCL_DEF_CONSTANT(CL_DEVICE_VENDOR);                            
    RCL_DEF_CONSTANT(CL_DEVICE_VERSION);                      
    RCL_DEF_CONSTANT(CL_DRIVER_VERSION);
    
    // cl_device_local_mem_type	
    RCL_DEF_CONSTANT(CL_GLOBAL);
    RCL_DEF_CONSTANT(CL_LOCAL);

    // cl_device_mem_cache_type	
    RCL_DEF_CONSTANT(CL_NONE);
    RCL_DEF_CONSTANT(CL_READ_ONLY_CACHE);
    RCL_DEF_CONSTANT(CL_READ_WRITE_CACHE);

    // cl_device_type
    RCL_DEF_CONSTANT(CL_DEVICE_TYPE_CPU);
    RCL_DEF_CONSTANT(CL_DEVICE_TYPE_GPU);
    RCL_DEF_CONSTANT(CL_DEVICE_TYPE_ACCELERATOR);
    RCL_DEF_CONSTANT(CL_DEVICE_TYPE_DEFAULT);
    RCL_DEF_CONSTANT(CL_DEVICE_TYPE_ALL);
    
    // cl_event_info	
    RCL_DEF_CONSTANT(CL_EVENT_COMMAND_QUEUE);
    RCL_DEF_CONSTANT(CL_EVENT_COMMAND_TYPE);
    RCL_DEF_CONSTANT(CL_EVENT_REFERENCE_COUNT);
    RCL_DEF_CONSTANT(CL_EVENT_COMMAND_EXECUTION_STATUS);

    // cl_filter_mode	
    RCL_DEF_CONSTANT(CL_FILTER_NEAREST);
    RCL_DEF_CONSTANT(CL_FILTER_LINEAR);

    // cl_gl_object_type	
    RCL_DEF_CONSTANT(CL_GL_OBJECT_BUFFER);
    RCL_DEF_CONSTANT(CL_GL_OBJECT_TEXTURE2D);
    RCL_DEF_CONSTANT(CL_GL_OBJECT_TEXTURE3D);
    RCL_DEF_CONSTANT(CL_GL_OBJECT_RENDERBUFFER);

    // cl_gl_texture_info	
    RCL_DEF_CONSTANT(CL_GL_TEXTURE_TARGET);
    RCL_DEF_CONSTANT(CL_GL_MIPMAP_LEVEL);

    // cl_image_info	
    RCL_DEF_CONSTANT(CL_IMAGE_FORMAT);
    RCL_DEF_CONSTANT(CL_IMAGE_ELEMENT_SIZE);
    RCL_DEF_CONSTANT(CL_IMAGE_ROW_PITCH);
    RCL_DEF_CONSTANT(CL_IMAGE_SLICE_PITCH);
    RCL_DEF_CONSTANT(CL_IMAGE_WIDTH);
    RCL_DEF_CONSTANT(CL_IMAGE_HEIGHT);
    RCL_DEF_CONSTANT(CL_IMAGE_DEPTH);

    // cl_kernel_info	
    RCL_DEF_CONSTANT(CL_KERNEL_FUNCTION_NAME);
    RCL_DEF_CONSTANT(CL_KERNEL_NUM_ARGS);
    RCL_DEF_CONSTANT(CL_KERNEL_REFERENCE_COUNT);
    RCL_DEF_CONSTANT(CL_KERNEL_CONTEXT);
    RCL_DEF_CONSTANT(CL_KERNEL_PROGRAM);

    // cl_kernel_work_group_info	
    RCL_DEF_CONSTANT(CL_KERNEL_WORK_GROUP_SIZE);
    RCL_DEF_CONSTANT(CL_KERNEL_COMPILE_WORK_GROUP_SIZE);
    RCL_DEF_CONSTANT(CL_KERNEL_LOCAL_MEM_SIZE);

    // cl_map_flags	
    RCL_DEF_CONSTANT(CL_MAP_READ);
    RCL_DEF_CONSTANT(CL_MAP_WRITE);

    // cl_mem_flags	
    RCL_DEF_CONSTANT(CL_MEM_READ_WRITE);
    RCL_DEF_CONSTANT(CL_MEM_WRITE_ONLY);
    RCL_DEF_CONSTANT(CL_MEM_READ_ONLY);
    RCL_DEF_CONSTANT(CL_MEM_USE_HOST_PTR);
    RCL_DEF_CONSTANT(CL_MEM_ALLOC_HOST_PTR);
    RCL_DEF_CONSTANT(CL_MEM_COPY_HOST_PTR);
    
    // cl_mem_object_type	
    RCL_DEF_CONSTANT(CL_MEM_OBJECT_BUFFER);
    RCL_DEF_CONSTANT(CL_MEM_OBJECT_IMAGE2D);
    RCL_DEF_CONSTANT(CL_MEM_OBJECT_IMAGE3D);
    
    // cl_platform_info 
    RCL_DEF_CONSTANT(CL_PLATFORM_PROFILE);
    RCL_DEF_CONSTANT(CL_PLATFORM_VERSION);
    RCL_DEF_CONSTANT(CL_PLATFORM_NAME);
    RCL_DEF_CONSTANT(CL_PLATFORM_VENDOR);
    RCL_DEF_CONSTANT(CL_PLATFORM_EXTENSIONS);
    
    // cl_profiling_info	
    RCL_DEF_CONSTANT(CL_PROFILING_COMMAND_QUEUED); 
    RCL_DEF_CONSTANT(CL_PROFILING_COMMAND_SUBMIT); 
    RCL_DEF_CONSTANT(CL_PROFILING_COMMAND_START); 
    RCL_DEF_CONSTANT(CL_PROFILING_COMMAND_END);

    // cl_program_build_info	
    RCL_DEF_CONSTANT(CL_PROGRAM_BUILD_STATUS); 
    RCL_DEF_CONSTANT(CL_PROGRAM_BUILD_OPTIONS); 
    RCL_DEF_CONSTANT(CL_PROGRAM_BUILD_LOG);

    // cl_sampler_info	
    RCL_DEF_CONSTANT(CL_SAMPLER_REFERENCE_COUNT); 
    RCL_DEF_CONSTANT(CL_SAMPLER_CONTEXT); 
    RCL_DEF_CONSTANT(CL_SAMPLER_NORMALIZED_COORDS); 
    RCL_DEF_CONSTANT(CL_SAMPLER_ADDRESSING_MODE); 
    RCL_DEF_CONSTANT(CL_SAMPLER_FILTER_MODE);    
}

// CHECK: add equality check for all classes?

/*
 * Capi::CLError
 *
 * error code and error message.
 */
 
static VALUE rb_eOpenCL;
static VALUE rcl_errors;    // { errcode => message }

#define RCL_DEF_CL_ERROR(errcode, errstr) \
    do { \
        rb_define_const(rb_mCapi, #errcode, INT2FIX(errcode)); \
        rb_hash_aset(rcl_errors, INT2FIX(errcode), rb_str_new2(errstr)); \
    } while (0)

static void
define_opencl_errors(void)
{
    rcl_errors = rb_hash_new();
    
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
    
    rb_obj_freeze(rcl_errors);
}

static VALUE
rcl_error_init(VALUE self, VALUE errcode)
{
    if (FIXNUM_P(errcode) && !NIL_P(rb_hash_aref(rcl_errors, errcode))) {
        rb_iv_set(self, "@cl_errcode", errcode);
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
    // TODO: find a way to raise an Exception object.
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
#define RPlatform(ptr)      (Data_Wrap_Struct(rb_cPlatform, 0, 0, (ptr)))
// ruby object -> cl ptr
#define Platform_Ptr(ro)    (DATA_PTR(ro))

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
            VALUE o = RPlatform(p_ids[i]);
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
    
    cl_platform_id platform = Platform_Ptr(self);
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

#define RDevice(ptr)      Data_Wrap_Struct(rb_cDevice, 0, 0, (ptr))
#define Device_Ptr(ro)    DATA_PTR(ro)

/*
 * :nodoc:
 */
static VALUE
rcl_device_alloc(VALUE self)
{
    rb_raise(rb_eRuntimeError, "Can't instantiate Device.");
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
    
    cl_platform_id pid = NIL_P(platform) ? NULL : Platform_Ptr(platform);
    cl_device_type dt = FIX2INT(type);
    res = clGetDeviceIDs(pid, dt, 256, d_ids, &num_id); // CHECK: ditto.
    
    VALUE devs = rb_ary_new();
    if (res != CL_SUCCESS) {
        Check_And_Warn(res);
    } else {
        int i;
        for (i = 0; i < num_id; i++) {
            VALUE o = RDevice(d_ids[i]);
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
    
    cl_device_id device = Device_Ptr(self);
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
typedef struct {
    cl_context  c;
} rcl_context_t;

static void
rcl_context_free(void *ptr)
{
    clReleaseContext(((rcl_context_t *)ptr)->c);
    free(ptr);
}

static inline VALUE
RContext(cl_context ptr)
{
    rcl_context_t *p = ALLOC_N(rcl_context_t, 1);
    p->c = (ptr);
    
    return Data_Wrap_Struct(rb_cContext, 0, rcl_context_free, p);   
}

static inline cl_context
Context_Ptr(VALUE ro)
{
    Check_RCL_Type(ro, Context);
    
    rcl_context_t *p;
    Data_Get_Struct(ro, rcl_context_t, p);
    
    return p->c;
}

static VALUE
rcl_context_alloc(VALUE klass)
{    
    return RContext(NULL);
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
        
        devs[i] = Device_Ptr(dev);
    }
}

static VALUE
build_device_array(cl_device_id *devs, size_t cb)
{
    VALUE ret = rb_ary_new();
    size_t num_dev = cb / sizeof(cl_device_id);
    
    int i;
    for (i = 0; i < num_dev; i++) {
        VALUE dev = RDevice(devs[i]);
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
    cl_context cxt = Context_Ptr(self);
    
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
    rb_define_method(rb_cContext, "initialize_copy", 
                                  rcl_context_init_copy, 1);
    rb_define_method(rb_cContext, "info", rcl_context_info, 1);
}

/*
 * class ImageFormat
 */

static VALUE rb_cImageFormat;

static inline VALUE
RImageFormat(cl_image_format *imf)
{
    VALUE ro = rb_obj_alloc(rb_cImageFormat);
    rb_iv_set(ro, "@channel_order", 
              INT2FIX(imf->image_channel_order));
    rb_iv_set(ro, "@channel_data_type", 
              INT2FIX(imf->image_channel_data_type));
    
    return ro;
}

static VALUE
rcl_image_format_init(VALUE self, VALUE order, VALUE type)
{
    if (!(FIXNUM_P(order) && FIXNUM_P(type))) {
        rb_raise(rb_eArgError, "Invalid argument types.");
    }
    
    rb_iv_set(self, "@channel_order", INT2FIX(order));
    rb_iv_set(self, "@channel_data_type", INT2FIX(type));
    
    return self;
}
 
static VALUE
rcl_context_supported_image_formats(VALUE self, VALUE mem_flag, VALUE type)
{
    if (!FIXNUM_P(type)) {
        rb_raise(rb_eArgError, "Invalid memory object type.");
    }
    cl_mem_object_type mt = FIX2INT(type);
    cl_mem_flags mf = NUM2UINT(mem_flag);    // FIXME: how to check type of ulong?
    
    cl_context cxt = Context_Ptr(self);
    cl_uint num_ret = 0;
    cl_int res = clGetSupportedImageFormats(cxt, mf, mt, 0, NULL, &num_ret);
    Check_And_Raise(res);
    
    cl_image_format *ifs = ALLOCA_N(cl_image_format, num_ret);
    if (NULL == ifs) {
        rb_raise(rb_eRuntimeError, "Out of memory.");
    }
    res = clGetSupportedImageFormats(cxt, mf, mt, num_ret, ifs, NULL);
    Check_And_Raise(res);
    
    VALUE ret = rb_ary_new2(num_ret);
    int i = 0;
    for (; i < num_ret; i++) {
        rb_ary_push(ret, RImageFormat(ifs + i));
    }
    return ret;
}

static void
define_class_image_format(void)
{
    rb_cImageFormat = rb_define_class_under(rb_mCapi, "ImageFormat", rb_cObject);
    rb_define_attr(rb_cImageFormat, "channel_order", 1, 0);
    rb_define_attr(rb_cImageFormat, "channel_data_type", 1, 0);
    rb_define_method(rb_cImageFormat, "initialize", rcl_image_format_init, 2);
    rb_define_method(rb_cContext, "supported_image_formats", 
                                  rcl_context_supported_image_formats, 2);
                                  
}

/*
 * class CommandQueue
 */
 
static VALUE rb_cCommandQueue;

typedef struct {
    cl_command_queue cq;
} rcl_command_queue_t;

static inline cl_command_queue
Command_Queue_Ptr(VALUE ro)
{
    Check_RCL_Type(ro, CommandQueue);
    
    rcl_command_queue_t *p;
    Data_Get_Struct(ro, rcl_command_queue_t, p);
    
    return p->cq;
}

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
    
    cl_context cxt = Context_Ptr(context);   
    cl_device_id did = Device_Ptr(device);
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
    
    intptr_t param_value;   // CHECK: extensibility.
    size_t sz_ret;
    
    cl_command_queue cq = Command_Queue_Ptr(self);
    cl_command_queue_info info = FIX2UINT(param);
    
    cl_int res = clGetCommandQueueInfo(cq, info, sizeof(intptr_t), (void *)&param_value, &sz_ret);
    Check_And_Raise(res);
    
    switch (info) {
    case CL_QUEUE_CONTEXT:
        return RContext((cl_context)param_value);
    case CL_QUEUE_DEVICE:
        return RDevice((cl_device_id)param_value);
    case CL_QUEUE_REFERENCE_COUNT:
        return INT2NUM(param_value);
    case CL_QUEUE_PROPERTIES:
        return UINT2NUM(param_value);
    default:
        break;
    }
    return Qnil;
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
    if (yesno != Qtrue && yesno != Qfalse) {
        rb_raise(rb_eArgError, "Argument 2 is not true or false.");
    }
    
    cl_command_queue q = Command_Queue_Ptr(self);
    cl_command_queue_properties pval = NUM2UINT(props);
    
    cl_int res = clSetCommandQueueProperty(q, pval, yesno, NULL);
    Check_And_Raise(res);
    
    return self;
}

static VALUE
rcl_flush(VALUE self)
{
    cl_int res = clFlush(CommandQueue_Ptr(self));
    Check_And_Raise(res);
    
    return self;
}

static VALUE
rcl_finish(VALUE self)
{
    cl_int res = clFinish(CommandQueue_Ptr(self));
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
    rb_define_method(rb_cCommandQueue, "flush", rcl_flush, 0);
    rb_define_method(rb_cCommandQueue, "finish", rcl_finish, 0);    
}

/*
 * class Sampler
 */

static VALUE rb_cSampler;

typedef struct {
    cl_sampler  s;
} rcl_sampler_t;

static inline cl_sampler
Sampler_Ptr(VALUE ro)
{
    Check_RCL_Type(ro, Sampler);
    
    rcl_sampler_t *p;
    Data_Get_Struct(ro, rcl_sampler_t, p);
    
    return p->s;
}

static void
rcl_sampler_free(void *ptr)
{
    clReleaseSampler(((rcl_sampler_t *)ptr)->s);
    free(ptr);
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

static inline VALUE
REvent(cl_event ptr)
{
    rcl_event_t *p = ALLOC_N(rcl_event_t, 1);
    p->e = ptr;
    
    return Data_Wrap_Struct(rb_cEvent, 0, rcl_event_free, p);
}

static inline cl_event
Event_Ptr(VALUE ro)
{
    Check_RCL_Type(ro, Event);
    
    rcl_event_t *p;
    Data_Get_Struct(ro, rcl_event_t, p);
    
    return p->e;
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
rcl_wait_for_events(VALUE self, VALUE events)
{
    return Qtrue;
}

static VALUE
rcl_event_profiling_info(VALUE self, VALUE param)
{
    if (!FIXNUM_P(param)) {
        rb_raise(rb_eArgError, "Invalid profiling info type.");
    }
    cl_profiling_info itype = FIX2INT(param);
    
    cl_ulong info;    
    cl_event e = Event_Ptr(self);
    
    cl_int res = clGetEventProfilingInfo(e, itype, sizeof(cl_ulong), &info, NULL);    
    Check_And_Raise(res);
    
    return ULONG2NUM(info);
}

static void
define_class_event(void)
{
    rb_cEvent = rb_define_class_under(rb_mCapi, "Event", rb_cObject);
    rb_define_alloc_func(rb_cEvent, rcl_event_alloc);
    rb_define_method(rb_cEvent, "initialize_copy", rcl_event_init_copy, 1);
    rb_define_method(rb_cEvent, "info", rcl_event_info, 1);
    rb_define_module_function(rb_mCapi, "wait_for_events", rcl_wait_for_events, 1);
    rb_define_method(rb_cEvent, "profiling_info", rcl_event_profiling_info, 1);
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
    define_class_image_format();
    define_class_command_queue();
    define_class_sampler();
    define_class_event();
    define_class_memory();
    define_class_program();
    define_class_kernel();
    
    define_capi_methods();    
}
