// Copyright (c) 2010, Che Kenan

#include "ieee_half_float.h"
#include "uthash.h"     // store rcl_type_t structures. Ruby Hash is heavy.

extern VALUE rcl_mOpenCL;

VALUE rcl_cPointer;
VALUE rcl_cMappedPointer;

/*
 * CL types.
 */

static VALUE type_cl_bool;
static VALUE type_cl_char;
static VALUE type_cl_uchar;
static VALUE type_cl_short;
static VALUE type_cl_ushort;
static VALUE type_cl_int;
static VALUE type_cl_uint;
static VALUE type_cl_long;
static VALUE type_cl_ulong;
static VALUE type_cl_half;
static VALUE type_cl_float;
static VALUE type_cl_double;
static VALUE type_cl_half;

static VALUE type_cl_char2;
static VALUE type_cl_char4;
static VALUE type_cl_char8;
static VALUE type_cl_char16;
static VALUE type_cl_uchar2;
static VALUE type_cl_uchar4;
static VALUE type_cl_uchar8;
static VALUE type_cl_uchar16;
static VALUE type_cl_short2;
static VALUE type_cl_short4;
static VALUE type_cl_short8;
static VALUE type_cl_short16;
static VALUE type_cl_ushort2;
static VALUE type_cl_ushort4;
static VALUE type_cl_ushort8;
static VALUE type_cl_ushort16;
static VALUE type_cl_int2;
static VALUE type_cl_int4;
static VALUE type_cl_int8;
static VALUE type_cl_int16;
static VALUE type_cl_uint2;
static VALUE type_cl_uint4;
static VALUE type_cl_uint8;
static VALUE type_cl_uint16;
static VALUE type_cl_long2;
static VALUE type_cl_long4;
static VALUE type_cl_long8;
static VALUE type_cl_long16;
static VALUE type_cl_ulong2;
static VALUE type_cl_ulong4;
static VALUE type_cl_ulong8;
static VALUE type_cl_ulong16;
static VALUE type_cl_float2;
static VALUE type_cl_float4;
static VALUE type_cl_float8;
static VALUE type_cl_float16;
static VALUE type_cl_double2;
static VALUE type_cl_double4;
static VALUE type_cl_double8;
static VALUE type_cl_double16;
// static VALUE type_cl_half2;
// static VALUE type_cl_half4;
// static VALUE type_cl_half8;
// static VALUE type_cl_half16;

#ifdef CL_VERSION_1_1
static VALUE type_cl_char3;
static VALUE type_cl_uchar3;
static VALUE type_cl_short3;
static VALUE type_cl_ushort3;
static VALUE type_cl_int3;
static VALUE type_cl_uint3;
static VALUE type_cl_long3;
static VALUE type_cl_ulong3;
static VALUE type_cl_float3;
static VALUE type_cl_double3;
// static VALUE type_cl_half3;
#endif

typedef struct {
    VALUE           id;
    int             size;
    int             length;
    UT_hash_handle  hh;
} rcl_type_t;

static rcl_type_t *rcl_types = NULL;

#define DEF_CL_TYPE(tp) \
    do { \
        rcl_type_t *ty = malloc(sizeof(rcl_type_t)); \
        type_##tp = ID2SYM(rb_intern( #tp )); \
        ty->id = type_##tp; \
        ty->size = sizeof(tp); \
        ty->length = 1; \
        HASH_ADD_PTR(rcl_types, id, ty); \
    } while (0)

#define DEF_CL_VECTOR_TYPE(tp, len) \
    do { \
        rcl_type_t *ty = malloc(sizeof(rcl_type_t)); \
        type_##tp##len = ID2SYM(rb_intern( #tp #len )); \
        ty->id = type_##tp##len; \
        ty->size = sizeof( tp##len ); \
        ty->length = len; \
        HASH_ADD_PTR(rcl_types, id, ty); \
    } while (0)


// referenced by kernel_set_arg in capi.c
size_t
rcl_type_size(VALUE type)
{
    rcl_type_t *ty = NULL;
    if (SYMBOL_P(type)) {
        HASH_FIND_PTR(rcl_types, &type, ty);
    }
    return NULL == ty ? 0 : ty->size;
}

/*
 * call-seq:
 *      OpenCL.type_size(type_tag)
 *
 * Returns the byte size of given type, or 0 if +type_tag+ is unrecognized
 * or invalid.
 */
static VALUE rcl_sizeof(VALUE self, VALUE id)
{
    return INT2FIX(rcl_type_size(id));
}

/*
 * call-seq:
 *      OpenCL.valid_vector?(type_tag)
 *
 * Returns +true+ if given type is a vector type, e.g., :cl_float16.
 *
 */
static VALUE rcl_is_type_vector(VALUE self, VALUE id)
{
    if (SYMBOL_P(id)) {
        rcl_type_t *ty = NULL;
        HASH_FIND_PTR(rcl_types, &id, ty);
        if (NULL != ty && ty->length > 1) {
            return Qtrue;
        }
    }
    return Qfalse;
}

/*
 * call-seq:
 *      OpenCL.valid_type?(type_tag)
 *
 * Returns +true+ if given type is a valid OpenCL type.
 */
static VALUE rcl_is_type_valid(VALUE self, VALUE id)
{
    if (SYMBOL_P(id)) {
        rcl_type_t *ty = NULL;
        HASH_FIND_PTR(rcl_types, &id, ty);
        if (NULL != ty) {
            return Qtrue;
        }
    }
    return Qfalse;
}

static void define_cl_types(void)
{
    DEF_CL_TYPE(cl_bool);
    DEF_CL_TYPE(cl_char);
    DEF_CL_TYPE(cl_uchar);
    DEF_CL_TYPE(cl_short);
    DEF_CL_TYPE(cl_ushort);
    DEF_CL_TYPE(cl_int);
    DEF_CL_TYPE(cl_uint);
    DEF_CL_TYPE(cl_long);
    DEF_CL_TYPE(cl_ulong);
    DEF_CL_TYPE(cl_half);
    DEF_CL_TYPE(cl_float);
    DEF_CL_TYPE(cl_double);

    DEF_CL_VECTOR_TYPE(cl_char,     2);
    DEF_CL_VECTOR_TYPE(cl_char,     4);
    DEF_CL_VECTOR_TYPE(cl_char,     8);
    DEF_CL_VECTOR_TYPE(cl_char,     16);
    DEF_CL_VECTOR_TYPE(cl_uchar,    2);
    DEF_CL_VECTOR_TYPE(cl_uchar,    4);
    DEF_CL_VECTOR_TYPE(cl_uchar,    8);
    DEF_CL_VECTOR_TYPE(cl_uchar,    16);
    DEF_CL_VECTOR_TYPE(cl_short,    2);
    DEF_CL_VECTOR_TYPE(cl_short,    4);
    DEF_CL_VECTOR_TYPE(cl_short,    8);
    DEF_CL_VECTOR_TYPE(cl_short,    16);
    DEF_CL_VECTOR_TYPE(cl_ushort,   2);
    DEF_CL_VECTOR_TYPE(cl_ushort,   4);
    DEF_CL_VECTOR_TYPE(cl_ushort,   8);
    DEF_CL_VECTOR_TYPE(cl_ushort,   16);
    DEF_CL_VECTOR_TYPE(cl_int,      2);
    DEF_CL_VECTOR_TYPE(cl_int,      4);
    DEF_CL_VECTOR_TYPE(cl_int,      8);
    DEF_CL_VECTOR_TYPE(cl_int,      16);
    DEF_CL_VECTOR_TYPE(cl_uint,     2);
    DEF_CL_VECTOR_TYPE(cl_uint,     4);
    DEF_CL_VECTOR_TYPE(cl_uint,     8);
    DEF_CL_VECTOR_TYPE(cl_uint,     16);
    DEF_CL_VECTOR_TYPE(cl_long,     2);
    DEF_CL_VECTOR_TYPE(cl_long,     4);
    DEF_CL_VECTOR_TYPE(cl_long,     8);
    DEF_CL_VECTOR_TYPE(cl_long,     16);
    DEF_CL_VECTOR_TYPE(cl_ulong,    2);
    DEF_CL_VECTOR_TYPE(cl_ulong,    4);
    DEF_CL_VECTOR_TYPE(cl_ulong,    8);
    DEF_CL_VECTOR_TYPE(cl_ulong,    16);
    DEF_CL_VECTOR_TYPE(cl_float,    2);
    DEF_CL_VECTOR_TYPE(cl_float,    4);
    DEF_CL_VECTOR_TYPE(cl_float,    8);
    DEF_CL_VECTOR_TYPE(cl_float,    16);
    DEF_CL_VECTOR_TYPE(cl_double,   2);
    DEF_CL_VECTOR_TYPE(cl_double,   4);
    DEF_CL_VECTOR_TYPE(cl_double,   8);
    DEF_CL_VECTOR_TYPE(cl_double,   16);
    // DEF_CL_VECTOR_TYPE(cl_half,     2);
    // DEF_CL_VECTOR_TYPE(cl_half,     4);
    // DEF_CL_VECTOR_TYPE(cl_half,     8);
    // DEF_CL_VECTOR_TYPE(cl_half,     16);

#ifdef CL_VERSION_1_1
    DEF_CL_VECTOR_TYPE(cl_char,     3);
    DEF_CL_VECTOR_TYPE(cl_uchar,    3);
    DEF_CL_VECTOR_TYPE(cl_short,    3);
    DEF_CL_VECTOR_TYPE(cl_ushort,   3);
    DEF_CL_VECTOR_TYPE(cl_int,      3);
    DEF_CL_VECTOR_TYPE(cl_uint,     3);
    DEF_CL_VECTOR_TYPE(cl_long,     3);
    DEF_CL_VECTOR_TYPE(cl_ulong,    3);
    DEF_CL_VECTOR_TYPE(cl_float,    3);
    DEF_CL_VECTOR_TYPE(cl_double,   3);
    // DEF_CL_VECTOR_TYPE(cl_half,     3);
#endif

    rb_define_module_function(rcl_mOpenCL, "type_size", rcl_sizeof, 1);
    rb_define_module_function(rcl_mOpenCL, "valid_type?", rcl_is_type_valid, 1);
    rb_define_module_function(rcl_mOpenCL, "valid_vector?", rcl_is_type_vector, 1);
}

#define IF_TYPE_TO_NATIVE(c_type, expector, convertor) \
    if (type == type_##c_type) { \
        expector(value); \
        c_type *ptr = (c_type *)address; \
        ptr[0] = (c_type)convertor(value); \
        return; \
    }

#define IF_VECTOR_TYPE_TO_NATIVE(base_c_type, n, expector, convertor) \
    if (type == type_##base_c_type##n) { \
        EXPECT_NON_EMPTY_ARRAY(value); \
        if (n > RARRAY_LEN(value)) { \
            rb_raise(rb_eArgError, "expected number of elements is %d, but got %ld", n, RARRAY_LEN(value)); \
        } \
        base_c_type *ptr = (base_c_type *)address; \
        for (int i = 0; i < n; i++) { \
            VALUE v = rb_ary_entry(value, i); \
            expector(v); \
            ptr[i] = (base_c_type)convertor(v); \
        } \
        return; \
    }

// Referenced by kernel_set_arg in capi.c
void
rcl_ruby2native(VALUE type, void *address, VALUE value)
{
    assert(!(NIL_P(value) || NULL == address));

    IF_TYPE_TO_NATIVE(cl_float,  EXPECT_FLOAT,   NUM2DBL);
    IF_TYPE_TO_NATIVE(cl_ushort, EXPECT_FIXNUM,  FIX2INT);
    IF_TYPE_TO_NATIVE(cl_uint,   EXPECT_INTEGER, NUM2UINT);
    IF_TYPE_TO_NATIVE(cl_char,   EXPECT_FIXNUM,  FIX2INT);
    IF_TYPE_TO_NATIVE(cl_uchar,  EXPECT_FIXNUM,  FIX2INT);
    IF_TYPE_TO_NATIVE(cl_short,  EXPECT_FIXNUM,  FIX2INT);
    IF_TYPE_TO_NATIVE(cl_int,    EXPECT_INTEGER, FIX2INT);
    IF_TYPE_TO_NATIVE(cl_long,   EXPECT_INTEGER, NUM2LONG);
    IF_TYPE_TO_NATIVE(cl_ulong,  EXPECT_INTEGER, NUM2ULONG);
    IF_TYPE_TO_NATIVE(cl_half,   EXPECT_FLOAT,   ExtractHalf);
    IF_TYPE_TO_NATIVE(cl_double, EXPECT_FLOAT,   NUM2DBL);

    IF_VECTOR_TYPE_TO_NATIVE(cl_char,   2,  EXPECT_FIXNUM,  FIX2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_char,   4,  EXPECT_FIXNUM,  FIX2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_char,   8,  EXPECT_FIXNUM,  FIX2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_char,   16, EXPECT_FIXNUM,  FIX2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_uchar,  2,  EXPECT_FIXNUM,  FIX2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_uchar,  4,  EXPECT_FIXNUM,  FIX2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_uchar,  8,  EXPECT_FIXNUM,  FIX2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_uchar,  16, EXPECT_FIXNUM,  FIX2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_short,  2,  EXPECT_FIXNUM,  FIX2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_short,  4,  EXPECT_FIXNUM,  FIX2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_short,  8,  EXPECT_FIXNUM,  FIX2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_short,  16, EXPECT_FIXNUM,  FIX2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_ushort, 2,  EXPECT_FIXNUM,  FIX2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_ushort, 4,  EXPECT_FIXNUM,  FIX2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_ushort, 8,  EXPECT_FIXNUM,  FIX2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_ushort, 16, EXPECT_FIXNUM,  FIX2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_int,    2,  EXPECT_INTEGER, FIX2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_int,    4,  EXPECT_INTEGER, FIX2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_int,    8,  EXPECT_INTEGER, FIX2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_int,    16, EXPECT_INTEGER, FIX2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_uint,   2,  EXPECT_INTEGER, NUM2UINT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_uint,   4,  EXPECT_INTEGER, NUM2UINT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_uint,   8,  EXPECT_INTEGER, NUM2UINT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_uint,   16, EXPECT_INTEGER, NUM2UINT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_long,   2,  EXPECT_INTEGER, NUM2LONG);
    IF_VECTOR_TYPE_TO_NATIVE(cl_long,   4,  EXPECT_INTEGER, NUM2LONG);
    IF_VECTOR_TYPE_TO_NATIVE(cl_long,   8,  EXPECT_INTEGER, NUM2LONG);
    IF_VECTOR_TYPE_TO_NATIVE(cl_long,   16, EXPECT_INTEGER, NUM2LONG);
    IF_VECTOR_TYPE_TO_NATIVE(cl_ulong,  2,  EXPECT_INTEGER, NUM2ULONG);
    IF_VECTOR_TYPE_TO_NATIVE(cl_ulong,  4,  EXPECT_INTEGER, NUM2ULONG);
    IF_VECTOR_TYPE_TO_NATIVE(cl_ulong,  8,  EXPECT_INTEGER, NUM2ULONG);
    IF_VECTOR_TYPE_TO_NATIVE(cl_ulong,  16, EXPECT_INTEGER, NUM2ULONG);
    IF_VECTOR_TYPE_TO_NATIVE(cl_float,  2,  EXPECT_FLOAT,   NUM2DBL);
    IF_VECTOR_TYPE_TO_NATIVE(cl_float,  4,  EXPECT_FLOAT,   NUM2DBL);
    IF_VECTOR_TYPE_TO_NATIVE(cl_float,  8,  EXPECT_FLOAT,   NUM2DBL);
    IF_VECTOR_TYPE_TO_NATIVE(cl_float,  16, EXPECT_FLOAT,   NUM2DBL);
    IF_VECTOR_TYPE_TO_NATIVE(cl_double, 2,  EXPECT_FLOAT,   NUM2DBL);
    IF_VECTOR_TYPE_TO_NATIVE(cl_double, 4,  EXPECT_FLOAT,   NUM2DBL);
    IF_VECTOR_TYPE_TO_NATIVE(cl_double, 8,  EXPECT_FLOAT,   NUM2DBL);
    IF_VECTOR_TYPE_TO_NATIVE(cl_double, 16, EXPECT_FLOAT,   NUM2DBL);
    // IF_VECTOR_TYPE_TO_NATIVE(cl_half,   2,  EXPECT_FLOAT,   ExtractHalf);
    // IF_VECTOR_TYPE_TO_NATIVE(cl_half,   4,  EXPECT_FLOAT,   ExtractHalf);
    // IF_VECTOR_TYPE_TO_NATIVE(cl_half,   8,  EXPECT_FLOAT,   ExtractHalf);
    // IF_VECTOR_TYPE_TO_NATIVE(cl_half,   16, EXPECT_FLOAT,   ExtractHalf);
#ifdef CL_VERSION_1_1
    IF_VECTOR_TYPE_TO_NATIVE(cl_char,   3,  EXPECT_FIXNUM,  FIX2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_uchar,  3,  EXPECT_FIXNUM,  FIX2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_short,  3,  EXPECT_FIXNUM,  FIX2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_ushort, 3,  EXPECT_FIXNUM,  FIX2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_int,    3,  EXPECT_INTEGER, FIX2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_uint,   3,  EXPECT_INTEGER, NUM2UINT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_long,   3,  EXPECT_INTEGER, NUM2LONG);
    IF_VECTOR_TYPE_TO_NATIVE(cl_ulong,  3,  EXPECT_INTEGER, NUM2ULONG);
    IF_VECTOR_TYPE_TO_NATIVE(cl_float,  3,  EXPECT_FLOAT,   NUM2DBL);
    IF_VECTOR_TYPE_TO_NATIVE(cl_double, 3,  EXPECT_FLOAT,   NUM2DBL);
    // IF_VECTOR_TYPE_TO_NATIVE(cl_half,   3,  EXPECT_FLOAT,   ExtractHalf);
#endif

    if (type == ID2SYM(rb_intern("cl_bool"))) {
        EXTRACT_BOOLEAN(value, bv);
        *(cl_bool *)address = bv;
        return;
    }

    rb_raise(rb_eArgError, "invalid type tag.");
}

#define IF_TYPE_TO_RUBY(c_type, convertor) \
    if (type == type_##c_type) { \
        c_type *ptr = (c_type *)address; \
        return convertor(ptr[0]); \
    }

#define IF_VECTOR_TYPE_TO_RUBY(base_c_type, n, convertor) \
    if (type == type_##base_c_type##n) { \
        VALUE ret = rb_ary_new2(n); \
        base_c_type *ptr = (base_c_type *)address; \
        for (int i = 0; i < n; i++) { \
            rb_ary_push(ret, convertor(ptr[i])); \
        } \
        return ret; \
    }

static inline VALUE
rcl_native2ruby(VALUE type, void *address)
{
    assert(NULL != address);

    IF_TYPE_TO_RUBY(cl_float,   rb_float_new);
    IF_TYPE_TO_RUBY(cl_ushort,  UINT2NUM);
    IF_TYPE_TO_RUBY(cl_uint,    UINT2NUM);
    IF_TYPE_TO_RUBY(cl_char,    INT2FIX);
    IF_TYPE_TO_RUBY(cl_uchar,   UINT2NUM);
    IF_TYPE_TO_RUBY(cl_short,   INT2FIX);
    IF_TYPE_TO_RUBY(cl_int,     INT2NUM);
    IF_TYPE_TO_RUBY(cl_long,    LONG2NUM);
    IF_TYPE_TO_RUBY(cl_ulong,   ULONG2NUM);
    IF_TYPE_TO_RUBY(cl_half,    rcl_half_float_new);
    IF_TYPE_TO_RUBY(cl_double,  rb_float_new);

    IF_VECTOR_TYPE_TO_RUBY(cl_char,   2,  INT2FIX);
    IF_VECTOR_TYPE_TO_RUBY(cl_char,   4,  INT2FIX);
    IF_VECTOR_TYPE_TO_RUBY(cl_char,   8,  INT2FIX);
    IF_VECTOR_TYPE_TO_RUBY(cl_char,   16, INT2FIX);
    IF_VECTOR_TYPE_TO_RUBY(cl_uchar,  2,  UINT2NUM);
    IF_VECTOR_TYPE_TO_RUBY(cl_uchar,  4,  UINT2NUM);
    IF_VECTOR_TYPE_TO_RUBY(cl_uchar,  8,  UINT2NUM);
    IF_VECTOR_TYPE_TO_RUBY(cl_uchar,  16, UINT2NUM);
    IF_VECTOR_TYPE_TO_RUBY(cl_short,  2,  INT2FIX);
    IF_VECTOR_TYPE_TO_RUBY(cl_short,  4,  INT2FIX);
    IF_VECTOR_TYPE_TO_RUBY(cl_short,  8,  INT2FIX);
    IF_VECTOR_TYPE_TO_RUBY(cl_short,  16, INT2FIX);
    IF_VECTOR_TYPE_TO_RUBY(cl_ushort, 2,  UINT2NUM);
    IF_VECTOR_TYPE_TO_RUBY(cl_ushort, 4,  UINT2NUM);
    IF_VECTOR_TYPE_TO_RUBY(cl_ushort, 8,  UINT2NUM);
    IF_VECTOR_TYPE_TO_RUBY(cl_ushort, 16, UINT2NUM);
    IF_VECTOR_TYPE_TO_RUBY(cl_int,    2,  INT2NUM);
    IF_VECTOR_TYPE_TO_RUBY(cl_int,    4,  INT2NUM);
    IF_VECTOR_TYPE_TO_RUBY(cl_int,    8,  INT2NUM);
    IF_VECTOR_TYPE_TO_RUBY(cl_int,    16, INT2NUM);
    IF_VECTOR_TYPE_TO_RUBY(cl_uint,   2,  UINT2NUM);
    IF_VECTOR_TYPE_TO_RUBY(cl_uint,   4,  UINT2NUM);
    IF_VECTOR_TYPE_TO_RUBY(cl_uint,   8,  UINT2NUM);
    IF_VECTOR_TYPE_TO_RUBY(cl_uint,   16, UINT2NUM);
    IF_VECTOR_TYPE_TO_RUBY(cl_long,   2,  LONG2NUM);
    IF_VECTOR_TYPE_TO_RUBY(cl_long,   4,  LONG2NUM);
    IF_VECTOR_TYPE_TO_RUBY(cl_long,   8,  LONG2NUM);
    IF_VECTOR_TYPE_TO_RUBY(cl_long,   16, LONG2NUM);
    IF_VECTOR_TYPE_TO_RUBY(cl_ulong,  2,  ULONG2NUM);
    IF_VECTOR_TYPE_TO_RUBY(cl_ulong,  4,  ULONG2NUM);
    IF_VECTOR_TYPE_TO_RUBY(cl_ulong,  8,  ULONG2NUM);
    IF_VECTOR_TYPE_TO_RUBY(cl_ulong,  16, ULONG2NUM);
    IF_VECTOR_TYPE_TO_RUBY(cl_float,  2,  rb_float_new);
    IF_VECTOR_TYPE_TO_RUBY(cl_float,  4,  rb_float_new);
    IF_VECTOR_TYPE_TO_RUBY(cl_float,  8,  rb_float_new);
    IF_VECTOR_TYPE_TO_RUBY(cl_float,  16, rb_float_new);
    IF_VECTOR_TYPE_TO_RUBY(cl_double, 2,  rb_float_new);
    IF_VECTOR_TYPE_TO_RUBY(cl_double, 4,  rb_float_new);
    IF_VECTOR_TYPE_TO_RUBY(cl_double, 8,  rb_float_new);
    IF_VECTOR_TYPE_TO_RUBY(cl_double, 16, rb_float_new);
    // IF_VECTOR_TYPE_TO_RUBY(cl_half,   2,  rcl_half_float_new);
    // IF_VECTOR_TYPE_TO_RUBY(cl_half,   4,  rcl_half_float_new);
    // IF_VECTOR_TYPE_TO_RUBY(cl_half,   8,  rcl_half_float_new);
    // IF_VECTOR_TYPE_TO_RUBY(cl_half,   16, rcl_half_float_new);
#ifdef CL_VERSINO_1_1
    IF_VECTOR_TYPE_TO_RUBY(cl_char,   3,  INT2FIX);
    IF_VECTOR_TYPE_TO_RUBY(cl_uchar,  3,  UINT2NUM);
    IF_VECTOR_TYPE_TO_RUBY(cl_short,  3,  INT2FIX);
    IF_VECTOR_TYPE_TO_RUBY(cl_ushort, 3,  UINT2NUM);
    IF_VECTOR_TYPE_TO_RUBY(cl_int,    3,  INT2NUM);
    IF_VECTOR_TYPE_TO_RUBY(cl_uint,   3,  UINT2NUM);
    IF_VECTOR_TYPE_TO_RUBY(cl_long,   3,  LONG2NUM);
    IF_VECTOR_TYPE_TO_RUBY(cl_ulong,  3,  ULONG2NUM);
    IF_VECTOR_TYPE_TO_RUBY(cl_float,  3,  rb_float_new);
    IF_VECTOR_TYPE_TO_RUBY(cl_double, 3,  rb_float_new);
    // IF_VECTOR_TYPE_TO_RUBY(cl_half,   3,  rcl_half_float_new);
#endif

    if (type == ID2SYM(rb_intern("cl_bool"))) {
        return (*(cl_bool *)address) ? Qtrue : Qfalse;
    }

    return Qnil;
}

/*
 * class HostPointer
 */

typedef struct {
    int8_t  *alloc_address;
    void    *address;
    size_t   size;              // in number of elements, not in byte.
    size_t   type_size;
    VALUE    type;
    int      is_wrapper;
    int      is_dirty;
} rcl_pointer_t;

static inline size_t
BytesOf(rcl_pointer_t *p)
{
    return p->size * p->type_size;
}

#define NEED_ALLOC(p)     (BytesOf(p) > sizeof(intptr_t) && !(p->is_wrapper))
#define IS_POINTER(p)     (p->alloc_address != NULL || p->is_wrapper)
#define ALLOC_SIZE_OF(p)  (BytesOf(p) + 0x80)

static inline void
AllocMemory(rcl_pointer_t *p)
{
    assert(p->size > 0 && p->type_size > 0);

    size_t alloc_sz = ALLOC_SIZE_OF(p);     // align in 128bytes.
    GC_WB(&p->alloc_address, ALLOC_N(int8_t, alloc_sz));
    bzero(p->alloc_address, alloc_sz);

    if (p->alloc_address == NULL) {
        rb_raise(rb_eRuntimeError, "out of host memory.");
    }
    p->address = (void *)(((intptr_t)(p->alloc_address) + 0x80) & ~0x7F);
}

static inline rcl_pointer_t *
PointerPtr(VALUE ptr)
{
    rcl_pointer_t *p;
    Data_Get_Struct(ptr, rcl_pointer_t, p);

    return p;
}

size_t
PointerSize(VALUE ptr)
{
    rcl_pointer_t *p = PointerPtr(ptr);
    return BytesOf(p);
}

// Used by Kernel#set_arg, so not local.
void *
PointerAddress(VALUE ptr)
{
    rcl_pointer_t *p = PointerPtr(ptr);

    if (!IS_POINTER(p)) {
        return p->size == 0 ? NULL : &(p->address);
    } else {
        return p->address;
    }
}

static inline void *
ElementAddress(rcl_pointer_t *ptr, size_t index)
{
    assert(index < ptr->size);
    if (!IS_POINTER(ptr)) {
        return (void *)((int8_t *)&(ptr->address) + index * ptr->type_size);
    } else {
        return (void *)((int8_t *)(ptr->address) + index * ptr->type_size);
    }
}

static void
rcl_pointer_free_func(void *ptr)
{
    // NOTE: It's safe to free a NULL pointer. So no special case for wrapped
    // pointer and short values.
    xfree(((rcl_pointer_t *)ptr)->alloc_address);
    xfree(ptr);
}

/*
 * call-seq:
 *      HostPointer.allocate    -> HostPointer
 *
 * Allocate a HostPointer object.
 */
static VALUE
rcl_pointer_alloc(VALUE klass)
{
    rcl_pointer_t *p;
    VALUE ro = Data_Make_Struct(klass, rcl_pointer_t, 0, rcl_pointer_free_func, p);

    p->alloc_address = p->address = NULL;
    p->size = 0;
    p->type = Qnil;
    p->type_size = sizeof(cl_uint);
    p->is_wrapper = 0;

    return ro;
}

/*
 * call-seq:
 *      HostPointer.wrap(aBuffer.pointer, :cl_float, 2048)
 *
 * Wraps a C pointer instead of allocating memory.
 *
 * +wrap()+ is a simple but dangerous optimization by
 * reducing memory copying.
 *
 * The wrapper is not on charge of releasing memory pointed by the wrapped
 * C pointer, and accessing the memory through the wrapper after the C pointer
 * is freed definitely causes a segment fault.
 *
 * Sending the wrapper a +free()+ message stops the wrapping.
 *
 * See also: +HostPointer#free()+
 */
static VALUE
rcl_pointer_wrap(VALUE klass, VALUE address, VALUE type, VALUE size)
{
    Check_Type(address, T_FIXNUM);
    void *addr = (void *)NUM2ULONG(address);

    Check_Type(type, T_SYMBOL);
    int tsz = rcl_type_size(type);
    if (tsz == 0) {
        rb_raise(rb_eArgError, "invalid type tag.");
    }

    EXTRACT_SIZE(size, sz);

    rcl_pointer_t *p;
    VALUE ro = Data_Make_Struct(klass, rcl_pointer_t, 0, rcl_pointer_free_func, p);

    p->alloc_address = NULL;
    p->address = addr;
    p->size = sz;
    p->type = type;
    p->type_size = tsz;
    p->is_wrapper = 1;

    assert(IS_POINTER(p));

    return ro;
}

/*
 * call-seq:
 *      HostPointer#clear   ->the receiver
 *
 * Set the memory that the receiver manages to zero.
 */
static VALUE
rcl_pointer_clear(VALUE self)
{
    rcl_pointer_t *p = PointerPtr(self);
    if (p->size > 0) {
        if (!IS_POINTER(p)) {
            p->address = 0;
        } else {
            bzero(p->alloc_address, ALLOC_SIZE_OF(p));
        }
    }
    return self;
}

/*
 * call-seq:
 *      HostPointer::new(type, size)    -> the receiver
 *
 * Allocate a managed host memory.
 */
static VALUE
rcl_pointer_init(VALUE self, VALUE type, VALUE size)
{
    rcl_pointer_t *p;
    Data_Get_Struct(self, rcl_pointer_t, p);

    Check_Type(type, T_SYMBOL);
    int tsz = rcl_type_size(type);
    if (tsz == 0) {
        rb_raise(rb_eArgError, "unrecognized type tag.");
    }

    EXTRACT_SIZE(size, lsz);
    if (lsz < 1) {
        rb_raise(rb_eArgError, "invalid size.");
    }

    p->type = type;
    p->type_size = tsz;
    p->size = lsz;
    p->is_dirty = Qfalse;

    if (NEED_ALLOC(p)) {
        AllocMemory(p);
    }
    return rcl_pointer_clear(self);
}

static VALUE
rcl_pointer_init_copy(VALUE copy, VALUE orig)
{
    EXPECT_RCL_TYPE(orig, Pointer);

    rcl_pointer_t *copy_p = PointerPtr(copy);
    rcl_pointer_t *orig_p = PointerPtr(orig);

    if (orig_p->size == 0) {
        assert(orig_p->alloc_address == NULL && orig_p->address == NULL);
        rb_raise(rb_eRuntimeError, "can't clone a null pointer.");
    }

    assert(copy_p->alloc_address == NULL);
    copy_p->type = orig_p->type;
    copy_p->size = orig_p->size;
    copy_p->type_size = orig_p->type_size;
    copy_p->is_dirty = Qfalse;

    if (!IS_POINTER(orig_p)) {
        assert(!NEED_ALLOC(copy_p));
        copy_p->address = orig_p->address;
    } else {
        AllocMemory(copy_p);
        memcpy(copy_p->address, orig_p->address, BytesOf(copy_p));
    }
    return copy;
}

/*
 * call-seq:
 *      aHostPointer[0]
 *
 * Returns the n-th element stored in the memory region managed by
 * the receiver.
 */
static VALUE
rcl_pointer_aref(VALUE self, VALUE index)
{
    EXPECT_FIXNUM(index);
    int i = FIX2INT(index);

    rcl_pointer_t *p = PointerPtr(self);
    if (i >= p->size || i < 0) {
        return Qnil;
    } else {
        return rcl_native2ruby(p->type, ElementAddress(p, i));
    }
}

/*
 * call-seq:
 *      aHostPointer[0] = 1234  -> the  receiver
 *
 * Sets the n-th element stored to the given value.
 * The value must match with the receiver's type.
 */
static VALUE
rcl_pointer_aset(VALUE self, VALUE index, VALUE value)
{
    rcl_pointer_t *p = PointerPtr(self);
    EXTRACT_SIZE(index, i);

    if (NIL_P(value)) {
        rb_raise(rb_eArgError, "value can't be nil.");
    }

    if (i >= p->size) {
        rb_raise(rb_eRuntimeError, "subscriber exceeds the boundary.");
    }
    rcl_ruby2native(p->type, ElementAddress(p, i), value);
    p->is_dirty = Qtrue;
    return self;
}

/*
 * call-seq:
 *      HostPointer#assign_pointer, 0x123456, 1024, 4096
 *
 * Copy contents from a C pointer to the receiver.
 *
 * Returns the receiver.
 */
static VALUE
rcl_pointer_assign(VALUE self, VALUE address, VALUE size, VALUE offset)
{
    Check_Type(address, T_FIXNUM);
    void *addr = (void *)NUM2ULONG(address);

    rcl_pointer_t *p = PointerPtr(self);

    EXTRACT_SIZE(size, sz);
    EXTRACT_SIZE(offset, os);
    if (sz + os > p->size) {
        rb_raise(rb_eArgError, "size or offset is too large.");
    }

    size_t cpysz = sz * p->type_size;
    memcpy(ElementAddress(p, os), addr, cpysz);
    p->is_dirty = Qtrue;

    return self;
}

/*
 * call-seq:
 *      HostPointer#assign_byte_string(aString, offset)     -> the receiver.
 */
static VALUE
rcl_pointer_assign_byte_string(VALUE self, VALUE value, VALUE offset)
{
    Check_Type(value, T_STRING);

    rcl_pointer_t *p = PointerPtr(self);
    EXTRACT_SIZE(offset, os);
    if (os >= p->size) {
        rb_raise(rb_eArgError, "offset exceeds the boundary.");
    }

    const char *ptr = RSTRING_PTR(value);
    size_t sz = RSTRING_LEN(value);
    if (sz % p->type_size != 0) {
        rb_raise(rb_eArgError, "size of byte string does not match the data type of receiver.");
    }

    size_t bos = os * p->type_size;
    size_t cpysz = (BytesOf(p) - bos) > sz ? sz : (BytesOf(p) - bos);
    memcpy(ElementAddress(p, os), (void *)ptr, cpysz);
    p->is_dirty = Qtrue;

    return self;
}

/*
 * Returns +true+ if the receiver has been changed on host side.
 *
 * call-seq:
 *      HostPointer#dirty?
 */
static VALUE
rcl_pointer_is_dirty(VALUE self)
{
    rcl_pointer_t *p = PointerPtr(self);
    return p->is_dirty;
}

/*
 * Sets the dirty flag of the receiver.
 *
 * call-seq:
 *      mark_dirty
 */
static VALUE
rcl_pointer_mark_dirty(VALUE self)
{
    rcl_pointer_t *p = PointerPtr(self);
    p->is_dirty = Qtrue;
    return self;
}

/*
 * Clears the dirty flag of the receiver.
 *
 * call-seq:
 *      HostPointer#clear_dirty
 */
static VALUE
rcl_pointer_clear_dirty(VALUE self)
{
    rcl_pointer_t *p = PointerPtr(self);
    p->is_dirty = Qfalse;
    return self;
}

/*
 * call-seq:
 *      HostPointer#address
 *
 * Returns an Integer that is the memory address the recever points to.
 * Needed for enqueuing buffers to CL.
 */
static VALUE
rcl_pointer_address(VALUE self)
{
    intptr_t addr = (intptr_t)PointerAddress(self);
    return addr == 0 ? Qnil : LONG2FIX(addr);
}

/*
 * call-seq:
 *      HostPointer#type    -> Symbol
 *
 * Returns the type of the receiver.
 */
static VALUE
rcl_pointer_type(VALUE self)
{
    return PointerPtr(self)->type;
}

/*
 * call-seq:
 *      HostPointer#size    -> Fixnum
 *
 * Returns the size of the receiver.
 */
static VALUE
rcl_pointer_size(VALUE self)
{
    return LONG2FIX(PointerPtr(self)->size);
}

/*
 * call-seq:
 *      aHostPointer.byte_size   -> Fixnum
 *
 * Returns the size of the receiver in byte.
 */
static VALUE
rcl_pointer_byte_size(VALUE self)
{
    return ULONG2NUM(PointerSize(self));
}

/*
 * call-seq:
 *      aHostPointer.free   -> the receiver.
 *
 * Frees the memory the receiver manages, or stops wrapping a C pointer
 * if the receiver is a wrapper.
 *
 * +free()+ is used to release memory resources explicitly.
 *
 * After +free()+, the receiver's address is set to +nil+, and size is set
 * to 0. So you can do nothing with the receiver any longer.
 */
static VALUE
rcl_pointer_free(VALUE self)
{
    rcl_pointer_t *ptr = PointerPtr(self);
    if (!IS_POINTER(ptr)) {
        ptr->address = NULL;
        ptr->size = 0;
        return self;
    }

    xfree(ptr->alloc_address);
    ptr->alloc_address = NULL;
    ptr->address = NULL;
    ptr->size = 0;
    ptr->is_wrapper = 0;

    return self;
}

/*
 * call-seq:
 *      HostPointer#copy_from(ptr)   -> the receiver
 *
 * Copy the contents of a HostPointer to the receiver. The source and receiver
 * must have identical type and size.
 *
 * Returns the receiver.
 *
 * Raises +RuntimeError+ if type or size mismatch.
 */
static VALUE
rcl_pointer_copy_from(VALUE self, VALUE src)
{
    EXPECT_RCL_TYPE(src, Pointer);

    rcl_pointer_t *p = PointerPtr(self);
    rcl_pointer_t *sp = PointerPtr(src);

    if (p->type != sp->type || p->size != sp->size) {
        rb_raise(rb_eRuntimeError, "size or type of source and target mismatch.");
    }
    if (!IS_POINTER(p)) {
        p->address = sp->address;
    } else {
        memcpy(p->address, sp->address, BytesOf(p));
    }
    return self;
}

/*
 * call-seq:
 *      HostPointer#slice(start, size)  -> HostPointer
 *
 * Returns a new created HostPointer object which contains copied data from
 * receiver. The range of data starts from +start+ and with length +size+.
 *
 * If the range exceeds the size of HostPointer, returns +nil+.
 */
static VALUE
rcl_pointer_slice(VALUE self, VALUE start, VALUE size)
{
    EXTRACT_SIZE(start, st);
    EXTRACT_SIZE(size, sz);

    rcl_pointer_t *p = PointerPtr(self);

    if (p->size == 0) return Qnil;
    if (st + sz > p->size) return Qnil;

    VALUE ro = rcl_pointer_alloc(rcl_cPointer);
    assert(CLASS_OF(ro) == rcl_cPointer);
    rcl_pointer_t *hp = PointerPtr(ro);
    assert(hp->alloc_address == NULL);

    hp->type = p->type;
    hp->size = sz;
    hp->type_size = p->type_size;

    if (NEED_ALLOC(hp)) {
        AllocMemory(hp);
    }
    memcpy(ElementAddress(hp, 0), ElementAddress(p, st), BytesOf(hp));

    return ro;
}

/*
 * class MappedPointer
 */

VALUE
rcl_create_mapped_pointer(void *address, size_t size)
{
    rcl_pointer_t *p;
    VALUE mp = Data_Make_Struct(rcl_cMappedPointer, rcl_pointer_t, 0, 0, p);

    p->type = ID2SYM(rb_intern("cl_uchar"));
    p->type_size = sizeof(cl_uchar);
    p->alloc_address = p->address = address;
    p->size = size;
    p->is_wrapper = 1;

    return mp;
}

// referenced by rcl_cq_enqueu_unmap_mem_object
void
rcl_invalidate_mapped_pointer(VALUE ptr)
{
    rcl_pointer_t *p = PointerPtr(ptr);
    p->alloc_address = p->address = NULL;
    p->size = 0;
}

/*
 * call-seq:
 *      cast_to(type)
 */
static VALUE
rcl_mapped_pointer_coerce(VALUE self, VALUE type)
{
    Check_Type(type, T_SYMBOL);

    rcl_pointer_t *p = PointerPtr(self);
    if (p->type == type) return self;
    if (p->size == 0) {
        rb_warn("receiver is a null pointer.");
        return self;
    }

    int tsz = rcl_type_size(type);
    if (tsz == 0) {
        rb_raise(rb_eArgError, "unrecognized type name.");
    }
    size_t sz = BytesOf(p);

    if (sz % tsz != 0) {
        rb_raise(rb_eRuntimeError, "casting to incompatible pointer type.");
    }
    size_t csz = sz / tsz;

    p->type = type;
    p->size = csz;
    p->type_size = tsz;

    return self;
}

/*
 * call-seq:
 *      read_as_type(address, type)
 */
static VALUE
rcl_mapped_pointer_read_as_type(VALUE self, VALUE address, VALUE type)
{
    Check_Type(type, T_SYMBOL);
    int tsz = rcl_type_size(type);
    if (tsz == 0) {
        rb_raise(rb_eArgError, "unrecognized type name.");
    }

    rcl_pointer_t *p = PointerPtr(self);

    EXTRACT_SIZE(address, addr);
    if (addr + tsz > BytesOf(p)) {
        rb_raise(rb_eArgError, "byte index is too large.");
    }
    return rcl_native2ruby(type, (void *)((int8_t *)p->address + addr));
}

static VALUE
rcl_mapped_pointer_write_as_type(VALUE self, VALUE address, VALUE type, VALUE value)
{
    Check_Type(type, T_SYMBOL);
    int tsz = rcl_type_size(type);
    if (tsz == 0) {
        rb_raise(rb_eArgError, "unrecognized type name.");
    }

    rcl_pointer_t *p = PointerPtr(self);

    EXTRACT_SIZE(address, addr);
    if (addr + tsz > BytesOf(p)) {
        rb_raise(rb_eArgError, "byte index is too large.");
    }
    rcl_ruby2native(type, (void *)((int8_t *)p->address + addr), value);
    p->is_dirty = Qtrue;
    return self;
}

/*
 * Exports.
 */

void
define_rcl_class_pointer(void)
{
    define_cl_types();

    rcl_cPointer = rb_define_class_under(rcl_mOpenCL, "HostPointer", rb_cObject);
    rb_define_singleton_method(rcl_cPointer, "wrap_pointer", rcl_pointer_wrap, 3);
    rb_define_alloc_func(rcl_cPointer, rcl_pointer_alloc);
    rb_define_method(rcl_cPointer, "initialize", rcl_pointer_init, 2);
    rb_define_method(rcl_cPointer, "initialize_copy", rcl_pointer_init_copy, 1);
    rb_define_method(rcl_cPointer, "[]", rcl_pointer_aref, 1);
    rb_define_method(rcl_cPointer, "[]=", rcl_pointer_aset, 2);
    rb_define_method(rcl_cPointer, "assign_pointer", rcl_pointer_assign, 3);
    rb_define_method(rcl_cPointer, "assign_byte_string", rcl_pointer_assign_byte_string, 2);
    rb_define_method(rcl_cPointer, "address", rcl_pointer_address, 0);
    rb_define_method(rcl_cPointer, "type", rcl_pointer_type, 0);
    rb_define_method(rcl_cPointer, "size", rcl_pointer_size, 0);
    rb_define_method(rcl_cPointer, "byte_size", rcl_pointer_byte_size, 0);
    rb_define_method(rcl_cPointer, "free", rcl_pointer_free, 0);
    rb_define_method(rcl_cPointer, "clear", rcl_pointer_clear, 0);
    rb_define_method(rcl_cPointer, "copy_from", rcl_pointer_copy_from, 1);
    rb_define_method(rcl_cPointer, "slice", rcl_pointer_slice, 2);

    rcl_cMappedPointer = rb_define_class_under(rcl_mOpenCL, "MappedPointer", rb_cObject);
    rb_undef_alloc_func(rcl_cMappedPointer);
    rb_define_method(rcl_cMappedPointer, "[]", rcl_pointer_aref, 1);
    rb_define_method(rcl_cMappedPointer, "[]=", rcl_pointer_aset, 2);
    rb_define_method(rcl_cMappedPointer, "dirty?", rcl_pointer_is_dirty, 0);
    rb_define_method(rcl_cMappedPointer, "mark_dirty", rcl_pointer_mark_dirty, 0);
    rb_define_method(rcl_cMappedPointer, "clear_dirty", rcl_pointer_clear_dirty, 0);
    rb_define_method(rcl_cMappedPointer, "assign_pointer", rcl_pointer_assign, 3);
    rb_define_method(rcl_cMappedPointer, "assign_byte_string", rcl_pointer_assign_byte_string, 2);
    rb_define_method(rcl_cMappedPointer, "address", rcl_pointer_address, 0);
    rb_define_method(rcl_cMappedPointer, "type", rcl_pointer_type, 0);
    rb_define_method(rcl_cMappedPointer, "size", rcl_pointer_size, 0);
    rb_define_method(rcl_cMappedPointer, "byte_size", rcl_pointer_byte_size, 0);
    rb_define_method(rcl_cMappedPointer, "clear", rcl_pointer_clear, 0);
    rb_define_method(rcl_cMappedPointer, "cast_to", rcl_mapped_pointer_coerce, 1);
    rb_define_method(rcl_cMappedPointer, "read_as_type", rcl_mapped_pointer_read_as_type, 2);
    rb_define_method(rcl_cMappedPointer, "write_as_type", rcl_mapped_pointer_write_as_type, 3);
}
