#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include "../include/bitfield.h"
#include "../include/parse_metafile.h"
#include "../include/log.h"
#include "../include/signal_handler.h"


void test_bitmap() {
    Bitmap map1, map2;
    unsigned char bf1[2] = {0xa0, 0xa0};
    unsigned char bf2[2] = {0xc0, 0xc0};

    map1.bitfield = bf1;
    map1.bitfield_length = 2;
    map1.valid_length = 11;
    map2.bitfield = bf2;
    map2.bitfield_length = 2;
    map2.valid_length = 11;

    int ret = is_interested(&map1, &map2);
    printf("%d\n", ret);
}


int main(int argc, char *argv[]) {
    int ret;

    if (argc != 2) {
        printf("usage:%s metafile\n", argv[0]);
        exit(-1);
    }

    // 设置信号处理函数
    ret = set_signal_hander();
    if (ret != 0) {
        printf("%s:%d error\n", __FILE__, __LINE__);
        return -1;
    }

    // 解析种子文件
    ret = parse_metafile(argv[1]);
    if (ret != 0) {
        printf("%s:%d error\n", __FILE__, __LINE__);
        return -1;
    }

    // 初始化非阻塞peer
    init_unchoke_peers();

    // 创建用于保存下载数据的文件
    ret = create_files();
    if (ret != 0) {
        printf("%s:%d error\n", __FILE__, __LINE__);
        return -1;
    }

    // 创建位图
    ret = create_bitfield();
    if (ret != 0) {
        printf("%s:%d error\n", __FILE__, __LINE__);
        return -1;
    }

    // 创建缓冲区
    ret = create_btcache();
    if (ret != 0) {
        printf("%s:%d error\n", __FILE__, __LINE__);
        return -1;
    }

    // 与所有Peer收发数据、交换消息
    download_upload_with_peers();

    // 释放动态分配的内存
    do_clear_work();

    return 0;
}