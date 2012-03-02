
#include "capi.h"

static void
rcl_sampler_free(void *ptr)
{
    rcl_sampler_t *p = (rcl_sampler_t *)ptr;
    if (p->s != NULL) {
        clReleaseSampler(p->s);
    }
    xfree(p);
}

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
    cl_context cxt = ContextPtr(context);

    EXTRACT_BOOLEAN(normalized_coords, nc);

    EXPECT_RCL_CONST(addressing_mode);
    EXPECT_RCL_CONST(filter_mode);
    cl_addressing_mode am = FIX2UINT(addressing_mode);
    cl_filter_mode fm = FIX2UINT(filter_mode);

    cl_int res;
    cl_sampler s = clCreateSampler(cxt, nc, am, fm, &res);
    CHECK_AND_RAISE(res);

    rcl_sampler_t *p;
    Data_Get_Struct(self, rcl_sampler_t, p);
    p->s = s;

    return self;
}

static VALUE
rcl_sampler_init_copy(VALUE copy, VALUE orig)
{
    if (copy == orig) return copy;

    EXPECT_RCL_TYPE(orig, Sampler);

    rcl_sampler_t *copy_p;
    Data_Get_Struct(copy, rcl_sampler_t, copy_p);
    rcl_sampler_t *orig_p;
    Data_Get_Struct(orig, rcl_sampler_t, orig_p);

    if (copy_p->s == orig_p->s) return copy;

    cl_int res;
    if (copy_p->s != NULL) {
        res = clReleaseSampler(copy_p->s);
        CHECK_AND_RAISE(res);
    }

    res = clRetainSampler(orig_p->s);
    CHECK_AND_RAISE(res);

    copy_p->s = orig_p->s;

    return copy;
}

static VALUE
rcl_sampler_info(VALUE self, VALUE sampler_info)
{
    EXPECT_RCL_CONST(sampler_info);
    cl_sampler_info si = FIX2UINT(sampler_info);

    cl_sampler s = SamplerPtr(self);

    cl_int res;
    intptr_t param_value = 0;   // CHECK: again, extensibility.
                                // The best way is to get size first.

    res = clGetSamplerInfo(s, si, sizeof(intptr_t), &param_value, NULL);
    CHECK_AND_RAISE(res);

    switch (si) {
    case CL_SAMPLER_REFERENCE_COUNT:
    case CL_SAMPLER_ADDRESSING_MODE:
    case CL_SAMPLER_FILTER_MODE:
        return LONG2FIX(param_value);
    case CL_SAMPLER_CONTEXT:
        return RContext((cl_context)param_value);
    case CL_SAMPLER_NORMALIZED_COORDS:
        return param_value ? Qtrue : Qfalse;
    default:
        break;
    }
    return Qnil;
}

void
rcl_define_class_sampler(VALUE mod)
{
    rcl_cSampler = rb_define_class_under(mod, "Sampler", rb_cObject);
    rb_define_alloc_func(rcl_cSampler, rcl_sampler_alloc);
    rb_define_method(rcl_cSampler, "initialize", rcl_sampler_init, 4);
    rb_define_method(rcl_cSampler, "initialize_copy", rcl_sampler_init_copy, 1);
    rb_define_method(rcl_cSampler, "info", rcl_sampler_info, 1);
}
