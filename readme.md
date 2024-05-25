# Sisop-4-2024-MH-IT06

## Soal 1
## inikaryakita.c
```c
static int xmp_getattr(const char *path, struct stat *stbuf)
{
    char full_path[PATH_MAX];
    snprintf(full_path, sizeof(full_path), "/home/shittim/Sisop4/portofolio/%s", path);
    int res = lstat(full_path, stbuf);
    if (res == -1) return -errno;
    return 0;
}
```
-  Fungsi xmp_getattr digunakan untuk mengambil atribut atau informasi file dari jalur relatif yang diberikan
-  Menggabungkannya dengan direktori dasar yang telah ditentukan
-  Mengisi struktur stat dengan informasi tersebut

```c
static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
    char full_path[PATH_MAX];
    snprintf(full_path, sizeof(full_path), "/home/shittim/Sisop4/portofolio/%s", path);
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
```
- Fungsi xmp_readdir digunakan untuk membaca isi direktori yang jalurnya diberikan relatif,menggabungkannya dengan direktori dasar yang telah ditentukan
- Membuka direktori tersebut
- Membaca setiap entri direktori
- Mengisi buffer dengan nama entri direktori dan informasi terkait menggunakan fungsi filler.

```c
static int xmp_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    char full_path[PATH_MAX];
    snprintf(full_path, sizeof(full_path), "/home/shittim/Sisop4/portofolio/%s", path);
    int fd;
    int res;
    (void) fi;

    fd = open(full_path, O_RDONLY);
    if (fd == -1) return -errno;

    const char *filename = strrchr(path, '/') ? strrchr(path, '/') + 1 : path;
    if (strstr(filename, "test") == filename && strstr(filename, ".txt") == filename + strlen(filename) - 4) {
 
        char *file_buf = malloc(size);
        if (file_buf == NULL) {
            close(fd);
            return -ENOMEM;
        }

        res = pread(fd, file_buf, size, 0);
        if (res == -1) {
            free(file_buf);
            close(fd);
            return -errno;
        }


        size_t file_size = res;
        for (size_t i = 0; i < file_size; ++i) {
            buf[i] = file_buf[file_size - 1 - i];
        }


        if (file_size > size) {
            file_size = size;
        }

        free(file_buf);
        close(fd);
        return file_size;
    } else {

        res = pread(fd, buf, size, offset);
        if (res == -1) res = -errno;

        close(fd);
        return res;
    }
}
```
- Menggabungkan jalur dasar dengan path untuk mendapatkan jalur lengkap.
- Membuka berkas dalam mode baca saja dengan open(full_path, O_RDONLY);
- Mendapatkan nama berkas dari path dengan strrchr(path, '/').
- Memeriksa apakah nama berkas diawali dengan "test" dan diakhiri dengan ".txt".
- Jika berkas sesuai kriteria, alokasi buffer untuk membaca isi berkas (file_buf).
    - Membaca isi berkas ke dalam file_buf menggunakan pread.
    - Jika operasi baca gagal, membersihkan dan mengembalikan kesalahan.
    - Membalik isi file_buf ke dalam buf (buffer yang diberikan).
    - Mengembalikan ukuran data yang dibalik.
- Jika berkas tidak sesuai kriteria, membaca isi berkas secara normal ke dalam buf menggunakan pread.
- Mengembalikan hasil operasi baca atau kesalahan jika terjadi.
- Berkas selalu ditutup setelah operasi selesai.
- Memori yang dialokasikan untuk file_buf dibebaskan setelah digunakan.

```c
int add_watermark(const char *filepath)
{
    char command[1024];
    snprintf(command, sizeof(command), "convert '%s' -gravity South -pointsize 80 -fill white -annotate +0+100 'inikaryakita.id' '%s'", filepath, filepath);
    int res = system(command);
    if (res != 0) {
        fprintf(stderr, "Error adding watermark to file: %s\n", filepath);
        return -1;
    }
    return 0;
}
```
- Mendefinisikan buffer command dengan ukuran 1024 karakter untuk menyimpan perintah sistem.
- Menggunakan snprintf untuk membentuk perintah ImageMagick convert yang akan menambahkan watermark ke gambar.
- Menjalankan perintah yang telah dibentuk menggunakan fungsi system.

```c
static int xmp_rename(const char *from, const char *to)
{
    char full_from[PATH_MAX];
    char full_to[PATH_MAX];
    snprintf(full_from, sizeof(full_from), "/home/shittim/Sisop4/portofolio/%s", from);
    snprintf(full_to, sizeof(full_to), "/home/shittim/Sisop4/portofolio/%s", to);
    int res;

    if (strstr(to, "wm") != NULL) {

        res = rename(full_from, full_to);
        if (res == -1) return -errno;
        

        res = add_watermark(full_to);
        if (res != 0) return -errno;
    } else {
        res = rename(full_from, full_to);
        if (res == -1) return -errno;
    }
    return res;
}
```
- Mendefinisikan buffer full_from dan full_to dengan ukuran PATH_MAX untuk menyimpan path lengkap file asli dan file tujuan.
- Menggunakan snprintf untuk membentuk path lengkap dari file asli (full_from) dan file tujuan (full_to).
- Mengecek apakah string to mengandung substring "wm":
- Jika ya:
    - Memanggil fungsi rename untuk mengganti nama file dari full_from ke full_to.
    - Memanggil fungsi add_watermark untuk menambahkan watermark ke file yang baru saja diganti namanya.
- Jika tidak:
    - Memanggil fungsi rename untuk mengganti nama file dari full_from ke full_to.

```c
static int xmp_mkdir(const char *path, mode_t mode)
{
    char full_path[PATH_MAX];
    snprintf(full_path, sizeof(full_path), "/home/shittim/Sisop4/portofolio/%s", path);
    int res;
    res = mkdir(full_path, mode);
    if (res == -1) return -errno;
    return 0;
}
```
- Mendefinisikan buffer full_path dengan ukuran PATH_MAX untuk menyimpan path lengkap direktori baru.
- Menggunakan snprintf untuk membentuk path lengkap dari direktori baru (full_path).
- Memanggil fungsi mkdir untuk membuat direktori baru dengan path lengkap dan mode yang diberikan.

```c
static int xmp_unlink(const char *path)
{
    char full_path[PATH_MAX];
    snprintf(full_path, sizeof(full_path), "/home/shittim/Sisop4/portofolio/%s", path);
    int res;
    res = unlink(full_path);
    if (res == -1) return -errno;
    return 0;
}
```
- Menggunakan snprintf untuk membentuk path lengkap dari file yang akan dihapus (full_path).
- Memanggil fungsi unlink untuk menghapus file dengan path lengkap yang telah dibentuk

```c
static int xmp_rmdir(const char *path)
{
    char full_path[PATH_MAX];
    snprintf(full_path, sizeof(full_path), "/home/shittim/Sisop4/portofolio/%s", path);
    int res;
    res = rmdir(full_path);
    if (res == -1) return -errno;
    return 0;
}
```
- Menggunakan snprintf untuk membentuk path lengkap dari direktori yang akan dihapus (full_path).
- Memanggil fungsi rmdir untuk menghapus direktori kosong dengan path lengkap yang telah dibentuk.

```c
static int xmp_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    char full_path[PATH_MAX];
    snprintf(full_path, sizeof(full_path), "/home/shittim/Sisop4/portofolio/%s", path);
    int fd;
    fd = open(full_path, fi->flags, mode);
    if (fd == -1) return -errno;
    fi->fh = fd;
    return 0;
}
```
- Menggunakan snprintf untuk membentuk path lengkap dari file yang akan dibuat (full_path).
- Memanggil fungsi open untuk membuat file baru dengan path lengkap yang telah dibentuk, serta mode dan flag yang diberikan.

```c
static int xmp_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    int res;
    res = pwrite(fi->fh, buf, size, offset);
    if (res == -1) return -errno;
    return res;
}
```
- Memanggil fungsi pwrite untuk menulis size byte data dari buffer buf ke file pada offset yang ditentukan.

```c
static int xmp_chmod(const char *path, mode_t mode)
{
    char full_path[PATH_MAX];
    snprintf(full_path, sizeof(full_path), "/home/shittim/Sisop4/portofolio/%s", path);
    int res;
    res = chmod(full_path, mode);
    if (res == -1) return -errno;
    return 0;
}
```
- Menggunakan snprintf untuk membentuk path lengkap dari file atau direktori yang izinnya akan diubah (full_path).
- Memanggil fungsi chmod untuk mengubah izin dari file atau direktori dengan path lengkap yang telah dibentuk.

```c
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
    .chmod = xmp_chmod,
};
```
- Struktur fuse_operations digunakan untuk mendefinisikan fungsi-fungsi callback yang diimplementasikan dalam sistem berkas FUSE (Filesystem in Userspace).

```c
int main(int argc, char *argv[])
{
    umask(0);
    return fuse_main(argc, argv, &xmp_oper, NULL);
}
```
- Fungsi ini mengatur mask izin file dan kemudian memanggil fuse_main untuk menjalankan sistem berkas FUSE dengan operasi yang telah ditentukan.

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







