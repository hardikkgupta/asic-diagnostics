#include "asic_diag.h"

/* Event buffer management */
struct event_entry {
    u32 timestamp;
    u32 event_type;
    u32 data;
};

int asic_diag_open(struct inode *inode, struct file *file)
{
    struct asic_diag_dev *dev = container_of(inode->i_cdev,
                                           struct asic_diag_dev,
                                           cdev);
    if (dev->is_open)
        return -EBUSY;

    dev->is_open = true;
    file->private_data = dev;
    return 0;
}

int asic_diag_release(struct inode *inode, struct file *file)
{
    struct asic_diag_dev *dev = file->private_data;
    dev->is_open = false;
    return 0;
}

ssize_t asic_diag_read(struct file *file, char __user *buf, size_t count,
                       loff_t *ppos)
{
    struct asic_diag_dev *dev = file->private_data;
    struct event_entry entry;
    unsigned long flags;
    int ret;

    if (count < sizeof(entry))
        return -EINVAL;

    spin_lock_irqsave(&dev->fifo_lock, flags);
    ret = kfifo_out(&dev->event_fifo, &entry, sizeof(entry));
    spin_unlock_irqrestore(&dev->fifo_lock, flags);

    if (ret != sizeof(entry))
        return -EAGAIN;

    if (copy_to_user(buf, &entry, sizeof(entry)))
        return -EFAULT;

    return sizeof(entry);
}

ssize_t asic_diag_write(struct file *file, const char __user *buf,
                        size_t count, loff_t *ppos)
{
    struct asic_diag_dev *dev = file->private_data;
    u32 value;

    if (count != sizeof(u32))
        return -EINVAL;

    if (copy_from_user(&value, buf, sizeof(u32)))
        return -EFAULT;

    /* Write to control register to trigger event */
    iowrite32(value, dev->regs + ASIC_CONTROL_REG);
    return sizeof(u32);
}

/* Debugfs interface for event buffer */
static int asic_diag_events_show(struct seq_file *m, void *v)
{
    struct asic_diag_dev *dev = m->private;
    struct event_entry entry;
    unsigned long flags;

    spin_lock_irqsave(&dev->fifo_lock, flags);
    while (kfifo_out(&dev->event_fifo, &entry, sizeof(entry)) == sizeof(entry)) {
        seq_printf(m, "Time: %u, Type: %u, Data: 0x%08x\n",
                  entry.timestamp, entry.event_type, entry.data);
    }
    spin_unlock_irqrestore(&dev->fifo_lock, flags);

    return 0;
}

static int asic_diag_events_open(struct inode *inode, struct file *file)
{
    return single_open(file, asic_diag_events_show, inode->i_private);
}

static const struct file_operations asic_diag_events_fops = {
    .owner = THIS_MODULE,
    .open = asic_diag_events_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = single_release,
};

int asic_diag_debugfs_init(struct asic_diag_dev *dev)
{
    dev->debugfs_dir = debugfs_create_dir(DRIVER_NAME, NULL);
    if (!dev->debugfs_dir)
        return -ENOMEM;

    debugfs_create_file("events", 0444, dev->debugfs_dir, dev,
                       &asic_diag_events_fops);

    return 0;
}

void asic_diag_debugfs_exit(struct asic_diag_dev *dev)
{
    debugfs_remove_recursive(dev->debugfs_dir);
} 