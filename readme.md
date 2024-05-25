# Sisop-4-2024-MH-IT06
## Soal 3
## Archeology.c
```
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
```
- #define FUSE_USE_VERSION 28: Menentukan versi API FUSE yang digunakan.
- #include <fuse.h>: Menyertakan header FUSE untuk mendefinisikan fungsi-fungsi FUSE.
- #include <stdio.h>: Menyertakan header standar input/output C.
- #include <stdlib.h>: Menyertakan header untuk fungsi-fungsi umum (misalnya malloc, free).
- #include <string.h>: Menyertakan header untuk fungsi manipulasi string.
- #include <errno.h>: Menyertakan header untuk konstanta kesalahan sistem.
- #include <fcntl.h>: Menyertakan header untuk kontrol file.
- #include <dirent.h>: Menyertakan header untuk manipulasi direktori.
- #include <sys/types.h>: Menyertakan header untuk mendefinisikan tipe data dasar sistem.
- #include <sys/stat.h>: Menyertakan header untuk informasi status file.
- #include <unistd.h>: Menyertakan header untuk deklarasi standar POSIX API.
```int asprintf(char **strp, const char *fmt, ...);```
- Deklarasi fungsi asprintf yang akan digunakan untuk membuat string format dinamis.
```static const char *root_path = "/home/maximumyeet/tugas/modul4/relics/discoveries/relics";```
- Mendefinisikan path root direktori di mana sistem file akan beroperasi.
```static int relics_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    (void) offset;
    (void) fi;

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
```
- Fungsi ini dipanggil saat pembacaan direktori.
- filler digunakan untuk mengisi buffer dengan nama direktori dan file.
- Mengisi buffer dengan entri direktori . (current directory) dan .. (parent directory).
  ```
    DIR *dp;
    struct dirent *de;

    char *dir_path;
    if (asprintf(&dir_path, "%s%s", root_path, path) == -1)
        return -ENOMEM;
  ```
- Mendeklarasikan variabel untuk direktori dan entri direktori.
- Menggabungkan root_path dengan path untuk membentuk path penuh direktori yang akan dibaca.
```
    dp = opendir(dir_path);
    if (dp == NULL) {
        free(dir_path);
        return -errno;
    }
```
- Membuka direktori yang telah dibentuk. Jika gagal, membebaskan dir_path dan mengembalikan error.
```
    while ((de = readdir(dp)) != NULL) {
        if (strstr(de->d_name, ".000") != NULL) {
            char base_name[256];
            strncpy(base_name, de->d_name, strlen(de->d_name) - 4);
            base_name[strlen(de->d_name) - 4] = '\0';
            filler(buf, base_name, NULL, 0);
        }
    }
```
- Membaca entri direktori satu per satu.
- Jika entri berakhir dengan .000, maka nama file tanpa .000 ditambahkan ke buffer.
```
    closedir(dp);
    free(dir_path);
    return 0;
}
```
- Menutup direktori dan membebaskan dir_path. Mengembalikan 0 yang menandakan kesuksesan.
- Fungsi relics_getattr
```
static int relics_getattr(const char *path, struct stat *stbuf) {
    memset(stbuf, 0, sizeof(struct stat));
    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
    } else {
        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1;
        stbuf->st_size = 0;
```
- Menginisialisasi struktur stat dengan nol.
- Jika path adalah root (/), mengatur mode direktori dan jumlah link.
- Jika bukan root, mengatur mode file regular, jumlah link, dan ukuran file awal 0.
```
        char *full_path;
        if (asprintf(&full_path, "%s%s", root_path, path) == -1)
            return -ENOMEM;
```
Membentuk path penuh untuk file.
```
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
```
- Menginisialisasi iterasi dan variabel untuk path bagian file.
- Loop untuk membuka setiap bagian file yang dinamai dengan format .000, .001, dst.
- Menambahkan ukuran setiap bagian ke ukuran total file.
```
        free(full_path);

        if (i == 0)
            return -ENOENT;
    }
    return 0;
}
```
- Membebaskan path penuh.
- Jika tidak ada bagian file ditemukan (i == 0), mengembalikan error ENOENT.
- Mengembalikan 0 untuk menandakan kesuksesan.
```
static int relics_open(const char *path, struct fuse_file_info *fi) {
    return 0;
}
```
- Fungsi ini dipanggil saat membuka file. Tidak ada yang dilakukan di sini, hanya mengembalikan 0.
```
static int relics_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    (void) fi;
    char *full_path;
    if (asprintf(&full_path, "%s%s", root_path, path) == -1)
        return -ENOMEM;
```
- Fungsi ini dipanggil saat membaca file.
- Membentuk path penuh untuk file.
  ```
    size_t total_read = 0;
    size_t part_offset = offset % 4096;
    int part_num = offset / 4096;
    char *part_path;
    FILE *fp;
  ```
- Menginisialisasi variabel untuk membaca file bagian-bagian.
- part_offset adalah offset dalam bagian file.
- part_num adalah nomor bagian file berdasarkan offset.
```
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
```
- Loop untuk membuka setiap bagian file.
- Membentuk path bagian file dan membuka file.
```
        fseek(fp, part_offset, SEEK_SET);
        size_t to_read = (4096 - part_offset) < size ? (4096 - part_offset) : size;
        size_t read_size = fread(buf + total_read, 1, to_read, fp);
        total_read += read_size;
        size -= read_size;
        part_offset = 0;
```
- Mengatur offset dalam file bagian.
- Menentukan berapa banyak data yang akan dibaca dari file bagian.
- Membaca data ke buffer.
- Memperbarui ukuran yang tersisa dan total yang sudah dibaca.
```
        fclose(fp);
        free(part_path);

        if (read_size < to_read) {
            break;
        }
    }

    free(full_path);

    if (total_read == 0)
        return -ENOENT;

    return total_read;
}
```
- Menutup file bagian dan membebaskan path bagian.
- Jika jumlah yang dibaca lebih kecil dari yang diminta, loop berhenti.
- Membebaskan path penuh.
- Jika tidak ada yang dibaca (total_read == 0), mengembalikan error ENOENT.
- Mengembalikan total yang dibaca.
### Struktur Operasi FUSE
```
static struct fuse_operations relics_oper = {
    .getattr = relics_getattr,
    .readdir = relics_readdir,
    .open = relics_open,
    .read = relics_read,
};
```
- Mendefinisikan struktur operasi FUSE yang berisi pointer ke fungsi-fungsi yang diimplementasikan.
### Fungsi main
```
int main(int argc, char *argv[]) {
    return fuse_main(argc, argv, &relics_oper, NULL);
}
```
 Fungsi main memanggil fuse_main dengan argumen dan struktur operasi yang telah didefinisikan untuk memulai sistem file.







