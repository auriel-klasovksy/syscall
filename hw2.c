#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/cred.h>   // For credentials (current process)
#include <linux/sched.h>  // For task_struct and process handling
#include <linux/errno.h>  // For error codes
#include <linux/types.h>  // For pid_t

#define SWORD_BIT 0
#define MIDNIGHT_BIT 1
#define CLAMP_BIT 2

asmlinkage long sys_hello(void){
	printk("Hello, World!\n");
	return 0;
}

// Utility function to check for valid clearance char
static inline int is_valid_clearance(char clr) {
    return clr == 's' || clr == 'm' || clr == 'c';
}

// Utility function to get the clearance bit for a character
static inline int get_clearance_bit(char clr) {
    switch (clr) {
        case 's': return SWORD_BIT;
        case 'm': return MIDNIGHT_BIT;
        case 'c': return CLAMP_BIT;
        default: return -1; // Invalid
    }
}

// Set security clearance for the current process
asmlinkage long sys_set_sec(int sword, int midnight, int clamp) {
    char new_clearance = 0;

    if (sword < 0 || midnight < 0 || clamp < 0) 
        return -EINVAL;

    if (sword > 0) new_clearance |= 1 << SWORD_BIT;
    if (midnight > 0) new_clearance |= 1 << MIDNIGHT_BIT;
    if (clamp > 0) new_clearance |= 1 << CLAMP_BIT;

    if (!capable(CAP_SYS_ADMIN))
        return -EPERM;

    current->clearance = new_clearance; // Assuming `clearance` is a char in task_struct
    return 0;
}

// Get clearance status for the current process
asmlinkage long sys_get_sec(char clr) {
    int bit;

    if (!is_valid_clearance(clr)) 
        return -EINVAL;

    bit = get_clearance_bit(clr);
    return (current->clearance & (1 << bit)) ? 1 : 0;
}

// Check clearance of another process
asmlinkage long sys_check_sec(pid_t pid, char clr) {
    struct task_struct *task;
    int bit;

    if (!is_valid_clearance(clr)) 
        return -EINVAL;

    rcu_read_lock();
    task = find_task_by_vpid(pid); // Find the task_struct for the given pid
    if (!task) {
        rcu_read_unlock();
        return -ESRCH;
    }

    bit = get_clearance_bit(clr);
    if (!(current->clearance & (1 << bit))) {
        rcu_read_unlock();
        return -EPERM;
    }

    rcu_read_unlock();
    return (task->clearance & (1 << bit)) ? 1 : 0;
}

// Set clearance to parent processes up to a certain height
asmlinkage long sys_set_sec_branch(int height, char clr) {
    struct task_struct *task = current->real_parent;
    int count = 0, bit;

    if (height < 0 || !is_valid_clearance(clr)) 
        return -EINVAL;

    bit = get_clearance_bit(clr);
    if (!(current->clearance & (1 << bit))) 
        return -EPERM;

    while (height > 0 && task != &init_task) { // Traverse up the process tree
        task->clearance |= (1 << bit); // Set clearance bit
        task = task->real_parent;
        count++;
        height--;
    }

    return count;
}
