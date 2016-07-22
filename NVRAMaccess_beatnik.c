/*+**************************************************************************
 *
 * Project:        RTEMS-GeSys
 * Module:         GEV boot parameters access
 * File:           NVRAMaccess_beatnik.c
 *
 * Description:    Support for motLoad GEV access on MVME5500/MVME6100
 *
 * Author(s):      Dan Eichel
 *
 * Copyright (c) 2013     Helmholtz-Zentrum Berlin 
 *                     fuer Materialien und Energie
 *                            Berlin, Germany
 *
 **************************************************************************-*/

/* values for MVME5500 board */
#define GEV_START        (0xf1100000 + 0x10000 + 0x70F8)
#define GEV_SIZE         3592
#define MOTSCRIPT_PART1  "dla=malloc 3000000\ntftpGet -d/dev/"
#define MOTSCRIPT_PART2  " -adla\nnetShut\ngo -adla"

#include "NVRAMaccess_GEV.c"
