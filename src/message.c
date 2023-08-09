/*************************************************************************
    > File Name: message.c
    > Author: racle
    > Mail: racleray@qq.com
    > Created Time:
 ************************************************************************/

#include "../include/message.h"


/**
 * @brief 根据当前的状态生成并发送消息、接收以及处理消息。
 * 
 */


#define HANDSHAKE    -2
#define KEEP_ALIVE   -1
#define CHOKE        0
#define UNCHOKE      1
#define INTERESTED   2
#define UNINTERESTED 3
#define HAVE         4
#define BITFIELD     5
#define REQUEST      6
#define PIECE        7
#define CANCEL       8
#define PORT         9

#define KEEP_ALIVE_TIME 45

extern Bitmap *bitmap;
extern char info_hash[20];
extern char peer_id[20];
extern int have_piece_index[64];
extern Peer *peer_head;

/**
 * @brief 获取整型变量i的各个字节，并保存到字符数组c中
 * 
 * @param i int
 * @param c 长度为4的数组，传参退化为指针
 * @return int 0
 */
int int_to_char(int i, unsigned char c[4]) {
    c[3] = i % 256;
    c[2] = (i - c[3]) / 256 % 256;
    c[1] = (i - c[3] - c[2] * 256) / 256 / 256 % 256;
    c[0] = (i - c[3] - c[2] * 256 - c[1] * 256 * 256) / 256 / 256 / 256 % 256;
    return 0;
}

/**
 * @brief 将字符数组中的字符转换为一个整型
 * 
 * @param c 长度为4的数组，传参退化为指针
 * @return int 重构出的int结果数字
 */
int char_to_int(unsigned char c[4]) {
    int i;

    i = c[0] * 256 * 256 * 256 + c[1] * 256 * 256 + c[2] * 256 + c[3];

    return i;
}

/**
 * @brief 创建 handshake 消息
 * 
 * @param info_hash info_hash 由 parse_metafile 中 get_info_hash 结合种子文件计算得到
 * @param peer_id parse_metafile中生成
 * @param peer 发送的目标 Peer节点指针
 * @return int 创建成功返回0，创建失败返回-1
 */
int create_handshake_msg(char *info_hash, char *peer_id, Peer *peer) {
    int i;
    unsigned char keyword[20] = "BitTorrent protocol", c = 0x00;
    unsigned char *buffer = (unsigned char *)peer->out_msg + peer->msg_len;
    int len = MSG_SIZE - peer->msg_len;

    if (len < 68) return -1; // 握手消息的固定长度 68

    buffer[0] = 19;
    for (i = 0; i < 19; i++) buffer[i + 1] = keyword[i];
    for (i = 0; i < 8; i++) buffer[i + 20] = c;
    for (i = 0; i < 20; i++) buffer[i + 28] = info_hash[i];
    for (i = 0; i < 20; i++) buffer[i + 48] = peer_id[i];

    peer->msg_len += 68;
    return 0;
}

/**
 * @brief 创建keep_alive消息
 * 
 * @param peer 发送的目标 Peer节点指针
 * @return int 创建成功返回0，创建失败返回-1
 */
int create_keep_alive_msg(Peer *peer) {
    unsigned char *buffer = (unsigned char *)peer->out_msg + peer->msg_len;
    int len = MSG_SIZE - peer->msg_len;

    if (len < 4) return -1; // 4为keep_alive消息的固定长度

    memset(buffer, 0, 4);
    peer->msg_len += 4;
    return 0;
}

/**
 * @brief 创建chock消息
 * 
 * @param type 可创建4种消息，即choke、unchoke、interested、uninterested消息。
               type值依次对应 0、1、2、3.
 * @param peer 发送的目标 Peer节点指针
 * @return int 创建成功返回0，创建失败返回-1
 */
int create_chock_interested_msg(int type, Peer *peer) {
    unsigned char *buffer = (unsigned char *)peer->out_msg + peer->msg_len;
    int len = MSG_SIZE - peer->msg_len;

    // 5为choke、unchoke、interested、uninterested消息的固定长度
    if (len < 5) return -1;

    memset(buffer, 0, 5);
    buffer[3] = 1;
    buffer[4] = type;

    peer->msg_len += 5;
    return 0;
}

/**
 * @brief 创建have消息
 * 
 * @param index index为piece的index
 * @param peer  发送的目标 Peer节点指针
 * @return int  创建成功返回0，创建失败返回-1
 */
int create_have_msg(int index, Peer *peer) {
    unsigned char *buffer = (unsigned char *)peer->out_msg + peer->msg_len;
    int len = MSG_SIZE - peer->msg_len;
    unsigned char c[4];

    if (len < 9) return -1; // 9为have消息的固定长度

    memset(buffer, 0, 9);
    buffer[3] = 5;
    buffer[4] = 4;

    int_to_char(index, c);
    buffer[5] = c[0];
    buffer[6] = c[1];
    buffer[7] = c[2];
    buffer[8] = c[3];

    peer->msg_len += 9;
    return 0;
}

/**
 * @brief 创建bitfield消息
 * 
 * @param bitfield bitfield指针
 * @param bitfield_len bitfield长度，字节数
 * @param peer 发送的目标 Peer节点指针
 * @return int 创建成功返回0，创建失败返回-1
 */
int create_bitfield_msg(unsigned char *bitfield, int bitfield_len, Peer *peer) {
    int i;
    unsigned char c[4];
    unsigned char *buffer = (unsigned char *)peer->out_msg + peer->msg_len;
    int len = MSG_SIZE - peer->msg_len;

    if (len < bitfield_len + 5) { // bitfield消息的长度为bitfield_len+5
        printf("%s:%d buffer too small\n", __FILE__, __LINE__);
        return -1;
    }

    int_to_char(bitfield_len + 1, c);
    for (i = 0; i < 4; i++) buffer[i] = c[i];
    buffer[4] = 5;
    for (i = 0; i < bitfield_len; i++) buffer[i + 5] = bitfield[i];

    peer->msg_len += bitfield_len + 5;
    return 0;
}

/**
 * @brief 创建数据请求消息
 * 
 * @param index index为请求的piece的index
 * @param begin begin为piece内的偏移量
 * @param length length为请求数据的长度
 * @param peer 发送的目标 Peer节点指针
 * @return int 创建成功返回0，创建失败返回-1
 */
int create_request_msg(int index, int begin, int length, Peer *peer) {
    int i;
    unsigned char c[4];
    unsigned char *buffer = (unsigned char *)peer->out_msg + peer->msg_len;
    int len = MSG_SIZE - peer->msg_len;

    if (len < 17) return -1; // 17为request消息的固定长度

    memset(buffer, 0, 17);
    buffer[3] = 13;
    buffer[4] = 6;
    int_to_char(index, c);
    for (i = 0; i < 4; i++) buffer[i + 5] = c[i];
    int_to_char(begin, c);
    for (i = 0; i < 4; i++) buffer[i + 9] = c[i];
    int_to_char(length, c);
    for (i = 0; i < 4; i++) buffer[i + 13] = c[i];

    peer->msg_len += 17;
    return 0;
}

/**
 * @brief 创建piece消息
 * 
 * @param index index为请求的piece的index
 * @param begin begin为piece内的偏移量
 * @param block block指向待发送的数据
 * @param b_len b_len为block所指向的数据的长度
 * @param peer  发送的目标 Peer节点指针
 * @return int 创建成功返回0，创建失败返回-1
 */
int create_piece_msg(int index, int begin, char *block, int b_len, Peer *peer) {
    int i;
    unsigned char c[4];
    unsigned char *buffer = (unsigned char *)peer->out_msg + peer->msg_len;
    int len = MSG_SIZE - peer->msg_len;

    if (len < b_len + 13) { // piece消息的长度为b_len+13
        printf("IP:%s len:%d\n", peer->ip, len);
        printf("%s:%d buffer too small\n", __FILE__, __LINE__);
        return -1;
    }

    int_to_char(b_len + 9, c);
    for (i = 0; i < 4; i++) buffer[i] = c[i];
    buffer[4] = 7;
    int_to_char(index, c);
    for (i = 0; i < 4; i++) buffer[i + 5] = c[i];
    int_to_char(begin, c);
    for (i = 0; i < 4; i++) buffer[i + 9] = c[i];
    for (i = 0; i < b_len; i++) buffer[i + 13] = block[i];

    peer->msg_len += b_len + 13;
    return 0;
}

/**
 * @brief 创建cancel消息
 * 
 * @param index index为请求的piece的index
 * @param begin begin为piece内的偏移量
 * @param length 从begin开始的piece内内容的长度
 * @param peer  发送的目标 Peer节点指针
 * @return int 创建成功返回0，创建失败返回-1
 */
int create_cancel_msg(int index, int begin, int length, Peer *peer) {
    int i;
    unsigned char c[4];
    unsigned char *buffer = (unsigned char *)peer->out_msg + peer->msg_len;
    int len = MSG_SIZE - peer->msg_len;

    if (len < 17) return -1; // 17为cancel消息的固定长度

    memset(buffer, 0, 17);
    buffer[3] = 13;
    buffer[4] = 8;
    int_to_char(index, c);
    for (i = 0; i < 4; i++) buffer[i + 5] = c[i];
    int_to_char(begin, c);
    for (i = 0; i < 4; i++) buffer[i + 9] = c[i];
    int_to_char(length, c);
    for (i = 0; i < 4; i++) buffer[i + 13] = c[i];

    peer->msg_len += 17;
    return 0;
}

/**
 * @brief 创建port消息。为以DHT的方式获取peer地址的应用程序所准备的。
 *        本程序中未使用。
 * 
 * @param port port号
 * @param peer 发送的目标 Peer节点指针
 * @return int 创建成功返回0，创建失败返回-1
 */
int create_port_msg(int port, Peer *peer) {
    unsigned char c[4];
    unsigned char *buffer = (unsigned char *)peer->out_msg + peer->msg_len;
    int len = MSG_SIZE - peer->msg_len;

    if (len < 7) return -1; // 7为port消息的固定长度

    memset(buffer, 0, 7);
    buffer[3] = 3;
    buffer[4] = 9;
    int_to_char(port, c);
    buffer[5] = c[2];
    buffer[6] = c[3];

    peer->msg_len += 7;
    return 0;
}

/**
 * @brief 判断缓冲区中是否存放了一条完整的消息
 * 
 * @param buff buff指向存放消息的缓冲区
 * @param len  缓冲区buff的长度
 * @param ok_len 用于返回缓冲区中完整消息的长度，即 buff[0]～buff[ok_len-1] 存放着一条或多条完整的消息
 * @return int 创建成功返回0，创建失败返回-1
 */
int is_complete_message(unsigned char *buff, unsigned int len, int *ok_len) {
    unsigned int i;
    char btkeyword[20];

    unsigned char keep_alive[4] = {0x0, 0x0, 0x0, 0x0};
    unsigned char chocke[5] = {0x0, 0x0, 0x0, 0x1, 0x0};
    unsigned char unchocke[5] = {0x0, 0x0, 0x0, 0x1, 0x1};
    unsigned char interested[5] = {0x0, 0x0, 0x0, 0x1, 0x2};
    unsigned char uninterested[5] = {0x0, 0x0, 0x0, 0x1, 0x3};
    unsigned char have[5] = {0x0, 0x0, 0x0, 0x5, 0x4};
    unsigned char request[5] = {0x0, 0x0, 0x0, 0xd, 0x6};
    unsigned char cancel[5] = {0x0, 0x0, 0x0, 0xd, 0x8};
    unsigned char port[5] = {0x0, 0x0, 0x0, 0x3, 0x9};

    if (buff == NULL || len <= 0 || ok_len == NULL) return -1;
    *ok_len = 0;

    btkeyword[0] = 19;
    memcpy(&btkeyword[1], "BitTorrent protocol", 19); // BitTorrent协议关键字

    unsigned char c[4];
    unsigned int length;

    for (i = 0; i < len;) {
        // 握手、chocke、have等消息的长度是固定的
        if (i + 68 <= len && memcmp(&buff[i], btkeyword, 20) == 0) i += 68;
        else if (i + 4 <= len && memcmp(&buff[i], keep_alive, 4) == 0)
            i += 4;
        else if (i + 5 <= len && memcmp(&buff[i], chocke, 5) == 0)
            i += 5;
        else if (i + 5 <= len && memcmp(&buff[i], unchocke, 5) == 0)
            i += 5;
        else if (i + 5 <= len && memcmp(&buff[i], interested, 5) == 0)
            i += 5;
        else if (i + 5 <= len && memcmp(&buff[i], uninterested, 5) == 0)
            i += 5;
        else if (i + 9 <= len && memcmp(&buff[i], have, 5) == 0)
            i += 9;
        else if (i + 17 <= len && memcmp(&buff[i], request, 5) == 0)
            i += 17;
        else if (i + 17 <= len && memcmp(&buff[i], cancel, 5) == 0)
            i += 17;
        else if (i + 7 <= len && memcmp(&buff[i], port, 5) == 0)
            i += 7;
        // bitfield消息的长度是变化的
        else if (i + 5 <= len && buff[i + 4] == 5) {
            c[0] = buff[i];
            c[1] = buff[i + 1];
            c[2] = buff[i + 2];
            c[3] = buff[i + 3];
            length = char_to_int(c);
            // 消息长度占4字节,消息本身占length个字节
            if (i + 4 + length <= len) i += 4 + length;
            else {
                *ok_len = i;
                return -1;
            }
        }
        // piece消息的长度也是变化的
        else if (i + 5 <= len && buff[i + 4] == 7) {
            c[0] = buff[i];
            c[1] = buff[i + 1];
            c[2] = buff[i + 2];
            c[3] = buff[i + 3];
            length = char_to_int(c);
            // 消息长度占4字节,消息本身占length个字节
            if (i + 4 + length <= len) i += 4 + length;
            else {
                *ok_len = i;
                return -1;
            }
        } else {
            // 处理未知类型的消息
            if (i + 4 <= len) {
                c[0] = buff[i];
                c[1] = buff[i + 1];
                c[2] = buff[i + 2];
                c[3] = buff[i + 3];
                length = char_to_int(c);
                // 消息长度占4字节,消息本身占length个字节
                if (i + 4 + length <= len) {
                    i += 4 + length;
                    continue;
                } else {
                    *ok_len = i;
                    return -1;
                }
            }
            // 如果也不是未知消息类型,则认为目前接收的数据还不是一个完整的消息
            *ok_len = i;
            return -1;
        }
    }

    *ok_len = i;
    return 1;
}

/**
 * @brief 处理接收到的来自peer的握手消息
 * 
 * @param peer 发送该条消息的 Peer节点指针
 * @param buff handshake消息
 * @param len  handshake消息长度
 * @return int 处理成功返回0，处理失败返回-1
 */
int process_handshake_msg(Peer *peer, unsigned char *buff, int len) {
    if (peer == NULL || buff == NULL) return -1;

    if (memcmp(info_hash, buff + 28, 20) != 0) {
        peer->state = CLOSING;
        // 丢弃发送缓冲区中的数据
        discard_send_buffer(peer);
        clear_btcache_before_peer_close(peer);
        close(peer->socket);
        return -1;
    }

    memcpy(peer->id, buff + 48, 20);
    (peer->id)[20] = '\0';

    if (peer->state == INITIAL) {
        peer->state = HANDSHAKED;
        create_handshake_msg(info_hash, peer_id, peer);
    }
    if (peer->state == HALFSHAKED) peer->state = HANDSHAKED;

    peer->start_timestamp = time(NULL);
    return 0;
}

/**
 * @brief 处理接收到的来自peer的keepv_alive消息
 * 
 * @param peer 发送该条消息的 Peer节点指针
 * @param buff keepv_alive消息
 * @param len  keepv_alive消息长度
 * @return int 处理成功返回0，处理失败返回-1
 */
int process_keep_alive_msg(Peer *peer, unsigned char *buff, int len) {
    if (peer == NULL || buff == NULL) return -1;

    peer->start_timestamp = time(NULL);
    return 0;
}

/**
 * @brief 处理接收到的来自peer的choke消息
 * 
 * @param peer 发送该条消息的 Peer节点指针
 * @param buff choke消息
 * @param len  choke消息长度
 * @return int 处理成功返回0，处理失败返回-1
 */
int process_choke_msg(Peer *peer, unsigned char *buff, int len) {
    if (peer == NULL || buff == NULL) return -1;

    if (peer->state != CLOSING && peer->peer_choking == 0) {
        peer->peer_choking = 1;
        peer->last_down_timestamp = 0;
        peer->down_count = 0;
        peer->down_rate = 0;
    }

    peer->start_timestamp = time(NULL);
    return 0;
}

/**
 * @brief 处理接收到的来自peer的unchoke消息
 * 
 * @param peer 发送该条消息的 Peer节点指针
 * @param buff unchoke消息
 * @param len  unchoke消息长度
 * @return int 处理成功返回0，处理失败返回-1
 */
int process_unchoke_msg(Peer *peer, unsigned char *buff, int len) {
    if (peer == NULL || buff == NULL) return -1;

    if (peer->state != CLOSING && peer->peer_choking == 1) {
        peer->peer_choking = 0;
        if (peer->am_interested == 1) create_req_slice_msg(peer);
        else {
            peer->am_interested = is_interested(&(peer->bitmap), bitmap);
            if (peer->am_interested == 1) create_req_slice_msg(peer);
            else
                printf(
                    "Received unchoke but Not interested to IP:%s \n",
                    peer->ip);
        }

        peer->last_down_timestamp = 0;
        peer->down_count = 0;
        peer->down_rate = 0;
    }

    peer->start_timestamp = time(NULL);
    return 0;
}

/**
 * @brief 处理接收到的来自peer的interested消息
 * 
 * @param peer 发送该条消息的 Peer节点指针
 * @param buff interested消息
 * @param len  interested消息长度
 * @return int 处理成功返回0，处理失败返回-1
 */
int process_interested_msg(Peer *peer, unsigned char *buff, int len) {
    if (peer == NULL || buff == NULL) return -1;

    if (peer->state != CLOSING && peer->state == DATA) {
        peer->peer_interested = is_interested(bitmap, &(peer->bitmap));
        if (peer->peer_interested == 0) return -1;
        if (peer->am_choking == 0) create_chock_interested_msg(1, peer);
    }

    peer->start_timestamp = time(NULL);
    return 0;
}

/**
 * @brief 处理接收到的来自peer的uninterested消息
 * 
 * @param peer 发送该条消息的 Peer节点指针
 * @param buff uninterested消息
 * @param len  uninterested消息长度
 * @return int 处理成功返回0，处理失败返回-1
 */
int process_uninterested_msg(Peer *peer, unsigned char *buff, int len) {
    if (peer == NULL || buff == NULL) return -1;

    if (peer->state != CLOSING && peer->state == DATA) {
        peer->peer_interested = 0;
        cancel_requested_list(peer);
    }

    peer->start_timestamp = time(NULL);
    return 0;
}

/**
 * @brief 处理接收到的来自peer的have消息
 * 
 * @param peer 发送该条消息的 Peer节点指针
 * @param buff have消息
 * @param len  have消息长度
 * @return int 处理成功返回0，处理失败返回-1
 */
int process_have_msg(Peer *peer, unsigned char *buff, int len) {
    int rand_num;
    unsigned char c[4];

    if (peer == NULL || buff == NULL) return -1;

    srand(time(NULL));
    rand_num = rand() % 3;

    if (peer->state != CLOSING && peer->state == DATA) {
        c[0] = buff[5];
        c[1] = buff[6];
        c[2] = buff[7];
        c[3] = buff[8];
        if (peer->bitmap.bitfield != NULL)
            set_bit_value(&(peer->bitmap), char_to_int(c), 1);

        if (peer->am_interested == 0) {
            peer->am_interested = is_interested(&(peer->bitmap), bitmap);
            // 由原来的对peer不感兴趣变为感兴趣时,发interested消息
            if (peer->am_interested == 1) create_chock_interested_msg(2, peer);
        } else { // 收到三个have则发一个interested消息
            if (rand_num == 0) create_chock_interested_msg(2, peer);
        }
    }

    peer->start_timestamp = time(NULL);
    return 0;
}

/**
 * @brief 处理接收到的来自peer的cancel消息
 * 
 * @param peer 发送该条消息的 Peer节点指针
 * @param buff cancel消息
 * @param len  cancel消息长度
 * @return int 处理成功返回0，处理失败返回-1
 */
int process_cancel_msg(Peer *peer, unsigned char *buff, int len) {
    unsigned char c[4];
    int index, begin, length;

    if (peer == NULL || buff == NULL) return -1;

    c[0] = buff[5];
    c[1] = buff[6];
    c[2] = buff[7];
    c[3] = buff[8];
    index = char_to_int(c);
    c[0] = buff[9];
    c[1] = buff[10];
    c[2] = buff[11];
    c[3] = buff[12];
    begin = char_to_int(c);
    c[0] = buff[13];
    c[1] = buff[14];
    c[2] = buff[15];
    c[3] = buff[16];
    length = char_to_int(c);

    Request_piece *p, *q;
    p = q = peer->Requested_piece_head;
    while (p != NULL) {
        if (p->index == index && p->begin == begin && p->length == length) {
            if (p == peer->Requested_piece_head)
                peer->Requested_piece_head = p->next;
            else
                q->next = p->next;
            free(p);
            break;
        }
        q = p;
        p = p->next;
    }

    peer->start_timestamp = time(NULL);
    return 0;
}

/**
 * @brief 处理接收到的来自peer的bitfield消息
 * 
 * @param peer 发送该条消息的 Peer节点指针
 * @param buff bitfield消息
 * @param len  bitfield消息长度
 * @return int 处理成功返回0，处理失败返回-1
 */
int process_bitfield_msg(Peer *peer, unsigned char *buff, int len) {
    unsigned char c[4];

    if (peer == NULL || buff == NULL) return -1;
    if (peer->state == HANDSHAKED || peer->state == SENDBITFIELD) {
        c[0] = buff[0];
        c[1] = buff[1];
        c[2] = buff[2];
        c[3] = buff[3];

        if (peer->bitmap.bitfield != NULL) {
            free(peer->bitmap.bitfield);
            peer->bitmap.bitfield = NULL;
        }
        peer->bitmap.valid_length = bitmap->valid_length;
        if (bitmap->bitfield_length != char_to_int(c) - 1) {
            peer->state = CLOSING;
            // 丢弃发送缓冲区中的数据
            discard_send_buffer(peer);
            clear_btcache_before_peer_close(peer);
            close(peer->socket);
            return -1;
        }
        peer->bitmap.bitfield_length = char_to_int(c) - 1;
        peer->bitmap.bitfield =
            (unsigned char *)malloc(peer->bitmap.bitfield_length);
        memcpy(peer->bitmap.bitfield, &buff[5], peer->bitmap.bitfield_length);

        // 如果原状态为已握手,收到位图后应该向peer发位图
        if (peer->state == HANDSHAKED) {
            create_bitfield_msg(
                bitmap->bitfield, bitmap->bitfield_length, peer);
            peer->state = DATA;
        }
        // 如果原状态为已发送位图，收到位图后可以准备交换数据
        if (peer->state == SENDBITFIELD) { peer->state = DATA; }

        // 判断peer是否对我们感兴趣
        peer->peer_interested = is_interested(bitmap, &(peer->bitmap));
        // 判断对peer是否感兴趣,若是则发送interested消息
        peer->am_interested = is_interested(&(peer->bitmap), bitmap);
        if (peer->am_interested == 1) create_chock_interested_msg(2, peer);
    }

    peer->start_timestamp = time(NULL);
    return 0;
}

/**
 * @brief 处理接收到的来自peer的request消息
 * 
 * @param peer 发送该条消息的 Peer节点指针
 * @param buff request消息
 * @param len  request消息长度
 * @return int 处理成功返回0，处理失败返回-1
 */
int process_request_msg(Peer *peer, unsigned char *buff, int len) {
    unsigned char c[4];
    int index, begin, length;
    Request_piece *request_piece, *p;

    if (peer == NULL || buff == NULL) return -1;

    if (peer->am_choking == 0 && peer->peer_interested == 1) {
        c[0] = buff[5];
        c[1] = buff[6];
        c[2] = buff[7];
        c[3] = buff[8];
        index = char_to_int(c);
        c[0] = buff[9];
        c[1] = buff[10];
        c[2] = buff[11];
        c[3] = buff[12];
        begin = char_to_int(c);
        c[0] = buff[13];
        c[1] = buff[14];
        c[2] = buff[15];
        c[3] = buff[16];
        length = char_to_int(c);

        // 错误的slice请求
        if (begin % (16 * 1024) != 0) { return 0; }

        // 查看该请求是否已存在,若已存在,则不进行处理
        p = peer->Requested_piece_head;
        while (p != NULL) {
            if (p->index == index && p->begin == begin && p->length == length) {
                break;
            }
            p = p->next;
        }
        if (p != NULL) return 0;

        // 将请求加入到请求队列中
        request_piece = (Request_piece *)malloc(sizeof(Request_piece));
        if (request_piece == NULL) {
            printf("%s:%d error", __FILE__, __LINE__);
            return 0;
        }
        request_piece->index = index;
        request_piece->begin = begin;
        request_piece->length = length;
        request_piece->next = NULL;
        if (peer->Requested_piece_head == NULL)
            peer->Requested_piece_head = request_piece;
        else {
            p = peer->Requested_piece_head;
            while (p->next != NULL) p = p->next;
            p->next = request_piece;
        }
        // printf("*** add a request FROM IP:%s index:%-6d begin:%-6x ***\n",
        //        peer->ip,index,begin);
    }

    peer->start_timestamp = time(NULL);
    return 0;
}

/**
 * @brief 处理接收到的来自peer的piece消息
 * 
 * @param peer 发送该条消息的 Peer节点指针
 * @param buff piece消息
 * @param len  piece消息长度
 * @return int 处理成功返回0，处理失败返回-1
 */
int process_piece_msg(Peer *peer, unsigned char *buff, int len) {
    unsigned char c[4];
    int index, begin, length;
    Request_piece *p;

    if (peer == NULL || buff == NULL) return -1;

    if (peer->peer_choking == 0) {
        c[0] = buff[0];
        c[1] = buff[1];
        c[2] = buff[2];
        c[3] = buff[3];
        length = char_to_int(c) - 9;
        c[0] = buff[5];
        c[1] = buff[6];
        c[2] = buff[7];
        c[3] = buff[8];
        index = char_to_int(c);
        c[0] = buff[9];
        c[1] = buff[10];
        c[2] = buff[11];
        c[3] = buff[12];
        begin = char_to_int(c);

        p = peer->Request_piece_head;
        while (p != NULL) {
            if (p->index == index && p->begin == begin && p->length == length)
                break;
            p = p->next;
        }
        if (p == NULL) {
            printf("did not found matched request\n");
            return -1;
        }

        if (peer->last_down_timestamp == 0)
            peer->last_down_timestamp = time(NULL);
        peer->down_count += length;
        peer->down_total += length;

        write_slice_to_btcache(index, begin, length, buff + 13, length, peer);

        create_req_slice_msg(peer);
    }

    peer->start_timestamp = time(NULL);
    return 0;
}

/**
 * @brief 处理收到的消息（peer的接收缓冲区中可能存放着多条消息），消息类型位上述12种消息
 * 
 * @param peer 需要处理的 Peer节点指针
 * @return int 处理成功返回0，处理失败返回-1
 */
int parse_response(Peer *peer) {
    unsigned char btkeyword[20];
    unsigned char keep_alive[4] = {0x0, 0x0, 0x0, 0x0};
    int index;
    unsigned char *buff = (unsigned char *)peer->in_buff;
    int len = peer->buff_len;

    if (buff == NULL || peer == NULL) return -1;

    // BitTorrent协议关键字
    btkeyword[0] = 19;
    memcpy(&btkeyword[1], "BitTorrent protocol", 19);

    // 分别处理12种消息
    for (index = 0; index < len;) {
        if ((len - index >= 68) && (memcmp(&buff[index], btkeyword, 20) == 0)) {
            process_handshake_msg(peer, buff + index, 68);
            index += 68;
        } else if (
            (len - index >= 4) && (memcmp(&buff[index], keep_alive, 4) == 0)) {
            process_keep_alive_msg(peer, buff + index, 4);
            index += 4;
        } else if ((len - index >= 5) && (buff[index + 4] == CHOKE)) {
            process_choke_msg(peer, buff + index, 5);
            index += 5;
        } else if ((len - index >= 5) && (buff[index + 4] == UNCHOKE)) {
            process_unchoke_msg(peer, buff + index, 5);
            index += 5;
        } else if ((len - index >= 5) && (buff[index + 4] == INTERESTED)) {
            process_interested_msg(peer, buff + index, 5);
            index += 5;
        } else if ((len - index >= 5) && (buff[index + 4] == UNINTERESTED)) {
            process_uninterested_msg(peer, buff + index, 5);
            index += 5;
        } else if ((len - index >= 9) && (buff[index + 4] == HAVE)) {
            process_have_msg(peer, buff + index, 9);
            index += 9;
        } else if ((len - index >= 5) && (buff[index + 4] == BITFIELD)) {
            process_bitfield_msg(
                peer, buff + index, peer->bitmap.bitfield_length + 5);
            index += peer->bitmap.bitfield_length + 5;
        } else if ((len - index >= 17) && (buff[index + 4] == REQUEST)) {
            process_request_msg(peer, buff + index, 17);
            index += 17;
        } else if ((len - index >= 13) && (buff[index + 4] == PIECE)) {
            unsigned char c[4];
            int length;

            c[0] = buff[index];
            c[1] = buff[index + 1];
            c[2] = buff[index + 2];
            c[3] = buff[index + 3];
            length = char_to_int(c) - 9;

            process_piece_msg(peer, buff + index, length + 13);
            index += length + 13; // length+13为piece消息的长度
        } else if ((len - index >= 17) && (buff[index + 4] == CANCEL)) {
            process_cancel_msg(peer, buff + index, 17);
            index += 17;
        } else if ((len - index >= 7) && (buff[index + 4] == PORT)) {
            index += 7;
        } else {
            // 如果是未知的消息类型,则跳过不予处理
            unsigned char c[4];
            int length;
            if (index + 4 <= len) {
                c[0] = buff[index];
                c[1] = buff[index + 1];
                c[2] = buff[index + 2];
                c[3] = buff[index + 3];
                length = char_to_int(c);
                if (index + 4 + length <= len) {
                    index += 4 + length;
                    continue;
                }
            }
            // 如果是一条错误的消息,清空接收缓冲区
            peer->buff_len = 0;
            return -1;
        }
    } // end for

    // 接收缓冲区中的消息处理完毕后,清空接收缓冲区
    peer->buff_len = 0;

    return 0;
}

/**
 * @brief 处理收到的缓冲区中的不完整的消息
 * 
 * @param peer   需要处理的 Peer节点指针
 * @param ok_len 接收缓冲区中完整消息的长度
 * @return int   处理成功返回0，处理失败返回-1
 */
int parse_response_uncomplete_msg(Peer *peer, int ok_len) {
    char *tmp_buff;
    int tmp_buff_len;

    // 分配存储空间,并保存接收缓冲区中不完整的消息
    tmp_buff_len = peer->buff_len - ok_len;
    if (tmp_buff_len <= 0) return -1;
    tmp_buff = (char *)malloc(tmp_buff_len);
    if (tmp_buff == NULL) {
        printf("%s:%d error\n", __FILE__, __LINE__);
        return -1;
    }
    memcpy(tmp_buff, peer->in_buff + ok_len, tmp_buff_len);

    // 处理接收缓冲区中前面完整的消息
    peer->buff_len = ok_len;
    parse_response(peer);

    // 将不完整的消息拷贝到接收缓冲区的开始处
    memcpy(peer->in_buff, tmp_buff, tmp_buff_len);
    peer->buff_len = tmp_buff_len;
    if (tmp_buff != NULL) free(tmp_buff);

    return 0;
}

// 
/**
 * @brief 准备发送 have 消息。当下载完一个piece时,应该向所有的peer发送have消息。
 * 
 * @return int 执行成功返回0，执行失败返回-1
 */
int prepare_send_have_msg() {
    Peer *p = peer_head;
    int i;

    if (peer_head == NULL) return -1;
    if (have_piece_index[0] == -1) return -1;

    while (p != NULL) {
        for (i = 0; i < 64; i++) {
            if (have_piece_index[i] != -1) {
                create_have_msg(have_piece_index[i], p);
            } else {
                break;
            }
        }
        p = p->next;
    }

    for (i = 0; i < 64; i++) {
        if (have_piece_index[i] == -1) {
            break;
        } else {
            have_piece_index[i] = -1;
        }
    }

    return 0;
}

// 主动创建发送给peer的消息,而不是等收到某个消息再作出响应
/**
 * @brief 创建发送给peer的消息，而不是等收到某个消息后再创建响应消息
 * 
 * @param peer 消息的目标 Peer节点
 * @return int 执行成功返回0，执行失败返回-1
 */
int create_response_message(Peer *peer) {
    if (peer == NULL) return -1;

    if (peer->state == INITIAL) {
        create_handshake_msg(info_hash, peer_id, peer);
        peer->state = HALFSHAKED;
        return 0;
    }

    if (peer->state == HANDSHAKED) {
        if (bitmap == NULL) return -1;
        create_bitfield_msg(bitmap->bitfield, bitmap->bitfield_length, peer);
        peer->state = SENDBITFIELD;
        return 0;
    }

    // 发送piece消息,即发送下载文件的内容
    if (peer->am_choking == 0 && peer->Requested_piece_head != NULL) {
        Request_piece *req_p = peer->Requested_piece_head;
        int ret = read_slice_for_send(
            req_p->index, req_p->begin, req_p->length, peer);
        if (ret < 0) {
            printf("read_slice_for_send ERROR\n");
        } else {
            if (peer->last_up_timestamp == 0)
                peer->last_up_timestamp = time(NULL);
            peer->up_count += req_p->length;
            peer->up_total += req_p->length;
            peer->Requested_piece_head = req_p->next;

            printf("********* sending a slice TO:%s index:%-5d begin:%-5x\
                   *********\n", peer->ip,req_p->index,req_p->begin);

            free(req_p);
            return 0;
        }
    }

    // 如果3分钟没有收到任何消息关闭连接
    time_t now = time(NULL); // 获取当前时间
    long interval1 = now - peer->start_timestamp;
    if (interval1 > 180) {
        peer->state = CLOSING;
        discard_send_buffer(peer); // 丢弃发送缓冲区中的数据
        clear_btcache_before_peer_close(peer);
        close(peer->socket);
    }
    // 如果45秒没有发送和接收消息,则发送一个keep_alive消息
    long interval2 = now - peer->recet_timestamp;
    if (interval1 > 45 && interval2 > 45 && peer->msg_len == 0)
        create_keep_alive_msg(peer);

    return 0;
}

/**
 * @brief 与peer断开时，丢弃发送缓冲区中的消息
 * 
 * @param peer 断开的目标 Peer节点
 */
void discard_send_buffer(Peer *peer) {
    struct linger lin;
    int lin_len;

    lin.l_onoff = 1;
    lin.l_linger = 0;
    lin_len = sizeof(lin);

    if (peer->socket > 0) {
        setsockopt(peer->socket, SOL_SOCKET, SO_LINGER, (char *)&lin, lin_len);
    }
}


// 以十六进制的形式打印消息的内容，用于调试
int print_msg_buffer(unsigned char *buffer, int len) {
    int i;

    for (i = 0; i < len; i++) {
        printf("%.2x ", buffer[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");

    return 0;
}