IMPLEMENTATION [arm && sunxi_cubieboard]: // ----------------

IMPLEMENT int Uart::irq() const { return 1; }

IMPLEMENTATION: // --------------------------------------------------------

#include "mem_layout.h"
#include "uart_omap35x.h"

IMPLEMENT Address Uart::base() const { return Mem_layout::Uart_phys_base; }

IMPLEMENT L4::Uart *Uart::uart()
{
  static L4::Uart_omap35x uart;
  return &uart;
}
