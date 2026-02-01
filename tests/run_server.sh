#!/bin/bash

cd $(dirname $0) || exit

SERVER_DIR=$PWD/../server

"$SERVER_DIR/server" germany "$PWD"/configuration/server_meta_data.yaml