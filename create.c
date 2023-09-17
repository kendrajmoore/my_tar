//
// Created by Kendra Moore on 9/9/23.
//
#include "create.h"
#include "header.h"
#include "helper.h"

//function to stream data to tar file
int stream_archive(int read_fd, int write_fd)
{
    char buffer[BLOCK_SIZE];
    int total_bytes_received = 0;
    int bytes_received;
    while((bytes_received = read(read_fd, buffer, sizeof(buffer))) > 0)
    {
        total_bytes_received += bytes_received;
        int bytes_written = write(write_fd, buffer, bytes_received);
        if(bytes_written != bytes_received)
        {
            write_stderr("Error writing to tar file");
            close(read_fd);
            close(write_fd);
            return 1;
        }
    }
    if(bytes_received == -1)
    {
        write_stderr("Cannot read file");
        close(read_fd);
        close(write_fd);
        return 1;
    }
    block_math(total_bytes_received, write_fd);
    return 0;
}
//entry point for -c
int create_archive(char *tar_name, char **files, int file_count)
{
    int dest = open_write(tar_name);
    struct tar_header header;
    for(int i = 0; i < file_count; i++) {
        if(write_archive(dest, files[i], &header))
        {
            return 1;
        }
    }
    end_padding(dest);
    close(dest);
    return 0;
}

int append_to_archive(char *tar_name, char **files, int file_count)
{
    // Open the existing tar file for reading and writing
    int dest = open_read_write(tar_name);
    // Seek to the end of the tar file to find the two 512-byte blocks of zeros
    off_t current_position = lseek(dest, -1024, SEEK_END);
    if (current_position == -1)
    {
        write_stderr("Error seeking in tar file");
        close(dest);
        return 1;
    }
    struct tar_header header;
    for (int i = 0; i < file_count; i++)
    {
        if(write_archive(dest, files[i], &header))
        {
            return 1;
        }
    }
    // Write the two 512-byte blocks of zeros to mark the end of the archive
    end_padding(dest);
    close(dest);
    return 0;
}
//entry point to -t
int list_archive(char *tar_name)
{
    int dest = open_read(tar_name);
    struct tar_header header;
    while(1)
    {
        if(read_archive(dest, &header))
        {
            return 1;
        }
        if(my_strlen(header.name) == 0)
        {
            break;
        }
        unsigned int size = octal_to_num(header.size, sizeof(header.size));
        write(1, header.name, my_strlen(header.name));
        write(1, "\n", 1);
        if(size == 0)
        {
            continue;
        } else
        {
            int blocks = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;
            if(lseek(dest, blocks * BLOCK_SIZE, SEEK_CUR) == -1)
            {
                write_stderr("Error seeking in a tar file");
                close(dest);
                return 1;
            }

        }

    }
    close(dest);
    return 0;
}
//entry point for -u
int update_archive(char *tar_name, char **files, int file_count)
{
    char *temp_tar = generate_file();
    if(!temp_tar)
    {
        write_stderr("Error generating temp file");
        return 1;
    }
    int temp_dest = open_read_write(temp_tar);
    int src = open_read(tar_name);
    struct tar_header header;
    while(read_archive(src, &header) == 0)
    {
        unsigned int size = octal_to_num(header.size, sizeof(header.size));
        int blocks = (size + BLOCK_SIZE -1) / BLOCK_SIZE;
        int file_in_list = 0;
        for(int i = 0; i < file_count; i++)
        {
            if(my_strcmp(header.name, files[i]) == 0)
            {
                struct stat file_stat;
                if(stat(files[i], &file_stat) != 0)
                {
                    write_stderr("Error getting file information");
                    close(src);
                    close(temp_dest);
                    unlink(temp_tar);
                    free(temp_tar);
                    return 1;
                }
                unsigned int tar_mtime = octal_to_num(header.mtime, sizeof(header.mtime));
                if(file_stat.st_mtime > (time_t) tar_mtime)
                {
                    if(write_archive(temp_dest, files[i], &header))
                    {
                        free(temp_tar);
                        return 1;
                    }
                }
                file_in_list = 1;
                break;
            }
        }
        if(!file_in_list)
        {
            write(temp_dest, &header, sizeof(header));
            char buffer[BLOCK_SIZE];
            for(int i = 0; i < blocks; i++)
            {
                read(src, buffer, BLOCK_SIZE);
                write(temp_dest, buffer, BLOCK_SIZE);
            }
        }
        else
        {
            lseek(src, blocks * BLOCK_SIZE, SEEK_CUR);
        }
    }
    close(src);
    close(temp_dest);
    if(unlink(tar_name) == -1 || link(temp_tar, tar_name) == -1 || unlink(temp_tar) == -1)
    {
        write_stderr("Error replacing the tar file");
        free(temp_tar);
        return 1;
    }
    free(temp_tar);
    return 0;
}
//entry point for -x
int extract_archive(const char *tar_name)
{
    int tar_fd = open_read(tar_name);
    struct tar_header header;
    while(read(tar_fd, &header, sizeof(header)) == sizeof(header))
    {
        if(my_strlen(header.name) == 0)
        {
            break;
        }
        unsigned int size = octal_to_num(header.size, sizeof(header.size));
        int blocks = (size + BLOCK_SIZE -1) / BLOCK_SIZE;
        if(header.typeflag == '5')
        {
            mkdir(header.name, 0755);
        } else if(header.typeflag == '2')
        {
            char link_target[100];
            my_strncpy(link_target, header.linkname, sizeof(header.linkname));
            symlink(link_target, header.name);
        } else
        {
            int file_fd = open_write(header.name);
            char buffer[BLOCK_SIZE];
            for(int i = 0; i < blocks; i++)
            {
                if(read(tar_fd, buffer, sizeof(buffer)) != sizeof(buffer))
                {
                    write_stderr("Error reading file content from tar");
                    close(file_fd);
                    close(tar_fd);
                    return 1;
                }
                int bytes_to_write;
                if(i == blocks -1)
                {
                    bytes_to_write = size % BLOCK_SIZE;
                } else
                {
                    bytes_to_write = sizeof(buffer);
                }
                write(file_fd, buffer, bytes_to_write);

            }
            close(file_fd);
        }
    }
    close(tar_fd);
    return 0;
}


