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
 *                     fuer Materialien und Energie
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

typedef struct
{
  uint32_t PacketVersionIdentifier;
  uint32_t NodeControlMemoryAddress;
  uint32_t BootFileLoadAddress;
  uint32_t BootFileExecutionAddress;
  uint32_t BootFileExecutionDelay;
  uint32_t BootFileLength;
  uint32_t BootFileByteOffset;
  uint32_t TraceBufferAddress;
  uint32_t ClientIPAddress;
  uint32_t ServerIPAddress;
  uint32_t SubnetIPAddressMask;
  uint32_t BroadcastIPAddressMask;
  uint32_t GatewayIPAddress;
  uint8_t  BootpRarpRetry;
  uint8_t  TftpRarpRetry;
  uint8_t  BootpRarpControl;
  uint8_t  UpdateControl;
  char     BootFilenameString[64];
  char     ArgumentFilenameString[64];
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
void
readNVram (BOOT_PARAMS * ptr)
{
 /*
whost:/opt/tftpdir/IOC/vxBoot/5.4.2/mv2100/vxWorks.st e=192.168.21.225:ffffff00 h=192.168.21.141 u=ioc f=0x8 tn=IOC1X115B s=/opt/IOC/
*/
  ppcbug_nvram bug_map;
  char buf[512], *cptr, *dotpos;

  /* read vxWorks boot parameters */
  byteCopy ((volatile char *) &buf, (volatile char *) NVRAM_VXPARAMS,
            sizeof (buf));
#if 0
  sprintf (buf,
           "%s(%i,%i)%s:%s e=%s b=%s h=%s g=%s u=%s pw=%s f=%i tn=%s s=%s o=%s",
           "dc", 0, 0, "vwhost", "/opt/tftfdir/vxWorks.st",
           "193.149.12.200:ffffff00", "", "193.149.12.29", "1", "ioc", "2",
           0x88, "box0", "/opt/test.st.cmd", "");
  sprintf (buf,
           "%s:%s e=%s h=%s u=%s f=%i tn=%s s=%s o=%s",
           "vwhost", "/opt/tftfdir/vxWorks.st",
           "193.149.12.200:ffffff00", "193.149.12.29", "ioc",
           0x88, "box0", "/opt/test.st.cmd", "");
#endif

  /* read ppcbug environment out */
  byteCopy ((volatile char *) &bug_map, (volatile char *) NVRAM_BUGPARAMS,
            sizeof (bug_map));

  dotpos = strchr (buf, ':'); /* find the first marker, tile string into two parts */ 
  if (dotpos == NULL) 
  {
    dotpos = buf;
    buf[1] = '\0';
  }
  *dotpos = '\0';
  
  cptr = strchr (buf, '(');
  if (cptr != NULL)
  {
    *cptr = ' ';
  
    cptr = strchr (buf, ',');
    if (cptr != NULL) *cptr = ' ';
    
    cptr = strchr (buf, ')');
    if (cptr != NULL) *cptr = ' ';
    
    sscanf (buf, "%s %i %i %s",
          ptr->bootDev, (int *) &(ptr->unitNum), (int *) &(ptr->procNum),
          ptr->hostName);

  } else
  {
    strcpy(ptr->bootDev, "dc");
    ptr->unitNum = 1;
    ptr->procNum = 0;

    if (strlen(buf))
      sscanf (buf, "%s", ptr->hostName);
    else
      ptr->hostName[0] = '\0';
  }

  sscanf (dotpos + 1, "%s", ptr->bootFile);

  getsubstr (buf, ptr->ead, param_lengths[3], "e=");    /* ead */
  getsubstr (buf, ptr->bad, param_lengths[4], "b=");    /* bad */
  getsubstr (buf, ptr->had, param_lengths[5], "h=");    /* had */
  getsubstr (buf, ptr->gad, param_lengths[6], "g=");    /* gad */
  getsubstr (buf, ptr->usr, param_lengths[9], "u=");    /* usr */
  getsubstr (buf, ptr->passwd, param_lengths[10], "pw=");       /* passwd */
  getsubstr (buf, ptr->targetName, param_lengths[2], "tn=");    /* targetName */
  getsubstr (buf, ptr->startupScript, param_lengths[8], "s=");  /* startupScript */
  getsubstr (buf, ptr->other, param_lengths[11], "o="); /* other */

  cptr = strstr (buf, "f=");
  if (cptr != NULL)
    ptr->flags = strtol (cptr + 2, NULL, 16);
  else
    ptr->flags = 0;

  /* override with ppc bug settings, if present */
  bootlib_addrToStr (buf, bug_map.ClientIPAddress);

  strcpy (ptr->ead, buf);
  sprintf (buf, ":%x", (unsigned int) bug_map.SubnetIPAddressMask);
  strcat (ptr->ead, buf);

  if (bootlib_addrToStr (buf, bug_map.ServerIPAddress) != NULL)
    strcpy (ptr->had, buf);

  if (bootlib_addrToStr (buf, bug_map.GatewayIPAddress) != NULL)
    strcpy (ptr->gad, buf);

  strcpy (bug_map.BootFilenameString, ptr->bootFile);
}


static void
appendString(char *buf, char *prefix, int force, char *str)
{
  if ((force == 1) || (*str != '\0'))
    sprintf (buf + strlen(buf), " %s=%s", prefix, str);
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
void
writeNVram (BOOT_PARAMS * ptr)
{
  ppcbug_nvram bug_map;
  char buf[512], *cptr;

  /* read bug environment out */
  byteCopy ((volatile char *) &bug_map, (volatile char *) NVRAM_BUGPARAMS,
            sizeof (bug_map));

  /* fill ppcbug structure */
  strcpy (buf, ptr->ead);
  if ((cptr = strchr (buf, ':')) != NULL)       /* subnet mask found */
    {
      *cptr++ = 0;              /* split inetaddr and subnet mask */
      bug_map.SubnetIPAddressMask = (uint32_t) bootlib_atoul (cptr);
    }
  else
    bug_map.SubnetIPAddressMask = DEFAULT_SUBNETMASK;   /* default: 255.255.255.0 */
  bug_map.ClientIPAddress = bootlib_addrToInt (buf);
  bug_map.ServerIPAddress = bootlib_addrToInt (ptr->had);
  if (strlen (ptr->gad) > 0)
    bug_map.GatewayIPAddress = bootlib_addrToInt (ptr->gad);
  else
    bug_map.GatewayIPAddress = 0;

  ptr->bootFile[param_lengths[7] - 1] = 0;      /* truncate bootfile length */
  strcpy (bug_map.BootFilenameString, ptr->bootFile);
  bug_map.ArgumentFilenameString[0] = 0;        /* delete argument */

  /* generate vxWorks boot string */
  sprintf (buf, "%s(%i,%i)%s:%s",
           ptr->bootDev, ptr->unitNum, ptr->procNum, ptr->hostName, ptr->bootFile);

  appendString(buf, "e", 1, ptr->ead);
  appendString(buf, "b", 0, ptr->bad);
  appendString(buf, "h", 1, ptr->had);
  appendString(buf, "g", 0, ptr->gad);
  appendString(buf, "u", 1, ptr->usr);
  appendString(buf, "pw",0, ptr->passwd);
  sprintf (buf + strlen(buf), " f=0x%x", ptr->flags);
  appendString(buf, "tn",1, ptr->targetName);
  appendString(buf, "s", 1, ptr->startupScript);
  appendString(buf, "o", 0, ptr->other);

  /* write vxWorks parameter into NVRAM */
  byteCopy ((volatile char *) NVRAM_VXPARAMS, (volatile char *) &buf,
            sizeof (buf));

  /* write bug environment back */
  byteCopy ((volatile char *) NVRAM_BUGPARAMS, (volatile char *) &bug_map,
            sizeof (bug_map));
}
