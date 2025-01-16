dhcp
tftpboot ${kernel_addr_r} 10.0.2.2:hello.img
go ${kernel_addr_r}
