/*
    Wojciech Pratkowiecki II UWr
    FAT16 FUSE driver
*/
/*
    fat16_fuse zawiera implementację poszczególnych funkcji interfejsu fuse.
    Funkcje te umożliwiają prawidłową obsługę systemu plików.
    Dokumentacja Fuse: http://libfuse.github.io/doxygen/ 
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

void fat16_init( void *userdata, struct fuse_conn_info *conn ); // funckja odpalana przy starcie programu
void fat16_destroy ( void *userdata ); // funkcja odpalana przy odmontowywaniu systemu plików
void fat16_lookup(fuse_req_t req, fuse_ino_t parent, const char *name); // wyszukiwanie plików w katalogu
void fat16_getattr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi); // pobieranie metadanych pliku
void fat16_readdir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, struct fuse_file_info *fi); // czytanie wpisów w katalogu
void fat16_opendir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi); // tworzenie uchwytu do katalogu
void fat16_releasedir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi); // zwolnienie uchwytu do katalogu
void fat16_open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi); // tworzenie uchwytu do pliku
void fat16_read(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, struct fuse_file_info *fi); // czytanie danego pliku
void fat16_release(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi); // zwolnienie uchwytu do pliku
void fat16_statfs(fuse_req_t req, fuse_ino_t ino); // pobranie metadancyh systemu plików

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
