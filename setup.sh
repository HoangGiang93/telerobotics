#!/bin/bash

start_time=$(date +%s)

cd $(dirname $0) || exit
SRC_DIR=$PWD/src
EXT_DIR=$PWD/ext
INSTALL_DIR=$PWD/install

LIBDATACHANNEL_DIR=$EXT_DIR/libdatachannel
LIBDATACHANNEL_INSTALL_DIR=$INSTALL_DIR/libdatachannel
LIBDATACHANNEL_VERSION="v0.24.1"
if [ ! -d "$LIBDATACHANNEL_DIR" ]; then
    git clone --depth 1 https://github.com/paullouisageneau/libdatachannel.git "$LIBDATACHANNEL_DIR" --branch $LIBDATACHANNEL_VERSION
fi

if [ ! -d "$LIBDATACHANNEL_INSTALL_DIR" ]; then
    mkdir -p "$LIBDATACHANNEL_INSTALL_DIR"
    (cd "$LIBDATACHANNEL_DIR" && git pull origin $LIBDATACHANNEL_VERSION && git submodule update --init --recursive)

    # Build libdatachannel
    mkdir -p "$LIBDATACHANNEL_DIR/build"
    cd "$LIBDATACHANNEL_DIR/build" || exit
    cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$LIBDATACHANNEL_INSTALL_DIR" -DBUILD_SHARED_LIBS=OFF
    make -j"$(nproc)"
    make install
fi

YAML_CPP_DIR=$EXT_DIR/yaml-cpp
YAML_CPP_INSTALL_DIR=$INSTALL_DIR/yaml-cpp
YAML_CPP_VERSION="0.7.0"
if [ ! -d "$YAML_CPP_DIR" ]; then
    git clone --depth 1 https://github.com/jbeder/yaml-cpp.git "$YAML_CPP_DIR" --branch yaml-cpp-$YAML_CPP_VERSION
fi

if [ ! -d "$YAML_CPP_INSTALL_DIR" ]; then
    mkdir -p "$YAML_CPP_INSTALL_DIR"
    (cd "$YAML_CPP_DIR" && git pull origin yaml-cpp-$YAML_CPP_VERSION)

    # Build yaml-cpp
    mkdir -p "$YAML_CPP_DIR/build"
    cd "$YAML_CPP_DIR/build" || exit
    cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$YAML_CPP_INSTALL_DIR" -DYAML_CPP_BUILD_TESTS=OFF 
    make -j"$(nproc)"
    make install
fi

SERVER_DIR=$PWD/server
CLIENT_DIR=$PWD/client
if [ ! -f "$SERVER_DIR/server" ] || [ ! -f "$CLIENT_DIR/client" ]; then
    # Build sender and receiver
    cd "$SRC_DIR" || exit
    make clean
    make
fi

end_time=$(date +%s)
elapsed=$(( end_time - start_time ))
echo "Setup completed in $elapsed seconds."