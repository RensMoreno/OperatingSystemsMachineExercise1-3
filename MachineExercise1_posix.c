#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define FILENAME_SIZE 512
#define BUFFER_SIZE 4096

static int write_all(int fd, const char *buffer, size_t length)
{
    size_t written = 0;

    while (written < length) {
        ssize_t result = write(fd, buffer + written, length - written);
        if (result < 0) {
            if (errno == EINTR) {
                continue;
            }
            return 0;
        }
        written += (size_t)result;
    }
    return 1;
}

static int read_filename(const char *prompt, char *filename, size_t size)
{
    size_t length = 0;
    char character;

    if (!write_all(STDOUT_FILENO, prompt, strlen(prompt))) {
        perror("Could not display prompt");
        return 0;
    }

    for (;;) {
        ssize_t result = read(STDIN_FILENO, &character, 1);
        if (result < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("Could not read filename");
            return 0;
        }
        if (result == 0 || character == '\n') {
            break;
        }
        if (length + 1 >= size) {
            while (result > 0 && character != '\n') {
                result = read(STDIN_FILENO, &character, 1);
                if (result < 0 && errno == EINTR) {
                    continue;
                }
            }
            fprintf(stderr, "Filename is too long.\n");
            return 0;
        }
        filename[length++] = character;
    }

    filename[length] = '\0';
    if (length == 0) {
        fprintf(stderr, "Filename cannot be empty.\n");
        return 0;
    }
    return 1;
}

int main(void)
{
    char source_name[FILENAME_SIZE];
    char destination_name[FILENAME_SIZE];
    char buffer[BUFFER_SIZE];
    struct stat source_info;
    struct stat destination_info;
    int source_fd;
    int destination_fd;
    int status = 0;

    if (!read_filename("Source file: ", source_name, sizeof source_name) ||
        !read_filename("Destination file: ", destination_name,
                       sizeof destination_name)) {
        return 1;
    }

    if (strcmp(source_name, destination_name) == 0) {
        fprintf(stderr, "Source and destination must be different files.\n");
        return 1;
    }

    source_fd = open(source_name, O_RDONLY);
    if (source_fd < 0) {
        perror("Could not open source file");
        return 1;
    }
    if (fstat(source_fd, &source_info) < 0) {
        perror("Could not inspect source file");
        close(source_fd);
        return 1;
    }

    destination_fd = open(destination_name, O_WRONLY | O_CREAT, 0666);
    if (destination_fd < 0) {
        perror("Could not open destination file");
        close(source_fd);
        return 1;
    }
    if (fstat(destination_fd, &destination_info) < 0) {
        perror("Could not inspect destination file");
        close(destination_fd);
        close(source_fd);
        return 1;
    }
        if (source_info.st_ino != 0 && destination_info.st_ino != 0 &&
            source_info.st_dev == destination_info.st_dev &&
            source_info.st_ino == destination_info.st_ino) {
        fprintf(stderr, "Source and destination refer to the same file.\n");
        close(destination_fd);
        close(source_fd);
        return 1;
    }
    if (ftruncate(destination_fd, 0) < 0) {
        perror("Could not truncate destination file");
        close(destination_fd);
        close(source_fd);
        return 1;
    }

    for (;;) {
        ssize_t bytes_read = read(source_fd, buffer, sizeof buffer);
        if (bytes_read < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("Could not read source file");
            status = 1;
            break;
        }
        if (bytes_read == 0) {
            break;
        }
        if (!write_all(destination_fd, buffer, (size_t)bytes_read)) {
            perror("Could not write destination file");
            status = 1;
            break;
        }
    }

    if (close(source_fd) < 0) {
        perror("Could not close source file");
        status = 1;
    }
    if (close(destination_fd) < 0) {
        perror("Could not close destination file");
        status = 1;
    }
    if (status == 0) {
        puts("File copied successfully.");
    }
    return status;
}
