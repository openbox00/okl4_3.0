import copy
from simulators import versatile_boot, qemu_versatile_sim

############################################################################
# Versatile machines
############################################################################

class versatile(arm926ejs):
    device_core = "versatile"
    virtual = False
    platform = "versatile"
    memory = arm926ejs.memory.copy()
    memory['physical'] = [Region(0x04100000L, 0x08900000L)]
    memory['rom'] = [Region(0x08900000L, 0x09000000L)]
    timer_driver_v2 = "sp804_timer"
    memory_timer = [Region(0x101e3000, 0x101e4000, "all", "uncached")]
    interrupt_timer = [5]
    serial_driver_v2 = "pl011_uart_v2"
    memory_serial = [Region(0x101f1000, 0x101f2000, "all", "uncached")]
    interrupt_serial = [12]
    memory_eth = [Region(0x10010000, 0x10020000, "all", "uncached")]
    interrupt_eth = [25]
    memory_sys = [Region(0x10000000, 0x10001000, "all", "uncached")]
    memory_clcd = [Region(0x10120000, 0x10121000, "all", "uncached")]
    interrupt_clcd = [16]
    memory_kmi0 = [Region(0x10006000, 0x10007000, "all", "uncached")]
    interrupt_kmi0 = [35]
    memory_kmi1 = [Region(0x10007000, 0x10008000, "all", "uncached")]
    interrupt_kmi1 = [36]
    v2_drivers = [
                  (timer_driver_v2, "vtimer", memory_timer, interrupt_timer),
                  (serial_driver_v2, "vserial", memory_serial, interrupt_serial),
                  ("eth_device", "veth", memory_eth, interrupt_eth),
                  ("versatilesys_device", "vversatilesys", memory_sys, []),
                  ("kmi0_device", "vkmi0", memory_kmi0, interrupt_kmi0),
                  ("kmi1_device", "vkmi1", memory_kmi1, interrupt_kmi1),
                  ("clcd_device", "vclcd", memory_clcd, interrupt_clcd),
                  ("test_device", "vtest", [], [6,7])
                 ]
    cpp_defines = arm926ejs.cpp_defines + ["VERSATILE_BOARD"]
    boot_binary = True
    zero_bss = True
    copy_elf = True
    uart = "serial"



class versatile_uboot(versatile):
    device_core = "versatile"
    timer_driver_v2 = "sp804_timer"
    memory_timer = [Region(0x101e3000, 0x101e4000, "all", "uncached")]
    interrupt_timer = [5]
    serial_driver_v2 = "pl011_uart_v2"
    memory_serial = [Region(0x101f1000, 0x101f2000, "all", "uncached")]
    interrupt_serial = [12]
    memory_irq_controller = [Region(0x10140000, 0x10141000, "all", "uncached")]
    memory_eth = [Region(0x10010000, 0x10020000, "all", "uncached")]
    interrupt_eth = [25]
    memory_sys = [Region(0x10000000, 0x10001000, "all", "uncached")]
    memory_clcd = [Region(0x10120000, 0x10121000, "all", "uncached")]
    interrupt_clcd = [16]
    memory_kmi0 = [Region(0x10006000, 0x10007000, "all", "uncached")]
    interrupt_kmi0 = [35]
    memory_kmi1 = [Region(0x10007000, 0x10008000, "all", "uncached")]
    interrupt_kmi1 = [36]
    v2_drivers = [
                  (timer_driver_v2, "vtimer", memory_timer, interrupt_timer),
                  (serial_driver_v2, "vserial", memory_serial, interrupt_serial),
                  ("eth_device", "veth", memory_eth, interrupt_eth),
                  ("versatilesys_device", "vversatilesys", memory_sys, []),
                  ("kmi0_device", "vkmi0", memory_kmi0, interrupt_kmi0),
                  ("kmi1_device", "vkmi1", memory_kmi1, interrupt_kmi1),
                  ("clcd_device", "vclcd", memory_clcd, interrupt_clcd),
                  ("kirq_device", "kinterrupt", memory_irq_controller, []),
                  ("test_device", "vtest", [], [6,7])
                 ]
    run_methods = { 'hardware': versatile_boot, 
        'qemu':qemu_versatile_sim }
    default_method = 'qemu'
    uart = "serial"
    boot_binary = True
