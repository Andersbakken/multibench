#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

int count = 1024;
static void *thread(void *idx)
{
    const int size = 1024 * 1024;
    unsigned char *mem = static_cast<unsigned char *>(malloc(size));
    for (int i=0; i<size; ++i) {
        mem[i] = i % 256;
    }

    unsigned long long tot = 0;
    for (int i=0; i<count; ++i) {
        for (int j=0; j<count; ++j) {
            unsigned long long val = mem[i];
            val >>= (i % 10) + 1;
            tot += val;
        }
    }
    free(mem);
    printf("%llu => %llu\n", reinterpret_cast<unsigned long long>(idx), tot);
    return 0;
}


int main(int argc, char **argv)
{
    int threads = 1;
    for (int i=1; i<argc; ++i) {
        if (!strncmp(argv[i], "--threads=", 10)) {
            threads = atoi(argv[i] + 10);
            if (!threads) {
                printf("[%s:%d]: if (!threads) {\n", __FILE__, __LINE__); fflush(stdout);
                return 1;
            }
        } else if (!strncmp(argv[i], "--count=", 8)) {
            count = atoi(argv[i] + 8);
            if (!count) {
                printf("[%s:%d]: if (!count) {\n", __FILE__, __LINE__); fflush(stdout);
                return 1;
            }
        }
    }
    pthread_t t[threads];
    for (unsigned long long i=0; i<threads; ++i) {
        pthread_create(&t[i], 0, ::thread, reinterpret_cast<unsigned long long*>(i));
    }
    for (int i=0; i<threads; ++i) {
        pthread_join(t[i], 0);
    }

    return 0;
}
