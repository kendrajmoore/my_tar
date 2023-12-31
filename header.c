
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <unistd.h>
#include "header.h"
#include "helper.h"

//prefix- allows filename up to 155
void set_prefix(struct tar_header *header, const char *filename){
    if(!header || !filename){
        write_stderr("Cannot create a file prefix for the header");
        return;
    }
    int length = my_strlen(filename);
    if(length <= 100) {
        my_strncpy(header->name, filename, sizeof(header->name));
    } else {
        int prefix_length = length - 100; //determine prefix length
        if(prefix_length > 155){
            write_stderr("Filename is too long for this file");
            return;
        }
        my_strncpy(header->prefix, filename, prefix_length);
        my_strncpy(header->name, filename + prefix_length, sizeof(header->name));
    }
}
//set device info for header
void dev_info(struct tar_header *header, const struct stat *system_stat){  //if file is a character device or block device
    if(!header || !system_stat){
        write_stderr("Cannot get device info for header");
        return;
    }
    if(S_ISCHR(system_stat->st_mode) || S_ISBLK(system_stat->st_mode)){
        num_to_octal(header->devmajor, sizeof(header->devmajor), major(system_stat->st_rdev));
        num_to_octal(header->devminor, sizeof(header->devminor), minor(system_stat->st_rdev));
    } else {   //just add zeros
        my_strncpy(header->devmajor, "0000000", sizeof(header->devmajor)-1);
        my_strncpy(header->devminor, "0000000", sizeof(header->devminor)-1);
        header->devmajor[sizeof(header->devmajor) -1] = '\0';
        header->devminor[sizeof(header->devminor) -1] = '\0';
    }

}

void calculate_checksum(struct tar_header *header){
    if(!header){
        write_stderr("Error calculating header checksum");
        return;
    }
    unsigned char *header_info = (unsigned char*)header;
    size_t checksum = 0;
    size_t checksum_offset = (size_t) &(header->checksum) - (size_t) header; //find start position
    for(size_t i = 0; i < sizeof (struct tar_header); i++){
        if(i >= checksum_offset && i < checksum_offset + sizeof(header->checksum)){
            checksum += ' ';
        } else {
            checksum += header_info[i];
        }
    }
    num_to_octal(header->checksum, sizeof(header->checksum), checksum);
}

void clear_header(struct tar_header *header){
    if(!header){
        write_stderr("Error creating tar header");
        return;
    }
    for(unsigned long i = 0; i < sizeof(struct tar_header); i++){
        ((char*)header)[i] = 0;
    }
}

void set_octal_values(struct tar_header *header, const struct stat *system_stat){
    if(!header || !system_stat){
        write_stderr("Error writing octal values in tar header");
        return;
    }
    if(S_ISDIR(system_stat->st_mode)){
        my_strncpy(header->size, "00000000000", sizeof(header->size) -1);
    }
    num_to_octal(header->size, sizeof(header->size), (unsigned)system_stat->st_size);
    num_to_octal(header->mtime, sizeof(header->mtime), (unsigned)system_stat->st_mtime);
    num_to_octal(header->mode, sizeof(header->mode), system_stat->st_mode & 07777);
    num_to_octal(header->userid, sizeof(header->userid), system_stat->st_uid);
    num_to_octal(header->groupid, sizeof(header->groupid), system_stat->st_gid);
}

void set_string_values(struct tar_header *header){
    if(!header){
        write_stderr("Error setting string values for tar header");
        return;
    }
    my_strncpy(header->magic, "ustar\0", sizeof(header->magic));
    my_strncpy(header->version, "00", sizeof(header->version));
    my_strncpy(header->padding, "\0", sizeof(header->padding));
}
//set guid, uid
void set_permissions(struct tar_header *header, const struct stat *system_stat){//get userid
    if(!header || !system_stat){
        write_stderr("Error writing to the header");
        return;
    }
    struct passwd *user_id = getpwuid(system_stat->st_uid);
    if(user_id){   //write to header
        my_strncpy(header->uname, "docode", sizeof(header->uname) -1);
    } else {
        write_stderr("Failed to get user_id for header");
        return;
    }
    struct group *group_id = getgrgid(system_stat->st_gid);
    if(group_id){
        my_strncpy(header->gname, "docode", sizeof(header->gname) -1);
    } else {
        write_stderr("Failed to get group_id for header");
        return;
    }

}
//check if it is a file or directory
void set_typeflag(struct tar_header *header, const struct stat *system_stat, const char *filename){
    if(!header || !system_stat || !filename){
        write_stderr("Error setting type flag for tar header");
        return;
    }
    if(S_ISLNK(system_stat->st_mode)){
        header->typeflag = '2';
        ssize_t link = readlink(filename, header->linkname, sizeof(header->linkname) -1);
        if(link != -1){
            write_stderr("Failed to read symlink for tar header");
            return;
        }
        header->linkname[link] = '\0';
    } else if(S_ISDIR(system_stat->st_mode)){
        header->typeflag = '5';
        if(header->name[my_strlen(header->name) -1] != '/'){
            my_strncat(header->name, "/", sizeof(header->name) - my_strlen(header->name) -1);
        }
    } else {
        header->typeflag = '0';
    }
}

void write_header(const char *path, struct tar_header *header, struct stat *system_stat){
    if(!path || !header || !system_stat){
        write_stderr("Error in writing header values.");
        return;
    }
    clear_header(header);
    header->name[sizeof(header->name) -1] = '\0';
    dev_info(header, system_stat);
    set_prefix(header, path);
    set_octal_values(header, system_stat);
    set_string_values(header);
    set_permissions(header, system_stat);
    set_typeflag(header, system_stat, path);
    calculate_checksum(header);
}







