//
// Created by Kendra Moore on 9/9/23.
//

#ifndef MY_TAR_CREATE_H
#define MY_TAR_CREATE_H
#define BLOCK_SIZE 512
#define ADD_PADDING 1024

int create_file(int read_fd, int write_fd);
int open_read(char *filename);
int open_read_write(char *filename);
int open_write(char *filename);
void block_math(int total_bytes_received, int write_fd);
int create_archive(char *tar_name, char **files, int file_count);
int append_to_archive(char *tar_name, char **files, int file_count);
int list_archive(char *tar_name);
int update_archive(char *tar_name, char **files, int file_count);
int extract_archive(const char *tar_name);

#endif //MY_TAR_CREATE_H
