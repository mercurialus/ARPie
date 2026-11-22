# ARPie

A small C++ learning project for exploring layer-2/3 networking with
[PcapPlusPlus](https://pcapplusplus.github.io/). It builds two command-line
programs that demonstrate packet capture, parsing, ARP, and traffic-generation
techniques.

> **For authorized use only.** Run ARPie only against hosts and networks you own
> or have explicit written permission to test (e.g. your own lab, a VM network,
> or a sanctioned course assignment). ARP spoofing and traffic flooding against
> networks you do not control are illegal in most jurisdictions.

## Project layout

```
ARPie/
├── CMakeLists.txt
├── README.md
├── include/            # shared headers
│   ├── ListInterfaces.h    # enumerate and describe network interfaces
│   ├── CapturePacket.h     # async / blocking packet capture with per-protocol counters
│   ├── ParsePacket.h       # read a .pcap file and dump Ethernet/IPv4/TCP/HTTP details
│   ├── floodie.h           # packet builders (SYN/TCP/UDP/HTTP/ICMP/ARP) + threaded sender
│   └── ARPieFlooder.h      # interactive front-end for the flood simulator
└── src/
    ├── main.cpp            # ARPieHelper entry point
    └── ARPie.cpp           # ARPie entry point
```

## Programs

| Executable     | Source           | What it does |
|----------------|------------------|--------------|
| `ARPieHelper`  | `src/main.cpp`   | Interactive menu: list interfaces, capture & tally packets, or run the flood simulator. |
| `ARPie`        | `src/ARPie.cpp`  | ARP cache poisoning demo (man-in-the-middle between a victim and gateway on a lab network). |

## Requirements

- A C++17 compiler
- CMake ≥ 3.10
- PcapPlusPlus and libpcap

Install PcapPlusPlus via Homebrew (macOS) or your package manager:

```sh
brew install pcapplusplus   # macOS
```

The `CMakeLists.txt` searches both `/usr/local` and `/opt/homebrew` include/lib
paths. If your installation lives elsewhere, adjust the `include_directories` /
`link_directories` entries.

## Build

```sh
cmake -S . -B build
cmake --build build
```

This produces `build/ARPie` and `build/ARPieHelper`.

## Run

Both tools open live network devices and therefore need elevated privileges:

```sh
sudo ./build/ARPieHelper
sudo ./build/ARPie -i <interface_ip> -c <victim_ip> -g <gateway_ip>
```

Press `Ctrl+C` to stop `ARPie`'s ARP-reply loop.
