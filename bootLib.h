/*+**************************************************************************
 *
 * Project:        RTEMS-GeSys
 * Module:         boot parameters access API for vxWorks compatibility
 * File:           bootLib.h
 *
 * Description:    interface for boot parameter access
 *
 * Author(s):      Dan Eichel
 *
 * Copyright (c) 2013     Helmholtz-Zentrum Berlin 
 *                     fuer Materialien und Energie
 *                            Berlin, Germany
 *
 **************************************************************************-*/

#ifndef BOOTLIB_H
#define BOOTLIB_H

#include <netinet/in.h>                 /* uint32_t */

#define BOOT_DEV_LEN            20      /* max chars in device name */
#define BOOT_HOST_LEN           20      /* max chars in host name */
#define BOOT_ADDR_LEN           30      /* max chars in net addr */
#define BOOT_TARGET_ADDR_LEN    50      /* IP address + mask + lease times */
#define BOOT_ADDR_LEN           30      /* max chars in net addr */
#define BOOT_FILE_LEN           80      /* max chars in file name */
#define BOOT_USR_LEN            20      /* max chars in user name */
#define BOOT_PASSWORD_LEN       20      /* max chars in password */
#define BOOT_OTHER_LEN          80      /* max chars in "other" field */
#define BOOT_FIELD_LEN          80      /* max chars in boot field */

typedef struct {                        /* BOOT_PARAMS */
    char bootDev[BOOT_DEV_LEN];         /* boot device code */
    char hostName[BOOT_HOST_LEN];       /* name of host */
    char targetName[BOOT_HOST_LEN];     /* name of target */
    char ead[BOOT_TARGET_ADDR_LEN];     /* ethernet internet addr */
    char bad[BOOT_TARGET_ADDR_LEN];     /* backplane internet addr */
    char had[BOOT_ADDR_LEN];            /* host internet addr */
    char gad[BOOT_ADDR_LEN];            /* gateway internet addr */
    char bootFile[BOOT_FILE_LEN];       /* name of boot file */
    char startupScript[BOOT_FILE_LEN];  /* name of startup script file */
    char usr[BOOT_USR_LEN];             /* user name */
    char passwd[BOOT_PASSWORD_LEN];     /* password */
    char other[BOOT_OTHER_LEN];         /* available for applications */
    int procNum;                        /* processor number */
    int flags;                          /* configuration flags */
    int unitNum;                        /* network device unit number */
} BOOT_PARAMS;


/*+**************************************************************************
 *
 * Variable:    BOOT_LINE_ADRS
 *
 * Description: vxWorks bootLib API - pointer to the NVRAM location where
 *              boot params are stored in vxWorks
 *
 **************************************************************************-*/
extern char *BOOT_LINE_ADRS;


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
extern int bootStructToString (char *paramString, BOOT_PARAMS * pBootParams);


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
extern char *bootStringToStruct (char *bootString, BOOT_PARAMS * pBootParams);

#endif /* BOOTLIB_H */
