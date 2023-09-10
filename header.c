#include "header.h"

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
//prefix- allows filename up to 155
void set_prefix(struct tar_header *header, const char *filename)
{
    if(!header || !filename)
    {
        write_stderr("Cannot create a prefix");
    }
    int length = my_strlen(filename);
    if(length <= 100) //if filename less than 100 copy to header
    {
        my_strncpy(header->name, filename, sizeof(header->name));
    } else
    {
        int prefix_length = length - 100; //determine prefix length
        if(prefix_length > 155)
        {
            write_stderr("filename is too long");
            return;
        }
        my_strncpy(header->prefix, filename, prefix_length);
        my_strncpy(header->name, filename + prefix_length, sizeof(header->name));
    }
}
//set device info for header
void dev_info(struct tar_header *header, const struct stat *system_stat)
{  //if file is a character device or block device
    if(!header || !system_stat)
    {
        write_stderr("Cannot get device info for header");
    }
    if(S_ISCHR(system_stat->st_mode) || S_ISBLK(system_stat->st_mode))
    {
        num_to_octal(header->devmajor, sizeof(header->devmajor), major(system_stat->st_rdev));
        num_to_octal(header->devminor, sizeof(header->devminor), minor(system_stat->st_rdev));
    } else
    {   //just add zeros
        my_strncpy(header->devmajor, "0000000", sizeof(header->devmajor)-1);
        my_strncpy(header->devminor, "0000000", sizeof(header->devminor)-1);
        header->devmajor[sizeof(header->devmajor) -1] = '\0';
        header->devminor[sizeof(header->devminor) -1] = '\0';
    }

}

void calculate_checksum(struct tar_header *header)
{
    unsigned char *header_info = (unsigned char*)header;
    size_t checksum = 0;
    size_t checksum_offset = (size_t) &(header->checksum) - (size_t) header; //find start position
    for(size_t i = 0; i < sizeof (struct tar_header); i++)
    {
        if(i >= checksum_offset && i < checksum_offset + sizeof(header->checksum))
        {
            checksum += ' ';
        } else {
            checksum += header_info[i];
        }
    }
    num_to_octal(header->checksum, sizeof(header->checksum), checksum);
}

void clear_header(struct tar_header *header)
{
    if(!header){
        write_stderr("Error creating tar header");
    }
    for(unsigned long i = 0; i < sizeof(struct tar_header); i++)
    {
        ((char*)header)[i] = 0;
    }
}

void set_octal_values(struct tar_header *header, const struct stat *system_stat)
{
    if(!header || !system_stat)
    {
        write_stderr("Error writing to header");
    }
    num_to_octal(header->size, sizeof(header->size), (unsigned)system_stat->st_size);
    num_to_octal(header->mtime, sizeof(header->mtime), (unsigned)system_stat->st_mtime);
    num_to_octal(header->mode, sizeof(header->mode), system_stat->st_mode & 07777);
    num_to_octal(header->userid, sizeof(header->userid), system_stat->st_uid);
    num_to_octal(header->groupid, sizeof(header->groupid), system_stat->st_gid);
}

void set_string_values(struct tar_header *header)
{
    my_strncpy(header->magic, "ustar\0", sizeof(header->magic));
    my_strncpy(header->version, "00", sizeof(header->version));
    my_strncpy(header->padding, "\0", sizeof(header->padding));
}
//set guid, uid
void set_permissions(struct tar_header *header, const struct stat *system_stat)
{//get userid
    if(!header || !system_stat)
    {
        write_stderr("Error writing to the header");
    }
    struct passwd *user_id = getpwuid(system_stat->st_uid);
    if(user_id){   //write to header
        my_strncpy(header->uname, user_id->pw_name, sizeof(header->uname) -1);
    } else {
        write_stderr("Failed to get user_id.");
    }
    struct group *group_id = getgrgid(system_stat->st_gid);
    if(group_id)
    {
        my_strncpy(header->gname, group_id->gr_name, sizeof(header->gname) -1);
    } else {
        write_stderr("Failed to get group_id.");
    }

}

void set_typeflag(struct tar_header *header, const struct stat *system_stat, const char *filename)
{
    if(S_ISLNK(system_stat->st_mode)){
        header->typeflag = '2';
        ssize_t link = readlink(filename, header->linkname, sizeof(header->linkname) -1);
        if(link != -1)
        {
            header->linkname[link] = '\0';
        }
    } else if(S_ISDIR(system_stat->st_mode))
    {
        header->typeflag = '5';
    } else {
        header->typeflag = '0';
    }
}

void write_header(const char *filename, struct tar_header *header)
{
    struct stat system_stat;
    clear_header(header);
    if(stat(filename, &system_stat) != 0)
    {
        write_stderr("Error getting file information");
        return;
    }
    printf("Actual File Size: %lld\n", system_stat.st_size);

    header->name[sizeof(header->name) -1] = '\0';
    dev_info(header, &system_stat);
    set_prefix(header, filename);
    set_octal_values(header, &system_stat);
    set_string_values(header);
    set_permissions(header, &system_stat);
    set_typeflag(header, &system_stat, filename);
    calculate_checksum(header);
}







