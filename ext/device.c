
#include "capi.h"

/*
 * call-seq:
 *      Capi.devices(CL_DEVICE_TYPE_GPU, platform)  -> Array of Device objects.
 *
 * Wrapps +clGetDeviceIDs()+
 */
static VALUE
rcl_devices(VALUE self, VALUE device_type, VALUE platform)
{
    cl_platform_id pid = NIL_P(platform) ? NULL : PlatformPtr(platform);

    EXPECT_RCL_CONST(device_type);
    cl_device_type dt = FIX2UINT(device_type);

    cl_device_id d_ids[256];    // CHECK: literal constant and correct value.
    cl_uint num_id;
    cl_int res;

    res = clGetDeviceIDs(pid, dt, 256, d_ids, &num_id);

    VALUE devs = rb_ary_new();
    if (res != CL_SUCCESS) {
        CHECK_AND_WARN(res);
    } else {
        for (cl_uint i = 0; i < num_id; i++) {
            VALUE o = RDevice(d_ids[i]);
            rb_ary_push(devs, o);
        }
    }
    return devs;
}

/*
 * call-seq:
 *      aDevice.info(CL_DEVICE_VENDOR)
 *
 * Wrapps +clGetDeviceInfo()+
 */
static VALUE
rcl_device_info(VALUE self, VALUE device_info)
{
    EXPECT_RCL_CONST(device_info);
    cl_device_info info = FIX2UINT(device_info);

    cl_int res;
    void *param_value;
    size_t param_value_size;

    cl_device_id device = DevicePtr(self);
    res = clGetDeviceInfo(device, info, 0, NULL, &param_value_size);
    CHECK_AND_RAISE(res);

    param_value = ALLOCA_N(int8_t, param_value_size);
    res = clGetDeviceInfo(device, info, param_value_size, param_value, NULL);
    CHECK_AND_RAISE(res);

    VALUE ret = Qnil;
    switch (info) {
        // cl_device_fp_config
        case CL_DEVICE_DOUBLE_FP_CONFIG:
        // case CL_DEVICE_HALF_FP_CONFIG:   // CHECK: undefined in SL 10.6.3.
        case CL_DEVICE_SINGLE_FP_CONFIG:
         // cl_device_mem_cache_type
        case CL_DEVICE_GLOBAL_MEM_CACHE_TYPE:
        // cl_device_local_mem_type
        case CL_DEVICE_LOCAL_MEM_TYPE:
        // cl_command_queue_properties
        case CL_DEVICE_QUEUE_PROPERTIES:
        // cl_device_exec_capabilities
        case CL_DEVICE_EXECUTION_CAPABILITIES:
        // cl_device_type
        case CL_DEVICE_TYPE:
            return INT2FIX(*(int *)param_value);
        // char[]
        case CL_DEVICE_EXTENSIONS:
        case CL_DEVICE_NAME:
        case CL_DEVICE_PROFILE:
        case CL_DEVICE_VENDOR:
        case CL_DEVICE_VERSION:
        case CL_DRIVER_VERSION:
#ifdef CL_VERSION_1_1
        case CL_DEVICE_OPENCL_C_VERSION:
#endif
            return rb_str_new2((char *)param_value);
        // cl_bool
        case CL_DEVICE_AVAILABLE:
        case CL_DEVICE_COMPILER_AVAILABLE:
        case CL_DEVICE_ENDIAN_LITTLE:
        case CL_DEVICE_ERROR_CORRECTION_SUPPORT:
        case CL_DEVICE_IMAGE_SUPPORT:
#ifdef CL_VERSION_1_1
        case CL_DEVICE_HOST_UNIFIED_MEMORY:
#endif
            return *(cl_bool *)param_value == CL_TRUE ? Qtrue : Qfalse;
        // cl_uint
        case CL_DEVICE_ADDRESS_BITS:
        case CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE:
        case CL_DEVICE_MAX_CLOCK_FREQUENCY:
        case CL_DEVICE_MAX_COMPUTE_UNITS:
        case CL_DEVICE_MAX_CONSTANT_ARGS:
        case CL_DEVICE_MAX_READ_IMAGE_ARGS:
        case CL_DEVICE_MAX_SAMPLERS:
        case CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS:
        case CL_DEVICE_MAX_WRITE_IMAGE_ARGS:
        case CL_DEVICE_MEM_BASE_ADDR_ALIGN:
        case CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE:
        case CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR:
        case CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT:
        case CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT:
        case CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG:
        case CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT:
        case CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE:
        case CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF:
        case CL_DEVICE_VENDOR_ID:
#ifdef CL_VERSION_1_1
        case CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR:
        case CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT:
        case CL_DEVICE_NATIVE_VECTOR_WIDTH_INT:
        case CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG:
        case CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT:
        case CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE:
        case CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF:
#endif
            return UINT2NUM(*(cl_uint *)param_value);
        // cl_ulong
        case CL_DEVICE_GLOBAL_MEM_CACHE_SIZE:
        case CL_DEVICE_GLOBAL_MEM_SIZE:
        case CL_DEVICE_LOCAL_MEM_SIZE:
        case CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE:
        case CL_DEVICE_MAX_MEM_ALLOC_SIZE:
        // size_t
        case CL_DEVICE_IMAGE2D_MAX_HEIGHT:
        case CL_DEVICE_IMAGE2D_MAX_WIDTH:
        case CL_DEVICE_IMAGE3D_MAX_DEPTH:
        case CL_DEVICE_IMAGE3D_MAX_HEIGHT:
        case CL_DEVICE_IMAGE3D_MAX_WIDTH:
        case CL_DEVICE_MAX_PARAMETER_SIZE:
        case CL_DEVICE_MAX_WORK_GROUP_SIZE:
        case CL_DEVICE_PROFILING_TIMER_RESOLUTION:
            return ULONG2NUM(*(cl_ulong *)param_value);
        // size_t[]
        case CL_DEVICE_MAX_WORK_ITEM_SIZES: {
            size_t n = param_value_size / sizeof(size_t);
            if (n == 0) return Qnil;
            VALUE ary = rb_ary_new2(n);
            size_t *szp = (size_t *)param_value;
            for (cl_uint i = 0; i < n; i++) {
                rb_ary_push(ary, ULONG2NUM(szp[i]));
            }
            return ary;
        }
        // cl_platform_id
        case CL_DEVICE_PLATFORM:
            return RPlatform(*(cl_platform_id *)param_value);
        default:
            ret = Qnil;
    }
    return ret;
}

/*
 * call-seq:
 *      aDevice.eql?(device)	-> true or false.
 */
static VALUE
rcl_device_eql(VALUE self, VALUE dev)
{
    cl_device_id myid = DevicePtr(self);
    cl_device_id yourid = DevicePtr(dev);

    return myid == yourid ? Qtrue : Qfalse;
}

/*
 * call-seq:
 *      aDevice.hash
 *
 * The hash number for the Device ojects.
 */
static VALUE
rcl_device_hash(VALUE self)
{
    return LONG2FIX((intptr_t)DevicePtr(self));
}

void
rcl_define_class_device(VALUE mod)
{
    rcl_cDevice = rb_define_class_under(mod, "Device", rb_cObject);
    rb_define_module_function(mod, "devices", rcl_devices, 2);
    rb_undef_alloc_func(rcl_cDevice);
    rb_define_method(rcl_cDevice, "info", rcl_device_info, 1);
    rb_define_method(rcl_cDevice, "eql?", rcl_device_eql, 1);
    rb_define_method(rcl_cDevice, "hash", rcl_device_hash, 0);
}
