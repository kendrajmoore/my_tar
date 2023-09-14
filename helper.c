//
// Created by Kendra Moore on 9/10/23.
//

//https://www.gnu.org/software/tar/manual/html_node/Standard.html
//u
#include "helper.h"

#define TEMP_DIR "/tmp"
#define MAX_TRIES 10

int my_strlen(const char* input)
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
    while((*dest++ = *src++));
    return old_dest;
}

int my_strcmp (char * param_1, char * param_2)
{
    int sum_1 = 0, sum_2 = 0;
    int i_1 = 0, i_2 = 0;

    while(i_1 < my_strlen(param_1))
    {
        sum_1 += param_1[i_1];
        i_1++;
    }

    while(i_2 < my_strlen(param_2))
    {
        sum_2 += param_2[i_2];
        i_2++;
    }

    if (sum_1 > sum_2)
        return -1;
    else if (sum_1 < sum_2)
        return 1;
    else
        return 0;
}

int my_strncmp(char *param_1, char *param_2, int size)
{
    if(size == 0)
    {
        return 0;
    }
    while(*param_1 && param_2 && size)
    {
        if(*param_1 != *param_2)
        {
            return (unsigned char)*param_1 - (unsigned char)*param_2;
        }
        param_1++;
        param_2++;
        size--;
    }
    if(size)
    {
        return (unsigned char)*param_1 - (unsigned char)*param_2;
    }
    return 0;
}
//custom function to write to stderr
void write_stderr(const char *error_message)
{
    int length = my_strlen(error_message);
    write(2, error_message, length);
    write(2, "\n", 1);
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

char *generate_file()
{
    char *temp_file = (char*)malloc(256);
    if(!temp_file)
    {
        return NULL;
    }
    int attempts = 0;
    while(attempts < MAX_TRIES)
    {
        char time_string[20] = {0};
        char pid_string[10] = {0};
        char random_string[10] = {0};

        char *ptr = temp_file;
        char *base = TEMP_DIR "/tempfile_";
        while((*ptr = *base))
        {
            ptr++;
            base++;
        }
        time_t now;
        time(&now);
        int_to_str((int)now, time_string);
        char *t_ptr = time_string;
        while((*ptr = *t_ptr))
        {
            ptr++;
            t_ptr++;
        }
        *ptr = '-';
        *ptr++;
        int_to_str(getpid(), pid_string);
        char *p_ptr = pid_string;
        while((*ptr = *p_ptr))
        {
            ptr++;
            p_ptr++;
        }
        *ptr = '-';
        int_to_str(rand(), random_string);
        char *r_ptr = random_string;
        while((*ptr = *r_ptr))
        {
            ptr++;
            r_ptr++;
        }
        char *extension = ".tmp";
        while((*ptr = *extension))
        {
            ptr++;
            extension++;
        }
        struct stat buffer;
        if(stat(temp_file, &buffer) != 0)
        {
            return temp_file;
        }
        attempts++;
    }
    free(temp_file);
    return NULL;
}


