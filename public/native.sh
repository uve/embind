g++ unpack.cpp -I /usr/local/include/ -I  /opt/homebrew/Cellar/boost/1.78.0_1/include \
  -Wall -Wl,-stack_size,0x10000000 -Wextra -Wcomment -std=gnu++17 -O3 && ./a.out