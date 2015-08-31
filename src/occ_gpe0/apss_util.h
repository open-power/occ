
#ifndef _APSS_UTIL_H
#define _APSS_UTIL_H

#include <apss_structs.h>

void apss_set_ffdc(GpeErrorStruct *o_error, uint32_t i_addr, uint32_t i_rc, uint64_t i_ffdc);

int wait_spi_completion(initGpioArgs_t *args, uint32_t reg, uint8_t timeout);

#endif //_APSS_UTIL_H
