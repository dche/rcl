

#include <math.h>
#include "capi.h"

extern VALUE rcl_mCapi;
extern VALUE rcl_cPointer;

// Macros for float composition
#define RCL_HLF_MAX         65200
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
static ID id_type_size_t;

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
    DEF_CL_TYPE(size_t);
    
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
    
    rb_obj_freeze(rcl_types);
    rb_obj_freeze(rcl_vector_types);
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
    assert(Is_Type_Valid(type));
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
    IF_TYPE_TO_NATIVE(size_t  ,  Expect_Integer, NUM2ULONG);
    IF_TYPE_TO_NATIVE(cl_half,   Expect_Float,   Extract_Half);
    IF_TYPE_TO_NATIVE(cl_float,  Expect_Float,   NUM2DBL);
    
}

#define IF_TYPE_TO_RUBY(c_type, convertor) \
    if (type == id_type_##c_type) { \
        return convertor(*(c_type *)address); \
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
    assert(Is_Type_Valid(type));
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
    IF_TYPE_TO_RUBY(size_t,     ULONG2NUM);
    IF_TYPE_TO_RUBY(cl_half,    rcl_half_float_new);
    IF_TYPE_TO_RUBY(cl_float,   rb_float_new);
    
    return Qnil;
}

/*
 * class Pointer
 */

typedef struct {
    
    int8_t  *alloc_address;
    void    *address;
    size_t   size;              // in number of elements.
    size_t   type_size;
    ID       type;
    
} rcl_pointer_t;

static inline rcl_pointer_t *
Pointer_Ptr(VALUE ptr)
{
    Expect_RCL_Type(ptr, Pointer);
    
    rcl_pointer_t *p;
    Data_Get_Struct(ptr, rcl_pointer_t, p);
    
    return p;
}

static inline int
Is_Immediate_Value(rcl_pointer_t *p)
{
    return (!Is_Type_Vector(p->type)) && (p->size == 1);
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
    if (Is_Immediate_Value(ptr)) {
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
    return p->size * p->type_size;
}

static void
rcl_pointer_free_func(void *ptr)
{
    free(((rcl_pointer_t *)ptr)->alloc_address);
    free(ptr);
}

static VALUE
rcl_pointer_alloc(VALUE klass)
{
    rcl_pointer_t *p;
    VALUE ro = Data_Make_Struct(klass, rcl_pointer_t, 0, rcl_pointer_free_func, p);
    
    p->alloc_address = p->address = NULL;
    p->type = id_type_cl_uint;
    p->type_size = sizeof(cl_uint);
    return ro;
}

static VALUE
rcl_pointer_clear(VALUE self)
{
    rcl_pointer_t *p = Pointer_Ptr(self);
    if (p->size > 0) {
        if (Is_Immediate_Value(p)) {
            p->address = 0;
        } else {
            bzero(p->address, p->size * p->type_size);
        }
    }
    return self;
}

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

    if (!Is_Immediate_Value(p)) {    
        // Allocate memory.
        size_t alloc_sz = p->size * p->type_size;
        if (p->size > 1) {
            alloc_sz += 0x80;     // align in 128bytes.
        }
        p->alloc_address = (int8_t *)ALLOC_N(int8_t, alloc_sz);
        if (p->alloc_address == NULL) {
            rb_raise(rb_eRuntimeError, "Out of host memory.");
        }
        p->address = (void *)(((intptr_t)(p->alloc_address) + 0x80) & ~0x7F);
    }
    return rcl_pointer_clear(self);
}

static VALUE
rcl_pointer_init_copy(VALUE copy, VALUE orig)
{
    Expect_RCL_Type(orig, Pointer);
    
    rcl_pointer_t *copy_p, *orig_p;
    Data_Get_Struct(copy, rcl_pointer_t, copy_p);
    Data_Get_Struct(orig, rcl_pointer_t, orig_p);
    
    assert(copy_p->address != orig_p->address);
}

/*
 * call-seq:
 *      Pointer#[0] ->  1.234 
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

static VALUE
rcl_pointer_aset(VALUE self, VALUE index, VALUE value)
{
    rcl_pointer_t *p = Pointer_Ptr(self);
    Extract_Size(index, i);
    
    if (p->size == 0 || i >= p->size) {
        rb_raise(rb_eRuntimeError, "Subscriber exceeds the boundary.");
    }
    Ruby2Native(p->type, Element_Address(p, i), value);
    return self;
}

static VALUE
rcl_pointer_address(VALUE self)
{    
    cl_ulong addr = (cl_ulong)Pointer_Address(self);
    return addr == 0 ? Qnil : LONG2FIX(addr);
}

static VALUE
rcl_pointer_type(VALUE self)
{    
    return ID2SYM(Pointer_Ptr(self)->type);
}

static VALUE
rcl_pointer_size(VALUE self)
{    
    return LONG2FIX(Pointer_Ptr(self)->size);
}

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

static VALUE
rcl_pointer_copy(VALUE self, VALUE dst)
{
    Expect_RCL_Type(dst, Pointer);
    
    return self;
}

static VALUE
rcl_pointer_ncopy(VALUE self, VALUE src, VALUE size)
{
    return self;
}

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
    rb_define_method(rcl_cPointer, "free", rcl_pointer_free, 0);
    rb_define_method(rcl_cPointer, "clear", rcl_pointer_clear, 0);
    rb_define_method(rcl_cPointer, "copy", rcl_pointer_copy, 1);
    rb_define_method(rcl_cPointer, "ncopy", rcl_pointer_ncopy, 2);
}
