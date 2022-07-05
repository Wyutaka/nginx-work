#include <rte_ring.h>

#ifdef __cplusplus
#define MY_EXTERN extern "C"
#else
#define MY_EXTERN extern
#endif

MY_EXTERN struct rte_ring* create_queue(unsigned int addr);

MY_EXTERN void destroy_queue();

MY_EXTERN struct rte_ring* get_queue(unsigned int addr);

MY_EXTERN struct rte_ring* search_queue_by_name(const char *name);
