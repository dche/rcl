// Copyright (c) 2010, Che Kenan

#include "ieee_half_float.h"

extern VALUE rcl_mOpenCL;

VALUE rcl_cPointer;
VALUE rcl_cMappedPointer;

/*
 * CL types.
 */

static ID id_type_cl_bool;
static ID id_type_cl_char;
static ID id_type_cl_uchar;
static ID id_type_cl_short;
static ID id_type_cl_ushort;
static ID id_type_cl_int;
static ID id_type_cl_uint;
static ID id_type_cl_long;
static ID id_type_cl_ulong;
static ID id_type_cl_half;
static ID id_type_cl_float;
static ID id_type_cl_double;

static ID id_type_cl_char2;
static ID id_type_cl_char4;
static ID id_type_cl_char8;
static ID id_type_cl_char16;
static ID id_type_cl_uchar2;
static ID id_type_cl_uchar4;
static ID id_type_cl_uchar8;
static ID id_type_cl_uchar16;
static ID id_type_cl_short2;
static ID id_type_cl_short4;
static ID id_type_cl_short8;
static ID id_type_cl_short16;
static ID id_type_cl_ushort2;
static ID id_type_cl_ushort4;
static ID id_type_cl_ushort8;
static ID id_type_cl_ushort16;
static ID id_type_cl_int2;
static ID id_type_cl_int4;
static ID id_type_cl_int8;
static ID id_type_cl_int16;
static ID id_type_cl_uint2;
static ID id_type_cl_uint4;
static ID id_type_cl_uint8;
static ID id_type_cl_uint16;
static ID id_type_cl_long2;
static ID id_type_cl_long4;
static ID id_type_cl_long8;
static ID id_type_cl_long16;
static ID id_type_cl_ulong2;
static ID id_type_cl_ulong4;
static ID id_type_cl_ulong8;
static ID id_type_cl_ulong16;
static ID id_type_cl_float2;
static ID id_type_cl_float4;
static ID id_type_cl_float8;
static ID id_type_cl_float16;
static ID id_type_cl_double2;
static ID id_type_cl_double4;
static ID id_type_cl_double8;
static ID id_type_cl_double16;

#define DEF_CL_TYPE(hash, type) \
    do { \
        id_type_##type = rb_intern( #type ); \
        size_t sz = sizeof(type); \
        rb_hash_aset(hash, ID2SYM(id_type_##type), LONG2FIX(sz)); \
    } while (0)

#define DEF_CL_VECTOR_TYPE(hash, type) \
    do { \
        id_type_##type = rb_intern( #type ); \
        size_t sz = sizeof(type); \
        rb_hash_aset(hash, ID2SYM(id_type_##type), LONG2FIX(sz)); \
    } while (0)

static VALUE rcl_sym_vector_types;
static VALUE rcl_sym_scalar_types;

/*
 * call-seq:
 *      OpenCL.type_size(type_tag)
 *
 * Returns the byte size of given type, or 0 if +type_tag+ is unrecognized
 * or invalid.
 */
static VALUE rcl_sizeof(VALUE self, VALUE id)
{
    VALUE sz = Qnil;
    if (SYMBOL_P(id)) {
        VALUE scalars = rb_const_get(self, rcl_sym_scalar_types);
        assert(!NIL_P(scalars));
        sz = rb_hash_lookup(scalars, id);
        if (NIL_P(sz)) {
            VALUE vectors = rb_const_get(self, rcl_sym_vector_types);
            sz = rb_hash_lookup(vectors, id);
        }
    }
    return NIL_P(sz) ? INT2FIX(0) : sz;
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
        VALUE vectors = rb_const_get(self, rcl_sym_vector_types);
        assert(!NIL_P(vectors));
        return NIL_P(rb_hash_lookup(vectors, id)) ? Qfalse : Qtrue;
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
    return rcl_sizeof(self, id) != INT2FIX(0) ? Qtrue : Qfalse;
}

static void define_cl_types(void)
{
    VALUE rcl_types = rb_hash_new();
    VALUE rcl_vector_types = rb_hash_new();

    DEF_CL_TYPE(rcl_types, cl_bool);
    DEF_CL_TYPE(rcl_types, cl_char);
    DEF_CL_TYPE(rcl_types, cl_uchar);
    DEF_CL_TYPE(rcl_types, cl_short);
    DEF_CL_TYPE(rcl_types, cl_ushort);
    DEF_CL_TYPE(rcl_types, cl_int);
    DEF_CL_TYPE(rcl_types, cl_uint);
    DEF_CL_TYPE(rcl_types, cl_long);
    DEF_CL_TYPE(rcl_types, cl_ulong);
    DEF_CL_TYPE(rcl_types, cl_half);
    DEF_CL_TYPE(rcl_types, cl_float);
    DEF_CL_TYPE(rcl_types, cl_double);

    DEF_CL_VECTOR_TYPE(rcl_vector_types, cl_char2);
    DEF_CL_VECTOR_TYPE(rcl_vector_types, cl_char4);
    DEF_CL_VECTOR_TYPE(rcl_vector_types, cl_char8);
    DEF_CL_VECTOR_TYPE(rcl_vector_types, cl_char16);
    DEF_CL_VECTOR_TYPE(rcl_vector_types, cl_uchar2);
    DEF_CL_VECTOR_TYPE(rcl_vector_types, cl_uchar4);
    DEF_CL_VECTOR_TYPE(rcl_vector_types, cl_uchar8);
    DEF_CL_VECTOR_TYPE(rcl_vector_types, cl_uchar16);
    DEF_CL_VECTOR_TYPE(rcl_vector_types, cl_short2);
    DEF_CL_VECTOR_TYPE(rcl_vector_types, cl_short4);
    DEF_CL_VECTOR_TYPE(rcl_vector_types, cl_short8);
    DEF_CL_VECTOR_TYPE(rcl_vector_types, cl_short16);
    DEF_CL_VECTOR_TYPE(rcl_vector_types, cl_ushort2);
    DEF_CL_VECTOR_TYPE(rcl_vector_types, cl_ushort4);
    DEF_CL_VECTOR_TYPE(rcl_vector_types, cl_ushort8);
    DEF_CL_VECTOR_TYPE(rcl_vector_types, cl_ushort16);
    DEF_CL_VECTOR_TYPE(rcl_vector_types, cl_int2);
    DEF_CL_VECTOR_TYPE(rcl_vector_types, cl_int4);
    DEF_CL_VECTOR_TYPE(rcl_vector_types, cl_int8);
    DEF_CL_VECTOR_TYPE(rcl_vector_types, cl_int16);
    DEF_CL_VECTOR_TYPE(rcl_vector_types, cl_uint2);
    DEF_CL_VECTOR_TYPE(rcl_vector_types, cl_uint4);
    DEF_CL_VECTOR_TYPE(rcl_vector_types, cl_uint8);
    DEF_CL_VECTOR_TYPE(rcl_vector_types, cl_uint16);
    DEF_CL_VECTOR_TYPE(rcl_vector_types, cl_long2);
    DEF_CL_VECTOR_TYPE(rcl_vector_types, cl_long4);
    DEF_CL_VECTOR_TYPE(rcl_vector_types, cl_long8);
    DEF_CL_VECTOR_TYPE(rcl_vector_types, cl_long16);
    DEF_CL_VECTOR_TYPE(rcl_vector_types, cl_ulong2);
    DEF_CL_VECTOR_TYPE(rcl_vector_types, cl_ulong4);
    DEF_CL_VECTOR_TYPE(rcl_vector_types, cl_ulong8);
    DEF_CL_VECTOR_TYPE(rcl_vector_types, cl_ulong16);
    DEF_CL_VECTOR_TYPE(rcl_vector_types, cl_float2);
    DEF_CL_VECTOR_TYPE(rcl_vector_types, cl_float4);
    DEF_CL_VECTOR_TYPE(rcl_vector_types, cl_float8);
    DEF_CL_VECTOR_TYPE(rcl_vector_types, cl_float16);
    DEF_CL_VECTOR_TYPE(rcl_vector_types, cl_double2);
    DEF_CL_VECTOR_TYPE(rcl_vector_types, cl_double4);
    DEF_CL_VECTOR_TYPE(rcl_vector_types, cl_double8);
    DEF_CL_VECTOR_TYPE(rcl_vector_types, cl_double16);

    // prevent from change.
    OBJ_FREEZE(rcl_types);
    OBJ_FREEZE(rcl_vector_types);

    rb_define_const(rcl_mOpenCL, "SCALAR_TYPES", rcl_types);
    rb_define_const(rcl_mOpenCL, "VECTOR_TYPES", rcl_vector_types);

    // for computing the ID of symbols only once.
    rcl_sym_scalar_types = rb_intern("SCALAR_TYPES");
    rcl_sym_vector_types = rb_intern("VECTOR_TYPES");

    rb_define_module_function(rcl_mOpenCL, "type_size", rcl_sizeof, 1);
    rb_define_module_function(rcl_mOpenCL, "valid_type?", rcl_is_type_valid, 1);
    rb_define_module_function(rcl_mOpenCL, "valid_vector?", rcl_is_type_vector, 1);
}

#define IS_TYPE_VALID(id)  (rcl_is_type_valid(rcl_mOpenCL, ID2SYM(id)) == Qtrue)
#define IS_TYPE_VECTOR(id) (rcl_is_type_vector(rcl_mOpenCL, ID2SYM(id)) == Qtrue)

size_t
rcl_type_size(ID id)
{
    return FIX2UINT(rcl_sizeof(rcl_mOpenCL, ID2SYM(id)));
}

#define IF_TYPE_TO_NATIVE(c_type, expector, convertor) \
    if (type == id_type_##c_type) { \
        expector(value); \
        *(c_type *)address = (c_type)convertor(value); \
        return; \
    }

#define IF_VECTOR_TYPE_TO_NATIVE(base_c_type, n, expector, convertor) \
    if (type == id_type_##base_c_type##n) { \
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
rcl_ruby2native(ID type, void *address, VALUE value)
{
    assert(!(NIL_P(value) || NULL == address));

    if (type == id_type_cl_bool) {
        EXTRACT_BOOLEAN(value, bv);
        *(cl_bool *)address = bv;
        return;
    }
    IF_TYPE_TO_NATIVE(cl_float,  EXPECT_FLOAT,   NUM2DBL);
    IF_TYPE_TO_NATIVE(cl_ushort, EXPECT_FIXNUM,  FIX2UINT);
    IF_TYPE_TO_NATIVE(cl_uint,   EXPECT_INTEGER, NUM2UINT);
    IF_TYPE_TO_NATIVE(cl_char,   EXPECT_FIXNUM,  FIX2INT);
    IF_TYPE_TO_NATIVE(cl_uchar,  EXPECT_FIXNUM,  FIX2UINT);
    IF_TYPE_TO_NATIVE(cl_short,  EXPECT_FIXNUM,  FIX2INT);
    IF_TYPE_TO_NATIVE(cl_int,    EXPECT_INTEGER, NUM2INT);
    IF_TYPE_TO_NATIVE(cl_long,   EXPECT_INTEGER, NUM2LONG);
    IF_TYPE_TO_NATIVE(cl_ulong,  EXPECT_INTEGER, NUM2ULONG);
    IF_TYPE_TO_NATIVE(cl_half,   EXPECT_FLOAT,   ExtractHalf);
    IF_TYPE_TO_NATIVE(cl_double, EXPECT_FLOAT,   NUM2DBL);

    IF_VECTOR_TYPE_TO_NATIVE(cl_char,   2,  EXPECT_FIXNUM,  FIX2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_char,   4,  EXPECT_FIXNUM,  FIX2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_char,   8,  EXPECT_FIXNUM,  FIX2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_char,   16, EXPECT_FIXNUM,  FIX2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_uchar,  2,  EXPECT_FIXNUM,  FIX2UINT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_uchar,  4,  EXPECT_FIXNUM,  FIX2UINT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_uchar,  8,  EXPECT_FIXNUM,  FIX2UINT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_uchar,  16, EXPECT_FIXNUM,  FIX2UINT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_short,  2,  EXPECT_FIXNUM,  FIX2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_short,  4,  EXPECT_FIXNUM,  FIX2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_short,  8,  EXPECT_FIXNUM,  FIX2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_short,  16, EXPECT_FIXNUM,  FIX2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_ushort, 2,  EXPECT_FIXNUM,  FIX2UINT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_ushort, 4,  EXPECT_FIXNUM,  FIX2UINT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_ushort, 8,  EXPECT_FIXNUM,  FIX2UINT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_ushort, 16, EXPECT_FIXNUM,  FIX2UINT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_int,    2,  EXPECT_INTEGER, NUM2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_int,    4,  EXPECT_INTEGER, NUM2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_int,    8,  EXPECT_INTEGER, NUM2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_int,    16, EXPECT_INTEGER, NUM2INT);
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

    rb_raise(rb_eArgError, "invalid type tag.");
}

#define IF_TYPE_TO_RUBY(c_type, convertor) \
    if (type == id_type_##c_type) { \
        return convertor(*(c_type *)address); \
    }

#define IF_VECTOR_TYPE_TO_RUBY(base_c_type, n, convertor) \
    if (type == id_type_##base_c_type##n) { \
        VALUE ret = rb_ary_new2(n); \
        base_c_type *ptr = (base_c_type *)address; \
        for (int i = 0; i < n; i++) { \
            rb_ary_push(ret, convertor(ptr[i])); \
        } \
        return ret; \
    }

static inline VALUE
rcl_native2ruby(ID type, void *address)
{
    assert(NULL != address);

    if (type == id_type_cl_bool) {
        return (*(cl_bool *)address) ? Qtrue : Qfalse;
    }
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
    ID       type;
    int      is_wrapper;
    int      is_dirty;
} rcl_pointer_t;

static inline size_t
BytesOf(rcl_pointer_t *p)
{
    return p->size * p->type_size;
}

#define NEED_ALLOC(p)   (BytesOf(p) > sizeof(intptr_t) && !(p->is_wrapper))
#define IS_POINTER(p)   (p->alloc_address != NULL || p->is_wrapper)
#define ALLOC_SIZE_OF(p)  (BytesOf(p) + 0x80)

static inline void
AllocMemory(rcl_pointer_t *p)
{
    assert(p->size > 0 && p->type_size > 0);

    size_t alloc_sz = ALLOC_SIZE_OF(p);     // align in 128bytes.
    p->alloc_address = (int8_t *)ALLOC_N(int8_t, alloc_sz);
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
 *      HostPointer.allocate    -> a HostPointer object.
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
    p->type = id_type_cl_uint;
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
    ID clt = SYM2ID(type);
    if (!IS_TYPE_VALID(clt)) {
        rb_raise(rb_eArgError, "invalid type tag.");
    }

    EXTRACT_SIZE(size, sz);

    rcl_pointer_t *p;
    VALUE ro = Data_Make_Struct(klass, rcl_pointer_t, 0, rcl_pointer_free_func, p);

    p->alloc_address = NULL;
    p->address = addr;
    p->size = sz;
    p->type = clt;
    p->type_size = rcl_type_size(clt);
    p->is_wrapper = 1;

    assert(IS_POINTER(p));

    return ro;
}

/*
 * call-seq:
 *      HostPointer#clear   -> receiver
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
 *      HostPointer::new(type, size)    -> a HostPointer
 *
 * Allocate a managed host memory.
 */
static VALUE
rcl_pointer_init(VALUE self, VALUE type, VALUE size)
{
    rcl_pointer_t *p;
    Data_Get_Struct(self, rcl_pointer_t, p);

    if (!SYMBOL_P(type)) {
        rb_raise(rb_eTypeError, "invalid type tag, expected a Symbol.");
    }
    p->type = SYM2ID(type);
    if (!IS_TYPE_VALID(p->type)) {
        rb_raise(rb_eArgError, "unrecognized type tag.");
    }

    if (!FIXNUM_P(size) || FIX2UINT(size) < 1) {
        rb_raise(rb_eArgError, "invalid size.");
    }
    p->size = FIX2UINT(size);
    p->type_size = rcl_type_size(p->type);
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
 *      HostPointer#[0] ->  1.234
 *
 * Returns the n-th element stored in the memory region managed by
 * the receiver.
 */
static VALUE
rcl_pointer_aref(VALUE self, VALUE index)
{
    rcl_pointer_t *p = PointerPtr(self);
    EXTRACT_SIZE(index, i);

    if (p->size == 0 || i >= p->size) {
        rb_raise(rb_eRuntimeError, "subscriber exceeds the boundary.");
    }
    return rcl_native2ruby(p->type, ElementAddress(p, i));
}

/*
 * call-seq:
 *      HostPointer#[0]= 1234 ->  receiver
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
 *      HostPointer#assign_byte_string(aString, offset) -> the receiver.
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
 * Clears the dirty flag of the receiver.
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
 *      HostPointer#type ->  :cl_float16
 *
 * Returns the type of the receiver.
 */
static VALUE
rcl_pointer_type(VALUE self)
{
    return ID2SYM(PointerPtr(self)->type);
}

/*
 * call-seq:
 *      HostPointer#size    -> aInteger
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
 *      HostPointer#byte_size   -> aInteger
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
 *      HostPointer#free
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
 *      HostPointer#copy_from(ptr)   -> receiver
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
 *      HostPointer#slice(start, size)  -> a HostPointer
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

    p->type = id_type_cl_uchar;
    p->type_size = sizeof(cl_uchar);
    p->alloc_address = p->address = address;
    p->size = size;
    p->is_wrapper = 1;

    return mp;
}

// references by rcl_cq_enqueu_unmap_mem_object
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
    if (!SYMBOL_P(type)) {
        rb_raise(rb_eArgError, "expected argument 2 is a Symbol.");
    }

    ID tid = SYM2ID(type);
    if (!IS_TYPE_VALID(tid)) {
        rb_raise(rb_eArgError, "unrecognized type name.");
    }

    rcl_pointer_t *p = PointerPtr(self);
    if (p->type == tid) return self;
    if (p->size == 0) {
        rb_warn("receiver is a null pointer.");
        return self;
    }

    size_t tsz = rcl_type_size(tid);
    size_t sz = BytesOf(p);

    if (sz % tsz != 0) {
        rb_raise(rb_eRuntimeError, "casting to incompatible pointer type.");
    }
    size_t csz = sz / tsz;

    p->type = tid;
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
    if (!SYMBOL_P(type)) {
        rb_raise(rb_eArgError, "expected argument 2 is a Symbol.");
    }

    ID tid = SYM2ID(type);
    if (!IS_TYPE_VALID(tid)) {
        rb_raise(rb_eArgError, "unrecognized type name.");
    }

    rcl_pointer_t *p = PointerPtr(self);

    EXTRACT_SIZE(address, addr);
    if (addr + rcl_type_size(tid) > BytesOf(p)) {
        rb_raise(rb_eArgError, "byte index is too large.");
    }
    return rcl_native2ruby(tid, (void *)((int8_t *)p->address + addr));
}

static VALUE
rcl_mapped_pointer_write_as_type(VALUE self, VALUE address, VALUE type, VALUE value)
{
    if (!SYMBOL_P(type)) {
        rb_raise(rb_eArgError, "expected argument 2 is a Symbol.");
    }

    ID tid = SYM2ID(type);
    if (!IS_TYPE_VALID(tid)) {
        rb_raise(rb_eArgError, "unrecognized type name.");
    }

    rcl_pointer_t *p = PointerPtr(self);

    EXTRACT_SIZE(address, addr);
    if (addr + rcl_type_size(tid) > BytesOf(p)) {
        rb_raise(rb_eArgError, "byte index is too large.");
    }
    rcl_ruby2native(tid, (void *)((int8_t *)p->address + addr), value);
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
