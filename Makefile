#
#  $Id: Makefile,v 1.17 2008/05/02 05:48:03 strauman Exp $
#
# Templates/Makefile.lib
#       Template library Makefile
#

LIBNAME=libbootLib.a        # xxx- your library names goes here
LIB=${ARCH}/${LIBNAME}

C_PIECES=bootLib NVRAMaccess
# C and C++ source names, if any, go here -- minus the .c or .cc
C_FILES=$(C_PIECES:%=%.c)
C_O_FILES=$(C_PIECES:%=${ARCH}/%.o)

CC_PIECES=
CC_FILES=$(CC_PIECES:%=%.cc)
CC_O_FILES=$(CC_PIECES:%=${ARCH}/%.o)

H_FILES=bootLib.h NVRAMaccess.h bootLibGEV.h

# Assembly source names, if any, go here -- minus the .S
S_PIECES=
S_FILES=$(S_PIECES:%=%.S)
S_O_FILES=$(S_FILES:%.S=${ARCH}/%.o)

SRCS=$(C_FILES) $(CC_FILES) $(H_FILES) $(S_FILES)
OBJS=$(C_O_FILES) $(CC_O_FILES) $(S_O_FILES)

include $(RTEMS_MAKEFILE_PATH)/Makefile.inc

include $(RTEMS_CUSTOM)
ifdef RTEMS_SHARE
# this variable is defined in RTEMS 5
include $(RTEMS_SHARE)/make/lib.cfg
else
include $(RTEMS_ROOT)/make/lib.cfg
endif

ifeq ($(wildcard NVRAMaccess_$(RTEMS_BSP).c),)
C_PIECES+=NVRAMaccess_dummy
else
C_PIECES+=NVRAMaccess_$(RTEMS_BSP)
endif

ifeq ($(wildcard gev_$(RTEMS_BSP).c),)
C_PIECES+=gev_dummy
C_PIECES+=bootLibGEV_dummy
else
C_PIECES+=gev_$(RTEMS_BSP)
endif

#
# Add local stuff here using +=
#

DEFINES  +=
CPPFLAGS +=
CFLAGS   +=

#
# Add your list of files to delete here.  The config files
#  already know how to delete some stuff, so you may want
#  to just run 'make clean' first to see what gets missed.
#  'make clobber' already includes 'make clean'
#

CLEAN_ADDITIONS +=
CLOBBER_ADDITIONS +=

all:    ${ARCH} $(SRCS) $(LIB)

$(LIB): ${OBJS}
	$(make-library)

ifndef RTEMS_SITE_INSTALLDIR
RTEMS_SITE_INSTALLDIR = $(PROJECT_RELEASE)
INSTINCDIR=${RTEMS_SITE_INSTALLDIR}/lib/include/
else
INSTINCDIR=${RTEMS_SITE_INSTALLDIR}/include/
endif
INSTLIBDIR=${RTEMS_SITE_INSTALLDIR}/lib/

${INSTLIBDIR} \
${INSTINCDIR}/bsp:
	test -d $@ || mkdir -p $@

# Install the library, appending _g or _p as appropriate.
# for include files, just use $(INSTALL_CHANGE)
install:  all ${INSTLIBDIR} ${INSTINCDIR}/bsp
	$(INSTALL_VARIANT) -m 644 ${LIB} ${INSTLIBDIR}
	$(INSTALL_CHANGE) -m 644 ${H_FILES} ${INSTINCDIR}/bsp/




































