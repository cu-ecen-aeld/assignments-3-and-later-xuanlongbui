/**
 * @file aesdchar.c
 * @brief Functions and data related to the AESD char driver implementation
 *
 * Based on the implementation of the "scull" device driver, found in
 * Linux Device Drivers example code.
 *
 * @author Dan Walkes
 * @date 2019-10-22
 * @copyright Copyright (c) 2019
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/fs.h> // file_operations
#include <linux/slab.h>
#include "aesdchar.h"
int aesd_major =   0; // use dynamic major
int aesd_minor =   0;

MODULE_AUTHOR("xuanlongbui"); /** TODO: fill in your name **/
MODULE_LICENSE("Dual BSD/GPL");

struct aesd_dev aesd_device;

int aesd_open(struct inode *inode, struct file *filp)
{
    struct aesd_dev *dev;
    PDEBUG("open");
    /**
     * TODO: handle open
     */
    dev = container_of(inode->i_cdev, struct aesd_dev, cdev);
    filp->private_data = dev;
    return 0;
}

int aesd_release(struct inode *inode, struct file *filp)
{
    PDEBUG("release");
    /**
     * TODO: handle release
     */
    return 0;
}

ssize_t aesd_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
    ssize_t retval = 0;
    /**
     * TODO: handle read
     */    
    struct aesd_dev *dev = filp->private_data;
    struct aesd_buffer_entry *entry = NULL;
    size_t entry_offset = 0;
    struct aesd_buffer_entry *data_ptr;
    size_t bytes_available;
    size_t bytes_to_read;
    PDEBUG("read %zu bytes with offset %lld", count, *f_pos);

    if (mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;

    data_ptr = aesd_circular_buffer_find_entry_offset_for_fpos(
        &dev->buffer, *f_pos, &entry_offset);

    if (!data_ptr->buffptr) {
        retval = 0;  // EOF
        goto out;
    }

    // Số byte còn lại trong entry kể từ vị trí offset
    bytes_available = entry->size - entry_offset;
    bytes_to_read = min(bytes_available, count);

    if (copy_to_user(buf, data_ptr->buffptr + entry_offset, bytes_to_read)) {
        retval = -EFAULT;
        goto out;
    }

    *f_pos += bytes_to_read;
    retval = bytes_to_read;

out:
    mutex_unlock(&dev->lock);
    return retval;
}

ssize_t aesd_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
    struct aesd_dev *dev = filp->private_data;
    ssize_t retval = -ENOMEM;
    char *kbuf = NULL;
    bool complete ;
    PDEBUG("write %zu bytes with offset %lld", count, *f_pos);

    if (mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;

    kbuf = kmalloc(count, GFP_KERNEL);
    if (!kbuf)
        goto out;

    if (copy_from_user(kbuf, buf, count)) {
        retval = -EFAULT;
        goto free_kbuf;
    }

    // Giả sử một entry là một dòng kết thúc bằng '\n'
    complete = memchr(kbuf, '\n', count) != NULL;

    // Tăng kích thước write_buffer và copy data
    dev->write_buffer = krealloc(dev->write_buffer, dev->write_buffer_size + count, GFP_KERNEL);
    if (!dev->write_buffer) {
        retval = -ENOMEM;
        goto free_kbuf;
    }

    memcpy(dev->write_buffer + dev->write_buffer_size, kbuf, count);
    dev->write_buffer_size += count;

    if (complete) {
        struct aesd_buffer_entry new_entry;

        new_entry.buffptr = dev->write_buffer;
        new_entry.size = dev->write_buffer_size;

        aesd_circular_buffer_add_entry(&dev->buffer, &new_entry);

        dev->write_buffer = NULL;
        dev->write_buffer_size = 0;
    }

    retval = count;

free_kbuf:
    kfree(kbuf);
out:
    mutex_unlock(&dev->lock);
    return retval;
}
struct file_operations aesd_fops = {
    .owner =    THIS_MODULE,
    .read =     aesd_read,
    .write =    aesd_write,
    .open =     aesd_open,
    .release =  aesd_release,
};

static int aesd_setup_cdev(struct aesd_dev *dev)
{
    int err, devno = MKDEV(aesd_major, aesd_minor);

    cdev_init(&dev->cdev, &aesd_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &aesd_fops;
    err = cdev_add (&dev->cdev, devno, 1);
    if (err) {
        printk(KERN_ERR "Error %d adding aesd cdev", err);
    }
    return err;
}



int aesd_init_module(void)
{
    dev_t dev = 0;
    int result;
    result = alloc_chrdev_region(&dev, aesd_minor, 1,
            "aesdchar");
    aesd_major = MAJOR(dev);
    if (result < 0) {
        printk(KERN_WARNING "Can't get major %d\n", aesd_major);
        return result;
    }
    memset(&aesd_device,0,sizeof(struct aesd_dev));

    /**
     * TODO: initialize the AESD specific portion of the device
     */
    aesd_circular_buffer_init(&aesd_device.buffer);
    mutex_init(&aesd_device.lock);
    aesd_device.write_buffer = NULL;
    aesd_device.write_buffer_size = 0;
    
    result = aesd_setup_cdev(&aesd_device);

    if( result ) {
        unregister_chrdev_region(dev, 1);
    }
    return result;

}

void aesd_cleanup_module(void)
{
    dev_t devno = MKDEV(aesd_major, aesd_minor);
    int i;

    cdev_del(&aesd_device.cdev);

    /**
     * TODO: cleanup AESD specific poritions here as necessary
     */
    if (aesd_device.write_buffer)
        kfree(aesd_device.write_buffer);
    for (i = 0; i < AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED; i++) {
        if (aesd_device.buffer.entry[i].buffptr != NULL) {
            kfree(aesd_device.buffer.entry[i].buffptr);
        }
    }

    unregister_chrdev_region(devno, 1);
}



module_init(aesd_init_module);
module_exit(aesd_cleanup_module);
