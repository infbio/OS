#include <linux/kernel.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

char input[256];
char output[256];

char* trim(char *s)
{
    
    while(isspace((unsigned char)*s)) s++;

    if(*s == 0)
        return s;

    char *end = s + strlen(s) - 1;
    while(end > s && isspace((unsigned char)*end)) end--;

    end[1] = '\0';

    return s;
}


void removeSpaces(char *input) {
    int count = 0;
    for (int i = 0; input[i]; i++) {
        if (input[i] != ' ') {
            input[count++] = input[i];
        }
    }
    input[count] = '\0';
}

bool checkValidInput(const char *input) {
    int operatorCount = 0;

    if (input[0] == '\0') return false;

    if (input[0] == '+' || input[0] == '-' ||
        input[strlen(input) - 1] == '+' ||
        input[strlen(input) - 1] == '-') {
        return false;
    }

    for (int i = 0; input[i] != '\0'; i++) {
        if (!isdigit(input[i])) {
            if (input[i] == '+' || input[i] == '-') {
                operatorCount++;
            } else {
                return false;
            }
        }
    }
    if (operatorCount > 1) return false;

    return true;
}

int main(void)
{
    while(1) {
        printf("Input: ");
        fgets(input, sizeof(input), stdin);
        size_t len = strlen(input);
        if(len > 0 && input[len-1] == '\n') {
            input[len-1] = '\0';
        }
        trim(input);
        if(input[0] == '\0') {
            break;
        }
        removeSpaces(input);
        if(!checkValidInput(input)) {
            printf("Wrong Input!\n");
            continue;
        }
        
        char *left, *right;
        size_t left_len, right_len;
        if (strchr(input, '+')) { // sys_plus_reverse() system call function
            left = strtok(input, "+");
            right = strtok(NULL, "+");
            left_len = strlen(left);
            right_len = strlen(right);
            long value = syscall(450, left, right, output, left_len, right_len);
            if(value < 0) {
                perror("system call failed");
                printf("errno: %d\n", errno);
                printf("sys_plus_reverse failed with error code %ld\n", value);
                continue;
            }
        } else if (strchr(input, '-')) { // sys_minus_reverse() system call function
            left = strtok(input, "-");
            right = strtok(NULL, "-");
            left_len = strlen(left);
            right_len = strlen(right);
            long value = syscall(451, left, right, output, left_len, right_len);
            if(value < 0) {
                perror("system call failed");
                printf("errno: %d\n", errno);
                printf("sys_minus_reverse failed with error code %ld\n", value);
                continue;
            }
        } else { // sys_print_reverse() system call function
            size_t input_len = strlen(input);
            long value = syscall(449, input, output, input_len);
            if(value < 0) {
                perror("system call failed");
                printf("errno: %d\n", errno);
                printf("sys_print_reverse failed with error code %ld\n", value);
                continue;
            }
        }
        printf("Output: %s\n", output);
    }
    return 0;
}
