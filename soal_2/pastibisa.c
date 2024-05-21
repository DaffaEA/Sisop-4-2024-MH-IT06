#define FUSE_USE_VERSION 31

#include <fuse3/fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

static char dirpath[1000];
char secret_password[100] = "inipassword";
int password_entered = 0;

void buatLog(const char *status, const char *activity, const char *info) {
    FILE *log_file = fopen("/path/to/logs-fuse.log", "a");
    if (log_file == NULL) {
        perror("Error opening log file");
        return;
    }

    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%d/%m/%Y-%H:%M:%S", local_time);

    fprintf(log_file, "[%s]::%s::[%s]::[%s]\n", status, timestamp, activity, info);
    fclose(log_file);
}

int check_password(const char *path) {
    if (!password_entered) {
        char input_password[100];
        printf("Masukkan kata sandi: ");
        scanf("%99s", input_password);
        if (strcmp(input_password, secret_password) != 0) {
            printf("Kata sandi salah. Akses ditolak.\n");
            buatLog("FAILED", "access", path);
            return 0;
        }
        password_entered = 1;
        buatLog("SUCCESS", "access", path);
    }
    return 1;
}

static int getattr_eazy(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
    (void) fi; // Unused parameter
    char fpath[1000];
    sprintf(fpath, "%s%s", dirpath, path);
    int res = lstat(fpath, stbuf);
    if (res == -1)
        return -errno;
    return 0;
}

static int readdir_eazy(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags) {
    (void) offset;
    (void) fi;
    (void) flags;

    char fpath[1000];
    if (strcmp(path, "/") == 0) {
        path = dirpath;
        sprintf(fpath, "%s", path);
    } else {
        sprintf(fpath, "%s%s", dirpath, path);
    }

    if (strstr(fpath, "rahasia") != NULL && !check_password(path)) {
        return -EACCES;
    }

    DIR *dp = opendir(fpath);
    if (dp == NULL)
        return -errno;

    struct dirent *de;
    while ((de = readdir(dp)) != NULL) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        if (filler(buf, de->d_name, &st, 0, FUSE_FILL_DIR_PLUS))
            break;
    }

    closedir(dp);
    password_entered = 0;
    return 0;
}

static int open_eazy(const char *path, struct fuse_file_info *fi) {
    char fpath[1000];
    sprintf(fpath, "%s%s", dirpath, path);
    if (strstr(fpath, "rahasia") != NULL && !check_password(path)) {
        return -EACCES;
    }
    int res = open(fpath, fi->flags);
    if (res == -1)
        return -errno;
    close(res);
    return 0;
}

static void decrypt_file_content(const char *path, char *buf, size_t size) {
    char *filename = strrchr(path, '/');
    if (filename != NULL) {
        filename++;
        if (strstr(filename, "base64") != NULL) {
            // Implementasi manual untuk decode Base64
            const char *base64_chars = 
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz"
                "0123456789+/";
            int in_len = size;
            int i = 0, j = 0;
            int in_ = 0;
            unsigned char char_array_4[4], char_array_3[3];

            while (in_len-- && (buf[in_] != '=') && isalnum(buf[in_])) {
                char_array_4[i++] = buf[in_]; in_++;
                if (i == 4) {
                    for (i = 0; i < 4; i++)
                        char_array_4[i] = strchr(base64_chars, char_array_4[i]) - base64_chars;

                    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                    for (i = 0; (i < 3); i++)
                        buf[j++] = char_array_3[i];
                    i = 0;
                }
            }

            if (i) {
                for (int k = i; k < 4; k++)
                    char_array_4[k] = 0;

                for (int k = 0; k < 4; k++)
                    char_array_4[k] = strchr(base64_chars, char_array_4[k]) - base64_chars;

                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                for (int k = 0; (k < i - 1); k++)
                    buf[j++] = char_array_3[k];
            }
            buf[j] = '\0';
        } else if (strstr(filename, "rot13") != NULL) {
            for (size_t i =        0; i < size; i++) {
                if (isalpha(buf[i])) {
                    if (islower(buf[i])) {
                        buf[i] = 'a' + (buf[i] - 'a' + 13) % 26;
                    } else {
                        buf[i] = 'A' + (buf[i] - 'A' + 13) % 26;
                    }
                }
            }
        } else if (strstr(filename, "hex") != NULL) {
            size_t decoded_size = size / 2;
            char *decoded_text = (char *)malloc(decoded_size);
            for (size_t i = 0, j = 0; i < size; i += 2, j++) {
                sscanf(&buf[i], "%2hhx", &decoded_text[j]);
            }
            memcpy(buf, decoded_text, decoded_size);
            buf[decoded_size] = '\0';
            free(decoded_text);
        } else if (strstr(filename, "rev") != NULL) {
            size_t len = strlen(buf);
            for (size_t i = 0; i < len / 2; i++) {
                char temp = buf[i];
                buf[i] = buf[len - i - 1];
                buf[len - i - 1] = temp;
            }
        }
    }
}

static int read_eazy(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    char fpath[1000];
    sprintf(fpath, "%s%s", dirpath, path);
    int fd = open(fpath, O_RDONLY);
    if (fd == -1)
        return -errno;

    int res = pread(fd, buf, size, offset);
    if (res == -1) {
        close(fd);
        return -errno;
    }

    if (strstr(path, "base64") != NULL || strstr(path, "rot13") != NULL || strstr(path, "hex") != NULL || strstr(path, "rev") != NULL) {
        decrypt_file_content(path, buf, res);
    }

    close(fd);
    return res;
}

static struct fuse_operations operations = {
    .getattr = getattr_eazy,
    .readdir = readdir_eazy,
    .open = open_eazy,
    .read = read_eazy,
};

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <mountpoint> <dirpath>\n", argv[0]);
        return 1;
    }

    strcpy(dirpath, argv[2]);

    umask(0);
    return fuse_main(argc, argv, &operations, NULL);
}

