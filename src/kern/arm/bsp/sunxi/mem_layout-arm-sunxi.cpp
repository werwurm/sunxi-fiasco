INTERFACE [arm && sunxi]: //-------------------------------------------

EXTENSION class Mem_layout
{
public:

  enum Phys_layout_sunxi : Address {
	Intc_phys_base			= 0x01c20400,
	Timer_phys_base			= 0x01c20c00,

	Uart0_phys_base			= 0x01c28000,
  };
};

INTERFACE [arm && sunxi_cubieboard]: //----------------------------------------

EXTENSION class Mem_layout
{
public:
  enum Virt_layout_sunxi_cubieboard : Address {
    Uart_phys_base			= Uart0_phys_base,
  };
};
