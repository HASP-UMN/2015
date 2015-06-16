#ifndef SIMPLEGPIO_H_
#define SIMPLEGPIO_H_

#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define POLL_TIMEOUT (3 * 1000) /* 3 seconds */
#define MAX_BUF 64
#define SYSFS_OMAP_MUX_DIR "/sys/kernel/debug/omap_mux/"

int gpio_get_value(unsigned int gpio);
int gpio_set_value(unsigned int gpio, unsigned int value);

#endif /* SIMPLEGPIO_H_ */