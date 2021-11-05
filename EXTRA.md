# Imporve WRR Scheduler
### Idea 1
- 로드 밸런스를 진행할때, 진행 조건이 까다롭다. weight_sum이 가장 큰 runqueue에서 task 를 옮겼을 때 대소관계가 바뀌지 않을 가장 큰 weight를 가진 task를 찾아야 하기 때문에 searching cost가 크다.
- 이를 줄이기 위해 weight_sum이 가장 큰 runqueue에서 첫번째 task부터 옮겼을 때 대소관계가 바뀌지 않을 경우가 찾아지기만 하면 곧바로 옮겨주는 것도 방법이 될 것 같다.

### Idea 2
- 로드 밸런스의 문제점중 하나는, 자꾸 weight가 큰 task가 옮겨질 수 있다는 점이다. 옮겨지는 것의 문제점은 옮겨질 runqueue의 가장 마지막에 들어가기 때문에, 순서가 밀리고, 순서가 밀려 있다가 또 그 task가 load balance 될 수 있다.
- 예를 들어, smallest runqueue의 weight 순서가 1, 1, 1, 1, 1, 1이고, biggest runqueue의 weight 순서가 1, 10, 5, 5, 5, 5 였다면 weight10 인 task가 옮겨지게 된다. 조금만 기다리면 바로 자기의 차례가 돌아올 때였는데 갑자기 맨 뒤로 밀려나게 되는 것이다. 
- 이는 weight를 우리가 높이 세팅 할수록 빨리 끝나길 기대하는 우리의 목적에 위배된다.
- 따라서 옮길 수 있는 task의 가장 큰 weight를 가진 task를 이동시키는 것이 아니라, 옮길 수 있는 task 중 가장 뒤에 있는 task를 옮겨주는 것도 하나의 방법이 될 것 같다.
- 또 다른 방법으로, 옮길 수 있는 task 중 첫번째 task를 제외한 가장 큰 weight를 가진 task를 옮기는 것도 하나의 방법이 될 것 같다.

### Idea 3
- weight를 스스로 boost up 하는 아이디어를 생각해볼 수 있다. 각 task마다 time slice를 몇번 다 소진해서 다시 requeue 됐는지 숫자를 세고, 우리가 정해준 어떤 숫자를 넘어서서 소진한다면, weight를 올려준다
- 계속 time slice를 다 소진했는데 끝나지 않느다는 것은 해당 task의 할 일이 많다는 것이고, 앞으로 많이 남아 있을 확률이 높다고 판단하고, weight를 올려준다.
