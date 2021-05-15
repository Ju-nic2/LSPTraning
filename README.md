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

