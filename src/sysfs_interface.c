#include "asic_diag.h"

static ssize_t show_register(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct asic_diag_dev *asic_dev = dev_get_drvdata(dev);
    unsigned long reg_offset;
    u32 value;

    if (kstrtoul(attr->attr.name, 16, &reg_offset))
        return -EINVAL;

    value = ioread32(asic_dev->regs + reg_offset);
    return sprintf(buf, "0x%08x\n", value);
}

static ssize_t store_register(struct device *dev, struct device_attribute *attr,
                            const char *buf, size_t count)
{
    struct asic_diag_dev *asic_dev = dev_get_drvdata(dev);
    unsigned long reg_offset;
    u32 value;

    if (kstrtoul(attr->attr.name, 16, &reg_offset))
        return -EINVAL;

    if (kstrtou32(buf, 16, &value))
        return -EINVAL;

    iowrite32(value, asic_dev->regs + reg_offset);
    return count;
}

/* Create sysfs attributes for each register */
#define REGISTER_ATTR(reg) \
    static DEVICE_ATTR(reg, 0644, show_register, store_register)

REGISTER_ATTR(00);  /* Status Register */
REGISTER_ATTR(04);  /* Control Register */
REGISTER_ATTR(08);  /* Data Register */
REGISTER_ATTR(0c);  /* Interrupt Mask Register */

static struct attribute *asic_diag_attrs[] = {
    &dev_attr_00.attr,
    &dev_attr_04.attr,
    &dev_attr_08.attr,
    &dev_attr_0c.attr,
    NULL,
};

static const struct attribute_group asic_diag_attr_group = {
    .attrs = asic_diag_attrs,
};

int asic_diag_sysfs_init(struct asic_diag_dev *dev)
{
    int ret;

    ret = sysfs_create_group(&dev->dev->kobj, &asic_diag_attr_group);
    if (ret) {
        dev_err(dev->dev, "Failed to create sysfs group\n");
        return ret;
    }

    return 0;
}

void asic_diag_sysfs_exit(struct asic_diag_dev *dev)
{
    sysfs_remove_group(&dev->dev->kobj, &asic_diag_attr_group);
} 