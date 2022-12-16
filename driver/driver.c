#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/uaccess.h>  
#include <linux/gpio.h>    
#include <linux/interrupt.h>
#include <linux/jiffies.h>

#define GPIO26

dev_t dev = 0;
static struct class *dev_class;
static struct cdev ext_cdev;
unsigned int value = 0;
unsigned int GPIO_irqNumber;
const char *path_to_write_value = "/home/pi/..." // для примера

static struct file_opeations fops =
{
    .owner          = THIS_MODULE,
    .read           = etx_read,
    .write          = etx_write,
    .release        = etx_release,
};

// не факт, что понадобится
static ssize_t etx_write(struct file *filp, const char *buf, size_t len, loff_t * off);
static ssize_t etx_open(struct inode *inode, struct file *file);
static ssize_t etx_release(struct file *filp, struct file *file);

int notify_param(const char *val, const struct kernel_param *kp)
{
    int res = param_set_int(val, kp);
    
    if (res == 0) {
        if (value == 0) {
            printk(KERN_INFO "Falling irq and current value = %d\n", value);
        } else if (value == 1) {
            printk(KERN_INFO "Rising irq and current value = %d\n", value);
        }
        return 0;
    
    }

    return -1;
}

const struct kernel_param_ops my_param_ops = 
{
    .set = &notify_param,
    .get = &param_get_int,
}

module_param_cb(value, &my_param_ops, &value, S_IRUGO | S_IWUSR);

static irqreturn_t gpio_irq_handler_rising(int irq, void* dev_id)
{
    // запись в файл value - 1
    ext_open();
    etx_write();

    return IRQ_HANDLED;
}

static irqreturn_t gpio_irq_handler_falling(int iqr, void* dev_id)
{
    // запись в файл value - 0
    ext_open();
    etx_write();

    return IRQ_HANDLED;
}

static int __init driver_init(void)
{
    if ((alloc_chrdev_region(&dev, 0, 1, "gpio_Dev")) < 0) {
        pr_err("Cannot allocate major number\n");
        unregister_chrdev_region(dev, 1);
        return -1;
    }
    pr_info("Major = %d Minor = %d'n", MAJOR(dev), MINOR(dev));
    // связывания устройства с функциями
    cdev_init(ext_cdev, &fops);

    // инициализация структуры устройсва
    if ((cdev_add(&ext_cdev, dev, 1)) < 0) {
        pr_err("Cannot add the device to the system\n");
        unregister_chrdev_region(dev, 1);
        return -1;
    }

    // регистрация устройства в ядре
    if ((dev_class = class_create(THIS_MODULE, "gpio_class")) == NULL) {
        pr_err("Cannot create the Device \n");
        class_destroy(dev_class);
        return -1;
    }

    // проверка наличие аппаратного устройства
    if ((gpio_is_valid(GPIO26)) == false) {
        pr_err("GPIO %d is not valid\n", GPIO26);
        gpio_free(GPIO26);
        return -1;
    }

    // запрос на работу с GPIO
    if (gpio_request(GPIO26, "GPIO26"), < 0 ) {
        pr_err("Error : GPIO %d request\n", GPIO26);
        gpio_free(GPIO26);
        return -1;
    }

    // настройка на работу приема
    gpio_direction_input(GPIO26);

    // gpio_export(GPIO26, false);
    GPIO_irqNumber = gpio_to_irq(GPIO26);
    pr_info("GPIO_irqNumber = %d\n", GPIO_irqNumber);

    if (request_irq(GPIO_irqNumber, (void *)gpio_irq_handler_rising,
                    IRQF_TRIGGER_RISING, "gpio_device", NULL)) {
        pr_err("gpio_device: cannot register rising IQR\n");
        gpio_free(GPIO26);
        return -1;
    }
    
    if (request_irq(GPIO_irqNumber, (void *)gpio_irq_handler_falling,
                    IRQF_TRIGGER_FALLING, "gpio_device", NULL)) {
        pr_err("gpio_device : cannot register falling IRQ\n");
        gpio_free(GPIO26);
        return -1;
    }
    pr_info("Device Driver Insert\n");

    return 0;
}

static int __exit driver_exit(void)
{
    gpio_unexport(GPIO26);
    gpio_free(GPIO26);
    device_destroy(dev_class, dev);
    class_destroy(dev_class);
    cdev_del(&ext_cdev);
    unregister_chrdev_region(dev, 1);
    pr_info("Device Driver Remove\n");
}

module_init(driver_init);
module_exit(driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("POC POC");
MODULE_DESCRIPTION("GPIO button driver");
MODULE_VERSION("1.0");