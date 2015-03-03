#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#if defined(__apple__)
#include <mach/mach.h>
#include <mach/mach_time.h>
#endif

int count = 16384;
unsigned char *data;
bool silent = false;
enum { MB = 1024 * 1024, Megs = 64, Size = Megs * MB };

unsigned long long monoMs()
{
    timeval time;
#if defined(__apple__)
    static mach_timebase_info_data_t info;
    static bool first = true;
    uint64_t machtime = mach_absolute_time();
    if (first) {
        first = false;
        mach_timebase_info(&info);
    }
    machtime = machtime * info.numer / (info.denom * 1000); // microseconds
    time.tv_sec = machtime / 1000000;
    time.tv_usec = machtime % 1000000;
#else
    timespec spec;
    const clockid_t cid = CLOCK_MONOTONIC_RAW;
    const int ret = ::clock_gettime(cid, &spec);
    if (ret == -1) {
        memset(&time, 0, sizeof(timeval));
        return false;
    }
    time.tv_sec = spec.tv_sec;
    time.tv_usec = spec.tv_nsec / 1000;
#endif
    return (time.tv_sec * static_cast<unsigned long long>(1000)) + (time.tv_usec / static_cast<unsigned long long>(1000));
}

static void *thread(void *thread)
{
    unsigned long long tot = 0;
    unsigned long long idx = 0;
    for (int i=0; i<count; ++i) {
        for (int j=0; j<count; ++j) {
            unsigned long long val = data[idx++ % Size];
            val >>= (i % 10) + 1;
            tot += val;
        }
    }
    if (!silent)
        printf("%llu => %llu\n", reinterpret_cast<unsigned long long>(thread), tot);
    return 0;
}


int main(int argc, char **argv)
{
    FILE *f = fopen("/dev/urandom", "r");
    if (!f) {
        printf("[%s:%d]: if (!f) {\n", __FILE__, __LINE__); fflush(stdout);
        return 1;
    }
    unsigned char mb[1024 * 1024];
    if (!fread(mb, sizeof(mb), 1, f)) {
        printf("[%s:%d]: if (!fread(data, Size, 1, f)) {\n", __FILE__, __LINE__); fflush(stdout);
        fclose(f);
        return 1;
    }
    fclose(f);
    data = static_cast<unsigned char*>(malloc(Size));
    for (int i=0; i<Megs; ++i) {
        memcpy(data + (i * MB), mb, MB);
    }

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
        } else if (!strcmp(argv[i], "--silent")) {
            silent = true;
        }
    }
    pthread_t t[threads];
    for (int i=0; i<threads; ++i) {
        pthread_create(&t[i], 0, ::thread, reinterpret_cast<void*>(i));
    }
    for (int i=0; i<threads; ++i) {
        pthread_join(t[i], 0);
    }
    free(data);

    return 0;
}
