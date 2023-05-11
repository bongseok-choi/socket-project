#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <mysql/jdbc.h>

using std::cout; using std::cin; using std::endl; using std::string; using std::to_string; using std::vector;

#define MAX_SIZE 1024
#define MAX_CLIENT 3

const string sql_server = "tcp://127.0.0.1:3306"; // 데이터베이스 주소 (3306: MySQL 데이터베이스 기본 포트 번호)
const string username = "root"; // 데이터베이스 사용자
const string password = "abcd1234!"; // 데이터베이스 접속 비밀번호

sql::mysql::MySQL_Driver* driver; // 추후 해제하지 않아도 Connector/C++가 자동으로 해제해 줌
sql::Connection* con;
sql::Statement* stmt;
sql::PreparedStatement* pstmt;
sql::ResultSet* result;

struct SOCKET_INFO
{
    SOCKET sck;
    string user_id;
    string nickname;
};

vector<SOCKET_INFO> sck_list;
SOCKET_INFO server_sock;

vector<vector<string>> chat_log_list;

int client_count = 0;
int unclient_count = 0;

int find_account(int mode, string user_id, string user_pw);
void in_account(string user_id, string user_pw, string nickname);
void server_init();
void add_client(int un_clinet);
string recv_send_msg(int mode, SOCKET sck, string msg);
void recv_msg(int idx);
void del_client(int idx);

int find_account(int mode, string user_id, string user_pw) //mode = 1 find id / mode = 2 find id, pw / mode = 3 로그아웃
{
    try 
    {
        driver = sql::mysql::get_mysql_driver_instance();
        con = driver->connect(sql_server, username, password);
    }
    catch (sql::SQLException& e) 
    {
        cout << "Could not connect to server. Error message: " << e.what() << endl;
        exit(1);
    }
    string find_id, find_pw, nickname = "";

    con->setSchema("chattings");
    pstmt = con->prepareStatement("SELECT * FROM users;");
    result = pstmt->executeQuery();

    if (mode == 1) 
    {
        while (result->next()) 
        {
            find_id = result->getString("user_id");
            if (find_id == user_id) 
            {
                return 1;
            }
        }
    }
    else if (mode == 2) 
    {
        while (result->next()) 
        {

            find_id = result->getString("user_id");
            find_pw = result->getString("user_pw");
            if (user_id == find_id && find_pw == user_pw)
            {
                pstmt = con->prepareStatement("UPDATE users SET login = ? WHERE user_id = ?");
                pstmt->setInt(1, 1);
                pstmt->setString(2, user_id);
                pstmt->executeQuery();
                return 1;
            }
        }
    }
    else if (mode == 3)
    {
        pstmt = con->prepareStatement("UPDATE users SET login = ? WHERE user_id = ?");
        pstmt->setInt(1, 0);
        pstmt->setString(2, user_id);
        pstmt->executeQuery();
    }

    return 0;
}

string get_nickname(string user_id)
{
    string find_id, nickname = "";
    con->setSchema("chattings");
    pstmt = con->prepareStatement("SELECT * FROM users;");
    result = pstmt->executeQuery();

    while (result->next()) 
    {
        find_id = result->getString("user_id");
        if (user_id == find_id)
        {
            nickname = result->getString("nickname");
            return nickname;
        }
    }
    return "";
}

void in_account(string user_id, string user_pw, string nickname)
{
    try 
    {
        driver = sql::mysql::get_mysql_driver_instance();
        con = driver->connect(sql_server, username, password); // MySQL 데이터베이스 연결 객체
    }
    catch (sql::SQLException& e) 
    {
        cout << "Could not connect to server. Error message: " << e.what() << endl;
        exit(1);
    }
    con->setSchema("chattings");

    stmt = con->createStatement();
    pstmt = con->prepareStatement("INSERT INTO users(user_id, user_pw, nickname, login) VALUES(?,?,?,?)");
    pstmt->setString(1, user_id);
    pstmt->setString(2, user_pw);
    pstmt->setString(3, nickname);
    pstmt->setInt(4, 0);
    pstmt->execute();
}
string in_chat_log(string user_id, char* chat_log) //chat = chat + time(ex: 20230507094224828)
{
    char* chat_log_p = &*chat_log;
    int j = 0;
    string time = "";
    string time_log = "";
    while (*chat_log)
        chat_log++;
    chat_log -= 17;

    while (*chat_log)
    {
        time.push_back(*chat_log);
        time_log.push_back(*chat_log);
        *chat_log = 0;
        chat_log++;
        switch (j)
        {
        case 3:
            time.push_back('-');
            break;
        case 5:
            time.push_back('-');
            break;
        case 7:
            time.push_back(' ');
            break;
        case 9:
            time.push_back(':');
            break;
        case 11:
            time.push_back(':');
            break;
        case 12:
        case 13:
        case 14:
            time.pop_back();
            break;
        }
        j++;
    }
    string sql_chat_log = string(chat_log_p);
    vector<string> v = { time, user_id, sql_chat_log };
    chat_log_list.push_back(v);

    return time;  //2023-05-07 09:42:24
}
int main() 
{
    WSADATA wsa;
    int code = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (!code)
    {
        server_init();
        std::thread th1[MAX_CLIENT];
        for (int i = 0; i < MAX_CLIENT; i++)
            th1[i] = std::thread(add_client, i);
        while (1)
        {
            string text, msg = "";
            std::getline(cin, text);
            const char* buf = text.c_str();
            msg = server_sock.user_id + ": " + buf + "\n";
            recv_send_msg(3, server_sock.sck, msg.c_str());
        }
        for (int i = 0; i < MAX_CLIENT; i++)
            th1[i].join();
        closesocket(server_sock.sck);
    }
    else
    {
        cout << "[Log] Server Error (Error code : " << code << ")";
    }
    WSACleanup();
    return 0;
}
void server_init()
{
    server_sock.sck = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    SOCKADDR_IN server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5476);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(server_sock.sck, (sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_sock.sck, SOMAXCONN);
    server_sock.user_id = "#SERVER#";
    cout << "[Log] Server On" << endl;
}
void add_client(int un_client)
{
    SOCKADDR_IN addr = {};
    int addrsize = sizeof(addr);
    string msg = "";
    string user_id, user_pw, nickname = "";
    char buf[MAX_SIZE] = { };
    ZeroMemory(&addr, addrsize);
    SOCKET_INFO new_client = {};
    new_client.sck = accept(server_sock.sck, (sockaddr*)&addr, &addrsize);
    cout << "[Log] Connect Unknown " << un_client << endl;
    //send(new_client.sck, "Welcome Chatting Server", MAX_SIZE, 0);
    recv_send_msg(2, new_client.sck, "Welcome Chatting Server");
    unclient_count++;
    while (1)
    {
        if (recv(new_client.sck, buf, MAX_SIZE, 0) > 0)
        {
            if (string(buf) == "login")
            {
                ZeroMemory(&buf, MAX_SIZE);
                cout << "[Log] Unknown " << un_client << " is trying login" << endl;
                while (1)
                {
                    user_id = recv_send_msg(1, new_client.sck, "");
                    user_pw = recv_send_msg(1, new_client.sck, "");
                    if (find_account(2, user_id, user_pw))
                    {
                        recv_send_msg(2, new_client.sck, "logsu");
                        new_client.user_id = user_id;
                        new_client.nickname = get_nickname(user_id);
                        recv_send_msg(2, new_client.sck, new_client.nickname);
                        break;
                    }
                    else
                        recv_send_msg(2, new_client.sck, "logfa");
                }
                break;
            }
            else if (string(buf) == "signin")
            {
                ZeroMemory(&buf, MAX_SIZE);
                cout << "[Log] Unknown " << un_client << " is trying sign in" << endl;
                while (1)
                {
                    user_id = recv_send_msg(1, new_client.sck, "");
                    if (find_account(1, user_id, ""))
                        recv_send_msg(2, new_client.sck, "useit");
                    else
                    {
                        recv_send_msg(2, new_client.sck, "usenotit");
                        break;
                    }
                }
                user_pw = recv_send_msg(1, new_client.sck, "");
                nickname = recv_send_msg(1, new_client.sck, "");
                in_account(user_id, user_pw, nickname);
                cout << "[Log] Unknown " << un_client << " Creat ID" << nickname << "(" << user_id << ")" << endl;
            }
        }
        else
        {
            closesocket(new_client.sck);
            cout << "[Log] Unconnect Unknown " << un_client << endl;
            return;
        }
    }
    cout << "[Log] Unknown " << un_client << " has entered as " << new_client.user_id << endl;
    sck_list.push_back(new_client); // client 정보를 담는 sck_list 배열에 새로운 client 추가
    recv_send_msg(2, new_client.sck, "[Log] ID " + new_client.user_id + " Connected" + "\n");
    std::thread th(recv_msg, client_count);
    client_count++;
    msg = "[Log] Now Client Number: " + to_string(client_count);
    for (int i = 0; i < client_count; i++)
    {
        msg += " " + sck_list[i].user_id;
    }
    msg += "\n";
    cout << msg << endl;
    recv_send_msg(3, new_client.sck, msg.c_str());
    recv_send_msg(4, new_client.sck, "");
    th.join();
}
string recv_send_msg(int mode, SOCKET sck, string msg)
{
    char buf[MAX_SIZE] = { };
    if (mode == 1)
    {
        recv(sck, buf, MAX_SIZE, 0);
        msg = (string)buf;
        return msg;
    }
    else if (mode == 2)
    {
        send(sck, msg.c_str(), MAX_SIZE, 0);
        return "";
    }
    else if (mode == 3)
    {
        for (int i = 0; i < client_count; i++) // 접속해 있는 모든 client에게 메시지 전송
            send(sck_list[i].sck, msg.c_str(), MAX_SIZE, 0);
        return "";
    }
    else if (mode == 4)
    {
        for (int i = 0; i < chat_log_list.size(); i++)
        {
            msg = chat_log_list[i][0] + " " + chat_log_list[i][1] + ": " + chat_log_list[i][2] + "\n";
            send(sck, msg.c_str(), MAX_SIZE, 0);
        }
        return "";
    }
}
void recv_msg(int idx)
{
    string msg = "";
    while (1)
    {
        char buf[MAX_SIZE] = { };
        if (recv(sck_list[idx].sck, buf, MAX_SIZE, 0) > 0)
        {
            string time = "";
            time = in_chat_log(sck_list[idx].user_id, buf);
            msg = time + " " + sck_list[idx].nickname + ": " + buf + "\n";
            cout << msg;
            recv_send_msg(3, server_sock.sck, msg.c_str());
        }
        else
        {
            msg = "[Log] " + sck_list[idx].user_id + " User Out" + "\n";
            cout << msg;
            recv_send_msg(3, server_sock.sck, msg.c_str());
            msg = "[Log] Now Client Number: " + to_string(client_count);
            for (int i = 0; i < client_count; i++)
                msg += " " + sck_list[i].user_id;
            msg += "\n";
            cout << msg;
            recv_send_msg(3, server_sock.sck, msg.c_str());
            find_account(3, sck_list[idx].user_id, "");
            del_client(idx);
            return;
        }
    }
}
void del_client(int idx)
{
    closesocket(sck_list[idx].sck);
    client_count--;
}