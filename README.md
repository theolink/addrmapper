# addrmapper
A virtual domain name - IPV6 address mapping tool

When you have an externally accessible ipv6 address, but the address is not fixed, you can use this tool to do a virtual domain-to-ip mapping.

## Requirements:
1. External accessible non-fixed ipv6 address
2. A server with fixed IP (ipv4)

**Ps: you don't need a real domain name, this tool is used to map IP domain names are fake domain names, do not exist, you can construct any domain name you want, such as "dog.cat.pig".** 

## Working principle:
1. Upload-client (the machine whose native ipv6 address will be used) sends the native ipv6 address to the server, and the server archives it
2. Download-client (the machine that needs to use the Upload-client's ipv6 address) download the address data from the server and write it into the hosts file

## Steps:
1. Cloning project  
`git clone `  

2. Compile:  
`cd addrmapper/addrmapper`  
`g++ addrmapper.cpp -o addrmapper`  
`chmod +x addrmapper`  

3. Server-side:  
`./addrmapper [-s] [-p port] [-k key]`  
**if you didn't input any arg of '-s', '-u', '-g', it will run as server by default.**  
Upload-client side:  
`./addrmapper -u -a server_addr [-p port] -d client_virtual_domain [-k key]`  
Download-client side:  
`./addrmapper -g -a server_addr [-p port] [-k key]`  

## arguments
1. Server:  
  -s: run as a server
2. Upload-client:  
  -u: run as update client.  
  -d: set client virtual domain name, like: "example.test.com".  
3. Download-client:  
  -g: run as get client.      
4. Other param：  
  -p: server port, if you didn't set a port, it will use the default port 12880, be sure it's accessible!  
  -a: set server address when run as a client.  
  -k: set a key, if you didn't set a key, it will use the default key!   
