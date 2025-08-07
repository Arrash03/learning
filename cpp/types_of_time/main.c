#define _GNU_SOURCE
#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <sched.h>

#define handle_error(msg) \
        do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define handle_error_en(en, msg) \
        do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

static void *
thread_start(void *arg)
{
    int core_id = arg ? *(int*)arg : 0; // Pass core id as argument
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);
    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0)
        perror("pthread_setaffinity_np");
    printf("Subthread starting infinite loop\n");
    struct timespec start_cpu, end_cpu, wall_start, wall_end;
    clockid_t c_id;
    bool is_valid = true;
    pthread_getcpuclockid(pthread_self(), &c_id);
    if (clock_gettime(CLOCK_MONOTONIC_RAW, &wall_start) != 0)
        is_valid = false;
    if (clock_gettime(c_id, &start_cpu) != 0)
        is_valid = false;


    for (volatile size_t i = 0; i < 1e8; ++i);
    // usleep(100);

    if (clock_gettime(CLOCK_MONOTONIC_RAW, &wall_end) != 0)
        is_valid = false;
    if (clock_gettime(c_id, &end_cpu) != 0)
        is_valid = false;

    uint64_t cpu = 0;
    uint64_t wall = 0;
    if (is_valid)
    {
        cpu = (end_cpu.tv_sec - start_cpu.tv_sec) * 1e6 + end_cpu.tv_nsec - start_cpu.tv_nsec;
        wall = (wall_end.tv_sec - wall_start.tv_sec) * 1e6 + wall_end.tv_nsec - wall_start.tv_nsec;
    }

    printf("cpu: %lu\n", cpu);
    printf("wall %lu\n", wall);
    puts("==========================\n");
    
    for (;;)
        continue;
}

static void
pclock(char *msg, clockid_t cid, size_t i)
{
    struct timespec ts;

    printf(msg, i);
    if (clock_gettime(cid, &ts) == -1)
        handle_error("clock_gettime");
    printf("%4jd.%03ld\n", (intmax_t) ts.tv_sec, ts.tv_nsec / 1000000);
}

int
main(void)
{
    pthread_t threads[5];
    clockid_t cid;

    for (size_t i = 0; i < 5; ++i)
    {
        int s = pthread_create(&threads[i], NULL, thread_start, &i);
        if (s != 0)
            handle_error_en(s, "pthread_create");
    }

    printf("Main thread sleeping\n");
    sleep(1);

    printf("Main thread consuming some CPU time...\n");
    for (unsigned int j = 0; j < 2000000; j++)
        getppid();

    pclock("Process total CPU time: ", CLOCK_PROCESS_CPUTIME_ID, 0);

    int s = pthread_getcpuclockid(pthread_self(), &cid);
    if (s != 0)
        handle_error_en(s, "pthread_getcpuclockid");
    pclock("Main thread CPU time:   ", cid, 0);

    /* The preceding 4 lines of code could have been replaced by:
       pclock("Main thread CPU time:   ", CLOCK_THREAD_CPUTIME_ID); */

    for (size_t i = 0; i < 5; ++i)
    {
        s = pthread_getcpuclockid(threads[i], &cid);
        if (s != 0)
            handle_error_en(s, "pthread_getcpuclockid");
        pclock("Subthread CPU time: %lu    ", cid, threads[i]);
    }

    exit(EXIT_SUCCESS);         /* Terminates both threads */
}
