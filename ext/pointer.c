// Copyright (c) 2010, Diego che

#include <math.h>
#include "capi.h"

extern VALUE rcl_mCapi;
VALUE rcl_cPointer;
VALUE rcl_cMappedPointer;

// Macros for float composition
#define RCL_HALF_BIAS       15
#define RCL_HALF_MANT_DIG   11
#define RCL_DOUBLE_BIAS     (CL_DBL_MAX_EXP - 1)

#define Expect_Integer(ro) \
    do { \
        if (!FIXNUM_P(ro) && TYPE(ro) != T_BIGNUM) { \
            rb_raise(rb_eTypeError, "Expected %s is an Integer.", #ro); \
        } \
    } while (0)

#define Expect_Float(ro) \
    do { \
        if (TYPE(ro) != T_FLOAT && !FIXNUM_P(ro) && TYPE(ro) != T_BIGNUM) { \
            rb_raise(rb_eTypeError, "Expected %s is of type Float.", #ro); \
        } \
    } while (0)

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
    rb_obj_freeze(rcl_types);
    rb_obj_freeze(rcl_vector_types);
    // Prevent from being GC.
    rb_gc_register_address(&rcl_types);
    rb_gc_register_address(&rcl_vector_types);
}

#define Is_Type_Valid(id)  (!NIL_P(rb_hash_lookup(rcl_types, ID2SYM(id))) || !NIL_P(rb_hash_lookup(rcl_vector_types, ID2SYM(id))))
#define Is_Type_Vector(id) (!NIL_P(rb_hash_lookup(rcl_vector_types, ID2SYM(id))))

static inline size_t
Type_Size(ID id)
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
            ptr[n - i + 1] = (base_c_type)convertor(v); \
        } \
    }
    
static inline cl_half Extract_Half(VALUE ro)
{
    Expect_Float(ro);

    uint64_t v = 0;
    *(double *)&v = NUM2DBL(ro);
    
    cl_half h = (v >> 48);
    uint16_t expo = ((v >> (CL_DBL_MANT_DIG - 1)) & 0x7FF);
    uint16_t frac = v >> (CL_DBL_MANT_DIG - RCL_HALF_MANT_DIG);

    assert(expo < 0x1F);
    h += (expo << (RCL_HALF_MANT_DIG - 1)) + frac;
    
    TRACE("The binary form of %f is 0x%x\n", *(double *)&v, h);
    return h;
}

static inline void Ruby2Native(ID type, void *address, VALUE value)
{
    assert(!(NIL_P(value) || NULL == address));
    
    if (type == id_type_cl_bool) {
        Expect_Boolean(value);
        *(cl_bool *)address = value ? 1 : 0;
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
            rb_ary_push(ret, convertor(ptr[n - i + 1])); \
        } \
        return ret; \
    }
    
static inline VALUE rcl_half_float_new(cl_half hf)
{
    uint64_t expo = (hf >> 10) & 0x1F;    
    if (expo == 0x1F) return rb_float_new(nanf(NULL));
    
    uint64_t sign = hf & 0x8000 >> 15;
    uint64_t frac = hf & 0x3FF;
    
    // real exponent number.
    expo = (expo == 0) ? (1 - RCL_HALF_BIAS) : (expo - RCL_HALF_BIAS);
    expo -= RCL_DOUBLE_BIAS;
    
    uint64_t f = (sign << 63) + (expo << (CL_DBL_MANT_DIG - 1)) + (frac << (CL_DBL_MANT_DIG - RCL_HALF_MANT_DIG));
    return rb_float_new(*(double *)&f);
}

static inline VALUE Native2Ruby(ID type, void *address)
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
    void    *address;           // FIXME: can't hold double in 32-bit machine.
    size_t   size;              // in number of elements, not in byte.
    size_t   type_size;
    ID       type;
    
} rcl_pointer_t;

#define BytesOf(p)    (p->size * p->type_size)

static inline void
Alloc_Memory(rcl_pointer_t *p)
{
    assert(p->size > 0 && p->type_size > 0);

    size_t alloc_sz = BytesOf(p) + 0x80;     // align in 128bytes.
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
    
    if (p->alloc_address == NULL) {
        return p->size == 0 ? NULL : &(p->address);
    } else {
        return p->address;        
    }
}

static inline void *
Element_Address(rcl_pointer_t *ptr, size_t index)
{
    if (ptr->alloc_address == NULL) {
        assert(index == 0);
        return &(ptr->address);
    } else {
        assert(index < ptr->size);
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
    free(((rcl_pointer_t *)ptr)->alloc_address);
    free(ptr);
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
    return ro;
}

/*
 * call-seq:
 *      HostPointer#clear   -> receiver
 * 
 * Set the memory the receiver manages to zero.
 */
static VALUE
rcl_pointer_clear(VALUE self)
{
    rcl_pointer_t *p = Pointer_Ptr(self);
    if (p->size > 0) {
        if (p->alloc_address == NULL) {
            p->address = 0;
        } else {
            bzero(p->address, BytesOf(p));
        }
    }
    return self;
}

/*
 * call-seq:
 *      HostPointer::new(type, size)    -> a HostPointer
 * 
 * Returns the n-th element stored in the memory region managed by 
 * the receiver.
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
    p->type_size = Type_Size(p->type);

    if (Is_Type_Vector(p->type) || p->size > 1) {    
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

    if (orig_p->alloc_address == NULL) {
        assert(copy_p->size == 1);
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
    
    if (i >= p->size) {
        rb_raise(rb_eRuntimeError, "Subscriber exceeds the boundary.");
    }
    Ruby2Native(p->type, Element_Address(p, i), value);
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
 * Free the memory the receiver manages.
 * After +free()+, the receiver's address is set to +nil+, and size is set
 * to 0.
 */
static VALUE
rcl_pointer_free(VALUE self)
{
    rcl_pointer_t *ptr = Pointer_Ptr(self);
    if (ptr->alloc_address == NULL && ptr->size == 0) {
        return self;
    }   

    free(ptr->alloc_address); 
    ptr->alloc_address = NULL;
    ptr->address = NULL;
    ptr->size = 0;
    
    return self;
}

/*
 * call-seq:
 *      HostPointer#copy_from(ptr)   -> receiver
 * 
 * Returns the n-th element stored in the memory region managed by 
 * the receiver.
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
    if (p->size == 1) {
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
    
    if (p->size == 1) {
        assert(p->alloc_address == NULL);
        hp->address = p->address;
    } else {
        Alloc_Memory(hp);
        memcpy(hp->address, (const void *)((int8_t *)p->address + st * p->type_size), BytesOf(hp));
    }
    
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
    
    return mp;
}

void
rcl_invalidate_mapped_pointer(VALUE ptr)
{
    rcl_pointer_t *p = Pointer_Ptr(ptr);
    p->alloc_address = p->address = NULL;
    p->size = 0;
}

static VALUE
rcl_mapped_pointer_alloc(VALUE klass)
{
    rb_raise(rb_eRuntimeError, "Can't instantiate a mapped pointer.");
    return Qnil;
}

static VALUE
rcl_mapped_pointer_init_copy(VALUE copy, VALUE orig)
{
    rb_raise(rb_eRuntimeError, "Can't clone mapped pointer.");
    return Qnil;
}

/*
 * Exports.
 */

void
define_rcl_class_pointer(void)
{
    define_cl_types();
    
    rcl_cPointer = rb_define_class_under(rcl_mCapi, "HostPointer", rb_cObject);
    rb_define_alloc_func(rcl_cPointer, rcl_pointer_alloc);
    rb_define_method(rcl_cPointer, "initialize", rcl_pointer_init, 2);
    rb_define_method(rcl_cPointer, "initialize_copy", rcl_pointer_init_copy, 1);
    rb_define_method(rcl_cPointer, "[]", rcl_pointer_aref, 1);
    rb_define_method(rcl_cPointer, "[]=", rcl_pointer_aset, 2);
    rb_define_method(rcl_cPointer, "address", rcl_pointer_address, 0);
    rb_define_method(rcl_cPointer, "type", rcl_pointer_type, 0);
    rb_define_method(rcl_cPointer, "size", rcl_pointer_size, 0);
    rb_define_method(rcl_cPointer, "byte_size", rcl_pointer_byte_size, 0);
    rb_define_method(rcl_cPointer, "free", rcl_pointer_free, 0);
    rb_define_method(rcl_cPointer, "clear", rcl_pointer_clear, 0);
    rb_define_method(rcl_cPointer, "copy_from", rcl_pointer_copy_from, 1);
    rb_define_method(rcl_cPointer, "slice", rcl_pointer_slice, 2);
    
    rcl_cMappedPointer = rb_define_class_under(rcl_mCapi, "MappedPointer", rb_cObject);
    rb_define_alloc_func(rcl_cMappedPointer, rcl_mapped_pointer_alloc);
    rb_define_method(rcl_cMappedPointer, "initialize_copy", rcl_mapped_pointer_init_copy, 1);
    rb_define_method(rcl_cMappedPointer, "[]", rcl_pointer_aref, 1);
    rb_define_method(rcl_cMappedPointer, "[]=", rcl_pointer_aset, 2);
    rb_define_method(rcl_cMappedPointer, "address", rcl_pointer_address, 0);
    rb_define_method(rcl_cMappedPointer, "type", rcl_pointer_type, 0);
    rb_define_method(rcl_cMappedPointer, "size", rcl_pointer_size, 0);
    rb_define_method(rcl_cMappedPointer, "byte_size", rcl_pointer_byte_size, 0);
    rb_define_method(rcl_cMappedPointer, "clear", rcl_pointer_clear, 0);
}
