#include <inttypes.h>
#include <stdbool.h>

/*
  Helper Macros 
*/

#define BIT(x) (1UL << (x))
// Saves the information about which GPIO bank (higher 8 bit) and pin number (lower 8 bit) together in one 16 bit number
#define PIN(bank, num) ((((bank) - 'A') << 8) | (num))
#define PINNO(pin) (pin & 255)
#define PINBANK(pin) (pin >> 8)

// Defines a GPIO peripheral with its fields
struct gpio {
  volatile uint32_t MODER, OTYPER, OSPEEDR, PUPDR, IDR, ODR, BSRR, LCKR, AFR[2];
};

// Defines a GPIO bank address with its offset from base banke mem address
#define GPIO(bank)((struct gpio *) (0x40020000 + 0x400 * (bank)))

// Possible pin modes as per datasheet: 00, 01, 10, 11
enum {GPIO_MODE_INPUT, GPIO_MODE_OUTPUT, GPIO_MODE_AF, GPIO_MODE_ANALOG};

// Reset and clock control register
struct rcc {
  volatile uint32_t CR, PLLCFGR, CFGR, CIR, AHB1RSTR, AHB2RSTR, AHB3RSTR,
      RESERVED0, APB1RSTR, APB2RSTR, RESERVED1[2], AHB1ENR, AHB2ENR, AHB3ENR,
      RESERVED2, APB1ENR, APB2ENR, RESERVED3[2], AHB1LPENR, AHB2LPENR,
      AHB3LPENR, RESERVED4, APB1LPENR, APB2LPENR, RESERVED5[2], BDCR, CSR,
      RESERVED6[2], SSCGR, PLLI2SCFGR;
};
#define RCC ((struct rcc *) 0x40023800)

/*
  GPIO API
*/

// Sets an GPIO bank to specified output mode
static inline void gpio_set_mode(uint16_t pin /* as in PIN(bank, num) pair*/, uint8_t mode){
  struct gpio *gpio = GPIO(PINBANK(pin)); // GPIO bank mem address
  int n = PINNO(pin);                     // pin number
  gpio->MODER &= ~(3U << (n * 2));        // clear mode
  gpio->MODER |= (mode & 3) << (n * 2);   // set new mode
}

// Sets the voltage to HIGH or LOW dependent on val of specifed PIN
static inline void gpio_write(uint16_t pin, bool val){
  struct gpio *gpio = GPIO(PINBANK(pin));
  gpio->BSRR = (1U << PINNO(pin)) << (val ? 0 : 16);
}

/*
  Helper Functions
*/

// Executes a NOP specified amount of times
static inline void spin(volatile uint32_t count) {
  while (count--) (void) 0;
}

/*
  Main Function
*/

int main(){
  uint16_t led_green = PIN('B', 0);                     // green user led
  uint16_t led_blue = PIN('B', 7);                      // blue user led
  uint16_t led_red = PIN('B', 14);                      // red user led
  RCC->AHB1ENR |= BIT((PINBANK(led_blue)));             // enables the needed GPIO bank clock for led
  RCC->AHB1ENR |= BIT((PINBANK(led_red)));              // enables the needed GPIO bank clock for led
  RCC->AHB1ENR |= BIT((PINBANK(led_green)));            // enables the needed GPIO bank clock for led
  gpio_set_mode(led_blue, GPIO_MODE_OUTPUT);            // sets blue user led to output mode
  gpio_set_mode(led_red, GPIO_MODE_OUTPUT);             // sets blue user led to output mode
  gpio_set_mode(led_green, GPIO_MODE_OUTPUT);           // sets blue user led to output mode

  // Toggles leds 
  for (;;) {
    gpio_write(led_blue, true);
    gpio_write(led_green, false);
    spin(399999);
    gpio_write(led_blue, false);
    gpio_write(led_red, true);
    spin(399999);
    gpio_write(led_red, false);
    gpio_write(led_green, true);
    spin(399999);
  }

  return 0;
}

/*
  Startup Code
*/

// Startup code: copies .data section from FLASH to RAM
__attribute__((naked, noreturn)) void _reset(void) {
  extern long _sbss, _ebss, _sdata, _edata, _sidata;
  for (long *dst = &_sbss; dst < &_ebss; dst++) *dst = 0;
  for (long *dst = &_sdata, *src = &_sidata; dst < &_edata;) *dst++ = *src++;

  main();             
  for (;;) (void) 0;  // Infinite loop in the case if main() returns
}

extern void _estack(void);  // Defined in link.ld, stack pointer

// Vector Table: 16 standard and 110 STM32-specific handlers
__attribute__((section(".vectors"))) void (*const tab[16 + 110])(void) = {
  _estack, _reset
};