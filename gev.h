#ifndef GEV_H
#define GEV_H

/* read/write the whole GEV to/from a user supplied buffer */

extern int write_gev(const char *src, size_t length);
extern int read_gev(char *dest, size_t length);

#endif /* GEV_H */
