#ifndef ASIC_DIAG_H
#define ASIC_DIAG_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/debugfs.h>
#include <linux/sysfs.h>
#include <linux/kfifo.h>

/* Driver name and version */
#define DRIVER_NAME "asic_diag"
#define DRIVER_VERSION "1.0.0"

/* PCI vendor and device IDs - replace with actual values */
#define ASIC_VENDOR_ID 0x1234
#define ASIC_DEVICE_ID 0x5678

/* Register definitions */
#define ASIC_STATUS_REG    0x00
#define ASIC_CONTROL_REG   0x04
#define ASIC_DATA_REG      0x08
#define ASIC_INT_MASK_REG  0x0C

/* Buffer sizes */
#define EVENT_BUFFER_SIZE 1024
#define MAX_REGISTERS     256

/* Driver private data structure */
struct asic_diag_dev {
    struct pci_dev *pdev;
    void __iomem *regs;
    struct kfifo event_fifo;
    spinlock_t fifo_lock;
    struct dentry *debugfs_dir;
    struct device *dev;
    int irq;
    bool is_open;
};

/* Function declarations */
int asic_diag_probe(struct pci_dev *pdev, const struct pci_device_id *id);
void asic_diag_remove(struct pci_dev *pdev);
irqreturn_t asic_diag_isr(int irq, void *dev_id);
int asic_diag_open(struct inode *inode, struct file *file);
int asic_diag_release(struct inode *inode, struct file *file);
ssize_t asic_diag_read(struct file *file, char __user *buf, size_t count, loff_t *ppos);
ssize_t asic_diag_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos);

/* Sysfs interface functions */
ssize_t show_register(struct device *dev, struct device_attribute *attr, char *buf);
ssize_t store_register(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);

/* Debugfs interface functions */
int asic_diag_debugfs_init(struct asic_diag_dev *dev);
void asic_diag_debugfs_exit(struct asic_diag_dev *dev);

#endif /* ASIC_DIAG_H */ 