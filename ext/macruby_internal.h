
// CHECK: in MacRuby v0.12, there's no way to include "macruby_internal.h",
//        in which GC_WB is defined. So we have to define this macro.

#ifndef RCL_MACRUBY_INTERNAL_H__
#define RCL_MACRUBY_INTERNAL_H__

#include <objc/objc-auto.h>

#if !defined(__AUTO_ZONE__)
boolean_t auto_zone_set_write_barrier(void *zone, const void *dest, const void *new_value);
extern void *__auto_zone;
#else
extern auto_zone_t *__auto_zone;
#endif

#define GC_WB_0(dst, newval, check) \
    do { \
	void *nv = (void *)newval; \
	if (!SPECIAL_CONST_P(nv)) { \
	    if (!auto_zone_set_write_barrier(__auto_zone, \
			(const void *)dst, (const void *)nv)) { \
		if (check) { \
		    rb_bug("destination %p isn't in the auto zone", dst); \
		} \
	    } \
	} \
	*(void **)dst = nv; \
    } \
    while (0)

#define GC_WB(dst, newval) GC_WB_0(dst, newval, true)

#endif
