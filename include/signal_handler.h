/*************************************************************************
    > File Name: signal_handler.h
    > Author: racle
    > Mail: racleray@qq.com
    > Created Time:
 ************************************************************************/

#ifndef _SIGNAL_HANDLER_H
#define _SIGNAL_HANDLER_H


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#include "parse_metafile.h"
#include "bitfield.h"
#include "peer.h"
#include "tracker.h"
#include "torrent.h"
#include "data.h"


// 释放动态分配的内存等
void do_clear_work();

// 处理一些信号
void terminate_signal_handler(int signo);

// 设置信号处理函数
int set_signal_hander();

#endif
