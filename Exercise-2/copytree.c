#include "copytree.h"
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void copy_file(const char *src, const char *dest, int copy_symlinks, int copy_permissions) {
    // If copy_symlinks is true, copy the symlink as a symlink
    if (copy_symlinks) {
        struct stat statbuf;

        // Check if the source is a symlink
        if (lstat(src, &statbuf) == -1) {
            perror("lstat failed");
            exit(EXIT_FAILURE);
        }

        // If the source is a symlink, copy it as a symlink
        if (S_ISLNK(statbuf.st_mode)) {
            char link_target[4096];
            ssize_t len = readlink(src, link_target, sizeof(link_target) - 1);
            if (len == -1) {
                perror("readlink failed");
                exit(EXIT_FAILURE);
            }
            link_target[len] = '\0';
            if (symlink(link_target, dest) == -1) {
                perror("symlink failed");
                exit(EXIT_FAILURE);
            }
            return;
        }
    }

    // If copy_symlinks is false or the source is not a symlink, copy the file
    int src_fd = open(src, O_RDONLY);
    if (src_fd == -1) {
        perror("open src failed");
        exit(EXIT_FAILURE);
    }

    // Create the destination file
    int dest_fd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (dest_fd == -1) {
        perror("open dest failed");
        close(src_fd);
        exit(EXIT_FAILURE);
    }

    char buffer[4096];
    ssize_t read_bytes;
    // Copy the file
    while ((read_bytes = read(src_fd, buffer, sizeof(buffer))) > 0) {
        if (write(dest_fd, buffer, read_bytes) != read_bytes) {
            perror("write failed");
            close(src_fd);
            close(dest_fd);
            exit(EXIT_FAILURE);
        }
    }
    // Check if read failed
    if (read_bytes == -1) {
        perror("read failed");
        close(src_fd);
        close(dest_fd);
        exit(EXIT_FAILURE);
    }

    // Copy the permissions
    if (copy_permissions) {
        struct stat statbuf;
        if (fstat(src_fd, &statbuf) == -1) {
            perror("fstat failed");
            close(src_fd);
            close(dest_fd);
            exit(EXIT_FAILURE);
        }
        if (fchmod(dest_fd, statbuf.st_mode) == -1) {
            perror("fchmod failed");
            close(src_fd);
            close(dest_fd);
            exit(EXIT_FAILURE);
        }
    }

    close(src_fd);
    close(dest_fd);
}

void copy_directory(const char *src, const char *dest, int copy_symlinks, int copy_permissions) {
    DIR *dir = opendir(src);
    if (!dir) {
        perror("opendir");
        exit(0);
    }

    struct stat src_stat;
    if (stat(src, &src_stat) == -1) {
        perror("stat");
        closedir(dir);
        exit(0);
    }

    if (copy_permissions) {
        if (mkdir(dest, src_stat.st_mode) == -1) {
            perror("mkdir");
            closedir(dir);
            exit(0);
        }
    } else {
        if (mkdir(dest, 0755) == -1) {
            perror("mkdir");
            closedir(dir);
            exit(0);
        }
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char src_path[4096];
        snprintf(src_path, sizeof(src_path), "%s/%s", src, entry->d_name);

        char dest_path[4096];
        snprintf(dest_path, sizeof(dest_path), "%s/%s", dest, entry->d_name);

        struct stat statbuf;
        if (lstat(src_path, &statbuf) == -1) {
            perror("lstat");
            continue;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            copy_directory(src_path, dest_path, copy_symlinks, copy_permissions);
        } else {
            copy_file(src_path, dest_path, copy_symlinks, copy_permissions);
        }
    }
}