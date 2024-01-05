#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>

int main() {
    DIR *dir;
    struct dirent *entry;
    char self_tty[256]; //current TTY
    ssize_t self_fd_len =  readlink("/proc/self/fd/0", self_tty, sizeof(self_tty)-1);
    self_tty[self_fd_len] ='\0';

    // open /proc
    dir = opendir("/proc");
    if (dir == NULL) {
        perror("/proc file could not be opened");
        return 1;
    }

    printf(" PID TTY       TIME CMD\n");

    //rounds all file
    while ((entry = readdir(dir)) != NULL) {
        int pid; // PID
        char cmd[256]; //CMD
        char stat_path[256]; //stat file path
        char fd_path[256]; // fd path
        char process_tty[256]; //process TTY

        unsigned long utime, stime;
        FILE *stat_file;

        // whether the directory name is numeric (check process)
        if (sscanf(entry->d_name, "%d", &pid) != 1) {
            continue;
        }
        snprintf(fd_path, sizeof(fd_path), "/proc/%d/fd/0", pid);
        ssize_t process_fd_len = readlink(fd_path, process_tty, sizeof(process_tty)-1);
        // if couldn't get process_fd_len
        if(process_fd_len == -1) {
            continue;
        }
        process_tty[process_fd_len] = '\0';

        // not print if the process does not match the current terminal
        if(strcmp(self_tty, process_tty) != 0) {
            continue;
        }
        // to subtract "/dev/"
        char* tty_ptr = strchr(process_tty, '/');
        tty_ptr = strchr(tty_ptr+1, '/');
        tty_ptr++;

        // getting PID, TIME, CMD in stat file
        snprintf(stat_path, sizeof(stat_path), "/proc/%d/stat", pid);

        stat_file = fopen(stat_path, "r");
        if (stat_file == NULL) {
            continue;
        }
        fscanf(stat_file, "%*d (%[^)]) %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu", cmd,  &utime, &stime);

        fclose(stat_file);
        unsigned long ticks = sysconf(_SC_CLK_TCK);
        unsigned long total_seconds = (utime + stime) / ticks;
        unsigned long days = total_seconds / (24 * 3600);
        unsigned long hours = (total_seconds % (24 * 3600)) / 3600;
        unsigned long minutes = (total_seconds % 3600) / 60;
        unsigned long seconds = total_seconds % 60;

        if(days > 0) {
            printf("%d %s %lu-%02lu:%02lu:%02lu %s\n", pid, tty_ptr, days, hours, minutes, seconds, cmd);
        } else {
            printf("%d %s %02lu:%02lu:%02lu %s\n", pid, tty_ptr, hours, minutes, seconds, cmd);
        }
    }
    closedir(dir);

    return 0;
}