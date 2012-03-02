
#include "capi.h"

static VALUE
rcl_command_queue_alloc(VALUE klass)
{
    return RCommandQueue(NULL);
}

void
rcl_command_queue_free(void *ptr)
{
    rcl_command_queue_t *p = (rcl_command_queue_t *)ptr;
    if (p->cq != NULL) {
        clReleaseCommandQueue(p->cq);
    }
    xfree(p);
}

/*
 * call-seq:
 *      CommandQueue.new(context, device, 0)
 *
 * Wrapps +clCreateCommandQueue()+.
 *
 * Note that command queue properties must be +0+,
 * +CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE+, +CL_QUEUE_PROFILING_ENABLE+,
 * or the bit-or of the last two.
 */

static VALUE
rcl_command_queue_init(VALUE self, VALUE context, VALUE device, VALUE props)
{
    cl_context cxt = ContextPtr(context);
    cl_device_id did = DevicePtr(device);

    EXPECT_RCL_CONST(props);
    cl_uint properties = FIX2UINT(props);

    cl_int res;
    cl_command_queue q = clCreateCommandQueue(cxt, did, properties, &res);
    CHECK_AND_RAISE(res);

    rcl_command_queue_t *pcq;
    Data_Get_Struct(self, rcl_command_queue_t, pcq);

    pcq->cq = q;

    return self;
}

static VALUE
rcl_command_queue_init_copy(VALUE copy, VALUE orig)
{
    if (copy == orig) return copy;

    EXPECT_RCL_TYPE(orig, CommandQueue);

    rcl_command_queue_t *copy_p;
    rcl_command_queue_t *orig_p;
    Data_Get_Struct(copy, rcl_command_queue_t, copy_p);
    Data_Get_Struct(orig, rcl_command_queue_t, orig_p);

    if (copy_p->cq == orig_p->cq) return copy;

    cl_int res;
    if (copy_p->cq != NULL) {
        res = clReleaseCommandQueue(copy_p->cq);
        CHECK_AND_RAISE(res);
    }
    res = clRetainCommandQueue(orig_p->cq);
    CHECK_AND_RAISE(res);

    copy_p->cq = orig_p->cq;

    return copy;
}

/*
 * call-seq:
 *   aCommandQueue.info(CL_COMMAND_QUEUE_PROPERTIES)
 */
static VALUE
rcl_command_queue_info(VALUE self, VALUE command_queue_info)
{
    EXPECT_RCL_CONST(command_queue_info);
    cl_command_queue_info info = FIX2UINT(command_queue_info);
    cl_command_queue cq = CommandQueuePtr(self);

    size_t sz_ret = 0;
    cl_int res = clGetCommandQueueInfo(cq, info, 0, NULL, &sz_ret);
    CHECK_AND_RAISE(res);

    char *param_value = ALLOCA_N(char, sz_ret);
    res = clGetCommandQueueInfo(cq, info, sz_ret, param_value, NULL);
    CHECK_AND_RAISE(res);

    switch (info) {
    case CL_QUEUE_CONTEXT:
        return RContext((cl_context)param_value);
    case CL_QUEUE_DEVICE:
        return RDevice((cl_device_id)param_value);
    case CL_QUEUE_REFERENCE_COUNT:
        return UINT2NUM(*(cl_uint *)param_value);
    case CL_QUEUE_PROPERTIES:
        return UINT2NUM(*(cl_uint *)param_value);
    default:
        break;
    }
    return Qnil;
}

static VALUE
rcl_flush(VALUE self)
{
    cl_int res = clFlush(CommandQueuePtr(self));
    CHECK_AND_RAISE(res);

    return self;
}

static VALUE
rcl_finish(VALUE self)
{
    cl_int res = clFinish(CommandQueuePtr(self));
    CHECK_AND_RAISE(res);

    return self;
}

static VALUE
rcl_cq_enqueue_read_buffer(VALUE self, VALUE buffer, VALUE blocking_read,
                           VALUE offset, VALUE size, VALUE host_ptr, VALUE events)
{
    EXTRACT_MEM_OBJECT(buffer, buf);
    EXTRACT_BOOLEAN(blocking_read, br);
    EXTRACT_SIZE(offset, os);
    EXTRACT_SIZE(size, cb);
    EXTRACT_POINTER(host_ptr, ptr);
    EXTRACT_WAIT_FOR_EVENTS(events, num_evt, pevts);

    cl_command_queue cq = CommandQueuePtr(self);
    cl_event e;
    cl_event *ep = blocking_read ? NULL : &e;

    cl_int res = clEnqueueReadBuffer(cq, buf, br, os, cb, ptr, num_evt, pevts, ep);
    CHECK_AND_RAISE(res);

    return blocking_read ? self : REvent(e);
}

static VALUE
rcl_cq_enqueue_write_buffer(VALUE self, VALUE buffer, VALUE blocking_write,
                            VALUE offset, VALUE size, VALUE host_ptr, VALUE events)
{
    EXTRACT_MEM_OBJECT(buffer, buf);
    EXTRACT_BOOLEAN(blocking_write, bw);
    EXTRACT_SIZE(offset, os);
    EXTRACT_SIZE(size, cb);
    EXTRACT_POINTER(host_ptr, ptr);
    EXTRACT_WAIT_FOR_EVENTS(events, num_evt, pevts);

    cl_command_queue cq = CommandQueuePtr(self);
    cl_event e;
    cl_event *ep = blocking_write ? NULL : &e;
    cl_int res = clEnqueueWriteBuffer(cq, buf, bw, os, cb, ptr, num_evt, pevts, ep);
    CHECK_AND_RAISE(res);

    return blocking_write ? self : REvent(e);
}

static VALUE
rcl_cq_enqueue_copy_buffer(VALUE self, VALUE src_buffer, VALUE dst_buffer,
                           VALUE src_offset, VALUE dst_offset,
                           VALUE size, VALUE events)
{
    EXTRACT_MEM_OBJECT(src_buffer, sbuf);
    EXTRACT_MEM_OBJECT(dst_buffer, dbuf);
    EXTRACT_SIZE(src_offset, sos);
    EXTRACT_SIZE(dst_offset, dos);
    EXTRACT_SIZE(size, cb);
    EXTRACT_WAIT_FOR_EVENTS(events, num_evt, pevts);

    cl_event e;
    cl_command_queue cq = CommandQueuePtr(self);
    cl_int res = clEnqueueCopyBuffer(cq, sbuf, dbuf, sos, dos, cb, num_evt, pevts, &e);
    CHECK_AND_RAISE(res);

    return REvent(e);
}

#ifdef CL_VERSION_1_1

static VALUE
rcl_cq_enqueue_read_buffer_rect(VALUE self, VALUE buffer, VALUE blocking_read,
                                VALUE src_origin, VALUE dst_origin, VALUE offset,
                                VALUE src_row_pitch, VALUE src_slice_pitch,
                                VALUE dst_row_pitch, VALUE dst_slice_pitch,
                                VALUE host_ptr, VALUE events)
{
    EXTRACT_MEM_OBJECT(buffer, buf);
    EXTRACT_BOOLEAN(blocking_read, br);
    EXTRACT_VECTOR(src_origin, sor);
    EXTRACT_VECTOR(dst_origin, dor);
    EXTRACT_VECTOR(offset, ofs);
    EXTRACT_SIZE(src_row_pitch, srp);
    EXTRACT_SIZE(src_slice_pitch, ssp);
    EXTRACT_SIZE(dst_row_pitch, drp);
    EXTRACT_SIZE(dst_slice_pitch, dsp);
    EXTRACT_POINTER(host_ptr, hp);
    EXTRACT_WAIT_FOR_EVENTS(events, num_evt, pevts);

    cl_command_queue cq = CommandQueuePtr(self);
    cl_event e;
    cl_event *ep = (br == CL_TRUE) ? NULL : &e;

    cl_int res = clEnqueueReadBufferRect(cq, buf, br, sor, dor, ofs, srp, ssp, drp, dsp, hp, num_evt, pevts, ep);
    CHECK_AND_RAISE(res);

    return (br == CL_TRUE) ? self : REvent(e);
}

static VALUE
rcl_cq_enqueue_write_buffer_rect(VALUE self, VALUE buffer, VALUE blocking_write,
                                 VALUE src_origin, VALUE dst_origin, VALUE offset,
                                 VALUE src_row_pitch, VALUE src_slice_pitch,
                                 VALUE dst_row_pitch, VALUE dst_slice_pitch,
                                 VALUE host_ptr, VALUE events)
{
    EXTRACT_MEM_OBJECT(buffer, buf);
    EXTRACT_BOOLEAN(blocking_write, bw);
    EXTRACT_VECTOR(src_origin, sor);
    EXTRACT_VECTOR(dst_origin, dor);
    EXTRACT_VECTOR(offset, ofs);
    EXTRACT_SIZE(src_row_pitch, srp);
    EXTRACT_SIZE(src_slice_pitch, ssp);
    EXTRACT_SIZE(dst_row_pitch, drp);
    EXTRACT_SIZE(dst_slice_pitch, dsp);
    EXTRACT_POINTER(host_ptr, hp);
    EXTRACT_WAIT_FOR_EVENTS(events, num_evt, pevts);

    cl_command_queue cq = CommandQueuePtr(self);
    cl_event e;
    cl_event *ep = (bw == CL_TRUE) ? NULL : &e;

    cl_int res = clEnqueueWriteBufferRect(cq, buf, bw, sor, dor, ofs, srp, ssp, drp, dsp, hp, num_evt, pevts, ep);
    CHECK_AND_RAISE(res);

    return (bw == CL_TRUE) ? self : REvent(e);
}

static VALUE
rcl_cq_enqueue_copy_buffer_rect(VALUE self, VALUE src_buffer, VALUE dst_buffer,
                                VALUE src_origin, VALUE dst_origin, VALUE offset,
                                VALUE src_row_pitch, VALUE src_slice_pitch,
                                VALUE dst_row_pitch, VALUE dst_slice_pitch,
                                VALUE events)
{
    EXTRACT_MEM_OBJECT(src_buffer, sbuf);
    EXTRACT_MEM_OBJECT(dst_buffer, dbuf);
    EXTRACT_VECTOR(src_origin, sor);
    EXTRACT_VECTOR(dst_origin, dor);
    EXTRACT_VECTOR(offset, ofs);
    EXTRACT_SIZE(src_row_pitch, srp);
    EXTRACT_SIZE(src_slice_pitch, ssp);
    EXTRACT_SIZE(dst_row_pitch, drp);
    EXTRACT_SIZE(dst_slice_pitch, dsp);
    EXTRACT_WAIT_FOR_EVENTS(events, num_evt, pevts);

    cl_command_queue cq = CommandQueuePtr(self);
    cl_event e;

    cl_int res = clEnqueueCopyBufferRect(cq, sbuf, dbuf, sor, dor, ofs, srp, ssp, drp, dsp, num_evt, pevts, &e);
    CHECK_AND_RAISE(res);

    return self;
}

#endif

static VALUE
rcl_cq_enqueue_read_image(VALUE self, VALUE image, VALUE blocking_read,
                          VALUE origin, VALUE region,
                          VALUE row_pitch, VALUE slice_pitch,
                          VALUE host_ptr, VALUE events)
{
    EXTRACT_MEM_OBJECT(image, img);
    EXTRACT_BOOLEAN(blocking_read, br);
    EXTRACT_VECTOR(origin, ovec);
    EXTRACT_VECTOR(region, rvec);
    EXTRACT_SIZE(row_pitch, rp);
    EXTRACT_SIZE(slice_pitch, sp);
    EXTRACT_POINTER(host_ptr, hp);
    EXTRACT_WAIT_FOR_EVENTS(events, num_evt, pevts);

    cl_event e;
    cl_event *ep = blocking_read ? NULL : &e;

    cl_command_queue cq = CommandQueuePtr(self);
    cl_int res = clEnqueueReadImage(cq, img, br, ovec, rvec, rp, sp, hp, num_evt, pevts, ep);
    CHECK_AND_RAISE(res);

    return blocking_read ? self : REvent(e);
}

static VALUE
rcl_cq_enqueue_write_image(VALUE self, VALUE image, VALUE blocking_write,
                           VALUE origin, VALUE region,
                           VALUE row_pitch, VALUE slice_pitch,
                           VALUE host_ptr, VALUE events)
{
    EXTRACT_MEM_OBJECT(image, img);
    EXTRACT_BOOLEAN(blocking_write, bw);
    EXTRACT_VECTOR(origin, ovec);
    EXTRACT_VECTOR(region, rvec);
    EXTRACT_SIZE(row_pitch, rp);
    EXTRACT_SIZE(slice_pitch, sp);
    EXTRACT_POINTER(host_ptr, hp);
    EXTRACT_WAIT_FOR_EVENTS(events, num_evt, pevts);

    cl_event e;
    cl_event *ep = blocking_write ? NULL : &e;

    cl_command_queue cq = CommandQueuePtr(self);
    cl_int res = clEnqueueWriteImage(cq, img, bw, ovec, rvec, rp, sp, hp, num_evt, pevts, ep);
    CHECK_AND_RAISE(res);

    return blocking_write ? self : REvent(e);
}

static VALUE
rcl_cq_enqueue_copy_image(VALUE self, VALUE src_image, VALUE dst_image,
                          VALUE src_origin, VALUE dst_origin, VALUE region,
                          VALUE events)
{
    EXTRACT_MEM_OBJECT(src_image, simg);
    EXTRACT_MEM_OBJECT(dst_image, dimg);
    EXTRACT_VECTOR(src_origin, sovec);
    EXTRACT_VECTOR(dst_origin, dovec);
    EXTRACT_VECTOR(region, rvec);
    EXTRACT_WAIT_FOR_EVENTS(events, num_evt, pevts);

    cl_event e;
    cl_command_queue cq = CommandQueuePtr(self);
    cl_int res = clEnqueueCopyImage(cq, simg, dimg, sovec, dovec, rvec, num_evt, pevts, &e);
    CHECK_AND_RAISE(res);

    return REvent(e);
}

static VALUE
rcl_cq_enqueue_copy_image_to_buffer(VALUE self, VALUE src_image, VALUE dst_buffer,
                                    VALUE src_origin, VALUE region,
                                    VALUE dst_offset, VALUE events)
{
    EXTRACT_MEM_OBJECT(src_image, img);
    EXTRACT_MEM_OBJECT(dst_buffer, buf);
    EXTRACT_VECTOR(src_origin, sovec);
    EXTRACT_VECTOR(region, rvec);
    EXTRACT_SIZE(dst_offset, cb);
    EXTRACT_WAIT_FOR_EVENTS(events, num_evt, pevts);

    cl_event e;
    cl_command_queue cq = CommandQueuePtr(self);
    cl_int res = clEnqueueCopyImageToBuffer(cq, img, buf, sovec, rvec, cb, num_evt, pevts, &e);
    CHECK_AND_RAISE(res);

    return REvent(e);
}

static VALUE
rcl_cq_enqueue_copy_buffer_to_image(VALUE self, VALUE src_buffer, VALUE dst_image,
                                    VALUE src_offset, VALUE dst_origin,
                                    VALUE region, VALUE events)
{
    EXTRACT_MEM_OBJECT(src_buffer, buf);
    EXTRACT_MEM_OBJECT(dst_image, img);
    EXTRACT_SIZE(src_offset, cb);
    EXTRACT_VECTOR(dst_origin, sovec);
    EXTRACT_VECTOR(region, rvec);
    EXTRACT_WAIT_FOR_EVENTS(events, num_evt, pevts);

    cl_event e;
    cl_command_queue cq = CommandQueuePtr(self);
    cl_int res = clEnqueueCopyBufferToImage(cq, buf, img, cb, sovec, rvec, num_evt, pevts, &e);
    CHECK_AND_RAISE(res);

    return REvent(e);
}

/*
 * call-seq:
 *      CommandQueue#enqueue_map_buffer(aMemObj, yesno, map_flags, offset, size, events) -> [aMappedPointer{, event}]
 *
 * Returns an Array in which the first element is a MappedPointer.
 * If blocking_map is false, the second element is the event for the map
 * command.
 */
static VALUE
rcl_cq_enqueue_map_buffer(VALUE self, VALUE mem_obj, VALUE blocking_map,
                          VALUE flags, VALUE offset, VALUE cb,
                          VALUE events)
{
    EXTRACT_MEM_OBJECT(mem_obj, mo);
    EXTRACT_BOOLEAN(blocking_map, bm);
    EXPECT_FIXNUM(flags);
    cl_mem_flags mf = FIX2INT(flags);
    EXTRACT_SIZE(offset, os);
    EXTRACT_SIZE(cb, sz);
    EXTRACT_WAIT_FOR_EVENTS(events, num_evt, pevts);

    cl_event e;
    cl_event *pe = blocking_map ? NULL : &e;

    cl_command_queue cq = CommandQueuePtr(self);
    cl_int res;
    void *ptr = clEnqueueMapBuffer(cq, mo, bm, mf, os, sz, num_evt, pevts, pe, &res);
    CHECK_AND_RAISE(res);

    VALUE mp = rcl_create_mapped_pointer(ptr, sz);
    VALUE ret = rb_ary_new();
    rb_ary_push(ret, mp);
    if (!blocking_map) {
        rb_ary_push(ret, REvent(e));
    }
    return ret;
}

/*
 * call-seq:
 *		aCommandQueue.enqueue_map_image(aImage, true, CL_MEM_FLAG_READ, ...)
 *
 * Returns an Array contains following information in order:
 * * A MappedPointer object
 * * The row pitch
 * * The slice pitch
 * * An Event object if blocking_map is false
 */
static VALUE
rcl_cq_enqueue_map_image(VALUE self, VALUE image, VALUE blocking_map,
                         VALUE flags, VALUE origin, VALUE region,
                         VALUE events)
{
    EXTRACT_MEM_OBJECT(image, img);
    EXTRACT_BOOLEAN(blocking_map, bm);
    EXPECT_FIXNUM(flags);
    cl_mem_flags mf = FIX2INT(flags);

    EXTRACT_VECTOR(origin, ovec);
    EXTRACT_VECTOR(region, rvec);

    EXTRACT_WAIT_FOR_EVENTS(events, num_evt, pevts);

    size_t row_pitch;
    size_t slice_pitch;

    cl_event e;
    cl_event *ep = blocking_map ? NULL : &e;

    cl_command_queue cq = CommandQueuePtr(self);
    cl_int res;
    void *ptr = clEnqueueMapImage(cq, img, bm, mf, ovec, rvec, &row_pitch, &slice_pitch, num_evt, pevts, ep, &res);
    CHECK_AND_RAISE(res);

    // compute the size of mapped memory.
    size_t byte_size = (slice_pitch == 0) ? (rvec[1] * row_pitch) : (rvec[2] * slice_pitch);
    VALUE mp = rcl_create_mapped_pointer(ptr, byte_size);
    VALUE ret = rb_ary_new();
    rb_ary_push(ret, mp);
    rb_ary_push(ret, ULONG2NUM(row_pitch));
    rb_ary_push(ret, ULONG2NUM(slice_pitch));
    if (!blocking_map) rb_ary_push(ret, REvent(e));
    return ret;
}

static VALUE
rcl_cq_enqueue_unmap_mem_obj(VALUE self, VALUE mem_obj, VALUE mapped_ptr,
                             VALUE events)
{
    EXTRACT_MEM_OBJECT(mem_obj, mo);
    if (CLASS_OF(mapped_ptr) != rcl_cMappedPointer) {
        rb_raise(rb_eArgError, "expected argument 2 is a MappedPointer.");
    }
    void *mp = PointerAddress(mapped_ptr);
    EXTRACT_WAIT_FOR_EVENTS(events, num_evt, pevts);

    cl_event e;
    cl_command_queue cq = CommandQueuePtr(self);
    cl_int res = clEnqueueUnmapMemObject(cq, mo, mp, num_evt, pevts, &e);
    CHECK_AND_RAISE(res);

    rcl_invalidate_mapped_pointer(mapped_ptr);
    return REvent(e);
}

/*
 * call-seq:
 *      aComamndQueue.enqueue_NDRange_kernel(...)   -> an Event object
 *
 * Wrapps the +clEnqueueNDRangeKernel()+.
 */
static VALUE
rcl_cq_enqueue_ndrange_kernel(VALUE self, VALUE kernel, VALUE work_dim,
                              VALUE global_work_size, VALUE local_work_size,
                              VALUE events)
{
    EXPECT_RCL_TYPE(kernel, Kernel);
    EXPECT_FIXNUM(work_dim);
    cl_uint wd = FIX2UINT(work_dim);
    EXTRACT_WAIT_FOR_EVENTS(events, num_evt, pevts);

    EXTRACT_SIZE_ARRAY(wd, global_work_size, gws);
    if (gws == NULL) {
        rb_raise(rb_eArgError, "global work sizes can't be nil.");
    }
    EXTRACT_SIZE_ARRAY(wd, local_work_size, lws);

    cl_event e;
    cl_kernel k = KernelPtr(kernel);
    cl_command_queue cq = CommandQueuePtr(self);

    cl_int res = clEnqueueNDRangeKernel(cq, k, wd, NULL, /* global work offset, must be NULL */
                                        gws, lws,
                                        num_evt, pevts, &e);
    CHECK_AND_RAISE(res);

    return REvent(e);
}

static VALUE
rcl_cq_enqueue_task(VALUE self, VALUE kernel, VALUE events)
{
    EXPECT_RCL_TYPE(kernel, Kernel);
    EXTRACT_WAIT_FOR_EVENTS(events, num_evt, pevts);

    cl_event e;
    cl_kernel k = KernelPtr(kernel);
    cl_command_queue cq = CommandQueuePtr(self);
    cl_int res = clEnqueueTask(cq, k, num_evt, pevts, &e);
    CHECK_AND_RAISE(res);

    return REvent(e);
}

static VALUE
rcl_cq_enqueue_native_kernel(VALUE self)
{
    rb_notimplement();
    return self;
}

/*
 * call-seq:
 *      CommandQueue#enqueue_marker     -> an Event object
 *
 * Wrapps +clEnqueueMarker()+.
 */
static VALUE
rcl_cq_enqueue_marker(VALUE self)
{
    cl_command_queue cq = CommandQueuePtr(self);

    cl_event e;
    cl_int res = clEnqueueMarker(cq, &e);
    CHECK_AND_RAISE(res);

    return REvent(e);
}

/*
 * call-seq:
 *      CommandQueue#enqueue_waitfor_events     -> the receiver
 *
 * Wrapps +clEnqueueWaitForEvents()+.
 */
static VALUE
rcl_cq_enqueue_waitfor_events(VALUE self, VALUE events)
{
    EXTRACT_WAIT_FOR_EVENTS(events, num_evt, pes);

    cl_command_queue cq = CommandQueuePtr(self);
    cl_int res = clEnqueueWaitForEvents(cq, num_evt, pes);
    CHECK_AND_RAISE(res);

    return self;
}

/*
 * call-seq:
 *      aCommandQueue.enqueue_barrier     -> the receiver
 *
 * Wrapps +clEnqueueBarrier()+.
 */
static VALUE
rcl_cq_enqueue_barrier(VALUE self)
{
    cl_command_queue cq = CommandQueuePtr(self);
    cl_int res = clEnqueueBarrier(cq);
    CHECK_AND_RAISE(res);

    return self;
}

/*
 * call-seq:
 *      CommandQueue#enqueue_acquire_gl_objects([Memory, ..], nil)   -> a Event
 *      CommandQueue#enqueue_acquire_gl_objects([], nil)     -> the receiver
 *
 * Wrapps +clEnqueueAcquireGLObjects()+.
 */
static VALUE
rcl_cq_enqueue_acquire_gl_objects(VALUE self, VALUE mem_objects, VALUE events)
{
    EXPECT_ARRAY(mem_objects);
    cl_uint num_mo = (cl_uint)RARRAY_LEN(mem_objects);
    if (num_mo == 0) return self;

    EXTRACT_WAIT_FOR_EVENTS(events, num_evt, pevts);

    cl_mem *pmos;
    EXTRACT_CL_POINTERS(mem_objects, Memory, cl_mem, pmos);

    cl_event evt;
    cl_command_queue cq = CommandQueuePtr(self);
    cl_int res = clEnqueueAcquireGLObjects(cq, num_mo, pmos, num_evt, pevts, &evt);
    CHECK_AND_RAISE(res);

    return REvent(evt);
}

/*
 * call-seq:
 *      CommandQueue#enqueue_release_gl_objects     -> Event
 *
 * Wrapps +clEnqueueReleaseGLObjects()+.
 */
static VALUE
rcl_cq_enqueue_release_gl_objects(VALUE self, VALUE mem_objects, VALUE events)
{
    EXPECT_ARRAY(mem_objects);
    cl_uint num_mo = (cl_uint)RARRAY_LEN(mem_objects);
    if (num_mo == 0) return self;

    EXTRACT_WAIT_FOR_EVENTS(events, num_evt, pevts);

    cl_mem *pmos;
    EXTRACT_CL_POINTERS(mem_objects, Memory, cl_mem, pmos);

    cl_event evt;
    cl_command_queue cq = CommandQueuePtr(self);
    cl_int res = clEnqueueReleaseGLObjects(cq, num_mo, pmos, num_evt, pevts, &evt);
    CHECK_AND_RAISE(res);

    return REvent(evt);
}

void
rcl_define_class_command_queue(VALUE mod)
{
    rcl_cCommandQueue = rb_define_class_under(mod, "CommandQueue", rb_cObject);
    rb_define_alloc_func(rcl_cCommandQueue, rcl_command_queue_alloc);
    rb_define_method(rcl_cCommandQueue, "initialize", rcl_command_queue_init, 3);
    rb_define_method(rcl_cCommandQueue, "initialize_copy", rcl_command_queue_init_copy, 1);
    rb_define_method(rcl_cCommandQueue, "info", rcl_command_queue_info, 1);

    // mem
    rb_define_method(rcl_cCommandQueue, "enqueue_read_buffer", rcl_cq_enqueue_read_buffer, 6);
    rb_define_method(rcl_cCommandQueue, "enqueue_write_buffer", rcl_cq_enqueue_write_buffer, 6);
    rb_define_method(rcl_cCommandQueue, "enqueue_copy_buffer", rcl_cq_enqueue_copy_buffer, 6);
#ifdef CL_VERSION_1_1
    rb_define_method(rcl_cCommandQueue, "enqueue_read_buffer_rect", rcl_cq_enqueue_read_buffer_rect, 11);
    rb_define_method(rcl_cCommandQueue, "enqueue_write_buffer_rect", rcl_cq_enqueue_write_buffer_rect, 11);
    rb_define_method(rcl_cCommandQueue, "enqueue_copy_buffer_rect", rcl_cq_enqueue_copy_buffer_rect, 10);
#endif
    rb_define_method(rcl_cCommandQueue, "enqueue_read_image", rcl_cq_enqueue_read_image, 8);
    rb_define_method(rcl_cCommandQueue, "enqueue_write_image", rcl_cq_enqueue_write_image, 8);
    rb_define_method(rcl_cCommandQueue, "enqueue_copy_image", rcl_cq_enqueue_copy_image, 6);
    rb_define_method(rcl_cCommandQueue, "enqueue_copy_image_to_buffer", rcl_cq_enqueue_copy_image_to_buffer, 6);
    rb_define_method(rcl_cCommandQueue, "enqueue_copy_buffer_to_image", rcl_cq_enqueue_copy_buffer_to_image, 6);
    rb_define_method(rcl_cCommandQueue, "enqueue_map_buffer", rcl_cq_enqueue_map_buffer, 6);
    rb_define_method(rcl_cCommandQueue, "enqueue_map_image", rcl_cq_enqueue_map_image, 6);
    rb_define_method(rcl_cCommandQueue, "enqueue_unmap_mem_object", rcl_cq_enqueue_unmap_mem_obj, 3);

    // execution
    rb_define_method(rcl_cCommandQueue, "enqueue_NDRange_kernel", rcl_cq_enqueue_ndrange_kernel, 5);
    rb_define_method(rcl_cCommandQueue, "enqueue_task", rcl_cq_enqueue_task, 2);
    rb_define_method(rcl_cCommandQueue, "enqueue_native_kernel", rcl_cq_enqueue_native_kernel, 0);

    // sync
    rb_define_method(rcl_cCommandQueue, "enqueue_marker",rcl_cq_enqueue_marker, 0);
    rb_define_method(rcl_cCommandQueue, "enqueue_wait_for_events", rcl_cq_enqueue_waitfor_events, 1);
    rb_define_method(rcl_cCommandQueue, "enequeu_barrier", rcl_cq_enqueue_barrier, 0);

    rb_define_method(rcl_cCommandQueue, "flush", rcl_flush, 0);
    rb_define_method(rcl_cCommandQueue, "finish", rcl_finish, 0);

    // gl interoperation
    rb_define_method(rcl_cCommandQueue, "enqueue_acquire_gl_objects", rcl_cq_enqueue_acquire_gl_objects, 2);
    rb_define_method(rcl_cCommandQueue, "enqueue_release_gl_objects", rcl_cq_enqueue_release_gl_objects, 2);

}
