#pragma once

#ifdef __cplusplus
#define MY_EXTERN extern "C"
#else
#define MY_EXTERN extern
#endif

MY_EXTERN int init_token(const char* name);
MY_EXTERN unsigned int* get_address_list(int* n);
MY_EXTERN unsigned int token2address(long token);
