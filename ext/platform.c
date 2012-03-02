
#include "capi.h"

/*
 * call-seq:
 *      OpenCL.platforms  -> Array of Platform objects.
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
        for(cl_uint i = 0; i < num_p; i++) {
            VALUE o = RPlatform(p_ids[i]);
            rb_ary_push(list, o);
        }
    } else {
        CHECK_AND_WARN(res);
    }
    return list;
}

/*
 * call-seq:
 *		aPlatform.info(CL_PLATFORM_VENDOR)
 *
 * Wrapps the +clGetPlatformInfo()+.
 */

static VALUE
rcl_platform_info(VALUE self, VALUE platform_info)
{
    EXPECT_RCL_CONST(platform_info);
    cl_platform_info info = FIX2UINT(platform_info);

    cl_int res;
    void *param_value;
    size_t param_value_size;

    cl_platform_id platform = PlatformPtr(self);
    res = clGetPlatformInfo(platform, info, 0, NULL, &param_value_size);
    CHECK_AND_RAISE(res);

    param_value = ALLOCA_N(int8_t, param_value_size);
    res = clGetPlatformInfo(platform, info, param_value_size, param_value, NULL);
    CHECK_AND_RAISE(res);

    return rb_str_new2(param_value);
}

void
rcl_define_class_platform(VALUE mod)
{
    rcl_cPlatform = rb_define_class_under(mod, "Platform", rb_cObject);
    rb_define_module_function(mod, "platforms", rcl_platforms, 0);
    rb_undef_alloc_func(rcl_cPlatform);
    rb_define_method(rcl_cPlatform, "info", rcl_platform_info, 1);
}
