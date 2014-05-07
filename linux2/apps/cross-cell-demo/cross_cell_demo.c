#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
    int count;
    int dev_fd;
    char *buf = argv[1];

    if (buf == NULL) {
        printf("Usage: cross-cell-demo [encrypted string]\n");
        return 0;
    }

    printf("buf: %s\n", buf);
    char *buf2 = malloc(strlen(buf) + 1);

    dev_fd = open("/dev/cell", O_RDWR);
    if (dev_fd == -1) {
        printf("the device could not be opened\n");
        exit(-1);
    }

    count = write(dev_fd, buf, strlen(buf) + 1);
    if (count < sizeof(buf)) {
        printf("Some characters weren't written\n");
        close(dev_fd);
        exit(-1);
    }

    count = read(dev_fd, buf2, strlen(buf) + 1);
    printf("Decrypted text: %s\n", buf2);

    close(dev_fd);
    return 0;
}
