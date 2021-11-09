#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <time.h>
#include <sys/sysmacros.h>

#include <openssl/sha.h>

char * sha256(char *string) {
    unsigned char hash[SHA256_DIGEST_LENGTH];

    char *outputBuffer = NULL;
    int len;

    if (!(outputBuffer = malloc(64)))
        return NULL;

    SHA256_CTX sha256;
    SHA256_Init(&sha256);

    len = strlen(string);
    SHA256_Update(&sha256, string, len);
    SHA256_Final(hash, &sha256);
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        sprintf(outputBuffer + (i * 2), "%02x", (unsigned char)hash[i]);

    outputBuffer[64] = 0;
    return outputBuffer;
}

char * cache(char *filename) {
    struct stat filestat;
    char info_to_hash[256];
    char *cache = NULL;

    if (lstat(filename, &filestat) == -1) {
        perror("lstat");
        exit(EXIT_FAILURE);
    }

    snprintf(info_to_hash, 256, "%li%ld", filestat.st_size, filestat.st_mtim.tv_nsec);
    cache = sha256(info_to_hash);
    return cache;
}
