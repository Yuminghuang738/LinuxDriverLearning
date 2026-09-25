#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define BUFF_SIZE 128

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Error usage!");
        return -1;
    }

    printf("--------------writing data--------------\n");
    
    char *file_path = argv[1];;
    int fd = open(file_path, O_RDWR);
    if (fd < 0)
    {
        printf("can't open file: %s\n", file_path);
        return -1;
    }

    char wbuf[BUFF_SIZE];
    char user_data[] = "Hello World!\n";
    memcpy(wbuf, user_data, sizeof(user_data));

    int ret = write(fd, wbuf, strlen(wbuf));
    if (ret < 0)
    {
        printf("write file %s failed!\n", file_path);
        return -1;
    }

    close(fd);

    sleep(1);

    printf("--------------reading data--------------\n");

    fd = open(file_path, O_RDWR);
    if (fd < 0)
    {
        printf("can't open file: %s\n", file_path);
        return -1;
    }

    char rbuf[BUFF_SIZE];
    ret = read(fd, rbuf, BUFF_SIZE);
    if (ret < 0)
    {
        printf("read file %s failed!\n", file_path);
        return -1;
    }
    else 
    {
        printf("read data: %s\n", rbuf);
    }

    close(fd);

    return 0;
}