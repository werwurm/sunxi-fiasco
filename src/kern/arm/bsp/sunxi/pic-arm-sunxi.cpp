INTERFACE [arm && sunxi]:

#include "kmem.h"

class Irq_base;

EXTENSION class Pic
{
public:
  enum
  {
    INTC_PROT_EN             = 0x008,
    NMI_INT_CTRL_REG         = 0x00c,
    INTC_IRQ_PEND_REGn_base  = 0x010,
    INTC_FIQ_PEND_REGn_base  = 0x020,
    INTC_IRQ_TYPE_SELn_base  = 0x030,
    INTC_EN_REGn_base        = 0x040,
    INTC_MASK_REGn_base      = 0x050,
    INTC_RESP_REGn_base      = 0x060,
    INTC_FF_REGn_base        = 0x070,
    INTC_PRIO_REGn_base      = 0x080,
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
#include "mmio_register_block.h"

class Irq_chip_arm_sunxi : public Irq_chip_gen, public Mmio_register_block
{
public:
  Irq_chip_arm_sunxi()
  : Irq_chip_gen(Pic::Num_irqs)
  , Mmio_register_block(Kmem::mmio_remap(Mem_layout::Intc_phys_base)){}
  unsigned set_mode(Mword, unsigned) { return Irq_base::Trigger_level; }
  void set_cpu(Mword, Cpu_number) {}
};

PRIVATE inline
void
Irq_chip_arm_sunxi::setBit(const Mword base, const Mword irq){
  write<Mword>(
	(1 << (irq & 0x1f)) | read<Mword>(base + ((irq & 0xe0) >> 3)),
	base + ((irq & 0xe0) >> 3));
}

PRIVATE inline
void
Irq_chip_arm_sunxi::unsetBit(const Mword base, const Mword irq){
  write<Mword>(
	~(1 << (irq & 0x1f)) & read<Mword>(base + ((irq & 0xe0) >> 3)),
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
  write<Mword>(1 << (irq & 0x1f), Pic::INTC_IRQ_PEND_REGn_base + ((irq & 0xe0) >> 3));
}

PUBLIC
void
Irq_chip_arm_sunxi::ack(Mword irq)
{
  write<Mword>(1 << (irq & 0x1f), Pic::INTC_IRQ_PEND_REGn_base + ((irq & 0xe0) >> 3));
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
  Mmio_register_block rb(Kmem::mmio_remap(Mem_layout::Intc_phys_base));

  // Disable all interrupts
  rb.write<Mword>(0x0, INTC_EN_REGn_base);
  rb.write<Mword>(0x0, INTC_EN_REGn_base + 0x04);
  rb.write<Mword>(0x0, INTC_EN_REGn_base + 0x08);

  // Mask all interrupts
  rb.write<Mword>(0xffffffff, INTC_MASK_REGn_base);
  rb.write<Mword>(0xffffffff, INTC_MASK_REGn_base + 0x04);
  rb.write<Mword>(0xffffffff, INTC_MASK_REGn_base + 0x08);

  // clear all pending
  rb.write<Mword>(0xffffffff, INTC_IRQ_PEND_REGn_base);
  rb.write<Mword>(0xffffffff, INTC_IRQ_PEND_REGn_base + 0x04);
  rb.write<Mword>(0xffffffff, INTC_IRQ_PEND_REGn_base + 0x08);
  rb.write<Mword>(0xffffffff, INTC_FIQ_PEND_REGn_base);
  rb.write<Mword>(0xffffffff, INTC_FIQ_PEND_REGn_base + 0x04);
  rb.write<Mword>(0xffffffff, INTC_FIQ_PEND_REGn_base + 0x08);

  // Set all irqs to IRQ (not FIQ)
  rb.write<Mword>(0, INTC_IRQ_TYPE_SELn_base);
  rb.write<Mword>(0, INTC_IRQ_TYPE_SELn_base + 4);
  rb.write<Mword>(0, INTC_IRQ_TYPE_SELn_base + 8);

  // Set all priorities to 0 (0: lowest - 3: highest)
  rb.write<Mword>(0, INTC_PRIO_REGn_base);
  rb.write<Mword>(0, INTC_PRIO_REGn_base + 0x04);
  rb.write<Mword>(0, INTC_PRIO_REGn_base + 0x08);
  rb.write<Mword>(0, INTC_PRIO_REGn_base + 0x0c);
  rb.write<Mword>(0, INTC_PRIO_REGn_base + 0x10);
  rb.write<Mword>(0, INTC_PRIO_REGn_base + 0x14);

  // Set privileg level to privileged
  rb.write<Mword>(1, INTC_PROT_EN);

  // Set NMI source
  rb.write<Mword>(0x00, NMI_INT_CTRL_REG);
  // Enable all interrupts
//  write<Mword>(0xffffffff, INTC_EN_REGn_base);
//  write<Mword>(0xffffffff, INTC_EN_REGn_base + 0x04);
//  write<Mword>(0xffffffff, INTC_EN_REGn_base + 0x08);

  Irq_mgr::mgr = mgr.construct();
}

IMPLEMENT inline
Pic::Status Pic::disable_all_save()
{ return 0; }

IMPLEMENT inline
void Pic::restore_all(Status)
{}

PUBLIC static inline
Unsigned32 Irq_chip_arm_sunxi::pending()
{
  static Mmio_register_block rb(Kmem::mmio_remap(Mem_layout::Intc_phys_base));
  for (int n = 0; n < (Pic::Num_irqs >> 5); ++n)
    {
      unsigned long x = rb.read<Mword>(Pic::INTC_IRQ_PEND_REGn_base + 4 * n);
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
