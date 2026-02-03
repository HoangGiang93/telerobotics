#!/bin/bash

cd $(dirname "$0")

SESSION="client"

# Start new tmux session (detached)
tmux new-session -d -s $SESSION

# Pane 0 — Multiverse Server
tmux send-keys -t $SESSION "bin/multiverse_server_cpp --transport zmq --bind tcp://127.0.0.1:7030" C-m

# Split window vertically for pane 1
tmux split-window -v -t $SESSION
tmux send-keys -t $SESSION "workon telerobotics && python init.py --server_port=7030 --port=7035 --data_path=init_client.yaml" C-m

# Split pane 1 horizontally for pane 2
tmux split-window -h -t $SESSION:0.1
tmux send-keys -t $SESSION "workon telerobotics && python ../client/ws_client.py germany" C-m

# Split pane 0 horizontally for pane 3
tmux split-window -h -t $SESSION:0.0
tmux send-keys -t $SESSION "./../client/client germany ./../tests/configuration/client_meta_data.yaml" C-m

# Optional: even layout
tmux select-layout -t $SESSION tiled

# Attach to session
tmux attach -t $SESSION
