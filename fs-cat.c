/*
 * fs-find.c
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

int
main(int argc, char *argv[])
{
    struct fs *superblock;
    struct stat st;
    char *disk_offset;
    int fd;
    int file_size;

    struct ufs2_dinode *root_inode;

    fd = open(argv[1], O_RDWR);
    if(fd == -1){
        perror("open");
        return 1;
    }

    if (stat(argv[1], &st) != 0){
        perror("stat");
    }
    file_size = st.st_size;
    
    disk_offset = mmap(0, file_size, PROT_READ, MAP_SHARED, fd, 0);
    if(disk_offset == MAP_FAILED){
        perror("mmap");
        return 1;
    }
    
    superblock = (struct fs *) (disk_offset + SBLOCK_UFS2);
    root_inode = (struct ufs2_dinode *) (inode_to_offset(superblock, UFS_ROOTINO) + disk_offset);


    printf("Argv2 %s\n", argv[2]);
    fflush(stdout);

    int file_ino_num = dps_find(root_inode, disk_offset, superblock, argv[2]);

    printf("Argv2 %s\n", argv[2]);
    fflush(stdout);

    struct ufs2_dinode *file_ino = (struct ufs2_dinode *)(inode_to_offset(superblock, file_ino_num) + disk_offset);

    printf("Files Inode = %d\n", file_ino_num);
    printf("file_ino->di_mtime %d\n", file_ino->di_ctime);
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
    char * temp_name;
    int temp_reclen;

    temp_name = strsep(&comp_string, "/");
    if (temp_name == NULL){
        return -1;
    }

    printf("temp_name = %s\n", temp_name);

    if (!is_inode_dir(inode)) {
        char* next = strsep(&comp_string, "/");
        if ((strcmp(temp_name, temp_dir->d_name) == 0) && (next == NULL)) {
            return temp_dir->d_ino;
        } else {
            return -1;
        }
    }
    else{
        root_dir = (struct direct *)((inode->di_db[0] * superblock->fs_fsize) + disk_offset);
        temp_reclen = root_dir->d_reclen;


        while(temp_reclen < sizeof(struct direct)){
            temp_dir = (struct direct *)((inode->di_db[0] * superblock->fs_fsize) + disk_offset + temp_reclen);
            temp_reclen += temp_dir->d_reclen;

            printf("f1\n");
            fflush(stdout);

            if ((strcmp(temp_dir->d_name,".") == 0) || (strcmp(temp_dir->d_name,"..") == 0)) {
                continue;
            }

            printf("f2\n");
            fflush(stdout);

            if (strcmp(temp_name, temp_dir->d_name) == 0){
                temp_inode = (struct ufs2_dinode *) (inode_to_offset(superblock, temp_dir->d_ino) + disk_offset);
                dps_find(temp_inode, disk_offset, superblock, comp_string);
            }  

            printf("f3\n");
            fflush(stdout);

            printf("comp_string: %s\n", comp_string);
            fflush(stdout); 
        }
    }
}