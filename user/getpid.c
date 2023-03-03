#include "syscall.h"
#include "stdio.h"


#define K 3


static inline long getpid() {
    long ret;
    asm volatile ("li a7, %1\n"
                  "ecall\n"
                  "mv %0, a0\n"
                : "+r" (ret) 
                : "i" (SYS_GETPID));
    return ret;
}

static inline long fork()
{
  long ret;
  asm volatile ("li a7, %1\n"
                "ecall\n"
                "mv %0, a0\n"
                : "+r" (ret) : "i" (SYS_CLONE));
  return ret;
}

int global_variable = 0;


int main1() {

  int pid;

  pid = fork();

    while(1) {
        printf("[PID = %d] is running, variable: %d\n", getpid(), global_variable++);
        for (unsigned int i = 0; i < 0x7FFFFFF * K; i++);
    }
}

int main2() {
    int pid;

    pid = fork();

    if (pid == 0) {
        while (1) {
            printf("[U-CHILD] pid: %ld is running!, global_variable: %d\n", getpid(), global_variable++);
            for (unsigned int i = 0; i < 0x7FFFFFF * K; i++);
        } 
    } else {
        while (1) {
            printf("[U-PARENT] pid: %ld is running!, global_variable: %d\n", getpid(), global_variable++);
            for (unsigned int i = 0; i < 0x7FFFFFF * K; i++);
        } 
    }
    return 0;
}



int main3() {
    global_variable = 0;
    int pid;

    for (int i = 0; i < 3; i++)
        printf("[U] pid: %ld is running!, global_variable: %d\n", getpid(), global_variable++);

    pid = fork();

    if (pid == 0) {
        while (1) {
            printf("[U-CHILD] pid: %ld is running!, global_variable: %d\n", getpid(), global_variable++);
            for (unsigned int i = 0; i < 0x7FFFFFF * K ; i++);
        } 
    } else {
        while (1) {
            printf("[U-PARENT] pid: %ld is running!, global_variable: %d\n", getpid(), global_variable++);
            for (unsigned int i = 0; i < 0x7FFFFFF * K; i++);
        } 
    }
    return 0;
}

int main() {
    global_variable = 0;

    printf("[U%ld] pid: %ld is running!, global_variable: %d\n", getpid(), getpid(), global_variable++);
    fork();

    printf("[U%ld] pid: %ld is running!, global_variable: %d\n", getpid(), getpid(), global_variable++);
    fork();

    while(1) {
        printf("[U%ld] pid: %ld is running!, global_variable: %d\n",getpid(),  getpid(), global_variable++);
        for (unsigned int i = 0; i < 0x7FFFFFF * K; i++);
    }
}