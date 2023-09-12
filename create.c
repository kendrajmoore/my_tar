//
// Created by Kendra Moore on 9/9/23.
//
#include "create.h"
#include "header.h"
#include "helper.h"

#define BLOCK_SIZE 512
int create_file(int read_fd, int write_fd)
{
    char buffer[BLOCK_SIZE];
    int total_bytes_received = 0;
    int bytes_received;
    while((bytes_received = read(read_fd, buffer, sizeof(buffer))) > 0){
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
    int remainder = total_bytes_received % BLOCK_SIZE;
    if(remainder != 0)
    {
        int padding_num = BLOCK_SIZE - remainder;
        char padding_char = '\0';
        char padding[padding_num];
        for(int i = 0; i < padding_num; i++)
        {
            my_strncpy(&padding[i], &padding_char, 1);
        }
        write(write_fd, padding, padding_num);

    }
    return 0;
}

int create_archive(char *tar_name, char **files, int file_count)
{
    int dest = open(tar_name, O_WRONLY | O_CREAT, 0644 );
    if(!dest)
    {
        write_stderr("Error");
        return 1;
    }
    struct tar_header header;
    for(int i = 0; i < file_count; i++) {
        write_header(files[i], &header);
        write(dest, &header, sizeof(header));
        if(header.typeflag != '5')
        {
            int src = open(files[i], O_RDONLY);
            if(src == -1)
            {
                close(dest);
                return 1;
            }
            if(create_file(src, dest) != 0)
            {
                close(src);
                close(dest);
                return 1;

            }
            close(src);

        }
    }
    char end_of_archive[1024] = {0};
    write(dest, end_of_archive, 1024);

    close(dest);
    return 0;
}

int append_to_archive(char *tar_name, char **files, int file_count)
{
    // Open the existing tar file for reading and writing
    int dest = open(tar_name, O_RDWR);
    if (dest == -1)
    {
        write_stderr("Error opening tar file");
        return 1;
    }

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
        write_header(files[i], &header);
        write(dest, &header, sizeof(header));

        if (header.typeflag != '5')
        {
            // If it's not a directory, write the file contents
            int src = open(files[i], O_RDONLY);
            if (src == -1)
            {
                write_stderr("Error opening file");
                close(dest);
                return 1;
            }
            if (create_file(src, dest) != 0)
            {
                close(src);
                close(dest);
                return 1;
            }
            close(src);
        }
    }

    // Write the two 512-byte blocks of zeros to mark the end of the archive
    char end_of_archive[1024] = {0};
    write(dest, end_of_archive, 1024);

    close(dest);
    return 0;
}

int list_archive(char *tar_name)
{
    int dest = open(tar_name, O_RDONLY);
    if(dest == -1)
    {
        write_stderr("Error opening tar file.");
        return 1;
    }
    struct tar_header header;
    while(1)
    {
        int total_bytes_received = read(dest, &header, sizeof(header));
        if(total_bytes_received == -1)
        {
            write_stderr("Error reading from tar file.");
            close(dest);
            return 1;
        }
        if(total_bytes_received == 0)
        {
            break;
        }
        if(header.name[0] == '\0')
        {
            break;
        }
        printf("%s\n", header.name);
        unsigned int size = octal_to_num(header.size, sizeof(header.size));
        printf("Raw size: %u\n", size);
        int blocks = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;
        printf("Size: %u, Blocks: %d\n", size, blocks);

        if(lseek(dest, blocks * BLOCK_SIZE, SEEK_CUR) == -1)
        {
            write_stderr("Error seeking in a tar file");
            close(dest);
            return 1;
        }

    }
    close(dest);
    return 0;
}


int update_archive(char *tar_name, char **files, int file_count)
{
    char *temp_tar = generate_file();
    if(!temp_tar)
    {
        write_stderr("Error generating temp file");
        return 1;
    }

    int temp_dest = open(temp_tar, O_RDWR | O_CREAT, 0644);
    if(temp_dest == -1)
    {
        write_stderr("Error creating temp file");
        free(temp_tar);
        return 1;
    }

    int src = open(tar_name, O_RDONLY);
    if(src == -1)
    {
        write_stderr("Error opening tar file");
        close(temp_dest);
        unlink(temp_tar);
        free(temp_tar);
        return 1;
    }
    struct tar_header header;
    while(read(src, &header, sizeof(header)) == sizeof(header))
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
                    write_header(files[i], &header);
                    write(temp_dest, &header, sizeof(header));
                    if(header.typeflag != '5')
                    {
                        int file_fd = open(files[i], O_RDONLY);
                        if(file_fd == -1)
                        {
                            write_stderr("Error opening file");
                            close(src);
                            close(temp_dest);
                            unlink(temp_tar);
                            free(temp_tar);
                            return 1;
                        }
                        if(create_file(file_fd, temp_dest) != 0){
                            close(file_fd);
                            close(src);
                            close(temp_dest);
                            unlink(temp_tar);
                            free(temp_tar);
                            return 1;
                        }
                        close(file_fd);
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

int extract_archive(const char *tar_name)
{
    int tar_fd = open(tar_name, O_RDONLY);
    if(tar_fd == -1)
    {
        write_stderr("Error opening tar file");
        return 1;
    }
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
            int file_fd = open(header.name, O_WRONLY | O_CREAT, 0644);
            if(file_fd == -1)
            {
                write_stderr("Error creating file for extraction");
                close(tar_fd);
                return 1;
            }
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


