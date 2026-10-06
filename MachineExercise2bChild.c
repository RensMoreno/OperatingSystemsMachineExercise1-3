/* ME2-B (child program): the "other source" that the child will load.
 * Compile: gcc me2b_child.c -o counter
 */
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    printf("[CHILD]: PID %d, starts counting:\n", getpid());
    for (int i = 1; i <= 10; i++) {
        printf("[CHILD]: i = %d\n", i);
        fflush(stdout);
        sleep(1);
    }
    return 0;
}