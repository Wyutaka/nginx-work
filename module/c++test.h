#ifndef __CPPSUB_H__
#define __CPPSUB_H__ 1
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

  extern void *new_object(double initial_value);
  extern double call_method(void *the_object, double param);
  extern void delete_object(void *the_object);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __CPPSUB_H__ */