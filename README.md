# Telerobotics

This repository contains a telerobotics application with server and client components that communicate via WebRTC.

## Setup

### Prerequisites
- Docker (for Cloudflare tunnel)
- CMake and build tools (for compiling the server and client)
- Git

### Installation Steps

1. **Clone the repository**
   ```bash
   git clone https://github.com/HoangGiang93/telerobotics.git
   cd telerobotics
   ```

2. **Set up environment variables**
   ```bash
   # Copy the example environment file
   cp .env.example .env
   ```

3. **Configure your Cloudflare tunnel token**
   
   ⚠️ **SECURITY WARNING**: Never commit your `.env` file or share your Cloudflare tunnel token publicly!
   
   - Go to [Cloudflare Zero Trust Dashboard](https://one.dash.cloudflare.com/)
   - Navigate to: **Zero Trust > Networks > Tunnels**
   - Select your tunnel (or create a new one)
   - Click **Configure** and copy your tunnel token
   - Edit the `.env` file and replace `your_token_here` with your actual token:
     ```bash
     CLOUDFLARE_TUNNEL_TOKEN=eyJhIjoiYour_Actual_Token_Here..."
     ```

4. **Run the setup script**
   ```bash
   ./setup.sh
   ```
   This will:
   - Clone and build libdatachannel
   - Clone and build yaml-cpp
   - Compile the server and client applications

5. **Load environment variables**
   ```bash
   source .env
   ```

### Running the Application

1. **Start the Cloudflare tunnel**
   ```bash
   ./run.sh
   ```
   This will start the Cloudflare tunnel using your token from the environment variable.

2. **Start the server** (in a new terminal)
   ```bash
   cd server
   ./server
   ```

3. **Start the client** (in another new terminal)
   ```bash
   cd client
   ./client
   ```

## Security Notes

- **NEVER** commit the `.env` file to git (it's already in `.gitignore`)
- **NEVER** share your Cloudflare tunnel token publicly
- If you accidentally expose your token, regenerate it immediately in the Cloudflare dashboard
- Keep your tokens secure and rotate them regularly

## What is this repository for?

* Quick summary
* Version
* [Learn Markdown](https://bitbucket.org/tutorials/markdowndemo)

## Configuration

* Summary of set up
* Configuration details
* Dependencies

## How to run tests

* Instructions for running tests

## Contribution guidelines

* Writing tests
* Code review
* Other guidelines

## Who do I talk to?

* Repo owner or admin
* Other community or team contact
