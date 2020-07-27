# UDP forwarder - dynPort

Create a UDP tunnel to forward a udp port. The tunnel applies some magic to trick the noob Great Firewall deep learning VPN
detection. 

Inspired by a naive udp-forward. I want to test what would happen, if I change the UDP server port number once a minute. 
(The GFW take 15 minutes to ban my OpenVPN UDP connection)

## Design

![explain.png](https://raw.githubusercontent.com/recolic/udp_forwarder_ng/master/res/explain.png)

## Build

```
mkdir build && cd build
cmake .. && make
./udp_forwarder_ng [args ...]
```

## Common Deployment

![solu.png](https://raw.githubusercontent.com/recolic/udp_forwarder_ng/master/res/solu.png)

