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

#include "NVRAMaccess_GEV.c"
