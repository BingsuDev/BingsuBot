#!/usr/bin/env /bin/bash

printf "Setting up submodules..."

git submodule init && git submodule update

printf "Setting up build enviornment..."

mkdir build && cd build
touch .env
cmake ..
echo "TOKEN='<bot-token>'" > .env
make

printf "You are now in the /build directory, please edit .env to add your bot's token, after that you can run ./BingsuBot and everything should work just fine!"