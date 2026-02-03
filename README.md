# Telerobotics

A WebRTC-based telerobotics system with server (robot/host) and client (operator) components.

This project enables low-latency remote control of a robot over the internet using peer-to-peer WebRTC data channels.

## Features

- Peer-to-peer communication via WebRTC (libdatachannel)
- Cloudflare Tunnel support for easy public access without port forwarding
- Simple bash-based startup scripts

## Prerequisites

- **[Docker](https://docs.docker.com/engine/install/ubuntu/#install-using-the-repository)** – needed for Cloudflare tunnel (recommended installation method)
- **CMake** + build-essential tools
- **Git**
- **tmux** (optional but used in run scripts)

Ubuntu example:

```bash
sudo apt update
sudo apt install -y git cmake build-essential tmux
```

## Installation

```bash
# 1. Clone the repository
git clone https://github.com/HoangGiang93/telerobotics.git
cd telerobotics

# 2. Make scripts executable
chmod +x *.sh server/server client/client tests/*.sh

# 3. Run the full setup (downloads & builds dependencies)
./setup.sh
```

The setup script will:

- Clone and build **libdatachannel** (WebRTC implementation)
- Clone and build **yaml-cpp**
- Compile the C++ server and client binaries

## Running the System

You can run both server and client on the same machine (for testing) or on two different machines connected to the internet.

### Server Side (Robot / Host machine)

1. **Set your Cloudflare Tunnel token**

   ```bash
   export CLOUDFLARE_TUNNEL_TOKEN=your_very_long_token_here
   ```

2. **Start Cloudflare Tunnel + support services**

   ```bash
   ./run.sh
   ```

3. **Verify tunnel is working**

   ```bash
   ping ws.multiverse-framework.com
   ```

   You should see replies similar to:

   ```bash
   PING ws.multiverse-framework.com (2a06:98c1:3120::4) 56(84) bytes of data.
   64 bytes from 2a06:98c1:3120::4: icmp_seq=1 ttl=57 time=12.4 ms
   ```

4. **Launch the WebRTC signaling & control server**

   ```bash
   bash tests/run_server.sh
   ```

### Client Side (Operator / Control machine)

1. **Start the client**

   ```bash
   bash tests/run_client.sh
   ```

   The client should connect automatically to the server through the WebRTC data channel once both sides are running.

## Quick Summary – Two-machine setup

**Server machine:**

```bash
export CLOUDFLARE_TUNNEL_TOKEN=xxx
./run.sh
# in another terminal:
bash tests/run_server.sh
```

**Client machine:**

```bash
bash tests/run_client.sh
```

## Troubleshooting Tips

- Make sure the Cloudflare token is correct and active
- Check that `ws.multiverse-framework.com` resolves and is reachable
- Look at terminal output for connection / ICE candidate / signaling errors
- Both machines need internet access (NAT traversal is handled by WebRTC + STUN/TURN)
