/*************************************************************************
    > File Name: sha1.h
    > Author: racle
    > Mail: racleray@qq.com
    > Created Time: 
 ************************************************************************/

#ifndef _SHA1_H
#define _SHA1_H


/**
Copy from opensource.

SHA-1 in C   By Steve Reid <steve@edmweb.com>   100% Public Domain

Test Vectors (from FIPS PUB 180-1)
"abc"
  A9993E36 4706816A BA3E2571 7850C26C 9CD0D89D
"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
  84983E44 1C3BD26E BAAE4AA1 F95129E5 E54670F1
A million repetitions of "a"
  34AA973C D4C4DAA4 F61EEB2B DBAD2731 6534016F
*/

#include <sys/types.h>

#define SHA1HANDSOFF

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    unsigned long state[5];
    unsigned long count[2];
    unsigned char buffer[64];
} SHA1_CTX;

void SHA1Transform(unsigned long state[5], unsigned char buffer[64]);
void SHA1Init(SHA1_CTX* context);
void SHA1Update(SHA1_CTX* context, unsigned char* data, unsigned int len);
void SHA1Final(unsigned char digest[20], SHA1_CTX* context);

#ifdef __cplusplus
}
#endif

#endif
