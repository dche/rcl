
#include "capi.h"

void
rcl_event_free(void *ptr)
{
    rcl_event_t *p = (rcl_event_t *)ptr;
    if (p->e != NULL) {
        clReleaseEvent(p->e);
    }
    xfree(p);
}

/*
 * call-seq:
 *      info(anEventInfoConstant)
 */
static VALUE
rcl_event_info(VALUE self, VALUE event_info)
{
    EXPECT_RCL_CONST(event_info);
    cl_event_info info = FIX2UINT(event_info);

    cl_event e = EventPtr(self);
    intptr_t param_value;

    cl_int res = clGetEventInfo(e, info, sizeof(intptr_t), &param_value, NULL);
    CHECK_AND_RAISE(res);

    switch (info) {
        case CL_EVENT_COMMAND_QUEUE:
            return RCommandQueue((cl_command_queue)param_value);
        case CL_EVENT_COMMAND_TYPE:
        case CL_EVENT_REFERENCE_COUNT:
            return LONG2FIX((cl_uint)param_value);
        case CL_EVENT_COMMAND_EXECUTION_STATUS:
            return INT2FIX((cl_int)param_value);
        default:
            break;
    }
    return Qnil;

}

/*
 * call-seq:
 *      Capi.wait_for_events(anArrayOfEventObject)
 */
static VALUE
rcl_wait_for_events(VALUE self, VALUE events)
{
    EXPECT_ARRAY(events);
    cl_uint num = (cl_uint)RARRAY_LEN(events);
    if (num == 0) return Qfalse;

    cl_event *pe;
    EXTRACT_CL_POINTERS(events, Event, cl_event, pe);

    cl_int res = clWaitForEvents(num, pe);
    CHECK_AND_RAISE(res);

    return Qtrue;
}

#ifdef CL_VERSION_1_1

static VALUE
rcl_set_user_event_status(VALUE self, VALUE execution_status)
{
    EXPECT_FIXNUM(execution_status);
    cl_int es = FIX2INT(execution_status);

    cl_event evt = EventPtr(self);
    cl_int res = clSetUserEventStatus(evt, es);
    CHECK_AND_RAISE(res);

    return self;
}

/*
 *
 */
static void
rcl_pfn_event_callback(cl_event evt, cl_int cmd_exec_status, void *usr_data)
{
    if (clRetainEvent(evt) != CL_SUCCESS)
        return;

    VALUE ro = REvent(evt);

    VALUE block = (VALUE)usr_data;
    assert(!NIL_P(block));

    rb_funcall(block, rb_intern("call"), 2, ro, INT2FIX(cmd_exec_status));
}

/*
 * call-seq:
 *      set_callback(CL_COMPLETE) do |event, command_exec_status| ...
 *
 */
static VALUE
rcl_set_event_callback(int argc, VALUE *argv, VALUE self)
{
    VALUE command_exec_status;
    VALUE block;
    cl_int cmd_exec_stat = CL_COMPLETE;

    // "01&" -> 0 mandatory arg, 1 optional arg, &block must provided.
    rb_scan_args(argc, argv, "01&", &command_exec_status, &block);
    if (!NIL_P(command_exec_status)) {
        EXPECT_RCL_CONST(command_exec_status);
        cmd_exec_stat = FIX2UINT(command_exec_status);
    }

    cl_event evt = EventPtr(self);
    cl_int res = clSetEventCallback(evt, cmd_exec_stat, rcl_pfn_event_callback, (void *)block);
    CHECK_AND_RAISE(res);

    return self;
}

#endif

/*
 * call-seq:
 *      profiling_info(CL_PROFILING_COMMAND_START)
 *
 */
static VALUE
rcl_event_profiling_info(VALUE self, VALUE profiling_info)
{
    EXPECT_RCL_CONST(profiling_info);
    cl_profiling_info itype = FIX2UINT(profiling_info);

    cl_ulong info;
    cl_event e = EventPtr(self);

    cl_int res = clGetEventProfilingInfo(e, itype, sizeof(cl_ulong), &info, NULL);
    CHECK_AND_RAISE(res);

    return ULONG2NUM(info);
}

void
rcl_define_class_event(VALUE mod)
{
    rcl_cEvent = rb_define_class_under(mod, "Event", rb_cObject);
    rb_undef_alloc_func(rcl_cEvent);
    rb_define_method(rcl_cEvent, "info", rcl_event_info, 1);
    rb_define_module_function(mod, "wait_for_events", rcl_wait_for_events, 1);
    rb_define_method(rcl_cEvent, "profiling_info", rcl_event_profiling_info, 1);
#ifdef CL_VERSION_1_1
    rb_define_method(rcl_cEvent, "set_status", rcl_set_user_event_status, 1);
    rb_define_method(rcl_cEvent, "set_callback", rcl_set_event_callback, -1);
#endif
}
