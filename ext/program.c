
#include "capi.h"

static VALUE
rcl_program_alloc(VALUE klass)
{
    return RProgram(NULL);
}

void
rcl_program_free(void *ptr)
{
    rcl_program_t *p = (rcl_program_t *)ptr;
    if (p->p != NULL) {
        clReleaseProgram(p->p);
    }
    xfree(ptr);
}

static cl_program
rcl_program_create_from_source(cl_context context, VALUE sources)
{
    EXPECT_ARRAY(sources);
    cl_uint num_src = (cl_uint)RARRAY_LEN(sources);

    const char **srcp = (const char **)ALLOCA_N(intptr_t, num_src);
    size_t *lenp = ALLOCA_N(size_t, num_src);

    for (cl_uint i = 0; i < num_src; i++) {
        VALUE srcstr = rb_ary_entry(sources, i);
        if (TYPE(srcstr) != T_STRING) {
            rb_raise(rb_eTypeError, "expected source is a String.");
        }
        srcp[i] = RSTRING_PTR(srcstr);
        lenp[i] = RSTRING_LEN(srcstr);
    }

    cl_int res;
    cl_program prog = clCreateProgramWithSource(context, num_src, srcp, lenp, &res);
    CHECK_AND_RAISE(res);

    return prog;
}

static cl_program
rcl_program_create_from_binary(cl_context context, VALUE devices, VALUE binaries)
{
    EXPECT_ARRAY(devices);
    EXPECT_ARRAY(binaries);

    cl_uint num_dev = (cl_uint)RARRAY_LEN(devices);
    if (RARRAY_LEN(binaries) != num_dev) {
        rb_raise(rb_eArgError, "number of binaries shall equal to number of devices.");
    }

    cl_device_id *devs = ALLOCA_N(cl_device_id, num_dev);
    size_t *len_ar = ALLOCA_N(size_t, num_dev);
    const unsigned char **bin_ar = (const unsigned char **)ALLOCA_N(intptr_t, num_dev);

    for (cl_uint i = 0; i < num_dev; i++) {
        VALUE dev = rb_ary_entry(devices, i);
        EXPECT_RCL_TYPE(dev, Device);

        VALUE bin = rb_ary_entry(binaries, i);
        if (TYPE(bin) != T_STRING) {
            rb_raise(rb_eTypeError, "invalid binary. Expected a byte string.");
        }
        devs[i] = DevicePtr(dev);
        len_ar[i] = RSTRING_LEN(bin);
        bin_ar[i] = (unsigned char *)RSTRING_PTR(bin);
    }

    cl_int res;
    cl_int *bin_status = ALLOCA_N(cl_int, num_dev);
    cl_program prog = clCreateProgramWithBinary(context, num_dev, devs, len_ar, bin_ar, bin_status, &res);
    CHECK_AND_RAISE(res);

    // FIXME: we got binary status, but do not use these information yet.

    return prog;
}

/*
 * call-seq:
 *      Program.new(Context, [String, ..])
 *      Program.new(Context, [Device, ..], [String, ..])
 */
static VALUE
rcl_program_init(int argc, VALUE *argv, VALUE self)
{
    if (argc != 2 && argc != 3) {
        rb_raise(rb_eArgError, "wrong number of arguments.");
    }

    EXPECT_RCL_TYPE(argv[0], Context);
    cl_context context = ContextPtr(argv[0]);

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

    EXPECT_RCL_TYPE(orig, Program);
    rcl_program_t *copy_p, *orig_p;

    Data_Get_Struct(copy, rcl_program_t, copy_p);
    Data_Get_Struct(orig, rcl_program_t, orig_p);

    if (copy_p->p == orig_p->p) return copy;

    cl_int res;
    if (copy_p->p != NULL) {
        res = clReleaseProgram(copy_p->p);
        CHECK_AND_RAISE(res);
    }
    res = clRetainProgram(orig_p->p);
    CHECK_AND_RAISE(res);

    copy_p->p = orig_p->p;

    return copy;
}

typedef void (*pfn_build_notify)(cl_program, void *user_data);

static void
build_notify(cl_program program, void *user_data)
{
    // NOT IMPLEMENTED. Seems that the context notifier will report
    // build erros too.
}

/*
 * call-seq:
 *      Program#build([Device, ..], "-D")   -> the receiver
 *
 * Wrapps +clProgramBuild()+.
 */
static VALUE
rcl_program_build(VALUE self, VALUE devices, VALUE options, VALUE memo)
{
    if (TYPE(options) != T_STRING) {
        rb_raise(rb_eTypeError, "expected options is a String.");
    }
    EXPECT_ARRAY(devices);

    cl_uint num_dev = (cl_uint)RARRAY_LEN(devices);
    cl_device_id *devs = NULL;
    EXTRACT_CL_POINTERS(devices, Device, cl_device_id, devs);

    cl_program prog = ProgramPtr(self);
    pfn_build_notify pbn = NIL_P(memo) ? NULL : build_notify;
    void *mm = NIL_P(memo) ? NULL : (void *)memo;
    cl_int res = clBuildProgram(prog, num_dev, devs, RSTRING_PTR(options), pbn, mm);
    CHECK_AND_RAISE(res);

    return self;
}

/*
 * call-seq:
 *      Program#build_info(Device, CL_PROGRAM_BUILD_LOG)
 *
 * Wrapps +clGetProgramBuildInfo()+.
 */
static VALUE
rcl_program_build_info(VALUE self, VALUE device, VALUE param_name)
{
    EXPECT_RCL_TYPE(device, Device);
    EXPECT_RCL_CONST(param_name);

    cl_program prog = ProgramPtr(self);
    cl_device_id dev = DevicePtr(device);
    cl_program_build_info bi = FIX2UINT(param_name);

    size_t sz_ret = 0;
    cl_int res = clGetProgramBuildInfo(prog, dev, bi, 0, NULL, &sz_ret);
    CHECK_AND_RAISE(res);

    char *param_value = ALLOCA_N(char, sz_ret);
    res = clGetProgramBuildInfo(prog, dev, bi, sz_ret, param_value, NULL);
    CHECK_AND_RAISE(res);

    switch (bi) {
    case CL_PROGRAM_BUILD_STATUS:
        return UINT2NUM(*(cl_build_status *)param_value);
    case CL_PROGRAM_BUILD_OPTIONS:
    case CL_PROGRAM_BUILD_LOG:
        return rb_str_new2(param_value);
    default:
        break;
    }

    return Qnil;
}

/*
 * call-seq:
 *      Program#info(CL_PROGRAM_SOURCE)
 *
 * Wrapps +clGetProgramInfo()+.
 */
static VALUE
rcl_program_info(VALUE self, VALUE param_name)
{
    EXPECT_RCL_CONST(param_name);

    cl_program prog = ProgramPtr(self);
    cl_program_info pi = FIX2UINT(param_name);

    size_t sz_ret;
    cl_int res = clGetProgramInfo(prog, pi, 0, NULL, &sz_ret);
    CHECK_AND_RAISE(res);

    char *param_value = ALLOCA_N(char, sz_ret);
    res = clGetProgramInfo(prog, pi, sz_ret, param_value, NULL);
    CHECK_AND_RAISE(res);

    switch (pi) {
    case CL_PROGRAM_REFERENCE_COUNT:
    case CL_PROGRAM_NUM_DEVICES:
        return UINT2NUM(*(cl_uint *)param_value);
    case CL_PROGRAM_CONTEXT:
        return RContext(*(cl_context *)param_value);
    case CL_PROGRAM_DEVICES:
        return build_device_array((cl_device_id *)param_value, sz_ret);
    case CL_PROGRAM_BINARY_SIZES: {
        size_t num_szs = sz_ret / sizeof(size_t);
        VALUE szs = rb_ary_new2(num_szs);
        for (cl_uint i = 0; i < num_szs; i++) {
            rb_ary_push(szs, UINT2NUM(((size_t *)(param_value))[i]));
        }
        return szs;
    }
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
 *      Program#create_kernels  -> Array of Kernel objects
 *
 * Wrapps +clCreateKernelsInProgram()+.
 */
static VALUE
rcl_program_create_kernels(VALUE self)
{
    cl_kernel kernels[128]; // TOOD: literal constant?
    cl_program prog = ProgramPtr(self);

    cl_uint num_ret;
    cl_int res = clCreateKernelsInProgram(prog, 128, kernels, &num_ret);
    CHECK_AND_RAISE(res);

    VALUE ary = rb_ary_new2(num_ret);
    for (cl_uint i = 0; i < num_ret; i++) {
        rb_ary_push(ary, RKernel(kernels[i]));
    }
    return ary;
}

/*
 * call-seq:
 *      Capi.unload_compiler    -> the receiver
 *
 * Wrapps +clUnloadCompiler()+.
 */

static VALUE
rcl_unload_compiler(VALUE self)
{
    cl_int res = clUnloadCompiler();
    CHECK_AND_RAISE(res);

    return self;
}

void
rcl_define_class_program(VALUE mod)
{
    rcl_cProgram = rb_define_class_under(mod, "Program", rb_cObject);
    rb_define_alloc_func(rcl_cProgram, rcl_program_alloc);
    rb_define_method(rcl_cProgram, "initialize", rcl_program_init, -1);
    rb_define_method(rcl_cProgram, "initialize_copy", rcl_program_init_copy, 1);
    rb_define_method(rcl_cProgram, "build", rcl_program_build, 3);
    rb_define_method(rcl_cProgram, "build_info", rcl_program_build_info, 2);
    rb_define_method(rcl_cProgram, "info", rcl_program_info, 1);
    rb_define_method(rcl_cProgram, "create_kernels", rcl_program_create_kernels, 0);
    rb_define_module_function(mod, "unload_compiler", rcl_unload_compiler, 0);
}
