rd  /s /q  build
mkdir build
cd build
cmake -DPICO_COPY_TO_RAM=1 -G "MinGW Makefiles" ..
echo copy pico_usb.uf2 D:>move.bat
make && move.bat