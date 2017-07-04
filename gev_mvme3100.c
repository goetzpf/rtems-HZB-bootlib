#include <fcntl.h>                      /* open                             */
#include <bsp.h>                        /* BSP_I2C_VPD_EEPROM_DEV_NAME      */
#include <rtems/libio.h>                /* rtems_filesystem_dev_mxxor_t     */
#include <libchip/i2c-2b-eeprom.h>      /* i2c_2b_eeprom_driver_descriptor  */

#define BSP_I2C_VPD_EEPROM_OFFSET 0x10f8

int write_gev(const char *src, size_t length)
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
       But in a first step we can stat the device and retrive the major
       and minor number of the device in the device driver table. In a
       second step we call direct a write function for a similar i2c eeprom
       device with the retrieved values from the fstat call...
     */
    if ((fd = open(BSP_I2C_VPD_EEPROM_DEV_NAME, 0)) < 0) {
        fprintf(stderr,"Can't open %s\n", BSP_I2C_VPD_EEPROM_DEV_NAME);
        return -1;
    }
    /* fetch the major / minor number from the registered device driver */
    if (!fstat(fd, &buf)) {
        fprintf(stderr,"Can't fstat %s\n", BSP_I2C_VPD_EEPROM_DEV_NAME);
        return -1;
    }

    major = rtems_filesystem_dev_major_t(buf.st_dev);
    minor = rtems_filesystem_dev_minor_t(buf.st_dev);

    rwargs.offset = BSP_I2C_VPD_EEPROM_OFFSET;
    rwargs.count = length;
    rwargs.buffer = src;

    BSP_BOOTPARMS_WRITE_ENABLE();
    i2c_2b_eeprom_driver_descriptor->ops->write_entry(major, minor, &rwargs);
    BSP_BOOTPARMS_WRITE_DISABLE();

    /* close the device at this point, so we can ensure the bus is still
       up and running */
    close(fd);
    return 0;
}

int read_gev(char *dest, size_t length)
{
    if ((fd = open(BSP_I2C_VPD_EEPROM_DEV_NAME, 0)) < 0) {
        fprintf(stderr,"Can't open %s\n", BSP_I2C_VPD_EEPROM_DEV_NAME);
        return -1;
    }
    if (lseek(fd, BSP_I2C_VPD_EEPROM_OFFSET, SEEK_SET)<0) {
        fprintf(stderr,"Can't lseek %s\n", BSP_I2C_VPD_EEPROM_DEV_NAME);
        return -1;
    }
    if (read(fd, dest, length) != sizeof gev_buf) {
        fprintf(stderr,"Can't read %s\n", BSP_I2C_VPD_EEPROM_DEV_NAME);
        return -1;
    }
    close(fd);
    return 0;
}
