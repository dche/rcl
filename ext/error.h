
#ifndef RCL_ERROR_H__
#define RCL_ERROR_H__

#define CHECK_AND_RAISE(code)   (check_cl_error(code, 0))
#define CHECK_AND_WARN(code)    (check_cl_error(code, 1))

void check_cl_error(cl_int, int);
void define_class_clerror(VALUE);

#endif
