/*************************************************************************
    > File Name: bitfield.h
    > Author: racle
    > Mail: racleray@qq.com
    > Created Time:
 ************************************************************************/

#ifndef _BITFIELD_H
#define _BITFIELD_H

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef struct _Bitmap {
    unsigned char *bitfield; // 保存位图
    int bitfield_length;     // 位图所占的总字节数
    int valid_length;        // 位图有效的总位数,每一位代表一个piece
} Bitmap;

// 创建位图,分配内存并进行初始化
int create_bitfield();
// 获取某一位的值
int get_bit_value(Bitmap *bitmap, int index);
// 设置某一位的值
int set_bit_value(Bitmap *bitmap, int index, unsigned char value);
int all_zero(Bitmap *bitmap);       // 全部清零
int all_set(Bitmap *bitmap);        // 全部设置为1
void release_memory_in_bitfield();  // 释放bitfield.c中动态分配的内存
int print_bitfield(Bitmap *bitmap); // 打印位图值,用于调试

int restore_bitmap(); // 将位图存储到文件中
                      // 在下次下载时,先读取该文件获取已经下载的进度
int is_interested(Bitmap *dst, Bitmap *src); // 拥有位图src的peer是否对拥有
                                             // dst位图的peer感兴趣
int get_download_piece_num(); // 获取当前已下载到的总piece数

#endif
