
// EXAMPLE 1

// #include <linux/kernel.h>
// #include <linux/init.h>
// #include <linux/module.h>
// #include <linux/kdev_t.h>
// #include <linux/fs.h>
// #include <linux/cdev.h>
// #include <linux/device.h>
// #include <linux/delay.h>
// #include <linux/uaccess.h>  
// #include <linux/gpio.h>     
 
// #define GPIO_24 (24)
 
// dev_t dev = 0;
// static struct class *dev_class;
// static struct cdev gpio_cdev;
 
// static int __init gpio_driver_init(void);
// static void __exit gpio_driver_exit(void);
 
// static int gpio_open(struct inode *inode, struct file *file);
// static int gpio_release(struct inode *inode, struct file *file);
// static ssize_t gpio_read(struct file *filp, 
//                 char __user *buf, size_t len,loff_t * off);
// static ssize_t gpio_write(struct file *filp, 
//                 const char *buf, size_t len, loff_t * off);
 
// static struct file_operations fops =
// {
//   .owner          = THIS_MODULE,
//   .read           = gpio_read,
//   .write          = gpio_write,
//   .open           = gpio_open,
//   .release        = gpio_release,
// };
 
// static int gpio_open(struct inode *inode, struct file *file)
// {
//   pr_info("Device File Opened...!!!\n");
//   return 0;
// }
 
// static int gpio_release(struct inode *inode, struct file *file)
// {
//   pr_info("Device File Closed...!!!\n");
//   return 0;
// }

// static ssize_t gpio_read(struct file *filp, 
//                 char __user *buf, size_t len, loff_t *off)
// {
//   uint8_t gpio_state = 0;
 
//   gpio_state = gpio_get_value(GPIO_24);
 
//   len = 1;
//   if( copy_to_user(buf, &gpio_state, len) > 0) {
//     pr_err("ERROR: Not all the bytes have been copied to user\n");
//   }
 
//   pr_info("Read function : GPIO_24 = %d \n", gpio_state);
 
//   return 0;
// }
 
// static ssize_t gpio_write(struct file *filp, 
//                 const char __user *buf, size_t len, loff_t *off)
// {
//   uint8_t rec_buf[10] = {0};
 
//   if( copy_from_user( rec_buf, buf, len ) > 0) {
//     pr_err("ERROR: Not all the bytes have been copied from user\n");
//   }
 
//   pr_info("Write Function : GPIO_24 Set = %c\n", rec_buf[0]);
 
//   if (rec_buf[0]=='1') {
//     gpio_set_value(GPIO_24, 1);
//   } else if (rec_buf[0]=='0') {
//     gpio_set_value(GPIO_24, 0);
//   } else {
//     pr_err("Unknown command : Please provide either 1 or 0 \n");
//   }
 
//   return len;
// }
 
// static int __init gpio_driver_init(void)
// {
//  pr_info("Allocating Major number\n"); 
//  // выделение номера устройство динамически
//  if((alloc_chrdev_region(&dev, 0, 1, "gpio_Dev")) <0){
//     pr_err("Cannot allocate major number\n");
//     // освобождение занятого номера
//     unregister_chrdev_region(dev,1);
//   }
//   pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));
 
//  // связывания устройство с написаними функциями
//   cdev_init(&gpio_cdev,&fops);
 
//   // инициализация структуры устройства
//   if((cdev_add(&gpio_cdev,dev,1)) < 0){
//     pr_err("Cannot add the device to the system\n");
//     unregister_chrdev_region(dev,1);
//     return -1;
//   }

//   // регистрация устройства в ядре
//   if((dev_class = class_create(THIS_MODULE,"gpio_class")) == NULL){
//     pr_err("Cannot create the struct class\n");
//     // уничтожение класса
//     class_destroy(dev_class);
//     return -1;
//   }
//   // добавление к созданому классу устройство
//   if((device_create(dev_class,NULL,dev,NULL,"gpio_device")) == NULL){
//     pr_err( "Cannot create the Device \n");
//     // удаление устройства
//     device_destroy(dev_class,dev);
//     return -1;
//   }
 
//   // проверка наличие аппаратного устройства
//   if(gpio_is_valid(GPIO_24) == false){
//     pr_err("GPIO %d is not valid\n", GPIO_24);
//        device_destroy(dev_class,dev);
//     return -1;
//   }

//   // запрос на работу с GPIO
//   if(gpio_request(GPIO_24,"GPIO_24") < 0){
//     pr_err("ERROR: GPIO %d request\n", GPIO_24);
//     gpio_free(GPIO_24);
//     return -1;
//   }
 
//   // настройка устройства на работу вывода
//   gpio_direction_output(GPIO_24, 0);
  
//   // экспортирование устройства + разрешение пользователю изменение направление - ввод и вывод
//   gpio_export(GPIO_24, false);
 
//   pr_info("Device Driver Insert\n");
//   return 0;
// }


// static void __exit gpio_driver_exit(void)
// {
//   gpio_unexport(GPIO_24);
//   gpio_free(GPIO_24);
//   device_destroy(dev_class,dev);
//   class_destroy(dev_class);
//   cdev_del(&gpio_cdev);
//   unregister_chrdev_region(dev, 1);
//   pr_info("Device Driver Remove\n");
// }
 
// module_init(gpio_driver_init);
// module_exit(gpio_driver_exit);
 
// MODULE_LICENSE("GPL");
// MODULE_AUTHOR("Ivan Ivanov");
// MODULE_DESCRIPTION("A simple GPIO Driver");
// MODULE_VERSION("1.0");


// EXAMPLE 2

// uint8_t kernel_buffer;
// uint8_t button;
// #define mem_size     1024 
// #define GPIO_23_IN  (23) //button is connected to this GPIO
// uint8_t led_toggle = 0; 
 
// extern unsigned long volatile jiffies;
// unsigned long old_jiffie = 0;
 
// unsigned int GPIO_irqNumber; 
 
// static irqreturn_t gpio_irq_handler(int irq,void *dev_id)

// {
//   static unsigned long flags = 0;
//    unsigned long diff = jiffies - old_jiffie;
//    if (diff < 20)
//    {
//      return IRQ_HANDLED;
//    }
//   old_jiffie = jiffies;
 
//   local_irq_save(flags);
//   led_toggle = (0x01 ^ led_toggle);                             
//   kernel_buffer = led_toggle;
//   local_irq_restore(flags);
//   pr_info("Interrupt Occurred : GPIO_23_IN : %d ",gpio_get_value(GPIO_23_IN));
//   return IRQ_HANDLED;
// }
 
// dev_t dev = 0;
// static struct class *dev_class;
// static struct cdev etx_cdev;
 
// static int __init etx_driver_init(void);
// static void __exit etx_driver_exit(void);
 
 
// static int etx_open(struct inode *inode, struct file *file);
// static int etx_release(struct inode *inode, struct file *file);
// static ssize_t etx_read(struct file *filp, 
//                 char __user *buf, size_t len,loff_t * off);
// static ssize_t etx_write(struct file *filp, 
//                 const char *buf, size_t len, loff_t * off);
 
// static struct file_operations fops =
// {
//   .owner          = THIS_MODULE,
//   .read           = etx_read,
//   .write          = etx_write,
//   .open           = etx_open,
//   .release        = etx_release,
// };
 
// static int etx_open(struct inode *inode, struct file *file)
// {
//       pr_info("Device File Opened...!!!\n");
//       return 0;
// }
 
// static int etx_release(struct inode *inode, struct file *file)
// {
//   pr_info("Device File Closed...!!!\n");
//   return 0;
// }
 
// static ssize_t etx_read(struct file *filp, 
//                             char __user *buf, size_t len, loff_t *off)
// {
//   uint8_t gpio_state = 0;
//   gpio_state = gpio_get_value(GPIO_23_IN);
//   pr_info("Read function : GPIO_23 = %d \n", gpio_state);
 
//   return 0;
 
// }
 
// static ssize_t etx_write(struct file *filp, 
//                 const char __user *buf, size_t len, loff_t *off)
// {
//   pr_info("You can't write to button\n");
//   return 0;
// }
 
// static int __init etx_driver_init(void)

// {
//   if((alloc_chrdev_region(&dev, 0, 1, "gpio_Dev")) <0){
//     pr_err("Cannot allocate major number\n");
//     unregister_chrdev_region(dev,1);
//     return -1;
//   }
//   pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));
 
//   cdev_init(&etx_cdev,&fops);
 
//   if((cdev_add(&etx_cdev,dev,1)) < 0){
//     pr_err("Cannot add the device to the system\n");
//     cdev_del(&etx_cdev);
//     return -1;
//   }
 
//   if((dev_class = class_create(THIS_MODULE,"gpio_class")) == NULL){
//     pr_err("Cannot create the struct class\n");
//     class_destroy(dev_class);
//     return -1;
//   }
 
//   if((device_create(dev_class,NULL,dev,NULL,"gpio_device")) == NULL){
//     pr_err( "Cannot create the Device \n");
//     device_destroy(dev_class,dev);
//     return -1;
//   }
 
//   if(gpio_is_valid(GPIO_23_IN) == false){
//     pr_err("GPIO %d is not valid\n", GPIO_23_IN);
//     gpio_free(GPIO_23_IN);
//     return -1;
//   }
 
//   if(gpio_request(GPIO_23_IN,"GPIO_23_IN") < 0){
//     pr_err("ERROR: GPIO %d request\n", GPIO_23_IN);
//     gpio_free(GPIO_23_IN);
//     return -1;
//   }
 
//   gpio_direction_input(GPIO_23_IN);
    // функция связавания GPIO_23_IN с прерыванием
//   GPIO_irqNumber = gpio_to_irq(GPIO_23_IN);  //Get the IRQ number for our GPIO
//   pr_info("GPIO_irqNumber = %d\n", GPIO_irqNumber);
    // функция связанвания функции обработки прерывания с GPIO
//   if (request_irq(GPIO_irqNumber,             //IRQ number
//                   (void *)gpio_irq_handler,   //IRQ handler
//                   IRQF_TRIGGER_RISING,        //Handler will be called in raising edge
//                   " gpio_device",               //used to identify the device name using this IRQ
//                   NULL)) {                    //device id for shared IRQ
//     pr_err("gpio_device: cannot register IRQ ");
//     gpio_free(GPIO_23_IN);
//     return -1;
//   }
 
//   pr_info("Device Driver Insert...Done!!!\n");
//   return 0;
// }
 
// static void __exit etx_driver_exit(void)
// {
//   free_irq(GPIO_irqNumber,NULL);
//   gpio_free(GPIO_23_IN);
//   device_destroy(dev_class,dev);
//   class_destroy(dev_class);
//   cdev_del(&etx_cdev);
//   unregister_chrdev_region(dev, 1);
//   pr_info("Device Driver Remove...Done!!\n");
// }
 
// module_init(etx_driver_init);
// module_exit(etx_driver_exit);