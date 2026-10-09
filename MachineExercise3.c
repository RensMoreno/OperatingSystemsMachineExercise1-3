#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUF_SIZE 256

static int write_all(int fd, const char *buffer, size_t length)
{
    size_t written = 0;

    while (written < length) {
        ssize_t n = write(fd, buffer + written, length - written);
        if (n > 0) {
            written += (size_t)n;
        } else if (n < 0 && errno == EINTR) {
            continue;
        } else {
            if (n == 0)
                errno = EIO;
            return -1;
        }
    }

    return 0;
}

static ssize_t read_message(int fd, char *buffer, size_t capacity)
{
    size_t received = 0;

    while (received < capacity) {
        ssize_t n = read(fd, buffer + received, capacity - received);
        if (n > 0) {
            received += (size_t)n;
        } else if (n == 0) {
            return (ssize_t)received;
        } else if (errno != EINTR) {
            return -1;
        }
    }

    char extra;
    ssize_t n;
    do {
        n = read(fd, &extra, 1);
    } while (n < 0 && errno == EINTR);

    if (n < 0)
        return -1;
    if (n > 0) {
        errno = EMSGSIZE;
        return -1;
    }

    return (ssize_t)received;
}

int main(void)
{
    int p2c[2];   /* parent -> child */
    int c2p[2];   /* child  -> parent */
    char msg[BUF_SIZE];

    printf("Input string message: ");
    if (fgets(msg, sizeof msg, stdin) == NULL) {
        fprintf(stderr, "No input.\n");
        return EXIT_FAILURE;
    }
    msg[strcspn(msg, "\n")] = '\0'; /* remove newline */

    if (pipe(p2c) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }
    if (pipe(c2p) == -1) {
        perror("pipe");
        close(p2c[0]);
        close(p2c[1]);
        return EXIT_FAILURE;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        close(p2c[0]);
        close(p2c[1]);
        close(c2p[0]);
        close(c2p[1]);
        return EXIT_FAILURE;
    }

    if (pid == 0) { /* ---- CHILD ---- */
        char buf[BUF_SIZE];
        close(p2c[1]); /* child only reads p2c */
        close(c2p[0]); /* child only writes c2p */

        ssize_t n = read_message(p2c[0], buf, sizeof buf - 1);
        if (n < 0) {
            perror("read message");
            close(p2c[0]);
            close(c2p[1]);
            _exit(EXIT_FAILURE);
        }
        buf[n] = '\0';
        printf("CHILD(%d): Received message\n", getpid());

        for (ssize_t i = 0; i < n; i++) {/* flip the case */
            if (isupper((unsigned char)buf[i]))
                buf[i] = tolower((unsigned char)buf[i]);
            else if (islower((unsigned char)buf[i]))
                buf[i] = toupper((unsigned char)buf[i]);
        }
        printf("CHILD(%d): Reversing the case of the string and sending to Parent\n",
               getpid());
        fflush(stdout);

        if (write_all(c2p[1], buf, (size_t)n) == -1) {
            perror("write response");
            close(p2c[0]);
            close(c2p[1]);
            _exit(EXIT_FAILURE);
        }
        close(p2c[0]);
        close(c2p[1]);
        _exit(EXIT_SUCCESS);
    }

    /* ---- PARENT ---- */
    char reply[BUF_SIZE];
    close(p2c[0]);/* parent only writes p2c */
    close(c2p[1]);/* parent only reads c2p */

    printf("PARENT(%d): Sending [%s] to Child\n", getpid(), msg);
    fflush(stdout);
    int write_failed = write_all(p2c[1], msg, strlen(msg)) == -1;
    if (write_failed)
        perror("write message");
    close(p2c[1]); /* done writing */

    ssize_t n = read_message(c2p[0], reply, sizeof reply - 1);
    if (n < 0)
        perror("read response");
    else
        reply[n] = '\0';
    close(c2p[0]);

    int status;
    while (waitpid(pid, &status, 0) == -1) {
        if (errno != EINTR) {
            perror("waitpid");
            return EXIT_FAILURE;
        }
    }

    if (write_failed || n < 0 || !WIFEXITED(status) ||
        WEXITSTATUS(status) != EXIT_SUCCESS)
        return EXIT_FAILURE;

    printf("PARENT(%d): Received [%s] from Child\n", getpid(), reply);
    return 0;
}