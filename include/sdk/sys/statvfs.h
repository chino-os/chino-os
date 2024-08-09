// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.
#pragma once
#include <sys/types.h>

#define ST_RDONLY 0x0001 /* Mount read-only.  */
#define ST_NOSUID 0x0002 /* Ignore suid and sgid bits.  */

#if defined(CONFIG_FS_LARGEFILE)
#define statvfs64 statvfs
#define fstatvfs64 fstatvfs
#endif

/****************************************************************************
 * Type Definitions
 ****************************************************************************/

struct statvfs {
    unsigned long f_bsize;   /* File system block size */
    unsigned long f_frsize;  /* Fundamental file system block size */
    fsblkcnt_t f_blocks;     /* Total number of blocks on file system in
                              * units of f_frsize */
    fsblkcnt_t f_bfree;      /* Total number of free blocks */
    fsblkcnt_t f_bavail;     /* Number of free blocks available to
                              * non-privileged process */
    fsfilcnt_t f_files;      /* Total number of file serial numbers */
    fsfilcnt_t f_ffree;      /* Total number of free file serial numbers */
    fsfilcnt_t f_favail;     /* Number of file serial numbers available to
                              * non-privileged process */
    unsigned long f_fsid;    /* File system ID */
    unsigned long f_flag;    /* Bit mask of f_flag values */
    unsigned long f_namemax; /* Maximum filename length */
};

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C" {
#else
#define EXTERN extern
#endif

int statvfs(const char *path, struct statvfs *buf);
int fstatvfs(int fd, struct statvfs *buf);

#undef EXTERN
#if defined(__cplusplus)
}
#endif
