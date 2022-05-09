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

    printf("Disk start = %x\n", disk_map);
    printf("Superblock = %x\n", superblock);
    printf("Mount point = %s\n", superblock->fs_fsmnt);

    printf("\n");

    printf("superblock->fs_cgsize = %d\n", superblock->fs_cgsize);
    printf("superblock->fs_bsize = %d\n", superblock->fs_bsize);
    printf("superblock->fs_fmsize = %d\n", superblock->fs_fsize);

    printf("\n");

    printf("ino_to_cg(superblock, UFS_ROOTINO) = %d\n", ino_to_cg(superblock, UFS_ROOTINO));
    printf("ino_to_fsba(superblock, UFS_ROOTINO) = %d\n", ino_to_fsba(superblock, UFS_ROOTINO));
    printf("ino_to_fsbo(superblock, UFS_ROOTINO) = %d\n", ino_to_fsbo(superblock, UFS_ROOTINO));
    printf("cgdata(superblock, ino_to_cg(superblock, UFS_ROOTINO)) = %d\n", cgdata(superblock, ino_to_cg(superblock, UFS_ROOTINO)));
    printf("cgbase(superblock, ino_to_cg(superblock, UFS_ROOTINO)) = %d\n", cgdata(superblock, ino_to_cg(superblock, UFS_ROOTINO)));

    printf("superblock->fs_iblkno = %d\n", superblock->fs_iblkno);
    printf("\n");
    
    // root_inode = (struct ufs2_dinode *) (disk_map + cgdata(superblock, ino_to_cg(superblock, UFS_ROOTINO)) + (ino_to_fsba(superblock, UFS_ROOTINO) * MINBSIZE) + (ino_to_fsbo(superblock, UFS_ROOTINO) * 512));
    // printf("Root inode = %x\n", root_inode); 
    // printf("Root inode di_nlink = %d\n", root_inode->di_nlink);
    // printf("Root inode di_atime = %d\n", root_inode->di_atime);
    // printf("Root inode di_size = %d\n", root_inode->di_size);
}
