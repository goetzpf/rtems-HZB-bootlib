/*+**************************************************************************
 *
 * Project:        RTEMS-GeSys
 * Module:         GEV boot parameters access
 * File:           NVRAMaccess_mvme5500.c
 *
 * Description:    Support for motLoad GEV access on MVME5500
 *
 * Author(s):      Dan Eichel
 *
 * Copyright (c) 2013     Helmholtz-Zentrum Berlin 
 *                     fuer Materialien und Energie
 *                            Berlin, Germany
 *
 **************************************************************************-*/

/* values for MVME5500 board */
#define GEV_START      (0xf1100000 + 0x10000 + 0x70F8)
#define GEV_SIZE       3592

#include "NVRAMaccess_GEV.c"
