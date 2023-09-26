//
// Created by Kendra Moore on 9/10/23.
//

#ifndef MY_TAR_HELPER_H
#define MY_TAR_HELPER_H
#define BLOCK_SIZE 512
#define ADD_PADDING 1024
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include "header.h"
#include "create.h"


char *my_strrchr(const char *input, int n);
int my_strlen(const char* input);
char *my_strncpy(char *dest, const char *src, int num);
char *my_strcpy(char *dest, const char *src);
int my_strcmp (char * param_1, char * param_2);
char *my_strncat(char *dest, const char *src, size_t count);
void write_stderr(const char *error_message);
int open_read(const char *filename);
int open_read_write(const char *filename);
int open_write(const char *filename);
void block_math(int total_bytes_received, int write_fd);
void end_padding(int dest);
void num_to_octal(char *string, int length, unsigned int num);
unsigned int octal_to_num(const char *string, int length);
void convert_to_two_digit_str(char *dest, int num);
void int_to_str(int num, char *string);
char *generate_file(const char *tar_name);
int write_archive(int dest, char *file_name, struct tar_header *header);
int read_archive(int tar_fd, struct tar_header *header);



#endif //MY_TAR_HELPER_H
