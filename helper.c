//
// Created by Kendra Moore on 9/10/23.
//

//https://www.gnu.org/software/tar/manual/html_node/Standard.html
//u
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
        string[i] = ' ';
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

}

unsigned int octal_to_num(const char *string, int length)
{
    unsigned int result = 0;
    for(int i = 0; i < length && (string[i] != ' ' || (i < length-1 && string[i+1] != ' ')); i++)
    {
        result = (result << 3) + (string[i] - '0');
    }
    return result;
}