
#ifndef RCL_TYPE_H__
#define RCL_TYPE_H__

extern VALUE rcl_cPlatform;
extern VALUE rcl_cDevice;
extern VALUE rcl_cContext;
extern VALUE rcl_cImageFormat;
extern VALUE rcl_cCommandQueue;
extern VALUE rcl_cSampler;
extern VALUE rcl_cEvent;
extern VALUE rcl_cMemory;
extern VALUE rcl_cProgram;
extern VALUE rcl_cKernel;

// Support classes
extern VALUE rcl_cPointer;
extern VALUE rcl_cMappedPointer;

extern void * PointerAddress(VALUE);
extern size_t PointerSize(VALUE);

extern VALUE rcl_create_mapped_pointer(void *, size_t);
extern void rcl_invalidate_mapped_pointer(VALUE);

#define EXPECT_RCL_CONST(ro) \
    do { \
        if (!FIXNUM_P(ro)) \
            rb_raise(rb_eTypeError, \
                    "expected %s an OpenCL enumerated constant. ", #ro); \
    } while (0)

#define EXTRACT_CL_POINTERS(ra, klass, c_type, svar) \
    do { \
        EXPECT_ARRAY(ra); \
        size_t num = RARRAY_LEN(ra); \
        if (num == 0) { \
            svar = NULL; \
        } else { \
            svar = ALLOCA_N(c_type, num); \
            for (cl_uint i = 0; i < num; i++) { \
                VALUE ro = rb_ary_entry(ra, i); \
                EXPECT_RCL_TYPE(ro, klass); \
                svar[i] = klass##Ptr(ro); \
            } \
        } \
    } while (0)

#define EXTRACT_MEM_OBJECT(mem, var) \
    cl_mem var; \
    do { \
        EXPECT_RCL_TYPE(mem, Memory); \
        var = MemoryPtr(mem); \
    } while (0)

#define EXTRACT_WAIT_FOR_EVENTS(events, num_evt, pevts) \
    cl_uint num_evt; \
    cl_event *pevts; \
    do { \
        if (NIL_P(events)) { \
            num_evt = 0; \
            pevts = NULL; \
        } else { \
            EXPECT_ARRAY(events); \
            num_evt = (cl_uint)RARRAY_LEN(events); \
            EXTRACT_CL_POINTERS(events, Event, cl_event, pevts); \
        } \
    } while (0)

#define EXTRACT_POINTER(ptr, var) \
    void *var; \
    do { \
        if (!NIL_P(ptr)) { \
            Check_Type(ptr, T_DATA); \
            if (rb_class_of(ptr) != rcl_cPointer && rb_class_of(ptr) != rcl_cMappedPointer) { \
                rb_raise(rb_eTypeError, "expected %s is an instance of Pointer or MappedPointer.", #ptr); \
            } \
        } \
        var = NIL_P(ptr) ? NULL : PointerAddress(ptr); \
    } while (0)

#define EXTRACT_SIZE_ARRAY(dim, ar, var) \
    size_t *var = NULL; \
    do { \
        if (!NIL_P(ar)) { \
            var = ALLOCA_N(size_t, dim); \
            if (TYPE(ar) == T_ARRAY && RARRAY_LEN(ar) == dim) { \
                for (cl_uint i = 0; i < dim; i++) { \
                    VALUE sz = rb_ary_entry(ar, i); \
                    if (TYPE(sz) != T_FIXNUM) { \
                        rb_raise(rb_eTypeError, "expected an Array of %u positive Integer.", dim); \
                    } \
                    var[i] = FIX2UINT(sz); \
                } \
            } else { \
                rb_raise(rb_eTypeError, "expected an Array of %u positive Integer.", dim); \
            } \
        } \
    } while (0)

// Extracts vec the Array of 3 size_t to var. If vec == nil, var is NULL.
#define EXTRACT_VECTOR(vec, var)    EXTRACT_SIZE_ARRAY(3, vec, var)

#define EXTRACT_IMAGE_FORMAT(imgfmt, var) \
    cl_image_format var; \
    do { \
        if (CLASS_OF(image_format) != rcl_cImageFormat) { \
            rb_raise(rb_eTypeError, "expected %s is a ImageFormat.", #imgfmt); \
        } \
        var.image_channel_order = FIX2INT(rb_iv_get(imgfmt, "@channel_order")); \
        var.image_channel_data_type = FIX2INT(rb_iv_get(imgfmt, "@channel_data_type")); \
    } while (0)

// NOTE: this macro needs local defined class variables
//       using a special pattern for name, /rcl_c\w+/
#define EXPECT_RCL_TYPE(o, klass) \
    do { \
        Check_Type(o, T_DATA); \
        if (rb_class_of(o) != rcl_c##klass) { \
            rb_raise(rb_eTypeError, "expected %s is an instance of %s.", #o, #klass); \
        } \
    } while (0)

#define EXTRACT_BOOLEAN(ro, var) \
    cl_bool var; \
    do { \
        EXPECT_BOOLEAN(ro); \
        var = ro == Qtrue ? CL_TRUE : CL_FALSE; \
    } while (0)

// WHEN DO YOU NEED A POINTER WRAPPER? WHEN YOU NEED THE ALLOC->INIT SEMANTICS
typedef struct {
    cl_context  c;
} rcl_context_t;

typedef struct {
    cl_command_queue cq;
} rcl_command_queue_t;

typedef struct {
    cl_sampler s;
} rcl_sampler_t;

typedef struct {
    cl_event e;
} rcl_event_t;

typedef struct {
    cl_mem  mem;
} rcl_mem_t;

typedef struct {
    cl_program p;
} rcl_program_t;

typedef struct {
    cl_kernel k;
} rcl_kernel_t;

void rcl_context_free(void *);
void rcl_command_queue_free(void *);
void rcl_event_free(void *);
void rcl_mem_free(void *);
void rcl_program_free(void *);
void rcl_kernel_free(void *);

#define RPlatform(ptr)      (Data_Wrap_Struct(rcl_cPlatform, 0, 0, (ptr)))

static inline cl_platform_id
PlatformPtr(VALUE ro)
{
    EXPECT_RCL_TYPE(ro, Platform);
    return DATA_PTR(ro);
}

#define RDevice(ptr)      Data_Wrap_Struct(rcl_cDevice, 0, 0, (ptr))

static inline cl_device_id
DevicePtr(VALUE ro)
{
    EXPECT_RCL_TYPE(ro, Device);
    return DATA_PTR(ro);
}

static inline cl_context
ContextPtr(VALUE ro)
{
    EXPECT_RCL_TYPE(ro, Context);

    rcl_context_t *p;
    Data_Get_Struct(ro, rcl_context_t, p);

    return p->c;
}

static inline VALUE
RContext(cl_context ptr)
{
    rcl_context_t *p;
    VALUE ret = Data_Make_Struct(rcl_cContext, rcl_context_t, 0, rcl_context_free, p);

    p->c = ptr;
    return ret;
}

static inline VALUE
RImageFormat(cl_image_format *imf)
{
    VALUE ro = rb_obj_alloc(rcl_cImageFormat);
    rb_iv_set(ro, "@channel_order",
              LONG2FIX(imf->image_channel_order));
    rb_iv_set(ro, "@channel_data_type",
              LONG2FIX(imf->image_channel_data_type));

    return ro;
}

static inline VALUE
RCommandQueue(cl_command_queue ptr)
{
    rcl_command_queue_t *p;
    VALUE ret = Data_Make_Struct(rcl_cCommandQueue, rcl_command_queue_t,
                                 0, rcl_command_queue_free, p);
    p->cq = ptr;
    return ret;
}

static inline cl_command_queue
CommandQueuePtr(VALUE ro)
{
    EXPECT_RCL_TYPE(ro, CommandQueue);

    rcl_command_queue_t *p;
    Data_Get_Struct(ro, rcl_command_queue_t, p);

    return p->cq;
}

static inline cl_sampler
SamplerPtr(VALUE ro)
{
    EXPECT_RCL_TYPE(ro, Sampler);

    rcl_sampler_t *ps;
    Data_Get_Struct(ro, rcl_sampler_t, ps);

    return ps->s;
}

static inline VALUE
REvent(cl_event ptr)
{
    rcl_event_t *p;
    VALUE ro = Data_Make_Struct(rcl_cEvent, rcl_event_t, 0, rcl_event_free, p);
    p->e = ptr;

    return ro;
}

static inline cl_event
EventPtr(VALUE ro)
{
    EXPECT_RCL_TYPE(ro, Event);

    rcl_event_t *p;
    Data_Get_Struct(ro, rcl_event_t, p);

    return p->e;
}

static inline cl_mem
MemoryPtr(VALUE ro)
{
    EXPECT_RCL_TYPE(ro, Memory);

    rcl_mem_t *p;
    Data_Get_Struct(ro, rcl_mem_t, p);

    return p->mem;
}

static inline VALUE
RMemory(cl_mem m)
{
    rcl_mem_t *p;
    VALUE ret = Data_Make_Struct(rcl_cMemory, rcl_mem_t, 0, rcl_mem_free, p);
    p->mem = m;

    return ret;
}

static inline VALUE
RProgram(cl_program prog)
{
    rcl_program_t *sp;
    VALUE ret = Data_Make_Struct(rcl_cProgram, rcl_program_t, 0, rcl_program_free, sp);

    sp->p = prog;
    return ret;
}

static inline cl_program
ProgramPtr(VALUE ro)
{
    EXPECT_RCL_TYPE(ro, Program);

    rcl_program_t *p;
    Data_Get_Struct(ro, rcl_program_t, p);

    return p->p;
}

static inline VALUE
RKernel(cl_kernel k)
{
    rcl_kernel_t *p;
    VALUE ret = Data_Make_Struct(rcl_cKernel, rcl_kernel_t, 0, rcl_kernel_free, p);

    p->k = k;
    return ret;
}

static inline cl_kernel
KernelPtr(VALUE ro)
{
    EXPECT_RCL_TYPE(ro, Kernel);

    rcl_kernel_t *p;
    Data_Get_Struct(ro, rcl_kernel_t, p);

    return p->k;
}

static inline VALUE
build_device_array(cl_device_id *devs, size_t cb)
{
    VALUE ret = rb_ary_new();
    size_t num_dev = cb / sizeof(cl_device_id);

    for (cl_uint i = 0; i < num_dev; i++) {
        VALUE dev = RDevice(devs[i]);
        rb_ary_push(ret, dev);
    }

    return ret;
}

#endif
