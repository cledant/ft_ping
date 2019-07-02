# ft_ping

This 42 project aims to reimplement ping binary.

## Compiling

You may compile `ft_ping` by running `cmake`.

## Usage

You need to be `root` or `sudo` in order to use `ft_ping`.

Usage: `ft_ping` [-vhqnDf] [-s packetsize] [-t ttl] destination [-w deadline]  
-v : Display packet errors  
-h : Display usage  
-q : Quiet output  
-n : No name lookup for host address  
-D : Print timestamp before each line  
-f : Flood. No wait time between icmp request