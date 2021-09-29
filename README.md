# [공통] 기본환경
## OS version
Ubunto 20.04
## compiler version
gcc 9.3.0
## 기본 사용법 보기
./실행파일명 --help

# Myls
## ls 명령어, -l,-t,-a,-i 옵션 구현
### 헤더 file flags.h
1. 데이터 저장 구조체 
2. 옵션 구분에 필요한 Flags
### 입력된 Argument Parsing 
들어온 argv 배열을 전부 탐색 하므로, 명령 입력 순서는 상관없음
### 예외처리
프로그램 종료되는 예외 X 모두 무시된후 기본 ls명령만수행됨 

# Mycmod
## chmod 명령어, -c 옵션, 문자형 permission 허용.

### 입력된 Argument Parsing 
들어온 argv배열에서 필요한 부분만 탐색 후 역활 자동지정, 입력 순서 중요함.
### 예외처리
1.프로그램 종료 - 구현 안된옵션, 3~4 넘어서는 숫자 permission, 문자열 기호+-= 외 다른기호.
2.프로그램 비종료 - user,gruop,other,all 중 지정 안된경우 각각 (u,g,o,a) 대소문자 상관없음.

# Mytouch
## touch 명령어, -r 옵션

### 입력된 Argument Parsing 
들어온 argv배열에서 필요한 부분만 탐색 후 역활 자동지정, 입력 순서 중요함.
### 예외처리
1.프로그램 종료 - r옵션시, 인자수가 맞지 않을 경우,argv[3]의 파일이 열리지 않는 경우


# Mytop
## top 명령어, Shift+m,Shift+t,Shift+c,화살표 옵션 구현
### 사용방법     
조건 1. 출력을 위해 curses 라이브러리를 사용하였다. 이 라이브러리가 설치되지 않았다면 오류가 발생한다. 설치 커맨드는 다음과 같다. 
apt-get install libncurses5-dev libncursesw5-dev   

Shift + C : 기본 출력과 같다. CPU 사용량 순으로 정렬해 보여준다.     
Shift + M : Memory 사용량 순으로 정렬해 보여준다.      
Shift + T : Cpu 사용 시간 순으로 정렬해 보여준다.      
a : 출력되는 Process 들이 한 줄 내려간다. Top 명령어의 아래 화살표 기능과 동일하다.      
b : 출력되는 Process 들이 한 줄 올라간다. Top 명령어의 위 화살표 기능과 동일하다.     
q : 출력을 종료하고 프로그램을 종료한다( Top 명령어처럼 해당 내용을 다시 출력해 주진 않는다)      


# Myps
## ps 명령어, a,u,x 옵션 구현 
### 사용방법   옵션 이 없을 시 : 현재 터미널의 tty와 같은 tty 를 가진 process 정보들만 출력된다. 출력 정보는 PID, TTY, TIME, CMD이다.    
옵션 ‘a’ : 터미널과 연관된 프로세스를 출력해 준다. TTY !=  “?” 인  프로세스들만 출력 된다.     
옵션 ‘u : 프로세스의 소유자자 이름이 현재 사용자 이름과 같은 프로세스들 중에 tty != “?” 인 프로세스들만  출력해 준다.      
옵션 ‘z’ : 프로세스의 소유자자 이름이 현재 사용자 이름과 같은 프로세스들을 모두 출력해 준다.     
+ 모든 옵션의 조합이 가능하다.     

# Mylscpu
## lscpu 명령어, -e옵션 구현
### 출력 목록
lscpu 명령어에서 Hypervisor Vendor제외 전부     
모든 정보 출력 : ./a.out -e -a       
원하는 정보만 출력 : ./a.out -e=CPU,NODE,… 이처럼 -e뒤에 = 를 붙인 후 원하는 정보의 이름을 ‘대문자’ 로 입력해야 한다. 각 정보의 구분은 ‘,’ 로 이루어진다.       
+ NODE 같은 만약 찾지 못했다면 모두 0으로 출력된다.        







