#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sched.h>
#include <sys/resource.h>
#define READ_END 0
#define WRITE_END 1

struct ChildProcessData
{
    pid_t pid;
    int nice_value;
    struct timespec start_time;
    struct timespec end_time;
    double elapsed_time;
};

void run() {
    int count = 0, k, i, j;
    int result[100][100] = {0};
    int A[100][100];
    int B[100][100];
    for (int i = 0; i < 100; ++i) {
        for (int j = 0; j < 100; ++j) {
            A[i][j] = 1;
            B[i][j] = 1;
        }
    }

    while(count < 100){
        for(k = 0; k < 100; k++){
            for(i = 0; i < 100; i++){
                for(j = 0; j < 100; j++){
                    result[k][j] += A[k][i] * B[i][j];
                }
            }
        }
        count++;
    }
}

void format_time(char *buffer, struct timespec ts) {
    struct tm *tm_info;
    char time_str[10];
    
    tm_info = localtime(&(ts.tv_sec));
    strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_info);
    snprintf(buffer, 26, "%s.%06ld", time_str, ts.tv_nsec/1000);
}

int main(void) {
    FILE *fp;
    pid_t pid[21];
    double total_elapsed_time = 0;
    int policy;
    int fd[2];
    struct sched_param param;
    struct ChildProcessData child[21];
    int time_quantum = 100;
    int timeslice;

    printf("Input the Scheduling Policy to apply:\n");
    printf("1. CFS_DEFAULT\n");
    printf("2. CFS_NICE\n");
    printf("3. RT_FIFO\n");
    printf("4. RT_RR\n");
    scanf("%d", &policy);

    if(pipe(fd) == -1) {
        fprintf(stderr, "Pipe failed");
        return 1;
    }

    switch(policy) {
        case 1: //CFS_DEFAULT
            for(int i = 0; i < 21; ++i) {
                pid[i] = fork();
                clock_gettime(CLOCK_REALTIME, &child[i].start_time);
                if(pid[i] < 0) {
                    perror("fork");
                    exit(EXIT_FAILURE);
                } else if (pid[i] == 0) {
                    close(fd[READ_END]);
                    child[i].pid = getpid();
                    param.sched_priority=0;
                    if(sched_setscheduler(child[i].pid, SCHED_OTHER, &param) == -1) {
                        perror("sched_setscheduler");
                        exit(EXIT_FAILURE);
                    }
                    run();
                    clock_gettime(CLOCK_REALTIME, &child[i].end_time);
                    child[i].elapsed_time = (child[i].end_time.tv_sec - child[i].start_time.tv_sec) + (child[i].end_time.tv_nsec - child[i].start_time.tv_nsec) / 1e9;
                    write(fd[WRITE_END], &child[i], sizeof(child[i]));
                    close(fd[WRITE_END]);
                    exit(EXIT_SUCCESS);
                }
            }
            break;
        case 2: //CFS_NICE
            for(int i=0; i < 21; ++i) {
                pid[i] = fork();
                clock_gettime(CLOCK_REALTIME, &child[i].start_time);
                if(pid[i] < 0) {
                    perror("fork");
                    exit(EXIT_FAILURE);
                } else if (pid[i] == 0) {
                    close(fd[READ_END]);
                    child[i].pid = getpid();
                    if(i<7) {
                        if(setpriority(PRIO_PROCESS, child[i].pid, -20) == -1) {
                            perror("setpriority");
                        }
                    } else if(i < 14) {
                        if(setpriority(PRIO_PROCESS, child[i].pid, 0) == -1) {
                            perror("setpriority");
                        }
                    } else {
                        if(setpriority(PRIO_PROCESS, child[i].pid, 19) == -1) {
                            perror("setpriority");
                        }
                    }
                    param.sched_priority=0;
                    if(sched_setscheduler(child[i].pid, SCHED_OTHER, &param) == -1) {
                        perror("sched_setscheduler");
                        exit(EXIT_FAILURE);
                    }
                    child[i].nice_value = getpriority(PRIO_PROCESS, child[i].pid);
                    run();
                    clock_gettime(CLOCK_REALTIME, &child[i].end_time);
                    child[i].elapsed_time = (child[i].end_time.tv_sec - child[i].start_time.tv_sec) + (child[i].end_time.tv_nsec - child[i].start_time.tv_nsec) / 1e9;
                    write(fd[WRITE_END], &child[i], sizeof(child[i]));
                    close(fd[WRITE_END]);
                    exit(EXIT_SUCCESS);
                }
            }
            break;
        case 3: //RT_FIFO
            for(int i = 0; i < 21; ++i) {
                pid[i] = fork();
                clock_gettime(CLOCK_REALTIME, &child[i].start_time);
                if(pid[i] < 0) {
                    perror("fork");
                    exit(EXIT_FAILURE);
                } else if (pid[i] == 0) {
                    close(fd[READ_END]);
                    child[i].pid = getpid();
                    param.sched_priority = sched_get_priority_max(SCHED_FIFO);
                    if(sched_setscheduler(child[i].pid, SCHED_FIFO, &param) == -1) {
                        perror("sched_setscheduler");
                        exit(EXIT_FAILURE);
                    }
                    run();
                    clock_gettime(CLOCK_REALTIME, &child[i].end_time);
                    child[i].elapsed_time = (child[i].end_time.tv_sec - child[i].start_time.tv_sec) + (child[i].end_time.tv_nsec - child[i].start_time.tv_nsec) / 1e9;
                    write(fd[WRITE_END], &child[i], sizeof(child[i]));
                    close(fd[WRITE_END]);
                    exit(EXIT_SUCCESS);
                }
            }
            break;
        case 4: //RT_RR
            for(int i = 0; i < 21; ++i) {
                pid[i] = fork();
                clock_gettime(CLOCK_REALTIME, &child[i].start_time);
                if(pid[i] < 0) {
                    perror("fork");
                    exit(EXIT_FAILURE);
                } else if (pid[i] == 0) {
                    close(fd[READ_END]);
                    child[i].pid = getpid();
                    param.sched_priority = sched_get_priority_max(SCHED_RR);
                    if(sched_setscheduler(child[i].pid, SCHED_RR, &param) == -1) {
                        perror("sched_setscheduler");
                        exit(EXIT_FAILURE);
                    }
                    run();
                    clock_gettime(CLOCK_REALTIME, &child[i].end_time);
                    child[i].elapsed_time = (child[i].end_time.tv_sec - child[i].start_time.tv_sec) + (child[i].end_time.tv_nsec - child[i].start_time.tv_nsec) / 1e9;
                    write(fd[WRITE_END], &child[i], sizeof(child[i]));
                    close(fd[WRITE_END]);
                    exit(EXIT_SUCCESS);
                }
            }
            break;
    }

    for(int i = 0; i < 21; ++i) {
        waitpid(pid[i], NULL, 0);
    }

    close(fd[WRITE_END]);

    for(int i=0; i < 21; ++i) {
        read(fd[READ_END], &child[i], sizeof(child[i]));
        total_elapsed_time += child[i].elapsed_time;
    }

    close(fd[READ_END]);

    if (policy == 2) {
        for(int i = 0; i < 21; ++i) {
            char start_time_str[26], end_time_str[26];
            format_time(start_time_str, child[i].start_time);
            format_time(end_time_str, child[i].end_time);
            printf("PID: %d | NICE: %d | Start time: %s | End time: %s | Elapsed time: %.6lf\n",
            child[i].pid, child[i].nice_value,
            start_time_str, end_time_str,
            child[i].elapsed_time);
        }
    } else {
        for(int i = 0; i < 21; ++i) {
            char start_time_str[26], end_time_str[26];
            format_time(start_time_str, child[i].start_time);
            format_time(end_time_str, child[i].end_time);
            printf("PID: %d | Start time: %s | End time: %s | Elapsed time: %.6lf\n",
            child[i].pid,
            start_time_str, end_time_str,
            child[i].elapsed_time);
        }
    }
    
    switch (policy) {
    case 1:
        printf("Scheduling Policy: CFS_DEFAULT | ");
        break;
    case 2:
        printf("Scheduling Policy: CFS_NICE | ");
        break;
    case 3:
        printf("Scheduling Policy: RT_FIFO | ");
        break;
    case 4:
        fp = fopen("/proc/sys/kernel/sched_rr_timeslice_ms", "r");
        if (fp == NULL) {
            printf("Err: cannot open file\n");
            return 1;
        }
        fscanf(fp, "%d", &timeslice);
        fclose(fp);
        printf("Scheduling Policy: RT_RR | Time Quantum : %d ms | ", timeslice);
        break;
    }
    printf("Average elapsed time: %.6lf\n", total_elapsed_time / (double)21);

    return 0;
}