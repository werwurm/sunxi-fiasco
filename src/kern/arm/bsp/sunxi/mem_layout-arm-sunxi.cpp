INTERFACE [arm && sunxi]: //-------------------------------------------

EXTENSION class Mem_layout
{
public:
  enum Virt_layout_sunxi : Address {
    Intc_map_base           = Devices1_map_base   + 0x20400,
    Timer_map_base          = Devices1_map_base   + 0x20c00,

    Uart0_map_base          = Devices1_map_base   + 0x28000,
  };

  enum Phys_layout_sunxi : Address {
    Devices1_phys_base       = 0x01c00000,
    Sdram_phys_base          = 0x40000000,
  };
};

INTERFACE [arm && sunxi_cubieboard]: //----------------------------------------

EXTENSION class Mem_layout
{
public:
  enum Virt_layout_sunxi_cubieboard : Address {
    Uart_base               = Uart0_map_base,
  };
};
