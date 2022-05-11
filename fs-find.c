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

int
main(int argc, char *argv[])
{
    struct fs *superblock;
    struct stat st;
    char *disk_map;
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
    
    disk_map = mmap(0, file_size, PROT_READ, MAP_SHARED, fd, 0);
    if(disk_map == MAP_FAILED){
        perror("mmap");
        return 1;
    }
    
    superblock = (struct fs *) (disk_map + SBLOCK_UFS2);
    root_inode = (struct ufs2_dinode *) (inode_to_offset(superblock, UFS_ROOTINO) + disk_map);

    printf("Root inode = %x\n", root_inode); 
    printf("Root inode di_nlink = %d\n", root_inode->di_nlink);
    printf("Root inode di_atime = %d\n", root_inode->di_atime);
    printf("Root inode di_size = %d\n", root_inode->di_size);
    printf("Is it a directory = %s\n", is_inode_dir(root_inode) ? "true" : "false");
}

long
inode_to_offset(struct fs *superblock, int inode_num){
    return ((ino_to_fsba(superblock, inode_num) * superblock->fs_fsize) + (ino_to_fsbo(superblock, inode_num) * sizeof(struct ufs2_dinode)));
}

int
is_inode_dir(struct ufs2_dinode * inode){
    return (inode->di_mode & IFMT) == IFDIR;
}