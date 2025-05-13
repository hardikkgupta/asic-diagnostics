#include "asic_diag.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("ASIC Diagnostics Driver");
MODULE_VERSION(DRIVER_VERSION);

static struct pci_device_id asic_diag_pci_tbl[] = {
    { PCI_DEVICE(ASIC_VENDOR_ID, ASIC_DEVICE_ID) },
    { 0, }
};
MODULE_DEVICE_TABLE(pci, asic_diag_pci_tbl);

static struct file_operations asic_diag_fops = {
    .owner = THIS_MODULE,
    .open = asic_diag_open,
    .release = asic_diag_release,
    .read = asic_diag_read,
    .write = asic_diag_write,
};

static struct pci_driver asic_diag_driver = {
    .name = DRIVER_NAME,
    .id_table = asic_diag_pci_tbl,
    .probe = asic_diag_probe,
    .remove = asic_diag_remove,
};

int asic_diag_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
    struct asic_diag_dev *dev;
    int ret;

    dev = devm_kzalloc(&pdev->dev, sizeof(*dev), GFP_KERNEL);
    if (!dev)
        return -ENOMEM;

    dev->pdev = pdev;
    pci_set_drvdata(pdev, dev);

    ret = pci_enable_device(pdev);
    if (ret) {
        dev_err(&pdev->dev, "Failed to enable PCI device\n");
        return ret;
    }

    ret = pci_request_regions(pdev, DRIVER_NAME);
    if (ret) {
        dev_err(&pdev->dev, "Failed to request PCI regions\n");
        goto err_disable;
    }

    dev->regs = pci_iomap(pdev, 0, 0);
    if (!dev->regs) {
        dev_err(&pdev->dev, "Failed to map PCI registers\n");
        ret = -ENOMEM;
        goto err_release;
    }

    ret = kfifo_alloc(&dev->event_fifo, EVENT_BUFFER_SIZE, GFP_KERNEL);
    if (ret) {
        dev_err(&pdev->dev, "Failed to allocate event buffer\n");
        goto err_unmap;
    }

    spin_lock_init(&dev->fifo_lock);

    dev->irq = pdev->irq;
    ret = request_irq(dev->irq, asic_diag_isr, IRQF_SHARED,
                     DRIVER_NAME, dev);
    if (ret) {
        dev_err(&pdev->dev, "Failed to request IRQ\n");
        goto err_fifo;
    }

    ret = asic_diag_debugfs_init(dev);
    if (ret) {
        dev_err(&pdev->dev, "Failed to initialize debugfs\n");
        goto err_irq;
    }

    dev_info(&pdev->dev, "ASIC Diagnostics driver loaded\n");
    return 0;

err_irq:
    free_irq(dev->irq, dev);
err_fifo:
    kfifo_free(&dev->event_fifo);
err_unmap:
    pci_iounmap(pdev, dev->regs);
err_release:
    pci_release_regions(pdev);
err_disable:
    pci_disable_device(pdev);
    return ret;
}

void asic_diag_remove(struct pci_dev *pdev)
{
    struct asic_diag_dev *dev = pci_get_drvdata(pdev);

    asic_diag_debugfs_exit(dev);
    free_irq(dev->irq, dev);
    kfifo_free(&dev->event_fifo);
    pci_iounmap(pdev, dev->regs);
    pci_release_regions(pdev);
    pci_disable_device(pdev);
    dev_info(&pdev->dev, "ASIC Diagnostics driver unloaded\n");
}

irqreturn_t asic_diag_isr(int irq, void *dev_id)
{
    struct asic_diag_dev *dev = dev_id;
    u32 status;
    unsigned long flags;

    status = ioread32(dev->regs + ASIC_STATUS_REG);
    if (!(status & 0x1))
        return IRQ_NONE;

    spin_lock_irqsave(&dev->fifo_lock, flags);
    kfifo_in(&dev->event_fifo, &status, sizeof(status));
    spin_unlock_irqrestore(&dev->fifo_lock, flags);

    /* Clear interrupt */
    iowrite32(0x1, dev->regs + ASIC_STATUS_REG);

    return IRQ_HANDLED;
}

static int __init asic_diag_init(void)
{
    return pci_register_driver(&asic_diag_driver);
}

static void __exit asic_diag_exit(void)
{
    pci_unregister_driver(&asic_diag_driver);
}

module_init(asic_diag_init);
module_exit(asic_diag_exit); 