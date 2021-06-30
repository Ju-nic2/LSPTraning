## Cell Matrix Game   
다중 프로세스, 다중 스레드 이용하여 Matrix연산 수행.    
연산은 전체 탐식 방식으로 진행됨.  
input.matrx file 을 읽어 일정한 규칙으로 다음 세대 Matrix를 계산함.    \
계산된 Matirx 는 gen_n.Matrix(중간), output.Matrix(최종결과)    
의 이름으로 세대 진행시 마다 써짐.   

### [공통] 기본환경
#### OS version
Ubunto 20.04
#### compiler version
gcc 9.3.0
#### 사용 cpu 
Intel® Core™ i5-10210U Processor(1.60 GHz up to 4.20 GHz 6 MB L3 Cache)    

#### 가상 머신 환경
4core 할당, 8GB RAM 

### 공통 사항 
OS : Ubunto 20.04
Compiler gcc

### 예외처리
Shared memory 예외 : shared memory 할당에 실패할 경우.   
Fork 예외 : fork() 함수 실행이 실패할 경우   
Signal 예외 : signal handler 등록에 실패할 경우.   
Semaphore 예외 : sem_init() 에 실패할 경우   
Barrier 예외 : pthread_barrier_init() 에 실패할 경우   

### 실행시 필요한 공간
(input file 의 모든 문자(‘0’,’1’) 수*(size of(char) + (size of(void*) * 행수) * 2   
-> shared memory 를 이용하여 기 때문에 충분한 크기의 메모리가 확보 되어야함. 

### 실행방법
ex > ./a.out <input file>.matrix file   
or ./a.out    -> 이경우 현재 디렉토리에 "input.matrix" 있어야함.   

## main.c
프로세스, 스레드 생성 오버헤드 체크 용. 
자식 프로세스, 스레드를 매 세대 진행시 마다 새로 생성함.   

## main2.c
다중 프로세스, 스레드의 연산 효과 증가 파악 용. 
자식 프로세스, 스레드를 한번 생성 후 모든 세대 연산 진행.    

### 실험 결과
### main2.c 이용
![image](https://user-images.githubusercontent.com/72595878/123920993-82204300-d9c1-11eb-9354-7608ff6570fa.png)

![image](https://user-images.githubusercontent.com/72595878/123921054-92382280-d9c1-11eb-8b14-6bc5aad29bf6.png)
### main.c 이용 
![image](https://user-images.githubusercontent.com/72595878/123921090-9cf2b780-d9c1-11eb-932c-5568c6ee65d1.png)

