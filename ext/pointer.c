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

// Storages for types.
static VALUE rcl_types;         // { type tag => type size }
static VALUE rcl_vector_types;

#define DEF_CL_TYPE(type) \
    do { \
        id_type_##type = rb_intern( #type ); \
        size_t sz = sizeof(type); \
        rb_hash_aset(rcl_types, ID2SYM(id_type_##type), LONG2FIX(sz)); \
    } while (0)

#define DEF_CL_VECTOR_TYPE(type) \
    do { \
        id_type_##type = rb_intern( #type ); \
        size_t sz = sizeof(type); \
        rb_hash_aset(rcl_vector_types, ID2SYM(id_type_##type), LONG2FIX(sz)); \
    } while (0)

static void define_cl_types(void)
{
    rcl_types = rb_hash_new();
    rcl_vector_types = rb_hash_new();

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

    DEF_CL_VECTOR_TYPE(cl_char2);
    DEF_CL_VECTOR_TYPE(cl_char4);
    DEF_CL_VECTOR_TYPE(cl_char8);
    DEF_CL_VECTOR_TYPE(cl_char16);
    DEF_CL_VECTOR_TYPE(cl_uchar2);
    DEF_CL_VECTOR_TYPE(cl_uchar4);
    DEF_CL_VECTOR_TYPE(cl_uchar8);
    DEF_CL_VECTOR_TYPE(cl_uchar16);
    DEF_CL_VECTOR_TYPE(cl_short2);
    DEF_CL_VECTOR_TYPE(cl_short4);
    DEF_CL_VECTOR_TYPE(cl_short8);
    DEF_CL_VECTOR_TYPE(cl_short16);
    DEF_CL_VECTOR_TYPE(cl_ushort2);
    DEF_CL_VECTOR_TYPE(cl_ushort4);
    DEF_CL_VECTOR_TYPE(cl_ushort8);
    DEF_CL_VECTOR_TYPE(cl_ushort16);
    DEF_CL_VECTOR_TYPE(cl_int2);
    DEF_CL_VECTOR_TYPE(cl_int4);
    DEF_CL_VECTOR_TYPE(cl_int8);
    DEF_CL_VECTOR_TYPE(cl_int16);
    DEF_CL_VECTOR_TYPE(cl_uint2);
    DEF_CL_VECTOR_TYPE(cl_uint4);
    DEF_CL_VECTOR_TYPE(cl_uint8);
    DEF_CL_VECTOR_TYPE(cl_uint16);
    DEF_CL_VECTOR_TYPE(cl_long2);
    DEF_CL_VECTOR_TYPE(cl_long4);
    DEF_CL_VECTOR_TYPE(cl_long8);
    DEF_CL_VECTOR_TYPE(cl_long16);
    DEF_CL_VECTOR_TYPE(cl_ulong2);
    DEF_CL_VECTOR_TYPE(cl_ulong4);
    DEF_CL_VECTOR_TYPE(cl_ulong8);
    DEF_CL_VECTOR_TYPE(cl_ulong16);
    DEF_CL_VECTOR_TYPE(cl_float2);
    DEF_CL_VECTOR_TYPE(cl_float4);
    DEF_CL_VECTOR_TYPE(cl_float8);
    DEF_CL_VECTOR_TYPE(cl_float16);
    DEF_CL_VECTOR_TYPE(cl_double2);
    DEF_CL_VECTOR_TYPE(cl_double4);
    DEF_CL_VECTOR_TYPE(cl_double8);
    DEF_CL_VECTOR_TYPE(cl_double16);

    // Prevent from change.
    OBJ_FREEZE(rcl_types);
    OBJ_FREEZE(rcl_vector_types);

    rb_define_const(rcl_mOpenCL, "SCALAR_TYPES", rcl_types);
    rb_define_const(rcl_mOpenCL, "VECTOR_TYPES", rcl_vector_types);
}

#define Is_Type_Valid(id)  (!NIL_P(rb_hash_lookup(rcl_types, ID2SYM(id))) || !NIL_P(rb_hash_lookup(rcl_vector_types, ID2SYM(id))))
#define Is_Type_Vector(id) (!NIL_P(rb_hash_lookup(rcl_vector_types, ID2SYM(id))))

size_t
rcl_type_size(ID id)
{
    VALUE sym = ID2SYM(id);
    VALUE sz = rb_hash_lookup(rcl_types, sym);

    if (NIL_P(sz)) {
        sz = rb_hash_lookup(rcl_vector_types, sym);
    }
    assert(FIXNUM_P(sz));
    return FIX2UINT(sz);
}

#define IF_TYPE_TO_NATIVE(c_type, expector, convertor) \
    if (type == id_type_##c_type) { \
        expector(value); \
        *(c_type *)address = (c_type)convertor(value); \
        return; \
    }

#define IF_VECTOR_TYPE_TO_NATIVE(base_c_type, n, expector, convertor) \
    if (type == id_type_##base_c_type##n) { \
        Expect_NonEmpty_Array(value); \
        if (n > RARRAY_LEN(value)) { \
            rb_raise(rb_eArgError, "Expected number of elements is %d, but got %ld", n, RARRAY_LEN(value)); \
        } \
        base_c_type *ptr = (base_c_type *)address; \
        for (int i = 0; i < n; i++) { \
            VALUE v = rb_ary_entry(value, i); \
            expector(v); \
            ptr[i] = (base_c_type)convertor(v); \
        } \
        return; \
    }

// Referenced by kernel_set_arg_with_type() in capi.c
void 
rcl_ruby2native(ID type, void *address, VALUE value)
{
    assert(!(NIL_P(value) || NULL == address));

    if (type == id_type_cl_bool) {
        Expect_Boolean(value, bv);
        *(cl_bool *)address = bv;
        return;
    }
    IF_TYPE_TO_NATIVE(cl_char,   Expect_Fixnum,  FIX2INT);
    IF_TYPE_TO_NATIVE(cl_uchar,  Expect_Fixnum,  FIX2UINT);
    IF_TYPE_TO_NATIVE(cl_short,  Expect_Fixnum,  FIX2INT);
    IF_TYPE_TO_NATIVE(cl_ushort, Expect_Fixnum,  FIX2UINT);
    IF_TYPE_TO_NATIVE(cl_int,    Expect_Integer, NUM2INT);
    IF_TYPE_TO_NATIVE(cl_uint,   Expect_Integer, NUM2UINT);
    IF_TYPE_TO_NATIVE(cl_long,   Expect_Integer, NUM2LONG);
    IF_TYPE_TO_NATIVE(cl_ulong,  Expect_Integer, NUM2ULONG);
    IF_TYPE_TO_NATIVE(cl_half,   Expect_Float,   Extract_Half);
    IF_TYPE_TO_NATIVE(cl_float,  Expect_Float,   NUM2DBL);
    IF_TYPE_TO_NATIVE(cl_double, Expect_Float,   NUM2DBL);

    IF_VECTOR_TYPE_TO_NATIVE(cl_char,   2,  Expect_Fixnum,  FIX2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_char,   4,  Expect_Fixnum,  FIX2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_char,   8,  Expect_Fixnum,  FIX2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_char,   16, Expect_Fixnum,  FIX2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_uchar,  2,  Expect_Fixnum,  FIX2UINT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_uchar,  4,  Expect_Fixnum,  FIX2UINT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_uchar,  8,  Expect_Fixnum,  FIX2UINT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_uchar,  16, Expect_Fixnum,  FIX2UINT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_short,  2,  Expect_Fixnum,  FIX2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_short,  4,  Expect_Fixnum,  FIX2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_short,  8,  Expect_Fixnum,  FIX2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_short,  16, Expect_Fixnum,  FIX2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_ushort, 2,  Expect_Fixnum,  FIX2UINT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_ushort, 4,  Expect_Fixnum,  FIX2UINT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_ushort, 8,  Expect_Fixnum,  FIX2UINT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_ushort, 16, Expect_Fixnum,  FIX2UINT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_int,    2,  Expect_Integer, NUM2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_int,    4,  Expect_Integer, NUM2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_int,    8,  Expect_Integer, NUM2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_int,    16, Expect_Integer, NUM2INT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_uint,   2,  Expect_Integer, NUM2UINT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_uint,   4,  Expect_Integer, NUM2UINT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_uint,   8,  Expect_Integer, NUM2UINT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_uint,   16, Expect_Integer, NUM2UINT);
    IF_VECTOR_TYPE_TO_NATIVE(cl_long,   2,  Expect_Integer, NUM2LONG);
    IF_VECTOR_TYPE_TO_NATIVE(cl_long,   4,  Expect_Integer, NUM2LONG);
    IF_VECTOR_TYPE_TO_NATIVE(cl_long,   8,  Expect_Integer, NUM2LONG);
    IF_VECTOR_TYPE_TO_NATIVE(cl_long,   16, Expect_Integer, NUM2LONG);
    IF_VECTOR_TYPE_TO_NATIVE(cl_ulong,  2,  Expect_Integer, NUM2ULONG);
    IF_VECTOR_TYPE_TO_NATIVE(cl_ulong,  4,  Expect_Integer, NUM2ULONG);
    IF_VECTOR_TYPE_TO_NATIVE(cl_ulong,  8,  Expect_Integer, NUM2ULONG);
    IF_VECTOR_TYPE_TO_NATIVE(cl_ulong,  16, Expect_Integer, NUM2ULONG);
    IF_VECTOR_TYPE_TO_NATIVE(cl_float,  2,  Expect_Float,   NUM2DBL);
    IF_VECTOR_TYPE_TO_NATIVE(cl_float,  4,  Expect_Float,   NUM2DBL);
    IF_VECTOR_TYPE_TO_NATIVE(cl_float,  8,  Expect_Float,   NUM2DBL);
    IF_VECTOR_TYPE_TO_NATIVE(cl_float,  16, Expect_Float,   NUM2DBL);
    IF_VECTOR_TYPE_TO_NATIVE(cl_double, 2,  Expect_Float,   NUM2DBL);
    IF_VECTOR_TYPE_TO_NATIVE(cl_double, 4,  Expect_Float,   NUM2DBL);
    IF_VECTOR_TYPE_TO_NATIVE(cl_double, 8,  Expect_Float,   NUM2DBL);
    IF_VECTOR_TYPE_TO_NATIVE(cl_double, 16, Expect_Float,   NUM2DBL);

    rb_raise(rb_eArgError, "Invalid type tag.");
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
Native2Ruby(ID type, void *address)
{
    assert(NULL != address);

    if (type == id_type_cl_bool) {
        return (*(cl_bool *)address) ? Qtrue : Qfalse;
    }
    IF_TYPE_TO_RUBY(cl_char,    INT2FIX);
    IF_TYPE_TO_RUBY(cl_uchar,   UINT2NUM);
    IF_TYPE_TO_RUBY(cl_short,   INT2FIX);
    IF_TYPE_TO_RUBY(cl_ushort,  UINT2NUM);
    IF_TYPE_TO_RUBY(cl_int,     INT2NUM);
    IF_TYPE_TO_RUBY(cl_uint,    UINT2NUM);
    IF_TYPE_TO_RUBY(cl_long,    LONG2NUM);
    IF_TYPE_TO_RUBY(cl_ulong,   ULONG2NUM);
    IF_TYPE_TO_RUBY(cl_half,    rcl_half_float_new);
    IF_TYPE_TO_RUBY(cl_float,   rb_float_new);
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
} rcl_pointer_t;

#define BytesOf(p)      (p->size * p->type_size)
#define Need_Alloc(p)   (BytesOf(p) > sizeof(intptr_t) && !(p->is_wrapper))
#define Is_Pointer(p)   (p->alloc_address != NULL || p->is_wrapper)
#define AllocSizeOf(p)  (BytesOf(p) + 0x80)

static inline void
Alloc_Memory(rcl_pointer_t *p)
{
    assert(p->size > 0 && p->type_size > 0);

    size_t alloc_sz = AllocSizeOf(p);     // align in 128bytes.
    p->alloc_address = (int8_t *)ALLOC_N(int8_t, alloc_sz);
    bzero(p->alloc_address, alloc_sz);

    if (p->alloc_address == NULL) {
        rb_raise(rb_eRuntimeError, "Out of host memory.");
    }
    p->address = (void *)(((intptr_t)(p->alloc_address) + 0x80) & ~0x7F);
}

static inline rcl_pointer_t *
Pointer_Ptr(VALUE ptr)
{
    rcl_pointer_t *p;
    Data_Get_Struct(ptr, rcl_pointer_t, p);

    return p;
}

// Used by Kernel#set_arg, so not local.
void *
Pointer_Address(VALUE ptr)
{
    rcl_pointer_t *p = Pointer_Ptr(ptr);

    if (!Is_Pointer(p)) {
        return p->size == 0 ? NULL : &(p->address);
    } else {
        return p->address;        
    }
}

static inline void *
Element_Address(rcl_pointer_t *ptr, size_t index)
{
    assert(index < ptr->size);
    if (!Is_Pointer(ptr)) {
        return (void *)((int8_t *)&(ptr->address) + index * ptr->type_size);
    } else {
        return (void *)((int8_t *)(ptr->address) + index * ptr->type_size);
    }
}

size_t
Pointer_Size(VALUE ptr)
{
    rcl_pointer_t *p = Pointer_Ptr(ptr);
    return BytesOf(p);
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
 * Wraps a C pointer instead of allocating new memory.
 *
 * +wrap()+ is a simple but dangerous optimization for
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
    if (!Is_Type_Valid(clt)) {
        rb_raise(rb_eArgError, "Invalid type tag.");
    }

    Extract_Size(size, sz);

    rcl_pointer_t *p;
    VALUE ro = Data_Make_Struct(klass, rcl_pointer_t, 0, rcl_pointer_free_func, p);

    p->alloc_address = NULL;
    p->address = addr;
    p->size = sz;
    p->type = clt;
    p->type_size = rcl_type_size(clt);
    p->is_wrapper = 1;

    assert(Is_Pointer(p));

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
    rcl_pointer_t *p = Pointer_Ptr(self);
    if (p->size > 0) {
        if (!Is_Pointer(p)) {
            p->address = 0;
        } else {
            bzero(p->alloc_address, AllocSizeOf(p));
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
        rb_raise(rb_eTypeError, "Invalid type tag, Expected a Symbol.");
    }
    p->type = SYM2ID(type);
    if (!Is_Type_Valid(p->type)) {
        rb_raise(rb_eArgError, "Unrecognized type tag.");
    }

    if (!FIXNUM_P(size) || FIX2UINT(size) < 1) {
        rb_raise(rb_eArgError, "Invalid size.");
    }
    p->size = FIX2UINT(size);
    p->type_size = rcl_type_size(p->type);

    if (Need_Alloc(p)) {
        Alloc_Memory(p);
    }
    return rcl_pointer_clear(self);
}

static VALUE
rcl_pointer_init_copy(VALUE copy, VALUE orig)
{
    Expect_RCL_Type(orig, Pointer);

    rcl_pointer_t *copy_p = Pointer_Ptr(copy);
    rcl_pointer_t *orig_p = Pointer_Ptr(orig);

    if (orig_p->size == 0) {
        assert(orig_p->alloc_address == NULL && orig_p->address == NULL);
        rb_raise(rb_eRuntimeError, "Can't clone a null pointer.");
    }

    assert(copy_p->alloc_address == NULL);
    copy_p->type = orig_p->type;
    copy_p->size = orig_p->size;
    copy_p->type_size = orig_p->type_size;

    if (!Is_Pointer(orig_p)) {
        assert(!Need_Alloc(copy_p));
        copy_p->address = orig_p->address;
    } else {
        Alloc_Memory(copy_p);
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
    rcl_pointer_t *p = Pointer_Ptr(self);
    Extract_Size(index, i);

    if (p->size == 0 || i > p->size - 1) {
        rb_raise(rb_eRuntimeError, "Subscriber exceeds the boundary.");
    }
    return Native2Ruby(p->type, Element_Address(p, i));
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
    rcl_pointer_t *p = Pointer_Ptr(self);
    Extract_Size(index, i);
    if (NIL_P(value)) {
        rb_raise(rb_eArgError, "Value can't be nil.");
    }

    if (i >= p->size) {
        rb_raise(rb_eRuntimeError, "Subscriber exceeds the boundary.");
    }
    rcl_ruby2native(p->type, Element_Address(p, i), value);
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

    rcl_pointer_t *p = Pointer_Ptr(self);

    Extract_Size(size, sz);
    Extract_Size(offset, os);
    if (sz + os > p->size) {
        rb_raise(rb_eArgError, "size or offset is too large.");
    }

    size_t cpysz = sz * p->type_size;
    memcpy(Element_Address(p, os), addr, cpysz);

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

    rcl_pointer_t *p = Pointer_Ptr(self);
    Extract_Size(offset, os);
    if (os >= p->size) {
        rb_raise(rb_eArgError, "Offset exceeds the boundary.");
    }
    
    void *ptr = RSTRING_PTR(value);
    size_t sz = RSTRING_LEN(value);
    if (sz % p->type_size != 0) {
        rb_raise(rb_eArgError, "Size of byte string does not match the data type of receiver.");
    }

    size_t bos = os * p->type_size;
    size_t cpysz = (BytesOf(p) - bos) > sz ? sz : (BytesOf(p) - bos);
    memcpy(Element_Address(p, os), ptr, cpysz);
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
    intptr_t addr = (intptr_t)Pointer_Address(self);
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
    return ID2SYM(Pointer_Ptr(self)->type);
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
    return LONG2FIX(Pointer_Ptr(self)->size);
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
    return ULONG2NUM(Pointer_Size(self));
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
    rcl_pointer_t *ptr = Pointer_Ptr(self);
    if (!Is_Pointer(ptr)) {
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
    Expect_RCL_Type(src, Pointer);

    rcl_pointer_t *p = Pointer_Ptr(self);
    rcl_pointer_t *sp = Pointer_Ptr(src);

    if (p->type != sp->type || p->size != sp->size) {
        rb_raise(rb_eRuntimeError, "Size or type of source and target mismatch.");
    }
    if (!Is_Pointer(p)) {
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
    Extract_Size(start, st);
    Extract_Size(size, sz);

    rcl_pointer_t *p = Pointer_Ptr(self);

    if (p->size == 0) return Qnil;
    if (st + sz > p->size) return Qnil;

    VALUE ro = rcl_pointer_alloc(rcl_cPointer);
    assert(CLASS_OF(ro) == rcl_cPointer);
    rcl_pointer_t *hp = Pointer_Ptr(ro);
    assert(hp->alloc_address == NULL);

    hp->type = p->type;
    hp->size = sz;
    hp->type_size = p->type_size;

    if (Need_Alloc(hp)) {
        Alloc_Memory(hp);
    }
    memcpy(Element_Address(hp, 0), Element_Address(p, st), BytesOf(hp));

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
    rcl_pointer_t *p = Pointer_Ptr(ptr);
    p->alloc_address = p->address = NULL;
    p->size = 0;
}

/*
 * call-seq:
 *      MappedPointer#cast_to(:cl_uint4)    -> receiver
 */
static VALUE
rcl_mapped_pointer_coerce(VALUE self, VALUE type)
{
    if (!SYMBOL_P(type)) {
        rb_raise(rb_eArgError, "Expected argument 2 is a Symbol.");
    }

    ID tid = SYM2ID(type);
    if (!Is_Type_Valid(tid)) {
        rb_raise(rb_eArgError, "Unrecognized type name.");
    }

    rcl_pointer_t *p = Pointer_Ptr(self);
    if (p->type == tid) return self;
    if (p->size == 0) {
        rb_warn("Receiver is a null pointer.");
        return self;
    }

    size_t tsz = rcl_type_size(tid);
    size_t sz = BytesOf(p);

    if (sz % tsz != 0) {
        rb_raise(rb_eRuntimeError, "Casting to incompatible pointer type.");
    }
    size_t csz = sz / tsz;

    p->type = tid;
    p->size = csz;
    p->type_size = tsz;

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
    rb_define_method(rcl_cMappedPointer, "assign_pointer", rcl_pointer_assign, 3);
    rb_define_method(rcl_cMappedPointer, "assign_byte_string", rcl_pointer_assign_byte_string, 2);
    rb_define_method(rcl_cMappedPointer, "address", rcl_pointer_address, 0);
    rb_define_method(rcl_cMappedPointer, "type", rcl_pointer_type, 0);
    rb_define_method(rcl_cMappedPointer, "size", rcl_pointer_size, 0);
    rb_define_method(rcl_cMappedPointer, "byte_size", rcl_pointer_byte_size, 0);
    rb_define_method(rcl_cMappedPointer, "clear", rcl_pointer_clear, 0);
    rb_define_method(rcl_cMappedPointer, "cast_to", rcl_mapped_pointer_coerce, 1);
}
