# Welcome Chatting Server
## 1. 프로젝트의 목적과 성과

### 목적
- 채팅 프로젝트의 목적은 사용자들이 실시간으로 메시지를 교환하고 대화할 수 있는 채팅 시스템을 개발하는 것입니다.
- 이를 통해 사용자들은 원하는 주제에 대해 의견을 나누고 소통할 수 있으며, 효율적인 커뮤니케이션을 가능하게 합니다.

### 성과
- 팀원과의 협업을 통해 의사소통을 원활하게 하고, git의 숙련도를 향상시켰습니다.
- 채팅 기능을 구현하기 위해 필요한 기능을 기획하고, 테이블을 직접 설계함으로써 전체적인 흐름을 주도적으로 이끌 수 있는 능력을 얻었습니다.
- Socket을 활용하여 서버와 클라이언트가 서로 통신할 수 있도록 구현했습니다.
- 서버는 MySQL과 연동하여 데이터베이스 활용 능력을 기르고, C++을 사용해서 추가 기능을 구현함으로써 개발자의 역량을 향상시켰습니다.

## 2. 업무 분담
기획: [namwlee99](https://github.com/namwlee99)   
서버 개발: [namwlee99](https://github.com/namwlee99)   
클라이언트 개발: [bongseok-choi](https://github.com/bongseok-choi)   
소캣 처리 및 채팅 송수신 개발: [namwlee99](https://github.com/namwlee99)   
데이터베이스 설계 및 구현: [bongseok-choi](https://github.com/bongseok-choi)  

### 동작방식
1. 클라이언트와 서버 접속
![image](https://github.com/bongseok-choi/socket-project/assets/123155552/e2756257-1813-4c95-9599-6ed14e49e92f)

클라이언트가 실행되면, 클라이언트는 서버에 접속을 시도합니다. 서버는 클라이언트의 접속을 허용하고, 클라이언트에게 설정한 공지사항을 전송합니다. 이후 클라이언트는 회원 가입이나 로그인을 시도할 수 있습니다.

3. 회원 가입
![image](https://github.com/bongseok-choi/socket-project/assets/123155552/d78dbb51-5ea4-4c26-b57e-44b74fe485ea)

클라이언트가 회원 가입을 시도하면, ID를 입력받아 서버를 통해 중복된 ID가 있는지 확인합니다. 중복된 ID가 아니라면, 비밀번호와 닉네임을 추가로 입력받아 서버로 전송합니다. 서버는 입력받은 회원 정보를 데이터베이스에 저장합니다. 

5. 로그인
![image](https://github.com/bongseok-choi/socket-project/assets/123155552/f9fde58b-fd6c-47b1-9543-b535787bdfaf)

클라이언트가 로그인을 선택하면, ID와 비밀번호를 입력합니다. 입력받은 ID와 비밀번호를 서버로 전송하여 로그인을 시도합니다. 서버는 데이터베이스에서 입력받은 ID와 비밀번호를 찾습니다. ID와 비밀번호가 일치하는 경우, 클라이언트에게 채팅 서버 접속을 허용하고 닉네임도 전송합니다.

7. 채팅
![image](https://github.com/bongseok-choi/socket-project/assets/123155552/d5564ba8-8ea9-4ae8-ac12-f99f76411f6a)

여러 클라이언트가 채팅 서버에 접속하면, 클라이언트들은 서로 메시지를 주고받을 수 있습니다. 클라이언트가 메시지를 주고받을 때마다, 서버는 해당 메시지를 데이터베이스에 저장합니다. 저장되는 데이터는 시간을 인덱스로 하며, 사용자 ID와 메시지 내용이 저장됩니다.

## 4. 데이터베이스 구조
### chattings 테이블 구조
![image](https://github.com/bongseok-choi/socket-project/assets/82445853/a36c9923-abe2-461a-b230-12a619e7ccca)   

### Users
![image](https://github.com/bongseok-choi/socket-project/assets/82445853/4b9aedc0-53f8-4005-9280-728a82c6cbd4)

유저 테이블에는 회원가입 시 클라이언트의 정보를 저장하기 위해 아이디와 패스워드 그리고 닉네임을 저장한다. 그리고 사용자가 로그인을 한 상태인지 알기 위해 0과 1로 구별하기 위해 로그인 필드도 생성하였다.

### Chatting
![image](https://github.com/bongseok-choi/socket-project/assets/82445853/ebc73edb-8d3e-480c-a045-ec4475f8f70c)

채팅 테이블에는 채팅을 입력 시 시간을 저장하기 위해 time 필드를 넣었고, 어떤 유저가 채팅을 쳤는지 구별하기 위해 유저 아이디를 넣었고, 대화 내용들을 저장하기 로그 필드를 생성하였다.   
