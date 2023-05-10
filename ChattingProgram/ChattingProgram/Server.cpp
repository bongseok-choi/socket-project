#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#define MAX_SIZE 1024
#define MAX_CLIENT 3
using std::cout; using std::cin; using std::endl; using std::string; using std::to_string; using std::vector;
struct SOCKET_INFO
{
    SOCKET sck;
    string user_id;
    string nickname;
};
vector<SOCKET_INFO> sck_list;
SOCKET_INFO server_sock;
int client_count = 0;
int unclient_count = 0;
vector<vector<string>> account_list = { {"abc", "1234", "ABC", "0"}, {"bcd", "2345", "BCD", "0"} }; //id, pw, nickname, loginlog
vector<vector<string>> chat_log_list;
int find_account(int mode, string user_id, string user_pw);
void in_account(string user_id, string user_pw, string nickname);
void server_init();
void add_client(int un_clinet);
string recv_send_msg(int mode, SOCKET sck, string msg);
void recv_msg(int idx);
void del_client(int idx);
int find_account(int mode, string user_id, string user_pw) //mode = 1 find id / mode = 2 find id, pw / mode = 3 find nickname / mode = 4 log out
{
    for (int i = 0; i < account_list.size(); i++)
        if (user_id == account_list[i][0] && mode == 1)
            return 1;
        else if (user_id == account_list[i][0] && mode == 2)
        {
            if (user_pw == account_list[i][1])
            {
                if (account_list[i][3] == "0")
                {
                    account_list[i][3] = "1";
                    return 1;
                }
                else
                    return 0;
            }
        }
        else if (user_id == account_list[i][0] && mode == 3)
            return i;
        else if (user_id == account_list[i][0] && mode == 4)
        {
            account_list[i][3] = "0";
            return 0;
        }
    return 0;
}
string get_nickname(int idx)
{
    return account_list[idx][2];
}
void in_account(string user_id, string user_pw, string nickname)
{
    try {
        driver = sql::mysql::get_mysql_driver_instance();
        con = driver->connect(sql_server, username, password); // MySQL �����ͺ��̽� ���� ��ü
    }
    catch (sql::SQLException& e) {
        cout << "Could not connect to server. Error message: " << e.what() << endl;
        exit(1);
    }
    con->setSchema("chattings");
    // db �ѱ� ������ ���� ����
    stmt = con->createStatement();
    pstmt = con->prepareStatement("INSERT INTO users(user_id, user_pw, nickname, login) VALUES(?,?,?,?)");
    pstmt->setString(1, user_id);
    pstmt->setString(2, user_pw);
    pstmt->setString(3, nickname);
    pstmt->setInt(4, 0);
    pstmt->execute();
    std::cout << "ȸ������ �Ϸ�Ǿ����ϴ�." << endl;
}
string in_chat_log(string user_id, char* chat_log) //chat = chat + time(ex: 20230507094224828)
{
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
    chat_log -= 16;
    while (*chat_log)
    {
        if (*chat_log)
            *chat_log = 0;
        chat_log--;
        *chat_log = 0;
        chat_log++;
    }
    return time;  //2023-05-07 09:42:24
}
int main() {
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
                        new_client.nickname = get_nickname(find_account(3, user_id, ""));
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
    sck_list.push_back(new_client); // client ������ ��� sck_list �迭�� ���ο� client �߰�
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
        for (int i = 0; i < client_count; i++) // ������ �ִ� ��� client���� �޽��� ����
            send(sck_list[i].sck, msg.c_str(), MAX_SIZE, 0);
        return "";
    }
    else if (mode == 4)
    {
        for (int i = 0; i < chat_log_list.size(); i++)
        {
            msg = chat_log_list[i][0] + " " + chat_log_list[i][1] + " " + chat_log_list[i][2] + "\n";
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