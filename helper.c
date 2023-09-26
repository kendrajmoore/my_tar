//
// Created by Kendra Moore on 9/10/23.
//

//https://www.gnu.org/software/tar/manual/html_node/Standard.html
//u
#include <dirent.h>
#include "helper.h"

#define TEMP_DIR "/tmp"
#define MAX_TRIES 3
//custom string functions

char *my_strrchr(const char *input, int n)
{
    char *last = NULL;
    char ch = (char)n;
    while(*input)
    {
        if(*input == ch)
        {
            last = (char*)input;
        }
        input++;
    }
    if(ch == '\0')
    {
        return (char*)input;
    }
    return last;
}

int my_strlen(const char *input)
{
    int length = 0;
    while(*input != '\0')
    {
        length++;
        input++;
    }
    return length;
}
//custom strlen to copy values to struct header and tar file
char *my_strncpy(char *dest, const char *src, int num)
{
    char* old_dest = dest;
    while(num > 0 && *src)
    {
        *dest++ = *src++;
        num--;
    }
    while(num > 0)
    {
        *dest++ = '\0';
        num--;
    }
    return old_dest;
}

char *my_strcpy(char *dest, const char *src)
{
    char *old_dest = dest;
   while(*src != '\0')
   {
       *dest = *src;
       dest++;
       src++;
   }
   *dest = '\0';
   return old_dest;
}

int my_strcmp (char *param_1, char *param_2)
{
    while(*param_1 && (*param_1 == *param_2))
    {
        param_1++;
        param_2++;
    }
    return *(unsigned char *)param_1 - *(unsigned char *)param_2;
}

char *my_strncat(char *dest, const char *src, size_t count)
{
    char *original_dest = dest;
    while(*dest)
    {
        dest++;
    }
    while(count-- && *src)
    {
        *dest++ = *src++;
    }
    *dest = '\0';
    return original_dest;
}

//custom function to write to stderr
void write_stderr(const char *error_message)
{
    int length = my_strlen(error_message);
    write(2, error_message, length);
    write(2, "\n", 1);
}
//open fd for read only
int open_read(const char *filename)
{
    int fd_read = open(filename,  O_RDONLY);
    if(fd_read == -1)
    {
        write_stderr("Error opening the file for reading");
        return -1;
    }
    return fd_read;

}
//open fd for read and write
int open_read_write(const char *filename)
{
    int fd_rw = open(filename, O_RDWR);
    if(fd_rw == -1)
    {
        write_stderr("Error opening the file for reading or writing");
        return -1;
    }
    return fd_rw;
}
//open fd for read and write
int open_write(const char *filename)
{
    int fd_wr = open(filename, O_WRONLY | O_CREAT, 0644);
    off_t current_offset = lseek(fd_wr, 0, SEEK_CUR);
    if(fd_wr == -1)
    {
        write_stderr("Error opening the file for writing");
        return -1;
    }
    return fd_wr;
}
//calculate how much padding is needed to fill last block
void block_math(int total_bytes_received, int write_fd)
{
    int remainder = total_bytes_received % BLOCK_SIZE;
    if (remainder != 0) {
        int padding_num = BLOCK_SIZE - remainder;
        char padding_char = '\0';
        char padding[padding_num];
        for (int i = 0; i < padding_num; i++) {
            my_strncpy(&padding[i], &padding_char, 1);
        }
        write(write_fd, padding, padding_num);

    }
}
//add two 512 block to the end
void end_padding(int dest)
{
    char end_of_archive[ADD_PADDING] = {0};
    write(dest, end_of_archive, ADD_PADDING);
}

//convert mode, uid, gid, size, mtime from chars to octal
void num_to_octal(char *string, int length, unsigned int num)
{   //make a string with spaces
    if(!string || length <= 0)
    {
        write_stderr("Error converting values for header");
        return;
    }
    for(int i = 0; i < length; i++)
    {
        string[i] = '0';
    }
    //start writing at second to last space, last space is ' '
    int position = length - 2;
    if(num == 0 && position >= 0)
    {
        string[position] = '0';
        return;
    }
    while(num && position >= 0) //while input num is not zero
    {
        string[position] = '0' + (num & 7); //bitwise AND with 111
        num >>= 3; //right shift num by 3 bitws
        position--;
    }
    string[length -1] = '\0';
}

unsigned int octal_to_num(const char *string, int length)
{
    unsigned int result = 0;
    for(int i = 0; i < length && string[i] != '\0'; i++)
    {
        result = (result << 3) + (string[i] - '0');
    }
    return result;
}

void int_to_str(int num, char *string)
{
    int idx = 0;
    if(num == 0)
    {
        string[idx] = '0';
        idx++;
    }
    while(num)
    {
        string[idx] = (num % 10) + '0';
        idx++;
        num /= 10;
    }
    for(int i = 0; i < idx / 2; i++)
    {
        char temp = string[i];
        string[i] = string[idx - i -1];
        string[idx - i -1] = temp;
    }
    string[idx] = '\0';
}

void convert_to_two_digit_str(char *dest, int num)
{
    dest[0] = '0' + (num / 10);  // Tens place
    dest[1] = '0' + (num % 10);  // Units place
    dest[2] = '\0';
}


//function to create a temp file for updating archive
char *generate_file(const char *tar_name)
{
    char *temp_file = (char*)malloc(256);
    if (!temp_file)
    {
        return NULL;
    }

    // Extract base name from tar_name (e.g., "dog" from "dog.tar" or "/path/dog.tar")
    const char *base_tar_name = my_strrchr(tar_name, '/');
    if (base_tar_name)
    {
        base_tar_name++; // move past the '/'
    }
    else
    {
        base_tar_name = tar_name;
    }
    char stripped_name[256] = {0};
    my_strncpy(stripped_name, base_tar_name, my_strlen(base_tar_name) - 4); // -4 to strip ".tar"
    stripped_name[my_strlen(base_tar_name) - 4] = '\0'; // Ensure null-termination after strncpy

    int attempts = 0;
    int fd; // File descriptor for the created file
    while (attempts < MAX_TRIES)
    {
        // Generate two random digits
        int random_digits = rand() % 100;

        // Convert random digits to string manually
        char digit_str[3];
        convert_to_two_digit_str(digit_str, random_digits);

        // Construct temp_file using strncpy and strncat
        my_strncpy(temp_file, TEMP_DIR, 255); // 255 to leave room for null-terminator
        my_strncat(temp_file, "/temp", 255 - my_strlen(temp_file));
        my_strncat(temp_file, stripped_name, 255 - my_strlen(temp_file));
        my_strncat(temp_file, digit_str, 255 - my_strlen(temp_file));
        my_strncat(temp_file, ".tmp", 255 - my_strlen(temp_file));

        // Try to create the file
        fd = open(temp_file, O_RDWR | O_CREAT | O_EXCL, 0666);
        if (fd != -1)
        {
            close(fd); // Close the file after creating it
            return temp_file; // Return the name of the successfully created file
        }

        attempts++;
    }
    free(temp_file);
    return NULL;
}

//checks if an input is a file or directory. If directory recursively open files
int write_archive(int dest, char *path, struct tar_header *header)
{
    struct stat system_stat;
    if(stat(path, &system_stat) != 0)
    {
        write_stderr("Cannot find that file or directory");
        return 1;
    }
    write_header(path, header, &system_stat);
    write(dest, header, sizeof(struct tar_header));

    if(S_ISDIR(system_stat.st_mode))
    {
        DIR *dir = opendir(path);
        if(!dir)
        {
            write_stderr("Cannot open the directoru");
            close(dest);
            return 1;
        }
        struct dirent *item;
        while((item = readdir(dir)) != NULL)
        {
            if(my_strcmp(item->d_name, ".") == 0 || my_strcmp(item->d_name, "..") == 0)
            {
                continue;
            }
            char full_path[1024];
            my_strncpy(full_path, path, sizeof(full_path) -1);
            my_strncat(full_path, "/", sizeof(full_path) - my_strlen(full_path) -1);
            my_strncat(full_path, item->d_name, sizeof(full_path) - my_strlen(full_path) -1);
            if(write_archive(dest, full_path, header))
            {
                closedir(dir);
                return 1;
            }
        }
        closedir(dir);
    } else if(S_ISREG(system_stat.st_mode))
    {
        int src = open(path, O_RDONLY);
        if(src == -1)
        {
            write_stderr("Cannot read the file");
        }
        if(stream_archive(src, dest))
        {
            close(src);
            return 1;
        }
        close(src);
    }
    return 0;

}
//function to read an archive
int read_archive(int tar_fd, struct tar_header *header)
{
    int bytes_read = read(tar_fd, header, sizeof(struct tar_header));
    if(bytes_read != sizeof(struct tar_header))
    {
        write_stderr("Error reading tar file, incomplete header read");
        return 1;
    }
    return 0;
}
