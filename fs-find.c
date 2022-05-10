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

    printf("File size: %d\n", file_size);
    
    disk_map = mmap(0, file_size, PROT_READ, MAP_SHARED, fd, 0);
    if(disk_map == MAP_FAILED){
        perror("mmap");
        return 1;
    }
    
    superblock = (struct fs *) (disk_map + SBLOCK_UFS2);

    printf("\n");

    printf("inode number to cylinder group number = %d\n", ino_to_cg(superblock, UFS_ROOTINO));
    printf("inode number to filesystem block address = %d\n", ino_to_fsba(superblock, UFS_ROOTINO));
    printf("inode number to filesystem block offset = %d\n", ino_to_fsbo(superblock, UFS_ROOTINO));
    printf("Cylinder group base zone = %d\n", cgbase(superblock, UFS_ROOTINO));

    printf("\n ________________________________________\n\n");
    long cylinder_offset = cgbase(superblock, ino_to_cg(superblock, UFS_ROOTINO)) * superblock->fs_bsize;
    long total_block_offset = (ino_to_fsba(superblock, UFS_ROOTINO) * superblock->fs_bsize) + (ino_to_fsbo(superblock, UFS_ROOTINO) * superblock->fs_fsize);
    long total_disk_offset = cylinder_offset + total_block_offset;



    root_inode = (struct ufs2_dinode *) (inode_to_offset(superblock, UFS_ROOTINO) + disk_map);

    printf("Root inode = %x\n", root_inode); 
    printf("Root inode di_nlink = %d\n", root_inode->di_nlink);
    printf("Root inode di_atime = %d\n", root_inode->di_atime);
    printf("Root inode di_size = %d\n", root_inode->di_size);
}


long
inode_to_offset(struct fs *superblock, int inode_num){
    long total_block_offset = ((ino_to_fsba(superblock, UFS_ROOTINO) * superblock->fs_fsize) + (ino_to_fsbo(superblock, UFS_ROOTINO) * sizeof(struct ufs2_dinode)));

    printf("total_disk_offset = %x\n", total_block_offset);
    return total_block_offset;
}