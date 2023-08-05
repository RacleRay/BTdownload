/*************************************************************************
    > File Name: torrent.h
    > Author: racle
    > Mail: racleray@qq.com
    > Created Time: 
 ************************************************************************/

#ifndef _TORRENT_H
#define _TORRENT_H

#include "tracker.h"

// 与所有 Peer 通信
int download_upload_with_peers();

int  print_peer_list();
void print_process_info();

void clear_connect_tracker();
void clear_connect_peer();
void clear_tracker_response();
void release_memory_in_torrent();

#endif
