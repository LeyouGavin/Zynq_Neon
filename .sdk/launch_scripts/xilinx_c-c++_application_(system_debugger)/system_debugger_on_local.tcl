connect -url tcp:127.0.0.1:3121
source G:/myLab/SDK/NEON/zed_hw_platform/ps7_init.tcl
targets -set -nocase -filter {name =~"APU*" && jtag_cable_name =~ "Platform Cable USB II 000018eaf66101"} -index 0
loadhw -hw G:/myLab/SDK/NEON/zed_hw_platform/system.hdf -mem-ranges [list {0x40000000 0xbfffffff}]
configparams force-mem-access 1
targets -set -nocase -filter {name =~"APU*" && jtag_cable_name =~ "Platform Cable USB II 000018eaf66101"} -index 0
stop
ps7_init
targets -set -nocase -filter {name =~ "ARM*#0" && jtag_cable_name =~ "Platform Cable USB II 000018eaf66101"} -index 0
rst -processor
targets -set -nocase -filter {name =~ "ARM*#0" && jtag_cable_name =~ "Platform Cable USB II 000018eaf66101"} -index 0
dow G:/myLab/SDK/NEON/Neon/Debug/Neon.elf
configparams force-mem-access 0
bpadd -addr &main
