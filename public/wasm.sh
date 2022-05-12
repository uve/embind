emcc -lembind -o unpack.js unpack.cpp  -ftemplate-depth=10000 \
 -I /usr/local/include/ -I  /opt/homebrew/Cellar/boost/1.78.0_1/include \
  -Wall -Wl -Wextra -std=gnu++17  -sALLOW_MEMORY_GROWTH -O3