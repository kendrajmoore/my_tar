//
// Created by Kendra Moore on 9/9/23.
//
#include "create.h"
#include "header.h"

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
        printf("Raw size: %s\n", header.size);
        unsigned int size = octal_to_num(header.size, sizeof(header.size));
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
    // Temporary tar file name
    char temp_tar_name[] = "temp_tar.XXXXXX";

    // Create a temporary file. This will be our new archive.
    int temp_dest = mkstemp(temp_tar_name);
    if (temp_dest == -1)
    {
        write_stderr("Error creating temporary tar file");
        return 1;
    }

    // Open the existing tar file for reading
    int src = open(tar_name, O_RDONLY);
    if (src == -1)
    {
        write_stderr("Error opening tar file");
        close(temp_dest);
        unlink(temp_tar_name);
        return 1;
    }

    struct tar_header header;
    while (read(src, &header, sizeof(header)) == sizeof(header))
    {
        unsigned int size = octal_to_num(header.size, sizeof(header.size));
        int blocks = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;

        int file_exists_in_new_list = 0;
        for (int i = 0; i < file_count; i++)
        {
            if (my_strcmp(header.name, files[i]) == 0)
            {
                struct stat file_stat;
                if (stat(files[i], &file_stat) != 0)
                {
                    write_stderr("Error getting file information");
                    close(src);
                    close(temp_dest);
                    unlink(temp_tar_name);
                    return 1;
                }

                unsigned int archived_mtime = octal_to_num(header.mtime, sizeof(header.mtime));
                if (file_stat.st_mtime > (time_t) archived_mtime)
                {
                    // New file is newer. Add the newer file to temp archive and skip the older one.
                    write_header(files[i], &header);
                    write(temp_dest, &header, sizeof(header));
                    if (header.typeflag != '5')
                    {
                        int file_fd = open(files[i], O_RDONLY);
                        if (file_fd == -1)
                        {
                            write_stderr("Error opening file");
                            close(src);
                            close(temp_dest);
                            unlink(temp_tar_name);
                            return 1;
                        }
                        if (create_file(file_fd, temp_dest) != 0)
                        {
                            close(file_fd);
                            close(src);
                            close(temp_dest);
                            unlink(temp_tar_name);
                            return 1;
                        }
                        close(file_fd);
                    }
                }
                file_exists_in_new_list = 1;
                break;
            }
        }

        if (!file_exists_in_new_list)
        {
            // Add the old file from the source tar to the temp archive.
            write(temp_dest, &header, sizeof(header));
            char buffer[BLOCK_SIZE];
            for (int b = 0; b < blocks; b++)
            {
                read(src, buffer, BLOCK_SIZE);
                write(temp_dest, buffer, BLOCK_SIZE);
            }
        }
        else
        {
            // Skip the old file data in the source tar.
            lseek(src, blocks * BLOCK_SIZE, SEEK_CUR);
        }
    }

    close(src);

    // Write the two 512-byte blocks of zeros to mark the end of the temporary archive
    char end_of_archive[1024] = {0};
    write(temp_dest, end_of_archive, 1024);

    close(temp_dest);

    // Replace the old tar with the new tar.
    if (rename(temp_tar_name, tar_name) != 0)
    {
        write_stderr("Error renaming temporary tar file");
        unlink(temp_tar_name);
        return 1;
    }

    return 0;
}



