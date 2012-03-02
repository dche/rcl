
#include "capi.h"

static VALUE
rcl_context_alloc(VALUE klass)
{
    return RContext(NULL);
}

void
rcl_context_free(void *ptr)
{
    rcl_context_t *p = (rcl_context_t *)ptr;
    if (p->c != NULL) clReleaseContext(p->c);

    xfree(p);
}

static void
rcl_pfn_notify(const char *errinfo, const void *private_info, size_t cb, void *user_data)
{
    TRACE("%s", errinfo);
}

static void
set_context_properties(cl_context_properties *props, VALUE arr, size_t len)
{
    for (cl_uint i = 0; i < len; i += 2) {
        VALUE pn = rb_ary_entry(arr, i);
        VALUE ptr = rb_ary_entry(arr, i + 1);

        assert(!(NIL_P(pn) || NIL_P(ptr)));

        props[i] = FIX2UINT(pn);
        switch (props[i]) {
            case CL_CONTEXT_PLATFORM:
                props[i+1] = (cl_context_properties)DATA_PTR(ptr);
                break;
            default:
                rb_raise(rb_eArgError, "invalid context property.");
        }
    }
}

/*
 * call-seq:
 *      Context.new([CL_CONTEXT_PLATFORM, platform], CL_DEVICE_TYPE_GPU)
 *      Context.new(nil, devices(CL_DEVICE_TYPE_GPU, platform))
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
            rb_raise(rb_eArgError, "invalid context properties list.");
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
        rb_raise(rb_eArgError, "invalid argument. Expected device type or device array.");
    }

    cl_int res;
    cl_context context;

    if (!NIL_P(devs)) {
        cl_uint num_dev = (cl_uint)RARRAY_LEN(devs);
        cl_device_id *dev_ids;
        EXTRACT_CL_POINTERS(devs, Device, cl_device_id, dev_ids);

        context = clCreateContext(props, num_dev, dev_ids, rcl_pfn_notify, NULL, &res);
    } else {
        context = clCreateContextFromType(props, dev_type, rcl_pfn_notify, NULL, &res);
    }
    CHECK_AND_RAISE(res);

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
    EXPECT_RCL_TYPE(orig, Context);

    rcl_context_t *copy_p;
    rcl_context_t *orig_p;

    Data_Get_Struct(copy, rcl_context_t, copy_p);
    Data_Get_Struct(orig, rcl_context_t, orig_p);

    if (copy_p->c == orig_p->c) return copy;

    cl_int res;
    if (copy_p->c != NULL) {
        res = clReleaseContext(copy_p->c);
        CHECK_AND_RAISE(res);
    }
    res = clRetainContext(orig_p->c);
    CHECK_AND_RAISE(res);

    copy_p->c = orig_p->c;

    return copy;
}

/*
 * call-seq:
 *      aContext.info(CL_CONTEXT_REFERENCE_COUNT)	-> Fixnum
 *
 * Wrapps +clGetContextInfo()+.
 */

static VALUE
rcl_context_info(VALUE self, VALUE context_info)
{
    EXPECT_RCL_CONST(context_info);
    cl_context_info iname = FIX2UINT(context_info);
    cl_context cxt = ContextPtr(self);

    char info[512];
    size_t info_size;

    cl_int res = clGetContextInfo(cxt, iname, 512, (void *)info, &info_size);
    CHECK_AND_RAISE(res);

    switch (iname) {
        case CL_CONTEXT_REFERENCE_COUNT:
#ifdef CL_VERSION_1_1
        case CL_CONTEXT_NUM_DEVICES:
#endif
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

#ifdef CL_VERSION_1_1

static VALUE
rcl_context_create_user_event(VALUE self)
{
    cl_context cxt = ContextPtr(self);
    cl_int res = CL_SUCCESS;

    cl_event evt = clCreateUserEvent(cxt, &res);
    CHECK_AND_RAISE(res);

    return REvent(evt);
}

#endif

static VALUE
rcl_context_supported_image_formats(VALUE self, VALUE mem_flag, VALUE mem_obj_type)
{
    EXPECT_RCL_CONST(mem_obj_type);
    cl_mem_object_type mt = FIX2INT(mem_obj_type);
    EXPECT_RCL_CONST(mem_flag);
    cl_mem_flags mf = FIX2UINT(mem_flag);

    cl_context cxt = ContextPtr(self);
    cl_uint num_ret = 0;
    cl_int res = clGetSupportedImageFormats(cxt, mf, mt, 0, NULL, &num_ret);
    CHECK_AND_RAISE(res);

    cl_image_format *ifs = ALLOCA_N(cl_image_format, num_ret);
    if (NULL == ifs) {
        rb_raise(rb_eRuntimeError, "out of memory.");
    }
    res = clGetSupportedImageFormats(cxt, mf, mt, num_ret, ifs, NULL);
    CHECK_AND_RAISE(res);

    VALUE ret = rb_ary_new2(num_ret);
    for (cl_uint i = 0; i < num_ret; i++) {
        rb_ary_push(ret, RImageFormat(ifs + i));
    }
    return ret;
}

void
rcl_define_class_context(VALUE mod)
{
    rcl_cContext = rb_define_class_under(mod, "Context", rb_cObject);
    rb_define_alloc_func(rcl_cContext, rcl_context_alloc);
    rb_define_method(rcl_cContext, "initialize", rcl_context_init, 2);
    rb_define_method(rcl_cContext, "initialize_copy", rcl_context_init_copy, 1);
    rb_define_method(rcl_cContext, "info", rcl_context_info, 1);
    rb_define_method(rcl_cContext, "supported_image_formats", rcl_context_supported_image_formats, 2);
#ifdef CL_VERSION_1_1
    rb_define_method(rcl_cContext, "create_user_event", rcl_context_create_user_event, 0);
#endif
}
