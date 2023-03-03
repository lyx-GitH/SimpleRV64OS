set auto-load safe-path /
target remote :1234
shell clear
b *0x8020005c
c
si
si
#b start_kernel
#delete breakpoints 1
#b dummy
#b __dummy
#b schedule
#c
#layout src


delete breakpoints 1
b __dummy

