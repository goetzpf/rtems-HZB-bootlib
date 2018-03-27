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

/* DO NOT change this struct, it is hard-coded in the ppc bug */
#define BUG_FILE_NAME_LEN 64
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
  char     BootFilenameString[BUG_FILE_NAME_LEN];
  char     ArgumentFilenameString[BUG_FILE_NAME_LEN];
} ppcbug_nvram;


static void
byteCopy (volatile char *dest, volatile char *src, int len)
{
  int i;
  for (i = len; i > 0; --i, ++dest, ++src)
    *dest = *src;
}

static void safe_strncpy(char *dest, const char *src, size_t n)
{
    strncpy(dest, src, n);
    dest[n-1] = 0;
}

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
  ppcbug_nvram bug_map;
  char buf[512], *cptr, *dot;

  /* read vxWorks boot parameters */
  byteCopy ((volatile char *) &buf, (volatile char *) NVRAM_VXPARAMS,
            sizeof (buf));
#if 0
  sprintf (buf,
           "%s(%i,%i)%s:%s e=%s b=%s h=%s g=%s u=%s pw=%s f=%i tn=%s s=%s o=%s",
           "dc", 0, 0, "vwhost", "/opt/tftfdir/vxWorks.st",
           "193.149.12.200:ffffff00", "", "193.149.12.29", "1", "ioc", "2",
           0x88, "box0", "/opt/test.st.cmd", "");
#endif
#if 0
  sprintf (buf,
           "%s:%s e=%s h=%s u=%s f=%i tn=%s s=%s o=%s",
           "vwhost", "/opt/tftfdir/vxWorks.st",
           "193.149.12.200:ffffff00", "193.149.12.29", "ioc",
           0x88, "box0", "/opt/test.st.cmd", "");
#endif
#if 0
  printf("\n\nvxBootStr: %s\n\n", buf);

#endif

  /* read ppcbug environment out */
  byteCopy ((volatile char *) &bug_map, (volatile char *) NVRAM_BUGPARAMS,
            sizeof (bug_map));

  /* find the first marker, tile string into two parts */ 
  dot = strchr (buf, ':');
  if (dot == NULL)
  {
    dot = buf;
    buf[1] = '\0'; /* invalidate whole bootstring */
  }
  *dot = '\0';
  
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
    /* if no information is present take defaults */
    safe_strncpy(ptr->bootDev, "dc", BOOT_DEV_LEN);
    ptr->unitNum = 1;
    ptr->procNum = 0;

    if (strlen(buf))
      sscanf (buf, "%s", ptr->hostName);
    else
      ptr->hostName[0] = '\0';
  }

  getsubstr (dot, ptr->bootFile, BOOT_FILE_LEN, ":");
  getsubstr (dot, ptr->ead, BOOT_TARGET_ADDR_LEN, "e=");    /* ead */
  getsubstr (dot, ptr->bad, BOOT_TARGET_ADDR_LEN, "b=");    /* bad */
  getsubstr (dot, ptr->had, BOOT_ADDR_LEN, "h=");    /* had */
  getsubstr (dot, ptr->gad, BOOT_ADDR_LEN, "g=");    /* gad */
  getsubstr (dot, ptr->usr, BOOT_USR_LEN, "u=");    /* usr */
  getsubstr (dot, ptr->passwd, BOOT_PASSWORD_LEN, "pw=");       /* passwd */
  getsubstr (dot, ptr->targetName, BOOT_HOST_LEN, "tn=");    /* targetName */
  getsubstr (dot, ptr->startupScript, BOOT_FILE_LEN, "s=");  /* startupScript */
  getsubstr (dot, ptr->other, BOOT_OTHER_LEN, "o="); /* other */

  cptr = strstr (dot, "f=");
  if (cptr != NULL)
    ptr->flags = strtol (cptr + 2, NULL, 16);
  else
    ptr->flags = 0;

  /* override with ppc bug settings, if present */
  bootlib_addrToStr (buf, bug_map.ClientIPAddress);
  safe_strncpy (ptr->ead, buf, BOOT_TARGET_ADDR_LEN);
/*  if (bug_map.SubnetIPAddressMask != DEFAULT_SUBNETMASK) */
    {
      sprintf (buf, ":%x", (unsigned int) bug_map.SubnetIPAddressMask);
      strncat (ptr->ead, buf, BOOT_TARGET_ADDR_LEN-strlen(ptr->ead)-1);
    }
  if (bootlib_addrToStr (buf, bug_map.ServerIPAddress) != NULL)
    {
      safe_strncpy (ptr->had, buf, BOOT_ADDR_LEN);
    }

  if (bug_map.GatewayIPAddress != 0)
    {
      if (bootlib_addrToStr (buf, bug_map.GatewayIPAddress) != NULL)
        safe_strncpy (ptr->gad, buf, BOOT_ADDR_LEN);
    }
  else
    ptr->gad[0] = 0;

  /* take always ppcbug bootfile, it cannot be empty else couldn't have booted */
  safe_strncpy (ptr->bootFile, bug_map.BootFilenameString, BOOT_FILE_LEN);
  if (bug_map.ArgumentFilenameString[0])
    safe_strncpy (ptr->startupScript, bug_map.ArgumentFilenameString, BOOT_FILE_LEN);
}


/*+**************************************************************************
 *
 * Function:    appendString
 *
 * Description: conditional buffer append string pair function
 *
 * Arg In:      buf    - pointer to string buffer
 *              prefix - string prefix
 *              force  - 1: append even if str is ''
 *                       0: append pair only if str isn't empty
 *              str    - string to append
 *
 * Return(s):   none
 *
 **************************************************************************-*/
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
      bug_map.SubnetIPAddressMask = strtoul(cptr, NULL, 16);
    }
  else
    bug_map.SubnetIPAddressMask = DEFAULT_SUBNETMASK;   /* default: 255.255.255.0 */
  bug_map.ClientIPAddress = bootlib_addrToInt (buf);
  bug_map.ServerIPAddress = bootlib_addrToInt (ptr->had);
  if (strlen (ptr->gad) > 0)
    bug_map.GatewayIPAddress = bootlib_addrToInt (ptr->gad);
  else
    bug_map.GatewayIPAddress = 0;

  safe_strncpy (bug_map.BootFilenameString, ptr->bootFile, BUG_FILE_NAME_LEN);
  safe_strncpy (bug_map.ArgumentFilenameString, ptr->startupScript, BUG_FILE_NAME_LEN);

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

  /* write vxWorks parameters into NVRAM */
  byteCopy ((volatile char *) NVRAM_VXPARAMS, (volatile char *) &buf,
            sizeof (buf));

  /* write bug environment back */
  byteCopy ((volatile char *) NVRAM_BUGPARAMS, (volatile char *) &bug_map,
            sizeof (bug_map));
}

void printBootString (BOOT_PARAMS * ptr)
{
  /* generate vxWorks boot string */
  printf ("%s(%i,%i)%s:%s e=%s b=%s h=%s g=%s u=%s pw=%s f=0x%x tn=%s s=%s o=%s",
          ptr->bootDev, ptr->unitNum, ptr->procNum, ptr->hostName,
          ptr->bootFile, ptr->ead, ptr->bad, ptr->had, ptr->gad, ptr->usr,
          ptr->passwd, ptr->flags, ptr->targetName, ptr->startupScript,
          ptr->other);
}
