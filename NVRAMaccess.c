/*+**************************************************************************
 *
 * Project:        RTEMS-GeSys
 * Module:         NVRAM boot parameters access
 * File:           NVRAMaccess.c
 *
 * Description:    some useful helper functions for
 *                 bootString / bootStruct handling
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

#include <sys/socket.h>
#include <arpa/inet.h>


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
char *
bootlib_addrToStr (char *cbuf, uint32_t addr)
{
  struct in_addr a;

  if ((a.s_addr = addr) == 0)
    return NULL;
  return (char *) inet_ntop (AF_INET, &a, cbuf, INET_ADDRSTRLEN);
}


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
int
bootlib_addrToInt (char *cbuf)
{
  struct in_addr a;

  inet_pton (AF_INET, cbuf, &a);
  return a.s_addr;
}

/*+**************************************************************************
 *  internal helper functions
 **************************************************************************-*/

void
getsubstr (char *buf, char *dest, int maxlen, char *marker)
{
  int len;
  char *cptr, *eptr;

  cptr = strstr (buf, marker);
  if (cptr != NULL)
    {
      cptr += strlen (marker);
      eptr = strchr (cptr, ' ');        /* find field delimiter */
      if (eptr == NULL)
        eptr = strchr (cptr, 0);        /* failed? looking for EOS */

      len = (int) (eptr - cptr);        /* calc len */
      if (len > maxlen)
        len = maxlen;           /* adjust size */
      if (len != 0)             /* empty field */
        strncpy (dest, cptr, len);
        dest[len-1] = 0;
    }
  else
    len = 0;
  dest[len] = 0;
}

char *
cvrtsmask (char *str, char *dest)
{
  char tmp[9];
  int a = 0, b = 0, c = 0, d = 0;

  sscanf (str, "%i.%i.%i.%i", &a, &b, &c, &d);
  sprintf (tmp, "%02x%02x%02x%02x", a, b, c, d);
  if (dest == NULL)
    return strdup (tmp);
  strcpy (dest, tmp);
  return dest;
}
