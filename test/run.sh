echo "mount -o rw,remount /dev/mmcblk0p2 /"
mount -o rw,remount /dev/mmcblk0p2 /
echo "./rotd &"
./rotd &
echo "./selector 7289 $1 &"
./selector 7289 $1 &
echo "./trial 1 $1 & ./trial 2 $1 & ./trial 3 $1"
./trial 1 $1 & ./trial 2 $1 & ./trial 3 $1
