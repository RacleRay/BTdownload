/*************************************************************************
    > File Name: policy.h
    > Author: racle
    > Mail: racleray@qq.com
    > Created Time:
 ************************************************************************/

#ifndef _POLICY_H
#define _POLICY_H

#include "peer.h"

// bittorrent协议相关算法：
// 1. 流水作业：一次生成对某个peer的多个slice请求，一般为5个
// 2. 片断选择算法：针对不同的下载阶段，有不同的选择策略
// 3. 阻塞算法：根据速度选择非阻塞peer，以及选择优化非阻塞peer
// 4. 判断是否下载完毕：根据位图作出判断，下载完毕即终止程序

#define COMPUTE_RATE_TIME 10 // 每隔10秒计算一次各个peer的上传下载速度
#define UNCHOKE_COUNT     4 // 存储下载速度最快的4个peer的指针
#define REQ_SLICE_NUM     5 // 每次请求的slice数

typedef struct _Unchoke_peers {
    Peer *unchkpeer[UNCHOKE_COUNT];
    int count;
    Peer *optunchkpeer;
} Unchoke_peers;

void init_unchoke_peers();    // 初始化全局变量unchoke_peers

int select_unchoke_peer();    // 选择unchoke peer
int select_optunchoke_peer(); // 从peer队列中选择一个优化非阻塞peer

int compute_rate(); // 计算最近一段时间(10秒)每个peer的上传下载速度
int compute_total_rate(); // 计算总的上传下载速度

int is_seed(Peer *node); // 判断某个peer是否为种子

// 构造数据请求,为了提高效率一次请求5个slice
int create_req_slice_msg(Peer *node);

#endif
