/*************************************************************************
    > File Name: signal_handler.c
    > Author: racle
    > Mail: racleray@qq.com
    > Created Time:
 ************************************************************************/

#include "../include/signal_handler.h"

extern int download_piece_num;
extern int  *fds;     // 开启的文件描述符数组
extern int  fds_len;  // 开启的文件描述符数量
extern Peer *peer_head;

/**
 * @brief 关闭socket，保存位图，关闭文件描述符，释放动态内存
 * 
 */
void do_clear_work() {
    Peer *p = peer_head;
	while(p != NULL) {
		if(p->state != CLOSING)  close(p->socket);
		p = p->next;
	}

    if(download_piece_num > 0) {
		restore_bitmap();
	}

    int i;
	for(i = 0; i < fds_len; i++) {
		close(fds[i]);
	}

    release_memory_in_parse_metafile();
	release_memory_in_bitfield();
	release_memory_in_btcache();
	release_memory_in_peer();
	release_memory_in_torrent();

    exit(0);
}


/**
 * @brief SIGINT 与 SIGTERM 的处理函数
 * 
 * @param signo 
 */
void terminate_signal_handler(int signo) {
    printf("Please wait for clear operations...\n");
    do_clear_work();
}


/**
 * @brief 设置信号处理函数
 * 
 * @return int 
 */
int set_signal_hander() {
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        perror("can not catch signal:sigpipe\n");
        return -1;
    }

    if (signal(SIGINT, terminate_signal_handler) == SIG_ERR) {
        perror("can not catch signal:sigint\n");
        return -1;
    }

    if (signal(SIGTERM, terminate_signal_handler) == SIG_ERR) {
        perror("can not catch signal:sigterm\n");
        return -1;
    }

    return 0;
}
