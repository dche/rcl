
#include "capi.h"

void
rcl_mem_free(void *ptr)
{
    rcl_mem_t *p = (rcl_mem_t *)ptr;
    if (p->mem != NULL) {
        clReleaseMemObject(p->mem);
    }
    xfree(p);
}

/*
 * call-seq:
 *      Memory.create_buffer(aContext, CL_MEM_FLAG_READ_WRITE, 0, aPointer)
 */
static VALUE
rcl_mem_create_buffer(VALUE mod, VALUE context, VALUE flags, VALUE size, VALUE host_ptr)
{
    EXPECT_RCL_TYPE(context, Context);
    EXPECT_FIXNUM(flags);
    if (!NIL_P(host_ptr)) EXPECT_RCL_TYPE(host_ptr, Pointer);

    cl_context cxt = ContextPtr(context);
    cl_mem_flags mf = FIX2INT(flags);

    EXTRACT_SIZE(size, sz);
    EXTRACT_POINTER(host_ptr, hp);

    if (NULL != hp && sz == 0) {
        sz = FIX2UINT(rb_funcall(host_ptr, rb_intern("byte_size"), 0));
    }

    cl_int res;
    cl_mem mem = clCreateBuffer(cxt, mf, sz, hp, &res);
    CHECK_AND_RAISE(res);

    return RMemory(mem);
}

#ifdef CL_VERSION_1_1

/*
 * call-seq:
 *      Memory.create_subbuffer(buffer, , [10, 100])
 *
 */
static VALUE
rcl_mem_create_subbuffer(VALUE mod, VALUE buffer,
                                     VALUE flags,
                                     VALUE region)
{
    EXPECT_RCL_TYPE(buffer, Memory);
    EXPECT_FIXNUM(flags);
    EXPECT_ARRAY(region);

    long i = RARRAY_LEN(region);
    if (i != 2) {
        rb_raise(rb_eArgError, "Expected the parameter region has 2 items, got (%ld).", i);
    }
    VALUE sz = rb_ary_entry(region, 0);
    EXTRACT_SIZE(sz, origin);
    sz = rb_ary_entry(region, 1);
    EXTRACT_SIZE(sz, offset);

    cl_buffer_region br;
    br.origin = origin;
    br.size = offset;

    cl_mem buf = MemoryPtr(buffer);
    cl_mem_flags mf = FIX2INT(flags);

    cl_int res = CL_SUCCESS;
    cl_mem subbuf = clCreateSubBuffer(buf, mf, CL_BUFFER_CREATE_TYPE_REGION, &br, &res);
    CHECK_AND_RAISE(res);

    return RMemory(subbuf);
}

// NOTE: clSetMemObjectDestructorCallback is not supported.
//       Please use Ruby's finalizer instead.

#endif

static VALUE
rcl_mem_create_image_2d(VALUE mod, VALUE context,
                                   VALUE flags,
                                   VALUE image_format,
                                   VALUE width, VALUE height, VALUE row_pitch,
                                   VALUE host_ptr)
{
    EXPECT_RCL_TYPE(context, Context);
    EXPECT_FIXNUM(flags);

    EXTRACT_SIZE(width, w);
    EXTRACT_SIZE(height, h);
    EXTRACT_SIZE(row_pitch, rp);

    cl_context cxt = ContextPtr(context);
    cl_mem_flags mf = FIX2INT(flags);
    EXTRACT_IMAGE_FORMAT(image_format, imgfmt);

    EXTRACT_POINTER(host_ptr, hp);

    cl_int res;
    cl_mem img = clCreateImage2D(cxt, mf, &imgfmt, w, h, rp, hp, &res);
    CHECK_AND_RAISE(res);

    return RMemory(img);
}

static VALUE
rcl_mem_create_image_3d(VALUE mod, VALUE context, VALUE flags,
                                   VALUE image_format,
                                   VALUE width, VALUE height, VALUE depth,
                                   VALUE row_pitch, VALUE slice_pitch,
                                   VALUE host_ptr)
{
    EXPECT_RCL_TYPE(context, Context);
    EXPECT_FIXNUM(flags);
    if (CLASS_OF(image_format) != rcl_cImageFormat) {
        rb_raise(rb_eTypeError, "expected argument 3 is a ImageFormat.");
    }
    cl_mem_flags mf = FIX2INT(flags);

    EXTRACT_SIZE(width, w);
    EXTRACT_SIZE(height, h);
    EXTRACT_SIZE(depth, d);
    EXTRACT_SIZE(row_pitch, rp);
    EXTRACT_SIZE(slice_pitch, sp);

    cl_context cxt = ContextPtr(context);
    EXTRACT_IMAGE_FORMAT(image_format, imgfmt);
    EXTRACT_POINTER(host_ptr, hp);

    cl_int res;
    cl_mem img = clCreateImage3D(cxt, mf, &imgfmt, w, h, d, rp, sp, hp, &res);
    CHECK_AND_RAISE(res);

    return RMemory(img);
}

static VALUE
rcl_mem_info(VALUE self, VALUE param_name)
{
    EXPECT_RCL_CONST(param_name);

    cl_mem_info mi = FIX2UINT(param_name);
    cl_mem m = MemoryPtr(self);
    intptr_t param_value;

    cl_int res = clGetMemObjectInfo(m, mi, sizeof(intptr_t), &param_value, NULL);
    CHECK_AND_RAISE(res);

    switch (mi) {
    case CL_MEM_TYPE:
    case CL_MEM_FLAGS:
    case CL_MEM_SIZE:
    case CL_MEM_MAP_COUNT:
    case CL_MEM_REFERENCE_COUNT:
        return UINT2NUM(param_value);
    case CL_MEM_HOST_PTR:   // CHECK: Should be wrapped to a HostPoiner? MappedPointer? No.
        return ULONG2NUM(param_value);
    case CL_MEM_CONTEXT:
        return RContext((cl_context)param_value);
    default:
        break;
    }

    return Qnil;
}

static VALUE
rcl_mem_image_info(VALUE self, VALUE param_name)
{
    EXPECT_RCL_CONST(param_name);
    cl_image_info ii = FIX2UINT(param_name);
    cl_mem m = MemoryPtr(self);

    cl_image_format imgfmt;

    cl_int res = clGetImageInfo(m, ii, sizeof(cl_image_format), (void *)&imgfmt, NULL);
    CHECK_AND_RAISE(res);

    switch (ii) {
    case CL_IMAGE_FORMAT:
        return RImageFormat(&imgfmt);
    case CL_IMAGE_ELEMENT_SIZE:
    case CL_IMAGE_ROW_PITCH:
    case CL_IMAGE_SLICE_PITCH:
    case CL_IMAGE_WIDTH:
    case CL_IMAGE_HEIGHT:
    case CL_IMAGE_DEPTH:
        return ULONG2NUM(*(size_t *)&imgfmt);
    }
    return Qnil;
}

static VALUE
rcl_mem_create_from_gl_buffer(VALUE self, VALUE context,
                              VALUE flags, VALUE bufobj)
{
    EXPECT_RCL_TYPE(context, Context);
    EXPECT_FIXNUM(flags);
    EXPECT_FIXNUM(bufobj);

    cl_context cxt = ContextPtr(context);
    cl_mem_flags mf = FIX2INT(flags);
    cl_GLuint glbuf = FIX2UINT(bufobj);

    cl_int res;
    cl_mem mem = clCreateFromGLBuffer(cxt, mf, glbuf, &res);
    CHECK_AND_RAISE(res);

    return RMemory(mem);
}

static VALUE
rcl_mem_create_from_gl_render_buffer(VALUE self)
{
    rb_notimplement();
    return Qnil;
}

static VALUE
rcl_mem_create_from_gl_texture_2d(VALUE self)
{
    rb_notimplement();
    return Qnil;
}

static VALUE
rcl_mem_create_from_gl_texture_3d(VALUE self)
{
    rb_notimplement();
    return Qnil;
}

static VALUE
rcl_mem_gl_obj_info(VALUE self)
{
    rb_notimplement();
    return Qnil;
}

static VALUE
rcl_mem_gl_texture_info(VALUE self)
{
    rb_notimplement();
    return Qnil;
}

void
rcl_define_class_memory(VALUE mod)
{
    rcl_cMemory = rb_define_class_under(mod, "Memory", rb_cObject);
    rb_undef_alloc_func(rcl_cMemory);
    rb_define_singleton_method(rcl_cMemory, "create_buffer", rcl_mem_create_buffer, 4);
#ifdef CL_VERSION_1_1
    rb_define_singleton_method(rcl_cMemory, "create_subbuffer", rcl_mem_create_subbuffer, 3);
#endif
    rb_define_singleton_method(rcl_cMemory, "create_image_2d", rcl_mem_create_image_2d, 7);
    rb_define_singleton_method(rcl_cMemory, "create_image_3d", rcl_mem_create_image_3d, 9);
    rb_define_method(rcl_cMemory, "info", rcl_mem_info, 1);
    rb_define_method(rcl_cMemory, "image_info", rcl_mem_image_info, 1);

    // GL sharing API.
    rb_define_singleton_method(rcl_cMemory, "create_from_gl_buffer", rcl_mem_create_from_gl_buffer, 0);
    rb_define_singleton_method(rcl_cMemory, "create_from_gl_render_buffer", rcl_mem_create_from_gl_render_buffer, 0);
    rb_define_singleton_method(rcl_cMemory, "create_from_gl_texture_2d", rcl_mem_create_from_gl_texture_2d, 0);
    rb_define_singleton_method(rcl_cMemory, "create_from_gl_texture_3d", rcl_mem_create_from_gl_texture_3d, 0);
    rb_define_method(rcl_cMemory, "gl_object_info", rcl_mem_gl_obj_info, 0);
    rb_define_method(rcl_cMemory, "gl_texture_info", rcl_mem_gl_texture_info, 0);
}
