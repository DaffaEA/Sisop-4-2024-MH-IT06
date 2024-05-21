#define FUSE_USE_VERSION 28

#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// Deklarasi untuk asprintf
int asprintf(char **strp, const char *fmt, ...);

static const char *root_path = "/home/maximumyeet/tugas/modul4/relics/discoveries/relics"; // Ganti dengan path sebenarnya

static int relics_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    (void) offset;
    (void) fi;

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    DIR *dp;
    struct dirent *de;

    char *dir_path;
    if (asprintf(&dir_path, "%s%s", root_path, path) == -1)
        return -ENOMEM;

    dp = opendir(dir_path);
    if (dp == NULL) {
        free(dir_path);
        return -errno;
    }

    while ((de = readdir(dp)) != NULL) {
        if (strstr(de->d_name, ".000") != NULL) {
            char base_name[256];
            strncpy(base_name, de->d_name, strlen(de->d_name) - 4);
            base_name[strlen(de->d_name) - 4] = '\0';
            filler(buf, base_name, NULL, 0);
        }
    }

    closedir(dp);
    free(dir_path);
    return 0;
}

static int relics_getattr(const char *path, struct stat *stbuf) {
    memset(stbuf, 0, sizeof(struct stat));
    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
    } else {
        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1;
        stbuf->st_size = 0;

        char *full_path;
        if (asprintf(&full_path, "%s%s", root_path, path) == -1)
            return -ENOMEM;

        int i = 0;
        char *part_path;
        FILE *fp;

        while (1) {
            if (asprintf(&part_path, "%s.%03d", full_path, i++) == -1) {
                free(full_path);
                return -ENOMEM;
            }
            fp = fopen(part_path, "rb");
            if (!fp) {
                free(part_path);
                break;
            }
            fseek(fp, 0L, SEEK_END);
            stbuf->st_size += ftell(fp);
            fclose(fp);
            free(part_path);
            i++;
        }

        free(full_path);

        if (i == 0)
            return -ENOENT;
    }
    return 0;
}

static int relics_open(const char *path, struct fuse_file_info *fi) {
    return 0;
}

static int relics_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    (void) fi;
    char *full_path;
    if (asprintf(&full_path, "%s%s", root_path, path) == -1)
        return -ENOMEM;

    size_t total_read = 0;
    size_t part_offset = offset % 4096;
    int part_num = offset / 4096;
    char *part_path;
    FILE *fp;

    while (size > 0) {
        if (asprintf(&part_path, "%s.%03d", full_path, part_num++) == -1) {
            free(full_path);
            return -ENOMEM;
        }
        fp = fopen(part_path, "rb");
        if (!fp) {
            free(part_path);
            break;
        }

        fseek(fp, part_offset, SEEK_SET);
        size_t to_read = (4096 - part_offset) < size ? (4096 - part_offset) : size;
        size_t read_size = fread(buf + total_read, 1, to_read, fp);
        total_read += read_size;
        size -= read_size;
        part_offset = 0;

        fclose(fp);
        free(part_path);

        if (read_size < to_read) {
            break; // End of file reached or read error
        }
    }

    free(full_path);

    if (total_read == 0)
        return -ENOENT;

    return total_read;
}

static struct fuse_operations relics_oper = {
    .getattr = relics_getattr,
    .readdir = relics_readdir,
    .open = relics_open,
    .read = relics_read,
};

int main(int argc, char *argv[]) {
    return fuse_main(argc, argv, &relics_oper, NULL);
}
