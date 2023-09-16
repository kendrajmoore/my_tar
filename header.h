//
// Created by Kendra Moore on 9/9/23.
//

#ifndef MY_TAR_HEADER_H
#define MY_TAR_HEADER_H

#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include "helper.h"

struct tar_header {
    char name[100];
    char mode[8];
    char userid[8];
    char groupid[8];
    char size[12];
    char mtime[12];
    char checksum[8];
    char typeflag;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char padding[12];
};

void set_prefix(struct tar_header *header, const char *filename);
void dev_info(struct tar_header *header, const struct stat *system_stat);
void calculate_checksum(struct tar_header *header);
void clear_header(struct tar_header *header);
void set_octal_values(struct tar_header *header, const struct stat *system_stat);
void set_string_values(struct tar_header *header);
void set_permissions(struct tar_header *header, const struct stat *system_stat);
void set_typeflag(struct tar_header *header, const struct stat *system_stat, const char *filename);
void write_header(const char *path, struct tar_header *header, struct stat *system_stat);


#endif //MY_TAR_HEADER_H
