#!/usr/bin/env /bin/bash

printf "Setting up submodules..."

git submodule init && git submodule update

printf "Setting up build environment..."

mkdir build && cd build
touch .env
cmake ..
echo "TOKEN='<bot-token>'" > .env
make -j$(nproc --all)

printf "All set up! Please go into /build and edit .env to add your bot's token, after that you can run ./BingsuBot and everything should work just fine!"
