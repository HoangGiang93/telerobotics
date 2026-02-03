#!/bin/bash

cd $(dirname "$0")

# Check if 'python' is already available in the PATH
if command -v python &> /dev/null; then
    echo "Python is already active. Skipping environment setup."
else
    echo "Python not detected. Managing telerobotics environment..."

    if [ ! -d "telerobotics" ]; then
        echo "Environment folder not found. Creating it now..."
        python3 -m venv telerobotics
        . telerobotics/bin/activate
        python -m pip install -U pip
        # Use -f to ensure we find the requirements file relative to the script location
        pip install -r ../requirements.txt
    else
        echo "Activating existing telerobotics environment..."
        . telerobotics/bin/activate
    fi
fi

SESSION="server"

# Start new tmux session (detached)
tmux new-session -d -s $SESSION

# Pane 0 — Multiverse Server
tmux send-keys -t $SESSION "bin/multiverse_server_cpp --transport zmq --bind tcp://127.0.0.1:7020" C-m

# Split window vertically for pane 1
tmux split-window -v -t $SESSION
tmux send-keys -t $SESSION "python init.py --server_port=7020 --port=7025 --data_path=init_server.yaml" C-m

# Split pane 1 horizontally for pane 2
tmux split-window -h -t $SESSION:0.1
tmux send-keys -t $SESSION "python ../server/ws_server.py germany" C-m

# Split pane 0 horizontally for pane 3
tmux split-window -h -t $SESSION:0.0
tmux send-keys -t $SESSION "./../server/server germany ./../tests/configuration/server_meta_data.yaml" C-m

# Optional: even layout
tmux select-layout -t $SESSION tiled

# Attach to session
tmux attach -t $SESSION
