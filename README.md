# UDP forwarder EX

Create a UDP tunnel to forward a udp port. The tunnel applies some magic to trick the noob Great Firewall deep learning VPN
detection. 

Inspired by a naive udp-forward. I want to test what would happen, if I change the UDP server port number once a minute. 
(The GFW take 15 minutes to ban my OpenVPN UDP connection)

## Design

![explain.png](https://raw.githubusercontent.com/recolic/udp-forwarder-ex/master/res/explain.jpg)

## Build

- Dependency: https://github.com/recolic/rlib

```
mkdir build && cd build
cmake .. && make
./udp-forwarder [args ...]
```

## Example

```
# Assume OpenVPN listens base.tw7.recolic.net:1194/UDP
# We run UDPFwd on server:
./udp-forwarder -i plain@0.0.0.0@443 -o plain@::1@1194 --filter reverse@aes@MyPassword

# Then we run UDPFwd on our home router, to provide OpenVPN service:
./udp-forwarder -i plain@0.0.0.0@1194 -o plain@base.tw7.recolic.net@1194 --filter aes@MyPassword
# Good! You can use RouterIP:1194 as your OpenVPN server address now! It will work.
```

## TODOs

dynport module (-i / -o dynport@fe80:1111::1@10000-11000)

obfs method (quic and wechat video) (--filter obfs@quic@some.video.host.apple.com) (--filter obfs@wechat-video)


