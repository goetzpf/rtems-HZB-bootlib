/*+**************************************************************************
 *
 * Project:        RTEMS-GeSys
 * Module:         NVRAM boot parameters access
 * File:           NVRAMaccess_dummy.c
 *
 * Description:    Dummy for unsupported BSPs
 *
 * Author(s):      Dan Eichel
 *
 * Copyright (c) 2015     Helmholtz-Zentrum Berlin 
 *                     fuer Materialien und Energie
 *                            Berlin, Germany
 *
 **************************************************************************-*/

#include "bootLib.h"


/*+**************************************************************************
 *
 * Function:    readNVram
 *
 * Description: does simply nothing
 *
 * Arg In:      ptr - pointer to BOOT_PARAMS structure
 *
 * Return(s):   none
 *
 **************************************************************************-*/
void
readNVram (BOOT_PARAMS * ptr)
{
}


/*+**************************************************************************
 *
 * Function:    writeNVram
 *
 * Description: does simply nothing
 *
 * Arg In:      ptr - pointer to BOOT_PARAMS structure
 *
 * Return(s):   none
 *
 **************************************************************************-*/
void
writeNVram (BOOT_PARAMS * ptr)
{
}
