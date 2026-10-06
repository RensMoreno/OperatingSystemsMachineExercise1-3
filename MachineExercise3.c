#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUF_SIZE 256

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

    if (pipe(p2c) == -1 || pipe(c2p) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) { /* ---- CHILD ---- */
        char buf[BUF_SIZE];
        close(p2c[1]); /* child only reads p2c */
        close(c2p[0]); /* child only writes c2p */

        ssize_t n = read(p2c[0], buf, sizeof buf - 1);
        if (n < 0) { perror("read"); _exit(EXIT_FAILURE); }
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

        write(c2p[1], buf, n);
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
    write(p2c[1], msg, strlen(msg));
    close(p2c[1]); /* done writing */

    ssize_t n = read(c2p[0], reply, sizeof reply - 1);
    if (n < 0) { perror("read"); return EXIT_FAILURE; }
    reply[n] = '\0';
    printf("PARENT(%d): Received [%s] from Child\n", getpid(), reply);
    close(c2p[0]);

    wait(NULL); /* clean up the child */
    return 0;
}