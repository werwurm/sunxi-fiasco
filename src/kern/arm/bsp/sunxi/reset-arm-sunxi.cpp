IMPLEMENTATION [arm && sunxi]: //--------------------------------------

#include "io.h"
#include "kmem.h"

void __attribute__ ((noreturn))
platform_reset(void)
{
  // reset by watchdog timeout
  enum Virt_wdog_map : Address {
    WDOG_CTRL_REG = Kmem::Timer_map_base + 0x94,
  };
  Io::write<Mword>(0, WDOG_CTRL_REG);
  for(volatile unsigned i = 100000; i > 0; --i){}
  Io::write<Mword>(3, WDOG_CTRL_REG);

  for (;;)
    ;
}
