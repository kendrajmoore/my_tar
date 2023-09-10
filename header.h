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

int my_strlen(const char* input);
char *my_strncpy(char *dest, const char *src, int num);
int my_strcmp (char * param_1, char * param_2);
void write_stderr(const char *error_message);
void num_to_octal(char *string, int length, unsigned int num);
unsigned int octal_to_num(const char *string, int length);
void set_prefix(struct tar_header *header, const char *filename);
void dev_info(struct tar_header *header, const struct stat *system_stat);
void calculate_checksum(struct tar_header *header);
void clear_header(struct tar_header *header);
void set_octal_values(struct tar_header *header, const struct stat *system_stat);
void set_string_values(struct tar_header *header);
void set_permissions(struct tar_header *header, const struct stat *system_stat);
void set_typeflag(struct tar_header *header, const struct stat *system_stat, const char *filename);
void write_header(const char *filename, struct tar_header *header);

#endif //MY_TAR_HEADER_H
