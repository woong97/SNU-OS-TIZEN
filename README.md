### Tip
- Execute under command in "Tizen shell" after execute ./qemu.sh
```
mount -o rw,remount /dev/mmcblk0p2 /
```

### How to run
- 실행의 편의를 위해 run.sh script 파일을 만들었습니다

```C
# run.sh
echo "mount -o rw,remount /dev/mmcblk0p2 /"
mount -o rw,remount /dev/mmcblk0p2 /
echo "./rotd &"
./rotd &
echo "./selector 7289 $1 &"
./selector 7289 $1 &
echo "./trial 1 $1 & ./trial 2 $1 & ./trial 3 $1"
./trial 1 $1 & ./trial 2 $1 & ./trial 3 $1
```

- 실행은 "bash run.sh" 또는 "bash run.sh slow" 로 실행시킵니다. slow는 read랑 write를 좀 더 천천히 수행하게끔 하는 모드입니다. sleep mode의 경우 trial.c, selector.c에서 while loop를 돌 때 수행을 끝내고 unlock 한 뒤 sleep함수를 호출합니다. sleep() 함수를 사용하지 않으면 number가 너무 빨리 증가해서, slow mode 라는 것을 만들어 shell script의 input argument에 'slow'를 추가해주면 slow mode로 실행시킵니다

```C
root:~> bash run.sh slow
./rotd &
./selector 7289 slow &
./trial 1 slow & ./trial 2 slow & ./trial 3 slow & 
Running mode is 'slow mode'
Running mode is 'slow mode'
Running mode is 'slow mode'
trial-1: 7289 = 37 * 197
trial-3: 7289 = 37 * 197
trial-2: 7289 = 37 * 197
trial-1: 7290 = 2 * 3 * 3 * 3 * 3 * 3 * 3 * 5
trial-3: 7290 = 2 * 3 * 3 * 3 * 3 * 3 * 3 * 5
trial-2: 7290 = 2 * 3 * 3 * 3 * 3 * 3 * 3 * 5
trial-2: 7291: 23 * 317
trial-1: 7291: 23 * 317
trial-3: 7291: 23 * 317
.
.
.
```

- 주의사항: tizen shell내에서 slow mode와 default mode를 두개 다 테스트 하고 싶을 때 수행 순서를 맞춰줘야 합니다. slow mode 테스트 -> slow mode로 실행시킨 process들 kill -> default mode 테스트
- 이 순서가 반대가 된다면, process들이 잘 killed되지 않습니다. 이 순서를 맞춰주지 않으면 특정 mode로 테스트 한뒤, qemu.sh를 종료하고 다시 재실행해서 테스트 해줘야 합니다. qemu.sh를 재실행해서 두번 테스트 하고 싶지 않을 때 이 순서대로 테스트대로 테스트하면 됩니다.
```C
root:~> bash run.sh slow
Ctrl+C...
root:~> bash kill.sh
root:~> bash run.sh
```

```C
# kill.sh
pkill -f "selector"
pkill -f "trial"
pkill -f "rotd"
```
