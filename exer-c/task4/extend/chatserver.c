#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define port 10140
#define max_user_num 5

typedef struct User {
    char name[128];
    char ip[128];
    int socket;
} User;

typedef struct Server {
    int socket;
    int user_num;
    User users[max_user_num];
} Server;

void fd_reset(Server* server, fd_set* fds);
int max_nfds(Server* server);
void null_last_char(char* str);
void new_user(Server* server, char* name, int socket, char* ip_str);
void pop_user(Server* server, int num);
void get_time_str(char* time_str, int len);

int main() {
    Server server;

    if ((server.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("socket");
        exit(1);
    }

    int reuse = 1;
    if (setsockopt(server.socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt");
        exit(1);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    if (bind(server.socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(1);
    }

    if (listen(server.socket, max_user_num * 2) < 0) {
        perror("listen");
        exit(1);
    }

    server.user_num = 0;

    fd_set rfds;
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    for (;;) {
        fd_reset(&server, &rfds);
        if (select(max_nfds(&server), &rfds, NULL, NULL, &tv) > 0) {
            if (FD_ISSET(server.socket, &rfds)) {
                int user_socket;
                struct sockaddr_in client;
                int clen = sizeof(client);
                if ((user_socket = accept(server.socket, (struct sockaddr*)&client, &clen)) < 0) {
                    perror("accept");
                    exit(2);
                }

                if (server.user_num >= max_user_num) {
                    printf("couldn't join because max people (%d) are currently participating.\n", max_user_num);
                    char dis_connect_str[18] = "REQUEST REJECTED\n";
                    write(user_socket, dis_connect_str, sizeof(dis_connect_str));
                    close(user_socket);
                    continue;
                }

                char connect_str[18] = "REQUEST ACCEPTED\n";
                write(user_socket, connect_str, sizeof(connect_str));

                char username[128];
                for (;;) {
                    ssize_t nbytes;
                    if ((nbytes = read(user_socket, username, sizeof(username))) < 0) {
                        perror("read");
                        exit(-1);
                    }
                    else if (nbytes == 0) {
                        printf("%s disconnected\n", inet_ntoa(client.sin_addr));
                        close(user_socket);
                        break;
                    }
                    null_last_char(username);

                    bool not_registed = true;
                    for (int i = 0; i < server.user_num; i++)
                        if (strcmp(server.users->name, username) == 0) not_registed = false;
                    if (!not_registed) {
                        printf("username : \"%s\" couldn't join because there was a participant with the same name.\n", username);
                        char not_regist_str[19] = "USERNAME REJECTED\n";
                        write(user_socket, not_regist_str, sizeof(not_regist_str));
                        continue;
                    }

                    char regist_str[21] = "USERNAME REGISTERED\n";
                    write(user_socket, regist_str, sizeof(regist_str));
                    printf("username : \"%s\" join!\n", username);
                    new_user(&server, username, user_socket, inet_ntoa(client.sin_addr));

                    char send_str[1024];
                    sprintf(send_str, "username : \"%s\" login!\n: <userlist>\n", username);
                    for (int i = 0; i < server.user_num; i++) {
                        strcat(send_str, ": ");
                        strcat(send_str, server.users[i].name);
                        if (server.user_num - 1 != i) strcat(send_str, "\n");
                    }
                    for (int i = 0; i < server.user_num; i++)
                        write(server.users[i].socket, send_str, sizeof(send_str));

                    break;
                }
                //close(user_socket);
            }
            for (int i = 0; i < server.user_num; i++)
                if (FD_ISSET(server.users[i].socket, &rfds)) {
                    char receive_str[512] = "";
                    ssize_t nbytes;
                    if ((nbytes = read(server.users[i].socket, receive_str, sizeof(receive_str))) < 0) {
                        perror("read");
                        break;
                    }
                    else if (nbytes == 0) {
                        printf("username : \"%s\" logout!\n", server.users[i].name);
                        char send_str[1024];
                        sprintf(send_str, "username : \"%s\" logout!\n: <userlist>\n", server.users[i].name);
                        pop_user(&server, i);
                        for (int i = 0; i < server.user_num; i++) {
                            strcat(send_str, ": ");
                            strcat(send_str, server.users[i].name);
                            if (server.user_num - 1 != i) strcat(send_str, "\n");
                        }
                        for (int i = 0; i < server.user_num; i++)
                            write(server.users[i].socket, send_str, sizeof(send_str));

                        continue;
                    }
                    if (strncmp(receive_str, "\\list", strlen("\\list")) == 0) {
                        bool is_ok = true;
                        for (int i = strlen("\\list"); receive_str[i] != '\0'; i++)
                            if (receive_str[i] != ' ') is_ok = false;
                        if (!is_ok) {
                            char send_str[1024] = "\x1b[31m<list> command takes no arguments!\x1b[39m";
                            write(server.users[i].socket, send_str, sizeof(send_str));
                            continue;
                        }
                        printf("receive list command from %s\n", server.users[i].name);
                        char send_str[1024] = ": <userlist>\n";
                        for (int i = 0; i < server.user_num; i++) {
                            strcat(send_str, ": ");
                            strcat(send_str, server.users[i].name);
                            if (server.user_num - 1 != i) strcat(send_str, "\n");
                        }
                        write(server.users[i].socket, send_str, sizeof(send_str));
                        continue;
                    }
                    if (strncmp(receive_str, "\\send", strlen("\\send")) == 0) {
                        int send_user_num;
                        bool is_ok = false;
                        for (int l = 0; l < server.user_num; l++)
                            if (strncmp(receive_str + strlen("\\send") + 1, server.users[l].name, strlen(server.users[l].name)) == 0) {
                                is_ok = true;
                                send_user_num = l;
                            }
                        if (!is_ok) {
                            char send_str[1024] = "\x1b[31m<send> command requires a valid user name!\nYou can look up the username using the <list> command.\x1b[39m";
                            write(server.users[i].socket, send_str, sizeof(send_str));
                            continue;
                        }
                        char send_str[1024];
                        sprintf(send_str, "direct message from %s > ", server.users[send_user_num].name);
                        strcat(send_str, receive_str + strlen("\\send") + strlen(server.users[send_user_num].name) + 2);
                        write(server.users[send_user_num].socket, send_str, sizeof(send_str));
                        continue;
                    }

                    char time_str[128];
                    get_time_str(time_str, sizeof(time_str));
                    char send_str[1024] = "";
                    strcat(send_str, time_str);
                    strcat(send_str, server.users[i].name);
                    strcat(send_str, " from ");
                    strcat(send_str, server.users[i].ip);
                    strcat(send_str, " > ");
                    strcat(send_str, receive_str);
                    for (int l = 0; l < server.user_num; l++) {
                        write(server.users[l].socket, send_str, sizeof(send_str));
                    }
                }
        }
    }
}

void fd_reset(Server* server, fd_set* fds) {
    FD_ZERO(fds);
    FD_SET(server->socket, fds);
    for (int i = 0; i < server->user_num; i++)
        FD_SET(server->users[i].socket, fds);
}

int max_nfds(Server* server) {
    int max = server->socket;
    for (int i = 0; i < max_user_num; i++) max = (max < server->users[i].socket) ? server->users[i].socket : max;
    return max + 1;
}

void null_last_char(char* str) {
    for (int i = 0;;i++)
        if (str[i] == '\n') {
            *(str + i) = '\0';
            return;
        }
}

void new_user(Server* server, char* name, int socket, char* ip_str) {
    strcpy(server->users[server->user_num].name, name);
    strcpy(server->users[server->user_num].ip, ip_str);
    server->users[server->user_num].socket = socket;
    server->user_num++;
}

void pop_user(Server* server, int num) {
    close(server->users[num].socket);
    for (int i = num + 1; i < server->user_num; i++) {
        server->users[i - 1] = server->users[i];
    }
    server->user_num--;
}

void get_time_str(char* time_str, int len) {
    time_t t = time(NULL);
    strftime(time_str, len, "%Y/%m/%d %a", localtime(&t));
    sprintf(time_str, "%s %02d:%02d:%02d\n", time_str, (t / 3600 + 9) % 24, t / 60 % 60, t % 60);
}