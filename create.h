//
// Created by Kendra Moore on 9/9/23.
//

#ifndef MY_TAR_CREATE_H
#define MY_TAR_CREATE_H

int create_file(int read_fd, int write_fd);
int create_archive(char *tar_name, char **files, int file_count);
int append_to_archive(char *tar_name, char **files, int file_count);
int list_archive(char *tar_name);
int update_archive(char *tar_name, char **files, int file_count);

#endif //MY_TAR_CREATE_H
