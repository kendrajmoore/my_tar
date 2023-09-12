//
// Created by Kendra Moore on 9/10/23.
//

#ifndef MY_TAR_HELPER_H
#define MY_TAR_HELPER_H
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <stdlib.h>



int my_strlen(const char* input);
char *my_strncpy(char *dest, const char *src, int num);
int my_strcmp (char * param_1, char * param_2);
void write_stderr(const char *error_message);
void num_to_octal(char *string, int length, unsigned int num);
unsigned int octal_to_num(const char *string, int length);
void int_to_str(int num, char *string);
char *generate_file();



#endif //MY_TAR_HELPER_H
