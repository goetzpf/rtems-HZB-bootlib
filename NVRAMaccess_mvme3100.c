/*+**************************************************************************
 *
 * Project:        RTEMS-GeSys
 * Module:         GEV boot parameters access
 * File:           NVRAMaccess_mvme3100.c
 *
 * Description:    Support for motLoad GEV access on MVME3100
 *
 * Author(s):      Dan Eichel
 *
 * Copyright (c) 2013     Helmholtz-Zentrum Berlin 
 *                     fuer Materialien und Energie
 *                            Berlin, Germany
 *
 **************************************************************************-*/

#include <fcntl.h>                 /* open                                     */
#include <bsp.h>                   /* BSP_I2C_VPD_EEPROM_DEV_NAME              */
#include <rtems/libio.h>           /* rtems_filesystem_dev_mxxor_t             */
#include <libchip/i2c-2b-eeprom.h> /* i2c_2b_eeprom_driver_descriptor          */

/* values for MVME3100 board */
#define BSP_I2C_VPD_EEPROM_OFFSET 0x10f8
#define GEV_SIZE                  3592
#define MOTSCRIPT_PART1           "tftpGet -d/dev/"
#define MOTSCRIPT_PART2           " -a0x40000\nnetShut\ngo -a0x40000"

#define WRITEBACKFUNCTION writeGEVback
static void
writeGEVback (char *ptr)
{
  int fd;
  struct stat buf;
  rtems_device_major_number major;
  rtems_device_minor_number minor;
  rtems_libio_rw_args_t rwargs;

  /*
     dirty hack : 
     Unfortunally the access to the VPD eeprom is implemented as read only,
     that means the write function is undefined.
     But in a first step we can stat the  device and retrive the major
     and minor number of the device in the device driver table. In a
     second step we call direct a write function for a similar i2c eeprom
     device with the retrieved values from the fstat call...
   */
  if ((fd = open (BSP_I2C_VPD_EEPROM_DEV_NAME, 0)) < 0)
    return;
  /* fetch the major / minor number from the registered device driver */
  fstat (fd, &buf);

  major = rtems_filesystem_dev_major_t (buf.st_dev);
  minor = rtems_filesystem_dev_minor_t (buf.st_dev);

  rwargs.offset = BSP_I2C_VPD_EEPROM_OFFSET;
  rwargs.count = GEV_SIZE;
  rwargs.buffer = ptr;

  BSP_BOOTPARMS_WRITE_ENABLE ();
  i2c_2b_eeprom_driver_descriptor->ops->write_entry (major, minor,
                                                     (void *) &rwargs);
  BSP_BOOTPARMS_WRITE_DISABLE ();

  /* close the device at this point, so we can ensure the bus is still
     up and running */
  close (fd);
}

#include "NVRAMaccess_GEV.c"
