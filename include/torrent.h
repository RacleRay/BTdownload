/*************************************************************************
    > File Name: torrent.h
    > Author: racle
    > Mail: racleray@qq.com
    > Created Time: 
 ************************************************************************/

#ifndef _TORRENT_H
#define _TORRENT_H

#include "tracker.h"

// 与所有peer收发数据、交换消息
int download_upload_with_peers();

// 打印peer链表中各个peer的IP和端口号
int  print_peer_list();
// 打印下载进度消息
void print_process_info();

// 释放与连接Tracker有关的一些动态存储空间
void clear_connect_tracker();
// 释放与连接peer有关的一些动态存储空间
void clear_connect_peer();
// 释放与解析Tracker回应有关的一些动态存储空间
void clear_tracker_response();
// 释放本模块中动态申请的存储空间
void release_memory_in_torrent();

#endif
