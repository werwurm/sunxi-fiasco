IMPLEMENTATION [arm && sunxi]: //--------------------------------------

#include "io.h"
#include "kmem.h"

void __attribute__ ((noreturn))
platform_reset(void)
{
  // reset by watchdog timeout
  enum Virt_wdog_map : Address {
    WDOG_CTRL_REG = Mem_layout::Timer_phys_base + 0x94,
  };
  Mword wdog_reg_remapped = Kmem::mmio_remap(WDOG_CTRL_REG);
  Io::write<Mword>(0, wdog_reg_remapped);
  for(volatile unsigned i = 100000; i > 0; --i){}
  Io::write<Mword>(3, wdog_reg_remapped);

  for (;;)
    ;
}
