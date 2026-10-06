#include <stdio.h>
#include <string.h>

static int read_filename(const char *prompt, char *filename, size_t size)
{
    size_t length;
    int character;

    fputs(prompt, stdout);
    if (fgets(filename, (int)size, stdin) == NULL) {
        fputs("Could not read a filename.\n", stderr);
        return 0;
    }

    length = strlen(filename);
    if (length > 0 && filename[length - 1] == '\n') {
        filename[length - 1] = '\0';
    } else if (!feof(stdin)) {
        while ((character = getchar()) != '\n' && character != EOF) {
        }
        fputs("Filename is too long.\n", stderr);
        return 0;
    }

    if (filename[0] == '\0') {
        fputs("Filename cannot be empty.\n", stderr);
        return 0;
    }
    return 1;
}

int main(void)
{
    char source_name[512];
    char destination_name[512];
    FILE *source;
    FILE *destination;
    int character;
    int status = 0;

    if (!read_filename("Source file: ", source_name, sizeof source_name) ||
        !read_filename("Destination file: ", destination_name,
                       sizeof destination_name)) {
        return 1;
    }

    if (strcmp(source_name, destination_name) == 0) {
        fputs("Source and destination must be different files.\n", stderr);
        return 1;
    }

    source = fopen(source_name, "rb");
    if (source == NULL) {
        perror("Could not open source file");
        return 1;
    }

    destination = fopen(destination_name, "wb");
    if (destination == NULL) {
        perror("Could not open destination file");
        if (fclose(source) == EOF) {
            perror("Could not close source file");
        }
        return 1;
    }

    while ((character = fgetc(source)) != EOF) {
        if (fputc(character, destination) == EOF) {
            perror("Could not write destination file");
            status = 1;
            break;
        }
    }
    if (ferror(source)) {
        perror("Could not read source file");
        status = 1;
    }
    if (fclose(source) == EOF) {
        perror("Could not close source file");
        status = 1;
    }
    if (fclose(destination) == EOF) {
        perror("Could not close destination file");
        status = 1;
    }

    if (status == 0) {
        puts("File copied successfully.");
    }
    return status;
}