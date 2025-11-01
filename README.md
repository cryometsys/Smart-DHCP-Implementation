# Smart DHCP: Adaptive IP Allocation Based on Client Type

This project implements an intelligent **Dynamic Host Configuration Protocol (DHCP)** server that dynamically adjusts IP lease durations based on client type (IoT, Desktop, Admin) to optimize IP address utilization in heterogeneous networks. 
The project was built entirely in **OMNeT++** without using any external frameworks. The simulation demonstrates how context-aware leasing improves efficiency, reduces address exhaustion, and enables faster IP reuse.

## Key Features

- **Client classification**: Clients declare type (`IoT`, `Desktop`, `Admin`, `NewIoT`) in `DHCPDISCOVER`.
- **Adaptive lease assignment**:
  - IoT → 10 seconds
  - Desktop → 30 seconds
  - Other devices → 60 seconds
- **Full DHCP handshake protocol**: DISCOVER → OFFER → REQUEST → ACK
- **Automatic lease renewal** at 80% of lease time
- **Active lease tracking**: Server periodically checks for expired leases and reclaims IPs
- **IP reuse demonstration**: A 4th client (`NewIoT`) is denied when the pool is full but acquires a freed IP after an IoT client disconnects
- **No INET dependency**: Pure message-passing using core OMNeT++ constructs

## Project Structure

**SmartDHCP/src/**
  * `SmartDHCP.ned` ⇒ Network topology
  * `DHCPMessage.msg` ⇒ Custom DHCP message format
  * `DHCPServer.h`/`.cc` ⇒ Smart DHCP server logic
  * `DHCPClient.h`/`.cc` ⇒ Client behavior (renewal, disconnection)
  * `omnetpp.ini` ⇒ Simulation configuration

## Requirements

- OMNeT++ 6.x (tested on v6.2.0)
- C++ compiler (Clang or GCC)
- Make

## How to Run

1. **Clone the repository**
   ```bash
   git clone https://github.com/cryometsys/Smart-DHCP-Implementation.git
   cd Smart-DHCP-Implementation
   ```
2. **Build the project**
    ```bash
    make clean
    make
    ```
3. **Run the simulation**
   In OMNeT++ IDE: Right-click `omnetpp.ini` → Run As → OMNeT++ Simulation
4. **Observe the output**
    - Watch messages flow in Qtenv
    - Check console logs for:
      - Adaptive lease assignment
      - `LEASE EXPIRED` and `Free IPs` messages
      - IP reuse by `NewIoT` after IoT client disconnects

## Expected Behavior
t = 0.1s: IoT, Desktop, Admin get all 3 IPs

t = 5s: NewIoT denied (no free IP)

t = 24s: IoT client disconnects after 2 renewals

t = 26s: IoT lease expires → IP reclaimed

t = 27s: NewIoT successfully acquires freed IP<br>

This validates that Smart DHCP improves IP pool utilization compared to traditional fixed-lease DHCP.

## Notes
The simulation uses a 3-IP pool to intentionally create scarcity.
Client disconnection is simulated by stopping renewal after 2 cycles (IoT only).
All logic is implemented in C++.

## Academic Context
Developed as part of ***CSE 4106: Computer Networks Laboratory***

Khulna University of Engineering & Technology
