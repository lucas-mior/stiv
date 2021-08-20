#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <openssl/sha.h>

void sha256(char *string, char outputBuffer[65]) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    int len;
    SHA256_CTX sha256;
    SHA256_Init(&sha256);

    len = strlen(string);
    SHA256_Update(&sha256, string, len);
    SHA256_Final(hash, &sha256);
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        sprintf(outputBuffer + (i * 2), "%02x", (unsigned char)hash[i]);

    outputBuffer[64] = 0;
}

char * sha256_file(char *path) {
    const int bufSize = 32768;
    int bytesRead = 0;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    unsigned char *buffer;
    char *outputBuffer = NULL;
    FILE *file;
    if (!(file = fopen(path, "rb")))
        return NULL;

    SHA256_CTX sha256;
    SHA256_Init(&sha256);

    if (!(buffer = malloc(bufSize)))
        return NULL;

    while((bytesRead = fread(buffer, 1, bufSize, file)))
        SHA256_Update(&sha256, buffer, bytesRead);

    SHA256_Final(hash, &sha256);

    if (!(outputBuffer = malloc(64)))
        return NULL;

    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        sprintf(outputBuffer + (i * 2), "%02x", (unsigned char)hash[i]);

    *(outputBuffer+64) = 0;

    fclose(file);
    free(buffer);
    return outputBuffer;
}
