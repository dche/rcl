
#include "capi.h"

static VALUE
rcl_image_format_init(VALUE self, VALUE channel_order, VALUE channel_data_type)
{
    EXPECT_RCL_CONST(channel_order);
    EXPECT_RCL_CONST(channel_data_type);

    VALUE fmt = rb_ary_new2(2);
    rb_ary_push(fmt, channel_order);
    rb_ary_push(fmt, channel_data_type);

    VALUE valid_fmts = rb_cvar_get(rcl_cImageFormat, rb_intern("@@formats"));
    VALUE elm_sz = rb_hash_aref(valid_fmts, fmt);

    if (NIL_P(elm_sz)) {
        rb_raise(rb_eArgError, "invalid channel order or data type.");
    }

    rb_iv_set(self, "@channel_order", channel_order);
    rb_iv_set(self, "@channel_data_type", channel_data_type);
    rb_iv_set(self, "@element_size", elm_sz);

    return self;
}

static VALUE
rcl_define_image_formats(void)
{
    VALUE fmts = rb_hash_new();

#define RCL_DEF_IMAGE_FORMAT(order, type, size) \
    do { \
        VALUE ary = rb_ary_new2(2); \
        rb_ary_push(ary, INT2FIX(order)); \
        rb_ary_push(ary, INT2FIX(type)); \
        rb_hash_aset(fmts, ary, INT2FIX(size)); \
    } while (0)

    RCL_DEF_IMAGE_FORMAT(CL_R, CL_SNORM_INT8,         1 * 1);
    RCL_DEF_IMAGE_FORMAT(CL_R, CL_SNORM_INT16,        1 * 2);
    RCL_DEF_IMAGE_FORMAT(CL_R, CL_UNORM_INT8,         1 * 1);
    RCL_DEF_IMAGE_FORMAT(CL_R, CL_UNORM_INT16,        1 * 2);
    RCL_DEF_IMAGE_FORMAT(CL_R, CL_SIGNED_INT8,        1 * 1);
    RCL_DEF_IMAGE_FORMAT(CL_R, CL_SIGNED_INT16,       1 * 2);
    RCL_DEF_IMAGE_FORMAT(CL_R, CL_SIGNED_INT32,       1 * 4);
    RCL_DEF_IMAGE_FORMAT(CL_R, CL_UNSIGNED_INT8,      1 * 1);
    RCL_DEF_IMAGE_FORMAT(CL_R, CL_UNSIGNED_INT16,     1 * 2);
    RCL_DEF_IMAGE_FORMAT(CL_R, CL_UNSIGNED_INT32,     1 * 4);
    RCL_DEF_IMAGE_FORMAT(CL_R, CL_HALF_FLOAT,         1 * 2);
    RCL_DEF_IMAGE_FORMAT(CL_R, CL_FLOAT,              1 * 4);

    RCL_DEF_IMAGE_FORMAT(CL_A, CL_SNORM_INT8,         1 * 1);
    RCL_DEF_IMAGE_FORMAT(CL_A, CL_SNORM_INT16,        1 * 2);
    RCL_DEF_IMAGE_FORMAT(CL_A, CL_UNORM_INT8,         1 * 1);
    RCL_DEF_IMAGE_FORMAT(CL_A, CL_UNORM_INT16,        1 * 2);
    RCL_DEF_IMAGE_FORMAT(CL_A, CL_SIGNED_INT8,        1 * 1);
    RCL_DEF_IMAGE_FORMAT(CL_A, CL_SIGNED_INT16,       1 * 2);
    RCL_DEF_IMAGE_FORMAT(CL_A, CL_SIGNED_INT32,       1 * 4);
    RCL_DEF_IMAGE_FORMAT(CL_A, CL_UNSIGNED_INT8,      1 * 1);
    RCL_DEF_IMAGE_FORMAT(CL_A, CL_UNSIGNED_INT16,     1 * 2);
    RCL_DEF_IMAGE_FORMAT(CL_A, CL_UNSIGNED_INT32,     1 * 4);
    RCL_DEF_IMAGE_FORMAT(CL_A, CL_HALF_FLOAT,         1 * 2);
    RCL_DEF_IMAGE_FORMAT(CL_A, CL_FLOAT,              1 * 4);

    RCL_DEF_IMAGE_FORMAT(CL_INTENSITY, CL_SNORM_INT8,   1 * 1);
    RCL_DEF_IMAGE_FORMAT(CL_INTENSITY, CL_SNORM_INT16,  1 * 2);
    RCL_DEF_IMAGE_FORMAT(CL_INTENSITY, CL_UNORM_INT8,   1 * 1);
    RCL_DEF_IMAGE_FORMAT(CL_INTENSITY, CL_UNORM_INT16,  1 * 2);
    RCL_DEF_IMAGE_FORMAT(CL_INTENSITY, CL_HALF_FLOAT,   1 * 2);
    RCL_DEF_IMAGE_FORMAT(CL_INTENSITY, CL_FLOAT,        1 * 4);

    RCL_DEF_IMAGE_FORMAT(CL_LUMINANCE, CL_SNORM_INT8,   1 * 1);
    RCL_DEF_IMAGE_FORMAT(CL_LUMINANCE, CL_SNORM_INT16,  1 * 2);
    RCL_DEF_IMAGE_FORMAT(CL_LUMINANCE, CL_UNORM_INT8,   1 * 1);
    RCL_DEF_IMAGE_FORMAT(CL_LUMINANCE, CL_UNORM_INT16,  1 * 2);
    RCL_DEF_IMAGE_FORMAT(CL_LUMINANCE, CL_HALF_FLOAT,   1 * 2);
    RCL_DEF_IMAGE_FORMAT(CL_LUMINANCE, CL_FLOAT,        1 * 4);

    RCL_DEF_IMAGE_FORMAT(CL_RG, CL_SNORM_INT8,         2 * 1);
    RCL_DEF_IMAGE_FORMAT(CL_RG, CL_SNORM_INT16,        2 * 2);
    RCL_DEF_IMAGE_FORMAT(CL_RG, CL_UNORM_INT8,         2 * 1);
    RCL_DEF_IMAGE_FORMAT(CL_RG, CL_UNORM_INT16,        2 * 2);
    RCL_DEF_IMAGE_FORMAT(CL_RG, CL_SIGNED_INT8,        2 * 1);
    RCL_DEF_IMAGE_FORMAT(CL_RG, CL_SIGNED_INT16,       2 * 2);
    RCL_DEF_IMAGE_FORMAT(CL_RG, CL_SIGNED_INT32,       2 * 4);
    RCL_DEF_IMAGE_FORMAT(CL_RG, CL_UNSIGNED_INT8,      2 * 1);
    RCL_DEF_IMAGE_FORMAT(CL_RG, CL_UNSIGNED_INT16,     2 * 2);
    RCL_DEF_IMAGE_FORMAT(CL_RG, CL_UNSIGNED_INT32,     2 * 4);
    RCL_DEF_IMAGE_FORMAT(CL_RG, CL_HALF_FLOAT,         2 * 2);
    RCL_DEF_IMAGE_FORMAT(CL_RG, CL_FLOAT,              2 * 4);

    RCL_DEF_IMAGE_FORMAT(CL_RA, CL_SNORM_INT8,         2 * 1);
    RCL_DEF_IMAGE_FORMAT(CL_RA, CL_SNORM_INT16,        2 * 2);
    RCL_DEF_IMAGE_FORMAT(CL_RA, CL_UNORM_INT8,         2 * 1);
    RCL_DEF_IMAGE_FORMAT(CL_RA, CL_UNORM_INT16,        2 * 2);
    RCL_DEF_IMAGE_FORMAT(CL_RA, CL_SIGNED_INT8,        2 * 1);
    RCL_DEF_IMAGE_FORMAT(CL_RA, CL_SIGNED_INT16,       2 * 2);
    RCL_DEF_IMAGE_FORMAT(CL_RA, CL_SIGNED_INT32,       2 * 4);
    RCL_DEF_IMAGE_FORMAT(CL_RA, CL_UNSIGNED_INT8,      2 * 1);
    RCL_DEF_IMAGE_FORMAT(CL_RA, CL_UNSIGNED_INT16,     2 * 2);
    RCL_DEF_IMAGE_FORMAT(CL_RA, CL_UNSIGNED_INT32,     2 * 4);
    RCL_DEF_IMAGE_FORMAT(CL_RA, CL_HALF_FLOAT,         2 * 2);
    RCL_DEF_IMAGE_FORMAT(CL_RA, CL_FLOAT,              2 * 4);

    RCL_DEF_IMAGE_FORMAT(CL_RGB, CL_UNORM_SHORT_565,    2);
    RCL_DEF_IMAGE_FORMAT(CL_RGB, CL_UNORM_SHORT_555,    2);
    RCL_DEF_IMAGE_FORMAT(CL_RGB, CL_UNORM_INT_101010,   4);

    RCL_DEF_IMAGE_FORMAT(CL_RGBA, CL_SNORM_INT8,         4 * 1);
    RCL_DEF_IMAGE_FORMAT(CL_RGBA, CL_SNORM_INT16,        4 * 2);
    RCL_DEF_IMAGE_FORMAT(CL_RGBA, CL_UNORM_INT8,         4 * 1);
    RCL_DEF_IMAGE_FORMAT(CL_RGBA, CL_UNORM_INT16,        4 * 2);
    RCL_DEF_IMAGE_FORMAT(CL_RGBA, CL_SIGNED_INT8,        4 * 1);
    RCL_DEF_IMAGE_FORMAT(CL_RGBA, CL_SIGNED_INT16,       4 * 2);
    RCL_DEF_IMAGE_FORMAT(CL_RGBA, CL_SIGNED_INT32,       4 * 4);
    RCL_DEF_IMAGE_FORMAT(CL_RGBA, CL_UNSIGNED_INT8,      4 * 1);
    RCL_DEF_IMAGE_FORMAT(CL_RGBA, CL_UNSIGNED_INT16,     4 * 2);
    RCL_DEF_IMAGE_FORMAT(CL_RGBA, CL_UNSIGNED_INT32,     4 * 4);
    RCL_DEF_IMAGE_FORMAT(CL_RGBA, CL_HALF_FLOAT,         4 * 2);
    RCL_DEF_IMAGE_FORMAT(CL_RGBA, CL_FLOAT,              4 * 4);

    RCL_DEF_IMAGE_FORMAT(CL_ARGB, CL_SNORM_INT8,         4 * 1);
    RCL_DEF_IMAGE_FORMAT(CL_ARGB, CL_UNORM_INT8,         4 * 1);
    RCL_DEF_IMAGE_FORMAT(CL_ARGB, CL_SIGNED_INT8,        4 * 1);
    RCL_DEF_IMAGE_FORMAT(CL_ARGB, CL_UNSIGNED_INT8,      4 * 1);

    RCL_DEF_IMAGE_FORMAT(CL_BGRA, CL_SNORM_INT8,         4 * 1);
    RCL_DEF_IMAGE_FORMAT(CL_BGRA, CL_UNORM_INT8,         4 * 1);
    RCL_DEF_IMAGE_FORMAT(CL_BGRA, CL_SIGNED_INT8,        4 * 1);
    RCL_DEF_IMAGE_FORMAT(CL_BGRA, CL_UNSIGNED_INT8,      4 * 1);


#ifdef CL_VERSION_1_1
    RCL_DEF_IMAGE_FORMAT(CL_Rx, CL_SNORM_INT8,         2 * 1);
    RCL_DEF_IMAGE_FORMAT(CL_Rx, CL_SNORM_INT16,        2 * 2);
    RCL_DEF_IMAGE_FORMAT(CL_Rx, CL_UNORM_INT8,         2 * 1);
    RCL_DEF_IMAGE_FORMAT(CL_Rx, CL_UNORM_INT16,        2 * 2);
    RCL_DEF_IMAGE_FORMAT(CL_Rx, CL_SIGNED_INT8,        2 * 1);
    RCL_DEF_IMAGE_FORMAT(CL_Rx, CL_SIGNED_INT16,       2 * 2);
    RCL_DEF_IMAGE_FORMAT(CL_Rx, CL_SIGNED_INT32,       2 * 4);
    RCL_DEF_IMAGE_FORMAT(CL_Rx, CL_UNSIGNED_INT8,      2 * 1);
    RCL_DEF_IMAGE_FORMAT(CL_Rx, CL_UNSIGNED_INT16,     2 * 2);
    RCL_DEF_IMAGE_FORMAT(CL_Rx, CL_UNSIGNED_INT32,     2 * 4);
    RCL_DEF_IMAGE_FORMAT(CL_Rx, CL_HALF_FLOAT,         2 * 2);
    RCL_DEF_IMAGE_FORMAT(CL_Rx, CL_FLOAT,              2 * 4);

    // 3 * ? I dount this.
    RCL_DEF_IMAGE_FORMAT(CL_RGx, CL_SNORM_INT8,         3 * 1);
    RCL_DEF_IMAGE_FORMAT(CL_RGx, CL_SNORM_INT16,        3 * 2);
    RCL_DEF_IMAGE_FORMAT(CL_RGx, CL_UNORM_INT8,         3 * 1);
    RCL_DEF_IMAGE_FORMAT(CL_RGx, CL_UNORM_INT16,        3 * 2);
    RCL_DEF_IMAGE_FORMAT(CL_RGx, CL_SIGNED_INT8,        3 * 1);
    RCL_DEF_IMAGE_FORMAT(CL_RGx, CL_SIGNED_INT16,       3 * 2);
    RCL_DEF_IMAGE_FORMAT(CL_RGx, CL_SIGNED_INT32,       3 * 4);
    RCL_DEF_IMAGE_FORMAT(CL_RGx, CL_UNSIGNED_INT8,      3 * 1);
    RCL_DEF_IMAGE_FORMAT(CL_RGx, CL_UNSIGNED_INT16,     3 * 2);
    RCL_DEF_IMAGE_FORMAT(CL_RGx, CL_UNSIGNED_INT32,     3 * 4);
    RCL_DEF_IMAGE_FORMAT(CL_RGx, CL_HALF_FLOAT,         3 * 2);
    RCL_DEF_IMAGE_FORMAT(CL_RGx, CL_FLOAT,              3 * 4);

    RCL_DEF_IMAGE_FORMAT(CL_RGBx, CL_UNORM_SHORT_565,    2);
    RCL_DEF_IMAGE_FORMAT(CL_RGBx, CL_UNORM_SHORT_555,    2);
    RCL_DEF_IMAGE_FORMAT(CL_RGBx, CL_UNORM_INT_101010,   4);

#endif

    return fmts;
}

void
rcl_define_class_image_format(VALUE mod)
{
    rcl_cImageFormat = rb_define_class_under(mod, "ImageFormat", rb_cObject);
    rb_define_class_variable(rcl_cImageFormat, "@@formats", rcl_define_image_formats());

    rb_define_attr(rcl_cImageFormat, "channel_order", 1, 0);
    rb_define_attr(rcl_cImageFormat, "channel_data_type", 1, 0);
    rb_define_attr(rcl_cImageFormat, "element_size", 1, 0);
    rb_define_method(rcl_cImageFormat, "initialize", rcl_image_format_init, 2);
}
