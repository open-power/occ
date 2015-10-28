
#ifndef _APSS_UTIL_H
#define _APSS_UTIL_H

#include <apss_structs.h>

void apss_set_ffdc(GpeErrorStruct *o_error, uint32_t i_addr, uint32_t i_rc, uint64_t i_ffdc);

int wait_spi_completion(GpeErrorStruct *error, uint32_t reg, uint8_t timeout);

// Read decrementer register
#define MFDEC(reg_var) \
asm volatile \
    ( \
    " mfdec %[dec_var] \n" \
    : [dec_var]"=r"(reg_var) \
    );

#endif //_APSS_UTIL_H
