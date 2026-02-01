#!/bin/bash

cd $(dirname $0) || exit

docker run --rm --network host \
  -v "$(pwd)"/config.json:/etc/cloudflared/config.json \
  cloudflare/cloudflared:latest \
  tunnel --no-autoupdate --config /etc/cloudflared/config.json run --token eyJhIjoiODY0Zjc4Y2ZiOTY3ZjFhNTExOGJhN2NlZDA1ZWZkMjgiLCJ0IjoiNDg4N2I3ZDQtNzc4ZC00MGRhLWI4MjctMjkxOWMwOTU0ZTdlIiwicyI6Ik9EQXhNek13TkdNdE9EWmxNQzAwWkdJNUxUZzFNak10TjJabFkyUTBORFV5WXpBeiJ9
