INTERFACE [arm && sunxi]:

#include "kmem.h"

class Irq_base;

EXTENSION class Pic
{
public:
  enum
  {
    INTC_PROT_EN             = Kmem::Intc_map_base + 0x008,
    NMI_INT_CTRL_REG         = Kmem::Intc_map_base + 0x00c,
    INTC_IRQ_PEND_REGn_base  = Kmem::Intc_map_base + 0x010,
    INTC_FIQ_PEND_REGn_base  = Kmem::Intc_map_base + 0x020,
    INTC_IRQ_TYPE_SELn_base  = Kmem::Intc_map_base + 0x030,
    INTC_EN_REGn_base        = Kmem::Intc_map_base + 0x040,
    INTC_MASK_REGn_base      = Kmem::Intc_map_base + 0x050,
    INTC_RESP_REGn_base      = Kmem::Intc_map_base + 0x060,
    INTC_FF_REGn_base        = Kmem::Intc_map_base + 0x070,
    INTC_PRIO_REGn_base      = Kmem::Intc_map_base + 0x080,
  };
};

INTERFACE [arm && sunxi]: //-------------------------------------------

EXTENSION class Pic
{
public:
  enum { Num_irqs                 = 96, };
};

//-------------------------------------------------------------------------
IMPLEMENTATION [arm && sunxi]:

#include "config.h"
#include "io.h"
#include "irq_chip_generic.h"
#include "irq_mgr.h"

class Irq_chip_arm_sunxi : public Irq_chip_gen
{
public:
  Irq_chip_arm_sunxi() : Irq_chip_gen(Pic::Num_irqs) {}
  unsigned set_mode(Mword, unsigned) { return Irq_base::Trigger_level; }
  void set_cpu(Mword, unsigned) {}
};

PRIVATE inline
void
Irq_chip_arm_sunxi::setBit(const Mword base, const Mword irq){
  Io::write<Mword>(
	(1 << (irq & 0x1f)) | Io::read<Mword>(base + ((irq & 0xe0) >> 3)),
	base + ((irq & 0xe0) >> 3));
}

PRIVATE inline
void
Irq_chip_arm_sunxi::unsetBit(const Mword base, const Mword irq){
  Io::write<Mword>(
	~(1 << (irq & 0x1f)) & Io::read<Mword>(base + ((irq & 0xe0) >> 3)),
	base + ((irq & 0xe0) >> 3));
}

PUBLIC
void
Irq_chip_arm_sunxi::mask(Mword irq)
{
  assert(cpu_lock.test());
  setBit(Pic::INTC_MASK_REGn_base, irq);
  unsetBit(Pic::INTC_EN_REGn_base, irq);
}

PUBLIC
void
Irq_chip_arm_sunxi::mask_and_ack(Mword irq)
{
  assert(cpu_lock.test());
  unsetBit(Pic::INTC_EN_REGn_base, irq);
  setBit(Pic::INTC_MASK_REGn_base, irq);
  Io::write<Mword>(1 << (irq & 0x1f), Pic::INTC_IRQ_PEND_REGn_base + ((irq & 0xe0) >> 3));
}

PUBLIC
void
Irq_chip_arm_sunxi::ack(Mword irq)
{
  Io::write<Mword>(1 << (irq & 0x1f), Pic::INTC_IRQ_PEND_REGn_base + ((irq & 0xe0) >> 3));
}

PUBLIC
void
Irq_chip_arm_sunxi::unmask(Mword irq)
{
  assert(cpu_lock.test());
  setBit(Pic::INTC_EN_REGn_base, irq);
  unsetBit(Pic::INTC_MASK_REGn_base, irq);
}

static Static_object<Irq_mgr_single_chip<Irq_chip_arm_sunxi > > mgr;

IMPLEMENT FIASCO_INIT
void Pic::init()
{

  // Disable all interrupts
  Io::write<Mword>(0x0, INTC_EN_REGn_base);
  Io::write<Mword>(0x0, INTC_EN_REGn_base + 0x04);
  Io::write<Mword>(0x0, INTC_EN_REGn_base + 0x08);

  // Mask all interrupts
  Io::write<Mword>(0xffffffff, INTC_MASK_REGn_base);
  Io::write<Mword>(0xffffffff, INTC_MASK_REGn_base + 0x04);
  Io::write<Mword>(0xffffffff, INTC_MASK_REGn_base + 0x08);

  // clear all pending
  Io::write<Mword>(0xffffffff, INTC_IRQ_PEND_REGn_base);
  Io::write<Mword>(0xffffffff, INTC_IRQ_PEND_REGn_base + 0x04);
  Io::write<Mword>(0xffffffff, INTC_IRQ_PEND_REGn_base + 0x08);
  Io::write<Mword>(0xffffffff, INTC_FIQ_PEND_REGn_base);
  Io::write<Mword>(0xffffffff, INTC_FIQ_PEND_REGn_base + 0x04);
  Io::write<Mword>(0xffffffff, INTC_FIQ_PEND_REGn_base + 0x08);

  // Set all irqs to IRQ (not FIQ)
  Io::write<Mword>(0, INTC_IRQ_TYPE_SELn_base);
  Io::write<Mword>(0, INTC_IRQ_TYPE_SELn_base + 4);
  Io::write<Mword>(0, INTC_IRQ_TYPE_SELn_base + 8);

  // Set all priorities to 0 (0: lowest - 3: highest)
  Io::write<Mword>(0, INTC_PRIO_REGn_base);
  Io::write<Mword>(0, INTC_PRIO_REGn_base + 0x04);
  Io::write<Mword>(0, INTC_PRIO_REGn_base + 0x08);
  Io::write<Mword>(0, INTC_PRIO_REGn_base + 0x0c);
  Io::write<Mword>(0, INTC_PRIO_REGn_base + 0x10);
  Io::write<Mword>(0, INTC_PRIO_REGn_base + 0x14);

  // Set privileg level to privileged
  Io::write<Mword>(1, INTC_PROT_EN);

  // Set NMI source
  Io::write<Mword>(0x00, NMI_INT_CTRL_REG);
  // Enable all interrupts
//  Io::write<Mword>(0xffffffff, INTC_EN_REGn_base);
//  Io::write<Mword>(0xffffffff, INTC_EN_REGn_base + 0x04);
//  Io::write<Mword>(0xffffffff, INTC_EN_REGn_base + 0x08);

  Irq_mgr::mgr = mgr.construct();
}

IMPLEMENT inline
Pic::Status Pic::disable_all_save()
{ return 0; }

IMPLEMENT inline
void Pic::restore_all(Status)
{}

PUBLIC static inline NEEDS["io.h"]
Unsigned32 Irq_chip_arm_sunxi::pending()
{
  for (int n = 0; n < (Pic::Num_irqs >> 5); ++n)
    {
      unsigned long x = Io::read<Mword>(Pic::INTC_IRQ_PEND_REGn_base + 4 * n);
      for (int i = 0; i < 32; ++i)
        if (x & (1 << i))
          return i + n * 32;
    }
  return 0;
}

extern "C"
void irq_handler()
{
  Unsigned32 i;
  while ((i = Irq_chip_arm_sunxi::pending()))
    mgr->c.handle_irq<Irq_chip_arm_sunxi>(i, 0);
}

//---------------------------------------------------------------------------
IMPLEMENTATION [debug && sunxi]:

PUBLIC
char const *
Irq_chip_arm_sunxi::chip_type() const
{ return "HW sunxi IRQ"; }
