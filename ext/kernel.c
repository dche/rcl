
#include "capi.h"

static VALUE
rcl_kernel_alloc(VALUE klass)
{
    return RKernel(NULL);
}

void
rcl_kernel_free(void *ptr)
{
    rcl_kernel_t *p = (rcl_kernel_t *)ptr;
    if (p->k != NULL) {
        clReleaseKernel(p->k);
    }
    xfree(ptr);
}

/*
 * call-seq:
 *      Kernel::new(aProgram, name)     -> a Kernel object.
 *
 * Wrapps +clCreateKernel()+.
 */
static VALUE
rcl_kernel_init(VALUE self, VALUE program, VALUE name)
{
    EXPECT_RCL_TYPE(program, Program);
    Check_Type(name, T_STRING);

    cl_program prog = ProgramPtr(program);
    const char *str = RSTRING_PTR(name);

    cl_int res;
    cl_kernel k = clCreateKernel(prog, str, &res);
    CHECK_AND_RAISE(res);

    rcl_kernel_t *pk;
    Data_Get_Struct(self, rcl_kernel_t, pk);

    pk->k = k;
    return self;
}

static VALUE
rcl_kernel_init_copy(VALUE copy, VALUE orig)
{
    if (copy == orig) return copy;
    EXPECT_RCL_TYPE(orig, Kernel);

    rcl_kernel_t *copy_p, *orig_p;
    Data_Get_Struct(copy, rcl_kernel_t, copy_p);
    Data_Get_Struct(orig, rcl_kernel_t, orig_p);

    if (copy_p->k == orig_p->k) return copy;

    cl_int res;
    if (copy_p->k != NULL) {
        res = clReleaseKernel(copy_p->k);
        CHECK_AND_RAISE(res);
    }
    res = clRetainKernel(orig_p->k);
    CHECK_AND_RAISE(res);

    copy_p->k = orig_p->k;
    return copy;
}


extern size_t rcl_type_size(VALUE);
extern void rcl_ruby2native(VALUE, void *, VALUE);

static VALUE   rcl_kernel_arg_type_memory;
static VALUE   rcl_kernel_arg_type_struct;
static VALUE   rcl_kernel_arg_type_local;
static VALUE   rcl_kernel_arg_type_sampler;

/*
 * call-seq:
 *      aKernel#set_arg(index, type, value)
 *
 * Wrapps +clSetKernelArg()+.
 *
 * Returns the receiver when success, or raise CLError.
 */

static VALUE
rcl_kernel_set_arg(VALUE self, VALUE index, VALUE type, VALUE value)
{
    EXTRACT_SIZE(index, idx);
    Check_Type(type, T_SYMBOL);

    size_t arg_size = 0;
    int8_t *arg_ptr = NULL;

    if (type == rcl_kernel_arg_type_memory) {
        if (!NIL_P(value)) {
            EXPECT_RCL_TYPE(value, Memory);
            arg_size = sizeof(cl_mem);

            rcl_mem_t *mem;
            Data_Get_Struct(value, rcl_mem_t, mem);
            arg_ptr = (void *)(&(mem->mem));
        }
    } else if (type == rcl_kernel_arg_type_local) {
        EXTRACT_SIZE(value, sz);
        arg_size = sz;
        arg_ptr = NULL;
    } else if (type == rcl_kernel_arg_type_struct) {
        // TODO: rb_call pointer
        //       rb_call address, size
        rb_notimplement();
    } else if (type == rcl_kernel_arg_type_sampler) {
        EXPECT_RCL_TYPE(value, Sampler);

        rcl_sampler_t *ps;
        Data_Get_Struct(value, rcl_sampler_t, ps);
        arg_ptr = (void *)(&(ps->s));

        arg_size = sizeof(cl_sampler);
    } else {
        arg_size = rcl_type_size(type);
        arg_ptr = ALLOCA_N(int8_t, arg_size);

        rcl_ruby2native(type, (void *)arg_ptr, value);
    }

    cl_int res = clSetKernelArg(KernelPtr(self), (cl_uint)idx, arg_size, arg_ptr);
    CHECK_AND_RAISE(res);

    return self;
}

/*
 * call-seq:
 *      Kernel#info(CL_KERNEL_FUNCTION_NAME)
 *
 * Wrapps +clGetKernelInfo()+.
 */
static VALUE
rcl_kernel_info(VALUE self, VALUE param_name)
{
    EXPECT_RCL_CONST(param_name);

    cl_kernel k = KernelPtr(self);
    cl_kernel_info ki = FIX2UINT(param_name);

    char param_value[128];
    size_t sz_ret;
    cl_int res = clGetKernelInfo(k, ki, 128, param_value, &sz_ret);
    CHECK_AND_RAISE(res);

    switch (ki) {
    case CL_KERNEL_FUNCTION_NAME:
        return rb_str_new2(param_value);
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
 *
 * deivce - A Device object. Can be +nil+ if there is only single device
 *          is used by the context to which the receiver belongs.
 * param_name - The information to query.
 */
static VALUE
rcl_kernel_workgroup_info(VALUE self, VALUE device, VALUE param_name)
{
    EXPECT_RCL_TYPE(device, Device);
    EXPECT_RCL_CONST(param_name);

    cl_kernel k = KernelPtr(self);
    cl_device_id dev = NIL_P(device) ? NULL : DevicePtr(device);
    cl_kernel_work_group_info kwi = FIX2UINT(param_name);

    size_t param_value[3];

    cl_uint res = clGetKernelWorkGroupInfo(k, dev, kwi, sizeof(size_t) * 3, param_value, NULL);
    CHECK_AND_RAISE(res);

    VALUE ret = Qnil;
    switch (kwi) {
    case CL_KERNEL_WORK_GROUP_SIZE:
    case CL_KERNEL_LOCAL_MEM_SIZE:
#ifdef CL_VERSION_1_1
    case CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE:
#endif
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

void
rcl_define_class_kernel(VALUE mod)
{
    rcl_cKernel = rb_define_class_under(mod, "Kernel", rb_cObject);
    rb_define_alloc_func(rcl_cKernel, rcl_kernel_alloc);
    rb_define_method(rcl_cKernel, "initialize", rcl_kernel_init, 2);
    rb_define_method(rcl_cKernel, "initialize_copy", rcl_kernel_init_copy, 1);
    rb_define_method(rcl_cKernel, "set_arg", rcl_kernel_set_arg, 3);
    rb_define_method(rcl_cKernel, "info", rcl_kernel_info, 1);
    rb_define_method(rcl_cKernel, "workgroup_info", rcl_kernel_workgroup_info, 2);

    // Initialize constants.
    rcl_kernel_arg_type_memory = ID2SYM(rb_intern("mem"));
    rcl_kernel_arg_type_struct = ID2SYM(rb_intern("struct"));
    rcl_kernel_arg_type_local = ID2SYM(rb_intern("local"));
    rcl_kernel_arg_type_sampler = ID2SYM(rb_intern("sampler"));
}
