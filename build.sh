echo "password" | sudo -S
make clean && ./build-rpi3-arm64.sh && sudo ./scripts/mkbootimg_rpi3.sh
