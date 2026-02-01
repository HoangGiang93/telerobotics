#!/bin/bash

cd $(dirname $0) || exit

CLIENT_DIR=$PWD/../client

"$CLIENT_DIR/client" germany "$PWD"/configuration/client_meta_data.yaml