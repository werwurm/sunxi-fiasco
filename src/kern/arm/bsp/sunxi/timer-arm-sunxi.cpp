INTERFACE [sunxi]: // ------------------------------------------------

#include "kmem.h"

class Timer_sunxi_gen
{
public:
  enum {
    TMR_IRQ_EN_REG      = Kmem::Timer_map_base + 0x000,
    TMR_IRQ_STA_REG     = Kmem::Timer_map_base + 0x004,
    TMR0_CTRL_REG       = Kmem::Timer_map_base + 0x010,
    TMR0_INTV_VALUE_REG = Kmem::Timer_map_base + 0x014,
    TMR0_CUR_VALUE_REG  = Kmem::Timer_map_base + 0x018,
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

#include "io.h"

PUBLIC static
void
Timer_sunxi_gen::init()
{
  Mword ctrl_val = 0 << 7 // Continuous mode (not one shot)
		 | 0 << 4 // prescaler set to 1
		 | 1 << 2 // clk src (24Mhz osc)
		 | 1 << 1 // reload
		 | 0 << 0;// disable
  Io::write<Mword>(ctrl_val, TMR0_CTRL_REG);

  // clear interrupt of timer0
  Io::write<Mword>(1, TMR_IRQ_STA_REG);

  // Enable timer0 interrupt
  Io::write<Mword>(1 | Io::read<Mword>(TMR_IRQ_EN_REG), TMR_IRQ_EN_REG);

  Mword val = 24000; // 1ms
  Io::write<Mword>(val, TMR0_INTV_VALUE_REG);

  // Enable timer0
  Io::write<Mword>(ctrl_val | 1, TMR0_CTRL_REG);
}

PUBLIC static inline NEEDS["io.h"]
void
Timer_sunxi_gen::acknowledge()
{
  Io::write<Mword>(1, TMR_IRQ_STA_REG);
}

IMPLEMENT
void Timer::init(unsigned)
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
