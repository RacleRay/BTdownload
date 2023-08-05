/*************************************************************************
    > File Name: bterror.c
    > Author: racle
    > Mail: racleray@qq.com
    > Created Time:
 ************************************************************************/

#include "../include/bterror.h"

void btexit(int errno, char *file, int line) {
    printf(">>> Exit at %s : %d with error number : %d\n", file, line, errno);
    exit(errno);
}