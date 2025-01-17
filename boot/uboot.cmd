setenv kboot_addr_r 0x85000000
tftp ${kboot_addr_r} hello.itb
bootm ${kboot_addr_r}
