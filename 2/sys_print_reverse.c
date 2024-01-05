#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

asmlinkage long sys_print_reverse(char __user *input, char __user *output, size_t input_len)
{
    char *buf;
    char *temp;
    long i, j;
    int ret;

    
    buf = (char*)kmalloc(input_len + 1, GFP_KERNEL);
    if (!buf) {
        return -ENOMEM;
    }
    ret = copy_from_user(buf, input, input_len);
    if (ret) {
        kfree(buf);
        return -EFAULT;
    }
    buf[input_len] = '\0';

    temp = (char*)kmalloc(input_len + 1, GFP_KERNEL);
    if (!temp) {
        kfree(buf);
        return -ENOMEM;
    }

    // reverse the string
    for (i = 0, j = input_len - 1; i < input_len; i++, j--) {
        temp[i] = buf[j];
    }
    temp[input_len] = '\0';

    
    ret = copy_to_user(output, temp, input_len + 1);
    if (ret) {
        kfree(buf);
        kfree(temp);
        return -EFAULT;
    }
    
    kfree(buf);
    kfree(temp);
    return 0;
}


SYSCALL_DEFINE3(print_reverse, char __user *, input, char __user *, output, size_t, input_len)
{
    return sys_print_reverse(input, output, input_len);
}
