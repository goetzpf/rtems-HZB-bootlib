/*+**************************************************************************
 *
 * Project:        RTEMS-GeSys
 * Module:         GEV boot parameters access
 * File:           NVRAMaccess_GEV.c
 *
 * Description:    Support for motLoad GEV access
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

#if !(defined(GEV_START) || defined(BSP_I2C_VPD_EEPROM_DEV_NAME)) && !defined(GEV_SIZE)
#error "GEV startaddress / size for target arch not defined!"
#endif

#define knownItems 23

typedef struct nvpair
{
  char *name;
  char *value;
  struct nvpair *nextitem;
} nvpair;

static struct
{
  nvpair motscript;

  nvpair enet0cipa;
  nvpair enet0sipa;
  nvpair enet0gipa;
  nvpair enet0snma;
  nvpair enet0file;

  nvpair enet1cipa;
  nvpair enet1sipa;
  nvpair enet1gipa;
  nvpair enet1snma;
  nvpair enet1file;

  nvpair dns_server;
  nvpair dns_name;
  nvpair client_name;
  nvpair epics_script;
  nvpair epics_nfsmount;
  nvpair epics_ntpserver;
  nvpair epics_tz;

  nvpair rsh_user;
  nvpair tftp_pw;
  nvpair boot_flags;
  nvpair host_name;

  nvpair otherItems;

} GEVstruct;

static nvpair *GEVptr[knownItems] = {
  &GEVstruct.motscript,
  &GEVstruct.enet0cipa,
  &GEVstruct.enet0sipa,
  &GEVstruct.enet0gipa,
  &GEVstruct.enet0snma,
  &GEVstruct.enet0file,
  &GEVstruct.enet1cipa,
  &GEVstruct.enet1sipa,
  &GEVstruct.enet1gipa,
  &GEVstruct.enet1snma,
  &GEVstruct.enet1file,
  &GEVstruct.dns_server,
  &GEVstruct.dns_name,
  &GEVstruct.client_name,
  &GEVstruct.epics_script,
  &GEVstruct.epics_nfsmount,
  &GEVstruct.epics_ntpserver,
  &GEVstruct.epics_tz,
  &GEVstruct.rsh_user,
  &GEVstruct.tftp_pw,
  &GEVstruct.host_name,
  &GEVstruct.boot_flags,
  &GEVstruct.otherItems
};

static char *GEVhash[knownItems - 1] = {
  "mot-script-boot",
  "mot-/dev/enet0-cipa",
  "mot-/dev/enet0-sipa",
  "mot-/dev/enet0-gipa",
  "mot-/dev/enet0-snma",
  "mot-/dev/enet0-file",
  "mot-/dev/enet1-cipa",
  "mot-/dev/enet1-sipa",
  "mot-/dev/enet1-gipa",
  "mot-/dev/enet1-snma",
  "mot-/dev/enet1-file",
  "rtems-dns-server",
  "rtems-dns-domainname",
  "rtems-client-name",
  "epics-script",
  "epics-nfsmount",
  "epics-ntpserver",
  "epics-tz",
  "rsh-user",
  "tftp-pw",
  "host-name",
  "boot-flags"
};

/* include private helper functions... */
#include "NVRAMaccess.c"

static int
getIndex (const char *str)
{
  int i;                        /*, items = sizeof(GEVhash) / sizeof(GEVhash[0]); */

  for (i = 0; i < knownItems - 1; ++i)
    if (strcmp (GEVhash[i], str) == 0)
      return i;
  return knownItems - 1;        /* otheritem */
}

#if 0
static int
kompareStrList (char *str, char *strlist)
{
  char *sptr = strlist, *eptr = strchr (strlist, ',');
  int len, slen = strlen (str);

  if (eptr == NULL)
    len = strlen (strlist);
  else
    len = (eptr - sptr);

  while ((sptr != NULL) && (*sptr != '\0'))
    {
      if ((slen == len) && (strncmp (str, sptr, len) == 0))
        return 1;
      sptr += len + 1;
      eptr = strchr (sptr, ',');
      if (eptr == NULL)
        len = strlen (sptr);
      else
        len = (eptr - sptr);
    }
  return 0;
}
#endif

static void
freeGEVstruct (void)
{
  int i;
  for (i = 0; i < knownItems; ++i)
    {
      if (GEVptr[i]->name != NULL)
        free (GEVptr[i]->name);
      GEVptr[i]->name = NULL;
      if (GEVptr[i]->value != NULL)
        free (GEVptr[i]->value);
      GEVptr[i]->value = NULL;
      if (GEVptr[i]->nextitem != NULL)
        {
          nvpair *delme, *ptr = GEVptr[i];
          while (ptr != NULL)
            {
              if (ptr->name != NULL)
                free (ptr->name);
              ptr->name = NULL;
              if (ptr->value != NULL)
                free (ptr->value);
              ptr->value = NULL;
              delme = ptr;
              ptr = ptr->nextitem;
              if (delme != GEVptr[i])
                free (delme);
            }
        }
      GEVptr[i]->nextitem = NULL;
    }
}


#ifdef DEBUG
static void
printGEVstruct (void)
{
  int i;
  for (i = 0; i < knownItems; ++i)
    {
      if (GEVptr[i]->name != NULL)
        printf ("#%i: %s=", i, GEVptr[i]->name);
      if (GEVptr[i]->value != NULL)
        printf ("\"%s\"\n", GEVptr[i]->value);
      if (GEVptr[i]->nextitem != NULL)
        {
          nvpair *ptr = GEVptr[i];
          while (ptr != NULL)
            {
              if (ptr->name != NULL)
                printf ("#%i [nonregistered] %s=", i, ptr->name);
              if (ptr->value != NULL)
                printf ("\"%s\"\n", ptr->value);
              ptr = ptr->nextitem;
            }
        }
    }
}
#endif


static void
fillGEVstruct (void)
{
  volatile char *ptr = NULL;
  char c, *key = NULL, *value = NULL;
  int modus = 0, index;

#if defined(GEV_START)
  volatile char *nvp = (volatile char *) (GEV_START);
#else
  volatile char *nvp;
  char gev_buf[GEV_SIZE];
  int fd;
#endif

#if defined(BSP_I2C_VPD_EEPROM_DEV_NAME)
  if ((fd = open (BSP_I2C_VPD_EEPROM_DEV_NAME, 0)) < 0)
    {
      printf ("Can't open %s\n", BSP_I2C_VPD_EEPROM_DEV_NAME);
      return;
    }
  lseek (fd, BSP_I2C_VPD_EEPROM_OFFSET, SEEK_SET);
  if (read (fd, gev_buf, sizeof gev_buf) != sizeof gev_buf)
    {
      printf ("Can't read %s\n", BSP_I2C_VPD_EEPROM_DEV_NAME);
      return;
    }
  close (fd);
  nvp = gev_buf;
#elif !defined(GEV_START)
#error "No way to read GEV!"
#endif

  memset ((void *) &GEVstruct, 0, sizeof (GEVstruct));
  if (*nvp == '\0')
    return;
  while (((c = *nvp) != '\0') || (modus == 2))  /* read til last variable */
    {
      switch (modus)
        {
        case 0:                /* search key */
          if (c != '\0')        /* found */
            {
              ptr = (char *) nvp;       /* save key addr */
              modus = 1;
            }
          break;
        case 1:                /* search value */
          if (c == '=')         /* field delimiter */
            {
              key = malloc (nvp - ptr + 1);
              byteCopy ((volatile char *) key, (volatile char *) ptr,
                        (int) (nvp - ptr));
              key[nvp - ptr] = 0;

              ptr = nvp + 1;
              modus = 2;
            }
          break;
        default:               /* search values end */
          if (c == '\0')
            {
              value = malloc (nvp - ptr + 1);
              byteCopy ((volatile char *) value, (volatile char *) ptr,
                        nvp - ptr + 1);
              modus = 0;        /* search next key/value pair */

#if 0
              if (kompareStrList (key, "epics-nfsmount,rtems-client-name") == 1)        /* is key in blacklist? */
                {               /* don't store GEV, search next one */
                  free (key);
                  free (value);
                  break;
                }
#endif
              /* store data */
              index = getIndex (key);
              if (index < knownItems - 1)
                {
                  GEVptr[index]->name = key;
                  GEVptr[index]->value = value;
                  GEVptr[index]->nextitem = NULL;
#ifdef DEBUG
                  printf ("registered GEV #%i found: %s=\"%s\"\n",
                          index, GEVptr[index]->name, GEVptr[index]->value);
#endif
                }
              else
                {
                  nvpair *root = GEVptr[index], *newnode;
#ifdef DEBUG
                  printf ("unknown GEV(#%i) found: %s=\"%s\" [",
                          index, key, value);
#endif

                  if (root->name == NULL)
                    {
                      root->name = key;
                      root->value = value;
                      root->nextitem = NULL;
                    }
                  else
                    {
                      newnode = malloc (sizeof (nvpair));
                      newnode->name = key;
                      newnode->value = value;
                      newnode->nextitem = NULL;
#ifdef DEBUG
                      printf ("->");
#endif
                      while (root != NULL)
                        {
                          if (root->nextitem != NULL)
                            {
#ifdef DEBUG
                              printf ("->");
#endif
                              root = root->nextitem;
                            }
                          else
                            break;
                        }
                      root->nextitem = newnode;
                    }
#ifdef DEBUG
                  printf ("o]\n");
#endif
                }
            }
          break;
        }
      ++nvp;
    }
}

static void
flushGEVstruct (void)
{
  nvpair *ptr;
  int i;
#if defined(GEV_START)
  volatile char *nvp = (volatile char *) (GEV_START);
  char *endaddr = (char *) (GEV_START + GEV_SIZE);
#else
  char gev_buf[GEV_SIZE];
  char *nvp = (char *) &gev_buf;
  char *endaddr = (char *) (nvp + GEV_SIZE);
#endif

  for (i = 0; i < knownItems; ++i)
    {
      if (GEVptr[i]->name != NULL)
        {
          byteCopy (nvp, (volatile char *) GEVptr[i]->name,
                    strlen (GEVptr[i]->name));
          nvp += strlen (GEVptr[i]->name);
          *nvp++ = '=';
          if (GEVptr[i]->value != NULL)
            {
              byteCopy (nvp, (volatile char *) GEVptr[i]->value,
                        strlen (GEVptr[i]->value) + 1);
              nvp += strlen (GEVptr[i]->value) + 1;
            }
          else
            *nvp++ = '\0';
        }

      ptr = GEVptr[i]->nextitem;
      while (ptr != NULL)
        {
          if (ptr->name != NULL)
            {
              byteCopy (nvp, (volatile char *) ptr->name, strlen (ptr->name));
              nvp += strlen (ptr->name);
              *nvp++ = '=';
              if (ptr->value != NULL)
                {
                  byteCopy (nvp, (volatile char *) ptr->value,
                            strlen (ptr->value) + 1);
                  nvp += strlen (ptr->value) + 1;
                }
              else
                *nvp++ = '\0';
            }
          ptr = ptr->nextitem;
        }
    }
  while (nvp < endaddr)
    *nvp++ = 0x0;               /* fill NVRAM with zeros */
#ifdef WRITEBACKFUNCTION
  WRITEBACKFUNCTION ((char *) &gev_buf);
#endif
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
  char buf[32];
  int bootdev;

  fillGEVstruct ();

  if (GEVstruct.motscript.name != NULL) /* get boot device */
    getsubstr (GEVstruct.motscript.value, ptr->bootDev, param_lengths[0],
               "-d/dev/");
  else
    strcpy (ptr->bootDev, "enet0");

  ptr->unitNum = bootlib_atoul ((char *) (ptr->bootDev + 4));   /* parse unit number */
  ptr->bootDev[4] = 0;          /* adjust bootdevice */

/* begin offset area */
  if (ptr->unitNum == 1)
    bootdev = 6;
  else
    bootdev = 1;                /* trim offset */

  if (GEVptr[bootdev]->name != NULL)    /* get client IP */
    strcpy (ptr->ead, GEVptr[bootdev]->value);
  else
    getsubstr (GEVstruct.motscript.value, ptr->ead, param_lengths[3], "-c");

  if (GEVstruct.client_name.name != NULL)       /* get client name */
    strcpy (ptr->targetName, GEVstruct.client_name.value);
  else
    strcpy (ptr->targetName, ptr->ead);

  strcpy (ptr->bad, "");        /* not used: backplane IP addr */
  ptr->procNum = 0;             /* allways processor number 0 */

  if (GEVptr[bootdev + 1]->name != NULL)        /* get host IP */
    strcpy (ptr->had, GEVptr[bootdev + 1]->value);
  else
    getsubstr (GEVstruct.motscript.value, ptr->had, param_lengths[5], "-s");

  if (GEVptr[bootdev + 2]->name != NULL)        /* get gateway IP */
    strcpy (ptr->gad, GEVptr[bootdev + 2]->value);
  else
    getsubstr (GEVstruct.motscript.value, ptr->gad, param_lengths[6], "-g");

  if (GEVptr[bootdev + 3]->name != NULL)        /* get subnetmask */
    strcpy (buf, GEVptr[bootdev + 3]->value);
  else
    getsubstr (GEVstruct.motscript.value, buf, sizeof (buf), "-m");
  cvrtsmask (buf, buf + 1);
  buf[0] = ':';
  strcat (ptr->ead, buf);

  if (GEVptr[bootdev + 4]->name != NULL)        /* get bootfile */
    strcpy (ptr->bootFile, GEVptr[bootdev + 4]->value);
  else
    getsubstr (GEVstruct.motscript.value, ptr->bootFile, param_lengths[7],
               "-f");
/* offset area end */

  if (GEVstruct.epics_script.name != NULL)      /* get startup script */
    strcpy (ptr->startupScript, GEVstruct.epics_script.value);
  else
    strcpy (ptr->startupScript, "");

  if (GEVstruct.epics_nfsmount.name != NULL)    /* get nfs server mount */
    strcpy (ptr->other, GEVstruct.epics_nfsmount.value);
  else
    strcpy (ptr->other, "");

  if (GEVstruct.rsh_user.name != NULL)  /* get user name */
    strcpy (ptr->usr, GEVstruct.rsh_user.value);
  else
    strcpy (ptr->usr, "");

  if (GEVstruct.tftp_pw.name != NULL)   /* get password */
    strcpy (ptr->passwd, GEVstruct.tftp_pw.value);
  else
    strcpy (ptr->passwd, "");

  if ((GEVstruct.boot_flags.name != NULL) && (GEVstruct.boot_flags.value != NULL) && (strlen (GEVstruct.boot_flags.value) > 2)) /* get bootflags */
    ptr->flags = strtol (GEVstruct.boot_flags.value + 2, NULL, 16);
  else
    ptr->flags = 0;

  if (GEVstruct.host_name.name != NULL) /* get host_name */
    strcpy (ptr->hostName, GEVstruct.host_name.value);
  else
    strcpy (ptr->hostName, "");
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
  char *cptr, buf[BOOT_DEV_LEN + 5];    /* unitnum is int - max 5 char - 65536 */
  int bootdev, i, script_corrupted = 0;

  if (ptr->unitNum != 1)
    ptr->unitNum = 0;
  bootdev = ptr->unitNum * 5 + 1;
  cptr = strchr (ptr->ead, ':');

/* begin offset area */
  if (GEVptr[bootdev]->name == NULL)    /* set client IP */
    GEVptr[bootdev]->name = strdup (GEVhash[bootdev]);
  if (GEVptr[bootdev]->value != NULL)
    free (GEVptr[bootdev]->value);
  if (cptr != NULL)
    GEVptr[bootdev]->value = strndup (ptr->ead, (int) (cptr - ptr->ead));
  else
    GEVptr[bootdev]->value = strdup (ptr->ead);

  if (GEVptr[bootdev + 1]->name == NULL)        /* set host IP */
    GEVptr[bootdev + 1]->name = strdup (GEVhash[bootdev + 1]);
  if (GEVptr[bootdev + 1]->value != NULL)
    free (GEVptr[bootdev + 1]->value);
  GEVptr[bootdev + 1]->value = strdup (ptr->had);

  if (GEVptr[bootdev + 2]->name == NULL)        /* set gateway IP */
    GEVptr[bootdev + 2]->name = strdup (GEVhash[bootdev + 2]);
  if (GEVptr[bootdev + 2]->value != NULL)
    free (GEVptr[bootdev + 2]->value);
  GEVptr[bootdev + 2]->value = strdup (ptr->gad);

  if (GEVptr[bootdev + 3]->name == NULL)        /* set subnet mask */
    GEVptr[bootdev + 3]->name = strdup (GEVhash[bootdev + 3]);
  if (GEVptr[bootdev + 3]->value != NULL)
    free (GEVptr[bootdev + 3]->value);
  if (cptr == NULL)
    GEVptr[bootdev + 3]->value = strdup (DEFAULT_SUBNETMASK_STR);
  else
    {
      unsigned long addr = bootlib_atoul (cptr + 1);

      GEVptr[bootdev + 3]->value = malloc (16);
      sprintf (GEVptr[bootdev + 3]->value, "%lu.%lu.%lu.%lu",
               (addr >> 24), (addr >> 16) & 0xFF, (addr >> 8) & 0xFF,
               addr & 0xFF);
    }

  if (GEVptr[bootdev + 4]->name == NULL)        /* set bootfile */
    GEVptr[bootdev + 4]->name = strdup (GEVhash[bootdev + 4]);
  if (GEVptr[bootdev + 4]->value != NULL)
    free (GEVptr[bootdev + 4]->value);
  GEVptr[bootdev + 4]->value = strdup (ptr->bootFile);

  bootdev = (1 - ptr->unitNum) * 5 + 1; /* estimate the other unused IP device offset */
  for (i = 0; i < 5; ++i)       /* clear client, host, gateway IP, subnetmask and bootfile  */
    {
      if (GEVptr[bootdev + i]->name != NULL)
        {
          free (GEVptr[bootdev + i]->name);
          GEVptr[bootdev + i]->name = NULL;
          if (GEVptr[bootdev + i]->value != NULL)
            {
              free (GEVptr[bootdev + i]->value);
              GEVptr[bootdev + i]->value = NULL;
            }
        }
    }
/* offset area end */

  if (GEVstruct.client_name.name == NULL)       /* set client name */
    GEVstruct.client_name.name = strdup (GEVhash[13]);
  if (GEVstruct.client_name.value != NULL)
    free (GEVstruct.client_name.value);
  GEVstruct.client_name.value = strdup (ptr->targetName);

  if (GEVstruct.epics_script.name == NULL)      /* set script */
    GEVstruct.epics_script.name = strdup (GEVhash[14]);
  if (GEVstruct.epics_script.value != NULL)
    free (GEVstruct.epics_script.value);
  GEVstruct.epics_script.value = strdup (ptr->startupScript);

  if (GEVstruct.epics_nfsmount.name == NULL)    /* set other */
    GEVstruct.epics_nfsmount.name = strdup (GEVhash[15]);
  if (GEVstruct.epics_nfsmount.value != NULL)
    free (GEVstruct.epics_nfsmount.value);
  GEVstruct.epics_nfsmount.value = strdup (ptr->other);

  if (GEVstruct.rsh_user.name == NULL)  /* set rsh_user */
    GEVstruct.rsh_user.name = strdup (GEVhash[18]);
  if (GEVstruct.rsh_user.value != NULL)
    free (GEVstruct.rsh_user.value);
  GEVstruct.rsh_user.value = strdup (ptr->usr);

  if (GEVstruct.tftp_pw.name == NULL)   /* set PW */
    GEVstruct.tftp_pw.name = strdup (GEVhash[19]);
  if (GEVstruct.tftp_pw.value != NULL)
    free (GEVstruct.tftp_pw.value);
  GEVstruct.tftp_pw.value = strdup (ptr->passwd);

  if (GEVstruct.host_name.name == NULL) /* set hostname */
    GEVstruct.host_name.name = strdup (GEVhash[20]);
  if (GEVstruct.host_name.value != NULL)
    free (GEVstruct.host_name.value);
  GEVstruct.host_name.value = strdup (ptr->hostName);

  if (GEVstruct.boot_flags.name == NULL)        /* set bootflags */
    GEVstruct.boot_flags.name = strdup (GEVhash[21]);
  if (GEVstruct.boot_flags.value != NULL)
    free (GEVstruct.boot_flags.value);
  GEVstruct.boot_flags.value = malloc (8);
  sprintf (GEVstruct.boot_flags.value, "0x%i", ptr->flags);

  /* patch motscript */
  sprintf (buf, "%s%i", ptr->bootDev, ptr->unitNum);
  if (GEVstruct.motscript.value != NULL)
    {
      char *tftpline = strstr (GEVstruct.motscript.value, "tftpGet");   /* go to the importend line for us... */

      if (tftpline != NULL)
        {
          killargs (tftpline, "csgmf"); /* delete args (-c, -s, -g, -m -f) */

          if ((cptr = strstr (tftpline, "-d/dev/")) != NULL)
            {
              char *eptr, *tptr;
              int space;

              cptr += 7;        /* point behind -d/dev/ */
              /* find field delimiter */
              eptr = strchr (cptr, '\0');
              if (((tptr = strchr (cptr, '\n')) != NULL) && (tptr < eptr))
                eptr = tptr;
              if (((tptr = strchr (cptr, ' ')) != NULL) && (tptr < eptr))
                eptr = tptr;

              space = (int) (eptr - cptr);      /* calculate space for boot device */
              if (space == strlen (buf))        /* puuh: best case, only override bytes */
                memcpy (cptr, buf, strlen (buf));
              else if (space > strlen (buf))    /* memmove - we have more space then we need */
                {
                  memcpy (cptr, buf, strlen (buf));
                  memmove (cptr + strlen (buf), eptr, strlen (eptr));
                }
              else              /* we have to enlarge the buffer */
                {
                  /* this buffer is in every case big enough... */
                  char *motscript =
                    malloc (strlen (GEVstruct.motscript.value) +
                            strlen (buf) + 1);

                  strncpy (motscript, GEVstruct.motscript.value,
                           (int) (cptr - GEVstruct.motscript.value));
                  strcat (motscript, buf);
                  strcat (motscript, eptr);

                  free (GEVstruct.motscript.value);
                  GEVstruct.motscript.value = motscript;
                }
            }
          else
            script_corrupted = 1;       /* this can only reached if the -d argument is missed - create new script: */
        }
      else
        script_corrupted = 1;   /* how could RTEMS loaded without tftpGet command? Anyway, overwrite script! */
    }
  if ((GEVstruct.motscript.name == NULL) || (script_corrupted)) /* set new motscript */
    {
      char part1[] = MOTSCRIPT_PART1;
      char part2[] = MOTSCRIPT_PART2;

      if (GEVstruct.motscript.name != NULL)
        free (GEVstruct.motscript.name);
      GEVstruct.motscript.name = strdup (GEVhash[0]);
      if (GEVstruct.motscript.value != NULL)
        free (GEVstruct.motscript.value);
      GEVstruct.motscript.value =
        malloc (strlen (part1) + strlen (buf) + strlen (part2) + 1);
      strcpy (GEVstruct.motscript.value, part1);
      strcat (GEVstruct.motscript.value, buf);
      strcat (GEVstruct.motscript.value, part2);
    }

  flushGEVstruct ();
  freeGEVstruct ();
}
