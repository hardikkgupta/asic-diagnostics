#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/random.h>

#define TEST_ITERATIONS 1000
#define MAX_EVENTS 100

static struct dentry *test_dir;
static struct file *reg_file;
static struct file *events_file;

static int test_register_access(void)
{
    char buf[32];
    int ret;
    u32 value;

    /* Test reading registers */
    for (int i = 0; i < 4; i++) {
        snprintf(buf, sizeof(buf), "/sys/bus/pci/devices/*/asic_diag/%02x", i * 4);
        reg_file = filp_open(buf, O_RDWR, 0);
        if (IS_ERR(reg_file)) {
            pr_err("Failed to open register file %s\n", buf);
            return PTR_ERR(reg_file);
        }

        ret = kernel_read(reg_file, buf, sizeof(buf), &reg_file->f_pos);
        if (ret < 0) {
            pr_err("Failed to read register\n");
            filp_close(reg_file, NULL);
            return ret;
        }

        filp_close(reg_file, NULL);
    }

    /* Test writing registers */
    for (int i = 0; i < TEST_ITERATIONS; i++) {
        get_random_bytes(&value, sizeof(value));
        snprintf(buf, sizeof(buf), "0x%08x", value);
        
        reg_file = filp_open("/sys/bus/pci/devices/*/asic_diag/04", O_RDWR, 0);
        if (IS_ERR(reg_file)) {
            pr_err("Failed to open control register\n");
            return PTR_ERR(reg_file);
        }

        ret = kernel_write(reg_file, buf, strlen(buf), &reg_file->f_pos);
        if (ret < 0) {
            pr_err("Failed to write register\n");
            filp_close(reg_file, NULL);
            return ret;
        }

        filp_close(reg_file, NULL);
        usleep_range(1000, 2000);  /* 1-2ms delay */
    }

    return 0;
}

static int test_event_capture(void)
{
    char buf[1024];
    int ret;
    int events_captured = 0;

    events_file = filp_open("/sys/kernel/debug/asic_diag/events", O_RDONLY, 0);
    if (IS_ERR(events_file)) {
        pr_err("Failed to open events file\n");
        return PTR_ERR(events_file);
    }

    /* Trigger events and verify capture */
    for (int i = 0; i < MAX_EVENTS; i++) {
        ret = kernel_read(events_file, buf, sizeof(buf), &events_file->f_pos);
        if (ret > 0) {
            events_captured++;
        }
        usleep_range(1000, 2000);
    }

    filp_close(events_file, NULL);

    if (events_captured == 0) {
        pr_err("No events captured\n");
        return -EIO;
    }

    return 0;
}

static int __init test_init(void)
{
    int ret;

    pr_info("Starting ASIC Diagnostics Driver Tests\n");

    /* Create test directory in debugfs */
    test_dir = debugfs_create_dir("asic_diag_test", NULL);
    if (!test_dir) {
        pr_err("Failed to create test directory\n");
        return -ENOMEM;
    }

    /* Run register access tests */
    ret = test_register_access();
    if (ret) {
        pr_err("Register access test failed\n");
        goto cleanup;
    }

    /* Run event capture tests */
    ret = test_event_capture();
    if (ret) {
        pr_err("Event capture test failed\n");
        goto cleanup;
    }

    pr_info("All tests passed successfully\n");
    return 0;

cleanup:
    debugfs_remove_recursive(test_dir);
    return ret;
}

static void __exit test_exit(void)
{
    debugfs_remove_recursive(test_dir);
    pr_info("ASIC Diagnostics Driver Tests completed\n");
}

module_init(test_init);
module_exit(test_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("ASIC Diagnostics Driver Test Module"); 