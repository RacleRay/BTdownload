#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include "../include/bitfield.h"
#include "../include/parse_metafile.h"
#include "../include/log.h"


void test_bitmap() {
    Bitmap map1, map2;
    unsigned char bf1[2] = { 0xa0, 0xa0 };
    unsigned char bf2[2] = { 0xc0, 0xc0 };

    map1.bitfield        = bf1;
    map1.bitfield_length = 2;
    map1.valid_length    = 11;
    map2.bitfield        = bf2;
    map2.bitfield_length = 2;
    map2.valid_length    = 11;

    int ret = is_interested(&map1,&map2);
    printf("%d\n",ret);
}


int main(int argc, char *argv[])
{
	int ret;

	if(argc != 2) {
		printf("usage:%s metafile\n",argv[0]);
		exit(-1);
	}

	// 解析种子文件
	ret = parse_metafile(argv[1]);
	if(ret != 0)  { printf("%s:%d error\n",__FILE__,__LINE__); return -1; }


    test_bitmap();


	return 0;
}