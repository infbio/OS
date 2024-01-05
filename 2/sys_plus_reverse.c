#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

asmlinkage long sys_plus_reverse(char __user *left, char __user *right, char __user *result, size_t left_len, size_t right_len)
{
    long left_num, right_num, operation_result;
    char *left_buf, *right_buf, *temp_result;
    int left_ret, right_ret;
    size_t result_len;

    left_buf = (char*)kmalloc(left_len + 1, GFP_KERNEL);
    right_buf = (char*)kmalloc(right_len + 1, GFP_KERNEL);
    if (!left_buf || !right_buf) {
        kfree(left_buf);
        kfree(right_buf);
        return -ENOMEM;
    }

    left_ret = copy_from_user(left_buf, left, left_len);
    right_ret = copy_from_user(right_buf, right, right_len);
    if(left_ret || right_ret) {
        kfree(left_buf);
        kfree(right_buf);
        return -EFAULT;
    }
    left_buf[left_len] = '\0';
    right_buf[right_len] = '\0';

    left_ret = kstrtoul(left_buf, 10, &left_num);
    right_ret = kstrtoul(right_buf, 10, &right_num);
    if(left_ret != 0 || right_ret != 0) {
        kfree(left_buf);
        kfree(right_buf);
        return -EINVAL;
    }

    operation_result = left_num - right_num;
    result_len = snprintf(NULL, 0, "%ld", operation_result);
    temp_result = (char*)kmalloc(result_len + 1, GFP_KERNEL);
    if(!temp_result) {
        kfree(left_buf);
        kfree(right_buf);
        return -ENOMEM;
    }

    snprintf(temp_result, result_len+1, "%ld", operation_result);

    left_ret = copy_to_user(result, temp_result, result_len + 1);
    if(left_ret){
        kfree(temp_result);
        kfree(left_buf);
        kfree(right_buf);
        return -EFAULT;
    }

    kfree(temp_result);
    kfree(left_buf);
    kfree(right_buf);

    return 0;
}

SYSCALL_DEFINE5(plus_reverse, char __user *, left, char __user *, right, char __user *, result, size_t, left_len, size_t, right_len)
{
    return sys_plus_reverse(left, right, result, left_len, right_len);
}

