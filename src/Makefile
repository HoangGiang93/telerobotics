# ==============================
# vr_telerobotics Makefile
# ==============================

CXX      := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra
LDFLAGS  :=
LIBS     := -lpthread

LIBDATACHANNEL_PREFIX := ../install/libdatachannel
YAML_CPP_PREFIX    := ../install/yaml-cpp
INCLUDES := -I$(LIBDATACHANNEL_PREFIX)/include -I$(YAML_CPP_PREFIX)/include -I../ext/include
LIBDIRS  := -L$(LIBDATACHANNEL_PREFIX)/lib -L$(YAML_CPP_PREFIX)/lib -L../lib
LIBS     += -ldatachannel -ljuice -lsrtp2 -lusrsctp -lssl -lcrypto -lyaml-cpp -lmultiverse_client_json_all -ljsoncpp -lmultiverse_client_all -lzmq

# Sources / targets
SERVER_SRC  := server.cpp
CLIENT_SRC 	:= client.cpp

SERVER_BIN  := ../server/server
CLIENT_BIN 	:= ../client/client

.PHONY: all clean

all: $(SERVER_BIN) $(CLIENT_BIN)

$(SERVER_BIN): $(SERVER_SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@ $(LIBDIRS) $(LIBS)

$(CLIENT_BIN): $(CLIENT_SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@ $(LIBDIRS) $(LIBS)

clean:
	rm -f $(SERVER_BIN) $(CLIENT_BIN)
