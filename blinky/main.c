int main(){
    return 0;
}

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