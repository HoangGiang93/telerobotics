#!/bin/bash

cd $(dirname $0) || exit

docker run --rm --network host \
  -v "$(pwd)"/config.json:/etc/cloudflared/config.json \
  cloudflare/cloudflared:latest \
  tunnel --no-autoupdate --config /etc/cloudflared/config.json run --token eyJhIjoiODY0Zjc4Y2ZiOTY3ZjFhNTExOGJhN2NlZDA1ZWZkMjgiLCJ0IjoiYTJlOTUwNTItOWIxYy00MjI5LWFlNjQtNmE4OGU5YzQyYzBhIiwicyI6IlltVm1ORE01WmpFdE16QmtOaTAwTUdNNExXRTRPVEF0TmpNNU1XRTNNbU5oWkdFMyJ9
