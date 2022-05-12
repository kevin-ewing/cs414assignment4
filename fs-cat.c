/*
 * fs-cat.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "/usr/src/sys/ufs/ffs/fs.h"
#include "/usr/src/sys/ufs/ufs/dinode.h"
#include "/usr/src/sys/ufs/ufs/dir.h"



long inode_to_offset(struct fs *superblock, int inode_num);
int is_inode_dir(struct ufs2_dinode * inode);
int dps_find(struct ufs2_dinode * inode, char * disk_offset, struct fs *superblock, char * comp_string);
void print_file(struct ufs2_dinode * inode, struct fs * superblock, char * disk_offset);

int
main(int argc, char *argv[])
{
    struct fs *superblock;
    struct stat st;
    char *disk_offset;
    int fd, file_size, file_ino_num;
    struct ufs2_dinode *root_inode, *file_ino;

    fd = open(argv[1], O_RDWR);
    if(fd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    if (stat(argv[1], &st) != 0)
    {
        perror("stat");
        exit(EXIT_FAILURE);
    }
    file_size = st.st_size;
    
    disk_offset = mmap(0, file_size, PROT_READ, MAP_SHARED, fd, 0);
    if(disk_offset == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    
    superblock = (struct fs *) (disk_offset + SBLOCK_UFS2);
    root_inode = (struct ufs2_dinode *) (inode_to_offset(superblock, UFS_ROOTINO) + disk_offset);

    file_ino_num = dps_find(root_inode, disk_offset, superblock, argv[2]);

    if (file_ino_num == -1) {
        printf("The path or file name may be incorrect...\n");
        exit(EXIT_FAILURE);
    }

    file_ino = (struct ufs2_dinode *)(inode_to_offset(superblock, file_ino_num) + disk_offset);

    print_file(file_ino, superblock, disk_offset);
}


long
inode_to_offset(struct fs *superblock, int inode_num){
    return ((ino_to_fsba(superblock, inode_num) * superblock->fs_fsize) + (ino_to_fsbo(superblock, inode_num) * sizeof(struct ufs2_dinode)));
}


int
is_inode_dir(struct ufs2_dinode * inode){
    return (inode->di_mode & IFMT) == IFDIR;
}


int
dps_find(struct ufs2_dinode * inode, char * disk_offset, struct fs *superblock, char * comp_string){
    struct direct * root_dir, * temp_dir;
    struct ufs2_dinode * temp_inode;
    char * target_name;
    int temp_reclen;

    // update our "target"
    target_name = strsep(&comp_string, "/");
    if (target_name == NULL) {
        return -1;
    }

    // set up directory's root
    root_dir = (struct direct *)((inode->di_db[0] * superblock->fs_fsize) + disk_offset);
    temp_reclen = root_dir->d_reclen;

    while(temp_reclen < sizeof(struct direct)) {
        temp_dir = (struct direct *)((inode->di_db[0] * superblock->fs_fsize) + disk_offset + temp_reclen);
        temp_reclen += temp_dir->d_reclen;

        if ((strcmp(temp_dir->d_name,".") == 0) || (strcmp(temp_dir->d_name,"..") == 0))
        {
            continue;
        }
       
        // name matches, now check to see if it is a directory:
        if (strcmp(target_name, temp_dir->d_name) == 0) {
            temp_inode = (struct ufs2_dinode *) (inode_to_offset(superblock, temp_dir->d_ino) + disk_offset);
            if (is_inode_dir(temp_inode)) {
                // this is a directory (if we overshoot, will trigger base case)
                return dps_find(temp_inode, disk_offset, superblock, comp_string);
            } else {
                // we have reached a file
                target_name = strsep(&comp_string, "/");
                if (target_name == NULL) {
                    return temp_dir->d_ino;
                }
                return -1;
            }   
        } 
    }

    // didn't find anything, return -1.
    return -1;
}

void 
print_file(struct ufs2_dinode * inode, struct fs * superblock, char * disk_offset){
    char * file_offset = 0;
    int db_index = 0;
    int check_write;

    while (file_offset + superblock->fs_bsize < (char * ) inode->di_size){
        check_write = write(1, (char *)((inode->di_db[db_index] * superblock->fs_fsize) + disk_offset), superblock->fs_bsize);
        if (check_write == -1){
            perror("write");
            exit(EXIT_FAILURE);
        }

        db_index += 1;
        file_offset += superblock->fs_bsize;
    }

    check_write = write(1, (char *)((inode->di_db[db_index] * superblock->fs_fsize) + disk_offset), (char *) inode->di_size -  file_offset);
    if (check_write == -1){
        perror("write");
        exit(EXIT_FAILURE);
    }

    check_write = write(1, "\n", 1);
    if (check_write == -1){
        perror("write");
        exit(EXIT_FAILURE);
    }

}