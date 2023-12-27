// #include <linux/init.h>
// #include <linux/module.h>
// #include <linux/kernel.h>
// #include <linux/io.h>
// #include <linux/gpio.h>
// #include <linux/interrupt.h>

#define GPIO_ADDR_BASE                  0x3F200000
#define LED_PIN	                        17
#define BUTTON_PIN	                    27
#define GPIOFSEL1                       1
#define GPIO_SET0                       7
#define GPIO_CLR0                       10
#define GPIO_LVL0                       13
#define GPIO_UD                         37

MODULE_LICENSE("GPL");
MODULE_AUTHOR("phuc");

static u32 irq_line_buttom;
static unsigned int *addr_base;
static struct tasklet_struct my_tasket;


static int get_value_pin(int pin)
{
    u32 val=ioread32(addr_base+13);
    return ((val>>pin)&1);
}
static int get_mode_pin(int pin)
{
    u32 val=ioread32(addr_base);
    return ((val>>((pin%10)*3))&7);
}
static void set_value_pin(int pin,int val)
{
    if(val==0)
    {
        iowrite32(1<<pin,addr_base+10);
    }
    else
    {
        iowrite32(1<<pin,addr_base+7);
    }
}
static void set_mode_pin(int pin,int mode)
{
    u32 val;
    u32 bit=(pin%10)*3;
    if(mode==0)
    {
        val=ioread32(addr_base);
        val&=(~(7<<bit));
        
    }
    else
    {
        val=ioread32(addr_base);
        val=((val&(~(7<<bit)))|(1<<bit));
    }
    iowrite32(val,addr_base);
}
static void set_pull_all_pin(int pull)
{
       iowrite32(pull,addr_base+GPIO_UD);
}
static void my_tasklet_function(unsigned long data)
{
    if(get_mode_pin(LED_PIN)==0)
    {
        printk(KERN_ALERT "error tasklet_function\n");
        return;
    }
    if(get_value_pin(LED_PIN))
    {
        set_value_pin(LED_PIN,1);
    }
    else
    {
        set_value_pin(LED_PIN,0);
    }
}
static irqreturn_t  irq_gpio_handler(int irq, void *dev_id)
{
    tasklet_init(&my_tasket,my_tasklet_function,0);
    tasklet_schedule(&my_tasket);
    return IRQ_HANDLED;

}
static int __init exam_init(void)
{
    int ret;
    addr_base=(unsigned int*)ioremap(GPIO_ADDR_BASE,0x100);
    if(addr_base==NULL)
    {
        printk(KERN_ALERT "Error maping \n");
        return -ENOMEM;
    }
    set_mode_pin(LED_PIN,1);
    set_value_pin(LED_PIN,0);

    set_mode_pin(BUTTON_PIN,0);
    set_pull_all_pin(2);
    irq_line_buttom=gpio_to_irq(BUTTON_PIN);
    if(irq_line_buttom<0)
    {
        printk(KERN_ALERT "Create irq line error!\b");
        return -1;
    }
    ret=request_irq(irq_line_buttom,irq_gpio_handler,IRQF_TRIGGER_FALLING,"buton",NULL);
    if(ret)
    {
        printk(KERN_ALERT "Establish irq error!\n");
    }
    printk(KERN_INFO "Done!\n");

    return ret;
}
static void __exit exam_exit(void)
{
   
    set_value_pin(LED_PIN,0);
    iounmap(addr_base);
    printk(KERN_INFO "End\n");
    free_irq(irq_line_buttom,NULL);
    tasklet_kill(&my_tasket);
    
}
module_init(exam_init);
module_exit(exam_exit)