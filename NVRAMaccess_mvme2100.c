/*+**************************************************************************
 *
 * Project:        RTEMS-GeSys
 * Module:         NVRAM boot parameters access
 * File:           NVRAMaccess_mvme2100.c
 *
 * Description:    Support for ppcbug NVRAM access
 *
 * Author(s):      Dan Eichel
 *
 * Copyright (c) 2013     Helmholtz-Zentrum Berlin 
 *                      fuer Materialien und Energie
 *                            Berlin, Germany
 *
 **************************************************************************-*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "NVRAMaccess.h"


/* values for MVME2100 board */
#define NVRAM_VXPARAMS     0xFF880100
#define NVRAM_BUGPARAMS    0xFFE81000

typedef struct {
    uint32_t    PacketVersionIdentifier;
    uint32_t    NodeControlMemoryAddress;
    uint32_t    BootFileLoadAddress;
    uint32_t    BootFileExecutionAddress;
    uint32_t    BootFileExecutionDelay;
    uint32_t    BootFileLength;
    uint32_t    BootFileByteOffset;
    uint32_t    TraceBufferAddress;
    uint32_t    ClientIPAddress;
    uint32_t    ServerIPAddress;
    uint32_t    SubnetIPAddressMask;
    uint32_t    BroadcastIPAddressMask;
    uint32_t    GatewayIPAddress;
    uint8_t     BootpRarpRetry;
    uint8_t     TftpRarpRetry;
    uint8_t     BootpRarpControl;
    uint8_t     UpdateControl;
    char        BootFilenameString[64];
    char        ArgumentFilenameString[64];
} ppcbug_nvram;

/* include private helper functions... */
#include "NVRAMaccess.c"


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
void readNVram(BOOT_PARAMS *ptr)
{
    ppcbug_nvram bug_map;
    char buf[512], *cptr;
    
    /* read vxWorks boot parameters */
    byteCopy((volatile char *) &buf, (volatile char *) NVRAM_VXPARAMS, sizeof(buf));
#if 0
    sprintf(buf, "%s(%i,%i)%s:%s e=%s b=%s h=%s g=%s u=%s pw=%s f=%i tn=%s s=%s o=%s", \
                 "dc", 0, 0, "vwhost", "/opt/tftfdir/vxWorks.st", "193.149.12.200:ffffff00", "", \
                 "193.149.12.29", "1", "ioc", "2", 0x88, "box0", "/opt/test.st.cmd", "");
#endif                

    /* read ppcbug environment out */
    byteCopy((volatile char *) &bug_map, (volatile char *) NVRAM_BUGPARAMS, sizeof(bug_map));

    cptr = strchr(buf, '('); if (cptr != NULL) *cptr = ' ';
    cptr = strchr(buf, ','); if (cptr != NULL) *cptr = ' ';
    cptr = strchr(buf, ')'); if (cptr != NULL) *cptr = ' ';
    cptr = strchr(buf, ':'); if (cptr != NULL) *cptr = ' '; /* between boothost and filename */

    sscanf(buf, "%s %i %i %s %s", \
        ptr->bootDev, (int *) &(ptr->unitNum), (int *) &(ptr->procNum), ptr->hostName, ptr->bootFile);

    getsubstr(buf, ptr->ead, param_lengths[3], "e="); /* ead */
    getsubstr(buf, ptr->bad, param_lengths[4], "b="); /* bad */
    getsubstr(buf, ptr->had, param_lengths[5], "h="); /* had */
    getsubstr(buf, ptr->gad, param_lengths[6], "g="); /* gad */
    getsubstr(buf, ptr->usr, param_lengths[9], "u="); /* usr */
    getsubstr(buf, ptr->passwd, param_lengths[10], "pw="); /* passwd */
    getsubstr(buf, ptr->targetName, param_lengths[2], "tn="); /* targetName */
    getsubstr(buf, ptr->startupScript, param_lengths[8], "s="); /* startupScript */
    getsubstr(buf, ptr->other, param_lengths[11], "o="); /* other */
    
    cptr = strstr(buf, "f="); if (cptr != NULL) ptr->flags = strtol(cptr + 2, NULL, 16); else ptr->flags = 0;

    /* override with ppc bug settings, if present */
    addrToStr(buf, bug_map.ClientIPAddress); strcpy(ptr->ead, buf);
    if (bug_map.SubnetIPAddressMask != DEFAULT_SUBNETMASK)
    {
        sprintf(buf, ":%x", (unsigned int) bug_map.SubnetIPAddressMask);
        strcat(ptr->ead, buf);
    }
    if (addrToStr(buf, bug_map.ServerIPAddress) != NULL) strcpy(ptr->had, buf);
    
    if (bug_map.GatewayIPAddress != 0)
    {
        if (addrToStr(buf, bug_map.GatewayIPAddress) != NULL) strcpy(ptr->gad, buf);
    } else ptr->gad[0] = 0;
    
    strcpy(bug_map.BootFilenameString, ptr->bootFile);
}


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
void writeNVram(BOOT_PARAMS *ptr)
{
    ppcbug_nvram bug_map;
    char buf[512], *cptr;
    
    /* read bug environment out */
    byteCopy((volatile char *) &bug_map, (volatile char *) NVRAM_BUGPARAMS, sizeof(bug_map));

    /* fill ppcbug structure */
    strcpy(buf, ptr->ead);
    if ((cptr = strchr(buf, ':')) != NULL) /* subnet mask found */
    {
        *cptr++ = 0; /* split inetaddr and subnet mask */
        bug_map.SubnetIPAddressMask = (uint32_t) myatoul(cptr);
    } else bug_map.SubnetIPAddressMask = DEFAULT_SUBNETMASK; /* default: 255.255.255.0 */
    bug_map.ClientIPAddress = addrToInt(buf);
    bug_map.ServerIPAddress = addrToInt(ptr->had);
    if (strlen(ptr->gad) > 0) 
        bug_map.GatewayIPAddress = addrToInt(ptr->gad);
    else
        bug_map.GatewayIPAddress = 0;

    ptr->bootFile[param_lengths[7] - 1] = 0; /* truncate bootfile length*/
    strcpy(bug_map.BootFilenameString, ptr->bootFile);
    bug_map.ArgumentFilenameString[0] = 0; /* delete argument */

    /* generate vxWorks boot string */
    sprintf(buf, "%s(%i,%i)%s:%s e=%s b=%s h=%s g=%s u=%s pw=%s f=0x%x tn=%s s=%s o=%s", \
        ptr->bootDev, ptr->unitNum, ptr->procNum, ptr->hostName, ptr->bootFile, \
        ptr->ead, ptr->bad, ptr->had, ptr->gad, ptr->usr, ptr->passwd, ptr->flags, \
        ptr->targetName, ptr->startupScript, ptr->other);
        
    /* write vxWorks parameter into NVRAM */
    byteCopy((volatile char *) NVRAM_VXPARAMS, (volatile char *) &buf, sizeof(buf));

    /* write bug environment back */
    byteCopy((volatile char *) NVRAM_BUGPARAMS, (volatile char *) &bug_map, sizeof(bug_map));
}
