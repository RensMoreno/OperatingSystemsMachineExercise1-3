#include <stdio.h>
#include <unistd.h>

int main(void)
{
    printf("[CHILD]: PID %ld, starts counting:\n", (long)getpid());
    for (int i = 1; i <= 10; i++) {
        printf("[CHILD]: i = %d\n", i);
        fflush(stdout);
        if (i < 10) {
            sleep(1);
        }
    }
    return 0;
}