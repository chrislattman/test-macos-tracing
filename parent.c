#include <mach/mach.h>
#include <signal.h>
#include <stdio.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#if defined(__amd64__)
typedef x86_thread_state64_t THREAD_STATE;
#elif defined(__aarch64__)
typedef arm_thread_state64_t THREAD_STATE;
#endif

int main(void)
{
    mach_port_name_t child_task_port;
    thread_act_array_t thread_list;
    mach_msg_type_number_t thread_count, sc, size;
    THREAD_STATE reg_state;
    vm_offset_t data;
    int value = 100;

    pid_t pid = fork();
    if (pid == 0) {
        // To trace an existing process use PT_ATTACHEXC
        ptrace(PT_TRACE_ME, 0, (caddr_t) 0, 0);
        execl("./child", "child", (char *) 0);
    } else {
        wait(NULL);
        ptrace(PT_CONTINUE, pid, (caddr_t) 1, 0);
        sleep(1);
        kill(pid, SIGINT);
        wait(NULL);
        task_for_pid(mach_task_self(), pid, &child_task_port);
        task_threads(child_task_port, &thread_list, &thread_count);
#if defined(__amd64__)
        sc = x86_THREAD_STATE64_COUNT;
        thread_get_state(thread_list[0], x86_THREAD_STATE64, (thread_state_t) &reg_state, &sc);
        printf("Register rbp = 0x%llx\n", reg_state.__rbp);
        vm_read(child_task_port, reg_state.__rbp - 4, sizeof(int), &data, &size);
        printf("arg1 = *(rbp - 4) = %d\n", *((int *) data));
        vm_read(child_task_port, reg_state.__rbp - 8, sizeof(int), &data, &size);
        printf("arg2 = *(rbp - 8) = %d\n", *((int *) data));
        vm_write(child_task_port, reg_state.__rbp - 4, (vm_offset_t) &value, sizeof(int));
#elif defined(__aarch64__)
        sc = ARM_THREAD_STATE64_COUNT;
        thread_get_state(thread_list[0], ARM_THREAD_STATE64, (thread_state_t) &reg_state, &sc);
        printf("Register sp = 0x%llx\n", reg_state.__sp);
        vm_read(child_task_port, reg_state.__sp + 12, sizeof(int), &data, &size);
        printf("arg1 = *(sp + 12) = %d\n", *((int *) data));
        vm_read(child_task_port, reg_state.__sp + 8, sizeof(int), &data, &size);
        printf("arg2 = *(sp + 8) = %d\n", *((int *) data));
        vm_write(child_task_port, reg_state.__sp + 12, (vm_offset_t) &value, sizeof(int));
#endif
        ptrace(PT_CONTINUE, pid, (caddr_t) 1, 0);
        wait(NULL);
    }

    return 0;
}
