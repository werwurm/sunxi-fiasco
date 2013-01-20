INTERFACE [arm && sunxi]:

enum {
  Cache_flush_area = 0,
};

//-----------------------------------------------------------------------------
IMPLEMENTATION [arm && sunxi]:

#include "mem_layout.h"
#include "io.h"

void
map_hw(void *pd)
{
  map_dev<Mem_layout::Devices1_phys_base>(pd, 1);
}
