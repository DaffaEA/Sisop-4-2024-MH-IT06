#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <limits.h>

static int xmp_getattr(const char *path, struct stat *stbuf)
{
    char full_path[PATH_MAX];
    sprintf(full_path, "/home/shittim/Sisop4/portofolio/%s", path);
    int res = lstat(full_path, stbuf);
    if (res == -1) return -errno;
    return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
    char full_path[PATH_MAX];
    sprintf(full_path, "/home/shittim/Sisop4/portofolio/%s", path);
    DIR *dp;
    struct dirent *de;
    (void) offset;
    (void) fi;

    dp = opendir(full_path);
    if (dp == NULL) return -errno;

    while ((de = readdir(dp)) != NULL) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        if (filler(buf, de->d_name, &st, 0)) break;
    }
    closedir(dp);
    return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    char full_path[PATH_MAX];
    sprintf(full_path, "/home/shittim/Sisop4/portofolio/%s", path);
    int fd;
    int res;
    (void) fi;

    fd = open(full_path, O_RDONLY);
    if (fd == -1) return -errno;

    res = pread(fd, buf, size, offset);
    if (res == -1) res = -errno;

    close(fd);
    return res;
}

int add_watermark(const char *filepath)
{
    char command[1024];
    snprintf(command, sizeof(command), "convert '%s' -gravity South -pointsize 140 -fill white -annotate +0+100 'inikaryakita.id' '%s'", filepath, filepath);
    return system(command);
}

static int xmp_rename(const char *from, const char *to)
{
    char full_from[PATH_MAX];
    char full_to[PATH_MAX];
    sprintf(full_from, "/home/shittim/Sisop4/portofolio/%s", from);
    sprintf(full_to, "/home/shittim/Sisop4/portofolio/%s", to);
    int res;

    // Check if the target directory has the prefix "wm."
    if (strstr(to, "wm.") != NULL) {
        // Rename (move) the file
        res = rename(full_from, full_to);
        if (res == -1) return -errno;
        
        // Add watermark to the moved file
        res = add_watermark(full_to);
        if (res != 0) return -errno;
    } else {
        res = rename(full_from, full_to);
        if (res == -1) return -errno;
    }
    return res;
}

static int xmp_mkdir(const char *path, mode_t mode)
{
    char full_path[PATH_MAX];
    sprintf(full_path, "/home/shittim/Sisop4/portofolio/%s", path);
    int res;
    res = mkdir(full_path, mode);
    if (res == -1) return -errno;
    return 0;
}

static int xmp_unlink(const char *path)
{
    char full_path[PATH_MAX];
    sprintf(full_path, "/home/shittim/Sisop4/portofolio/%s", path);
    int res;
    res = unlink(full_path);
    if (res == -1) return -errno;
    return 0;
}

static int xmp_rmdir(const char *path)
{
    char full_path[PATH_MAX];
    sprintf(full_path, "/home/shittim/Sisop4/portofolio/%s", path);
    int res;
    res = rmdir(full_path);
    if (res == -1) return -errno;
    return 0;
}

static int xmp_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    char full_path[PATH_MAX];
    sprintf(full_path, "/home/shittim/Sisop4/portofolio/%s", path);
    int fd;
    fd = open(full_path, fi->flags, mode);
    if (fd == -1) return -errno;
    fi->fh = fd;
    return 0;
}

static int xmp_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    int res;
    res = pwrite(fi->fh, buf, size, offset);
    if (res == -1) return -errno;
    return res;
}

static struct fuse_operations xmp_oper = {
    .getattr = xmp_getattr,
    .readdir = xmp_readdir,
    .read = xmp_read,
    .rename = xmp_rename,
    .mkdir = xmp_mkdir,
    .unlink = xmp_unlink,
    .rmdir = xmp_rmdir,
    .create = xmp_create,
    .write = xmp_write,
};

int main(int argc, char *argv[])
{
    umask(0);
    return fuse_main(argc, argv, &xmp_oper, NULL);
}