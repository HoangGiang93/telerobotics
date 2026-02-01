#!/bin/bash

cd $(dirname $0) || exit

# Check if CLOUDFLARE_TUNNEL_TOKEN is set
if [ -z "$CLOUDFLARE_TUNNEL_TOKEN" ]; then
  echo "Error: CLOUDFLARE_TUNNEL_TOKEN environment variable is not set."
  echo ""
  echo "To fix this:"
  echo "1. Edit .env and add your Cloudflare tunnel token"
  echo "2. Load the environment variables:  source .env"
  echo "3. Run this script again"
  echo ""
  echo "Alternatively, you can set the token directly:"
  echo "  export CLOUDFLARE_TUNNEL_TOKEN='your_token_here'"
  echo ""
  exit 1
fi

docker run --rm --network host \
  -v "$(pwd)"/config.json:/etc/cloudflared/config.json \
  cloudflare/cloudflared:latest \
  tunnel --no-autoupdate --config /etc/cloudflared/config.json run --token "$CLOUDFLARE_TUNNEL_TOKEN"
