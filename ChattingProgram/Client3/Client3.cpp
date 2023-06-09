#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h> //Winsock 헤더파일 include. WSADATA 들어있음.
#include <WS2tcpip.h>
#include <string>
#include <sstream>
#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>
#define MAX_SIZE 1024
using std::cout; using std::cin; using std::endl; using std::string; using std::to_string;
char buf[MAX_SIZE] = { };
SOCKET client_sock;
string user_id;
string user_pw;
string nickname;
string get_time()
{
    using namespace std::chrono;
    system_clock::time_point tp = system_clock::now();
    std::stringstream str;
    __time64_t t1 = system_clock::to_time_t(tp);
    system_clock::time_point t2 = system_clock::from_time_t(t1);
    if (t2 > tp)
        t1 = system_clock::to_time_t(tp - seconds(1));
    tm tm{};
    localtime_s(&tm, &t1);
    str << std::put_time(&tm, "%Y%m%d%H%M%S") << std::setfill('0') << std::setw(3)
        << (std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count() % 1000);
    return str.str();
}
void buf_reset(char* buf)
{
    while (*buf)
    {
        *buf = 0;
        buf++;
    }
}
int chat_recv()
{
    while (1)
    {
        ZeroMemory(&buf, MAX_SIZE);
        if (recv(client_sock, buf, MAX_SIZE, 0) > 0)
            cout << buf;
        else
        {
            cout << "Server Off" << endl;
            return -1;
        }
    }
}
int main()
{
    WSADATA wsa;
    // Winsock를 초기화하는 함수. MAKEWORD(2, 2)는 Winsock의 2.2 버전을 사용하겠다는 의미.
    // 실행에 성공하면 0을, 실패하면 그 이외의 값을 반환.
    // 0을 반환했다는 것은 Winsock을 사용할 준비가 되었다는 의미.
    int code = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (!code) {
        client_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP); //
        SOCKADDR_IN client_addr = {};
        client_addr.sin_family = AF_INET;
        client_addr.sin_port = htons(5476);
        InetPton(AF_INET, TEXT("127.0.0.1"), &client_addr.sin_addr);
        while (1)
        {
            if (!connect(client_sock, (SOCKADDR*)&client_addr, sizeof(client_addr)))
            {
                recv(client_sock, buf, MAX_SIZE, 0);
                cout << buf << endl;
                buf_reset(buf);
                break;
            }
            else
            {
                cout << "Connecting..." << endl;
                break;
            }
        }
        while (1)
        {
            int option = 0;
            cout << "1. Log in 2. Sign in" << endl;
            cin >> option;
            if (option == 1)
            {
                send(client_sock, "login", MAX_SIZE, 0);
                while (1)
                {
                    cout << "ID: ";
                    cin >> user_id;
                    send(client_sock, user_id.c_str(), MAX_SIZE, 0);
                    cout << "Password: ";
                    cin >> user_pw;
                    send(client_sock, user_pw.c_str(), MAX_SIZE, 0);
                    recv(client_sock, buf, MAX_SIZE, 0);
                    if (string(buf) == "logsu")
                    {
                        cout << "Login success" << endl;
                        recv(client_sock, buf, MAX_SIZE, 0);
                        nickname = string(buf);
                        cout << "Hello! " << nickname << endl;
                        break;
                    }
                    else if (string(buf) == "logfa")
                        cout << "Login fail" << endl;
                }
                buf_reset(buf);
                break;
            }
            else if (option == 2)
            {
                send(client_sock, "signin", MAX_SIZE, 0);
                while (1)
                {
                    cout << "ID: ";
                    cin >> user_id;
                    send(client_sock, user_id.c_str(), MAX_SIZE, 0);
                    recv(client_sock, buf, MAX_SIZE, 0);
                    if (string(buf) == "useit")
                        cout << "ID use it" << endl;
                    else if (string(buf) == "usenotit")
                        break;
                    buf_reset(buf);
                }
                buf_reset(buf);
                cout << "Password: ";
                cin >> user_pw;
                send(client_sock, user_pw.c_str(), MAX_SIZE, 0);
                cout << "Nickname: ";
                cin >> nickname;
                send(client_sock, nickname.c_str(), MAX_SIZE, 0);
            }
        }
    }
    std::thread th2(chat_recv);
    int i = 0;
    while (1)
    {
        string text;
        cin >> text;
        text += get_time();
        send(client_sock, text.c_str(), strlen(text.c_str()), 0);
        cout << "\033[F";
        for (int i = 0; i < text.size(); i++)
            cout << "\b";
        for (int i = 0; i < text.size(); i++)
            cout << " ";
        for (int i = 0; i < text.size(); i++)
            cout << "\b";
    }
    th2.join();
    closesocket(client_sock);
    WSACleanup();
    return 0;
}