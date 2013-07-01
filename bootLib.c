/*+**************************************************************************
 *
 * Project:        RTEMS-GeSys
 * Module:         boot parameters access API for vxWorks compatibility
 * File:           bootLib.c
 *
 * Description:    interface for boot parameter access
 *
 * Author(s):      Dan Eichel
 *
 * Copyright (c) 2013     Helmholtz-Zentrum Berlin 
 *                     fuer Materialilien und Energie
 *                            Berlin, Germany
 *
 **************************************************************************-*/

#include "bootLib.h"
#include "NVRAMaccess.h"


/*+**************************************************************************
 *
 * Variable:    BOOT_LINE_ADRS
 *
 * Description: vxWorks bootLib API - under vxWorks points this variable
 *              into the NVRAM location where boot params stored
 *
 **************************************************************************-*/
char *BOOT_LINE_ADRS = NULL;



/*+**************************************************************************
 *
 * Function:    bootStructToString
 *
 * Description: vxWorks bootLib API - reads boot parameters from NVRAM
 *
 * Arg In:      paramString - not used char pointer (only for 
 *              compatibility with vxWorks bootLib)
 *              pBootParams - pointer to valid BOOT_PARAMS structure
 *
 * Return(s):   0 - success
 *
 **************************************************************************-*/
int bootStructToString(char *paramString, BOOT_PARAMS *pBootParams)
{
    writeNVram(pBootParams);
    return 0;
}


/*+**************************************************************************
 *
 * Function:    bootStringToStruct
 *
 * Description: vxWorks bootLib API - writes nww boot parameters into NVRAM
 *
 * Arg In:      bootString - not used char pointer (only for 
 *              compatibility with vxWorks bootLib)
 *              pBootParams - pointer to BOOT_PARAMS structure
 *
 * Return(s):   pBootParams structure filled with actual boot parameters
 *              pointer to bootString
 *
 **************************************************************************-*/
char *bootStringToStruct(char *bootString, BOOT_PARAMS *pBootParams)
{
    readNVram(pBootParams);
    return bootString;
}

