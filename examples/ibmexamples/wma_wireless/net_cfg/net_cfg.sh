./tunctl
ifconfig tap0 11.11.1.12 netmask 255.255.255.0 up
route add -net 10.11.1.0 netmask 255.255.255.0 gw 11.11.1.12 tap0

#arp -s 10.5.9.253 00:0D:60:28:02:0b
#arp -s 10.5.6.253 00:0D:60:28:06:09

