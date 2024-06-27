#include "buffered_open.h"
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// Function to wrap the original open function
buffered_file_t *buffered_open(const char *pathname, int flags, mode_t mode) {
    int orginal_flags = flags;
    flags &= ~O_PREAPPEND;
    int fd = open(pathname, flags, mode);
    if (fd == -1) {
        return NULL;
    }
    buffered_file_t *bf = (buffered_file_t *)malloc(sizeof(buffered_file_t));
    if (bf == NULL) {
        close(fd);
        return NULL;
    }
    bf->fd = fd;
    bf->read_buffer = (char *)malloc(BUFFER_SIZE);
    if (bf->read_buffer == NULL) {
        close(fd);
        free(bf);
        return NULL;
    }
    bf->write_buffer = (char *)malloc(BUFFER_SIZE);
    if (bf->write_buffer == NULL) {
        close(fd);
        free(bf->read_buffer);
        free(bf);
        return NULL;
    }
    bf->read_buffer_size = BUFFER_SIZE;
    bf->write_buffer_size = BUFFER_SIZE;
    bf->read_buffer_pos = 0;
    bf->write_buffer_pos = 0;
    bf->flags = flags;
    bf->preappend = ((orginal_flags & O_PREAPPEND) == O_PREAPPEND);
    bf->last_op = 0;
    return bf;
}

// Function to write to the buffered file
ssize_t buffered_write(buffered_file_t *bf, const void *buf, size_t count) {
    const char *data = (const char *)buf;
    size_t written = 0;
    bf->last_op = 1;
    while (count > 0) {
        size_t space_left = bf->write_buffer_size - bf->write_buffer_pos;
        size_t to_copy;
        if (count < space_left) {
            to_copy = count;
        } else {
            to_copy = space_left;
        }
        memcpy(bf->write_buffer + bf->write_buffer_pos, data + written, to_copy);
        bf->write_buffer_pos += to_copy;
        written += to_copy;
        count -= to_copy;
        if (bf->write_buffer_pos == bf->write_buffer_size) {
            if (buffered_flush(bf) == -1)
                return -1;
        }
    }
    return written;
}

// Function to read from the buffered file
ssize_t buffered_read(buffered_file_t *bf, void *buf, size_t count) {
    // Cast the buffer to a char pointer
    char *data = (char *)buf;

    // Initialize the number of bytes read
    size_t read_bytes = 0;

    // Check if the last operation was a write
    // If so, flush the buffer to the file
    if (bf->last_op == 1) {
        if (buffered_flush(bf) == -1) {
            return -1;
        }
    }
    while (count > 0) {
        if (bf->read_buffer_pos == bf->read_buffer_size || bf->last_op == 0) {
            bf->last_op = 2;
            ssize_t bytes_read = read(bf->fd, bf->read_buffer, BUFFER_SIZE);
            if (bytes_read <= 0) {
                if (read_bytes > 0) {
                    return read_bytes;
                } else {
                    return bytes_read;
                }
            }
            bf->read_buffer_pos = 0;
            bf->read_buffer_size = bytes_read;
        }

        size_t space_left = bf->read_buffer_size - bf->read_buffer_pos;
        size_t to_copy;
        if (count < space_left) {
            to_copy = count;
        } else {
            to_copy = space_left;
        }

        memcpy(data + read_bytes, bf->read_buffer + bf->read_buffer_pos, to_copy);
        bf->read_buffer_pos += to_copy;
        read_bytes += to_copy;
        count -= to_copy;
    }
    bf->last_op = 2;
    return read_bytes;
}

// Function to flush the buffer to the file
int buffered_flush(buffered_file_t *bf) {
    // Handle the case where the last operation was a write
    if (bf->last_op == 1 && bf->write_buffer_pos > 0) {
        if (bf->preappend) {
            // Handle O_PREAPPEND logic
            // Save the current file offset
            off_t current_offset = lseek(bf->fd, 0, SEEK_CUR);
            if (current_offset == -1) {
                return -1;
            }

            // Move to the end to determine file size
            off_t original_offset = lseek(bf->fd, 0, SEEK_END);
            if (original_offset == -1) {
                return -1;
            }

            // Allocate a buffer for existing content
            char *temp_buffer = (char *)malloc(original_offset);
            if (!temp_buffer) {
                return -1;
            }

            // Read the existing content
            if (lseek(bf->fd, 0, SEEK_SET) == -1) {
                free(temp_buffer);
                return -1;
            }
            ssize_t existing_size = read(bf->fd, temp_buffer, original_offset);
            if (existing_size == -1) {
                free(temp_buffer);
                return -1;
            }

            // Write the new data to the beginning
            if (lseek(bf->fd, 0, SEEK_SET) == -1) {
                free(temp_buffer);
                return -1;
            }
            ssize_t written = write(bf->fd, bf->write_buffer, bf->write_buffer_pos);
            if (written == -1 || (size_t)written != bf->write_buffer_pos) {
                free(temp_buffer);
                return -1;
            }

            // Append the existing content
            ssize_t appended = write(bf->fd, temp_buffer, existing_size);
            free(temp_buffer);
            if (appended == -1 || (size_t)appended != (size_t)existing_size) {
                return -1;
            }
            bf->write_buffer_pos = 0;

            // Restore the original file offset
            if (lseek(bf->fd, current_offset, SEEK_SET) == -1) {
                return -1;
            }
        } else {
            ssize_t written = write(bf->fd, bf->write_buffer, bf->write_buffer_pos);
            if (written == -1 || (size_t)written != bf->write_buffer_pos) {
                return -1;
            }
            bf->write_buffer_pos = 0;
        }
    } else if (bf->last_op == 2) {
        // Handle the case where the last operation was a read
        // Ensure any buffered data is read before flushing
        if (bf->preappend) {
            // Save the current file offset
            off_t current_offset = lseek(bf->fd, 0, SEEK_CUR);
            if (current_offset == -1) {
                return -1;
            }

            // Move to the end to determine file size
            off_t original_offset = lseek(bf->fd, 0, SEEK_END);
            if (original_offset == -1) {
                return -1;
            }

            // Allocate a buffer for existing content
            char *temp_buffer = (char *)malloc(original_offset);
            if (!temp_buffer) {
                return -1;
            }

            // Read the existing content
            if (lseek(bf->fd, 0, SEEK_SET) == -1) {
                free(temp_buffer);
                return -1;
            }
            ssize_t existing_size = read(bf->fd, temp_buffer, original_offset);
            if (existing_size == -1) {
                free(temp_buffer);
                return -1;
            }

            // Restore the original file offset
            if (lseek(bf->fd, current_offset, SEEK_SET) == -1) {
                free(temp_buffer);
                return -1;
            }
            free(temp_buffer);
        }

        // Ensure the write buffer is flushed after reading
        if (bf->write_buffer_pos > 0) {
            ssize_t written = write(bf->fd, bf->write_buffer, bf->write_buffer_pos);
            if (written == -1 || (size_t)written != bf->write_buffer_pos) {
                return -1;
            }
            bf->write_buffer_pos = 0;
        }
    }

    return 0;
}

// Function to close the buffered file
int buffered_close(buffered_file_t *bf) {
    if (buffered_flush(bf) == -1) {
        return -1;
    }
    int ret = close(bf->fd);
    free(bf->read_buffer);
    free(bf->write_buffer);
    free(bf);
    return ret;
}