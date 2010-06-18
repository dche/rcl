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
 
// Modules
static VALUE rcl_mOpenCL;
static VALUE rcl_mCapi;

// CL objects
static VALUE rb_cPlatform;
static VALUE rb_cDevice;
static VALUE rb_cContext;
static VALUE rb_cImageFormat;
static VALUE rb_cCommandQueue;
static VALUE rb_cSampler;
static VALUE rb_cEvent;
static VALUE rb_cMemory;
static VALUE rb_cProgram;
static VALUE rb_cKernel;

// Support classes
static VALUE rb_cPointer;

// NOTE: this macro needs local defined class variables 
//       take specific pattern of name, /rb_c\w+/
#define Expect_RCL_Type(o, klass) \
    do { \
        Check_Type(o, T_DATA); \
        if (rb_class_of(o) != rb_c##klass) { \
            rb_raise(rb_eTypeError, "Expected %s is a instance of %s.", #o, #klass); \
        } \
    } while (0)

#define Expect_RCL_Const(ro) \
    do { \
        if (!FIXNUM_P(ro)) \
            rb_raise(rb_eTypeError, \
                    "Expected %s an OpenCL enumerated constant. ", #ro); \
    } while (0)

#define Expect_Boolean(ro) \
    do { \
        if (ro != Qtrue && ro != Qfalse) \
            rb_raise(rb_eTypeError, \
                    "Expected %s is true or false.", #ro); \
    } while (0)
    
#define Expect_Array(ro) \
    do { \
        if (TYPE(ro) != T_ARRAY) \
            rb_raise(rb_eTypeError, \
                    "Expected %s is an Array.", #ro); \
    } while (0)
    
#define Expect_NonEmpty_Array(ro) \
    do { \
        if (TYPE(ro) != T_ARRAY) \
            rb_raise(rb_eTypeError, \
                    "Expected %s is an Array.", #ro); \
        if (RARRAY_LEN(ro) == 0) \
            rb_raise(rb_eArgError, \
                    "Expected %s is not empty.", #ro); \
    } while (0)

#define CL_Pointers(ra, klass, c_type, svar) \
    do { \
        Expect_Array(ra); \
        cl_uint num = RARRAY_LEN(ra); \
        if (num == 0) { \
            svar = NULL; \
        } else { \
            svar = ALLOCA_N(c_type, num); \
            for (int i = 0; i < num; i++) { \
                VALUE ro = rb_ary_entry(ra, i); \
                Expect_RCL_Type(ro, klass); \
                svar[i] = klass##_Ptr(ro); \
            } \
        } \
    } while (0)


#define RPlatform(ptr)      (Data_Wrap_Struct(rb_cPlatform, 0, 0, (ptr)))

static inline cl_platform_id
Platform_Ptr(VALUE ro)
{
    Expect_RCL_Type(ro, Platform);
    return DATA_PTR(ro);
}

#define RDevice(ptr)      Data_Wrap_Struct(rb_cDevice, 0, 0, (ptr))

static inline cl_device_id
Device_Ptr(VALUE ro)
{
    Expect_RCL_Type(ro, Device);
    return DATA_PTR(ro);
}

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

static inline cl_context
Context_Ptr(VALUE ro)
{
    Expect_RCL_Type(ro, Context);
    
    rcl_context_t *p;
    Data_Get_Struct(ro, rcl_context_t, p);
    
    return p->c;
}

static inline VALUE
RContext(cl_context ptr)
{
    rcl_context_t *p;
    VALUE ret = Data_Make_Struct(rb_cContext, rcl_context_t, 0, rcl_context_free, p);
    
    p->c = ptr;
    return ret;   
}

static inline VALUE
RImageFormat(cl_image_format *imf)
{
    VALUE ro = rb_obj_alloc(rb_cImageFormat);
    rb_iv_set(ro, "@channel_order", 
              UINT2FIX(imf->image_channel_order));
    rb_iv_set(ro, "@channel_data_type", 
              UINT2FIX(imf->image_channel_data_type));
    
    return ro;
}

typedef struct {
    cl_command_queue cq;
} rcl_command_queue_t;

static void
rcl_command_queue_free(void *ptr)
{
    clReleaseCommandQueue(((rcl_command_queue_t *)ptr)->cq);
    free(ptr);
}

static inline VALUE
RCommandQueue(cl_command_queue ptr)
{
    rcl_command_queue_t *p;
    VALUE ret = Data_Make_Struct(rb_cCommandQueue, rcl_command_queue_t, 
                                 0, rcl_command_queue_free, p);
    p->cq = ptr;
    return ret;
}

static inline cl_command_queue
CommandQueue_Ptr(VALUE ro)
{
    Expect_RCL_Type(ro, CommandQueue);
    
    rcl_command_queue_t *p;
    Data_Get_Struct(ro, rcl_command_queue_t, p);
    
    return p->cq;
}

typedef struct {
    cl_sampler s;
} rcl_sampler_t;

static inline cl_sampler
Sampler_Ptr(VALUE ro)
{
    Expect_RCL_Type(ro, Sampler);
    
    rcl_sampler_t *ps;
    Data_Get_Struct(ro, rcl_sampler_t, ps);
    
    return ps->s;
}

static void
rcl_sampler_free(void *ptr)
{
    clReleaseSampler(((rcl_sampler_t *)ptr)->s);
    free(ptr);
}

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
    Expect_RCL_Type(ro, Event);
    
    rcl_event_t *p;
    Data_Get_Struct(ro, rcl_event_t, p);
    
    return p->e;
}

typedef struct {
    cl_mem  mem;
} rcl_mem_t;

static void
rcl_mem_free(void *ptr)
{
    clReleaseMemObject(((rcl_mem_t *)ptr)->mem);
    free(ptr);
}

static inline cl_mem
Memory_Ptr(VALUE ro)
{
    Expect_RCL_Type(ro, Memory);
    
    rcl_mem_t *p;
    Data_Get_Struct(ro, rcl_mem_t, p);
    
    return p->mem;    
}

typedef struct {
    cl_program p;
} rcl_program_t;

static void
rcl_program_free(void *ptr)
{
    clReleaseProgram(((rcl_program_t *)ptr)->p);
    free(ptr);
}

static inline VALUE
RProgram(cl_program prog)
{
    rcl_program_t *sp;
    VALUE ret = Data_Make_Struct(rb_cProgram, rcl_program_t, 0, rcl_program_free, sp);

    sp->p = prog;
    return ret;
}

static inline cl_program
Program_Ptr(VALUE ro)
{
    Expect_RCL_Type(ro, Program);

    rcl_program_t *p;
    Data_Get_Struct(ro, rcl_program_t, p);

    return p->p;
}


typedef struct {
    cl_kernel k;
    size_t    argc;
} rcl_kernel_t;

static void
rcl_kernel_free(void *ptr)
{
    clReleaseKernel(((rcl_kernel_t *)ptr)->k);
    free(ptr);
}

static inline VALUE
RKernel(cl_kernel k)
{
    rcl_kernel_t *p;
    VALUE ret = Data_Make_Struct(rb_cKernel, rcl_kernel_t, 0, rcl_kernel_free, p);
    
    p->k = k;
    p->argc = 0;
    return ret;
}

static inline cl_kernel
Kernel_Ptr(VALUE ro)
{
    Expect_RCL_Type(ro, Kernel);
    
    rcl_kernel_t *p;
    Data_Get_Struct(ro, rcl_kernel_t, p);
    
    return p->k;
}

/*
 * OpenCL::Capi::constants definition.
 */

#define RCL_DEF_CONSTANT(constant) \
    do \
        rb_define_const(rcl_mCapi, #constant, INT2FIX(constant)); \
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
// TODO: check all Array that might get an empty value.
// TODO: check all arguments that can be +nil+.

/*
 * Capi::CLError
 *
 * error code and error message.
 */
 
static VALUE rb_eOpenCL;
static VALUE rcl_errors;    // { errcode => message }

#define RCL_DEF_CL_ERROR(errcode, errstr) \
    do { \
        rb_define_const(rcl_mCapi, #errcode, INT2FIX(errcode)); \
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

static void
define_class_clerror(void)
{
    rb_eOpenCL = rb_define_class_under(rcl_mCapi, "CLError", rb_eRuntimeError);
    rb_define_method(rb_eOpenCL, "initialize", rcl_error_init, 1);
    rb_define_attr(rb_eOpenCL, "cl_errcode", 1, 0);    
}

/*
 * CL types.
 */

// Storages for types.
static VALUE rcl_types;
static VALUE rcl_vector_types;

#define CL_TYPE_ID(type)   (rb_intern( #type ))

#define DEF_CL_TYPE(type) \
    do { \
        ID id = CL_TYPE_ID(type); \
        size_t sz = sizeof(type); \
        rb_hash_aset(rcl_types, ID2SYM(id), UINT2FIX(sz)); \
    } while (0)
    
#define DEF_CL_VECTOR_TYPE(type) \
    do { \
        int n = 2; \
        ID id; \
        size_t sz; \
        char type_name[32]; \
        for (int i = 0; i < 4; i++) { \
            snprintf(type_name, 32, "%s%d", #type, n << i); \
            id = rb_intern(type_name); \
            sz = sizeof(type) * n; \
            rb_hash_aset(rcl_vector_types, ID2SYM(id), UINT2FIX(sz)); \
        } \
    } while (0)

static void define_cl_types(void)
{
    rcl_types = rb_hash_new();
    rcl_vector_types = rb_hash_new();
    
    // DEF_CL_TYPE(bool);
    DEF_CL_TYPE(cl_char);
    DEF_CL_TYPE(cl_uchar);
    DEF_CL_TYPE(cl_short);
    DEF_CL_TYPE(cl_ushort);
    DEF_CL_TYPE(cl_int);
    DEF_CL_TYPE(cl_uint);
    DEF_CL_TYPE(cl_long);
    DEF_CL_TYPE(cl_ulong);
    DEF_CL_TYPE(cl_float);
    DEF_CL_TYPE(cl_half);
    DEF_CL_TYPE(size_t);
    
    rb_obj_freeze(rcl_types);

    DEF_CL_VECTOR_TYPE(cl_char);
    DEF_CL_VECTOR_TYPE(cl_uchar);
    DEF_CL_VECTOR_TYPE(cl_short);    
    DEF_CL_VECTOR_TYPE(cl_ushort);
    DEF_CL_VECTOR_TYPE(cl_int);
    DEF_CL_VECTOR_TYPE(cl_uint);
    DEF_CL_VECTOR_TYPE(cl_long);
    DEF_CL_VECTOR_TYPE(cl_ulong);
    DEF_CL_VECTOR_TYPE(cl_float);
    
    rb_obj_freeze(rcl_vector_types);
}

#define Is_Type_Valid(id)  (!NIL_P(rb_hash_lookup(rcl_types, ID2SYM(id))) || !NIL_P(rb_hash_lookup(rcl_vector_types, ID2SYM(id))))

#define Is_Type_Vector(id) (!NIL_P(rb_hash_lookup(rcl_vector_types, ID2SYM(id))))

#define Extract_Size(sizet, var) \
    size_t var; \
    do { \
        if (!FIXNUM_P(sizet)) { \
            rb_raise(rb_eTypeError, "Expected %s a Fixnum.", #sizet); \
        } \
        var = FIX2UINT(sizet); \
    } while (0)


static inline size_t
Type_Size(ID id)
{
    VALUE type = ID2SYM(id);
    VALUE sz = rb_hash_lookup(rcl_types, type);
    
    if (NIL_P(sz)) sz = rb_hash_lookup(rcl_vector_types, type);
    
    assert(FIXNUM_P(sz));
    return FIX2UINT(sz);
}

static inline void Ruby2Native(ID type, void *address, VALUE value)
{
    
}

static inline VALUE Native2Ruby(ID type, void *address)
{
    return Qnil;
}

/*
 * class Pointer
 */

typedef struct {
    
    int8_t  *alloc_address;
    void    *address;
    size_t   size;    // in number of elements.
    ID       type;
    
} rcl_pointer_t;

static inline rcl_pointer_t *
Pointer_Ptr(VALUE ptr)
{
    Expect_RCL_Type(ptr, Pointer);
    
    rcl_pointer_t *p;
    Data_Get_Struct(ptr, rcl_pointer_t, p);
    
    return p;
}

static inline int
Is_Immediate_Value(rcl_pointer_t *p)
{
    return !Is_Type_Vector(ID2SYM(p->type)) && (p->size == 1);
}

// Returns the memory address that holds value. Note that for
// immediate types, this address is not the 'address' field of pointer.
static inline void *
Pointer_Address(VALUE ptr)
{
    rcl_pointer_t *p = Pointer_Ptr(ptr);
    
    if (p->alloc_address == NULL) {
        assert(Is_Immediate_Value(p));
        return &(p->address);
    } else {
        return p->address;        
    }
}

static void
rcl_pointer_free_func(void *ptr)
{
    rcl_pointer_t *p = (rcl_pointer_t *)ptr;
    
    if (!Is_Immediate_Value(p)) {
        free(p->alloc_address);
    }
    free(ptr);
}

static VALUE
rcl_pointer_alloc(VALUE klass)
{
    rcl_pointer_t *p;
    return Data_Make_Struct(klass, rcl_pointer_t, 0, rcl_pointer_free_func, p);
}

static VALUE
rcl_pointer_init(VALUE self, VALUE type, VALUE size)
{
    rcl_pointer_t *p;
    Data_Get_Struct(self, rcl_pointer_t, p);
    
    if (!SYMBOL_P(type)) {
        rb_raise(rb_eTypeError, "Invalid type tag, Expected a Symbol.");
    }
    p->type = SYM2ID(type);
    if (!Is_Type_Valid(type)) {
        rb_raise(rb_eArgError, "Unrecognized type tag.");
    }
    if (!FIXNUM_P(size) || FIX2UINT(size) < 1) {
        rb_raise(rb_eArgError, "Invalid size.");
    }
    p->size = FIX2UINT(size);
    
    if (!Is_Immediate_Value(p)) {    
        // Allocate memory.
        size_t alloc_sz = p->size * Type_Size(p->type);
        if (p->size > 1) {
            alloc_sz += 0x80;     // align in 128bytes.
        }
        p->alloc_address = (int8_t *)ALLOC_N(int8_t, alloc_sz);
        if (p->alloc_address == NULL) {
            rb_raise(rb_eRuntimeError, "Out of host memory.");
        }
        p->address = (void *)(((intptr_t)(p->alloc_address) + 0x80) & ~0x7F);
    }
    return self;
}

static VALUE
rcl_pointer_init_copy(VALUE copy, VALUE orig)
{
    Expect_RCL_Type(orig, Pointer);
    
    rcl_pointer_t *copy_p, *orig_p;
    Data_Get_Struct(copy, rcl_pointer_t, copy_p);
    Data_Get_Struct(orig, rcl_pointer_t, orig_p);
    
    assert(copy_p->address != orig_p->address);
}

/*
 * call-seq:
 *      Pointer#[0] ->  1.234 
 * 
 * Returns the n-th element stored in the memory region managed by 
 * the receiver.
 */
static VALUE
rcl_pointer_aref(VALUE self, VALUE index)
{
    rcl_pointer_t *p = Pointer_Ptr(self);
    Extract_Size(index, i);
    
    if (p->size == 0 || i > p->size - 1) {
        rb_raise(rb_eRuntimeError, "");
    }
    
    return Is_Immediate_Value(p) ? Native2Ruby(p->type, &p->address) : Native2Ruby(p->type, p->address);
}

static VALUE
rcl_pointer_aset(VALUE self, VALUE index, VALUE value)
{
    rcl_pointer_t *p = Pointer_Ptr(self);
    Extract_Size(index, i);
    
    if (p->size == 0 || i > p->size - 1) {
        rb_raise(rb_eRuntimeError, "");
    }
    
    Is_Immediate_Value(p) ? Ruby2Native(p->type, &p->address, value) : Ruby2Native(p->type, p->address, value);
    return self;
}

static VALUE
rcl_pointer_address(VALUE self)
{    
    rcl_pointer_t *p = Pointer_Ptr(self);
    
    if (p->alloc_address == NULL) {
        return Qnil;
    }
    cl_ulong a = (cl_ulong)(Is_Immediate_Value(p) ? &(p->address) : p->address);
    return UINT2FIX(a);
}

static VALUE
rcl_pointer_type(VALUE self)
{    
    return ID2SYM(Pointer_Ptr(self)->type);
}

static VALUE
rcl_pointer_size(VALUE self)
{    
    return UINT2FIX(Pointer_Ptr(self)->size);
}

static VALUE
rcl_pointer_free(VALUE self)
{
    rcl_pointer_t *ptr = Pointer_Ptr(self);
    if (ptr->alloc_address == NULL) {
        return self;
    }   
    if (!Is_Immediate_Value(ptr)) {
        free(ptr->alloc_address);        
    }
    
    ptr->alloc_address = NULL;
    ptr->address = NULL;
    ptr->size = 0;
    
    return self;
}

static VALUE
rcl_pointer_set(VALUE self, VALUE value)
{
    return self;
}

static VALUE
rcl_pointer_copy(VALUE self, VALUE dst)
{
    return self;
}

static VALUE
rcl_pointer_ncopy(VALUE self, VALUE src, VALUE size)
{
    return self;
}

static void
define_class_pointer(void)
{
    define_cl_types();
    
    rb_cPointer = rb_define_class_under(rcl_mOpenCL, "Pointer", rb_cObject);
    rb_define_alloc_func(rb_cPointer, rcl_pointer_alloc);
    rb_define_method(rb_cPointer, "initialize", rcl_pointer_init, 2);
    rb_define_method(rb_cPointer, "initialize_copy", rcl_pointer_init_copy, 1);
    rb_define_method(rb_cPointer, "[]", rcl_pointer_aref, 1);
    rb_define_method(rb_cPointer, "[]=", rcl_pointer_aset, 2);
    rb_define_method(rb_cPointer, "address", rcl_pointer_address, 0);
    rb_define_method(rb_cPointer, "type", rcl_pointer_type, 0);
    rb_define_method(rb_cPointer, "size", rcl_pointer_size, 0);
    rb_define_method(rb_cPointer, "free", rcl_pointer_free, 0);
    rb_define_method(rb_cPointer, "set", rcl_pointer_set, 1);
    rb_define_method(rb_cPointer, "copy", rcl_pointer_copy, 1);
    rb_define_method(rb_cPointer, "ncopy", rcl_pointer_ncopy, 2);
}

/*
 * class Platform
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
    
    VALUE list = rb_ary_new();
    cl_int res = clGetPlatformIDs(16, p_ids, &num_p);   // CHECK: ditto.
    if (CL_SUCCESS == res) {
        for(int i = 0; i < num_p; i++) {
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
rcl_platform_info(VALUE self, VALUE platform_info)
{
    Expect_RCL_Const(platform_info);
    cl_platform_info info = FIX2UINT(platform_info);
    
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
    rb_cPlatform = rb_define_class_under(rcl_mCapi, "Platform", rb_cObject);
    rb_define_module_function(rcl_mCapi, "platforms", rcl_platforms, 0);
    rb_define_alloc_func(rb_cPlatform, rcl_platform_alloc);
    rb_define_method(rb_cPlatform, "info", rcl_platform_info, 1);    
}

/*
 * class Device
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
rcl_devices(VALUE self, VALUE device_type, VALUE platform)
{
    cl_platform_id pid = NIL_P(platform) ? NULL : Platform_Ptr(platform);

    Expect_RCL_Const(device_type);
    cl_device_type dt = FIX2UINT(device_type);

    cl_device_id d_ids[256];    // CHECK: literal constant and correct value.
    cl_uint num_id;
    cl_int res;
    
    res = clGetDeviceIDs(pid, dt, 256, d_ids, &num_id); // CHECK: ditto.
    
    VALUE devs = rb_ary_new();
    if (res != CL_SUCCESS) {
        Check_And_Warn(res);
    } else {
        for (int i = 0; i < num_id; i++) {
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
rcl_device_info(VALUE self, VALUE device_info)
{
    Expect_RCL_Const(device_info);
    cl_device_info info = FIX2UINT(device_info);
    
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
    rb_cDevice = rb_define_class_under(rcl_mCapi, "Device", rb_cObject);
    rb_define_module_function(rcl_mCapi, "devices", rcl_devices, 2);
    rb_define_alloc_func(rb_cDevice, rcl_device_alloc);
    rb_define_method(rb_cDevice, "info", rcl_device_info, 1);
    rb_define_method(rb_cDevice, "available?", rcl_device_is_available, 0);
    rb_define_attr(rb_cDevice, "type", 1, 0);    
}

/*
 * class Context
 */
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
    for (int i = 0; i < len; i += 2) {
        VALUE pn = rb_ary_entry(arr, i);
        VALUE ptr = rb_ary_entry(arr, i + 1);
        
        assert(!(NIL_P(pn) || NIL_P(ptr)));
                
        props[i] = FIX2UINT(pn);
        switch (props[i]) {
            case CL_CONTEXT_PLATFORM:
                props[i+1] = (cl_context_properties)DATA_PTR(ptr);
                break;
            default:
                rb_raise(rb_eArgError, "Invalid context property.");
        }
    }
}

static VALUE
build_device_array(cl_device_id *devs, size_t cb)
{
    VALUE ret = rb_ary_new();
    uint num_dev = cb / sizeof(cl_device_id);
    
    for (int i = 0; i < num_dev; i++) {
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
        dev_type = FIX2UINT(darg);
    } else {
        rb_raise(rb_eArgError, "Invalid argument. Expected device type or device array.");
    }
    
    cl_int res;
    cl_context context;
    
    if (!NIL_P(devs)) {
        cl_uint num_dev = RARRAY_LEN(devs);
        cl_device_id *dev_ids;
        CL_Pointers(devs, Device, cl_device_id, dev_ids);
        
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
    Expect_RCL_Type(orig, Context);
    
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
rcl_context_info(VALUE self, VALUE context_info)
{
    Expect_RCL_Const(context_info); 
    cl_context_info iname = FIX2UINT(context_info);       
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
    rb_cContext = rb_define_class_under(rcl_mCapi, "Context", rb_cObject);
    rb_define_alloc_func(rb_cContext, rcl_context_alloc);
    rb_define_method(rb_cContext, "initialize", rcl_context_init, 2);
    rb_define_method(rb_cContext, "initialize_copy", 
                                  rcl_context_init_copy, 1);
    rb_define_method(rb_cContext, "info", rcl_context_info, 1);
}

/*
 * class ImageFormat
 */
static VALUE
rcl_image_format_init(VALUE self, VALUE channel_order, VALUE channel_data_type)
{
    Expect_RCL_Const(channel_order);
    Expect_RCL_Const(channel_data_type);
    
    rb_iv_set(self, "@channel_order", channel_order);
    rb_iv_set(self, "@channel_data_type", channel_data_type);
    
    return self;
}
 
static VALUE
rcl_context_supported_image_formats(VALUE self, VALUE mem_flag, VALUE mem_obj_type)
{
    Expect_RCL_Const(mem_obj_type);
    cl_mem_object_type mt = FIX2INT(mem_obj_type);
    Expect_RCL_Const(mem_flag);
    cl_mem_flags mf = FIX2UINT(mem_flag);
    
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
    for (int i = 0; i < num_ret; i++) {
        rb_ary_push(ret, RImageFormat(ifs + i));
    }
    return ret;
}

static void
define_class_image_format(void)
{
    rb_cImageFormat = rb_define_class_under(rcl_mCapi, "ImageFormat", rb_cObject);
    rb_define_attr(rb_cImageFormat, "channel_order", 1, 0);
    rb_define_attr(rb_cImageFormat, "channel_data_type", 1, 0);
    rb_define_method(rb_cImageFormat, "initialize", rcl_image_format_init, 2);
    rb_define_method(rb_cContext, "supported_image_formats", 
                                  rcl_context_supported_image_formats, 2);
                                  
}

/*
 * class CommandQueue
 */
static VALUE
rcl_command_queue_alloc(VALUE klass)
{
    return RCommandQueue(NULL);
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
    cl_context cxt = Context_Ptr(context);   
    cl_device_id did = Device_Ptr(device);
    
    Expect_RCL_Const(props);
    cl_uint properties = FIX2UINT(props);
    
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
    
    Expect_RCL_Type(orig, CommandQueue);
    
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
rcl_command_queue_info(VALUE self, VALUE command_queue_info)
{
    Expect_RCL_Const(command_queue_info);
    cl_command_queue_info info = FIX2UINT(command_queue_info);
    
    intptr_t param_value;   // CHECK: extensibility.
    size_t sz_ret;
    
    cl_command_queue cq = CommandQueue_Ptr(self);
    
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
    Expect_Boolean(yesno);
    
    cl_command_queue q = CommandQueue_Ptr(self);
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

#define Extract_Mem_Object(mem, var) \
    cl_mem var; \
    do { \
        Expect_RCL_Type(mem, Memory); \
        var = Memory_Ptr(mem); \
    } while (0)

#define Extract_Wait_For_Events(events, num_evt, pevts) \
    cl_uint num_evt; \
    cl_event *pevts; \
    do { \
        Expect_Array(events); \
        num_evt = RARRAY_LEN(events); \
        CL_Pointers(events, Event, cl_event, pevts); \
    } while (0)
    
#define Extract_Pointer(ptr, var) \
    void *var; \
    do { \
        Expect_RCL_Type(ptr, Pointer); \
        var = Pointer_Address(ptr); \
    } while (0)
    
#define Extract_Vector(vec, var) \
    cl_uint vec[3]; \
    do { \
        if (NIL_P(vec)) { \
            vec[0] = vec[1] = vec[2] = 0; \
        } else { \
            Expect_Array(vec); \
            if (RARRAY_LEN(vec) == 0) { \
                vec[0] = vec[1] = vec[2] = 0; \
            } else { \
                for (int i = 0; i < 3; i++) { \
                    VALUE n = rb_ary_entry(vec); \
                    Extract_Size(n, v); \
                    vec[i] = v; \
                } \
            } \
        } \
    } while (0)

static VALUE
rcl_cq_enqueue_read_buffer(VALUE self, VALUE buffer, VALUE blocking_read, 
                           VALUE offset, VALUE size, VALUE host_ptr, VALUE events)
{
    Extract_Mem_Object(buffer, buf);    
    Expect_Boolean(blocking_read);
    Extract_Size(offset, os);
    Extract_Size(size, cb);
    Extract_Pointer(host_ptr, ptr);
    Extract_Wait_For_Events(events, num_evt, pevts);
    
    cl_command_queue cq = CommandQueue_Ptr(self);
    cl_event e;
    cl_event *ep = blocking_read ? NULL : &e;
    
    cl_int res = clEnqueueReadBuffer(cq, buf, blocking_read, os, cb, ptr, num_evt, pevts, ep);
    Check_And_Raise(res);
    
    return blocking_read ? self : REvent(e);
}

static VALUE
rcl_cq_enqueue_write_buffer(VALUE self, VALUE buffer, VALUE blocking_write,
                            VALUE offset, VALUE size, VALUE host_ptr, VALUE events)
{
    Extract_Mem_Object(buffer, buf);    
    Expect_Boolean(blocking_write);
    Extract_Size(offset, os);
    Extract_Size(size, cb);
    Extract_Pointer(host_ptr, ptr);
    Extract_Wait_For_Events(events, num_evt, pevts);
    
    cl_command_queue cq = CommandQueue_Ptr(self);
    cl_event e;
    cl_event *ep = blocking_write ? NULL : &e;
    
    cl_int res = clEnqueueWriteBuffer(cq, buf, blocking_write, os, cb, ptr, num_evt, pevts, ep);
    Check_And_Raise(res);
    
    return blocking_write ? self : REvent(e);
}

static VALUE
rcl_cq_enqueue_copy_buffer(VALUE self, VALUE src_buffer, VALUE dst_buffer,
                           VALUE src_offset, VALUE dst_offset,
                           VALUE size, VALUE events)
{
    Extract_Mem_Object(src_buffer, sbuf);
    Extract_Mem_Object(dst_buffer, dbuf);
    Extract_Size(src_offset, sos);
    Extract_Size(dst_offset, dos);
    Extract_Size(size, cb);
    Extract_Wait_For_Events(events, num_evt, pevts);
    
    cl_command_queue cq = CommandQueue_Ptr(self);
    

    return self;
}

static VALUE
rcl_cq_enqueue_read_image(VALUE self, VALUE image, VALUE blocking_read,
                          VALUE origin, VALUE region,
                          VALUE row_pitch, VALUE slice_pitch, 
                          VALUE host_ptr, VALUE events)
{



    return self;
}

static VALUE
rcl_cq_enqueue_write_image(VALUE self, VALUE image, VALUE blocking_write,
                           VALUE origin, VALUE region,
                           VALUE row_pitch, VALUE slice_pitch, 
                           VALUE host_ptr, VALUE events)
{
    return self;
}

static VALUE
rcl_cq_enqueue_copy_image(VALUE self, VALUE src_image, VALUE dst_image,
                          VALUE src_origin, VALUE dst_origin, VALUE region,
                          VALUE events)
{
    return self;
}

static VALUE
rcl_cq_enqueue_copy_image_to_buffer(VALUE self, VALUE src_image, VALUE dst_buffer,
                                    VALUE src_origin, VALUE region,
                                    VALUE dst_offset, VALUE events)
{
    return self;
}

static VALUE
rcl_cq_enqueue_copy_buffer_to_image(VALUE self, VALUE src_buffer, VALUE dst_image,
                                    VALUE src_offset, VALUE dst_origin, 
                                    VALUE region, VALUE events)
{
    return self;
}

// static VALUE
// rcl_cq_enqueue_map_buffer(VALUE self, )
// {
//     return self;
// }
// 
// static VALUE
// rcl_cq_enqueue_map_image(VALUE self, )
// {
//     return self;
// }
// 
// static VALUE
// rcl_cq_enqueue_unmap_mem_obj(VALUE self, )
// {
//     return self;
// }

// static VALUE
// rcl_cq_enqueue_ndrange_kernel(VALUE self, )
// {
//     return self;
// }
// 
// static VALUE
// rcl_cq_enqueue_task(VALUE self, )
// {
//     return self;
// }
// 
// static VALUE
// rcl_cq_enqueue_native_kernel(VALUE self, )
// {
//     rb_notimplement();
//     return self;
// }

/*
 * call-seq:
 *      CommandQueue#enqueue_marker     -> Event
 *
 * Wrapps +clEnqueueMarker()+.
 */
static VALUE
rcl_cq_enqueue_marker(VALUE self)
{
    cl_command_queue cq = CommandQueue_Ptr(self);
    
    cl_event e;
    cl_int res = clEnqueueMarker(cq, &e);
    Check_And_Raise(res);
    
    return REvent(e);
}

/*
 * call-seq:
 *      CommandQueue#enqueue_waitfor_events     -> Receiver
 *
 * Wrapps +clEnqueueWaitForEvents()+.
 */
static VALUE
rcl_cq_enqueue_waitfor_events(VALUE self, VALUE events)
{
    Extract_Wait_For_Events(events, num_evt, pes);

    cl_command_queue cq = CommandQueue_Ptr(self);
    cl_int res = clEnqueueWaitForEvents(cq, num_evt, pes);
    Check_And_Raise(res);
    
    return self;
}

/*
 * call-seq:
 *      CommandQueue#enqueue_barrier     -> Receiver
 *
 * Wrapps +clEnqueueBarrier()+.
 */
static VALUE
rcl_cq_enqueue_barrier(VALUE self)
{
    cl_command_queue cq = CommandQueue_Ptr(self);
    cl_int res = clEnqueueBarrier(cq);
    Check_And_Raise(res);
    
    return self;
}

/*
 * call-seq:
 *      CommandQueue#enqueue_acquire_gl_objects([Memory, ..], nil)   -> Event
 *      CommandQueue#enqueue_acquire_gl_objects([], nil)     -> receiver
 *
 * Wrapps +clEnqueueAcquireGLObjects()+.
 */
static VALUE
rcl_cq_enqueue_acquire_gl_objects(VALUE self, VALUE mem_objects, VALUE events)
{
    Expect_Array(mem_objects);    
    cl_uint num_mo = RARRAY_LEN(mem_objects);
    if (num_mo == 0) return self;

    cl_uint num_evt = 0;
    cl_event *pevts = NULL;
    CL_Pointers(events, Event, cl_event, pevts);
    
    cl_mem *pmos;
    CL_Pointers(mem_objects, Memory, cl_mem, pmos);
    
    cl_event evt;
    cl_command_queue cq = CommandQueue_Ptr(self);
    cl_int res = clEnqueueAcquireGLObjects(cq, num_mo, pmos, num_evt, pevts, &evt);
    Check_And_Raise(res);
    
    return REvent(evt);
}

/*
 * call-seq:
 *      CommandQueue#enqueue_release_gl_objects     -> Event
 *
 * Wrapps +clEnqueueReleaseGLObjects()+.
 */
static VALUE
rcl_cq_enqueue_release_gl_objects(VALUE self, VALUE mem_objects, VALUE events)
{
    Expect_Array(mem_objects);    
    cl_uint num_mo = RARRAY_LEN(mem_objects);
    if (num_mo == 0) return self;

    cl_uint num_evt = 0;
    cl_event *pevts = NULL;
    CL_Pointers(events, Event, cl_event, pevts);
    
    cl_mem *pmos;
    CL_Pointers(mem_objects, Memory, cl_mem, pmos);
    
    cl_event evt;
    cl_command_queue cq = CommandQueue_Ptr(self);
    cl_int res = clEnqueueReleaseGLObjects(cq, num_mo, pmos, num_evt, pevts, &evt);
    Check_And_Raise(res);
    
    return REvent(evt);
}

static void
define_class_command_queue(void)
{
    rb_cCommandQueue = rb_define_class_under(rcl_mCapi, "CommandQueue", rb_cObject);
    rb_define_alloc_func(rb_cCommandQueue, rcl_command_queue_alloc);
    rb_define_method(rb_cCommandQueue, "initialize", rcl_command_queue_init, 3);
    rb_define_method(rb_cCommandQueue, "initialize_copy", rcl_command_queue_init_copy, 1);
    rb_define_method(rb_cCommandQueue, "info", rcl_command_queue_info, 1);
    rb_define_method(rb_cCommandQueue, "set_property", rcl_command_queue_set_property, 2);
    
    // mem
    rb_define_method(rb_cCommandQueue, "enqueue_read_buffer", rcl_cq_enqueue_read_buffer, 6);
    rb_define_method(rb_cCommandQueue, "enqueue_write_buffer", rcl_cq_enqueue_write_buffer, 6);
    rb_define_method(rb_cCommandQueue, "enqueue_copy_buffer", rcl_cq_enqueue_copy_buffer, 6);
    rb_define_method(rb_cCommandQueue, "enqueue_read_image", rcl_cq_enqueue_read_image, 8);
    rb_define_method(rb_cCommandQueue, "enqueue_write_image", rcl_cq_enqueue_write_image, 8);
    rb_define_method(rb_cCommandQueue, "enqueue_copy_image", rcl_cq_enqueue_copy_image, 6);
    rb_define_method(rb_cCommandQueue, "enqueue_copy_image_to_buffer", rcl_cq_enqueue_copy_image_to_buffer, 6);
    rb_define_method(rb_cCommandQueue, "enqueue_copy_buffer_to_image", rcl_cq_enqueue_copy_buffer_to_image, 6);
    // rb_define_method(rb_cCommandQueue, "enqueue_map_buffer", rcl_cq_enqueue_map_buffer, );
    // rb_define_method(rb_cCommandQueue, "enqueue_map_image", rcl_cq_enqueue_map_image, );
    // rb_define_method(rb_cCommandQueue, "enqueue_unmap_mem_object", rcl_cq_enqueue_unmap_mem_obj, );
    // 
    // // execution
    // rb_define_method(rb_cCommandQueue, "enqueue_NDRange_kernel", rcl_cq_enqueue_ndrange_kernel, 6);
    // rb_define_method(rb_cCommandQueue, "enqueue_task", rcl_cq_enqueue_task, 2);
    // rb_define_method(rb_cCommandQueue, "enqueue_native_kernel", rcl_cq_enqueue_native_kernel, 5);
    
    // sync 
    rb_define_method(rb_cCommandQueue, "enqueue_marker",rcl_cq_enqueue_marker, 0);
    rb_define_method(rb_cCommandQueue, "enqueue_wait_for_events", rcl_cq_enqueue_waitfor_events, 1);
    rb_define_method(rb_cCommandQueue, "enequeu_barrier", rcl_cq_enqueue_barrier, 0);
    
    rb_define_method(rb_cCommandQueue, "flush", rcl_flush, 0);
    rb_define_method(rb_cCommandQueue, "finish", rcl_finish, 0);
    
    // gl interoperation
    rb_define_method(rb_cCommandQueue, "enqueue_acquire_gl_objects", rcl_cq_enqueue_acquire_gl_objects, 2);
    rb_define_method(rb_cCommandQueue, "enqueue_release_gl_objects", rcl_cq_enqueue_release_gl_objects, 2);
    
}

/*
 * class Sampler
 */
static VALUE
rcl_sampler_alloc(VALUE klass)
{    
    rcl_sampler_t *ps;
    return Data_Make_Struct(klass, rcl_sampler_t, 0, rcl_sampler_free, ps);
}

static VALUE
rcl_sampler_init(VALUE self, VALUE context, VALUE normalized_coords, 
                 VALUE addressing_mode, VALUE filter_mode)
{
    cl_context cxt = Context_Ptr(context);
    
    Expect_Boolean(normalized_coords);    
    
    Expect_RCL_Const(addressing_mode);
    Expect_RCL_Const(filter_mode);    
    cl_addressing_mode am = FIX2UINT(addressing_mode);
    cl_filter_mode fm = FIX2UINT(filter_mode);
    
    cl_int res;
    cl_sampler s = clCreateSampler(cxt, normalized_coords, am, fm, &res);
    Check_And_Raise(res);
    
    return Data_Wrap_Struct(rb_cSampler, 0, rcl_sampler_free, s);
}

static VALUE
rcl_sampler_init_copy(VALUE copy, VALUE orig)
{
    if (copy == orig) return copy;
    
    Expect_RCL_Type(orig, Sampler);
    
    rcl_sampler_t *copy_p;
    Data_Get_Struct(copy, rcl_sampler_t, copy_p);
    rcl_sampler_t *orig_p;
    Data_Get_Struct(orig, rcl_sampler_t, orig_p);
    
    if (copy_p->s == orig_p->s) return copy;
    
    cl_int res;
    if (copy_p->s != NULL) {
        res = clReleaseSampler(copy_p->s);
        Check_And_Raise(res);
    }
    
    res = clRetainSampler(orig_p->s);
    Check_And_Raise(res);
    
    copy_p->s = orig_p->s;
    
    return copy;
}

static VALUE
rcl_sampler_info(VALUE self, VALUE sampler_info)
{
    Expect_RCL_Const(sampler_info);
    cl_sampler_info si = FIX2UINT(sampler_info);
    
    cl_sampler s = Sampler_Ptr(self);
    
    cl_int res;
    intptr_t param_value = 0;   // CHECK: again, extensibility.
                                // The best way is to get size first.
    
    res = clGetSamplerInfo(s, si, sizeof(intptr_t), &param_value, NULL);
    Check_And_Raise(res);
    
    switch (si) {
    case CL_SAMPLER_REFERENCE_COUNT:
    case CL_SAMPLER_ADDRESSING_MODE:
    case CL_SAMPLER_FILTER_MODE:
        return UINT2FIX(param_value);
    case CL_SAMPLER_CONTEXT:
        return RContext((cl_context)param_value);
    case CL_SAMPLER_NORMALIZED_COORDS:
        return param_value ? Qtrue : Qfalse;
    default:
        break;
    }
    return Qnil;
}

static void
define_class_sampler(void)
{
    rb_cSampler = rb_define_class_under(rcl_mCapi, "Sampler", rb_cObject);
    rb_define_alloc_func(rb_cSampler, rcl_sampler_alloc);
    rb_define_method(rb_cSampler, "initialize", rcl_sampler_init, 4);
    rb_define_method(rb_cSampler, "initialize_copy", rcl_sampler_init_copy, 1);
    rb_define_method(rb_cSampler, "info", rcl_sampler_info, 1);       
}

/*
 * class Event
 */
static VALUE
rcl_event_alloc(VALUE klass)
{
    rb_raise(rb_eRuntimeError, "Can't instantiate Event.");
    return Qnil;
}

static VALUE
rcl_event_init_copy(VALUE copy, VALUE orig)
{
    if (copy == orig) return copy;
    
    Expect_RCL_Type(copy, Event);
    Expect_RCL_Type(orig, Event);
    
    rcl_event_t *copy_p, *orig_p;
    Data_Get_Struct(copy, rcl_event_t, copy_p);
    Data_Get_Struct(orig, rcl_event_t, orig_p);
    
    if (copy_p->e == orig_p->e) return copy;
    
    cl_int res;
    if (copy_p->e != NULL) {
        res = clReleaseEvent(copy_p->e);
        Check_And_Raise(res);
    }
    res = clRetainEvent(orig_p->e);
    copy_p->e = orig_p->e;
    
    return copy;
}

static VALUE
rcl_event_info(VALUE self, VALUE event_info)
{
    Expect_RCL_Const(event_info);
    cl_event_info info = FIX2UINT(event_info);
    
    cl_event e = Event_Ptr(self);
    intptr_t param_value;
    
    cl_int res = clGetEventInfo(e, info, sizeof(intptr_t), &param_value, NULL);
    Check_And_Raise(res);
    
    switch (info) {
        case CL_EVENT_COMMAND_QUEUE:
            return RCommandQueue((cl_command_queue)param_value);
        case CL_EVENT_COMMAND_TYPE:
        case CL_EVENT_REFERENCE_COUNT:
            return UINT2FIX((cl_uint)param_value);
        case CL_EVENT_COMMAND_EXECUTION_STATUS:
            return INT2FIX((cl_int)param_value);
        default:
            break;
    }
    return Qnil;
    
}

static VALUE
rcl_wait_for_events(VALUE self, VALUE events)
{
    Expect_Array(events);
    cl_uint num = RARRAY_LEN(events);
    if (num == 0) return Qfalse;
    
    cl_event *pe;
    CL_Pointers(events, Event, cl_event, pe);

    cl_int res = clWaitForEvents(num, pe);
    Check_And_Raise(res);
    
    return Qtrue;
}

static VALUE
rcl_event_profiling_info(VALUE self, VALUE profiling_info)
{
    Expect_RCL_Const(profiling_info);
    cl_profiling_info itype = FIX2UINT(profiling_info);
    
    cl_ulong info;    
    cl_event e = Event_Ptr(self);
    
    cl_int res = clGetEventProfilingInfo(e, itype, sizeof(cl_ulong), &info, NULL);    
    Check_And_Raise(res);
    
    return ULONG2NUM(info);
}

static void
define_class_event(void)
{
    rb_cEvent = rb_define_class_under(rcl_mCapi, "Event", rb_cObject);
    rb_define_alloc_func(rb_cEvent, rcl_event_alloc);
    rb_define_method(rb_cEvent, "initialize_copy", rcl_event_init_copy, 1);
    rb_define_method(rb_cEvent, "info", rcl_event_info, 1);
    rb_define_module_function(rcl_mCapi, "wait_for_events", rcl_wait_for_events, 1);
    rb_define_method(rb_cEvent, "profiling_info", rcl_event_profiling_info, 1);
}

/*
 * class Memory
 */
static VALUE
rcl_mem_alloc(VALUE klass)
{
    rb_raise(rb_eRuntimeError, "Use class methods instead.");
    return Qnil;
}

static VALUE
rcl_mem_info(VALUE self, VALUE param_name)
{
    Expect_RCL_Const(param_name);
    
    cl_mem_info mi = FIX2UINT(param_name);
    cl_mem m = Memory_Ptr(self);
    cl_ulong param_value;
    
    cl_int res = clGetMemObjectInfo(m, mi, sizeof(intptr_t), &param_value, NULL);
    Check_And_Raise(res);
    
    switch (mi) {
    case CL_MEM_TYPE:
    case CL_MEM_FLAGS:
    case CL_MEM_SIZE:
    case CL_MEM_HOST_PTR:
    case CL_MEM_MAP_COUNT:
    case CL_MEM_REFERENCE_COUNT:
        return UINT2FIX(param_value);
    case CL_MEM_CONTEXT:
        return RContext((cl_context)param_value);
    default:
        break;
    }
    
    return Qnil;
}

static VALUE
rcl_mem_image_info(VALUE self)
{
    
}

static VALUE
rcl_mem_gl_obj_info(VALUE self)
{
    
}

static VALUE
rcl_mem_gl_texture_info(VALUE self)
{
    
}

static void
define_class_memory(void)
{
    rb_cMemory = rb_define_class_under(rcl_mCapi, "Memory", rb_cObject);
    rb_define_alloc_func(rb_cMemory, rcl_mem_alloc);
    // rb_define_method(rb_cMemory, "initialize", rcl_mem_init, -1);
    // rb_deinfe_singleton_method(rb_cMemory, "create_buffer", rcl_mem_create_buffer, -1);
    // rb_define_singleton_method(rb_cMemory, "create_image2d", rcl_mem_create_image_2d, -1);
    // rb_define_singleton_method(rb_cMemory, "create_image3d", rcl_mem_create_image_3d, -1);
    rb_define_method(rb_cMemory, "info", rcl_mem_info, 1);
    rb_define_method(rb_cMemory, "image_info", rcl_mem_image_info, -1);

    // GL sharing API.
    // rb_define_singleton_method(rb_cMemory, "create_from_gl_buffer", rcl_mem_create_from_gl_buffer, -1);
    // rb_define_singleton_method(rb_cMemory, "create_from_gl_render_buffer", rcl_mem_create_from_gl_render_buffer, -1);
    // rb_define_singleton_method(rb_cMemory, "create_from_gl_texture_2d", rcm_mem_create_from_gl_texture_2d, -1);
    // rb_define_singleton_method(rb_cMemory, "create_from_gl_texture_3d", rb_mem_create_from_gl_texture_3d, -1);
    rb_define_method(rb_cMemory, "gl_object_info", rcl_mem_gl_obj_info, -1);
    rb_define_method(rb_cMemory, "gl_texture_info", rcl_mem_gl_texture_info, -1);
}

/*
 * class Program
 */
static VALUE
rcl_program_alloc(VALUE klass)
{
    return RProgram(NULL);
}

static cl_program
rcl_program_create_from_source(cl_context context, VALUE sources)
{
    Expect_Array(sources);
    uint num_src = RARRAY_LEN(sources);
    
    const char **srcp = (const char **)ALLOCA_N(intptr_t, num_src);
    size_t *lenp = ALLOCA_N(size_t, num_src);
    
    for (int i = 0; i < num_src; i++) {
        VALUE srcstr = rb_ary_entry(sources, i);
        if (TYPE(srcstr) != T_STRING) {
            rb_raise(rb_eTypeError, "Invalid source found. Not a String.");
        }
        srcp[i] = RSTRING_PTR(srcstr);
        lenp[i] = RSTRING_LEN(srcstr);
    }
    
    cl_int res;
    cl_program prog = clCreateProgramWithSource(context, num_src, srcp, lenp, &res);
    Check_And_Raise(res);
    
    return prog; 
}

static cl_program
rcl_program_create_from_binary(cl_context context, VALUE devices, VALUE binaries)
{
    Expect_Array(devices);
    Expect_Array(binaries);
    
    uint num_dev = RARRAY_LEN(devices);
    if (RARRAY_LEN(binaries) != num_dev) {
        rb_raise(rb_eArgError, "Number of binaries shall equal to number of devices.");
    }
    
    cl_device_id *devs = ALLOCA_N(cl_device_id, num_dev);
    size_t *len_ar = ALLOCA_N(size_t, num_dev);
    const unsigned char **bin_ar = (const unsigned char **)ALLOCA_N(intptr_t, num_dev);
    
    for (int i = 0; i < num_dev; i++) {
        VALUE dev = rb_ary_entry(devices, i);
        Expect_RCL_Type(dev, Device);
        
        VALUE bin = rb_ary_entry(binaries, i);
        if (TYPE(bin) != T_STRING) {
            rb_raise(rb_eTypeError, "Invalid binary. Expected a byte string.");
        }
        devs[i] = Device_Ptr(dev);
        len_ar[i] = RSTRING_LEN(bin);
        bin_ar[i] = (unsigned char *)RSTRING_PTR(bin);
    }
    
    cl_int res;
    cl_int *bin_status = ALLOCA_N(cl_int, num_dev);
    cl_program prog = clCreateProgramWithBinary(context, num_dev, devs, len_ar, bin_ar, bin_status, &res);
    Check_And_Raise(res);
    
    // FIXME: we got binary status, but do not use these information yet.
    
    return prog;
}

/*
 * call-seq:
 *      Program.new(Context, [String, ..])
 *      Program.new(Context, [Device, ..], [String, ..])
 *
 */
static VALUE
rcl_program_init(int argc, VALUE *argv, VALUE self)
{
    if (argc != 2 && argc != 3) {
        rb_raise(rb_eArgError, "Wrong number of arguments.");
    }
    
    Expect_RCL_Type(argv[0], Context);
    cl_context context = Context_Ptr(argv[0]);
    
    rcl_program_t *dp;
    Data_Get_Struct(self, rcl_program_t, dp);
    
    if (argc == 2) {
        VALUE sources = argv[1];
        
        dp->p = rcl_program_create_from_source(context, sources);
    } else {
        VALUE devices = argv[1];
        VALUE binaries = argv[2];
        
        dp->p = rcl_program_create_from_binary(context, devices, binaries);
    }
    
    return self;
}

static VALUE
rcl_program_init_copy(VALUE copy, VALUE orig)
{
    if (copy == orig) return copy;
    
    Expect_RCL_Type(orig, Program);
    rcl_program_t *copy_p, *orig_p;
    
    Data_Get_Struct(copy, rcl_program_t, copy_p);
    Data_Get_Struct(orig, rcl_program_t, orig_p);
    
    if (copy_p->p == orig_p->p) return copy;
    
    cl_int res;
    if (copy_p->p != NULL) {
        res = clReleaseProgram(copy_p->p);
        Check_And_Raise(res);
    }
    res = clRetainProgram(orig_p->p);
    Check_And_Raise(res);
    
    copy_p->p = orig_p->p;
    
    return copy;
}

typedef void (*pfn_build_notify)(cl_program, void *user_data);

static void
build_notify(cl_program program, void *user_data)
{
    // TODO: do something.
}

/*
 * call-seq:
 *      Program#build([Device, ..], "-D")   -> the receiver
 *      Program#build([Device, ..], "", memo) do |receiver, memo|
 * 
 * Wrapps +clProgramBuild()+.
 */
static VALUE
rcl_program_build(VALUE self, VALUE devices, VALUE options, VALUE memo)
{
    if (TYPE(options) != T_STRING) {
        rb_raise(rb_eTypeError, "Expected options is a String.");        
    }
    Expect_Array(devices);
    
    uint num_dev = RARRAY_LEN(devices);
    cl_device_id *devs;
    CL_Pointers(devices, Device, cl_device_id, devs);
        
    cl_program prog = Program_Ptr(self);
    pfn_build_notify pbn = NIL_P(memo) ? NULL : build_notify;
    cl_int res = clBuildProgram(prog, num_dev, devs, RSTRING_PTR(options), pbn, (void *)memo);
    Check_And_Raise(res);

    return self;
}

/*
 * call-seq:
 *      Program#build_info(Device, CL_PROGRAM_BUILD_LOG)    -> String
 *
 * Wrapps +clGetProgramBuildInfo()+.
 */
static VALUE
rcl_program_build_info(VALUE self, VALUE device, VALUE param_name)
{
    Expect_RCL_Type(device, Device);
    Expect_RCL_Const(param_name);
    
    cl_program prog = Program_Ptr(self);
    cl_device_id dev = Device_Ptr(device);
    cl_program_build_info bi = FIX2UINT(param_name);
    
    size_t sz_ret = 0;
    cl_int res = clGetProgramBuildInfo(prog, dev, bi, 0, NULL, &sz_ret);
    Check_And_Raise(res);
    
    char *param_value = ALLOCA_N(char, sz_ret);
    res = clGetProgramBuildInfo(prog, dev, bi, sz_ret, param_value, NULL);
    Check_And_Raise(res);
    
    switch (bi) {
    case CL_PROGRAM_BUILD_STATUS:
        return UINT2NUM(*(cl_build_status *)param_value);
    case CL_PROGRAM_BUILD_OPTIONS:
    case CL_PROGRAM_BUILD_LOG:
        return rb_str_new(param_value, sz_ret);
    default:
        break;
    }
    
    return Qnil;
}

/*
 * call-seq:
 *      
 *
 * Wrapps 
 */
static VALUE
rcl_program_info(VALUE self, VALUE param_name)
{
    Expect_RCL_Const(param_name);
    
    cl_program prog = Program_Ptr(self);
    cl_program_info pi = FIX2UINT(param_name);
    
    size_t sz_ret;
    cl_int res = clGetProgramInfo(prog, pi, 0, NULL, &sz_ret);
    Check_And_Raise(res);
    
    char *param_value = ALLOCA_N(char, sz_ret);
    res = clGetProgramInfo(prog, pi, sz_ret, param_value, NULL);
    Check_And_Raise(res);
    
    uint num_szs = 0;
    switch (pi) {
    case CL_PROGRAM_REFERENCE_COUNT:
    case CL_PROGRAM_NUM_DEVICES:
        return UINT2NUM(*(cl_uint *)param_value);
    case CL_PROGRAM_CONTEXT:
        return RContext(*(cl_context *)param_value);
    case CL_PROGRAM_DEVICES:
        return build_device_array((cl_device_id *)param_value, sz_ret);
    case CL_PROGRAM_BINARY_SIZES:
        num_szs = sz_ret / sizeof(size_t);
        VALUE szs = rb_ary_new2(num_szs);
        for (int i = 0; i < num_szs; i++) {
            rb_ary_push(szs, UINT2NUM(((size_t *)(param_value))[i]));
        }
        return szs;
    case CL_PROGRAM_SOURCE:
        return rb_str_new2(param_value);
    case CL_PROGRAM_BINARIES:
        rb_notimplement();
        break;
    default:
        break;
    }
    
    return Qnil;
}

/*
 * call-seq:
 *      Program#create_kernels  -> [Kernel, ...]
 *
 * Wrapps +clCreateKernelsInProgram()+.
 */
static VALUE
rcl_program_create_kernels(VALUE self)
{
    cl_kernel kernels[128];
    cl_program prog = Program_Ptr(self);
    
    cl_uint num_ret;
    cl_int res = clCreateKernelsInProgram(prog, 128, kernels, &num_ret);
    Check_And_Raise(res);
    
    VALUE ary = rb_ary_new2(num_ret);
    for (int i = 0; i < num_ret; i++) {
        rb_ary_push(ary, RKernel(kernels[i]));
    }
    return ary;
}

/*
 * call-seq:
 *      Capi.unload_compiler    -> Capi
 *
 * Wrapps +clUnloadCompiler()+.
 */

static VALUE
rcl_unload_compiler(VALUE self)
{
    cl_int res = clUnloadCompiler();
    Check_And_Raise(res);
    
    return self;
}

static void
define_class_program(void)
{
    rb_cProgram = rb_define_class_under(rcl_mCapi, "Program", rb_cObject);
    rb_define_alloc_func(rb_cProgram, rcl_program_alloc);
    rb_define_method(rb_cProgram, "initialize", rcl_program_init, -1);
    rb_define_method(rb_cProgram, "initialize_copy", rcl_program_init_copy, 1);
    rb_define_method(rb_cProgram, "build", rcl_program_build, 3);
    rb_define_method(rb_cProgram, "build_info", rcl_program_build_info, 2);
    rb_define_method(rb_cProgram, "info", rcl_program_info, 1);
    rb_define_method(rb_cProgram, "create_kernels", rcl_program_create_kernels, 0);
    rb_define_module_function(rcl_mCapi, "unload_compiler", rcl_unload_compiler, 0);
}

/*
 * class Kernel
 */
static VALUE
rcl_kernel_alloc(VALUE klass)
{
    return RKernel(NULL);
}

/*
 * call-seq:
 *      
 *
 * Wrapps 
 */
static VALUE
rcl_kernel_init(VALUE self, VALUE program, VALUE name)
{
    Expect_RCL_Type(program, Program);
    Check_Type(name, T_STRING);
    
    cl_program prog = Program_Ptr(program);
    char *str = RSTRING_PTR(name);
    
    cl_int res;
    cl_kernel k = clCreateKernel(prog, str, &res);
    Check_And_Raise(res);
    
    rcl_kernel_t *pk;
    Data_Get_Struct(self, rcl_kernel_t, pk);
    
    pk->k = k;
    return self;
}

/*
 * call-seq:
 *      
 *
 * Wrapps 
 */
static VALUE
rcl_kernel_init_copy(VALUE copy, VALUE orig)
{
    if (copy == orig) return copy;
    Expect_RCL_Type(orig, Kernel);
    
    rcl_kernel_t *copy_p, *orig_p;
    Data_Get_Struct(copy, rcl_kernel_t, copy_p);
    Data_Get_Struct(orig, rcl_kernel_t, orig_p);
    
    if (copy_p->k == orig_p->k) return copy;
    
    cl_int res;
    if (copy_p->k != NULL) {
        res = clReleaseKernel(copy_p->k);
        Check_And_Raise(res);
    }
    res = clRetainKernel(orig_p->k);
    Check_And_Raise(res);
    
    copy_p->k = orig_p->k;
    return copy;
}

/*
 * call-seq:
 *      
 *
 * Wrapps +clSetKernelArg()+.
 */
static VALUE
rcl_kernel_set_arg(VALUE self, VALUE index, VALUE arg_value)
{
    if (!FIXNUM_P(index)) {
        rb_raise(rb_eArgError, "Expected index is of type Fixnum.");
    }
    
    size_t arg_size;
    void *arg_ptr;
    
    VALUE klass = CLASS_OF(arg_value);
    if (klass == rb_cPointer) {
        ;
    } else if (klass == rb_cSampler) {
        arg_value = (void *)Sampler_Ptr(arg_value);
        arg_size = sizeof(cl_sampler); 
    } else if (NIL_P(arg_value)) {
        arg_value = NULL;
        arg_size = 0;        
    } else if (klass == rb_cEvent) {
        ;
    } else {
        rb_raise(rb_eArgError, "Invalid kernel argument type.");
    }
    
    cl_kernel k = Kernel_Ptr(self);
    cl_int res = clSetKernelArg(self, FIX2UINT(index), arg_size, arg_ptr);
    Check_And_Raise(res);
    
    return self;
}

/*
 * call-seq:
 *      Kernel#info(CL_KERNEL_FUNCTION_NAME)    -> "filter"
 *
 * Wrapps +clGetKernelInfo()+.
 */
static VALUE
rcl_kernel_info(VALUE self, VALUE param_name)
{
    Expect_RCL_Const(param_name);
    
    cl_kernel k = Kernel_Ptr(self);
    cl_kernel_info ki = FIX2UINT(param_name);
    
    char param_value[128];
    size_t sz_ret;
    cl_int res = clGetKernelInfo(k, ki, 128, param_value, &sz_ret);
    Check_And_Raise(res);
    
    switch (ki) {
    case CL_KERNEL_FUNCTION_NAME:
        return rb_str_new(param_value, sz_ret);
    case CL_KERNEL_NUM_ARGS:
    case CL_KERNEL_REFERENCE_COUNT:
        return UINT2NUM((*(cl_uint *)param_value));        
    case CL_KERNEL_CONTEXT:
        return RContext(*(cl_context *)param_value);
    case CL_KERNEL_PROGRAM:
        return RProgram(*(cl_program *)param_value);
    default:
        break;
    }
    return Qnil;
}

/*
 * call-seq:
 *      Kernel#workgroup_info(Device, CL_KERNEL_WORK_GROUP_SIZE)
 *
 * Wrapps +clGetKernelWorkGroupInfo()+.
 */
static VALUE
rcl_kernel_workgroup_info(VALUE self, VALUE device, VALUE param_name)
{
    Expect_RCL_Type(device, Device);
    Expect_RCL_Const(param_name);
    
    cl_kernel k = Kernel_Ptr(self);
    cl_device_id dev = Device_Ptr(device);
    cl_kernel_work_group_info kwi = FIX2UINT(param_name);
    
    size_t param_value[3];
    
    cl_uint res = clGetKernelWorkGroupInfo(k, dev, kwi, sizeof(size_t) * 3, param_value, NULL);
    Check_And_Raise(res);
    
    VALUE ret = Qnil;
    switch (kwi) {
    case CL_KERNEL_WORK_GROUP_SIZE:
    case CL_KERNEL_LOCAL_MEM_SIZE:
        return ULONG2NUM(param_value[0]);
    case CL_KERNEL_COMPILE_WORK_GROUP_SIZE:
        ret = rb_ary_new2(3);
        for (int i = 0; i < 3; i++) {
            rb_ary_push(ret, ULONG2NUM(param_value[i]));
        }
        return ret;
    default:
        break;
    }
    return Qnil;
}

static void
define_class_kernel(void)
{
    rb_cKernel = rb_define_class_under(rcl_mCapi, "Kernel", rb_cObject);
    rb_define_alloc_func(rb_cKernel, rcl_kernel_alloc);
    rb_define_method(rb_cKernel, "initialize", rcl_kernel_init, 2);
    rb_define_method(rb_cKernel, "initialize_copy", rcl_kernel_init_copy, 1);
    rb_define_method(rb_cKernel, "set_arg", rcl_kernel_set_arg, 2);
    rb_define_method(rb_cKernel, "info", rcl_kernel_info, 1);
    rb_define_method(rb_cKernel, "workgroup_info", rcl_kernel_workgroup_info, 2);
}

/*
 * Entry point
 */
 
void 
Init_capi()
{
    rcl_mOpenCL = rb_define_module("OpenCL");
    rcl_mCapi = rb_define_module_under(rcl_mOpenCL, "Capi");
    
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
    
    define_class_pointer();
}
