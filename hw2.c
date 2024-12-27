#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/cred.h>

#define SWORD_BIT 1
#define MIDNIGHT_BIT 2
#define CLAMP_BIT 4

asmlinkage long sys_hello(void)	{
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

asmlinkage long sys_set_sec(int sword, int midnight, int clamp) {
	char new_clearance = 0;
	if(!capable(CAP_SYS_ADMIN)) {
		return -EPERM;
	}
	if(sword < 0 || midnight < 0 || clamp < 0) {
		return -EINVAL;
	}
	printk("666 sys_ste 1 %d\n", (int)current->clearance);
	if(sword > 0) {
		new_clearance |= SWORD_BIT;
	}
	else {
		new_clearance &= ~SWORD_BIT;
	}
	if(midnight > 0) {
		new_clearance |= MIDNIGHT_BIT;
	}
	else {
		new_clearance &= ~MIDNIGHT_BIT;
	}
	if(clamp > 0) {
		new_clearance |= CLAMP_BIT;
	}
	else {
		new_clearance &= CLAMP_BIT;
	}
	printk("666 sys_set 2 %d\n", (int)new_clearance);
	current->clearance = new_clearance;
	return 0;
}

bool is_have_flag(char var, int flag) {
	return (bool)(var & flag);
}

asmlinkage long sys_get_sec(char clr) {
	int bit;
	printk("666 sys_get %d\n", (int)current->clearance);
	if(!is_valid_clearance(clr)) {
		return -EINVAL;
	}
	printk("666 sys_get %d\n", (int)current->clearance);
	bit = get_clearance_bit(clr);
	printk("666 sys_get bit %d\n",bit); 
	return is_have_flag(current->clearance, bit);
}


asmlinkage long sys_check_sec(pid_t pid, char clr) {
	int bit;
	struct task_struct *task;
	if(!is_valid_clearance(clr)) {
		return -EINVAL;
	}
	bit = get_clearance_bit(clr);
	if(!is_have_flag(current->clearance,bit)) {
		return -ESRCH;
	}
	task = find_task_by_vpid(pid);
	if(!task) {
		return -ESRCH;
	}
	return is_have_flag(task->clearance,bit);

}
asmlinkage long sys_set_sec_branch(int height, char clr) {
	struct task_struct *task;
	int count, bit;

	if(height < 0 || !is_valid_clearance(clr))
       		return -EINVAL;

	bit = get_clearance_bit(clr);
	if(!is_have_flag(current->clearance,bit))
        	return -EPERM;

	count = 0;
	task = current->real_parent;

	while(height > 0 && is_idle_task(task)) { // Traverse up the process tree
		if(!is_have_flag(task->clearance,bit)) {
        		task->clearance |= bit; // Set clearance bit
        		count++;
		}	       
        	task = task->real_parent;
		height--;
	}

	return count;
}
