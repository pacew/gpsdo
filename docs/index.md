 <!-- -*- mode:gfm -*- -->

# GPS Disciplined Oscillator

## next rev

* larger size 10uF caps
* maybe supercap for vbat to gps module?

## data sheets

### Processor
* [STM32F407 data sheet](stm32f405rg.pdf) STM32F407VET6
* [STM32Fxxx Reference Manual](stm32f4-rm.pdf)

### Other chips
* [ATGM336H](ATGM336H-5N31_C90770.pdf) GPS module
  * [AT6558](gps-chip.pdf) GPS chip used in ATGM336H gps module
  * [GPS technical manual, Chinese](Multimode_satellite_navigation_receiver_cn.pdf)
* [AOCJY1](AOCJY1.pdf) Oven Compensated Crystal Oscillator 10MHz
* [AMS1117-3.3](1811201117_Advanced-Monolithic-Systems-AMS-AMS1117-3-3_C6186.pdf) 
  3.3 volt regulator 1A ; 1v dropout
* [SRV05-4](ProTek-Devices-SRV05-4-P-T7_C85364.pdf) usb esd protection

### Discretes

* [caps](Samsung-Electro-Mechanics-CL05B104KO5NNNC_C1525.pdf) ceramic 0402 100nF/16v 10uF/6.3v 2.2uF/16v
* [caps](Guangdong-Fenghua-Advanced-Tech-0402CG120J500NT_C1547.pdf) ceramic 0402 12pf/50v
* [led](Hubei-KENTO-Elec-Green-0805-Iv-207-249-mcd-atIF-20mA_C2297.pdf) 0805 green
* [schottky diode](1903041730_MDD-Jiangsu-Yutai-Elec-SS210_C14996.pdf) SS210
* [fuse](TECHFUSE-nSMD025_C70068.pdf) nSMD025 250mA (hold) Rmax 2.5 ohms
* [ferrite bead](Sunlord-GZ2012D101TF_C1015.pdf) 100 ohm @ 100 MHz; 0.15 ohm @ DC; 800 mA max
* [Thermistor](Murata-Electronics-NCP18WB473J03RB_C86142.pdf) 0603 47k
* [Inductor](Sunlord-SDCL1608C47NJTDF_C29683.pdf) 47nH for active GPS antenna
* [resistors](Uniroyal-Elec-0402WGF1002TCE_C25744.pdf) 0402 1/16W 1% 10k 100k 220

### Other parts
* [USB micro connector mechanical drawing](usb-micro-C404969.pdf)
* [74AHC1G04](Texas-Instruments-TI-SN74AHC1G04DCKR_C7466.pdf) inverter
* [16mhz crystal](Yangxing-Tech-X322516MLB4SI_C13738.pdf)

### Generic ARM docs
* [ARM Cortext M4 Technical Reference Manual
  ](arm_cortexm4_processor_trm_100166_0001_00_en.pdf)
* [PM0214](cortexm4-pm.pdf) STM32 Cortex-m4 programming manual
* [ARMv7-M Architecture Reference Manual](DDI0403E_d_armv7m_arm.pdf)
* [ARM Debug Interface Architecture Specification ADIv5.2](IHI0031C_debug_interface_as.pdf)
* [Coresite v3.0 Architecture Specifiation
  ](coresight_v3_0_architecture_specification_IHI0029E.pdf)
  defines arm debug features 
* [CoreSite DAP-Lite](DDI0316D_dap_lite_trm.pdf) Technical Reference Manual
* [ARM Cortex-M4 Generic User Guide](DUI0553A_cortex_m4_dgug.pdf)

### For future projects
* [Jauch 246525](6000mah_-_lp906090jh_1s1p_2_wire_70mm.pdf) battery
* [DRV8801](drv8801.pdf) TI Full Bridge Motor Driver


