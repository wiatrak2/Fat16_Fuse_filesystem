/*
    Wojciech Pratkowiecki II UWr
    FAT16 FUSE driver
*/
#ifndef fat16_fuse_h
#define fat16_fuse_h

#include <fuse_lowlevel.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include "fat16.h"

void fat16_init( void *userdata, struct fuse_conn_info *conn );
void fat16_destroy ( void *userdata );
void fat16_lookup(fuse_req_t req, fuse_ino_t parent, const char *name);
void fat16_getattr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);
void fat16_readdir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, struct fuse_file_info *fi);
void fat16_opendir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);
void fat16_releasedir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);
void fat16_open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);
void fat16_read(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, struct fuse_file_info *fi);
void fat16_release(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);
void fat16_statfs(fuse_req_t req, fuse_ino_t ino);

struct fuse_lowlevel_ops fat16_ops = {
    .init       = fat16_init,
    .destroy    = fat16_destroy,
    .lookup     = fat16_lookup,
    .getattr    = fat16_getattr,
    .readdir    = fat16_readdir,
    .opendir    = fat16_opendir,
    .releasedir = fat16_releasedir,
    .open       = fat16_open,
    .read       = fat16_read,
    .release    = fat16_release,
    .statfs     = fat16_statfs,
};


#endif /* fat16_fuse_h */
