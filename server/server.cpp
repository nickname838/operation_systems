#include <iostream>
#include <csignal>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 1234

volatile sig_atomic_t wasSigHup = 0;

void sigHupHadler(int r)
{
    wasSigHup = 1;
}

void setupSigHupHandler(sigset_t *origMask)
{
    struct sigaction sa;
    sigaction(SIGHUP, NULL, &sa);
    sa.sa_handler = sigHupHadler;
    sa.sa_flags |= SA_RESTART;
    sigaction(SIGHUP, &sa, NULL);

    sigset_t blockedMask;
    sigemptyset(&blockedMask);
    sigaddset(&blockedMask, SIGHUP);
    sigprocmask(SIG_BLOCK, &blockedMask, origMask);
}

int main()
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) != 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    sigset_t origMask;
    setupSigHupHandler(&origMask);

    int client_fd = -1;

    while (1)
    {
        if (wasSigHup)
        {
            wasSigHup = 0;
            puts("sighup");
            continue;
        }

        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(server_fd, &fds);
        int maxFd = server_fd;

        if (client_fd != -1)
        {
            FD_SET(client_fd, &fds);
            if (client_fd > maxFd)
            {
                maxFd = client_fd;
            }
        }

        if (pselect(maxFd + 1, &fds, NULL, NULL, NULL, &origMask) < 0 && errno != EINTR)
        {
            // if (errno == EINTR)
            // {
            //     continue;
            // }
            perror("pselect");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(server_fd, &fds))
        {
            struct sockaddr_in client_addr;
            socklen_t len = sizeof(client_addr);
            int connect = accept(server_fd, (struct sockaddr *)&client_addr, &len);
            if (connect < 0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }
            else
            {
                client_fd = connect;
                puts("accept");
            }
        }
        if (client_fd != -1 && FD_ISSET(client_fd, &fds))
        {
            char buffer[1024]{};
            int read_len = read(client_fd, buffer, 1024);
            if (read_len > 0)
            {
                // buffer[1023] = 0;
                puts(buffer);
            }
            else
            {
                close(client_fd);
                // FD_CLR(client_fd, &fds);
                client_fd = -1;
                puts("close");
            }
        }
    }
}