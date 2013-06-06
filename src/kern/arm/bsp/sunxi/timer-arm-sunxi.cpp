INTERFACE [sunxi]: // ------------------------------------------------

#include "kmem.h"
#include "mmio_register_block.h"

class Timer_sunxi_gen
{
public:
  enum {
    TMR_IRQ_EN_REG      = 0x000,
    TMR_IRQ_STA_REG     = 0x004,
    TMR0_CTRL_REG       = 0x010,
    TMR0_INTV_VALUE_REG = 0x014,
    TMR0_CUR_VALUE_REG  = 0x018,
  };
};

// --------------------------------------------------------------------------
INTERFACE [arm && sunxi]:

EXTENSION class Timer
{
public:
  static unsigned irq() { return 22; }
};

IMPLEMENTATION [sunxi]: // ------------------------------------------------

#include "mmio_register_block.h"

PUBLIC static
void
Timer_sunxi_gen::init()
{
  Mmio_register_block rb(Kmem::mmio_remap(Mem_layout::Timer_phys_base));

  Mword ctrl_val = 0 << 7 // Continuous mode (not one shot)
		 | 0 << 4 // prescaler set to 1
		 | 1 << 2 // clk src (24Mhz osc)
		 | 1 << 1 // reload
		 | 0 << 0;// disable
  rb.write<Mword>(ctrl_val, TMR0_CTRL_REG);

  // clear interrupt of timer0
  rb.write<Mword>(1, TMR_IRQ_STA_REG);

  // Enable timer0 interrupt
  rb.write<Mword>(1 | rb.read<Mword>(TMR_IRQ_EN_REG), TMR_IRQ_EN_REG);

  Mword val = 24000; // 1ms
  rb.write<Mword>(val, TMR0_INTV_VALUE_REG);

  // Enable timer0
  rb.write<Mword>(ctrl_val | 1, TMR0_CTRL_REG);
}

PUBLIC static inline
void
Timer_sunxi_gen::acknowledge()
{
  Mmio_register_block rb(Kmem::mmio_remap(Mem_layout::Timer_phys_base));
  rb.write<Mword>(1, TMR_IRQ_STA_REG);
}

IMPLEMENT
void Timer::init(Cpu_number)
{
  Timer_sunxi_gen::init();
}

PUBLIC static inline NEEDS[Timer_sunxi_gen::acknowledge]
void Timer::acknowledge()
{
  Timer_sunxi_gen::acknowledge();
}

// -----------------------------------------------------------------------
IMPLEMENTATION [arm && sunxi]:

#include "config.h"
#include "kip.h"

static inline
Unsigned64
Timer::timer_to_us(Unsigned32 /*cr*/)
{ return 0; }

static inline
Unsigned64
Timer::us_to_timer(Unsigned64 us)
{ (void)us; return 0; }

IMPLEMENT inline
void
Timer::update_one_shot(Unsigned64 wakeup)
{
  (void)wakeup;
}

IMPLEMENT inline NEEDS["config.h", "kip.h"]
Unsigned64
Timer::system_clock()
{
  if (Config::Scheduler_one_shot)
    return 0;
  else
    return Kip::k()->clock;
}
