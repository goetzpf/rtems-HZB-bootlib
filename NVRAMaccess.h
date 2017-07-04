/*+**************************************************************************
 *
 * Project:        RTEMS-GeSys
 * Module:         NVRAM boot parameters access
 * File:           NVRAMaccess.h
 *
 * Description:    Interface for NVRAM access and some useful tools
 *
 * Author(s):      Dan Eichel
 *
 * Copyright (c) 2013     Helmholtz-Zentrum Berlin 
 *                     fuer Materialien und Energie
 *                            Berlin, Germany
 *
 **************************************************************************-*/

#ifndef __NVRAMACCESS__
#define __NVRAMACCESS__

#include "bootLib.h"

#define DEFAULT_SUBNETMASK     0xFFFFFF00
#define DEFAULT_SUBNETMASK_STR "255.255.255.0"



/*+**************************************************************************
 *
 * Function:    readNVram
 *
 * Description: reads boot parameters from NVRAM and fills the structure
 *              ptr points to
 *
 * Arg In:      ptr - pointer to BOOT_PARAMS structure
 *
 * Return(s):   none
 *
 **************************************************************************-*/
extern void readNVram (BOOT_PARAMS * ptr);


/*+**************************************************************************
 *
 * Function:    writeNVram
 *
 * Description: writes boot parameters from structure ptr points to into 
 *              NVRAM
 *
 * Arg In:      ptr - pointer to BOOT_PARAMS structure
 *
 * Return(s):   none
 *
 **************************************************************************-*/
extern void writeNVram (BOOT_PARAMS * ptr);


/*+**************************************************************************
 *
 * Function:    bootlib_addrToStr
 *
 * Description: This function converts the network address
 *              into a character string
 *
 * Arg In:      1) pointer to string buffer
 *              2) network address
 *
 * Return(s):   pointer to string buffer filled with address in dot notation
 *
 **************************************************************************-*/
extern char *bootlib_addrToStr (char *cbuf, uint32_t addr);


/*+**************************************************************************
 *
 * Function:    bootlib_addrToInt
 *
 * Description: This function converts the character string into a
 *              network address
 *
 * Arg In:      pointer to character string containing address in dot notation
 *
 * Return(s):   network address
 *
 **************************************************************************-*/
extern int bootlib_addrToInt (char *cbuf);


/*+**************************************************************************
 *
 * Internal helper functions
 *
 **************************************************************************-*/

extern void getsubstr (char *buf, char *dest, int maxlen, char *marker);
extern char *cvrtsmask (char *str, char *dest);


#endif /* __NVRAMACCESS__ */
