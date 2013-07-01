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
 *                     fuer Materialilien und Energie
 *                            Berlin, Germany
 *
 **************************************************************************-*/

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define DEFAULT_SUBNETMASK 0xFFFFFF00


/*+**************************************************************************
 *  helper functions
 **************************************************************************-*/

char *addrToStr(char *cbuf, uint32_t addr)
{
    struct in_addr a;

    if ((a.s_addr = addr) == 0)
        return NULL;
    return (char *)inet_ntop(AF_INET, &a, cbuf, INET_ADDRSTRLEN);
}


int addrToInt(char *cbuf)
{
    struct in_addr a;
    
    inet_pton(AF_INET, cbuf, &a);
    return a.s_addr;
}


unsigned int myatoul(char *ptr)
{
    unsigned int hi = 0;
    int offs = 0;
    char buf[5];
    
    if (strlen(ptr) > 4) /* read first word */
    {
        buf[4] = 0;
        strncpy(buf, ptr, 4);
        hi = strtol(buf, NULL, 16) << 16;
        offs = 4;
    }
    return (hi | (strtol(ptr + offs, NULL, 16) & 0xFFFF));
}


static void byteCopy(volatile char *dest, volatile char *src, int len)
{
    int i;
    for (i = len; i > 0; --i, ++dest, ++src) *dest = *src;
}


static void getsubstr(char *buf, char *dest, int maxlen, char *marker)
{
    int len;
    char *cptr, *eptr;
    
    cptr = strstr(buf, marker);
    if (cptr != NULL) 
    {
        cptr += strlen(marker);
        eptr = strchr(cptr, ' '); /* find field delimiter */
        if (eptr == NULL) eptr = strchr(cptr, 0); /* failed? looking for EOS */
        
        len = (int) (eptr - cptr); /* calc len */
        if (len > maxlen) len = maxlen; /* adjust size */
        if (len != 0) /* empty field */
            strncpy(dest, cptr, len);
    } else len = 0;
    dest[len] = 0;
}


#ifdef DEBUG
void printBootString(BOOT_PARAMS *ptr)
{
   /* generate vxWorks boot string */
   printf(buf, "%s(%i,%i)%s:%s e=%s b=%s h=%s g=%s u=%s pw=%s f=0x%x tn=%s s=%s o=%s", \
       ptr->bootDev, ptr->unitNum, ptr->procNum, ptr->hostName, ptr->bootFile, \
       ptr->ead, ptr->bad, ptr->had, ptr->gad, ptr->usr, ptr->passwd, ptr->flags, \
       ptr->targetName, ptr->startupScript, ptr->other);

}

#if 0
void memgrep(char *str, unsigned long base)
{
    int i = 1024, b = 256, len;
    volatile char *nvram = (char *) base;
    
    len = strlen(str);
    while (memcmp(nvram, str, len) != 0)
    {
        if (i == 1024) printf("searching 0x%X\n", base);
        ++nvram;
        ++base;
        if (i) --i; else i = 1024;
    }
    printf("%s found at 0x%X\n", str, base);

    printf("0x%X :", (unsigned long) (base - b));
    nvram = (char *) (base - b);
    for (i = 0; i < 512; ++i)
    {
        if (*nvram > 31) /* displayable charakter */
            printf("%c", *nvram);
        else printf(" ");

        ++nvram;
    }
}
#endif

#endif